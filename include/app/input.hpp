#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "game/maze.hpp"

enum class PhysicsMode { 
    EARTH, 
    MOON 
};

enum class DesignTheme { 
    DEFAULT = 1, 
    SPACE = 2, 
    DESERT = 3 
};
struct InputConfig {
    bool useWebcam = false;
    bool usePhone  = false;
    std::string phoneUrl;
    std::string videoPath = "../data/Video_AR_1.mp4";;
    std::string calibPath = "../data/camera.yaml";
    game::Difficulty difficulty = game::Difficulty::EASY; // Par défau
    
    PhysicsMode speedMode = PhysicsMode::EARTH;
    PhysicsMode bounceMode = PhysicsMode::EARTH;
    DesignTheme designTheme = DesignTheme::DEFAULT;
};

bool openVideoSource(cv::VideoCapture& cap, const InputConfig& cfg);
bool parseArgs(int argc, char** argv, InputConfig& cfg);
