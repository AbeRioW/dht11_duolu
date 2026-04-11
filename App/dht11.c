#include "dht11.h"
#include "main.h"

// 微秒级延时函数
void DHT11_Delay_us(uint32_t us)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < (us / 1000));
}

// 读取一个位
uint8_t DHT11_ReadBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    uint8_t retry = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET && retry < 100) {
        retry++;
        DHT11_Delay_us(1);
    }
    retry = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET && retry < 100) {
        retry++;
        DHT11_Delay_us(1);
    }
    return (retry > 30) ? 1 : 0;
}

// 读取一个字节
uint8_t DHT11_ReadByte(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= DHT11_ReadBit(GPIOx, GPIO_Pin);
    }
    return byte;
}

// 读取DHT11数据
uint8_t DHT11_ReadData(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, DHT11_Data* data)
{
    // 1. 发送开始信号
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
    DHT11_Delay_us(30);
    
    // 2. 切换到输入模式
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    // 3. 等待DHT11响应
    uint8_t retry = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_SET && retry < 100) {
        retry++;
        DHT11_Delay_us(1);
    }
    if (retry >= 100) return 1;
    
    retry = 0;
    while (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == GPIO_PIN_RESET && retry < 100) {
        retry++;
        DHT11_Delay_us(1);
    }
    if (retry >= 100) return 1;
    
    // 4. 读取数据
    uint8_t buf[5];
    for (uint8_t i = 0; i < 5; i++) {
        buf[i] = DHT11_ReadByte(GPIOx, GPIO_Pin);
    }
    
    // 5. 校验数据
    if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4]) {
        data->humidity = buf[0];
        data->temperature = buf[2];
        return 0;
    } else {
        return 1;
    }
}