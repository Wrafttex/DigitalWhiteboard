#include "segmentator.hpp"
#include <chrono>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

segmentator::segmentator(){
    module = torch::jit::load("/data/user/0/com.example.digitalwhiteboard/files/noQ_ADE20K_ViT_Seg_T_Mask_fbgemm_CPU.pt");
    module.eval();
}

segmentator::segmentator(const std::string& path){
   module = torch::jit::load(path);
   module.eval();
}

cv::Mat segmentator::segmentate(cv::Mat& input) {
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    auto tensor_image = CVMatToTensor(input);
//    tensor_image = tensor_image.vulkan();
    inputs.emplace_back(std::move(tensor_image));

    // Execute the model and turn its output into a tensor.
    at::Tensor output = module.forward(inputs).toTensor();
    postProcessing(output);

    // Convert tensor to cv::Mat
    auto outputMat = TensorToCVMat(output);

    // Dilate the output to make it more visible
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10));
    cv::dilate(outputMat, outputMat, kernel);

    return outputMat.clone();
}

void segmentator::postProcessing(torch::Tensor& result){
    result = std::get<1>(result.max(1));
    result = (result == 12) * 255;
}

torch::Tensor segmentator::CVMatToTensor(cv::Mat& matFloat) { // TODO: maby normalize?
    matFloat.convertTo(matFloat, CV_32F);
    auto size = matFloat.size();
    auto nChannels = matFloat.channels();
    auto tensor = torch::from_blob(matFloat.data, {1, size.height, size.width, nChannels});
    return (tensor.permute({0, 3, 1, 2}) / 255).to(c10::MemoryFormat::ChannelsLast);
}

cv::Mat segmentator::TensorToCVMat(torch::Tensor& tensor) {
    tensor = tensor.detach().permute({1, 2, 0});
    tensor = tensor.to(torch::kByte);
    int64_t height = tensor.size(0);
    int64_t width = tensor.size(1);
    cv::Mat mat(height, width, CV_8UC1);
    std::memcpy((void *)mat.data, tensor.data_ptr<uchar>(), sizeof(uchar) * tensor.numel());
    return mat.clone();
}