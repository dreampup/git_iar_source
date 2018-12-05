/* Includes ------------------------------------------------------------------*/

/**************************************************
说明：该显示方案是按照 PCtolcd2002软件设置的字库：
阴码 逐列式 逆向(低位在前) 十六进制 点阵14 像素大小10   代码每行点阵14，索引每行14

*/
#include "stdlib.h"

#include "gpio.h"
#include "display.h"
#include "usart.h"
#include "string.h"
#include "w25xflash.h"
#include "delay.h"
#include "font.h"

static uint8_t wordType=1;     // 字体设置
static uint16_t flashFreq=1000; //字符移动的频率
uint8_t DisplayInvert=0;//方向,显示屏是否倒置 0 未倒置 1 倒置

void Disp_SetInv(uint8_t inv)
{
  if(inv>0)
  DisplayInvert =1;    
  else DisplayInvert=0;
}

//9位数的命令
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
  //数据被写入在wr上升沿  
  for(i=0;i<3;i++)//写命令字：3位
  {
    if(CMD_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET); 
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); //上升沿写 
    delay_ns(1);
  }
  for(j=0;j<len;j++) 
  {
    for(i=0;i<9;i++)  //写命令字：8位，高位在前
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
  * @brief  小端模式写数据到RAM
  * @param  CS_PINx:点阵片选1~2
            ramAddress:
            *data:数组指针
						len:数组长度
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
	//数据被写入在wr上升沿 
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_RESET);
  for(i=0;i<3;i++)//写命令字：3位
  {
    if(WR_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);  		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);    
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); 
    delay_ns(1);
  }
  for(i=0;i<7;i++)//写地址字：7位，高位在前
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
    for(i=0;i<8;i++)  //数据:低位在前 4bit +4bit
    {
      if(*(data+j) & (0x01<<i)) //小端模式:先传低位，再传高位
//		if(*(data+j) & (0x80>>i)) //大端模式:先传高位，再传低位
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 				
      else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);  
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);     
      delay_ns(1);
      HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET);   //上升沿发送出去
      delay_ns(1);
    }   
  }
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_SET);
}

/**
  * @brief  小端模式读取RAM数据
  * @param  CS_PINx:点阵片选1~2
            ramAddress:
            *data:数组指针
						len:数组长度
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
	//数据被写入在wr上升沿 
  for(i=0;i<3;i++)//写命令字：3位
  {
    if(RD_MODE & (0x04>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET); 		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET); 
    delay_ns(1);
  }
  for(i=0;i<7;i++)//写地址字：7位，高位在前
  {
    if(ramAddress & (0x40>>i)) 
      HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET); 
    else HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_RESET);		
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_RESET);
    delay_ns(1);
    HAL_GPIO_WritePin(HT1632C_WR_GPIO_Port, HT1632C_WR_Pin, GPIO_PIN_SET);
    delay_ns(1);
  } 
 //--输入   
  HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
  Set_GPIO_INPUT_UP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
	//数据在rd下降沿时被送出到DATA线，数据读取在上升沿和下一次下降沿之间
  for(j=0;j<len;j++) 
  {
    *(data+j)=0;
    for(i=0;i<8;i++)  //数据:低位在前 4bit +4bit
    { 
     	HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_RESET);  //下跳沿数据被送到DATA  
			delay_ns(1);
      HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
			delay_ns(1);			
			*(data+j) >>=1;
      if(HAL_GPIO_ReadPin(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin))				//在下次下降沿到来时读取
      *(data+j) |=0x80;   
    }
  }
  HAL_GPIO_WritePin(CS_PROTx, CS_PINx, GPIO_PIN_SET);
  delay_ns(100);	
  Set_GPIO_OUTPUT_PP(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin); 
  HAL_GPIO_WritePin(HT1632C_DATA_GPIO_Port, HT1632C_DATA_Pin, GPIO_PIN_SET);	
}

/**
  * @brief  显示驱动配置，注意：带记忆功能
  * @param  None
  * @retval None
  */
void DisplayConfig(void)
{
  uint8_t temp[5]={0};

  temp[0]=CMD_SYS_DIS;//CMD_SYS_DIS;
  temp[1]=CMD_COM_N16;//CMD_COM_N16;  NMOS驱动
  temp[2]=CMD_MASTER_RC;//CMD_MASTER_RC;  主驱动模式
  temp[3]=CMD_SYS_EN;//CMD_SYS_EN;
  temp[4]=CMD_LED_OFF;//CMD_LED_OFF;  
  Disp_CMD(HT1632C_CS1_Pin,temp,5);   
  temp[2]=CMD_SLAVE_MODE;//CMD_SLAVE_MODE; 从被驱动  
  Disp_CMD(HT1632C_CS2_Pin,temp,5);
  Disp_CMD(HT1632C_CS3_Pin,temp,5);  
  delay_ms(50);  
  
  temp[0]=CMD_BLINK_OFF;//关闭LED闪烁   
  temp[1]=CMD_PWM_DUTY7;//亮度调节1~16级  
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
//  temp[0]=CMD_BLINK_ON;//LED闪烁   
//  temp[1]=CMD_PWM_DUTY16;//亮度调节1~16级  
//  temp[2]=CMD_LED_ON;//CMD_LED_ON; 
//  DisplayClean(0,DIS_MAX,1); //全亮
//  for(i=0;i<16;i++)
//  {
//    Disp_CMD(HT1632C_CS1_Pin,temp,3);
//    Disp_CMD(HT1632C_CS2_Pin,temp,3);
//    Disp_CMD(HT1632C_CS3_Pin,temp,3); 
//    temp[1]=CMD_PWM_DUTY1 | i;//亮度调节1~16级  
//    delay_ms(1000); 
//  }
//  temp[0]=CMD_BLINK_OFF;//关闭LED闪烁 
//  Disp_CMD(HT1632C_CS1_Pin,temp,3);
//  Disp_CMD(HT1632C_CS2_Pin,temp,3);
//  Disp_CMD(HT1632C_CS3_Pin,temp,3); 
//  DisplayClean(0,DIS_MAX,0);   
  


	  DisplayInvert=0;
	  for(i=0;i<6;i++)
	  {  
	  DisplayClean(0,DIS_MAX,1); //全亮
	  
	  delay_ms(500); 
	  DisplayClean(0,DIS_MAX,0);  
	   delay_ms(500);  
		 DisplayString7X14(0,65,"123456789",9); 
		 delay_ms(500);  
	  }
		DisplayInvert=1;
	  for(i=0;i<6;i++)
	  {  
	  DisplayClean(0,DIS_MAX,1); //全亮	  
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
  * @brief  光线强度调节
  * @param  level:0~15   0~15为16级光线强度调节
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
  * @brief  字体设置
  * @param  tp: 1 宋体 2楷体 ...待补充   
	    frq: 字符移动速度
  * @retval None
  */
void DisplayWordSet(uint8_t tp,uint16_t frq)
{
  wordType=tp;	
  flashFreq=frq;
}

/**
  * @brief  在指定位置显示一个字符
  * @param  x:起始位置 0~DIS_MAX
            *buf:字符的字库数组地址
			len:  数组长度,因为是2行所以数组长度/2为字符所占宽度
  * @retval None
  */
void DisplayOneWord(uint8_t x,uint8_t *buf,uint8_t len )
{
	uint8_t xStart;
	uint8_t *temp = NULL,i;
	uint16_t temp16=0;
	if(len<1 || x>DIS_MAX) return;
	temp=(uint8_t*)malloc(len);
 	if(DisplayInvert)  //---需要倒序处理
	{
		if(DIS_MAX-x <len/2) //显示不下
		{
			free(temp);
			return;
		}
		
		xStart = DIS_MAX-x-len/2;
		//字符倒序处理-----------------------
		//--字节颠倒，
		for(i=0;i<len;i++)
		temp[i] =buf[len-i-1];
		//--bit位颠倒,并左移2位(因为只能显示14bit)
		for(i=0;i<len;i++)
		{
			//--反转
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
			
			if(i&0x01)//第2个字节左移2位低位到第1个字节高位
			{
				temp16= (temp[i]<<8)|temp[i-1]  ;
				temp16 >>=2;//--整体向低位移2位
				temp[i-1] = temp16;
				temp[i] =temp16>>8;
			}			
		}		
	}
	else{
		xStart=x;
		memcpy(temp,buf,len);		
	} 
	if(xStart<22-len/2) //屏1能装下
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,len);	
	}	
	else if(xStart<22) //屏1能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,(22-xStart)*2);
		Disp_WriteToRam(HT1632C_CS2_Pin,0,temp+(22-xStart)*2,len-(22-xStart)*2);	
	}  
	else if(xStart<44-len/2) //屏2能装下
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,len);
	}
	else if(xStart<44)//屏2能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,(44-xStart)*2);
		Disp_WriteToRam(HT1632C_CS3_Pin,0,temp+(44-xStart)*2,len-(44-xStart)*2);
	}  
	else if(xStart<66-len/2) //屏3能装下
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,len);
	}
	else if(xStart<66)//屏2能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,(66-xStart)*2);
	}	
	free(temp);
}


/**
  * @brief  清除一个区域
  * @param  x:起始位置 0~DIS_MAX
            y:结束位置(包含) 0~DIS_MAX
            ch: 0 为黑屏(全灭)  1为白屏(全亮)
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
    if(xStart<22) //屏1	
    {		
      Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2);//显示到前面
    }else if(xStart<44) //屏2
    {
      Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2);//显示到前面
    }else //屏3
    {
      Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2);//显示到前面
    }    
  }
}

/**
  * @brief  左移一列字符
  * @param  x:起始位置 >=0
            y:结束位置(包含y) <DIS_MAX+1
						dir:方向 0 左移  1 右移  2循环左移  3循环右移
  * @retval None
  */
void DisplayShift(uint8_t x,uint8_t y,uint8_t dir)
{
  uint8_t temp[2]={0},buf[2]={0};  //buf用于存储循环移出的数组
	uint8_t xStart,xEnd;	
	if(y>DIS_MAX || x>y) return;
	if(DisplayInvert)
	{ 
		dir ^=0x01;  //低位异或 倒置的左右与正的左右相反
		xStart = DIS_MAX-y;	    
		xEnd = DIS_MAX-x;
	}else{
		xStart=x;
		xEnd=y;		
	}
	
	if(dir &0x01 )  //整个屏右移一列 dir=1或3
	{
		if(dir >1) //如为循环移动，则记录下最右边的1列
		{
			if(xEnd<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xEnd*4,buf,2);
			else if(xEnd<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22)*4,buf,2);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44)*4,buf,2);
		}
		for(;xEnd>xStart;xEnd--) //从最后一列读
		{
			if(xEnd<22) //屏1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,(xEnd-1)*4,temp,2);//取前1列 *4位RAM地址
				Disp_WriteToRam(HT1632C_CS1_Pin,xEnd*4,temp,2);//显示到本列
			}
			else if(xEnd<44) //屏2	
			{
				if(xEnd==22)Disp_ReadFromRam(HT1632C_CS1_Pin,21*4,temp,2);//读屏1的最后一列
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22-1)*4,temp,2);//取后面
				Disp_WriteToRam(HT1632C_CS2_Pin,(xEnd-22)*4,temp,2);//显示到前面
			}					
			else //屏3	
			{
				if(xEnd==44) Disp_ReadFromRam(HT1632C_CS2_Pin,43*4,temp,2);//读屏2的最后一列
				else Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44-1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xEnd-44)*4,temp,2);//显示到前面
			}
		}	
	}
	else  //整个屏左移一列 dir=0或2
	{
		if(dir >0)//读取最左边的1列
		{
			if(xStart<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,buf,2);
			else if(xStart<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2);
		}
		for(;xStart<xEnd;xStart++)
		{
			if(xStart<22) //屏1	
			{
				if(xStart==21)Disp_ReadFromRam(HT1632C_CS2_Pin,0,temp,2);//读屏2
				else Disp_ReadFromRam(HT1632C_CS1_Pin,(xStart+1)*4,temp,2);//取后面
				Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2);//显示到前面
			}		
			else if(xStart<44)//屏2	
			{
				if(xStart==43) Disp_ReadFromRam(HT1632C_CS3_Pin,0,temp,2);//读屏3
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22+1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2);//显示到前面
			}
			else //屏3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44+1)*4,temp,2);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2);//显示到前面
			}
		}
	}	
//xStart==xEnd 	显示最后1个数组 分循环和非循环
	if(xStart<22) //屏1	
	Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,2);//显示到前面
	else if(xStart<44)//屏2	
	Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2);//显示到前面
	else 	//屏3	
	Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2);//显示到前面
  
}

/**
  * @brief  在指定区域显示字符串,如果字符串大于显示区域，则移到全部显示完成为止,目前默认左移
  * @param  xStar:显示区域的起始位置 0~DIS_MAX
						xEnd: 显示区域结束位置(包含xEnd)
						*chSrc:移位字符
						len:字符长度 汉字长度为字符长度的2倍	
  * @retval None
  */
void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len)
{
  uint8_t i=0,j=0;
  uint8_t pxSize;
  uint32_t position=0;  //字库位置	
  uint8_t temp[28]={0}; //用于取得字符数组 字符最大为汉字14*14 数组14*2
  uint8_t xColumn=0;	  //记录x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {
//----1、取字库
    if(chSrc[i]<128)		//为ASCII码
    {		
      pxSize =14; 		
      position=chSrc[i]*pxSize;//(chSrc[i]-0x20); 
      if(wordType==1)	     //ASCII 7*14英文字体1
      {
       // ASCII_714
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1)+position,temp, pxSize); 
      } 
      else if(wordType==2) //ASCII 7*14英文字体2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2)+position,temp, pxSize); 
      }		
	}
   //---GBK2312b编码      
   /*
  第一个字节为“高字节”，对应94个区；第二个字节为“低字节”，对应94个位(即94个汉字)。所以它的区位码范围是：0101－9494。
  区号和位号分别加上0xA0就是GB2312编码。
   例如最后一个码位是9494，区号和位号分别转换成十六进制是5E5E，0x5E+0xA0＝0xFE，所以该码位的GB2312编码是FEFE。
  GB2312编码范围：A1A1－FEFE，其中汉字的编码范围为B0A1-F7FE，第一字节0xB0-0xF7（对应区号：16－87），第二个字节0xA1-0xFE（对应位号：01－94）
    */
    else if(chSrc[i]>0xA0 && chSrc[i+1]>0xA0)     
    {
      pxSize =14*2; 
      position=((chSrc[i]-0xA1)*94+chSrc[i+1]-0xA1)*pxSize;  //地址位从0开始
      if(wordType==1)	     //14*14 汉字字体1
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK7X14_1,SectorZK7X14_1)+position,temp, pxSize); 
      }
      else if(wordType==2) //14*14 汉字字体2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK7X14_2,SectorZK7X14_2)+position,temp, pxSize); 
      }
      i++;  //汉字为2字节
    }
	else return;
//---2、计算显示区域
	if(xEnd-xColumn>=pxSize/2)	   //能显示整个数组	
    {	
		DisplayOneWord(xColumn,temp,pxSize);	
		xColumn +=pxSize/2;	
	}
	else  //剩余的位子不能完全显示剩下的内容
    {			
      DisplayOneWord(xColumn,temp,(xEnd-xColumn)*2);			
      for(j=0;j<pxSize/2+xColumn-xEnd;j++)//剩余未显示的一个字符的数组
      {
        delay_ms(flashFreq);	
        DisplayShift(xStar,xEnd-1,0);  //左移1列
        DisplayOneWord(xEnd-1,temp+(xEnd-xColumn+j)*2,2);								
      }
    }	
  }
}
/**
  * @brief  在指定位置显示一个符号图形（天气符号）
  * @param  x:起始位置 0~43
            *ch:单个字为2字节数组（ascii码为1字节，汉字为2字节）
  * @retval None
  */
void DisplaySymbol(uint8_t x,uint8_t *ch)
{
	
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/