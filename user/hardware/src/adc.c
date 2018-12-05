/**
  ******************************************************************************
  * File Name          : ADC.c
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
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
#include "adc.h"
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "delay.h"
#include "usart.h"
#include "display.h"



__IO uint8_t         ubSequenceCompleted = RESET; 
/* Variable containing ADC conversions results */

__IO uint16_t   aADCxConvertedValues=0,aADCxCompletedValues=0;
/* USER CODE END 0 */

ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;
/* ADC init function */
void MX_ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;//ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN; //ADC_OVR_DATA_PRESERVED
  hadc.Init.SamplingTimeCommon    = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.SamplingTime=ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  
    /**ADC GPIO Configuration    
    PA7     ------> ADC_IN7 
    */
    GPIO_InitStruct.Pin = Light_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(Light_ADC_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

    /* Peripheral DMA init*/		
		hdma_adc.Instance = DMA1_Channel1;
		hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
		hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
		hdma_adc.Init.Mode = DMA_NORMAL;//DMA_CIRCULAR;
		hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;  
		if (HAL_DMA_Init(&hdma_adc) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc);
  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  
    /**ADC GPIO Configuration    
    PA7     ------> ADC_IN7 
    */
    HAL_GPIO_DeInit(Light_ADC_GPIO_Port, Light_ADC_Pin);

  }
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

void Light_StartDMA(void)
{
  uint8_t  i;
  uint32_t aADCTempValues=0;
  
  for(i=0;i<AVERAGE_TIM;i++)
  { 
		HAL_ADC_Stop_DMA(&hadc);   
    ubSequenceCompleted=RESET;
    HAL_ADC_Start_DMA(&hadc,(uint32_t *)&aADCxConvertedValues,1);
    while(!ubSequenceCompleted);
    aADCTempValues +=aADCxConvertedValues;  //平均值   
		//
    delay_ms(2);
  }
  aADCxCompletedValues  =aADCTempValues /AVERAGE_TIM;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  ubSequenceCompleted=SET;
}
/**
  * @brief  得到adc值:
  * @param  *u16ADCValue:0~2^12(4096)
  * @retval 0:成功 0xff错误
  */
//uint8_t LightValue(uint16_t *u16ADCValue)
//{
//  HAL_ADC_Start(&hadc);
//  if(HAL_ADC_PollForConversion(&hadc, 10)!=HAL_OK)
//    return 0xff;
//  *u16ADCValue = HAL_ADC_GetValue(&hadc);
//  HAL_ADC_Stop(&hadc);
//  return 0;
//}

void LightTest(void)
{
	//--display初始化亮度为6  (CMD_PWM_DUTY7)
  static uint8_t oldValue=6,newValue = 0;

	Light_StartDMA();
  newValue = aADCxCompletedValues*16/4096;
  if(oldValue != newValue)
  {
      oldValue = newValue;
      Usart_Printf(&huart1,"Light:%d\r\n",newValue); //16级亮度
      DisplayLightAdjust(oldValue);
   }
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
