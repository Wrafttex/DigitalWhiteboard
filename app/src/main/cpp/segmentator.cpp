#include "segmentator.hpp"
#include <chrono>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

segmentator::segmentator(){
    module = torch::jit::load("/mnt/d/temp ting/sw8 projekt/libtorch/assets/vit_base_patch8_224.ptl");
    module.eval();
}

cv::Mat segmentator::segmentate(cv::Mat& input) {
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    auto tensor_image = CVMatToTensor(input);
    inputs.push_back(tensor_image);

    // Execute the model and turn its output into a tensor.
    auto t0 = clk::now();
    at::Tensor output = std::get<1>(module.forward(inputs).toTensor().max(1));
    auto t1 = clk::now();
    std::cout << "forward took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
    output = (output == 12) * 255;


    // Convert tensor [270, 480] to cv::Mat
    auto outputMat = TensorToCVMat(output);

    // dilate the output to make it more visible
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
    cv::dilate(outputMat, outputMat, kernel);

    return outputMat.clone();
}

torch::Tensor segmentator::CVMatToTensor(cv::Mat mat) { // TODO: maby normalize?
    cv::Mat matFloat;
    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
    mat.convertTo(matFloat, CV_32F);
    auto size = matFloat.size();
    auto nChannels = matFloat.channels();
    auto sizes = at::IntArrayRef({1, size.height, size.width, nChannels});
    auto tensor = torch::from_blob(matFloat.data, sizes);
    return tensor.permute({0, 3, 1, 2}) / 255;
}

cv::Mat segmentator::TensorToCVMat(torch::Tensor tensor) {
    tensor = tensor.detach().permute({1, 2, 0});
    tensor = tensor.to(torch::kByte);
    int64_t height = tensor.size(0);
    int64_t width = tensor.size(1);
    cv::Mat mat(height, width, CV_8UC1);
    std::memcpy((void *)mat.data, tensor.data_ptr<uchar>(), sizeof(uchar) * tensor.numel());
    return mat.clone();
}