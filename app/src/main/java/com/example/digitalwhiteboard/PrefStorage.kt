package com.example.digitalwhiteboard

import android.content.Context
import android.content.SharedPreferences
//import android.content
//import android.content.ContextWrapper
import androidx.appcompat.app.AppCompatActivity
//private val context: Context
class PrefStorage (private val context: Context) {
    //val context = object: Context(){}
    private fun storagePointer (): SharedPreferences {
    return context.getSharedPreferences("com.example.digitalwhiteboard.settings", Context.MODE_PRIVATE)
    }
    fun storageRead (place:String,datatype:String): String? {
        val sharedPref = storagePointer()
        return sharedPref.getString(place,null)
    }
    fun storageRead (place:String,datatype: Boolean):Boolean{
        val sharedPref = storagePointer()
        return sharedPref.getBoolean(place,false)
    }
    fun storageRead (place:String,datatype: Int):Int{
        val sharedPref = storagePointer()
        return sharedPref.getInt(place,0)
    }

    fun storageWrite (place: String,datavalue:String){
        val sharedPref = storagePointer()
        val editor = sharedPref.edit()
        editor.apply{
            putString(place,datavalue)
            apply()
        }
    }
    fun storageWrite (place: String,datavalue:Int){
        val sharedPref = storagePointer()
        val editor = sharedPref.edit()
        editor.apply{
            putInt(place,datavalue)
            apply()
        }
    }
    fun storageWrite (place: String,datavalue:Boolean){
        val sharedPref = storagePointer()
        val editor = sharedPref.edit()
        editor.apply{
            putBoolean(place,datavalue)
            apply()
        }
    }
}