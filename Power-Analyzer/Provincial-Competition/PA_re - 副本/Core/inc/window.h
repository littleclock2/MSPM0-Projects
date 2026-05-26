#include "ti_msp_dl_config.h"
void hanning_window(float *input,uint16_t size);
void hanning_window_revise(float *input,uint16_t size);
void hamming_window(float *input,uint16_t size);
void hanning_window_revise(float *input,uint16_t size);
void Hanning_five(float * input, int size); 
void FFT_revise(float* input, int size, float* I_arr);
//void FFT_revise(float* input, int size, float* I_arr, float* phase);