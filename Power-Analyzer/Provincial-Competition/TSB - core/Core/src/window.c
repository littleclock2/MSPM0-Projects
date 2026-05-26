#include "arm_math.h"
#include "ti_msp_dl_config.h"
#define FFT_SIZE 1024

void hanning_window(float *input,uint16_t size)//加窗
{
    uint16_t i;

    for (i = 0; i < size; i++)
        input[i] =  (input[i] * 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (size - 1))));
}
void hanning_window_revise(float *input,uint16_t size){//修正
    for (uint16_t i = 0; i < size; i++)
        input[i] *=  2;
}

void hamming_window(float *input,uint16_t size)//加汉明窗
{
    uint16_t i;

    for (i = 0; i < size; i++)
        input[i] =  (input[i] * (0.54f - 0.46f * arm_cos_f32(2.0f * PI * i / (size - 1))));
}
void hamming_window_revise(float *input,uint16_t size){//修正
    for (uint16_t i = 0; i < size; i++)
        input[i] *=  1.852;
}
void Hanning_five(float * input, int size) {
	//处理一个已经hanning窗的数据
		float temp[5] = { input[0],input[1],input[2],input[3],input[4] };
		int idx = 0;
		for (int i = 2; i < size - 3; i++) {
				input[i] = 1.0 / 60 * temp[(idx + 2) % 5] - 1.0 / 90 * (temp[(idx + 1) % 5] + temp[(idx + 3) % 5]) + 1.0 / 360 * (temp[idx] + temp[(idx + 4) % 5]);
				temp[idx] = input[i + 3];
				idx = (idx + 1) % 5;
		}
		return;
}

void FFT_revise(float* input, int size, float* I_arr, float* phase) {
	int idx_peak = 0, idx_subpeak = 0, idx_main = 0;
	float a = 0, k = 0;
	float temp = 0;
	for (int i = 10; i <= 100; i += 10) {//十次谐波对应的谱线
		/*步骤一 寻找峰值与次大谱线*/
		idx_peak = i - 3;
		for (int j = i - 3; j < i + 3; i++)
			idx_peak = (input[idx_peak] < input[j]) ? j : idx_peak;//极大值

		if (idx_peak == i - 3)//次大谱线
			idx_subpeak = idx_peak + 1;
		else if (idx_peak == i)
			idx_subpeak = idx_peak - 1;
		else
			idx_subpeak = (input[idx_peak - 2] > input[idx_peak + 2]) ? idx_peak - 2 : idx_peak + 2;
		idx_main = (idx_peak < idx_subpeak) ? idx_peak : idx_subpeak;///得到主谱线

		a = fabs(input[idx_subpeak] / input[idx_subpeak + 1]);//得到幅值比
		k = (4 - 3 * a) / (1 + a);//偏移值
		//校正
		temp = (k * (k * k - 1) * (k * k - 4) * (k * k - 9));//临时数
		temp = fabs(1.0 / temp);
		I_arr[i / 10 - 1] = temp * 4 * PI * 4 * fabs(input[idx_main]) / (1024 * arm_sin_f32(PI * k));
		if (i == 10) {//基波
			*phase = *phase - k * PI;
		}
	}
	return;
}