#include "ti_msp_dl_config.h"
#include "oled_spi.h"
#include "arm_const_structs.h"
#include "arm_math.h"
#include "stdio.h"
#include "board.h"
#include "hc05.h"
//捕获定时器变量
#define TIMER_CAPTURE_DURATION (CAPTURE_0_INST_LOAD_VALUE)
volatile bool gCheckEdgeCaptureTime;

#define DAC12_REF_VOLTAGE_mV (3300)
#define DAC12_OUTPUT_VOLTAGE_mV (1000)
#define BUFFER_SIZE 1024
uint8_t choice  =0;
uint16_t dac_value =0;
uint32_t dac_output; // 用于存储计算出的DAC值
uint32_t current_power = 0;
uint16_t CaptureCnt_Pos_Now, CaptureCnt_Pos_Past;
uint16_t CaptureCnt_Neg_Now ,CaptureCnt_Neg_Past;
float PWM_Detect_Preiod;
float PWM_Detect_Duty;
float PWM_Detect_Ton;
float PWM_Output_Preiod;
float PWM_Output_Duty;
float PWM_Output_Ton;
uint16_t PWM_Period_Cnt;
uint16_t PWM_Duty_Cnt;
uint32_t period;

//PID结构体定义
typedef struct{
  float kp, ki, kd; 						      // 三个静态系数
  float change_p, change_i, change_d;	          // 三个动态参数
  float error, last_error; 						  // 误差、之前误差
  float max_change_i; 							  // 积分限幅
  float output, max_output; 				      // 输出、输出限幅
  int target;                                     // 目标
}PID;
PID power_pid;
volatile bool gCheckADC;        //ADC采集成功标志位
volatile uint16_t ADC_VALUE[BUFFER_SIZE];//ADC采集的数据保存地址
void OLED_Show();
float PID_Calc(PID *pid, float target, float current);
void PID_Init(PID* pid, float p, float i, float d, float maxI, float maxOut, int target);
PID Power_Control(int target_power);
void DMA_Reset(void* data, uint16_t size);

int main(void){
    char show_buff[100]={0};
    unsigned int adc_value = 0;
    float voltage_value = 0;
    //传入power_pid地址
    //设定 kp 为35
    //设定 ki 为6
    //设定 kd 为10
    //设定 I的最大值 为9999
    //设定 PID最大输出值 为9999
    //设定 目标值 为 98
    PID_Init(&power_pid, 35, 6, 10, 9999, 9999, 2000);

    /* 系统初始化 */
    SYSCFG_DL_init();
	//开启捕获中断
	NVIC_EnableIRQ(CAPTURE_0_INST_INT_IRQN);
	gCheckEdgeCaptureTime = false;
	DL_TimerG_startCounter(CAPTURE_0_INST);
	//开启按键引脚的GPIOB端口中断
    NVIC_EnableIRQ(KEY_INT_IRQN);
	//开启定时器
	DL_TimerG_startCounter(TIMER_0_INST);
	//通信初始化
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN );//清除中断标志??
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN );
    //OLED初始化
    OLED_Init();
    OLED_Clear();
    gCheckADC = false;
    //开启ADC中断
    NVIC_ClearPendingIRQ(ADC_Volt_INST_INT_IRQN );//
    NVIC_EnableIRQ(ADC_Volt_INST_INT_IRQN);    
	DL_ADC12_enableConversions(ADC_Volt_INST);
	//配置DMA地址等
	DL_ADC12_enableDMA(ADC_Volt_INST);
	DL_DMA_setSrcAddr(DMA,DMA_CH0_CHAN_ID,DL_ADC12_getMemResultAddress(ADC_Volt_INST,DL_ADC12_MEM_IDX_0));//DL_ADC12_getMemResultAddress(ADC12_0_INST,DL_ADC12_MEM_IDX_0));
    DL_DMA_setDestAddr(DMA,DMA_CH0_CHAN_ID,(uint32_t)ADC_VALUE);
    DL_DMA_setTransferSize(DMA,DMA_CH0_CHAN_ID,BUFFER_SIZE);
    DL_DMA_enableChannel(DMA,DMA_CH0_CHAN_ID);
	
	
	
    
//	while(!gCheckADC){
//            continue;
//        }
//        DMA_Reset((void*)ADC_VALUE, BUFFER_SIZE);
    while (1){
		while (false == gCheckEdgeCaptureTime) {
            __WFE();
        }
		gCheckEdgeCaptureTime = false;
		period =TIMER_CAPTURE_DURATION - (DL_Timer_getCaptureCompareValue(CAPTURE_0_INST, DL_TIMER_CC_0_INDEX));
		DL_TimerG_setTimerCount(CAPTURE_0_INST, TIMER_CAPTURE_DURATION);
        if(gCheckADC){

			//功耗自动控制模块
			Power_Control(power_pid.target);
			delay_ms(50);
			gCheckADC = false;
		}
		//重新设置DMA的地址和数据块大小
		DMA_Reset((void*)ADC_VALUE, BUFFER_SIZE);
		//OLED显示模块
		OLED_Show();
		OLED_Refresh();
    }
}

void CAPTURE_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(CAPTURE_0_INST)) {
        case DL_TIMERG_IIDX_CC0_DN:
            gCheckEdgeCaptureTime  = true;
            break;
        default:
            break;
    }
}

//通信接受中断函数
void  UART_0_INST_IRQHandler(){
   switch (DL_UART_getPendingInterrupt(UART_0_INST )){ //检测是否串口中??
        case DL_UART_MAIN_IIDX_RX:
            choice = DL_UART_receiveData(UART_0_INST);
            DL_UART_transmitData(UART_0_INST,1);
            break;
        default:
            break;
    }
}

//DMA重新设置
void DMA_Reset(void* data, uint16_t size) {
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, DL_ADC12_getMemResultAddress(ADC_Volt_INST,DL_ADC12_MEM_IDX_0));
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)data);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, size);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
    return;
}

//ADC中断服务函数
void ADC_VOLTAGE_INST_IRQHandler(void){
	//查询并清除ADC中断
	switch (DL_ADC12_getPendingInterrupt(ADC_Volt_INST)){
		//检查是否完成数据采集
		case DL_ADC12_IIDX_DMA_DONE:
			gCheckADC = true;//将标志位置1
			break;
		default:
			break;
	}
}

void GROUP1_IRQHandler(void){//Group1的中断服务函数
    //读取Group1的中断寄存器并清除中断标志位
    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) ){
        //检查是否是KEY的GPIOB端口中断，注意是INT_IIDX，不是PIN_22_IIDX
        case KEY_INT_IIDX:
            //如果按键按下变为高电平
            if( DL_GPIO_readPins(KEY_PORT, KEY_B21_PIN) > 0 ){
                //设置LED引脚状态翻转
                DL_GPIO_togglePins(LED_PORT, LED_PIN_1_PIN);
                power_pid.target += 200;
                if(power_pid.target >2000){
                    power_pid.target =200;
                }
            }
        break;
    }
}
 void OLED_Show(void){
    //当前功耗
	OLED_ShowString(0,0,"Current Power");
	OLED_ShowNum(0,2,dac_value,5,16);
	//目标功耗
    OLED_ShowString(0,4,"Target Power");
    OLED_ShowNum(0,6,power_pid.target,5,16);
    OLED_ShowString(44,6,"mW");
 }            
  // 定时器中断处理
void TIMER_0_INST_IRQHandler(void)
{
   
}


 /****************************************************
功能：PID计算
参数：pid = pid的参数输入
     target = 目标值
     current = 当前值
返回：PID计算后的结果
****************************************************/
float PID_Calc(PID *pid, float target, float current)
{
    //用上一次的误差值更新 之前误差last_error
    pid->last_error = pid->error;
    //获取新的误差 = 目标值 - 当前值
    pid->error = target - current;

    //计算比例P = 目标值与实际值之间的误差e
    float pout = pid->error;
    //计算积分I = 误差e的累加
    pid->change_i += pid->error;
    //计算微分D = 当前误差e - 之前的误差last_e
    float dout = pid->error - pid->last_error;

    //积分I 限制不能超过正负最大值
    if(pid->change_i > pid->max_change_i)
    {
      pid->change_i = pid->max_change_i;
    }
    else if(pid->change_i < -pid->max_change_i)
    {
      pid->change_i = -pid->max_change_i;
    }

    //计算输出PID_OUT = （Kp x P）+ （Ki x I）+（Kd x D）
    pid->output = (pid->kp * pout) + (pid->ki * pid->change_i) + (pid->kd * dout);

    //输出 限制不能超过正负最大值
    if(pid->output > pid->max_output) pid->output = pid->max_output;
    else if(pid->output < -pid->max_output) pid->output = -pid->max_output;

    //返回PID计算的结果
    return pid->output;
}

/************************************************
功能：初始化PID各参数
参数：pid = 对应pid的结构体地址
      p = pid的静态kp值
      i = pid的静态ki值
      d = pid的静态kd值
      maxI = pid计算后的I最大值，即最大误差累加值
      maxOut = PID最大输出值
      target = 目标值
************************************************/
void PID_Init(PID* pid, float p, float i, float d, float maxI, float maxOut, int target)
{
    pid->kp = p;
    pid->ki = i;
    pid->kd = d;
    pid->max_change_i = maxI;
    pid->max_output = maxOut;
    pid->target = target;
}

/************************************************
功能：PID-DAC电压输出控制器
参数：target_power = 目标值
返回：对应PID的地址
************************************************/
PID Power_Control(int target_power)
{
	int temp;
    //传入PID静态参数、目标值（目标功率）、当前值（当前获取的实时功率）
    //PID输出的值为控制信号，传入到PWM变量中
	temp = PID_Calc( &power_pid, target_power, current_power);
    //设置DAC输出值
	// 将PID输出转换为DAC值 (0-4095)
    dac_value = (uint16_t)((temp * 4095) / power_pid.max_output);
    
    // 计算实际输出电压值(mV)
    dac_output = (dac_value * DAC12_REF_VOLTAGE_mV) / 4095;
    
    // 设置DAC输出
    DL_DAC12_output12(DAC0, dac_value);  
    DL_DAC12_enable(DAC0);
	return power_pid;
}
