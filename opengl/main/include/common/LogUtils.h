#include "android/log.h"
#include "sys/time.h"

#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_ 1


#ifndef LOG_UTILS_TAG
#define LOG_UTILS_TAG "default"
#endif //LOG_UTILS_TAG

#define CarLogE(...) \
        __android_log_print(ANDROID_LOG_ERROR, LOG_UTILS_TAG, __VA_ARGS__)

#define CarLogD(...)  \
        __android_log_print(ANDROID_LOG_DEBUG, LOG_UTILS_TAG, __VA_ARGS__)

#define CarLogI(...)  \
        __android_log_print(ANDROID_LOG_INFO, LOG_UTILS_TAG, __VA_ARGS__)

#define CarLogW(...)  \
        __android_log_print(ANDROID_LOG_WARN, LOG_UTILS_TAG, __VA_ARGS__)

#define LOGE CarLogE
#define LOGD CarLogD
#define LOGI CarLogI
#define LOGW CarLogW

#define FUN_BEGIN_TIME(FUN) {\
    LOGI("%s func start", FUN); \
    long long t0 = getSysCurrentTime();

#define FUN_END_TIME(FUN) \
    long long t1 = getSysCurrentTime(); \
    LOGI("%s func cost time %ldms", FUN, (long)(t1-t0));}

#define BEGIN_TIME(FUN) {\
    LOGI("%s func start", FUN); \
    long long t0 = getSysCurrentTime();

#define END_TIME(FUN) \
    long long t1 = getSysCurrentTime(); \
    LOGI("%s func cost time %ldms", FUN, (long)(t1-t0));}

static long long getSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}

#define GO_CHECK_GL_ERROR(...)   LOGE("CHECK_GL_ERROR %s glGetError = %d, line = %d, ",  __FUNCTION__, glGetError(), __LINE__)

#endif //_LOG_UTILS_H_