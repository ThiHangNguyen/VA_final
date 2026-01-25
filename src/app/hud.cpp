/**
 * @file hud.cpp
 * @brief Implémentation de l'affichage HUD.
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#include "app/hud.hpp"
#include <cstdio>

namespace app {

void drawPauseOverlay(cv::Mat& frame) {
    std::string pauseMsg = "PAUSE. Espace/Echap -> reprendre/quitter.";
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(pauseMsg, cv::FONT_HERSHEY_SIMPLEX, 1.2, 3, &baseline);
    cv::Point pos((frame.cols - textSize.width) / 2, 40);

    // Fond noir pour lisibilité
    cv::rectangle(frame,
        pos + cv::Point(-10, baseline + 5),
        pos + cv::Point(textSize.width + 10, -textSize.height - 5),
        cv::Scalar(0, 0, 0), -1);

    // Texte jaune
    cv::putText(frame, pauseMsg, pos, cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 255, 255), 3);
}

void drawFPS(cv::Mat& frame, float fps) {
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);
    cv::putText(frame, fpsText, cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}

void drawTimer(cv::Mat& frame, double elapsedSeconds) {
    int mins = static_cast<int>(elapsedSeconds) / 60;
    int secs = static_cast<int>(elapsedSeconds) % 60;
    int ms = static_cast<int>((elapsedSeconds - static_cast<int>(elapsedSeconds)) * 100);

    char timeText[32];
    snprintf(timeText, sizeof(timeText), "%02d:%02d.%02d", mins, secs, ms);

    int baseline = 0;
    cv::Size textSize = cv::getTextSize(timeText, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);
    cv::Point pos(frame.cols - textSize.width - 10, 30);
    cv::putText(frame, timeText, pos, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}

void drawCenteredMessage(cv::Mat& frame, const std::string& message) {
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(message, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, &baseline);

    // Centrer le texte
    cv::Point textOrg((frame.cols - textSize.width) / 2,
                      (frame.rows + textSize.height) / 2);

    // Fond noir pour lisibilité
    cv::rectangle(frame,
        textOrg + cv::Point(0, baseline),
        textOrg + cv::Point(textSize.width, -textSize.height),
        cv::Scalar(0, 0, 0), -1);

    // Texte jaune
    cv::putText(frame, message, textOrg, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 255), 2);
}

void drawWarningMessage(cv::Mat& frame, const std::string& message) {
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(message, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);

    // Afficher en bas de l'écran
    cv::Point textOrg((frame.cols - textSize.width) / 2, frame.rows - 30);

    // Fond rouge pour attirer l'attention
    cv::rectangle(frame,
        textOrg + cv::Point(-10, baseline + 5),
        textOrg + cv::Point(textSize.width + 10, -textSize.height - 5),
        cv::Scalar(0, 0, 100), -1);

    // Texte blanc
    cv::putText(frame, message, textOrg, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
}

} // namespace app
