#include "binarization.hpp"
#include "changeDetector.hpp"
#include "segmentator.hpp"
#include "perspectiveTransformer.hpp"
#include <chrono>

using std::chrono::duration;
using clk = std::chrono::high_resolution_clock;

class CaptureService {
private:
    const char* TAG = "CaptureService";
    cv::Mat currentModel;
    changeDetector ChangeDetector = changeDetector(cv::Size_<int>());
    binarization Binarization{};
    segmentator Segmentator{};
public:
    explicit CaptureService(cv::Size_<int> size) {
        currentModel = cv::Mat::ones(size, CV_8UC1) * 255;
        ChangeDetector = changeDetector(size);
    }

    cv::Mat capture(const cv::Mat& imgBgr) {
        // Segmentation
        std::cout << TAG << " capture: Segmentation started." << std::endl;
        auto t0 = clk::now();
        cv::Mat matPerspectiveRgb;
        cv::cvtColor(imgBgr, matPerspectiveRgb, cv::COLOR_BGR2RGB);
        cv::Mat imgSegMap = Segmentator.segmentate(matPerspectiveRgb);
        auto t1 = clk::now();
        std::cout << "segmentate took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;

        // Binarize a gray scale version of the image.
        cv::Mat imgWarpGray;
        cv::cvtColor(imgBgr, imgWarpGray, cv::COLOR_BGR2GRAY);
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
    cv::Mat removeSegmentArea(cv::Mat& imgBinarized, const cv::Mat& imgSegMap) { //TODO something seems wrong
        cv::Mat currentModelCopy = currentModel.clone();

        auto t0 = clk::now();

        imgBinarized &= ~imgSegMap;
        currentModelCopy &= imgSegMap;

        auto t1 = clk::now();
        std::cout << "removeSegmentArea took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
        return currentModelCopy;
    }

    void updateModel(cv::Mat imgBinarized, const cv::Mat& imgPersistentChanges) {
        auto t0 = clk::now();

        currentModel = currentModel & imgPersistentChanges;
        imgBinarized = imgBinarized & ~imgPersistentChanges;
        currentModel = currentModel | imgBinarized;

        auto t1 = clk::now();
        std::cout << "updateModel took: " << duration<double, std::milli>(t1-t0).count() << " ms" << std::endl;
    }
};