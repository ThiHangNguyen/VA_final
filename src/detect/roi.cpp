#include "detect/roi.hpp"
#include <algorithm>
#include <numeric>

namespace detect {

cv::Rect AdaptiveROI::clampRect(const cv::Rect& r, const cv::Size& sz){
  int x = std::max(0, r.x);
  int y = std::max(0, r.y);
  int w = std::min(r.width,  sz.width  - x);
  int h = std::min(r.height, sz.height - y);
  return {x,y,w,h};
}

void AdaptiveROI::init(const cv::Size& frameSize, const cv::Rect& initBox){
  frameSize_ = frameSize;
  if (initBox.area() > 0) roi_ = clampRect(initBox, frameSize_);
  else                    roi_ = {0,0, frameSize_.width, frameSize_.height};
  lostCount_ = 0;
}

void AdaptiveROI::reset(){
  roi_ = {0,0, frameSize_.width, frameSize_.height};
  lostCount_ = 0;
}

void AdaptiveROI::updateWithCorners(const std::vector<cv::Point2f>& corners4){
  if (corners4.size()!=4) return;
  float minx=1e9f, miny=1e9f, maxx=-1e9f, maxy=-1e9f;
  for (auto&p: corners4){ minx=std::min(minx,p.x); miny=std::min(miny,p.y); maxx=std::max(maxx,p.x); maxy=std::max(maxy,p.y); }
  int pad = params_.padPx;
  cv::Rect nb( (int)std::floor(minx)-pad,
               (int)std::floor(miny)-pad,
               (int)std::ceil (maxx-minx)+2*pad,
               (int)std::ceil (maxy-miny)+2*pad );
  nb = clampRect(nb, frameSize_);
  if (nb.width < params_.minW) { int cx=nb.x+nb.width/2; nb.x = std::max(0, cx-params_.minW/2); nb.width = std::min(params_.minW, frameSize_.width-nb.x); }
  if (nb.height< params_.minH) { int cy=nb.y+nb.height/2; nb.y = std::max(0, cy-params_.minH/2); nb.height= std::min(params_.minH, frameSize_.height-nb.y);}
  roi_ = nb;
  lostCount_ = 0;
}

void AdaptiveROI::markLost(){
  lostCount_++;
  if (lostCount_ >= params_.lostReset) reset();
  // sinon on garde la ROI courante (ça évite de sauter au plein cadre trop vite)
}

bool AdaptiveROI::trackLK(const cv::Mat& prevGray, const cv::Mat& gray,
                          const std::vector<cv::Point2f>& prevPts4,
                          std::vector<cv::Point2f>& nextPts4) const {
  if (prevGray.empty() || gray.empty() || prevPts4.size()!=4) return false;
  std::vector<cv::Point2f> next;
  std::vector<unsigned char> status;
  std::vector<float> err;

  cv::calcOpticalFlowPyrLK(prevGray, gray, prevPts4, next, status, err,
                           cv::Size(21,21), 3,
                           {cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, 0.01},
                           0, 1e-4);

  // Valider qu’on a au moins 3/4 bons points
  int good=0; nextPts4.resize(4);
  for (int i=0;i<4;i++){
    if (i<(int)status.size() && status[i]){ nextPts4[i]=next[i]; good++; }
  }
  return good>=3;
}

void AdaptiveROI::drawDebug(cv::Mat& bgr) const{
  if (!params_.debug) return;
  cv::rectangle(bgr, roi_, {0,255,255}, 2, cv::LINE_AA);
  cv::putText(bgr, "ROI", {roi_.x+5, roi_.y+20}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {0,255,255}, 2, cv::LINE_AA);
}

} // namespace detect
