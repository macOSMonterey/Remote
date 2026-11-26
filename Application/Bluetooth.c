#include "cmsis_os2.h"
#include "stm32f1xx_hal.h"                  // Device header
#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_gpio.h"
#include "string.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "stdlib.h"
#include "Bluetooth.h"
#include "usart.h"
#include "fifo.h"
#include <stdint.h>

HC05_HandleTypeDef HC_05;
hc_05_unpack_data_t hc_05_unpack_obj;
//uint8_t BlueTooth_RxPacket[FRAME_MAX_SIZE];
fifo_s_t bluetooth_fifo;
uint8_t bluetooth_fifo_buf[BLUETOOTH_FIFO_BUF_LENGTH];

hc_05_link_t hc_05_link;

HAL_StatusTypeDef hc_05_unpack_fifo_data(void);

void BlueTooth_Init(HC05_HandleTypeDef *hc_05)
{
	hc_05->hw.EN_Pin=GPIO_PIN_15;
	hc_05->hw.EN_Port=GPIOA;
	hc_05->hw.STATE_Pin=GPIO_PIN_8;
	hc_05->hw.STATE_Port=GPIOA;
	hc_05->hw.huart=&huart1;
	fifo_s_init(&bluetooth_fifo, bluetooth_fifo_buf, BLUETOOTH_FIFO_BUF_LENGTH);
}

void BlueTooth_W_EN(HC05_HandleTypeDef *hc_05,uint8_t BitValue)
{
	HAL_GPIO_WritePin(hc_05->hw.EN_Port,hc_05->hw.EN_Pin,(GPIO_PinState)BitValue);
}

uint8_t BlueTooth_R_STATE(HC05_HandleTypeDef *hc_05)
{
	hc_05->state.at_flag=!(HAL_GPIO_ReadPin(hc_05->hw.STATE_Port,hc_05->hw.STATE_Pin));
	return hc_05->state.at_flag;
}

void BlueTooth_Transmit(HC05_HandleTypeDef *hc_05,uint8_t *string,uint16_t Size)
{
	if(BlueTooth_R_STATE(hc_05)==0)HAL_UART_Transmit(hc_05->hw.huart,string,Size,100);
}

HAL_StatusTypeDef BlueTooth_Command(HC05_HandleTypeDef *hc_05,const char *string,uint8_t time)
{
	BlueTooth_W_EN(hc_05,1);
	if(!time)
	{
		osDelay(pdMS_TO_TICKS(100));
		BlueTooth_W_EN(hc_05,0);
	}
	osDelay(pdMS_TO_TICKS(100));
	HAL_UART_Transmit(hc_05->hw.huart,(const uint8_t *)string,strlen(string),100);
	if(time)
	{
		BlueTooth_W_EN(hc_05,0);
	}
	osDelay(50);
	return hc_05_unpack_fifo_data();
}

HAL_StatusTypeDef hc_05_unpack_fifo_data(void)
{
  static uint8_t clear_flag=1;
  uint8_t byte = 0;
  static uint8_t pre_byte = 0xff;
  uint8_t sof_1 = 0x0D;
  uint8_t sof_2 = 0x0A;
  hc_05_unpack_data_t *p_obj = &hc_05_unpack_obj;

  while ( fifo_s_used(&bluetooth_fifo) )
  {
    byte = fifo_s_get(&bluetooth_fifo);
    switch(p_obj->unpack_step)
    {
      case STEP_TAIL_SOF:
      {
        if(pre_byte == sof_1&&byte == sof_2)
        {
			if(p_obj->index>0)
			{
				p_obj->index--;
				p_obj->protocol_packet[p_obj->index] = 0x00;
				if(clear_flag==0)
				{
					memcpy(&(HC_05.state.info[strlen((const char *)HC_05.state.info)]),p_obj->protocol_packet, p_obj->index);
					clear_flag=1;
					if(p_obj->protocol_packet[0]=='O'&&p_obj->protocol_packet[1]=='K')return HAL_OK;
					else return HAL_ERROR;
				}
				else if(p_obj->protocol_packet[0]=='+')
				{
					memset(HC_05.state.info, 0,64);
					memcpy(HC_05.state.info,&(p_obj->protocol_packet[1]) , p_obj->index-1);
					HC_05.state.info[p_obj->index-1]=',';
					clear_flag=0;
				}
				else
				{
					memcpy(HC_05.state.info,p_obj->protocol_packet , p_obj->index);
					if(p_obj->protocol_packet[0]=='O'&&p_obj->protocol_packet[1]=='K')return HAL_OK;
					else return HAL_ERROR;
				}
				pre_byte = 0xff;
				p_obj->unpack_step = STEP_TAIL_SOF;
				p_obj->index = 0;
			}
        }
        else
        {
			if(p_obj->index<FRAME_MAX_SIZE)
			p_obj->protocol_packet[p_obj->index++] = byte;
        }
		pre_byte=byte;
      }break;
      default:
      {
		pre_byte = 0xff;
        p_obj->unpack_step = STEP_TAIL_SOF;
        p_obj->index = 0;
      }break;
    }
  }
  return HAL_ERROR;
}

LINK_StatusTypeDef HC_05_link(HC05_HandleTypeDef *hc_05)
{
  LINK_StatusTypeDef result=HC_05_ERROR;
  if(BlueTooth_R_STATE(&HC_05))
  {
	hc_05_link.retry_count=0;
	hc_05_link.retry_max=2;
	hc_05_link.link_step=STEP_AVAILABLE;
	while(hc_05_link.retry_count < hc_05_link.retry_max)
    {
		hc_05_link.retry_count++;
		if(hc_05_link.link_step==STEP_AVAILABLE)
		{
			if(BlueTooth_Command(hc_05,"AT\r\n", 0)==HAL_OK)
			{
				hc_05_link.link_step=STEP_LINK;
			}
			else
			{
				result=HC_05_NOAT;
				osDelay(100);
				continue;
			}
		}
		if(hc_05_link.link_step==STEP_LINK)
		{
			if(BlueTooth_Command(hc_05,"AT+LINK=0425,03,0709D5\r\n", 0)==HAL_OK)
			{
				osDelay(8000);
				hc_05_link.link_step=STEP_CHECK;
			}
			else
			{
				result=HC_05_NOLINK;
				osDelay(100);
				continue;
			}
		}
		if(hc_05_link.link_step==STEP_CHECK)
		{
			if(BlueTooth_R_STATE(hc_05)==0)
			{
				hc_05_link.link_step=STEP_AVAILABLE;
				result=HC_05_OK;
				break;
			}
			else
			{
				result=HC_05_NOCHECK;
				osDelay(100);
				continue;
			}
		}
	}
  }
  else
  {
	BlueTooth_Command(hc_05,"AT+DISC\r\n", 1);
	osDelay(2000);
	if(BlueTooth_R_STATE(hc_05))
	{
		result=HC_05_OK;
	}
	else
	{
		result=HC_05_NODISC;
	}
  }
  return result;
}
/*
void BlueTooth_Task(void *p)
{	
	uint8_t i=0;
	while(1)
	{
		if(pRead!=pWrite)
		{
			taskENTER_CRITICAL();
			if(ATState==0)
			{
				if(BlueTooth_Buffer[pRead]=='[')
				{
					i=0;
					pRead++;
					if(pRead>255)pRead=0;
					while(i<62)
					{
						if(pRead==pWrite||BlueTooth_Buffer[pRead]==']')break;
						BlueTooth_RxPacket[i]=BlueTooth_Buffer[pRead];
						i++;
						pRead++;
						if(pRead>127)pRead=0;
					}
					BlueTooth_RxPacket[i]='\0';
					i=0;
					xTaskNotifyGive(xHandle_BT_Check);
				}
			}
			else
			{
				i=0;
				while(i<62)
				{
					if(pRead==pWrite||BlueTooth_Buffer[pRead]=='\n')break;
					BlueTooth_RxPacket[i]=BlueTooth_Buffer[pRead];
					i++;
					pRead++;
					if(pRead>127)pRead=0;
				}
				BlueTooth_RxPacket[i++]='\n';
				BlueTooth_RxPacket[i]='\0';
				i=0;
				xTaskNotifyGive(xHandle_BT_Check);
				ATState=0;
			}
			if(pRead!=pWrite)
			{
				pRead++;
				if(pRead>127)pRead=0;
			}
			taskEXIT_CRITICAL();
		}
		else
		{
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}

void BT_Check_Task(void *p)
{
	double Vx=0,Vy=0,Wz=0,Dz=0;
	char buffer[32]={0};
	while(1)
	{
		if(BlueTooth_MODE==0)
		{
			vTaskPrioritySet(NULL,1);
			OLED_ShowLabel(1,5,9,"%s","DISABLE");
			OLED_ShowLabel(2,1,21,"");
			OLED_ShowLabel(3,1,21,"");
			vTaskDelay(pdMS_TO_TICKS(500));
		}
		else
		{
			while(BlueTooth_MODE)
			{
				BlueTooth_Command("AT\r\n",0);
				ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(1000));
				if(strstr(BlueTooth_RxPacket,"OK")!=NULL)break;
				OLED_ShowLabel(1,5,9,"%s","NO DEVICE");
				vTaskDelay(pdMS_TO_TICKS(2000));
			}
			if(BlueTooth_MODE==0)continue;
			OLED_ShowLabel(1,5,9,"%s","READY NOW");
			vTaskDelay(pdMS_TO_TICKS(2000));
			vTaskPrioritySet(NULL,2);
			OLED_ShowLabel(1,5,9,"%s","LINKING");
		    vTaskDelay(pdMS_TO_TICKS(1000));
		    BlueTooth_Command("AT+LINK=0425,03,0709D5\r\n",0);
		    ulTaskNotifyTake(pdTRUE,pdMS_TO_TICKS(5000));
			vTaskDelay(pdMS_TO_TICKS(4000));
			if(BlueTooth_MODE==0)
			{
				OLED_ShowLabel(1,5,9,"%s","DISABLE");
				continue;
			}
		    if(BlueTooth_R_STATE()==0)
		    {
				OLED_ShowLabel(1,5,9,"%s","TIME OUT");
				vTaskDelay(pdMS_TO_TICKS(2000));
		    	continue;
		    }
			OLED_ShowLabel(1,5,9,"%s","CONNECTED");
			while(BlueTooth_MODE==1)
			{
				memset(buffer,0,32);
				sprintf(buffer,"[handle,%d,%d,%d,%d]",(int)(AD_GetValue_Z()*AD_GetValue_Z()*0.0000059+AD_GetValue_Z()*0.0248-100.10),0,(int)(AD_GetValue_X()*AD_GetValue_X()*0.0000059+AD_GetValue_X()*0.0248-100.10),(int)(AD_GetValue_Y()*AD_GetValue_Y()*0.0000059+AD_GetValue_Y()*0.0248-100.10));
				BlueTooth_Send(buffer);
				Tiqu(BlueTooth_RxPacket,&Vx,&Vy,&Wz,&Dz);
				OLED_ShowLabel(2,1,21,"Vx:%.2f Vy:%.2f",Vx,Vy);
				OLED_ShowLabel(3,1,21,"Wz:%.2f Dz:%.1f",Wz,Dz);
				if(BlueTooth_R_STATE()==0)break;
				vTaskDelay(pdMS_TO_TICKS(200));
			}
			BlueTooth_Command("AT+DISC\r\n",1);
			ulTaskNotifyTake(pdTRUE,5000);
			OLED_ShowLabel(1,5,9,"%s","DISABLE");
		}
	}
}
*/