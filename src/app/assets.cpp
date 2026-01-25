/**
 * @file assets.cpp
 * @brief Implémentation du chargement des ressources graphiques.
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#include "app/assets.hpp"
#include "glx/texture.hpp"
#include <iostream>

namespace app {

std::string getThemePath(DesignTheme theme) {
    return "../data/design_" + std::to_string(static_cast<int>(theme)) + "/";
}

ThemeAssets loadThemeAssets(DesignTheme theme, const cv::Mat& fallbackTexture) {
    ThemeAssets assets;
    std::string themePath = getThemePath(theme);
    std::cout << "[ASSETS] Chargement depuis : " << themePath << "\n";

    // === 1. TEXTURE BALLE ===
    cv::Mat ballImg = cv::imread(themePath + "balle.png");
    if (ballImg.empty()) {
        std::cerr << "[ASSETS] WARN: balle.png introuvable, fallback design_1\n";
        ballImg = cv::imread("../data/design_1/balle.png");
    }
    if (!ballImg.empty()) {
        cv::cvtColor(ballImg, ballImg, cv::COLOR_BGR2RGB);
    }
    assets.ballTexture = glx::createTextureFromMat(ballImg.empty() ? fallbackTexture : ballImg);

    // === 2. TEXTURE SOL ===
    cv::Mat groundImg = cv::imread(themePath + "sol.png");
    if (groundImg.empty()) {
        groundImg = cv::imread("../data/design_1/sol.png");
    }
    if (!groundImg.empty()) {
        cv::cvtColor(groundImg, groundImg, cv::COLOR_BGR2RGB);
    }
    assets.groundTexture = glx::createTextureFromMat(groundImg.empty() ? fallbackTexture : groundImg);

    // === 3. TEXTURE CIEL ===
    cv::Mat skyImg = cv::imread(themePath + "ciel.jpg");
    if (skyImg.empty()) {
        skyImg = cv::imread("../data/design_1/ciel.jpg");
    }
    if (!skyImg.empty()) {
        cv::cvtColor(skyImg, skyImg, cv::COLOR_BGR2RGB);
        cv::flip(skyImg, skyImg, 0);  // Flip vertical pour OpenGL
    }
    assets.skyTexture = glx::createTextureFromMat(skyImg.empty() ? fallbackTexture : skyImg);

    return assets;
}

void freeThemeAssets(ThemeAssets& assets) {
    glDeleteTextures(1, &assets.ballTexture);
    glDeleteTextures(1, &assets.groundTexture);
    glDeleteTextures(1, &assets.skyTexture);
}

} // namespace app
