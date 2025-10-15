#pragma once
#include <GL/glew.h>
#include <opencv2/core.hpp>

/**
 * @file texture.hpp
 * @brief Création et mise à jour de textures OpenGL depuis cv::Mat.
 */
namespace glx {

GLuint createTextureRGBA(int w, int h);
void   updateTextureRGBA(GLuint tex, const cv::Mat& rgbaFlipped);
GLuint createTextureFromMat(const cv::Mat& imgRGBorRGBA);

} // namespace glx
