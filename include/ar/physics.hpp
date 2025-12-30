#pragma once
#include <glm/glm.hpp>

namespace ar {

/**
 * @brief Gère la collision d'une sphère (balle) avec un segment (mur).
 * Modifie la position et la vitesse en cas de rebond.
 */
void resolveWallCollision(glm::vec3& pos, glm::vec3& vel, float radius, 
                          float x1, float y1, float x2, float y2);

} // namespace ar