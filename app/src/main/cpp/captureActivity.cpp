#include "captureActivity.hpp"
#include <chrono>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

captureActivity::captureActivity(std::vector<cv::Point> corners, cv::Mat imgBgr) : corners(corners) {
    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners); //TODO: move to init function
    if (!imgPerspective.empty()) {
        captureService = CaptureService(imgPerspective.size());
    } else {
        throw std::invalid_argument("Invalid corners");
    }
}

auto captureActivity::capture(cv::Mat& imgBgr) -> cv::Mat {
    // Perspective transform
    auto t0 = clk::now();
    cv::Mat imgPerspective = PerspectiveTransformer.getPerspective(imgBgr, corners);
    auto t1 = clk::now();
    std::cout << "PerspectiveTransformer loop took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;

    if (!imgPerspective.empty()) {
        // Capture
        return captureService.capture(imgPerspective);
    }
    return {};

}

