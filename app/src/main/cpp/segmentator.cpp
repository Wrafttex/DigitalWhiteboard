#include "segmentator.hpp"
#include <chrono>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

segmentator::segmentator(){
    module = torch::jit::load("/data/user/0/com.example.digitalwhiteboard/files/CPU_model_best.pt");
    module.eval();
}

segmentator::segmentator(const std::string& path){
    module = torch::jit::load(path);
    module.eval();
}

cv::Mat segmentator::segmentate(cv::Mat&& input) {
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    auto tensor_image = CVMatToTensor(std::move(input));
    inputs.emplace_back(std::move(tensor_image));

    // Execute the model and turn its output into a tensor.
    auto t0 = clk::now();
    at::Tensor output = std::get<1>(module.forward(inputs).toTensor().max(1));
    auto t1 = clk::now();
    // std::cout << "forward took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
    __android_log_print(ANDROID_LOG_DEBUG, "forward", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());
    
    output = (output == 12) * 255;

    // Convert tensor to cv::Mat
    auto outputMat = TensorToCVMat(output);

    // dilate the output to make it more visible
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
    cv::dilate(outputMat, outputMat, kernel);

    return outputMat.clone();
}

torch::Tensor segmentator::CVMatToTensor(cv::Mat&& matFloat) { // TODO: maby normalize?
//    cv::Mat matFloat;
//    cv::cvtColor(mat, mat, cv::COLOR_RGBA2RGB);
    matFloat.convertTo(matFloat, CV_32F);
    auto size = matFloat.size();
    auto nChannels = matFloat.channels();
//    auto sizes = at::IntArrayRef({1, size.height, size.width, nChannels});
    auto tensor = torch::from_blob(matFloat.data, {1, size.height, size.width, nChannels}); //TODO: this is the problem now
    std::ostringstream oss;
    oss << "nChannels: " << nChannels << " size: " << size;
    __android_log_print(ANDROID_LOG_DEBUG, "CVMatToTensor", "%s ", oss.str().c_str());
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