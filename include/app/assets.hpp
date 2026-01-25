/**
 * @file assets.hpp
 * @brief Chargement des ressources graphiques (textures) par thème.
 *
 * Ce module gère le chargement dynamique des textures selon le thème
 * sélectionné (DEFAULT, SPACE, DESERT). Il fournit une structure
 * centralisée pour stocker les IDs OpenGL des textures.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>
#include <string>
#include "app/game.hpp"  // Pour DesignTheme

namespace app {

/**
 * @struct ThemeAssets
 * @brief Contient les identifiants OpenGL des textures d'un thème.
 */
struct ThemeAssets {
    GLuint ballTexture;   ///< Texture de la balle
    GLuint groundTexture; ///< Texture du sol (mode VR)
    GLuint skyTexture;    ///< Texture du ciel (mode VR)
};

/**
 * @brief Charge les textures du thème sélectionné.
 *
 * Charge les fichiers balle.png, sol.png et ciel.jpg depuis le dossier
 * data/design_X/ où X correspond au numéro du thème. En cas d'échec,
 * utilise les textures par défaut (design_1).
 *
 * @param theme Le thème à charger (DEFAULT=1, SPACE=2, DESERT=3).
 * @param fallbackTexture Texture de secours si le chargement échoue.
 * @return ThemeAssets Structure contenant les IDs des textures chargées.
 */
ThemeAssets loadThemeAssets(DesignTheme theme, const cv::Mat& fallbackTexture);

/**
 * @brief Libère les textures OpenGL d'un thème.
 *
 * @param assets Les assets à libérer.
 */
void freeThemeAssets(ThemeAssets& assets);

/**
 * @brief Retourne le chemin du dossier d'un thème.
 *
 * @param theme Le thème concerné.
 * @return std::string Chemin relatif (ex: "../data/design_1/").
 */
std::string getThemePath(DesignTheme theme);

} // namespace app
