package com.example.digitalwhiteboard

import android.graphics.Bitmap

class captureActivity {
    external fun capture(bitmapIn: Bitmap, bitmapOut: Bitmap)

    companion object {
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }
}