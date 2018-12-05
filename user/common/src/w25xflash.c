       /* Includes ------------------------------------------------------------------*/
#include "w25xflash.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "iap.h"

#include "usart.h"
#include "delay.h"

//ϵͳ��Ϣ�Ƚṹ���
W25xFLASH_AddrDef W25xFLASH_AddrTab;

/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void W25xFLASH_WriteEnable(void)
{
	uint8_t cmd;
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	cmd=W25xFLASH_CMD_WEN;
	/*!< Send "Write Enable" instruction */
	/*##-2- Start the Full Duplex Communication process ########################*/
	HAL_SPI_Transmit(&hspi2,&cmd,1,200);
	/*##-3- Wait for the end of the transfer ###################################*/
	/*  Before starting a new communication transfer, you need to check the current
	    state of the peripheral; if it�s busy you need to wait for the end of current
	    transfer before starting a new one.
	    For simplicity reasons, this example is just waiting till the end of the
	    transfer, but application may perform other tasks while transfer operation
	    is ongoing. */
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY)
	{
	}
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
}
/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write operation has completed.
  * @param  None
  * @retval None
  */
void W25xFLASH_WaitForWriteEnd(void)
{
	uint8_t cmd,receiv;
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	cmd= W25xFLASH_CMD_RDSR1;
	/*!< Send "Read Status Register" instruction */
	HAL_SPI_Transmit(&hspi2,&cmd,1,200);
//  HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	cmd=W25xFLASH_CMD_DUMMY;
	/*!< Loop as long as the memory is busy with a write cycle */
	do
	{
		/*!< Send a dummy byte to generate the clock needed by the FLASH
		and put the value of the status register in W25xFLASH_Status variable */
		HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
		while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	}
	while ((receiv & 0x01) == SET); /* Write in progress */

	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
}
/**
  * @brief  Reads FLASH identification.
    W25Q16  ��ID:0xEF14
    W25Q32  ��ID:0xEF15
    W25Q64  ��ID:0xEF16
    W25Q128 ��ID:0xEF17
  * @param  None
  * @retval FLASH identification
  */
uint16_t W25xFLASH_ReadID(void)
{
	uint16_t Temp = 0;
	uint8_t cmd,receiv;
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	cmd = W25xFLASH_CMD_MFID;
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	cmd = W25xFLASH_CMD_DUMMY;
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	cmd = 0;
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	cmd = W25xFLASH_CMD_DUMMY;
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	Temp =receiv<<8;
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	Temp |=receiv;
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
  
  if(Temp == 0xEF14) Usart_Printf(&huart1,"Flash ID: W25Q16\n");
  else if(Temp == 0xEF15) Usart_Printf(&huart1,"Flash ID: W25Q32\n");
  else if(Temp == 0xEF16) Usart_Printf(&huart1,"Flash ID: W25Q64\n");
  else if(Temp == 0xEF17) Usart_Printf(&huart1,"Flash ID: W25Q128\n");
  else Usart_Printf(&huart1,"err: Flash ID:0x%2x\n",Temp);
	return Temp;
}


/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void W25xFLASH_ReadBuffer( uint32_t ReadAddr,uint8_t* pBuffer, uint16_t NumByteToRead)
{
	uint8_t cmd[4],receiv[4],*p=NULL;
	/*!< Select the FLASH: Chip Select low */
	cmd[0] = W25xFLASH_CMD_RD;
	cmd[1] = (ReadAddr & 0xFF0000) >> 16;
	cmd[2] = (ReadAddr& 0xFF00) >> 8;
	cmd[3] =  ReadAddr & 0xFF;
	p=malloc(NumByteToRead);//��������;
	memset(p,W25xFLASH_CMD_DUMMY,NumByteToRead);
	W25xFLASH_CS_LOW();
	/*!< Send "Read from Memory " instruction */
	HAL_SPI_TransmitReceive(&hspi2,cmd,receiv,4,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	HAL_SPI_TransmitReceive(&hspi2,p,pBuffer,NumByteToRead,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
	free(p);
}

/**
  * @brief  Erases the specified FLASH sector.
  * @param  SectorAddr: address of the sector to erase.
  * ��С������λΪ1������(4k)
  * @retval None
  */

void W25xFLASH_EraseOneSector(uint32_t SectorAddr)
{
	uint8_t cmd[4],receiv[4];
	cmd[0] = W25xFLASH_CMD_SCER;
	cmd[1] = (SectorAddr & 0xFF0000) >> 16;
	cmd[2] = (SectorAddr& 0xFF00) >> 8;
	cmd[3] =  SectorAddr & 0xFF;
	/*!< Send write enable instruction */
	W25xFLASH_WriteEnable();
	/*!< Sector Erase */
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	HAL_SPI_TransmitReceive(&hspi2,cmd,receiv,4,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
	/*!< Wait the end of Flash writing */
	W25xFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the specified FLASH block 32k.
  * @param  BlockAddr: address of the block to erase.
  * һ���Բ���1/2��(32K),=8��sector(����)
  * @retval None
  */

void W25xFLASH_EraseBlock32(uint32_t BlockAddr)
{
	uint8_t cmd[4],receiv[4];
	cmd[0] = W25xFLASH_CMD_BLER1;
	cmd[1] = (BlockAddr & 0xFF0000) >> 16;
	cmd[2] = (BlockAddr& 0xFF00) >> 8;
	cmd[3] =  BlockAddr & 0xFF;
	/*!< Send write enable instruction */
	W25xFLASH_WriteEnable();

	/*!< Sector Erase */
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	/*!< Send Block Erase instruction */
	HAL_SPI_TransmitReceive(&hspi2,cmd,receiv,4,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();

	/*!< Wait the end of Flash writing */
	W25xFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the specified FLASH block 64k.
  * @param  BlockAddr: address of the block to erase.
  * һ���Բ���1��(64K),=16 ��sector(����)
* @retval None
  */
void W25xFLASH_EraseBlock64(uint32_t BlockAddr)
{
	uint8_t cmd[4],receiv[4];
	cmd[0] = W25xFLASH_CMD_BLER2;
	cmd[1] = (BlockAddr & 0xFF0000) >> 16;
	cmd[2] = (BlockAddr& 0xFF00) >> 8;
	cmd[3] =  BlockAddr & 0xFF;
	/*!< Send write enable instruction */
	W25xFLASH_WriteEnable();

	/*!< Sector Erase */
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();
	/*!< Send Block Erase instruction */
	HAL_SPI_TransmitReceive(&hspi2,cmd,receiv,4,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();

	/*!< Wait the end of Flash writing */
	W25xFLASH_WaitForWriteEnd();
}

/**
  * @brief  Erases the entire FLASH.
  * @param  None
  * @retval None
  */
void W25xFLASH_EraseBulk(void)
{
	uint8_t cmd,receiv;
	cmd = W25xFLASH_CMD_CHER;
	W25xFLASH_WriteEnable();
	W25xFLASH_CS_LOW();
	/*!< Send Bulk Erase instruction  */
	HAL_SPI_TransmitReceive(&hspi2,&cmd,&receiv,1,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	W25xFLASH_CS_HIGH();
	W25xFLASH_WaitForWriteEnd();
}


/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * һ�����д��һҳ page=256bytes д��ǰ�����Ȳ���,�ú���û�в�������
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "W25xFLASH_PAGESIZE" value��o<256Bytes(a page,flash��?buff?����?�䨮D?)
  * @retval None
  */
void W25xFLASH_OnlyWritePage( uint32_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite)
{
	uint8_t cmd[4],receiv[4],*p=NULL;
	/*!< Select the FLASH: Chip Select low */
	cmd[0] = W25xFLASH_CMD_PGPR;
	cmd[1] = (WriteAddr & 0xFF0000) >> 16;
	cmd[2] = (WriteAddr& 0xFF00) >> 8;
	cmd[3] =  WriteAddr & 0xFF;
	p=malloc(NumByteToWrite);//
	memset(p,W25xFLASH_CMD_DUMMY,NumByteToWrite);
	/*!< Enable the write access to the FLASH */
	W25xFLASH_WriteEnable();
	/*!< Select the FLASH: Chip Select low */
	W25xFLASH_CS_LOW();

	HAL_SPI_TransmitReceive(&hspi2,cmd,receiv,4,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	HAL_SPI_TransmitReceive(&hspi2,pBuffer,p,NumByteToWrite,200);
	while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) { }
	/*!< Deselect the FLASH: Chip Select high */
	W25xFLASH_CS_HIGH();
	/*!< Wait the end of Flash writing */
	W25xFLASH_WaitForWriteEnd();
	free(p);
}

/**
  * @brief  Writes block of data to the FLASH. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  * д��ǰ�����Ȳ���,�ú���û�в�������
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH.
  * @retval None
  */
void W25xFLASH_OnlyWriteBuffer(uint32_t WriteAddr, uint8_t* pBuffer,  uint16_t NumByteToWrite)
{
	uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = WriteAddr % W25xFLASH_PageSize;  //flash ÿ�����д��1page
	count = W25xFLASH_PageSize - Addr;      //��?o����?��3��???����
	NumOfPage =  NumByteToWrite / W25xFLASH_PageSize;  //��??��������3
	NumOfSingle = NumByteToWrite % W25xFLASH_PageSize;  //����??��?

	if (Addr == 0) /*!< WriteAddr is W25xFLASH_PAGESIZE aligned 256��??��????��? */
	{
		if (NumOfPage == 0) /*!< NumByteToWrite < W25xFLASH_PAGESIZE */
		{
			W25xFLASH_OnlyWritePage(WriteAddr, pBuffer, NumByteToWrite);
		}
		else /*!< NumByteToWrite > W25xFLASH_PAGESIZE */
		{
			while (NumOfPage--)
			{
				W25xFLASH_OnlyWritePage(WriteAddr, pBuffer, W25xFLASH_PageSize);
				WriteAddr +=  W25xFLASH_PageSize;
				pBuffer += W25xFLASH_PageSize;
			}

			W25xFLASH_OnlyWritePage(WriteAddr, pBuffer, NumOfSingle);
		}
	}
	else /*!< WriteAddr is not W25xFLASH_PAGESIZE aligned  */
	{
		if (NumOfPage == 0) /*!< NumByteToWrite < W25xFLASH_PAGESIZE */
		{
			if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > W25xFLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				W25xFLASH_OnlyWritePage(WriteAddr, pBuffer, count);
				WriteAddr +=  count;
				pBuffer += count;
				W25xFLASH_OnlyWritePage(WriteAddr,pBuffer,  temp);
			}
			else
			{
				W25xFLASH_OnlyWritePage(WriteAddr,pBuffer,  NumByteToWrite);
			}
		}
		else /*!< NumByteToWrite > W25xFLASH_PAGESIZE */
		{
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / W25xFLASH_PageSize;
			NumOfSingle = NumByteToWrite % W25xFLASH_PageSize;

			W25xFLASH_OnlyWritePage(WriteAddr,pBuffer,  count);
			WriteAddr +=  count;
			pBuffer += count;

			while (NumOfPage--)
			{
				W25xFLASH_OnlyWritePage(WriteAddr, pBuffer, W25xFLASH_PageSize);
				WriteAddr +=  W25xFLASH_PageSize;
				pBuffer += W25xFLASH_PageSize;
			}

			if (NumOfSingle != 0)
			{
				W25xFLASH_OnlyWritePage( WriteAddr,pBuffer, NumOfSingle);
			}
		}
	}
}

/**
  * @brief  д�����������򷵻�0,���򷵻�����ֵ.
  */
// void flash_write_test(void)
// {
	// uint8_t *readBuf;
	// uint16_t flashId=0,i;
	// const uint8_t ASCII_test[504]=
	// {
		// 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*" ",0*/
		// 0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,/*"!",1*/
		// 0x00,0x00,0x08,0x00,0x04,0x00,0x0A,0x00,0x04,0x00,0x02,0x00,0x00,0x00,/*""",2*/
		// 0x10,0x01,0xD0,0x0F,0x3C,0x01,0x10,0x01,0xD0,0x0F,0x3C,0x01,0x00,0x00,/*"#",3*/
		// 0x00,0x00,0x38,0x06,0x44,0x08,0xFE,0x1F,0x84,0x08,0x18,0x07,0x00,0x00,/*"$",4*/
		// 0x78,0x00,0x84,0x0C,0x78,0x03,0xB0,0x07,0x4C,0x08,0x80,0x07,0x00,0x00,/*"%",5*/
		// 0x80,0x07,0x78,0x08,0xC4,0x09,0x38,0x06,0xC0,0x0B,0x40,0x08,0x00,0x04,/*"&",6*/
		// 0x00,0x00,0x12,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"'",7*/
		// 0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x03,0x0C,0x0C,0x02,0x10,0x00,0x00,/*"(",8*/
		// 0x00,0x00,0x02,0x10,0x0C,0x0C,0xF0,0x03,0x00,0x00,0x00,0x00,0x00,0x00,/*")",9*/
		// 0x00,0x00,0x90,0x00,0x60,0x00,0xF8,0x01,0x60,0x00,0x90,0x00,0x00,0x00,/*"*",10*/
		// 0x80,0x00,0x80,0x00,0x80,0x00,0xF0,0x07,0x80,0x00,0x80,0x00,0x80,0x00,/*"+",11*/
		// 0x00,0x00,0x00,0x24,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*",",12*/
		// 0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,/*"-",13*/
		// 0x00,0x00,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*".",14*/
		// 0x00,0x00,0x00,0x10,0x00,0x0E,0x80,0x01,0x60,0x00,0x1C,0x00,0x02,0x00,/*"/",15*/
		// 0xF0,0x03,0x08,0x04,0x04,0x08,0x04,0x08,0x08,0x04,0xF0,0x03,0x00,0x00,/*"0",16*/
		// 0x00,0x00,0x08,0x08,0x08,0x08,0xFC,0x0F,0x00,0x08,0x00,0x08,0x00,0x00,/*"1",17*/
		// 0x00,0x00,0x18,0x0C,0x04,0x0A,0x04,0x09,0xC4,0x08,0x38,0x0C,0x00,0x00,/*"2",18*/
		// 0x00,0x00,0x18,0x06,0x04,0x08,0x44,0x08,0x44,0x08,0xB8,0x07,0x00,0x00,/*"3",19*/
		// 0x00,0x00,0x80,0x01,0x60,0x01,0x10,0x09,0xFC,0x0F,0x00,0x09,0x00,0x00,/*"4",20*/
		// 0x00,0x00,0x7C,0x06,0x24,0x08,0x24,0x08,0x24,0x08,0xC4,0x07,0x00,0x00,/*"5",21*/
		// 0xF0,0x03,0x48,0x04,0x24,0x08,0x24,0x08,0x28,0x08,0xC0,0x07,0x00,0x00,/*"6",22*/
		// 0x00,0x00,0x0C,0x00,0x04,0x00,0x84,0x0F,0x74,0x00,0x0C,0x00,0x00,0x00,/*"7",23*/
		// 0x38,0x07,0xC4,0x08,0x44,0x08,0x44,0x08,0xC4,0x08,0x38,0x07,0x00,0x00,/*"8",24*/
		// 0x78,0x00,0x84,0x04,0x84,0x08,0x84,0x08,0x48,0x04,0xF0,0x03,0x00,0x00,/*"9",25*/
		// 0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x0C,0x60,0x0C,0x00,0x00,0x00,0x00,/*":",26*/
		// 0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x18,0x00,0x00,0x00,0x00,0x00,0x00,/*";",27*/
		// 0x00,0x00,0xC0,0x00,0x20,0x01,0x10,0x02,0x08,0x04,0x04,0x08,0x00,0x00,/*"<",28*/
		// 0x20,0x01,0x20,0x01,0x20,0x01,0x20,0x01,0x20,0x01,0x20,0x01,0x20,0x01,/*"=",29*/
		// 0x00,0x00,0x04,0x08,0x08,0x04,0x10,0x02,0x20,0x01,0xC0,0x00,0x00,0x00,/*">",30*/
		// 0x00,0x00,0x30,0x00,0x08,0x00,0x88,0x0D,0x48,0x0C,0x30,0x00,0x00,0x00,/*"?",31*/
		// 0xF0,0x03,0x08,0x04,0xE4,0x09,0x14,0x0A,0xF4,0x0B,0x08,0x0A,0xF0,0x05,/*"@",32*/
		// 0x00,0x08,0x00,0x0E,0xE0,0x09,0x18,0x01,0xE0,0x09,0x00,0x0E,0x00,0x08,/*"A",33*/
		// 0x08,0x08,0xF8,0x0F,0x88,0x08,0x88,0x08,0x88,0x08,0x70,0x07,0x00,0x00,/*"B",34*/
		// 0xE0,0x03,0x10,0x04,0x08,0x08,0x08,0x08,0x08,0x08,0x38,0x04,0x00,0x00,/*"C",35*/
	// };
	// flashId=W25xFLASH_ReadID();
	// if(flashId!=0xEF17)
	// {
		// Usart_Printf(&huart1,"flashId is ERR:%d\r\n",flashId);
		// return;
	// }
	// Usart_Printf(&huart1,"EraseSector...\r\n");

	// W25xFLASH_EraseOneSector(0);
	// Usart_Printf(&huart1,"WriteBuffer...\r\n");
	// W25xFLASH_OnlyWriteBuffer(0, ASCII_test, sizeof(ASCII_test));
	// Usart_Printf(&huart1,"ReadBuffer...\r\n");
	// readBuf=calloc(sizeof(ASCII_test),1);

	// W25xFLASH_ReadBuffer(0,readBuf,sizeof(ASCII_test));
	// Usart_Printf(&huart1,"Compare Buffer...\r\n");
	// for(i=0; i<sizeof(ASCII_test); i++)
	// {
		// if(readBuf[i] !=ASCII_test[i])
			// Usart_Printf(&huart1,"Compare err at %d\r\n",i);
	// }
	// Usart_Printf(&huart1,"Test Successed!\r\n");
	// free(readBuf);
// }

//----------------------------------------------------

/**
* ����Flashָ��sector��
* ������
		block: ��ʼblock�ţ�W25x64 ��ΧΪ0~127 ��128��block
		sec: ��ʼsector�ţ���ΧΪ0~15 ��16��sector (1 Block = 16 Sector)
		num�������ĸ���
*/
void W25xFLASH_EraseSector(uint8_t blc, uint8_t sec, uint8_t num)
{
	uint8_t i;
	if(sec+num >16) num = 16-sec;//����num��������
	for(i=0; i<num; i++)
	{
		W25xFLASH_EraseOneSector(W25xFLASH_BlockSector(blc,sec+i));//����1������
	}	
}

/**
* ����Flashָ��block��
* ������
		blc: ��ʼblock��ţ�W25x64 ��ΧΪ0~127 ��128��block
		num�������ĸ���
*/
void W25xFLASH_EraseBlock(uint8_t blc, uint8_t num)
{
	uint8_t i;
	if(blc+num >128) num = 128-blc;//����num��������
	for(i=0; i<num; i++)
	{
		W25xFLASH_EraseBlock64(W25xFLASH_BlockSector(blc+i,0));//����1����
	}	
}

/**
 * ���Flash���������ݣ����������while֮ǰӦ�ö�ȡ
 */
void W25xFLASH_GetConfig(void)
{
	W25xFLASH_ReadBuffer(AddrStatues,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));
}
/**
* ��дFlash����������
*/
void W25xFLASH_RenewConfig(void)
{
	W25xFLASH_EraseOneSector(AddrStatues);//�Ȳ�������д��
	W25xFLASH_OnlyWriteBuffer(AddrStatues,(uint8_t*)&W25xFLASH_AddrTab,sizeof(W25xFLASH_AddrDef));
}
/**
* дFlash��ID���汾�������ͨ��smartuartд��
*/
void W25xFLASH_SetProductId(uint8_t* pId,uint8_t versionBoot, uint8_t versionApp)
{
	// W25xFLASH_GetConfig();
	memcpy(W25xFLASH_AddrTab.ProductId, pId, strlen((char*)pId));
	W25xFLASH_AddrTab.version[0]=versionBoot;
	W25xFLASH_AddrTab.version[1]=versionApp;
	W25xFLASH_RenewConfig();
}

/*
��flash����sAddr��ַ��fsize���ȵ����� ������dAddr��ַ��
ǰ�᣺dAddr��Ҫ�Ȳ���
����˵����
	dAddr: Ŀ�ĵ�ַ
	sAddr:Դ��ַ
	fsize������
	ÿ���ƶ�512�ֽ�
*/
void W25xFLASH_MoveData(uint32_t dAddr, uint32_t sAddr, uint32_t fsize)
{
	uint8_t buf[512]={0};
	uint32_t tempLength=0;
	
	for(tempLength=0; tempLength< fsize; tempLength +=512)
	{
		W25xFLASH_ReadBuffer(sAddr +tempLength, buf, 512);
		W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, buf, 512);					
	}
	tempLength -=512;
	W25xFLASH_ReadBuffer(sAddr +tempLength, buf, fsize-tempLength);
	W25xFLASH_OnlyWriteBuffer(dAddr +tempLength, buf, fsize-tempLength);	
}


/*
�ճ̴�����
	*buf:"06101200xxx"	��ʽ��
	    ����(06/10)+ʱ��(12:00)+����(xxx)
	����flash�ĸ�ʽ��len+����(����Ϊlen)
	
	���أ��洢��λ��>=1
	    0 ʧ��
		
*/
/*
uint8_t W25xFLASH_WriteSchedule(uint8_t* buf)
{
	uint8_t i,j;
	uint8_t len;
	len = strlen((char*)buf);
	if(len<8) return 0;
	len -= 8; //���ݳ���
	for(i=0; i<2; i++)
	{
		for(j=0; j<8; j++)
		{
			if((W25xFLASH_AddrTab.ScheduleStatus[i] & (0x01<<j)) ==0) //�ĸ�λ���ǿ���
			{
				W25xFLASH_AddrTab.ScheduleStatus[i] |= (0x01<<j);  //��λ
				memcpy(W25xFLASH_AddrTab.SchedulTime[i*8+j], buf, 8);//i*8+j ȡֵΪ0~15��16��schedule
				W25xFLASH_AddrTab.ScheduleNum++;  //�ճ̼���
				
				buf[7] = len;//�����޸��ճ�buf�ṹ��len(1�ֽ�)+����(len���ֽ�)
				W25xFLASH_EraseSector(BlockSchedul,SectorSchedul+i*8+j,1); //������Ӧ�洢��	
				W25xFLASH_OnlyWriteBuffer(W25xFLASH_BlockSector(BlockSchedul,SectorSchedul+i*8+j),buf+7,len+1);//д��
				return (i*8+j)+1;
			}	
		}
	}
	return 0;
}
*/
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
