#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class changeDetector {
private:
    cv::Mat prevImgChanges;
public:
    changeDetector(cv::Size_<int> size) {
        prevImgChanges = cv::Mat::zeros(size, CV_8UC1);
    }

    cv::Mat detectChanges(cv::Mat& img1, cv::Mat& img2){
        cv::Mat currentImgChanges;
        cv::absdiff(img1, img2, currentImgChanges);

        cv::Mat imgPersistentChanges = getPersistentChanges(currentImgChanges, img1.size());

        prevImgChanges = currentImgChanges; //TODO should it be imgPersistentChanges instead

        return imgPersistentChanges;
    }

    cv::Mat getPersistentChanges(cv::Mat& currentImgChanges, cv::Size_<int> size){
        cv::Mat imgPersistentChanges = cv::Mat::zeros(size, CV_8UC1);

        if (!prevImgChanges.empty()){
            // Find changes that survived from previous round and set changes to white.
            // Make persistent changes white on black background.

            // for (int i = 0 i < prevImgChanges.rows() i++):
            //     for (int j = 0 j < prevImgChanges.cols() j++):
            //         if (prevImgChanges.get(i, j)[0] == 255 && currentImgChanges.get(i, j)[0] == 255):
            //             imgPersistentChanges.put(i, j, 255)

            cv::Mat bufferPrevChanges = prevImgChanges.clone();
            cv::Mat bufferCurrentChanges = currentImgChanges.clone();
            cv::Mat bufferPersistentChanges = imgPersistentChanges.clone();

            for (int i = 0; i < bufferPrevChanges.rows; i++){
                for (int j = 0; j < bufferPrevChanges.cols; j++){
                    if (bufferPrevChanges.at<uchar>(i, j) == 255 && bufferCurrentChanges.at<uchar>(i, j) == 255){
                        bufferPersistentChanges.at<uchar>(i, j) = 255;
                    }
                }
            }

            imgPersistentChanges = bufferPersistentChanges;
        }

        return imgPersistentChanges;
    }
};