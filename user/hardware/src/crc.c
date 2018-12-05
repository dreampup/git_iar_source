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



/* flashת¼������CRC32У�麯��
//����ʽ:0x04C11DB7 
//��ֵ:  0xFFFFFFFF
//���ֵ: 0x00000000 
---��flash��һ����ַ���������ת�Ƶ���һ������ָ���ص�
---ǰ��,Ŀ�ĵ�ַ����Ҫ�Ȳ���
 ����˵����
    crcString:CRC32�ַ���
	dAddr��Ŀ�ĵ�ַ
	sAddr��Դ��ַ
	fsize��ת�Ƴ���

����value��
	    0 �����,У��ʧ��
        1 ���,У��ɹ�
*/

uint8_t CRC32CheckAndMove(uint8_t* crcString , uint32_t dAddr,uint32_t sAddr,uint32_t fsize)
{
	uint32_t temp1 = 0 ,temp2 = 0,tempLength = 0;
	uint8_t *p=NULL,len=0, i=0; 
	
	p =(uint8_t*)calloc(512,1); //�����ڴ沢��ʼ��Ϊ0
	__HAL_CRC_DR_RESET(&hcrc);//��λCRC32��ʼֵ	
	for(tempLength=0; tempLength< fsize; tempLength +=512)
	{
		W25xFLASH_ReadBuffer(sAddr +tempLength, p, 512);		
		W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, p, 512);	
		HAL_CRC_Accumulate(&hcrc, (uint32_t *)&p, 512);//����crc	
	}
	tempLength -=512;
	W25xFLASH_ReadBuffer(sAddr +tempLength, p, fsize-tempLength);
	W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, p, fsize-tempLength);
	temp2 =HAL_CRC_Accumulate(&hcrc, (uint32_t *)&p, fsize-tempLength);//��������crc,LSB	
	free(p);
		
	len = strlen((char*)crcString);
	p =(uint8_t*)calloc(len/2,1); //�����ڴ沢��ʼ��Ϊ0,��ʽ����Ϊ��־���ַ����ṹ,��ż��λ   
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
