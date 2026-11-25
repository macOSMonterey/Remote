#ifndef __Bluetooth_H
#define __Bluetooth_H
#include "stm32f1xx_hal.h"                  // Device header

#define FRAME_MAX_SIZE 64
#define BLUETOOTH_FIFO_BUF_LENGTH 1024

// 蓝牙连接状态枚举
typedef enum
{
    HC05_STATUS_DISCONNECT = 0,   // 未连接
    HC05_STATUS_CONNECTED,         // 已连接（STATE高）
    HC05_STATUS_AT_MODE,           // AT配置模式
    HC05_STATUS_ERROR              // 异常
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
    uint32_t last_recv_tick;        // 最后收到数据的系统滴答，用于超时判断
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

void bluetooth_usart_task(void *argument);
uint8_t BlueTooth_R_STATE(HC05_HandleTypeDef *hc_05);

#endif
