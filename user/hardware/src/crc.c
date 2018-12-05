/**
  ******************************************************************************
  * File Name          : CRC.c
  * Description        : This file provides code for the configuration
  *                      of the CRC instances.
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
#include "crc.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include "w25xflash.h"
#include "comment.h"
/* USER CODE END 0 */

CRC_HandleTypeDef hcrc;

/* CRC init function */
void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  }
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */



/* flash转录并进行CRC32校验函数
//多项式:0x04C11DB7 
//初值:  0xFFFFFFFF
//异或值: 0x00000000 
---将flash中一个地址区域的数据转移到另一个区域指定地点
---前提,目的地址区域要先擦除
 参数说明：
    crcString:CRC32字符串
	dAddr：目的地址
	sAddr：源地址
	fsize：转移长度

返回value：
	    0 不相等,校验失败
        1 相等,校验成功
*/

uint8_t CRC32CheckAndMove(uint8_t* crcString , uint32_t dAddr,uint32_t sAddr,uint32_t fsize)
{
	uint32_t temp1 = 0 ,temp2 = 0,tempLength = 0;
	uint8_t *p=NULL,len=0, i=0; 
	
	p =(uint8_t*)calloc(512,1); //分配内存并初始化为0
	__HAL_CRC_DR_RESET(&hcrc);//复位CRC32初始值	
	for(tempLength=0; tempLength< fsize; tempLength +=512)
	{
		W25xFLASH_ReadBuffer(sAddr +tempLength, p, 512);		
		W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, p, 512);	
		HAL_CRC_Accumulate(&hcrc, (uint32_t *)&p, 512);//计算crc	
	}
	tempLength -=512;
	W25xFLASH_ReadBuffer(sAddr +tempLength, p, fsize-tempLength);
	W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, p, fsize-tempLength);
	temp2 =HAL_CRC_Accumulate(&hcrc, (uint32_t *)&p, fsize-tempLength);//计算最终crc,LSB	
	free(p);
		
	len = strlen((char*)crcString);
	p =(uint8_t*)calloc(len/2,1); //分配内存并初始化为0,格式必须为标志的字符串结构,即偶数位   
	CharTo16Byte(p,crcString,len);
	len /=2;
	for( i=0; i< len ;i++){
		temp1 += p[i]<<((len-i-1)*8);
	}
	free(p);	
	   
	if(temp1 !=temp2) return 0;   	
    return 1;  
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
