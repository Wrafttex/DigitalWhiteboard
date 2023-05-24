package com.example.digitalwhiteboard

import org.junit.Assert.*
import org.junit.Before
import org.junit.Test

class CornerTest{
    var getCorner: Corner = Corner(1f,2f)

    @Test
    fun cornerInvalid(){
        println(getCorner.isTouched(x = 180f, y = 180f))
        assertFalse( getCorner.isTouched(x = 180f, y = 180f))
    }
    @Test
    fun cornerValid(){
        assertTrue(getCorner.isTouched(x = 0f, y = 0f))
    }
}