#include "ar/calib.hpp"
#include <opencv2/core.hpp>
#include <stdexcept>

namespace ar {

/**
 * @brief Charge les paramètres de calibration caméra depuis un fichier YAML/XML.
 * 
 * @param filename chemin du fichier de calibration.
 * @return Calibration structure contenant la matrice caméra et les coefficients de distorsion.
 * @throw std::runtime_error si le fichier est introuvable ou invalide.
 */
Calibration loadCalibration(const std::string& filename) {
  cv::FileStorage fs(filename, cv::FileStorage::READ);
  if (!fs.isOpened())
    throw std::runtime_error("Impossible d'ouvrir " + filename);

  Calibration c{};
  fs["camera_matrix"] >> c.cameraMatrix;
  fs["distortion_coefficients"] >> c.distCoeffs;

  // Vérifie que les deux matrices sont bien présentes
  if (c.cameraMatrix.empty() || c.distCoeffs.empty()) {
    throw std::runtime_error("Calibration invalide (camera_matrix / distortion_coefficients manquants)");
  }

  // Conversion en double pour éviter les erreurs de type
  c.cameraMatrix.convertTo(c.cameraMatrix, CV_64F);
  c.distCoeffs.convertTo(c.distCoeffs, CV_64F);

  return c;
}

} // namespace ar
