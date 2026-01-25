/**
 * @file physics.cpp
 * @brief Gestion de la physique de la balle dans le plan AR.
 *
 * Ce fichier implémente la logique physique principale du jeu :
 * - calcul de la gravité projetée sur le plan détecté (A4)
 * - intégration du mouvement de la balle
 * - gestion des collisions avec les murs du labyrinthe
 * - gestion des collisions avec les bords de la feuille A4
 * - application de paramètres utilisateur (vitesse, rebond)
 *
 * Le système inclut plusieurs mécanismes de stabilisation :
 * - filtrage passe-bas (EMA) de l'accélération pour réduire le jitter AR
 * - sous-échantillonnage du mouvement (sub-stepping) pour éviter le tunneling
 * - limitation de la vitesse maximale
 *
 * La physique est découplée du rendu et repose sur les données
 * de pose caméra (rvec) issues du pipeline de détection AR.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */


#include "ar/physics.hpp"
#include "ar/filter.hpp"
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/calib3d.hpp>

namespace ar {

// --- La fonction de base (Maths pures) ---
// Inchangée, elle gère la logique d'un segment
void resolveWallCollision(glm::vec3& pos, glm::vec3& vel, float radius, 
                          float x1, float y1, float x2, float y2) {
    glm::vec2 A(x1, y1);
    glm::vec2 B(x2, y2);
    glm::vec2 P(pos.x, pos.y);
    
    glm::vec2 AB = B - A;
    glm::vec2 AP = P - A;
    
    float lenSq = glm::dot(AB, AB);
    if (lenSq < 1e-6f) return;

    float t = glm::dot(AP, AB) / lenSq;
    t = glm::clamp(t, 0.0f, 1.0f); 
    
    glm::vec2 closest = A + t * AB;
    glm::vec2 distVec = P - closest;
    float dist = glm::length(distVec);
    
    if (dist < radius && dist > 1e-5f) {
        glm::vec2 n = glm::normalize(distVec);
        
        float penetration = radius - dist;
        P += n * penetration;
        pos.x = P.x;
        pos.y = P.y;
        
        glm::vec2 v2(vel.x, vel.y);
        float vDotN = glm::dot(v2, n);
        
        if (vDotN < 0) {
            float restitution = 0.6f; 
            v2 -= (1.0f + restitution) * vDotN * n;
            vel.x = v2.x;
            vel.y = v2.y;
        }
    }
}

// --- MISE À JOUR : Gestion globale avec la structure Wall ---
void handleCollisions(glm::vec3& ballPos, 
                      glm::vec3& ballVel, 
                      float ballRadius,
                      const std::vector<glx::Wall>& walls, // Changé ici
                      float wallThickness)
{
    float effectiveRadius = ballRadius + (wallThickness / 2.0f);

    for (const auto& wall : walls) {
        // On utilise wall.start et wall.end au lieu de s[0], s[1]...
        resolveWallCollision(
            ballPos, 
            ballVel, 
            effectiveRadius,
            wall.start.x, wall.start.y, 
            wall.end.x,   wall.end.y
        );
    }
}

// --- PHYSIQUE GLOBALE ---
void updatePhysics(const cv::Mat& rvec, 
                   float dt,
                   glm::vec3& ballPos, 
                   glm::vec3& ballVel, 
                   glm::mat4& ballRotationMatrix,
                   float ballRadius,
                   const std::vector<glx::Wall>& walls, 
                   float wallThickness,
                   float paramAccel,       //  Paramètre Vitesse/Gravité
                   float paramRestitution  //  Paramètre Rebond
                   )
{
    // 1. Gravité & Orientation
    cv::Mat Rcv;
    cv::Rodrigues(rvec, Rcv);
    glm::vec3 X(Rcv.at<double>(0,0), Rcv.at<double>(1,0), Rcv.at<double>(2,0));
    glm::vec3 Y(Rcv.at<double>(0,1), Rcv.at<double>(1,1), Rcv.at<double>(2,1));
    glm::vec3 N(Rcv.at<double>(0,2), Rcv.at<double>(1,2), Rcv.at<double>(2,2));
    
    X = glm::normalize(X); Y = glm::normalize(Y); N = glm::normalize(N);
    glm::vec3 gCam(0.f, 0.f, 1.f);
    glm::vec3 gPlane = gCam - glm::dot(gCam, N) * N;
    
    float ax = glm::dot(gPlane, X);
    float ay = glm::dot(gPlane, Y);

    if (std::abs(ax) < 0.05f) ax = 0.0f;
    if (std::abs(ay) < 0.05f) ay = 0.0f;

    // ANTI-JITTER : Filtre passe-bas sur l'accélération
    // Réduit les tremblotements causés par le tracking bruité
    static LowPassFilter2D accelFilter(0.2f); // alpha=0.2 : bon compromis jitter/latence
    glm::vec2 accel_filtered = accelFilter.update(glm::vec2(ax, ay));

    // 2. Physique avec PARAMÈTRES et accélération filtrée
    ballVel.x += accel_filtered.x * paramAccel * dt;
    ballVel.y += accel_filtered.y * paramAccel * dt; 
    
    // Friction (Damping)
    float damping = 0.98f; 
    ballVel *= (1.0f - (1.0f - damping) * 60.0f * dt);

    float maxSpeed = 800.0f; // Réduit pour éviter le tunneling
    if (glm::length(ballVel) > maxSpeed) ballVel = glm::normalize(ballVel) * maxSpeed;

    // 3. Intégration avec SUB-STEPPING (anti-tunneling)
    // On découpe le mouvement en petits pas pour ne jamais sauter un mur
    glm::vec3 totalStep = ballVel * dt;
    float totalDist = glm::length(totalStep);

    // Taille max d'un pas = rayon de la balle (garantit qu'on ne traverse pas un mur)
    float maxStepSize = ballRadius * 0.5f;
    int numSteps = std::max(1, (int)std::ceil(totalDist / maxStepSize));

    glm::vec3 stepPerIter = totalStep / (float)numSteps;

    for (int i = 0; i < numSteps; i++) {
        ballPos += stepPerIter;

        // Collision à chaque sous-pas
        handleCollisions(ballPos, ballVel, ballRadius, walls, wallThickness);
    }

    // 4. Rotation visuelle
    float distMoved = totalDist;
    if (distMoved > 0.001f) {
        glm::vec3 axis = glm::cross(totalStep, glm::vec3(0,0,1));
        if (glm::length(axis) > 0.001f) {
            float angle = distMoved / ballRadius;
            ballRotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis)) * ballRotationMatrix;
        }
    }

    // 5. Collisions Bords A4 (déjà géré dans la boucle pour les murs)
    
    // Bords A4
    float limitX = 105.0f - (wallThickness/2) - ballRadius;
    float limitY = 148.5f - (wallThickness/2) - ballRadius;
    
    // Facteur de rebond (négatif pour inverser la direction)
    float bounce = -paramRestitution;

    if (ballPos.x > limitX)  { ballPos.x = limitX;  ballVel.x *= bounce; }
    if (ballPos.x < -limitX) { ballPos.x = -limitX; ballVel.x *= bounce; }
    if (ballPos.y > limitY)  { ballPos.y = limitY;  ballVel.y *= bounce; }
    if (ballPos.y < -limitY) { ballPos.y = -limitY; ballVel.y *= bounce; }
    
    ballPos.z = ballRadius;
}

} // namespace ar