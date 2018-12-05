#include "stm32f0xx_hal.h"
#include "delay.h"

/*
 ��дHAL_InitTickλ��HAL_Init();����
���в���TickPriority���ȼ��ɲ��ã���Ϊ��������HAL_Init�����ж�HAL_MspInit()����������
������hal_msp.c����д��
*/
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
   /*Configure the SysTick to have interrupt in 1ms time basis*/
  //systickʱ��Դ���ã�ϵͳ8��Ƶ,���õ�48/8,Ĭ���ǲ���Ƶ.
 HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); 
//  /**Configure the Systick interrupt time ��������ʱ��1ms�ж�
 //ע�͵�����������ȥ��ϵͳ��systickʱ�Ӻ��жϣ���ʡ�ж���Դ
//HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq() / 1000) �������ж�ʱ��Ϊ1ms
//HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq() / 1000000) �������ж�ʱ��Ϊ1us

  /*Configure the SysTick IRQ priority �Ѿ���xx_hal_.msp.c�ж����� */
//  HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0U);

   /* Return function status */
  return HAL_OK;
}

//���������������HAL_Init()���Ͳ���Ҫ���øú���
//void delay_init(void)
//{
//  //systickʱ��Դ���ã�ϵͳ8��Ƶ,���õ�48/8,Ĭ���ǲ���Ƶ.
// HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
//  /*Configure the SysTick to have interrupt in 1ms time basis,��������ʱ*/
//// HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
/////*Configure the SysTick IRQ priority  �������ж�*/
//
//}

/*
 ��дHAL_Delay()
//��Ҫ�ڹ���������USE_HAL_DRIVER�󼴿�ʹ��HAL_Delay()
//��д���ŵ㣬����Ҫ����systick�жϴ���
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

//��ʱnus
//ע��nus�ķ�Χ�����8��Ƶ������Ҫ*6��6*8=48��
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//us<=LOAD(0xffffff)/SYSCLK(=48/8) =2,796,202.7us
//װ��ֵSysTick->LOAD= nus*SYSCLK(=48)  
////��SYSCLK=48M������,nus <=349,525 
void delay_us(uint32_t nus)
{	
  SysTick->LOAD=(uint32_t)nus*6; 	//ʱ�����(SysTick->LOADΪ24bit)		  
  SysTick->VAL=0UL;        				//��ռ�����/* Load the SysTick Counter Value */
  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;                //��ʼ���� /* SysTick Timer */

  while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));	//�ȴ�ʱ�䵽��   
  SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //�رռ�����  
}

//��ʱnms
//ע��nms�ķ�Χ
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=LOAD(0xffffff)/SYSCLK(=48/8��Ƶ)/1000=2796ms 
//SYSCLK��λΪHz,nms��λΪms
//��SYSCLK=48M�������粻��Ƶ,nms<=2796/8=349ms 

void delay_xms(uint16_t nms)
{	
//  SysTick->LOAD=(uint32_t)nms*48000;		//ʱ�����(SysTick->LOADΪ24bit)
//  SysTick->VAL =0UL;          		//��ռ�����
//  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;      //��ʼ����    
//  while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));	//�ȴ�ʱ�䵽��   
//  SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //�رռ�����  
  delay_us(nms*1000);
  
} 
//��ʱnms 
//nms:0~65535
void delay_ms(uint16_t nms)
{	 	 
  uint8_t repeat=nms/300;	//delay_xms�����ʱ2796
  uint16_t remain=nms%300; 
  while(repeat)
  {
    delay_xms(300);
    repeat--;
  }
  if(remain)delay_xms(remain);
 // HAL_Delay(nms);
} 
