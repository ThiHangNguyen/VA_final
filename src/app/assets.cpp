/**
 * @file assets.cpp
 * @brief Implémentation du chargement et de la libération des ressources graphiques.
 *
 * Ce fichier gère le chargement des textures associées
 * à un thème graphique (balle, sol, ciel).
 * Un mécanisme de repli (fallback) est utilisé si certaines
 * ressources sont manquantes.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

#include "app/assets.hpp"
#include "glx/texture.hpp"
#include <iostream>

namespace app {

/**
 * @brief Construit le chemin vers le dossier du thème graphique.
 *
 * Les thèmes sont organisés sous la forme :
 * ../data/design_<id>/
 *
 * @param theme Thème graphique sélectionné.
 * @return Chemin vers le dossier du thème.
 */
std::string getThemePath(DesignTheme theme) {
    return "../data/design_" + std::to_string(static_cast<int>(theme)) + "/";
}

/**
 * @brief Charge l'ensemble des textures associées à un thème graphique.
 *
 * Cette fonction charge successivement :
 * - la texture de la balle
 * - la texture du sol
 * - la texture du ciel
 *
 * Si une texture est absente du thème sélectionné,
 * une texture de secours (design_1) est utilisée.
 * En dernier recours, une texture de repli est appliquée.
 *
 * @param theme Thème graphique à charger.
 * @param fallbackTexture Texture OpenCV utilisée si aucun fichier valide n'est trouvé.
 * @return Structure ThemeAssets contenant les textures OpenGL chargées.
 */
ThemeAssets loadThemeAssets(DesignTheme theme, const cv::Mat& fallbackTexture) {
    ThemeAssets assets;

    // Résolution du chemin du thème
    std::string themePath = getThemePath(theme);
    std::cout << "[ASSETS] Chargement depuis : " << themePath << "\n";

    // =====================================================
    // 1. TEXTURE DE LA BALLE
    // =====================================================
    cv::Mat ballImg = cv::imread(themePath + "balle.png");
    if (ballImg.empty()) {
        std::cerr << "[ASSETS] WARN: balle.png introuvable, fallback design_1\n";
        ballImg = cv::imread("../data/design_1/balle.png");
    }

    // Conversion BGR -> RGB (OpenCV -> OpenGL)
    if (!ballImg.empty()) {
        cv::cvtColor(ballImg, ballImg, cv::COLOR_BGR2RGB);
    }

    // Création de la texture OpenGL (fallback si nécessaire)
    assets.ballTexture =
        glx::createTextureFromMat(ballImg.empty() ? fallbackTexture : ballImg);

    // =====================================================
    // 2. TEXTURE DU SOL
    // =====================================================
    cv::Mat groundImg = cv::imread(themePath + "sol.png");
    if (groundImg.empty()) {
        groundImg = cv::imread("../data/design_1/sol.png");
    }

    if (!groundImg.empty()) {
        cv::cvtColor(groundImg, groundImg, cv::COLOR_BGR2RGB);
    }

    assets.groundTexture =
        glx::createTextureFromMat(groundImg.empty() ? fallbackTexture : groundImg);

    // =====================================================
    // 3. TEXTURE DU CIEL
    // =====================================================
    cv::Mat skyImg = cv::imread(themePath + "ciel.jpg");
    if (skyImg.empty()) {
        skyImg = cv::imread("../data/design_1/ciel.jpg");
    }

    if (!skyImg.empty()) {
        cv::cvtColor(skyImg, skyImg, cv::COLOR_BGR2RGB);
        cv::flip(skyImg, skyImg, 0);  // Inversion verticale pour le repère OpenGL
    }

    assets.skyTexture =
        glx::createTextureFromMat(skyImg.empty() ? fallbackTexture : skyImg);

    return assets;
}

/**
 * @brief Libère les textures OpenGL associées à un thème graphique.
 *
 * Cette fonction doit être appelée lors du changement de thème
 * ou à la fermeture de l'application afin d'éviter toute fuite
 * de mémoire GPU.
 *
 * @param assets Structure contenant les textures à libérer.
 */
void freeThemeAssets(ThemeAssets& assets) {
    glDeleteTextures(1, &assets.ballTexture);
    glDeleteTextures(1, &assets.groundTexture);
    glDeleteTextures(1, &assets.skyTexture);
}

} // namespace app
