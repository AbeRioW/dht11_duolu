#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "stm32f1xx_hal.h" // 根据实际MCU型号修改

// 外部变量声明（如果需要在其他地方访问）
extern uint32_t msg_id;

// 发布标识符类型
typedef enum {
    MQTT_PUB_SET1 = 0,
    MQTT_PUB_SET2,
    MQTT_PUB_SET3,
    MQTT_PUB_CARD1,
    MQTT_PUB_CARD2,
    MQTT_PUB_TIME_SET
} MQTT_Publish_Type;

// 发布数据到OneNET
void MQTT_Publish_Data(const char* key, const char* value);

// 发布SET1标识符
void MQTT_Publish_SET1(const char* value);

// 发布SET2标识符
void MQTT_Publish_SET2(const char* value);

// 发布SET3标识符
void MQTT_Publish_SET3(const char* value);

// 发布card1标识符
void MQTT_Publish_card1(const char* value);

// 发布card2标识符
void MQTT_Publish_card2(const char* value);

// 发布time_set标识符
void MQTT_Publish_time_set(const char* value);
void MQTT_Publish_temp(const char* value);
void MQTT_Publish_humidity(const char* value);
void MQTT_Publish_co2(const char* value);
void MQTT_Publish_mq5(const char* value);

#endif
