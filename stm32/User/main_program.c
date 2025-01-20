#include "main_program.h"
#include <stdio.h>
#include "time_handle.h"
#include "oled.h"
#include "dht11.h"
#include "esp8266.h"
#include "led.h"
#include "light.h"
#include "soil_humidity.h"

/*
*************************************
�궨��
*************************************
*/

/*
*************************************
��������
*************************************
*/
extern uint16_t index_oled;
extern uint16_t index_dht11;
extern uint8_t index_send_msg;
extern uint8_t index_led;
/*
*************************************
��������
*************************************
*/

/*
*************************************
��������
*************************************
*/
uint8_t led_status =0;
uint8_t temp_value =0;
uint8_t humi_value =0;
uint16_t light_val = 0;
uint8_t opeCode;
uint8_t g_ucaDataBuff[2];//����
/*
*************************************
��������
*************************************
*/

/**
  * @brief          ��ʼ������,�൱��Arduino��setup()����,ֻ�ڳ�ʼ��ʱ��ִ��һ��
  * @param[in]      none
  * @retval         none
  */

void user_init_program(void)
{
  OLED_init(); 
  esp8266_init();
	BH1750_Init();			/*��ʼ��BH1750���ϵ�*/
  opeCode= BH1750_CONT_HI_RSLT_1;	/*���ù���ģʽΪ�����߷ֱ���ת����1lx���ȣ�120msת������*/
  while(dht11_init())
  {
    HAL_Delay(500);
    printf("dht11 init faild\r\n");
    OLED_printf (0,0,"dht11 init faild");
  }
  OLED_printf (0,0,"  Alibaba Cloud IOT ");
  time_slot_start();
}
/**
  * @brief          ��ʼ������,�൱��Arduino��loop()����,һֱִ�иú���
  * @param[in]      none
  * @retval         none
  */
void user_main_program(void)
{
	BH1750_WriteOpecode(&opeCode,1);
	HAL_Delay(180);
	BH1750_ReadData(g_ucaDataBuff,2);
  float light = (g_ucaDataBuff[0]<<8 | g_ucaDataBuff[1]) / 1.2f + 0.5f;
	uint16_t adc_value1 = Get_Soil_Humidity_Value();
    // ��ԭʼADCֵת��Ϊʵ�ʵ�ʪ��ֵ������ע��ת�����ʪ��ֵ���ͺͷ�Χ�Ƿ���Ϻ���ʹ��Ҫ��
  float humidity = Convert_To_Humidity(adc_value1);
  if(esp8266_receive_msg()!=1)
  {
  }
  if(index_dht11 ==1)
  {
    dht11_read_data(&temp_value,&humi_value);
    index_dht11=0;
  }
  if(index_led==1)
  {
    set_led(led_status);
    index_led=0;
  }
  if(index_oled==1)
  {
    OLED_printf (2,0,"temp:%d",temp_value);
    OLED_printf (4,0,"humi:%d",humi_value);
		OLED_printf (2,8,"light:%f",light);
    OLED_printf (4,8,"humidy:%f",humidity);
    index_oled=0;
  }
  if(index_send_msg==1)
	{
		index_send_msg =0;
		if(esp8266_send_msg()==0)
		{
		}
		else
		{
			printf("msg send error\r\n");
		}
	}
}

