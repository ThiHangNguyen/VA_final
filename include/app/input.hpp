#pragma once
#include <opencv2/opencv.hpp>
#include <string>

struct InputConfig {
    bool useWebcam = false;
    bool usePhone  = false;
    std::string phoneUrl;
    std::string videoPath = "../data/Video_AR_1.mp4";;
    std::string calibPath = "../data/camera.yaml";
};

bool openVideoSource(cv::VideoCapture& cap, const InputConfig& cfg);
bool parseArgs(int argc, char** argv, InputConfig& cfg);
