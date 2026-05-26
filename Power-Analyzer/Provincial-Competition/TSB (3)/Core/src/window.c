#include "arm_math.h"

#define FFT_SIZE 512

void hanning_window(uint32_t *input,uint16_t size)//加窗
{
    uint16_t i;

    for (i = 0; i < size; i++)
        output[i] =  (int)(input[i] * 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (size - 1))));
}
void hanning_window_revise(uint16_t *input,uint16_t size){//修正
    for (i = 0; i < size; i++)
        output[i] *=  2;
}

void hamming_window(uint32_t *input,uint16_t size)//加汉明窗
{
    uint16_t i;

    for (i = 0; i < size; i++)
        output[i] =  (int)(input[i] * (0.54f - 0.46f * arm_cos_f32(2.0f * PI * i / (size - 1))));
}
void hanning_window_revise(uint16_t *input,uint16_t size){//修正
    for (i = 0; i < size; i++)
        output[i] *=  2;
}
