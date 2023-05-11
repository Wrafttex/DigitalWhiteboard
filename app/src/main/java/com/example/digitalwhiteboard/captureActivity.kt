package com.example.digitalwhiteboard

import android.graphics.Bitmap

class captureActivity (floatArray: FloatArray, bitmapIn: Bitmap) {

    private external fun createNativeCaptureActivity(floatArray: FloatArray, bitmapIn: Bitmap): Long
    private external fun destroyNativeCaptureActivity(ptr: Long)
    external fun capture(ptr: Long, bitmapIn: Bitmap, bitmapOut: Bitmap)

    companion object {
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }

    // Store a pointer to the C++ object
    private var nativePtr: Long

    init {
        nativePtr = createNativeCaptureActivity(floatArray, bitmapIn)
    }

    fun finalize() {
        // Destroy the C++ object
        destroyNativeCaptureActivity(nativePtr)
    }

    fun capture(BitmapIn: Bitmap, BitmapOut: Bitmap) {
        capture(nativePtr, BitmapIn, BitmapOut)
    }
}