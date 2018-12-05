/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W25XFLASH_H
#define __W25XFLASH_H
#ifdef __cplusplus
extern "C" {
#endif

	/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "gpio.h"

//F_CS:PG6  SPI1_SCK:PB3   SPI1_MISO:PB4  SPI1_MOSI:PB5
	/* Exported macro ------------------------------------------------------------*/
#define W25xFLASH_CS_LOW()       HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET) /* Select FLASH: Chip Select pin low */
#define W25xFLASH_CS_HIGH()      HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET)   /* Deselect FLASH: Chip Select pin high */

	/* W25QXX SPI Flash ??��?���� */
#define W25xFLASH_CMD_WEN		(uint8_t)0x06        /* Write enable instruction */
#define W25xFLASH_CMD_WDIS		0x04        /* Write disable instruction */
#define W25xFLASH_CMD_RDSR1		0x05        /* Read Status Register1 instruction  */
#define W25xFLASH_CMD_RDSR2		0x35        /* Read Status Register2 instruction  */
#define W25xFLASH_CMD_WRSR		0x01        /* Write Status Register instruction */

#define W25xFLASH_CMD_BUSY		0x01        /*Status Register Write In Progress (WIP) flag ?y?��D��,?|D?o? ��?�̨�???a1*/

#define W25xFLASH_CMD_RD		0x03        /* Read from Memory instruction */
#define W25xFLASH_CMD_FRD	    0x0B        /* Fast Read from Memory instruction */

#define W25xFLASH_CMD_PGPR		0x02        /* Page Program instruction D�䨺y?Y*/

#define W25xFLASH_CMD_BLER2 	0xD8        /* Block Erase instruction  ?��2��3y 64K*/
#define W25xFLASH_CMD_BLER1  	0x52        /* Block Erase instruction  ?��2��3y 32K*/
#define W25xFLASH_CMD_SCER		0x20        /* Sector Erase instruction ����??2��3y 4K*/
#define W25xFLASH_CMD_CHER		0xC7        /* Chip Erase instruction   */
#define W25xFLASH_CMD_POWD		0xB9        /* Power down instruction   */

#define W25xFLASH_CMD_REPD  	0xAB        /* Release Power down instruction   */
#define W25xFLASH_CMD_ID		0xAB        /* Device ID instruction   */
#define W25xFLASH_CMD_MFID  	0x90        /* Manufacturer(???������) Device ID instruction   */
#define W25xFLASH_CMD_JEDECID	0x9F        /* JEDEC ID identification  ����??��?��?ID*/

#define W25xFLASH_CMD_DUMMY   0xFF        /* send a dummy data(?TD�쨺y?Y��?D��?a��y?Y) ?����??a��?��a?��   */

/* Private define ------------------------------------------------------------
* W25Q128 128MBit = 16MByte�����洢�ռ�16M
* ��ַ��Χ: 0x000000~ 0xFFFFFF 16M
* ��256��(Block), 1 Block= 64KByte
* ��4096������(Sector), 1 Sector= 4KByte 
* ��ϵ��1 Block = 16 Sector
* ��С������λΪ 1������
*/
#define W25xFLASH_PageSize     0x100     /* W25Q64 ҳ��С ÿ��д�����ֻ��д��һҳ ��1page=256bytes */
#define W25xFLASH_BlockSector(x,y)   ((uint32_t)x<<16 | (uint32_t)y<<12)  //x<=127Block ,y<=15Sector 
#define AddrStatues        0x000000

//��������(x,y)���壺
//ʹ�÷���Ϊ:W25xFLASH_BlockSector(Block,Sector)

#define BlockZK5X7_1        (0)
#define BlockZK5X7_2        (0)
#define BlockZKSymb_1       (0)
#define BlockZKSymb_2       (0)
#define BlockZKSymb_Usr     (0)
#define BlockZK7X14_1       (1)
#define BlockZK7X14_2       (5)
#define BlockBootload       (9)
#define BlockAPP            (10)
#define BlockSysWav         (11)
#define BlockUsrWav         (51)
#define BlockSchedul        (91)
#define BlockLeaveMSG       (92)
#define BlockTempBuff       (120) //flash���ڻ�������block��ַ

#define SectorZK5X7_1        (1)
#define SectorZK5X7_2        (2)
#define SectorZKSymb_1       (3)
#define SectorZKSymb_2       (4)
#define SectorZKSymb_Usr     (5)
#define SectorZK7X14_1       (0)
#define SectorZK7X14_2       (0)
#define SectorBootload       (0)
#define SectorAPP            (0)
#define SectorSysWav         (0)
#define SectorUsrWav         (0)
#define SectorSchedul        (0)
#define SectorLeaveMSG       (0)
#define SectorTempBuff       (0) //flash���ڻ�������block��ַ


// #define AddrZK5X7_1        (0,1)
// #define AddrZK5X7_2        (0,2)
// #define AddrZKSymb_1       (0,3)
// #define AddrZKSymb_2       (0,4)
// #define AddrZKSymb_Usr     (0,5)
// #define AddrZK7X14_1       (1,0)
// #define AddrZK7X14_2       (5,0)
// #define AddrBootload           (9,0)
// #define AddrAPP           (10,0)
// #define AddrSysWav         (11,0)
// #define AddrUsrWav         (51,0)
// #define AddrSchedul        (91,0)
// #define AddrLeaveMSG       (92,0)
// #define AddrTempBuff       (120,0) //flash���ڻ�������block��ַ



// #define AddrStatues           0x000000
// #define AddrZK5X7_1        W25xFLASH_BlockSector(0,1)
// #define AddrZK5X7_2        W25xFLASH_BlockSector(0,2)
// #define AddrZKSymb_1       W25xFLASH_BlockSector(0,3)
// #define AddrZKSymb_2       W25xFLASH_BlockSector(0,4)
// #define AddrZKSymb_Usr     W25xFLASH_BlockSector(0,5)
// #define AddrZK7X14_1       W25xFLASH_BlockSector(1,0)
// #define AddrZK7X14_2       W25xFLASH_BlockSector(5,0)
// #define AddrBootload       W25xFLASH_BlockSector(9,0)
// #define AddrAPP            W25xFLASH_BlockSector(10,0)
// #define AddrSysWav         W25xFLASH_BlockSector(11,0)
// #define AddrUsrWav         W25xFLASH_BlockSector(51,0)
// #define AddrSchedul        W25xFLASH_BlockSector(91,0)
// #define AddrLeaveMSG       W25xFLASH_BlockSector(92,0)
// #define AddrTempBuff       W25xFLASH_BlockSector(FlashTempArea)

typedef __packed struct   //packedָ������Ҫ����
{
//-----------------������Ϣ		
	//��ƷID ÿ5-10s����һ��
	uint8_t ProductId[12];  //201701010000
	//����汾,ֱ�������ִ�С
	//  [0]       [1]         
	//Bootload    APP
	uint8_t version[2];
	//2���ֽڵĳ���״̬��Ϣ,0x5A/0xA5��ʾ��Ҫ����
	//  [0]                       [1]         
	//Bootload�Ƿ����(0x5A)    APP�Ƿ����(0xA5)
	uint8_t UpdateStatus[2];	
	//�����ļ���С,˳��-��λ~��λ:[3]~[0]
	uint8_t BootloadSize[4]; 
	uint8_t APPSize[4];	
	//��������ʱ��APP��ROM��ַ,ROM(��64ҳ-stm32f0c8t6 ��64k)Ĭ��14ҳ��Ԥ��14k��bootload,ʣ��50k��app ����Ĭ��ֵ14
	uint8_t APPStartPage;  
		
//-------------�ļ������ԣ��Ƿ���ڣ�
	//�ֿ�״̬��Ϣ0-7λ��ʾ�ֿ��Ƿ���ڣ������1������Ϊ0��Ĭ�ϲ�����λ0xff
	//  [0]      [1]       [2]      [3]       [4]       [5]      [6]      [7]
	// ZK5X7_1,ZK5X7_2,ZKSymb_1,ZKSymb_2,ZKSymb_User,ZK7X14_1,ZK7X14_2 , null
	uint8_t ZKStatus;
	//ϵͳ��Ƶ״̬��Ϣ0~40bit:24Hours+16Msg �Ƿ���ڣ������1������Ϊ0
	uint8_t SYSWavStatus[5];		
	//�û���Ƶ״̬��Ϣ0~40bit:24Hours+16Msg �Ƿ���ڣ������1������Ϊ0
	uint8_t UserWavStatus[5];
//-----------------------ϵͳ������Ϣ
	//��ʾ����5λ
	//  [0]       [1]        [2]        [3]       [4]   
	//ʱ�����壬��Ϣ���壬��ʾ���֣���ʾЧ��     Ԥ��
	uint8_t DisplaySet[5];	
	//��Ϣ����3λ
	//��Ϣ��������
	// [1]      [2]       [3]    
	//��ʾ���� ��ʾ����s  Ԥ��
	uint8_t MessageSet[3];	
	//�ճ���������3λ
	// [1]     [2]       [3]   
	//��ʾ���� ��ʾ����s  Ԥ��
	uint8_t SchedualSet[3];
	//�������ã� [0] ϵͳ��������0/�û���������1 [1]��0/��1
	// [0]: bit0        bit1         bit2        bit3     bit4    bit5 ~ bit7
	// ��ʱ����    ϵͳ��ʾ����      null        null     null     null
	// [1]: bit0        bit1         bit2        bit3     bit4    bit5  ~ bit7
	//��ʱ����     ���ѿ���     �ճ����ѿ���   ���ӿ���  null     null
	uint8_t SoundSet[2];	
	
//----------δ������������Ϣ,��ʱ���ţ�ѭ���洢---------
	uint8_t VoiceNumbers;//��������������һ�κ����[1-7]
	uint8_t VoiceCounter;//����һ������λ�� [0~6]	
	//8�ֽ�ʱ���ʽ�� [0][1]  [2][3]  [4][5] [6][7]  /......
	//                 ��      ��       ʱ     ��
	uint8_t VoiceTime[7][8];	
//-------------�ճ̣����ò���ʽ��������ʱ������-------		
	//�ճ����� -�������16����ÿ����Ϣռһ������
	int8_t ScheduleNum;//ʧЧһ������1��	
	//8�ֽ�ʱ���ʽ�� [0][1]  [2][3]  [4][5] [6][7]  /......
	//                 ��      ��       ʱ     ��
	uint8_t SchedulTime[16][8];
	
} W25xFLASH_AddrDef;

extern W25xFLASH_AddrDef W25xFLASH_AddrTab;
	
uint16_t W25xFLASH_ReadID(void);
void W25xFLASH_ReadBuffer( uint32_t ReadAddr,uint8_t* pBuffer, uint16_t NumByteToRead);
	
void W25xFLASH_EraseSector(uint8_t blc, uint8_t sec, uint8_t num);	//����blc�����sec��ʼ��num������
void W25xFLASH_EraseBlock(uint8_t blc, uint8_t num);//������who��ʼ��num����
void W25xFLASH_EraseBulk(void);//������Ƭflash
	
void W25xFLASH_OnlyWritePage( uint32_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite);//д��ǰ�����Ȳ���
void W25xFLASH_OnlyWriteBuffer(uint32_t WriteAddr, uint8_t* pBuffer,  uint16_t NumByteToWrite);//д��ǰ�����Ȳ���

//---------------------------
void W25xFLASH_GetConfig(void);
void W25xFLASH_RenewConfig(void);
void W25xFLASH_SetProductId(uint8_t* pId,uint8_t versionBoot, uint8_t versionApp); 

void W25xFLASH_MoveData(uint32_t dAddr, uint32_t sAddr, uint32_t fsize);
//uint8_t W25xFLASH_WriteSchedule(uint8_t* buf);	
	
	
	
#ifdef __cplusplus
}
#endif
#endif /*__ spi_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
