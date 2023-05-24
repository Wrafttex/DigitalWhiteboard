//
// Created by gislu on 26/04/2023.
//

#pragma once

#include "CaptureService.cpp"
#include "perspectiveTransformer.hpp"
#include <opencv2/opencv.hpp>

class captureActivity {
private:
    CaptureService captureService;
    perspectiveTransformer pt = perspectiveTransformer{};
    int rounds = 0;
    double totalTime = 0;
public:
    std::vector<cv::Point> corners;
    captureActivity(const std::vector<cv::Point>& corners, const cv::Size& imgSize, const cv::Mat& imgBgr);
    captureActivity(const std::vector<cv::Point>& corners, const cv::Size& imgBgr, const std::string& path);
    cv::Mat capture(const cv::Mat& imgBgr);
};
