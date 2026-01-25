/**
 * @file input.cpp
 * @brief Gestion des sources vidéo et du parsing des arguments utilisateur.
 *
 * Ce fichier centralise :
 * - l'ouverture de la source vidéo (webcam, téléphone, fichier)
 * - le parsing des arguments de la ligne de commande
 *
 * Il permet de configurer dynamiquement l'application AR
 * sans modifier le code source.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

#include "app/input.hpp"
#include <iostream>
#include <cmath>

/**
 * @brief Ouvre la source vidéo en fonction de la configuration utilisateur.
 *
 * Priorité :
 * 1. Webcam locale
 * 2. Flux vidéo téléphone (DroidCam / IP Webcam)
 * 3. Fichier vidéo
 *
 * Pour la webcam, la fonction tente d'abord une configuration
 * en MJPEG (meilleures performances), puis bascule en YUYV
 * si les paramètres demandés ne sont pas respectés.
 *
 * @param cap Objet OpenCV VideoCapture à initialiser.
 * @param cfg Configuration des entrées utilisateur.
 * @return true si la source est ouverte avec succès.
 * @return false en cas d'échec.
 */
bool openVideoSource(cv::VideoCapture& cap, const InputConfig& cfg)
{
    // =====================================================
    // 1. WEBCAM LOCALE
    // =====================================================
    if (cfg.useWebcam) {
        int camIndex = 0;
        int reqW = 1280, reqH = 720, reqFPS = 30;

        // Ouverture de la webcam via V4L2 (Linux)
        if (!cap.open(camIndex, cv::CAP_V4L2)) {
            std::cerr << "Erreur : webcam non accessible\n";
            return false;
        }

        // Tentative de configuration en MJPEG (meilleur débit)
        cap.set(cv::CAP_PROP_FOURCC,
                cv::VideoWriter::fourcc('M','J','P','G'));
        cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
        cap.set(cv::CAP_PROP_FPS,          reqFPS);

        // Fallback en YUYV si la configuration MJPEG échoue
        if ((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)  != reqW ||
            (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT) != reqH ||
            (int)std::round(cap.get(cv::CAP_PROP_FPS)) != reqFPS)
        {
            cap.set(cv::CAP_PROP_FOURCC,
                    cv::VideoWriter::fourcc('Y','U','Y','V'));
            cap.set(cv::CAP_PROP_FRAME_WIDTH,  reqW);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, reqH);
            cap.set(cv::CAP_PROP_FPS,          reqFPS);
        }

        // Affichage des paramètres réellement appliqués (diagnostic)
        int actualW = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int actualH = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double actualFPS = cap.get(cv::CAP_PROP_FPS);
        int fourcc = (int)cap.get(cv::CAP_PROP_FOURCC);

        char codec[5] = {
            (char)(fourcc & 0xFF),
            (char)((fourcc >> 8) & 0xFF),
            (char)((fourcc >> 16) & 0xFF),
            (char)((fourcc >> 24) & 0xFF),
            '\0'
        };

        std::cout << "[INFO] Webcam ouverte: "
                  << actualW << "x" << actualH
                  << " @ " << actualFPS << " FPS"
                  << " [Codec: " << codec << "]\n";

        // Avertissement si le framerate est trop faible
        if (actualFPS < 25) {
            std::cout << "[WARN] FPS faible : la caméra ne supporte peut-être pas 30 FPS à cette résolution.\n";
            std::cout << "       Réduis la résolution ou vérifie le support MJPEG.\n";
        }

        return true;
    }

    // =====================================================
    // 2. FLUX VIDÉO TÉLÉPHONE
    // =====================================================
    if (cfg.usePhone) {
        std::cout << "[INFO] Connexion DroidCam: " << cfg.phoneUrl << "\n";
        if (!cap.open(cfg.phoneUrl)) {
            std::cerr << "Erreur : flux téléphone inaccessible\n";
            return false;
        }
        return true;
    }

    // =====================================================
    // 3. FICHIER VIDÉO
    // =====================================================
    std::cout << "[INFO] Lecture vidéo: " << cfg.videoPath << "\n";
    if (!cap.open(cfg.videoPath)) {
        std::cerr << "Erreur : vidéo introuvable\n";
        return false;
    }

    return true;
}

/**
 * @brief Analyse les arguments de la ligne de commande.
 *
 * Cette fonction permet de configurer :
 * - la source vidéo (webcam, téléphone, fichier)
 * - la calibration caméra associée
 * - la difficulté du jeu
 * - le thème graphique
 * - les paramètres physiques (vitesse, rebond)
 *
 * Les valeurs par défaut sont définies dans la structure InputConfig.
 *
 * @param argc Nombre d'arguments.
 * @param argv Tableau des arguments.
 * @param[out] cfg Structure de configuration à remplir.
 * @return true si le parsing est valide.
 * @return false en cas d'erreur ou d'argument inconnu.
 */
bool parseArgs(int argc, char** argv, InputConfig& cfg)
{
    // Aucun argument : on conserve la configuration par défaut
    if (argc <= 1) return true;

    // Parcours séquentiel des arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // =================================================
        // SOURCE VIDÉO
        // =================================================
        if (arg == "--webcam") {
            cfg.useWebcam = true;
            cfg.calibPath = "../data/camera_webcam.yaml";
        }
        else if (arg == "--phone") {
            if (i + 1 < argc) {
                cfg.usePhone = true;
                cfg.phoneUrl = argv[++i];
                cfg.calibPath = "../data/camera_ip11.yaml";
            } else {
                std::cerr << "Usage: --phone <url_droidcam>\n";
                return false;
            }
        }
        else if (arg == "--video") {
            if (i + 2 < argc) {
                cfg.videoPath = argv[++i];
                cfg.calibPath = argv[++i];
            } else {
                std::cerr << "Usage: --video <video_path> <calibration_path>\n";
                return false;
            }
        }

        // =================================================
        // DIFFICULTÉ DU JEU
        // =================================================
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

        // =================================================
        // DESIGN GRAPHIQUE
        // =================================================
        else if (arg == "--design") {
            if (i + 1 < argc) {
                int d = std::stoi(argv[++i]);
                if (d == 1) cfg.designTheme = DesignTheme::DEFAULT;
                else if (d == 2) cfg.designTheme = DesignTheme::SPACE;
                else if (d == 3) cfg.designTheme = DesignTheme::DESERT;
                else std::cerr << "[WARN] Design inconnu (1–3), défaut appliqué.\n";
            }
        }

        // =================================================
        // PHYSIQUE : VITESSE / GRAVITÉ
        // =================================================
        else if (arg == "--speed") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "moon")  cfg.speedMode = PhysicsMode::MOON;
                else if (val == "earth") cfg.speedMode = PhysicsMode::EARTH;
            }
        }

        // =================================================
        // PHYSIQUE : REBOND
        // =================================================
        else if (arg == "--bounce") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "moon")  cfg.bounceMode = PhysicsMode::MOON;
                else if (val == "earth") cfg.bounceMode = PhysicsMode::EARTH;
            }
        }
        else {
            std::cerr << "Argument inconnu ou mal placé : " << arg << "\n";
            return false;
        }
    }

    return true;
}
