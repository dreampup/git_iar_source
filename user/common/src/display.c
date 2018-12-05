/* Includes ------------------------------------------------------------------*/
/**************************************************
˵��������ʾ�����ǰ��� �ֿ����������HZKCreator ������ASCII�룩�Ͷ���ֿ����ts3�����ɺ��֣�������õ��ֿ⣺
����ȡģ����λ��ǰ������ʽ���Ⱥ���������
ʮ������ ����14 ���ش�С10   ����ÿ�е���14������ÿ��14

*/
#include "stdlib.h"

#include "gpio.h"
#include "display.h"
#include "usart.h"
#include "string.h"
#include "w25xflash.h"
#include "delay.h"
#include "wifi.h"

static uint8_t wordType=1;     // ��������
static uint16_t flashFreq=1000; //�ַ��ƶ���Ƶ��
static uint8_t disDirect=0;//����,��ʾ���Ƿ��� 0 δ���� 1 ����


//9λ��������
void Disp_CMD( uint16_t CS_PINx,uint8_t *cmd, uint8_t len)
{
  uint8_t i,j; 
  GPIO_TypeDef* CS_PROTx;
  if(CS_PINx==HT1632C_CS1_Pin)  
    CS_PROTx=HT1632C_CS1_GPIO_Port;
  else if(CS_PINx==HT1632C_CS2_Pin) 
    CS_PROTx=HT1632C_CS2_GPIO_Port;
  else 
    CS_PROTx=HT1632C_CS3_GPIO_Port;  
  // Set_GPIO_OUTPUT_PP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_RESET);
  //���ݱ�д����wr������  
  for(i=0;i<3;i++)//д�����֣�3λ
  {
    if(CMD_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET); 
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); //������д 
    delay_ns(1);
  }
  for(j=0;j<len;j++) 
  {
    for(i=0;i<9;i++)  //д�����֣�8λ����λ��ǰ
    {
      if(*(cmd+j) & (0x80>>i)) 
        HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
      else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 			
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);  
      delay_ns(1);
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); 
      delay_ns(1);
    }   
  }
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_SET);
}
/**
  * @brief  С��ģʽд���ݵ�RAM
  * @param  CS_PINx:����Ƭѡ1~2
            ramAddress:
            *data:����ָ��
			len:���鳤��
			bSize: ��д��bit���ȣ���Ϊ4�ı���
  * @retval ע�⣺ÿ�ζ�д��λ��������4λ4λ�Ķ�д����fsizeΪ4�ı���
  */
void Disp_WriteToRam(uint16_t CS_PINx,uint8_t ramAddress, uint8_t *data, uint8_t len,uint8_t bSize)
{
  uint8_t i,j;
  GPIO_TypeDef* CS_PROTx;

  if(CS_PINx==HT1632C_CS1_Pin)  
    CS_PROTx=HT1632C_CS1_GPIO_Port;
  else if(CS_PINx==HT1632C_CS2_Pin) 
    CS_PROTx=HT1632C_CS2_GPIO_Port;
  else 
    CS_PROTx=HT1632C_CS3_GPIO_Port;   
  // Set_GPIO_OUTPUT_PP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
	//���ݱ�д����wr������ 
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_RESET);
  for(i=0;i<3;i++)//д�����֣�3λ
  {
    if(WR_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);  		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);    
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); 
    delay_ns(1);
  }
  for(i=0;i<7;i++)//д��ַ�֣�7λ����λ��ǰ
  {
    if(ramAddress & (0x40>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 
		HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET); 
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET);
    delay_ns(1);
  } 
  for(j=0;j<len;j++) 
  {
    for(i=0;i<bSize;i++)  //����:��λ��ǰ 4bit +4bit ע�⣺ÿ�ζ�д��λ��������4λ4λ�Ķ�д����������4�ı�����������1�ֽ�λ����д
    {
      if(*(data+j) & (0x01<<i)) //С��ģʽ:�ȴ���λ���ٴ���λ
//		if(*(data+j) & (0x80>>i)) //���ģʽ:�ȴ���λ���ٴ���λ
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 				
      else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);  
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);     
      delay_ns(1);
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET);   //�����ط��ͳ�ȥ
      delay_ns(1);
    }   
  }
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_SET);
}

/**
  * @brief  С��ģʽ��ȡRAM����
  * @param  CS_PINx:����Ƭѡ1~2
            ramAddress:
            *data:����ָ��
			len:���鳤��	
			bSize: ��д��bit���ȣ���Ϊ4�ı���
  * @retval ע�⣺ÿ�ζ�д��λ��������4λ4λ�Ķ�д����fsizeΪ4�ı���
  */
void Disp_ReadFromRam(uint16_t CS_PINx,uint8_t ramAddress, uint8_t *data, uint8_t len,uint8_t bSize)
{
  uint8_t i,j;
  GPIO_TypeDef* CS_PROTx;
  
  if(CS_PINx==HT1632C_CS1_Pin)  
    CS_PROTx=HT1632C_CS1_GPIO_Port;
  else if(CS_PINx==HT1632C_CS2_Pin) 
    CS_PROTx=HT1632C_CS2_GPIO_Port;
  else 
    CS_PROTx=HT1632C_CS3_GPIO_Port; 
  // Set_GPIO_OUTPUT_PP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
  
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_RESET);  
	//���ݱ�д����wr������ 
  for(i=0;i<3;i++)//д�����֣�3λ
  {
    if(RD_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); 
    delay_ns(1);
  }
  for(i=0;i<7;i++)//д��ַ�֣�7λ����λ��ǰ
  {
    if(ramAddress & (0x40>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET);
    delay_ns(1);
  } 
 //--����   
  HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
  Set_GPIO_INPUT_UP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
	//������rd�½���ʱ���ͳ���DATA�ߣ����ݶ�ȡ�������غ���һ���½���֮��
  for(j=0;j<len;j++) 
  {
    *(data+j)=0;
    for(i=0;i<bSize;i++)  //����:��λ��ǰ 4bit +4bit ע�⣺ÿ�ζ�д��λ��������4λ4λ�Ķ�д����������4�ı�����������1�ֽ�λ����д
    { 
     	HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_RESET);  //���������ݱ��͵�DATA  
		delay_ns(1);
		HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
		delay_ns(1);			
		//*(data+j) >>=1;
		if(HAL_GPIO_ReadPin(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin))				//���´��½��ص���ʱ��ȡ
		//*(data+j) |=0x80; 
		*(data+j) |=(0x01 << i);
    }
  }
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_SET);
  delay_ns(100);	
  Set_GPIO_OUTPUT_PP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
  HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET);	
}

/**
  * @brief  ��ʾ�������ã�ע�⣺�����书��
  * @param  None
  * @retval None
  */
void DisplayConfig(void)
{
  uint8_t temp[5]={0};

  temp[0]=CMD_SYS_DIS;//CMD_SYS_DIS;
  temp[1]=CMD_COM_N16;//CMD_COM_N16;  NMOS����
  temp[2]=CMD_MASTER_RC;//CMD_MASTER_RC;  ������ģʽ
  temp[3]=CMD_SYS_EN;//CMD_SYS_EN;
  temp[4]=CMD_LED_OFF;//CMD_LED_OFF;  
  Disp_CMD(HT1632C_CS1_Pin,temp,5);   
  temp[2]=CMD_SLAVE_MODE;//CMD_SLAVE_MODE; �ӱ�����  
  Disp_CMD(HT1632C_CS2_Pin,temp,5);
  Disp_CMD(HT1632C_CS3_Pin,temp,5);  
  delay_ms(50);  
  
  temp[0]=CMD_BLINK_OFF;//�ر�LED��˸   
  temp[1]=CMD_PWM_DUTY7;//����Ϊ6�����ȵ���1~16��  
  temp[2]=CMD_LED_ON;//CMD_LED_ON;   
  Disp_CMD(HT1632C_CS1_Pin,temp,3);
  Disp_CMD(HT1632C_CS2_Pin,temp,3);
  Disp_CMD(HT1632C_CS3_Pin,temp,3); 
  delay_ms(50); 
  DisplayClean(0,DIS_MAX,0);

  delay_ms(1000); 
}

void DisplayTest(void)
{
 
 	  uint8_t i;  
//  temp[0]=CMD_BLINK_ON;//LED��˸   
//  temp[1]=CMD_PWM_DUTY16;//���ȵ���1~16��  
//  temp[2]=CMD_LED_ON;//CMD_LED_ON; 
//  DisplayClean(0,DIS_MAX,1); //ȫ��
//  for(i=0;i<16;i++)
//  {
//    Disp_CMD(HT1632C_CS1_Pin,temp,3);
//    Disp_CMD(HT1632C_CS2_Pin,temp,3);
//    Disp_CMD(HT1632C_CS3_Pin,temp,3); 
//    temp[1]=CMD_PWM_DUTY1 | i;//���ȵ���1~16��  
//    delay_ms(1000); 
//  }
//  temp[0]=CMD_BLINK_OFF;//�ر�LED��˸ 
//  Disp_CMD(HT1632C_CS1_Pin,temp,3);
//  Disp_CMD(HT1632C_CS2_Pin,temp,3);
//  Disp_CMD(HT1632C_CS3_Pin,temp,3); 
//  DisplayClean(0,DIS_MAX,0);   
  
}
/**
  * @brief  ����ǿ�ȵ���
  * @param  level:0~15   0~15Ϊ16������ǿ�ȵ���
  * @retval None
  */
void DisplayLightAdjust(uint8_t level)  
{
  uint8_t cmd=0xA0;
  if(level>15)
    cmd=CMD_PWM_DUTY16;	
  else
    cmd |=level; 

  Disp_CMD(HT1632C_CS1_Pin,&cmd,1); 
  Disp_CMD(HT1632C_CS2_Pin,&cmd,1); 
  Disp_CMD(HT1632C_CS3_Pin,&cmd,1); 
}
/**
  * @brief  ��������
  * @param  tp: 1 ���� 2���� ...������   
	    frq: �ַ��ƶ��ٶ�
  * @retval None
  */
void DisplayWordSet(uint8_t tp,uint16_t frq)
{
	wordType=tp;	
	flashFreq=frq;
	disDirect = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//���ÿ���
	disDirect =!disDirect; //�߼�ȡ��
}

/**
  * @brief  ��ָ��λ����ʾһ���ַ�
  * @param  xStart:��ʼλ�� 0~DIS_MAX
            *buf:�ַ����ֿ������ַ
			len:  ���鳤��,��Ϊ��2���������鳤��/2Ϊ�ַ���ռ���
  * @retval ע�⣬������ʾ���ַ����ǡ�����ȡģ����λ��ǰ�����ַ�
			�����ֿ�ĸ߶���14��ռ�������ĸ߶ȣ�len����Ϊ�ַ����*2��
			��Ϊ��·�ӷ���Ĭ��һ����16��(����14��)��
  */
void DisplayOneWord(uint8_t xStart,uint8_t *buf,uint8_t len)
{
	uint8_t *temp = NULL,i;
	uint16_t temp16=0;
	if(len<1 || xStart>DIS_MAX+1-len/2) return; //������Χ����ʾ����
	//---����λ��ǰ������ȡģ���ֿ����ת�����������ڴ����
	//--ת��Ϊ ��λ��ǰ������ȡģ-����Ŀǰ�ĵ�·�ṹ������---
	//���ڵ���ǰת������Ȼ������ֽ���λ�������
/* 	for (i = 0; i<len; i++)
	{
		for (j = 0; j<8; j++)
		{
			//--�°���Ļֻ��5�����󣬶�ľͲ���Ҫ������
			if ((i % 2) && (j > 5)) break;
			//--����Ǻ��֣�˫�ֽ�8*2=16����i/16 ���ж�
			//--len==14ΪASCII�� ==28Ϊ���֣�len��Ϊ14�ı���
			if (buf[((i % 2) * 8 + j)*(len/14) +(i/16)] & (0x80 >> (i%16/ 2))) //���λ��λ
				temp[i] |= 0x01 << j;				
		}
	} */	
  
 	if(disDirect)  //---��Ҫ������
	{
		xStart = DIS_MAX+1-xStart-len/2;
		temp=(uint8_t*)malloc(len);	
		//�ַ�������-----------------------
    //--1��ת���ԭ��buf
		// for(i=0;i<len;i++)
		// {
			// buf[i] =temp[i];
		// }
		//--2���ֽڵߵ���
		for(i=0;i<len;i++)
		{
			temp[i] =buf[len-i-1];
		}	
		//--bitλ�ߵ�,������2λ(��Ϊֻ����ʾ14bit)
		for(i=0;i<len;i++)
		{
			//--��ת
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
			//��Ϊ���ظ߶�Ϊ14�������°����ֽڵ������Ҫ�ӵ��ϰ����ֽں���,������ѭ����λ2�ֽ�
			if(i&0x01)//��2���ֽ�����2λ��λ����1���ֽڸ�λ
			{
				temp16= (temp[i]<<8)|temp[i-1] ; 
				temp16 >>=2;//--�������λ��2λ
				buf[i-1] = temp16;
				buf[i] =temp16>>8;
			}			
		}
		free(temp);
	}
  
	if(xStart<22-len/2) //��1��װ��
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,len,8);	
	}	
	else if(xStart<22) //��1��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,(22-xStart)*2,8);
		Disp_WriteToRam(HT1632C_CS2_Pin,0,buf+(22-xStart)*2,len-(22-xStart)*2,8);	
	}  
	else if(xStart<44-len/2) //��2��װ��
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,len,8);
	}
	else if(xStart<44)//��2��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,(44-xStart)*2,8);
		Disp_WriteToRam(HT1632C_CS3_Pin,0,buf+(44-xStart)*2,len-(44-xStart)*2,8);
	}  
	else if(xStart<66-len/2) //��3��װ��
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,len,8);
	}
	else if(xStart<66)//��2��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,(66-xStart)*2,8);
	}
}


/**
  * @brief  ���һ������
  * @param  x:��ʼλ�� 0~DIS_MAX
            y:����λ��(����) 0~DIS_MAX
            ch: 0 Ϊ����(ȫ��)  1Ϊ����(ȫ��)
  * @retval None
  */
void DisplayClean(uint8_t x,uint8_t y, uint8_t ch)
{
  uint8_t temp[2]={0};
	uint8_t xStart,xEnd;
	
  if((y>DIS_MAX) || (x>y)) return;	

	if(disDirect)
	{
		xEnd = DIS_MAX-x;
		xStart = DIS_MAX-y;
	}else{
		xStart=x;
		xEnd=y;		
	}
  if(ch>0)
  {
    temp[0]=temp[1]=0xFF;
  }
  for(;xStart<=xEnd;xStart++)
  {
    if(xStart<22) //��1	
    {		
      Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);//��ʾ��ǰ��
    }else if(xStart<44) //��2
    {
      Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);//��ʾ��ǰ��
    }else //��3
    {
      Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);//��ʾ��ǰ��
    }    
  }
}

/**
  * @brief  ����һ���ַ�
  * @param  x:��ʼλ�� >=0
            y:����λ��(����y) <DIS_MAX+1
						dir:���� 0 ����  1 ����  2ѭ������  3ѭ������
  * @retval None
  */
void DisplayShift(uint8_t x,uint8_t y,uint8_t dir)
{
  uint8_t temp[2]={0},buf[2]={0};  //buf���ڴ洢ѭ���Ƴ�������
	uint8_t xStart,xEnd;	
	if(y>DIS_MAX || x>=y) return;
	if(disDirect)
	{ 
		dir ^=0x01;  //��λ��� ���õ����������������෴
		xStart = DIS_MAX-y;	    
		xEnd = DIS_MAX-x;
	}else{
		xStart=x;
		xEnd=y;		
	}
	
	if(dir &0x01 )  //����������һ�� dir=1��3
	{
		if(dir >1) //��Ϊѭ���ƶ������¼�����ұߵ�1��
		{
			if(xEnd<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xEnd*4,buf,2,8);
			else if(xEnd<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22)*4,buf,2,8);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44)*4,buf,2,8);
		}
		for(;xEnd>xStart;xEnd--) //�����һ�ж�
		{
			if(xEnd<22) //��1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,(xEnd-1)*4,temp,2,8);//ȡǰ1�� *4λRAM��ַ
				Disp_WriteToRam(HT1632C_CS1_Pin,xEnd*4,temp,2,8);//��ʾ������
			}
			else if(xEnd<44) //��2	
			{
				if(xEnd==22)Disp_ReadFromRam(HT1632C_CS1_Pin,21*4,temp,2,8);//����1�����һ��
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-21)*4,temp,2,8);//ȡ����
				Disp_WriteToRam(HT1632C_CS2_Pin,(xEnd-22)*4,temp,2,8);//��ʾ��ǰ��
			}					
			else //��3	
			{
				if(xEnd==44) Disp_ReadFromRam(HT1632C_CS2_Pin,21*4,temp,2,8);//����2�����һ��
				else Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-43)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xEnd-44)*4,temp,2,8);//��ʾ��ǰ��
			}
		}	
	}
	else  //����������һ�� dir=0��2
	{
		if(dir >0)//��ȡ����ߵ�1��
		{
			if(xStart<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,buf,2,8);
			else if(xStart<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2,8);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2,8);
		}
		for(;xStart<xEnd;xStart++)
		{
			if(xStart<22) //��1	
			{
				if(xStart==21)Disp_ReadFromRam(HT1632C_CS2_Pin,0,temp,2,8);//����2
				else Disp_ReadFromRam(HT1632C_CS1_Pin,(xStart+1)*4,temp,2,8);//ȡ����
				Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);//��ʾ��ǰ��
			}		
			else if(xStart<44)//��2	
			{
				if(xStart==43) Disp_ReadFromRam(HT1632C_CS3_Pin,0,temp,2,8);//����3
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-21)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);//��ʾ��ǰ��
			}
			else //��3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-43)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);//��ʾ��ǰ��
			}
		}
	}	
//xStart==xEnd 	��ʾ���1������ ��ѭ���ͷ�ѭ��
	if(xStart<22) //��1	
	Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,2,8);//��ʾ��ǰ��
	else if(xStart<44)//��2	
	Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2,8);//��ʾ��ǰ��
	else 	//��3	
	Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2,8);//��ʾ��ǰ��
  
}

/**
  * @brief  �����ƶ�1ҳ �˳�
  * @param  x:��ʼλ�� >=0
            y:����λ��(����y) <DIS_MAX+1
			dir:����  0 ����  1 ���� 
			*data:�����������,���� xEnd-xStart
 * @retval None
  */
  
  
 void DisplayUpOrDown(uint8_t xStart,uint16_t *buf,uint8_t len,uint8_t dir)
{
	uint8_t temp[2] = {0},i,j;
	uint16_t tem16=0;
	if(len<1 || xStart>DIS_MAX+1-len) return; //������Χ����ʾ����
	for(k=0;k<14;k++)
	{
	//--bitλ�ߵ�,������2λ(��Ϊֻ����ʾ14bit)
	for(i=0;i<len;i++,xStart++)
	{
			//--1���ȶ���ԭ��������
		if(xStart<22) //��1
		{
			Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //��2
		{
			Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);	
		}
		else if(xStart<66) //��3
		{
			Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);	
		}	
		//--2����λ
		tem16 = (temp[1]<<8 +temp[0]) & 0x3fff;
		if(dir) //���ƶ�
		{
			tem16 <<=1;
			if(buf[i] & (0x01<<(13-k))) tem16 |=0x01;
			temp[1] = tem16 >>8;
			temp[0] = tem16;
		}		
		else{//���ƶ�
			tem16 >>=1;
			if(buf[i] & (0x01<<k)) tem16 |=0x01 <<13;
			temp[1] = tem16 >>8;
			temp[0] = tem16;
			
		}
				//--3�����»�ȥ
		if(xStart<22) //��1
		{
			Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //��2
		{
			Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);
		}
		else if(xStart<66) //��3
		{
			Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);
		}	
	
	}
	delay_ms(flashFreq);	
	}
	
}	
void DisplayUpDown(uint8_t xStart,uint8_t xEnd,uint8_t dir)
{
	uint8_t i,x,temp[2]={0};  //buf���ڴ洢ѭ���Ƴ�������
	uint8_t start,end;
	uint16_t temp16 =0;
	if(xEnd>DIS_MAX || xStart>xEnd) return;
	if(disDirect)
	{ 
		dir ^=0x01;  //��λ��� ���õ����������������෴
		start = DIS_MAX-xEnd;	    
		end = DIS_MAX-xStart;
	}else{
		start=xStart;
		end=xEnd;		
	}

	for(i=0;i<14;i++)//14��
	{
		x = start;
		for(;x<end+1;x++)
		{
			if(x<22) //��1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,x*4,temp,2,8);//����2
			}		
			else if(x<44)//��2	
			{
				Disp_ReadFromRam(HT1632C_CS2_Pin,(x-22)*4,temp,2,8);//����2�����һ��
			}
			else //��3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(x-44)*4,temp,2,8);
			}	
	//--���ݴ��� 	
			temp16 =(uint16_t) (temp[1] <<8);
			temp16 += temp[0];
			if(dir)
			{ 
				temp16 <<= 1;
			}
			else
			{
				temp16 >>= 1;
			}
			temp[1] = temp16 >>8;
			temp[0] = temp16;
			if(x<22) //��1	
			{
				Disp_WriteToRam(HT1632C_CS1_Pin,x*4,temp,2,8);//����2
			}		
			else if(x<44)//��2	
			{
				Disp_WriteToRam(HT1632C_CS2_Pin,(x-22)*4,temp,2,8);//����2�����һ��
			}
			else //��3	
			{				
				Disp_WriteToRam(HT1632C_CS3_Pin,(x-44)*4,temp,2,8);
			}	
		}
		delay_ms(flashFreq);		
	}

	
}
/**
  * @brief  ��ת��ʾ
  * @param  
  * @retval None
  */
void DisplayInvert(uint8_t dir)
{
	uint8_t i=0,j=0;
	uint8_t buf[4]={0};
	uint16_t temp=0;
	disDirect = dir; //��÷���
	
	//if(disDirect)
	{
		for(i=0;i<(DIS_MAX+1)/2;i++)			
		{
			if(i<22) //��1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,i*4,buf,2,8);//��ʾ��ǰ��
				Disp_ReadFromRam(HT1632C_CS3_Pin,(21-i)*4,buf+2,2,8);//��ʾ��ǰ��????????????????????????
			}		
			else if(i<44)//��2	
			{
				Disp_ReadFromRam(HT1632C_CS2_Pin,(i-22)*4,buf,2,8);//��ʾ��ǰ��
				Disp_ReadFromRam(HT1632C_CS2_Pin,(43-i)*4,buf+2,2,8);//��ʾ��ǰ��
			}
			// for(j=0;j<4;j++)
			// Usart_Printf(&huart1,"0x%02x ",buf[j]);
		
			for(j=0;j<4;j+=2)
			{
				temp= (buf[j+1]<<8)|buf[j] ;
				//--��ת
				temp = ((temp &0xAAAA)>>1 )| ((temp &0x5555)<<1 );//2��2��Ϊһ�飬����ǰһ��ͺ�һ��,
				temp = ((temp &0xCCCC)>>2 )| ((temp &0x3333)<<2 );//4��4��Ϊһ�飬����ǰһ��ͺ�һ��
				temp = ((temp &0xF0F0)>>4 )| ((temp &0x0F0F)<<4 );//��8��Ϊһ�飬����ǰһ��ͺ�һ��
				temp = ((temp &0xFF00)>>8 )| ((temp &0x00FF)<<8 );//��16��Ϊһ�飬����ǰһ��ͺ�һ��
				//��Ϊ���ظ߶�Ϊ14�������°����ֽڵ������Ҫ�ӵ��ϰ����ֽں���,������ѭ����λ2�ֽ�
				temp >>=2;
				buf[j] =(uint8_t)temp;
				buf[j+1] = (uint8_t)(temp>>8);
			}

			// Usart_Printf(&huart1,"\n");
			// for(j=0;j<4;j++)
			// Usart_Printf(&huart1,"0x%02x ",buf[j]);
			if(i<22) //��1	
			{
				Disp_WriteToRam(HT1632C_CS1_Pin,i*4,buf+2,2,8);//��ʾ��ǰ��
				Disp_WriteToRam(HT1632C_CS3_Pin,(21-i)*4,buf,2,8);//��ʾ��ǰ��
			}		
			else if(i<44)//��2	
			{
				Disp_WriteToRam(HT1632C_CS2_Pin,(i-22)*4,buf+2,2,8);//��ʾ��ǰ��
				Disp_WriteToRam(HT1632C_CS2_Pin,(43-i)*4,buf,2,8);//��ʾ��    ǰ��
			}
		}		
	}
	 
}

/**
  * @brief  ��ָ��������ʾ�ַ���,����ַ���������ʾ�������Ƶ�ȫ����ʾ���Ϊֹ,ĿǰĬ������
  * @param  xStar:��ʾ�������ʼλ�� 0~DIS_MAX
						xEnd: ��ʾ�������λ��(����xEnd) <=DIS_MAX
						*chSrc:��λ�ַ�
						len:�ַ����� ���ֳ���Ϊ�ַ����ȵ�2��	
  * @retval ע�⣺�ֿ�������ַ��ǡ�����ȡģ����λ��ǰ�����ַ����ú������Զ�����ת���ɡ�����ȡģ������Ŀǰ�ĵ�·�ṹ������
  */
void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len)
{
  uint8_t i=0,j=0,k=0;
  uint8_t pxSize=14;
  uint32_t position=0;  //�ֿ�λ��	
  uint8_t *temp=NULL,*buf=NULL; //����ȡ���ַ����� �ַ����Ϊ����14*14 ����14*2
  uint8_t xColumn=0;	  //��¼x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {
//----1��ȡ�ֿ�
    if(chSrc[i]<128)		//ΪASCII��
    {		
		pxSize =14; 
		temp = (uint8_t*)malloc(pxSize);			
		position=chSrc[i]*pxSize;//(chSrc[i]-0x20); 
      if(wordType==1)	     //ASCII 7*14Ӣ������1
      {
       // ASCII_714
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZKSymb_1,SectorZKSymb_1)+position,temp, pxSize); 
      } 
      else if(wordType==2) //ASCII 7*14Ӣ������2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZKSymb_2,SectorZKSymb_2)+position,temp, pxSize); 
      }		
	}
   //---GBK2312b����      
   /*
  ��һ���ֽ�Ϊ�����ֽڡ�����Ӧ94�������ڶ����ֽ�Ϊ�����ֽڡ�����Ӧ94��λ(��94������)������������λ�뷶Χ�ǣ�0101��9494��
  ���ź�λ�ŷֱ����0xA0����GB2312���롣
   �������һ����λ��9494�����ź�λ�ŷֱ�ת����ʮ��������5E5E��0x5E+0xA0��0xFE�����Ը���λ��GB2312������FEFE��
  GB2312���뷶Χ��A1A1��FEFE�����к��ֵı��뷶ΧΪB0A1-F7FE����һ�ֽ�0xB0-0xF7����Ӧ���ţ�16��87�����ڶ����ֽ�0xA1-0xFE����Ӧλ�ţ�01��94��
    */
    else if(chSrc[i]>0xA0 && chSrc[i+1]>0xA0)     
    {
      pxSize =14*2; 
	  temp = (uint8_t*)malloc(pxSize);
      position=((chSrc[i]-0xA1)*94+chSrc[i+1]-0xA1)*pxSize;  //��ַλ��0��ʼ
      if(wordType==1)	     //14*14 ��������1
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK7X14_1,SectorZK7X14_1)+position,temp, pxSize); 
      }
      else if(wordType==2) //14*14 ��������2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK7X14_2,SectorZK7X14_2)+position,temp, pxSize); 
      }
      i++;  //����Ϊ2�ֽ�
    }
	else return;
	
//2���ַ�����ת����������ȡģ����λ��ǰ��temp->������ȡģ����λ��ǰ��buf
	buf = (uint8_t*)calloc(pxSize,1);
	for (j = 0; j<pxSize; j++)
	{
		for (k = 0; k<8; k++)
		{
			//--�°���Ļֻ��5�����󣬶�ľͲ���Ҫ������
			if ((j % 2) && (k > 5)) break;
			//--����Ǻ��֣�˫�ֽ�8*2=16����i/16 ���ж�
			//--pxSize==14ΪASCII�� ==28Ϊ���֣�len��Ϊ14�ı���
			if (temp[((j % 2) * 8 + k)*(pxSize/14) +(j/16)] & (0x80 >> (j%16/ 2))) //���λ��λ
				buf[j] |= 0x01 << k;				
		}
	}
	free(temp);
//---3��������ʾ����
	if(pxSize/2 <= xEnd-xColumn+1)	   //����ʾ��������	
    {	
		DisplayOneWord(xColumn,buf,pxSize);	
		xColumn +=pxSize/2;	
	}
	else  //ʣ���λ�Ӳ�����ȫ��ʾʣ�µ�����
    {	
		k = xEnd-xColumn+1;
		DisplayOneWord(xColumn,buf,k*2);	
		xColumn = xEnd+1;	
		for(j=0;j<pxSize/2-k;j++)//ʣ��δ��ʾ��һ���ַ�������
		{
			delay_ms(flashFreq);	
			DisplayShift(xStar,xEnd,0);  //����1��
			DisplayOneWord(xEnd,buf+(k+j)*2,2);//ÿ����ʾ1��(2������)								
		}
		
    }
	free(buf);
  }
}
/**
  * @brief  ��ָ��λ����ʾһ�����֣��������ţ�
  * @param  xStart:��ʼλ�� 0~43
            *buf:��������5*7����������
			len:���鳤��,һ��Ϊ5,ÿ���ַ����1�п���=6
			line:������0,7��
  * @retval ע�⣬������ʾ���ַ����ǡ�����ȡģ����λ��ǰ�����ַ�
			�����ֿ�ĸ߶���7��ռ1�����ĸ߶ȣ�len����Ϊ�ַ����5��
			��Ϊ��·�ӷ���ע��ram��ַ 
  */
void DisplayOneNumber(uint8_t xStart,uint8_t *buf,uint8_t len,uint8_t line)
{
	uint8_t *temp = NULL,i,j;
	if(len<1 || xStart>DIS_MAX+1-len) return; //������Χ����ʾ����
	temp=(uint8_t*)calloc(len,1);	
	if(disDirect)  //---��Ҫ������
	{
		line = 7-line;
		//--λ���л�
		xStart = DIS_MAX+1-xStart-len;
		//�ַ�������-----------------------    
		//--�ֽڵߵ���
		for(i=0;i<len;i++)
		{
			temp[i] =buf[len-i-1];
		}	
		//--bitλ�ߵ�,������2λ(��Ϊֻ����ʾ14bit)
		for(i=0;i<len;i++)
		{
			//--��ת
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
		}
		//ת���ԭ��buf

		for(i=0;i<len;i++)
		{
			buf[i] = temp[i] >>1;//��Ϊ���ظ߶�Ϊ7��������λ����λ1�ֽ�(ԭ�����������1�ֽ�)		
		}		 
	}
	
	for(i=0;i<len;i++,xStart++)
	{
		
		//--1���ȶ���ԭ��������
		if(xStart<22) //��1
		{
			Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //��2
		{
			Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);	
		}
		else if(xStart<66) //��3
		{
			Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);	
		}		
		//--2��д���buf[i]���ݣ�buf�����ص����λ��Ч��line<14
		for(j=0;j<7;j++)
		{
			
			if(buf[i] & (0x01<<j))			
			{
				if(line+j<8)//temp[0]
				{
					temp[0] |= 0x01<<(line+j);
				}	
				else if(line+j<14) //���
				{					
					temp[1] |= 0x01<<(line-8+j);
				}				
			}
			else
			{
				if(line+j<8)//temp[0]
				{
					temp[0] &= ~(0x01<<(line+j));
				}					 
				else if(line+j<14) //���
				{
					temp[1] &= ~(0x01<<(line-8+j));
				}					
			}
		} 

		
		/*
		for(j=0;j<7;j++)
		{
			
			if(buf[i] & (0x01<<j))			
			{
				if(line+j<8)//temp[0]
				{
					temp[0] |= 0x01<<(line+j);
				}	
				else if(line+j<14) //���
				{					
					temp[1] |= 0x01<<(line-8+j);
				}				
			}
			else
			{
				if(line+j<8)//temp[0]
				{
					temp[0] &= ~(0x01<<(line+j));
				}					
				else if(line+j<14) //���
				{
					temp[1] &= ~(0x01<<(line-8+j));
				}					
			}
		} 
*/		
		//--3�����»�ȥ
		if(xStart<22) //��1
		{
			Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //��2
		{
			Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);
		}
		else if(xStart<66) //��3
		{
			Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);
		}	
	
	}	
	free(temp);	
}

/**
  * @brief  ��ָ��������ʾ�ַ���,����ַ���������ʾ�������Ƶ�ȫ����ʾ���Ϊֹ,ĿǰĬ������
  * @param  xStar:��ʾ�������ʼλ�� 0~DIS_MAX
			xEnd: ��ʾ�������λ��(����xEnd)<=DIS_MAX(65)
			*chSrc:��λ�ַ�
			len:�ַ����ȣ�ע��ÿ���ַ��ĳ��Ȱ���5+1�ֽڿ�ȼ���
			line:������0,1,2������ʾ����Ϊ3��:������
  * @retval ע�⣬������ʾ���ַ����ǡ�����ȡģ����λ��ǰ�����ַ�
			�����ֿ�ĸ߶���7��ռ1�����ĸ߶ȣ�len����Ϊ�ַ����5��Ĭ��ÿ���ַ����1�п���
  */
void DisplayString5X7(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len,uint8_t line)
{
	uint8_t i=0,j,k;
	uint32_t position=0;  //�ֿ�λ��	
	uint8_t temp[6]={0}; //����ȡ���ַ����� �ַ����Ϊ����5*7 5��+1���� ��Ӧ�պ�
	uint8_t xColumn=0;	  //��¼x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {  
//----1��ȡ�ֿ�,�������ֿ�
	position = (chSrc[i] - ' ') *5;
	if(wordType==1)	     //5X7����1
	{
		W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1)+position,temp, 5); 		
	} 
	else if(wordType==2)  //5X7����2
	{ 
		W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2)+position,temp, 5); 
	}
	
	temp[5]=0;//�������
//---2��������ʾ����
	if(chSrc[i] == '.') //С����
	{
		temp[0]=0x60;
		temp[1]=0;
		DisplayOneNumber(xColumn,temp,2,line);	
		xColumn +=2;
	}
	//' ',':'��'-' ����2�п� ���ַ�����0��5λ�գ�ȥ��
	else if((chSrc[i] == ' ') ||(chSrc[i] == ':') || (chSrc[i] == '-') /*||(chSrc[i]  == '/')*/)
	{
		if(xEnd-xColumn + 1 >= 4)	   //����ʾ��������	
		{		
			DisplayOneNumber(xColumn,temp+1,4,line);	
			xColumn +=4;
		}
/* 		else if(xEnd-xColumn +1 ==3)	    //ʣ���λ�Ӹպ���ʾ���1���ַ�������Ҫ����
		{	
			DisplayOneNumber(xColumn,temp+1,3,line);	
			xColumn +=3;	
		} */
		else
		{
			k = xEnd-xColumn+1;
			DisplayOneNumber(xColumn,temp,k,line);	
			xColumn = xEnd+1;	
			for(j=0;j<6-k;j++)//ʣ��δ��ʾ��һ���ַ�������
			{
				delay_ms(flashFreq);	
				DisplayShift(xStar,xEnd,0);  //����1��
				DisplayOneNumber(xEnd,temp+(k+j),1,line);//ÿ����ʾ1��(2������)								
			}

		}
	}
	else 
	{
		if(xEnd-xColumn+1>=6)	   //����ʾ��������	,����ʾ1�п������ڸ�������
		{	
			
			DisplayOneNumber(xColumn,temp,6,line);	
			xColumn +=6;			
		}
/* 		else if(xEnd-xColumn +1==5)	    //ʣ���λ�Ӹպ���ʾ���1���ַ�������Ҫ����
		{	
			
			DisplayOneNumber(xColumn,temp,5,line);	
			xColumn +=5;	
		} */
		else
		{	
	
			k = xEnd-xColumn+1;
			DisplayOneNumber(xColumn,temp,k,line);	
			xColumn = xEnd+1;	
			for(j=0;j<6-k;j++)//ʣ��δ��ʾ��һ���ַ�������
			{
				delay_ms(flashFreq);	
				DisplayShift(xStar,xEnd,0);  //����1��
				DisplayOneNumber(xEnd,temp+(k+j),1,line);//ÿ����ʾ1��(2������)								
			}		
		}		
	}
	
  }
}



/**
  * @brief  ��Ҫ���ڳ��������ʾ
  * @param  x:��ʼλ�� 0~43
            *ch:������Ϊ2�ֽ����飨ascii��Ϊ1�ֽڣ�����Ϊ2�ֽڣ�
  * @retval None
  */
void DisplayMsg(uint8_t* msg)
{
#ifdef _DEBUG_PRINT
	DisplayClean(0,65,0);
	DisplayString7X14(0,65,msg,strlen((char*)msg));
#endif
}





/**
  * @brief  ��ָ��λ����ʾһ������ͼ�Σ��������ţ�
  * @param  x:��ʼλ�� 0~43
            *ch:������Ϊ2�ֽ����飨ascii��Ϊ1�ֽڣ�����Ϊ2�ֽڣ�
  * @retval None
  */
void DisplaySymbol(uint8_t x,uint8_t *ch)
{
	
}





/*
dateStr��ʽ "2017-10-17 5" ,����8 ��20171017,������
            timeStr��ʽ "20:01:50" ,
*/

void DisplayTime(void )
{
	//DisplayClean(0,65,0);	
	DisplayString5X7(0,65,timeStr,5,3);
	//DisplayString5X7(0,65,dateStr,strlen((char*)dateStr),1);	
}























/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/