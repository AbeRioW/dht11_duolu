#include "mqtt_publisher.h"
#include "esp8266.h" // 包含ESP8266 MQTT相关函数
#include "usart.h"       // 包含UART相关函数
#include "oled.h"
#include <stdio.h>
#include <string.h>

// 模块内部变量
static uint32_t last_pub = 0;
#define PUB_INTERVAL_MS 5000 // 发布间隔5000ms

// MQTT发布状态机
#define MQTT_STATE_IDLE 0
#define MQTT_STATE_PREPARING 1
#define MQTT_STATE_SENDING 2
static uint8_t mqtt_state = MQTT_STATE_IDLE;
static char mqtt_payload[256];
static uint32_t mqtt_start_time = 0;

// 外部变量定义
uint32_t msg_id = 0;

// 初始化MQTT发布状态机
void MQTT_Init(void)
{
    mqtt_state = MQTT_STATE_IDLE;
    last_pub = 0;
}

// 准备MQTT数据（非阻塞，立即返回）
void MQTT_Prepare_Data(const char* key, const char* value)
{
    uint32_t now = HAL_GetTick();
    
    // 检查是否在发布间隔内
    if ((uint32_t)(now - last_pub) < PUB_INTERVAL_MS) return;
    
    // 如果当前状态为空闲，开始准备数据
    if (mqtt_state == MQTT_STATE_IDLE) {
        // 构造JSON payload
        int n = snprintf(mqtt_payload, sizeof(mqtt_payload),
                         "{\\\"id\\\":\\\"%lu\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%s}}}",
                         (unsigned long)msg_id, key, value);
        
        if (n > 0 && n < (int)sizeof(mqtt_payload)) {
            mqtt_state = MQTT_STATE_PREPARING;
            mqtt_start_time = now;
        }
    }
}

// MQTT状态机处理（非阻塞，需要在主循环中定期调用）
void MQTT_Process(void)
{
    uint32_t now = HAL_GetTick();
    
    switch (mqtt_state) {
        case MQTT_STATE_IDLE:
            // 空闲状态，不做任何处理
            break;
            
        case MQTT_STATE_PREPARING:
            // 准备状态，等待短暂时间确保DHT11时序不受影响
            if ((uint32_t)(now - mqtt_start_time) > 10) {
                mqtt_state = MQTT_STATE_SENDING;
                mqtt_start_time = now;
            }
            break;
            
        case MQTT_STATE_SENDING:
            // 发送状态，尝试发布数据（最多等待100ms）
            if (ESP8266_MQTT_Publish(MQTT_TOPIC_POST, mqtt_payload, 0, 0)) {
                // 发布成功
                msg_id++;
                last_pub = now;
                mqtt_state = MQTT_STATE_IDLE;
            } else if ((uint32_t)(now - mqtt_start_time) > 100) {
                // 发布超时，放弃本次发布
                mqtt_state = MQTT_STATE_IDLE;
            }
            break;
    }
}

// 兼容性函数（保持原有接口）
void MQTT_Publish_Data(const char* key, const char* value)
{
    MQTT_Prepare_Data(key, value);
}


// 发布SET1标识符
//void MQTT_Publish_SET1(const char* value)
//{
//    MQTT_Publish_Data("SET1", value);
//}

//// 发布SET2标识符
//void MQTT_Publish_SET2(const char* value)
//{
//    MQTT_Publish_Data("SET2", value);
//}

//// 发布SET3标识符
//void MQTT_Publish_SET3(const char* value)
//{
//    MQTT_Publish_Data("SET3", value);
//}

//// 发布card1标识符
//void MQTT_Publish_card1(const char* value)
//{
//    MQTT_Publish_Data("card1", value);
//}

//// 发布card2标识符
//void MQTT_Publish_card2(const char* value)
//{
//    MQTT_Publish_Data("card2", value);
//}

//// 发布time_set标识符
//void MQTT_Publish_time_set(const char* value)
//{
//    MQTT_Publish_Data("time_set", value);
//}

//// 发布time_set标识符
//void MQTT_Publish_temp(const char* value)
//{
//    MQTT_Publish_Data("temperature", value);
//}

//// 发布time_set标识符
//void MQTT_Publish_humidity(const char* value)
//{
//    MQTT_Publish_Data("humidity", value);
//}

//void MQTT_Publish_co2(const char* value)
//{
//    MQTT_Publish_Data("CO2", value);
//}


//void MQTT_Publish_mq5(const char* value)
//{
//    MQTT_Publish_Data("MQ5", value);
//}


