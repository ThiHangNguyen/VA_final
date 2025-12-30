#include "ar/physics.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>  

namespace ar {

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
    float t = glm::dot(AP, AB) / lenSq;
    t = glm::clamp(t, 0.0f, 1.0f); 
    
    // Point le plus proche sur le mur
    glm::vec2 closest = A + t * AB;
    
    // Distance entre la balle et ce point
    glm::vec2 distVec = P - closest;
    float dist = glm::length(distVec);
    
    // Si on touche (distance < rayon)
    if (dist < radius && dist > 0.0001f) {
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
        
        // On ne rebondit que si on va vers le mur
        if (vDotN < 0) {
            float restitution = 0.5f; // Rebond mou
            v2 -= (1.0f + restitution) * vDotN * n;
            vel.x = v2.x;
            vel.y = v2.y;
        }
    }
}

} // namespace ar