#include <jni.h>
#include <android/log.h>
#include <arm_neon.h>
#include <string.h>

#define LOG_TAG "NV21Converter"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void nv21ToRgbStandard(const uint8_t* nv21, uint8_t* rgb, int width, int height) {
    const int frameSize = width * height;
    const uint8_t* y = nv21;
    const uint8_t* vu = nv21 + frameSize;

    for (int yIndex = 0; yIndex < height; yIndex++) {
        for (int xIndex = 0; xIndex < width; xIndex++) {
            // Y 分量索引
            int yPos = yIndex * width + xIndex;
            int yVal = y[yPos] & 0xFF;

            // UV 分量索引（注意：NV21 是 VU 交错存储）
            int uvIndex = (yIndex / 2) * width + (xIndex / 2) * 2;
            int vVal = vu[uvIndex] & 0xFF;     // V 分量
            int uVal = vu[uvIndex + 1] & 0xFF; // U 分量

            // YUV 到 RGB 转换（使用整数运算避免浮点）
            int c = yVal - 16;
            int d = uVal - 128;
            int e = vVal - 128;

            int r = (298 * c + 409 * e + 128) >> 8;
            int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
            int b = (298 * c + 516 * d + 128) >> 8;

            // 限制在 0-255 范围内
            r = (r < 0) ? 0 : (r > 255) ? 255 : r;
            g = (g < 0) ? 0 : (g > 255) ? 255 : g;
            b = (b < 0) ? 0 : (b > 255) ? 255 : b;

            // 存储 RGB 数据（注意：BGR 还是 RGB 取决于需求）
            int rgbIndex = (yIndex * width + xIndex) * 3;
            rgb[rgbIndex] = (uint8_t)r;     // R
            rgb[rgbIndex + 1] = (uint8_t)g; // G
            rgb[rgbIndex + 2] = (uint8_t)b; // B
        }
    }
}

// 修正的 NEON 版本
void nv21ToRgbNeon(const uint8_t* nv21, uint8_t* rgb, int width, int height) {
    const int frameSize = width * height;
    const uint8_t* yPtr = nv21;
    const uint8_t* vuPtr = nv21 + frameSize;

    // NEON 转换系数
    const int16x8_t yCoeff = vdupq_n_s16(298);
    const int16x8_t uCoeff = vdupq_n_s16(409);
    const int16x8_t vCoeff = vdupq_n_s16(516);
    const int16x8_t uCoeff2 = vdupq_n_s16(100);
    const int16x8_t vCoeff2 = vdupq_n_s16(208);
    const int16x8_t const16 = vdupq_n_s16(16);
    const int16x8_t const128 = vdupq_n_s16(128);
    const int16x8_t zero = vdupq_n_s16(0);
    const int16x8_t maxVal = vdupq_n_s16(255);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 8) {
            // 加载 Y 分量
            uint8x8_t yVal = vld1_u8(yPtr + y * width + x);

            // 计算 UV 索引并加载 UV 分量
            int uvX = x / 2;
            int uvY = y / 2;
            uint8x8_t vu = vld1_u8(vuPtr + uvY * width + uvX * 2);

            // 分离 V 和 U（NV21 是 VU 交错）
            uint8x8_t vVal = vdup_lane_u8(vu, 0); // V 分量
            uint8x8_t uVal = vdup_lane_u8(vu, 1); // U 分量

            // 转换为有符号16位
            int16x8_t y16 = vreinterpretq_s16_u16(vmovl_u8(yVal));
            int16x8_t u16 = vreinterpretq_s16_u16(vmovl_u8(uVal));
            int16x8_t v16 = vreinterpretq_s16_u16(vmovl_u8(vVal));

            // 减去偏移量
            y16 = vsubq_s16(y16, const16);
            u16 = vsubq_s16(u16, const128);
            v16 = vsubq_s16(v16, const128);

            // 计算 R = Y + 1.402 * V
            int16x8_t r = vmlaq_s16(y16, v16, uCoeff);
            r = vshrq_n_s16(r, 8);

            // 计算 G = Y - 0.344 * U - 0.714 * V
            int16x8_t g = vmlaq_s16(y16, u16, uCoeff2);
            g = vmlsq_s16(g, v16, vCoeff2);
            g = vshrq_n_s16(g, 8);

            // 计算 B = Y + 1.772 * U
            int16x8_t b = vmlaq_s16(y16, u16, vCoeff);
            b = vshrq_n_s16(b, 8);

            // 限制范围 0-255
            r = vmaxq_s16(zero, vminq_s16(maxVal, r));
            g = vmaxq_s16(zero, vminq_s16(maxVal, g));
            b = vmaxq_s16(zero, vminq_s16(maxVal, b));

            // 转换为8位
            uint8x8_t r8 = vqmovun_s16(r);
            uint8x8_t g8 = vqmovun_s16(g);
            uint8x8_t b8 = vqmovun_s16(b);

            // 交错存储 RGB
            uint8x8x3_t rgbChunk;
            rgbChunk.val[0] = r8;
            rgbChunk.val[1] = g8;
            rgbChunk.val[2] = b8;

            vst3_u8(rgb + (y * width + x) * 3, rgbChunk);
        }
    }
}

// 安全获取 Image 的 Plane 数据
typedef struct {
    uint8_t* y;
    uint8_t* u;
    uint8_t* v;
    int32_t yStride;
    int32_t uvStride;
    int32_t uvPixelStride;
    int32_t width;
    int32_t height;
} YUVData;

YUVData getYUVData(JNIEnv* env, jobject image) {
    YUVData data = {0};

    // 获取 Image 的 Planes
    jclass imageClass = env->GetObjectClass(image);
    jmethodID getPlanes = env->GetMethodID(imageClass, "getPlanes", "()[Landroid/media/Image$Plane;");
    jobjectArray planes = (jobjectArray)env->CallObjectMethod(image, getPlanes);

    // 获取 Y 分量信息
    jobject yPlane = env->GetObjectArrayElement(planes, 0);
    jclass planeClass = env->GetObjectClass(yPlane);
    jmethodID getBuffer = env->GetMethodID(planeClass, "getBuffer", "()Ljava/nio/ByteBuffer;");
    jobject yBuffer = env->CallObjectMethod(yPlane, getBuffer);
    data.y = static_cast<uint8_t*>(env->GetDirectBufferAddress(yBuffer));

    // 获取 U/V 分量信息
    jobject uPlane = env->GetObjectArrayElement(planes, 1);
    jobject vPlane = env->GetObjectArrayElement(planes, 2);
    jobject uBuffer = env->CallObjectMethod(uPlane, getBuffer);
    jobject vBuffer = env->CallObjectMethod(vPlane, getBuffer);
    data.u = static_cast<uint8_t*>(env->GetDirectBufferAddress(uBuffer));
    data.v = static_cast<uint8_t*>(env->GetDirectBufferAddress(vBuffer));

    // 获取图像参数
    jmethodID getWidth = env->GetMethodID(imageClass, "getWidth", "()I");
    jmethodID getHeight = env->GetMethodID(imageClass, "getHeight", "()I");
    data.width = env->CallIntMethod(image, getWidth);
    data.height = env->CallIntMethod(image, getHeight);

    // 获取 Stride 和 PixelStride
    jmethodID getRowStride = env->GetMethodID(planeClass, "getRowStride", "()I");
    jmethodID getPixelStride = env->GetMethodID(planeClass, "getPixelStride", "()I");
    data.yStride = env->CallIntMethod(yPlane, getRowStride);
    data.uvStride = env->CallIntMethod(uPlane, getRowStride);
    data.uvPixelStride = env->CallIntMethod(uPlane, getPixelStride);

    return data;
}

// NEON 优化的 UV 处理
void processUV_NEON(uint8_t* dst, const uint8_t* uSrc, const uint8_t* vSrc, 
                   int width, int height, int uvStride, int uvPixelStride) {
    const int uvWidth = width / 2;
    const int uvHeight = height / 2;

    for (int y = 0; y < uvHeight; ++y) {
        const uint8_t* uRow = uSrc + y * uvStride;
        const uint8_t* vRow = vSrc + y * uvStride;
        uint8_t* uvDstRow = dst + y * width;

        int x = 0;
        // 每次处理 16 个像素
        for (; x <= uvWidth - 16; x += 16) {
            uint8x16_t u, v;
            if (uvPixelStride == 1) {
                u = vld1q_u8(uRow + x);
                v = vld1q_u8(vRow + x);
            } else {
                // 处理非连续存储（如 PixelStride=2）
                uint8x16x2_t uChunk = vld2q_u8(uRow + x * uvPixelStride);
                uint8x16x2_t vChunk = vld2q_u8(vRow + x * uvPixelStride);
                u = uChunk.val[0];
                v = vChunk.val[0];
            }

            uint8x16x2_t vu = {v, u};
            vst2q_u8(uvDstRow + x * 2, vu);
        }

        // 处理剩余像素
        for (; x < uvWidth; ++x) {
            uvDstRow[x * 2] = vRow[x * uvPixelStride];
            uvDstRow[x * 2 + 1] = uRow[x * uvPixelStride];
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_unisound_aik_FaceImage_convertToNV21Native(
    JNIEnv* env,
    jobject thiz,
    jobject image,
    jbyteArray nv21Array
) {
    // 1. 安全获取 YUV 数据
    YUVData yuv = getYUVData(env, image);
    if (!yuv.y || !yuv.u || !yuv.v) {
        LOGE("Invalid YUV planes!");
        return;
    }

    // 2. 获取目标数组指针
    jbyte* nv21 = env->GetByteArrayElements(nv21Array, nullptr);
    uint8_t* nv21Ptr = reinterpret_cast<uint8_t*>(nv21);

    // 3. 拷贝 Y 分量
    for (int y = 0; y < yuv.height; ++y) {
        memcpy(nv21Ptr + y * yuv.width, 
               yuv.y + y * yuv.yStride, 
               yuv.width);
    }

    // 4. 处理 UV 分量（NEON 优化）
    processUV_NEON(nv21Ptr + yuv.width * yuv.height,
                  yuv.u,
                  yuv.v,
                  yuv.width,
                  yuv.height,
                  yuv.uvStride,
                  yuv.uvPixelStride);

    // 5. 释放资源（不拷贝数据）
    env->ReleaseByteArrayElements(nv21Array, nv21, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_unisound_aik_FaceImage_convertNV21ToRgb(
        JNIEnv* env,
        jobject thiz,
        jbyteArray nv21Array,
        jbyteArray rgbArray
) {
    // 获取数组指针和长度
    jbyte* nv21 = env->GetByteArrayElements(nv21Array, nullptr);
    jbyte* rgb = env->GetByteArrayElements(rgbArray, nullptr);

    jsize nv21Len = env->GetArrayLength(nv21Array);
    jsize rgbLen = env->GetArrayLength(rgbArray);

    // 验证数组长度
    // NV21 长度应该是 width * height * 3 / 2
    // RGB 长度应该是 width * height * 3
    // 对于 640x480：
    // NV21: 640 * 480 * 3 / 2 = 460800
    // RGB: 640 * 480 * 3 = 921600

    if (nv21Len != 460800) {
        LOGE("Invalid NV21 array size: %d, expected: 460800", nv21Len);
        return ;
    }

    if (rgbLen != 921600) {
        LOGE("Invalid RGB array size: %d, expected: 921600", rgbLen);
        return ;
    }

    const int width = 640;
    const int height = 480;

    // 使用 NEON 优化版本
    nv21ToRgbStandard(
            reinterpret_cast<const uint8_t*>(nv21),
            reinterpret_cast<uint8_t*>(rgb),
            width,
            height
    );

    // 如果需要性能，可以切换到 NEON 版本
    // nv21ToRgbNeon(
    //         reinterpret_cast<const uint8_t*>(nv21),
    //         reinterpret_cast<uint8_t*>(rgb),
    //         width,
    //         height
    // );

    // 释放数组并提交更改
    env->ReleaseByteArrayElements(nv21Array, nv21, JNI_ABORT);
    env->ReleaseByteArrayElements(rgbArray, rgb, JNI_ABORT);
}