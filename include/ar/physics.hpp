#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Pour glm::mat4
#include <opencv2/opencv.hpp>           // Pour cv::Mat
#include <vector>
#include <array>

namespace ar {

void resolveWallCollision(glm::vec3& pos, glm::vec3& vel, float radius, 
                          float x1, float y1, float x2, float y2);

void handleCollisions(glm::vec3& ballPos, 
                      glm::vec3& ballVel, 
                      float ballRadius,
                      const std::vector<std::array<float, 4>>& walls,
                      float wallThickness);

/**
 * @brief Calcule TOUTE la physique : Mouvement, Gravité, Rotation et Collisions.
 */
void updatePhysics(const cv::Mat& rvec, 
                   float dt,
                   glm::vec3& ballPos, 
                   glm::vec3& ballVel, 
                   glm::mat4& ballRotationMatrix,
                   float ballRadius,
                   const std::vector<std::array<float, 4>>& walls,
                   float wallThickness);

} // namespace ar