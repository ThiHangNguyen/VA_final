#pragma once
/**
 * @file roi.hpp
 * @brief ROI adaptative + compteur "lost" + reset plein cadre.
 *
 * Cycle:
 * 1) On travaille dans la ROI courante.
 * 2) Si détection OK -> ROI = bbox(coins) + marge (pad).
 * 3) Si échec -> lost++ ; quand lost dépasse le seuil -> reset plein cadre.
 *
 * NB: On ne fait PAS la détection ici, juste la gestion ROI & suivi LK (optionnel).
 */

#include <opencv2/opencv.hpp>
#include <vector>

namespace detect {

struct AdaptiveROIParams {
  int   padPx          = 40;     ///< marge autour de la bbox en pixels
  int   minW           = 160;    ///< largeur mini ROI
  int   minH           = 120;    ///< hauteur mini ROI
  int   lostReset      = 8;      ///< nb frames perdues avant plein cadre
  bool  debug          = false;  ///< dessiner la ROI
};

class AdaptiveROI {
public:
  void setParams(const AdaptiveROIParams& p) { params_ = p; }
  void init(const cv::Size& frameSize, const cv::Rect& initBox = cv::Rect());

  cv::Rect getRect() const { return roi_; }

  /// Mise à jour ROI à partir de 4 coins (TL,BL,BR,TR) -> bbox + pad
  void updateWithCorners(const std::vector<cv::Point2f>& corners4);

  /// La frame courante n'a pas validé la feuille
  void markLost();

  /// Reset plein cadre (et lost=0)
  void reset();

  /// Suivi LK (fallback). Retourne vrai si les 4 points sont suivis avec un taux d'outliers faible.
  bool trackLK(const cv::Mat& prevGray, const cv::Mat& gray,
               const std::vector<cv::Point2f>& prevPts4,
               std::vector<cv::Point2f>& nextPts4) const;

  /// Dessin debug de la ROI
  void drawDebug(cv::Mat& bgr) const;

private:
  cv::Size frameSize_{};
  cv::Rect roi_{};
  AdaptiveROIParams params_{};
  int lostCount_ = 0;

  static cv::Rect clampRect(const cv::Rect& r, const cv::Size& sz);
};

} // namespace detect
