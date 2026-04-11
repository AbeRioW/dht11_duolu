#ifndef __DHT11_H
#define __DHT11_H

#include "stdint.h"
#include "main.h"

// DHT11数据结构
typedef struct {
    uint8_t humidity;
    uint8_t temperature;
} DHT11_Data;

// DHT11引脚定义
#define DHT11_1_PORT DHT11_1_GPIO_Port
#define DHT11_1_PIN DHT11_1_Pin

#define DHT11_2_PORT DHT11_2_GPIO_Port
#define DHT11_2_PIN DHT11_2_Pin

#define DHT11_3_PORT DHT11_3_GPIO_Port
#define DHT11_3_PIN DHT11_3_Pin

#define DHT11_4_PORT DHT11_4_GPIO_Port
#define DHT11_4_PIN DHT11_4_Pin

// 函数声明
uint8_t DHT11_ReadData(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, DHT11_Data* data);

#endif