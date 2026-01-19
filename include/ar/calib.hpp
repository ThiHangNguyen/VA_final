#pragma once
#include <string>
#include <opencv2/core.hpp>

/**
 * @file calib.hpp
 * @brief Chargement des parametres de calibration camera depuis YAML.
 *
 * La calibration contient la matrice intrinseque (fx, fy, cx, cy)
 * et les coefficients de distorsion. Ces valeurs sont necessaires
 * pour solvePnP et la projection 3D.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
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
