#define _GNU_SOURCE

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

#include <errno.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <sys/resource.h>
#include <tinyalsa/asoundlib.h>

#include <sched.h>
#include <string.h>
#include <unistd.h>

#include "uni_4mic_hal.h"
#include "uni_4mic_ringBuf.h"
#include "uni_4mic_array.h"
#include "ual_fixed_direct_enhance.h"

int log_g_pcm = 0;
int log_g_pcm_af = 0;
int log_g_4mic = 0;
int log_g_4mic_af = 0;

int tmp_flag = 0;
int g_write_count = 0;
int g_write_count_raw = 0;
int g_debug_file_raw = 0;
int g_debug_file = 0;

int g_pcm_read_error = 0;

volatile int g_pcm_test_flag = 0;

int g_set_path = 0;
char g_path[1024] = {0};

typedef int (*GET_INVOKE_COUNT_CALLBACK)(); //get how many invoke

static GET_INVOKE_COUNT_CALLBACK g_invoke_conut_fun = NULL;

typedef int (*UNISOUND_EVENT_CALLBACK)(int event);

static UNISOUND_EVENT_CALLBACK g_unisound_event_callback = NULL;

#define ___DEBUG

#ifdef ___DEBUG
struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define FORMAT_PCM 1

pthread_mutex_t lock_uni;

FILE *file_all_read_noflag = NULL;
FILE *file_all_read = NULL;
FILE *file_all_read_raw = NULL;
FILE *file_aec_read = NULL;
FILE *file_mic_read = NULL;
FILE *file_out_read = NULL;
FILE *file_4mic = NULL;
FILE *file_2aec = NULL;
FILE *file_out = NULL;
FILE *file_8mic = NULL;
FILE *file_all_read_org = NULL;
FILE *file_2mic = NULL;
FILE *file_2aec_de = NULL;

uint32_t file_4mic_data_size = 0;
uint32_t file_2aec_data_size = 0;
uint32_t file_out_data_size = 0;
struct wav_header header_4mic;
struct wav_header header_2aec;
struct wav_header header_out;
#endif
int save_file_on = 0;

int top_aec_enlarge = 1;
int g_flag_codec_id = 0; //0: v1/v2, 1: v3
#define FILE_READ_TEST    0

#define ONESHOT_READY_FLAG    0xab01
#define SLOT_FRAMES                   256
#define SLOT_FRAMES_BYTES        (SLOT_FRAMES * 2)
#define SLOT_FRAMES_PACKED_BYTES (SLOT_FRAMES_BYTES * 2)
#define SLOT_FRAMES_ALL_RAW_BYTES (SLOT_FRAMES_BYTES * RAW_CHAN_NUM)

#define RECORD_BUF_CNT            1000
#define RECORD_BUF_SIZE           (SLOT_FRAMES * 2 * 4)
#define ECHO_BUF_SIZE               (SLOT_FRAMES * 2 * 2)
#define RECORD_BAK_BUF_SIZE    (SLOT_FRAMES * 2 * 2)
#define A_SLOT_DELAY                 (SLOT_FRAMES / 16) //in ms
#define PACKED_CHAN_NUM                       12
#define PACKED_FRAME_BYTES                  (PACKED_CHAN_NUM * 4)
#define RAW_CHAN_NUM                       10
#define RAW_FRAME_BYTES                  (RAW_CHAN_NUM * 2)

#if (PACKED_CHAN_NUM == 12)
#define PACKED_SAMPLE_RATE                   96000
#elif (PACKED_CHAN_NUM == 6)
#define PACKED_SAMPLE_RATE                   48000
#endif

#define RAW_SAMPLE_RATE        16000

#define RTTIME        0

#if RTTIME
struct timeval start, end;
double result_all=0;
int cnt=0;
int i;
unsigned long long num1;
double num21;
double result1;
double result;

double result2;
unsigned long long num11;
double num2;

double result3;
double rt;
#endif


struct uni_4mic_hal {
    struct env_4mic_cfg env_cfg;
    int use_4mic;
    int ch_num;
    int raw_pcm;
    struct pcm *pcm;
    pthread_t pcm_read_thread;
    pthread_t mic_array_thread;
    int read_start;
    int pcm_read_thread_running;
    int mic_array_thread_running;
    RingBuf_t RecordRingBuffer;
    RingBuf_t micArrayRingBuf;
    RingBuf_t RecordRingBuffer_org;

    void *MicHandle;
    int is_waked;
    int utteranceTime;
    int delayTime;
    int is_4mic_closed;
    int OneShotStartLen;
    int WakeupStartLen;
    int OneShotReady;
    int DoaResult;
    int debugMode;

    pthread_mutex_t lock;
};

static struct uni_4mic_hal *s_hal = NULL;


extern int uni_4mic_board_init(struct env_4mic_cfg *env_cfg);

extern int uni_4mic_board_deinit(struct env_4mic_cfg *env_cfg);

typedef int tUIPC_CH_ID;

typedef enum {
    UIPC_OPEN_EVT = 0x0001,
    UIPC_CLOSE_EVT = 0x0002,
    UIPC_RX_DATA_EVT = 0x0004,
    UIPC_RX_DATA_READY_EVT = 0x0008,
    UIPC_TX_DATA_READY_EVT = 0x0010
} tUIPC_EVENT;

static void uni_micipc_data_cb(tUIPC_CH_ID ch_id, tUIPC_EVENT event) {

    //ALOGE("uni_micipc_data_cb ");

    HAL_PRINT_ERR("uni_micipc_data_cb");

}

int uni_hal_invoke_count_register_callback(GET_INVOKE_COUNT_CALLBACK callback) {
    HAL_PRINT_INFO("uni_hal_invoke_count_register_callback\n");
    g_invoke_conut_fun = callback;
    return 0;
    //static GET_INVOKE_COUNT_CALLBACK g_invoke_conut_fun = NULL;
}

static int get_item_value(const char *item, char *line_str, int *val) {
    int n, line_n;
    int i;
    int tmp;

    line_n = strlen(line_str);
    n = strlen(item);
    if (strncmp(item, line_str, n)) {
        return -1;
    }

    //remove space/tab
    for (i = n; i < line_n; i++) {
        if (' ' == line_str[i] || '\t' == line_str[i]) {
            continue;
        }

        break;
    }

    if (n == i) { //no space/tab
        return -1;
    }
    if (line_n == i) { //no digit
        return -1;
    }

    n = i;
    if ('0' > line_str[n] || '9' < line_str[n]) { //not digit
        return -1;
    }

    *val = atoi(&line_str[n]);

    return 0;
}

static int get_item_string(const char *item, char *line_str, char **str) {
    int n, line_n;
    int i;
    int tmp;
    char *p;

    line_n = strlen(line_str);
    n = strlen(item);
    if (strncmp(item, line_str, n)) {
        return -1;
    }

    //remove space/tab
    for (i = n; i < line_n; i++) {
        if (' ' == line_str[i] || '\t' == line_str[i]) {
            continue;
        }

        break;
    }

    if (n == i) { //no space/tab
        return -1;
    }
    if (line_n == i) { //no string
        return -1;
    }

    n = i;
    *str = &line_str[n];
    p = &line_str[n];

    //remove '\r''\n'
    for (i = 0; p[i]; i++) {
        if ('\r' == p[i] || '\n' == p[i]) {
            p[i] = 0;
            break;
        }
    }

    return 0;
}

int uni_4mic_get_hw_param(struct env_4mic_cfg *env_cfg) {
    char filename[256];
    FILE *fp;
    char line_str[50 + 1];
    char *str = NULL;
    int ret;

    memset(filename, 0, sizeof(filename));

    if (g_set_path == 0) {

        memcpy(filename, env_cfg->mic_array_cfg_path, strlen(env_cfg->mic_array_cfg_path));
        strcat(filename, "/pcm_hw_config.txt");
        HAL_PRINT_INFO("g_set_path==0\n");
    }

    if (g_set_path == 1) {

        memcpy(filename, g_path, strlen(g_path));
        strcat(filename, "/pcm_hw_config.txt");

        HAL_PRINT_INFO("g_set_path==1 g_path %s\n", g_path);

    }


    fp = fopen(filename, "r");
    if (NULL == fp) {
        HAL_PRINT_ERR("failed to open %s, %s", filename, strerror(errno));
        return -1;
    }

    while (!feof(fp)) {
        memset(line_str, 0, sizeof(line_str));
        fgets(line_str, 50, fp);

        ret = get_item_value("pcm_card", line_str, &env_cfg->pcm_card);
        if (0 == ret) {
            HAL_PRINT_INFO("pcm_card=%d\n", env_cfg->pcm_card);
            continue;
        }

        ret = get_item_value("pcm_device", line_str, &env_cfg->pcm_device);
        if (0 == ret) {
            HAL_PRINT_INFO("pcm_device=%d\n", env_cfg->pcm_device);
            continue;
        }

        ret = get_item_value("i2s_bits", line_str, &env_cfg->i2s_bits);
        if (0 == ret) {
            HAL_PRINT_INFO("i2s_bits=%d\n", env_cfg->i2s_bits);
            continue;
        }

        ret = get_item_value("linear_4mic", line_str, &env_cfg->linear_4mic);
        if (0 == ret) {
            HAL_PRINT_INFO("linear_4mic=%d\n", env_cfg->linear_4mic);
            continue;
        }

        ret = get_item_value("no_vad", line_str, &env_cfg->no_vad);
        if (0 == ret) {
            HAL_PRINT_INFO("no_vad=%d\n", env_cfg->no_vad);
            continue;
        }

        ret = get_item_value("aec_enlarge_4mic", line_str, &top_aec_enlarge);
        if (0 == ret) {
            HAL_PRINT_INFO("aec_enlarge_4mic=%d\n", top_aec_enlarge);
            continue;
        }
        ret = get_item_value("save_file_on", line_str, &save_file_on);
        HAL_PRINT_ERR("save_file_on=%d\n", save_file_on);
        if (0 == ret) {
            HAL_PRINT_ERR("save_file_on=%d\n", save_file_on);
            continue;
        }

        ret = get_item_value("high_priority", line_str, &env_cfg->high_priority);
        if (0 == ret) {
            HAL_PRINT_INFO("high_priority=%d\n", env_cfg->high_priority);
            continue;
        }

        ret = get_item_value("raw_pcm", line_str, &env_cfg->raw_pcm);
        if (0 == ret) {
            HAL_PRINT_INFO("raw_pcm=%d\n", env_cfg->raw_pcm);
            continue;
        }

        ret = get_item_string("mic_array_rw_path", line_str, &str);
        if (0 == ret) {
            memset(env_cfg->mic_array_rw_path, 0, sizeof(env_cfg->mic_array_rw_path));
            strncpy(env_cfg->mic_array_rw_path, str, sizeof(env_cfg->mic_array_rw_path));
            HAL_PRINT_INFO("mic_array_rw_path=%s\n", env_cfg->mic_array_rw_path);
            continue;
        }

        ret = get_item_string("debug_file_path", line_str, &str);
        if (0 == ret) {
            memset(env_cfg->debug_file_path, 0, sizeof(env_cfg->debug_file_path));
            strncpy(env_cfg->debug_file_path, str, sizeof(env_cfg->debug_file_path));
            HAL_PRINT_INFO("debug_file_path=%s\n", env_cfg->debug_file_path);
            continue;
        }

    }

    fclose(fp);
    return 0;
}

int uni_4mic_hal_init(int use4Mic) {
    int ret;
    struct uni_4mic_hal *hal = s_hal;
    const char *ver;
    int val;
    int board_init = 0;
    char *cfg_path;
    char *spath;

    int aec_on = 0, env_on = 0, fbf_on = 1;
    int enhance_angle, bias_angle;
    float time_protect;
    float tmp;
    // UIPC_Init(NULL);
    // uni_ipc_4mic_ipc_server();
    // UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
    //  UIPC_Open("/data/misc/audioserver/.uni_mic_data", uni_micipc_data_cb);
    if (hal) {
        HAL_PRINT_ERR("%s: hal is already inited.\n", __func__);
        return -1;
    }

    pthread_mutex_init(&lock_uni, NULL);

    HAL_PRINT_INFO("UNI_4MIC_VERSION: %s_20230818_10ch_16bit_release_noflag_v3\n",
                   UNI_4MIC_VERSION);

    hal = (struct uni_4mic_hal *) calloc(sizeof(struct uni_4mic_hal), 1);
    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->use_4mic = 1;
    HAL_PRINT_INFO("use_4mic=%d.\n", hal->use_4mic);

    do {
        ret = uni_4mic_board_init(&hal->env_cfg);
        if (0 > ret) {
            HAL_PRINT_ERR("%s: uni_4mic_board_init failed\n", __func__);
            break;
        }
        board_init = 1;

        uni_4mic_get_hw_param(&hal->env_cfg);
        hal->raw_pcm = hal->env_cfg.raw_pcm;

        if (hal->use_4mic) {
            // HAL_PRINT_INFO("mic_array_cfg_path=%s\n", hal->env_cfg.mic_array_cfg_path);
            // HAL_PRINT_INFO("mic_array_rw_path=%s\n", hal->env_cfg.mic_array_rw_path);
            cfg_path = calloc(512, 1);
            if (NULL == cfg_path) {
                HAL_PRINT_ERR("(NULL == cfg_path)\n");
                break;
            }

            if (g_set_path == 0) {
                // sprintf(cfg_path, "%s;%s", hal->env_cfg.mic_array_cfg_path, hal->env_cfg.mic_array_rw_path);
                //  HAL_PRINT_INFO("cfg_path=%s\n", cfg_path);
                spath = "/system/usr/uni_4mic_config";
                strcpy(cfg_path, spath);
                HAL_PRINT_INFO("230823g_set_path==0 cfg_path=%s\n", cfg_path);

            }

            if (g_set_path == 1) {
                //  sprintf(cfg_path, "%s;%s", g_path, "/sdcard");
                // cfg_path=g_path;

                strcpy(cfg_path, g_path);
                HAL_PRINT_INFO("g_set_path==1 cfg_path=%s\n", g_path);
                ///system/usr/uni_4mic_config;/sdcard
            }

            hal->MicHandle = uni_hal_4mic_array_init(hal->env_cfg.linear_4mic, cfg_path);
            free(cfg_path);
            if (hal->MicHandle) {
                HAL_PRINT_INFO("uni_hal_4mic_array_init sucess \n");
            } else {
                HAL_PRINT_ERR("uni_hal_4mic_array_init error \n");
                break;
            }

            ver = uni_hal_4mic_array_version();
            HAL_PRINT_INFO("uni_hal_4mic_array_version=%s\n", ver);

            aec_on = 1;
            fbf_on = 1;
            enhance_angle = 0;
            bias_angle = 25;
            time_protect = 0.5;


            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_AEC_ON, &aec_on);
            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_ENV_ON, &env_on);
            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_FBF_ON, &fbf_on);
            // ������ǿ�Ƕȣ�Ĭ��ֵΪ�����ļ��еĳ�ʼ����EnhanceAngle
            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_ENHANCE_ANGLE, &enhance_angle);
            // ������ǿ�Ƕȷ�Χ��Ĭ��ֵΪ�����ļ��еĳ�ʼ����BiasAngle
            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_BIAS_ANGLE, &bias_angle);
            ual_fixed_direct_get(hal->MicHandle, UAL_FDE_PROTECT_TIME, &tmp);
            //	  printf("time :%f\n", tmp);
            ual_fixed_direct_set(hal->MicHandle, UAL_FDE_PROTECT_TIME, &time_protect);
            ual_fixed_direct_get(hal->MicHandle, UAL_FDE_PROTECT_TIME, &tmp);
            //	  printf("time :%f\n", tmp);

            ual_fixed_direct_get(hal->MicHandle, UAL_FDE_BIAS_ANGLE, &bias_angle);
            HAL_PRINT_INFO("===============bias angle = %d\n", bias_angle);


        } else {
            hal->MicHandle = NULL;
        }

        s_hal = hal;
        return 0;
    } while (0);

    if (board_init) {
        uni_4mic_board_deinit(&hal->env_cfg);
    }

    if (hal) {
        free(hal);
    }

    return -1;
}

int uni_4mic_hal_release(void) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    HAL_PRINT_INFO("%s \n", __func__);

    if (hal->MicHandle) {
        uni_hal_4mic_array_release(hal->MicHandle);
        hal->MicHandle = NULL;
    }

    uni_4mic_board_deinit(&hal->env_cfg);

    free(hal);
    s_hal = NULL;

    return 0;
}

static void *pcm_read_raw_thread_proc(void *arg) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) arg;
    int ret;
    int read_size;
    char *pcm_buf;
    struct timeval temp;
    if (save_file_on == 1) {

        char filenameorg[256];
        sprintf(filenameorg, "%s/unidata/file_raw_org.pcm", "/sdcard");
        file_all_read_org = fopen(filenameorg, "ab+");


        char filename2mic[256];
        sprintf(filename2mic, "%s/unidata/file_8mic.pcm", "/sdcard");
        file_2mic = fopen(filename2mic, "ab+");

        char filename2aec[256];
        sprintf(filename2aec, "%s/unidata/file_2aec.pcm", "/sdcard");
        file_2aec_de = fopen(filename2aec, "ab+");


        char filename3[256];
        sprintf(filename3, "%s/unidata/file_out.pcm", "/sdcard");
        file_out_read = fopen(filename3, "ab+");


    }
//HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).\n", __func__, __LINE__);	
    if (NULL == hal) {
        HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).\n", __func__, __LINE__);
        return NULL;
    }
//HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).\n", __func__, __LINE__);
    read_size = 10240;
    //HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal). SLOT_FRAMES_ALL_RAW_BYTES %d\n", __func__, __LINE__, SLOT_FRAMES_ALL_RAW_BYTES);
    pcm_buf = (char *) calloc(read_size, 1);
    if (NULL == pcm_buf) {
        HAL_PRINT_ERR("[%s:%d] ERROR: failed to calloc pcm_buf .\n", __func__, __LINE__);
        return NULL;
    }
    // HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).  \n", __func__, __LINE__);

    char filename[256];
    sprintf(filename, "%s/file_all.pcm", "/data/misc/audioserver");
    //file_all_read = fopen(filename, "wb+");

    char filename1[256];
    sprintf(filename1, "%s/file_aec.pcm", "/data/misc/audioserver");
// file_aec_read = fopen(filename1, "wb+");

    char filename2[256];
    sprintf(filename2, "%s/file_mic.pcm", "/data/misc/audioserver");
    //file_mic_read = fopen(filename2, "wb+");

    // char filename3[256];
    //sprintf(filename3, "%s/file_out.pcm", "/data/misc/audioserver");
// file_out_read = fopen(filename3, "wb+");


    while (hal->pcm_read_thread_running) {
        if (pcm_is_ready(hal->pcm)) {
            //	HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read return %d. 1\n", __func__, __LINE__, ret);

            if (log_g_pcm == 0) {
                log_g_pcm = 1;
                HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read \n", __func__, __LINE__);
            }
            ret = pcm_read(hal->pcm, pcm_buf, read_size);


            //	fwrite(pcm_buf, 1, 19200, file_all_read);
            //   fflush(file_all_read);
            if (0) {
                if (g_write_count_raw == 0) {

                    char filename[256];
                    sprintf(filename, "%s/file_raw_%d.pcm", "/data/misc/audioserver",
                            g_debug_file_raw);
                    file_all_read_raw = fopen(filename, "wb+");
                    g_debug_file_raw++;
                }

                if (g_write_count_raw == 1000) {
                    fclose(file_all_read_raw);
                    char filename[256];
                    sprintf(filename, "%s/file_raw_%d.pcm", "/data/misc/audioserver",
                            g_debug_file_raw);
                    file_all_read_raw = fopen(filename, "wb+");
                    g_write_count_raw = 1;
                    g_debug_file_raw++;
                }

                g_write_count_raw++;
                fwrite(pcm_buf, 1, read_size, file_all_read_raw);
                fflush(file_all_read_raw);
            }

            //		HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read return %d. read_size %d\n", __func__, __LINE__, ret, read_size);
            if (0 > ret) {
                HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read return %d.\n", __func__, __LINE__, ret);
                //system("reboot");
                if (g_unisound_event_callback != NULL) {
                    HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read return %d.---\n", __func__, __LINE__,
                                  ret);
                    g_pcm_read_error = 1;
                    g_unisound_event_callback(ret);
                    sleep(1);
                    goto l_exit;

                }
            }
        } else {
            HAL_PRINT_ERR("[%s:%d] ERROR: hal is %p, device is not ready for reading.\n", __func__,
                          __LINE__, hal);
            goto l_exit;
        }
#if 0
        //------------32bit 2 16bit------------------------------

        //int temp = read_size;
            int count= 576/4;
            int p_size=0;

            int* data_raw = (int*)malloc(count);
            short* data_ref = (short*)malloc(count);
           // memcpy(data_raw,(int*)&buf,count);
            char *buff=NULL;
            buff=(char *)malloc(5);

            char *buff_t =NULL;
            buff_t=(char *)malloc(read_size);


            unsigned int* date32 = NULL;//32λ�Ĳ�������
            unsigned short date16 = 0;//16λ�Ĳ�������
            char tmp_p[3] = {0};

            unsigned short m_date16 = 0;

            for(;p_size<count;p_size++)
            {
                 //data_ref[p_size]=data_raw[p_size]>>16;
                 memcpy(buff,pcm_buf+4*p_size,4);

                //  fwrite(buff, 1, 4, file_all_read);
              //   fflush(file_all_read);

                 date32 = ( unsigned int*)buff;
                 date16 = (*date32) >> 16;
                 m_date16 = date16  - 65535;

              memcpy(&tmp_p,(char*)&m_date16,2);

              memcpy(buff_t+2*p_size, tmp_p, 2);

            }

            memcpy(pcm_buf, buff_t, 2*p_size);
          //  read_size=2*p_size;
           // printf("uni_hal_audio_read--- %d \n", 2*p_size);
            free(buff);
            free(buff_t);
            free(data_raw);
            free(data_ref);

        //-------------------------------------------------------
          //  fwrite(pcm_buf, 1, p_size*2, file_all_read);
          //  fflush(file_all_read);
#endif
        //HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read return123 %d.hal->read_start  %d\n", __func__, __LINE__, ret, hal->read_start);
        //if(hal->read_start){
        //HAL_PRINT_ERR("[%s:%d] ERROR: hal is %p, device is not ready for reading. 123\n", __func__, __LINE__, hal);
        writeBuf(&hal->RecordRingBuffer, pcm_buf, read_size);
        //}
#if 0
        if(g_pcm_test_flag==1){
            g_pcm_test_flag=0;
            char ffff[30] = {0};
            memset(ffff, 0xff, 20);
            ffff[0] = 0x00;
            ffff[2] = 0x00;
            ffff[4] = 0x00;
            ffff[6] = 0x00;
            ffff[8] = 0x00;
            ffff[10] = 0x00;
            ffff[12] = 0x00;
            ffff[14] = 0x00;
            ffff[16] = 0x00;
            ffff[18] = 0x00;
        //	ffff[10] = 0x00;
        //	ffff[12] = 0x00;

            ffff[1] = 0x70;
            ffff[3] = 0x70;
            ffff[5] = 0x70;
            ffff[7] = 0x70;
            ffff[9] = 0x70;
            ffff[11] = 0x70;
            ffff[13] = 0x70;
            ffff[15] = 0x70;
            ffff[17] = 0x70;
            ffff[19] = 0x70;


            writeBuf(&hal->RecordRingBuffer_org, ffff, 20);

        }
#endif
        writeBuf(&hal->RecordRingBuffer_org, pcm_buf, read_size);
        //   temp.tv_sec = 0;
        //  temp.tv_usec = 1000;
#if 0
        //   select(0, NULL, NULL, NULL, &temp);
        int logp =0;
      short   *p_out_data = (short *)pcm_buf;

      // char t =0x88;
      // char e = 0xff;
      // short uuu = (t << 8) | e ;
      // unsigned short uuu =(((short)t)<<8)|((short)e);
      // printf("%d \n", uuu);
        for(;logp<10240/2;logp++){
             short u= ((short*)pcm_buf)[logp];

            // c = (b << 8) | (a & 0xFF);
            //short u =
        //	int y = logp*2;

        //	 short u= (pcm_buf[logp*2] << 8) | (pcm_buf[logp*2+1] );
        // printf(" %d\n", u);

        // HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read %d \n", __func__, __LINE__, u);

             if(u>32767){
                   printf(" 32767\n");
                  // exit(0);
                   HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read  >32768 \n", __func__, __LINE__);
                   break;
             }

             if(u<-32767){
                 printf(" 32767\n");
        //	 exit(0);
                   HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read  <-32768 \n", __func__, __LINE__);
                   break;
             }

             if(u==32767||u==-32767){
                 printf(" 32767\n");
             //	exit(0);
                  HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read  ==-32768  ==32767 \n", __func__, __LINE__);
                  break;
            }

       }
#endif
        if (log_g_pcm_af == 0) {
            log_g_pcm_af = 1;
            HAL_PRINT_ERR("[%s:%d] ERROR: first pcm_read end \n", __func__, __LINE__);
        }

    }

    l_exit:
    free(pcm_buf);
    return NULL;
}

static int raw_pcm_split(char *packbuf, int size, char *mic_pcm, char *aec_pcm) {
    int i, m, a;
    char *pos;

//HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).RAW_FRAME_BYTES %d size %d\n", __func__, __LINE__, RAW_FRAME_BYTES, size);


    for (i = 0, m = 0, a = 0; i < size; i += RAW_FRAME_BYTES, m += 16, a += 4) {
        pos = packbuf + i;
        memcpy(mic_pcm + m, pos + 16, 16);
        memcpy(aec_pcm + a, pos + 32, 4);
    }


    // fwrite(mic_pcm, 1, (SLOT_FRAMES_ALL_RAW_BYTES/18) * 8, file_8mic);
    // fflush(file_8mic);

    return 0;
}

static void *pcm_read_packed_thread_proc(void *arg) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) arg;
    int ret;
    int read_size;
    int start_pos;
    int i;
    int left;
    int remainder;
    char *pcm_buf;
    const char frame_ids[PACKED_CHAN_NUM] = {
#if (PACKED_CHAN_NUM == 12)
            0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0,
            0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0,
#elif (PACKED_CHAN_NUM == 6)
            0x00, 0x80, 0x40,
            0x10, 0x90, 0x50,
#endif
    };
    HAL_PRINT_ERR("[%s:%d] ERROR: failed to calloc pcm_buf .\n", __func__, __LINE__);
    const char frame_ids_v3[PACKED_CHAN_NUM] = {
            0x00, 0xff, 0x80, 0xff, 0x40, 0xff, 0x10, 0xff, 0x90, 0xff, 0x50, 0x00,
    };


    if (NULL == hal) {
        HAL_PRINT_ERR("[%s:%d] ERROR: (NULL == hal).\n", __func__, __LINE__);
        return NULL;
    }

    read_size = SLOT_FRAMES_PACKED_BYTES;
    pcm_buf = (char *) calloc(read_size + PACKED_FRAME_BYTES, 1);
    if (NULL == pcm_buf) {
        HAL_PRINT_ERR("[%s:%d] ERROR: failed to calloc pcm_buf .\n", __func__, __LINE__);
        return NULL;
    }

    remainder = (read_size % PACKED_FRAME_BYTES) / 4;
    //VAR_DBG(remainder);

    while (hal->pcm_read_thread_running) {
        do {   //to find the fist valid pcm data, and the index is start_pos
            if (pcm_is_ready(hal->pcm)) {
                ret = pcm_read(hal->pcm, pcm_buf, PACKED_FRAME_BYTES);
                if (0 > ret) {
                    HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read_ex return %d.\n", __func__, __LINE__,
                                  ret);
                }
            } else {
                HAL_PRINT_ERR("[%s:%d] ERROR: hal is %p, device is not ready for reading.\n",
                              __func__, __LINE__, hal);
                goto l_exit;
            }

            start_pos = PACKED_CHAN_NUM;
            for (i = 0; i < PACKED_CHAN_NUM; i++) {
                if (frame_ids[0] == pcm_buf[4 * i + 1]) {
                    start_pos = i;
                    if (g_flag_codec_id == 0) {
                        if (pcm_buf[4 * (i + 1) + 1] == 0xff) {
                            if (pcm_buf[4 * (i + 3) + 1] == 0xff) {
                                g_flag_codec_id = 1;
                            }
                        }
                        HAL_PRINT_DEBUG("[%s:%d] 0 v2 v3 g_flag_codec_id== %d\n", __func__,
                                        __LINE__, g_flag_codec_id);
                    }
                    break;
                }
            }
        } while (hal->pcm_read_thread_running && PACKED_CHAN_NUM == start_pos);

        //VAR_DBG(start_pos);
        for (i = 0; i < PACKED_CHAN_NUM; i++) {
            HAL_PRINT_INFO("i:%02d,0x%02x\n", i, pcm_buf[4 * i + 1]);
        }

        left = PACKED_FRAME_BYTES - start_pos * 4;
        memcpy(pcm_buf, pcm_buf + start_pos * 4, left);
        start_pos = 0;

        while (hal->pcm_read_thread_running) {
            if (pcm_is_ready(hal->pcm)) {
                ret = pcm_read(hal->pcm, pcm_buf + left, read_size);
                if (0 > ret) {
                    HAL_PRINT_ERR("[%s:%d] ERROR: pcm_read_ex return %d.\n", __func__, __LINE__,
                                  ret);
                }
            } else {
                HAL_PRINT_ERR("[%s:%d] ERROR: hal is %p, device is not ready for reading.\n",
                              __func__, __LINE__, hal);
                goto l_exit;
            }

            if (hal->read_start) {
#if 0
                for(i = 0; i < PACKED_FRAME_BYTES; i += 4){
                HAL_PRINT_DEBUG("%02x,%02x,%02x,%02x\n", pcm_buf[i], pcm_buf[i+1], pcm_buf[i+2], pcm_buf[i+3]);
                }
                HAL_PRINT_DEBUG("\n");
#endif
                writeBuf(&hal->RecordRingBuffer, pcm_buf, read_size);
            }
            memcpy(pcm_buf, pcm_buf + read_size, left);

            start_pos = (start_pos + remainder) % PACKED_CHAN_NUM;
            if (g_flag_codec_id == 1) {
                if (frame_ids_v3[start_pos] != pcm_buf[1]) {
                    HAL_PRINT_ERR(
                            "[%s:%d] ERROR: the pcm v3 data is broken, [start_pos:%02d,%02x].\n",
                            __func__, __LINE__, start_pos, pcm_buf[1]);
                    break;
                }
            } else {
                if (frame_ids[start_pos] != pcm_buf[1]) {
                    HAL_PRINT_ERR("[%s:%d] ERROR: the pcm data is broken, [start_pos:%02d,%02x].\n",
                                  __func__, __LINE__, start_pos, pcm_buf[1]);
                    break;
                }

            }
        }
    }

    l_exit:
    free(pcm_buf);
    return NULL;
}

//for NAU85L40
static int unpack(char *packbuf, int size, char *mic_pcm, char *aec_pcm) {
    int i, k;
    char *pos;

    for (k = 0; k < size / PACKED_CHAN_NUM / 4; k++) {
        pos = packbuf + k * PACKED_CHAN_NUM * 4;

//0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0: mic2, mic4, AECL, NC, NC, NC
//0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0: mic1, mic3, AECR, NC, NC, NC
        for (i = 0; i < PACKED_CHAN_NUM; i++) {
            switch ((*(pos + i * 4 + 1))) {
                case 0x10:  //mic0
                    mic_pcm[k * 8 + 0] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 1] = *(pos + i * 4 + 3);
                    break;

                case 0x00:  //mic1
                    mic_pcm[k * 8 + 2] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 3] = *(pos + i * 4 + 3);
                    break;

                case 0x90:  //mic2
                    mic_pcm[k * 8 + 4] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 5] = *(pos + i * 4 + 3);
                    break;

                case 0x80:  //mic3
                    mic_pcm[k * 8 + 6] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 7] = *(pos + i * 4 + 3);
                    break;

                case 0x50:  //aecR
                    aec_pcm[k * 4 + 0] = *(pos + i * 4 + 2);
                    aec_pcm[k * 4 + 1] = *(pos + i * 4 + 3);
                    break;

                case 0x40:  //aecL
                    aec_pcm[k * 4 + 2] = *(pos + i * 4 + 2);
                    aec_pcm[k * 4 + 3] = *(pos + i * 4 + 3);
                    break;

                default:
                    break;
            }

        }

    }

    return 0;
}

//for ma4v2 and ma4v3
static int unpack_es(char *packbuf, int size, char *mic_pcm, char *aec_pcm) {
    int i, k;
    char *pos;

    HAL_PRINT_DEBUG("[%s:%d] .\n", __func__, __LINE__);

    for (k = 0; k < size / PACKED_CHAN_NUM / 4; k++) {
        pos = packbuf + k * PACKED_CHAN_NUM * 4;

//0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0: mic2, mic4, AECL, NC, NC, NC
//0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0: mic1, mic3, AECR, NC, NC, NC
        for (i = 0; i < PACKED_CHAN_NUM; i++) {
            switch ((*(pos + i * 4 + 1))) {
                case 0x00:  //mic0
                    mic_pcm[k * 8 + 0] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 1] = *(pos + i * 4 + 3);
                    break;

                case 0x10:  //mic1
                    mic_pcm[k * 8 + 2] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 3] = *(pos + i * 4 + 3);
                    break;

                case 0x80:  //mic2
                    mic_pcm[k * 8 + 4] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 5] = *(pos + i * 4 + 3);
                    break;

                case 0x90:  //mic3
                    mic_pcm[k * 8 + 6] = *(pos + i * 4 + 2);
                    mic_pcm[k * 8 + 7] = *(pos + i * 4 + 3);
                    break;

                case 0x50:  //aecR
                    aec_pcm[k * 4 + 0] = *(pos + i * 4 + 2);
                    aec_pcm[k * 4 + 1] = *(pos + i * 4 + 3);
                    break;

                case 0x40:  //aecL
                    aec_pcm[k * 4 + 2] = *(pos + i * 4 + 2);
                    aec_pcm[k * 4 + 3] = *(pos + i * 4 + 3);
                    break;

                default:
                    break;
            }

        }

    }

    return 0;
}

static int unpack_raw(char *packbuf, int size, char *mic_pcm, char *aec_pcm) {
    // int i, m, a;
    //  char *pos;

//HAL_PRINT_ERR("[%s:%d] be .RAW_FRAME_BYTES %d size %d\n", __func__, __LINE__, RAW_FRAME_BYTES, size);


    // for(i = 0, m = 0, a = 0; i < size; i += RAW_FRAME_BYTES, m += 16, a += 4){
    //      pos = packbuf + i;
    //      memcpy(mic_pcm + m, pos, 16);
    //       memcpy(aec_pcm + a, pos + 32, 4);
    //  }

    int k = 0;
    char *pos;
#if 0
    if(save_file_on==1){
   fwrite(packbuf, 1, size, file_all_read_org);
   fflush(file_all_read_org);
    }
#endif
    // for(k = 0; k < size / PACKED_CHAN_NUM / 8; k++){
    for (k = 0; k < size / 20; k++) {
        pos = packbuf + k * 20;

        mic_pcm[k * 16 + 0] = *(pos + 0);
        mic_pcm[k * 16 + 1] = *(pos + 1);

        mic_pcm[k * 16 + 2] = *(pos + 2);
        mic_pcm[k * 16 + 3] = *(pos + 3);

        mic_pcm[k * 16 + 4] = *(pos + 4);
        mic_pcm[k * 16 + 5] = *(pos + 5);

        mic_pcm[k * 16 + 6] = *(pos + 6);
        mic_pcm[k * 16 + 7] = *(pos + 7);

        mic_pcm[k * 16 + 8] = *(pos + 8);
        mic_pcm[k * 16 + 9] = *(pos + 9);

        mic_pcm[k * 16 + 10] = *(pos + 10);
        mic_pcm[k * 16 + 11] = *(pos + 11);

        mic_pcm[k * 16 + 12] = *(pos + 12);
        mic_pcm[k * 16 + 13] = *(pos + 13);

        mic_pcm[k * 16 + 14] = *(pos + 14);
        mic_pcm[k * 16 + 15] = *(pos + 15);

        aec_pcm[k * 4 + 0] = *(pos + 16);
        aec_pcm[k * 4 + 1] = *(pos + 17);
        aec_pcm[k * 4 + 2] = *(pos + 18);
        aec_pcm[k * 4 + 3] = *(pos + 19);
    }
    // fwrite(mic_pcm, 1, (size/20) * 16, file_8mic);
    // fflush(file_8mic);
// HAL_PRINT_ERR("[%s:%d] af .RAW_FRAME_BYTES %d size %d\n", __func__, __LINE__, RAW_FRAME_BYTES, size);   
#if 0
    if(save_file_on==2){

           fwrite(mic_pcm, 1, SLOT_FRAMES_BYTES * 8 , file_2mic);
           fflush(file_2mic);

           fwrite(aec_pcm, 1, SLOT_FRAMES_BYTES * 2, file_2aec_de);
           fflush(file_2aec_de);
            }
#endif
    return 0;
}


static void *mic_array_thread_proc(void *arg) {
#define MICARRAY_OUTLEN_MAX         512
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) arg;
    char *packbuf = NULL;
#if FILE_READ_TEST
    char *packbuf1 = NULL;
#endif
    char *mic_pcm = NULL;
    char *aec_pcm = NULL;
    char *out_pcm = NULL;
    int out_size;
    int i = 0;
    short *Out_asr = NULL;
    short *Out_vad = NULL;
    short *p_out_data = NULL;
    int outlen;
    int val;
    int mode_change;

    HAL_PRINT_INFO("%s start SLOT_FRAMES_ALL_RAW_BYTES %d\n", __func__, SLOT_FRAMES_ALL_RAW_BYTES);

    char *mic_pcm_tmp = NULL;
    char *aec_pcm_tmp = NULL;

    char test_doa_flag_string[4] = {0};
    char test_doa_string[4] = {0};
    int damaged_mic_detect = 1;
    float detected_time = 3.0f;

    do {
        if (hal->raw_pcm) {
            packbuf = (char *) malloc(SLOT_FRAMES_ALL_RAW_BYTES);
            if (NULL == packbuf) {
                HAL_PRINT_ERR("%s: (NULL == packbuf)\n", __func__);
                break;
            }

#if FILE_READ_TEST
            packbuf1 = (char *)malloc(SLOT_FRAMES_ALL_RAW_BYTES);
           if(NULL == packbuf1){
               HAL_PRINT_ERR("%s: (NULL == packbuf)\n", __func__);
               break;
           }
#endif
            mic_pcm_tmp = (char *) calloc(
                    SLOT_FRAMES_ALL_RAW_BYTES / RAW_CHAN_NUM * 8 / sizeof(short), sizeof(short));
            if (NULL == mic_pcm_tmp) {
                //HAL_PRINT_ERR("%s: (NULL == mic_pcm)\n", __func__);
                break;
            }

            mic_pcm = (char *) calloc(SLOT_FRAMES_ALL_RAW_BYTES / RAW_CHAN_NUM * 8 / sizeof(short),
                                      sizeof(short));
            if (NULL == mic_pcm) {
                HAL_PRINT_ERR("%s: (NULL == mic_pcm)\n", __func__);
                break;
            }

            aec_pcm = (char *) calloc(SLOT_FRAMES_ALL_RAW_BYTES / RAW_CHAN_NUM * 2 / sizeof(short),
                                      sizeof(short));
            if (NULL == aec_pcm) {
                HAL_PRINT_ERR("%s: (NULL == aec_pcm)\n", __func__);
                break;
            }

            aec_pcm_tmp = (char *) calloc(
                    SLOT_FRAMES_ALL_RAW_BYTES / RAW_CHAN_NUM * 2 / sizeof(short), sizeof(short));
            if (NULL == aec_pcm_tmp) {
                HAL_PRINT_ERR("%s: (NULL == aec_pcm)\n", __func__);
                break;
            }


            out_pcm = (char *) calloc(MICARRAY_OUTLEN_MAX * 2, sizeof(short));
            if (NULL == out_pcm) {
                HAL_PRINT_ERR("%s: (NULL == out_pcm)\n", __func__);
                break;
            }
        } else {
            packbuf = (char *) malloc(SLOT_FRAMES_PACKED_BYTES * PACKED_CHAN_NUM);
            if (NULL == packbuf) {
                HAL_PRINT_ERR("%s: (NULL == packbuf)\n", __func__);
                break;
            }

            mic_pcm = (char *) calloc(SLOT_FRAMES_PACKED_BYTES / 2 * 4 / sizeof(short),
                                      sizeof(short));
            if (NULL == mic_pcm) {
                HAL_PRINT_ERR("%s: (NULL == mic_pcm)\n", __func__);
                break;
            }

            aec_pcm = (char *) calloc(SLOT_FRAMES_PACKED_BYTES / 2 * 2 / sizeof(short),
                                      sizeof(short));
            if (NULL == aec_pcm) {
                HAL_PRINT_ERR("%s: (NULL == aec_pcm)\n", __func__);
                break;
            }

            out_pcm = (char *) calloc(MICARRAY_OUTLEN_MAX * 2, sizeof(short));
            if (NULL == out_pcm) {
                HAL_PRINT_ERR("%s: (NULL == out_pcm)\n", __func__);
                break;
            }
        }
#if FILE_READ_TEST
        FILE* file= fopen("/sdcard/unidata/file_raw_10ch.pcm", "rb");
         unsigned int totalBytesRead = 0;
        unsigned int bytesRead;
#endif
        ual_fixed_direct_set(hal->MicHandle, UAL_FDE_DAMAGED_MIC_DETECT, &damaged_mic_detect);
        ual_fixed_direct_set(hal->MicHandle, UAL_FDE_DAMAGED_MIC_DETECTED_TIME, &(detected_time));


        int idx = 1;
        ual_fixed_direct_set(hal->MicHandle, UAL_FDE_AEC_MORE, &idx);
        //set cpu
        /*int cpu_index = 1;
        cpu_set_t thread_cpu_set;
        CPU_ZERO(&thread_cpu_set);
        CPU_SET(cpu_index, &thread_cpu_set);
        if (sched_setaffinity(0, sizeof(thread_cpu_set), &thread_cpu_set) == -1) {
            printf("sched_setaffinity error[%d]:%s\n", errno, strerror(errno));
            return NULL;
        }
        printf("cpu[%d] start\n", cpu_index);*/
        while (hal->mic_array_thread_running) {
            if (hal->raw_pcm) {
                //HAL_PRINT_ERR("readBuf =%d, max=%d.  -----123  SLOT_FRAMES_ALL_RAW_BYTES %d\n", outlen, MICARRAY_OUTLEN_MAX, SLOT_FRAMES_ALL_RAW_BYTES);
                readBuf(&hal->RecordRingBuffer, packbuf, SLOT_FRAMES_ALL_RAW_BYTES);
                //  HAL_PRINT_ERR("readBuf =%d, max=%d.  -----123456 pcm_read_error %d\n", outlen, MICARRAY_OUTLEN_MAX, g_pcm_read_error);
#if FILE_READ_TEST
                bytesRead = fread(packbuf1, 1, SLOT_FRAMES_ALL_RAW_BYTES, file);

            //HAL_PRINT_ERR("zxl-fread file_raw_10ch.pcm %d \n",bytesRead);

        if (bytesRead > 0) {

            totalBytesRead += bytesRead;
        }
        if(bytesRead != SLOT_FRAMES_ALL_RAW_BYTES){
            hal->mic_array_thread_running=0;
            HAL_PRINT_ERR("zxl-read file_raw_10ch.pcm end2 \n");
        break;


        }
                unpack_raw(packbuf1, SLOT_FRAMES_ALL_RAW_BYTES, mic_pcm, aec_pcm);
#else

                unpack_raw(packbuf, SLOT_FRAMES_ALL_RAW_BYTES, mic_pcm, aec_pcm);
#endif
                //   raw_pcm_split(packbuf, SLOT_FRAMES_ALL_RAW_BYTES, mic_pcm, aec_pcm);
            }


            if (0) {
                short *aec = aec_pcm;
                int high_limit = 32767 / top_aec_enlarge;
                int low_limit = -32767 / top_aec_enlarge;
                for (i = 0; i < 512; i++) {
                    if (aec[i] > high_limit) {
                        aec[i] = high_limit;
                        aec[i] *= top_aec_enlarge;
                        // HAL_PRINT_ERR("top_aec_enlarge 1  %d  \n", top_aec_enlarge);
                    } else if (aec[i] < low_limit) {
                        aec[i] = low_limit;
                        aec[i] *= top_aec_enlarge;
                        // HAL_PRINT_ERR("top_aec_enlarge 2  %d  \n", top_aec_enlarge);
                    } else {
                        aec[i] *= top_aec_enlarge;
                        // HAL_PRINT_ERR("top_aec_enlarge 3  %d   \n", top_aec_enlarge );
                    }
                }
            }

            if (1) {


#if 0
                if(save_file_on==0){

                       fwrite(mic_pcm, 1, SLOT_FRAMES_BYTES * 8 , file_2mic);
                       fflush(file_2mic);

                       fwrite(aec_pcm, 1, SLOT_FRAMES_BYTES * 2, file_2aec_de);
                       fflush(file_2aec_de);
                        }
#endif
#if FILE_READ_TEST
                if(save_file_on==1){
               fwrite(packbuf1, 1, SLOT_FRAMES_BYTES * 10, file_all_read_org);
               fflush(file_all_read_org);
                }

#else
                if (save_file_on == 1) {
                    fwrite(packbuf, 1, SLOT_FRAMES_BYTES * 10, file_all_read_org);
                    fflush(file_all_read_org);
                }


#endif

                pthread_mutex_lock(&lock_uni);

                uni_hal_4mic_array_process(hal->MicHandle,
                                           (short *) mic_pcm,
                                           SLOT_FRAMES,
                                           (short *) aec_pcm,
                                           hal->is_waked,
                                           &Out_asr,
                                           &Out_vad,
                                           &outlen
                );
                pthread_mutex_unlock(&lock_uni);





                //  #define UAL_FDE_MODE_CHANGE       1110
                //    ual_fixed_direct_get(handle, UAL_FDE_MODE_CHANGE, &mode_change);
                uni_hal_4mic_array_get_org(hal->MicHandle, 1110, &mode_change);
                if (mode_change) {
                    // printf("mode change!data time:%f\n", (float)cur_frame * 256.0f/16000.0f);
                    if (g_unisound_event_callback != NULL) {
                        g_unisound_event_callback(1);

                    }
                }

                if (MICARRAY_OUTLEN_MAX < outlen) {
                    HAL_PRINT_ERR("wrong outlen=%d, max=%d.\n", outlen, MICARRAY_OUTLEN_MAX);
                    break;
                }

                val = -1;
                // uni_hal_4mic_array_get(hal->MicHandle, UNI_FDE_DOA, (void*)(&val));
                //HAL_PRINT_INFO("UAL_FDE_DOA=%d  %d\n", val, outlen);
                sprintf(test_doa_string, "%d", val);

                if (outlen < 256) {
                    // HAL_PRINT_INFO("UAL_FDE_DOA=%d  %d---20200724v2 111\n", val, outlen);
                    // HAL_PRINT_INFO("UAL_FDE_DOA=%d  %d\n", val, outlen);
                } else {
                    // HAL_PRINT_INFO("UAL_FDE_DOA=%d  %d---20200724v6 000\n", val, outlen);
                    // writeBuf(&hal->micArrayRingBuf, test_doa_flag_string, 4);
                    if (g_pcm_read_error == 0) {
                        writeBuf(&hal->micArrayRingBuf, (char *) (&val), 4);
                    } else {
                        break;
                    }
                    //  writeBuf(&hal->micArrayRingBuf, test_doa_flag_string, 4);
                }
                p_out_data = (short *) out_pcm;
                // short tmp_data ;
                if (0 == hal->env_cfg.no_vad) {


                    for (i = 0; i < outlen; i++) {
                        *(p_out_data + 2 * i) = *(Out_asr + i);
                        //tmp_data = *(Out_asr + i);
                        // *(p_out_data + 2 * i + 1) = *(Out_vad + i);
                        *(p_out_data + 2 * i + 1) = *(Out_asr + i);

                    }
                    out_size = outlen * 4;
                    if (g_pcm_read_error == 0) {
                        writeBuf(&hal->micArrayRingBuf, out_pcm, out_size);
                    }

                    if (save_file_on == 1) {
                        fwrite(Out_asr, 1, SLOT_FRAMES_BYTES, file_out_read);
                        fflush(file_out_read);
                    }


                } else {
                    out_size = outlen * 2;
                    if (g_pcm_read_error == 0) {
                        writeBuf(&hal->micArrayRingBuf, Out_asr, out_size);
                    }
                }
            } else {

                int c_debug = SLOT_FRAMES_PACKED_BYTES * PACKED_CHAN_NUM;
                int c_out_debug = c_debug * 4 / 6;


                if (2 == hal->ch_num) {
                    int jj = 0;
                    int uu = 0;
                    out_size = SLOT_FRAMES * 4;

//HAL_PRINT_INFO("uni_hal_4mic_array_process undo  g_invoke_conut_fun  %d c_out_debug %d  c_debug %d out_size %d \n", \
//(*g_invoke_conut_fun)(), c_out_debug,c_debug, out_size ); 
                    for (i = 0; i < out_size / 4; i++) {
                        out_pcm[4 * i + 0] = mic_pcm[16 * i + 0];
                        out_pcm[4 * i + 1] = mic_pcm[16 * i + 1];
                        out_pcm[4 * i + 2] = mic_pcm[16 * i + 2];
                        out_pcm[4 * i + 3] = mic_pcm[16 * i + 3];
                    }


#if 1
                    short *aec = out_pcm;
                    for (jj = 0; jj < out_size / 2; jj++) {
                        aec[jj] *= 8.0f;
                        if (aec[jj] > 32767) {
                            aec[jj] = 32767;
                        } else if (aec[jj] < -32768) {
                            aec[jj] = -32768;
                        }
                        /*
                        int tmp =0;
                        tmp = aec[jj] * 10.0f;
                        aec[jj] = tmp;*/
                    }

                    out_pcm = aec;
#endif

                    if (1) {
                        int r = 0;


                        //  HAL_PRINT_ERR("%s: failed--20190707---%d ----\n ", __func__, r);

                    }

                } else if (1 == hal->ch_num) {
                    out_size = SLOT_FRAMES_BYTES;
                    for (i = 0; i < out_size / 2; i++) {
                        out_pcm[2 * i + 0] = mic_pcm[4 * i + 0];
                        out_pcm[2 * i + 1] = mic_pcm[4 * i + 1];
                    }
                } else {
                    out_size = 0;
                }

/*
    if(can_write==1){
    	int r=0;
      	
        r = write(g_fd ,out_pcm, out_size);
        HAL_PRINT_ERR("%s: failed--20190702---%d ----\n ", __func__, r);
                	
    }*/
                if (g_pcm_read_error == 0) {
                    writeBuf(&hal->micArrayRingBuf, out_pcm, out_size);
                }


            }

#ifdef ___DEBUG
            //  if(hal->debugMode){
            if (0) {


                if (g_write_count == 0) {

                    char filename[256];
                    sprintf(filename, "%s/file_10ch_16000_16bit_%d.pcm", "/sdcard/unidata",
                            g_debug_file);

                    char filenamenoflag[256];
                    sprintf(filenamenoflag, "%s/file_file_10ch_16000_16bit_%d.pcm",
                            "/sdcard/unidata/", g_debug_file);

                    // file_all_read_noflag = fopen(filenamenoflag, "wb+");
                    file_all_read = fopen(filename, "wb+");
                    g_debug_file++;
                }

                if (g_write_count == 10000) {
                    fclose(file_all_read);
                    // fclose(file_all_read_noflag);

                    char filename[256];
                    sprintf(filename, "%s/file_10ch_16000_16bit_%d.pcm", "/sdcard/unidata",
                            g_debug_file);

                    char filenamenoflag[256];
                    // sprintf(filenamenoflag, "%s/file_noflagout_%d.pcm", "/data/misc/audioserver", g_debug_file);
                    // file_all_read_noflag = fopen(filenamenoflag, "wb+");
                    file_all_read = fopen(filename, "wb+");
                    g_write_count = 1;
                    g_debug_file++;
                }

                g_write_count++;
                if (outlen < 256) {

                } else {
                    // fwrite(test_doa_flag_string, 1, 4, file_all_read);
                    // fwrite(test_doa_string, 1, 4, file_all_read);
                }

                // packbuf, SLOT_FRAMES_ALL_RAW_BYTES
                fwrite(packbuf, 1, SLOT_FRAMES_ALL_RAW_BYTES, file_all_read);
                fflush(file_all_read);

                //fwrite(out_pcm, 1, out_size, file_all_read_noflag);
                //fflush(file_all_read_noflag);
            }
#endif
        }

    } while (0);

    if (out_pcm) {
        free(out_pcm);
    }
    if (aec_pcm) {
        free(aec_pcm);
    }
    if (aec_pcm_tmp) {
        free(aec_pcm_tmp);
    }
    if (mic_pcm) {
        free(mic_pcm);
    }
    if (mic_pcm_tmp) {
        free(mic_pcm_tmp);
    }
    if (packbuf) {
        free(packbuf);
    }

    HAL_PRINT_INFO("%s end\n", __func__);
    return NULL;
}

#ifdef ___DEBUG

static int debug_record_files(void *handle, int on) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) handle;
    if (1) {
        if (0) {
            char *filename = calloc(512, 1);
            char *cmd = filename;
            if (NULL == filename) {
                HAL_PRINT_ERR("[%s:%d] (NULL == filename)\n", __func__, __LINE__);
            }

            memset(cmd, 0, 512);
            sprintf(cmd, "mkdir -p %s", hal->env_cfg.debug_file_path);
            system(cmd);

            memset(filename, 0, 512);
            if (0) {
                sprintf(filename, "%s/waked_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_4mic_name);
            } else {
                sprintf(filename, "%s/waking_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_4mic_name);
            }

            HAL_PRINT_ERR("[%s:%d] (NULL == filename) %s\n", __func__, __LINE__, filename);
            file_4mic = fopen(filename, "wb");
            if (NULL == file_4mic) {
                HAL_PRINT_ERR("file_4mic open fail: %s\n", filename);
            }

            memset(filename, 0, 512);
            if (0) {
                sprintf(filename, "%s/waked_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_2aec_name);
            } else {
                sprintf(filename, "%s/waking_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_2aec_name);
            }
            file_2aec = fopen(filename, "wb");
            if (NULL == file_2aec) {
                HAL_PRINT_ERR("file_2aec open fail: %s\n", filename);
            }

            memset(filename, 0, 512);
            if (0) {
                sprintf(filename, "%s/waked_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_out_name);
            } else {
                sprintf(filename, "%s/waking_%s", "/data/misc/audioserver",
                        hal->env_cfg.debug_file_out_name);
            }
            file_out = fopen(filename, "wb");
            if (NULL == file_out) {
                HAL_PRINT_ERR("file_out open fail: %s\n", filename);
            }

            free(filename);

            if (file_4mic) {
                fseek(file_4mic, sizeof(struct wav_header), SEEK_SET);
            }
            if (file_2aec) {
                fseek(file_2aec, sizeof(struct wav_header), SEEK_SET);
            }
            if (file_out) {
                fseek(file_out, sizeof(struct wav_header), SEEK_SET);
            }

            header_4mic.riff_id = ID_RIFF;
            header_4mic.riff_sz = 0;
            header_4mic.riff_fmt = ID_WAVE;
            header_4mic.fmt_id = ID_FMT;
            header_4mic.fmt_sz = 16;
            header_4mic.audio_format = FORMAT_PCM;
            header_4mic.num_channels = 4;
            header_4mic.sample_rate = 16000;
            header_4mic.bits_per_sample = pcm_format_to_bits(PCM_FORMAT_S16_LE);
            header_4mic.byte_rate = (header_4mic.bits_per_sample / 8) * header_4mic.num_channels *
                                    header_4mic.sample_rate;
            header_4mic.block_align = header_4mic.num_channels * (header_4mic.bits_per_sample / 8);
            header_4mic.data_id = ID_DATA;

            header_2aec.riff_id = ID_RIFF;
            header_2aec.riff_sz = 0;
            header_2aec.riff_fmt = ID_WAVE;
            header_2aec.fmt_id = ID_FMT;
            header_2aec.fmt_sz = 16;
            header_2aec.audio_format = FORMAT_PCM;
            header_2aec.num_channels = 2;
            header_2aec.sample_rate = 16000;
            header_2aec.bits_per_sample = pcm_format_to_bits(PCM_FORMAT_S16_LE);
            header_2aec.byte_rate = (header_2aec.bits_per_sample / 8) * header_2aec.num_channels *
                                    header_2aec.sample_rate;
            header_2aec.block_align = header_2aec.num_channels * (header_2aec.bits_per_sample / 8);
            header_2aec.data_id = ID_DATA;

            header_out.riff_id = ID_RIFF;
            header_out.riff_sz = 0;
            header_out.riff_fmt = ID_WAVE;
            header_out.fmt_id = ID_FMT;
            header_out.fmt_sz = 16;
            header_out.audio_format = FORMAT_PCM;
            header_out.num_channels = (hal->env_cfg.no_vad) ? 1 : 2;
            header_out.sample_rate = 16000;
            header_out.bits_per_sample = pcm_format_to_bits(PCM_FORMAT_S16_LE);
            header_out.byte_rate = (header_out.bits_per_sample / 8) * header_out.num_channels *
                                   header_out.sample_rate;
            header_out.block_align = header_out.num_channels * (header_out.bits_per_sample / 8);
            header_out.data_id = ID_DATA;

            file_4mic_data_size = 0;
            file_2aec_data_size = 0;
            file_out_data_size = 0;
        } else {
            if (file_4mic) {
                header_4mic.data_sz = file_out_data_size * 2;
                header_4mic.riff_sz = header_4mic.data_sz + sizeof(header_4mic) - 8;
                fseek(file_4mic, 0, SEEK_SET);
                fwrite(&header_4mic, sizeof(struct wav_header), 1, file_4mic);

                fclose(file_4mic);
                file_4mic = NULL;
            }
            if (file_2aec) {
                header_2aec.data_sz = file_out_data_size * 2;
                header_2aec.riff_sz = header_2aec.data_sz + sizeof(header_2aec) - 8;
                fseek(file_2aec, 0, SEEK_SET);
                fwrite(&header_2aec, sizeof(struct wav_header), 1, file_2aec);

                fclose(file_2aec);
                file_2aec = NULL;
            }
            if (file_out) {
                header_out.data_sz = file_out_data_size;
                header_out.riff_sz = header_out.data_sz + sizeof(header_out) - 8;
                fseek(file_out, 0, SEEK_SET);
                fwrite(&header_out, sizeof(struct wav_header), 1, file_out);

                fclose(file_out);
                file_out = NULL;
            }

            file_4mic_data_size = 0;
            file_2aec_data_size = 0;
            file_out_data_size = 0;
        }
    }
    return 0;
}

#endif

int uni_4mic_pcm_close(void *handle) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) handle;
    int ret;
    void *status;

    HAL_PRINT_ERR("%s start\n", __func__);

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }


//pthread_mutex_destroy(&lock_uni); 

#ifdef ___DEBUG
    // debug_record_files(handle, 0);
#endif

    if (0 == hal->read_start) {
        //  uni_4mic_pcm_start(handle);
    }

    if (hal->mic_array_thread_running) {
        hal->mic_array_thread_running = 0;
        ret = pthread_join(hal->mic_array_thread, &status);
        if (ret) {
            HAL_PRINT_ERR("mic_array_thread pthread_join error, ret=%d, %s\n", ret,
                          strerror(errno));
        }
    }

    if (hal->pcm_read_thread_running) {
        hal->pcm_read_thread_running = 0;
        ret = pthread_join(hal->pcm_read_thread, &status);
        if (ret) {
            HAL_PRINT_ERR("pcm_read_thread pthread_join error, ret=%d, %s\n", ret, strerror(errno));
        }
    }

    if (hal->micArrayRingBuf.buf) {
        destroyRingBuf(&hal->micArrayRingBuf);
    }

    if (hal->RecordRingBuffer.buf) {
        destroyRingBuf(&hal->RecordRingBuffer);
    }

    if (hal->RecordRingBuffer_org.buf) {
        destroyRingBuf(&hal->RecordRingBuffer_org);
    }


    if (hal->pcm) {
        pcm_close(hal->pcm);
        hal->pcm = NULL;
    }

    if (!hal->use_4mic) {
        uni_4mic_hal_release();
    }

    HAL_PRINT_INFO("%s end\n", __func__);
    sleep(1);
    return 0;
}

void *uni_4mic_pcm_open(int ch_num) {
    struct uni_4mic_hal *hal = s_hal;
    int ret;
    struct pcm_config pcm_config;
    unsigned int card = unifindcard();
    unsigned int device = 0;
    unsigned int i2s_bits = 0;
    pthread_attr_t attr;
    struct sched_param param;
    void *pattr = NULL;


    int ringbuf_size;
    int min_rw_size;

    g_pcm_read_error = 0;
    pthread_attr_init(&attr);
    param.sched_priority = 1;
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);

    if (card == -1) {
        HAL_PRINT_ERR("%s: (NULL == hal  card not exist)\n", __func__);
        return NULL;

    }
    if (NULL == hal) {
        uni_4mic_hal_init(1);
        hal = s_hal;
    }

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return NULL;
    }

    hal->ch_num = ch_num;
    HAL_PRINT_ERR("zxl-ch_num=%d  tmp_flag %d  RAW_CHAN_NUM %d  RAW_SAMPLE_RATE %d\n", hal->ch_num,
                  tmp_flag, RAW_CHAN_NUM, RAW_SAMPLE_RATE);

    do {
        // card = hal->env_cfg.pcm_card;
        device = hal->env_cfg.pcm_device;
        i2s_bits = hal->env_cfg.i2s_bits;

        pcm_config.channels = 10;
        pcm_config.rate = 16000;
        pcm_config.period_size = 1024;
        pcm_config.period_count = 4;
        pcm_config.format = PCM_FORMAT_S16_LE;
        pcm_config.start_threshold = 0;
        pcm_config.stop_threshold = pcm_config.period_size * pcm_config.period_count;
        pcm_config.silence_threshold = 0;
        //  pcm_config.silence_size = 4294967295;
        pcm_config.silence_size = 2831482871;
        pcm_config.avail_min = 2831495184;
        //pcm_config.flag = -281656496;


//        pcm_config.channels = 10;
//        pcm_config.rate = 16000;
//        pcm_config.period_size = 1024;
//        pcm_config.period_count = 4;
//        pcm_config.format = PCM_FORMAT_S16_LE;
//        pcm_config.start_threshold = 0;
//        pcm_config.stop_threshold = 0;
//        pcm_config.silence_threshold = 0;

        HAL_PRINT_ERR("zxl-card=%d, device=%d, i2s_bits=%d RAW_CHAN_NUM=%d RAW_SAMPLE_RATE = %d\n",
                      card, device, i2s_bits, RAW_CHAN_NUM, RAW_SAMPLE_RATE);
        hal->pcm = pcm_open(2, 0, PCM_IN, &pcm_config);
        HAL_PRINT_ERR("pcm open finished");
        if (NULL == hal->pcm || !pcm_is_ready(hal->pcm)) {
            // char error_info[1024] = {0};
            HAL_PRINT_ERR("Unable to open PCM card=%d,device=%d, (%s)\n", card, device,
                          pcm_get_error(hal->pcm));
            if (g_unisound_event_callback != NULL) {
                g_unisound_event_callback(-9998); //busy

            }

            break;
        }
        HAL_PRINT_ERR("pcm open finished start next");
        if (hal->raw_pcm) {
            ringbuf_size = RECORD_BUF_CNT * 2 * SLOT_FRAMES_ALL_RAW_BYTES;
            min_rw_size = RAW_FRAME_BYTES;
        } else {
            ringbuf_size = PACKED_CHAN_NUM * RECORD_BUF_CNT * 2 * SLOT_FRAMES_PACKED_BYTES;
            min_rw_size = 4;
        }
        HAL_PRINT_ERR("[%s:%d] initRingBuf:MicArray 123\n", __func__, ringbuf_size);
//HAL_PRINT_ERR("[%s:%d] initRingBuf:record failed: %s  ringbuf_size %d \n", __func__, __LINE__, ringbuf_size  );

//RingBuf_t RecordRingBuffer;

        initRingBuf(&hal->RecordRingBuffer_org, "Recordorg", ringbuf_size, min_rw_size);


        ret = initRingBuf(&hal->RecordRingBuffer, "Record--", ringbuf_size, min_rw_size);
        if (0 > ret) {
            HAL_PRINT_ERR("[%s:%d] initRingBuf:record failed: %s\n", __func__, __LINE__);
            break;
        }

        ret = initRingBuf(&hal->micArrayRingBuf, "MicArray", ringbuf_size, 4);
        if (0 > ret) {
            HAL_PRINT_ERR("[%s:%d] initRingBuf:MicArray failed: %s\n", __func__, __LINE__);
            break;
        }
        HAL_PRINT_ERR("[%s:%d] raw thread: %s\n", __func__, __LINE__, "1");
        hal->read_start = 0;
        if (hal->env_cfg.high_priority) {
            pattr = (void *) &attr;
        } else {
            pattr = NULL;
        }
        HAL_PRINT_ERR("[%s:%d] raw thread: %s\n", __func__, __LINE__, "2");
        hal->pcm_read_thread_running = 1;
        if (hal->raw_pcm) {
            HAL_PRINT_ERR("[%s:%d] raw thread: %s\n", __func__, __LINE__, "3");
            ret = pthread_create(&hal->pcm_read_thread, NULL, pcm_read_raw_thread_proc, hal);
        } else {
            ret = pthread_create(&hal->pcm_read_thread, NULL, pcm_read_packed_thread_proc, hal);
        }
        if (ret) {
            HAL_PRINT_ERR("[%s:%d] failed to create pcm_read_thread: %s\n", __func__, __LINE__,
                          strerror(errno));
            hal->pcm_read_thread_running = 0;
            break;
        }

        hal->mic_array_thread_running = 1;
        ret = pthread_create(&hal->mic_array_thread, NULL, mic_array_thread_proc, hal);
        //  while(1){
        //  	sleep(1);
        // }
        if (ret) {
            HAL_PRINT_ERR("[%s:%d] failed to create mic_array_thread: %s\n", __func__, __LINE__,
                          strerror(errno));
            hal->mic_array_thread_running = 0;
            break;
        }

        // char filename8mic[256];
        //sprintf(filename8mic, "%s/file_raw_raw8mic.pcm", "/sdcard/unidata");
        // file_8mic = fopen(filename8mic, "wb+");

#ifdef ___DEBUG
//tmp_flag++;
//if(tmp_flag==1){
        //tmp_flag++;
        // debug_record_files(hal, 1);

        // }
#endif

        return hal;
    } while (0);

    uni_4mic_pcm_close(hal);
    return NULL;
}

int uni_4mic_pcm_read(void *handle, char *buf, unsigned int size) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) handle;
    int ret;

    if (log_g_4mic == 0) {
        log_g_4mic = 1;
        HAL_PRINT_ERR("%s: (NULL == hal) first pcm_read uni_4mic_pcm_read \n", __func__);
    }

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    if (hal != s_hal) {
        HAL_PRINT_ERR("%s: hal=%p, s_hal=%p.\n", __func__, hal, s_hal);
        return -1;
    }

    if (NULL == buf) {
        HAL_PRINT_ERR("%s: (NULL == buf)\n", __func__);
        return -1;
    }

    hal->read_start = 1;


    // HAL_PRINT_ERR("%s size=%d\n", __func__, size);
    ret = readBuf(&hal->micArrayRingBuf, buf, size);
    if (0 > ret) {
        HAL_PRINT_ERR("%s: failed\n ", __func__);
        return -1;
    }

    if (log_g_4mic_af == 0) {
        log_g_4mic_af = 1;
        HAL_PRINT_ERR("%s: (NULL == hal) first pcm_read uni_4mic_pcm_read  end\n", __func__);
    }


/*
    if(can_write==1){
    	int r=0;
      	
        r = write(g_fd ,buf, size);
        HAL_PRINT_ERR("%s: failed--20190706---%d ----\n ", __func__, r);
                	
    }
       */
    return ret;
}

int uni_4mic_pcm_start(void *handle) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) handle;
    int ret;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    if (hal != s_hal) {
        HAL_PRINT_ERR("%s: hal=%p, s_hal=%p.\n", __func__, hal, s_hal);
        return -1;
    }

    HAL_PRINT_INFO("%s\n", __func__);

    ClearRingBuf(&hal->RecordRingBuffer);
    ClearRingBuf(&hal->micArrayRingBuf);

    /*
    ret = pcm_start(hal->pcm);
    if(0 > ret){
        HAL_PRINT_ERR("%s: failed.\n", __func__);
        return -1;
    }
    */

    hal->read_start = 1;

    return 0;
}

int uni_4mic_pcm_stop(void *handle) {
    struct uni_4mic_hal *hal = (struct uni_4mic_hal *) handle;
    int ret;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    if (hal != s_hal) {
        HAL_PRINT_ERR("%s: hal=%p, s_hal=%p.\n", __func__, hal, s_hal);
        return -1;
    }

    HAL_PRINT_INFO("%s\n", __func__);

    hal->read_start = 0;

    /*    
    ret = pcm_stop(hal->pcm);
    if(0 > ret){
        HAL_PRINT_ERR("%s: failed.\n", __func__);
        return -1;
    }
    */

    ResetRingBuf(&hal->RecordRingBuffer);
    ResetRingBuf(&hal->micArrayRingBuf);
    ResetRingBuf(&hal->RecordRingBuffer_org);

    return 0;
}


int set4MicWakeUpStatus(int status) {
    struct uni_4mic_hal *hal = s_hal;
    float time_delay;
    float time_len;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->is_waked = !!status;
#if 0
    if(hal->is_waked){
        time_delay = (float)(hal->delayTime);
        time_len = (float)(hal->utteranceTime);
    
        //uni_hal_4mic_array_set(hal->MicHandle, UNI_HAL_MICARRAY_WAKE_UP_TIME_DELAY, (void *)(&time_delay));
        //uni_hal_4mic_array_set(hal->MicHandle, UNI_HAL_MICARRAY_WAKE_UP_TIME_LEN, (void *)&time_len);
        
        uni_hal_4mic_array_compute_DOA(hal->MicHandle, time_len, time_delay);
        uni_hal_4mic_array_get(hal->MicHandle, UNI_HAL_MICARRAY_DOA_RESULT, (void *)&hal->DoaResult);
        
        HAL_PRINT_ERR("%s: (NULL == hal) 20190524  %d\n", __func__, (void *)&hal->DoaResult);
    }
    uni_hal_4mic_array_reset (hal->MicHandle);
#endif
    uni_hal_4mic_array_reset(hal->MicHandle);

    return 0;
}

int set4MicUtteranceTimeLen(int length) //adev_set_4mic_doatimelen
{
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->utteranceTime = length;
    HAL_PRINT_ERR("%s: (NULL == hal) length %d \n", __func__, length);

    return 0;
}

int set4MicDelayTime(int delayTime) //adev_set_4mic_doatimedelay
{
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->delayTime = delayTime;
    HAL_PRINT_ERR("%s: (NULL == hal)  delayTime %d\n", __func__, delayTime);
    return 0;
}

int close4MicAlgorithm(int status) {
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->is_4mic_closed = !!status;

    return 0;
}

int set4MicOneShotStartLen(int startTimeLen) {
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->OneShotStartLen = startTimeLen;

    return 0;
}

int set4MicWakeupStartLen(int startTimeLen) {
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->WakeupStartLen = startTimeLen;

    return 0;
}

int set4MicOneShotReady(int status) {
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->OneShotReady = status;

    return 0;
}

int get4MicOneShotReady(void) {
    struct uni_4mic_hal *hal = s_hal;

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }


    return hal->OneShotReady;
}

int get4MicRecordercount(void) {

    //HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);


    return 0;
}

int get4MicDoaResult(void) {
    struct uni_4mic_hal *hal = s_hal;
    float time_delay;
    float time_len;
    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }


    time_delay = (float) (hal->delayTime) + 150; //pro DOA  buffer latency
    time_len = (float) (hal->utteranceTime);

    //uni_hal_4mic_array_set(hal->MicHandle, UNI_HAL_MICARRAY_WAKE_UP_TIME_DELAY, (void *)(&time_delay));
    //uni_hal_4mic_array_set(hal->MicHandle, UNI_HAL_MICARRAY_WAKE_UP_TIME_LEN, (void *)&time_len);

    uni_hal_4mic_array_compute_DOA(hal->MicHandle, time_len, time_delay);
    uni_hal_4mic_array_get(hal->MicHandle, UNI_HAL_MICARRAY_DOA_RESULT, (void *) &hal->DoaResult);

    //   HAL_PRINT_ERR("%s: (NULL == hal) 20190605  %d  time_delay %f time_len %f\n", __func__, (void *)&hal->DoaResult, time_delay , time_len);

    uni_hal_4mic_array_reset(hal->MicHandle);


    HAL_PRINT_ERR("%s:  %d\n", __func__, hal->DoaResult);

    return hal->DoaResult;
}

char *get4MicBoardVersion(void) {
    char *ver = UNI_4MIC_VERSION;

    return ver;
}

int set4MicDebugMode(int debugMode) {
    struct uni_4mic_hal *hal = s_hal;
    int debug_level;

    if (debugMode) {
        debug_level = HAL_DBG_LEVEL_INFO;
    } else {
        debug_level = HAL_DBG_LEVEL_ERR;
    }
    uni_4mic_set_debug_level(debug_level);

    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }

    hal->debugMode = debugMode;

    return 0;
}

int unifindcard() {
    //int fd = 0;
    FILE *fd;
    int card_in = 0;
    char cardstring[128] = {0};
    char info[1024] = {0};
    for (; card_in < 10; card_in++) {
        memset(cardstring, 0x00, 128);
        sprintf(cardstring, "/proc/asound/card%d/stream0", card_in);

        //fd = open(cardstring, O_RDONLY);
        fd = fopen(cardstring, "r");
        HAL_PRINT_INFO("%s fd %d card_in %d\n", cardstring, fd, card_in);
        //if(fd>0 || fd==0){
        // 	  HAL_PRINT_INFO("card %d \n", card_in);
        // 	  break;
        //}
        //int num;
        //read(fd,&info,1024);
        if (fd != NULL) {
            fread(info, 1, 1024, fd);
            HAL_PRINT_ERR("card %d info %s \n", card_in, info);
            char *tp;
            tp = strstr(info, "Channels: 10");

            if (tp == NULL) {
                HAL_PRINT_INFO("not find card Channels: 10 \n");
                //return -1;
            } else {
                HAL_PRINT_INFO("find card Channels: 10 \n");
                fclose(fd);
                break;
            }
            fclose(fd);

        }
    }

    if (card_in > 8) {

        if (g_unisound_event_callback != NULL) {
            g_unisound_event_callback(-9999); //����������   usb soundcard not exist!
            return -1;
        }
    }


    return card_in;
}

int set4MicSSLON(int status) {
    struct uni_4mic_hal *hal = s_hal;
    int val;


    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }
    val = status;
    //UNI_MICARRAY_SSL_ON  301
    uni_hal_4mic_array_set(hal->MicHandle, 301, (void *) (&val));
    HAL_PRINT_INFO("set4MicSSLON %d\n", status);
    //uni_hal_4mic_array_reset (hal->MicHandle); 

    return 0;
}

int set_enhance_angle(int status) {
    struct uni_4mic_hal *hal = s_hal;
    int val;


    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }
    val = status;
    //UNI_MICARRAY_SSL_ON  301
    uni_hal_4mic_array_set_org(hal->MicHandle, 111, (void *) (&val));
    HAL_PRINT_ERR("zxl-set_enhance_angle %d\n", status);
    //uni_hal_4mic_array_reset (hal->MicHandle); 

    return 0;
}

int set_bias_angle(int status) {
    struct uni_4mic_hal *hal = s_hal;
    int val;


    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }
    val = status;
    //UNI_MICARRAY_SSL_ON  301
    uni_hal_4mic_array_set_org(hal->MicHandle, 1010, (void *) (&val));
    HAL_PRINT_ERR("zxl-set_bias_angle %d\n", status);
    //uni_hal_4mic_array_reset (hal->MicHandle); 

    return 0;
}

int get4MicDoaResult_10ch(float time_len, float time_delay) {
    struct uni_4mic_hal *hal = s_hal;
    int val;
    int peak_angle;


    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }
    // val = status;

    //#define UAL_FDE_DOA               110
    //  uni_hal_4mic_array_set_org(hal->MicHandle, 110, (void*)(&val));

    pthread_mutex_lock(&lock_uni);

    g_pcm_test_flag = 1;

    uni_hal_4mic_array_compute_DOA(hal->MicHandle, time_len, time_delay);
    uni_hal_4mic_array_get_org(hal->MicHandle, 110, (void *) (&val));

    //#define UAL_FDE_ANGLE_PEAK        1011
    uni_hal_4mic_array_get_org(hal->MicHandle, 1011, &(peak_angle));
    //   HAL_PRINT_ERR("zxl-%s: val %d  %f  %f  %d\n", __func__, val,time_len , time_delay , peak_angle);
    ual_fixed_direct_reset(hal->MicHandle);
    pthread_mutex_unlock(&lock_uni);

    return peak_angle;
}

int get4MicDoaResult_v1_10ch(float time_len, float time_delay) {
    struct uni_4mic_hal *hal = s_hal;
    int val;
    int peak_angle;


    if (NULL == hal) {
        HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
        return -1;
    }
    // val = status;

    //#define UAL_FDE_DOA               110
    //  uni_hal_4mic_array_set_org(hal->MicHandle, 110, (void*)(&val));

    pthread_mutex_lock(&lock_uni);

    uni_hal_4mic_array_compute_DOA(hal->MicHandle, time_len, time_delay);
    uni_hal_4mic_array_get_org(hal->MicHandle, 110, (void *) (&val));

    //#define UAL_FDE_ANGLE_PEAK        1011
    uni_hal_4mic_array_get_org(hal->MicHandle, 1011, &(peak_angle));
    //  HAL_PRINT_ERR("zxl-%s: val %d  %f  %f  %d\n", __func__, val,time_len , time_delay , peak_angle);
    // ual_fixed_direct_reset(hal->MicHandle);
    pthread_mutex_unlock(&lock_uni);

    return peak_angle;
}


void uni_set_configpath(const char *cfg_path) {

    HAL_PRINT_INFO("uni_set_configpath %s\n", cfg_path);
    g_set_path = 1;

    sprintf(g_path, "%s", cfg_path);

}

int uni_4mic_pcm_read_org(char *buf, unsigned int size) {
    // struct uni_4mic_hal *hal = (struct uni_4mic_hal *)handle;

    struct uni_4mic_hal *hal = s_hal;
    int ret;

    //  if(NULL == hal){
    //      HAL_PRINT_ERR("%s: (NULL == hal)\n", __func__);
    //     return -1;
    // }

    //   if(hal != s_hal){
    //      HAL_PRINT_ERR("%s: hal=%p, s_hal=%p.\n", __func__, hal, s_hal);
    //      return -1;
    //  }

    if (NULL == buf) {
        HAL_PRINT_ERR("%s: (NULL == buf)\n", __func__);
        return -1;
    }

    hal->read_start = 1;

    // HAL_PRINT_ERR("%s size=%d\n", __func__, size);
    ret = readBuf(&hal->RecordRingBuffer_org, buf, size);
    if (0 > ret) {
        HAL_PRINT_ERR("%s: failed\n ", __func__);
        return -1;
    }

/*
    if(can_write==1){
    	int r=0;
      	
        r = write(g_fd ,buf, size);
        HAL_PRINT_ERR("%s: failed--20190706---%d ----\n ", __func__, r);
                	
    }
       */
    return ret;
}

int uni_michal_event_register_callback(UNISOUND_EVENT_CALLBACK callback) {
    HAL_PRINT_ERR("UNISOUND_EVENT_CALLBACK\n");
    g_unisound_event_callback = callback;
    return 0;
}
