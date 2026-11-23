#include "cmsis_os2.h"
#include "stm32f1xx_hal.h"
#include "Joystick.h"
#include "adc.h"

Joystick_ADC_t joystick;

uint16_t adc_clamp(int32_t val)
{
    if(val < 0) return 0;
    if(val > 4095) return 4095;
    return (uint16_t)val;
}

void JOYSTICK_Init(Joystick_ADC_t *joy)
{
    joy->Data.mid_x=joy->Data.mid_y=2048;
}

void joy_mapadc(Joystick_ADC_t *joy)
{
    int32_t tmp;
    // X轴
    if(joy->RawData.adc_x < joy->Data.mid_x)
    {
        tmp = 2048LL * joy->RawData.adc_x / joy->Data.mid_x;
    }
    else
    {
        tmp = 2048LL + 2048LL * (joy->RawData.adc_x - joy->Data.mid_x) / (4095U - joy->Data.mid_x);
    }
    joy->Data.adc_x = adc_clamp(tmp);

    // Y轴
    if(joy->RawData.adc_y < joy->Data.mid_y)
    {
        tmp = 2048LL * joy->RawData.adc_y / joy->Data.mid_y;
    }
    else
    {
        tmp = 2048LL + 2048LL * (joy->RawData.adc_y - joy->Data.mid_y) / (4095U - joy->Data.mid_y);
    }
    joy->Data.adc_y = adc_clamp(tmp);
}

void joy_mapx(Joystick_ADC_t *joy,float Delta)
{
    if(joy->Data.adc_x-2048>=Delta)
    {
        joy->Data.x=(joy->Data.adc_x-Delta-2048.0f)/(2048.0f-Delta);
    }
    else if(joy->Data.adc_x-2048<=-Delta)
    {
        joy->Data.x=(joy->Data.adc_x+Delta-2048.0f)/(2048.0f-Delta);
    }
    else 
    {
        joy->Data.x=0;
    }
}

void joy_mapy(Joystick_ADC_t *joy,float Delta)
{
    if(joy->Data.adc_y-2048>=Delta)
    {
        joy->Data.y=(joy->Data.adc_y-Delta-2048.0f)/(2048.0f-Delta);
    }
    else if(joy->Data.adc_y-2048<=-Delta)
    {
        joy->Data.y=(joy->Data.adc_y+Delta-2048.0f)/(2048.0f-Delta);
    }
    else 
    {
        joy->Data.y=0;
    }
}

void JOYSTICK_Read(Joystick_ADC_t *joy)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    // ========== 读取 IN0 ==========
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        joy->RawData.adc_x = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    // ========== 读取 IN1 ==========
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
    if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        joy->RawData.adc_y = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
	
    joy_mapadc(joy);
    // 电压换算 3.3V参考
    joy->Data.vol_x = joy->Data.adc_x * 5.0f / 4095.0f;
    joy->Data.vol_y = joy->Data.adc_y * 5.0f / 4095.0f;

    // 归一化，中位2048
    joy_mapx(joy,100.0f);
    joy_mapy(joy,100.0f);
}

HAL_StatusTypeDef JOYSTICK_Calib(Joystick_ADC_t *joy,uint16_t samples)
{
    int32_t sum_x = 0, sum_y = 0;
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
    for(uint16_t i = 0; i < samples; i++)
    {
        if(i%10==0)HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
        JOYSTICK_Read(joy);
        sum_x += (int32_t)(joy->RawData.adc_x);
        sum_y += (int32_t)(joy->RawData.adc_y);
        osDelay(10);
    }
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
    joy->Data.mid_x = sum_x / samples;
    joy->Data.mid_y = sum_y / samples;
    return HAL_OK;
}
