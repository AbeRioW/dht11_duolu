#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// WiFi配置信息,备注WIFI模块只能连接2.4G的网路
#define WIFI_SSID       "CMCC-402-2.4G-ext"               // WiFi名称
#define WIFI_PASSWORD   "13851901825"            // WiFi密码

// MQTT三元组信息
#define PRODUCT_ID      "dU5jVg1L9b"          // 产品ID
#define DEVICE_NAME     "test"                // 设备名称
#define MQTT_TOKEN      "version=2018-10-31&res=products%2FdU5jVg1L9b%2Fdevices%2Ftest&et=2810377042&method=md5&sign=1wRzfPZDjJ6ztNqWYY9lIg%3D%3D"  // 设备token
//#define MQTT_TOKEN      "cFRrYmZrVHlsdkx6UXhwd0ZBbGpBb0psYzJTdk5MMGM="  // 设备token

// MQTT 主题宏定义
#define MQTT_TOPIC_POST        "$sys/dU5jVg1L9b/test/thing/property/post"         // 发布属性主题
#define MQTT_TOPIC_POST_REPLY  "$sys/dU5jVg1L9b/test/thing/property/post/reply"   // 发布属性回复主题
#define MQTT_TOPIC_SET_REPLY   "$sys/dU5jVg1L9b/test/thing/property/set_reply"    // 回复设置主题
#define MQTT_TOPIC_SET         "$sys/dU5jVg1L9b/test/thing/property/set"          // 订阅设置主题

// 接收状态定义
#define REV_OK      1
#define REV_WAIT    0

// 缓冲区大小配置
#define ESP8266_BUF_SIZE 512

// 全局变量声明（供中断回调使用）
extern volatile uint8_t esp8266_buf[ESP8266_BUF_SIZE];
extern volatile uint16_t esp8266_cnt;
extern volatile uint16_t esp8266_cntPre;
//extern uint8_t esp8266_rx_byte;

extern bool led1_status;
extern bool led2_status;

// 函数声明
void ESP8266_Clear(void);
bool ESP8266_WaitRecive(void);
bool ESP8266_SendCmd(const char *cmd, const char *expected_resp);
void ESP8266_Init(void);
bool ESP8266_ConnectWiFi(void);
bool ESP8266_ConnectCloud(void);

bool ESP8266_MQTT_Subscribe(const char *topic, uint8_t qos);
bool ESP8266_MQTT_Publish(const char *topic, const char *payload, uint8_t qos, uint8_t retain);

void ESP8266_ProcessMessages(void);

#endif /* __ESP8266_H */