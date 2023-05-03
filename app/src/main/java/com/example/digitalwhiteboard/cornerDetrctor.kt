package com.example.digitalwhiteboard

class cornerDetrctor {

    external fun findCorners()

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}