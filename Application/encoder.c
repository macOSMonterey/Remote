#include "stm32f1xx_hal.h"                  // Device header
#include "math.h"
#include "encoder.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"

Encoder_t encoder;
void Encoder_Init(Encoder_t *encoder)
{
	encoder->count=0;
	encoder->htim=&htim3;
	encoder->ppr=20;

	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL); 
}
void Encoder_Get(Encoder_t *encoder)
{
	encoder->count = (int16_t)__HAL_TIM_GET_COUNTER(encoder->htim);
}

void Encoder_to_Angle(Encoder_t *encoder)
{
	encoder->angle=encoder->count/4.0f/encoder->ppr*2*M_PI;
}
