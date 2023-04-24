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
import com.example.digitalwhiteboard.databinding.ActivityMainBinding
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors


class MainActivity : AppCompatActivity(), ImageAnalysis.Analyzer {

    private lateinit var binding: ActivityMainBinding
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
    private var corners: ArrayList<ArrayList<Float>> = ArrayList()
    private lateinit var drawing: Bitmap
    private var path: Path = Path()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
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
        }, ContextCompat.getMainExecutor(this))
    }
    private fun buildPreviewUseCase(): Preview {
        return Preview.Builder()
            .build().also { it.setSurfaceProvider(binding.viewFinder.surfaceProvider) }
    }

    private fun buildImageAnalysisUseCase(): ImageAnalysis {
        return ImageAnalysis.Builder()
            .setTargetResolution(Size(1280, 960))
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
        if (autoCornerBool) {
            val newCorners: ArrayList<ArrayList<Float>> = ArrayList()
            updateCorners(newCorners)
        }
        val bitmap = image.toBitmap()
        val rotatedImage = if (resources.configuration.orientation == android.content.res.Configuration.ORIENTATION_PORTRAIT) bitmap.rotate(90f) else bitmap
        val finalImage = rotatedImage.copy(rotatedImage.config, true)
        myFlip(rotatedImage, finalImage)
        runOnUiThread {
            binding.imageView.setImageBitmap(finalImage)
        }
        image.close()
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
        if (corners.isEmpty()) {
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
        }
        corners[0][0] = widthMargin
        corners[0][1] = heightMargin
        corners[1][0] = widthMargin
        corners[1][1] = (drawingOverlay.height - heightMargin)
        corners[2][0] = (drawingOverlay.width - widthMargin)
        corners[2][1] = (drawingOverlay.height - heightMargin)
        corners[3][0] = (drawingOverlay.width - widthMargin)
        corners[3][1] = heightMargin
    }


    /*
    private fun drawRectangle() {
        var canvas = Canvas(drawing)
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        path.reset()
        path.moveTo(corners[0][0], corners[0][1])
        for (ArrayList in corners) {
            canvas.drawCircle(ArrayList[0], ArrayList[1], ArrayList[2], cornerPaint)
            path.lineTo(ArrayList[0], ArrayList[1])
        }
        path.close()
        canvas.drawPath(path, rectanglePaint)
        canvas = overlayHolder.lockCanvas(null)
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        canvas.drawBitmap(drawing, 0f, 0f, null)
        overlayHolder.unlockCanvasAndPost(canvas)
    }
    */

    private fun updateCorners(newCorners: ArrayList<ArrayList<Float>>) {
        if (corners.isEmpty()) {
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
            corners.add(arrayListOf(0f, 0f, 20f))
        }
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
        for (ArrayList in corners) {
            canvas.drawCircle(ArrayList[0], ArrayList[1], ArrayList[2], cornerPaint)
            path.lineTo(ArrayList[0], ArrayList[1])
        }
        canvas.drawPath(path, boxPaint)
        overlayHolder.unlockCanvasAndPost(canvas)
    }

    fun Bitmap.rotate(degrees: Float): Bitmap {
        val matrix = Matrix().apply { postRotate(degrees) }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    fun btnFlipOnClick(view: View) {
        setCorners()
        drawBoxAndCorners()
    }

    fun autoDetectCorners(view: View) {
        autoCornerBool = autoCornerBool == false
    }

    /*
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

     */

    /*
    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
        this.doBlur()
    }

     */
    /**
     * A native method that is implemented by the 'digitalwhiteboard' native library,
     * which is packaged with this application.
     */
    /*
    override fun onStartTrackingTouch(seekBar: SeekBar?) {}
    */
    /*
    override fun onStopTrackingTouch(seekBar: SeekBar?) {}
    */
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