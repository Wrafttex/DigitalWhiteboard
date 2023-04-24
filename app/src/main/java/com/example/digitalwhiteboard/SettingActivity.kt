package com.example.digitalwhiteboard

import android.os.Bundle
import android.view.View
import android.widget.*
import androidx.appcompat.app.AppCompatActivity


class SettingActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_setting)
        val dropdown: Spinner = findViewById(R.id.spinner2)
        val prefStor = PrefStorage(this)
        val moduleItems = arrayOf("1", "2", "3")
        val mySwitch = findViewById<Switch>(R.id.switch_is_color)
        val saveModuleItem = prefStor.storageRead("module","")
        println(moduleItems.indexOf(saveModuleItem))

        dropdown.adapter = ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, moduleItems)
        if (moduleItems.indexOf(saveModuleItem) >=0)
            dropdown.setSelection(moduleItems.indexOf(saveModuleItem))

        dropdown.onItemSelectedListener  = object : AdapterView.OnItemSelectedListener{
            override fun onNothingSelected(parent: AdapterView<*>?) {

            }
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                prefStor.storageWrite("module",dropdown.selectedItem.toString())

            }
        }
        mySwitch.isChecked = prefStor.storageRead("color",false)
        mySwitch.setOnCheckedChangeListener {buttonView , isChecked ->
            if (isChecked){
                prefStor.storageWrite("color",true)
                //println("ischecked")
            }
            else {
                prefStor.storageWrite("color",false)
                //println("isunchecked")
            }
        }
        val btn_set_return = findViewById<Button>(R.id.btn_set_return)
        btn_set_return.setOnClickListener(){
            finish()
        }


    }

}