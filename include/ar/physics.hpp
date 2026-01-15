#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "glx/mesh.hpp" // OBLIGATOIRE pour accéder à glx::Wall

namespace ar {

// Mise à jour de la signature pour accepter std::vector<glx::Wall>
void updatePhysics(const cv::Mat& rvec, 
                   float dt,
                   glm::vec3& ballPos, 
                   glm::vec3& ballVel, 
                   glm::mat4& ballRotationMatrix,
                   float ballRadius,
                   const std::vector<glx::Wall>& walls,  
                   float wallThickness);

// Si handleCollisions est déclaré dans le hpp, mettez-le à jour aussi :
void handleCollisions(glm::vec3& ballPos, 
                      glm::vec3& ballVel, 
                      float ballRadius,
                      const std::vector<glx::Wall>& walls,
                      float wallThickness);

}

#endif