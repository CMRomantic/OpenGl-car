#include <errno.h>
#ifndef _UNI_4MIC_RINGBUF_H_
#define _UNI_4MIC_RINGBUF_H_

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct RINGBUF{
    char name[30];
    pthread_mutex_t lock;
    pthread_cond_t cond;
    char *buf;
    int size;
    int head;
    int tail;
    int min_rw_size;
    int reset_flag;
}RingBuf_t;


extern int initRingBuf(RingBuf_t *ringBuf, char *name, int size, int min_rw_size);
extern int ResetRingBuf(RingBuf_t *ringBuf);
extern int ClearRingBuf(RingBuf_t *ringBuf);
extern int destroyRingBuf(RingBuf_t *ringBuf);
extern int  readBuf(RingBuf_t *ringBuf, char *buf, int size);
extern int writeBuf(RingBuf_t *ringBuf,char *buf, int size);


#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* _UNI_4MIC_RINGBUF_H_ */

