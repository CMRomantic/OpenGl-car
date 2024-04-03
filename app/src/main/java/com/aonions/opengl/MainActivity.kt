package com.aonions.opengl

import android.util.Log
import com.aonions.opengl.databinding.ActivityMainBinding
import com.aonions.opengl.jni.ISPJni
import com.aonions.opengl.jni.IoHandle
import com.aonions.opengl.jni.MainApp
import com.aonions.opengl.surface.MyGLRender

class MainActivity : BaseActivity<ActivityMainBinding>() {

    //private val glRender = MyGLRender()

    companion object {
        private const val TAG = "MainActivity"
    }

    override fun setBinding(): ActivityMainBinding = ActivityMainBinding.inflate(layoutInflater)

    override fun initData() {
        //MainApp.test(assets)
        ISPJni.checksum("123".toByteArray(), 3)
//        Log.d(TAG, MainApp.test(resources.assets))
        //getBinding().glSurface.setRender(glRender)
        val handle = IoHandle()
        handle.devOpen = 1
        handle.bResendFlag = 1
        handle.usCheckSum = "123".toByte()
        handle.uCmdIndex = 1
        handle.buffer = byteArrayOf(1,2,3)
        getBinding().open.setOnClickListener {
            ISPJni.open(handle)
        }
        val config = arrayOf(1, 2,3)
        getBinding().update.setOnClickListener {
            ISPJni.updateConfig(
                handle,
                config.toIntArray(),
                config.toIntArray(),
            )
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        //glRender.onDestroy()
    }
}