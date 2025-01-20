#include "stm32f1xx_hal.h"
#include "yuzhi.h"
#include "light.h"
#include "soil_humidity.h"
#include "tim.h"

// �������� ����
uint16_t lightup;
uint16_t turangdown;

//     ���������־   ���չ�ǿ��־
uint16_t flag_tu = 0, flag_light = 0;
// ��¼��һ��flag_light��ֵ�����ڶԱ��ж��Ƿ���Ҫת�����
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
    // ��ԭʼֵת��Ϊʵ�ʵĹ���ֵ
    float lig_val = (g_ucaDataBuff[0]<<8 | g_ucaDataBuff[1]) / 1.2f + 0.5f;
	  

    if (lig_val >= lightup) {
        flag_light = 1;
    }
    else {
        flag_light = 0;
    }

    // �жϵ�ǰ��flag_lightֵ����һ�ε�ֵ�Ƿ����˱仯
    if (prev_flag_light!= flag_light) {
        if (flag_light == 1) {
            // ����ʹ�õĶ�ʱ����TIM1��ͨ��1���ƶ��������־Ϊ1ʱ�����ƶ����ת180�ȣ���Ӧ2.5ms�ĸߵ�ƽʱ�䣨����ǰ�����õ����ںͼ����Ȳ����������Ӧ����ֵΪ2500���ң�
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 2500);
            // �����������ʵ���ʱ���ȴ����ת����λ������ʹ��HAL�����ʱ����
            HAL_Delay(1000);
        }
        else if (flag_light == 0) {
            // ����־Ϊ0ʱ�����ƶ����ת����������ת��90�ȣ���Ӧ1.5ms�ߵ�ƽʱ�䣬����ֵ1500��
            __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 1500);
            // ͬ������ʵ���ʱ�ȴ����ת����λ
            HAL_Delay(1000);
        }
        // ����prev_flag_light��ֵ����¼��ǰ��ε�flag_light״̬�������´ζԱ�
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