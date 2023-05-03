#include "captureActivity.hpp"
#include "jni.h"

captureActivity::captureActivity(std::vector<cv::Point> corners, cv::Mat imgBgr) : corners(corners) {
    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners);
    if (!imgPerspective.empty()) {
        captureService = CaptureService(imgPerspective.size());
    } else {
        throw std::invalid_argument("Invalid corners");
    }
}

auto captureActivity::capture(cv::Mat& imgBgr) -> cv::Mat {
    // Perspective transform
    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners);

    if (!imgPerspective.empty()) {
        // Capture
        return captureService.capture(imgPerspective);
    }
    return {};

}

