/* Includes ------------------------------------------------------------------*/
/**************************************************
说明：该显示方案是按照 字库制作软件：HZKCreator （生成ASCII码）和多国字库软件ts3（生成汉字）软件设置的字库：
横向取模：高位在前，逐列式（先横向，再纵向）
十六进制 点阵14 像素大小10   代码每行点阵14，索引每行14

*/
#include "stdlib.h"

#include "gpio.h"
#include "display.h"
#include "usart.h"
#include "string.h"
#include "w25xflash.h"
#include "delay.h"
#include "wifi.h"

static uint8_t wordType=1;     // 字体设置
static uint16_t flashFreq=1000; //字符移动的频率
static uint8_t disDirect=0;//方向,显示屏是否倒置 0 未倒置 1 倒置


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
			bSize: 读写的bit长度，须为4的倍数
  * @retval 注意：每次读写的位数必须是4位4位的读写，即fsize为4的倍数
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
    for(i=0;i<bSize;i++)  //数据:低位在前 4bit +4bit 注意：每次读写的位数必须是4位4位的读写，即必须是4的倍数，这里以1字节位数读写
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
			bSize: 读写的bit长度，须为4的倍数
  * @retval 注意：每次读写的位数必须是4位4位的读写，即fsize为4的倍数
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
    for(i=0;i<bSize;i++)  //数据:低位在前 4bit +4bit 注意：每次读写的位数必须是4位4位的读写，即必须是4的倍数，这里以1字节位数读写
    { 
     	HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_RESET);  //下跳沿数据被送到DATA  
		delay_ns(1);
		HAL_GPIO_WritePin(HT1632C_RD_GPIO_Port, HT1632C_RD_Pin, GPIO_PIN_SET);  
		delay_ns(1);			
		//*(data+j) >>=1;
		if(HAL_GPIO_ReadPin(HT1632C_DATA_GPIO_Port,HT1632C_DATA_Pin))				//在下次下降沿到来时读取
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
  temp[1]=CMD_PWM_DUTY7;//亮度为6，亮度调节1~16级  
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
	disDirect = HAL_GPIO_ReadPin(Topple_GPIO_Port,Topple_Pin);//倒置开关
	disDirect =!disDirect; //逻辑取反
}

/**
  * @brief  在指定位置显示一个字符
  * @param  xStart:起始位置 0~DIS_MAX
            *buf:字符的字库数组地址
			len:  数组长度,因为是2行所以数组长度/2为字符所占宽度
  * @retval 注意，这里显示的字符都是【纵向取模，高位在前】的字符
			整个字库的高度是14，占整个屏的高度，len长度为字符宽度*2，
			因为电路接法，默认一列是16个(用了14个)点
  */
void DisplayOneWord(uint8_t xStart,uint8_t *buf,uint8_t len)
{
	uint8_t *temp = NULL,i;
	uint16_t temp16=0;
	if(len<1 || xStart>DIS_MAX+1-len/2) return; //超出范围，显示不下
	//---将高位在前，横向取模的字库进行转换，方便向内存存入
	//--转化为 高位在前，纵向取模-（由目前的电路结构决定）---
	//须在调用前转换，不然多余的字节移位会出问题
/* 	for (i = 0; i<len; i++)
	{
		for (j = 0; j<8; j++)
		{
			//--下半屏幕只有5个点阵，多的就不需要处理了
			if ((i % 2) && (j > 5)) break;
			//--如果是汉字，双字节8*2=16，用i/16 来判断
			//--len==14为ASCII码 ==28为汉字，len需为14的倍数
			if (buf[((i % 2) * 8 + j)*(len/14) +(i/16)] & (0x80 >> (i%16/ 2))) //依次获得位
				temp[i] |= 0x01 << j;				
		}
	} */	
  
 	if(disDirect)  //---需要倒序处理
	{
		xStart = DIS_MAX+1-xStart-len/2;
		temp=(uint8_t*)malloc(len);	
		//字符倒序处理-----------------------
    //--1、转存回原来buf
		// for(i=0;i<len;i++)
		// {
			// buf[i] =temp[i];
		// }
		//--2、字节颠倒，
		for(i=0;i<len;i++)
		{
			temp[i] =buf[len-i-1];
		}	
		//--bit位颠倒,并左移2位(因为只能显示14bit)
		for(i=0;i<len;i++)
		{
			//--反转
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
			//因为像素高度为14，所以下班屏字节倒序后需要接到上半屏字节后面,并整体循环移位2字节
			if(i&0x01)//第2个字节左移2位低位到第1个字节高位
			{
				temp16= (temp[i]<<8)|temp[i-1] ; 
				temp16 >>=2;//--整体向低位移2位
				buf[i-1] = temp16;
				buf[i] =temp16>>8;
			}			
		}
		free(temp);
	}
  
	if(xStart<22-len/2) //屏1能装下
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,len,8);	
	}	
	else if(xStart<22) //屏1能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,(22-xStart)*2,8);
		Disp_WriteToRam(HT1632C_CS2_Pin,0,buf+(22-xStart)*2,len-(22-xStart)*2,8);	
	}  
	else if(xStart<44-len/2) //屏2能装下
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,len,8);
	}
	else if(xStart<44)//屏2能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,(44-xStart)*2,8);
		Disp_WriteToRam(HT1632C_CS3_Pin,0,buf+(44-xStart)*2,len-(44-xStart)*2,8);
	}  
	else if(xStart<66-len/2) //屏3能装下
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,len,8);
	}
	else if(xStart<66)//屏2能装下字符的一部分
	{
		Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,(66-xStart)*2,8);
	}
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
    if(xStart<22) //屏1	
    {		
      Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);//显示到前面
    }else if(xStart<44) //屏2
    {
      Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);//显示到前面
    }else //屏3
    {
      Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);//显示到前面
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
	if(y>DIS_MAX || x>=y) return;
	if(disDirect)
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
			if(xEnd<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xEnd*4,buf,2,8);
			else if(xEnd<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-22)*4,buf,2,8);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-44)*4,buf,2,8);
		}
		for(;xEnd>xStart;xEnd--) //从最后一列读
		{
			if(xEnd<22) //屏1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,(xEnd-1)*4,temp,2,8);//取前1列 *4位RAM地址
				Disp_WriteToRam(HT1632C_CS1_Pin,xEnd*4,temp,2,8);//显示到本列
			}
			else if(xEnd<44) //屏2	
			{
				if(xEnd==22)Disp_ReadFromRam(HT1632C_CS1_Pin,21*4,temp,2,8);//读屏1的最后一列
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xEnd-21)*4,temp,2,8);//取后面
				Disp_WriteToRam(HT1632C_CS2_Pin,(xEnd-22)*4,temp,2,8);//显示到前面
			}					
			else //屏3	
			{
				if(xEnd==44) Disp_ReadFromRam(HT1632C_CS2_Pin,21*4,temp,2,8);//读屏2的最后一列
				else Disp_ReadFromRam(HT1632C_CS3_Pin,(xEnd-43)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xEnd-44)*4,temp,2,8);//显示到前面
			}
		}	
	}
	else  //整个屏左移一列 dir=0或2
	{
		if(dir >0)//读取最左边的1列
		{
			if(xStart<22)Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,buf,2,8);
			else if(xStart<44)Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2,8);
			else	Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2,8);
		}
		for(;xStart<xEnd;xStart++)
		{
			if(xStart<22) //屏1	
			{
				if(xStart==21)Disp_ReadFromRam(HT1632C_CS2_Pin,0,temp,2,8);//读屏2
				else Disp_ReadFromRam(HT1632C_CS1_Pin,(xStart+1)*4,temp,2,8);//取后面
				Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);//显示到前面
			}		
			else if(xStart<44)//屏2	
			{
				if(xStart==43) Disp_ReadFromRam(HT1632C_CS3_Pin,0,temp,2,8);//读屏3
				else Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-21)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);//显示到前面
			}
			else //屏3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-43)*4,temp,2,8);		
				Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);//显示到前面
			}
		}
	}	
//xStart==xEnd 	显示最后1个数组 分循环和非循环
	if(xStart<22) //屏1	
	Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,buf,2,8);//显示到前面
	else if(xStart<44)//屏2	
	Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,buf,2,8);//显示到前面
	else 	//屏3	
	Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,buf,2,8);//显示到前面
  
}

/**
  * @brief  上下移动1页 退出
  * @param  x:起始位置 >=0
            y:结束位置(包含y) <DIS_MAX+1
			dir:方向  0 上移  1 下移 
			*data:待移入的数据,长度 xEnd-xStart
 * @retval None
  */
  
  
 void DisplayUpOrDown(uint8_t xStart,uint16_t *buf,uint8_t len,uint8_t dir)
{
	uint8_t temp[2] = {0},i,j;
	uint16_t tem16=0;
	if(len<1 || xStart>DIS_MAX+1-len) return; //超出范围，显示不下
	for(k=0;k<14;k++)
	{
	//--bit位颠倒,并左移2位(因为只能显示14bit)
	for(i=0;i<len;i++,xStart++)
	{
			//--1、先读出原来的内容
		if(xStart<22) //屏1
		{
			Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //屏2
		{
			Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);	
		}
		else if(xStart<66) //屏3
		{
			Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);	
		}	
		//--2、移位
		tem16 = (temp[1]<<8 +temp[0]) & 0x3fff;
		if(dir) //下移动
		{
			tem16 <<=1;
			if(buf[i] & (0x01<<(13-k))) tem16 |=0x01;
			temp[1] = tem16 >>8;
			temp[0] = tem16;
		}		
		else{//上移动
			tem16 >>=1;
			if(buf[i] & (0x01<<k)) tem16 |=0x01 <<13;
			temp[1] = tem16 >>8;
			temp[0] = tem16;
			
		}
				//--3、更新回去
		if(xStart<22) //屏1
		{
			Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //屏2
		{
			Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);
		}
		else if(xStart<66) //屏3
		{
			Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);
		}	
	
	}
	delay_ms(flashFreq);	
	}
	
}	
void DisplayUpDown(uint8_t xStart,uint8_t xEnd,uint8_t dir)
{
	uint8_t i,x,temp[2]={0};  //buf用于存储循环移出的数组
	uint8_t start,end;
	uint16_t temp16 =0;
	if(xEnd>DIS_MAX || xStart>xEnd) return;
	if(disDirect)
	{ 
		dir ^=0x01;  //低位异或 倒置的左右与正的左右相反
		start = DIS_MAX-xEnd;	    
		end = DIS_MAX-xStart;
	}else{
		start=xStart;
		end=xEnd;		
	}

	for(i=0;i<14;i++)//14行
	{
		x = start;
		for(;x<end+1;x++)
		{
			if(x<22) //屏1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,x*4,temp,2,8);//读屏2
			}		
			else if(x<44)//屏2	
			{
				Disp_ReadFromRam(HT1632C_CS2_Pin,(x-22)*4,temp,2,8);//读屏2的最后一列
			}
			else //屏3	
			{				
				Disp_ReadFromRam(HT1632C_CS3_Pin,(x-44)*4,temp,2,8);
			}	
	//--数据处理 	
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
			if(x<22) //屏1	
			{
				Disp_WriteToRam(HT1632C_CS1_Pin,x*4,temp,2,8);//读屏2
			}		
			else if(x<44)//屏2	
			{
				Disp_WriteToRam(HT1632C_CS2_Pin,(x-22)*4,temp,2,8);//读屏2的最后一列
			}
			else //屏3	
			{				
				Disp_WriteToRam(HT1632C_CS3_Pin,(x-44)*4,temp,2,8);
			}	
		}
		delay_ms(flashFreq);		
	}

	
}
/**
  * @brief  翻转显示
  * @param  
  * @retval None
  */
void DisplayInvert(uint8_t dir)
{
	uint8_t i=0,j=0;
	uint8_t buf[4]={0};
	uint16_t temp=0;
	disDirect = dir; //获得方向
	
	//if(disDirect)
	{
		for(i=0;i<(DIS_MAX+1)/2;i++)			
		{
			if(i<22) //屏1	
			{
				Disp_ReadFromRam(HT1632C_CS1_Pin,i*4,buf,2,8);//显示到前面
				Disp_ReadFromRam(HT1632C_CS3_Pin,(21-i)*4,buf+2,2,8);//显示到前面????????????????????????
			}		
			else if(i<44)//屏2	
			{
				Disp_ReadFromRam(HT1632C_CS2_Pin,(i-22)*4,buf,2,8);//显示到前面
				Disp_ReadFromRam(HT1632C_CS2_Pin,(43-i)*4,buf+2,2,8);//显示到前面
			}
			// for(j=0;j<4;j++)
			// Usart_Printf(&huart1,"0x%02x ",buf[j]);
		
			for(j=0;j<4;j+=2)
			{
				temp= (buf[j+1]<<8)|buf[j] ;
				//--反转
				temp = ((temp &0xAAAA)>>1 )| ((temp &0x5555)<<1 );//2个2个为一组，交换前一半和后一半,
				temp = ((temp &0xCCCC)>>2 )| ((temp &0x3333)<<2 );//4个4个为一组，交换前一半和后一半
				temp = ((temp &0xF0F0)>>4 )| ((temp &0x0F0F)<<4 );//再8个为一组，交换前一半和后一半
				temp = ((temp &0xFF00)>>8 )| ((temp &0x00FF)<<8 );//再16个为一组，交换前一半和后一半
				//因为像素高度为14，所以下班屏字节倒序后需要接到上半屏字节后面,并整体循环移位2字节
				temp >>=2;
				buf[j] =(uint8_t)temp;
				buf[j+1] = (uint8_t)(temp>>8);
			}

			// Usart_Printf(&huart1,"\n");
			// for(j=0;j<4;j++)
			// Usart_Printf(&huart1,"0x%02x ",buf[j]);
			if(i<22) //屏1	
			{
				Disp_WriteToRam(HT1632C_CS1_Pin,i*4,buf+2,2,8);//显示到前面
				Disp_WriteToRam(HT1632C_CS3_Pin,(21-i)*4,buf,2,8);//显示到前面
			}		
			else if(i<44)//屏2	
			{
				Disp_WriteToRam(HT1632C_CS2_Pin,(i-22)*4,buf+2,2,8);//显示到前面
				Disp_WriteToRam(HT1632C_CS2_Pin,(43-i)*4,buf,2,8);//显示到    前面
			}
		}		
	}
	 
}

/**
  * @brief  在指定区域显示字符串,如果字符串大于显示区域，则移到全部显示完成为止,目前默认左移
  * @param  xStar:显示区域的起始位置 0~DIS_MAX
						xEnd: 显示区域结束位置(包含xEnd) <=DIS_MAX
						*chSrc:移位字符
						len:字符长度 汉字长度为字符长度的2倍	
  * @retval 注意：字库里面的字符是【横向取模，高位在前】的字符，该函数会自动进行转换成【纵向取模】（由目前的电路结构决定）
  */
void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len)
{
  uint8_t i=0,j=0,k=0;
  uint8_t pxSize=14;
  uint32_t position=0;  //字库位置	
  uint8_t *temp=NULL,*buf=NULL; //用于取得字符数组 字符最大为汉字14*14 数组14*2
  uint8_t xColumn=0;	  //记录x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {
//----1、取字库
    if(chSrc[i]<128)		//为ASCII码
    {		
		pxSize =14; 
		temp = (uint8_t*)malloc(pxSize);			
		position=chSrc[i]*pxSize;//(chSrc[i]-0x20); 
      if(wordType==1)	     //ASCII 7*14英文字体1
      {
       // ASCII_714
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZKSymb_1,SectorZKSymb_1)+position,temp, pxSize); 
      } 
      else if(wordType==2) //ASCII 7*14英文字体2
      { 
        W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZKSymb_2,SectorZKSymb_2)+position,temp, pxSize); 
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
	  temp = (uint8_t*)malloc(pxSize);
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
	
//2、字符数组转换：【横向取模，高位在前】temp->【纵向取模，高位在前】buf
	buf = (uint8_t*)calloc(pxSize,1);
	for (j = 0; j<pxSize; j++)
	{
		for (k = 0; k<8; k++)
		{
			//--下半屏幕只有5个点阵，多的就不需要处理了
			if ((j % 2) && (k > 5)) break;
			//--如果是汉字，双字节8*2=16，用i/16 来判断
			//--pxSize==14为ASCII码 ==28为汉字，len需为14的倍数
			if (temp[((j % 2) * 8 + k)*(pxSize/14) +(j/16)] & (0x80 >> (j%16/ 2))) //依次获得位
				buf[j] |= 0x01 << k;				
		}
	}
	free(temp);
//---3、计算显示区域
	if(pxSize/2 <= xEnd-xColumn+1)	   //能显示整个数组	
    {	
		DisplayOneWord(xColumn,buf,pxSize);	
		xColumn +=pxSize/2;	
	}
	else  //剩余的位子不能完全显示剩下的内容
    {	
		k = xEnd-xColumn+1;
		DisplayOneWord(xColumn,buf,k*2);	
		xColumn = xEnd+1;	
		for(j=0;j<pxSize/2-k;j++)//剩余未显示的一个字符的数组
		{
			delay_ms(flashFreq);	
			DisplayShift(xStar,xEnd,0);  //左移1列
			DisplayOneWord(xEnd,buf+(k+j)*2,2);//每次显示1列(2个数组)								
		}
		
    }
	free(buf);
  }
}
/**
  * @brief  在指定位置显示一个数字（天气符号）
  * @param  xStart:起始位置 0~43
            *buf:单个数字5*7的像素数组
			len:数组长度,一般为5,每个字符间加1列空列=6
			line:行数【0,7】
  * @retval 注意，这里显示的字符都是【纵向取模，高位在前】的字符
			整个字库的高度是7，占1半屏的高度，len长度为字符宽度5，
			因为电路接法，注意ram地址 
  */
void DisplayOneNumber(uint8_t xStart,uint8_t *buf,uint8_t len,uint8_t line)
{
	uint8_t *temp = NULL,i,j;
	if(len<1 || xStart>DIS_MAX+1-len) return; //超出范围，显示不下
	temp=(uint8_t*)calloc(len,1);	
	if(disDirect)  //---需要倒序处理
	{
		line = 7-line;
		//--位置切换
		xStart = DIS_MAX+1-xStart-len;
		//字符倒序处理-----------------------    
		//--字节颠倒，
		for(i=0;i<len;i++)
		{
			temp[i] =buf[len-i-1];
		}	
		//--bit位颠倒,并左移2位(因为只能显示14bit)
		for(i=0;i<len;i++)
		{
			//--反转
			temp[i] = ((temp[i] &0xAA)>>1 )| ((temp[i] &0x55)<<1 );
			temp[i] = ((temp[i] &0xCC)>>2 )| ((temp[i] &0x33)<<2 );
			temp[i] = ((temp[i] &0xF0)>>4 )| ((temp[i] &0x0F)<<4 );
		}
		//转存回原来buf

		for(i=0;i<len;i++)
		{
			buf[i] = temp[i] >>1;//因为像素高度为7，并右移位出低位1字节(原来的左移最高1字节)		
		}		 
	}
	
	for(i=0;i<len;i++,xStart++)
	{
		
		//--1、先读出原来的内容
		if(xStart<22) //屏1
		{
			Disp_ReadFromRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //屏2
		{
			Disp_ReadFromRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);	
		}
		else if(xStart<66) //屏3
		{
			Disp_ReadFromRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);	
		}		
		//--2、写入的buf[i]数据，buf数据特点最高位无效，line<14
		for(j=0;j<7;j++)
		{
			
			if(buf[i] & (0x01<<j))			
			{
				if(line+j<8)//temp[0]
				{
					temp[0] |= 0x01<<(line+j);
				}	
				else if(line+j<14) //最大
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
				else if(line+j<14) //最大
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
				else if(line+j<14) //最大
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
				else if(line+j<14) //最大
				{
					temp[1] &= ~(0x01<<(line-8+j));
				}					
			}
		} 
*/		
		//--3、更新回去
		if(xStart<22) //屏1
		{
			Disp_WriteToRam(HT1632C_CS1_Pin,xStart*4,temp,2,8);	
		}
		else if(xStart<44) //屏2
		{
			Disp_WriteToRam(HT1632C_CS2_Pin,(xStart-22)*4,temp,2,8);
		}
		else if(xStart<66) //屏3
		{
			Disp_WriteToRam(HT1632C_CS3_Pin,(xStart-44)*4,temp,2,8);
		}	
	
	}	
	free(temp);	
}

/**
  * @brief  在指定区域显示字符串,如果字符串大于显示区域，则移到全部显示完成为止,目前默认左移
  * @param  xStar:显示区域的起始位置 0~DIS_MAX
			xEnd: 显示区域结束位置(包含xEnd)<=DIS_MAX(65)
			*chSrc:移位字符
			len:字符长度，注意每个字符的长度按照5+1字节宽度计算
			line:行数【0,1,2】，显示屏分为3行:上中下
  * @retval 注意，这里显示的字符都是【纵向取模，高位在前】的字符
			整个字库的高度是7，占1半屏的高度，len长度为字符宽度5，默认每个字符间加1列空列
  */
void DisplayString5X7(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len,uint8_t line)
{
	uint8_t i=0,j,k;
	uint32_t position=0;  //字库位置	
	uint8_t temp[6]={0}; //用于取得字符数组 字符最大为汉字5*7 5列+1空列 对应空号
	uint8_t xColumn=0;	  //记录x	

  if(xEnd>DIS_MAX || xStar>xEnd) return;  
	xColumn = xStar;

  for(i=0;i<len;i++)
  {  
//----1、取字库,待完善字库
	position = (chSrc[i] - ' ') *5;
	if(wordType==1)	     //5X7数组1
	{
		W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1)+position,temp, 5); 		
	} 
	else if(wordType==2)  //5X7数组2
	{ 
		W25xFLASH_ReadBuffer( W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2)+position,temp, 5); 
	}
	
	temp[5]=0;//插入空列
//---2、计算显示区域
	if(chSrc[i] == '.') //小数点
	{
		temp[0]=0x60;
		temp[1]=0;
		DisplayOneNumber(xColumn,temp,2,line);	
		xColumn +=2;
	}
	//' ',':'和'-' 缩减2列空 即字符数组0和5位空，去掉
	else if((chSrc[i] == ' ') ||(chSrc[i] == ':') || (chSrc[i] == '-') /*||(chSrc[i]  == '/')*/)
	{
		if(xEnd-xColumn + 1 >= 4)	   //能显示整个数组	
		{		
			DisplayOneNumber(xColumn,temp+1,4,line);	
			xColumn +=4;
		}
/* 		else if(xEnd-xColumn +1 ==3)	    //剩余的位子刚好显示最后1个字符，不需要空列
		{	
			DisplayOneNumber(xColumn,temp+1,3,line);	
			xColumn +=3;	
		} */
		else
		{
			k = xEnd-xColumn+1;
			DisplayOneNumber(xColumn,temp,k,line);	
			xColumn = xEnd+1;	
			for(j=0;j<6-k;j++)//剩余未显示的一个字符的数组
			{
				delay_ms(flashFreq);	
				DisplayShift(xStar,xEnd,0);  //左移1列
				DisplayOneNumber(xEnd,temp+(k+j),1,line);//每次显示1列(2个数组)								
			}

		}
	}
	else 
	{
		if(xEnd-xColumn+1>=6)	   //能显示整个数组	,多显示1列空列用于隔开数字
		{	
			
			DisplayOneNumber(xColumn,temp,6,line);	
			xColumn +=6;			
		}
/* 		else if(xEnd-xColumn +1==5)	    //剩余的位子刚好显示最后1个字符，不需要空列
		{	
			
			DisplayOneNumber(xColumn,temp,5,line);	
			xColumn +=5;	
		} */
		else
		{	
	
			k = xEnd-xColumn+1;
			DisplayOneNumber(xColumn,temp,k,line);	
			xColumn = xEnd+1;	
			for(j=0;j<6-k;j++)//剩余未显示的一个字符的数组
			{
				delay_ms(flashFreq);	
				DisplayShift(xStar,xEnd,0);  //左移1列
				DisplayOneNumber(xEnd,temp+(k+j),1,line);//每次显示1列(2个数组)								
			}		
		}		
	}
	
  }
}



/**
  * @brief  主要用于程序调试显示
  * @param  x:起始位置 0~43
            *ch:单个字为2字节数组（ascii码为1字节，汉字为2字节）
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
  * @brief  在指定位置显示一个符号图形（天气符号）
  * @param  x:起始位置 0~43
            *ch:单个字为2字节数组（ascii码为1字节，汉字为2字节）
  * @retval None
  */
void DisplaySymbol(uint8_t x,uint8_t *ch)
{
	
}





/*
dateStr格式 "2017-10-17 5" ,长度8 即20171017,星期五
            timeStr格式 "20:01:50" ,
*/

void DisplayTime(void )
{
	//DisplayClean(0,65,0);	
	DisplayString5X7(0,65,timeStr,5,3);
	//DisplayString5X7(0,65,dateStr,strlen((char*)dateStr),1);	
}























/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/