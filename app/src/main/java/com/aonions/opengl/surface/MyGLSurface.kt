package com.aonions.opengl.surface

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

/**
 * Created by cmm on 2023/4/25.
 */
class MyGLSurface(context: Context?, attrs: AttributeSet?) : GLSurfaceView(context, attrs) {

    init {
        setEGLContextClientVersion(2)
        setEGLConfigChooser(8, 8, 8, 8, 16, 8)
    }

    fun setRender(render: MyGLRender){
        setRenderer(render)
        renderMode = RENDERMODE_CONTINUOUSLY
    }
}