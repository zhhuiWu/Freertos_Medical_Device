/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
uint16_t adc_buff[4];
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buff,sizeof(adc_buff)/sizeof(uint16_t));
  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA5     ------> ADC1_IN5
    PC4     ------> ADC1_IN14
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    */
    GPIO_InitStruct.Pin = SPO_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(SPO_ADC_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ECG_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ECG_ADC_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CUFF_AD_Pin|PULSE_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA5     ------> ADC1_IN5
    PC4     ------> ADC1_IN14
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(SPO_ADC_GPIO_Port, SPO_ADC_Pin);

    HAL_GPIO_DeInit(ECG_ADC_GPIO_Port, ECG_ADC_Pin);

    HAL_GPIO_DeInit(GPIOB, CUFF_AD_Pin|PULSE_ADC_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uint16_t ReadADCChannel(uint8_t channel)
{
  // 直接从DMA缓冲区获取最新值
  if(channel == SPO_CHANNEL) {
    return adc_buff[0]; // PA5 - 血氧信号
  } else if(channel == ECG_CHANNEL) {
    return adc_buff[1]; // PC4 - 心电信号
  } else if(channel == CUFF_CHANNEL) {
    return adc_buff[2]; // PB0 - 新增通道
  } else if(channel == PULSE_CHANNEL) {
    return adc_buff[3]; // PB1 - 新增通道
  }
  return 0;
}

// 滤波器初始化
void InitFilters(SPO2Filter* filter) {
    // 初始化移动平均滤波器 - 窗口大小设为6
    float dt = 0.002f; // 采样间隔2ms

    // 调整高通滤波器参数 - 保持适中
    float rc_hp = 1.0f / (2.0f * 3.1415926f * 1.0f); // 截止频率设为1Hz
    float alpha_hp = rc_hp / (rc_hp + dt);

    // 调整低通滤波器参数 - 略微加强（降低截止频率）
    float rc_lp = 1.0f / (2.0f * 3.1415926f * 12.0f); // 截止频率从15Hz降低到12Hz
    float alpha_lp = dt / (rc_lp + dt);

    int i;

    // 设置移动平均窗口大小为6
    for(i = 0; i < 6; i++) {
        filter->ma_red.buffer[i] = 0;
        filter->ma_ir.buffer[i] = 0;
    }
    filter->ma_red.index = 0;
    filter->ma_ir.index = 0;
    filter->ma_red.sum = 0;
    filter->ma_ir.sum = 0;

    // 初始化高通滤波器
    filter->hp_red.alpha = alpha_hp;
    filter->hp_ir.alpha = alpha_hp;
    filter->hp_red.prev_output = 0;
    filter->hp_ir.prev_output = 0;
    filter->hp_red.prev_input = 0;
    filter->hp_ir.prev_input = 0;

    // 初始化低通滤波器
    filter->lp_red.alpha = alpha_lp;
    filter->lp_ir.alpha = alpha_lp;
    filter->lp_red.prev_output = 0;
    filter->lp_ir.prev_output = 0;
}

// 移动平均滤波器处理
uint16_t MovingAverageFilterProcess(MovingAverageFilter* filter, uint16_t input) {
    // 减去最早的值
    filter->sum -= filter->buffer[filter->index];

    // 加上新的值
    filter->sum += input;
    filter->buffer[filter->index] = input;

    // 更新索引
    filter->index = (filter->index + 1) % 6; // 窗口大小改为6

    // 返回平均值
    return (uint16_t)(filter->sum / 6); // 窗口大小改为6
}

// 高通滤波器处理
float HighPassFilterProcess(HighPassFilter* filter, float input) {
    float output = filter->alpha * (filter->prev_output + input - filter->prev_input);
    filter->prev_output = output;
    filter->prev_input = input;
    return output;
}

// 低通滤波器处理
float LowPassFilterProcess(LowPassFilter* filter, float input) {
    float output = filter->alpha * input + (1 - filter->alpha) * filter->prev_output;
    filter->prev_output = output;
    return output;
}

// 红光信号完整滤波处理
uint16_t FilterSPO2Red(SPO2Filter* filter, uint16_t input) {
    // 1. 移动平均滤波
    uint16_t ma_output = MovingAverageFilterProcess(&filter->ma_red, input);

    // 2. 高通滤波
    float hp_output = HighPassFilterProcess(&filter->hp_red, (float)ma_output);

    // 3. 低通滤波
    float lp_output = LowPassFilterProcess(&filter->lp_red, hp_output)+500;

    return (uint16_t)lp_output;
}

// 红外信号完整滤波处理
uint16_t FilterSPO2IR(SPO2Filter* filter, uint16_t input) {
    // 1. 移动平均滤波
    uint16_t ma_output = MovingAverageFilterProcess(&filter->ma_ir, input);

    // 2. 高通滤波
    float hp_output = HighPassFilterProcess(&filter->hp_ir, (float)ma_output);

    // 3. 低通滤波
    float lp_output = LowPassFilterProcess(&filter->lp_ir, hp_output)+500;

    return (uint16_t)lp_output;
}
/* USER CODE END 1 */
