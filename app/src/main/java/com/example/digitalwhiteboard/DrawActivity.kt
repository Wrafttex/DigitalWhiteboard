package com.example.digitalwhiteboard

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.Matrix
import android.os.Bundle
import android.util.Log
import android.util.Size
import android.view.View
import android.widget.Button
import android.widget.ImageView
import android.widget.Toast
import androidx.activity.OnBackPressedCallback
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityDrawBinding
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class DrawActivity : AppCompatActivity(), ImageAnalysis.Analyzer {

    private lateinit var binding: ActivityDrawBinding
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var imageView: ImageView
    private lateinit var startButton: Button
    private var startBoolean: Boolean = false
    private lateinit var corners: FloatArray
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDrawBinding.inflate(layoutInflater)
        val intent = intent
        corners = intent.getSerializableExtra("CornerValue") as FloatArray // TODO: getSerializableExtra is a deprecated method
        setContentView(binding.root)
        this.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        startButton = findViewById(R.id.start)
        imageView = findViewById(R.id.imageView)

        startButton.setOnClickListener {
            startOnClick(imageView)
        }

        onBackPressedDispatcher.addCallback(this, object: OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val intent = Intent (this@DrawActivity, CornerActivity::class.java)
                startActivity(intent)
            }
        })
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
                val imageAnalysisUseCase = buildImageAnalysisUseCase()
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(
                    this,
                    CameraSelector.DEFAULT_BACK_CAMERA,
                    imageAnalysisUseCase
                )
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }, ContextCompat.getMainExecutor(this))
    }

    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        Log.v("buildImageAnalysisUseCase","inside buildImageAnalysisUseCase")
        return ImageAnalysis.Builder()
            .setTargetResolution(Size(1600,900))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also { it.setAnalyzer(cameraExecutor, this) }
    }

    override fun analyze(image: ImageProxy) {
        val bitmap = image.toBitmap()
        val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(90f) else bitmap
        if (startBoolean) {
            val manipulatedImage = bitmap.rotate(90f)
            runOnUiThread {
                binding.imageView.setImageBitmap(manipulatedImage)
            }
        } else {
            runOnUiThread {
                binding.imageView.setImageBitmap(rotatedImage)
            }
        }
        image.close()
    }


    private fun Bitmap.rotate(degrees: Float): Bitmap {
        val matrix = Matrix().apply { postRotate(degrees) }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    fun startOnClick(view: View) {
        if (!startBoolean) {
            startButton.text = getString(R.string.OFF)
            startBoolean = true
        } else {
            startButton.text = getString(R.string.ON)
            startBoolean = false
            imageView.setImageBitmap(null)
        }
    }

    /**
     * A native method that is implemented by the 'digitalwhiteboard' native library,
     * which is packaged with this application.
     */

    companion object {
        // Used to load the 'digitalwhiteboard' library on application startup.
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }
}