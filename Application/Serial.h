#ifndef __serial_H
#define __serial_H

#include "main.h"
#define JUDGE_MAX_TX_LENGTH 40

typedef struct __packed
{
    float x;       
    float y;     
    float Yaw;      
} Move_t;     

/**
 * @brief 自定义控制器通信帧头
 */
typedef struct __packed
{
    uint8_t     SOF;
    uint16_t    DataLength;
    uint8_t     Seq;
    uint8_t     CRC8;
} xFrameHeader;             //帧头结构体		5字节

/**
 * @brief 自定义控制器完整发送帧
 */
typedef struct __packed
{
    xFrameHeader                    txFrameHeader;  //帧头			5字节
    uint16_t                        CmdID;          //命令码			2字节
    Move_t                  move;       //电机映射数据段	12字节
    uint16_t                        FrameTail;      //CRC16 			2字节
} customController_t;		//											39字节

typedef struct
{
    uint8_t data[JUDGE_MAX_TX_LENGTH];
    uint16_t frameLength;
} JudgeTxFrame;

void CustomController_StructSend(Move_t *data);

#endif
