package com.example.digitalwhiteboard

import android.content.Context
import android.content.SharedPreferences
import androidx.appcompat.app.AppCompatActivity
import androidx.test.core.app.ApplicationProvider
import androidx.test.core.app.ApplicationProvider.getApplicationContext
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mockito.mock
import org.mockito.junit.MockitoJUnitRunner
import org.mockito.Mock
import org.mockito.Mockito.mock
import org.mockito.kotlin.doReturn
import org.robolectric.RobolectricTestRunner


@RunWith(RobolectricTestRunner::class)
class PrefStorageTest {

    private lateinit var context: Context
    private lateinit var sharedPreferences:SharedPreferences
    private lateinit var prefStorage: PrefStorage

    @Before
    fun  setup(){
        context = getApplicationContext()
        prefStorage = PrefStorage(context)
    }

    @Test
    fun stringValidator(){
        prefStorage.storageWrite("string_unit_test","hello")
        assertEquals("hello",prefStorage.storageRead("string_unit_test",""))
        assertEquals(null,prefStorage.storageRead("string_unit_test1",""))
    }
    @Test
    fun intValidator(){
        prefStorage.storageWrite("int_unit_test",12)
        assertEquals(12,prefStorage.storageRead("int_unit_test",1))
        assertEquals(0,prefStorage.storageRead("int_unit_test1",1))
    }

    @Test
    fun booleanValidator(){
        prefStorage.storageWrite("boolean_unit_test",true)
        assertEquals(true,prefStorage.storageRead("boolean_unit_test",true))
        assertEquals(false,prefStorage.storageRead("bolean_unit_test1",true))
    }



}