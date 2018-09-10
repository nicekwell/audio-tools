#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <audio_tools.h>

int main(int argc, char *argv[])
{
    FILE* fp_in = fopen(argv[1], "r");
#define MS 22
    aud16bi* aud_mic = aud16bi_alloc(16000, 4, 16*MS);
    FILE* fp_aud_mic = fopen("out/1-aud_mic_16k4c16bi.pcm", "w");
    aud16bi* aud_spk = aud16bi_alloc(16000, 2, 16*MS);
    FILE* fp_aud_spk = fopen("out/2-aud_spk_16k2c16bi.pcm", "w");

    //类型转换
    aud32bi* aud_mic_32bi = aud32bi_alloc(16000, 4, 16*MS);
    FILE* fp_aud_mic_32bi = fopen("out/3-aud_mic_32bi_16k4c32bi.pcm", "w");
    aud32bf* aud_mic_32bf = aud32bf_alloc(16000, 4, 16*MS);
    FILE* fp_aud_mic_32bf = fopen("out/4-aud_mic_32bf_16k4c32bf.pcm", "w");
    //通道拆分
    aud32bf* aud_mic12_32bf = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_mic12_32bf = fopen("out/5-aud_mic12_32bf_16k2c32bf.pcm", "w");
    aud32bf* aud_mic34_32bf = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_mic34_32bf = fopen("out/6-aud_mic34_32bf_16k2c32bf.pcm", "w");
    //复制
    aud32bf* aud_spk_copy = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_spk_copy = fopen("out/7-aud_spk_copy_16k2c32bf.pcm", "w");
    //延时
    aud32bf* aud_mic34_delay = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_mic34_delay = fopen("out/8-aud_mic34_delay_16k2c32bf.pcm", "w");
    m_aud_delay_t* delay_aud_mic34 = aud_delay_create(16000, 2, 100);
    //DC
    aud32bf* aud_mic12_dc = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_mic12_dc = fopen("out/9-aud_mic12_dc_16k2c32bf.pcm", "w");
    //volume
    aud32bf* aud_mic34_volume = aud32bf_alloc(16000, 2, 16*MS);
    FILE* fp_aud_mic34_volume = fopen("out/10-aud_mic34_volume_16k2c32bf.pcm", "w");
    //通道融合
    aud32bf* aud_mix1234 = aud32bf_alloc(16000, 4, 16*MS);
    FILE* fp_aud_mix1234 = fopen("out/11-aud_mix1234_16k4c32bf.pcm", "w");
    aud32bf* aud_mix123456 = aud32bf_alloc(16000, 6, 16*MS);
    FILE* fp_aud_mix123456 = fopen("out/12-aud_mix123456_16k6c32bf.pcm", "w");
    //resample
    aud32bf* aud_resampled = aud32bf_alloc(48000, 6, 48*MS);
    FILE* fp_aud_resampled = fopen("out/13-aud_aud_resampled_48k6c32bf.pcm", "w");
    m_aud_src_t* src = aud_src_create(16000, 48000, 6);
    //类型转换
    aud16bi* aud_final_16bi = aud16bi_alloc(48000, 6, 48*MS);
    FILE* fp_aud_final_16bi = fopen("out/14-aud_final_16bi_48k6c16bi.pcm", "w");

    while(1) {
        unsigned char in_buf[16*MS*(4+2)*2];
        long read_len;
        read_len = fread(in_buf, 1, 16*MS*(4+2)*2, fp_in);
        if(read_len <= 0){
            break;
        }
        //获取mic通道和spk通道
        {
            long i;
            for(i=0;i<16*MS;i++) {
                memcpy(aud_mic->data + i*4, &(in_buf[i*6*2]), 4*2);
                memcpy(aud_spk->data + i*2, &(in_buf[i*6*2+8]), 2*2);
            }
            aud_mic->len = 16*MS;
            aud_spk->len = 16*MS;

            fwrite(aud_mic->data, 1, aud_mic->len * aud_mic->cn * sizeof(int16_t), fp_aud_mic);
            fwrite(aud_spk->data, 1, aud_spk->len * aud_spk->cn * sizeof(int16_t), fp_aud_spk);
        }
        //类型转换
        aud16bi_2_aud32bi(aud_mic_32bi, aud_mic);
        fwrite(aud_mic_32bi->data, 1, aud_mic_32bi->len * aud_mic_32bi->cn * sizeof(int32_t), fp_aud_mic_32bi);
        aud16bi_2_aud32bf(aud_mic_32bf, aud_mic);
        fwrite(aud_mic_32bf->data, 1, aud_mic_32bf->len * aud_mic_32bf->cn * sizeof(float32_t), fp_aud_mic_32bf);
        //通道拆分
        aud_split(aud_mic_32bf, 2, aud_mic12_32bf, aud_mic34_32bf);
        fwrite(aud_mic12_32bf->data, 1, aud_mic12_32bf->len * aud_mic12_32bf->cn * sizeof(float32_t), fp_aud_mic12_32bf);
        fwrite(aud_mic34_32bf->data, 1, aud_mic34_32bf->len * aud_mic34_32bf->cn * sizeof(float32_t), fp_aud_mic34_32bf);
        //复制
        aud_copy(aud_spk_copy, aud_mic12_32bf);
        fwrite(aud_spk_copy->data, 1, aud_spk_copy->len * aud_spk_copy->cn * sizeof(float32_t), fp_aud_spk_copy);
        //延时
        aud_copy(aud_mic34_delay, aud_mic34_32bf);
        aud_delay(delay_aud_mic34, aud_mic34_delay);
        fwrite(aud_mic34_delay->data, 1, aud_mic34_delay->len * aud_mic34_delay->cn * sizeof(float32_t), fp_aud_mic34_delay);
        //DC
        {
            float32_t val[2] = {0, 0.23};
            aud_copy(aud_mic12_dc, aud_mic12_32bf);
            aud_dc(aud_mic12_dc, val);
            fwrite(aud_mic12_dc->data, 1, aud_mic12_dc->len * aud_mic12_dc->cn * sizeof(float32_t), fp_aud_mic12_dc);
        }
        //volume
        aud_copy(aud_mic34_volume, aud_mic34_delay);
        aud_vol(aud_mic34_volume, 2);
        fwrite(aud_mic34_volume->data, 1, aud_mic34_volume->len * aud_mic34_volume->cn * sizeof(float32_t), fp_aud_mic34_volume);
        //融合
        aud_mix(aud_mic12_dc, aud_mic34_volume, aud_mix1234);
        fwrite(aud_mix1234->data, 1, aud_mix1234->len * aud_mix1234->cn * sizeof(float32_t), fp_aud_mix1234);
        aud_mix(aud_mix1234, aud_spk_copy, aud_mix123456);
        fwrite(aud_mix123456->data, 1, aud_mix123456->len * aud_mix123456->cn * sizeof(float32_t), fp_aud_mix123456);
        //resample
        aud_resample(src, aud_mix123456, aud_resampled);
        fwrite(aud_resampled->data, 1, aud_resampled->len * aud_resampled->cn * sizeof(float32_t), fp_aud_resampled);
        //类型转换
        aud32bf_2_aud16bi(aud_final_16bi, aud_resampled);
        fwrite(aud_final_16bi->data, 1, aud_final_16bi->len * aud_final_16bi->cn * sizeof(int16_t), fp_aud_final_16bi);
    }

    aud16bi_free(aud_mic);
    aud16bi_free(aud_spk);
    aud32bi_free(aud_mic_32bi);
    aud32bf_free(aud_mic_32bf);
    aud32bf_free(aud_mic12_32bf);
    aud32bf_free(aud_mic34_32bf);
    aud32bf_free(aud_spk_copy);
    aud32bf_free(aud_mic34_delay);
    aud_delay_free(delay_aud_mic34);
    aud32bf_free(aud_mic12_dc);
    aud32bf_free(aud_mic34_volume);
    aud32bf_free(aud_mix1234);
    aud32bf_free(aud_mix123456);
    aud32bf_free(aud_resampled);
    aud_src_free(src);
    aud16bi_free(aud_final_16bi);
    fclose(fp_in);
    fclose(fp_aud_mic);
    fclose(fp_aud_spk);
    fclose(fp_aud_mic_32bi);
    fclose(fp_aud_mic_32bf);
    fclose(fp_aud_mic12_32bf);
    fclose(fp_aud_mic34_32bf);
    fclose(fp_aud_spk_copy);
    fclose(fp_aud_mic34_delay);
    fclose(fp_aud_mic12_dc);
    fclose(fp_aud_mic34_volume);
    fclose(fp_aud_mix1234);
    fclose(fp_aud_mix123456);
    fclose(fp_aud_resampled);
    fclose(fp_aud_final_16bi);
    return 0;
}
