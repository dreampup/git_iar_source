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
* ����: ����FLASH,д��ǰ�����Ȳ���
* ����: startAddress--Flash��������ʼ��ַ
        numbPages--Flash������ҳ��
* ����: HAL_OK �����ɹ�
       HAL_ERROR �������� 
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
* ����: д��APP����,����С��ģʽ,��д��λ,����Ҫ�ٲ���
* ����: 
      StartPages -- ��ʾҳ��� 0~63
      AppBuf     --App����
      AppSize    --APP��С,�ֽ�
* ����: HAL_OK �����ɹ�
       HAL_ERROR �������� 
*/

uint8_t IAP_Flash_Write(uint8_t StartPages,uint8_t *AppBuf,uint32_t AppSize)
{
  uint8_t status;
  uint8_t NumPages=0;
  uint8_t *buf=AppBuf;
  uint32_t DataTemp=0,i=0;
  uint32_t StartAddress;

  if(StartPages > 63 ) // ����flash��Χ
    return HAL_ERROR;
  NumPages = AppSize / FLASH_PAGE_SIZE + (((AppSize % FLASH_PAGE_SIZE) > 0) ? 1 : 0 );
  if(StartPages + NumPages > 64)  //app����flash����
    return HAL_ERROR;
  
//  IAP_Flash_Set(StartPages,StartPages + NumPages);  //����flash��ʼ��ַ�ͽ�����ַ

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
  
  if(i<AppSize)//��С����4�ı���,���λ��0xff
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
* ����: ��FLASH����,������
* ����: 
      StartPages -- ��ʾҳ��� 0~63
      AppSize    --flash��С,�ֽ�
* ����: ��
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
  if(i<dataSize)//��С����4�ı���,���λ��0xff
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
  * @brief ����bin�ļ�����
  * @param 
         *pAddr:bin�ļ�����ʼ��ַ,����Ϊ8λ���ַ���           
         fileSize:���յ�BEN�ļ���С
  * @retval  >0 ���󣬷��ش������
             0  �ɹ�
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
  CharTo16Byte(ucStrAddr,ucByteAddr,8); //��8λ�ַ���ת��Ϊ4byte��ַ
  ulBinAddr = (ucByteAddr[0]<<24) + (ucByteAddr[1]<<16) + (ucByteAddr[2]<<8) +ucByteAddr[3];
 
	//�ȶ�ȡ״̬��,���ⱻ����
	W25xFLASH_ReadBuffer(W25xFLASH_Block0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//��ȡ״̬��Ϣ	
  IAP_EraseBinFlash(ulBinAddr);  //д��ǰ�Ȳ���
  Usart_Printf(&huart1,"-->Please Input data at 0x%08x:\r\n",ulBinAddr);
  while(receiveSize<fileSize)
  {
    //-���ò����ʱ
    len= Usart_Scanf(&recBuff1, &pDataBuf, 500);  //
    if(len >6)  //2byte����+4byteCRCУ��
    {  
      len -=6;
      uiDataLen  = (pDataBuf[1]<<8)+pDataBuf[0];
      ulReceiveCRC = (pDataBuf[5]<<24)+(pDataBuf[4]<<16)+(pDataBuf[3]<<8)+pDataBuf[2];
  Usart_Printf(&huart1,"Receive len=%ld ,Receive crc=0x%08x\r\n",uiDataLen,ulReceiveCRC);       
      if(uiDataLen !=len) 
      {
        FreeScanBuf(&pDataBuf); 
				if(ulBinAddr==0)
				W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//״̬��Ϣ����
        Usart_Printf(&huart1,"Receive Length ERR!\r\n");
        return ;      
      }       
      ulCalculateCRC = HAL_CRC_Calculate(&hcrc, (uint32_t *)(pDataBuf+6), len); 
      Usart_Printf(&huart1,"Calculate crc=0x%08x\r\n",ulCalculateCRC);
      if(ulCalculateCRC !=ulReceiveCRC)
      {
        FreeScanBuf(&pDataBuf); 
				if(ulBinAddr==0)
				W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//״̬��Ϣ����
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
		W25xFLASH_OnlyWriteBuffer(0,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));//״̬��Ϣ����
    Usart_Printf(&huart1,"Totle lenngth err!\r\n");
    return ;
	}
  Usart_Printf(&huart1,"Receive Success!\r\n");
//--��λ���±�־λ 
  IAP_FlagSet(ulBinAddr);
}
*/


//-----------------------------------------------
/**
 *Rom Boatload����
 *˵��:APP����ʱ���и��£�������ɺ��������
 */
void IAP_BoatloadUpdate(void)
{
  uint8_t buf[FLASH_PAGE_SIZE]={0};  //ROMһҳ��1k��С
  uint8_t i,num1;
  uint16_t num2;
  uint32_t fsize=0,addr=0;
    //APP����ROM��FLASHҳ(��64ҳ),Ĭ��14ҳ��Ԥ��14k��boatload,ʣ��50k��app  
  if(W25xFLASH_AddrTab.APPStartPage ==0 || W25xFLASH_AddrTab.APPStartPage >63) 
    W25xFLASH_AddrTab.APPStartPage = 14; //������APP��ת��ַ,Ĭ�ϵ�14��FLASHҳ��0x08003800 ��ǰ14K 
 
  if(W25xFLASH_AddrTab.UpdateStatus[0]==0x5a)//bootload��Ҫ����
  {	
	fsize = W25xFLASH_AddrTab.BootloadSize[3]<<24 + W25xFLASH_AddrTab.BootloadSize[2]<<16 +	W25xFLASH_AddrTab.BootloadSize[1]<<8 + W25xFLASH_AddrTab.BootloadSize[0];
	if(fsize==0 ||fsize>65536) return;
	
    num1=fsize/FLASH_PAGE_SIZE;
    num2=fsize%FLASH_PAGE_SIZE;   
	addr = 	W25xFLASH_BlockSector(BlockBootload,SectorBootload); //��ʼ��ַ
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
 *Rom APP����
 *˵��:Boatload����ʱ���и��£���ÿ�ο���ʱ����
 */
void IAP_APPUpdate(void)
{
  uint8_t buf[FLASH_PAGE_SIZE]={0};  //ROMһҳ��1k��С
  uint8_t i,num1;
  uint16_t num2;
  uint32_t fsize=0,addr=0;
    //APP����ROM��FLASHҳ(��64ҳ),Ĭ��14ҳ��Ԥ��14k��boatload,ʣ��50k��app  
  if(W25xFLASH_AddrTab.APPStartPage ==0 || W25xFLASH_AddrTab.APPStartPage >63) 
    W25xFLASH_AddrTab.APPStartPage = 14; //������APP��ת��ַ,Ĭ�ϵ�14��FLASHҳ��0x08003800 ��ǰ14K 
 
  if(W25xFLASH_AddrTab.UpdateStatus[1]==0xA5)//APP��Ҫ����
  {
	fsize = W25xFLASH_AddrTab.APPSize[3]<<24 + W25xFLASH_AddrTab.APPSize[2]<<16 +	W25xFLASH_AddrTab.APPSize[1]<<8 + W25xFLASH_AddrTab.APPSize[0];
	if(fsize==0 ||fsize>65536) return;
	
    num1=fsize/FLASH_PAGE_SIZE;
    num2=fsize%FLASH_PAGE_SIZE;   
	addr = 	W25xFLASH_BlockSector(BlockAPP,SectorAPP); //��ʼ��ַ                                 
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









