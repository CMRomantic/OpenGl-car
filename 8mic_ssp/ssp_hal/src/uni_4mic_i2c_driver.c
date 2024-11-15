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


static char *i2c_dev = NULL;
static int i2c_slave_id = 0;
static int i2c_fd = -1;


struct i2c_msg_ext {
	uint16_t addr;	/* slave address			*/
	uint16_t flags;
	uint16_t len;		/* msg length				*/
	uint8_t *buf;		/* pointer to msg data			*/
	uint32_t scl_rate;		/* add by kfx */
};

struct i2c_rdwr_ioctl_data_ext {
	struct i2c_msg_ext __user *msgs;	/* pointers to i2c_msgs */
	__u32 nmsgs;			/* number of i2c_msgs */
};


int uni_4mic_i2c_open(char *dev_name, int slave_id, int timeout, int retry)
{
    int ret;
    
    i2c_dev = dev_name;
    i2c_slave_id = slave_id;
    
    i2c_fd = open(i2c_dev, O_RDWR);
    if(0 > i2c_fd){
        HAL_PRINT_ERR("%s: failed to open %s\n", __func__, i2c_dev);
        return -1;
    }

    ret = ioctl(i2c_fd, I2C_TIMEOUT, timeout);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_TIMEOUT timeout=%d failed, %s\n", __func__, timeout, strerror(errno));
        close(i2c_fd);
        return -1;
    }
        
    ioctl(i2c_fd, I2C_RETRIES, retry);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_RETRIES retry=%d failed, %s\n", __func__, retry, strerror(errno));
        close(i2c_fd);
        return -1;
    }
    
    return 0;
}

int uni_4mic_i2c_close(void)
{
    if(0 > i2c_fd){
        HAL_PRINT_ERR("wrong i2c_fd=%d.\n", i2c_fd);
        return -1;
    }
    
    close(i2c_fd);
    i2c_fd = -1;
    return 0;
}

int uni_4mic_i2c_read_reg(uint16_t reg, uint16_t *val)
{
    int ret;
    struct i2c_rdwr_ioctl_data_ext ioctl_data;
    struct i2c_rdwr_ioctl_data_ext *data = &ioctl_data;
    struct i2c_msg_ext __user xfer[2];
    uint8_t buf[2];
    uint16_t slave_id = i2c_slave_id;

    if(0 >= slave_id){
        HAL_PRINT_ERR("%s: wrong slave_id=%d.\n", __func__, slave_id);
        return -1;
    }
    
    data->nmsgs = 2;
    data->msgs = xfer;
    
    data->msgs[0].addr = slave_id;
    data->msgs[0].flags = 0;
    data->msgs[0].len = 2;
    data->msgs[0].buf = buf;
    buf[0] = (reg >> 8) & 0xff;
    buf[1] = reg & 0xff;

    data->msgs[1].addr = slave_id;
    data->msgs[1].flags = I2C_M_RD;
    data->msgs[1].len = 2;
    data->msgs[1].buf = buf;
    
    ret = ioctl(i2c_fd, I2C_RDWR, data);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_RDWR failed, %s\n", __func__, strerror(errno));
        return -1;
    }

    *val = buf[0];
    *val <<= 8;
    *val |= buf[1];
    
    return 0;
}

int uni_4mic_i2c_write_reg(uint16_t reg, uint16_t val)
{
    int ret;
    struct i2c_rdwr_ioctl_data_ext ioctl_data;
    struct i2c_rdwr_ioctl_data_ext *data = &ioctl_data;
    struct i2c_msg_ext __user xfer[2];
    uint8_t buf[4];
    uint16_t slave_id = i2c_slave_id;

    if(0 >= slave_id){
        HAL_PRINT_ERR("%s: wrong slave_id=%d.\n", __func__, slave_id);
        return -1;
    }    

    data->nmsgs = 1;
    data->msgs = xfer;
    
    data->msgs[0].addr = slave_id;
    data->msgs[0].flags = 0;
    data->msgs[0].len = 4;
    data->msgs[0].buf = buf;
    buf[0] = (reg >> 8) & 0xff;
    buf[1] = reg & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = val & 0xff;
        
    ret = ioctl(i2c_fd, I2C_RDWR, data);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_RDWR failed, %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

int uni_4mic_i2c_update_bits(uint16_t reg, uint16_t mask, uint16_t val)
{
    int ret;
    uint16_t tmp;
    uint16_t orig;
    
    ret = uni_4mic_i2c_read_reg(reg, &orig);
    if (0 > ret){
        HAL_PRINT_ERR("[%s:%d] failed\n", __func__, __LINE__);
        return -1;
    }

    tmp = orig & ~mask;
    tmp |= val & mask;

    if (tmp != orig) {
        ret = uni_4mic_i2c_write_reg(reg, tmp);
        if (0 > ret){
            HAL_PRINT_ERR("[%s:%d] failed\n", __func__, __LINE__);
            return -1;
        }
    }
    
    return 0;
}

int uni_4mic_i2c_read_buf(uint8_t *buf, int size)
{
    int ret;
    struct i2c_rdwr_ioctl_data_ext ioctl_data;
    struct i2c_rdwr_ioctl_data_ext *data = &ioctl_data;
    struct i2c_msg_ext __user xfer[2];
    uint16_t slave_id = i2c_slave_id;

    if(0 >= slave_id){
        HAL_PRINT_ERR("%s: wrong slave_id=%d.\n", __func__, slave_id);
        return -1;
    }
    
    data->nmsgs = 1;
    data->msgs = xfer;
    
    data->msgs[0].addr = slave_id;
    data->msgs[0].flags = I2C_M_RD;
    data->msgs[0].len = size;
    data->msgs[0].buf = buf;
    
    ret = ioctl(i2c_fd, I2C_RDWR, data);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_RDWR failed, %s\n", __func__, strerror(errno));
        return -1;
    }
    
    return 0;
}

int uni_4mic_i2c_write_buf(uint8_t *buf, int size)
{
    int ret;
    struct i2c_rdwr_ioctl_data_ext ioctl_data;
    struct i2c_rdwr_ioctl_data_ext *data = &ioctl_data;
    struct i2c_msg_ext __user xfer[2];
    uint16_t slave_id = i2c_slave_id;

    if(0 >= slave_id){
        HAL_PRINT_ERR("%s: wrong slave_id=%d.\n", __func__, slave_id);
        return -1;
    }    

    data->nmsgs = 1;
    data->msgs = xfer;
    
    data->msgs[0].addr = slave_id;
    data->msgs[0].flags = 0;
    data->msgs[0].len = size;
    data->msgs[0].buf = buf;
        
    ret = ioctl(i2c_fd, I2C_RDWR, data);
    if(0 > ret){
        HAL_PRINT_ERR("%s: ioctl I2C_RDWR failed, %s\n", __func__, strerror(errno));
        return -1;
    }

    return 0;
}

