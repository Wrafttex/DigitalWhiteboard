package com.example.digitalwhiteboard

import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.pm.PackageManager
import android.graphics.*
import android.os.Bundle
import android.util.Size
import android.view.MotionEvent
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

class CornerActivity : AppCompatActivity(), ImageAnalysis.Analyzer, View.OnTouchListener {
    private lateinit var binding: ActivityCornerBinding
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var testImage: PreviewView
    private lateinit var imageView: ImageView
    private lateinit var drawingOverlay: SurfaceView
    private lateinit var overlayHolder: SurfaceHolder
    private lateinit var autoButton: Button
    private var autoCornerBool: Boolean = false
    private var cornerPaint: Paint = Paint()
    private var boxPaint: Paint = Paint()
    private var corners: Array<Corner> = Array(4) { Corner(0f, 0f) }
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
        drawingOverlay.setOnTouchListener(this)
        val btn_set = findViewById<Button>(R.id.next)
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
        val widthMargin = drawingOverlay.width / 4f
        val heightMargin = drawingOverlay.height / 4f
        corners[0].x = widthMargin
        corners[0].y = heightMargin
        corners[1].x = widthMargin
        corners[1].y = (drawingOverlay.height - heightMargin)
        corners[2].x = (drawingOverlay.width - widthMargin)
        corners[2].y = (drawingOverlay.height - heightMargin)
        corners[3].x = (drawingOverlay.width - widthMargin)
        corners[3].y = heightMargin
    }

    private fun updateCorners(newCorners: Array<FloatArray>) {
        for (i in 0..3) {
            corners[i].x = newCorners[i][0]
            corners[i].y = newCorners[i][1]
        }
        drawBoxAndCorners()
    }


    override fun onTouch(view: View?, event: MotionEvent?): Boolean {
        when (event?.action) {
            MotionEvent.ACTION_DOWN -> for (corner in corners) {
                if (corner.isTouched(event.x, event.y)) {
                    corner.actionDown = true
                    break
                }
            }
            MotionEvent.ACTION_MOVE -> for (corner in corners) {
                if (corner.actionDown) {
                    corner.x = if (event.x > drawingOverlay.width) drawingOverlay.width.toFloat() else if (event.x < 0f) 0f else event.x
                    corner.y = if (event.y > drawingOverlay.height) drawingOverlay.height.toFloat() else if (event.y < 0f) 0f else event.y
                    drawBoxAndCorners()
                }
            }
            MotionEvent.ACTION_UP -> for (corner in corners) {
                corner.actionDown = false
            }
        }
        return true
    }

    private fun drawBoxAndCorners() {
        // var canvas = Canvas(drawing)
        val canvas = overlayHolder.lockCanvas()
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        path.reset()
        path.moveTo(corners[3].x, corners[3].y)
        for (corner in corners) {
            canvas.drawCircle(corner.x, corner.y, 20f, cornerPaint)
            path.lineTo(corner.x, corner.y)
        }
        canvas.drawPath(path, boxPaint)
        // canvas = overlayHolder.lockCanvas(null)
        // canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        // canvas.drawBitmap(drawing, 0f, 0f, null)
        overlayHolder.unlockCanvasAndPost(canvas)
    }

    fun btnFlipOnClick(view: View) {
        setCorners()
        drawBoxAndCorners()
    }

    fun autoDetectCorners(view: View) {
        if (!autoCornerBool) {
            autoCornerBool = true
            autoButton.text = "ON"
        } else {
            autoCornerBool = false
            autoButton.text = "OFF"
        }
    }

    external fun myFlip(bitmap: Bitmap, bitmapOut: Bitmap)

    companion object {
        // Used to load the 'digitalwhiteboard' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

}