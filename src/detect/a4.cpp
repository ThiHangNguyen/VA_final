#include "detect/a4.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace detect {

static inline double sqr(double x){ return x*x; }

bool orderFourCorners(const std::vector<cv::Point>& approx,
                      std::vector<cv::Point2f>& ordered) {
  if (approx.size() != 4) return false;
  std::vector<cv::Point2f> pts(4);
  for (int i=0;i<4;i++) pts[i] = approx[i];

  // Tri: d'abord par y (haut -> bas), puis par x (gauche -> droite)
  std::sort(pts.begin(), pts.end(), [](const cv::Point2f& a, const cv::Point2f& b){
    if (std::abs(a.y - b.y) > 1e-3) return a.y < b.y;
    return a.x < b.x;
  });

  std::vector<cv::Point2f> top = { pts[0], pts[1] };
  std::vector<cv::Point2f> bot = { pts[2], pts[3] };

  // top[0]=TL, top[1]=TR ; bot[0]=BL, bot[1]=BR
  if (top[0].x > top[1].x) std::swap(top[0], top[1]);
  if (bot[0].x > bot[1].x) std::swap(bot[0], bot[1]);

  // *** Ordre final demandé: TL, BL, BR, TR ***
  ordered = { top[0], bot[0], bot[1], top[1] };
  return true;
}

bool detectA4Corners(const cv::Mat& frameBGR,
                     std::vector<cv::Point2f>& imagePts) {
  cv::Mat gray, blurred, thresh;
  cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);
  cv::GaussianBlur(gray, blurred, cv::Size(7,7), 0);

  const int H = blurred.rows, W = blurred.cols;
  const int rw = W/2, rh = H/2;
  const int rx = (W-rw)/2, ry = (H-rh)/2;
  const cv::Rect roi(rx, ry, rw, rh);
  const double meanROI = cv::mean(blurred(roi))[0];
  const int offset = 15;
  const double T = std::clamp(meanROI - offset, 0.0, 255.0);
  cv::threshold(blurred, thresh, T, 255, cv::THRESH_BINARY);

  std::vector<std::vector<cv::Point>> contours; std::vector<cv::Vec4i> hier;
  cv::findContours(thresh, contours, hier, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (contours.empty()) return false;

  double maxA = 0; int maxIdx = -1;
  for (int i=0;i<(int)contours.size();++i){
    const double A = std::fabs(cv::contourArea(contours[i]));
    if (A > maxA){ maxA = A; maxIdx = i; }
  }
  if (maxIdx < 0) return false;

  std::vector<cv::Point> approx;
  const double eps = 0.02 * cv::arcLength(contours[maxIdx], true);
  cv::approxPolyDP(contours[maxIdx], approx, eps, true);

  if (approx.size() != 4) return false;
  return orderFourCorners(approx, imagePts);
}

void drawOrderedCorners(cv::Mat& img, const std::vector<cv::Point2f>& pts) {
  if (pts.size()!=4) return;

  // TL, BL, BR, TR
  const cv::Scalar colors[4] = {
    {0,0,255},   // TL red
    {0,255,255}, // BL yellow
    {255,0,0},   // BR blue
    {0,255,0}    // TR green
  };
  const char* labels[4] = {"TL","BL","BR","TR"};

  for (int i=0;i<4;i++){
    cv::circle(img, pts[i], 8, colors[i], -1, cv::LINE_AA);
    cv::putText(img, labels[i], pts[i] + cv::Point2f(10,-10),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, colors[i], 2, cv::LINE_AA);
  }
}

} // namespace detect
