package com.aonions.opengl

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.viewbinding.ViewBinding

/**
 * Created by cmm on 2023/4/11.
 */
abstract class BaseActivity<T : ViewBinding> : AppCompatActivity() {

    private lateinit var mBinding: T

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mBinding = setBinding()
        setContentView(mBinding.root)

        initData()
    }

    protected abstract fun setBinding(): T

    open fun getBinding(): T {
        return mBinding
    }

    open fun initData() {}

}