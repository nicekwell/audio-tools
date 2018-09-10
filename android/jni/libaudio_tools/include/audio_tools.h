#ifndef __AUDIO_TOOLS_H__
#define __AUDIO_TOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

    #include <stdio.h>
    #include <queue.h>
    #include <resample.h>

    typedef short int16_t;
    typedef int int32_t;
    typedef float float32_t;

    typedef struct {
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        int16_t len;    //采样个数(长度)
        int16_t *data;  //指向数据的指针，在内存中的存放顺序是: data[0] data[1] （小端存储）
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //音频片段占用的内存长度是 len*cn*sizeof(int16_t)
    } aud16bi;

    typedef struct {
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        int16_t len;    //采样个数(长度)
        int32_t *data;  //指向数据的指针，在内存中的存放顺序是: data[0] data[1] data[2] data[3] （小端存储）
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //音频片段占用的内存长度是 len*cn*sizeof(int32_t)
    } aud32bi;

    typedef struct {
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        int16_t len;    //采样个数(长度)
        float32_t *data;        //指向数据的指针，在内存中的存放顺序是: data[0] data[1] data[2] data[3] （小端存储）
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //音频片段占用的内存长度是 len*cn*sizeof(float32_t)
    } aud32bf;

//申请和释放
    /*
      申请和释放音频片段内存
      参数分别是采样率、通道数、内存的采样个数
      申请的音频片段时间长度是len/sr，数据部分占用内存空间是 len*cn*sizeof(int16_t)    */
    aud16bi* aud16bi_alloc(int32_t sr, int16_t cn, int16_t len);
    /*
      申请和释放音频片段内存
      参数分别是采样率、通道数、内存的采样个数
      申请的音频片段时间长度是len/sr，数据部分占用内存空间是 len*cn*sizeof(int32_t)    */
    aud32bi* aud32bi_alloc(int32_t sr, int16_t cn, int16_t len);
    /*
      申请和释放音频片段内存
      参数分别是采样率、通道数、内存的采样个数
      申请的音频片段时间长度是len/sr，数据部分占用内存空间是 len*cn*sizeof(float32_t)    */
    aud32bf* aud32bf_alloc(int32_t sr, int16_t cn, int16_t len);
    
    void aud16bi_free(aud16bi* aud);
    void aud32bi_free(aud32bi* aud);
    void aud32bf_free(aud32bf* aud);


//音频类型转换，使用前需要先申请转换前和转换后的音频片段
    //要求传入的两个音频片段的 采样率、通道数 相同
    int aud16bi_2_aud32bi(aud32bi* out, aud16bi* in);   //返回0代表成功
    int aud32bi_2_aud16bi(aud16bi* out, aud32bi* in);

    int aud16bi_2_aud32bf(aud32bf* out, aud16bi* in);
    int aud32bf_2_aud16bi(aud16bi* out, aud32bf* in);
    
//以下所有操作都是针对aud32bf类型的，需要进行各种操作需要先转换成aud32bf类型

//音频复制
    int aud_copy(aud32bf* dst, aud32bf* src);   //复制音频片段，必须要求采样率、通道数都一样，返回0表示成功
                                                //只复制音频片段的数据部分，采样率、通道数属性仍保持不变
//音频追加
    int aud_cat(aud32bf* dst, aud32bf* src);    //把音频片段src追加到音频片段dst后面，返回0表示成功
                                                //必须要求两个音频片段的采样率、通道数相同
//通道拆分
    /*
      使用前需要先申请转换前和转换后的音频片段。
      从第num号通道开始分割，[0, num-1]通道输出到out1，[num, ] 通道输出到out2。
      分割后，out1的通道数应当为num，out2的通道数应当为 in->sr - num    */
    int aud_split(aud32bf* in, int num, aud32bf* out1, aud32bf* out2);

//通道融合
    /*
      使用前需要先申请转换前和转换后的音频片段。
      把in1和in2合成为一个音频。
      要求in1和in2的采样率、通道数、len都相同。    */
    int aud_mix(aud32bf* in1, aud32bf* in2, aud32bf* out);

//把所有通道延时
    /*说明一下为什么是把所有通道延时，而不是把某些通道延时。因为有split，可以先把原来的音频split，再用这里的延时，这样即可达到效果。
      这里的延时基于我写的queue模块。    */
    typedef struct {
        int16_t delay;  //延时时间，单位ms
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        queue_t* que;   //延时用到的队列
    } m_aud_delay_t;
    //创建一个延时算法，传入采样率、通道数、延时时间，会固定申请一个400k的队列
    m_aud_delay_t* aud_delay_create(int32_t sr, int16_t cn, int16_t delay);
    //进行延时，延时后的输出就是原来的输入地址，相当于把原音频进行了更改
    int aud_delay(m_aud_delay_t* m_aud_delay, aud32bf* aud);
    //释放方法
    void aud_delay_free(m_aud_delay_t* m_aud_delay);

//RDC 把所有通道叠加一个值
    /*
      把音频各个通道分别叠加一个值，传入的是float32_t数组，会把每个通道依次叠加这里的数值。
      输出和输入是一个地址，相当于把原音频修改     */
    int aud_dc(aud32bf* aud, float32_t* val);

//音量控制，把所有通道乘以某个值
    /*
      把音频所有通道的所有数据乘以m
      输出和输入是一个地址，相当于把原音频修改     */
    int aud_vol(aud32bf* aud, float32_t m);

//resample
    /*
      使用前需要先申请转换前和转换后的音频片段。
      这个功能基于libsamplerate库，在android工程里由libsamplerate源码编译出so，在mac下使用brew安装的libsamplerate so。
      在libsamplerate库基础上，用tony写的resample.c二次封装的函数来进行resample。
      而我在这里相当于再次封装成使用这里定义的aud32bf格式文件。     */
    typedef struct {
        int32_t sr_in;
        int32_t sr_out;
        int16_t cn;
        int16_t ms;     //创建resample时用到，固定为10ms
        src* aud_src;   //这是src算法的结构体
    } m_aud_src_t;
    m_aud_src_t* aud_src_create(int32_t sr_in, int32_t sr_out, int16_t cn);
    /*
      输入的采样率为 in->sr，帧数为 in->len；
      输出的采样率为 out->sr，帧数为 in->len / in->sr * out->sr
     */
    int aud_resample(m_aud_src_t* m_aud_src, aud32bf* in, aud32bf* out);
    void aud_src_free(m_aud_src_t* m_aud_src);

#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */

#endif
