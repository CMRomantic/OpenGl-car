#ifndef _UNI_4MIC_HAL_H_
#define _UNI_4MIC_HAL_H_

#if defined(__cplusplus)
extern "C" {
#endif


#define UNUSED(_var)   (void)(_var)


/*#if defined(UNI_ANDROID)
#define UNI_4MIC_VERSION    "UNI_4MIC_HAL_ANDROID_V1.1"
#else
#error "OS platform not defined !"
#endif*/


/* DEBUG */
#if defined(UNI_ANDROID)
#define HAL_DBG_MODE_PRINTF                  (1 << 0)    
#define HAL_DBG_MODE_ALOGD                  (1 << 1)    
#define HAL_DBG_MODE_ALL                      (HAL_DBG_MODE_ALOGD | HAL_DBG_MODE_PRINTF)  
#define HAL_DBG_MODE_DEFAULT               HAL_DBG_MODE_ALOGD
#else
#define HAL_DBG_MODE_PRINTF                  (1 << 0)    
#define HAL_DBG_MODE_ALL                       (HAL_DBG_MODE_PRINTF)  
#define HAL_DBG_MODE_DEFAULT               HAL_DBG_MODE_PRINTF
#endif

#define HAL_DBG_LEVEL_MIN                     0    
#define HAL_DBG_LEVEL_ERR                     1    
#define HAL_DBG_LEVEL_WARN                  2    
#define HAL_DBG_LEVEL_NOTICE                3    
#define HAL_DBG_LEVEL_INFO                    4    
#define HAL_DBG_LEVEL_DEBUG                 5    
#define HAL_DBG_LEVEL_MAX                    6    
#define HAL_DBG_LEVEL_DEFAULT             HAL_DBG_LEVEL_DEBUG //HAL_DBG_LEVEL_ERR

#define HAL_PRINT_ERR(x ...)              uni_4mic_debug_print(HAL_DBG_LEVEL_ERR, "<ERR> " x)
#define HAL_PRINT_WARN(x ...)           uni_4mic_debug_print(HAL_DBG_LEVEL_WARN, "<WARN> " x)
#define HAL_PRINT_NOTICE(x ...)         uni_4mic_debug_print(HAL_DBG_LEVEL_NOTICE, "<NOTICE> " x)
#define HAL_PRINT_INFO(x ...)            uni_4mic_debug_print(HAL_DBG_LEVEL_INFO, "<INFO> " x)
#define HAL_PRINT_DEBUG(x ...)         uni_4mic_debug_print(HAL_DBG_LEVEL_DEBUG, "<DEBUG> " x)

struct env_4mic_cfg{
#ifdef DO_BOARD_INIT
#ifdef DO_BOARD_RESET
    int reset_gpio;
#endif   
    const char *i2c_dev;
#endif
    int pcm_card;
    int pcm_device;
    int i2s_bits;
    int linear_4mic;
    int no_vad;
    int high_priority;
    int raw_pcm;
   
    const char *mic_array_cfg_path;
    char mic_array_rw_path[128];

    const char *debug_file_4mic_name;
    const char *debug_file_2aec_name;
    const char *debug_file_out_name;
    char debug_file_path[128];
};
typedef int (*UNISOUND_EVENT_CALL_BACK)(int event);


/* hal interface */
extern void uni_set_configpath(const char* cfg_path);
extern int uni_4mic_hal_init(int use4Mic);
extern int uni_4mic_hal_release(void);
extern int uni_michal_event_register_callback(UNISOUND_EVENT_CALL_BACK callback);

/* 4mic logic interface */
extern int set4MicWakeUpStatus(int status);
extern int set4MicUtteranceTimeLen(int length);
extern int set4MicDelayTime(int delayTime);
extern int close4MicAlgorithm(int status);
extern int set4MicOneShotStartLen(int startTimeLen);
extern int set4MicWakeupStartLen(int startTimeLen);
extern int set4MicOneShotReady(int status);
extern int get4MicOneShotReady(void);
extern int get4MicDoaResult(void);
extern char *get4MicBoardVersion(void);
extern int set4MicDebugMode(int debugMode);
extern int set4Micarray_AEC_Status(int status);
extern int set_bias_angle(int status);
extern int set_enhance_angle(int status);

/* audio interface */
extern void* uni_4mic_pcm_open(int ch_num);
extern int uni_4mic_pcm_close(void *handle);
extern int uni_4mic_pcm_read(void *handle, char *buf, unsigned int size);
extern int uni_4mic_pcm_start(void *handle);
extern int uni_4mic_pcm_stop(void *handle);

/* debug interface */
extern int uni_4mic_set_debug_name(char *debug_name);
extern int uni_4mic_set_debug_level(int level);
extern int uni_4mic_set_debug_mode(int mode);
extern int uni_4mic_debug_print(int level, const char *fmt, ...);


#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* _UNI_4MIC_HAL_H_ */

