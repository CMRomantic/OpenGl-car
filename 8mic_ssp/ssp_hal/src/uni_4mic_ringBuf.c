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

#include "uni_4mic_hal.h"
#include "uni_4mic_ringBuf.h"


static inline void RingBufTrace(RingBuf_t *ringBuf, const char *func_name)
{
    //HAL_PRINT_INFO("%s: <%s> head=%d,tail=%d.\n", func_name, ringBuf->name, ringBuf->head, ringBuf->tail);
    //printf("%s: <%s> head=%d,tail=%d.\n", func_name, ringBuf->name, ringBuf->head, ringBuf->tail);
}

static inline int getReadableBufSize(RingBuf_t *ringBuf)
{
    return (ringBuf->head + ringBuf->size - ringBuf->tail) % ringBuf->size;
}

static inline int getWriteableBufSize(RingBuf_t *ringBuf)
{
    return (ringBuf->tail + ringBuf->size - ringBuf->min_rw_size - ringBuf->head) % ringBuf->size;
}

int initRingBuf(RingBuf_t *ringBuf, char *name, int size, int min_rw_size)
{
    unsigned int len;

    ringBuf->buf = malloc(size);
    if(NULL == ringBuf->buf){
        HAL_PRINT_ERR("%s: (NULL == ringBuf->buf)\n", __func__);
        return -1;
    }
    
    memset(ringBuf->name, 0, sizeof(ringBuf->name));
    len = strlen(name);
    len = sizeof(ringBuf->name) - 1 < len ? sizeof(ringBuf->name) - 1 : len;
    memcpy(ringBuf->name, name, len);
    
    ringBuf->size = size;
    ringBuf->head = 0;
    ringBuf->tail = 0;
    ringBuf->min_rw_size = min_rw_size;
    ringBuf->reset_flag = 0;

    pthread_mutex_init(&(ringBuf->lock), NULL);
    pthread_cond_init(&(ringBuf->cond), NULL);
    
    HAL_PRINT_INFO("%s: %s, size=%d, min_rw_size=%d\n", __func__, ringBuf->name, size, min_rw_size);

    return 0;
}

int ResetRingBuf(RingBuf_t *ringBuf)
{
    pthread_mutex_lock(&(ringBuf->lock));
    ringBuf->head = 0;
    ringBuf->tail = 0;
    ringBuf->reset_flag = 1;
    pthread_mutex_unlock(&(ringBuf->lock));
    
    return 0;
}

int ClearRingBuf(RingBuf_t *ringBuf)
{
    pthread_mutex_lock(&(ringBuf->lock));
    ringBuf->head = 0;
    ringBuf->tail = 0;
    ringBuf->reset_flag = 0;
    pthread_mutex_unlock(&(ringBuf->lock));
    
    return 0;
}

int destroyRingBuf(RingBuf_t *ringBuf)
{
    free(ringBuf->buf);
    ringBuf->buf = NULL;
    ringBuf->head = 0;
    ringBuf->tail = 0;
    ringBuf->reset_flag = 1;
    pthread_mutex_destroy(&(ringBuf->lock));
    pthread_cond_destroy(&(ringBuf->cond));
    
    HAL_PRINT_INFO("%s: %s\n", __func__, ringBuf->name);

    return 0;
}

int readBuf(RingBuf_t *ringBuf, char *buf, int size)
{
    int size1 = 0;
    int size2 = 0;
    
   struct timeval now;
struct timespec outtime;
//struct timespec temp;
struct timeval temp;
struct timeval tempp;

gettimeofday(&now, NULL);
outtime.tv_sec = now.tv_sec + 5;
outtime.tv_nsec = now.tv_usec * 1000;

//outtime.tv_sec = 5;
//outtime.tv_nsec = 0;

        temp.tv_sec = 0;
        temp.tv_usec = 10 * 1000;
        
        tempp.tv_sec = 0;
        tempp.tv_usec = 10 * 1000;

        


    if(NULL == buf){
        HAL_PRINT_ERR("%s: (NULL == buf)\n", __func__);
        return -1;
    }

    if(0 == size){
        HAL_PRINT_ERR("%s: (0 == size)\n", __func__);
        return -1;
    }

    if(size % ringBuf->min_rw_size){
       HAL_PRINT_ERR("%s: size=%d, min_rw_size=%d\n", __func__, size, ringBuf->min_rw_size);
        return -1;
    }

    if(ringBuf->size - ringBuf->min_rw_size < size){
        HAL_PRINT_ERR("%s: size=%d, max read size=%d\n", __func__, size, ringBuf->size - ringBuf->min_rw_size);
        return -1;
    }
    //select(0, NULL, NULL, NULL, &tempp);

    pthread_mutex_lock(&(ringBuf->lock));
  //  HAL_PRINT_ERR("%s:%s error nodata bbbbb %d %d \n", __func__, ringBuf->name, getReadableBufSize(ringBuf), size); 
    
#if 1  
int yyy=0;
    while(getReadableBufSize(ringBuf) < size){
        if(ringBuf->reset_flag){
            memset(buf, 0x5a, size);
            size = 0;
           HAL_PRINT_INFO("%s:%s reset_flag=1\n", __func__, ringBuf->name);
            break;
        }
      //  HAL_PRINT_ERR("%s:%s error nodata be %d \n", __func__, ringBuf->name,size );
      //  HAL_PRINT_INFO("%s:%s be cond.value=%d\n", __func__, ringBuf->name, ringBuf->cond.value);
     // HAL_PRINT_INFO("%s:%s reset_flag=1 be\n", __func__, ringBuf->name);
    //  pthread_cond_wait(&(ringBuf->cond), &(ringBuf->lock));
     pthread_cond_timedwait(&(ringBuf->cond), &(ringBuf->lock), &outtime);
       yyy++;
      // select(0, NULL, NULL, NULL, &temp);
      usleep(10000);
     //  HAL_PRINT_ERR("%s:%s error nodata--- bebe %d  %d--- yyy%d\n", __func__, ringBuf->name, size, getReadableBufSize(ringBuf),yyy );
     //  if(getReadableBufSize(ringBuf) > size){
     //  	 yyy=0;
    //   	 break;
       	
    //  }
      if(yyy>500){
       	 HAL_PRINT_ERR("%s:%s error nodata--- bex %d --- \n", __func__, ringBuf->name, size );
      	 break;
     }
       // HAL_PRINT_INFO("%s:%s reset_flag=1 after\n", __func__, ringBuf->name);
       // HAL_PRINT_INFO("%s:%s after cond.value=%d\n", __func__, ringBuf->name, ringBuf->cond.value);
    }
#endif
    
   // HAL_PRINT_ERR("%s:%s error nodata  ttttt %d %d \n", __func__, ringBuf->name, getReadableBufSize(ringBuf), size);
    
   // pthread_cond_timedwait(&(ringBuf->cond), &(ringBuf->lock), &outtime);
    if(getReadableBufSize(ringBuf) < size){
    	HAL_PRINT_ERR("%s:%s error nodata---- \n", __func__, ringBuf->name);
    	return -1;
    	 
    }

    if(ringBuf->tail + size > ringBuf->size){
        size1 = ringBuf->size - ringBuf->tail;
        size2 = size - size1;
        memcpy(buf, ringBuf->buf + ringBuf->tail, size1);
        memcpy(buf + size1, ringBuf->buf, size2);
    } else {
        memcpy(buf, ringBuf->buf + ringBuf->tail, size);
    }
    
    ringBuf->tail = (ringBuf->tail  + size) % ringBuf->size;

    RingBufTrace(ringBuf, __func__);
    
    pthread_mutex_unlock(&(ringBuf->lock));
//HAL_PRINT_ERR("%s:%s error nodata af %d %d\n", __func__, ringBuf->name, getReadableBufSize(ringBuf),size);
    return size;
}

int writeBuf(RingBuf_t *ringBuf,char *buf, int size)
{
    int size1 = 0;
    int size2 = 0;
    int overflow_size;


    if(NULL == buf){
       // HAL_PRINT_ERR("%s: (NULL == buf)\n", __func__);
        return -1;
    }

    if(0 == size){
        //HAL_PRINT_ERR("%s: (0 == size)\n", __func__);
        return -1;
    }

    if(size % ringBuf->min_rw_size){
       // HAL_PRINT_ERR("%s: size=%d, min_rw_size=%d\n", __func__, size, ringBuf->min_rw_size);
        return -1;
    }

    if(ringBuf->size - ringBuf->min_rw_size < size){
       // HAL_PRINT_ERR("%s: size=%d, max write size=%d\n", __func__, size, ringBuf->size - ringBuf->min_rw_size);
        return -1;
    }
    
    pthread_mutex_lock(&(ringBuf->lock));
    
//HAL_PRINT_ERR("%s:%s \n", __func__, ringBuf->name);

    if(ringBuf->head + size > ringBuf->size){
        size1 = ringBuf->size - ringBuf->head;
        size2 = size - size1;
        memcpy(ringBuf->buf + ringBuf->head, buf, size1);
        memcpy(ringBuf->buf, buf + size1, size2);
    } else {
        memcpy(ringBuf->buf + ringBuf->head, buf, size);
    }
    ringBuf->head = (ringBuf->head + size) % ringBuf->size;

    overflow_size = size - getWriteableBufSize(ringBuf);
    if(0 < overflow_size){
       // ringBuf->tail = (ringBuf->tail + overflow_size) % ringBuf->size;
       // HAL_PRINT_ERR("%s: %s, overflow_size=%d\n", __func__, ringBuf->name, overflow_size);
        //ResetRingBuf(&ringBuf);
       // exit(0);
    }
    
    pthread_cond_signal(&(ringBuf->cond));

    pthread_mutex_unlock(&(ringBuf->lock));
    
    return 0;
}

