#include "detect/a4.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace detect {

/**
 * @brief Renvoie le carré d'un nombre.
 * @param x La valeur à élever au carré.
 * @return x²
 */
static inline double sqr(double x) { return x * x; }

/**
 * @brief Trie et ordonne les 4 coins détectés dans l'ordre TL, BL, BR, TR.
 *
 * @param approx Vecteur contenant 4 points détectés (désordonnés).
 * @param ordered Vecteur de sortie contenant les 4 coins dans l'ordre TL, BL, BR, TR.
 * @return true si le tri a été effectué correctement.
 */

bool orderFourCorners(const std::vector<cv::Point>& approx,
                      std::vector<cv::Point2f>& ordered)
{
  if (approx.size() != 4) return false;
  std::vector<cv::Point2f> pts(4);
  for (int i = 0; i < 4; i++) pts[i] = approx[i];

  // Tri: d'abord par y (du plus haut au plus bas), puis par x
  std::sort(pts.begin(), pts.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
    if (std::abs(a.y - b.y) > 1e-3) return a.y < b.y;
    return a.x < b.x;
  });

  std::vector<cv::Point2f> top = { pts[0], pts[1] };
  std::vector<cv::Point2f> bot = { pts[2], pts[3] };

  // Identifier correctement TL/TR et BL/BR
  if (top[0].x > top[1].x) std::swap(top[0], top[1]);
  if (bot[0].x > bot[1].x) std::swap(bot[0], bot[1]);

  // Résultat final : TL, BL, BR, TR
  ordered = { top[0], bot[0], bot[1], top[1] };
  return true;
}


/**
 * @brief Détecte les coins d’une feuille A4 dans une image couleur.
 *
 * Cette fonction applique une série d'étapes robustes :
 * - conversion en niveaux de gris,
 * - floutage pour réduire le bruit,
 * - seuillage dynamique basé sur la luminance moyenne d'une ROI centrale,
 * - détection des contours,
 * - approximation polygonale pour vérifier qu’on a bien un quadrilatère.
 *
 * Si un contour quadrilatéral est trouvé, ses 4 coins sont ordonnés (TL, TR, BR, BL)
 * et retournés dans `imagePts`.
 *
 * @param frameBGR Image couleur d’entrée (BGR).
 * @param imagePts Vecteur de 4 points 2D (float) retourné si détection réussie.
 * @return true si une feuille A4 (ou une forme assimilée) est détectée.
 */
bool detectA4Corners(const cv::Mat& frameBGR,
                     std::vector<cv::Point2f>& imagePts) {
  // --- Étape 1 : conversion en niveaux de gris ---
  cv::Mat gray, blurred, thresh;
  cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);

  // --- Étape 2 : flou Gaussien pour lisser les détails/bruits ---
  cv::GaussianBlur(gray, blurred, cv::Size(7,7), 0);

  // --- Étape 3 : seuillage dynamique basé sur la luminance moyenne du centre ---
  const int H = blurred.rows, W = blurred.cols;
  const int rw = W/2, rh = H/2;                   // taille ROI = moitié image
  const int rx = (W-rw)/2, ry = (H-rh)/2;         // coin haut-gauche du ROI
  const cv::Rect roi(rx, ry, rw, rh);             // ROI centrale
  cv::Mat roiImg = blurred(roi);
  cv::Mat tempThresh; // image temporaire pour Otsu dans la ROI
  
  // Cette fonction retourne le seuil calculé (ex: 145.0) sans modifier l'image entière
  double calculatedT = cv::threshold(roiImg, tempThresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

  // 3. Sécurité : Si le centre est trop noir (caméra cachée), on rejette
  if (calculatedT < 30.0) return false;

  // 4. On applique ce seuil calculé à TOUTE l'image
  // Ici on utilise le seuil 'calculatedT' qu'on vient de trouver
  cv::threshold(blurred, thresh, calculatedT, 255, cv::THRESH_BINARY);

  
  // --- Étape 4 : détection de contours ---
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hier;
  cv::findContours(thresh, contours, hier, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (contours.empty()) return false;

  // --- Étape 5 : trouver le plus grand contour ---
  double maxA = 0; int maxIdx = -1;
  for (int i = 0; i < (int)contours.size(); ++i){
    const double A = std::fabs(cv::contourArea(contours[i]));
    if (A > maxA){ maxA = A; maxIdx = i; }
  }
  if (maxIdx < 0) return false;

  // --- Étape 6 : approximer ce contour avec un polygone à 4 côtés ---
  std::vector<cv::Point> approx;
  const double eps = 0.02 * cv::arcLength(contours[maxIdx], true); // epsilon = 2% du périmètre
  cv::approxPolyDP(contours[maxIdx], approx, eps, true);

  // Si ce n’est pas un quadrilatère → rejet
  if (approx.size() != 4) return false;

  // --- Étape 7 : ordonner les coins dans l’ordre TL, TR, BR, BL ---
  return orderFourCorners(approx, imagePts);
}


/**
 * @brief Dessine les coins détectés avec des couleurs et étiquettes.
 *
 * - TL (rouge)
 * - BL (jaune)
 * - BR (bleu)
 * - TR (vert)
 *
 * @param img Image sur laquelle dessiner.
 * @param pts Points dans l'ordre TL, BL, BR, TR.
 */
void drawOrderedCorners(cv::Mat& img, const std::vector<cv::Point2f>& pts) {
  if (pts.size() != 4) return;

  const cv::Scalar colors[4] = {
    {0, 0, 255},   // TL red
    {0, 255, 255}, // BL yellow
    {255, 0, 0},   // BR blue
    {0, 255, 0}    // TR green
  };
  const char* labels[4] = { "TL", "BL", "BR", "TR" };

  for (int i = 0; i < 4; i++) {
    cv::circle(img, pts[i], 8, colors[i], -1, cv::LINE_AA);
    cv::putText(img, labels[i], pts[i] + cv::Point2f(10, -10),
      cv::FONT_HERSHEY_SIMPLEX, 0.7, colors[i], 2, cv::LINE_AA);
  }
}

} // namespace detect
