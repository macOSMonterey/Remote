#ifndef __encoder_H
#define __encoder_H
#include "stm32f1xx_hal.h"                  // Device header

// 编码器结构体
typedef struct
{
    TIM_HandleTypeDef *htim;    // 编码器使用的TIM句柄
    int16_t count;              // 累计计数值（带符号，解决溢出）
    //int16_t last_raw;           // 上一次原始寄存器值
    //int32_t delta;              // 本次增量
    //float speed;                // 转速，单位：脉冲/秒 或者 rad/s、r/min
    uint16_t ppr;               // 编码器线数 PPR
    //float sample_period;        // 采样周期 s，比如 0.01 = 10ms
    float angle;
} Encoder_t;

void Encoder_Init(Encoder_t *encoder);
void Encoder_Get(Encoder_t *encoder);
void Encoder_to_Angle(Encoder_t *encoder);

#endif
