#include "esp8266.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "oled.h"
#include "core_json.h"
#include "soil_humidity.h"
#include "light.h"

/*
*************************************
宏定义
*************************************
*/
#define WIFI_SSID        "bbb"
#define WIFI_PASSWD      "wuwuwuvip666"

#define MQTT_CLIENT_ID   "k0kb9AENSWP.test1|securemode=2\\,signmethod=hmacsha256\\,timestamp=1735733905866|"   
#define MQTT_USER_NAME   "test1&k0kb9AENSWP"
#define MQTT_PASSWD      "e98352eb3e4cffd8ef6f47c511c20f33e05ed63f3bdba9e611f49c3fdafa2320"
#define BROKER_ASDDRESS  "k0kb9AENSWP.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define SUB_TOPIC        "/sys/k0kb9AENSWP/test1/thing/event/property/set"
#define PUB_TOPIC        "/sys/k0kb9AENSWP/test1/thing/event/property/post"
#define JSON_FORMAT      "{\\\"params\\\":{\\\"temperature\\\":%d\\,\\\"humidity\\\":%d\\,\\\"lightIntensity\\\":%d\\,\\\"humidity1\\\":%d\\,}\\,\\\"version\\\":\\\"1.0.0\\\"}"

/*
*************************************
宏定义
*************************************
*/
/*
*************************************
变量声明
*************************************
*/
/*
*************************************
变量定义
*************************************
*/
unsigned char receive_buf[512];	  //串口2接收缓存数组
unsigned char receive_start = 0;	//串口2接收开始标志位
uint16_t receive_count = 0;	      //串口2接收数据计数器
uint16_t receive_finish = 0;	    //串口2接收结束标志位 

extern uint8_t led_status;
extern uint8_t temp_value;
extern uint8_t humi_value;
extern uint16_t light_val;
/**
  * @brief          解析json数据
  * @param[in]      json_msg:json数据,json_len:json数据长度
  * @retval         返回0找到指定json数据，否则返回1
  */
uint8_t parse_json_msg(uint8_t *json_msg,uint8_t json_len)
{
  uint8_t retval =0;
  
  JSONStatus_t result;
  char query[] = "params.led";
  size_t queryLength = sizeof( query ) - 1;
  char * value;
  size_t valueLength;
  result = JSON_Validate((const char *)json_msg, json_len);
  if( result == JSONSuccess)
  {
    result = JSON_Search((char *)json_msg, json_len, query, queryLength,&value, &valueLength );
    if( result == JSONSuccess)
    {
      char save = value[valueLength];
      value[valueLength] = '\0';
      printf("Found: %s %d-> %s\n", query, valueLength,value);
      value[valueLength] = save;
      led_status = atoi(value);
      retval = 0;
    }
    else
    {
      retval = 1;
    }
  }
  else
  {
    retval = 1;
  }  
  return retval;
}
/**
  * @brief          串口2数据接收处理函数
  * @param[in]      none
  * @retval         none
  */
void uart2_receiver_handle(void)
{
  unsigned char receive_data = 0;   
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE) != RESET)
  {
    HAL_UART_Receive(&huart2, &receive_data, 1, 1000);//串口2接收1位数据
    receive_buf[receive_count++] = receive_data;
    receive_start = 1;	                              //串口2接收数据开始标志位置1
    receive_finish = 0;	                              //串口2接收数据完成标志位清0
  }
}
/**
  * @brief          串口2数据接收清0函数
  * @param[in]      len:清空的数据长度
  * @retval         none
  */
void uart2_receiver_clear(uint16_t len)	
{
	memset(receive_buf,0x00,len);
	receive_count = 0;
	receive_start = 0;
	receive_finish = 0;
}
/**
  * @brief          esp8266发送命令函数
  * @param[in]      cmd:发送的命令,len:命令的长度,rec_data:期望接收数据
  * @retval         none
  */
uint8_t esp8266_send_cmd(unsigned char *cmd,unsigned char len,char *rec_data)	
{
  unsigned char retval =0;
  unsigned int count = 0;

  HAL_UART_Transmit(&huart2, cmd, len, 1000);	                                   
  while((receive_start == 0)&&(count<1000))
  {
    count++;
    HAL_Delay(1);
  }

  if(count >= 1000)	
  {
    retval = 1;	
  }
  else	
  {
    do
    {
      receive_finish++;
      HAL_Delay(1);
    }
    while(receive_finish < 500);
    retval = 2;
    if(strstr((const char*)receive_buf, rec_data))	
    {
      retval = 0;	
    }
  }
  uart2_receiver_clear(receive_count);
  return retval;
}
/**
  * @brief          esp8266配置wifi网络
  * @param[in]      none
  * @retval         网络配置成功返回0,否则返回1
  */
uint8_t esp8266_config_network(void)
{
	uint8_t retval =0;
	uint16_t count = 0;
	
	HAL_UART_Transmit(&huart2, (unsigned char *)"AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n",strlen("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWD"\"\r\n"), 1000);
	
	while((receive_start == 0)&&(count<1000))
	{
		count++;
		HAL_Delay(1);
	}
	
	if(count >= 1000)	
	{
		retval = 1;	
	}
	else
	{
		HAL_Delay(8000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
		}
    else
    {
      retval = 1;
    }
	}
  uart2_receiver_clear(receive_count);
	return retval;
}
/**
  * @brief          esp8266连接服务
  * @param[in]      none
  * @retval         连接成功返回0,否则返回1
  */
uint8_t esp8266_connect_server(void)
{
	uint8_t retval=0;
	uint16_t count = 0;

	HAL_UART_Transmit(&huart2, (unsigned char *)"AT+MQTTCONN=0,\""BROKER_ASDDRESS"\",1883,0\r\n",strlen("AT+MQTTCONN=0,\""BROKER_ASDDRESS"\",1883,0\r\n"), 1000);	
	while((receive_start == 0)&&(count<1000))	
	{
		count++;
		HAL_Delay(1);
	}
	
	if(count >= 1000)	
	{
		retval = 1;	
	}
	else	
	{
		HAL_Delay(5000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
		}
    else
    {
      retval = 1;	
    }
	}
  uart2_receiver_clear(receive_count);	
	return retval;
}
/**
  * @brief          esp8266复位
  * @param[in]      none
  * @retval         返回0复位成功,返回1复位失败
  */
uint8_t esp8266_reset(void)
{
	uint8_t retval =0;
	uint16_t count = 0;
	
	HAL_UART_Transmit(&huart2, (unsigned char *)"AT+RST\r\n",8, 1000);
	while((receive_start == 0)&&(count<2000))	
	{
		count++;
		HAL_Delay(1);
	}
	if(count >= 2000)	
	{
		retval = 1;	
	}
	else	
	{
		HAL_Delay(5000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
		}
    else
    {
      retval = 1;	
    }
	}
  uart2_receiver_clear(receive_count);	
	return retval;
}
/**
  * @brief          esp8266发送数据
  * @param[in]      none
  * @retval         返回0发送数据成功,返回1发送数据失败
  */
uint8_t esp8266_send_msg(void)	
{
	uint8_t retval =0;	
  uint16_t count = 0;			
	static uint8_t error_count=0;
	unsigned char msg_buf[256];
  // 获取光照传感器的值
	uint8_t g_ucaDataBuff[2];
    BH1750_ReadData(g_ucaDataBuff,2);
    // 将原始值转换为实际的光照值
    float light = (g_ucaDataBuff[0]<<8 | g_ucaDataBuff[1]) / 1.2f + 0.5f;
    // 这里假设你最终要发送的是光照值取整后的整型数据，你可以根据阿里云要求调整为合适的数据类型和格式
    uint8_t light_int = (uint8_t)light;  
	// 获取土壤湿度传感器的原始ADC值
    uint16_t adc_value1 = Get_Soil_Humidity_Value();
    // 将原始ADC值转换为实际的湿度值（这里注意转换后的湿度值类型和范围是否符合后续使用要求）
    float humidity = Convert_To_Humidity(adc_value1);
    // 这里假设你最终要发送的是湿度值取整后的整型数据，你可以根据阿里云要求调整为合适的数据类型和格式
    uint8_t humidity_int = (uint8_t)humidity;  
  sprintf((char *)msg_buf,"AT+MQTTPUB=0,\""PUB_TOPIC"\",\""JSON_FORMAT"\",0,0\r\n",temp_value,humi_value,light_int,humidity_int);
	HAL_UART_Transmit(&huart2, (unsigned char *)msg_buf,strlen((const char *)msg_buf), 1000);	
  HAL_UART_Transmit(&huart1, (unsigned char *)msg_buf,strlen((const char *)msg_buf), 1000);	
	while((receive_start == 0)&&(count<500))	
	{
		count++;
		HAL_Delay(1000);
	}
	if(count >= 500)	
	{
		retval = 1;	
	}
	else	
	{
		HAL_Delay(1000);
		if(strstr((const char*)receive_buf, "OK"))	
		{
			retval = 0;	
			error_count=0;
		}
		else 
		{
			error_count++;
			if(error_count==5)
			{
				error_count=0;
        printf("RECONNECT MQTT BROKER!!!\r\n");
				esp8266_init();
			}
		}
	}
  uart2_receiver_clear(receive_count);	
	return retval;
}

/**
  * @brief          esp8266接收数据
  * @param[in]      none
  * @retval         返回0接收数据正常,返回1接收数据异常或无数据
  */
uint8_t esp8266_receive_msg(void)
{
    // 函数返回值，初始化为 0
    uint8_t retval = 0;
    // 存储消息长度
    int msg_len = 0;
    // 存储消息体，最大长度为 128 字节，初始化为 0
    uint8_t msg_body[128] = {0};

    // 检查是否开始接收
    if (receive_start == 1)
    {
        do
        {
            // 增加接收完成标志
            receive_finish++;
            // 延迟 500 毫秒
            HAL_Delay(500);
        }
        // 等待接收完成标志达到 5
        while (receive_finish < 5);

        // 检查接收缓冲区是否包含 "+MQTTSUBRECV:"
        if (strstr((const char*)receive_buf, "+MQTTSUBRECV:"))
        {
            // 从接收缓冲区解析消息长度和消息体
            sscanf((const char *)receive_buf, "+MQTTSUBRECV:0,\""SUB_TOPIC"\",%d,%s", &msg_len, msg_body);
            // 打印消息长度和消息体
            printf("len:%d,msg:%s\r\n", msg_len, msg_body);
            // 检查消息体长度是否与解析出的长度相等
            if (strlen((const char*)msg_body) == msg_len)
            {
                // 解析 JSON 消息
                retval = parse_json_msg(msg_body, msg_len);
            }
            else
            {
                // 长度不匹配，返回 1
                retval = 1;
            }
        }
        else
        {
            // 未包含 "+MQTTSUBRECV:"，返回 1
            retval = 1;
        }
    }
    else
    {
        // 未开始接收，返回 1
        retval = 1;
    }
    // 清除 UART2 接收缓冲区
    uart2_receiver_clear(receive_count);
    // 返回结果
    return retval;
}
/**
  * @brief          esp8266初始化
  * @param[in]      none
  * @retval         none
  */
void esp8266_init(void)
{
	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);           											//打开串口2接收中断
	
	printf("1.SETTING STATION MODE\r\n");
  OLED_printf(0,0,"1.SETTING STATION MODE                   ");
	while(esp8266_send_cmd((uint8_t *)"AT+CWMODE=1\r\n",strlen("AT+CWMODE=1\r\n"),"OK")!=0)
	{
		HAL_Delay(1000);
	}
	printf("2.CLOSE ESP8266 ECHO\r\n");
  OLED_printf(0,0,"2.CLOSE ESP8266 ECHO                     ");
	while(esp8266_send_cmd((uint8_t *)"ATE0\r\n",strlen("ATE0\r\n"),"OK")!=0)
	{
		HAL_Delay(1000);
	}
	printf("3.NO AUTO CONNECT WIFI\r\n"); 
  OLED_printf(0,0,"3.NO AUTO CONNECT WIFI                   ");
	while(esp8266_send_cmd((uint8_t *)"AT+CWAUTOCONN=0\r\n",strlen("AT+CWAUTOCONN=0\r\n"),"OK")!=0)
	{
		HAL_Delay(1000);
	}
	printf("4.RESET ESP8266\r\n");
  OLED_printf(0,0,"4.RESET ESP8266                          ");
	while(esp8266_reset() != 0)
	{
		HAL_Delay(5000);
	}
	printf("5.CONFIG WIFI NETWORK\r\n");
  OLED_printf(0,0,"5.CONFIG WIFI NETWORK                    ");
	while(esp8266_config_network() != 0)
	{
		HAL_Delay(8000);
	}
	printf("6.MQTT USER CONFIG\r\n");
  OLED_printf(0,0,"6.MQTT USER CONFIG                       ");
	while(esp8266_send_cmd((uint8_t *)"AT+MQTTUSERCFG=0,1,\""MQTT_CLIENT_ID"\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n",
                          strlen("AT+MQTTUSERCFG=0,1,\""MQTT_CLIENT_ID"\",\""MQTT_USER_NAME"\",\""MQTT_PASSWD"\",0,0,\"\"\r\n"),"OK")!=0)
	{
		HAL_Delay(2000);
	}
	printf("7.CONNECT MQTT BROKER\r\n");
  OLED_printf(0,0,"7.CONNECT MQTT BROKER                    ");
	while(esp8266_connect_server() != 0)
	{
		HAL_Delay(8000);
	}
	printf("8.SUBSCRIBE TOPIC\r\n");
  OLED_printf(0,0,"8.SUBSCRIBE TOPIC                ");
	while(esp8266_send_cmd((uint8_t *)"AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n",strlen("AT+MQTTSUB=0,\""SUB_TOPIC"\",0\r\n"),"OK")!=0)
	{
		HAL_Delay(2000);
	}
	printf("9.ESP8266 INIT OK!!!\r\n");
  OLED_printf(0,0,"9.ESP8266 INIT OK!!!                ");
}





























