/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>

#include "dac.h"
#include "usart.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
#define SPO2_BUF_SIZE 1024
#define RR_TABLE_SIZE 11
static const uint16_t RR_TABLE[RR_TABLE_SIZE] = {560, 600, 630, 650, 680, 710, 740, 760, 790, 820, 860};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 血氧计算相关结构体
typedef struct {
  uint16_t redBuffer[SPO2_BUF_SIZE];  // 红光数据缓冲区
  uint16_t irBuffer[SPO2_BUF_SIZE];   // 红外光数据缓冲区
  uint16_t index;                     // 缓冲区索引
  uint8_t calcFlag;                   // 计算标志位
  uint16_t arrRRBuff[6];              // R值缓冲区
  uint8_t spo2Value;                  // 计算得到的血氧值
} SPO2Calculator;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void StoreSPO2Data(SPO2Calculator* calculator, uint16_t redValue, uint16_t irValue);
uint8_t CalcSpO2(SPO2Calculator* calculator);
void AdjustLight(uint16_t redValue, uint16_t irValue);
void InitSPO2Calculator(SPO2Calculator* calculator);
uint16_t ECG_ADC =0;
uint16_t SPO_RED_ADC =0;
uint16_t SPO_IR_ADC =0;
uint16_t CUFF_ADC =0;
uint16_t PULSE_ADC =0;
extern SPO2Filter spo2_filter;
SPO2Calculator spo2_calculator;   // 血氧计算器（也可作为任务局部静态，但为了方便可以在任务外初始化）
// 添加DAC电压全局变量
float g_redDACVoltage = 0.21f;   // 红光DAC初始电压
float g_irDACVoltage = 0.13f;    // 红外光DAC初始电压
/* 探头脱落和调光完成标志（参考裸机） */
uint8_t g_probeDetached = 1;       // 初始为脱落
uint8_t g_lightAdjustComplete = 0; // 调光未完成
char message[50]= "";
char stamsg2[50]="";
char stamsg3[50]="";
char stamsg4[50]="";
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ECG */
osThreadId_t ECGHandle;
uint32_t ECGBuffer[ 512 ];
osStaticThreadDef_t ECGControlBlock;
const osThreadAttr_t ECG_attributes = {
  .name = "ECG",
  .cb_mem = &ECGControlBlock,
  .cb_size = sizeof(ECGControlBlock),
  .stack_mem = &ECGBuffer[0],
  .stack_size = sizeof(ECGBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SPO2 */
osThreadId_t SPO2Handle;
uint32_t SPO2Buffer[ 512 ];
osStaticThreadDef_t SPO2ControlBlock;
const osThreadAttr_t SPO2_attributes = {
  .name = "SPO2",
  .cb_mem = &SPO2ControlBlock,
  .cb_size = sizeof(SPO2ControlBlock),
  .stack_mem = &SPO2Buffer[0],
  .stack_size = sizeof(SPO2Buffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for NIBP */
osThreadId_t NIBPHandle;
uint32_t NIBPBuffer[ 512 ];
osStaticThreadDef_t NIBPControlBlock;
const osThreadAttr_t NIBP_attributes = {
  .name = "NIBP",
  .cb_mem = &NIBPControlBlock,
  .cb_size = sizeof(NIBPControlBlock),
  .stack_mem = &NIBPBuffer[0],
  .stack_size = sizeof(NIBPBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for COLLECT */
osThreadId_t COLLECTHandle;
const osThreadAttr_t COLLECT_attributes = {
  .name = "COLLECT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LED */
osThreadId_t LEDHandle;
const osThreadAttr_t LED_attributes = {
  .name = "LED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for KEY */
osThreadId_t KEYHandle;
const osThreadAttr_t KEY_attributes = {
  .name = "KEY",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for xQueueECG */
osMessageQueueId_t xQueueECGHandle;
const osMessageQueueAttr_t xQueueECG_attributes = {
  .name = "xQueueECG"
};
/* Definitions for xQueueSPO2 */
osMessageQueueId_t xQueueSPO2Handle;
const osMessageQueueAttr_t xQueueSPO2_attributes = {
  .name = "xQueueSPO2"
};
/* Definitions for xQueueNIBP */
osMessageQueueId_t xQueueNIBPHandle;
const osMessageQueueAttr_t xQueueNIBP_attributes = {
  .name = "xQueueNIBP"
};
/* Definitions for xSemaphore500us */
osSemaphoreId_t xSemaphore500usHandle;
const osSemaphoreAttr_t xSemaphore500us_attributes = {
  .name = "xSemaphore500us"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void vTaskECG(void *argument);
void vTaskSPO2(void *argument);
void vTaskNIBP(void *argument);
void vTaskCollect(void *argument);
void vTaskLED(void *argument);
void vKeyScanTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of xSemaphore500us */
  xSemaphore500usHandle = osSemaphoreNew(1, 1, &xSemaphore500us_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of xQueueECG */
  xQueueECGHandle = osMessageQueueNew (16, sizeof(uint16_t), &xQueueECG_attributes);

  /* creation of xQueueSPO2 */
  xQueueSPO2Handle = osMessageQueueNew (16, sizeof(SPO2_Data_t), &xQueueSPO2_attributes);

  /* creation of xQueueNIBP */
  xQueueNIBPHandle = osMessageQueueNew (16, sizeof(NIBP_Data_t), &xQueueNIBP_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ECG */
  ECGHandle = osThreadNew(vTaskECG, NULL, &ECG_attributes);

  /* creation of SPO2 */
  SPO2Handle = osThreadNew(vTaskSPO2, NULL, &SPO2_attributes);

  /* creation of NIBP */
  NIBPHandle = osThreadNew(vTaskNIBP, NULL, &NIBP_attributes);

  /* creation of COLLECT */
  COLLECTHandle = osThreadNew(vTaskCollect, NULL, &COLLECT_attributes);

  /* creation of LED */
  LEDHandle = osThreadNew(vTaskLED, NULL, &LED_attributes);

  /* creation of KEY */
  KEYHandle = osThreadNew(vKeyScanTask, NULL, &KEY_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  InitSPO2Calculator(&spo2_calculator);  // 初始化血氧计算器
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_vTaskECG */
/**
* @brief Function implementing the ECG thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskECG */
void vTaskECG(void *argument)
{
  /* USER CODE BEGIN vTaskECG */
  static uint16_t ecgad;
  /* Infinite loop */
  for(;;)
  {
    osMessageQueueGet(xQueueECGHandle,&ecgad,0,osWaitForever);
    ECG_ADC = ecgad;
    // sprintf(message,"%d\r\n",ECG_ADC);
    // HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
  }
  /* USER CODE END vTaskECG */
}

/* USER CODE BEGIN Header_vTaskSPO2 */
/**
* @brief Function implementing the SPO2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskSPO2 */
void vTaskSPO2(void *argument)
{
  /* USER CODE BEGIN vTaskSPO2 */
  static SPO2_Data_t spoad;
  uint8_t spo2_value;
  /* Infinite loop */
  for(;;)
  {
    osMessageQueueGet(xQueueSPO2Handle,&spoad,0,osWaitForever);
    StoreSPO2Data(&spo2_calculator, spoad.red, spoad.ir);
    // SPO_RED_ADC = spoad.red;
    // SPO_IR_ADC  = spoad.ir;
    // sprintf(message,"%d,%d\r\n",SPO_RED_ADC,SPO_IR_ADC);
    // HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
    /* 检查是否已累积到足够的数据（缓冲区满） */
    if (spo2_calculator.calcFlag == 1)
    {
      /* 计算血氧值（CalcSpO2 内部会检查探头脱落和调光完成标志） */
      spo2_value = CalcSpO2(&spo2_calculator);

      /* 通过串口打印结果（可改为发送到显示队列） */
      sprintf(stamsg4,"[[4,SPO:%d]]\r\n",spo2_value);
      HAL_UART_Transmit(&huart1,(uint8_t*) stamsg4, strlen(stamsg4), 1000);

      /* 注意：CalcSpO2 内部会重置 calcFlag 和 index，无需额外操作 */
    }
  }
  /* USER CODE END vTaskSPO2 */
}

/* USER CODE BEGIN Header_vTaskNIBP */
/**
* @brief Function implementing the NIBP thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskNIBP */
void vTaskNIBP(void *argument)
{
  /* USER CODE BEGIN vTaskNIBP */
  /* Infinite loop */
  static NIBP_Data_t adc;
  for(;;)
  {
    osMessageQueueGet(xQueueNIBPHandle,&adc,0,osWaitForever);
    CUFF_ADC = adc.cuff;
    PULSE_ADC = adc.pulse;
    //osDelay(1);
  }
  /* USER CODE END vTaskNIBP */
}

/* USER CODE BEGIN Header_vTaskCollect */
/**
* @brief Function implementing the COLLECT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskCollect */
void vTaskCollect(void *argument)
{
  /* USER CODE BEGIN vTaskCollect */
  /* 静态变量用于时序控制 */
  static uint8_t state = 0;          // LED 状态: 0关,1红,2关,3红外,4关
  static uint8_t state_counter = 0;  // 500us 计数器
  static uint8_t s_iCnt3 = 0;        // 采样延时计数器
  static uint8_t s_redSampled = 0, s_irSampled = 0;
  static uint16_t s_redValue = 0, s_irValue = 0;
  static uint16_t red_filtered = 0, ir_filtered = 0;
  /* 2ms 计数器 */
  static uint8_t two_ms_divider = 0;
  /* 临时变量 */
  SPO2_Data_t spo2_data;
  NIBP_Data_t nibp_data;
  uint16_t adc_value;
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreAcquire(xSemaphore500usHandle, osWaitForever);
    /* ---------- 1. 血氧 LED 状态机 ---------- */
    state_counter++;
    if (state_counter >= 4) {//2ms变一次
      state_counter = 0;
      state = (state + 1) % 5;  // 0->1->2->3->4->0
    }
    switch (state) {
    case 0: case 2: case 4://全灭
      HAL_GPIO_WritePin(SPO_RED_CS_GPIO_Port, SPO_RED_CS_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(SPO_IR_CS_GPIO_Port,  SPO_IR_CS_Pin,  GPIO_PIN_SET);
      break;
    case 1://红光亮
      HAL_GPIO_WritePin(SPO_RED_CS_GPIO_Port, SPO_RED_CS_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(SPO_IR_CS_GPIO_Port,  SPO_IR_CS_Pin,  GPIO_PIN_SET);
      Set_DAC_Voltage(g_redDACVoltage);
      break;
    case 3://红外灯亮
      HAL_GPIO_WritePin(SPO_RED_CS_GPIO_Port, SPO_RED_CS_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(SPO_IR_CS_GPIO_Port,  SPO_IR_CS_Pin,  GPIO_PIN_RESET);
      Set_DAC_Voltage(g_irDACVoltage);
      break;
    }
    /* ---------- 2. 血氧采样时刻判断 ---------- */
    if ((state == 1 || state == 3) && s_iCnt3 < 3) {
      s_iCnt3++;
      if (s_iCnt3 == 3) {
        /* 从 DMA 缓冲区读取血氧通道值（假设索引 0） */
        adc_value = ReadADCChannel(SPO_CHANNEL);
        if (state == 1) {
          s_redValue = adc_value;
          s_redSampled = 1;
        } else {
          s_irValue = adc_value;
          s_irSampled = 1;
        }
      }
    } else if (state == 0 || state == 2 || state == 4) {
      s_iCnt3 = 0;
    }
    /* 如果红光和红外都采样完成，打包发送给 SPO2 任务 */
    /* 如果红光和红外都采样完成，先进行探头检测和调光，再决定是否发送滤波后的数据 */
    if (s_redSampled && s_irSampled) {
      // 检测探头脱落（以原始ADC值为准）
      // sprintf(message,"%d,%d\r\n",s_redValue,s_irValue);
      // HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
      if (s_redValue < 20 || s_irValue < 20) {
        g_probeDetached = 1;
        g_lightAdjustComplete = 0;
        // 重置血氧计算器（清空缓冲区）
        spo2_calculator.index = 0;
        spo2_calculator.calcFlag = 0;
        //printf("[[2,探头脱落]]\r\n"); //脱落
        sprintf(stamsg2,"[[2,探头脱落]]\r\n");
        HAL_UART_Transmit(&huart1,(uint8_t*) stamsg2, strlen(stamsg2), 1000);
      } else
      {
        // 探头已连接
        if (g_probeDetached) {
          g_probeDetached = 0;
          //printf("[[2,连接]]\r\n"); //脱落
          sprintf(stamsg2,"[[2,探头连接]]\r\n");
          HAL_UART_Transmit(&huart1,(uint8_t*) stamsg2, strlen(stamsg2), 1000);
        }
        // 调光逻辑
        AdjustLight(s_redValue, s_irValue);

        // 检查调光是否完成
        if (s_redValue >= 825 && s_redValue <= 975 &&
            s_irValue >= 825 && s_irValue <= 975) {
          if (!g_lightAdjustComplete) {
            g_lightAdjustComplete = 1;
            //printf("[[3,调光完成]]\r\n"); //脱落
            sprintf(stamsg3,"[[3,调光完成]]\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t*) stamsg3, strlen(stamsg3), 1000);
          }

          // 发送滤波后的数据给血氧任务
          red_filtered = FilterSPO2Red(&spo2_filter, s_redValue);
          ir_filtered = FilterSPO2IR(&spo2_filter, s_irValue);
          spo2_data.red = red_filtered;
          spo2_data.ir  = ir_filtered;
          // sprintf(message,"%d,%d\r\n",red_filtered,ir_filtered);
          // HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
          osMessageQueuePut(xQueueSPO2Handle, &spo2_data, 0, 0);
            }else {
            g_lightAdjustComplete = 0;
              //printf("[[3,调光未完成]]\r\n"); //脱落
            sprintf(stamsg3,"[[3,调光未完成]]\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t*) stamsg3, strlen(stamsg3), 1000);
          }
      }
      s_redSampled = 0;
      s_irSampled = 0;
    }
    /* ---------- 3. 每 2ms 采集 ECG 和 NIBP ---------- */
    two_ms_divider++;
    if (two_ms_divider >= 4) {
      two_ms_divider = 0;

      /* 读取 ECG 通道 */
      adc_value = ReadADCChannel(ECG_CHANNEL);
      osMessageQueuePut(xQueueECGHandle, &adc_value, 0, 0);

      /* 读取 NIBP 通道 */
      nibp_data.cuff  = ReadADCChannel(CUFF_CHANNEL);
      nibp_data.pulse = ReadADCChannel(PULSE_CHANNEL);
      sprintf(message,"%d,%d\r\n",nibp_data.cuff,nibp_data.pulse);
      HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
      osMessageQueuePut(xQueueNIBPHandle, &nibp_data, 0, 0);
    }
  }


  /* USER CODE END vTaskCollect */
}

/* USER CODE BEGIN Header_vTaskLED */
/**
* @brief Function implementing the LED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLED */
void vTaskLED(void *argument)
{
  /* USER CODE BEGIN vTaskLED */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_WritePin(GPIOC, LED1_Pin,1);
    HAL_GPIO_WritePin(GPIOC, LED2_Pin, 0);
    osDelay(250);
    HAL_GPIO_WritePin(GPIOC, LED1_Pin,0);
    HAL_GPIO_WritePin(GPIOC, LED2_Pin, 1);
    osDelay(250);
    // sprintf(message,"%d\r\n",ECG_ADC);
    // HAL_UART_Transmit(&huart1,(uint8_t*) message, strlen(message), 1000);
  }
  /* USER CODE END vTaskLED */
}

/* USER CODE BEGIN Header_vKeyScanTask */
/**
* @brief Function implementing the KEY thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vKeyScanTask */
void vKeyScanTask(void *argument)
{
  /* USER CODE BEGIN vKeyScanTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END vKeyScanTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
// 存储血氧数据
void StoreSPO2Data(SPO2Calculator* calculator, uint16_t redValue, uint16_t irValue)
{
  if(calculator->index < SPO2_BUF_SIZE)
  {
    calculator->redBuffer[calculator->index] = redValue;
    calculator->irBuffer[calculator->index] = irValue;
    calculator->index++;
  }
  // 缓冲区满时设置计算标志
  if(calculator->index >= SPO2_BUF_SIZE)
  {
    calculator->calcFlag = 1;
  }
}

// 血氧计算函数
uint8_t CalcSpO2(SPO2Calculator* calculator)
{
    uint8_t spo2 = 0;
    uint16_t IRMax = 0, RedMax = 0;
    uint16_t IRMin = 0xFFFF, RedMin = 0xFFFF;
    uint16_t i, j, temp;
    uint16_t irADRng, redADRng, rrVal;
    uint16_t arr_sort[6] = {0};
    uint32_t IRAvg, RedAvg;

		if(!g_lightAdjustComplete || g_probeDetached)
        return calculator->spo2Value;

    if(calculator->calcFlag == 1)
    {

        // 找最大值和最小值
        for(i = 0; i < SPO2_BUF_SIZE; i++)
        {
            if(calculator->irBuffer[i] > IRMax) IRMax = calculator->irBuffer[i];
            if(calculator->irBuffer[i] < IRMin) IRMin = calculator->irBuffer[i];
            if(calculator->redBuffer[i] > RedMax) RedMax = calculator->redBuffer[i];
            if(calculator->redBuffer[i] < RedMin) RedMin = calculator->redBuffer[i];
        }

        irADRng = IRMax - IRMin;      // 红外光交流分量
        redADRng = RedMax - RedMin;   // 红光交流分量

        IRAvg = (IRMax + IRMin) / 2;     // 红外光直流分量
        RedAvg = (RedMax + RedMin) / 2;  // 红光直流分量

        if(irADRng == 0 || RedAvg == 0 || IRAvg == 0)
        {
            calculator->arrRRBuff[5] = 0;
				    calculator->calcFlag = 0;
            calculator->index = 0;
            return 0;
        }

        // 缓冲R值
        for(i = 0; i < 5; i++)
        {
            calculator->arrRRBuff[i] = calculator->arrRRBuff[i+1];
            arr_sort[i] = calculator->arrRRBuff[i];
        }

        // 计算R值
        calculator->arrRRBuff[5] = (redADRng * 1000) / irADRng;
        arr_sort[5] = calculator->arrRRBuff[5];
        //printf("[[2,%d]]\r\n", calculator->arrRRBuff[5]); //设置测量结果显示
        // 对R值进行排序（冒泡排序）
        for(i = 0; i < 5; i++)
        {
            for(j = 0; j < 5-i; j++)
            {
                if(arr_sort[j] > arr_sort[j+1])
                {
                    temp = arr_sort[j];
                    arr_sort[j] = arr_sort[j+1];
                    arr_sort[j+1] = temp;
                }
            }
        }

        // 取中值作为最终R值
        rrVal = (arr_sort[2] + arr_sort[3]) / 2;

        //printf("[[1,%d]]\r\n", rrVal); //设置测量结果显示

				//printf("[[3,%d]]\r\n", redADRng); //设置测量结果显示
				//printf("[[4,%d]]\r\n", irADRng); //设置测量结果显示

        // 查表计算血氧值
        for(i = 0; i < RR_TABLE_SIZE; i++)
        {
            if(rrVal <= RR_TABLE[i])
            {
                break;
            }
        }

        spo2 = 101 - i;  // 计算血氧饱和度

				if(spo2 == 100)
        {
            spo2 = 99;
        }

				if(spo2 == 101)
        {
            spo2 = 99;
        }

        calculator->calcFlag = 0;
				calculator->index = 0;  // 重置索引，开始新一轮数据收集
        calculator->spo2Value = spo2;

        //printf("血氧计算完成: %d%%\r\n", spo2);
				//printf("[[1,计算完成]]\r\n"); //设置测量结果显示
        return spo2;
    }
    return calculator->spo2Value;  // 返回上一次计算的值
}

void AdjustLight(uint16_t redValue, uint16_t irValue)
{
  // 只有探头连接且调光未完成时才调光
  if(g_probeDetached || g_lightAdjustComplete)
    return;

  // 红光调光逻辑
  if(redValue > 975) // 光强太弱，需要增加电压
  {
    g_redDACVoltage += 0.01f;
  }
  else if(redValue < 825) // 光强太强，需要减小电压
  {
    g_redDACVoltage -= 0.01f;
  }

  // 红外光调光逻辑
  if(irValue > 975) // 光强太弱，需要增加电压
  {
    g_irDACVoltage += 0.01f;
  }
  else if(irValue < 825) // 光强太强，需要减小电压
  {
    g_irDACVoltage -= 0.01f;
  }

  // 限制电压范围
  if(g_redDACVoltage < 0.01f) g_redDACVoltage = 0.01f;
  if(g_redDACVoltage > 3.3f) g_redDACVoltage = 3.3f;
  if(g_irDACVoltage < 0.01f) g_irDACVoltage = 0.01f;
  if(g_irDACVoltage > 3.3f) g_irDACVoltage = 3.3f;
}

// 初始化血氧计算器
void InitSPO2Calculator(SPO2Calculator* calculator)
{
  int i;

  calculator->index = 0;
  calculator->calcFlag = 0;
  calculator->spo2Value = 0;

  // 初始化R值缓冲区
  for(i = 0; i < 6; i++)
  {
    calculator->arrRRBuff[i] = 560; // 初始值
  }
}
/* USER CODE END Application */

