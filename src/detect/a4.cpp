/**
 * @file a4.cpp
 * @brief Implementation de la detection de feuille A4.
 * @author Thi Hang NGUYEN & Bichoy DAOUD
 */

#include "detect/a4.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

namespace detect {

/**
 * @brief Retourne les coordonnees 3D des coins d'une feuille A4.
 *
 * Format A4: 210mm x 297mm, centre a l'origine.
 * L'ordre correspond a celui de orderFourCornersGeometric: TL, TR, BR, BL
 */
std::vector<cv::Point3f> getA4ObjectPoints() {
    const float W = 210.f;  // Largeur A4 en mm
    const float H = 297.f;  // Hauteur A4 en mm
    return {
        {-W * 0.5f, -H * 0.5f, 0.0f}, // Top-Left
        {+W * 0.5f, -H * 0.5f, 0.0f}, // Top-Right
        {+W * 0.5f, +H * 0.5f, 0.0f}, // Bottom-Right
        {-W * 0.5f, +H * 0.5f, 0.0f}  // Bottom-Left
    };
}

// ===========================================================
// Variables statiques pour le tracking inter-frames
// ===========================================================
static std::vector<cv::Point2f> prevCorners;  ///< Coins de la frame precedente
static bool hasTracking = false;               ///< True si on a deja detecte au moins une fois
static int lostFramesCount = 0;                ///< Nb de frames sans detection
const int MAX_LOST_FRAMES = 3;                 ///< Tolerance avant de perdre le tracking

/// Distance au carre entre 2 points (pour eviter sqrt)
static double distSq(const cv::Point2f& p1, const cv::Point2f& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return dx*dx + dy*dy;
}

// ===========================================================
// Tri des coins par methode geometrique (somme/difference)
// ===========================================================
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

    // Ordre final: TL, BL, BR, TR (sens anti-horaire depuis le haut-gauche)
    ordered[0] = approx[idxTL];
    ordered[1] = approx[idxBL];
    ordered[2] = approx[idxBR];
    ordered[3] = approx[idxTR];
    return true;
}

// ===========================================================
// Tri par tracking (associe chaque coin au plus proche de la frame precedente)
// ===========================================================
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

// ===========================================================
// RECHERCHE DU MEILLEUR CONTOUR A4 (fonction utilitaire)
// ===========================================================
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

// ===========================================================
// DETECTION PRINCIPALE (3 méthodes en cascade)
// ===========================================================
bool detectA4Corners(const cv::Mat& frameBGR, std::vector<cv::Point2f>& imagePts) {
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
    // 2. SEUIL DYNAMIQUE (basé sur luminosité centrale)
    // =============================
    const int rw = W / 3;
    const int rh = H / 3;
    const int rx = (W - rw) / 2;
    const int ry = (H - rh) / 2;

    cv::Rect roi(rx, ry, rw, rh);
    double meanLuma = cv::mean(blurred(roi))[0];
    double threshVal = std::max(10.0, std::min(240.0, meanLuma - 15.0));

    std::vector<cv::Point> approx;

    // =============================
    // MÉTHODE 1 — THRESHOLD DYNAMIQUE
    // =============================
    {
        cv::Mat bin;
        cv::threshold(blurred, bin, threshVal, 255, cv::THRESH_BINARY);

        // Morphologie pour améliorer la détection
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(bin, bin, cv::MORPH_CLOSE, kernel);

        if (findBestA4Contour(bin, approx, W, H)) {
            if (orderFourCornersGeometric(approx, imagePts)) {
                // Persistance pour stabilité
                if (hasTracking) {
                    orderCornersTracking(approx, imagePts);
                }
                prevCorners = imagePts;
                hasTracking = true;
                lostFramesCount = 0;
                return true;
            }
        }
    }

    // =============================
    // MÉTHODE 2 — OTSU (adaptatif)
    // =============================
    {
        cv::Mat bin;
        cv::threshold(blurred, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(bin, bin, cv::MORPH_CLOSE, kernel);
        cv::dilate(bin, bin, kernel);

        if (findBestA4Contour(bin, approx, W, H)) {
            if (orderFourCornersGeometric(approx, imagePts)) {
                if (hasTracking) {
                    orderCornersTracking(approx, imagePts);
                }
                prevCorners = imagePts;
                hasTracking = true;
                lostFramesCount = 0;
                return true;
            }
        }
    }

    // =============================
    // MÉTHODE 3 — CANNY (dernier recours)
    // =============================
    {
        cv::Mat edges;
        cv::Canny(blurred, edges, 30, 80);

        // Dilatation pour connecter les bords
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::dilate(edges, edges, kernel);

        if (findBestA4Contour(edges, approx, W, H)) {
            if (orderFourCornersGeometric(approx, imagePts)) {
                if (hasTracking) {
                    orderCornersTracking(approx, imagePts);
                }
                prevCorners = imagePts;
                hasTracking = true;
                lostFramesCount = 0;
                return true;
            }
        }
    }

    // =============================
    // ÉCHEC - Persistance rétinienne
    // =============================
    if (hasTracking && lostFramesCount < MAX_LOST_FRAMES) {
        imagePts = prevCorners;
        lostFramesCount++;
        return true;
    }

    hasTracking = false;
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

// ===========================================================
// DETECTION DE FLOU (Variance du Laplacien)
// ===========================================================

double measureBlur(const cv::Mat& frame) {
    if (frame.empty()) return 0.0;

    cv::Mat gray;
    if (frame.channels() == 3) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame;
    }

    // Calcul du Laplacien (détection de bords)
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);

    // Calcul de la variance (mean et stddev)
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);

    // La variance est le carré de l'écart-type
    double variance = stddev[0] * stddev[0];

    return variance;
}

bool isBlurry(const cv::Mat& frame, double threshold) {
    double sharpness = measureBlur(frame);
    return sharpness < threshold;
}

} // namespace detect
