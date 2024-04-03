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

#endif //_LOG_UTILS_H_