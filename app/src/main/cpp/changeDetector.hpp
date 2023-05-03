#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class changeDetector {
private:
    cv::Mat prevImgChanges;
    cv::Mat imgPersistentChanges;
public:
    changeDetector(cv::Size_<int> size) {
        prevImgChanges = cv::Mat::ones(size, CV_8UC1);
        imgPersistentChanges = cv::Mat::zeros(size, CV_8UC1);
    }

    cv::Mat detectChanges(cv::Mat& img1, cv::Mat& img2){
        cv::Mat currentImgChanges;
        cv::absdiff(img1, img2, currentImgChanges);

        cv::bitwise_and(prevImgChanges, currentImgChanges, imgPersistentChanges);

        prevImgChanges = currentImgChanges;

        return imgPersistentChanges;
    }
};