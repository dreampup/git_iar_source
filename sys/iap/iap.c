#include "iap.h"
#include "stdlib.h"
#include "w25xflash.h"
#include "string.h"
#include "crc.h"
#include "usart.h"
#include "delay.h"
/* Private macro -------------------------------------------------------------*/
extern W25xFLASH_AddrDef W25xFLASH_AddrTab;
/* Private variables ---------------------------------------------------------*/

/**
* 功能: 擦除FLASH,写入前必须先擦除
* 参数: startAddress--Flash擦除的起始地址
        numbPages--Flash擦除的页数
* 返回: HAL_OK 擦除成功
       HAL_ERROR 擦除错误 
*/
HAL_StatusTypeDef IAP_Flash_Erase(uint32_t startAddress,uint8_t numbPages)
{ 
  uint32_t PageError=0;
  HAL_StatusTypeDef status = HAL_ERROR;
  /*Variable used for Erase procedure*/
  static FLASH_EraseInitTypeDef EraseInitStruct;
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = startAddress;
  EraseInitStruct.NbPages = numbPages;

  status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
  return status;
}
 

/**
* 功能: 写入APP数据,采用小端模式,先写低位,不需要再擦除
* 参数: 
      StartPages -- 启示页序号 0~63
      AppBuf     --App数据
      AppSize    --APP大小,字节
* 返回: HAL_OK 擦除成功
       HAL_ERROR 擦除错误 
*/

uint8_t IAP_Flash_Write(uint8_t StartPages,uint8_t *AppBuf,uint32_t AppSize)
{
  uint8_t status;
  uint8_t NumPages=0;
  uint8_t *buf=AppBuf;
  uint32_t DataTemp=0,i=0;
  uint32_t StartAddress;

  if(StartPages > 63 ) // 超出flash范围
    return HAL_ERROR;
  NumPages = AppSize / FLASH_PAGE_SIZE + (((AppSize % FLASH_PAGE_SIZE) > 0) ? 1 : 0 );
  if(StartPages + NumPages > 64)  //app超出flash容量
    return HAL_ERROR;
  
//  IAP_Flash_Set(StartPages,StartPages + NumPages);  //设置flash起始地址和结束地址

  StartAddress = ADDR_FLASH_PAGE_0 + FLASH_PAGE_SIZE*StartPages;
  
    /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  status = IAP_Flash_Erase(StartAddress,NumPages);
  if(status != HAL_OK) return status;

  for(i=0; i<AppSize-3; i+=4)
  {					   
    DataTemp  = (uint32_t)buf[i+3]<<24;   
    DataTemp |= (uint32_t)buf[i+2]<<16;    
    DataTemp |= (uint32_t)buf[i+1]<<8;
    DataTemp |= (uint32_t)buf[i];	
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,StartAddress+ i, DataTemp);
    if(status != HAL_OK) return status;
  }
  
  if(i<AppSize)//大小不是4的倍数,则高位补0xff
  {
    if(AppSize -i >2)  
    {
      DataTemp  = (uint32_t)0xFF<<24;
      DataTemp |= (uint32_t)buf[i+2]<<16;
      DataTemp |= (uint32_t)buf[i+1]<<8;
      DataTemp |= (uint32_t)buf[i];
    }
    else if(AppSize -i >1) 
    {
      DataTemp  = (uint32_t)0xFFFF<<16;
      DataTemp |= (uint32_t)buf[i+1]<<8;
      DataTemp |= (uint32_t)buf[i];	
    }
    else if(AppSize -i >0)  
    {
      DataTemp  = (uint32_t)0xFFFFFF<<8;
      DataTemp |= (uint32_t)buf[i];	   
    }
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,StartAddress+ i, DataTemp);
    if(status != HAL_OK) return status;
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
  return HAL_OK;
}
/**
* 功能: 读FLASH数据,测试用
* 参数: 
      StartPages -- 启示页序号 0~63
      AppSize    --flash大小,字节
* 返回: 无
*/
void IAP_Flash_Read(uint8_t StartPages,uint32_t dataSize)
{
  uint32_t DataTemp=0,i=0;
  uint32_t StartAddress=0;
  uint8_t *dataBuf;
  dataBuf=calloc(dataSize+1,1);
  StartAddress = ADDR_FLASH_PAGE_0 + FLASH_PAGE_SIZE*StartPages;
  for(i=0; i<dataSize-3; i+=4)
  {	
    DataTemp = *(__IO uint32_t *)(StartAddress+i);
    dataBuf[i]   = DataTemp;
    dataBuf[i+1] = DataTemp>>8;
    dataBuf[i+2] = DataTemp>>16;
    dataBuf[i+3] = DataTemp>>24;
  }
  if(i<dataSize)//大小不是4的倍数,则高位补0xff
  {
    DataTemp = *(__IO uint32_t *)(StartAddress+i);
    if(dataSize -i >2)  
    {
      dataBuf[i]   = DataTemp;
      dataBuf[i+1] = DataTemp>>8;
      dataBuf[i+2] = DataTemp>>16;
    }
    else if(dataSize -i >1) 
    {
      dataBuf[i]   = DataTemp;
      dataBuf[i+1] = DataTemp>>8;
    }
    else if(dataSize -i >0)  
    {
      dataBuf[i]   = DataTemp;  
    } 
  }
//  Usart_Printf(&huart1,"flash read:%s",dataBuf);
  free(dataBuf);
}




/**
  * @brief 接受bin文件函数
  * @param 
         *pAddr:bin文件的起始地址,长度为8位的字符串           
         fileSize:接收的BEN文件大小
  * @retval  >0 错误，返回错误代码
             0  成功
  * @note          
*/
/*
void IAP_ReceiveBinFile(uint8_t* pAddr, uint32_t fileSize)
{
  uint8_t *pDataBuf=NULL;
  uint8_t i,j,ucStrAddr[9]={0},ucByteAddr[4]={0};
  uint16_t len=0;
  uint16_t uiDataLen=0;
  
  uint32_t ulBinAddr=0,receiveSize=0;
  uint32_t ulCalculateCRC = 0,ulReceiveCRC=0;
  j=strlen((char*)pAddr);
  if(j>8) 
  {
    Usart_Printf(&huart1,"Address err!\r\n");
    return ;
  }
  memset(ucStrAddr,'0',8);
  for(i=0;i<j;i++) 
  {
    ucStrAddr[8-j+i] = pAddr[i];   
  }
  CharTo16Byte(ucStrAddr,ucByteAddr,8); //将8位字符串转换为4byte地址
  ulBinAddr = (ucByteAddr[0]<<24) + (ucByteAddr[1]<<16) + (ucByteAddr[2]<<8) +ucByteAddr[3];
 
	//先读取状态字,避免被擦除
	W25xFLASH_ReadBuffer(W25xFLASH_Block0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//读取状态信息	
  IAP_EraseBinFlash(ulBinAddr);  //写入前先擦除
  Usart_Printf(&huart1,"-->Please Input data at 0x%08x:\r\n",ulBinAddr);
  while(receiveSize<fileSize)
  {
    //-可用补充计时
    len= Usart_Scanf(&recBuff1, &pDataBuf, 500);  //
    if(len >6)  //2byte长度+4byteCRC校验
    {  
      len -=6;
      uiDataLen  = (pDataBuf[1]<<8)+pDataBuf[0];
      ulReceiveCRC = (pDataBuf[5]<<24)+(pDataBuf[4]<<16)+(pDataBuf[3]<<8)+pDataBuf[2];
  Usart_Printf(&huart1,"Receive len=%ld ,Receive crc=0x%08x\r\n",uiDataLen,ulReceiveCRC);       
      if(uiDataLen !=len) 
      {
        FreeScanBuf(&pDataBuf); 
				if(ulBinAddr==0)
				W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//状态信息不变
        Usart_Printf(&huart1,"Receive Length ERR!\r\n");
        return ;      
      }       
      ulCalculateCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *)(pDataBuf+6), len); 
      Usart_Printf(&huart1,"Calculate crc=0x%08x\r\n",ulCalculateCRC);
      if(ulCalculateCRC !=ulReceiveCRC)
      {
        FreeScanBuf(&pDataBuf); 
				if(ulBinAddr==0)
				W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//状态信息不变
        Usart_Printf(&huart1,"Calculate CRC ERR!\r\n");
        return;
      } 
      W25xFLASH_OnlyWriteBuffer(ulBinAddr +receiveSize,pDataBuf+6,len);   
      receiveSize +=len;
      Usart_Printf(&huart1,"Received %d/%d\r\n",receiveSize,fileSize);      
    }  
    FreeScanBuf(&pDataBuf);  
  }

  if(receiveSize !=fileSize) 
	{
		if(ulBinAddr==0)
		W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//状态信息不变
    Usart_Printf(&huart1,"Totle lenngth err!\r\n");
    return ;
	}
  Usart_Printf(&huart1,"Receive Success!\r\n");
//--置位更新标志位 
  IAP_FlagSet(ulBinAddr);
}
*/


//-----------------------------------------------
/**
 *Rom Boatload升级
 *说明:APP运行时进行更新，开机完成后进行升级
 */
void IAP_BoatloadUpdate(void)
{
  uint8_t buf[FLASH_PAGE_SIZE]={0};  //ROM一页是1k大小
  uint8_t i,num1;
  uint16_t num2;
  uint32_t fsize=0,addr=0;
    //APP更新ROM的FLASH页(共64页),默认14页即预留14k给boatload,剩余50k给app  
  if(W25xFLASH_AddrTab.APPStartPage ==0 || W25xFLASH_AddrTab.APPStartPage >63) 
    W25xFLASH_AddrTab.APPStartPage = 14; //先设置APP跳转地址,默认第14个FLASH页即0x08003800 即前14K 
 
  if(W25xFLASH_AddrTab.UpdateStatus[0]==0x5a)//bootload需要升级
  {	
	fsize = W25xFLASH_AddrTab.BootloadSize[3]<<24 + W25xFLASH_AddrTab.BootloadSize[2]<<16 +	W25xFLASH_AddrTab.BootloadSize[1]<<8 + W25xFLASH_AddrTab.BootloadSize[0];
	if(fsize==0 ||fsize>65536) return;
	
    num1=fsize/FLASH_PAGE_SIZE;
    num2=fsize%FLASH_PAGE_SIZE;   
	addr = 	W25xFLASH_BlockSector(BlockBootload,SectorBootload); //起始地址
    for(i=0;i<num1;i++)  //NUM1<64
    {
      W25xFLASH_ReadBuffer(addr + FLASH_PAGE_SIZE*i,buf,FLASH_PAGE_SIZE);
      IAP_Flash_Write(W25xFLASH_AddrTab.APPStartPage+i,buf,FLASH_PAGE_SIZE);     
    }
    if(num2>0)
    {
      W25xFLASH_ReadBuffer(addr + FLASH_PAGE_SIZE*i,buf,num2);
      IAP_Flash_Write(W25xFLASH_AddrTab.APPStartPage+i,buf,num2);   
    } 
    W25xFLASH_AddrTab.UpdateStatus[0]=0xFF;
    W25xFLASH_RenewConfig();
  }
}


/**
 *Rom APP升级
 *说明:Boatload运行时进行更新，即每次开机时更新
 */
void IAP_APPUpdate(void)
{
  uint8_t buf[FLASH_PAGE_SIZE]={0};  //ROM一页是1k大小
  uint8_t i,num1;
  uint16_t num2;
  uint32_t fsize=0,addr=0;
    //APP更新ROM的FLASH页(共64页),默认14页即预留14k给boatload,剩余50k给app  
  if(W25xFLASH_AddrTab.APPStartPage ==0 || W25xFLASH_AddrTab.APPStartPage >63) 
    W25xFLASH_AddrTab.APPStartPage = 14; //先设置APP跳转地址,默认第14个FLASH页即0x08003800 即前14K 
 
  if(W25xFLASH_AddrTab.UpdateStatus[1]==0xA5)//APP需要升级
  {
	fsize = W25xFLASH_AddrTab.APPSize[3]<<24 + W25xFLASH_AddrTab.APPSize[2]<<16 +	W25xFLASH_AddrTab.APPSize[1]<<8 + W25xFLASH_AddrTab.APPSize[0];
	if(fsize==0 ||fsize>65536) return;
	
    num1=fsize/FLASH_PAGE_SIZE;
    num2=fsize%FLASH_PAGE_SIZE;   
	addr = 	W25xFLASH_BlockSector(BlockAPP,SectorAPP); //起始地址                                 
    for(i=0;i<num1;i++)  //NUM1<64
    {
      W25xFLASH_ReadBuffer(addr + FLASH_PAGE_SIZE*i,buf,FLASH_PAGE_SIZE);
      IAP_Flash_Write(W25xFLASH_AddrTab.APPStartPage+i,buf,FLASH_PAGE_SIZE);     
    }
    if(num2>0)
    {
      W25xFLASH_ReadBuffer(addr + FLASH_PAGE_SIZE*i,buf,num2);
      IAP_Flash_Write(W25xFLASH_AddrTab.APPStartPage+i,buf,num2);    
    } 
    W25xFLASH_AddrTab.UpdateStatus[1]=0xFF;
    W25xFLASH_RenewConfig();
  }
}









