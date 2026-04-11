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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "dht11.h"
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

/* Private variables ---------------------------------------------------------

/* USER CODE BEGIN PV */
uint8_t current_screen = 1; // 界面切换变量，1表示界面1，2表示界面2，3表示界面3

// 数据采集和存储
#define MAX_DATA_POINTS 128 // 最大数据点数量
uint8_t humidity_data[MAX_DATA_POINTS]; // 湿度数据
uint8_t temperature_data[MAX_DATA_POINTS]; // 温度数据
uint16_t data_index = 0; // 当前数据点索引
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  OLED_Refresh();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    DHT11_Data data1, data2, data3, data4;
    
    // 读取四个DHT11的温湿度数据
    DHT11_ReadData(DHT11_1_PORT, DHT11_1_PIN, &data1);
    DHT11_ReadData(DHT11_2_PORT, DHT11_2_PIN, &data2);
    DHT11_ReadData(DHT11_3_PORT, DHT11_3_PIN, &data3);
    DHT11_ReadData(DHT11_4_PORT, DHT11_4_PIN, &data4);
    
    // 计算平均值
    uint8_t avg_humidity = (data1.humidity + data2.humidity + data3.humidity + data4.humidity) / 4;
    uint8_t avg_temperature = (data1.temperature + data2.temperature + data3.temperature + data4.temperature) / 4;
    
    // 存储数据
    humidity_data[data_index] = avg_humidity;
    temperature_data[data_index] = avg_temperature;
    data_index = (data_index + 1) % MAX_DATA_POINTS;
    
    // 显示数据到OLED
    OLED_Clear();
    
    if (current_screen == 1) {
      // 界面1：显示四个DHT11的原始数据
      // 显示DHT11_1的数据
      OLED_ShowString(0, 0, (uint8_t*)"DHT11_1:", 8, 1);
      OLED_ShowString(56, 0, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 0, data1.humidity, 2, 8, 1);
      OLED_ShowString(80, 0, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 0, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 0, data1.temperature, 2, 8, 1);
      OLED_ShowString(112, 0, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_2的数据
      OLED_ShowString(0, 16, (uint8_t*)"DHT11_2:", 8, 1);
      OLED_ShowString(56, 16, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 16, data2.humidity, 2, 8, 1);
      OLED_ShowString(80, 16, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 16, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 16, data2.temperature, 2, 8, 1);
      OLED_ShowString(112, 16, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_3的数据
      OLED_ShowString(0, 32, (uint8_t*)"DHT11_3:", 8, 1);
      OLED_ShowString(56, 32, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 32, data3.humidity, 2, 8, 1);
      OLED_ShowString(80, 32, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 32, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 32, data3.temperature, 2, 8, 1);
      OLED_ShowString(112, 32, (uint8_t*)"C", 8, 1);
      
      // 显示DHT11_4的数据
      OLED_ShowString(0, 48, (uint8_t*)"DHT11_4:", 8, 1);
      OLED_ShowString(56, 48, (uint8_t*)"H:", 8, 1);
      OLED_ShowNum(64, 48, data4.humidity, 2, 8, 1);
      OLED_ShowString(80, 48, (uint8_t*)"%", 8, 1);
      OLED_ShowString(88, 48, (uint8_t*)"T:", 8, 1);
      OLED_ShowNum(96, 48, data4.temperature, 2, 8, 1);
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
      // 界面3：显示平均值的走势图
      OLED_ShowString(0, 0, (uint8_t*)"Trend Chart:", 8, 1);
      
      // 绘制湿度走势图
      OLED_ShowString(0, 16, (uint8_t*)"Humidity:", 8, 1);
      for (uint16_t i = 0; i < MAX_DATA_POINTS; i++) {
        uint16_t x = i * 128 / MAX_DATA_POINTS;
        uint16_t y = 32 - (humidity_data[(data_index + i) % MAX_DATA_POINTS] * 16 / 100);
        if (i == 0) {
          OLED_DrawPoint(x, y, 1);
        } else {
          uint16_t x_prev = (i - 1) * 128 / MAX_DATA_POINTS;
          uint16_t y_prev = 32 - (humidity_data[(data_index + i - 1) % MAX_DATA_POINTS] * 16 / 100);
          OLED_DrawLine(x_prev, y_prev, x, y, 1);
        }
      }
      
      // 绘制温度走势图
      OLED_ShowString(0, 40, (uint8_t*)"Temperature:", 8, 1);
      for (uint16_t i = 0; i < MAX_DATA_POINTS; i++) {
        uint16_t x = i * 128 / MAX_DATA_POINTS;
        uint16_t y = 56 - (temperature_data[(data_index + i) % MAX_DATA_POINTS] * 16 / 50);
        if (i == 0) {
          OLED_DrawPoint(x, y, 1);
        } else {
          uint16_t x_prev = (i - 1) * 128 / MAX_DATA_POINTS;
          uint16_t y_prev = 56 - (temperature_data[(data_index + i - 1) % MAX_DATA_POINTS] * 16 / 50);
          OLED_DrawLine(x_prev, y_prev, x, y, 1);
        }
      }
    }
    
    OLED_Refresh();
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
