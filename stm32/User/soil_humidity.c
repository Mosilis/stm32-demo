#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "soil_humidity.h"
#include "adc.h"


#define HUMIDITY_READ_TIMES 5  
#define HUMIDITY_CALIBRATION_COEFFICIENT 1.2  
#define HUMIDITY_OFFSET 5  



// 初始化土壤湿度传感器相关配置（主要是ADC配置）
void Soil_Humidity_Init(void)
{
    // 初始化ADC（这里假设基本配置已由STM32CubeMX完成，如时钟等）
    if (HAL_ADC_Init(&hadc2)!= HAL_OK)
    {
        Error_Handler();
    }
}

// 获取ADC转换后的原始数值（12位无符号整型）
uint16_t Get_Soil_Humidity_Value(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;  // 对应PA0配置为ADC2_IN0的通道
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;  // 采样时间，可根据实际情况调整
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig)!= HAL_OK)
    {
        Error_Handler();
    }

    // 启动ADC转换
    if (HAL_ADC_Start(&hadc2)!= HAL_OK)
    {
        Error_Handler();
    }

    // 等待转换完成并获取结果
    if (HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK)
    {
        return (uint16_t)HAL_ADC_GetValue(&hadc2);
    }
    return 0;
}

// 将ADC原始数值转换为实际的湿度值（这里只是简单示例，实际需根据传感器校准曲线调整转换关系）
float Convert_To_Humidity(uint16_t adc_value)
{
    uint32_t sum_adc_value = 0;
    for (uint8_t i = 0; i < HUMIDITY_READ_TIMES; i++)
    {
        sum_adc_value += adc_value;
        HAL_Delay(5);
    }
    sum_adc_value /= HUMIDITY_READ_TIMES;

    float humidity = (float)(ADC_RESOLUTION - sum_adc_value) / ADC_RESOLUTION * 100;
    humidity = humidity * HUMIDITY_CALIBRATION_COEFFICIENT + HUMIDITY_OFFSET;
    return humidity;
}