/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */
extern uint8_t current_screen; // 外部声明界面切换变量
extern uint8_t temp_threshold; // 外部声明温度阈值
extern uint8_t humi_threshold; // 外部声明湿度阈值
extern uint8_t setting_mode;   // 外部声明设置模式
extern void save_thresholds_to_flash(void); // 外部声明Flash保存函数
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

  /* USER CODE END DMA1_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

  /* USER CODE END DMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
  {
      __HAL_UART_CLEAR_IDLEFLAG(&huart1);

      // ? DMA ???????????
      HAL_UART_DMAStop(&huart1);

      // ?????????????????????? ESP8266_BUF_SIZE-1???????? Receive_DMA ??????????
      uint16_t received = (uint16_t)((ESP8266_BUF_SIZE - 1) - __HAL_DMA_GET_COUNTER(huart1.hdmarx));
      if (received > (ESP8266_BUF_SIZE - 1)) received = (ESP8266_BUF_SIZE - 1);

      esp8266_cnt = received;
      esp8266_buf[received] = '\0';

      // ???????? DMA ???????
      HAL_UART_Receive_DMA(&huart1, (uint8_t*)esp8266_buf, ESP8266_BUF_SIZE - 1);
  }
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
  HAL_GPIO_EXTI_IRQHandler(KEY2_Pin);
  HAL_GPIO_EXTI_IRQHandler(KEY3_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

		
	    if (huart == &huart1)
    {
        // 计算实际接收到的字节数
        esp8266_cnt = ESP8266_BUF_SIZE - 1 - huart->RxXferCount;
        if (esp8266_cnt > 0) {
            esp8266_buf[esp8266_cnt] = '\0';
        }
        // 重新启动 DMA 接收
        HAL_UART_Receive_DMA(&huart1, (uint8_t*)esp8266_buf, ESP8266_BUF_SIZE - 1);
    }
}

/**
  * @brief  EXTI line detection callbacks
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == KEY1_Pin) {
    if (current_screen == 3) {
      // 设置界面中：KEY1用于减少阈值
      if (setting_mode == 0) {
        // 温度阈值减少（范围0-40）
        if (temp_threshold > 0) temp_threshold--;
      } else if (setting_mode == 1) {
        // 湿度阈值减少（范围0-99）
        if (humi_threshold > 0) humi_threshold--;
      }
    } else {
      // 其他界面：KEY1在平均值界面（界面2）和主界面（界面1）之间切换
      if (current_screen == 1) {
        current_screen = 2;
      } else if (current_screen == 2) {
        current_screen = 1;
      }
    }
  } else if (GPIO_Pin == KEY2_Pin) {
    if (current_screen == 3) {
      // 设置界面中：KEY2用于切换设置模式
      setting_mode = (setting_mode + 1) % 3;
      
      // 如果切换到保存返回模式，保存阈值到Flash并返回主界面
      if (setting_mode == 2) {
        save_thresholds_to_flash();
        current_screen = 1;
        setting_mode = 0;
      }
    } else {
      // 其他界面：KEY2进入设置界面（界面3）
      current_screen = 3;
      setting_mode = 0; // 重置为温度阈值设置模式
    }
  } else if (GPIO_Pin == KEY3_Pin) {
    if (current_screen == 3) {
      // 设置界面中：KEY3用于增加阈值
      if (setting_mode == 0) {
        // 温度阈值增加（范围0-40）
        if (temp_threshold < 40) temp_threshold++;
      } else if (setting_mode == 1) {
        // 湿度阈值增加（范围0-99）
        if (humi_threshold < 99) humi_threshold++;
      }
    }
    // 其他界面中KEY3保持原有功能
  }
}

/* USER CODE END 1 */
