package com.example.digitalwhiteboard

import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.pm.PackageManager
import android.graphics.ImageFormat
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraManager
import android.hardware.camera2.params.StreamConfigurationMap
import android.os.Bundle
import android.util.Size
import android.view.WindowManager
import android.widget.Button
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.camera2.interop.Camera2CameraInfo
import androidx.camera.core.CameraSelector
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var resolutions: Array<Size>
    private lateinit var resolution: Size
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val previewView = findViewById<PreviewView>(R.id.viewFinder)
        val prefStorage = PrefStorage(this)
        val preferredResolution = prefStorage.storageRead("module","")
        if (preferredResolution.isNullOrEmpty() || !preferredResolution.contains('x')) {
            resolution = Size(1280, 720)
        } else {
            val splitString = preferredResolution.split("x")
            resolution = Size(splitString[0].toInt(), splitString[1].toInt())
        }
        previewView.layoutParams.width = resolution.width
        previewView.layoutParams.height = resolution.height
        this.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        requestPermission()
        val cornerButton = findViewById<Button>(R.id.corner)
        val settingsButton = findViewById<Button>(R.id.settings)

        cornerButton.setOnClickListener(){
            val intent = Intent (this@MainActivity, CornerActivity::class.java)
            startActivity(intent)
            finish()
        }

        settingsButton.setOnClickListener {
            val intent = Intent (this@MainActivity, SettingActivity::class.java)
            val availableResolutions: Array<IntArray>
            if (resolutions.isNullOrEmpty()) {
                resolutions = arrayOf(Size(1280, 720))
                availableResolutions = Array(resolutions.size) { intArrayOf(0, 0) }
            } else {
                availableResolutions = Array(resolutions.size) { intArrayOf(0, 0) }
                for (i in resolutions.indices) {
                    availableResolutions[i][0] = resolutions[i].width
                    availableResolutions[i][1] = resolutions[i].height
                }
            }
            intent.putExtra("key","value")
            intent.putExtra("availableResolutions", availableResolutions)
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

    @androidx.annotation.OptIn(androidx.camera.camera2.interop.ExperimentalCamera2Interop::class)
    private fun startCamera() {
        val processCameraProvider = ProcessCameraProvider.getInstance(this)
        processCameraProvider.addListener({
            try {
                val cameraProvider = processCameraProvider.get()
                val previewUseCase = buildPreviewUseCase()
                cameraProvider.unbindAll()
                val camera = cameraProvider.bindToLifecycle(
                    this,
                    CameraSelector.DEFAULT_BACK_CAMERA,
                    previewUseCase
                )
                val cameraId = Camera2CameraInfo.from(camera.cameraInfo).cameraId
                val cameraManager = binding.root.context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
                val characteristics = cameraManager.getCameraCharacteristics(cameraId)
                val configs: StreamConfigurationMap? = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                resolutions = configs?.getOutputSizes(ImageFormat.JPEG)!!
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }, ContextCompat.getMainExecutor(this))
    }

    private fun buildPreviewUseCase(): Preview {
        return Preview.Builder()
            .setTargetResolution(resolution)
            .build().also { it.setSurfaceProvider(binding.viewFinder.surfaceProvider) }
    }

}