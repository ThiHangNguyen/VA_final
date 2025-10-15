#include "ar/calib.hpp"
#include <opencv2/core.hpp>
#include <stdexcept>

namespace ar {

Calibration loadCalibration(const std::string& filename) {
  cv::FileStorage fs(filename, cv::FileStorage::READ);
  if (!fs.isOpened()) throw std::runtime_error("Impossible d'ouvrir " + filename);

  Calibration c{};
  fs["camera_matrix"] >> c.cameraMatrix;
  fs["distortion_coefficients"] >> c.distCoeffs;

  if (c.cameraMatrix.empty() || c.distCoeffs.empty()) {
    throw std::runtime_error("Calibration invalide (camera_matrix / distortion_coefficients manquants)");
  }
  c.cameraMatrix.convertTo(c.cameraMatrix, CV_64F);
  c.distCoeffs.convertTo(c.distCoeffs, CV_64F);
  return c;
}

} // namespace ar
