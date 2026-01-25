#include "detect/tracking.hpp"

void initTracking(
    A4Tracker& tracker,
    const std::vector<cv::Point2f>& corners,
    const cv::Mat& rvec,
    const cv::Mat& tvec
) {
    tracker.prevCorners = corners;
    tracker.currCorners = corners;
    tracker.rvec = rvec.clone();
    tracker.tvec = tvec.clone();
    tracker.rvecFiltered = rvec.clone();
    tracker.tvecFiltered = tvec.clone();
    tracker.lostFrames = 0;
    tracker.initialized = true;
}

void processTracking(
    const cv::Mat& prevGray,
    const cv::Mat& currGray,
    A4Tracker& tracker,
    const std::vector<cv::Point3f>& objectPts,
    const cv::Mat& K,
    const cv::Mat& dist
) {
    if (!tracker.initialized) return;

    std::vector<uchar> status;
    std::vector<float> err;

    cv::calcOpticalFlowPyrLK(
        prevGray, currGray,
        tracker.prevCorners,
        tracker.currCorners,
        status, err,
        cv::Size(21,21), 3
    );

    int valid = 0;
    for (auto s : status) if (s) valid++;

    if (valid < 4) {
        if (++tracker.lostFrames > 8)
            tracker.initialized = false;
        return;
    }

    bool ok = cv::solvePnP(
        objectPts,
        tracker.currCorners,
        K, dist,
        tracker.rvec,
        tracker.tvec,
        true
    );

    if (!ok) return;

    const double alpha = 0.15;
    tracker.rvecFiltered =
        alpha * tracker.rvec + (1-alpha) * tracker.rvecFiltered;
    tracker.tvecFiltered =
        alpha * tracker.tvec + (1-alpha) * tracker.tvecFiltered;

    tracker.prevCorners = tracker.currCorners;
    tracker.lostFrames = 0;
}
