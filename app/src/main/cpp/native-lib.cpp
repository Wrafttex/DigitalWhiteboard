#include <jni.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

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

auto convertToJavaPointList(JNIEnv* env, const std::vector<cv::Point>& points) -> jobject {
    jclass listClass = env->FindClass("java/util/ArrayList");
    jmethodID listInit = env->GetMethodID(listClass, "<init>", "()V");
    jmethodID listAdd = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");

    jclass pointClass = env->FindClass("android/graphics/Point");
    jmethodID pointInit = env->GetMethodID(pointClass, "<init>", "(II)V");

    jobject listObj = env->NewObject(listClass, listInit);
    for (cv::Point point : points) {
        jobject pointObj = env->NewObject(pointClass, pointInit, point.x, point.y);
        env->CallBooleanMethod(listObj, listAdd, pointObj);
    }

    return listObj;
}

extern "C" {
    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) -> jstring {
        std::string hello = "Hello from C++";
        return env->NewStringUTF(hello.c_str());
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_MainActivity_myFlip(JNIEnv* env, jobject, jobject bitmapIn, jobject bitmapOut) {
        Mat src;
        bitmapToMat(env, bitmapIn, src, false);
        myFlip(src);
        matToBitmap(env, src, bitmapOut, false);
    }   

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_MainActivity_myBlur(JNIEnv * env, jobject, jobject bitmapIn, jobject bitmapOut, jfloat sigma) {
        Mat src;
        bitmapToMat(env, bitmapIn, src, false);
        myBlur(src, sigma);
        matToBitmap(env, src, bitmapOut, false);
    }

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_captureActivity_createNativeCaptureActivity(JNIEnv * env, jobject thiz, jlong cornersPtr, jlong imgBgrPtr)->jlong {
        return reinterpret_cast<jlong>(new captureActivity(*reinterpret_cast<std::vector<cv::Point>*>(cornersPtr), *reinterpret_cast<cv::Mat*>(imgBgrPtr)));
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_destroyNativeCaptureActivity(JNIEnv * env, jobject thiz, jlong ptr) {
        delete reinterpret_cast<captureActivity*>(ptr);
    }

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_captureActivity_capture(JNIEnv * env, jobject thiz, jlong ptr, jlong imgBgrPtr)->jobject {
        cv::Mat imgBgr = *reinterpret_cast<cv::Mat*>(imgBgrPtr);
        cv::Mat imgCaptured = reinterpret_cast<captureActivity*>(ptr)->capture(imgBgr);
        if (!imgCaptured.empty()) {
            jobject bitmap = nullptr;
            matToBitmap(env, imgCaptured, bitmap, false);
            return bitmap;
        }
        return nullptr;
    }

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_createNativeObject(JNIEnv * env, jobject obj)->jlong {
        return reinterpret_cast<jlong>(new cornerDetrctor());
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_destroyNativeObject(JNIEnv * env, jobject obj, jlong ptr) {
        delete reinterpret_cast<cornerDetrctor*>(ptr);
    }

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_findCorners(JNIEnv * env, jobject obj, jlong ptr, jlong imgAddr)->jobject {
        cv::Mat& img = *reinterpret_cast<cv::Mat*>(imgAddr);
        std::vector<cv::Point> corners = reinterpret_cast<cornerDetrctor*>(ptr)->findCorners(std::move(img));
        return convertToJavaPointList(env, corners);
    }
}

JNIEXPORT auto JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) -> jint {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;

    jclass clazz = env->FindClass("com/example/digitalwhiteboard");
    if (clazz == nullptr) return JNI_ERR;

    static const JNINativeMethod methods[] = {
        {"createNativeObject", "()J", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_createNativeCaptureActivity)},
        {"destroyNativeObject", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_destroyNativeCaptureActivity)},
        {"capture", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_captureActivity_capture)},
        {"createNativeObject", "()J", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_createNativeObject)},
        {"destroyNativeObject", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_destroyNativeObject)},
        {"findCorners", "(J)V", reinterpret_cast<void*>(Java_com_example_digitalwhiteboard_cornerDetrctor_findCorners)},
    };

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod)) != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}