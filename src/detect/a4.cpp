#include "detect/a4.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

namespace detect {

// --- VARIABLES STATIQUES ---
static std::vector<cv::Point2f> prevCorners;
static bool hasTracking = false;
// Compteur pour garder l'affichage quelques frames si on perd la détection (anti-clignotement)
static int lostFramesCount = 0; 
const int MAX_LOST_FRAMES = 5; 

// Distance au carré
static double distSq(const cv::Point2f& p1, const cv::Point2f& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return dx*dx + dy*dy;
}

// --- TRI GÉOMÉTRIQUE ROBUSTE (Somme/Différence) ---
bool orderFourCornersGeometric(const std::vector<cv::Point>& approx,
                               std::vector<cv::Point2f>& ordered)
{
    if (approx.size() != 4) return false;
    ordered.resize(4);

    float minSum = 1e9, maxSum = -1e9;
    float minDiff = 1e9, maxDiff = -1e9;
    int idxTL = 0, idxBR = 0, idxTR = 0, idxBL = 0;

    for (int i = 0; i < 4; i++) {
        float s = (float)(approx[i].x + approx[i].y);
        float d = (float)(approx[i].y - approx[i].x);

        if (s < minSum) { minSum = s; idxTL = i; } // TL
        if (s > maxSum) { maxSum = s; idxBR = i; } // BR
        if (d < minDiff) { minDiff = d; idxTR = i; } // TR
        if (d > maxDiff) { maxDiff = d; idxBL = i; } // BL
    }
    
    ordered[0] = approx[idxTL]; 
    ordered[1] = approx[idxBL]; 
    ordered[2] = approx[idxBR]; 
    ordered[3] = approx[idxTR]; 
    return true;
}

// --- TRI PAR TRACKING ---
bool orderCornersTracking(const std::vector<cv::Point>& approx,
                          std::vector<cv::Point2f>& ordered)
{
    if (approx.size() != 4) return false;
    ordered.resize(4);
    std::vector<bool> used(4, false);

    for (int i = 0; i < 4; i++) {
        int bestIdx = -1;
        double minDist = 1e9;

        for (int j = 0; j < 4; j++) {
            if (used[j]) continue;
            double d = distSq(prevCorners[i], cv::Point2f((float)approx[j].x, (float)approx[j].y));
            if (d < minDist) { minDist = d; bestIdx = j; }
        }

        // Si on bouge très vite, la distance peut être grande.
        // On augmente la tolérance à ~300px (90000) car avec le flou, les coins "glissent".
        if (bestIdx == -1 || minDist > 90000.0) return false; 

        ordered[i] = cv::Point2f((float)approx[bestIdx].x, (float)approx[bestIdx].y);
        used[bestIdx] = true;
    }
    return true;
}

// --- DÉTECTION PRINCIPALE ---
bool detectA4Corners(const cv::Mat& frameBGR, std::vector<cv::Point2f>& imagePts) {
  
  // 1. Pré-traitement
  cv::Mat gray, blurred, thresh;
  cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);
  
  // Flou léger pour enlever le bruit caméra
  cv::GaussianBlur(gray, blurred, cv::Size(5,5), 0);

  // 2. Otsu Robuste
  const int H = blurred.rows, W = blurred.cols;
  // On prend un échantillon central pour calculer le seuil (évite les bordures noires de caméra)
  cv::Rect roi(W/4, H/4, W/2, H/2); 
  cv::Mat roiImg = blurred(roi);
  cv::Mat tempThresh;
  
  double calculatedT = cv::threshold(roiImg, tempThresh, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
  if (calculatedT < 40.0) calculatedT = 40.0; // Sécurité ambiance sombre
  
  cv::threshold(blurred, thresh, calculatedT, 255, cv::THRESH_BINARY);

  // --- AMÉLIORATION MAJEURE : MORPHOLOGIE ---
  // C'est ICI qu'on gère le mouvement rapide.
  // "Dilater" puis "Eroder" (Close) va reconnecter les lignes brisées par le flou de bougé.
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
  cv::morphologyEx(thresh, thresh, cv::MORPH_CLOSE, kernel); 
  // On dilate un peu pour "engraisser" les contours fins
  cv::dilate(thresh, thresh, kernel);

  // 3. Contours
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (contours.empty()) { 
      // Si on perd le tracking, on garde l'ancienne position quelques frames (persistance rétinienne)
      if (hasTracking && lostFramesCount < MAX_LOST_FRAMES) {
          imagePts = prevCorners;
          lostFramesCount++;
          return true;
      }
      hasTracking = false; return false; 
  }

  // Trouver le plus grand contour
  double maxA = 0; int maxIdx = -1;
  for (int i = 0; i < (int)contours.size(); ++i){
    const double A = std::fabs(cv::contourArea(contours[i]));
    if (A > maxA && A > (W*H*0.02)){ // Doit faire au moins 2% de l'image
        maxA = A; maxIdx = i; 
    }
  }
  
  if (maxIdx < 0) { 
       if (hasTracking && lostFramesCount < MAX_LOST_FRAMES) {
          imagePts = prevCorners;
          lostFramesCount++;
          return true;
      }
      hasTracking = false; return false; 
  }

  // --- AMÉLIORATION MAJEURE : CONVEX HULL ---
  // Si le mouvement est flou, le contour est dentelé.
  // ConvexHull crée une enveloppe lisse autour (comme un élastique), ce qui redonne 4 coins propres.
  std::vector<cv::Point> hull;
  cv::convexHull(contours[maxIdx], hull);

  std::vector<cv::Point> approx;
  // Approximation polygonale sur le Hull, pas sur le contour brut !
  double eps = 0.04 * cv::arcLength(hull, true); // 0.04 est plus permissif que 0.02
  cv::approxPolyDP(hull, approx, eps, true);

  // Si on a raté, on réessaie avec un epsilon plus large
  if (approx.size() != 4) {
      eps = 0.05 * cv::arcLength(hull, true);
      cv::approxPolyDP(hull, approx, eps, true);
  }

  if (approx.size() != 4) { 
       if (hasTracking && lostFramesCount < MAX_LOST_FRAMES) {
          imagePts = prevCorners;
          lostFramesCount++;
          return true;
      }
      hasTracking = false; return false; 
  }

  // 4. Tri et Validation
  bool ok = false;

  if (hasTracking) {
      ok = orderCornersTracking(approx, imagePts);
      if (!ok) ok = orderFourCornersGeometric(approx, imagePts);
  } else {
      ok = orderFourCornersGeometric(approx, imagePts);
  }

  if (ok) {
      prevCorners = imagePts;
      hasTracking = true;
      lostFramesCount = 0; // Reset du compteur de perte
  } else {
      // Si le tri échoue mais qu'on avait un tracking, on temporise
      if (hasTracking && lostFramesCount < MAX_LOST_FRAMES) {
          imagePts = prevCorners;
          lostFramesCount++;
          return true;
      }
      hasTracking = false;
  }

  return ok;
}

void drawOrderedCorners(cv::Mat& img, const std::vector<cv::Point2f>& pts) {
  if (pts.size() != 4) return;
  const cv::Scalar colors[4] = {{0,0,255}, {0,255,255}, {255,0,0}, {0,255,0}}; // TL, BL, BR, TR
  for (int i = 0; i < 4; i++) {
    cv::line(img, pts[i], pts[(i+1)%4], colors[i], 3); // Lignes plus épaisses
    cv::circle(img, pts[i], 5, colors[i], -1);
  }
}

} // namespace detect