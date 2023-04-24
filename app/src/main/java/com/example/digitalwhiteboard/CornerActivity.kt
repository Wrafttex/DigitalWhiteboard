package com.example.digitalwhiteboard

import android.content.Intent
import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.util.Size
import android.widget.Button
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityCornerBinding
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class CornerActivity : AppCompatActivity(), ImageAnalysis.Analyzer {
    private lateinit var binding: ActivityCornerBinding
    private lateinit var cameraExecutor: ExecutorService

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityCornerBinding.inflate(layoutInflater)
        setContentView(binding.root)
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        val btn_set = findViewById<Button>(R.id.button_draw)
        btn_set.setOnClickListener(){
            val intent = Intent (this@CornerActivity, DrawActivity::class.java)
            /* Use this to send data from one activity to another, it can take basically all type value  */
            intent.putExtra("key","value")
            intent.putExtra("CornerValue", floatArrayOf(1.1f,2.2f,3.3f,4.4f))
            startActivity(intent)
            finish()
        }
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
            .build().also { it.setSurfaceProvider(binding.viewCorner?.surfaceProvider) }
    }

    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        Log.v("buildImageAnalysisUseCase","inside buildImageAnalysisUseCase")
        return ImageAnalysis.Builder()
            //.setTargetResolution(Size(imageView.measuredWidth, imageView.measuredHeight))
            .setTargetResolution(Size(1280, 720))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also { it.setAnalyzer(cameraExecutor, this) }
    }
    override fun analyze(image: ImageProxy) {
        //println("corner: we're inside analyze")
        val bitmap = image.toBitmap()
        //val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(270f) else bitmap
        val rotatedImage = bitmap
        val finalImage = rotatedImage.copy(rotatedImage.config, true)
        //myFlip(rotatedImage, finalImage)
//        if (!startBoolean){
//            runOnUiThread {
//                //binding.imageView.setVisibility(View.VISIBLE)
//                //binding.imageView.setImageBitmap(rotatedImage)
//            }
//        }
//        else {
//            //binding.imageView.setVisibility(View.INVISIBLE)
//        }
        image.close()
    }
}