package com.aonions.opengl.jni

import android.content.res.AssetManager

/**
 * Created by cmm on 2023/4/11.
 */
object MainApp {

    init {
        System.loadLibrary("aonions")
    }

    external fun test(assets: AssetManager):String

    external fun onSurfaceCreated()

    external fun onSurfaceChanged(width: Int, height:Int)

    external fun onDrawFrame()

    external fun onDestroy()
}