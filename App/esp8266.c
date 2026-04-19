#include "esp8266.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <stdarg.h>
#include "cJSON.h"
#include "gpio.h"
#include "stdlib.h"
#include "oled.h"

        
// 全局变量定义
volatile uint8_t esp8266_buf[ESP8266_BUF_SIZE];          //接收缓冲区，存储 ESP8266 返回的数据
volatile uint16_t esp8266_cnt = 0, esp8266_cntPre = 0;   //缓冲区计数变量，用于判断接收是否完成
// uint8_t esp8266_rx_byte = 0;                    //中断接收的单个字节临时存储(可保留或删除，DMA 不用)


/* 等待缓冲中出现指定字符串（简单实现，单位 ms） */
static bool ESP8266_WaitForStr(const char *str, uint32_t timeout_ms)
{
    uint32_t ticks = timeout_ms / 10;
    while(ticks--)
    {
        HAL_Delay(10);
     // 快照长度并复制到本地缓冲，避免并发读取问题
        uint16_t len = esp8266_cnt;
        if (len)
        {
            if (len > ESP8266_BUF_SIZE - 1) len = ESP8266_BUF_SIZE - 1;
            char check_buf[ESP8266_BUF_SIZE];
            memcpy(check_buf, (const char*)esp8266_buf, len);
            check_buf[len] = '\0';
            if (str && strstr(check_buf, str) != NULL) return true;
        }
    }
    return false;
}

/**
*	函数名称：	ESP8266_Clear
*	函数功能：	清空接收缓存
*/
void ESP8266_Clear(void)
{
    memset((void *)esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
}

/**
*   函数名称：	ESP8266_WaitRecive
*	函数功能：	等待接收完成
*	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
*/
bool ESP8266_WaitRecive(void)
{
    if(esp8266_cnt == 0)
        return REV_WAIT;

    if(esp8266_cnt == esp8266_cntPre)
    {
        // 接收数据稳定，返回 OK（不要在这里清空 esp8266_cnt，避免竞态）
        esp8266_cntPre = 0;
        return REV_OK;
    }

    esp8266_cntPre = esp8266_cnt;
    return REV_WAIT;
}

/**
*	函数名称：	ESP8266_SendCmd
*	函数功能：	发送AT指令并等待响应
*	入口参数：	cmd：指令内容	expected_resp：期望的响应
*	返回参数：	true-成功	false-失败
*/
bool ESP8266_SendCmd(const char *cmd, const char *expected_resp)
{
  uint16_t timeOut = 60000; // 增加到60秒，以处理MQTT连接的延迟

    // 清空接收缓冲区
    ESP8266_Clear();

    // 发送指令
    HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), 200);

    // 给模块一点时间开始回复（针对长命令适当延时）
    HAL_Delay(50);

    // 等待响应
    while(timeOut--)
    {
        // 直接检查缓冲区，不依赖ESP8266_WaitRecive，避免错过+MQTTCONNECTED响应
        if(esp8266_cnt > 0)
        {
            // 等待更多字节到达，避免分片丢失
            HAL_Delay(200);

            // 拷贝缓冲到本地（用 esp8266_cnt 快照）
            char local_buf[ESP8266_BUF_SIZE];
            uint16_t len = esp8266_cnt;
            if (len > ESP8266_BUF_SIZE - 1) len = ESP8266_BUF_SIZE - 1;
            memcpy(local_buf, (const char*)esp8266_buf, len);
            local_buf[len] = '\0';

            // 如果包含期望响应，返回成功
            if(expected_resp && strstr(local_buf, expected_resp) != NULL)
            {
                ESP8266_Clear();
                return true;
            }

            // 清空缓冲区，避免数据积累
            ESP8266_Clear();
        }
        HAL_Delay(10);
    }

    // 超时，拷贝并打印一次缓冲以便排查
    {
        char dump_buf[ESP8266_BUF_SIZE];
        uint16_t dump_len = esp8266_cnt;
        if (dump_len > ESP8266_BUF_SIZE - 1) dump_len = ESP8266_BUF_SIZE - 1;
        memcpy(dump_buf, (const char*)esp8266_buf, dump_len);
        dump_buf[dump_len] = '\0';
    }

    ESP8266_Clear();
    return false;
}

/**
*	函数名称：	ESP8266_Reset
*	函数功能：	ESP8266硬件复位
*/
//void ESP8266_Reset(void){

//    // 拉低RST引脚复位
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
//    delay_ms(100);
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
//    delay_ms(2000); // 等待模块重启
//}

/**
*	函数名称：	ESP8266_Init
*	函数功能：	ESP8266基础初始化（测试+模式+DHCP）
*/
void ESP8266_Init(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0,(uint8_t*)"ESP8266 Init...", 8, 1);
    OLED_Refresh();
    HAL_Delay(500);

    // 启动串口 DMA 接收（替换原 HAL_UART_Receive_IT）
    HAL_UART_Receive_DMA(&huart1, (uint8_t*)esp8266_buf, ESP8266_BUF_SIZE - 1);
    
    // 测试AT指令
    OLED_ShowString(0, 8,(uint8_t*)"Testing AT...", 8, 1);
    OLED_Refresh();
    if(ESP8266_SendCmd("AT\r\n", "OK"))
    {
        OLED_ShowString(0, 8,(uint8_t*)"AT test OK", 8, 1);
        HAL_Delay(500);
    }else {
        OLED_ShowString(0, 8,(uint8_t*)"AT test failed", 8, 1);
        HAL_Delay(500);
    }
    OLED_Refresh();
    
    // 关闭回显，避免回显干扰后续匹配（ATE0）
    OLED_ShowString(0, 16,(uint8_t*)"Disabling echo...", 8, 1);
    OLED_Refresh();
    for (int i = 0; i < 3; ++i)
    {
        if (ESP8266_SendCmd("ATE0\r\n", "OK")) {
            OLED_ShowString(0, 16,(uint8_t*)"Echo disabled", 8, 1);
            break;
        }
        HAL_Delay(200);
    }
    OLED_Refresh();

    // 设置Station模式
    OLED_ShowString(0, 24,(uint8_t*)"Setting Station...", 8, 1);
    OLED_Refresh();
    if(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
    {
        OLED_ShowString(0, 24,(uint8_t*)"Station mode OK", 8, 1);
        HAL_Delay(500);
    }else{
        OLED_ShowString(0, 24,(uint8_t*)"Station mode failed", 8, 1);
        HAL_Delay(500);
    }
    OLED_Refresh();

    
    // 启用DHCP
    if(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
    {
        OLED_ShowString(0, 32,(uint8_t*)"DHCP enabled", 8, 1);
        HAL_Delay(500);
    }else{
        OLED_ShowString(0, 32,(uint8_t*)"Set DHCP failed", 8, 1);
        HAL_Delay(500);
    }
		OLED_Refresh();
		HAL_Delay(2000);
		OLED_Clear();
}

/**
 *	函数名称：	ESP8266_ConnectWiFi
 *	函数功能：	连接WiFi网络
 *	返回参数：	true-连接成功	false-连接失败
 */
bool ESP8266_ConnectWiFi(void)
{
    //const char *wifi_cmd = "AT+CWJAP=\"mhuan\",\"12345678\"\r\n";
    char wifi_cmd[128];  // 注意预留足够空间
    snprintf(wifi_cmd, sizeof(wifi_cmd), 
             "AT+CWJAP=\"%s\",\"%s\"\r\n", 
             WIFI_SSID, WIFI_PASSWORD); 
    
    OLED_ShowString(0, 0,(uint8_t*)"Connecting WiFi...", 8, 1);
    OLED_Refresh();

    
    // 连接WiFi，等待GOT IP响应
    if(ESP8266_SendCmd(wifi_cmd, "GOT IP"))
    {
        OLED_ShowString(0, 8,(uint8_t*)"WiFi connected", 8, 1);
        OLED_Refresh();
        HAL_Delay(500);
        OLED_Clear();
        return true;
    }
    else
    {
        OLED_ShowString(0, 8,(uint8_t*)"WiFi connect failed", 8, 1);
        OLED_Refresh();
        HAL_Delay(500);
        OLED_Clear();
        return false;
    }
}



/**
 *	函数名称：	ESP8266_ConnectCloud
 *	函数功能：	连接OneNET云平台(MQTT)
 *	返回参数：	true-连接成功	false-连接失败
 */
bool ESP8266_ConnectCloud(void)
{
    // 先断开之前的MQTT连接（如果有）
//    ESP8266_SendCmd("AT+MQTTDISCONN=0\r\n", "OK");
//    HAL_Delay(500);
    
    // 清除之前的MQTT配置
//    ESP8266_SendCmd("AT+MQTTUSERCFG=0,0,\"\",\"\",\"\",0,0,\"\"\r\n", "OK");
//    HAL_Delay(500);
    
    char mqtt_user_cfg[512];
    snprintf(mqtt_user_cfg, sizeof(mqtt_user_cfg), 
             "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
             DEVICE_NAME, PRODUCT_ID, MQTT_TOKEN);

    const char *mqtt_conn = "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n";
    
    OLED_ShowString(0, 0,(uint8_t*)"Connecting Cloud...", 8, 1);
    OLED_Refresh();
    
    // 配置MQTT用户信息，最多尝试5次
    int mqtt_user_cfg_try = 0;
    bool mqtt_user_cfg_success = false;
    bool auto_connected = false;  // 标记是否自动连接
    
    while (mqtt_user_cfg_try < 10 && !mqtt_user_cfg_success)
    {
        
        // 显示尝试次数
        char try_str[32];
        snprintf((char*)try_str, sizeof(try_str), "Try: %d/10", mqtt_user_cfg_try + 1);
        OLED_ShowString(0, 8, (uint8_t*)try_str, 8, 1);
        OLED_Refresh();
        
        // 清空接收缓冲区
        ESP8266_Clear();
        
        // 等待ESP8266空闲
        HAL_Delay(1000);
        
        // 发送长命令，增加超时时间到 5000ms
        HAL_UART_Transmit(&huart1, (uint8_t *)mqtt_user_cfg, strlen(mqtt_user_cfg), 5000);
        
        // 给ESP8266更多时间处理长命令
        HAL_Delay(1500);
        
        // 等待响应，增加超时时间
        uint16_t timeOut = 10000; // 增加到10秒
        bool got_response = false;
        
        while(timeOut--)
        {
            if(esp8266_cnt > 0) // 只要有数据就检查
            {
                // 等待更多字节到达
                HAL_Delay(300);
                
                char local_buf[ESP8266_BUF_SIZE];
                uint16_t len = esp8266_cnt;
                if (len > ESP8266_BUF_SIZE - 1) len = ESP8266_BUF_SIZE - 1;
                memcpy(local_buf, (const char*)esp8266_buf, len);
                local_buf[len] = '\0';
                
                if(strstr(local_buf, "OK") != NULL)
                {
                    got_response = true;
                    mqtt_user_cfg_success = true;
                    ESP8266_Clear();
                    break;
                }
                
                if(strstr(local_buf, "ERROR") != NULL)
                {
                    got_response = true;
                    ESP8266_Clear();
                    break;
                }
                
                // 清空缓冲区，避免死循环打印
                ESP8266_Clear();
            }
            HAL_Delay(10);
        }
        
        if(mqtt_user_cfg_success)
        {
            if(auto_connected)
            {
                OLED_ShowString(0, 16,(uint8_t*)"Auto connected", 8, 1);
                OLED_Refresh();
            }
            else
            {
                OLED_ShowString(0, 16,(uint8_t*)"User cfg OK", 8, 1);
                OLED_Refresh();
            }
        }
        else
        {
            OLED_ShowString(0, 16,(uint8_t*)"User cfg failed", 8, 1);
            OLED_Refresh();
            mqtt_user_cfg_try++;
            HAL_Delay(3000); // 等待3秒后重试
        }
    }
		//User cfg
    
    if(!mqtt_user_cfg_success)
    {
        OLED_ShowString(0, 24,(uint8_t*)"Cloud connect failed", 8, 1);
        OLED_Refresh();
        HAL_Delay(1000);
        OLED_Clear();
        return false;
    }
    
    // 如果没有自动连接，则手动发送连接命令
    if(!auto_connected)
    {
        OLED_ShowString(0, 24,(uint8_t*)"Connecting MQTT...", 8, 1);
        OLED_Refresh();
        
        // 连接MQTT服务器，最多尝试5次
        int mqtt_conn_try = 0;
        bool mqtt_conn_success = false;
        
        while (mqtt_conn_try < 5 && !mqtt_conn_success)
        {
            // 显示连接尝试次数
            char conn_try_str[32];
            snprintf((char*)conn_try_str, sizeof(conn_try_str), "Conn Try: %d/5", mqtt_conn_try + 1);
            OLED_ShowString(0, 32, (uint8_t*)conn_try_str, 8, 1);
            OLED_Refresh();
            
            // 连接MQTT服务器，期望收到+MQTTCONNECTED响应
					#if 1
					 HAL_UART_Transmit(&huart1, (uint8_t *)mqtt_conn, strlen(mqtt_conn), 5000);
					  return true;
					#else
//            if(ESP8266_SendCmd(mqtt_conn, "+MQTTCONNECTED"))
//            {
//                mqtt_conn_success = true;
//                OLED_ShowString(0, 32,(uint8_t*)"Cloud connected", 8, 1);
//                OLED_Refresh();
//                HAL_Delay(1000);
//                OLED_Clear();
//                return true;
//            }
//            else
//            {
//                OLED_ShowString(0, 32,(uint8_t*)"MQTT connect failed", 8, 1);
//                OLED_Refresh();
//                mqtt_conn_try++;
//                HAL_Delay(3000); // 等待3秒后重试
//            }
#endif
        }
        
        // 多次尝试后仍然失败
        OLED_ShowString(0, 32,(uint8_t*)"MQTT connect failed", 8, 1);
        OLED_Refresh();
        HAL_Delay(1000);
        OLED_Clear();
        return false;
    }
    
    // 已经自动连接成功
    OLED_ShowString(0, 32,(uint8_t*)"Cloud connected", 8, 1);
    OLED_Refresh();
    HAL_Delay(1000);
    OLED_Clear();
    return true;
}

/**
 * 订阅 MQTT 主题
 * topic: 主题字符串，不带引号
 * qos: 0/1/2
 */
bool ESP8266_MQTT_Subscribe(const char *topic, uint8_t qos)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    
    // 显示订阅状态
    OLED_ShowString(0, 0,(uint8_t*)"Subscribing...", 8, 1);
    char topic_str[64];
    snprintf((char*)topic_str, sizeof(topic_str), "Topic: %s", topic);
    OLED_ShowString(0, 8, (uint8_t*)topic_str, 8, 1);
    char qos_str[32];
    snprintf((char*)qos_str, sizeof(qos_str), "QoS: %d", qos);
    OLED_ShowString(0, 16, (uint8_t*)qos_str, 8, 1);
    OLED_Refresh();
    
    bool sub_result = ESP8266_SendCmd(cmd, "OK");
    
    if (sub_result)
    {
        OLED_ShowString(0, 24,(uint8_t*)"Subscribe OK", 8, 1);
        OLED_Refresh();
        HAL_Delay(1000);
        OLED_Clear();
        return true;
    }
    else
    {
        OLED_ShowString(0, 24,(uint8_t*)"Subscribe failed", 8, 1);
        OLED_ShowString(0, 32,(uint8_t*)"Retrying...", 8, 1);
        OLED_Refresh();
        HAL_Delay(200);
        
        sub_result = ESP8266_SendCmd(cmd, "OK");
        
        if (sub_result)
        {
            OLED_ShowString(0, 24,(uint8_t*)"Subscribe OK", 8, 1);
            OLED_ShowString(0, 32,(uint8_t*)"Retry success", 8, 1);
            OLED_Refresh();
            HAL_Delay(1000);
            OLED_Clear();
            return true;
        }
        else
        {
            OLED_ShowString(0, 24,(uint8_t*)"Subscribe failed", 8, 1);
            OLED_ShowString(0, 32,(uint8_t*)"Retry failed", 8, 1);
            OLED_Refresh();
            HAL_Delay(1000);
            OLED_Clear();
            return false;
        }
    }
}


/**
 * 发布 MQTT 消息
 * topic: 主题字符串，不带引号
 * payload: 要发送的 JSON 或文本
 * qos: 0/1/2
 * retain: 0/1
 */
/* 使用长度模式发布 MQTT（发送 AT+MQTTPUBRAW 带长度，等待 '>' 再发送 payload） */
bool ESP8266_MQTT_Publish(const char *topic, const char *payload, uint8_t qos, uint8_t retain)
{
    if(!topic || !payload) return false;

    char cmd[512];
    
    // 构造命令，使用与 tim.c 相同的格式，确保双引号和反斜杠正确转义
    // 格式：AT+MQTTPUB=0,"topic","payload",<qos>,<retain>
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",%d,%d\r\n", 
             topic, payload, qos, retain);

    // 显示发布状态
//    OLED_ShowString(0, 0,(uint8_t*)"Publishing...", 8, 1);
//    char topic_str[64];
//    snprintf((char*)topic_str, sizeof(topic_str), "Topic: %s", topic);
//    OLED_ShowString(0, 8, (uint8_t*)topic_str, 8, 1);
//    char qos_str[32];
//    snprintf((char*)qos_str, sizeof(qos_str), "QoS: %d, Retain: %d", qos, retain);
//    OLED_ShowString(0, 16, (uint8_t*)qos_str, 8, 1);
//    OLED_Refresh();

    // 清空接收缓存并发送命令
    ESP8266_Clear();
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 4000);

    // 非阻塞式等待，使用短时间轮询，让出CPU时间给其他任务
    uint32_t start = HAL_GetTick();
    while((HAL_GetTick() - start) < 500)  // 最多等待500ms
    {
        // 检查是否收到 OK
        uint16_t len = esp8266_cnt;
        if (len)
        {
            if (len > ESP8266_BUF_SIZE - 1) len = ESP8266_BUF_SIZE - 1;
            char check_buf[ESP8266_BUF_SIZE];
            memcpy(check_buf, (const char*)esp8266_buf, len);
            check_buf[len] = '\0';
            if (strstr(check_buf, "OK") != NULL) 
            {
//                OLED_ShowString(0, 24,(uint8_t*)"Publish OK", 8, 1);
//                OLED_Refresh();
//                HAL_Delay(500);
//                OLED_Clear();
                return true;
            }
            if (strstr(check_buf, "ERROR") != NULL)
            {
//                OLED_ShowString(0, 24,(uint8_t*)"Publish failed", 8, 1);
//                OLED_Refresh();
//                HAL_Delay(500);
//                OLED_Clear();
                return false;
            }
        }
        // 短暂延时让出CPU，不阻塞其他任务
        HAL_Delay(1);
    }
    
//    OLED_ShowString(0, 24,(uint8_t*)"Publish timeout", 8, 1);
//    OLED_Refresh();
//    HAL_Delay(500);
//    OLED_Clear();
    return false;
}
#if 0
/**
 * 检查接收缓冲区，寻找订阅主题的下发消息并解析 JSON 控制 LED
 * 说明：主循环中定期调用该函数（避免在中断中做复杂解析）
 */
void ESP8266_ProcessMessages(void)
{
    char local_buf[ESP8266_BUF_SIZE];
    uint16_t len = 0;

    len = (esp8266_cnt < ESP8266_BUF_SIZE - 1) ? esp8266_cnt : (ESP8266_BUF_SIZE - 1);
    if (len == 0) return;  
    memcpy(local_buf, (const char*)esp8266_buf, len);
    // 清空计数以表示已消费（由主线程负责）
    esp8266_cnt = 0;
    ((char*)esp8266_buf)[0] = '\0';
    local_buf[len] = '\0';

    // 调试打印收到的数据
    HAL_UART_Transmit(&huart2, (uint8_t*)"--PROCESS RX: ", 14, 100);
    HAL_UART_Transmit(&huart2, (uint8_t*)local_buf, strlen(local_buf), 500);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);

    // 检查是否是MQTT订阅消息，如果不是则直接返回
    if (strstr(local_buf, "+MQTTSUBRECV") == NULL)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Not a SUBRECV message\r\n", 32, 100);
        return;
    }

    // 找到第一个 JSON 并提取完整 JSON（简单配对）
    char *p_json = strchr(local_buf, '{');
    if (!p_json) 
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: No JSON found\r\n", 24, 100);
        return;
    }

    char json_buf[ESP8266_BUF_SIZE];
    char *p = p_json;
    int depth = 0;
    int idx = 0;
    while (*p && idx < (int)sizeof(json_buf) - 1)
    {
        if (*p == '{') depth++;
        if (*p == '}') depth--;
        json_buf[idx++] = *p++;
        if (depth == 0) break;
    }
    json_buf[idx] = '\0';
    if (idx == 0)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Empty JSON\r\n", 21, 100);
        return;
    }

    // 调试打印提取的 JSON
    HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Extracted JSON: ", 22, 100);
    HAL_UART_Transmit(&huart2, (uint8_t*)json_buf, strlen(json_buf), 500);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);

    // 简化测试：直接处理 LED 控制
    HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Using simplified LED control\r\n", 38, 100);
    
    // 检查 JSON 中是否包含 "LED":true
    if (strstr(json_buf, "\"LED\":true") != NULL)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"LED1 ON\r\n", 10, 100);
			  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    }
    else if (strstr(json_buf, "\"LED\":false") != NULL)
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"LED1 OFF\r\n", 9, 100);
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: LED value not found\r\n", 30, 100);
    }

    // 构造并发送回执 payload
    HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Creating reply payload\r\n", 32, 100);
    char reply_payload[128];
    
    // 简单提取 id
    char *id_start = strstr(json_buf, "\"id\":\"");
    if (id_start)
    {
        id_start += 6; // 跳过 "id":" 
        char *id_end = strchr(id_start, '"');
        if (id_end)
        {
            *id_end = '\0';
            snprintf(reply_payload, sizeof(reply_payload),
                     "{\"id\":\"%s\",\"code\":200,\"data\":null,\"msg\":\"success\"}",
                     id_start);
            *id_end = '"'; // 恢复原始字符
        }
        else
        {
            snprintf(reply_payload, sizeof(reply_payload),
                     "{\"id\":\"%lu\",\"code\":200,\"data\":null,\"msg\":\"success\"}",
                     (unsigned long)HAL_GetTick());
        }
    }
    else
    {
        snprintf(reply_payload, sizeof(reply_payload),
                 "{\"id\":\"%lu\",\"code\":200,\"data\":null,\"msg\":\"success\"}",
                 (unsigned long)HAL_GetTick());
    }

    // 发布回执
    HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Publishing reply\r\n", 27, 100);
    ESP8266_MQTT_Publish(MQTT_TOPIC_SET_REPLY, reply_payload, 0, 0);
    
    HAL_UART_Transmit(&huart2, (uint8_t*)"DEBUG: Processing complete\r\n", 28, 100);


}
#endif

/**
*	函数名称：	HAL_UART_RxCpltCallback
*	函数功能：	串口接收中断回调函数（覆盖弱定义）
*	入口参数：	huart：串口句柄
*/
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart == &huart1)
//    {
//        // DMA 传输完整（缓冲已满），标记为满并重启接收
//        esp8266_cnt = (ESP8266_BUF_SIZE - 1);
//        esp8266_buf[esp8266_cnt] = '\0';
//        HAL_UART_Receive_DMA(&huart1, (uint8_t*)esp8266_buf, ESP8266_BUF_SIZE - 1);
//    }
//}