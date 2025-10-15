#include "ar/temporal.hpp"
#include <cmath>

namespace ar {

float PoseTemporalFilter::rmseReproj(const std::vector<cv::Point3f>& obj,
                                     const std::vector<cv::Point2f>& img,
                                     const cv::Mat& K, const cv::Mat& D,
                                     const cv::Mat& rvec, const cv::Mat& tvec){
  std::vector<cv::Point2f> proj;
  cv::projectPoints(obj, rvec, tvec, K, D, proj);
  double s = 0.0;
  for (size_t i=0;i<img.size();++i){
    cv::Point2f d = proj[i] - img[i];
    s += d.dot(d);
  }
  return std::sqrt(s / std::max<size_t>(1, img.size()));
}

float PoseTemporalFilter::angleDegBetween(const cv::Mat& r1, const cv::Mat& r2){
  cv::Mat R1, R2; cv::Rodrigues(r1,R1); cv::Rodrigues(r2,R2);
  cv::Mat R = R2 * R1.t();
  double trace = (R.at<double>(0,0)+R.at<double>(1,1)+R.at<double>(2,2));
  trace = std::clamp(trace, -1.0, 3.0);
  double ang = std::acos(std::clamp((trace-1.0)/2.0, -1.0, 1.0));
  return (float)(ang * 180.0 / CV_PI);
}

bool PoseTemporalFilter::validateAndFilter(const std::vector<cv::Point3f>& obj,
                                           const std::vector<cv::Point2f>& img,
                                           const cv::Mat& K, const cv::Mat& D,
                                           cv::Mat& rvec, cv::Mat& tvec)
{
  // 1) reprojection RMSE
  const float rmse = rmseReproj(obj, img, K, D, rvec, tvec);
  if (rmse > params_.rmseMaxPx) return false;

  if (!hasPrev_){
    rPrev_ = rvec.clone(); tPrev_ = tvec.clone(); hasPrev_ = true; return true;
  }

  // 2) garde-fous (delta translation/rotation entre frames)
  cv::Mat dt = tvec - tPrev_;
  float deltaT = (float)cv::norm(dt);
  float deltaR = angleDegBetween(rPrev_, rvec);
  if (deltaT > params_.maxDeltaTrans || deltaR > params_.maxDeltaDeg){
    // trop brusque => rejeter cette mise à jour
    return false;
  }

  // 3) EMA
  const float a = params_.emaAlpha;
  rvec = a*rvec + (1-a)*rPrev_;
  tvec = a*tvec + (1-a)*tPrev_;

  rPrev_ = rvec.clone(); tPrev_ = tvec.clone();
  return true;
}

} // namespace ar
