#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
//#include <torch/script.h>

class segmentator {
public:
    cv::Mat segmentate(const cv::Mat& image){return image;}
};

// class segmentator {
// private:
//     const std::string TAG = "segmentator";
//     const int NUM_THREADS = 4;
//     const std::string SEGMENTATION_MODEL_NAME = "deeplabv3_257_mv_gpu.tflite";
//     const int ALPHA_VALUE = 128;
//     cv::Mat maskTensor;
//     std::vector<int> pixelsGlobal;

//     torch::Device DEVICE = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;
//     torchvision::transforms::Normalize<> TRANSFORM = torchvision::models::segmentation::deeplabv3_mobilenet_v3_large::Weights::COCO_WITH_VOC_LABELS_V1.transforms();

// public:
//     segmentator() {
//         try {
//             imageSegmenter = torchvision::models::segmentation::deeplabv3_mobilenet_v3_large(torchvision::models::segmentation::deeplabv3_mobilenet_v3_large::Options().weights("DEFAULT")).to(DEVICE).eval();
//         } catch (const std::exception& e) {
//             throw std::runtime_error("Exception occurred while loading model from file " + std::string(e.what()));
//         }
//     }

//     cv::Mat segmentate(const cv::Mat& image) {
//         auto startTime = (double)cv::getTickCount();

//         // Do segmentation
//         auto imageSegmentationTime = (double)cv::getTickCount();
//         cv::Mat image_tensor = image.clone();
//         cv::cvtColor(image_tensor, image_tensor, cv::COLOR_BGR2RGB);
//         image_tensor = image_tensor.permute(2, 0, 1).unsqueeze(0);
//         torch::Tensor pre = TRANSFORM(image_tensor);
//         torch::Tensor results = imageSegmenter(pre)["out"];
//         imageSegmentationTime = (double)cv::getTickCount() - imageSegmentationTime;
//         std::cout << TAG << " Time to run the segmentation model: " << (imageSegmentationTime / cv::getTickFrequency()) << " ms" << std::endl;

//         torch::Tensor output_predictions = results[0].argmax(0);
//         int h = image.rows;
//         int w = image.cols;
//         cv::Mat mask = (output_predictions != 0).byte().cpu().numpy().astype("uint8") * 255;
//         cv::resize(mask, mask, cv::Size(w, h), cv::INTER_NEAREST);

//         startTime = (double)cv::getTickCount() - startTime;
//         std::cout << TAG << " Total time in segmentation step: " << (startTime / cv::getTickFrequency()) << " ms" << std::endl;
//         return mask;
//     }
// };

