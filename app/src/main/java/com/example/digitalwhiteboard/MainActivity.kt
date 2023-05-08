package com.example.digitalwhiteboard

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import com.example.digitalwhiteboard.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val btn_set = findViewById<Button>(R.id.button_start)
        btn_set.setOnClickListener(){
            val intent = Intent (this@MainActivity, CornerActivity::class.java)
            startActivity(intent)
            finish()
        }


    }

}