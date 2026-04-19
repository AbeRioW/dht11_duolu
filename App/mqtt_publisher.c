#include "mqtt_publisher.h"
#include "esp8266.h" // 包含ESP8266 MQTT相关函数
#include "usart.h"       // 包含UART相关函数
#include "oled.h"
#include <stdio.h>
#include <string.h>

// 模块内部变量
static uint32_t last_pub = 0;
#define PUB_INTERVAL_MS 1000 // 发布间隔1000ms

// 外部变量定义
uint32_t msg_id = 0;

// 通用发布函数（非阻塞式）
void MQTT_Publish_Data(const char* key, const char* value)
{
//    uint32_t now = HAL_GetTick();
//    
//    // 非阻塞定时检查
//    if ((uint32_t)(now - last_pub) < PUB_INTERVAL_MS) return;
//    last_pub = now;

    // 构造JSON payload
    char payload[256];
    int n = snprintf(payload, sizeof(payload),
                     "{\\\"id\\\":\\\"%lu\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%s}}}",
                     (unsigned long)msg_id, key, value);

    
    if (n > 0 && n < (int)sizeof(payload))
    {

        if (ESP8266_MQTT_Publish(MQTT_TOPIC_POST, payload, 0, 0))
        {

            msg_id++;
        }
    }
}

// 发布SET1标识符
void MQTT_Publish_SET1(const char* value)
{
    MQTT_Publish_Data("temperature1", value);
}

// 发布SET2标识符
void MQTT_Publish_SET2(const char* value)
{
    MQTT_Publish_Data("SET2", value);
}

// 发布SET3标识符
void MQTT_Publish_SET3(const char* value)
{
    MQTT_Publish_Data("SET3", value);
}

// 发布card1标识符
void MQTT_Publish_card1(const char* value)
{
    MQTT_Publish_Data("card1", value);
}

// 发布card2标识符
void MQTT_Publish_card2(const char* value)
{
    MQTT_Publish_Data("card2", value);
}

// 发布time_set标识符
void MQTT_Publish_time_set(const char* value)
{
    MQTT_Publish_Data("time_set", value);
}

// 发布time_set标识符
void MQTT_Publish_temp(const char* value)
{
    MQTT_Publish_Data("temperature1", value);
}

// 发布time_set标识符
void MQTT_Publish_humidity(const char* value)
{
    MQTT_Publish_Data("humidity1", value);
}

void MQTT_Publish_co2(const char* value)
{
    MQTT_Publish_Data("CO2", value);
}


void MQTT_Publish_mq5(const char* value)
{
    MQTT_Publish_Data("MQ5", value);
}


