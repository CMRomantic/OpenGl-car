#include <jni.h>
#include <android/log.h>
#include <arm_neon.h>
#include <string.h>

#define LOG_TAG "NV21Converter"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

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