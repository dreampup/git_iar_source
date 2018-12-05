#include "wav.h"
#include "stm32f0xx_hal.h"
#include "i2s.h"
#include "w25xflash.h"
#include "string.h"
#include "stdlib.h"
#include "usart.h"
#include "delay.h"
/* Addresses defines */
struct RIFF_HEADER	riff_header;
struct FMT_BLOCK	fmt_block;
struct FACT_BLOCK	fact_block;
struct DATA_BLOCK	data_block;
/**
* 功能: 初始化NS4168功放
* 参数: 
       @参数1：deep-高通滤波等级 取值1~11
* 返回: 
* 说明:  通滤波,滤除低频噪声
        目前只支持右声道,因为左声道CTRL电压在0.9v
*/
void WAV_Init(uint8_t deep)
{
  uint8_t i;
//  HAL_GPIO_WritePin(I2S_Left_Ctrl_GPIO_Port, I2S_Left_Ctrl_Pin, GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(I2S_Righ_Ctrl_GPIO_Port, I2S_Righ_Ctrl_Pin, GPIO_PIN_SET);
  if(deep>11 || deep<1) return;
  if(deep==1) deep=12;
  else  if(deep==2) deep=1;    
  
  HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_RESET);   
  delay_us(150);//>100us  
  //--1线滤波配置
  for(i=0;i<deep;i++)    
  {
    HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_SET);
    delay_us(6); //>1 <12us
    HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_RESET);
    delay_us(6); //>1 <12us
  }
  HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_SET); 
}

//8位 扩充位16位左右声道，deep为音量控制
//8到16位转换过程方法：高字节为0,低字节最高位取反
void PCM8To16LR(uint8_t *src,uint16_t *des,uint16_t len,uint8_t deep)
{
  uint16_t i=0,j=0;
  if(deep<8 && deep>0)
  {
    for(i=0,j=0; i<len;i++,j+=2)
    {
         des[j]=(uint16_t)(src[i]/*^0x80*/)<<deep; //^0x80 为单声道到双声道的变化，Windows录音机的结果 实际测试效果不行
         des[j+1]=des[j];
    }
  }
}

/**
* 功能: wav音频文件信息读取
* 参数: addr: 音频文件的地址   
* 返回:音频数据的长度>0
*/

uint32_t Wav_info(void)
{  
  uint32_t addr=0xA8000;
  uint16_t pos=0;
  //Read RIFF_HEADER
  pos=sizeof(struct RIFF_HEADER);
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&riff_header,pos);//读取状态信息	
  if(	memcmp(riff_header.szRiffID, "RIFF", 4) != 0 ||
		memcmp(riff_header.szRiffFormat, "WAVE", 4) != 0 )
	{
		Usart_Printf(&huart1,"No a vaild wave file!\n");
		return 0;
	}
  addr +=pos;    
  //Read FMT_BLOCK  
  pos=sizeof(struct FMT_BLOCK); //这里按8+18的长度计算
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&fmt_block,pos);//读取状态信息	  
	if(	memcmp(fmt_block.szFmtID, "fmt ", 4) !=0)
	{
		Usart_Printf(&huart1,"No a vaild wave file!\n");
		return 0;
	}
  addr +=pos;
	//Try to read FACT_BLOCK
  if(fmt_block.dwFmtSize !=16) //有附加信息
  {
    addr +=2; //加2个字节为18
    pos=sizeof(struct FACT_BLOCK);
    W25xFLASH_ReadBuffer(addr,(uint8_t*)&fact_block,pos);//读取状态信息	 
    if( memcmp(fact_block.szFactID, "fact", 4) != 0 )
    {	
		Usart_Printf(&huart1,"Read fact_block ERR!\n");
		return 0;
    }
    addr +=pos;
  }
  //Get DATA ADDR  
  pos=sizeof(struct DATA_BLOCK); //这里按8+18的长度计算
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&data_block,pos);//读取状态信息	  
	if (memcmp(data_block.szDataID, "data", 4) != 0)
	{
		Usart_Printf(&huart1,"OOh what error?\n");
		return 0;
	}
  addr +=pos;
  Usart_Printf(&huart1,"声道:%d,采样率:%d,比特率:%d",fmt_block.wavFormat.wChannels,fmt_block.wavFormat.dwSamplesPerSec,fmt_block.wavFormat.dwAvgBytesPerSec);
  
  MX_I2S1_Init(fmt_block.wavFormat.dwSamplesPerSec); 
  
  return   data_block.dwDataSize;
}


void Wav_Play(uint8_t deep)
{
  uint16_t i,len=0;
  uint8_t wavBuf8[200];
  uint16_t wavBuf16[400];
  if(Wav_info()==0) return;
  WAV_Init(8);  //高通滤波,滤除低频噪声
  len=data_block.dwDataSize/200;
  for(i=0;i<len;i++)
  {
      W25xFLASH_ReadBuffer(W25xFLASH_BlockSector(10,8)+44+i*200,(uint8_t*)wavBuf8,200);
      PCM8To16LR(wavBuf8,wavBuf16,200,deep);  
      HAL_I2S_Transmit(&hi2s1,wavBuf16,400,100);  
  }
  len=data_block.dwDataSize%200;
  if(len>0)
  {
    W25xFLASH_ReadBuffer(W25xFLASH_BlockSector(10,8)+44+i*200,(uint8_t*)wavBuf8,len);
    PCM8To16LR(wavBuf8,wavBuf16,len,deep); 
    HAL_I2S_Transmit(&hi2s1,wavBuf16,len*2,100); 
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/