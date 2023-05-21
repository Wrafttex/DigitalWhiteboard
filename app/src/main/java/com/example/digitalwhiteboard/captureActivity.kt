package com.example.digitalwhiteboard

import android.graphics.Bitmap


class captureActivity (floatArray: FloatArray, bitmapIn: Bitmap) {

    private external fun createNativeCaptureActivity(floatArray: FloatArray, bitmapIn: Bitmap): Long
    private external fun destroyNativeCaptureActivity(ptr: Long)
    private external fun getSize(floatArray: FloatArray, iArrOut: IntArray)
    external fun capture(ptr: Long, bitmapIn: Bitmap, bitmapOut: Bitmap)

    companion object {
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }

    // Store a pointer to the C++ object
    private var nativePtr: Long
    var width: Int
    var height: Int

    init {
        nativePtr = createNativeCaptureActivity(floatArray, bitmapIn)
        val size = IntArray(2)
        getSize(floatArray, size)
        width = size[0]
        height = size[1]

    }

    fun finalize() {
        // Destroy the C++ object
        destroyNativeCaptureActivity(nativePtr)
    }

    fun capture(BitmapIn: Bitmap, BitmapOut: Bitmap) {
        capture(nativePtr, BitmapIn, BitmapOut)
    }
}