#include "app/score.hpp"
#include "app/init.hpp"   
#include "app/game.hpp"
#include <opencv2/opencv.hpp>
#include <string>
#include <iomanip>
#include <sstream>
struct ClickContext { int choice = -1; };

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        ClickContext* ctx = (ClickContext*)userdata;
        // Bouton Restart (Zone gauche)
        if (x > 100 && x < 280 && y > 320 && y < 370) ctx->choice = 0;
        // Bouton Menu (Zone droite)
        if (x > 320 && x < 500 && y > 320 && y < 370) ctx->choice = 1;
    }
}

int showScoreWindow(const GameResult& result) {
    std::string winName = "Resultats du Jeu";
    cv::namedWindow(winName, cv::WINDOW_AUTOSIZE);
    cv::Mat scoreImg = cv::Mat::zeros(400, 600, CV_8UC3);
    scoreImg.setTo(cv::Scalar(45, 30, 30));

    // Textes de score (comme avant)
    cv::putText(scoreImg, "VICTOIRE !", cv::Point(180, 80), cv::FONT_HERSHEY_DUPLEX, 1.5, cv::Scalar(0, 215, 255), 3);
    cv::putText(scoreImg, "Temps : " + std::to_string(result.time).substr(0, 5) + "s", cv::Point(150, 180), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    // Dessin des boutons
    cv::rectangle(scoreImg, cv::Point(100, 320), cv::Point(280, 370), cv::Scalar(0, 200, 0), -1); // Vert
    cv::putText(scoreImg, "RESTART", cv::Point(130, 355), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    cv::rectangle(scoreImg, cv::Point(320, 320), cv::Point(500, 370), cv::Scalar(200, 0, 0), -1); // Bleu
    cv::putText(scoreImg, "MENU", cv::Point(375, 355), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    ClickContext clickCtx;
    cv::setMouseCallback("Resultats du Jeu", onMouse, &clickCtx);

    while (clickCtx.choice == -1) {
        cv::imshow("Resultats du Jeu", scoreImg);
        if (cv::waitKey(10) == 27) break; // Sortie de secours avec ESC
    }

    cv::destroyWindow("Resultats du Jeu");
    return clickCtx.choice;
}