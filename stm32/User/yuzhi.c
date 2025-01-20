#include "stm32f1xx_hal.h"
#include "yuzhi.h"
#include "light.h"
#include "soil_humidity.h"
#include "tim.h"

// 光照上限 下限
uint16_t lightup;
uint16_t turangdown;

//     土壤干燥标志   光照过强标志
uint16_t flag_tu = 0, flag_light = 0;
// 记录上一次flag_light的值，用于对比判断是否需要转动舵机
uint16_t prev_flag_light = 0; 
uint16_t prev_flag_turang = 0; 

void limit_handle_init() {
    lightup = 100;
	  turangdown = 10;

}

void limit_handle(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_Delay(180);
    uint8_t g_ucaDataBuff[2];
    BH1750_ReadData(g_ucaDataBuff,2);
    // 将原始值转换为实际的光照值
    float lig_val = (g_ucaDataBuff[0]<<8 | g_ucaDataBuff[1]) / 1.2f + 0.5f;
	  

    if (lig_val >= lightup) {
        flag_light = 1;
    }
    else {
        flag_light = 0;
    }

    // 判断当前的flag_light值与上一次的值是否发生了变化
    if (prev_flag_light!= flag_light) {
        if (flag_light == 1) {
            // 假设使用的定时器是TIM1，通道1控制舵机，当标志为1时，控制舵机旋转180度，对应2.5ms的高电平时间（根据前面设置的周期和计数等参数，这里对应脉冲值为2500左右）
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 2500);
            // 这里可以添加适当延时，等待舵机转动到位，比如使用HAL库的延时函数
            HAL_Delay(1000);
        }
        else if (flag_light == 0) {
            // 当标志为0时，控制舵机回转，这里假设回转到90度（对应1.5ms高电平时间，脉冲值1500）
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 1500);
            // 同样添加适当延时等待舵机转动到位
            HAL_Delay(1000);
        }
        // 更新prev_flag_light的值，记录当前这次的flag_light状态，用于下次对比
        prev_flag_light = flag_light;
    }
}
void limit_handle2(void){
	  uint16_t adc_vall = Get_Soil_Humidity_Value();
    float tu_val = Convert_To_Humidity(adc_vall);
	
	 if (tu_val >= turangdown) {
        flag_tu = 0;
    }
    else{
        flag_tu = 1;
    }
	if(prev_flag_turang!= flag_tu){
		if(flag_tu==1){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); 
			HAL_Delay(1000);
		}
		if(flag_tu==0){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); 
			HAL_Delay(1000);
		}
		prev_flag_turang = flag_tu;
	}
}