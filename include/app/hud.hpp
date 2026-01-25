/**
 * @file hud.hpp
 * @brief Affichage du HUD (Head-Up Display) sur la frame vidéo.
 *
 * Ce module gère l'affichage des informations à l'écran :
 * - Indicateur de pause avec instructions
 * - Compteur de FPS
 * - Chronomètre de jeu
 * - Messages d'avertissement (pas de détection, image floue)
 *
 * Tous les éléments sont rendus avec OpenCV directement sur la frame
 * vidéo avant conversion en texture OpenGL.
 *
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace app {

/**
 * @brief Affiche l'indicateur de pause avec les instructions.
 *
 * Dessine "PAUSE" en jaune au centre-haut de l'écran avec un fond noir
 * pour la lisibilité, ainsi que les touches pour reprendre/quitter.
 *
 * @param frame Image sur laquelle dessiner (modifiée en place).
 */
void drawPauseOverlay(cv::Mat& frame);

/**
 * @brief Affiche le compteur de FPS.
 *
 * Dessine le nombre de FPS en vert dans le coin supérieur gauche.
 *
 * @param frame Image sur laquelle dessiner.
 * @param fps Valeur des FPS à afficher.
 */
void drawFPS(cv::Mat& frame, float fps);

/**
 * @brief Affiche le chronomètre de jeu.
 *
 * Dessine le temps écoulé au format MM:SS.cc dans le coin supérieur droit.
 *
 * @param frame Image sur laquelle dessiner.
 * @param elapsedSeconds Temps écoulé en secondes.
 */
void drawTimer(cv::Mat& frame, double elapsedSeconds);

/**
 * @brief Affiche un message d'erreur centré.
 *
 * Utilisé pour afficher "Pas de A4 détecté" ou autres messages importants.
 *
 * @param frame Image sur laquelle dessiner.
 * @param message Le message à afficher.
 */
void drawCenteredMessage(cv::Mat& frame, const std::string& message);

/**
 * @brief Affiche un avertissement en bas de l'écran.
 *
 * Utilisé pour l'avertissement "Image floue" avec fond rouge.
 *
 * @param frame Image sur laquelle dessiner.
 * @param message Le message d'avertissement.
 */
void drawWarningMessage(cv::Mat& frame, const std::string& message);

} // namespace app
