//
// Created by gislu on 26/04/2023.
//

#pragma once

#include "CaptureService.cpp"
#include "perspectiveTransformer.hpp"
#include <opencv2/opencv.hpp>

class captureActivity {
private:
    CaptureService captureService;// = CaptureService(cv::Size_<int>(), "");
    perspectiveTransformer PerspectiveTransformer{};
public:
    std::vector<cv::Point> corners;
    captureActivity(std::vector<cv::Point>&& corners, cv::Size imgBgr);
    captureActivity(std::vector<cv::Point>&& corners, cv::Size&& imgBgr, const std::string& path);
    cv::Mat capture(cv::Mat&& imgBgr);
};
