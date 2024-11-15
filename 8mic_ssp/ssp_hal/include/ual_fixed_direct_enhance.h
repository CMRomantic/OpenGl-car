#ifndef _UAL_FIXED_DIRECT_ENHANCE_INCLUDED_H_
#define _UAL_FIXED_DIRECT_ENHANCE_INCLUDED_H_

typedef void* FDHandle;

#define UAL_FDE_AEC_ON            1
#define UAL_FDE_ENV_ON            2
#define UAL_FDE_FBF_ON            3
#define UAL_FDE_ECHO_NUM          10
#define UAL_FDE_MIC_NUM           11
#define UAL_FDE_AEC_FILTER_NUM    100
#define UAL_FDE_AEC_NSHIFT        101
#define UAL_FDE_DOA               110
#define UAL_FDE_ENHANCE_ANGLE     111
#define UAL_FDE_BIAS_ANGLE        1010
#define UAL_FDE_ANGLE_PEAK        1011
#define UAL_FDE_DAMAGED_MIC_DETECT 1100
#define UAL_FDE_DAMAGED_MIC_DETECTED_TIME 1101
#define UAL_FDE_MODE_CHANGE       1110
#define UAL_FDE_PROTECT_TIME      1111
#define UAL_FDE_AEC_MORE          10001
#define UAL_FDE_DISTANCE          10011

#ifdef __cplusplus
extern "C"
{
#endif

FDHandle ual_fixed_direct_init(const char* cfg_path);

int ual_fixed_direct_process(FDHandle handle, const short* mic_data, const short* echo_data, int in_len, short** out_data, int* out_len);

int ual_fixed_direct_compute_doa(FDHandle handle, float time_len, float time_delay);

void ual_fixed_direct_get(FDHandle handle, int type, void* value);

void ual_fixed_direct_reset(FDHandle handle);

void ual_fixed_direct_set(FDHandle handle, int type, void* value);

void ual_fixed_direct_release(FDHandle handle);

const char* ual_fixed_direct_version();

#ifdef __cplusplus
}

#endif
#endif
