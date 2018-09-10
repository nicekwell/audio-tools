#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <audio_tools.h>

aud16bi* aud16bi_alloc(int32_t sr, int16_t cn, int16_t len)
{
    aud16bi* p = malloc(sizeof(aud16bi));
    p->sr = sr;
    p->cn = cn;
    p->len = 0;
    p->data = malloc(len*cn*sizeof(int16_t));
    memset(p->data, 0, len*cn*sizeof(int16_t));
    return p;
}
void aud16bi_free(aud16bi* aud)
{
    if(aud) {
        if(aud->data)
            free(aud->data);
        free(aud);
    }
}
aud32bi* aud32bi_alloc(int32_t sr, int16_t cn, int16_t len)
{
    aud32bi* p = malloc(sizeof(aud32bi));
    p->sr = sr;
    p->cn = cn;
    p->len = 0;
    p->data = malloc(len*cn*sizeof(int32_t));
    memset(p->data, 0, len*cn*sizeof(int32_t));
    return p;
}
void aud32bi_free(aud32bi* aud)
{
    if(aud) {
        if(aud->data)
            free(aud->data);
        free(aud);
    }
}
aud32bf* aud32bf_alloc(int32_t sr, int16_t cn, int16_t len)
{
    aud32bf* p = malloc(sizeof(aud32bf));
    p->sr = sr;
    p->cn = cn;
    p->len = 0;
    p->data = malloc(len*cn*sizeof(float32_t));
    memset(p->data, 0, len*cn*sizeof(float32_t));
    return p;
}
void aud32bf_free(aud32bf* aud)
{
    if(aud) {
        if(aud->data)
            free(aud->data);
        free(aud);
    }
}

static void int16_to_float32_array (const int16_t *in, float32_t *out, int32_t len)   //传入的len是采样次数，采样数*通道数
{
    while (len) {
	len -- ;
        out [len] = (float32_t) (in [len] / (1.0 * 0x8000)) ;
    } ;
    return ;
} /* short_to_float_array */
#define CPU_CLIPS_POSITIVE 0
#define CPU_CLIPS_NEGATIVE 1
static void float32_to_int16_array (const float32_t *in, int16_t *out, int32_t len)     //传入的len是采样次数，采样数*通道数
{	double scaled_value ;

    while (len)
    {	len -- ;

        scaled_value = in [len] * (8.0 * 0x10000000) ;
        if (CPU_CLIPS_POSITIVE == 0 && scaled_value >= (1.0 * 0x7FFFFFFF))
        {	out [len] = 32767 ;
            continue ;
        } ;
        if (CPU_CLIPS_NEGATIVE == 0 && scaled_value <= (-8.0 * 0x10000000))
        {	out [len] = -32768 ;
            continue ;
        } ;

        out [len] = (int16_t) (((long) scaled_value) >> 16) ;
    } ;

} /* src_float_to_short_array */
int aud16bi_2_aud32bi(aud32bi* out, aud16bi* in)
{
    int16_t i;
    int16_t *p_in;
    int32_t *p_out;
    if(in->sr != out->sr || in->cn != out->cn)
        return -1;
    p_in = in->data;
    p_out = out->data;
    for(i = 0;i < (int32_t)((int32_t)(in->len) * (int32_t)(in->cn)); i++) {
        *p_out = (int32_t)(*p_in);
        p_in ++;
        p_out ++;
    }
    out->len = in->len;
    return 0;
}
int aud32bi_2_aud16bi(aud16bi* out, aud32bi* in)
{
    int16_t i;
    int32_t *p_in;
    int16_t *p_out;
    if(in->sr != out->sr || in->cn != out->cn)
        return -1;
    p_in = in->data;
    p_out = out->data;
    for(i = 0;i < (int32_t)((int32_t)(in->len) * (int32_t)(in->cn)); i++) {
        *p_out = (int16_t)(*p_in);
        p_in ++;
        p_out ++;
    }
    out->len = in->len;
    return 0;
}
int aud16bi_2_aud32bf(aud32bf* out, aud16bi* in)
{
    int16_t i;
    int16_t* p_in;
    float32_t* p_out;
    if(in->sr != out->sr || in->cn != out->cn)
        return -1;
    p_in = in->data;
    p_out = out->data;
    int16_to_float32_array(in->data, out->data, (int32_t)((int32_t)(in->len) * (int32_t)(in->cn)));
    out->len = in->len;
    return 0;
}
int aud32bf_2_aud16bi(aud16bi* out, aud32bf* in)
{
    int16_t i;
    float32_t* p_in;
    int16_t* p_out;
    if(in->sr != out->sr || in->cn != out->cn)
        return -1;
    p_in = in->data;
    p_out = out->data;
    float32_to_int16_array(in->data, out->data, (int32_t)((int32_t)(in->len) * (int32_t)(in->cn)));
    out->len = in->len;
    return 0;
}

/*************** 以下函数都是针对aud32bf类型 ***************/
int aud_copy(aud32bf* dst, aud32bf* src)
{
    if(src->sr != dst->sr || src->cn != dst->cn)
        return -1;
    memcpy(dst->data, src->data, src->len * src->cn * sizeof(float32_t));
    dst->len = src->len;
    return 0;
}

int aud_cat(aud32bf* dst, aud32bf* src)
{
    int32_t src_memlen = src->len * src->cn * sizeof(float32_t);
    int32_t dst_memlen = dst->len * dst->cn * sizeof(float32_t);
    if(src->sr != dst->sr || src->cn != dst->cn)
        return -1;
    memcpy(dst->data + dst_memlen, src->data, src_memlen);
    dst->len += src->len;
    return 0;
}

int aud_split(aud32bf* in, int num, aud32bf* out1, aud32bf* out2)
{
    int16_t i;
    float32_t *p_in, *p_out1, *p_out2;
    int32_t size1 = out1->cn * sizeof(float32_t);
    int32_t size2 = out2->cn * sizeof(float32_t);
    if(in->sr != out1->sr || in->sr != out2->sr || out1->sr != out2->sr)
        return -1;
    if(num > in->cn || out1->cn != num || out2->cn != (in->cn - num))
        return -2;
    p_in = in->data;
    p_out1 = out1->data;
    p_out2 = out2->data;
    for(i = 0; i < in->len; i++) {
        memcpy(p_out1, p_in, size1);
        memcpy(p_out2, ((unsigned char*)p_in) + size1, size2);
        p_in += in->cn;
        p_out1 += out1->cn;
        p_out2 += out2->cn;
    }
    out1->len = in->len;
    out2->len = in->len;
    return 0;
}

int aud_mix(aud32bf* in1, aud32bf* in2, aud32bf* out)
{
    int16_t i;
    float32_t *p_in1, *p_in2, *p_out;
    int32_t size1 = in1->cn * sizeof(float32_t);
    int32_t size2 = in2->cn * sizeof(float32_t);
    if(in1->sr != in2->sr || in1->sr != out->sr || in2->sr != out->sr)
        return -1;
    if(in1->cn + in2->cn != out->cn)
        return -2;
    if(in1->len != in2->len)
        return -3;
    p_in1 = in1->data;
    p_in2 = in2->data;
    p_out = out->data;
    for(i = 0; i < in1->len; i++) {
        memcpy(p_out, p_in1, size1);
        memcpy(((unsigned char*)p_out) + size1, p_in2, size2);
        p_in1 += in1->cn;
        p_in2 += in2->cn;
        p_out += out->cn;
    }
    out->len = in1->len;
    return 0;
}

m_aud_delay_t* aud_delay_create(int32_t sr, int16_t cn, int16_t delay)
{
    int32_t buf_size = (sr/1000) * delay * cn * sizeof(float32_t);
    m_aud_delay_t* p = malloc(sizeof(m_aud_delay_t));
    p->delay = delay;
    p->sr = sr;
    p->cn = cn;
    p->que = queue_alloc(400000);
    queue_add_dat(p->que, 0, buf_size);
    return p;
}
void aud_delay_free(m_aud_delay_t* m_aud_delay)
{
    if(m_aud_delay) {
        if(m_aud_delay->que)
            queue_free(m_aud_delay->que);
        free(m_aud_delay);
    }
}
int aud_delay(m_aud_delay_t* m_aud_delay, aud32bf* aud)
{
    int32_t size = aud->len * aud->cn * sizeof(float32_t);
    if(m_aud_delay->sr != aud->sr || m_aud_delay->cn != aud->cn)
        return -1;
    queue_add_buf(m_aud_delay->que, (unsigned char*)(aud->data), size);
    queue_get_buf(m_aud_delay->que, (unsigned char*)(aud->data), size);
    return 0;
}

int aud_dc(aud32bf* aud, float32_t* val)
{
    int16_t i, j;
    float32_t* p = aud->data;
    for(i=0; i < aud->len; i++) {
        for(j=0; j < aud->cn; j++) {
            *p += val[j];
            p++;
        }
    }
    return 0;
}

int aud_vol(aud32bf* aud, float32_t m)
{
    int16_t i;
    float32_t* p = aud->data;
    for(i=0; i < (int32_t)((int32_t)(aud->len) * (int32_t)(aud->cn)); i++) {
        *p *= m;
        p++;
    }
    return 0;
}

m_aud_src_t* aud_src_create(int32_t sr_in, int32_t sr_out, int16_t cn)
{
    m_aud_src_t* p = malloc(sizeof(m_aud_src_t));
    p->sr_in = sr_in;
    p->sr_out = sr_out;
    p->cn = cn;
    p->ms = 10; //这个值固定为10ms
    p->aud_src = resample_create(p->sr_in, p->sr_out, p->cn, p->ms);
    return p;
}
void aud_src_free(m_aud_src_t* m_aud_src)
{
    if(m_aud_src) {
        if(m_aud_src->aud_src)
            resample_delete(m_aud_src->aud_src);
        free(m_aud_src);
    }
}
int aud_resample(m_aud_src_t* m_aud_src, aud32bf* in, aud32bf* out)
{
    if(m_aud_src->cn != in->cn || m_aud_src->cn != out->cn || in->cn != out->cn)
        return -1;
    if(m_aud_src->sr_in != in->sr || m_aud_src->sr_out != out->sr)
        return -2;
    resample_float(m_aud_src->aud_src, in->data, out->data, in->len * in->cn);
    out->len = in->len * out->sr / in->sr;
    return 0;
}






