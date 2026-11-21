#ifndef __Joystick_H
#define __Joystick_H

typedef struct
{
    uint16_t adc_x;   // X轴原始ADC值 0~4095
    uint16_t adc_y;   // Y轴原始ADC值 0~4095
} Joystick_ADC_RawData_t;

typedef struct
{
    uint16_t adc_x;   // X轴原始ADC值 0~4095
    uint16_t adc_y;   // Y轴原始ADC值 0~4095
    float vol_x;      // X轴电压
    float vol_y;      // Y轴电压
    int16_t offset_x; // X轴零点偏移（校准用）
    int16_t offset_y; // Y轴零点偏移（校准用）
    int16_t x;        // 归一化后坐标，例如 -2000 ~ +2000
    int16_t y;        // 归一化后坐标，例如 -2000 ~ +2000
} Joystick_ADC_Data_t;

// 手柄XY ADC数据结构体
typedef struct
{
    Joystick_ADC_RawData_t RawData;
    Joystick_ADC_Data_t Data;
} Joystick_ADC_t;

void JOYSTICK_Read(Joystick_ADC_t *joy);

#endif
