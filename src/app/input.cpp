#include "app/input.hpp"
#include <iostream>
#include <cmath>

bool openVideoSource(cv::VideoCapture& cap, const InputConfig& cfg)
{
    if (cfg.useWebcam) {
        int camIndex = 0;
        int reqW = 1280, reqH = 720, reqFPS = 30;

        if (!cap.open(camIndex, cv::CAP_V4L2)) {
            std::cerr << "Erreur : webcam non accessible\n";
            return false;
        }

        // Essai MJPEG
        cap.set(cv::CAP_PROP_FOURCC,
                cv::VideoWriter::fourcc('M','J','P','G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
        cap.set(cv::CAP_PROP_FPS,          reqFPS);

        // Fallback YUYV
        if ((int)cap.get(cv::CAP_PROP_FRAME_WIDTH) != reqW ||
            (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT) != reqH ||
            (int)std::round(cap.get(cv::CAP_PROP_FPS)) != reqFPS)
        {
            cap.set(cv::CAP_PROP_FOURCC,
                    cv::VideoWriter::fourcc('Y','U','Y','V'));
            cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
            cap.set(cv::CAP_PROP_FPS,          reqFPS);
        }

        std::cout << "[INFO] Webcam ouverte: "
                  << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
                  << cap.get(cv::CAP_PROP_FRAME_HEIGHT)
                  << " @ " << cap.get(cv::CAP_PROP_FPS) << " FPS\n";
        return true;
    }

    if (cfg.usePhone) {
        std::cout << "[INFO] Connexion DroidCam: " << cfg.phoneUrl << "\n";
        if (!cap.open(cfg.phoneUrl)) {
            std::cerr << "Erreur : flux téléphone inaccessible\n";
            return false;
        }
        return true;
    }

    // Vidéo fichier
    std::cout << "[INFO] Lecture vidéo: " << cfg.videoPath << "\n";
    if (!cap.open(cfg.videoPath)) {
        std::cerr << "Erreur : vidéo introuvable\n";
        return false;
    }

    return true;
}


bool parseArgs(int argc, char** argv, InputConfig& cfg)
{
    if (argc <= 1) return true; // valeurs par défaut

    std::string mode = argv[1];

    if (mode == "--webcam") {
        cfg.useWebcam = true;
        cfg.calibPath = "../data/camera_webcam.yaml";
        return true;
    }

    if (mode == "--phone") {
        if (argc < 3) {
            std::cerr << "Usage: --phone <url_droidcam>\n";
            return false;
        }
        cfg.usePhone = true;
        cfg.phoneUrl = argv[2];
        return true;
    }

    if (mode == "--video") {
        if (argc < 4) {
            std::cerr << "Usage: --video <video_path> <calibration_path>\n";
            return false;
        }
        cfg.videoPath = argv[2];
        cfg.calibPath = argv[3];
        return true;
    }

    std::cerr << "Argument inconnu: " << mode << "\n";
    return false;
}


