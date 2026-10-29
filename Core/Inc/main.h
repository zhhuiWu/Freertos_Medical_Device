/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
  typedef struct {
    uint16_t red;
    uint16_t ir;
  } SPO2_Data_t;

  typedef struct {
    uint16_t cuff;
    uint16_t pulse;
  } NIBP_Data_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UART_TX_Pin GPIO_PIN_13
#define UART_TX_GPIO_Port GPIOC
#define OVER_PRESS_SEL_Pin GPIO_PIN_0
#define OVER_PRESS_SEL_GPIO_Port GPIOC
#define M_PROTECT_Pin GPIO_PIN_2
#define M_PROTECT_GPIO_Port GPIOC
#define OLED_DC_Pin GPIO_PIN_3
#define OLED_DC_GPIO_Port GPIOC
#define SLOW_Pin GPIO_PIN_0
#define SLOW_GPIO_Port GPIOA
#define FAST_Pin GPIO_PIN_2
#define FAST_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_3
#define UART_RX_GPIO_Port GPIOA
#define SPO_ADC_Pin GPIO_PIN_5
#define SPO_ADC_GPIO_Port GPIOA
#define ECG_LEADOFF_Pin GPIO_PIN_7
#define ECG_LEADOFF_GPIO_Port GPIOA
#define ECG_ADC_Pin GPIO_PIN_4
#define ECG_ADC_GPIO_Port GPIOC
#define CUFF_AD_Pin GPIO_PIN_0
#define CUFF_AD_GPIO_Port GPIOB
#define PULSE_ADC_Pin GPIO_PIN_1
#define PULSE_ADC_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_12
#define OLED_CS_GPIO_Port GPIOB
#define OLED_SCK_Pin GPIO_PIN_13
#define OLED_SCK_GPIO_Port GPIOB
#define OLED_RES_Pin GPIO_PIN_14
#define OLED_RES_GPIO_Port GPIOB
#define OLED_DIN_Pin GPIO_PIN_15
#define OLED_DIN_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOC
#define PULSE_EN_Pin GPIO_PIN_10
#define PULSE_EN_GPIO_Port GPIOC
#define KEY1_Pin GPIO_PIN_12
#define KEY1_GPIO_Port GPIOC
#define SPO_IR_CS_Pin GPIO_PIN_6
#define SPO_IR_CS_GPIO_Port GPIOB
#define SPO_RED_CS_Pin GPIO_PIN_7
#define SPO_RED_CS_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_8
#define KEY2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern osSemaphoreId_t xSemaphore500usHandle;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
