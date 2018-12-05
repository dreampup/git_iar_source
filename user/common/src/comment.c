/* USER CODE BEGIN 0 */

#include "stdlib.h"
#include "string.h"
#include "comment.h"
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
 


/*
检测在字符串a中查询是否有匹配b相等的一部分
返回: 第一个相等字符的位置（>0）
    =0  没有匹配的
    >0   为第几个  如第1个字节就匹配，则=1
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
				i--; //回退一位
			}
		}
		if(j==bSize)
			return (i+2-j);
	}
	return 0;
}






/**
* 功能: 将字符串格式的十六进制 转换为16进制的字节
        如将"050403"(6byte)转化为0x050403(3byte)
* 参数: *hBuf:转换后的十六进制字符数组 长度为len/2
		*cBuf:被转换的字符串,格式必须为标志的字符串结构        
        len:字符串格式的十六进制的长度,必为偶数位      
* 返回:无
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
* 功能: sum校验
* 参数: *cBuf:为字符串格式的十六进制
        len:cBuf字符串格式的十六进制的长度  
		hBuf:转化为十六进制数组
* 返回: 0  err
        1 成功
*/

uint8_t sumCheck(uint8_t *cBuf,uint8_t *hBuf,uint8_t len)
{
  uint8_t buf[40];
  uint8_t sumData=0,i=0;
  CharTo16Byte(cBuf,buf,len); //buf为十六进制数组
  for(i=0;i<len/2-1;i++)  //len/2-1:需要减去1字节的sum
  {
    sumData +=buf[i];
  }
  sumData ^=0xFF;
  sumData +=0x01;
  if(sumData == buf[i])//校验成功
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
* 功能: 将hex文件转bin文件,并写入flash
* 参数: hexAddr:  hex文件的起始地址
        binAddr: bin文件的起始地址
* 返回: =0 err
       >0  写入的bin数据总长度
*/
/*
uint32_t IAP_HexToBin(uint32_t hexAddr,uint32_t *binAddr)
{
  uint8_t buf[45]={0};
  uint8_t dataLen=0;
  uint8_t sizeBuf[3]={0};
  uint8_t dataBuf[16]={0};
  
  uint32_t shiftSize=0; //flash文件的偏移地址
  uint32_t offsetSize=0; //hex文件的偏移地址
  uint32_t writeLen=0; //实际写入的数据长度
  
  W25xFLASH_ReadBuffer(hexAddr,buf,9); //hex前9个字节 
  if(buf[0] !=':' || buf[8] !='4') return 0; //'4':Extended Linear Address Record，起始地址
  CharTo16Byte(buf+1,sizeBuf,2);//得到数据的长度
  dataLen=sizeBuf[0];
  W25xFLASH_ReadBuffer(hexAddr+1,buf,8+2+dataLen*2); //8:去除冒号后的8个固定字节,2:sum 
  if(!sumCheck(buf,dataBuf,8+2+dataLen*2)) return 0;  //校验出错  
  
  *binAddr=(dataBuf[0]<<24)+(dataBuf[1]<<16);
   
  offsetSize =13+dataLen*2;  //13=9字节的头+2sum+2回车换行    
  while(1)
  {  
     
    W25xFLASH_ReadBuffer(hexAddr+offsetSize,buf,9); //9:hex前9个字节  
    if(buf[0] !=':') break;
    
    switch(buf[8])
    {   
      case '5'://Start Linear Address Record,文件超过64k，这回出现该地址
        CharTo16Byte(buf+1,sizeBuf,2);//得到数据的长度
        dataLen=sizeBuf[0];  
        offsetSize +=13+dataLen*2;  //13=9字节的头+2sum+2回车换行
        break;            
      case '1'://hex文件结束标志
        return writeLen;
        break;         
      case '0'://Data Record,数据
        CharTo16Byte(buf+1,sizeBuf,6);//得到数据的长度
        dataLen=sizeBuf[0];
        shiftSize =(sizeBuf[1]<<8)+sizeBuf[2];
         
        W25xFLASH_ReadBuffer(hexAddr+offsetSize+1,buf,8+2+dataLen*2); //8:去除冒号后的8个固定字节,2:sum 
        if(!sumCheck(buf,dataBuf,8+2+dataLen*2)) return 0;  //校验出错
                 
        W25xFLASH_OnlyWriteBuffer((*binAddr)+shiftSize,dataBuf,dataLen);
        writeLen +=dataLen;   
        offsetSize +=13+dataLen*2;  //13=9字节的头+2sum+2回车换行  
        break;
    default: break;
    }
  }
  return 0;
}
*/











/* USER CODE END 1 */