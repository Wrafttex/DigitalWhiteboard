package com.example.digitalwhiteboard

class captureActivity {
    external fun capture()

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}