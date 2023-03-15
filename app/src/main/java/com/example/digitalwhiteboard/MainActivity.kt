package com.example.digitalwhiteboard

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.media.Image
import android.os.Bundle
import android.view.View
import android.widget.ImageView
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import java.lang.Float.max


class MainActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener {
    var srcBitmap: Bitmap? = null
    var dstBitmap: Bitmap? = null
    private lateinit var testImage: ImageView
    lateinit var sldSigma: SeekBar
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        testImage = findViewById(R.id.imageView)
        sldSigma = findViewById(R.id.sldSigma)

        srcBitmap = BitmapFactory.decodeResource(this.resources, R.drawable.testimage)
        dstBitmap = srcBitmap!!.copy(srcBitmap!!.config, true)
        testImage.setImageBitmap(dstBitmap)

        sldSigma.setOnSeekBarChangeListener(this)


    }

    fun btnFlipOnClick(view: View) {
        myFlip(srcBitmap!!,srcBitmap!!)
        this.doBlur()
    }

    private fun doBlur() {
        // The SeekBar range is 0-100 convert it to 0.1-10
        val sigma = max(0.1F, sldSigma.progress / 10F)

        // This is the actual call to the blur method inside native-lib.cpp
        this.myBlur(srcBitmap!!, dstBitmap!!, sigma)
    }

    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
        this.doBlur()
    }
    /**
     * A native method that is implemented by the 'digitalwhiteboard' native library,
     * which is packaged with this application.
     */
    override fun onStartTrackingTouch(seekBar: SeekBar?) {}
    override fun onStopTrackingTouch(seekBar: SeekBar?) {}
    external fun stringFromJNI(): String
    external fun myFlip(bitmap: Bitmap, bitmapOut: Bitmap)
    external fun myBlur(bitmap: Bitmap, bitmapOut: Bitmap, sigma: Float)

    companion object {
        // Used to load the 'digitalwhiteboard' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}