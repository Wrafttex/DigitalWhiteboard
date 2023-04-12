package com.example.digitalwhiteboard

import android.content.Context
import android.os.Bundle
import android.view.View
import android.widget.*
import androidx.appcompat.app.AppCompatActivity


class SettingActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_setting)
        val dropdown: Spinner = findViewById(R.id.spinner2)
        val items = arrayOf("1", "2", "3")
        var checker = 0
        dropdown.adapter = ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, items)
        dropdown.setSelection(2)

        val prefstorage = PrefStorage(this)
        dropdown.onItemSelectedListener  = object : AdapterView.OnItemSelectedListener{
            override fun onNothingSelected(parent: AdapterView<*>?) {

            }
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                if (checker ==1){
                    prefstorage.storageWrite("module",dropdown.selectedItem.toString())
                    println("selected item")
                }
                else {
                    checker = 1
                    println("first creation")
                }
            }
        }
        val mySwitch = findViewById<Switch>(R.id.switch_is_color)
        mySwitch.setOnCheckedChangeListener {buttonView , isChecked ->
            if (isChecked){
                prefstorage.storageWrite("color",true)
                println("ischecked")
            }
            else {
                prefstorage.storageWrite("color",false)
                println("isunchecked")
            }
        }
        val btn_set_return = findViewById<Button>(R.id.btn_set_return)
        btn_set_return.setOnClickListener(){
            finish()
        }


    }
}