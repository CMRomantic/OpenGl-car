package com.aonions.opengl

import android.util.Log
import com.aonions.opengl.databinding.ActivityMainBinding
import com.aonions.opengl.jni.MainApp

class MainActivity : BaseActivity<ActivityMainBinding>() {

    companion object {
        private const val TAG = "MainActivity"
    }

    override fun setBinding(): ActivityMainBinding = ActivityMainBinding.inflate(layoutInflater)

    override fun initData() {
        getBinding().name.text = MainApp.test()
    }
}