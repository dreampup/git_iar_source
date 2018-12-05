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
#define RX_TEMP_LENGTH2  50       /*!< UART Rx Transfer size   ���BUF���С�㣬���˻�������һ���ֽڶ�ʧ <100    */  
   
#define RX_MAX_LENGTH1  200 //ÿ�ν��յ���ַ���
#define RX_MAX_LENGTH2  640 //ÿ�ν��յ���ַ���  

typedef struct
{  
  uint16_t  startPos;     //ѭ��buf����ʼָ��
  uint16_t  endPos;       //ѭ��buf�Ľ���ָ�� 
  uint8_t   posDir;       //posDirΪѭ������ 0:startPos��endPosָ��ͬ�� 1:����
  uint8_t	nReceiveOver; //������ɱ�־
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
  // uint16_t    iReceiveSize;       /*!< UART Tx Transfer size ���λΪ���ձ�־,��˽��ճ���<0x7fff  */
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
