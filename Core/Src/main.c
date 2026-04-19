/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "dht11.h"
#include "esp8266.h"
#include "mqtt_publisher.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t current_screen = 1; // 界面切换变量，1表示界面1（主界面），2表示界面2（平均值界面），3表示设置界面

// 数据采集和存储
#define MAX_DATA_POINTS 128 // 最大数据点数量
uint8_t humidity_data[MAX_DATA_POINTS]; // 湿度数据
uint8_t temperature_data[MAX_DATA_POINTS]; // 温度数据
uint16_t data_index = 0; // 当前数据点索引

// 数据滤波相关变量
#define FILTER_WINDOW_SIZE 5 // 滤波窗口大小
uint8_t humidity_filter_buf[FILTER_WINDOW_SIZE][4]; // 4个传感器的湿度滤波缓冲区
uint8_t temperature_filter_buf[FILTER_WINDOW_SIZE][4]; // 4个传感器的温度滤波缓冲区
uint8_t filter_index = 0; // 滤波缓冲区索引
uint8_t filter_ready = 0; // 滤波是否就绪标志

// 阈值设置相关变量
uint8_t temp_threshold = 25; // 温度阈值（默认25°C，范围0-40）
uint8_t humi_threshold = 60; // 湿度阈值（默认60%，范围0-99）
uint8_t setting_mode = 0; // 设置模式：0-温度阈值，1-湿度阈值，2-保存返回

// Flash存储相关定义
#define FLASH_THRESHOLD_ADDR 0x0800FC00 // Flash存储地址（最后一页）
#define FLASH_MAGIC_NUMBER 0xAA55 // 魔数用于验证数据有效性

// 阈值数据结构
typedef struct {
    uint16_t magic;        // 魔数
    uint8_t temp_threshold; // 温度阈值
    uint8_t humi_threshold; // 湿度阈值
    uint8_t reserved[2];   // 保留字节
} ThresholdData_t;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// 数据滤波函数
uint8_t moving_average_filter(uint8_t* buffer, uint8_t sensor_index, uint8_t new_value);

// Flash存储函数
void save_thresholds_to_flash(void);
void load_thresholds_from_flash(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t wifi_try = 0, mqtt_try = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  
  // 从Flash加载阈值设置
  load_thresholds_from_flash();
  
  // 显示欢迎界面2秒
  OLED_ShowString(20, 0, (uint8_t*)"Welcome", 16, 1);
  OLED_ShowString(10, 20, (uint8_t*)"DHT11 System", 16, 1);
  OLED_Refresh();
  
  // 延时2秒
  HAL_Delay(2000);
  
  // 清屏并进入主循环
  OLED_Clear();
  OLED_Refresh();
  	ESP8266_Init();
		  while (wifi_try < 5 && !ESP8266_ConnectWiFi())
  {
      wifi_try++;
      HAL_Delay(1000);
  }
	
	  //上云
	if(ESP8266_ConnectCloud()==false)
	{
		  while(1);
	}
	HAL_Delay(5000);
	ESP8266_Clear();
	OLED_Clear();
	
	//订阅
	if(!ESP8266_MQTT_Subscribe(MQTT_TOPIC_POST_REPLY,1))
	{
		  while(1);
	}
	
	//发布
		if(!ESP8266_MQTT_Subscribe(MQTT_TOPIC_SET,0))
	{
		  while(1);
	}
	
	// 初始化MQTT发布状态机
	MQTT_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  // 主循环状态机变量
  static uint8_t loop_state = 0; // 0: DHT11读取, 1: 数据处理, 2: MQTT处理
  static uint32_t last_dht11_read = 0;
  static DHT11_Data data1, data2, data3, data4;
  static DHT11_Data filtered_data1, filtered_data2, filtered_data3, filtered_data4;
  static uint8_t avg_humidity = 0, avg_temperature = 0;
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    // 状态机：分离DHT11读取和MQTT处理
    switch (loop_state) {
        case 0: // DHT11读取状态
            {
                uint32_t now = HAL_GetTick();
                // 每2秒读取一次DHT11数据
                if ((uint32_t)(now - last_dht11_read) >= 2000) {
                    // 读取四个DHT11的温湿度数据（禁用中断保护时序）
                    DHT11_ReadData(DHT11_1_PORT, DHT11_1_PIN, &data1);
                    DHT11_ReadData(DHT11_2_PORT, DHT11_2_PIN, &data2);
                    DHT11_ReadData(DHT11_3_PORT, DHT11_3_PIN, &data3);
                    DHT11_ReadData(DHT11_4_PORT, DHT11_4_PIN, &data4);
                    
                    last_dht11_read = now;
                    loop_state = 1; // 切换到数据处理状态
                }
                break;
            }
        case 1: // 数据处理状态
            {
                // 应用数据滤波
                filtered_data1.humidity = moving_average_filter((uint8_t*)humidity_filter_buf, 0, data1.humidity);
                filtered_data1.temperature = moving_average_filter((uint8_t*)temperature_filter_buf, 0, data1.temperature);
                
                filtered_data2.humidity = moving_average_filter((uint8_t*)humidity_filter_buf, 1, data2.humidity);
                filtered_data2.temperature = moving_average_filter((uint8_t*)temperature_filter_buf, 1, data2.temperature);
                
                filtered_data3.humidity = moving_average_filter((uint8_t*)humidity_filter_buf, 2, data3.humidity);
                filtered_data3.temperature = moving_average_filter((uint8_t*)temperature_filter_buf, 2, data3.temperature);
                
                filtered_data4.humidity = moving_average_filter((uint8_t*)humidity_filter_buf, 3, data4.humidity);
                filtered_data4.temperature = moving_average_filter((uint8_t*)temperature_filter_buf, 3, data4.temperature);
                
                // 更新滤波索引
                filter_index = (filter_index + 1) % FILTER_WINDOW_SIZE;
                if (filter_index == 0) {
                    filter_ready = 1; // 滤波缓冲区已填满
                }
                
                // 计算平均值（使用滤波后的数据）
                avg_humidity = (filtered_data1.humidity + filtered_data2.humidity + filtered_data3.humidity + filtered_data4.humidity) / 4;
                avg_temperature = (filtered_data1.temperature + filtered_data2.temperature + filtered_data3.temperature + filtered_data4.temperature) / 4;
                
                // 根据阈值控制LED
                // DHT11_1温度或湿度高于阈值，LED1拉低（点亮），否则拉高（熄灭）
                if (filtered_data1.temperature > temp_threshold || filtered_data1.humidity > humi_threshold) {
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); // 拉低，点亮LED
                } else {
                    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   // 拉高，熄灭LED
                }
                
                // DHT11_2温度或湿度高于阈值，LED2拉低（点亮），否则拉高（熄灭）
                if (filtered_data2.temperature > temp_threshold || filtered_data2.humidity > humi_threshold) {
                    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); // 拉低，点亮LED
                } else {
                    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);   // 拉高，熄灭LED
                }
                
                // DHT11_3温度或湿度高于阈值，LED3拉低（点亮），否则拉高（熄灭）
                if (filtered_data3.temperature > temp_threshold || filtered_data3.humidity > humi_threshold) {
                    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); // 拉低，点亮LED
                } else {
                    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);   // 拉高，熄灭LED
                }
                
                // DHT11_4温度或湿度高于阈值，LED4拉低（点亮），否则拉高（熄灭）
                if (filtered_data4.temperature > temp_threshold || filtered_data4.humidity > humi_threshold) {
                    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET); // 拉低，点亮LED
                } else {
                    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET);   // 拉高，熄灭LED
                }
                
                // 存储数据
                humidity_data[data_index] = avg_humidity;
                temperature_data[data_index] = avg_temperature;
                data_index = (data_index + 1) % MAX_DATA_POINTS;
                
                // 在平均值界面控制蜂鸣器
                if (current_screen == 2) {
                    // 平均值界面：温度或湿度平均值超过阈值，拉低BEEP工作（蜂鸣），否则拉高BEEP（静音）
                    if (avg_temperature > temp_threshold || avg_humidity > humi_threshold) {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET); // 拉低，蜂鸣器工作
        } else {
            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);   // 拉高，蜂鸣器静音
        }
    } else {
        // 其他界面：蜂鸣器静音
        HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    }
    
    // 通过MQTT发布DHT11传感器数据（发布温度平均值）
    char avg_temp_str[8], avg_humi_str[4];
    
    // 计算平均值（使用滤波后的数据）
    avg_humidity = (filtered_data1.humidity + filtered_data2.humidity + filtered_data3.humidity + filtered_data4.humidity) / 4;
    avg_temperature = (filtered_data1.temperature + filtered_data2.temperature + filtered_data3.temperature + filtered_data4.temperature) / 4;
    
    // 将平均值转换为字符串（使用%d.%d格式）
    snprintf(avg_temp_str, sizeof(avg_temp_str), "%d.%d", avg_temperature, 0);
    snprintf(avg_humi_str, sizeof(avg_humi_str), "%d.%d", avg_humidity,0);
    
    // 准备MQTT数据（非阻塞方式）
    MQTT_Prepare_Data("temperature1", avg_temp_str);
    MQTT_Prepare_Data("humidity1", avg_humi_str);
    
    // 显示数据到OLED
    OLED_Clear();
    
    if (current_screen == 1) {
      // 界面1：显示四个DHT11的原始数据
      // 显示DHT11_1的数据（使用滤波后的数据）
      OLED_ShowString(0, 0, (uint8_t*)"DHT11_1:", 8, 1);
      OLED_ShowString(56, 0, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 0, filtered_data1.humidity, 2, 8, 1);
      OLED_ShowString(80, 0, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 0, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 0, filtered_data1.temperature, 2, 8, 1);
      OLED_ShowString(112, 0, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_2的数据（使用滤波后的数据）
      OLED_ShowString(0, 16, (uint8_t*)"DHT11_2:", 8, 1);
      OLED_ShowString(56, 16, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 16, filtered_data2.humidity, 2, 8, 1);
      OLED_ShowString(80, 16, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 16, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 16, filtered_data2.temperature, 2, 8, 1);
      OLED_ShowString(112, 16, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_3的数据（使用滤波后的数据）
      OLED_ShowString(0, 32, (uint8_t*)"DHT11_3:", 8, 1);
      OLED_ShowString(56, 32, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 32, filtered_data3.humidity, 2, 8, 1);
      OLED_ShowString(80, 32, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 32, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 32, filtered_data3.temperature, 2, 8, 1);
      OLED_ShowString(112, 32, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_4的数据（使用滤波后的数据）
      OLED_ShowString(0, 48, (uint8_t*)"DHT11_4:", 8, 1);
      OLED_ShowString(56, 48, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 48, filtered_data4.humidity, 2, 8, 1);
      OLED_ShowString(80, 48, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 48, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 48, filtered_data4.temperature, 2, 8, 1);
      OLED_ShowString(112, 48, (uint8_t*)"C", 8, 1);
    } else if (current_screen == 2) {
      // 界面2：显示DHT11的温湿度平均值
      OLED_ShowString(0, 0, (uint8_t*)"Average:", 8, 1);
      OLED_ShowString(0, 24, (uint8_t*)"Humidity:", 8, 1);
      OLED_ShowNum(64, 24, avg_humidity, 2, 8, 1);
      OLED_ShowString(80, 24, (uint8_t*)"%", 8, 1);
      OLED_ShowString(0, 48, (uint8_t*)"Temperature:", 8, 1);
      OLED_ShowNum(80, 48, avg_temperature, 2, 8, 1);
      OLED_ShowString(96, 48, (uint8_t*)"C", 8, 1);
    } else if (current_screen == 3) {
      // 界面3：设置界面（温湿度阈值在同一页面）
      OLED_ShowString(0, 0, (uint8_t*)"Settings:", 8, 1);
      
      if (setting_mode == 0) {
        // 温度阈值设置模式
        OLED_ShowString(0, 16, (uint8_t*)"Temp:", 8, 1);
        OLED_ShowNum(40, 16, temp_threshold, 2, 8, 1);
        OLED_ShowString(56, 16, (uint8_t*)"C", 8, 1);
        OLED_ShowString(80, 16, (uint8_t*)"<-", 8, 1);
        
        OLED_ShowString(0, 32, (uint8_t*)"Humi:", 8, 1);
        OLED_ShowNum(40, 32, humi_threshold, 2, 8, 1);
        OLED_ShowString(56, 32, (uint8_t*)"%", 8, 1);
        
        OLED_ShowString(0, 48, (uint8_t*)"KEY1:- KEY3:+ KEY2:OK", 8, 1);
      } else if (setting_mode == 1) {
        // 湿度阈值设置模式
        OLED_ShowString(0, 16, (uint8_t*)"Temp:", 8, 1);
        OLED_ShowNum(40, 16, temp_threshold, 2, 8, 1);
        OLED_ShowString(56, 16, (uint8_t*)"C", 8, 1);
        
        OLED_ShowString(0, 32, (uint8_t*)"Humi:", 8, 1);
        OLED_ShowNum(40, 32, humi_threshold, 2, 8, 1);
        OLED_ShowString(56, 32, (uint8_t*)"%", 8, 1);
        OLED_ShowString(80, 32, (uint8_t*)"<-", 8, 1);
        
        OLED_ShowString(0, 48, (uint8_t*)"KEY1:- KEY3:+ KEY2:OK", 8, 1);
      } else {
        // 保存并返回模式
        OLED_ShowString(0, 0, (uint8_t*)"Settings:", 8, 1);
        OLED_ShowString(0, 16, (uint8_t*)"Temp:", 8, 1);
        OLED_ShowNum(40, 16, temp_threshold, 2, 8, 1);
        OLED_ShowString(56, 16, (uint8_t*)"C", 8, 1);
        
        OLED_ShowString(0, 32, (uint8_t*)"Humi:", 8, 1);
        OLED_ShowNum(40, 32, humi_threshold, 2, 8, 1);
        OLED_ShowString(56, 32, (uint8_t*)"%", 8, 1);
        
        OLED_ShowString(0, 48, (uint8_t*)"Press KEY2 to Save", 8, 1);
      }
      
      // 显示当前设置模式指示器
      OLED_ShowString(104, 0, (uint8_t*)"M:", 8, 1);
      OLED_ShowNum(120, 0, setting_mode, 1, 8, 1);
    }
    
    OLED_Refresh();
    
    // 处理MQTT发布状态机（非阻塞）
    MQTT_Process();
    
    HAL_Delay(1000); // 每2秒读取一次数据
  /* USER CODE END 3 */
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// 移动平均滤波函数
uint8_t moving_average_filter(uint8_t* buffer, uint8_t sensor_index, uint8_t new_value)
{
    uint16_t sum = 0;
    
    // 更新滤波缓冲区（正确使用二维数组索引）
    uint8_t (*filter_buf)[4] = (uint8_t(*)[4])buffer;
    filter_buf[filter_index][sensor_index] = new_value;
    
    // 计算平均值
    for (uint8_t i = 0; i < FILTER_WINDOW_SIZE; i++) {
        sum += filter_buf[i][sensor_index];
    }
    
    return (uint8_t)(sum / FILTER_WINDOW_SIZE);
}

// 保存阈值到Flash
void save_thresholds_to_flash(void)
{
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;
    
    // 解锁Flash
    HAL_FLASH_Unlock();
    
    // 配置擦除参数
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = FLASH_THRESHOLD_ADDR;
    erase_init.NbPages = 1;
    
    // 擦除Flash页
    if (HAL_FLASHEx_Erase(&erase_init, &page_error) == HAL_OK) {
        // 准备阈值数据
        ThresholdData_t threshold_data;
        threshold_data.magic = FLASH_MAGIC_NUMBER;
        threshold_data.temp_threshold = temp_threshold;
        threshold_data.humi_threshold = humi_threshold;
        threshold_data.reserved[0] = 0;
        threshold_data.reserved[1] = 0;
        
        // 写入Flash
        uint32_t *data_ptr = (uint32_t*)&threshold_data;
        for (uint8_t i = 0; i < sizeof(ThresholdData_t) / 4; i++) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_THRESHOLD_ADDR + i * 4, data_ptr[i]);
        }
    }
    
    // 锁定Flash
    HAL_FLASH_Lock();
}

// 从Flash读取阈值
void load_thresholds_from_flash(void)
{
    ThresholdData_t *threshold_data = (ThresholdData_t*)FLASH_THRESHOLD_ADDR;
    
    // 检查魔数是否有效
    if (threshold_data->magic == FLASH_MAGIC_NUMBER) {
        temp_threshold = threshold_data->temp_threshold;
        humi_threshold = threshold_data->humi_threshold;
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
