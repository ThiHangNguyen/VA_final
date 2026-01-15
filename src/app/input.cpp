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
    // Valeurs par défaut déjà définies dans le struct InputConfig 
    // (difficulty = EASY, videoPath = default, etc.)
    if (argc <= 1) return true; 

    // On parcourt tous les arguments un par un
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // --- CHOIX DE LA SOURCE ---
        if (arg == "--webcam") {
            cfg.useWebcam = true;
            cfg.calibPath = "../data/camera_webcam.yaml";
        }
        else if (arg == "--phone") {
            if (i + 1 < argc) {
                cfg.usePhone = true;
                cfg.phoneUrl = argv[++i]; // On consomme l'argument suivant (l'URL)
            } else {
                std::cerr << "Usage: --phone <url_droidcam>\n";
                return false;
            }
        }
        else if (arg == "--video") {
            if (i + 2 < argc) {
                cfg.videoPath = argv[++i]; // On prend le path
                cfg.calibPath = argv[++i]; // On prend le calib
            } else {
                std::cerr << "Usage: --video <video_path> <calibration_path>\n";
                return false;
            }
        }
        // --- CHOIX DE LA DIFFICULTÉ ---
        else if (arg == "--ez") {
            cfg.difficulty = game::Difficulty::EASY;
            std::cout << "[CONFIG] Difficulté : FACILE\n";
        }
        else if (arg == "--med") {
            cfg.difficulty = game::Difficulty::MEDIUM;
            std::cout << "[CONFIG] Difficulté : MOYEN\n";
        }
        else if (arg == "--hard") {
            cfg.difficulty = game::Difficulty::HARD;
            std::cout << "[CONFIG] Difficulté : DIFFICILE\n";
        }
        // --- DESIGN ---
        else if (arg == "--design") {
            if (i + 1 < argc) {
                int d = std::stoi(argv[++i]);
                if (d == 1) cfg.designTheme = DesignTheme::DEFAULT;
                else if (d == 2) cfg.designTheme = DesignTheme::SPACE;
                else if (d == 3) cfg.designTheme = DesignTheme::DESERT;
                else std::cerr << "[WARN] Design inconnu (1-3), defaut applique.\n";
            }
        }

        // --- SPEED (Vitesse/Gravité) ---
        else if (arg == "--speed") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "moon") cfg.speedMode = PhysicsMode::MOON;
                else if (val == "earth") cfg.speedMode = PhysicsMode::EARTH;
            }
        }

        // --- BOUNCE (Rebond) ---
        else if (arg == "--bounce") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "moon") cfg.bounceMode = PhysicsMode::MOON;
                else if (val == "earth") cfg.bounceMode = PhysicsMode::EARTH;
            }
        }
        else {
            std::cerr << "Argument inconnu ou mal placé : " << arg << "\n";
            return false;}
    }

    return true;
}