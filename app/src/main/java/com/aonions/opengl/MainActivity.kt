package com.aonions.opengl

import android.util.Log
import com.aonions.opengl.databinding.ActivityMainBinding
import com.aonions.opengl.jni.MainApp
import com.aonions.opengl.surface.MyGLRender

class MainActivity : BaseActivity<ActivityMainBinding>() {

    private val glRender = MyGLRender()

    companion object {
        private const val TAG = "MainActivity"
    }

    override fun setBinding(): ActivityMainBinding = ActivityMainBinding.inflate(layoutInflater)

    override fun initData() {
        MainApp.test(assets)
//        Log.d(TAG, MainApp.test(resources.assets))
        getBinding().glSurface.setRender(glRender)
    }

    override fun onDestroy() {
        super.onDestroy()
        glRender.onDestroy()
    }
}