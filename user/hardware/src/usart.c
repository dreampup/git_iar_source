/* Includes ------------------------------------------------------------------*/
#include "usart.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "delay.h"
//#define _DEBUG_PRINT �������Լ�����


/**
  * @brief  UART handle Structure definition
  */

uint8_t pReceiveTemp1[RX_TEMP_LENGTH1]= {0}; //����buf�����ַ
uint8_t pReceiveTemp2[RX_TEMP_LENGTH2]= {0}; //����buf�����ַ
//------ѭ��buf���-----------
uint8_t circleMod =0; //�Ƿ񸲸Ǵ洢 0�������Ǵ���(���յ����ݳ���ʣ����������) ,1:���Ǵ���(��ʼָ��=����ָ�벢����λ��ȫ���洢)
uint8_t pReceiveTotal1[RX_MAX_LENGTH1]= {0},pReceiveTotal2[RX_MAX_LENGTH2]= {0}; //ѭ��buf
  
UART_RecBuffTypeDef recBuff1 ={0,0,0,0,RX_MAX_LENGTH1,pReceiveTotal1}; 
UART_RecBuffTypeDef recBuff2 ={0,0,0,0,RX_MAX_LENGTH2,pReceiveTotal2}; 

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;



/* USART1 init function */

void MX_USART1_UART_Init(uint32_t bandRate)
{

	huart1.Instance = USART1;
	huart1.Init.BaudRate =  bandRate;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}

}
/* USART2 init function */

void MX_USART2_UART_Init(uint32_t bandRate)
{

	huart2.Instance = USART2;
	huart2.Init.BaudRate = bandRate;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if(uartHandle->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();
		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		GPIO_InitStruct.Pin = Uart1_txd_Pin|Uart1_rxd_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Peripheral interrupt init */
		HAL_NVIC_EnableIRQ(USART1_IRQn);
		/* USER CODE BEGIN USART1_MspInit 1 */
		//����Ҫ��ʼ��������ֱ��relloc
//    recBuff1.iReceiveSize=0;
//    recBuff1.pRecBuffPtr=(uint8_t*)malloc(sizeof(char));/*1.�ṹ���Աָ����Ҫ��ʼ��*/

		__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);  //ʹ�ܿ����ж�
		__HAL_UART_CLEAR_FLAG(uartHandle, UART_CLEAR_IDLEF);
		/* USER CODE END USART1_MspInit 1 */
	}
	else if(uartHandle->Instance==USART2)
	{
		/* USER CODE BEGIN USART2_MspInit 0 */

		/* USER CODE END USART2_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();

		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		GPIO_InitStruct.Pin = WiFi_rxd_Pin|WiFi_txd_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Peripheral DMA init*/

		hdma_usart2_tx.Instance = DMA1_Channel4;
		hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart2_tx.Init.Mode = DMA_NORMAL;
		hdma_usart2_tx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

		hdma_usart2_rx.Instance = DMA1_Channel5;
		hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_usart2_rx.Init.Mode = DMA_NORMAL;
		hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
		if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

		/* Peripheral interrupt init */
		HAL_NVIC_EnableIRQ(USART2_IRQn); //��Ϊʹ���˿����ж�
		/* USER CODE BEGIN USART2_MspInit 1 */
		//����Ҫ��ʼ��������ֱ��relloc
//    recBuff2.iReceiveSize=0;
//    recBuff2.pRecBuffPtr=(uint8_t*)malloc(sizeof(char));/*1.�ṹ���Աָ����Ҫ��ʼ��*/
		__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);  //ʹ�ܿ����ж�
		__HAL_UART_CLEAR_FLAG(uartHandle, UART_CLEAR_IDLEF);
		/* USER CODE END USART2_MspInit 1 */
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

	if(uartHandle->Instance==USART1)
	{
		/* USER CODE BEGIN USART1_MspDeInit 0 */

		/* USER CODE END USART1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART1_CLK_DISABLE();

		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		HAL_GPIO_DeInit(GPIOA, Uart1_txd_Pin|Uart1_rxd_Pin);

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(USART1_IRQn);

		/* USER CODE BEGIN USART1_MspDeInit 1 */
		__HAL_UART_DISABLE_IT(uartHandle, UART_IT_IDLE);  //�رտ����ж�
		/* USER CODE END USART1_MspDeInit 1 */
	}
	else if(uartHandle->Instance==USART2)
	{
		/* USER CODE BEGIN USART2_MspDeInit 0 */

		/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		HAL_GPIO_DeInit(GPIOA, WiFi_rxd_Pin|WiFi_txd_Pin);

		/* Peripheral DMA DeInit*/
		HAL_DMA_DeInit(uartHandle->hdmatx);
		HAL_DMA_DeInit(uartHandle->hdmarx);

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(USART2_IRQn);

		/* USER CODE BEGIN USART2_MspDeInit 1 */
		__HAL_UART_DISABLE_IT(uartHandle, UART_IT_IDLE);  //�رտ����ж�
		/* USER CODE END USART2_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/*--ѭ���洢����
	* ����˵��:
	* who: 
	     0:huart1
		 1:huart2
	* saveBuf:�����������
	* len: ����saveBuf�ĳ���
		��circleMod:
	     0: ѭ��buf�п���λ��<len ���������
		 1: ���򸲸�ʽ���棬������ѭ��buf�в�������

	*/
void CircleStorage(UART_RecBuffTypeDef *p, uint8_t* saveBuf, uint16_t len)
{
	uint16_t i, j, k;	
	
	if (p->posDir > 0) //����״̬ ��ʱ p->endPos<=startPos
	{
		if (p->endPos + len <= p->startPos)
		{
			for (i = 0; i < len; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
		}
		else if( circleMod > 0 )//������ֽڸ��Ǵ���
		{
			j = p->startPos - p->endPos; //����״̬j>=0
			for (i = 0; i < j; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
			//--��ʱp->endPos == startPos
			for (; i < len; i++)  //ʣ���ֽ�
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				if (p->endPos >= p->maxLen)//--ʣ���ֽڳ���buf����
					p->endPos = 0;
			}
			p->startPos = p->endPos;
		}
	}
	else //ͬ���� ��ʱ *startPos<=p->endPos
	{
		if (p->endPos + len < p->maxLen)
		{
			for (i = 0; i < len; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
		}
		else //��ȻҪ����
		{
			j = p->maxLen - p->endPos; //ʣ�����λ���ֽ�
			k = len - j; //ʣ���ֽ�
			if (p->startPos >= k)  //�п�λ��װ������ָ�ȷ��ָ��������ԣ�startPos>=0
			{
				for (i = 0; i < j; i++)
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
				p->endPos = 0;
				p->posDir = 1; //���� ���ѭ������
				for (; i < len; i++)  //ʣ���ֽ�
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
			}
			else if (circleMod > 0)  //װ�������������circleMod������ѭ������
			{
				for (i = 0; i < j; i++)
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
				p->endPos = 0;
				p->posDir = 1; //���� ���ѭ������
				for (; i < len; i++)  //ʣ���ֽ�
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
					if (p->endPos >= p->maxLen)//--ʣ���ֽڳ���buf����
						p->endPos = 0;
				}
				p->startPos = p->endPos;
			}
		}
	}
}


/**
  * @brief Usart_Scanf���ڽ��պ���,
  * @param
    huart: UART handle.
          **buffer,����ָ�룬�����յ������ݳ������Զ�����ռ�
          voertime,��ʱ���ã���λms
  * @retval ��������ĳ��� >0��Ч
  * @note ����buffer����Ϊָ���ͣ������Զ�̬����ռ�
  */
uint16_t Usart_Scanf(UART_RecBuffTypeDef *p,uint8_t **buffer,uint16_t overtime)
{
	uint16_t timeN=0;	
	uint16_t i, j, len=0;
	uint16_t  tempEndPos;       //ѭ��buf�Ľ���ָ�� 
	
  if(overtime > 0)  
  {
    while(timeN<overtime)
    {
      if(p->nReceiveOver >0 )  //���ܵ�һ���ַ�
      {
        break;
      }
      HAL_Delay(1);
      timeN++;
    }
    if((timeN > overtime-1) && (p->nReceiveOver ==0 )) // ������ʱ���ٴ�ȷ����û���յ�����
    {      
      return 0;//û�յ����ݣ���ʱ�˳�	
    }
  }
  else  //����ʱ
  {
    if(p->nReceiveOver ==0 ) return 0;
  }
  //--������
	p->nReceiveOver =0;

	tempEndPos = p->endPos; //��ֹ�����в����ж��޸�endPos��ֵ
	
	len = tempEndPos > p->startPos ? (tempEndPos - p->startPos) : ( p->posDir>0?  (p->maxLen - p->startPos + tempEndPos):0);
	if(len == 0) return 0;
	*buffer = (uint8_t *)malloc(len+1);

	if (tempEndPos > p->startPos)
	{
		for (i = 0; i < len; i++)
		{
			(*buffer)[i] = (p->pReceiveTotal)[(p->startPos)++];
		}
		(*buffer)[i]  =0;
	}
	else if (p->posDir > 0)	//������� ��tempEndPos <= p->startPos ���˵�tempEndPos==p->startPos�����
	{
		j = p->maxLen - p->startPos;
		for (i = 0; i < j; i++)
		{
			(*buffer)[i] = (p->pReceiveTotal)[(p->startPos)++];
		}
		p->startPos = 0;
		p->posDir = 0;
		for (; i < len; i++)
		{
			(*buffer)[i] = (p->pReceiveTotal)[(p->startPos)++];
		}
		(*buffer)[i] =0;
	}
#ifdef _DEBUG_PRINT
	Usart_Printf_Len(&huart1,"-->",3);
	Usart_Printf_Len(&huart1,*buffer,len);
#endif
	return len;
}

/****************************************************************************
* ��    �ƣ�Usart_Printf_Len
* ��    �ܣ��������򴮿ڴ�ӡ����
* ��ڲ����� uartHandle:huart1,huart2
            len :��ӡ���� <256
            buffer :��ӡ����
* ���ڲ�������
* ˵    ����
* ���÷�������
****************************************************************************/

//void Usart2_DMA_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart2.gState != HAL_UART_STATE_READY) ;//����ϴλ���Ϊ������ɵ���ȴ��������
//	HAL_UART_Transmit_DMA(&huart2,buffer,len);
//}
//void Usart1_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart1.gState != HAL_UART_STATE_READY) ;//����ϴλ���Ϊ������ɵ���ȴ��������
//	HAL_UART_Transmit(&huart1,buffer,len,100);
//}
//
//void Usart1_Printf(uint8_t *fmt, ...)
//{
//#ifdef _DEBUG_PRINT
//	char buffer[151]= {0}; //���bufС�˻�������󼸸��ַ���������
//	uint8_t len;
//	va_list arg_ptr; //Define convert parameters variable
//	va_start(arg_ptr, fmt); //Init variable ָ��ָ��fmt��ĵ�һ������
//	len = vsnprintf(buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
//	if(len > 0)
//		Usart1_Printf_Len((uint8_t *)buffer,len);
//	va_end(arg_ptr);
//#endif
//}

//void DMA_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart2.gState != HAL_UART_STATE_READY) ;//����ϴλ���Ϊ������ɵ���ȴ��������
//	HAL_UART_Transmit_DMA(&huart2,buffer,len);
//}
void Usart_Printf_Len(UART_HandleTypeDef* uartHandle, uint8_t *buffer,uint16_t len)
{
	while (uartHandle->gState != HAL_UART_STATE_READY) ;//����ϴλ���Ϊ������ɵ���ȴ��������
	if(uartHandle->Instance == USART1)
	{
		HAL_UART_Transmit(uartHandle,buffer,len,100);//���������ʱʱ�䲻��С��20ms���������ݶ��˻ᵼ��ֻ������һ��
	}
	else if(uartHandle->Instance == USART2)
	{
		HAL_UART_Transmit_DMA(uartHandle,buffer,len);
	}	
}

void Usart_Printf(UART_HandleTypeDef* uartHandle,uint8_t *fmt, ...)
{
#ifdef _DEBUG_PRINT
	char buffer[121]= {0}; //���bufС�˻�������󼸸��ַ���������
	uint8_t len;
	va_list arg_ptr; //Define convert parameters variable
	va_start(arg_ptr, fmt); //Init variable ָ��ָ��fmt��ĵ�һ������
	len = vsnprintf(buffer, 120,(char*) fmt, arg_ptr); //parameters list format to buffer
	va_end(arg_ptr);
	if(len > 0)
	{
		Usart_Printf_Len(uartHandle,(uint8_t *)buffer,len);
	}
	HAL_Delay(20); 
#endif
}

/*
*��Զ���ָ����ͷţ�Usart_Scanf
*/
void FreeScanBuf(unsigned char **ppBuf)
{
	if (NULL != (*ppBuf))
	{
		free(*ppBuf);
		(*ppBuf) = NULL;
		ppBuf = NULL;
	}
}

/*
	���㴮�ڽ���buf�ṹ��
*/
void ClearReceiveDef(UART_RecBuffTypeDef *p)
{
	p->startPos = 0;     //ѭ��buf����ʼָ��
	p->endPos = 0;       //ѭ��buf�Ľ���ָ�� 
	p->posDir = 0;       //posDirΪѭ������ 0:startPos��endPosָ��ͬ�� 1:����
	p->nReceiveOver = 0; //������ɱ�־
}

/*----------------------�жϵ��ú���----------------------------------------*/
/**
  * @brief Handle UART interrupt request.���յ�ָ�����ȵ������жϺ���
  * @param huart: UART handle.
  * @retval �жϺ���UART_Receive_IT()��  huart->RxXferCount=0;����
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UART_RecBuffTypeDef *p=NULL;
//--�����жϽ��գ�����HAL_UART_Receive_IT()��������յ�ָ�����ȵ����ݺ���øú���
	if(huart->Instance == USART1) //���Զ��رս����ж�
	{
		p = &recBuff1;
		CircleStorage(p,pReceiveTemp1,RX_TEMP_LENGTH1);	
		p->nReceiveOver =0; //��������жϱ�־
		HAL_UART_Receive_IT(huart,pReceiveTemp1,RX_TEMP_LENGTH1);  //��������
	}
//---DMA�жϽ��գ�����HAL_UART_Receive_DMA()��������յ�ָ�����ȵ����ݺ���øú���
	else if(huart->Instance == USART2)
	{		
		p = &recBuff2;
		HAL_UART_DMAStop(huart); //������õ�Ϊѭ������ģʽ������Ҫ�ֶ��رս����ж�	
		CircleStorage(p,pReceiveTemp2,RX_TEMP_LENGTH2);	
		p->nReceiveOver =0; //��������жϱ�־
		HAL_UART_Receive_DMA(huart,pReceiveTemp2,RX_TEMP_LENGTH2);
	}
}
/**
  * @brief Handle UART interrupt request.���ڿ����жϻ����
  * @param huart: UART handle.//���������жϺ�,ϵͳ���һ���ж�
  * @retval None
  */
void My_HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
	uint16_t len;
	UART_RecBuffTypeDef *p=NULL;
	/* UART in mode Transmitter (transmission end) -----------------------------*/
	if((__HAL_UART_GET_IT(huart, UART_IT_IDLE) != RESET) &&(__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET) )
	{
		/*
		1���ж�ģʽ��huart->RxXferCountΪ�ݼ�����,��UART_Receive_IT()����
			��ʼֵhuart->RxXferCount=huart->RxXferSize,�յ�1����RxXferCount-1
			len=huart->RxXferSize - huart->RxXferCount;Ϊ�յ������ݸ���
		2��DMAģʽ��__HAL_DMA_GET_COUNTER(huart->hdmarx)ҲΪ�ݼ�ֵ,�õ���ֵ��ʾʣ��δ��������ݼ���,����ֵΪ0ʱDMAֹͣ����
			len=huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
		3��len=0�����
		   1)��λʱ�Ļ���ֵ�һ�������жϣ��޿ɱ��⣬��λֵ__HAL_DMA_GET_COUNTER(huart->hdmarx)=0��
		   2)DMA�жϽ��յ�����huart->RxXferSize����,�Ҹպ�������Ϊ��������ʱp->iReceiveSize=0x4000

		   */
		if(huart->Instance == USART1 )//������õ�Ϊѭ������ģʽ������Ҫ�ֶ��رս����ж�
		{
			p = &recBuff1;
			len =huart->RxXferSize - huart->RxXferCount;
			if(len > 0 )
			{			
				CircleStorage(p,pReceiveTemp1,len);
			}
			p->nReceiveOver =1; //�����ж���ɱ�־
			huart->RxState = HAL_UART_STATE_READY; // ������λ
			HAL_UART_Receive_IT(huart,pReceiveTemp1,RX_TEMP_LENGTH1);  //���ÿ�����ĵ�һ�������ж������������ж�
		}
		else if(huart->Instance == USART2 )
		{
			p = &recBuff2;
			HAL_UART_DMAStop(huart); //������õ�Ϊѭ������ģʽ������Ҫ�ֶ��رս����ж�
			len =huart->RxXferSize-__HAL_DMA_GET_COUNTER(huart->hdmarx);
			if(len > 0 )
			{			
				CircleStorage(p,pReceiveTemp2,len);
			}
			p->nReceiveOver =1; //�����ж���ɱ�־
			huart->RxState = HAL_UART_STATE_READY; // ������λ
			HAL_UART_Receive_DMA(huart,pReceiveTemp2,RX_TEMP_LENGTH2);//���ÿ�����ĵ�һ�������ж������������ж�
		}
		/* Clear the idle flag in the ICR register */
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_IDLEF);
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
