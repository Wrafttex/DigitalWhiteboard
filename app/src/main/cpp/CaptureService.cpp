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
    int rounds = 0;
    double totalSegmentatorTime = 0;
    double totalBinarizationTime = 0;
    double totalRemoveTime = 0;
    double totalChangeTime = 0;
    double totalUpdateTime = 0;
public:
    explicit CaptureService(const cv::Size_<int> size): size(size) {
        currentModel = cv::Mat::ones(size, CV_8UC1) * 255;
        ChangeDetector = changeDetector(size);
    }

    explicit CaptureService(const cv::Size_<int>& size, const std::string& path) {
        currentModel = cv::Mat::ones(size, CV_8UC1) * 255;
        ChangeDetector = changeDetector(size);
        Segmentator = segmentator{path};
    }

    cv::Mat capture(const cv::Mat& imgBgr) {
        // Segmentation
        __android_log_print(ANDROID_LOG_DEBUG, "capture", "Beginning segmentation");
        rounds++;
        auto t0 = clk::now();
        cv::Mat matPerspectiveRgb;
        cv::cvtColor(imgBgr, matPerspectiveRgb, cv::COLOR_RGBA2RGB);
        cv::resize(matPerspectiveRgb, matPerspectiveRgb, cv::Size(480, 270));
        cv::Mat imgSegMap = Segmentator.segmentate(matPerspectiveRgb);
        cv::resize(imgSegMap, imgSegMap, size);
        auto t1 = clk::now();
        totalSegmentatorTime += duration<double, std::milli>(t1-t0).count();
        __android_log_print(ANDROID_LOG_DEBUG, "segmentate", "%s ms", std::to_string(totalSegmentatorTime/rounds).c_str());

        // Binarize a gray scale version of the image.
        t0 = clk::now();
        cv::Mat imgWarpGray;
        cv::cvtColor(imgBgr, imgWarpGray, cv::COLOR_RGBA2GRAY);
        cv::Mat imgBinarized = Binarization.binarize(imgWarpGray);
        t1 = clk::now();
        totalBinarizationTime += duration<double, std::milli>(t1-t0).count();
        __android_log_print(ANDROID_LOG_DEBUG, "binarize", "%s ms", std::to_string(totalBinarizationTime/rounds).c_str());

        // Remove segments before change detection.
        t0 = clk::now();
        cv::Mat currentModelCopy = removeSegmentArea(imgBinarized, imgSegMap);
        t1 = clk::now();
        totalRemoveTime += duration<double, std::milli>(t1-t0).count();
        __android_log_print(ANDROID_LOG_DEBUG, "removeSegmentArea", "%s ms", std::to_string(totalRemoveTime/rounds).c_str());

        // Change detection
        t0 = clk::now();
        cv::Mat imgPersistentChanges = ChangeDetector.detectChanges(imgBinarized, currentModelCopy);
        t1 = clk::now();
        totalChangeTime += duration<double, std::milli>(t1-t0).count();
        __android_log_print(ANDROID_LOG_DEBUG, "detectChanges", "%s ms", std::to_string(totalChangeTime/rounds).c_str());

        // Update current model with persistent changes.
        t0 = clk::now();
        updateModel(imgBinarized, imgPersistentChanges);
        t1 = clk::now();
        totalUpdateTime += duration<double, std::milli>(t1-t0).count();
        __android_log_print(ANDROID_LOG_DEBUG, "updateModel", "%s ms", std::to_string(totalUpdateTime/rounds).c_str());
        return currentModel;
    }

    // Removes segment area from image
    cv::Mat removeSegmentArea(cv::Mat& imgBinarized, cv::Mat& imgSegMap) { //TODO something seems wrong
        cv::Mat currentModelCopy = currentModel.clone();
        imgBinarized &= ~imgSegMap;
        currentModelCopy &= imgSegMap;
        return currentModelCopy;
    }

    void updateModel(cv::Mat imgBinarized, const cv::Mat& imgPersistentChanges) {
        currentModel = currentModel & imgPersistentChanges;
        imgBinarized = imgBinarized & ~imgPersistentChanges;
        currentModel = currentModel | imgBinarized;
    }
};