/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include "string.h"
#include "display.h"
#include "wifi.h"
/*-------------BUG--------------
//STM32CubeMX��BUG�����β�������RTC_ALARMMASK_ALL
  ���޸�MX_RTC_Init�Ķ�ʱ,
  sAlarm.AlarmMask =RTC_ALARMMASK_ALL;// RTC_ALARMMASK_DATEWEEKDAY; 
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE; //����ÿ���ж�

���⣺
MX_RTC_Init()�����ڲ�ѯ�Ƿ��һ�ε��òų�ʼ��ʱ�䡣
----------------------------------*/
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */
  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
    /* Peripheral interrupt init */
    HAL_NVIC_EnableIRQ(RTC_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(RTC_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

void RTC_StopAlarm(void)
{
  HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);  
}
void RTC_ResetAlarm(void)
{
 RTC_AlarmTypeDef sNowAlarm; 
// HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
 HAL_RTC_GetAlarm(&hrtc,&sNowAlarm,RTC_ALARM_A,RTC_FORMAT_BCD);
//��Ҫ ��������Alarm��������HAL_RTC_DeactivateAlarm
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sNowAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
}
/**
  * @brief  ��������ʱ��,�������ж�
  * @param  
          @arg pDate ��ʽ "2017-10-17 5" ,����8 ��20171017,������
          @arg pTime ��ʽ "20:01:50" ,
  * @retval None
  * note 
  */
void RTC_SetTime(uint8_t *pDate,uint8_t* pTime)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  RTC_AlarmTypeDef sAlarm;  
  // if((strlen((char*)pDate)!=8)  || strlen((char*)pTime)!=6) return;
  sTime.Hours = ((pTime[0]-'0')<<4)+((pTime[1]-'0')&0x0F);
  sTime.Minutes =((pTime[3]-'0')<<4)+((pTime[4]-'0')&0x0F);
  sTime.Seconds = ((pTime[6]-'0')<<4)+((pTime[7]-'0')&0x0F);
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  sDate.Year = ((pDate[2]-'0')<<4)+((pDate[3]-'0')&0x0F);
  sDate.Month = ((pDate[5]-'0')<<4)+((pDate[6]-'0')&0x0F);
  sDate.Date = ((pDate[8]-'0')<<4)+((pDate[9]-'0')&0x0F);
  sDate.WeekDay =(pDate[11]-'0')&0x07;
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }    
  
    /**Enable the Alarm A 
     *�������ж�,HAL_RTC_SetTime()�����ȵ��ù�
    */
  sAlarm.AlarmMask =RTC_ALARMMASK_ALL; 
   //500ms ��ss14_7  
	sAlarm.AlarmSubSecondMask =RTC_ALARMSUBSECONDMASK_SS14_7;// RTC_ALARMSUBSECONDMASK_NONE; //���ж�ͨ��������ʵ��
  sAlarm.Alarm = RTC_ALARM_A;
  HAL_RTC_DeactivateAlarm(&hrtc,sAlarm.Alarm);
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  } 
}


/**
  * @brief  Display the current time.
  * @param  showtime : pointer to buffer
            dateStr��ʽ "2017-10-17 5" ,����8 ��20171017,������
            timeStr��ʽ "20:01:50" ,
  * @retval None
  */
void RTC_GetDateTime(void)
{
	//uint8_t showtime[30]={0};
	static uint8_t show=0;
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date �����ȶ�Time�ٶ�date�����Ĵ���*/  
  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);  
 	if(show)
	{
		show= 0;
		sprintf((char*)timeStr,"%02d:%02d:%02d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);   
	}
	else
	{
		sprintf((char*)timeStr,"%02d %02d:%02d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds);   
		show =1;
	}
  
 // sprintf((char*)showtime,"%02d:%02d:%02d-%d",stimestructureget.Hours, stimestructureget.Minutes,stimestructureget.Seconds,stimestructureget.SubSeconds);
 // Usart_Printf(&huart1,showtime );
	// DisplayString5X7(0,42,showtime,strlen((char*)showtime),0);
  sprintf((char*)dateStr,"20%02d-%02d-%02d %d",sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date,sdatestructureget.WeekDay );
  //Usart_Printf(&huart1,showtime);  
  // DisplayString5X7(0,42,showtime,strlen((char*)showtime),1);
  DisplayTime();
}



/**
  * @brief  Alarm A callback.�����жϳ���
  * @param  hrtc: RTC handle
  * @retval None
  */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{ 
	RTC_GetDateTime();
//  u8Second =(stimestructureget.Seconds+1);
//  if(u8Second > 59) u8Second=0;
//  
//  sAlarm.AlarmTime.Seconds = u8Second;  
//  
//  sAlarm.AlarmMask &= ~RTC_ALARMMASK_SECONDS;  //RTC_ALARMMASK_SECONDS
//  sAlarm.Alarm = RTC_ALARM_A;
//  HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_A);
//  if (HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }

//  if (HAL_RTC_SetAlarm(hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RTC_AlarmAEventCallback could be implemented in the user file
   */
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
