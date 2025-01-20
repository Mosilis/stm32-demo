#ifndef SOIL_HUMIDITY_H
#define SOIL_HUMIDITY_H

#include "stm32f1xx_hal.h"  // 根据你实际使用的STM32芯片系列修改这里，比如stm32f1xx_hal.h、stm32f4xx_hal.h等

#define ADC_RESOLUTION  4096  // 12位ADC的分辨率，2^12 = 4096

// 函数声明
void Soil_Humidity_Init(void);
uint16_t Get_Soil_Humidity_Value(void);
float Convert_To_Humidity(uint16_t adc_value);

#endif