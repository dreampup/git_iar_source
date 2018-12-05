/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "main.h"

/* USER CODE BEGIN Includes */
#define RX_TEMP_LENGTH1  50       /*!< UART Rx Transfer size   >0  <0X10000       */   
#define RX_TEMP_LENGTH2  50       /*!< UART Rx Transfer size   这个BUF设计小点，大了会出现最后一个字节丢失 <100    */  
   
#define RX_MAX_LENGTH1  200 //每次接收的最长字符数
#define RX_MAX_LENGTH2  640 //每次接收的最长字符数  

typedef struct
{  
  uint16_t  startPos;     //循环buf的起始指针
  uint16_t  endPos;       //循环buf的结束指针 
  uint8_t   posDir;       //posDir为循环方向 0:startPos与endPos指针同向 1:反向
  uint8_t	nReceiveOver; //接收完成标志
  uint16_t  maxLen;  
  uint8_t	*pReceiveTotal;  
}UART_RecBuffTypeDef;
/* USER CODE END Includes */
extern UART_HandleTypeDef huart1,huart2;
extern UART_RecBuffTypeDef recBuff1,recBuff2;
extern uint8_t circleMod ;
/* USER CODE BEGIN Private defines */
// typedef struct
// {  
  // uint16_t    iReceiveSize;       /*!< UART Tx Transfer size 最高位为接收标志,因此接收长度<0x7fff  */
  // uint8_t     *pRecBuffPtr;      /*!< Pointer to UART Tx transfer Buffer */
// }UART_RecBuffTypeDef;


void MX_USART1_UART_Init(uint32_t bandRate);
void MX_USART2_UART_Init(uint32_t bandRate);

/* USER CODE BEGIN Prototypes */
void CircleStorage(UART_RecBuffTypeDef *p, uint8_t* saveBuf, uint16_t len);
uint16_t Usart_Scanf(UART_RecBuffTypeDef *p,uint8_t **buffer,uint16_t overtime);
void FreeScanBuf(unsigned char **ppBuf);
void ClearReceiveDef(UART_RecBuffTypeDef *p);

void Usart_Printf_Len(UART_HandleTypeDef* uartHandle,uint8_t *buffer,uint16_t len);
void Usart_Printf(UART_HandleTypeDef* uartHandle,uint8_t *fmt, ...);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
