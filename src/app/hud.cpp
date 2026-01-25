/**
 * @file hud.cpp
 * @brief Implémentation des éléments HUD (Head-Up Display).
 *
 * Ce fichier regroupe les fonctions de dessin des informations
 * d'interface superposées à l'image caméra :
 * - pause
 * - FPS
 * - timer
 * - messages informatifs et d'avertissement
 *
 * Le HUD est dessiné directement dans l'image OpenCV avant
 * l'affichage OpenGL.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

#include "app/hud.hpp"
#include <cstdio>

namespace app {

/**
 * @brief Affiche un message de pause centré en haut de l'écran.
 *
 * Un fond noir est dessiné derrière le texte afin d'assurer
 * une bonne lisibilité quelles que soient les conditions de lumière.
 *
 * @param frame Image courante sur laquelle dessiner le HUD.
 */
void drawPauseOverlay(cv::Mat& frame) {
    std::string pauseMsg = "PAUSE. Espace/Echap -> reprendre/quitter.";

    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(pauseMsg, cv::FONT_HERSHEY_SIMPLEX, 1.2, 3, &baseline);

    // Position centrée horizontalement, proche du haut de l'écran
    cv::Point pos((frame.cols - textSize.width) / 2, 40);

    // Fond noir pour améliorer la lisibilité
    cv::rectangle(
        frame,
        pos + cv::Point(-10, baseline + 5),
        pos + cv::Point(textSize.width + 10, -textSize.height - 5),
        cv::Scalar(0, 0, 0),
        -1
    );

    // Texte jaune (contraste fort)
    cv::putText(
        frame,
        pauseMsg,
        pos,
        cv::FONT_HERSHEY_SIMPLEX,
        1.2,
        cv::Scalar(0, 255, 255),
        3
    );
}

/**
 * @brief Affiche le nombre d'images par seconde (FPS).
 *
 * Le FPS est affiché en haut à gauche de l'écran
 * afin de surveiller les performances temps réel.
 *
 * @param frame Image courante.
 * @param fps Valeur des FPS calculée.
 */
void drawFPS(cv::Mat& frame, float fps) {
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);

    cv::putText(
        frame,
        fpsText,
        cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(0, 255, 0),
        2
    );
}

/**
 * @brief Affiche le temps écoulé depuis le début de la partie.
 *
 * Le temps est affiché au format :
 * mm:ss.ms
 *
 * @param frame Image courante.
 * @param elapsedSeconds Temps écoulé en secondes.
 */
void drawTimer(cv::Mat& frame, double elapsedSeconds) {
    int mins = static_cast<int>(elapsedSeconds) / 60;
    int secs = static_cast<int>(elapsedSeconds) % 60;
    int ms   = static_cast<int>((elapsedSeconds - static_cast<int>(elapsedSeconds)) * 100);

    char timeText[32];
    snprintf(timeText, sizeof(timeText), "%02d:%02d.%02d", mins, secs, ms);

    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(timeText, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);

    // Position en haut à droite de l'écran
    cv::Point pos(frame.cols - textSize.width - 10, 30);

    cv::putText(
        frame,
        timeText,
        pos,
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(0, 255, 0),
        2
    );
}

/**
 * @brief Affiche un message centré à l'écran.
 *
 * Utilisé pour des messages généraux :
 * - début de partie
 * - victoire / défaite
 * - information utilisateur
 *
 * @param frame Image courante.
 * @param message Texte à afficher.
 */
void drawCenteredMessage(cv::Mat& frame, const std::string& message) {
    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(message, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, &baseline);

    // Position centrée horizontalement et verticalement
    cv::Point textOrg(
        (frame.cols - textSize.width) / 2,
        (frame.rows + textSize.height) / 2
    );

    // Fond noir pour assurer la lisibilité
    cv::rectangle(
        frame,
        textOrg + cv::Point(0, baseline),
        textOrg + cv::Point(textSize.width, -textSize.height),
        cv::Scalar(0, 0, 0),
        -1
    );

    // Texte jaune
    cv::putText(
        frame,
        message,
        textOrg,
        cv::FONT_HERSHEY_SIMPLEX,
        1.0,
        cv::Scalar(0, 255, 255),
        2
    );
}

/**
 * @brief Affiche un message d'avertissement en bas de l'écran.
 *
 * Utilisé pour signaler des situations critiques :
 * - image floue
 * - perte de tracking
 * - erreur utilisateur
 *
 * @param frame Image courante.
 * @param message Message d'avertissement à afficher.
 */
void drawWarningMessage(cv::Mat& frame, const std::string& message) {
    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(message, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);

    // Position centrée horizontalement, proche du bas de l'écran
    cv::Point textOrg(
        (frame.cols - textSize.width) / 2,
        frame.rows - 30
    );

    // Fond rouge pour attirer l'attention
    cv::rectangle(
        frame,
        textOrg + cv::Point(-10, baseline + 5),
        textOrg + cv::Point(textSize.width + 10, -textSize.height - 5),
        cv::Scalar(0, 0, 100),
        -1
    );

    // Texte blanc
    cv::putText(
        frame,
        message,
        textOrg,
        cv::FONT_HERSHEY_SIMPLEX,
        0.8,
        cv::Scalar(255, 255, 255),
        2
    );
}

} // namespace app
