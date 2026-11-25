#include "usart.h"
#include "Serial.h"
#include "crc.h"
#include "Bluetooth.h"

JudgeTxFrame            txFrame;
customController_t      CustomController;
extern HC05_HandleTypeDef HC_05;

void CustomController_StructSend(Move_t *data)
{
    //datapack_ordorcount = HAL_GetTick();
    //data->res[0] = datapack_ordorcount>>8;
    //data->res[1] = datapack_ordorcount & 0xff;
    //设置帧头
    CustomController.txFrameHeader.SOF = 0XA5;
    CustomController.txFrameHeader.DataLength = sizeof(Move_t);
    CustomController.txFrameHeader.Seq = 0;
    //将帧头复制到发送帧数据中
    memcpy(txFrame.data, &CustomController.txFrameHeader, sizeof(xFrameHeader));
    //计算并添加CRC8校验码
    Append_CRC8_Check_Sum(txFrame.data, sizeof(xFrameHeader));
    //设置数据帧头
    CustomController.CmdID = 0x01;  // 数据帧ID
    //将映射数据复制到数据帧中
    CustomController.move = *data;
    //将数据帧的其余部分复制到发送帧数据中
    memcpy(txFrame.data + sizeof(xFrameHeader),
           (uint8_t*)&CustomController.CmdID,
           sizeof(CustomController.CmdID) + sizeof(Move_t));
    // 计算并添加CRC16校验码
    Append_CRC16_Check_Sum(txFrame.data, sizeof(xFrameHeader) + sizeof(CustomController.CmdID) + sizeof(CustomController.move)+2);
    // 设置发送帧的总长度
    txFrame.frameLength = sizeof(xFrameHeader) + sizeof(CustomController.CmdID) + sizeof(CustomController.move)+ sizeof(CustomController.FrameTail);
    // 初始化发送缓冲区
    memset(txFrame.data + txFrame.frameLength, 0, JUDGE_MAX_TX_LENGTH - txFrame.frameLength);
    // 发送数据
    if(BlueTooth_R_STATE(&HC_05)==0)
    HAL_UART_Transmit(&huart1, txFrame.data, txFrame.frameLength,100);
}
