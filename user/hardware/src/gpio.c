/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
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
#include "gpio.h"
/* USER CODE BEGIN 0 */
#include "delay.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure 默认IO电平 GPIO pin Output Level */
  HAL_GPIO_WritePin(WiFi_ON_GPIO_Port, WiFi_ON_Pin, GPIO_PIN_RESET); //wifi_on   拉低
  HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_RESET); //I2S  拉低 
  HAL_GPIO_WritePin(GPIOB, HT1632C_CS1_Pin|HT1632C_CS2_Pin|HT1632C_CS3_Pin|Flash_CS_Pin, GPIO_PIN_SET); //flash_cs  置高
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, HT1632C_DATA_Pin|HT1632C_WR_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, HT1632C_RD_Pin, GPIO_PIN_SET);
  /*Configure GPIO pins : PBPin PBPin PBPin PBPin 
                           PBPin PBPin */
  GPIO_InitStruct.Pin = HT1632C_DATA_Pin|HT1632C_WR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = HT1632C_RD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(HT1632C_RD_GPIO_Port, &GPIO_InitStruct);  
  
  GPIO_InitStruct.Pin = HT1632C_CS1_Pin|HT1632C_CS2_Pin|HT1632C_CS3_Pin|Flash_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /*Configure GPIO pins : PFPin PFPin */
  GPIO_InitStruct.Pin = I2S_Ctrl_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(I2S_Ctrl_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin :WIFI上电IO WIFI_ON */
  GPIO_InitStruct.Pin = WiFi_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(WiFi_ON_GPIO_Port, &GPIO_InitStruct);  
  
  /*Configure GPIO pin : 倒置开关IO */
  GPIO_InitStruct.Pin = Topple_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Topple_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : 按键IO*/
  GPIO_InitStruct.Pin = S3_BT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(S3_BT1_GPIO_Port, &GPIO_InitStruct);  
  GPIO_InitStruct.Pin = S4_BT3_Pin|S5_BT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(S4S5_GPIO_Port, &GPIO_InitStruct);  

}

/* USER CODE BEGIN 2 */

void Set_GPIO_OUTPUT_PP(GPIO_TypeDef  *GPIO_Portx, uint32_t GPIO_Pinx)
{
  GPIO_InitTypeDef GPIO_InitStruct;  
  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_Pinx;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; 
  HAL_GPIO_Init(GPIO_Portx, &GPIO_InitStruct);
}
void Set_GPIO_OUTPUT_OD(GPIO_TypeDef  *GPIO_Portx, uint32_t GPIO_Pinx)
{
  GPIO_InitTypeDef GPIO_InitStruct;  
  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_Pinx;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; 
  HAL_GPIO_Init(GPIO_Portx, &GPIO_InitStruct);
}
void Set_GPIO_INPUT_UP(GPIO_TypeDef  *GPIO_Portx, uint32_t GPIO_Pinx)
{
  GPIO_InitTypeDef GPIO_InitStruct;  
  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_Pinx;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; 
  HAL_GPIO_Init(GPIO_Portx, &GPIO_InitStruct);
}

void Set_GPIO_INPUT_DOWN(GPIO_TypeDef  *GPIO_Portx, uint32_t GPIO_Pinx)
{
  GPIO_InitTypeDef GPIO_InitStruct;  
  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPIO_Pinx;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM; 
  HAL_GPIO_Init(GPIO_Portx, &GPIO_InitStruct);
}

/*
wifi电源开关
参数：
sw : 0 关
     1 开
*/
void GPIO_WiFiSwitch(uint8_t sw)
{
  if(sw>0){
    HAL_GPIO_WritePin(WiFi_ON_GPIO_Port,WiFi_ON_Pin,GPIO_PIN_SET);    
  }
  else {
    HAL_GPIO_WritePin(WiFi_ON_GPIO_Port,WiFi_ON_Pin,GPIO_PIN_RESET);   
  }
}

/*
按键查询
返回值：
BT_NON ：无按键按下
BT_SET  : S3按键按下
BT_ADD  ：S4按键按下
BT_SUB  ：S5按键按下
*/
ButtonState GPIO_ButtonDect(void)
{
  ButtonState bt = BT_NON;
  if(HAL_GPIO_ReadPin(S3_BT1_GPIO_Port,S3_BT1_Pin)== GPIO_PIN_RESET ){
    delay_ms(15);
    if(HAL_GPIO_ReadPin(S3_BT1_GPIO_Port,S3_BT1_Pin)== GPIO_PIN_RESET )  bt = BT_SET;
  }
  else if(HAL_GPIO_ReadPin(S4S5_GPIO_Port,S4_BT3_Pin)== GPIO_PIN_RESET ){
    delay_ms(15);
    if(HAL_GPIO_ReadPin(S4S5_GPIO_Port,S4_BT3_Pin)== GPIO_PIN_RESET )  bt = BT_ADD;
  }
  else if(HAL_GPIO_ReadPin(S4S5_GPIO_Port,S5_BT2_Pin)== GPIO_PIN_RESET ){
    delay_ms(15);
    if(HAL_GPIO_ReadPin(S4S5_GPIO_Port,S5_BT2_Pin)== GPIO_PIN_RESET )  bt = BT_SUB;
  }        
       
   return  bt;   
}    

/* USER CODE END 2 */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
