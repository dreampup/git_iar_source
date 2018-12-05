/* Includes ------------------------------------------------------------------*/
#include "usart.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "delay.h"
//#define _DEBUG_PRINT 工程中以及定义


/**
  * @brief  UART handle Structure definition
  */

uint8_t pReceiveTemp1[RX_TEMP_LENGTH1]= {0}; //接收buf缓存地址
uint8_t pReceiveTemp2[RX_TEMP_LENGTH2]= {0}; //接收buf缓存地址
//------循环buf设计-----------
uint8_t circleMod =0; //是否覆盖存储 0：不覆盖处理(接收的内容超过剩余容量丢弃) ,1:覆盖处理(起始指针=结束指针并会移位到全部存储)
uint8_t pReceiveTotal1[RX_MAX_LENGTH1]= {0},pReceiveTotal2[RX_MAX_LENGTH2]= {0}; //循环buf
  
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
		//不需要初始化，可以直接relloc
//    recBuff1.iReceiveSize=0;
//    recBuff1.pRecBuffPtr=(uint8_t*)malloc(sizeof(char));/*1.结构体成员指针需要初始化*/

		__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);  //使能空闲中断
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
		HAL_NVIC_EnableIRQ(USART2_IRQn); //因为使能了空闲中断
		/* USER CODE BEGIN USART2_MspInit 1 */
		//不需要初始化，可以直接relloc
//    recBuff2.iReceiveSize=0;
//    recBuff2.pRecBuffPtr=(uint8_t*)malloc(sizeof(char));/*1.结构体成员指针需要初始化*/
		__HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE);  //使能空闲中断
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
		__HAL_UART_DISABLE_IT(uartHandle, UART_IT_IDLE);  //关闭空闲中断
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
		__HAL_UART_DISABLE_IT(uartHandle, UART_IT_IDLE);  //关闭空闲中断
		/* USER CODE END USART2_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/*--循环存储函数
	* 参数说明:
	* who: 
	     0:huart1
		 1:huart2
	* saveBuf:被保存的数组
	* len: 数组saveBuf的长度
		当circleMod:
	     0: 循环buf中空闲位置<len 则放弃保存
		 1: 否则覆盖式保存，将覆盖循环buf中部分内容

	*/
void CircleStorage(UART_RecBuffTypeDef *p, uint8_t* saveBuf, uint16_t len)
{
	uint16_t i, j, k;	
	
	if (p->posDir > 0) //反向状态 此时 p->endPos<=startPos
	{
		if (p->endPos + len <= p->startPos)
		{
			for (i = 0; i < len; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
		}
		else if( circleMod > 0 )//多余的字节覆盖处理
		{
			j = p->startPos - p->endPos; //反向状态j>=0
			for (i = 0; i < j; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
			//--此时p->endPos == startPos
			for (; i < len; i++)  //剩余字节
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				if (p->endPos >= p->maxLen)//--剩余字节超出buf长度
					p->endPos = 0;
			}
			p->startPos = p->endPos;
		}
	}
	else //同方向 此时 *startPos<=p->endPos
	{
		if (p->endPos + len < p->maxLen)
		{
			for (i = 0; i < len; i++)
			{
				(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
			}
		}
		else //必然要反向
		{
			j = p->maxLen - p->endPos; //剩余空闲位置字节
			k = len - j; //剩余字节
			if (p->startPos >= k)  //有空位子装下整个指令，确保指令的完整性，startPos>=0
			{
				for (i = 0; i < j; i++)
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
				p->endPos = 0;
				p->posDir = 1; //换向 如果循环处理
				for (; i < len; i++)  //剩余字节
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
			}
			else if (circleMod > 0)  //装不下如果定义了circleMod则启用循环覆盖
			{
				for (i = 0; i < j; i++)
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
				}
				p->endPos = 0;
				p->posDir = 1; //换向 如果循环处理
				for (; i < len; i++)  //剩余字节
				{
					(p->pReceiveTotal)[(p->endPos)++] = saveBuf[i];
					if (p->endPos >= p->maxLen)//--剩余字节超出buf长度
						p->endPos = 0;
				}
				p->startPos = p->endPos;
			}
		}
	}
}


/**
  * @brief Usart_Scanf串口接收函数,
  * @param
    huart: UART handle.
          **buffer,数组指针，根据收到的数据长度来自动申请空间
          voertime,超时设置，单位ms
  * @retval 返回数组的长度 >0有效
  * @note 参数buffer必须为指针型，即可以动态分配空间
  */
uint16_t Usart_Scanf(UART_RecBuffTypeDef *p,uint8_t **buffer,uint16_t overtime)
{
	uint16_t timeN=0;	
	uint16_t i, j, len=0;
	uint16_t  tempEndPos;       //循环buf的结束指针 
	
  if(overtime > 0)  
  {
    while(timeN<overtime)
    {
      if(p->nReceiveOver >0 )  //接受到一段字符
      {
        break;
      }
      HAL_Delay(1);
      timeN++;
    }
    if((timeN > overtime-1) && (p->nReceiveOver ==0 )) // 经过延时后再次确认有没有收到数据
    {      
      return 0;//没收到数据，超时退出	
    }
  }
  else  //不延时
  {
    if(p->nReceiveOver ==0 ) return 0;
  }
  //--有数据
	p->nReceiveOver =0;

	tempEndPos = p->endPos; //防止过程中产生中断修改endPos的值
	
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
	else if (p->posDir > 0)	//反向情况 ：tempEndPos <= p->startPos 过滤掉tempEndPos==p->startPos的情况
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
* 名    称：Usart_Printf_Len
* 功    能：按长度向串口打印数据
* 入口参数： uartHandle:huart1,huart2
            len :打印长度 <256
            buffer :打印内容
* 出口参数：无
* 说    明：
* 调用方法：无
****************************************************************************/

//void Usart2_DMA_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart2.gState != HAL_UART_STATE_READY) ;//如果上次还有为传输完成的则等待传输完成
//	HAL_UART_Transmit_DMA(&huart2,buffer,len);
//}
//void Usart1_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart1.gState != HAL_UART_STATE_READY) ;//如果上次还有为传输完成的则等待传输完成
//	HAL_UART_Transmit(&huart1,buffer,len,100);
//}
//
//void Usart1_Printf(uint8_t *fmt, ...)
//{
//#ifdef _DEBUG_PRINT
//	char buffer[151]= {0}; //这个buf小了会引起最后几个字符出现乱码
//	uint8_t len;
//	va_list arg_ptr; //Define convert parameters variable
//	va_start(arg_ptr, fmt); //Init variable 指针指向fmt后的第一个参数
//	len = vsnprintf(buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
//	if(len > 0)
//		Usart1_Printf_Len((uint8_t *)buffer,len);
//	va_end(arg_ptr);
//#endif
//}

//void DMA_Printf_Len(uint8_t *buffer,uint16_t len)
//{
//	while (huart2.gState != HAL_UART_STATE_READY) ;//如果上次还有为传输完成的则等待传输完成
//	HAL_UART_Transmit_DMA(&huart2,buffer,len);
//}
void Usart_Printf_Len(UART_HandleTypeDef* uartHandle, uint8_t *buffer,uint16_t len)
{
	while (uartHandle->gState != HAL_UART_STATE_READY) ;//如果上次还有为传输完成的则等待传输完成
	if(uartHandle->Instance == USART1)
	{
		HAL_UART_Transmit(uartHandle,buffer,len,100);//这个发送延时时间不能小于20ms，否则数据多了会导致只发送了一半
	}
	else if(uartHandle->Instance == USART2)
	{
		HAL_UART_Transmit_DMA(uartHandle,buffer,len);
	}	
}

void Usart_Printf(UART_HandleTypeDef* uartHandle,uint8_t *fmt, ...)
{
#ifdef _DEBUG_PRINT
	char buffer[121]= {0}; //这个buf小了会引起最后几个字符出现乱码
	uint8_t len;
	va_list arg_ptr; //Define convert parameters variable
	va_start(arg_ptr, fmt); //Init variable 指针指向fmt后的第一个参数
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
*针对二级指针的释放，Usart_Scanf
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
	清零串口接收buf结构体
*/
void ClearReceiveDef(UART_RecBuffTypeDef *p)
{
	p->startPos = 0;     //循环buf的起始指针
	p->endPos = 0;       //循环buf的结束指针 
	p->posDir = 0;       //posDir为循环方向 0:startPos与endPos指针同向 1:反向
	p->nReceiveOver = 0; //接收完成标志
}

/*----------------------中断调用函数----------------------------------------*/
/**
  * @brief Handle UART interrupt request.接收到指定长度的数据中断函数
  * @param huart: UART handle.
  * @retval 中断函数UART_Receive_IT()中  huart->RxXferCount=0;触发
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UART_RecBuffTypeDef *p=NULL;
//--串口中断接收：调用HAL_UART_Receive_IT()函数后接收到指定长度的内容后调用该函数
	if(huart->Instance == USART1) //会自动关闭接收中断
	{
		p = &recBuff1;
		CircleStorage(p,pReceiveTemp1,RX_TEMP_LENGTH1);	
		p->nReceiveOver =0; //清零接收中断标志
		HAL_UART_Receive_IT(huart,pReceiveTemp1,RX_TEMP_LENGTH1);  //继续接收
	}
//---DMA中断接收：调用HAL_UART_Receive_DMA()函数后接收到指定长度的内容后调用该函数
	else if(huart->Instance == USART2)
	{		
		p = &recBuff2;
		HAL_UART_DMAStop(huart); //如果设置的为循环接收模式，则需要手动关闭接收中断	
		CircleStorage(p,pReceiveTemp2,RX_TEMP_LENGTH2);	
		p->nReceiveOver =0; //清零接收中断标志
		HAL_UART_Receive_DMA(huart,pReceiveTemp2,RX_TEMP_LENGTH2);
	}
}
/**
  * @brief Handle UART interrupt request.串口空闲中断会调用
  * @param huart: UART handle.//开启空闲中断后,系统会进一次中断
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
		1、中断模式下huart->RxXferCount为递减计数,见UART_Receive_IT()函数
			初始值huart->RxXferCount=huart->RxXferSize,收到1个数RxXferCount-1
			len=huart->RxXferSize - huart->RxXferCount;为收到的数据个数
		2、DMA模式下__HAL_DMA_GET_COUNTER(huart->hdmarx)也为递减值,得到的值表示剩余未传输的数据计数,当该值为0时DMA停止传输
			len=huart->RxXferSize - __HAL_DMA_GET_COUNTER(huart->hdmarx);
		3、len=0情况：
		   1)复位时的会出现第一条空闲中断，无可避免，复位值__HAL_DMA_GET_COUNTER(huart->hdmarx)=0；
		   2)DMA中断接收到定长huart->RxXferSize数据,且刚好数据量为定长，此时p->iReceiveSize=0x4000

		   */
		if(huart->Instance == USART1 )//如果设置的为循环接收模式，则需要手动关闭接收中断
		{
			p = &recBuff1;
			len =huart->RxXferSize - huart->RxXferCount;
			if(len > 0 )
			{			
				CircleStorage(p,pReceiveTemp1,len);
			}
			p->nReceiveOver =1; //接收中断完成标志
			huart->RxState = HAL_UART_STATE_READY; // 重新置位
			HAL_UART_Receive_IT(huart,pReceiveTemp1,RX_TEMP_LENGTH1);  //利用开机后的第一个空闲中断来开启接收中断
		}
		else if(huart->Instance == USART2 )
		{
			p = &recBuff2;
			HAL_UART_DMAStop(huart); //如果设置的为循环接收模式，则需要手动关闭接收中断
			len =huart->RxXferSize-__HAL_DMA_GET_COUNTER(huart->hdmarx);
			if(len > 0 )
			{			
				CircleStorage(p,pReceiveTemp2,len);
			}
			p->nReceiveOver =1; //接收中断完成标志
			huart->RxState = HAL_UART_STATE_READY; // 重新置位
			HAL_UART_Receive_DMA(huart,pReceiveTemp2,RX_TEMP_LENGTH2);//利用开机后的第一个空闲中断来开启接收中断
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
