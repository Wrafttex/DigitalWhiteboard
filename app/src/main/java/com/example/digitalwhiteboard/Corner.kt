package com.example.digitalwhiteboard

import android.graphics.Canvas
import android.graphics.Paint
import kotlin.math.pow
import kotlin.math.sqrt

class Corner(var x: Float, var y: Float) : java.io.Serializable {
    var actionDown = false

    fun isTouched (x: Float, y: Float): Boolean {
        return sqrt((this.x - x).pow(2) + (this.y - y).pow(2)) < 60f
    }
}