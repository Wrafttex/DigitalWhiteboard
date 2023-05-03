//
// Created by gislu on 26/04/2023.
//

#pragma once

#include "CaptureService.cpp"
#include "perspectiveTransformer.hpp"
#include <opencv2/opencv.hpp>

class captureActivity {
private:
    std::vector<cv::Point> corners;
    CaptureService captureService = CaptureService(cv::Size_<int>());
    perspectiveTransformer PerspectiveTransformer{};
public:
    captureActivity(std::vector<cv::Point> corners, cv::Mat imgBgr);
    cv::Mat capture(cv::Mat& imgBgr);
};
