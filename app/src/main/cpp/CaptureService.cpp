#include "binarization.hpp"
#include "changeDetector.hpp"
#include "segmentator.hpp"
#include "perspectiveTransformer.hpp"
#include <chrono>
#include <android/log.h>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

class CaptureService {
private:
    const char* TAG = "CaptureService";
    cv::Mat currentModel;
    cv::Size size;
    changeDetector ChangeDetector = changeDetector(cv::Size_<int>());
    binarization Binarization{};
    segmentator Segmentator{};
public:
    CaptureService(cv::Size_<int> size): size(size) {
        currentModel = cv::Mat::ones(size, CV_8UC1) * 255;
        ChangeDetector = changeDetector(size);
    }

    explicit CaptureService(cv::Size_<int>& size, const std::string& path) {
        currentModel = cv::Mat::ones(size, CV_8UC1) * 255;
        ChangeDetector = changeDetector(size);
        Segmentator = segmentator{path};
    }

    cv::Mat capture(const cv::Mat& imgBgr) {
        // Segmentation
        // std::cout << TAG << " capture: Segmentation started." << std::endl;
        __android_log_print(ANDROID_LOG_DEBUG, "capture", "Beginning segmentation");
        auto t0 = clk::now();
        cv::Mat matPerspectiveRgb;
        cv::cvtColor(imgBgr, matPerspectiveRgb, cv::COLOR_RGBA2RGB);
        cv::Mat imgSegMap = Segmentator.segmentate(std::move(matPerspectiveRgb));
//        cv::Mat imgSegMap;
//        cv::cvtColor(matPerspectiveRgb, imgSegMap, cv::COLOR_BGR2GRAY);
        auto t1 = clk::now();
        // std::cout << "segmentate took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
        __android_log_print(ANDROID_LOG_DEBUG, "segmentate", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());

        // Binarize a gray scale version of the image.
        __android_log_print(ANDROID_LOG_DEBUG, "capture", "Beginning Binarization");
        cv::Mat imgWarpGray;
        cv::cvtColor(imgBgr, imgWarpGray, cv::COLOR_RGBA2GRAY);
        cv::Mat imgBinarized = Binarization.binarize(imgWarpGray);

        // Remove segments before change detection.
        cv::Mat currentModelCopy = removeSegmentArea(imgBinarized, imgSegMap);

        // Change detection
        cv::Mat imgPersistentChanges = ChangeDetector.detectChanges(imgBinarized, currentModelCopy);

        // Update current model with persistent changes.
        updateModel(imgBinarized, imgPersistentChanges);
        return currentModel;
    }

    // Removes segment area from image
    cv::Mat removeSegmentArea(cv::Mat& imgBinarized, cv::Mat& imgSegMap) { //TODO something seems wrong
        __android_log_print(ANDROID_LOG_DEBUG, "capture", "Beginning removeSegmentArea");
        cv::Mat currentModelCopy = currentModel.clone();
        std::ostringstream oss;
        oss << "imgBinarized size: " << imgBinarized.size() << " currentModelCopy size: " << currentModelCopy.size() << " imgSegMap size: " << imgSegMap.size();
        __android_log_print(ANDROID_LOG_DEBUG, "CVMatToTensor", "%s ", oss.str().c_str());

        auto t0 = clk::now();

        imgBinarized &= ~imgSegMap;
        currentModelCopy &= imgSegMap;

        auto t1 = clk::now();
        // std::cout << "removeSegmentArea took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
        __android_log_print(ANDROID_LOG_DEBUG, "removeSegmentArea", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());
        return currentModelCopy;
    }

    void updateModel(cv::Mat imgBinarized, const cv::Mat& imgPersistentChanges) {
        auto t0 = clk::now();

        currentModel = currentModel & imgPersistentChanges;
        imgBinarized = imgBinarized & ~imgPersistentChanges;
        currentModel = currentModel | imgBinarized;

        auto t1 = clk::now();
        // std::cout << "updateModel took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
        __android_log_print(ANDROID_LOG_DEBUG, "updateModel", "%s ms", std::to_string(duration<double, std::milli>(t1-t0).count()).c_str());
    }
};