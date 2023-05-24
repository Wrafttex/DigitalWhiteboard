#include <jni.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

#include "android/bitmap.h"
#include "opencvUtils.h"
#include "cornerDetrctor.hpp"
#include "captureActivity.hpp"

void bitmapToMat(JNIEnv* env, jobject bitmap, Mat& dst, jboolean needUnPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void* pixels = 0;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (needUnPremultiplyAlpha)
                cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            else
                tmp.copyTo(dst);
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception& e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}

void matToBitmap(JNIEnv* env, Mat src, jobject bitmap, jboolean needPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void* pixels = 0;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        std::ostringstream oss;
        oss << "height: " << info.height << " rows: " << (uint32_t)src.rows << " width: " << info.width << "cols: " << (uint32_t)src.cols << " dims: " << src.dims;
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib", "%s ", oss.str().c_str());
        CV_Assert(src.dims == 2 && info.height == (uint32_t)src.rows && info.width == (uint32_t)src.cols);
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (src.type() == CV_8UC1) {
                cvtColor(src, tmp, COLOR_GRAY2RGBA);
            } else if (src.type() == CV_8UC3) {
                cvtColor(src, tmp, COLOR_RGB2RGBA);
            } else if (src.type() == CV_8UC4) {
                if (needPremultiplyAlpha)
                    cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                else
                    src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if (src.type() == CV_8UC1) {
                cvtColor(src, tmp, COLOR_GRAY2BGR565);
            } else if (src.type() == CV_8UC3) {
                cvtColor(src, tmp, COLOR_RGB2BGR565);
            } else if (src.type() == CV_8UC4) {
                cvtColor(src, tmp, COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception& e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return;
    }
}

void vectorToJFloatArray(JNIEnv* env, std::vector<cv::Point>& points, jfloatArray fArrOut) {
    jfloat* data =  env->GetFloatArrayElements(fArrOut, nullptr);
    for (int i = 0; i < points.size(); i++) {
        data[i * 2] = points[i].x;
        data[i * 2 + 1] = points[i].y;
    }
    env->ReleaseFloatArrayElements(fArrOut, data, 0);
}

std::vector<cv::Point> JFloatArrayToVector(JNIEnv* env, jfloatArray fArrIn) {
    std::vector<cv::Point> points;
    jfloat* data = env->GetFloatArrayElements(fArrIn, nullptr);
    for (int i = 0; i < env->GetArrayLength(fArrIn); i += 2) {
        points.emplace_back(data[i], data[i + 1]);
    }
    env->ReleaseFloatArrayElements(fArrIn, data, 0);
    return points;
}

cv::Size getPerspectiveSize(std::vector<cv::Point>& cornerPoints) { //TODO: sum ting wong
    // Approx target corners.
    cv::Point tl = cornerPoints[0];
    cv::Point tr = cornerPoints[1];
    cv::Point br = cornerPoints[2];
    cv::Point bl = cornerPoints[3];

    auto widthBtm = cv::norm(bl - br);
    auto widthTop = cv::norm(tl - tr);
    auto maxWidth = fmax(widthBtm, widthTop);

    auto heightRight = cv::norm(br - tr);
    auto heightLeft = cv::norm(bl - tl);
    auto maxHeight = fmax(heightRight, heightLeft);
    return {static_cast<int>(maxWidth), static_cast<int>(maxHeight)};
}

extern "C" {
    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) -> jstring {
        std::string hello = "Hello from C++";
        return env->NewStringUTF(hello.c_str());
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_CornerActivity_myFlip(JNIEnv* env, jobject, jobject bitmapIn, jobject bitmapOut) {
        Mat src;
        bitmapToMat(env, bitmapIn, src, false);
        myFlip(src);
        matToBitmap(env, src, bitmapOut, false);
    }   

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_DrawActivity_myBlur(JNIEnv * env, jobject, jobject bitmapIn, jobject bitmapOut, jfloat sigma) {
        Mat src;
        bitmapToMat(env, bitmapIn, src, false);
        myBlur(src, sigma);
        matToBitmap(env, src, bitmapOut, false);
    }

    JNIEXPORT jlong JNICALL Java_com_example_digitalwhiteboard_captureActivity_createNativeCaptureActivity(JNIEnv * env, jobject obj, jfloatArray corners, jobject bitmapIn){
        Mat dst;
        bitmapToMat(env, bitmapIn, dst, false);

        std::vector<cv::Point> points = JFloatArrayToVector(env, corners);

        return reinterpret_cast<jlong>(new captureActivity(std::move(points), getPerspectiveSize(points)));
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_getSize(JNIEnv * env, jobject obj, jfloatArray corners, jintArray iArrOut){

        std::vector<cv::Point_<int>> points = JFloatArrayToVector(env, corners);
        auto size = getPerspectiveSize((points));
        jint* data =  env->GetIntArrayElements(iArrOut, nullptr);
        data[0] = size.width;
        data[1] = size.height;
        env->ReleaseIntArrayElements(iArrOut, data, 0);
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_destroyNativeCaptureActivity(JNIEnv * env, jobject obj, jlong ptr) {
        delete reinterpret_cast<captureActivity*>(ptr);
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_capture(JNIEnv * env, jobject obj, jlong ptr, jobject bitmapIn, jobject bitmapOut){
        Mat dst;
        bitmapToMat(env, bitmapIn, dst, true);
//        auto tensor_image = CVMatToTensor(dst.clone());
        cv::Mat imgCaptured = reinterpret_cast<captureActivity*>(ptr)->capture(std::move(dst));
        std::ostringstream oss;
        oss << "nChannels: " << imgCaptured.channels() << " size: " << imgCaptured.size() << " dims: " << imgCaptured.dims;
        __android_log_print(ANDROID_LOG_DEBUG, "native-lib", "%s ", oss.str().c_str());
        matToBitmap(env, imgCaptured, bitmapOut, false);
    }

    JNIEXPORT jlong JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_createNativeObject(JNIEnv * env, jobject obj) {
        return reinterpret_cast<jlong>(new cornerDetrctor());
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_destroyNativeObject(JNIEnv * env, jobject obj, jlong ptr) {
        delete reinterpret_cast<cornerDetrctor*>(ptr);
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_findCorners(JNIEnv * env, jobject obj, jlong ptr, jobject bitmapIn, jfloatArray fArrOut) {
        Mat dst;
        bitmapToMat(env, bitmapIn, dst, false);
        std::vector<cv::Point> corners = reinterpret_cast<cornerDetrctor*>(ptr)->findCorners(std::move(dst));
        vectorToJFloatArray(env, corners, fArrOut);
    }
}

/*
JNIEXPORT auto JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) -> jint {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;

    jclass captureClass = env->FindClass("com/example/digitalwhiteboard/captureActivity");
    jclass cornerClass = env->FindClass("com/example/digitalwhiteboard/cornerDetrctor");

    if (captureClass == nullptr || cornerClass == nullptr) return JNI_ERR;

    static const JNINativeMethod cornerMethods[] = {
        {"createNativeObject", "()J", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_createNativeObject)},
        {"destroyNativeObject", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_destroyNativeObject)},
        {"findCorners", "(J)[F", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_findCorners)}
    };

    static const JNINativeMethod captureMethods[] = {
            {"createNativeObject", "()J", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_createNativeCaptureActivity)},
            {"destroyNativeObject", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_destroyNativeCaptureActivity)},
            {"capture", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_capture)}
    };

    if (env->RegisterNatives(captureClass, captureMethods, sizeof(captureMethods) / sizeof(JNINativeMethod)) != JNI_OK) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(cornerClass, cornerMethods, sizeof(cornerMethods) / sizeof(JNINativeMethod)) != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
*/
/*
https://stackoverflow.com/questions/51613950/kotlin-ndk-and-c-interactions
https://medium.com/tompee/android-ndk-jni-primer-and-cheat-sheet-18dd006ec07f
https://developer.android.com/training/articles/perf-jni#kotlin
https://kotlinlang.org/docs/object-declarations.html#data-objects
https://developer.android.com/ndk/samples/sample_hellojni.html
*/