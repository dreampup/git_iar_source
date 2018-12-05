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
#if   (defined ( __CC_ARM ))  //keil编译器
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))  //iar编译器
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
 *系统软复位
 */
void SoftReset(void)
{
	//__set_FAULTMASK(1);      // 关闭所有中端
	NVIC_SystemReset();// 复位
}


/* USER CODE END 0 */

int main(void)
{
	uint8_t* Uart_Buf = NULL;
	uint8_t netStatus = 0; //'2' 连上了WIFI  '5'没有连上wifi
  
	/* USER CODE BEGIN 1 */
	uint8_t ii=0;
	uint8_t jsonSendBuf[40]={0};
	uint32_t APPLICATION_ADDRESS = 0;
	uint8_t toppleNew=0,toppleOld=0;
	ButtonState btStatus=BT_NON;
	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();//用到HAL_Delay等时用，同时需要在工程中配置USE_HAL_DRIVER
	//同时进行了中断优先级设置在stm32f0xx_hal_msp.c
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
	W25xFLASH_GetConfig();//获得Flash的配置数据,进入while之前读取	
	delay_ms(2000);//延时2S等待显示和wifiOK
	DisplayConfig();
	LightTest();
	DisplayWordSet(1,100);//显示字体设置	
	// DisplayClean(0,65,0);	
	toppleNew = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//倒置开关
	toppleOld = toppleNew; //默认放置方向
	Usart_Printf(&huart1,"PowerON\r\n");	
	
	// DisplayString7X14(0,65,"已连接服务",10);
//	 DisplayString5X7(0,65,"123456:7890",11,1);
//	 while(1)
//	 {
//		 //Usart_Scanf(&recBuff2,&Uart_Buf,0);
//		 smart_uart_scanf();
//	 }
	
	
	//DisplayString7X14(0,65,"123456789",9);
	WiFi_InitConfig();
//	Usart_Printf(&huart1,"===1、上网检测===\n"); 
//====================1、上网检测=============================  
// netStatus：‘3’TCP上服务器,接下来可以收发数据了
//       ‘2’、‘4’连上了wifi 但没有TCP上服务器,外网问题
//       ‘5’没有连上了wifi   
	netStatus = WiFi_Setup();  
	
	
	if(netStatus == '3')
  { //已建立TCP连接		
//===============2、上电时，进行自检（字库检测,主要用于出厂时更新字库），获得时间信息、天气信息==========		
		//发送ProductId
		sprintf((char*)jsonSendBuf,"{\"ProductID\":\"%s\"}",W25xFLASH_AddrTab.ProductId);
		WiFi_TCPSend(jsonSendBuf,strlen((char*)jsonSendBuf)); 
		delay_ms(1000); //连接服务器等待时间
   //----------------------a.字库检测------------------------------
		if((W25xFLASH_AddrTab.ZKStatus & 0x01) ==0) //目前只判断了 [ASCII码字库0]
		{
			WiFi_UpdateFile("ziku","ZK5X7_1");			
		}			
		
		if((W25xFLASH_AddrTab.ZKStatus & (0x01<<2)) ==0) //目前只判断了[符号字库0]
		{
			WiFi_UpdateFile("ziku","ZKSYMB_1");	
		}			 

		if((W25xFLASH_AddrTab.ZKStatus & (0x01<<5)) ==0) //目前只判断了[汉字字库0]
		{
			WiFi_UpdateFile("ziku","ZK7X14_1");	
		}
		Usart_Printf(&huart1,"--->To Renewt Time!\n");
		DisplayMsg("时间同步"); 
		WiFi_RenewTime();
		if(newClock)
		{		
			DisplayMsg("同步成功"); 
			Usart_Printf(&huart1,"date:%s,time:%s\n",dateStr,timeStr);
		}		
   }
//--时间显示
	RTC_SetTime(dateStr,timeStr);
	DisplayClean(0,65,0);	
	DisplayTime();
	
	HAL_TIM_Base_Start_IT(&htim3); //开始网络连接定时，8s连接1次服务器
	MX_IWDG_Init();//开启看门狗
	ClearReceiveDef(&recBuff2);
	/* USER CODE END 2 */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	

	while(1)
	{
		LightTest();
		btStatus = GPIO_ButtonDect();    
		if(BT_SET ==btStatus)  // 用户随时微信配置网络
		{
			//补充关闭RTC
			RTC_StopAlarm();
			
			
			
		   if(WiFi_SmartConfig() ==0) //配网成功
		   { 
			  netStatus = '2';
			  Usart_Printf(&huart1,"wifi已连网!\n"); 
			  delay_ms(200);
			  if(WiFi_StartTCPConnect(SERVICE,PORT)==0)
			  {   
				 netStatus = '3'; //准备就绪   
			  }             
			}
			RTC_ResetAlarm();
		}  

		if(timeCountFlag > 0 ) //定时8s
		{
			
		  timeCountFlag = 0;
		  HAL_IWDG_Refresh(&hiwdg);//26S内要喂狗1次
		  Usart_Printf(&huart1,"--heart--\n"); 
		  netStatus = WiFi_Status();//每个一个定时周期，进行一次网络检测,如果掉线自动重连
			delay_ms(50);
   		if(netStatus == '2')//连上了wifi
			{
				if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//开启TCP连接
       {   
         	netStatus = '3'; //TCP连接成功
       }				
			}
		  if(netStatus =='3')  //已经建立TCP连接
		  {
	//============向服务器发送心跳===========================
			sprintf((char*)jsonSendBuf,"{\"ProductID\":\"%s\"}",W25xFLASH_AddrTab.ProductId);
			WiFi_TCPSend(jsonSendBuf,strlen((char*)jsonSendBuf)); 	
			
			//--WiFi_RenewTime();
			if(!newClock)  //没更新过时间
			{				
				Usart_Printf(&huart1,"--->To Renewt Time Again!\n");
				delay_ms(800); //每条指令延时
				WiFi_RenewTime();
			if(newClock)  //没更新过时间
			{
					Usart_Printf(&huart1,"date:%s,time:%s\n",dateStr,timeStr);
					RTC_SetTime(dateStr,timeStr);
					DisplayClean(0,65,0);	
					DisplayTime();
				
			}					
			}
	
		
		  } 
		}
		
		toppleNew = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//倒置开关
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


	/* 系统滴答定时器时钟源 */



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
