#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include "game/maze.hpp"

/**
 * @enum PhysicsMode
 * @brief Définit le modèle physique utilisé dans la simulation.
 *
 * Ces modes influencent principalement :
 * - la gravité
 * - la vitesse de déplacement
 * - le rebond de la balle
 */
enum class PhysicsMode { 
    EARTH, ///< Physique terrestre (gravité standard)
    MOON   ///< Physique lunaire (gravité réduite)
};

/**
 * @enum DesignTheme
 * @brief Thèmes visuels de l'application AR.
 *
 * Chaque thème modifie l'apparence graphique
 * (couleurs, textures, ambiance visuelle).
 */
enum class DesignTheme { 
    DEFAULT = 1, ///< Thème classique
    SPACE   = 2, ///< Thème spatial
    DESERT  = 3  ///< Thème désertique
};

/**
 * @struct InputConfig
 * @brief Configuration globale des entrées et paramètres utilisateur.
 *
 * Cette structure centralise :
 * - la source vidéo (webcam, téléphone, fichier)
 * - la calibration caméra
 * - les paramètres de jeu
 * - les modes physiques et graphiques
 *
 * Elle est généralement remplie via la ligne de commande
 * ou par configuration par défaut.
 */
struct InputConfig {

    bool useWebcam = false; ///< Utiliser la webcam locale comme source vidéo
    bool usePhone  = false; ///< Utiliser un flux vidéo depuis un téléphone

    std::string phoneUrl;  ///< URL du flux vidéo du téléphone (ex: IP Webcam)
    std::string videoPath = "../data/Video_AR_1.mp4"; ///< Chemin vers la vidéo de test
    std::string calibPath = "../data/camera.yaml";    ///< Fichier de calibration caméra

    game::Difficulty difficulty = game::Difficulty::EASY; ///< Difficulté du jeu (par défaut EASY)
    
    PhysicsMode speedMode  = PhysicsMode::EARTH; ///< Modèle physique pour la vitesse
    PhysicsMode bounceMode = PhysicsMode::EARTH; ///< Modèle physique pour le rebond

    DesignTheme designTheme = DesignTheme::DEFAULT; ///< Thème graphique sélectionné
};

/**
 * @brief Ouvre la source vidéo en fonction de la configuration utilisateur.
 *
 * Cette fonction choisit automatiquement la bonne source :
 * - webcam
 * - flux vidéo téléphone
 * - fichier vidéo local
 *
 * @param cap Objet OpenCV VideoCapture à initialiser
 * @param cfg Configuration des entrées
 * @return true si la source vidéo est ouverte avec succès
 * @return false en cas d'échec
 */
bool openVideoSource(cv::VideoCapture& cap, const InputConfig& cfg);

/**
 * @brief Analyse les arguments de la ligne de commande.
 *
 * Cette fonction :
 * - lit les options utilisateur (source vidéo, thème, difficulté, etc.)
 * - met à jour la structure InputConfig
 * - applique les valeurs par défaut si nécessaire
 *
 * @param argc Nombre d'arguments
 * @param argv Tableau des arguments
 * @param cfg Structure de configuration à remplir
 * @return true si le parsing est valide
 * @return false en cas d'erreur d'arguments
 */
bool parseArgs(int argc, char** argv, InputConfig& cfg);
