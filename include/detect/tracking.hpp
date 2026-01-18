#ifndef A4_TRACKING_HPP
#define A4_TRACKING_HPP

#include <opencv2/opencv.hpp>
#include <vector>

// ===============================
// A4 Tracker structure
// ===============================
struct A4Tracker {
    bool initialized = false;

    // Corners
    std::vector<cv::Point2f> prevCorners;
    std::vector<cv::Point2f> currCorners;

    // Pose
    cv::Mat rvec, tvec;
    cv::Mat rvecFiltered, tvecFiltered;

    // State
    int lostFrames = 0;

    // ROI
    cv::Rect lastROI;
};

// ===============================
// API
// ===============================

// Init
void initTracking(
    A4Tracker& tracker,
    const std::vector<cv::Point2f>& corners,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    const cv::Rect& roi
);

// Optical Flow tracking
bool trackCornersLK(
    const cv::Mat& prevGray,
    const cv::Mat& currGray,
    A4Tracker& tracker
);

// Pose update + validation
bool updatePose(
    A4Tracker& tracker,
    const std::vector<cv::Point3f>& objectPts,
    const cv::Mat& K,
    const cv::Mat& dist,
    double maxReprojError
);

// Temporal smoothing
void smoothPoseEMA(
    A4Tracker& tracker,
    double alpha = 0.2
);

// ROI update
cv::Rect computeROI(
    const std::vector<cv::Point2f>& corners,
    const cv::Size& frameSize,
    int margin = 40
);

// Fallback logic
void handleTrackingState(
    A4Tracker& tracker,
    bool trackingOK,
    int maxLost = 10
);

// Main per-frame call
void processTracking(
    const cv::Mat& prevGray,
    const cv::Mat& currGray,
    A4Tracker& tracker,
    const std::vector<cv::Point3f>& objectPts,
    const cv::Mat& K,
    const cv::Mat& dist
);

#endif // A4_TRACKING_HPP
