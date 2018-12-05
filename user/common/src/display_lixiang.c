/* Includes ------------------------------------------------------------------*/

/**************************************************
˵��������ʾ�����ǰ��� PCtolcd2002������õ��ֿ⣺
���� ����ʽ ����(��λ��ǰ) ʮ������ ����14 ���ش�С10   ����ÿ�е���14������ÿ��14

*/
#include "stdlib.h"

#include "gpio.h"
#include "display.h"
#include "usart.h"
#include "string.h"
#include "w25xflash.h"
#include "delay.h"
#include "font.h"

static uint8_t wordType=1;     // ��������
static uint16_t flashFreq=1000; //�ַ��ƶ���Ƶ��
uint8_t DisplayInvert=0;//����,��ʾ���Ƿ��� 0 δ���� 1 ����

void Disp_SetInv(uint8_t inv)
{
  if(inv>0)
  DisplayInvert =1;    
  else DisplayInvert=0;
}

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
  * @retval None
  */
void Disp_WriteToRam(uint16_t CS_PINx,uint8_t ramAddress, uint8_t *data, uint8_t len)
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
    for(i=0;i<8;i++)  //����:��λ��ǰ 4bit +4bit
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
  * @retval None
  */
void Disp_ReadFromRam(uint16_t CS_PINx,uint8_t ramAddress, uint8_t *data, uint8_t len)
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
    for(i=0;i<8;i++)  //����:��λ��ǰ 4bit +4bit
    { 
     	HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_RESET);  //���������ݱ��͵�DATA  
			delay_ns(1);
      HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
			delay_ns(1);			
			*(data+j) >>=1;
      if(HAL_GPIO_ReadPin(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin))				//���´��½��ص���ʱ��ȡ
      *(data+j) |=0x80;   
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
  temp[1]=CMD_PWM_DUTY7;//���ȵ���1~16��  
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
  


	  DisplayInvert=0;
	  for(i=0;i<6;i++)
	  {  
	  DisplayClean(0,DIS_MAX,1); //ȫ��
	  
	  delay_ms(500); 
	  DisplayClean(0,DIS_MAX,0);  
	   delay_ms(500);  
		 DisplayString7X14(0,65,"123456789",9); 
		 delay_ms(500);  
	  }
		DisplayInvert=1;
	  for(i=0;i<6;i++)
	  {  
	  DisplayClean(0,DIS_MAX,1); //ȫ��	  
	  delay_ms(500); 
	  DisplayClean(0,DIS_MAX,0);  
	   delay_ms(500);  
		 DisplayString7X14(0,65,"123456789",9); 
		 delay_ms(500);   
	  }

	DisplayInvert=0;	  


    // uint8_t temp[]={0x00,0x00,0x08,0x08,0x08,0x08,0xFC,0x0F,0x00,0x08,0x00,0x08,0x00,0x00};/*"1",17*/
		// uint8_t buf[14];

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
}

/**
  * @brief  ��ָ��λ����ʾһ���ַ�
  * @param  x:��ʼλ�� 0~DIS_MAX
            *buf:�ַ����ֿ������ַ
			len:  ���鳤��,��Ϊ��2���������鳤��/2Ϊ�ַ���ռ���
  * @retval None
  */
void DisplayOneWord(uint8_t x,uint8_t *buf,uint8_t len )
{
	uint8_t xStart;
	uint8_t *temp = NULL,i;
	uint16_t temp16=0;
	if(len<1 || x>DIS_MAX) return;
	temp=(uint8_t*)malloc(len);
 	if(DisplayInvert)  //---��Ҫ������
	{
		if(DIS_MAX-x <len/2) //��ʾ����
		{
			free(temp);
			return;
		}
		
		xStart = DIS_MAX-x-len/2;
		//�ַ�������-----------------------
		//--�ֽڵߵ���
		for(i=0;i<len;i++)
		temp[i] =buf[len-i-1];
		//--bitλ�ߵ�,������2λ(��Ϊֻ����ʾ14bit)
		for(i=0;i<len;i++)
		{
			//--��ת
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
			
			if(i&0x01)//��2���ֽ�����2λ��λ����1���ֽڸ�λ
			{
				temp16= (temp[i]<<8)|temp[i-1]  ;
				temp16 >>=2;//--�������λ��2λ
				temp[i-1] = temp16;
				temp[i] =temp16>>8;
			}			
		}		
	}
	else{
		xStart=x;
		memcpy(temp,buf,len);		
	} 
	if(xStart<22-len/2) //��1��װ��
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,len);	
	}	
	else if(xStart<22) //��1��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,(22-xStart)*2);
		Disp_WriteToRam(HT1632C_CS2_Pin,0,temp+(22-xStart)*2,len-(22-xStart)*2);	
	}  
	else if(xStart<44-len/2) //��2��װ��
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,len);
	}
	else if(xStart<44)//��2��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,(44-xStart)*2);
		Disp_WriteToRam(HT1632C_CS3_Pin,0,temp+(44-xStart)*2,len-(44-xStart)*2);
	}  
	else if(xStart<66-len/2) //��3��װ��
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,len);
	}
	else if(xStart<66)//��2��װ���ַ���һ����
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,(66-xStart)*2);
	}	
	free(temp);
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

	if(DisplayInvert)
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
      Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2);//��ʾ��ǰ��
    }else if(xStart<44) //��2
    {
      Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2);//��ʾ��ǰ��
    }else //��3
    {
      Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2);//��ʾ��ǰ��
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
	if(y>DIS_MAX || x>y) return;
	if(DisplayInvert)
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
			if(xEnd<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xEnd*4,buf,2);
			else if(xEnd<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22)*4,buf,2);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44)*4,buf,2);
		}
		for(;xEnd>xStart;xEnd--) //�����һ�ж�
		{
			if(xEnd<22) //��1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,(xEnd-1)*4,temp,2);//ȡǰ1�� *4λRAM��ַ
				Disp_WriteToRam(HT1632C_CS1_Pin,xEnd*4,temp,2);//��ʾ������
			}
			else if(xEnd<44) //��2	
			{
				if(xEnd==22)Disp_ReadFromRam(HT1632C_CS1_Pin,21*4,temp,2);//����1�����һ��
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22-1)*4,temp,2);//ȡ����
				Disp_WriteToRam(HT1632C_CS2_Pin,(xEnd-22)*4,temp,2);//��ʾ��ǰ��
			}					
			else //��3	
			{
				if(xEnd==44) Disp_ReadFromRam(HT1632C_CS2_Pin,43*4,temp,2);//����2�����һ��
				else Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44-1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xEnd-44)*4,temp,2);//��ʾ��ǰ��
			}
		}	
	}
	else  //����������һ�� dir=0��2
	{
		if(dir >0)//��ȡ����ߵ�1��
		{
			if(xStart<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,buf,2);
			else if(xStart<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2);
		}
		for(;xStart<xEnd;xStart++)
		{
			if(xStart<22) //��1	
			{
				if(xStart==21)Disp_ReadFromRam(HT1632C_CS2_Pin,0,temp,2);//����2
				else Disp_ReadFromRam(HT1632C_CS1_Pin,(xStart+1)*4,temp,2);//ȡ����
				Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2);//��ʾ��ǰ��
			}		
			else if(xStart<44)//��2	
			{
				if(xStart==43) Disp_ReadFromRam(HT1632C_CS3_Pin,0,temp,2);//����3
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22+1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2);//��ʾ��ǰ��
			}
			else //��3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44+1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2);//��ʾ��ǰ��
			}
		}
	}	
//xStart==xEnd 	��ʾ���1������ ��ѭ���ͷ�ѭ��
	if(xStart<22) //��1	
	Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,2);//��ʾ��ǰ��
	else if(xStart<44)//��2	
	Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2);//��ʾ��ǰ��
	else 	//��3	
	Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2);//��ʾ��ǰ��
  
}

/**
  * @brief  ��ָ��������ʾ�ַ���,����ַ���������ʾ�������Ƶ�ȫ����ʾ���Ϊֹ,ĿǰĬ������
  * @param  xStar:��ʾ�������ʼλ�� 0~DIS_MAX
						xEnd: ��ʾ�������λ��(����xEnd)
						*chSrc:��λ�ַ�
						len:�ַ����� ���ֳ���Ϊ�ַ����ȵ�2��	
  * @retval None
  */
void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len)
{
  uint8_t i=0,j=0;
  uint8_t pxSize;
  uint32_t position=0;  //�ֿ�λ��	
  uint8_t temp[28]={0}; //����ȡ���ַ����� �ַ����Ϊ����14*14 ����14*2
  uint8_t xColumn=0;	  //��¼x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {
//----1��ȡ�ֿ�
    if(chSrc[i]<128)		//ΪASCII��
    {		
      pxSize =14; 		
      position=chSrc[i]*pxSize;//(chSrc[i]-0x20); 
      if(wordType==1)	     //ASCII 7*14Ӣ������1
      {
       // ASCII_714
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1)+position,temp, pxSize); 
      } 
      else if(wordType==2) //ASCII 7*14Ӣ������2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2)+position,temp, pxSize); 
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
//---2��������ʾ����
	if(xEnd-xColumn>=pxSize/2)	   //����ʾ��������	
    {	
		DisplayOneWord(xColumn,temp,pxSize);	
		xColumn +=pxSize/2;	
	}
	else  //ʣ���λ�Ӳ�����ȫ��ʾʣ�µ�����
    {			
      DisplayOneWord(xColumn,temp,(xEnd-xColumn)*2);			
      for(j=0;j<pxSize/2+xColumn-xEnd;j++)//ʣ��δ��ʾ��һ���ַ�������
      {
        delay_ms(flashFreq);	
        DisplayShift(xStar,xEnd-1,0);  //����1��
        DisplayOneWord(xEnd-1,temp+(xEnd-xColumn+j)*2,2);								
      }
    }	
  }
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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/