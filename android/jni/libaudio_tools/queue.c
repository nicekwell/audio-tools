#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>

queue_t* queue_alloc(long size)
{
    queue_t* p = malloc(sizeof(queue_t));
    p->size = size;
    p->buf = malloc(size);
    memset(p->buf, 0, size);
    p->len = 0;
    return p;
}
void queue_free(queue_t* que)
{
    if(que) {
        if(que->buf)
            free(que->buf);
        free(que);
    }
}
void queue_add_dat(queue_t* que, unsigned char dat, long len)
{
    memset(que->buf + que->len, dat, len);
    que->len += len;
}
void queue_add_buf(queue_t* que, unsigned char *dat, long len)
{
    memcpy(que->buf + que->len, dat, len);
    que->len += len;
}
long queue_get_buf(queue_t* que, unsigned char *p, long len)
{
    long i;
    if(len > que->len)
        len = que->len;
    memcpy(p, que->buf, len);
    for(i=0; i<que->len - len;i++) {
        que->buf[i] = que->buf[i+len];
    }
    que->len -= len;
    return len;
}



