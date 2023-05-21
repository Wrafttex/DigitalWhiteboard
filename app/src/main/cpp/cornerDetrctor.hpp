#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

class cornerDetrctor {
private:
    const int THRESHOLD_CANNY_MIN = 20;
    const int THRESHOLD_CANNY_MAX = 150;

    const int BLUR_KERNEL_SIZE = 5;
    const int GAUSS_SIGMA = 1;

    const int DILATION_KERNEL_SIZE = 3;

    std::vector<cv::Point> corners;

public:
    void cornerRunningAVG(std::vector<cv::Point> cornerPoints);
    std::vector<cv::Point> findCorners(cv::Mat&& imgBgr);
    cv::Mat makeEdgeImage(const cv::Mat& imgBgr) const;
    static std::vector<cv::Point> getCorners(const cv::Mat& imgEdges);
    static std::vector<cv::Point> findLargestShapePoints(const std::vector<std::vector<cv::Point>>& contours);
    static std::vector<cv::Point> approxCornerPoints(std::vector<cv::Point> shapePoints, const cv::Mat& img);
    static int getMinDistanceIndex(std::vector<cv::Point> points, cv::Point targetPoint);
    static std::vector<cv::Point> orderPoints(std::vector<cv::Point> cornerPoints);
    static void drawCorners(std::vector<cv::Point> cornerPoints, cv::Mat& imgBgr);
};
