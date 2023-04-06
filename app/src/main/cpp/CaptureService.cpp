#include "binarization.hpp"
#include "changeDetector.hpp"
#include "segmentator.hpp"
#include "perspectiveTransformer.hpp"

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
        cv::Mat matPerspectiveRgb;
        cv::cvtColor(imgBgr, matPerspectiveRgb, cv::COLOR_BGR2RGB);
        cv::Mat imgSegMap = Segmentator.segmentate(matPerspectiveRgb);
        std::cout << TAG << " capture: Segmentation done." << std::endl;

        // Binarize a gray scale version of the image.
        cv::Mat imgWarpGray;
        cv::cvtColor(imgBgr, imgWarpGray, cv::COLOR_BGR2GRAY);
        cv::Mat imgBinarized = Binarization.binarize(imgWarpGray);

        // Remove segments before change detection.
        cv::Mat currentModelCopy = removeSegmentArea(imgSegMap);

        // Change detection
        cv::Mat imgPersistentChanges = ChangeDetector.detectChanges(imgBinarized, currentModelCopy);

        // Update current model with persistent changes.
        updateModel(imgBinarized, imgPersistentChanges);
        return currentModel;
    }

    // Removes segment area from image
    cv::Mat removeSegmentArea(const cv::Mat& imgSegMap) { //TODO something seems wrong
        cv::Mat currentModelCopy = currentModel.clone();

        auto startTime = (double)cv::getTickCount();

        currentModelCopy = currentModelCopy & imgSegMap;

        auto endTime = (double)cv::getTickCount();
        std::cout << "Remove segment loop took: " << (endTime - startTime) / cv::getTickFrequency() << " milliseconds" << std::endl;
        return currentModelCopy;
    }

    void updateModel(cv::Mat imgBinarized, const cv::Mat& imgPersistentChanges) {
        auto startTime = (double)cv::getTickCount();

        currentModel = currentModel & ~imgPersistentChanges;
        imgBinarized = imgBinarized & imgPersistentChanges;
        currentModel = currentModel | imgBinarized;

        auto endTime = (double)cv::getTickCount();
        std::cout << "Update model loop took: " << (endTime - startTime) / cv::getTickFrequency() << " milliseconds" << std::endl;
    }
};