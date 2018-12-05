/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define TIM_1M 48
#define CONNECT_FRQ 8

#define Uart1_txd_Pin GPIO_PIN_9
#define Uart1_txd_GPIO_Port GPIOA
#define Uart1_rxd_Pin GPIO_PIN_10
#define Uart1_rxd_GPIO_Port GPIOA
//--I2S
#define I2S_Ctrl_Pin GPIO_PIN_7
#define I2S_Ctrl_GPIO_Port GPIOF
#define I2S1_WS_Pin GPIO_PIN_15
#define I2S1_WS_GPIO_Port GPIOA
#define I2S1_CK_Pin GPIO_PIN_3
#define I2S1_CK_GPIO_Port GPIOB
#define I2S1_SD_Pin GPIO_PIN_5
#define I2S1_SD_GPIO_Port GPIOB
//--DHT12
#define DHT12_SCL_Pin GPIO_PIN_6
#define DHT12_SCL_GPIO_Port GPIOB
#define DHT12_DAT_Pin GPIO_PIN_7
#define DHT12_DAT_GPIO_Port GPIOB

//--FLASH
#define Flash_CS_Pin GPIO_PIN_12
#define Flash_CS_GPIO_Port GPIOB
#define Flash_SCK_Pin GPIO_PIN_13
#define Flash_SCK_GPIO_Port GPIOB
#define Flash_MISO_Pin GPIO_PIN_14
#define Flash_MISO_GPIO_Port GPIOB
#define Flash_MOSI_Pin GPIO_PIN_15
#define Flash_MOSI_GPIO_Port GPIOB
//HT1632C
#define HT1632C_DATA_Pin GPIO_PIN_6
#define HT1632C_WR_Pin GPIO_PIN_7
#define HT1632C_RD_Pin GPIO_PIN_0
#define HT1632C_CS1_Pin GPIO_PIN_1
#define HT1632C_CS2_Pin GPIO_PIN_2
#define HT1632C_CS3_Pin GPIO_PIN_10

#define HT1632C_DATA_GPIO_Port GPIOA
#define HT1632C_WR_GPIO_Port GPIOA
#define HT1632C_RD_GPIO_Port GPIOB
#define HT1632C_CS1_GPIO_Port GPIOB
#define HT1632C_CS2_GPIO_Port GPIOB
#define HT1632C_CS3_GPIO_Port GPIOB
//--WIFI
#define WiFi_ON_Pin GPIO_PIN_1
#define WiFi_ON_GPIO_Port GPIOA
#define WiFi_rxd_Pin GPIO_PIN_2
#define WiFi_rxd_GPIO_Port GPIOA
#define WiFi_txd_Pin GPIO_PIN_3
#define WiFi_txd_GPIO_Port GPIOA
//光照传感器
#define Light_ADC_Pin GPIO_PIN_5
#define Light_ADC_GPIO_Port GPIOA
//--倒置开关
#define Topple_Pin GPIO_PIN_4
#define Topple_GPIO_Port GPIOA
//--按键
#define S3_BT1_Pin GPIO_PIN_13
#define S4_BT3_Pin GPIO_PIN_8
#define S5_BT2_Pin GPIO_PIN_9

#define S3_BT1_GPIO_Port GPIOC
#define S4S5_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
