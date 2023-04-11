#include <android/log.h>

#ifndef _CAR_LOG_H_
#define _CAR_LOG_H_ 1


#ifndef CAR_LOG_TAG
#define CAR_LOG_TAG "default"
#endif //CAR_LOG_TAG

#define CAR_LOG_LEVEL_ERROR  1
#define CAR_LOG_LEVEL_DEBUG  2
#define CAR_LOG_LEVEL_INFO  3

#define CarLogE(...) \
        __android_log_print(ANDROID_LOG_ERROR, CAR_LOG_TAG, __VA_ARGS__)

#define CarLogD(...)  \
        __android_log_print(ANDROID_LOG_DEBUG, CAR_LOG_TAG, __VA_ARGS__)

#define CarLogI(...)  \
        __android_log_print(ANDROID_LOG_INFO, CAR_LOG_TAG, __VA_ARGS__)

#define CarLogW(...)  \
        __android_log_print(ANDROID_LOG_WARN, CAR_LOG_TAG, __VA_ARGS__)

#define LOGE CarLogE
#define LOGD CarLogD
#define LOGI CarLogI
#define LOGW CarLogW

#define RETIF_LOGE( condition, ... ) \
	if( condition ){ \
	    LOGE(__VA_ARGS__); \
		return; \
	}

#define RETIF_LOGD( condition, ... ) \
	if( condition ){ \
	    LOGD(__VA_ARGS__); \
		return; \
	}

#define RETNIF_LOGE( condition, n, ... ) \
	if( condition ){ \
	    LOGE(__VA_ARGS__); \
		return n; \
	}

#define RETNIF_LOGD( condition, n, ... ) \
	if( condition ){ \
	    LOGD(__VA_ARGS__); \
		return n; \
	}

#define BREAKIF_LOGE( condition, ... ) \
	if( condition ){ \
	    LOGE(__VA_ARGS__); \
		break; \
	}

#define BREAKIF_LOGD( condition, ... ) \
	if( condition ){ \
	    LOGD(__VA_ARGS__); \
		break; \
	}

#endif //_CAR_LOG_H_