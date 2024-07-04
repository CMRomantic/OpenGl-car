package com.aonions.opengl

import com.aonions.opengl.databinding.ActivityCameraBinding

/**
 * Created by mm on 2024/4/3.
 */
class CameraActivity : BaseActivity<ActivityCameraBinding>() {

    override fun setBinding(): ActivityCameraBinding {
        return ActivityCameraBinding.inflate(layoutInflater)
    }
}