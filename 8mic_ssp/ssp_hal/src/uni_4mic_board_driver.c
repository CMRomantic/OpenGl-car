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

#include "uni_4mic_hal.h"


#ifdef DO_BOARD_INIT
extern void uni_4mic_i2c_slave_id(int slave_id);
extern int uni_4mic_i2c_open(char *dev_name, int slave_id, int timeout, int retry);
extern int uni_4mic_i2c_close(void);
extern int uni_4mic_i2c_read_reg(uint16_t reg, uint16_t* val);
extern int uni_4mic_i2c_write_reg(uint16_t reg, uint16_t val);
extern int uni_4mic_i2c_update_bits(uint16_t reg, uint16_t mask, uint16_t val);
extern int uni_4mic_i2c_read_buf(uint8_t *buf, int size);
extern int uni_4mic_i2c_write_buf(uint8_t *buf, int size);


#define NAU8540_REG_SW_RESET			0x00
#define NAU8540_REG_POWER_MANAGEMENT	0x01
#define NAU8540_REG_CLOCK_CTRL		0x02
#define NAU8540_REG_CLOCK_SRC			0x03
#define NAU8540_REG_FLL1			0x04
#define NAU8540_REG_FLL2			0x05
#define NAU8540_REG_FLL3			0x06
#define NAU8540_REG_FLL4			0x07
#define NAU8540_REG_FLL5			0x08
#define NAU8540_REG_FLL6			0x09
#define NAU8540_REG_FLL_VCO_RSV		0x0A
#define NAU8540_REG_PCM_CTRL0			0x10
#define NAU8540_REG_PCM_CTRL1			0x11
#define NAU8540_REG_PCM_CTRL2			0x12
#define NAU8540_REG_PCM_CTRL3			0x13
#define NAU8540_REG_PCM_CTRL4			0x14
#define NAU8540_REG_ALC_CONTROL_1		0x20
#define NAU8540_REG_ALC_CONTROL_2		0x21
#define NAU8540_REG_ALC_CONTROL_3		0x22
#define NAU8540_REG_ALC_CONTROL_4		0x23
#define NAU8540_REG_ALC_CONTROL_5		0x24
#define NAU8540_REG_ALC_GAIN_CH12		0x2D
#define NAU8540_REG_ALC_GAIN_CH34		0x2E
#define NAU8540_REG_ALC_STATUS		0x2F
#define NAU8540_REG_NOTCH_FIL1_CH1		0x30
#define NAU8540_REG_NOTCH_FIL2_CH1		0x31
#define NAU8540_REG_NOTCH_FIL1_CH2		0x32
#define NAU8540_REG_NOTCH_FIL2_CH2		0x33
#define NAU8540_REG_NOTCH_FIL1_CH3		0x34
#define NAU8540_REG_NOTCH_FIL2_CH3		0x35
#define NAU8540_REG_NOTCH_FIL1_CH4		0x36
#define NAU8540_REG_NOTCH_FIL2_CH4		0x37
#define NAU8540_REG_HPF_FILTER_CH12		0x38
#define NAU8540_REG_HPF_FILTER_CH34		0x39
#define NAU8540_REG_ADC_SAMPLE_RATE		0x3A
#define NAU8540_REG_DIGITAL_GAIN_CH1		0x40
#define NAU8540_REG_DIGITAL_GAIN_CH2		0x41
#define NAU8540_REG_DIGITAL_GAIN_CH3		0x42
#define NAU8540_REG_DIGITAL_GAIN_CH4		0x43
#define NAU8540_REG_DIGITAL_MUX		0x44
#define NAU8540_REG_P2P_CH1			0x48
#define NAU8540_REG_P2P_CH2			0x49
#define NAU8540_REG_P2P_CH3			0x4A
#define NAU8540_REG_P2P_CH4			0x4B
#define NAU8540_REG_PEAK_CH1			0x4C
#define NAU8540_REG_PEAK_CH2			0x4D
#define NAU8540_REG_PEAK_CH3			0x4E
#define NAU8540_REG_PEAK_CH4			0x4F
#define NAU8540_REG_GPIO_CTRL			0x50
#define NAU8540_REG_MISC_CTRL			0x51
#define NAU8540_REG_I2C_CTRL			0x52
#define NAU8540_REG_I2C_DEVICE_ID		0x58
#define NAU8540_REG_RST			0x5A
#define NAU8540_REG_VMID_CTRL			0x60
#define NAU8540_REG_MUTE			0x61
#define NAU8540_REG_ANALOG_ADC1		0x64
#define NAU8540_REG_ANALOG_ADC2		0x65
#define NAU8540_REG_ANALOG_PWR		0x66
#define NAU8540_REG_MIC_BIAS			0x67
#define NAU8540_REG_REFERENCE			0x68
#define NAU8540_REG_FEPGA1			0x69
#define NAU8540_REG_FEPGA2			0x6A
#define NAU8540_REG_FEPGA3			0x6B
#define NAU8540_REG_FEPGA4			0x6C
#define NAU8540_REG_PWR			0x6D
#define NAU8540_REG_MAX			NAU8540_REG_PWR


/* POWER_MANAGEMENT (0x01) */
#define NAU8540_ADC4_EN		(0x1 << 3)
#define NAU8540_ADC3_EN		(0x1 << 2)
#define NAU8540_ADC2_EN		(0x1 << 1)
#define NAU8540_ADC1_EN		0x1

/* CLOCK_CTRL (0x02) */
#define NAU8540_CLK_ADC_EN		(0x1 << 15)
#define NAU8540_CLK_I2S_EN		(0x1 << 1)

/* CLOCK_SRC (0x03) */
#define NAU8540_CLK_SRC_SFT		15
#define NAU8540_CLK_SRC_MASK		(1 << NAU8540_CLK_SRC_SFT)
#define NAU8540_CLK_SRC_VCO		(1 << NAU8540_CLK_SRC_SFT)
#define NAU8540_CLK_SRC_MCLK		(0 << NAU8540_CLK_SRC_SFT)
#define NAU8540_CLK_ADC_SRC_SFT	6
#define NAU8540_CLK_ADC_SRC_MASK	(0x3 << NAU8540_CLK_ADC_SRC_SFT)
#define NAU8540_CLK_MCLK_SRC_MASK	0xf

/* FLL1 (0x04) */
#define NAU8540_FLL_RATIO_MASK	0x7f

/* FLL3 (0x06) */
#define NAU8540_FLL_CLK_SRC_SFT	10
#define NAU8540_FLL_CLK_SRC_MASK	(0x3 << NAU8540_FLL_CLK_SRC_SFT)
#define NAU8540_FLL_CLK_SRC_MCLK	(0 << NAU8540_FLL_CLK_SRC_SFT)
#define NAU8540_FLL_CLK_SRC_BLK	(0x2 << NAU8540_FLL_CLK_SRC_SFT)
#define NAU8540_FLL_CLK_SRC_FS		(0x3 << NAU8540_FLL_CLK_SRC_SFT)
#define NAU8540_FLL_INTEGER_MASK	0x3ff

/* FLL4 (0x07) */
#define NAU8540_FLL_REF_DIV_SFT	10
#define NAU8540_FLL_REF_DIV_MASK	(0x3 << NAU8540_FLL_REF_DIV_SFT)

/* FLL5 (0x08) */
#define NAU8540_FLL_PDB_DAC_EN	(0x1 << 15)
#define NAU8540_FLL_LOOP_FTR_EN	(0x1 << 14)
#define NAU8540_FLL_CLK_SW_MASK	(0x1 << 13)
#define NAU8540_FLL_CLK_SW_N2		(0x1 << 13)
#define NAU8540_FLL_CLK_SW_REF	(0x0 << 13)
#define NAU8540_FLL_FTR_SW_MASK	(0x1 << 12)
#define NAU8540_FLL_FTR_SW_ACCU	(0x1 << 12)
#define NAU8540_FLL_FTR_SW_FILTER	(0x0 << 12)

/* FLL6 (0x9) */
#define NAU8540_DCO_EN			(0x1 << 15)
#define NAU8540_SDM_EN			(0x1 << 14)

/* PCM_CTRL0 (0x10) */
#define NAU8540_I2S_BP_SFT		7
#define NAU8540_I2S_BP_INV		(0x1 << NAU8540_I2S_BP_SFT)
#define NAU8540_I2S_PCMB_SFT		6
#define NAU8540_I2S_PCMB_EN		(0x1 << NAU8540_I2S_PCMB_SFT)
#define NAU8540_I2S_DL_SFT		2
#define NAU8540_I2S_DL_MASK		(0x3 << NAU8540_I2S_DL_SFT)
#define NAU8540_I2S_DL_16		(0 << NAU8540_I2S_DL_SFT)
#define NAU8540_I2S_DL_20		(0x1 << NAU8540_I2S_DL_SFT)
#define NAU8540_I2S_DL_24		(0x2 << NAU8540_I2S_DL_SFT)
#define NAU8540_I2S_DL_32		(0x3 << NAU8540_I2S_DL_SFT)
#define NAU8540_I2S_DF_MASK		0x3
#define NAU8540_I2S_DF_RIGTH		0
#define NAU8540_I2S_DF_LEFT		0x1
#define NAU8540_I2S_DF_I2S		0x2
#define NAU8540_I2S_DF_PCM_AB		0x3

/* PCM_CTRL1 (0x11) */
#define NAU8540_I2S_LRC_DIV_SFT	12
#define NAU8540_I2S_LRC_DIV_MASK	(0x3 << NAU8540_I2S_LRC_DIV_SFT)
#define NAU8540_I2S_DO12_OE		(0x1 << 4)
#define NAU8540_I2S_MS_SFT		3
#define NAU8540_I2S_MS_MASK		(0x1 << NAU8540_I2S_MS_SFT)
#define NAU8540_I2S_MS_MASTER		(0x1 << NAU8540_I2S_MS_SFT)
#define NAU8540_I2S_MS_SLAVE		(0x0 << NAU8540_I2S_MS_SFT)
#define NAU8540_I2S_BLK_DIV_MASK	0x7

/* PCM_CTRL1 (0x12) */
#define NAU8540_I2S_DO34_OE		(0x1 << 11)
#define NAU8540_I2S_TSLOT_L_MASK	0x3ff

/* PCM_CTRL4 (0x14) */
#define NAU8540_TDM_MODE		(0x1 << 15)
#define NAU8540_TDM_OFFSET_EN		(0x1 << 14)
#define NAU8540_TDM_TX_MASK		0xf

/* ADC_SAMPLE_RATE (0x3A) */
#define NAU8540_ADC_OSR_MASK		0x3
#define NAU8540_ADC_OSR_256		0x3
#define NAU8540_ADC_OSR_128		0x2
#define NAU8540_ADC_OSR_64		0x1
#define NAU8540_ADC_OSR_32		0x0

/* VMID_CTRL (0x60) */
#define NAU8540_VMID_EN		(1 << 6)
#define NAU8540_VMID_SEL_SFT		4
#define NAU8540_VMID_SEL_MASK		(0x3 << NAU8540_VMID_SEL_SFT)

/* MIC_BIAS (0x67) */
#define NAU8540_PU_PRE			(0x1 << 8)

/* REFERENCE (0x68) */
#define NAU8540_PRECHARGE_DIS		(0x1 << 13)
#define NAU8540_GLOBAL_BIAS_EN	(0x1 << 12)


/* System Clock Source */
enum {
	NAU8540_CLK_DIS,
	NAU8540_CLK_MCLK,
	NAU8540_CLK_INTERNAL,
	NAU8540_CLK_FLL_MCLK,
	NAU8540_CLK_FLL_BLK,
	NAU8540_CLK_FLL_FS,
};

/* Programmable divider in master mode */
enum {
	NAU8540_BCLKDIV,
	NAU8540_FSDIV,
};


static int nau8540_dapm_ctrl(void)
{
    uint16_t tmp;

    uni_4mic_i2c_write_reg(0x10, 0x02);
    uni_4mic_i2c_write_reg(0x11, 0x3000);
    uni_4mic_i2c_write_reg(0x12, 0x00);
    uni_4mic_i2c_write_reg(0x03, 0x80);

    uni_4mic_i2c_write_reg(0x67, 0x0d04);
    uni_4mic_i2c_write_reg(0x66, 0x0f);
    uni_4mic_i2c_write_reg(0x6d, 0xf000);
    uni_4mic_i2c_write_reg(0x01, 0x0f);
    
    return 0;
}

static void nau8540_init_regs(void)
{
	/* Enable Bias/VMID/VMID Tieoff */
    uni_4mic_i2c_update_bits(NAU8540_REG_VMID_CTRL, NAU8540_VMID_EN | NAU8540_VMID_SEL_MASK, NAU8540_VMID_EN | (0x2 << NAU8540_VMID_SEL_SFT));
    uni_4mic_i2c_update_bits(NAU8540_REG_REFERENCE, NAU8540_PRECHARGE_DIS | NAU8540_GLOBAL_BIAS_EN, NAU8540_PRECHARGE_DIS | NAU8540_GLOBAL_BIAS_EN);
    
    usleep(2000);
    
    uni_4mic_i2c_update_bits(NAU8540_REG_MIC_BIAS, NAU8540_PU_PRE, NAU8540_PU_PRE);
    uni_4mic_i2c_update_bits(NAU8540_REG_CLOCK_CTRL, NAU8540_CLK_ADC_EN | NAU8540_CLK_I2S_EN, NAU8540_CLK_ADC_EN | NAU8540_CLK_I2S_EN);
    uni_4mic_i2c_write_reg(NAU8540_REG_HPF_FILTER_CH12, 0x1010);
    uni_4mic_i2c_write_reg(NAU8540_REG_HPF_FILTER_CH34, 0x1010);

    /* ADC OSR selection, CLK_ADC = Fs * OSR */
    uni_4mic_i2c_update_bits(NAU8540_REG_ADC_SAMPLE_RATE, NAU8540_ADC_OSR_MASK, NAU8540_ADC_OSR_64);
    
    uni_4mic_i2c_update_bits(NAU8540_REG_DIGITAL_GAIN_CH1, 0x7ff, 0x520);
    uni_4mic_i2c_update_bits(NAU8540_REG_DIGITAL_GAIN_CH2, 0x7ff, 0x520);
    uni_4mic_i2c_update_bits(NAU8540_REG_DIGITAL_GAIN_CH3, 0x7ff, 0x520);
    uni_4mic_i2c_update_bits(NAU8540_REG_DIGITAL_GAIN_CH4, 0x7ff, 0x520);
}

static void nau8540_reset_chip(void)
{
    uni_4mic_i2c_write_reg(NAU8540_REG_SW_RESET, 0x00);
    uni_4mic_i2c_write_reg(NAU8540_REG_SW_RESET, 0x00);
}

static void nau8540_reg_dump(void)
{
    int reg;
    uint16_t val;

    for(reg = 0; reg <= 0x006D; reg++){
        if(0 == reg
            || (0x0b <= reg && 0x0f >= reg)
            || (0x15 <= reg && 0x1f >= reg)
            || (0x25 <= reg && 0x2c >= reg)
            || (0x3b <= reg && 0x3f >= reg)
            || (0x45 <= reg && 0x47 >= reg)
            || (0x53 <= reg && 0x56 >= reg)
            || (0x5a <= reg && 0x5f >= reg)
            ){
            continue;
        }
        uni_4mic_i2c_read_reg(reg, &val);
        printf("%04x, %04x\n", reg, val);
    }
}

static int nau8540_init(const char *i2c_dev)
{
    int ret = 0;
    int val = 0;

    ret = uni_4mic_i2c_open((char *)i2c_dev, 0x1c, 1, 3);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_i2c_open failed, i2c_dev=%s\n", __func__, i2c_dev);
        return -1;
    }

    do{   
        nau8540_reset_chip();

        ret = uni_4mic_i2c_read_reg(NAU8540_REG_I2C_DEVICE_ID, (uint16_t*)&val);
        if(0 > ret){
            HAL_PRINT_ERR("%s: get device id failed\n", __func__);
            ret = -1;
            break;
        }
        HAL_PRINT_INFO("nau8540 device id=0x%x\n", val);

        nau8540_init_regs();
        nau8540_dapm_ctrl();
        //nau8540_reg_dump();

        ret = 0;
    }while(0);

    uni_4mic_i2c_close();
    return ret;
}

static int read_fpga_chip_uid(const char *i2c_dev, uint64_t *uid)
{
    int ret;
    uint8_t buf[8];
    
    ret = uni_4mic_i2c_open((char *)i2c_dev, 0x40, 1, 3);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_i2c_open failed, i2c_dev=%s\n", __func__, i2c_dev);
        return -1;
    }

    do{
        buf[0] = 0x19;
        buf[1] = 0x00;
        buf[2] = 0x00;
        buf[3] = 0x00;
        ret = uni_4mic_i2c_write_buf(buf, 4);
        if(0 > ret){
            HAL_PRINT_ERR("%s: write cmd failed\n", __func__);
            ret = -1;
            break;
        }
        
        memset(buf, 0, sizeof(buf));
        ret = uni_4mic_i2c_read_buf(buf, 8);
        if(0 > ret){
            HAL_PRINT_ERR("%s: read data failed\n", __func__);
            ret = -1;
            break;
        }
        HAL_PRINT_INFO("uid:%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x.\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

        if(uid){
            memcpy((void *)uid, buf, 8);
        }

        ret = 0;
    }while(0);
    
    uni_4mic_i2c_close();
    return ret;
}

static int uni_4mic_board_hw_reset(int reset_pin, int status)
{
    int ret;
    char cmd[128];
    
    snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export", reset_pin);
    ret = system(cmd);
    if(0 > ret){
        HAL_PRINT_ERR("%s: [%s:%d] failed, %s\n", __func__, __LINE__, cmd, strerror(errno));
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "echo out > /sys/class/gpio/gpio%d/direction", reset_pin);
    ret = system(cmd);
    if(0 > ret){
        HAL_PRINT_ERR("%s: [%s:%d] failed, %s\n", __func__, __LINE__, cmd, strerror(errno));
        return -1;
    }
    
    snprintf(cmd, sizeof(cmd), "echo %d >  /sys/class/gpio/gpio%d/value", !!status, reset_pin);
    ret = system(cmd);
    if(0 > ret){
        HAL_PRINT_ERR("%s: [%s:%d] failed, %s\n", __func__, __LINE__, cmd, strerror(errno));
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/unexport", reset_pin);
    ret = system(cmd);
    if(0 > ret){
        HAL_PRINT_ERR("%s: [%s:%d] failed, %s\n", __func__, __LINE__, cmd, strerror(errno));
        return -1;
    }    
    
    return 0;
}

int uni_4mic_env_cfg_init(struct env_4mic_cfg *env_cfg)
{
    if(NULL == env_cfg){
        HAL_PRINT_ERR("%s: (NULL == env_cfg)\n", __func__);
        return -1;
    }

#ifdef DO_BOARD_RESET
    env_cfg->reset_gpio = 11;
#endif
    env_cfg->i2c_dev = "/dev/i2c-3";
    env_cfg->mic_array_cfg_path = "/system/usr/uni_4mic_config";
    strncpy(env_cfg->mic_array_rw_path, "/sdcard", sizeof(env_cfg->mic_array_rw_path));

    strncpy(env_cfg->debug_file_path, "/data/unidata", sizeof(env_cfg->debug_file_path));
    env_cfg->debug_file_4mic_name = "file_8mic.pcm";
    env_cfg->debug_file_2aec_name = "file_2aec.pcm";
    env_cfg->debug_file_out_name = "file_out.pcm";
    
    return 0;
}

int uni_4mic_board_init(struct env_4mic_cfg *env_cfg)
{
    int ret = 0;
    
    ret = uni_4mic_env_cfg_init(env_cfg);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_env_cfg_init failed.\n", __func__);
        return -1;
    }

#ifdef DO_BOARD_RESET
    ret = uni_4mic_board_hw_reset(env_cfg->reset_gpio, 1);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_board_hw_reset failed\n", __func__);
        return -1;
    }
#endif

    ret = nau8540_init(env_cfg->i2c_dev);
    if(0 > ret){
        HAL_PRINT_ERR("%s: nau8540_init failed\n", __func__);
        return -1;
    }
    
    ret = read_fpga_chip_uid(env_cfg->i2c_dev, NULL);
    if(0 > ret){
        HAL_PRINT_ERR("%s: read_fpga_chip_uid failed\n", __func__);
        return -1;
    }
    
    return 0;
}

int uni_4mic_board_deinit(struct env_4mic_cfg *env_cfg)
{
    int ret = 0;
    
    if(NULL == env_cfg){
        HAL_PRINT_ERR("%s: (NULL == env_cfg)\n", __func__);
        return -1;
    }

#ifdef DO_BOARD_RESET
    ret = uni_4mic_board_hw_reset(env_cfg->reset_gpio, 0);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_board_hw_reset failed\n", __func__);
        return -1;
    }
#endif
    
    return 0;
}

#else
int uni_4mic_env_cfg_init(struct env_4mic_cfg *env_cfg)
{
    if(NULL == env_cfg){
        HAL_PRINT_ERR("%s: (NULL == env_cfg)\n", __func__);
        return -1;
    }

    env_cfg->mic_array_cfg_path = "/system/usr/uni_4mic_config";
    strncpy(env_cfg->mic_array_rw_path, "/sdcard", sizeof(env_cfg->mic_array_rw_path));

    strncpy(env_cfg->debug_file_path, "/data/unidata", sizeof(env_cfg->debug_file_path));
    env_cfg->debug_file_4mic_name = "file_8mic.pcm";
    env_cfg->debug_file_2aec_name = "file_2aec.pcm";
    env_cfg->debug_file_out_name = "file_out.pcm";
    
    return 0;
}

int uni_4mic_board_init(struct env_4mic_cfg *env_cfg)
{
    int ret = 0;
    
    ret = uni_4mic_env_cfg_init(env_cfg);
    if(0 > ret){
        HAL_PRINT_ERR("%s: uni_4mic_env_cfg_init failed.\n", __func__);
        return -1;
    }

    return 0;
}

int uni_4mic_board_deinit(struct env_4mic_cfg *env_cfg)
{
    int ret = 0;
    
    if(NULL == env_cfg){
        HAL_PRINT_ERR("%s: (NULL == env_cfg)\n", __func__);
        return -1;
    }
    
    return 0;
}

#endif

