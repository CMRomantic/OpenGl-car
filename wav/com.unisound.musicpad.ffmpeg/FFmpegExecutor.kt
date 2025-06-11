package com.unisound.musicpad.ffmpeg

/**
 * Created by cmm on 2025/6/9.
 */
object FFmpegExecutor {
    init {
        System.loadLibrary("ffmpeg-convert")
    }

    external fun convertMp4ToWav(inputPath: String, outputPath: String): Int
    external fun getProgress(): Int
    external fun cancelConversion()
    external fun truncateWavFile(inputPath: String, outputPath: String, maxDurationMs: Int): Int
}