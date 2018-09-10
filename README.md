
# Android里使用方法

audio_tools和其依赖的samplerate都以独立开编译成静态库，只需要添加自己需要的模块(jni库或者bin)并使用这些静态库即可。

在自己的模块中使用这些库函数需要做的工作有：

**1、复制头文件**  
添加libaudio_tools、queue、libsamplerate提供的头文件，需要用到这里面的数据结构，需要的头文件有：

```
audio_tools.h
queue.h
resample.h
samplerate.h
```

**2、使用audio_tools提供的库函数**  
不需要管queue、resample和samplerate了，audio_tools在他们基础上封装了音频处理数据结构和方法。  
只要 `#include <audio_tools.h>` 之后就可以使用这些方法了。
    

# 全局数据定义

```c
typedef short int16_t
typedef int int32_t
typedef float float32_t
```

# 数据结构

这里的数据结构是用来描述一个音频片段。

描述音频片段的结构有如下几种：

### 16位整型
```c
    typedef struct {
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        int16_t len;    //采样个数(长度)
        int16_t *data;  //指向数据的指针，在内存中的存放顺序是: data[0] data[1] （小端存储）
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //包含所有通道的总采样次数是 len*cn
        //音频片段占用的内存长度是 len*cn*sizeof(int16_t)
    } aud16bi;
```

### 32位整型
```c
    typedef struct {
        int32_t sr;     //采样率
        int16_t cn;     //通道数
        int16_t len;    //采样个数(长度)
        int32_t *data;  //指向数据的指针，在内存中的存放顺序是: data[0] data[1] data[2] data[3] （小端存储）
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //包含所有通道的总采样次数是 len*cn
        //音频片段占用的内存长度是 len*cn*sizeof(int32_t)
    } aud32bi;
```

### 32位浮点型
```c
    typedef struct {
        int32_t sr;
        int16_t cn;
        int16_t len;
        float32_t *data;
        //以下是隐藏信息
        //音频片段占用的时间是 len/sr
        //包含所有通道的总采样次数是 len*cn
        //音频片段占用的内存长度是 len*cn*sizeof(float32_t)
    } aud32bf;
```

# 包含的操作有

### 申请和释放音频片段内存

```c
aud16bi* aud16bi_alloc(int32_t sr, int16_t cn, int16_t len);    //len是采样帧数，比如48k采样率1s会采样48k帧
aud32bi* aud32bi_alloc(int32_t sr, int16_t cn, int16_t len);
aud32bf* aud32bf_alloc(int32_t sr, int16_t cn, int16_t len);
```

```c
void aud16bi_free(aud16bi* aud);
void aud32bi_free(aud32bi* aud);
void aud32bf_free(aud32bf* aud);
```

### 各个类型转换(16bi和32bi互转有问题)
**使用前需要先申请转换前和转换后的音频片段。  
要求传入的两个音频片段的 采样率、通道数 相同**

```c
int aud16bi_2_aud32bi(aud32bi*, aud16bi*);  //有问题
int aud32bi_2_aud16bi(aud16bi*, aud32bi*);  //有问题

int aud16bi_2_aud32bf(aud32bf*, aud16bi*);
int aud32bf_2_aud16bi(aud16bi*, aud32bf*);
```

**下面所有操作都是针对aud32bf类型的，要进行各种操作都需要先转换成aud32bf类型**

### 音频复制

```c
int aud_copy(aud32bf* dst, aud32bf* src);   //复制音频片段，必须要求采样率、通道数都一样，只复制音频片段的数据部分，采样率、通道数属性仍保持不变
```

### 音频追加(未测试)

```c
int aud_cat(aud32bf* dst, aud32bf* src);    //把音频片段src追加到音频片段dst后面
                                            //必须要求两个音频片段的采样率、通道数相同
```

### 通道拆分
**使用前需要先申请转换前和转换后的音频片段。**

```c
int aud_split(aud32bf* in, int num, aud32bf* out1, aud32bf* out2);
从第num号通道开始分割，[0, num-1]通道输出到out1，[num, ] 通道输出到out2。
```

### 通道融合
**使用前需要先申请转换前和转换后的音频片段。  
要求in1和in2的采样率、通道数、len都相同。**

```c
int aud_mix(aud32bf* in1, aud32bf* in2, aud32bf* out);
把in1和in2合成为一个音频。
```

### 把所有通道延时

**使用前需要先申请转换前和转换后的音频片段。**

说明一下为什么是把所有通道延时，而不是把某些通道延时。  
因为有split，可以先把原来的音频split，再用这里的延时，这样即可达到效果。  
**这里的延时基于我写的queue模块。**

```c
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
```

### RDC 把所有通道叠加一个值
**使用前需要先申请转换前和转换后的音频片段。**

```c
int aud_dc(aud32bf* aud, float32_t* val);   //把音频各个通道分别叠加一个值，传入的是float32_t数组，会把每个通道依次叠加这里的数值。
                                        //输出和输入是一个地址，相当于把原音频修改
```

### 音量控制，把所有通道乘以某个值

```c
int aud_vol(aud32bf* aud, float32_t m);   //把音频所有通道的所有数据乘以m
                                        //输出和输入是一个地址，相当于把原音频修改
```

### resample

**使用前需要先申请转换前和转换后的音频片段。**

这个功能基于libsamplerate库，在android工程里由libsamplerate源码编译出so，在mac下使用brew安装的libsamplerate so。  
在libsamplerate库基础上，用tony写的resample.c二次封装的函数来进行resample。  
而我在这里相当于再次封装成使用这里定义的aud32bf格式文件。

```c
//基于libsamplerate库和tony写的resample.c
typedef struct {
    int32_t sr_in;
    int32_t sr_out;
    int16_t cn;
    int16_t ms;     //创建resample时用到，固定为10ms
    src* aud_src;
} m_aud_src_t;
m_aud_src_t* aud_src_create(int32_t sr_in, int32_t sr_out, int16_t cn);
int aud_resample(aud_src_t*, aud32bf* in, aud32bf* out);    //进行resample
void aud_src_free(aud_src_t*);
```





