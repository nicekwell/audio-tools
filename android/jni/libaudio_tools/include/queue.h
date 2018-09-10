#ifndef __QUEUE_H__
#define __QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

    #include <stdio.h>
    typedef struct {
        long size;      //缓存大小，单位是字节
        unsigned char* buf;     //缓存首地址
        long len;       //目前缓存的数据长度
    } queue_t;

    queue_t* queue_alloc(long size);  //申请一个队列
    void queue_add_dat(queue_t* que, unsigned char dat, long len);    //添加数值
    void queue_add_buf(queue_t* que, unsigned char *dat, long len);   //添加数据
    long queue_get_buf(queue_t* que, unsigned char *p, long len);     //获取数据，返回读到的数据字节数
    void queue_free(queue_t *queue);

    
#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */

#endif
