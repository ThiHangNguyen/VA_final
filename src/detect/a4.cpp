#include "detect/a4.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <opencv2/highgui.hpp> // Pour imshow et les fenêtres

namespace detect {

std::vector<cv::Point3f> getA4ObjectPoints() {
    const float W = 210.f;
    const float H = 297.f;
    return {
        {-W * 0.5f, -H * 0.5f, 0.0f}, // Top-Left
        {+W * 0.5f, -H * 0.5f, 0.0f}, // Top-Right
        {+W * 0.5f, +H * 0.5f, 0.0f}, // Bottom-Right
        {-W * 0.5f, +H * 0.5f, 0.0f}  // Bottom-Left
    };
}

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

bool orderFourCorners(const std::vector<cv::Point>& approx, std::vector<cv::Point2f>& ordered) {
    if (approx.size() != 4) return false;

    std::vector<cv::Point2f> pts;
    for (const auto& p : approx) pts.push_back(cv::Point2f(p.x, p.y));

    // Calcul du centre
    cv::Point2f center(0, 0);
    for (const auto& p : pts) center += p;
    center *= 0.25f;

    // Tri par angle
    std::sort(pts.begin(), pts.end(), [center](const cv::Point2f& a, const cv::Point2f& b) {
        return atan2(a.y - center.y, a.x - center.x) < atan2(b.y - center.y, b.x - center.x);
    });

    // ÉTAPE CRUCIALE : On vérifie l'orientation (Produit en croix)
    // Si le sens est CCW, on inverse pour obtenir CW (Axe Z vers caméra)
    float area = 0;
    for (int i = 0; i < 4; i++) {
        area += (pts[i].x * pts[(i+1)%4].y - pts[(i+1)%4].x * pts[i].y);
    }
    if (area > 0) { // Si l'aire est positive, c'est CCW dans OpenCV (Y vers le bas)
        std::reverse(pts.begin(), pts.end());
    }

    // On recale le point 0 sur le Haut-Gauche (somme x+y minimale)
    int tl = 0;
    float minSum = pts[0].x + pts[0].y;
    for(int i=1; i<4; i++) {
        if (pts[i].x + pts[i].y < minSum) {
            minSum = pts[i].x + pts[i].y;
            tl = i;
        }
    }

    ordered.clear();
    for (int i = 0; i < 4; i++) {
        ordered.push_back(pts[(tl + i) % 4]);
    }

    return true;
}

// --- 1. RECHERCHE DE CONTOUR (FONCTION UTILITAIRE) ---
bool findBestA4Contour(
    const cv::Mat& mask,
    std::vector<cv::Point>& outApprox,
    int W,
    int H
) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double bestScore = 0.0;
    std::vector<cv::Point> best;

    for (const auto& c : contours) {
        double peri = cv::arcLength(c, true);
        if (peri < (W + H) * 0.3) continue; // trop petit → bruit

        std::vector<cv::Point> approx;
        cv::approxPolyDP(c, approx, 0.02 * peri, true);

        if (approx.size() != 4) continue;
        if (!cv::isContourConvex(approx)) continue;

        double area = std::fabs(cv::contourArea(approx));
        if (area < W * H * 0.02) continue;

        // score = périmètre + aire → favorise la vraie feuille
        double score = peri + 0.001 * area;
        if (score > bestScore) {
            bestScore = score;
            best = approx;
        }
    }

    if (!best.empty()) {
        outApprox = best;
        return true;
    }
    return false;
}

// --- MÉTHODE SUPPLÉMENTAIRE : LISSAGE TEMP RELATIF ---
void smoothCorners(std::vector<cv::Point2f>& current, const std::vector<cv::Point2f>& previous) {
    // Facteur de lissage : 0.0 (garde l'ancien) à 1.0 (prend le nouveau tout de suite)
    // 0.2 est une valeur très stable pour la réalité augmentée
    const float alpha = 0.2f; 

    for (int i = 0; i < 4; i++) {
        current[i].x = (1.0f - alpha) * previous[i].x + alpha * current[i].x;
        current[i].y = (1.0f - alpha) * previous[i].y + alpha * current[i].y;
    }
}
bool detectA4Corners(
    const cv::Mat& frameBGR,
    std::vector<cv::Point2f>& imagePts
) {
    imagePts.clear();

    const int W = frameBGR.cols;
    const int H = frameBGR.rows;

    // =============================
    // 1. PRÉTRAITEMENT
    // =============================
    cv::Mat gray, blurred;
    cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(7, 7), 0);

    // =============================
    // 2. SEUIL DYNAMIQUE (ROI CENTRALE)
    // =============================
    const int rw = W / 3;
    const int rh = H / 3;
    const int rx = (W - rw) / 2;
    const int ry = (H - rh) / 2;

    cv::Rect roi(rx, ry, rw, rh);
    double meanLuma = cv::mean(blurred(roi))[0];
    double threshVal = std::clamp(meanLuma - 15.0, 10.0, 240.0);

    std::vector<cv::Point> approx;

    // =============================
    // ÉTAPE 1 — THRESHOLD DYNAMIQUE
    // =============================
    {
        cv::Mat bin;
        cv::threshold(blurred, bin, threshVal, 255, cv::THRESH_BINARY);

        if (findBestA4Contour(bin, approx, W, H)) {
            return orderFourCorners(approx, imagePts);
        }
    }

    // =============================
    // ÉTAPE 2 — OTSU
    // =============================
    {
        cv::Mat bin;
        cv::threshold(blurred, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        if (findBestA4Contour(bin, approx, W, H)) {
            return orderFourCorners(approx, imagePts);
        }
    }

    // =============================
    // ÉTAPE 3 — CANNY (dernier recours)
    // =============================
    {
        cv::Mat edges;
        cv::Canny(blurred, edges, 30, 80);

        if (findBestA4Contour(edges, approx, W, H)) {
            return orderFourCorners(approx, imagePts);
        }
    }

    return false;
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