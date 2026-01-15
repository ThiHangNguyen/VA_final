#include "ar/physics.hpp"
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

    // 2. Physique avec PARAMÈTRES
    ballVel.x += ax * paramAccel * dt; // Utilise paramAccel
    ballVel.y += ay * paramAccel * dt; 
    
    // Friction (Damping)
    float damping = 0.98f; 
    ballVel *= (1.0f - (1.0f - damping) * 60.0f * dt);

    float maxSpeed = 2500.0f;
    if (glm::length(ballVel) > maxSpeed) ballVel = glm::normalize(ballVel) * maxSpeed;

    // 3. Intégration
    glm::vec3 step = ballVel * dt;
    ballPos += step;

    // 4. Rotation visuelle
    float distMoved = glm::length(step);
    if (distMoved > 0.001f) {
        glm::vec3 axis = glm::cross(step, glm::vec3(0,0,1));
        if (glm::length(axis) > 0.001f) {
            float angle = distMoved / ballRadius;
            ballRotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis)) * ballRotationMatrix;
        }
    }

    // 5. Collisions Murs & Bords avec REBOND
    handleCollisions(ballPos, ballVel, ballRadius, walls, wallThickness);
    
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