/**
 * @file physics.hpp
 * @brief Simulation physique de la bille sur plan incline.
 *
 * Ce module gere le deplacement de la bille en fonction de l'inclinaison
 * du marqueur A4 detecte. On projette la gravite sur le plan puis on
 * integre la position avec un schema semi-implicite (stable a tous les FPS).
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "glx/mesh.hpp"

namespace ar {

/**
 * @brief Met a jour la physique de la bille pour une frame.
 *
 * Pipeline:
 * 1. Extraction de l'orientation du plan depuis rvec (Rodrigues)
 * 2. Calcul du vecteur "up" du plan pour projeter la gravite
 * 3. Integration semi-implicite: vel += accel*dt, pos += vel*dt
 * 4. Application du damping (friction)
 * 5. Detection et reponse aux collisions avec les murs
 * 6. Mise a jour de la matrice de rotation (effet de roulement)
 *
 * @param rvec         Vecteur rotation OpenCV (3x1) issu de solvePnP
 * @param dt           Delta time en secondes (on clamp a 0.05 max pour eviter les explosions)
 * @param ballPos      Position 3D de la bille (modifiee)
 * @param ballVel      Vitesse 3D de la bille (modifiee)
 * @param ballRotationMatrix  Matrice rotation pour l'effet visuel de roulement
 * @param ballRadius   Rayon de la sphere
 * @param walls        Liste des murs du labyrinthe
 * @param wallThickness Epaisseur des murs pour les collisions
 * @param paramAccel   Facteur d'acceleration (sensibilite a l'inclinaison)
 * @param paramRestitution Coefficient de rebond [0..1]
 */
void updatePhysics(const cv::Mat& rvec,
                   float dt,
                   glm::vec3& ballPos,
                   glm::vec3& ballVel,
                   glm::mat4& ballRotationMatrix,
                   float ballRadius,
                   const std::vector<glx::Wall>& walls,
                   float wallThickness,
                   float paramAccel,
                   float paramRestitution);

/**
 * @brief Gere les collisions bille-murs avec correction de penetration.
 *
 * Pour chaque mur, on calcule la distance point-segment et on verifie
 * si la bille penetre. Si oui:
 * - On repousse la bille hors du mur
 * - On inverse la composante normale de la vitesse (rebond)
 * - On applique le coefficient de restitution
 *
 * @param ballPos      Position de la bille (modifiee si collision)
 * @param ballVel      Vitesse de la bille (modifiee si rebond)
 * @param ballRadius   Rayon de la sphere
 * @param walls        Murs du labyrinthe
 * @param wallThickness Epaisseur des murs
 */
void handleCollisions(glm::vec3& ballPos,
                      glm::vec3& ballVel,
                      float ballRadius,
                      const std::vector<glx::Wall>& walls,
                      float wallThickness);

} // namespace ar

#endif