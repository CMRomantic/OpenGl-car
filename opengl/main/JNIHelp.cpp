

#if defined(__ANDROID__)
/* libnativehelper is built by NDK 19 in one variant, which doesn't yet have the GNU strerror_r. */
#undef _GNU_SOURCE
#endif

#include "jni.h"
#include "JNIHelp.h"
#include "car_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <string>

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "JNIOnload"

extern "C" int jniRegisterNativeMethods(JNIEnv* env, const char* className,
    const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    LOGD("%s, success\n", __func__);
    return JNI_TRUE;
}