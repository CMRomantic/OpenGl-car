#define LOG_TAG "UNI_4MIC"
#define LOG_NDEBUG 0

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <stdarg.h>

#if defined(UNI_ANDROID)
#include <android/log.h>
#endif

#include "uni_4mic_hal.h"


#define TEXT_BUFFER_SIZE 512

static char print_buf[TEXT_BUFFER_SIZE];
static char s_debug_name[16] = "[UNI_4MIC]:";
static int s_debug_level = HAL_DBG_LEVEL_DEFAULT;
static int s_debug_mode = HAL_DBG_MODE_DEFAULT;




int uni_4mic_set_debug_name(char *debug_name)
{
    int max = sizeof(s_debug_name) - 1;
    int name_len;
    
    if(NULL == debug_name){
        printf("ERROR: %s, (NULL == debug_name)\n", __func__);
        return -1;
    }

    name_len = strlen(debug_name);
    if(name_len > max){
        printf("ERROR: %s, name_len=%d, max=%d\n", __func__, name_len, max);
        return -1;
    }

    memset(s_debug_name, 0, sizeof(s_debug_name));
    strncpy(s_debug_name, debug_name, max);
    
    return 0;
}

int uni_4mic_set_debug_level(int level)
{
    if(HAL_DBG_LEVEL_MIN >= level){
        level = HAL_DBG_LEVEL_MIN + 1;
    }
    
    if(HAL_DBG_LEVEL_MAX <= level){
        level = HAL_DBG_LEVEL_MAX - 1;
    }
    
    s_debug_level = level;
    printf("%s, debug_level=%d\n", __func__, level);

    return level;
}

int uni_4mic_set_debug_mode(int mode)
{
    if(mode && (HAL_DBG_MODE_ALL & mode) == mode){
        s_debug_mode = mode;
        return 0;
    }

    printf("ERROR: %s, mode=0x%x, mode_all_bits is 0x%x\n", __func__, mode, HAL_DBG_MODE_ALL);
    return -1;
}

int uni_4mic_debug_print(int level, const char *fmt, ...)
{
    va_list ap;
    int n;
    int maxlen = TEXT_BUFFER_SIZE - 1;
    char *buf = print_buf;

    if(s_debug_level < level){
        return -1;
    }
    
    va_start(ap, fmt);
    n = vsnprintf(buf, maxlen, fmt, ap);
    va_end(ap);
    
    if(n < 0 || n >= maxlen){
        n = maxlen;
    }
    
    //maxlen -= n;
    //snprintf(buf + n, maxlen, "\n");

    buf[n] = 0;

    if(HAL_DBG_MODE_PRINTF & s_debug_mode){
        printf("%s%s", s_debug_name, buf);
    }

#if defined(UNI_ANDROID)
    // if(HAL_DBG_MODE_ALOGD & s_debug_mode){
    //     ALOGD("%s%s", s_debug_name, buf);
    // }
    __android_log_print(ANDROID_LOG_DEBUG, "mic_hal", "%s%s", s_debug_name, buf);
#endif

    return 0;
}

