#pragma once
#include <opencv2/core.hpp>
#include <glm/glm.hpp>
#include <vector>

/**
 * @file pose.hpp
 * @brief Conversion pose OpenCV -> matrices GLM, projection à partir de K.
 */
namespace ar {

/**
 * @brief Crée la matrice de projection OpenGL à partir des intrinsics OpenCV.
 * @param K Matrice intrinsèque 3x3 (CV_64F)
 * @param w Largeur du viewport/framebuffer (pixels)
 * @param h Hauteur du viewport/framebuffer (pixels)
 * @param n Plan proche (>0)
 * @param f Plan lointain (>n)
 */
glm::mat4 projectionFromCV(const cv::Mat& K, float w, float h, float n, float f);

/**
 * @brief Construit une View matrix OpenGL (column-major) depuis rvec/tvec OpenCV.
 * @param rvec Vecteur de rotation 3x1 (CV_64F) – Rodrigues
 * @param tvec Vecteur de translation 3x1 (CV_64F)
 * @return Matrice 4x4 GLM view.
 */
glm::mat4 viewFromRvecTvec(const cv::Mat& rvec, const cv::Mat& tvec);

} // namespace ar
