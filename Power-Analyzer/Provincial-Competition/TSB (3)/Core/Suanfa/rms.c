#include <math.h>
#include "ti_msp_dl_config.h"
#include "rms.h"
double cal_phase(uint16_t *input1){  //参数是AD采样的交流电压或电流值，待测信号的
//周期T=20ms，假设每个周期采304个点，每次采样500个点（即一个半周期左右）再进//行运算

    float Xr1=0,Xi1=0;  
    int  max_vaule, min_vaule;//最大值，最小值
    int mid; //中值，可以理解为用最大值和最小值计算出来的平均值，就是直流分量
 
    unsigned int i;
    unsigned int start,end;  
    min_vaule=4095;//12位AD的满量程AD转换值
    max_vaule=0; 
    start=0;//一个完整周期的起点
    end =0; //一个完整周期的终点
    for (i=1; i<512; i++){//303是一个周期的采样点数   第0个AD采样值不一定正确，舍弃掉，所以从第1个点开始
      if( input1[i]>max_vaule){		  
          max_vaule=input1[i];   //找最大值       
        }
      if( input1[i]<min_vaule){ 
          min_vaule=input1[i];	//找最小值	          
        }       
    }
    mid  = (max_vaule + min_vaule)/2;//计算平均值I（直流分量）
    for(i=1;i<152;i++){//150是半个周期的采样点数
        if(((input1[i]<mid )&&(input1[i+2]>=mid))||((input1[i]>mid )&&(input1[i+2]<=mid))){
         start = i+1;//找过零点，可能为正的过零点也可能为负的过零点，记为起点
         break;
        }
    }
    for(i=(start+180);i<500;i++){//500是采样点数，start加180的作用是为了跳过从起点算起的半周期过零点，这个数字与一个周期的采样点数有关，根据实际情况确定具体应为多少
        if(((input1[i]<mid)&&(input1[i+2]>=mid))||((input1[i]>mid)&&(input1[i+2]<=mid))){
            end = i ;//找终点
            break;
        }
    }
    //计算一个完整周期的采样点数
	uint16_t N = end-start+1;	
	float d1 = 6.2831852 / N; //6.2831852 = 2*3.1415926*1 对应DFT运算的基波分量
    //同理也可以计算K次谐波分量
    for(i=0;i<N;i++){//DFT运算   
        Xr1 += input1[i] * cos(d1*i);//实部
        Xi1 -= input1[i] * sin(d1*i);	 // 虚部
    }
	 float phase=atan2(Xi1,Xr1)*180/3.1415926; //初相位计算
    //根据实部和虚部也可以计算有效值
	 return ( phase);
	
}
//这是我们曾经用过的算法，是正确的，但在B题中也可以用FFT算法，为了保证测量精度，需要增加每个周期的采样点数，比如每个周期采样512个点，用FFT算法是最好采样点数是2的N次方，DFT和FFT都可以用，比较一下哪种更好

double get_Vrms(uint16_t *digital_data,int k){ //
//基波频率为50Hz,参数是有至少500个样本点的数字信号数组

    double real_part = 0,imag_part = 0;            //实部虚部
    int max_v = 0,min_v = 4095;           //记录数组的最大最小值，分别初始化为12为数的最小最大值
    int mid_v;                            //记录数组中间值
    int start = 0,end = 0;                //一个完整周期的起点终点

    for(int i = 0;i < 500;i++){
        if(digital_data[i] > max_v) max_v = digital_data[i]; //更新最大值
        if(digital_data[i] < min_v) min_v = digital_data[i]; //更新最小值
    }
    
    mid_v = (max_v + min_v) / 2; //得到中值

    for(int i = 0;i < 250;i++){           //在半个周期中必出现零点
        if( ((digital_data[i] < mid_v) && (digital_data[i+2] >= mid_v)) || //上升零点
            ((digital_data[i] > mid_v) && (digital_data[i+2] <= mid_v)) ){  //下降零点
            start = i + 1;
            break;
        }            //找到起点
    }
    for(int i = (start+100-1);i < 500;i++){
        if( ((digital_data[i] < mid_v) && (digital_data[i+2] >= mid_v)) || //上升零点
            ((digital_data[i] > mid_v) && (digital_data[i+2] <= mid_v)) ){  //下降零点
            end = i;
            break;
         }                  //找到终点
    }

    uint16_t N_DATA = end - start + 1; //一个周期内的采样点
    
    double divide_k = k * 6.2831852 / N_DATA;              //k为基频次数此处取1

    for(int i = 0;i < N_DATA;i++){
        double t = divide_k * i;
        double c = cos(t);
        double s = sin(t);
        real_part += digital_data[i] * cos(t); //计算实部
        imag_part -= digital_data[i] * sin(t ); //计算虚部
         if(i==200){
            i = 200;}
    }

    return (sqrt(real_part * real_part + imag_part * imag_part) / N_DATA) ;//计算有效值
}