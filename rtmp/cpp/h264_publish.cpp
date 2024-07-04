
#include <libyuv.h>
#include "logger.h"
#include "h264_publish.h"

void H264Publisher::InitPublish(const char *outputPath, int width, int height) {
    this->outputPath = outputPath;
    this->width = width;
    this->height = height;
}

int H264Publisher::EncodeFrame(AVCodecContext *pCodecCtx, AVFrame *pFrame, AVPacket *avPacket) {
    int ret = avcodec_send_frame(pCodecCtx, pFrame);
    if (ret < 0) {
        LOGE("Failed to send frame for encoding: %s", av_err2str(ret));
        return -1;
    }

    while (!ret) {
        ret = avcodec_receive_packet(pCodecCtx, avPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // No more packets available in the current context
            return 0;
        } else if (ret < 0) {
            LOGE("Error during encoding: %s", av_err2str(ret));
            return -1;
        }

        // Set packet presentation timestamp (PTS)
        avPacket->pts = av_rescale_q(avPacket->pts, pCodecCtx->time_base, pStream->time_base);
        avPacket->dts = avPacket->pts;
        avPacket->duration = av_rescale_q(avPacket->duration, pCodecCtx->time_base, pStream->time_base);
        avPacket->stream_index = pStream->index;

        /*LOGI("Send frame index:%d, pts:%lld, dts:%lld, duration:%lld, time_base:%d/%d",
             index,
             (long long)avPacket->pts,
             (long long)avPacket->dts,
             (long long)avPacket->duration,
             pStream->time_base.num, pStream->time_base.den);*/

        ret = av_interleaved_write_frame(out_fmt, avPacket);
        if (ret < 0) {
            LOGE("av_interleaved_write_frame failed: %s", av_err2str(ret));
            if (ret == AVERROR(EPIPE)) {
                // Close existing connection
                avio_close(out_fmt->pb);

                // Re-open the connection
                if (avio_open(&out_fmt->pb, outputPath, AVIO_FLAG_READ_WRITE) < 0) {
                    LOGE("Failed to re-open output file!\n");
                    return -1;
                }
                
                // Write file header again
                int result = avformat_write_header(out_fmt, NULL);
                if (result < 0) {
                    LOGE("Error occurred when re-opening output URL %d", result);
                    return -1;
                }

                // Retry sending the packet after reconnect
                ret = av_interleaved_write_frame(out_fmt, avPacket);
                if (ret != 0) {
                    LOGE("Retry av_interleaved_write_frame failed: %s", av_err2str(ret));
                    return -1;
                }
            }
        }

        av_packet_unref(avPacket);
        index++;
    }
    return 0;
}

void H264Publisher::StartPublish() {

    //1.注册所有组件
    av_register_all();
    //2.初始化网络
    avformat_network_init();

    //3.初始化输出码流的AVFormatContext
    avformat_alloc_output_context2(&out_fmt, NULL, "flv", outputPath);
    if (!out_fmt) {
        LOGE("Failed to allocate output context for %s", outputPath);
        return;
    }
    LOGE("avformat_alloc_output_context2 outputPath:%s", outputPath);
    //4.查找H.264编码器
    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pCodec) {
        LOGE("avcodec not found!");
        return;
    }
    //5.分配编码器并设置参数
    pCodecCtx = avcodec_alloc_context3(pCodec);
    //编码器的ID,这里是H264编码器
    pCodecCtx->codec_id = pCodec->id;
    //编码器编码的数据类型
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    //像素的格式
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->framerate = (AVRational) {fps, 1};
    pCodecCtx->time_base = (AVRational) {1, fps};// 设置时间基准，这里表示帧率为30fps
    pCodecCtx->gop_size = 30;// 设置关键帧间隔
    pCodecCtx->max_b_frames = 0;// 设置最大B帧数量
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 50;
    pCodecCtx->bit_rate = 100 * 1024 * 8;// 设置比特率
    pCodecCtx->level = 41;
    pCodecCtx->refs = 1;
    pCodecCtx->qcompress = 0.6;

    if (out_fmt->oformat->flags & AVFMT_GLOBALHEADER) {
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    //H.264
    AVDictionary *opts = NULL;
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&opts, "preset", "superfast", 0);
        av_dict_set(&opts, "tune", "zerolatency", 0);
    }
    //6. 打开编码器
    int result = avcodec_open2(pCodecCtx, pCodec, &opts);
    if (result < 0) {
        LOGE("open encoder failed %d", result);
        return;
    }

    //7. 根据输入流创建一个输出流
    pStream = avformat_new_stream(out_fmt, pCodec);
    if (!pStream) {
        LOGE("Failed allocating output outputPath");
        return;
    }
    pStream->time_base.num = 1;
    pStream->time_base.den = fps;
    pStream->codecpar->codec_tag = 0;
    if (avcodec_parameters_from_context(pStream->codecpar, pCodecCtx) < 0) {
        LOGE("Failed av codec parameters_from_context");
        return;
    }

    //8.打开网络输出流
    if (avio_open(&out_fmt->pb, outputPath, AVIO_FLAG_READ_WRITE) < 0) {
        LOGE("Failed to open output file!\n");
        return;
    }
    //9.写文件头部
    result = avformat_write_header(out_fmt, NULL);
    if (result < 0) {
        LOGE("Error occurred when opening output URL %d", result);
        return;
    }

    //初始化帧
    pFrame = av_frame_alloc();
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;
    pFrame->format = pCodecCtx->pix_fmt;
    int bufferSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width,
                                              pCodecCtx->height, 1);
    pFrameBuffer = (uint8_t *) av_malloc(bufferSize);
    av_image_fill_arrays(pFrame->data, pFrame->linesize, pFrameBuffer, pCodecCtx->pix_fmt,
                         pCodecCtx->width, pCodecCtx->height, 1);

    //创建已编码帧
    av_new_packet(&avPacket, bufferSize * 3);

    //标记正在转换
    this->transform = true;
}

void H264Publisher::EncodeBuffer(unsigned char *yuv420Buffer) {
    uint8_t *i420_y = pFrameBuffer;
    uint8_t *i420_u = pFrameBuffer + width * height;
    uint8_t *i420_v = pFrameBuffer + width * height * 5 / 4;

    // RGB888 转 I420 
    //BGRAToI420 RGBAToI420  RGB24ToI420  ARGBToI420
    // libyuv::ARGBToI420(rgbBuffer, width * 3, 
    //                     i420_y, width, 
    //                     i420_u, (width + 1) / 2, 
    //                     i420_v, (width + 1) / 2, 
    //                     width, height);

    memcpy(i420_y, yuv420Buffer, width * height);
    memcpy(i420_u, yuv420Buffer + width * height, width * height / 4);
    memcpy(i420_v, yuv420Buffer + width * height * 5 / 4, width * height / 4);

    pFrame->data[0] = i420_y;
    pFrame->data[1] = i420_u;
    pFrame->data[2] = i420_v;

    // 设置帧属性
    pFrame->format = AV_PIX_FMT_YUV420P;
    pFrame->width = width;
    pFrame->height = height;

    //编码H.264
    EncodeFrame(pCodecCtx, pFrame, &avPacket);
}

void H264Publisher::StopPublish() {
    //标记转换结束
    this->transform = false;

    int result = EncodeFrame(pCodecCtx, NULL, &avPacket);
    if (result >= 0) {
        //10.封装文件尾
        av_write_trailer(out_fmt);
        //释放内存
        if (pCodecCtx != NULL) {
            avcodec_close(pCodecCtx);
            avcodec_free_context(&pCodecCtx);
            pCodecCtx = NULL;
        }
        if (pFrame != NULL) {
            av_free(pFrame);
            pFrame = NULL;
        }
        if (pFrameBuffer != NULL) {
            av_free(pFrameBuffer);
            pFrameBuffer = NULL;
        }
        if (out_fmt != NULL) {
            avio_close(out_fmt->pb);
            avformat_free_context(out_fmt);
            out_fmt = NULL;
        }
    }
}