// ffmpeg_wrapper.cpp
#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/log.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#define LOG_TAG "FFmpegWrapper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 全局状态变量
static volatile int g_progress = 0;
static volatile bool g_cancel = false;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// FFmpeg日志回调
void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl) {
    if (level <= AV_LOG_WARNING) {
        char log[1024];
        vsnprintf(log, sizeof(log), fmt, vl);
        LOGD("FFmpeg: %s", log);
    }
}

void write_wav_header(FILE* file, int data_size, int sample_rate, int channels) {
    // RIFF header
    fwrite("RIFF", 1, 4, file);
    int file_size = data_size + 36;
    fwrite(&file_size, 4, 1, file);
    fwrite("WAVE", 1, 4, file);
    
    // fmt chunk
    fwrite("fmt ", 1, 4, file);
    int fmt_size = 16;
    fwrite(&fmt_size, 4, 1, file);
    short audio_format = 1; // PCM
    fwrite(&audio_format, 2, 1, file);
    fwrite(&channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    int byte_rate = sample_rate * channels * 2; // 16-bit
    fwrite(&byte_rate, 4, 1, file);
    short block_align = channels * 2; // 16-bit
    fwrite(&block_align, 2, 1, file);
    short bits_per_sample = 16;
    fwrite(&bits_per_sample, 2, 1, file);
    
    // data chunk
    fwrite("data", 1, 4, file);
    fwrite(&data_size, 4, 1, file);
}

// 实际的MP4转WAV转换函数
int convert_mp4_to_wav(const char* input_path, const char* output_path) {
    // 将所有变量声明在作用域顶部
    AVFormatContext* in_ctx = nullptr;
    AVCodecContext* codec_ctx = nullptr;
    SwrContext* swr_ctx = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* pkt = nullptr;
    FILE* out_file = nullptr;
    int audio_stream_index = -1;
    int ret = 0;
    int64_t total_samples = 0;
    const int max_duration_ms = 30000; // 30秒
    const int out_sample_rate = 44100;
    const int out_channels = 2;
	
	in_ctx = avformat_alloc_context();
    
    // 初始化FFmpeg
    av_log_set_callback(ffmpeg_log_callback);
    av_log_set_level(AV_LOG_DEBUG);
	
	LOGD("input path: %s, \noutput path: %s", input_path, output_path);
    
    // 1. 打开输入文件
    if ((ret = avformat_open_input(&in_ctx, input_path, nullptr, nullptr))) {
        LOGE("Could not open input file: %s", av_err2str(ret));
        return ret;
    }
    
    // 2. 查找流信息
    if ((ret = avformat_find_stream_info(in_ctx, nullptr))) {
        LOGE("Could not find stream info: %s", av_err2str(ret));
        avformat_close_input(&in_ctx);
        return ret;
    }
    
    // 3. 查找音频流
    for (int i = 0; i < in_ctx->nb_streams; i++) {
        if (in_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }
    
    if (audio_stream_index == -1) {
        LOGE("No audio stream found");
        avformat_close_input(&in_ctx);
        return -1;
    }
    
    // 4. 获取解码器
    AVCodecParameters* codec_params = in_ctx->streams[audio_stream_index]->codecpar;
    const AVCodec* decoder = avcodec_find_decoder(codec_params->codec_id);
    if (!decoder) {
        LOGE("Unsupported codec");
        avformat_close_input(&in_ctx);
        return -1;
    }
    
    // 5. 创建解码器上下文
    codec_ctx = avcodec_alloc_context3(decoder);
    if (!codec_ctx) {
        LOGE("Failed to allocate codec context");
        avformat_close_input(&in_ctx);
        return AVERROR(ENOMEM);
    }
    
    if ((ret = avcodec_parameters_to_context(codec_ctx, codec_params)) < 0) {
        LOGE("Failed to copy codec parameters: %s", av_err2str(ret));
        avformat_close_input(&in_ctx);
        avcodec_free_context(&codec_ctx);
        return ret;
    }
    
    // 6. 打开解码器
    if ((ret = avcodec_open2(codec_ctx, decoder, nullptr)) < 0) {
        LOGE("Failed to open codec: %s", av_err2str(ret));
        avformat_close_input(&in_ctx);
        avcodec_free_context(&codec_ctx);
        return ret;
    }
    
    // 7. 设置重采样器 (转换为PCM格式) - 修复部分开始
    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        LOGE("Failed to allocate resampler");
        avformat_close_input(&in_ctx);
        avcodec_free_context(&codec_ctx);
        return AVERROR(ENOMEM);
    }

    // 检查输入通道布局，如果未设置则使用默认布局
    int64_t in_channel_layout = codec_ctx->channel_layout;
    if (in_channel_layout == 0) {
        // 使用通道数创建默认布局
        if (codec_ctx->channels > 0) {
            in_channel_layout = av_get_default_channel_layout(codec_ctx->channels);
            LOGD("Using default channel layout for %d channels", codec_ctx->channels);
        } else {
            LOGE("No channel count information available");
            avformat_close_input(&in_ctx);
            avcodec_free_context(&codec_ctx);
            swr_free(&swr_ctx);
            return AVERROR(EINVAL);
        }
    }

    // 设置重采样参数
    av_opt_set_int(swr_ctx, "in_channel_layout", in_channel_layout, 0);
    av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    // 初始化重采样器
    if ((ret = swr_init(swr_ctx)) < 0) {
        LOGE("Failed to initialize resampler: %s", av_err2str(ret));
        avformat_close_input(&in_ctx);
        avcodec_free_context(&codec_ctx);
        swr_free(&swr_ctx);
        return ret;
    }
    // 修复部分结束
    
    // 8. 打开输出文件
    out_file = fopen(output_path, "wb");
    if (!out_file) {
        LOGE("Failed to open output file");
        avformat_close_input(&in_ctx);
        avcodec_free_context(&codec_ctx);
        swr_free(&swr_ctx);
        return -1;
    }
    
    // 写入临时WAV文件头
    write_wav_header(out_file, 0, out_sample_rate, out_channels);
    
    // 9. 分配帧和包
    frame = av_frame_alloc();
    if (!frame) {
        LOGE("Failed to allocate frame");
        ret = AVERROR(ENOMEM);
        goto final_cleanup;
    }
    
    pkt = av_packet_alloc();
    if (!pkt) {
        LOGE("Failed to allocate packet");
        ret = AVERROR(ENOMEM);
        goto final_cleanup;
    }
    
    // 10. 读取并处理音频帧
    while (av_read_frame(in_ctx, pkt) >= 0 && !g_cancel) {
        if (pkt->stream_index != audio_stream_index) {
            av_packet_unref(pkt);
            continue;
        }
        
        // 发送数据包到解码器
        if ((ret = avcodec_send_packet(codec_ctx, pkt)) < 0) {
            LOGE("Error sending packet: %s", av_err2str(ret));
            av_packet_unref(pkt);
            continue;
        }
        
        // 接收解码后的帧
        while (true) {
            ret = avcodec_receive_frame(codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                LOGE("Error receiving frame: %s", av_err2str(ret));
                break;
            }
            
            // 计算当前时长
            int64_t current_ms = total_samples * 1000 / out_sample_rate;
            if (current_ms >= max_duration_ms) {
                g_progress = 100;
                av_frame_unref(frame);
                break;
            }
            
            // 更新进度
            pthread_mutex_lock(&g_mutex);
            g_progress = (int)(current_ms * 100 / max_duration_ms);
            pthread_mutex_unlock(&g_mutex);
            
            // 重采样
            int out_samples = swr_get_out_samples(swr_ctx, frame->nb_samples);
            int buffer_size = av_samples_get_buffer_size(
                nullptr, out_channels, out_samples, AV_SAMPLE_FMT_S16, 0);
            
            uint8_t* out_buffer = static_cast<uint8_t*>(av_malloc(buffer_size));
            if (!out_buffer) {
                LOGE("Failed to allocate output buffer");
                av_frame_unref(frame);
                break;
            }
            
            int converted = swr_convert(swr_ctx, &out_buffer, out_samples, 
                                       (const uint8_t**)frame->data, frame->nb_samples);
            
            if (converted < 0) {
                LOGE("Error during conversion: %s", av_err2str(converted));
                av_free(out_buffer);
                av_frame_unref(frame);
                break;
            }
            
            // 写入PCM数据
            fwrite(out_buffer, 1, converted * out_channels * 2, out_file);
            total_samples += converted;
            
            av_free(out_buffer);
            av_frame_unref(frame);
        }
        
        av_packet_unref(pkt);
    }
    
    // 11. 更新WAV文件头
    fseek(out_file, 0, SEEK_SET);
    write_wav_header(out_file, total_samples * out_channels * 2, out_sample_rate, out_channels);
    
final_cleanup:
    // 12. 清理资源
    if (pkt) av_packet_free(&pkt);
    if (frame) av_frame_free(&frame);
    if (out_file) {
        fclose(out_file);
    }
    if (in_ctx) avformat_close_input(&in_ctx);
    if (codec_ctx) avcodec_free_context(&codec_ctx);
    if (swr_ctx) swr_free(&swr_ctx);
	in_ctx = nullptr;
    
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_musicpad_ffmpeg_FFmpegExecutor_convertMp4ToWav(
    JNIEnv* env,
    jobject /* this */,
    jstring input_jstr,
    jstring output_jstr) {
    
    const char* input_path = env->GetStringUTFChars(input_jstr, 0);
    const char* output_path = env->GetStringUTFChars(output_jstr, 0);
    
    // 重置状态
    g_progress = 0;
    g_cancel = false;
    
    // 使用FFmpeg API直接转换
    int ret = convert_mp4_to_wav(input_path, output_path);
    
    env->ReleaseStringUTFChars(input_jstr, input_path);
    env->ReleaseStringUTFChars(output_jstr, output_path);
    
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_musicpad_ffmpeg_FFmpegExecutor_getProgress(
    JNIEnv* env,
    jobject /* this */) {
    
    pthread_mutex_lock(&g_mutex);
    int progress = g_progress;
    pthread_mutex_unlock(&g_mutex);
    
    return progress;
}

extern "C" JNIEXPORT void JNICALL
Java_com_unisound_musicpad_ffmpeg_FFmpegExecutor_cancelConversion(
    JNIEnv* env,
    jobject /* this */) {
    
    g_cancel = true;
}

//extern "C" JNIEXPORT jstring JNICALL
//Java_com_unisound_musicpad_ffmpeg_FFmpegExecutor_getFFmpegInfo(
//    JNIEnv* env,
//    jobject /* this */) {
//    
//    std::string info;
//    
//    // 获取版本信息
//    info += "FFmpeg version: ";
//    info += av_version_info();
//    info += "\n";
//    
//    // 获取支持的格式
//    info += "Supported formats:\n";
//    AVInputFormat *ifmt = nullptr;
//    while ((ifmt = av_demuxer_iterate((void**)&ifmt))) {
//        info += " - ";
//        info += ifmt->name;
//        info += " (";
//        info += ifmt->long_name;
//        info += ")\n";
//    }
//    
//    // 检查 MP4 支持
//    AVInputFormat *mp4_demuxer = av_find_input_format("mp4");
//    if (mp4_demuxer) {
//        info += "\nMP4 demuxer available: YES\n";
//    } else {
//        info += "\nMP4 demuxer available: NO\n";
//    }
//	
//	LOGD("info: %s", info.c_str());
//    
//    return env->NewStringUTF(info.c_str());
//}