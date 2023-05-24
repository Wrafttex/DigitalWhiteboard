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
import androidx.activity.OnBackPressedCallback
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityCornerBinding
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import com.example.digitalwhiteboard.cornerDetrctor

class CornerActivity : AppCompatActivity(), ImageAnalysis.Analyzer, View.OnTouchListener {
    private lateinit var binding: ActivityCornerBinding
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var imageView: ImageView
    private lateinit var drawingOverlay: SurfaceView
    private lateinit var overlayHolder: SurfaceHolder
    private lateinit var autoButton: Button
    private lateinit var nextButton: Button
    private var autoCornerBool: Boolean = false
    private var cornerPaint: Paint = Paint()
    private var boxPaint: Paint = Paint()
    private var corners: Array<Corner> = Array(4) { Corner(0f, 0f) }
    private lateinit var newCorners: FloatArray
    private var path: Path = Path()
    private var cornerDetrctor = cornerDetrctor()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityCornerBinding.inflate(layoutInflater)
        setContentView(binding.root)
        this.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        setPaint()
        autoButton = findViewById(R.id.autoCorner)
        nextButton = findViewById(R.id.next)
        nextButton.visibility = View.INVISIBLE
        imageView = findViewById(R.id.imageView)
        drawingOverlay = findViewById(R.id.drawingOverlay)
        drawingOverlay.setZOrderOnTop(true)
        overlayHolder = drawingOverlay.holder
        overlayHolder.setFormat(PixelFormat.TRANSPARENT)
        drawingOverlay.setOnTouchListener(this)
        val nextButton = findViewById<Button>(R.id.next)

        nextButton.setOnClickListener {
            val intent = Intent (this@CornerActivity, DrawActivity::class.java)
            /* Use this to send data from one activity to another, it can take basically all type value  */
            for (i in 0..3) {
                newCorners[i*2] = corners[i].x
                newCorners[(i*2)+1] = corners[i].y
            }
            intent.putExtra("key","value")
            intent.putExtra("CornerValue", newCorners)
            startActivity(intent)
            finish()
        }

        onBackPressedDispatcher.addCallback(this, object: OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val intent = Intent (this@CornerActivity, MainActivity::class.java)
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
        return ImageAnalysis.Builder()
            .setTargetResolution(Size(1280, 960))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .build().also { it.setAnalyzer(cameraExecutor, this) }
    }

    override fun analyze(image: ImageProxy) {
        val bitmap = image.toBitmap()
        if (autoCornerBool) {
            newCorners = FloatArray(8) { 0f }
            cornerDetrctor.findCorners(bitmap, newCorners)
            updateCorners(newCorners)
        }
        val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(90f) else bitmap // Makes image turn correctly in relation to portrait/landscape
        runOnUiThread {
            binding.imageView.setImageBitmap(rotatedImage)
        }
        image.close()
    }

    private fun Bitmap.rotate(degrees: Float): Bitmap {
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

    private fun updateCorners(newCorners: FloatArray) {
        for (i in 0..3) {
            corners[i].x = if (newCorners[i*2] > drawingOverlay.width) drawingOverlay.width.toFloat() else if (newCorners[i*2] < 0f) 0f else newCorners[i*2]
            corners[i].y = if (newCorners[(i*2)+1] > drawingOverlay.height) drawingOverlay.height.toFloat() else if (newCorners[(i*2)+1] < 0f) 0f else newCorners[(i*2)+1]
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
        val canvas = overlayHolder.lockCanvas()
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        path.reset()
        path.moveTo(corners[3].x, corners[3].y)
        for (corner in corners) {
            canvas.drawCircle(corner.x, corner.y, 20f, cornerPaint)
            path.lineTo(corner.x, corner.y)
        }
        canvas.drawPath(path, boxPaint)
        overlayHolder.unlockCanvasAndPost(canvas)
    }

    fun autoDetectCorners(view: View) {
        if (!autoCornerBool) {
            autoCornerBool = true
            autoButton.text = getString(R.string.OFF)
        } else {
            autoCornerBool = false
            autoButton.text = getString(R.string.ON)
        }
        nextButton.visibility = View.VISIBLE
    }

    companion object {
        // Used to load the 'digitalwhiteboard' library on application startup.
        init {
            System.loadLibrary("digitalwhiteboard")
        }
    }

}