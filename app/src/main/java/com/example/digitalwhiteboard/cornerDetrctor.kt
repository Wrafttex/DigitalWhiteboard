package com.example.digitalwhiteboard

import android.graphics.Bitmap


class cornerDetrctor {

    external fun findCorners(bitmap: Bitmap, fAarOut: FloatArray)

    companion object {
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }
}