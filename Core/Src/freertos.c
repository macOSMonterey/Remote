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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include <stdint.h>
#include "OLED.h"
#include "Joystick.h"
#include "filter.h"
#include "referee_usart_task.h"
#include "Serial.h"
#include "encoder.h"
#include "Bluetooth.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_Pin       GPIO_PIN_0
#define KEY_GPIO_Port GPIOB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
Move_t move;
Kalman1D_t Kalman1D_ADC_X,Kalman1D_ADC_Y,Kalman1Dvol_x,Kalman1Dvol_y,Kalman1D_X,Kalman1D_Y;
extern Joystick_ADC_t joystick;
extern Encoder_t encoder;
/* USER CODE END Variables */
/* Definitions for Joystick_Task */
osThreadId_t Joystick_TaskHandle;
const osThreadAttr_t Joystick_Task_attributes = {
  .name = "Joystick_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for OLED_Task */
osThreadId_t OLED_TaskHandle;
const osThreadAttr_t OLED_Task_attributes = {
  .name = "OLED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SerialData_Tsk */
osThreadId_t SerialData_TskHandle;
const osThreadAttr_t SerialData_Tsk_attributes = {
  .name = "SerialData_Tsk",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Key_Task */
osThreadId_t Key_TaskHandle;
const osThreadAttr_t Key_Task_attributes = {
  .name = "Key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for referee_task */
osThreadId_t referee_taskHandle;
const osThreadAttr_t referee_task_attributes = {
  .name = "referee_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ENCODER_Task */
osThreadId_t ENCODER_TaskHandle;
const osThreadAttr_t ENCODER_Task_attributes = {
  .name = "ENCODER_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for BLUETOOTH_Task */
osThreadId_t BLUETOOTH_TaskHandle;
const osThreadAttr_t BLUETOOTH_Task_attributes = {
  .name = "BLUETOOTH_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Joy_Mutex */
osMutexId_t Joy_MutexHandle;
const osMutexAttr_t Joy_Mutex_attributes = {
  .name = "Joy_Mutex"
};
/* Definitions for sem_KeyPressed */
osSemaphoreId_t sem_KeyPressedHandle;
const osSemaphoreAttr_t sem_KeyPressed_attributes = {
  .name = "sem_KeyPressed"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void Joystick(void *argument);
void OLED(void *argument);
void SerialData(void *argument);
void Key(void *argument);
extern void referee_usart_task(void *argument);
void ENCODER(void *argument);
extern void bluetooth_usart_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of Joy_Mutex */
  Joy_MutexHandle = osMutexNew(&Joy_Mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of sem_KeyPressed */
  sem_KeyPressedHandle = osSemaphoreNew(1, 0, &sem_KeyPressed_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Joystick_Task */
  Joystick_TaskHandle = osThreadNew(Joystick, NULL, &Joystick_Task_attributes);

  /* creation of OLED_Task */
  OLED_TaskHandle = osThreadNew(OLED, NULL, &OLED_Task_attributes);

  /* creation of SerialData_Tsk */
  SerialData_TskHandle = osThreadNew(SerialData, NULL, &SerialData_Tsk_attributes);

  /* creation of Key_Task */
  Key_TaskHandle = osThreadNew(Key, NULL, &Key_Task_attributes);

  /* creation of referee_task */
  referee_taskHandle = osThreadNew(referee_usart_task, NULL, &referee_task_attributes);

  /* creation of ENCODER_Task */
  ENCODER_TaskHandle = osThreadNew(ENCODER, NULL, &ENCODER_Task_attributes);

  /* creation of BLUETOOTH_Task */
  BLUETOOTH_TaskHandle = osThreadNew(bluetooth_usart_task, NULL, &BLUETOOTH_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Joystick */
/**
  * @brief  Function implementing the Joystick_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Joystick */
void Joystick(void *argument)
{
  /* USER CODE BEGIN Joystick */
  JOYSTICK_Init(&joystick);
  Kalman1D_Init(&Kalman1D_ADC_X, 0.2, 2.0, 2048);
  Kalman1D_Init(&Kalman1D_ADC_Y, 0.2, 2.0, 2048);
  Kalman1D_Init(&Kalman1Dvol_x, 0.2, 2.0, 2.5);
  Kalman1D_Init(&Kalman1Dvol_y, 0.2, 2.0, 2.5);
  Kalman1D_Init(&Kalman1D_X, 0.2, 2.0, 0);
  Kalman1D_Init(&Kalman1D_Y, 0.2, 2.0, 0);
  
  uint32_t tick = osKernelGetTickCount();
  uint32_t T_ms = 10;
  /* Infinite loop */
  for(;;)
  {
    osMutexAcquire(Joy_MutexHandle, osWaitForever);
    tick = osKernelGetTickCount();
    JOYSTICK_Read(&joystick);
    joystick.filteredData.adc_x=kalman_filter(&Kalman1D_ADC_X, joystick.Data.adc_x);
    joystick.filteredData.adc_y=kalman_filter(&Kalman1D_ADC_Y, joystick.Data.adc_y);
    joystick.filteredData.vol_x=kalman_filter(&Kalman1Dvol_x, joystick.Data.vol_x);
    joystick.filteredData.vol_y=kalman_filter(&Kalman1Dvol_y, joystick.Data.vol_y);
    joystick.filteredData.x=kalman_filter(&Kalman1D_X, joystick.Data.x);
    joystick.filteredData.y=kalman_filter(&Kalman1D_Y, joystick.Data.y);
    osMutexRelease(Joy_MutexHandle);
    tick += pdMS_TO_TICKS(T_ms);
    osDelayUntil(tick);
  }
  /* USER CODE END Joystick */
}

/* USER CODE BEGIN Header_OLED */
/**
* @brief Function implementing the OLED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_OLED */
void OLED(void *argument)
{
  /* USER CODE BEGIN OLED */
  OLED_Init();
  OLED_ShowString(1, 1, "Vx:");
  OLED_ShowString(2, 1, "Vy:");
  OLED_ShowString(3, 1, "Yaw:");
  /* Infinite loop */
  for(;;)
  {
    OLED_ShowFloat(1, 4, move.Vx, 2, 2);
    OLED_ShowFloat(2, 4, move.Vy, 2, 2);
    OLED_ShowFloat(3, 5, move.Yaw, 2, 2);
    osDelay(10);
  }
  /* USER CODE END OLED */
}

/* USER CODE BEGIN Header_SerialData */
/**
* @brief Function implementing the SerialData_Tsk thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SerialData */
void SerialData(void *argument)
{
  /* USER CODE BEGIN SerialData */
  /* Infinite loop */
  for(;;)
  {
    move.Vx=joystick.filteredData.x*40.0f;
    move.Vy=joystick.filteredData.y*40.0f;
    move.Yaw=encoder.angle;
    CustomController_StructSend(&move);
    osDelay(10);
  }
  /* USER CODE END SerialData */
}

/* USER CODE BEGIN Header_Key */
/**
* @brief Function implementing the Key_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Key */
void Key(void *argument)
{
  /* USER CODE BEGIN Key */
  uint32_t press_time = 0;
  /* Infinite loop */
  for(;;)
  {// 1. 没按键时，死等在这里睡觉（0% CPU占用）
    if (osSemaphoreAcquire(sem_KeyPressedHandle, osWaitForever) == osOK) {
      osDelay(20);
      if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) != GPIO_PIN_RESET) {
          continue; 
      }
      press_time = osKernelGetTickCount(); // 记录按下的时间
      // 2. 判断按住的时间（这是一个简单的循环，不复杂）
      while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
        if ((osKernelGetTickCount() - press_time) >= 2000) {
          osMutexAcquire(Joy_MutexHandle, osWaitForever);
          if(JOYSTICK_Calib(&joystick,500)!=HAL_OK)HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
          osSemaphoreAcquire(sem_KeyPressedHandle, 0);
          osMutexRelease(Joy_MutexHandle);
          break;
        }
        osDelay(10);
      }
    }
  }
  /* USER CODE END Key */
}

/* USER CODE BEGIN Header_ENCODER */
/**
* @brief Function implementing the ENCODER_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ENCODER */
void ENCODER(void *argument)
{
  /* USER CODE BEGIN ENCODER */
  Encoder_Init(&encoder);
  /* Infinite loop */
  for(;;)
  {
    Encoder_Get(&encoder);
    Encoder_to_Angle(&encoder);
    osDelay(10);
  }
  /* USER CODE END ENCODER */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) 
{
  // 1. 识别是不是我们的按键引脚（PE4）
  if (GPIO_Pin == KEY_Pin) {
    // 2. （可选防抖）再读一次电平，确认是真的按下了，而不是毛刺
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
      // 3. 释放信号量，通知任务
      // 注意：在中断中调用，不要用带阻塞的API，osSemaphoreRelease是安全的
      osSemaphoreRelease(sem_KeyPressedHandle);
    }
  }
}
/* USER CODE END Application */

