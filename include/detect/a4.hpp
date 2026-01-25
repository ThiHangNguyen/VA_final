#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include <string>

/**
 * @file a4.hpp
 * @brief Détection d'une feuille A4 et ordonnancement de ses coins.
 *
 * Pipeline de détection :
 * 1. Conversion en niveaux de gris + flou gaussien
 * 2. Seuillage Otsu sur une ROI centrale (adaptatif à la luminosité)
 * 3. Opérations morphologiques (close + dilate) pour gérer le flou de mouvement
 * 4. Recherche du plus grand contour quadrilatère
 * 5. ConvexHull + approxPolyDP pour obtenir 4 coins propres
 * 6. Tri des coins (TL, TR, BR, BL) par méthode géométrique ou via le tracking
 *
 * Ce module est utilisé comme étape de détection initiale
 * avant le calcul de pose (solvePnP) en réalité augmentée.
 *
 * @author Thi Hang NGUYEN
 * @author Bichoy DAOUD
 */

namespace detect {

/**
 * @brief Renvoie les points 3D du modèle A4 dans son repère objet.
 *
 * Les points sont centrés sur la feuille A4 et exprimés en millimètres.
 * Ils sont destinés à être utilisés avec solvePnP.
 *
 * @return Vecteur de 4 points 3D correspondant aux coins de la feuille A4.
 */
std::vector<cv::Point3f> getA4ObjectPoints();

/**
 * @brief Ordonne 4 points dans l’ordre (TL, TR, BR, BL).
 *
 * Le tri repose sur des critères géométriques (position relative
 * et angles par rapport au centre de masse).
 * Il garantit une correspondance cohérente entre points image
 * et points objet pour le calcul de pose.
 *
 * @param approx 4 points issus d’une approximation polygonale (ex: approxPolyDP).
 * @param[out] ordered Points convertis en float et ordonnés (taille = 4).
 * @return true si le tri est valide (approx contient exactement 4 points).
 */
bool orderFourCorners(const std::vector<cv::Point>& approx,
                      std::vector<cv::Point2f>& ordered);

/**
 * @brief Détecte les 4 coins d'une feuille A4 dans une image couleur.
 *
 * Pipeline :
 * - Conversion en niveaux de gris + floutage
 * - Seuillage dynamique basé sur la moyenne d'une ROI centrale
 * - Recherche et filtrage des contours
 * - Sélection du meilleur quadrilatère
 * - Ordonnancement des coins (TL, TR, BR, BL)
 *
 * @param frameBGR Image d’entrée en couleur (BGR).
 * @param[out] imagePts Vecteur de 4 points image ordonnés.
 * @return true si la détection est réussie.
 */
bool detectA4Corners(const cv::Mat& frameBGR,
                     std::vector<cv::Point2f>& imagePts);

/**
 * @brief Dessine les coins ordonnés sur une image pour le débogage visuel.
 *
 * Les coins sont affichés avec des couleurs distinctes et étiquetés
 * "TL", "TR", "BR", "BL" afin de vérifier la cohérence de l'ordre.
 *
 * @param img Image sur laquelle dessiner.
 * @param pts Points 2D ordonnés (taille = 4).
 */
void drawOrderedCorners(cv::Mat& img,
                        const std::vector<cv::Point2f>& pts);

/**
 * @brief Recherche le meilleur contour correspondant à une feuille A4.
 *
 * Cette fonction sélectionne le plus grand quadrilatère valide
 * dans un masque binaire, en tenant compte de contraintes
 * géométriques (aire, convexité, ratio).
 *
 * @param mask Image binaire issue du seuillage.
 * @param[out] outApprox Approximation polygonale du contour retenu.
 * @param W Largeur de l'image.
 * @param H Hauteur de l'image.
 * @return true si un contour valide est trouvé.
 */
bool findBestA4Contour(const cv::Mat& mask,
                       std::vector<cv::Point>& outApprox,
                       int W,
                       int H);

/**
 * @brief Finalise la détection A4 à partir d’un contour candidat.
 *
 * Cette étape valide géométriquement le contour, ordonne les coins
 * et met à jour les points image utilisables pour le calcul de pose.
 *
 * @param approx Contour quadrilatère candidat.
 * @param frame Image source.
 * @param[out] imagePts Points image ordonnés.
 * @param method Méthode utilisée (détection, tracking, fallback, etc.).
 * @return true si la détection est validée.
 */
bool finalizeDetection(const std::vector<cv::Point>& approx,
                       const cv::Mat& frame,
                       std::vector<cv::Point2f>& imagePts,
                       const std::string& method);

/**
 * @brief Mesure la netteté d'une image via la variance du Laplacien.
 *
 * Une image nette présente une variance élevée (présence de bords),
 * tandis qu'une image floue présente une variance faible.
 *
 * @param frame Image d'entrée en BGR ou en niveaux de gris.
 * @return Score de netteté (variance du Laplacien).
 *
 * @note Valeurs indicatives :
 * - < 100  : très flou
 * - 100–300 : flou
 * - > 300  : image nette
 */
double measureBlur(const cv::Mat& frame);

/**
 * @brief Indique si une image est considérée comme floue.
 *
 * @param frame Image d'entrée en BGR ou niveaux de gris.
 * @param threshold Seuil de netteté (défaut = 100.0).
 * @return true si l'image est floue, false sinon.
 */
bool isBlurry(const cv::Mat& frame, double threshold = 100.0);

} // namespace detect
