#pragma once

#include <torch/script.h> // https://github.com/pytorch/pytorch/issues/30158
#include <opencv2/opencv.hpp>
#include <memory>

class segmentator {
public:
    segmentator();
    void loadModel(std::string path);
    cv::Mat segmentate(cv::Mat& input);
private:
    torch::jit::script::Module module;
    cv::Mat TensorToCVMat(torch::Tensor tensor);
    cv::Mat TensorToCVMat8UC3(torch::Tensor tensor);
    torch::Tensor CVMatToTensor(cv::Mat mat);
    
};
