#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

class binarization {
private:
    const int MAX_THRESH_VALUE = 255;
    const int BLOCK_SIZE = 21;
    const int C = 4;
    const int BLUR_KERNEL_SIZE = 3;

public:
    // Binarizes a gray scale image.
    cv::Mat binarize(cv::Mat& imgGray) {
        cv::Mat imgBinarizedThreshold;
        cv::adaptiveThreshold(imgGray, imgBinarizedThreshold, MAX_THRESH_VALUE, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, BLOCK_SIZE, C);

        // Clean image - removes most of the noise
        cv::Mat imgBinarizedThresholdBlur;
        cv::medianBlur(imgBinarizedThreshold, imgBinarizedThresholdBlur, BLUR_KERNEL_SIZE);

        // Invert image (black on white background)
        cv::Mat imgBinarizedFinal;
        cv::bitwise_not(imgBinarizedThresholdBlur, imgBinarizedFinal);

        return imgBinarizedFinal;
    }
};