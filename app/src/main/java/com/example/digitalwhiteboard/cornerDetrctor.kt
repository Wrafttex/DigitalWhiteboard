package com.example.digitalwhiteboard

import android.graphics.Bitmap


class cornerDetrctor() {

    private external fun createNativeObject(): Long
    private external fun destroyNativeObject(ptr: Long)
    external fun findCorners(ptr: Long, bitmap: Bitmap, fAarOut: FloatArray)

    companion object {
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }

    // Store a pointer to the C++ object
    private var nativePtr: Long

    init {
        nativePtr = createNativeObject()
    }

    fun finalize() {
        // Destroy the C++ object
        destroyNativeObject(nativePtr)
    }

    fun findCorners(bitmap: Bitmap, fAarOut: FloatArray) {
        findCorners(nativePtr, bitmap, fAarOut)
    }
}