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
* ����: ��ʼ��NS4168����
* ����: 
       @����1��deep-��ͨ�˲��ȼ� ȡֵ1~11
* ����: 
* ˵��:  ͨ�˲�,�˳���Ƶ����
        Ŀǰֻ֧��������,��Ϊ������CTRL��ѹ��0.9v
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
  //--1���˲�����
  for(i=0;i<deep;i++)    
  {
    HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_SET);
    delay_us(6); //>1 <12us
    HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_RESET);
    delay_us(6); //>1 <12us
  }
  HAL_GPIO_WritePin(I2S_Ctrl_GPIO_Port, I2S_Ctrl_Pin, GPIO_PIN_SET); 
}

//8λ ����λ16λ����������deepΪ��������
//8��16λת�����̷��������ֽ�Ϊ0,���ֽ����λȡ��
void PCM8To16LR(uint8_t *src,uint16_t *des,uint16_t len,uint8_t deep)
{
  uint16_t i=0,j=0;
  if(deep<8 && deep>0)
  {
    for(i=0,j=0; i<len;i++,j+=2)
    {
         des[j]=(uint16_t)(src[i]/*^0x80*/)<<deep; //^0x80 Ϊ��������˫�����ı仯��Windows¼�����Ľ�� ʵ�ʲ���Ч������
         des[j+1]=des[j];
    }
  }
}

/**
* ����: wav��Ƶ�ļ���Ϣ��ȡ
* ����: addr: ��Ƶ�ļ��ĵ�ַ   
* ����:��Ƶ���ݵĳ���>0
*/

uint32_t Wav_info(void)
{  
  uint32_t addr=0xA8000;
  uint16_t pos=0;
  //Read RIFF_HEADER
  pos=sizeof(struct RIFF_HEADER);
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&riff_header,pos);//��ȡ״̬��Ϣ	
  if(	memcmp(riff_header.szRiffID, "RIFF", 4) != 0 ||
		memcmp(riff_header.szRiffFormat, "WAVE", 4) != 0 )
	{
		Usart_Printf(&huart1,"No a vaild wave file!\n");
		return 0;
	}
  addr +=pos;    
  //Read FMT_BLOCK  
  pos=sizeof(struct FMT_BLOCK); //���ﰴ8+18�ĳ��ȼ���
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&fmt_block,pos);//��ȡ״̬��Ϣ	  
	if(	memcmp(fmt_block.szFmtID, "fmt ", 4) !=0)
	{
		Usart_Printf(&huart1,"No a vaild wave file!\n");
		return 0;
	}
  addr +=pos;
	//Try to read FACT_BLOCK
  if(fmt_block.dwFmtSize !=16) //�и�����Ϣ
  {
    addr +=2; //��2���ֽ�Ϊ18
    pos=sizeof(struct FACT_BLOCK);
    W25xFLASH_ReadBuffer(addr,(uint8_t*)&fact_block,pos);//��ȡ״̬��Ϣ	 
    if( memcmp(fact_block.szFactID, "fact", 4) != 0 )
    {	
		Usart_Printf(&huart1,"Read fact_block ERR!\n");
		return 0;
    }
    addr +=pos;
  }
  //Get DATA ADDR  
  pos=sizeof(struct DATA_BLOCK); //���ﰴ8+18�ĳ��ȼ���
  W25xFLASH_ReadBuffer(addr,(uint8_t*)&data_block,pos);//��ȡ״̬��Ϣ	  
	if (memcmp(data_block.szDataID, "data", 4) != 0)
	{
		Usart_Printf(&huart1,"OOh what error?\n");
		return 0;
	}
  addr +=pos;
  Usart_Printf(&huart1,"����:%d,������:%d,������:%d",fmt_block.wavFormat.wChannels,fmt_block.wavFormat.dwSamplesPerSec,fmt_block.wavFormat.dwAvgBytesPerSec);
  
  MX_I2S1_Init(fmt_block.wavFormat.dwSamplesPerSec); 
  
  return   data_block.dwDataSize;
}


void Wav_Play(uint8_t deep)
{
  uint16_t i,len=0;
  uint8_t wavBuf8[200];
  uint16_t wavBuf16[400];
  if(Wav_info()==0) return;
  WAV_Init(8);  //��ͨ�˲�,�˳���Ƶ����
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