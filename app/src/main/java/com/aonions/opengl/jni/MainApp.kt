package com.aonions.opengl.jni

/**
 * Created by cmm on 2023/4/11.
 */
object MainApp {

    init {
        System.loadLibrary("aonions")
    }

    external fun test(): String
}