package com.example.digitalwhiteboard

import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.Matrix
import android.os.Bundle
import android.util.Log
import android.util.Size
import android.view.View
import android.widget.Button
import android.widget.ImageView
import android.widget.SeekBar
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityDrawBinding
import java.lang.Float.max
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class DrawActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener, ImageAnalysis.Analyzer {

    private lateinit var binding: ActivityDrawBinding
    private lateinit var cameraExecutor: ExecutorService
    var srcBitmap: Bitmap? = null
    var dstBitmap: Bitmap? = null
    private lateinit var testImage: PreviewView
    private lateinit var imageView: ImageView
    private lateinit var sldSigma: SeekBar
    private lateinit var startButton: Button
    private var startBoolean: Boolean = false
    private lateinit var corners: Array<Corner>
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDrawBinding.inflate(layoutInflater)
        val intent = intent

        /*use this to get data from another activity, tho get depends on type value*/
        corners = intent.getSerializableExtra("CornerValue") as Array<Corner> // TODO: getSerializableExtra is a deprecated method
        setContentView(binding.root)
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        testImage = findViewById(R.id.viewFinder)
        startButton = findViewById(R.id.btnFlip)
        sldSigma = findViewById(R.id.sldSigma)
        imageView = findViewById(R.id.imageView)
        sldSigma.setOnSeekBarChangeListener(this)

        val btn_set = findViewById<Button>(R.id.SettingsButton)
        btn_set.setOnClickListener(){
            val intent = Intent (this, SettingActivity::class.java)
            startActivity(intent)
        }
        startButton.setOnClickListener(){
            btnFlipOnClick(imageView)
        }


        //if (testImage.previewStreamState.value == PreviewView.StreamState.STREAMING) {
        //    srcBitmap = testImage.bitmap
        //    imageView.setImageBitmap(srcBitmap)
        //}
    }

    private fun requestPermission() {
        requestCameraPermissionIfMissing { granted ->
            if (granted)
                startCamera()
            else
                Toast.makeText(this, "Please allow", Toast.LENGTH_LONG).show()
        }
    }

    private fun requestCameraPermissionIfMissing(onResult: ((Boolean) -> Unit)) {
        if(ContextCompat.checkSelfPermission(this, android.Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED)
            onResult(true)
        else
            registerForActivityResult(ActivityResultContracts.RequestPermission()) {
                onResult(it)
            }.launch(android.Manifest.permission.CAMERA)
    }

    private fun startCamera() {
        val processCameraProvider = ProcessCameraProvider.getInstance(this)
        processCameraProvider.addListener({
            try {
                val cameraProvider = processCameraProvider.get()
                val previewUseCase = buildPreviewUseCase()
                val imageAnalysisUseCase = buildImageAnalysisUseCase()
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(
                    this,
                    CameraSelector.DEFAULT_BACK_CAMERA,
                    previewUseCase,
                    imageAnalysisUseCase
                )
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }, ContextCompat.getMainExecutor(this)
        )
    }
    private fun buildPreviewUseCase(): Preview {
        return Preview.Builder()
            .build().also { it.setSurfaceProvider(binding.viewFinder?.surfaceProvider) }
    }

    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        Log.v("buildImageAnalysisUseCase","inside buildImageAnalysisUseCase")
        return ImageAnalysis.Builder()
            .setTargetResolution(Size(imageView.measuredWidth, imageView.measuredHeight))
            //.setTargetResolution(Size(1280, 720))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also { it.setAnalyzer(cameraExecutor, this) }
    }

    /*
    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        return ImageAnalysis.Builder()
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also {
                it.setAnalyzer(cameraExecutor) { image ->
                    val bitmap = image.toBitmap()
                    val rotatedImage = bitmap.rotate(90f)
                    val finalImage = rotatedImage!!.copy(rotatedImage!!.config, true)
                    myFlip(rotatedImage!!, finalImage!!)
                    runOnUiThread {
                        binding.imageView.setImageBitmap(finalImage)
                    }
                    image.close()
                }
            }
    }
    */

    override fun analyze(image: ImageProxy) {
        //println("we're inside analyze")
        val bitmap = image.toBitmap()
        //val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(270f) else bitmap
        val rotatedImage = bitmap
        val finalImage = rotatedImage.copy(rotatedImage.config, true)
        //myFlip(rotatedImage, finalImage)
        if (!startBoolean){
            runOnUiThread {
                //binding.imageView.setVisibility(View.VISIBLE)
                //binding.imageView.setImageBitmap(rotatedImage)
            }
        }
        else {
            //binding.imageView.setVisibility(View.INVISIBLE)
        }
        image.close()
    }


    fun Bitmap.rotate(degrees: Float): Bitmap {
        val matrix = Matrix().apply { postRotate(degrees) }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    fun btnFlipOnClick(view: View) {
        if (!startBoolean) {
            startButton.text = "Stop"
            startBoolean = true
        } else {
            startButton.text = "Start"
            startBoolean = false
            imageView.setImageBitmap(null)
        }


        /*
        if (srcBitmap == null && dstBitmap == null) {
            srcBitmap = testImage.bitmap
            dstBitmap = srcBitmap!!.copy(srcBitmap!!.config, true)
        }
        myFlip(srcBitmap!!,srcBitmap!!)
        this.doBlur()
         */
    }

    private fun doBlur() {
        if (srcBitmap == null && dstBitmap == null) {
            srcBitmap = testImage.bitmap
            dstBitmap = srcBitmap!!.copy(srcBitmap!!.config, true)
        }
        // The SeekBar range is 0-100 convert it to 0.1-10
        val sigma = max(0.1F, sldSigma.progress / 10F)

        // This is the actual call to the blur method inside native-lib.cpp
        this.myBlur(srcBitmap!!, dstBitmap!!, sigma)
        imageView.setImageBitmap(dstBitmap)
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