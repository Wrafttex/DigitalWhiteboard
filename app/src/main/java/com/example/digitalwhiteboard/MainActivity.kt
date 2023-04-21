package com.example.digitalwhiteboard

import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.ImageView
import android.widget.SeekBar
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import com.example.digitalwhiteboard.databinding.ActivityMainBinding
import org.pytorch.IValue
import org.pytorch.Module
import org.pytorch.Tensor
import org.pytorch.torchvision.TensorImageUtils
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.lang.Float.max
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors


class MainActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener {

    private lateinit var binding: ActivityMainBinding
    private lateinit var cameraExecutor: ExecutorService
    var srcBitmap: Bitmap? = null
    var dstBitmap: Bitmap? = null
    private lateinit var testImage: PreviewView
    private lateinit var imageView: ImageView
    private lateinit var sldSigma: SeekBar
    var module: Module? = null
    private var testBitmap: Bitmap? = null
    private val TAG = "AssetUtils"


    //private fun assetAsIS(assentName: String )
    //    = this::class.java.getResourceAsStream(assentName).bufferedReader().readLine()

    private fun assetFilePath (context: Context, assetName: String): String? {
        val file = File(context.filesDir, assetName)
        if (file.exists() && file.length() > 0) {
            return file.absolutePath
        }
        try {
            context.assets.open(assetName).use { inputStream ->
                FileOutputStream(file).use { outputStream ->
                    val buffer = ByteArray(4 * 1024)
                    var read: Int
                    while (inputStream.read(buffer).also { read = it } != -1) {
                        outputStream.write(buffer, 0, read)
                    }
                    outputStream.flush()
                }
            }
            return file.absolutePath
        } catch (e: IOException) {
            Log.e(TAG, "Error process asset $assetName to file path", e)
        }
        return null
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        cameraExecutor = Executors.newSingleThreadExecutor()
        requestPermission()
        testImage = findViewById(R.id.viewFinder)
        sldSigma = findViewById(R.id.sldSigma)
        imageView = findViewById(R.id.imageView)
        sldSigma.setOnSeekBarChangeListener(this)

        try {
            //val inputStream: InputStream =  assetAsIS("app/src/main/res/drawable-nodpi/testimage2.png").byteInputStream()
            //testBitmap = BitmapFactory.decodeStream(inputStream)
            //Module.load("app/src/main/assets/vit_base_patch8_224.ptl")
            testBitmap = BitmapFactory.decodeStream(getAssets().open("testimage2.png"))
            module = Module.load(assetFilePath(this, "vit_base_patch8_224.ptl"))
        }catch(e: Exception) {
            e.printStackTrace()
        }

        /**
        val inputTensor: Tensor = TensorImageUtils.bitmapToFloat32Tensor(testBitmap, TensorImageUtils.TORCHVISION_NORM_MEAN_RGB, TensorImageUtils.TORCHVISION_NORM_MEAN_RGB)
        val outputTensor: Tensor = module!!.forward(IValue.from(inputTensor)).toTensor()

        val output: FloatArray = outputTensor.dataAsFloatArray
        println(output)
         **/

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
        }, ContextCompat.getMainExecutor(this))
    }
    private fun buildPreviewUseCase(): Preview {
        return Preview.Builder().build().also { it.setSurfaceProvider(binding.viewFinder.surfaceProvider) }
    }

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

    fun Bitmap.rotate(degrees: Float): Bitmap {
        val matrix = Matrix().apply { postRotate(degrees) }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    fun btnFlipOnClick(view: View) {
        /**
        if (srcBitmap == null && dstBitmap == null) {
            srcBitmap = testImage.bitmap
            dstBitmap = srcBitmap!!.copy(srcBitmap!!.config, true)
        }
        myFlip(srcBitmap!!,srcBitmap!!)
        this.doBlur()
**/
        val inputTensor: Tensor = TensorImageUtils.bitmapToFloat32Tensor(testBitmap, TensorImageUtils.TORCHVISION_NORM_MEAN_RGB, TensorImageUtils.TORCHVISION_NORM_MEAN_RGB)
        val outputTensor: Tensor = module!!.forward(IValue.from(inputTensor)).toTensor()

        val output: FloatArray = outputTensor.dataAsFloatArray
        println(output)
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