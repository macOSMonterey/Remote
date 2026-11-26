#ifndef __Bluetooth_H
#define __Bluetooth_H
#include "stm32f1xx_hal.h"                  // Device header
#include <stdint.h>

#define FRAME_MAX_SIZE 64
#define BLUETOOTH_FIFO_BUF_LENGTH 1024

// 蓝牙连接状态枚举
typedef enum
{
    HC05_STATUS_DISCONNECT = 0,   // 未连接
    HC05_STATUS_CONNECTED = 1,         // 已连接（STATE高）
} HC05_ConnStateTypeDef;

// HC05 硬件引脚信息结构体
typedef struct
{
    GPIO_TypeDef* EN_Port;
    uint16_t      EN_Pin;

    GPIO_TypeDef* STATE_Port;
    uint16_t      STATE_Pin;

    UART_HandleTypeDef *huart;     // 绑定串口句柄
} HC05_HwInfoTypeDef;

// HC05 软件运行状态结构体
typedef struct
{
    HC05_ConnStateTypeDef conn_state;
    uint8_t at_flag;                // 1：当前处于AT模式，0：透传
    uint8_t info[64];
} HC05_SoftStateTypeDef;

// 合并总结构体，一个实例管理一个HC05模块
typedef struct
{
    HC05_HwInfoTypeDef hw;
    HC05_SoftStateTypeDef state;
} HC05_HandleTypeDef;

//解包步骤
typedef enum
{
  STEP_TAIL_SOF  = 0,

} hc_05_unpack_step_e;

//解包状态机
typedef struct
{
  uint8_t        protocol_packet[FRAME_MAX_SIZE];
  hc_05_unpack_step_e  unpack_step;
  uint16_t       index;
} hc_05_unpack_data_t;

//连接步骤
typedef enum
{
  STEP_AVAILABLE  = 0,
  STEP_LINK  = 1,
  STEP_CHECK  = 2,
} hc_05_link_step_e;

typedef enum
{
  HC_05_OK     = 0x00U,
  HC_05_NOAT       = 0x01U,
  HC_05_NOLINK    = 0x02U,
  HC_05_NOCHECK     = 0x03U,
  HC_05_NODISC     = 0x04U,
  HC_05_ERROR     = 0x05U,
} LINK_StatusTypeDef;

//连接状态机
typedef struct
{
  uint8_t        retry_count;
  uint8_t        retry_max;
  hc_05_link_step_e  link_step;
  uint8_t  result[8];
} hc_05_link_t;

void BlueTooth_Init(HC05_HandleTypeDef *hc_05);
void bluetooth_usart_task(void *argument);
uint8_t BlueTooth_R_STATE(HC05_HandleTypeDef *hc_05);
LINK_StatusTypeDef HC_05_link(HC05_HandleTypeDef *hc_05);

#endif
