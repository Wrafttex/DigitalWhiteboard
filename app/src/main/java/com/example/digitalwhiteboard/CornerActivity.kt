package com.example.digitalwhiteboard

import android.content.pm.ActivityInfo
import android.content.pm.PackageManager
import android.graphics.*
import android.os.Bundle
import android.util.Size
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.widget.Button
import android.widget.ImageView
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
import com.example.digitalwhiteboard.databinding.ActivityCornerBinding
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import android.content.Intent

class CornerActivity : AppCompatActivity(), ImageAnalysis.Analyzer {
    private lateinit var binding: ActivityCornerBinding
    private lateinit var cameraExecutor: ExecutorService
    var srcBitmap: Bitmap? = null
    var dstBitmap: Bitmap? = null
    private lateinit var testImage: PreviewView
    private lateinit var imageView: ImageView
    private lateinit var drawingOverlay: SurfaceView
    private lateinit var overlayHolder: SurfaceHolder
    private lateinit var autoButton: Button
    var autoCornerBool: Boolean = false
    var startBool: Boolean = false
    var testBool: Boolean = false
    private var cornerPaint: Paint = Paint()
    private var boxPaint: Paint = Paint()
    private var corners: Array<FloatArray> = Array(4){ FloatArray(2) }
    private lateinit var drawing: Bitmap
    private var path: Path = Path()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityCornerBinding.inflate(layoutInflater)
        setContentView(binding.root)
        this.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        setPaint()
        testImage = findViewById(R.id.viewFinder)
        autoButton = findViewById(R.id.autoCorner)
        imageView = findViewById(R.id.imageView)
        drawingOverlay = findViewById(R.id.drawingOverlay)
        drawingOverlay.setZOrderOnTop(true)
        overlayHolder = drawingOverlay.holder
        overlayHolder.setFormat(PixelFormat.TRANSPARENT)
        val btn_set = findViewById<Button>(R.id.manualCorner)
        btn_set.setOnClickListener(){
            val intent = Intent (this@CornerActivity, DrawActivity::class.java)
            /* Use this to send data from one activity to another, it can take basically all type value  */
            intent.putExtra("key","value")
            intent.putExtra("CornerValue", corners)
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
            .build().also { it.setSurfaceProvider(binding.viewFinder?.surfaceProvider) }
    }

    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        return ImageAnalysis.Builder()
            .setTargetResolution(Size(1280, 960))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also { it.setAnalyzer(cameraExecutor, this) }
    }

    override fun analyze(image: ImageProxy) {
        if (autoCornerBool) {
            val newCorners: Array<FloatArray> = Array(4){ FloatArray(2) }
            newCorners[0][0] = 1280f
            newCorners[0][1] = 960f
            newCorners[1][0] = 1280f
            newCorners[1][1] = 0f
            newCorners[2][0] = 0f
            newCorners[2][1] = 0f
            newCorners[3][0] = 0f
            newCorners[3][1] = 960f
            updateCorners(newCorners)
        }
        val bitmap = image.toBitmap()
        val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(90f) else bitmap
        val finalImage = rotatedImage.copy(rotatedImage.config, true)
        myFlip(rotatedImage, finalImage)
        runOnUiThread {
            binding.imageView?.setImageBitmap(finalImage)
        }
        image.close()
    }

    fun Bitmap.rotate(degrees: Float): Bitmap {
        val matrix = Matrix().apply { postRotate(degrees) }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    private fun setPaint() {
        cornerPaint.style = Paint.Style.FILL
        cornerPaint.color = Color.RED
        boxPaint.style = Paint.Style.STROKE
        boxPaint.color = Color.RED
        boxPaint.strokeWidth = 10f
    }

    private fun setCorners() {
        val widthMargin = drawingOverlay.width / 1f
        val heightMargin = drawingOverlay.height / 1f
        corners[0][0] = widthMargin
        corners[0][1] = heightMargin
        corners[1][0] = widthMargin
        corners[1][1] = (drawingOverlay.height - heightMargin)
        corners[2][0] = (drawingOverlay.width - widthMargin)
        corners[2][1] = (drawingOverlay.height - heightMargin)
        corners[3][0] = (drawingOverlay.width - widthMargin)
        corners[3][1] = heightMargin
    }

    private fun updateCorners(newCorners: Array<FloatArray>) {
        for (i in 0..3) {
            for (j in 0..1) {
                corners[i][j] = newCorners[i][j]
            }
        }
        drawBoxAndCorners()
    }

    private fun drawBoxAndCorners() {
        val canvas = overlayHolder.lockCanvas()
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
        path.reset()
        path.moveTo(corners[3][0], corners[3][1])
        for (Array in corners) {
            canvas.drawCircle(Array[0], Array[1], 20f, cornerPaint)
            path.lineTo(Array[0], Array[1])
        }
        canvas.drawPath(path, boxPaint)
        overlayHolder.unlockCanvasAndPost(canvas)
    }

    fun btnFlipOnClick(view: View) {
        setCorners()
        drawBoxAndCorners()
    }

    fun autoDetectCorners(view: View) {
        autoCornerBool = autoCornerBool == false
    }

    external fun myFlip(bitmap: Bitmap, bitmapOut: Bitmap)

    companion object {
        // Used to load the 'digitalwhiteboard' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

}