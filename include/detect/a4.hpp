#pragma once
#include <opencv2/core.hpp>
#include <vector>

/**
 * @file a4.hpp
 * @brief Détection d'une feuille A4 (forme quadrilatère) et ordonnancement des coins.
 *
 * Fournit les fonctions :
 * - pour détecter automatiquement une forme de type A4 dans une image BGR,
 * - pour ordonner les 4 coins détectés (haut-gauche, haut-droit, bas-droit, bas-gauche),
 * - pour dessiner ces coins sur une image avec des labels utiles pour le débogage.
 */

namespace detect {

/**
 * @brief Ordonne 4 points dans l’ordre (TL, TR, BR, BL) selon leur position spatiale.
 *
 * Ce tri est basé sur la position verticale (y) en priorité,
 * puis sur la distance par rapport au coin supérieur gauche.
 * Il est utile pour garantir la cohérence géométrique en AR et en solvePnP.
 *
 * @param approx 4 points issus d’une approximation polygonale (e.g., approxPolyDP).
 * @param[out] ordered Points convertis en float et ordonnés (taille = 4).
 * @return true si succès (approx contient bien 4 points).
 */
bool orderFourCorners(const std::vector<cv::Point>& approx,
                      std::vector<cv::Point2f>& ordered);

/**
 * @brief Détecte les 4 coins d'une feuille A4 (ou forme équivalente) dans une image BGR.
 *
 * Pipeline de détection :
 * - Conversion en niveaux de gris + floutage
 * - Seuillage dynamique basé sur la moyenne d'une ROI centrale
 * - Recherche de contours
 * - Sélection du plus grand quadrilatère
 * - Ordonnancement TL/TR/BR/BL
 *
 * @param frameBGR Image d’entrée en couleur (BGR).
 * @param[out] imagePts Vecteur de 4 points image (ordonnés).
 * @return true si la détection est réussie.
 */
bool detectA4Corners(const cv::Mat& frameBGR,
                     std::vector<cv::Point2f>& imagePts);

/**
 * @brief Dessine les 4 coins ordonnés sur une image avec des couleurs et labels.
 *
 * Les coins sont étiquetés "TL", "TR", "BR", "BL" dans l'ordre.
 * Permet de vérifier visuellement que l’ordre des points est correct.
 *
 * @param img Image sur laquelle dessiner les coins.
 * @param pts Points 2D ordonnés (taille 4, en float).
 */
void drawOrderedCorners(cv::Mat& img, const std::vector<cv::Point2f>& pts);

} // namespace detect
