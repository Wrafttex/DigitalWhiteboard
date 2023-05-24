package com.example.digitalwhiteboard

import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.util.DisplayMetrics
import android.view.View
import android.view.WindowManager
import android.widget.*
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity


class SettingActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_setting)
        this.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        val dropdown: Spinner = findViewById(R.id.spinner2)
        val prefStorage = PrefStorage(this)
        val metrics = DisplayMetrics()
        windowManager.defaultDisplay.getMetrics(metrics)
        val height = metrics.heightPixels
        val width = metrics.widthPixels
        val availableResolutions = ArrayList<String>()
        val resolutions = intent.getSerializableExtra("availableResolutions") as Array<IntArray> // TODO: getSerializableExtra is a deprecated method
        for (i in resolutions.indices) {
            if (width >= resolutions[i][0] && height >= resolutions[i][1] && resolutions[i][1] >= 360) {
                availableResolutions.add(resolutions[i][0].toString() + "x" + resolutions[i][1].toString())
            }
        }
        val mySwitch = findViewById<Switch>(R.id.switch_is_color)
        val saveModuleItem = prefStorage.storageRead("module","")
        println(availableResolutions.indexOf(saveModuleItem))



        dropdown.adapter = ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, availableResolutions)
        if (availableResolutions.indexOf(saveModuleItem) >=0)
            dropdown.setSelection(availableResolutions.indexOf(saveModuleItem))

        dropdown.onItemSelectedListener  = object : AdapterView.OnItemSelectedListener{
            override fun onNothingSelected(parent: AdapterView<*>?) {

            }
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                prefStorage.storageWrite("module",dropdown.selectedItem.toString())

            }
        }
        mySwitch.isChecked = prefStorage.storageRead("color",false)
        mySwitch.setOnCheckedChangeListener {buttonView , isChecked ->
            if (isChecked){
                prefStorage.storageWrite("color",true)
            }
            else {
                prefStorage.storageWrite("color",false)
            }
        }

        onBackPressedDispatcher.addCallback(this, object: OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val intent = Intent (this@SettingActivity, MainActivity::class.java)
                startActivity(intent)
            }
        })
    }

}