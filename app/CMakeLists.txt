cmake_minimum_required(VERSION 3.22.1)
set(TARGET digitalwhiteboard)
project(${TARGET} CXX)
set(CMAKE_CXX_STANDARD 17)

set(build_DIR ${CMAKE_SOURCE_DIR}/build)

#set(ANDROID_NDK C:/Users/Kure/AppData/Local/Android/Sdk/ndk/25.2.9519653)
#set(CMAKE_TOOLCHAIN_FILE ${ANDROID_NDK}/build/cmake/android.toolchain.cmake)


set(pytorch_testapp_cpp_DIR ${CMAKE_CURRENT_LIST_DIR}/src/main/cpp)
file(GLOB SOURCES
        ${pytorch_testapp_cpp_DIR}/cppNative.cpp
        )

add_library(${TARGET} SHARED

        ${pytorch_testapp_cpp_DIR}/cppNative.cpp
        ${pytorch_testapp_cpp_DIR}/native-lib.cpp
        ${pytorch_testapp_cpp_DIR}/captureActivity.cpp
        ${pytorch_testapp_cpp_DIR}/cornerDetrctor.cpp
        ${pytorch_testapp_cpp_DIR}/opencvUtils.cpp
        ${pytorch_testapp_cpp_DIR}/CaptureService.cpp
        )


file(GLOB PYTORCH_INCLUDE_DIRS "${build_DIR}/pytorch_android*.aar/headers")
file(GLOB PYTORCH_LINK_DIRS "${build_DIR}/pytorch_android*.aar/jni/${ANDROID_ABI}")

target_compile_options(${TARGET} PRIVATE
        -fexceptions
        )


set(BUILD_SUBDIR ${ANDROID_ABI})

find_library(PYTORCH_LIBRARY pytorch_jni
        PATHS ${PYTORCH_LINK_DIRS}
        NO_CMAKE_FIND_ROOT_PATH
        )
find_library(FBJNI_LIBRARY fbjni
        PATHS ${PYTORCH_LINK_DIRS}
        NO_CMAKE_FIND_ROOT_PATH
        )

# OpenCV
set(OPENCV_ANDROID_SDK D:/DigitalWhiteBoard/8_Semester/OpenCV-android-sdk)
#message(WARNING "OPENCV_ANDROID_SDK: ${OPENCV_ANDROID_SDK}")
#if(NOT DEFINED ${OPENCV_ANDROID_SDK})
#  message(FATAL_ERROR "Environment var OPENCV_ANDROID_SDK set")
#endif()

set(OPENCV_INCLUDE_DIR "${OPENCV_ANDROID_SDK}/sdk/native/jni/include")
#message(WARNING "<OPENCV_INCLUDE_DIR: ${OPENCV_INCLUDE_DIR}")

target_include_directories(${TARGET} PRIVATE
        ${OPENCV_INCLUDE_DIR}
        ${PYTORCH_INCLUDE_DIRS})

#target_include_directories(native-lib PRIVATE
#        ${OPENCV_INCLUDE_DIR}
#        ${PYTORCH_INCLUDE_DIRS})

set(OPENCV_LIB_DIR "${OPENCV_ANDROID_SDK}/sdk/native/libs/${ANDROID_ABI}")
#message(WARNING "<OPENCV_ANDROID_SDK: ${OPENCV_ANDROID_SDK}")

find_library(OPENCV_LIBRARY opencv_java4
        PATHS ${OPENCV_LIB_DIR}
        NO_CMAKE_FIND_ROOT_PATH
        )


find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library(jnigraphics-lib jnigraphics)

target_link_libraries(${TARGET}
        #native-lib
        ${PYTORCH_LIBRARY}
        ${FBJNI_LIBRARY}
        ${OPENCV_LIBRARY}
        ${jnigraphics-lib}
        ${CMAKE_DL_LIBS}
        ${log-lib})