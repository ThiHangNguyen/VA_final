#include "ar/pose.hpp"
#include <opencv2/calib3d.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ar {

glm::mat4 projectionFromCV(const cv::Mat& K, float w, float h, float n, float f) {
  const double fx = K.at<double>(0,0);
  const double fy = K.at<double>(1,1);
  const double cx = K.at<double>(0,2);
  const double cy = K.at<double>(1,2);

  glm::mat4 P(0.0f);
  P[0][0] =  2.0f * static_cast<float>(fx) / w;
  P[1][1] =  2.0f * static_cast<float>(fy) / h;
  P[2][0] =  1.0f - 2.0f * static_cast<float>(cx) / w;
  P[2][1] =  2.0f * static_cast<float>(cy) / h - 1.0f;
  P[2][2] = -(f + n) / (f - n);
  P[2][3] = -1.0f;
  P[3][2] = -2.0f * f * n / (f - n);
  return P;
}

glm::mat4 viewFromRvecTvec(const cv::Mat& rvec, const cv::Mat& tvec) {
  if (rvec.empty() || tvec.empty()) return glm::mat4(1.0f);

  cv::Mat R; cv::Rodrigues(rvec, R);
  cv::Mat view = cv::Mat::eye(4, 4, CV_64F);
  R.copyTo(view(cv::Rect(0,0,3,3)));
  tvec.copyTo(view(cv::Rect(3,0,1,3)));

  // OpenCV (x→, y↓, z→) -> OpenGL (x→, y↑, z←)
  cv::Mat cvToGl = (cv::Mat_<double>(4,4) <<
    1,  0,  0, 0,
    0, -1,  0, 0,
    0,  0, -1, 0,
    0,  0,  0, 1);
  view = cvToGl * view;

  cv::Mat view32f; view.convertTo(view32f, CV_32F);
  cv::Mat view32fT = view32f.t();
  return glm::make_mat4(view32fT.ptr<float>());
}

} // namespace ar
