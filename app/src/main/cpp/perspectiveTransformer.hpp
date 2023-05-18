#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

class perspectiveTransformer {
private:
    int CROP_MARGIN = 10;

    int MIN_DIMENSION = 100;
    int MAX_DIMENSION = 5000;
public:

    bool hasExtremeDimensions(float& maxWidth, float& maxHeight){
        return (maxWidth < MIN_DIMENSION || maxHeight < MIN_DIMENSION || maxWidth > MAX_DIMENSION || maxHeight > MAX_DIMENSION);
    }
    
    cv::Mat getPerspective(cv::Mat&& imgRgb, std::vector<cv::Point>& cornerPoints){
        // Approx target corners.
        cv::Point tl = cornerPoints[0];
        cv::Point tr = cornerPoints[1];
        cv::Point br = cornerPoints[2];
        cv::Point bl = cornerPoints[3];

        int widthBtm = cv::norm(bl - br);
        int widthTop = cv::norm(tl - tr);
        float maxWidth = fmax(widthBtm, widthTop);

        int heightRight = cv::norm(br - tr);
        int heightLeft = cv::norm(bl - tl);
        float maxHeight = fmax(heightRight, heightLeft);

        // Ensure no extreme dimensions.
//        if (hasExtremeDimensions(maxWidth, maxHeight)){
//            return {};
//        }

        // Create rectangle from approximated corners.
        cv::Mat targetCorners = (cv::Mat_<float>(4, 2) << 0, 0, maxWidth - 1, 0, maxWidth - 1, maxHeight - 1, 0, maxHeight - 1);
        // std::vector<cv::Point2f> targetCorners = {cv::Point2f{0,0}, cv::Point2f{maxWidth - 1,0}, cv::Point2f{maxWidth - 1,maxHeight - 1}, cv::Point2f{0,maxHeight - 1}};
        cv::Mat cornerPointsMat = (cv::Mat_<float>(4, 2) << tl.x, tl.y, tr.x, tr.y, br.x, br.y, bl.x, bl.y);

        // Get matrix for image perspective perspective
        // Note: Target corners represents the "real"/actual dimensions of the input points i.e. actual
        // ratio of paper.
        cv::Mat perspectiveMatrix = cv::getPerspectiveTransform(cornerPointsMat, targetCorners);

        // Perform perspective transformation.
        // Note: The given width and height are for cropping only i.e. does not affect how image is transformed.
        cv::Mat imgPerspective;
        cv::warpPerspective(imgRgb, imgPerspective, perspectiveMatrix, cv::Size(maxWidth, maxHeight));

        cv::Mat croppedMat = cropAndResize(imgPerspective, CROP_MARGIN);

        return croppedMat;
    }

    cv::Mat cropAndResize(cv::Mat& imgPerspective, const int& cropMargin){ //TODO check if this is correct
        cv::Mat croppedMat = imgPerspective(cv::Rect(cropMargin, cropMargin, imgPerspective.cols - cropMargin, imgPerspective.rows - cropMargin));
        cv::resize(croppedMat, croppedMat, imgPerspective.size());
        return croppedMat;
    }
};
