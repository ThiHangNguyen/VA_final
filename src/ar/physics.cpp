#include "ar/physics.hpp"
#include <cmath>
#include <algorithm> // pour std::max, std::min si besoin

namespace ar {

// --- La fonction de base (Maths pures) ---
void resolveWallCollision(glm::vec3& pos, glm::vec3& vel, float radius, 
                          float x1, float y1, float x2, float y2) {
    // Vecteur du mur
    glm::vec2 A(x1, y1);
    glm::vec2 B(x2, y2);
    glm::vec2 P(pos.x, pos.y);
    
    glm::vec2 AB = B - A;
    glm::vec2 AP = P - A;
    
    // Projection du point sur le segment (clampé entre 0 et 1)
    float lenSq = glm::dot(AB, AB);
    if (lenSq < 1e-6f) return; // Sécurité division par zéro

    float t = glm::dot(AP, AB) / lenSq;
    t = glm::clamp(t, 0.0f, 1.0f); 
    
    // Point le plus proche sur le mur
    glm::vec2 closest = A + t * AB;
    
    // Distance entre la balle et ce point
    glm::vec2 distVec = P - closest;
    float dist = glm::length(distVec);
    
    // Si on touche (distance < rayon effectif)
    if (dist < radius && dist > 1e-5f) {
        // Normale de collision
        glm::vec2 n = glm::normalize(distVec);
        
        // 1. Correction de position (on pousse la balle hors du mur)
        float penetration = radius - dist;
        P += n * penetration;
        pos.x = P.x;
        pos.y = P.y;
        
        // 2. Rebond (réflexion de la vitesse)
        glm::vec2 v2(vel.x, vel.y);
        float vDotN = glm::dot(v2, n);
        
        // On ne rebondit que si on va VERS le mur
        if (vDotN < 0) {
            float restitution = 0.6f; // Coefficient de rebond (0.6 = un peu mou)
            v2 -= (1.0f + restitution) * vDotN * n;
            vel.x = v2.x;
            vel.y = v2.y;
        }
    }
}

// --- La nouvelle fonction de gestion globale ---
void handleCollisions(glm::vec3& ballPos, 
                      glm::vec3& ballVel, 
                      float ballRadius,
                      const std::vector<std::array<float, 4>>& walls,
                      float wallThickness)
{
    // Calcul du rayon "augmenté" pour simuler l'épaisseur du mur
    float effectiveRadius = ballRadius + (wallThickness / 2.0f);

    for (const auto& s : walls) {
        resolveWallCollision(
            ballPos, 
            ballVel, 
            effectiveRadius, // On utilise le rayon augmenté
            s[0], s[1], s[2], s[3]
        );
    }
}

// --- 3. PHYSIQUE GLOBALE (Avec sécurité anti-bug) ---
void updatePhysics(const cv::Mat& rvec, 
                   float dt,
                   glm::vec3& ballPos, 
                   glm::vec3& ballVel, 
                   glm::mat4& ballRotationMatrix,
                   float ballRadius,
                   const std::vector<std::array<float, 4>>& walls,
                   float wallThickness)
{
    // --- A. Orientation & Gravité ---
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
    
    if (std::abs(ax) < 0.1f) ax = 0.0f;
    if (std::abs(ay) < 0.1f) ay = 0.0f;

    // --- B. Vitesse ---
    float accel = 2000.f; 
    float damping = 1.0f;
    ballVel.x += ax * accel * dt; 
    ballVel.y += ay * accel * dt; 
    ballVel *= 1.f / (1.f + damping * dt);

    // [SECURITE 1] Limite vitesse max (évite de traverser les murs trop vite)
    float maxSpeed = 3000.0f;
    if (glm::length(ballVel) > maxSpeed) {
        ballVel = glm::normalize(ballVel) * maxSpeed;
    }

    // --- C. Mouvement ---
    glm::vec3 deplacement = ballVel * dt;
    ballPos += deplacement;

    // --- D. Rotation ---
    float dist = glm::length(deplacement);
    if (dist > 0.0001f) {
        glm::vec3 axis = glm::cross(deplacement, glm::vec3(0,0,1));
        axis = glm::normalize(axis);
        float angle = dist / ballRadius; 
        ballRotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis) * ballRotationMatrix;
    }

    // --- E. Collisions Murs ---
    handleCollisions(ballPos, ballVel, ballRadius, walls, wallThickness);
    
    // --- [SECURITE 2] --- FILET DE SAUVETAGE ---
    // Si la balle a traversé un mur à cause d'un lag, on la force à rentrer.
    // Limites A4 (210x297) moins l'épaisseur des murs
    float limitX = 105.0f - (wallThickness * 0.5f) - ballRadius;
    float limitY = 148.5f - (wallThickness * 0.5f) - ballRadius;

    // Si on dépasse X
    if (ballPos.x > limitX)  { ballPos.x = limitX;  if(ballVel.x > 0) ballVel.x = -ballVel.x * 0.5f; }
    if (ballPos.x < -limitX) { ballPos.x = -limitX; if(ballVel.x < 0) ballVel.x = -ballVel.x * 0.5f; }
    
    // Si on dépasse Y
    if (ballPos.y > limitY)  { ballPos.y = limitY;  if(ballVel.y > 0) ballVel.y = -ballVel.y * 0.5f; }
    if (ballPos.y < -limitY) { ballPos.y = -limitY; if(ballVel.y < 0) ballVel.y = -ballVel.y * 0.5f; }

    // --- F. Sol ---
    if (ballPos.z < ballRadius) ballPos.z = ballRadius;
}

} // namespace ar