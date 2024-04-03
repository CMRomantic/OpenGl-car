package com.aonions.opengl

import android.media.AudioManager
import android.media.MediaPlayer
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import com.aonions.opengl.databinding.ActivityExoPlayerBinding


/**
 * Created by cmm on 2024/3/19.O
 */
class ExoActivity : BaseActivity<ActivityExoPlayerBinding>() {

    private var isUserSeeking = false

//    val url = "http://lj.sycdn.kuwo.cn/095a3f42c77236897ba6ec70aa243566/65f931cb/resource/a2/82/81/706946768.aac"
    val url = "http://lw.sycdn.kuwo.cn/5a640c48841d655d701ab3193f31d0ba/65f931c5/resource/30106/trackmedia/C200003V7OMc0D2QCg.m4a"

    private var mMediaPlayer: MediaPlayer? = null


    override fun setBinding(): ActivityExoPlayerBinding {
        return ActivityExoPlayerBinding.inflate(layoutInflater)
    }

    override fun initData() {

        mMediaPlayer = MediaPlayer()

        mMediaPlayer?.setAudioStreamType(AudioManager.STREAM_MUSIC) // 设置音频类型

        mMediaPlayer?.setDataSource(url)

        mMediaPlayer?.prepareAsync()

        getBinding().next.setOnClickListener {
            mMediaPlayer?.start()
        }

        // 设置 SeekBar 的监听器
        getBinding().seekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    val duration = mMediaPlayer?.duration ?: 0
                    val newPosition = duration * progress / 1000
                    mMediaPlayer?.seekTo(newPosition)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {
                isUserSeeking = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {
                isUserSeeking = false
            }
        })

        // 定时更新 SeekBar 的进度

        // 定时更新 SeekBar 的进度
        Thread {
            while (true) {
                if (!isUserSeeking) {
                    val currentPosition = mMediaPlayer?.currentPosition ?: 0
                    val duration = mMediaPlayer?.duration ?: 0
                    val progress = (currentPosition * 1000L / duration).toInt()
                    runOnUiThread { getBinding().seekBar.progress = progress }
                }
                try {
                    Thread.sleep(1000) // 每秒更新一次进度
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
            }
        }.start()
    }

    override fun onDestroy() {
        super.onDestroy()
    }

}