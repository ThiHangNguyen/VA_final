#pragma once
/**
 * @file temporal.hpp
 * @brief Validation temporelle: reprojection RMSE + EMA + garde-fous (translation/rotation).
 */

#include <opencv2/opencv.hpp>
#include <vector>

namespace ar {

struct TemporalParams {
  float rmseMaxPx     = 3.0f;   ///< reprojection RMSE max
  float emaAlpha      = 0.7f;   ///< lissage rvec/tvec
  float maxDeltaTrans = 120.f;  ///< mm/frame
  float maxDeltaDeg   = 20.f;   ///< deg/frame
};

class PoseTemporalFilter {
public:
  PoseTemporalFilter() = default;
  void setParams(const TemporalParams& p){ params_ = p; }

  /// Valide et filtre la pose. Renvoie true si la mise à jour est acceptée.
  bool validateAndFilter(const std::vector<cv::Point3f>& obj,
                         const std::vector<cv::Point2f>& img,
                         const cv::Mat& K, const cv::Mat& D,
                         cv::Mat& rvec, cv::Mat& tvec);

private:
  bool hasPrev_ = false;
  cv::Mat rPrev_, tPrev_;
  TemporalParams params_{};

  static float rmseReproj(const std::vector<cv::Point3f>& obj,
                          const std::vector<cv::Point2f>& img,
                          const cv::Mat& K, const cv::Mat& D,
                          const cv::Mat& rvec, const cv::Mat& tvec);
  static float angleDegBetween(const cv::Mat& r1, const cv::Mat& r2);
};

} // namespace ar
