#include <cstring>
#include "jni.h"
#include "ISPLib.h"
#include "LogUtils.h"

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "MainApp"

void parseHandle(JNIEnv *env, jobject handle, io_handle_t *io_handle_t) {
    jclass ioHandleClass = env->GetObjectClass(handle);

    jfieldID devOpenFieldID = env->GetFieldID(ioHandleClass, "devOpen", "I");
    jfieldID bResendFlagFieldID = env->GetFieldID(ioHandleClass, "bResendFlag", "I");
    jfieldID usCheckSumFieldID = env->GetFieldID(ioHandleClass, "usCheckSum", "B");
    jfieldID uCmdIndexFieldID = env->GetFieldID(ioHandleClass, "uCmdIndex", "I");
    jfieldID bufferFieldID = env->GetFieldID(ioHandleClass, "buffer", "[B");
    // jfieldID devIoFieldID = env->GetFieldID( ioHandleClass, "devIo", "Lcom/example/jnitest/IOHandle$DEV_IO;");
    jfieldID mDevIoFieldID = env->GetFieldID(ioHandleClass, "mDevIo",
                                             "Lcom/aonions/opengl/jni/IoHandle$DevIo;");

    jclass devIoClass = env->FindClass("com/aonions/opengl/jni/IoHandle$DevIo");
    jmethodID initMethodID = env->GetMethodID(devIoClass, "init", "()V");
    jmethodID openMethodID = env->GetMethodID(devIoClass, "open", "()I");
    jmethodID closeMethodID = env->GetMethodID(devIoClass, "close", "()V");
    jmethodID writeMethodID = env->GetMethodID(devIoClass, "write", "(I[B)I");
    jmethodID readMethodID = env->GetMethodID(devIoClass, "read", "(I[B)I");

    jint devOpen = env->GetIntField(handle, devOpenFieldID);
    jint bResendFlag = env->GetIntField(handle, bResendFlagFieldID);
    jbyte usCheckSum = env->GetByteField(handle, usCheckSumFieldID);
    jint uCmdIndex = env->GetIntField(handle, uCmdIndexFieldID);
    jbyteArray bufferArray = (jbyteArray) env->GetObjectField(handle, bufferFieldID);
//    jobject devIoObject = env->GetObjectField(handle, devIoFieldID);
    jobject mDevIoObject = env->GetObjectField(handle, mDevIoFieldID);

    io_handle_t->dev_open = devOpen;
    io_handle_t->bResendFlag = bResendFlag;
    io_handle_t->m_usCheckSum = usCheckSum;
    io_handle_t->m_uCmdIndex = uCmdIndex;
}

void jintArrayToConfig(JNIEnv *env, jintArray jArray, unsigned int *config) {
    jsize length = env->GetArrayLength(jArray);

    jint *jInts = env->GetIntArrayElements(jArray, NULL);

    for (jsize i = 0; i < length; ++i) {
        config[i] = static_cast<unsigned int>(jInts[i]);
    }

    env->ReleaseIntArrayElements(jArray, jInts, 0);
}

extern "C"
JNIEXPORT jshort JNICALL
Java_com_aonions_opengl_jni_ISPJni_checksum(JNIEnv *env, jclass clazz, jbyteArray buffer,
                                            jint len) {
    short s = Checksum(reinterpret_cast<unsigned char *>(buffer), len);
    LOGD("checksum ---- %d", s);
    return s;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_aonions_opengl_jni_ISPJni_open(JNIEnv *env, jclass clazz, jobject handle) {
    io_handle_t ioHandle;
    //const char* buffer = reinterpret_cast<const char *>(bufferArray);

    //size_t bufferLength = strlen(buffer);
    //strncpy(reinterpret_cast<char *>(ioHandle.ac_buffer), buffer, bufferLength);

    //ioHandle.ac_buffer[65] = *reinterpret_cast<unsigned char *>(bufferArray);
    //ioHandle.m_dev_io = mDevIoObject;

    parseHandle(env, handle, &ioHandle);

    jint result = ISP_Open(&ioHandle);
    LOGD("isp open result:%d", result);
    return result;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_aonions_opengl_jni_ISPJni_close(JNIEnv *env, jclass clazz, jobject handle) {
    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);

    ISP_Close(&ioHandle);
    LOGD("isp close");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_aonions_opengl_jni_ISPJni_updateConfig(JNIEnv *env, jclass clazz, jobject handle,
                                                jintArray config, jintArray response) {

    jsize config_len = env->GetArrayLength(config);
    jsize response_len = env->GetArrayLength(response);

    unsigned int handle_config[config_len];
    unsigned int handle_response[response_len];
    jintArrayToConfig(env, config, handle_config);
    jintArrayToConfig(env, response, handle_response);

    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);

    ISP_UpdateConfig(&ioHandle, handle_config, handle_response);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_aonions_opengl_jni_ISPJni_readConfig(JNIEnv *env, jclass clazz, jobject handle,
                                              jintArray config) {
    jsize config_len = env->GetArrayLength(config);
    unsigned int handle_config[config_len];
    jintArrayToConfig(env, config, handle_config);

    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);

    ISP_ReadConfig(&ioHandle, handle_config);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_aonions_opengl_jni_ISPJni_syncPackNo(JNIEnv *env, jclass clazz, jobject handle) {
    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);
    ISP_SyncPackNo(&ioHandle);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_aonions_opengl_jni_ISPJni_connect(JNIEnv *env, jclass clazz, jobject handle,
                                           jint dw_milliseconds) {
    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);
    int result = ISP_Connect(&ioHandle, dw_milliseconds);
    LOGD("connect result:%d", result);
    return result;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_aonions_opengl_jni_ISPJni_resend(JNIEnv *env, jclass clazz, jobject handle) {
    io_handle_t ioHandle;
    parseHandle(env, handle, &ioHandle);
    return ISP_Resend(&ioHandle);
}