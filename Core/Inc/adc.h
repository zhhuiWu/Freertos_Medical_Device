/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
  // 滤波器结构体定义
  typedef struct {
    float alpha;        // 滤波器系数
    float prev_output;  // 上一次输出值(用于IIR滤波器)
    float prev_input;   // 上一次输入值(用于高通滤波器)
  } HighPassFilter;

  typedef struct {
    float alpha;        // 滤波器系数
    float prev_output;  // 上一次输出值
  } LowPassFilter;

  typedef struct {
    uint16_t buffer[6];     // 滑动窗口缓冲区
    uint8_t index;           // 当前写入位置
    uint32_t sum;            // 当前窗口内数据之和
  } MovingAverageFilter;

  // 血氧信号滤波器组
  typedef struct {
    MovingAverageFilter ma_red;    // 红光移动平均滤波器
    MovingAverageFilter ma_ir;     // 红外移动平均滤波器
    HighPassFilter hp_red;         // 红光高通滤波器
    HighPassFilter hp_ir;          // 红外高通滤波器
    LowPassFilter lp_red;          // 红光低通滤波器
    LowPassFilter lp_ir;           // 红外低通滤波器
  } SPO2Filter;

  // 函数声明
  void InitFilters(SPO2Filter* filter);
  uint16_t MovingAverageFilterProcess(MovingAverageFilter* filter, uint16_t input);
  float HighPassFilterProcess(HighPassFilter* filter, float input);
  float LowPassFilterProcess(LowPassFilter* filter, float input);
  uint16_t FilterSPO2Red(SPO2Filter* filter, uint16_t input);
  uint16_t FilterSPO2IR(SPO2Filter* filter, uint16_t input);
/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
  typedef enum {
    ECG_CHANNEL = 0,
    SPO_CHANNEL = 1,
    CUFF_CHANNEL = 2,  // 新增PB0通道
     PULSE_CHANNEL = 3  // 新增PB1通道
} ADC_Channel;
/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
uint16_t ReadADCChannel(uint8_t channel);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

