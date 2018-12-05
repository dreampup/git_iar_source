/* USER CODE BEGIN 0 */

#include "stdlib.h"
#include "string.h"
#include "comment.h"
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
 


/*
������ַ���a�в�ѯ�Ƿ���ƥ��b��ȵ�һ����
����: ��һ������ַ���λ�ã�>0��
    =0  û��ƥ���
    >0   Ϊ�ڼ���  ���1���ֽھ�ƥ�䣬��=1
*/
uint16_t CheckIsEquel(uint8_t *aStr,uint16_t aSize,uint8_t *bStr,uint16_t bSize)
{
	uint16_t i,j=0;

	for(i=0; i<aSize; i++)
	{
		if(j<bSize)
		{
			if(aStr[i]==bStr[j])
			{
				j++;
			}
			else if(j>0)
			{
				j=0;
				i--; //����һλ
			}
		}
		if(j==bSize)
			return (i+2-j);
	}
	return 0;
}






/**
* ����: ���ַ�����ʽ��ʮ������ ת��Ϊ16���Ƶ��ֽ�
        �罫"050403"(6byte)ת��Ϊ0x050403(3byte)
* ����: *hBuf:ת�����ʮ�������ַ����� ����Ϊlen/2
		*cBuf:��ת�����ַ���,��ʽ����Ϊ��־���ַ����ṹ        
        len:�ַ�����ʽ��ʮ�����Ƶĳ���,��Ϊż��λ      
* ����:��
*/
void CharTo16Byte(uint8_t *hBuf, uint8_t *cBuf, uint8_t len)
{
  uint8_t i = 0, j = 0;
  for (i = 0; i<len; i++)
  {
    if (cBuf[i] >= '0' && cBuf[i] <= '9')
    {
      if (i & 1) hBuf[j++] |= cBuf[i] - '0'; 
      else hBuf[j] = (cBuf[i] - '0') << 4;
    }
    else if (cBuf[i] >= 'a' && cBuf[i] <= 'f')
    {
      if (i & 1) hBuf[j++] |= cBuf[i] - 'a' + 10;
      else hBuf[j] = (cBuf[i] - 'a' + 10) << 4;
    }
    else if (cBuf[i] >= 'A' && cBuf[i] <= 'F')
    {
      if (i & 1) hBuf[j++] |= cBuf[i] - 'A' + 10;
      else hBuf[j] = (cBuf[i] - 'A' + 10) << 4;
    }
  }
}






/**
* ����: sumУ��
* ����: *cBuf:Ϊ�ַ�����ʽ��ʮ������
        len:cBuf�ַ�����ʽ��ʮ�����Ƶĳ���  
		hBuf:ת��Ϊʮ����������
* ����: 0  err
        1 �ɹ�
*/

uint8_t sumCheck(uint8_t *cBuf,uint8_t *hBuf,uint8_t len)
{
  uint8_t buf[40];
  uint8_t sumData=0,i=0;
  CharTo16Byte(cBuf,buf,len); //bufΪʮ����������
  for(i=0;i<len/2-1;i++)  //len/2-1:��Ҫ��ȥ1�ֽڵ�sum
  {
    sumData +=buf[i];
  }
  sumData ^=0xFF;
  sumData +=0x01;
  if(sumData == buf[i])//У��ɹ�
  {
    for(i=0;i<buf[0];i++)
    {      
      hBuf[i]=buf[i+4];
    }
    return 1;
  }
  return 0;  //err  
}

/**
* ����: ��hex�ļ�תbin�ļ�,��д��flash
* ����: hexAddr:  hex�ļ�����ʼ��ַ
        binAddr: bin�ļ�����ʼ��ַ
* ����: =0 err
       >0  д���bin�����ܳ���
*/
/*
uint32_t IAP_HexToBin(uint32_t hexAddr,uint32_t *binAddr)
{
  uint8_t buf[45]={0};
  uint8_t dataLen=0;
  uint8_t sizeBuf[3]={0};
  uint8_t dataBuf[16]={0};
  
  uint32_t shiftSize=0; //flash�ļ���ƫ�Ƶ�ַ
  uint32_t offsetSize=0; //hex�ļ���ƫ�Ƶ�ַ
  uint32_t writeLen=0; //ʵ��д������ݳ���
  
  W25xFLASH_ReadBuffer(hexAddr,buf,9); //hexǰ9���ֽ� 
  if(buf[0] !=':' || buf[8] !='4') return 0; //'4':Extended Linear Address Record����ʼ��ַ
  CharTo16Byte(buf+1,sizeBuf,2);//�õ����ݵĳ���
  dataLen=sizeBuf[0];
  W25xFLASH_ReadBuffer(hexAddr+1,buf,8+2+dataLen*2); //8:ȥ��ð�ź��8���̶��ֽ�,2:sum 
  if(!sumCheck(buf,dataBuf,8+2+dataLen*2)) return 0;  //У�����  
  
  *binAddr=(dataBuf[0]<<24)+(dataBuf[1]<<16);
   
  offsetSize =13+dataLen*2;  //13=9�ֽڵ�ͷ+2sum+2�س�����    
  while(1)
  {  
     
    W25xFLASH_ReadBuffer(hexAddr+offsetSize,buf,9); //9:hexǰ9���ֽ�  
    if(buf[0] !=':') break;
    
    switch(buf[8])
    {   
      case '5'://Start Linear Address Record,�ļ�����64k����س��ָõ�ַ
        CharTo16Byte(buf+1,sizeBuf,2);//�õ����ݵĳ���
        dataLen=sizeBuf[0];  
        offsetSize +=13+dataLen*2;  //13=9�ֽڵ�ͷ+2sum+2�س�����
        break;            
      case '1'://hex�ļ�������־
        return writeLen;
        break;         
      case '0'://Data Record,����
        CharTo16Byte(buf+1,sizeBuf,6);//�õ����ݵĳ���
        dataLen=sizeBuf[0];
        shiftSize =(sizeBuf[1]<<8)+sizeBuf[2];
         
        W25xFLASH_ReadBuffer(hexAddr+offsetSize+1,buf,8+2+dataLen*2); //8:ȥ��ð�ź��8���̶��ֽ�,2:sum 
        if(!sumCheck(buf,dataBuf,8+2+dataLen*2)) return 0;  //У�����
                 
        W25xFLASH_OnlyWriteBuffer((*binAddr)+shiftSize,dataBuf,dataLen);
        writeLen +=dataLen;   
        offsetSize +=13+dataLen*2;  //13=9�ֽڵ�ͷ+2sum+2�س�����  
        break;
    default: break;
    }
  }
  return 0;
}
*/











/* USER CODE END 1 */