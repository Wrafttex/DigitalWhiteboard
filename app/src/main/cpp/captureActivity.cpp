#include "captureActivity.hpp"
#include <chrono>
#include <utility>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

captureActivity::captureActivity(std::vector<cv::Point>&& corners, cv::Size imgSize) : corners(corners), captureService(imgSize) {}

captureActivity::captureActivity(std::vector<cv::Point>&& corners, cv::Size&& imgSize, const std::string& path) : corners(std::move(corners)), captureService(CaptureService(imgSize, path)) {
//    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners); //TODO: move to init function
//    if (!imgPerspective.empty()) {
//        captureService = CaptureService(imgPerspective.size(), std::move(path));
//    } else {
//        throw std::invalid_argument("Invalid corners");
//    }
}

auto captureActivity::capture(cv::Mat&& imgBgr) -> cv::Mat {
    // Perspective transform
    auto t0 = clk::now();
    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners);
    auto t1 = clk::now();
    __android_log_print(ANDROID_LOG_DEBUG, "PerspectiveTransformer", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());
//    std::cout << "PerspectiveTransformer loop took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;

    if (!imgPerspective.empty()) {
        // Capture
        return captureService.capture(imgPerspective);
    }
    return imgPerspective;

}

