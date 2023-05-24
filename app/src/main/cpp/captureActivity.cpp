#include "captureActivity.hpp"
#include <chrono>
#include <utility>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

captureActivity::captureActivity(const std::vector<cv::Point>& corners, const cv::Size& imgSize, const cv::Mat& imgBgr) : corners(corners), captureService(imgSize) {
    pt = perspectiveTransformer{imgBgr, corners};
}

captureActivity::captureActivity(const std::vector<cv::Point>& corners, const cv::Size& imgSize, const std::string& path):
    corners(corners), captureService(CaptureService(imgSize, path)) {}

auto captureActivity::capture(const cv::Mat& imgBgr) -> cv::Mat {
    // Perspective transform
    auto t0 = clk::now();
    cv::Mat imgPerspective = pt.perspectiveWraping(imgBgr);
    auto t1 = clk::now();
    rounds++;
    totalTime += duration<double, std::milli>(t1-t0).count();
    __android_log_print(ANDROID_LOG_DEBUG, "PerspectiveTransformer", "avg: %s ms", std::to_string(totalTime/rounds).c_str());

    if (!imgPerspective.empty()) {
        // Capture
        return captureService.capture(imgPerspective);
    }
    return imgPerspective;

}

