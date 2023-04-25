package com.aonions.opengl.surface

import android.opengl.GLSurfaceView
import com.aonions.opengl.jni.MainApp
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * Created by cmm on 2023/4/25.
 */
class MyGLRender: GLSurfaceView.Renderer {

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        MainApp.onSurfaceCreated()
    }

    override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
        MainApp.onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(p0: GL10?) {
        MainApp.onDrawFrame()
    }

    fun onDestroy(){
        MainApp.onDestroy()
    }
}