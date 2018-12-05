#include "temperature.h"
#include "stm32f0xx_hal.h"
#include "i2c.h"
#include "usart.h"

/* Addresses defines */
#include "display.h"
#include "string.h"
#define DH12_ADDRESS  0xB8	
#define REG_BACK   0x00 
/* USER CODE BEGIN 1 */
/**
  * @brief  读取温湿度值
  * @param  tValue：温度指针
            hValue：湿度指针
  * @retval 0：成功
            1：获取失败
  */
uint8_t get_temp_humi(float *tValue,float *hValue)
{
  uint8_t temp[5];
   /* Timeout is set to 500ms */ 
  if (HAL_I2C_Mem_Read(&hi2c1, DH12_ADDRESS ,REG_BACK,I2C_MEMADD_SIZE_8BIT,temp,5,500) != HAL_OK)
  {
    /* Reading process Error */
   // Error_Handler();
    return 1;
  }
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
  {
  } 
  if(temp[4]!=(temp[0]+temp[1]+temp[2]+temp[3])) return 1;
  
  *hValue=(uint16_t)(temp[0]) *10+temp[1];  //湿度值
  if(temp[3]&0X80)	//温度值为负
  {
    *tValue =0-((uint16_t)(temp[2])*10+(temp[3]&0x7F));
  }
  else   
  {
    *tValue=(uint16_t)(temp[2])*10+(temp[3]&0x7F);
  }
  //温度范围：-20~60		湿度：20~95
  if(*hValue>950) 
  {
     *hValue=950;
  }
  if(*hValue<200)
  {
     *hValue =200;
  }
  if(*tValue>600)
  {
    *tValue=600;
  }
  if(*tValue<-200)
  {
    *tValue = -200;
  }
  *hValue=(*hValue)/10;
  *tValue=(*tValue)/10;

  return 0; 
}

void printf_TH(void)
{
	uint8_t showBuf[20]={0};
	float tempValue=0.0,humiValue=0.0;
	static uint8_t sw=0;//每次显示1个温湿度
  if(!get_temp_humi(&tempValue,&humiValue))
  {
	  
	  DisplayClean(36,65,0);
		//DisplayUpDown(46,65,0);
//		sprintf((char*)showBuf,"%0.1f\" %0.1f%%",tempValue,humiValue);
//		Usart_Printf(&huart1,showBuf,strlen((char*)showBuf));
//		DisplayString5X7(36,59,showBuf,strlen((char*)showBuf),1);
		 if(sw)
		 {
			 sw = 0;
			 sprintf((char*)showBuf,"%0.1f\"",tempValue);
			 DisplayString5X7(36,65,showBuf,5,3);
			 
		 }
		 else{
			 sw = 1;
			 sprintf((char*)showBuf,"%0.1f%%",humiValue);
			 DisplayString5X7(36,65,showBuf,5,3);
		 }

  }   
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/