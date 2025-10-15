#pragma once
#include <string>
#include <opencv2/core.hpp>

/**
 * @file calib.hpp
 * @brief Chargement et stockage des paramètres intrinsèques de caméra.
 */
namespace ar {

/**
 * @brief Structure de calibration caméra.
 */
struct Calibration {
  cv::Mat cameraMatrix;   //!< Matrice intrinsèque 3x3 (CV_64F)
  cv::Mat distCoeffs;     //!< Coefficients de distorsion (1xN, CV_64F)
};

/**
 * @brief Charge la calibration depuis un fichier YAML OpenCV.
 * @param filename Chemin du fichier (ex: ../data/camera.yaml)
 * @return Calibration (cameraMatrix + distCoeffs)
 * @throws std::runtime_error si le fichier est invalide
 */
Calibration loadCalibration(const std::string& filename);

} // namespace ar
