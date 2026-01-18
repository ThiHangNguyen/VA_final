#include "detect/tracking.hpp"

// ===============================
// Init
// ===============================
void initTracking(
    A4Tracker& tracker,
    const std::vector<cv::Point2f>& corners,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    const cv::Rect& roi
) {
    tracker.prevCorners = corners;
    tracker.currCorners = corners;

    tracker.rvec = rvec.clone();
    tracker.tvec = tvec.clone();
    tracker.rvecFiltered = rvec.clone();
    tracker.tvecFiltered = tvec.clone();

    tracker.lastROI = roi;
    tracker.lostFrames = 0;
    tracker.initialized = true;
}

// ===============================
// Optical Flow (Lucas–Kanade)
// ===============================
bool trackCornersLK(
    const cv::Mat& prevGray,
    const cv::Mat& currGray,
    A4Tracker& tracker
) {
    std::vector<uchar> status;
    std::vector<float> err;

    cv::calcOpticalFlowPyrLK(
        prevGray,
        currGray,
        tracker.prevCorners,
        tracker.currCorners,
        status,
        err,
        cv::Size(15, 15),  // Réduit de 21x21 à 15x15
        2,                  // Réduit de 3 à 2 niveaux de pyramide
        cv::TermCriteria(
            cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
            20,             // Réduit de 30 à 20 itérations
            0.01
        )
    );

    int valid = 0;
    for (uchar s : status) {
        if (s) valid++;
    }

    return valid == 4;
}

// ===============================
// Reprojection error
// ===============================
static double reprojectionError(
    const std::vector<cv::Point3f>& objectPts,
    const std::vector<cv::Point2f>& imagePts,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    const cv::Mat& K,
    const cv::Mat& dist
) {
    std::vector<cv::Point2f> projected;
    cv::projectPoints(objectPts, rvec, tvec, K, dist, projected);

    double err = 0.0;
    for (size_t i = 0; i < projected.size(); ++i) {
        err += cv::norm(projected[i] - imagePts[i]);
    }
    return err / projected.size();
}

// ===============================
// Pose update
// ===============================
bool updatePose(
    A4Tracker& tracker,
    const std::vector<cv::Point3f>& objectPts,
    const cv::Mat& K,
    const cv::Mat& dist,
    double maxReprojError
) {
    bool ok = cv::solvePnP(
        objectPts,
        tracker.currCorners,
        K,
        dist,
        tracker.rvec,
        tracker.tvec,
        true,
        cv::SOLVEPNP_ITERATIVE
    );

    if (!ok) return false;

    double err = reprojectionError(
        objectPts,
        tracker.currCorners,
        tracker.rvec,
        tracker.tvec,
        K,
        dist
    );

    return err < maxReprojError;
}

// ===============================
// EMA smoothing
// ===============================
void smoothPoseEMA(
    A4Tracker& tracker,
    double alpha
) {
    tracker.rvecFiltered =
        alpha * tracker.rvec + (1.0 - alpha) * tracker.rvecFiltered;

    tracker.tvecFiltered =
        alpha * tracker.tvec + (1.0 - alpha) * tracker.tvecFiltered;
}

// ===============================
// ROI update
// ===============================
cv::Rect computeROI(
    const std::vector<cv::Point2f>& corners,
    const cv::Size& frameSize,
    int margin
) {
    cv::Rect box = cv::boundingRect(corners);

    box.x = std::max(0, box.x - margin);
    box.y = std::max(0, box.y - margin);

    box.width = std::min(frameSize.width - box.x, box.width + 2 * margin);
    box.height = std::min(frameSize.height - box.y, box.height + 2 * margin);

    return box;
}

// ===============================
// Fallback logic
// ===============================
void handleTrackingState(
    A4Tracker& tracker,
    bool trackingOK,
    int maxLost
) {
    if (trackingOK) {
        tracker.lostFrames = 0; // Tout va bien
    } else {
        tracker.lostFrames++; // On commence à compter l'échec
    }

    // Si on a perdu le fil pendant X frames (ex: 5 ou 10)
    if (tracker.lostFrames > maxLost) {
        tracker.initialized = false; // CE DÉCLENCHEUR EST CRUCIAL
        tracker.prevCorners.clear();
    }
}

// ===============================
// Per-frame pipeline
// ===============================
void processTracking(
    const cv::Mat& prevGray,
    const cv::Mat& currGray,
    A4Tracker& tracker,
    const std::vector<cv::Point3f>& objectPts,
    const cv::Mat& K,
    const cv::Mat& dist
) {
    if (!tracker.initialized) return;

    // ⚡ OPTIMISATION : Ne faire solvePnP complet que toutes les N frames
    static int poseUpdateCounter = 0;
    poseUpdateCounter++;
    const int POSE_UPDATE_INTERVAL = 2; // Mise à jour complète 1 frame sur 2
    bool shouldUpdateFullPose = (poseUpdateCounter % POSE_UPDATE_INTERVAL == 0);

    // 1. Calcul du mouvement
    bool tracked = trackCornersLK(prevGray, currGray, tracker);

    // 2. Sécurité : mouvement brusque (seuil plus permissif pour mouvements rapides)
    if (tracked) {
        float movement = cv::norm(tracker.currCorners[0] - tracker.prevCorners[0]);
        if (movement > 80.0) tracked = false; // Augmenté de 40 à 80px
    }

    bool poseOK = false;
    if (tracked && shouldUpdateFullPose) {
        // 3. Calcul de la pose COMPLET (seuil plus permissif pour éviter les pertes)
        poseOK = updatePose(tracker, objectPts, K, dist, 8.0); // Augmenté de 4.0 à 8.0

        if (poseOK) {
            // --- VÉRIFICATION DE LA STABILITÉ DE L'ANGLE ---
            // Calcul de l'angle actuel
            double currentAngleDeg = cv::norm(tracker.rvec) * (180.0 / CV_PI);

            // On rejette si l'inclinaison est trop forte
            if (currentAngleDeg > 70.0) { // Augmenté de 50 à 70°
                poseOK = false;
            }

            // --- COMPARAISON AVEC LA FRAME PRÉCÉDENTE ---
            // Si l'angle change trop brutalement entre deux images, c'est une dérive
            static double lastAngle = 0;
            if (std::abs(currentAngleDeg - lastAngle) > 15.0) { // Augmenté de 8 à 15°
                poseOK = false;
            }
            lastAngle = currentAngleDeg;
        }

        if (poseOK) {
            // Lissage plus fort (alpha plus petit = plus de stabilité, moins de tremblements)
            smoothPoseEMA(tracker, 0.08);

            // 4. ANTI-DRIFT : Recalage géométrique sur la feuille
            std::vector<cv::Point2f> projected;
            cv::projectPoints(objectPts, tracker.rvec, tracker.tvec, K, dist, projected);
            tracker.currCorners = projected;

            tracker.prevCorners = tracker.currCorners;
            tracker.lastROI = computeROI(tracker.currCorners, currGray.size());
        }
    } else if (tracked && !shouldUpdateFullPose) {
        // ⚡ Entre les mises à jour complètes : juste utiliser les corners du tracking optique
        poseOK = true; // On garde la pose précédente (déjà dans tracker.rvecFiltered/tvecFiltered)
        tracker.prevCorners = tracker.currCorners;
    }

    // 5. Redétection si échec (Délai réduit à 3 frames pour réagir vite)
    handleTrackingState(tracker, tracked && poseOK, 3); 
}