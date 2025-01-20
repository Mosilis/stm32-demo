/*
 * dht11.h
 *
 *  Created on: 2022?6?17?
 *      Author: 28980
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"

uint8_t dht11_init (void);
uint8_t dht11_read_data(uint8_t *temp,uint8_t *humi);

#endif /* INC_DHT11_H_ */

