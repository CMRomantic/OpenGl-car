#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <string.h>
#include "uni_4mic_hal.h"
#include "tinyalsa/asoundlib.h"
#include <iostream>
#include <thread>
#include <fstream>
#define LOG_TAG    "UnimicJni"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


JavaVM *g_VM = NULL;
jobject cur_thiz = NULL;

extern "C" JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    g_VM = jvm;
    result = JNI_VERSION_1_6;
    return result;
}


extern "C" JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM* vm, void* reserved)
{
    g_VM = NULL;
}

extern "C" int hal_mic_error_cb(int error){
    LOGI("UniMicHalJNI hal_mic_status_cb enter error:%d",error);
    return 0;
}


extern "C" int hal_mic_status_cb(int event){
    LOGI("UniMicHalJNI hal_mic_status_cb enter event:%d",event);
    //接收到的event值
    //需要回调上层
    //attach
    JNIEnv *env;
    int ret = g_VM->GetEnv((void**)&env,JNI_VERSION_1_6);
    bool isAttach = JNI_FALSE;
    if (ret == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
            return 0;
        }
        isAttach = JNI_TRUE;
    }

    //获取类的
    if(NULL == cur_thiz) {
         LOGI("UniMicHalJNI object not found.");
         if(isAttach == JNI_TRUE) {
             g_VM->DetachCurrentThread();
         }
         return 0;
    }
    //获取object 的class 类
    jclass currentClazz = env->GetObjectClass(cur_thiz);
    if(NULL == currentClazz){
        LOGE("UniMicHalJNI class not set to gclass.");
        if(isAttach == JNI_TRUE) {
             g_VM->DetachCurrentThread();
        }
        return 0;
    }
    jmethodID callMethodId = env->GetMethodID(currentClazz,"onCallbackError","(I)V");
    if (NULL == callMethodId) {
        LOGE("Unable to find method:onCallbackError(I)V");
        env->DeleteLocalRef(currentClazz);
        if(isAttach == JNI_TRUE) {
           g_VM->DetachCurrentThread();
        }
        return 0;
    }
    env->CallVoidMethod(cur_thiz,callMethodId,event);
    env->DeleteLocalRef(currentClazz);
    //detach
    if(isAttach == JNI_TRUE) {
        g_VM->DetachCurrentThread();
    }
    env = NULL;
    LOGI("UniMicHalJNI hal_mic_status_cb exit event:%d",event);
    return 0;
}


/* 4mic logic interface	*/
extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicWakeUpStatus(JNIEnv *env, jobject thiz, jint status){
    return set4MicWakeUpStatus(status);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicUtteranceTimeLen(JNIEnv *env, jobject thiz, jint length){
    return set4MicUtteranceTimeLen(length);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicDelayTime(JNIEnv *env, jobject thiz, jint delayTime){
    return set4MicDelayTime(delayTime);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_close4MicAlgorithm(JNIEnv *env, jobject thiz, jint status){
    return close4MicAlgorithm(status);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicOneShotStartLen(JNIEnv *env, jobject thiz, jint startTimeLen){
    return set4MicOneShotStartLen(startTimeLen);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicWakeupStartLen(JNIEnv *env, jobject thiz, jint startTimeLen){
    return set4MicWakeupStartLen(startTimeLen);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicOneShotReady(JNIEnv *env, jobject thiz, jint status){
    return set4MicOneShotReady(status);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_get4MicOneShotReady(JNIEnv *env, jobject thiz){
    return get4MicOneShotReady();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_get4MicDoaResult(JNIEnv *env, jobject thiz){
    return get4MicDoaResult();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_unisound_mic_UniMicHalJNI_get4MicBoardVersion(JNIEnv *env, jobject thiz){
    char *ver = get4MicBoardVersion();
    return env->NewStringUTF(ver);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_set4MicDebugMode(JNIEnv *env, jobject thiz, jint debugMode){
    return set4MicDebugMode(debugMode);
}


/* audio interface */
extern "C" JNIEXPORT jlong JNICALL
Java_com_unisound_mic_UniMicHalJNI_openAudioIn(JNIEnv *env, jobject thiz, jint ch_num){
    return (jlong)uni_4mic_pcm_open(ch_num);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_closeAudioIn(JNIEnv *env, jobject thiz, jlong handle){
    return uni_4mic_pcm_close((void *)handle);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_readData(JNIEnv *env, jobject thiz, jlong handle, jbyteArray buffer, jint size){
    int ret;
    char* buf = (char*)env->GetByteArrayElements(buffer, 0);
    ret = uni_4mic_pcm_read((void *)handle, buf, size);
    env->ReleaseByteArrayElements(buffer, (jbyte*) buf, 0);
    return ret;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_startRecorder(JNIEnv *env, jobject thiz, jlong handle){
    return uni_4mic_pcm_start((void *)handle);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_stopRecorder(JNIEnv *env, jobject thiz, jlong handle){
    return uni_4mic_pcm_stop((void *)handle);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_initHal(JNIEnv *env, jobject thiz, jstring path){
    const char* config_dir = env->GetStringUTFChars(path,0);
    if(strlen(config_dir) > 0){
        uni_set_configpath(config_dir);
    }else{
        env->ReleaseStringUTFChars(path,config_dir);
        return -1;
    }
    if(NULL == cur_thiz){
        cur_thiz = env->NewGlobalRef(thiz);
    }
    env->ReleaseStringUTFChars(path,config_dir);
    int ret = uni_michal_event_register_callback(hal_mic_error_cb);
    return uni_4mic_hal_init(1);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_unisound_mic_UniMicHalJNI_releaseHal(JNIEnv *env, jobject thiz){
    if(NULL != cur_thiz){
        env->DeleteGlobalRef(cur_thiz);
        cur_thiz = NULL;
    }
    return uni_4mic_hal_release();
}

static struct pcm *pcm = nullptr;
static bool isRecording = false;

extern "C" JNIEXPORT void JNICALL
Java_com_unisound_mic_UniMicHalJNI_testTiny(JNIEnv *env, jobject thiz){
    struct pcm_config config;
    config.channels = 10;
    config.rate = 16000;
    config.period_size = 1024;
    config.period_count = 4;
    config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pcm = pcm_open(2, 0, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        LOGI("Unable to open PCM device (%s)", pcm_get_error(pcm));
        if (pcm) {
            pcm_close(pcm);
        }
        return;
    }
    LOGD("pcm_open configure");
    isRecording = true;

    //创建一个独立的线程进行录音
    std::thread([=]() {
        int16_t buffer[1024 * config.channels];

        // 打开输出文件，保存为二进制格式
        std::ofstream outFile("/sdcard/recorded_audio.pcm", std::ios::binary);
        if (!outFile.is_open()) {
            LOGI("Failed to open output file");
            isRecording = false;
            return;
        }

        while (isRecording) {
            if (pcm_read(pcm, buffer, sizeof(buffer)) < 0) {
                LOGI("Failed to read audio data");
                break;
            }
            // 将buffer写入文件
            outFile.write(reinterpret_cast<char*>(buffer), sizeof(buffer));
        }

        // 关闭文件和PCM设备
        outFile.close();
        pcm_close(pcm);
        pcm = nullptr;
        isRecording = false;
    }).detach();
}

