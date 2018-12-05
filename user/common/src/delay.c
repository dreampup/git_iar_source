#include "stm32f0xx_hal.h"
#include "delay.h"

/*
 重写HAL_InitTick位于HAL_Init();函数
其中参数TickPriority优先级可不用，因为接下来的HAL_Init函数中对HAL_MspInit()进行了配置
配置在hal_msp.c中重写了
*/
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
   /*Configure the SysTick to have interrupt in 1ms time basis*/
  //systick时钟源配置：系统8分频,即得到48/8,默认是不分频.
 HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); 
//  /**Configure the Systick interrupt time 并开启了时钟1ms中断
 //注释掉下面这句可以去掉系统的systick时钟和中断，节省中断资源
//HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq() / 1000) 定义了中断时间为1ms
//HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq() / 1000000) 定义了中断时间为1us

  /*Configure the SysTick IRQ priority 已经在xx_hal_.msp.c中定义了 */
//  HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0U);

   /* Return function status */
  return HAL_OK;
}

//如果主函数调用了HAL_Init()，就不需要调用该函数
//void delay_init(void)
//{
//  //systick时钟源配置：系统8分频,即得到48/8,默认是不分频.
// HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
//  /*Configure the SysTick to have interrupt in 1ms time basis,并启动计时*/
//// HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
/////*Configure the SysTick IRQ priority  不启用中断*/
//
//}

/*
 重写HAL_Delay()
//需要在工程中配置USE_HAL_DRIVER后即可使用HAL_Delay()
//重写的优点，不需要进行systick中断处理
*/
void HAL_Delay(__IO uint32_t Delay)
{
  delay_ms(Delay);
}

//===============================================================
void delay_ns(uint16_t nms)
{
  while(nms--)
  {
  __NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();
  }
}

//延时nus
//注意nus的范围如果是8分频，这里要*6（6*8=48）
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//us<=LOAD(0xffffff)/SYSCLK(=48/8) =2,796,202.7us
//装载值SysTick->LOAD= nus*SYSCLK(=48)  
////对SYSCLK=48M条件下,nus <=349,525 
void delay_us(uint32_t nus)
{	
  SysTick->LOAD=(uint32_t)nus*6; 	//时间加载(SysTick->LOAD为24bit)		  
  SysTick->VAL=0UL;        				//清空计数器/* Load the SysTick Counter Value */
  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;                //开始倒数 /* SysTick Timer */

  while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));	//等待时间到达   
  SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器  
}

//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=LOAD(0xffffff)/SYSCLK(=48/8分频)/1000=2796ms 
//SYSCLK单位为Hz,nms单位为ms
//对SYSCLK=48M条件下如不分频,nms<=2796/8=349ms 

void delay_xms(uint16_t nms)
{	
//  SysTick->LOAD=(uint32_t)nms*48000;		//时间加载(SysTick->LOAD为24bit)
//  SysTick->VAL =0UL;          		//清空计数器
//  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;      //开始倒数    
//  while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));	//等待时间到达   
//  SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器  
  delay_us(nms*1000);
  
} 
//延时nms 
//nms:0~65535
void delay_ms(uint16_t nms)
{	 	 
  uint8_t repeat=nms/300;	//delay_xms最大延时2796
  uint16_t remain=nms%300; 
  while(repeat)
  {
    delay_xms(300);
    repeat--;
  }
  if(remain)delay_xms(remain);
 // HAL_Delay(nms);
} 
