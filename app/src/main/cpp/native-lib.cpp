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

auto convertToJavaFloatArray(JNIEnv* env, const std::vector<cv::Point>& points) -> jobject {
    jclass floatArrayClass = env->FindClass("[F");
    jobjectArray floatArray = env->NewObjectArray(points.size(), floatArrayClass, nullptr);
    for (int i = 0; i < points.size(); i++) {
        jfloatArray point = env->NewFloatArray(2);
        jfloat *pointPtr = env->GetFloatArrayElements(point, nullptr);
        pointPtr[0] = points[i].x;
        pointPtr[1] = points[i].y;
        env->ReleaseFloatArrayElements(point, pointPtr, 0);
        env->SetObjectArrayElement(floatArray, i, point);
    }
    return floatArray;
}

jfloatArray convertToJavaArray(JNIEnv* env, const std::vector<cv::Point>& points) {
    jfloatArray array = env->NewFloatArray(points.size() * 2);
    jfloat* data = env->GetFloatArrayElements(array, nullptr);
    for (int i = 0; i < points.size(); i++) {
        data[i * 2] = points[i].x;
        data[i * 2 + 1] = points[i].y;
    }
    env->ReleaseFloatArrayElements(array, data, 0);
    return array;
}

void vectorToJFloatArray(JNIEnv* env, std::vector<cv::Point>& points, jfloatArray fArrOut) {
    jfloat* data =  env->GetFloatArrayElements(fArrOut, nullptr);
    for (int i = 0; i < points.size(); i++) {
        data[i * 2] = points[i].x;
        data[i * 2 + 1] = points[i].y;
    }
    env->ReleaseFloatArrayElements(fArrOut, data, 0);
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

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_captureActivity_createNativeCaptureActivity(JNIEnv * env, jobject obj, jlong cornersPtr, jlong imgBgrPtr)->jlong {
        return reinterpret_cast<jlong>(new captureActivity(*reinterpret_cast<std::vector<cv::Point>*>(cornersPtr), *reinterpret_cast<cv::Mat*>(imgBgrPtr)));
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_destroyNativeCaptureActivity(JNIEnv * env, jobject obj, jlong ptr) {
        delete reinterpret_cast<captureActivity*>(ptr);
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_captureActivity_capture(JNIEnv * env, jobject obj, jlong ptr, jobject bitmapIn, jobject bitmapOut){
        Mat dst;
        bitmapToMat(env, bitmapIn, dst, false);
        cv::Mat imgCaptured = reinterpret_cast<captureActivity*>(ptr)->capture(dst);
        matToBitmap(env, imgCaptured, bitmapOut, false);
    }

    JNIEXPORT auto JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_createNativeObject(JNIEnv * env, jobject obj)->jlong {
        return reinterpret_cast<jlong>(new cornerDetrctor());
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_destroyNativeObject(JNIEnv * env, jobject obj, jlong ptr) {
        delete reinterpret_cast<cornerDetrctor*>(ptr);
    }

    JNIEXPORT void JNICALL Java_com_example_digitalwhiteboard_cornerDetrctor_findCorners(JNIEnv * env, jobject obj, jobject bitmapIn, jfloatArray fArrOut) {
        //cv::Mat& img = *reinterpret_cast<cv::Mat*>(imgAddr);
        /*
        AndroidBitmapInfo info;
        void* srcPixels = 0;
        if (AndroidBitmap_getInfo(env, bitmapIn, &info) < 0) throw std::runtime_error("No info found");

        if (AndroidBitmap_lockPixels(env, bitmapIn, &srcPixels) < 0) throw std::runtime_error("No pixel found");


        int x = info.width;
        int y = info.height;
        */

        Mat dst;
        bitmapToMat(env, bitmapIn, dst, false);
        //std::vector<cv::Point> corners = reinterpret_cast<cornerDetrctor*>(ptr)->findCorners(std::move(dst));
        std::vector<cv::Point> corners = {cv::Point{0,0}, cv::Point{0,200}, cv::Point{200,200}, cv::Point{200,0}};
        vectorToJFloatArray(env, corners, fArrOut);
        //return convertToJavaArray(env, corners);
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