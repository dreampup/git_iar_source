/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
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
#include "main.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "iwdg.h"
/* USER CODE BEGIN Includes */
#include "stdlib.h"
#include "string.h"
#include "wifi.h"
#include "display.h"
#include "w25xflash.h"
#include "temperature.h"
#include "smart_uart.h"
#include "iap.h"
#include "wav.h"
#include "delay.h"


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if   (defined ( __CC_ARM ))  //keil������
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))  //iar������
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )  //GCC
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#endif


//uint8_t WiFi_Name[10]={"LYG"};
//uint8_t WiFi_Password[20]={"24022402"};
//uint8_t WiFi_TcpWeb[20]={"192.168.31.101"};
//uint8_t WiFi_TcpPort[6]={"60000"};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/**
 *ϵͳ��λ
 */
void SoftReset(void)
{
	//__set_FAULTMASK(1);      // �ر������ж�
	NVIC_SystemReset();// ��λ
}


/* USER CODE END 0 */

int main(void)
{
	uint8_t* Uart_Buf = NULL;
	uint8_t netStatus = 0; //'2' ������WIFI  '5'û������wifi
  
	/* USER CODE BEGIN 1 */
	uint8_t ii=0;
	uint8_t jsonSendBuf[40]={0};
	uint32_t APPLICATION_ADDRESS = 0;
	uint8_t toppleNew=0,toppleOld=0;
	ButtonState btStatus=BT_NON;
	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();//�õ�HAL_Delay��ʱ�ã�ͬʱ��Ҫ�ڹ���������USE_HAL_DRIVER
	//ͬʱ�������ж����ȼ�������stm32f0xx_hal_msp.c
	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();  
	MX_DMA_Init();
	MX_I2C1_Init();
	MX_ADC_Init();
	MX_RTC_Init();
	MX_SPI2_Init();
	MX_TIM3_Init();
	MX_CRC_Init();
	MX_USART2_UART_Init(115200);
	MX_USART1_UART_Init(115200);  
	/* USER CODE BEGIN 2 */
//  W25xFLASH_CheckAPP();
//   /* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/
//  APPLICATION_ADDRESS= ADDR_FLASH_PAGE_0+W25xFLASH_AddrTab.APPStartPage*FLASH_PAGE_SIZE;
//  /* Copy the vector table from the Flash (mapped at the base of the application
//     load address 0x08004000) to the base address of the SRAM at 0x20000000. */
//  for(i = 0; i < 48; i++)
//  {
//    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
//  }
//
//  /* Enable the SYSCFG peripheral clock*/
//  __HAL_RCC_SYSCFG_CLK_ENABLE();
//  /* Remap SRAM at 0x00000000 */
//  __HAL_SYSCFG_REMAPMEMORY_SRAM();
	W25xFLASH_GetConfig();//���Flash����������,����while֮ǰ��ȡ	
	delay_ms(2000);//��ʱ2S�ȴ���ʾ��wifiOK
	DisplayConfig();
	LightTest();
	DisplayWordSet(1,100);//��ʾ��������	
	// DisplayClean(0,65,0);	
	toppleNew = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//���ÿ���
	toppleOld = toppleNew; //Ĭ�Ϸ��÷���
	Usart_Printf(&huart1,"PowerON\r\n");	
	
	// DisplayString7X14(0,65,"�����ӷ���",10);
//	 DisplayString5X7(0,65,"123456:7890",11,1);
//	 while(1)
//	 {
//		 //Usart_Scanf(&recBuff2,&Uart_Buf,0);
//		 smart_uart_scanf();
//	 }
	
	
	//DisplayString7X14(0,65,"123456789",9);
	WiFi_InitConfig();
//	Usart_Printf(&huart1,"===1���������===\n"); 
//====================1���������=============================  
// netStatus����3��TCP�Ϸ�����,�����������շ�������
//       ��2������4��������wifi ��û��TCP�Ϸ�����,��������
//       ��5��û��������wifi   
	netStatus = WiFi_Setup();  
	
	
	if(netStatus == '3')
  { //�ѽ���TCP����		
//===============2���ϵ�ʱ�������Լ죨�ֿ���,��Ҫ���ڳ���ʱ�����ֿ⣩�����ʱ����Ϣ��������Ϣ==========		
		//����ProductId
		sprintf((char*)jsonSendBuf,"{\"ProductID\":\"%s\"}",W25xFLASH_AddrTab.ProductId);
		WiFi_TCPSend(jsonSendBuf,strlen((char*)jsonSendBuf)); 
		delay_ms(1000); //���ӷ������ȴ�ʱ��
   //----------------------a.�ֿ���------------------------------
		if((W25xFLASH_AddrTab.ZKStatus & 0x01) ==0) //Ŀǰֻ�ж��� [ASCII���ֿ�0]
		{
			WiFi_UpdateFile("ziku","ZK5X7_1");			
		}			
		
		if((W25xFLASH_AddrTab.ZKStatus & (0x01<<2)) ==0) //Ŀǰֻ�ж���[�����ֿ�0]
		{
			WiFi_UpdateFile("ziku","ZKSYMB_1");	
		}			 

		if((W25xFLASH_AddrTab.ZKStatus & (0x01<<5)) ==0) //Ŀǰֻ�ж���[�����ֿ�0]
		{
			WiFi_UpdateFile("ziku","ZK7X14_1");	
		}
		Usart_Printf(&huart1,"--->To Renewt Time!\n");
		DisplayMsg("ʱ��ͬ��"); 
		WiFi_RenewTime();
		if(newClock)
		{		
			DisplayMsg("ͬ���ɹ�"); 
			Usart_Printf(&huart1,"date:%s,time:%s\n",dateStr,timeStr);
		}		
   }
//--ʱ����ʾ
	RTC_SetTime(dateStr,timeStr);
	DisplayClean(0,65,0);	
	DisplayTime();
	
	HAL_TIM_Base_Start_IT(&htim3); //��ʼ�������Ӷ�ʱ��8s����1�η�����
	MX_IWDG_Init();//�������Ź�
	ClearReceiveDef(&recBuff2);
	/* USER CODE END 2 */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	

	while(1)
	{
		LightTest();
		btStatus = GPIO_ButtonDect();    
		if(BT_SET ==btStatus)  // �û���ʱ΢����������
		{
			//����ر�RTC
			RTC_StopAlarm();
			
			
			
		   if(WiFi_SmartConfig() ==0) //�����ɹ�
		   { 
			  netStatus = '2';
			  Usart_Printf(&huart1,"wifi������!\n"); 
			  delay_ms(200);
			  if(WiFi_StartTCPConnect(SERVICE,PORT)==0)
			  {   
				 netStatus = '3'; //׼������   
			  }             
			}
			RTC_ResetAlarm();
		}  

		if(timeCountFlag > 0 ) //��ʱ8s
		{
			
		  timeCountFlag = 0;
		  HAL_IWDG_Refresh(&hiwdg);//26S��Ҫι��1��
		  Usart_Printf(&huart1,"--heart--\n"); 
		  netStatus = WiFi_Status();//ÿ��һ����ʱ���ڣ�����һ��������,��������Զ�����
			delay_ms(50);
   		if(netStatus == '2')//������wifi
			{
				if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//����TCP����
       {   
         	netStatus = '3'; //TCP���ӳɹ�
       }				
			}
		  if(netStatus =='3')  //�Ѿ�����TCP����
		  {
	//============���������������===========================
			sprintf((char*)jsonSendBuf,"{\"ProductID\":\"%s\"}",W25xFLASH_AddrTab.ProductId);
			WiFi_TCPSend(jsonSendBuf,strlen((char*)jsonSendBuf)); 	
			
			//--WiFi_RenewTime();
			if(!newClock)  //û���¹�ʱ��
			{				
				Usart_Printf(&huart1,"--->To Renewt Time Again!\n");
				delay_ms(800); //ÿ��ָ����ʱ
				WiFi_RenewTime();
			if(newClock)  //û���¹�ʱ��
			{
					Usart_Printf(&huart1,"date:%s,time:%s\n",dateStr,timeStr);
					RTC_SetTime(dateStr,timeStr);
					DisplayClean(0,65,0);	
					DisplayTime();
				
			}					
			}
	
		
		  } 
		}
		
		toppleNew = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//���ÿ���
		if(toppleOld != toppleNew )
		{
		  toppleOld =toppleNew;
		  DisplayInvert(!toppleOld);
		}
		if(++ii > 5)
		{
			ii=0;
			printf_TH();			
		}
		
//    if(HAL_GPIO_ReadPin(S3_BT1_GPIO_Port,S3_BT1_Pin)==GPIO_PIN_RESET)
//    {
//      Wav_Play(7);
//    }

		smart_uart_scanf();
		

	}
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */


	/* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1
	                                     |RCC_PERIPHCLK_RTC;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}

	/**Configure LSE Drive Capability
	*/
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);


	/* ϵͳ�δ�ʱ��ʱ��Դ */



}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
	/* USER CODE END Error_Handler */
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
