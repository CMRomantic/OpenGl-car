#include <errno.h>
#ifndef _UNI_4MIC_ARRAY_H_
#define _UNI_4MIC_ARRAY_H_

#if defined(__cplusplus)
extern "C" {
#endif


#define UNI_HAL_MICARRAY_VERBOSE                         0x1001
#define UNI_HAL_MICARRAY_AEC                                 0x1002
#define UNI_HAL_MICARRAY_MCLP                               0x1003
#define UNI_HAL_MICARRAY_AGC                                 0x1004
#define UNI_HAL_MICARRAY_MICROPHONE_NUM           0x1005
#define UNI_HAL_MICARRAY_ECHO_NUM                       0x1006
#define UNI_HAL_MICARRAY_WAKE_UP_TIME_LEN         0x1007
#define UNI_HAL_MICARRAY_WAKE_UP_TIME_DELAY     0x1008
#define UNI_HAL_MICARRAY_ROBOT_FACE_DEGREE       0x1009
#define UNI_HAL_MICARRAY_DOA_START                      0x100A
#define UNI_HAL_MICARRAY_DOA_RESULT                     0x100B
#define UNI_HAL_MICARRAY_AEC_NSHIFT                     0x100C
#define UNI_HAL_MICARRAY_AEC_FILTER_NUM             0x100D


#define UNI_HAL_FDE_AEC_ON  0x2001
#define UNI_HAL_FDE_ENV_ON  0x2002
#define UNI_FDE_FBF_ON      0x2003
#define UNI_FDE_ECHO_NUM    0x2004
#define UNI_FDE_MIC_NUM     0x2005
#define UNI_FDE_AEC_FILTER_NUM  0x2006
#define UNI_FDE_AEC_NSHIFT      0x2007
#define UNI_FDE_DOA      0x2008


extern void *uni_hal_4mic_array_init(int linear, const char* cfg_path);
extern int uni_hal_4mic_array_process(void* handle, const short* in, int in_len, short* echo_ref, int is_waked, short** out_asr, short** out_vad, int* out_len);
extern int uni_hal_4mic_array_compute_DOA(void* handle, float time_len, float time_delay);
extern void uni_hal_4mic_array_reset(void* handle);
extern void uni_hal_4mic_array_get(void* handle, int type, void* value);
extern void uni_hal_4mic_array_set(void* handle, int type, void* value);
extern void uni_hal_4mic_array_release(void* handle);
extern void uni_hal_4mic_array_set_org(void* handle, int type, void* value);
extern void uni_hal_4mic_array_get_org(void* handle, int type, void* value);
extern const char* uni_hal_4mic_array_version();

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* _UNI_4MIC_ARRAY_H_ */

