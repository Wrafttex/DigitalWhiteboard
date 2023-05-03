#include "opencvUtils.h"
#include <opencv2/imgproc.hpp>
//#include <opencv2/opencv.hpp>

void myFlip(Mat src){
    flip(src, src, 0);
}
void myBlur(Mat src, float sigma){
    GaussianBlur(src, src, Size(), sigma);
}