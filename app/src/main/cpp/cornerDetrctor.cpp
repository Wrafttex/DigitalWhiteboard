#include "cornerDetrctor.hpp"
#include <chrono>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

void cornerDetrctor::cornerRunningAVG(std::vector<cv::Point>& cornerPoints) {
    if (corners.empty()) corners = cornerPoints;
    else {
        for (int i = 0; i < corners.size(); i++) {
            cv::Point& corner = corners[i];
            const cv::Point& newCorner = cornerPoints[i];
            corner.x = static_cast<int>((corner.x * 0.6) + (newCorner.x * 0.4));
            corner.y = static_cast<int>((corner.y * 0.6) + (newCorner.y * 0.4));
        }
    }
}

std::vector<cv::Point> cornerDetrctor::findCorners(cv::Mat& imgBgr) {
    auto t0 = clk::now();
    cv::Mat imgEdges = makeEdgeImage(imgBgr);
    std::vector<cv::Point> cornerPoints = getCorners(imgEdges);
    if (cornerPoints.size() == 4) cornerPoints = orderPoints(cornerPoints);
    this->cornerRunningAVG(cornerPoints);
    auto t1 = clk::now(); // TEST CODE
    rounds++;
    totalTime += duration<double, std::milli>(t1-t0).count();
    __android_log_print(ANDROID_LOG_DEBUG, "findCorners", "avg: %s ms", std::to_string(totalTime/rounds).c_str());

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
        double perimeterMargin = PERIMETER_MARGIN_PERCENT * cv::arcLength(contour, true);
        cv::approxPolyDP(contour, contour2f, perimeterMargin, true);

        double perimeter = cv::arcLength(contour2f, false);
        if (perimeter > maxPerimeter) {
            cv::approxPolyDP(contour, shapePoints, perimeterMargin, false);
            maxPerimeter = perimeter;
        }
    }

    return shapePoints;
}

std::vector<cv::Point> cornerDetrctor::approxCornerPoints(std::vector<cv::Point> shapePoints, const cv::Mat& img) {
    std::vector<cv::Point> imgCorners = {cv::Point(0, 0), cv::Point(img.cols, 0), cv::Point(img.cols, img.rows), cv::Point(0, img.rows)};
    for (auto & imgCorner : imgCorners) {
        if (shapePoints.empty()) break;

        auto minDistanceIter = std::min_element(shapePoints.begin(), shapePoints.end(),
                                                [&imgCorner](const cv::Point& p1, const cv::Point& p2) {
                                                    return cv::norm(p1 - imgCorner) < cv::norm(p2 - imgCorner);
                                                });

        imgCorner = *minDistanceIter;
        shapePoints.erase(minDistanceIter);
    }

    return imgCorners;
}

std::vector<cv::Point> cornerDetrctor::orderPoints(std::vector<cv::Point> cornerPoints) {
    std::vector<cv::Point> orderedPoints;
    orderedPoints.reserve(4);

    int diffMin = std::numeric_limits<int>::max();
    int diffMinIndex = 0;
    int diffMax = std::numeric_limits<int>::min();
    int diffMaxIndex = 0;
    int sumMin = std::numeric_limits<int>::max();
    int sumMinIndex = 0;
    int sumMax = std::numeric_limits<int>::min();
    int sumMaxIndex = 0;

    for (int i = 0; i < cornerPoints.size(); i++) {
        const cv::Point& corner = cornerPoints[i];
        int diff = corner.x - corner.y;
        int sum = corner.x + corner.y;

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

    orderedPoints.emplace_back(cornerPoints[sumMinIndex]);
    orderedPoints.emplace_back(cornerPoints[diffMaxIndex]);
    orderedPoints.emplace_back(cornerPoints[sumMaxIndex]);
    orderedPoints.emplace_back(cornerPoints[diffMinIndex]);

    return orderedPoints;
}