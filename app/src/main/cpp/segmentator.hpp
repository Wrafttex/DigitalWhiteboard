#pragma once

#include <torch/script.h> // https://github.com/pytorch/pytorch/issues/30158
#include <opencv2/opencv.hpp>
#include <memory>
#include <jni.h>

class segmentator {
public:
    segmentator();
    segmentator(const std::string& path);
    cv::Mat segmentate(cv::Mat& input);
private:
    torch::jit::script::Module module;
    static cv::Mat TensorToCVMat(torch::Tensor& tensor);
    static torch::Tensor CVMatToTensor(cv::Mat& mat);
    void postProcessing(torch::Tensor& result);
};
