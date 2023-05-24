#include "cornerDetrctor.hpp"
#include <chrono>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

void cornerDetrctor::cornerRunningAVG(std::vector<cv::Point> cornerPoints) {
    if (corners.empty()) corners = cornerPoints;
    else {
        for (int i = 0; i < corners.size(); i++) {
            corners[i].x = int((corners[i].x * 0.7) + (cornerPoints[i].x * 0.3));
            corners[i].y = int((corners[i].y * 0.7) + (cornerPoints[i].y * 0.3));
        }
    }
}

std::vector<cv::Point> cornerDetrctor::findCorners(cv::Mat&& imgBgr) {
    auto t0 = clk::now();
    cv::Mat imgEdges = makeEdgeImage(imgBgr);
    std::vector<cv::Point> cornerPoints = getCorners(imgEdges);
    if (cornerPoints.size() == 4) cornerPoints = orderPoints(cornerPoints);
    this->cornerRunningAVG(cornerPoints);
    auto t1 = clk::now();
   __android_log_print(ANDROID_LOG_DEBUG, "findCorners", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());

    return this->corners;
}

cv::Mat cornerDetrctor::makeEdgeImage(const cv::Mat& imgBgr) const {
    cv::Mat imgGray;
    cv::cvtColor(imgBgr, imgGray, cv::COLOR_BGR2GRAY);

    cv::Mat imgBlur;
    cv::GaussianBlur(imgGray, imgBlur, cv::Size(BLUR_KERNEL_SIZE, BLUR_KERNEL_SIZE), GAUSS_SIGMA);

    cv::Mat imgEdgesCanny;
    cv::Canny(imgBlur, imgEdgesCanny, THRESHOLD_CANNY_MIN, THRESHOLD_CANNY_MAX);

    cv::Mat dilationKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(DILATION_KERNEL_SIZE, DILATION_KERNEL_SIZE));
    cv::Mat imgEdgesDilated;
    cv::dilate(imgEdgesCanny, imgEdgesDilated, dilationKernel);

    return imgEdgesDilated;
}

std::vector<cv::Point> cornerDetrctor::getCorners(const cv::Mat& imgEdges) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(imgEdges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) throw std::runtime_error("No contours found");

    std::vector<cv::Point> shapePoints = findLargestShapePoints(contours);
    std::vector<cv::Point> cornerPoints = approxCornerPoints(shapePoints, imgEdges);
    return cornerPoints;
}

std::vector<cv::Point> cornerDetrctor::findLargestShapePoints(const std::vector<std::vector<cv::Point>>& contours) {
    std::vector<cv::Point> shapePoints;
    float PERIMETER_MARGIN_PERCENT = 0.02;
    double maxPerimeter = 0;
    for (const std::vector<cv::Point>& contour : contours) {
        std::vector<cv::Point> contour2f;
        cv::approxPolyDP(contour, contour2f, (PERIMETER_MARGIN_PERCENT * cv::arcLength(contour, true)), true);

        double perimeter = cv::arcLength(contour2f, false);
        if (perimeter > maxPerimeter) {
            cv::approxPolyDP(contour, shapePoints, (PERIMETER_MARGIN_PERCENT * perimeter), false);
            maxPerimeter = perimeter;
        }
    }

    return shapePoints;
}

std::vector<cv::Point> cornerDetrctor::approxCornerPoints(std::vector<cv::Point> shapePoints, const cv::Mat& img) {
    std::vector<cv::Point> imgCorners = {cv::Point(0, 0), cv::Point(img.cols, 0), cv::Point(img.cols, img.rows), cv::Point(0, img.rows)};
    for (auto & imgCorner : imgCorners) {
        if (shapePoints.empty()) break;

        int minDistanceIndex = getMinDistanceIndex(shapePoints, imgCorner);

        imgCorner = shapePoints[minDistanceIndex];
        shapePoints.erase(shapePoints.begin() + minDistanceIndex);
    }

    return imgCorners;
}

int cornerDetrctor::getMinDistanceIndex(std::vector<cv::Point> points, cv::Point targetPoint) {
    int minDistanceIndex = 0;
    double minDistance = std::numeric_limits<double>::max();
    for (int i = 0; i < points.size(); i++) {
        double distance = cv::norm(points[i] - targetPoint);
        if (distance < minDistance) {
            minDistance = distance;
            minDistanceIndex = i;
        }
    }

    return minDistanceIndex;
}

std::vector<cv::Point> cornerDetrctor::orderPoints(std::vector<cv::Point> cornerPoints) {
    int diffMin = std::numeric_limits<int>::max();
    int diffMinIndex = 0;
    int diffMax = std::numeric_limits<int>::min();
    int diffMaxIndex = 0;
    int sumMin = std::numeric_limits<int>::max();
    int sumMinIndex = 0;
    int sumMax = std::numeric_limits<int>::min();
    int sumMaxIndex = 0;
    for (int i = 0; i < cornerPoints.size(); i++) {
        int diff = cornerPoints[i].x - cornerPoints[i].y;
        int sum = cornerPoints[i].x + cornerPoints[i].y;

        if (diff < diffMin) {
            diffMin = diff;
            diffMinIndex = i;
        }

        if (diff > diffMax) {
            diffMax = diff;
            diffMaxIndex = i;
        }

        if (sum < sumMin) {
            sumMin = sum;
            sumMinIndex = i;
        }

        if (sum > sumMax) {
            sumMax = sum;
            sumMaxIndex = i;
        }
    }

    std::vector<cv::Point> orderedPoints = {cornerPoints[sumMinIndex], cornerPoints[diffMaxIndex], cornerPoints[sumMaxIndex], cornerPoints[diffMinIndex]};
    return orderedPoints;
}

void cornerDetrctor::drawCorners(std::vector<cv::Point> cornerPoints, cv::Mat& imgBgr) {
    for (int i = 0; i < cornerPoints.size(); i++) {
        cv::circle(imgBgr, cornerPoints[i], 10, cv::Scalar(0, 0, 255), cv::FILLED);
        cv::putText(imgBgr, std::to_string(i), cornerPoints[i], cv::FONT_HERSHEY_PLAIN, 4, cv::Scalar(0, 255, 0), 4);
    }
}