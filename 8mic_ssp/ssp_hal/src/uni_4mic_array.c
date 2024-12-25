#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <tinyalsa/asoundlib.h>
#include "uni_4mic_hal.h"
#include "uni_4mic_array.h"


#define _4MIC_LINEAR
//#define _4MIC_CYCLE
#ifdef _4MIC_LINEAR
#include "ual_fixed_direct_enhance.h"
#endif

#ifdef _4MIC_CYCLE
#include "UniMicArray.h"
#endif


static int linear_4mic = 0;


static int _type_convert(int linear, int type)
{
    switch(type){
    case UNI_HAL_FDE_AEC_ON:
        return UAL_FDE_AEC_ON;
        break;

    case UNI_HAL_FDE_ENV_ON:
        return UAL_FDE_ENV_ON;
        break;

    case UNI_FDE_FBF_ON:
        return UAL_FDE_FBF_ON;
        break;

    case UNI_FDE_ECHO_NUM:
        return UAL_FDE_ECHO_NUM;
        break;

    case UNI_FDE_MIC_NUM:
        return UAL_FDE_MIC_NUM;
        break;

    case UNI_FDE_AEC_FILTER_NUM:
        return UAL_FDE_AEC_FILTER_NUM;
        break;

    case UNI_FDE_AEC_NSHIFT:
        return UAL_FDE_AEC_NSHIFT;
        break;
        
    case UNI_FDE_DOA:
        return UAL_FDE_DOA;
        break;

    }
  
    return 16;
}

void *uni_hal_4mic_array_init(int linear, const char* cfg_path)
{ 
    linear_4mic = linear;

    HAL_PRINT_INFO("[%s:%d] linear=%d\n.", __func__, __LINE__, linear);
    if(linear){
#ifdef _4MIC_LINEAR
        return ual_fixed_direct_init(cfg_path);
#endif
    } else {
#ifdef _4MIC_CYCLE
        return Unisound_MicArray_Init(cfg_path);
#endif
    }
    return NULL;
}

int uni_hal_4mic_array_process(void* handle, const short* in, int in_len, short* echo_ref, int is_waked, short** out_asr, short** out_vad, int* out_len)
{
    if(linear_4mic){
#ifdef _4MIC_LINEAR
       return ual_fixed_direct_process(handle, in, echo_ref, in_len, out_asr, out_len);
#endif
    } else {
#ifdef _4MIC_CYCLE
       // return Unisound_MicArray_Process(handle, in, in_len, echo_ref, is_waked, out_asr, out_vad, out_len);
#endif
    } 
}

int uni_hal_4mic_array_compute_DOA(void* handle, float time_len, float time_delay)
{
    if(linear_4mic){
#ifdef _4MIC_LINEAR
       return ual_fixed_direct_compute_doa(handle, time_len, time_delay);
#endif
    } else {
#ifdef _4MIC_CYCLE
      //  return Unisound_MicArray_Compute_DOA(handle, time_len, time_delay);
#endif
    } 
}

void uni_hal_4mic_array_reset(void* handle)
{
    if(linear_4mic){
#ifdef _4MIC_LINEAR
        ual_fixed_direct_reset(handle);
#endif
    } else {
#ifdef _4MIC_CYCLE
      //  Unisound_MicArray_Reset(handle);
#endif
    }
}

void uni_hal_4mic_array_get(void* handle, int type, void* value)
{
    int _type = _type_convert(linear_4mic, type);
    //  HAL_PRINT_INFO("[%s:%d] _type=%d\n.", __func__, __LINE__, _type);
    ual_fixed_direct_get(handle, _type, value);
}

void uni_hal_4mic_array_set(void* handle, int type, void* value)
{
    int _type = _type_convert(linear_4mic, type);

    HAL_PRINT_INFO("[%s:%d] _type=%d\n.", __func__, __LINE__, _type);

    ual_fixed_direct_set(handle, _type, value);
}

void uni_hal_4mic_array_release(void* handle)
{
    ual_fixed_direct_release(handle);
}

const char* uni_hal_4mic_array_version()
{
    return ual_fixed_direct_version();
}

void uni_hal_4mic_array_set_org(void* handle, int type, void* value)
{
    int _type = type;

    HAL_PRINT_INFO("[%s:%d] _type=%d\n.", __func__, __LINE__, _type);

    ual_fixed_direct_set(handle, _type, value);
}

void uni_hal_4mic_array_get_org(void* handle, int type, void* value)
{
    int _type = type;
    //  HAL_PRINT_INFO("[%s:%d] _type=%d\n.", __func__, __LINE__, _type);
    ual_fixed_direct_get(handle, _type, value);
}
