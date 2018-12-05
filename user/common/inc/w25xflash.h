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

	/* W25QXX SPI Flash ??á?±í */
#define W25xFLASH_CMD_WEN		(uint8_t)0x06        /* Write enable instruction */
#define W25xFLASH_CMD_WDIS		0x04        /* Write disable instruction */
#define W25xFLASH_CMD_RDSR1		0x05        /* Read Status Register1 instruction  */
#define W25xFLASH_CMD_RDSR2		0x35        /* Read Status Register2 instruction  */
#define W25xFLASH_CMD_WRSR		0x01        /* Write Status Register instruction */

#define W25xFLASH_CMD_BUSY		0x01        /*Status Register Write In Progress (WIP) flag ?y?úD′,?|D?o? ×?μí???a1*/

#define W25xFLASH_CMD_RD		0x03        /* Read from Memory instruction */
#define W25xFLASH_CMD_FRD	    0x0B        /* Fast Read from Memory instruction */

#define W25xFLASH_CMD_PGPR		0x02        /* Page Program instruction D′êy?Y*/

#define W25xFLASH_CMD_BLER2 	0xD8        /* Block Erase instruction  ?é2á3y 64K*/
#define W25xFLASH_CMD_BLER1  	0x52        /* Block Erase instruction  ?é2á3y 32K*/
#define W25xFLASH_CMD_SCER		0x20        /* Sector Erase instruction éè??2á3y 4K*/
#define W25xFLASH_CMD_CHER		0xC7        /* Chip Erase instruction   */
#define W25xFLASH_CMD_POWD		0xB9        /* Power down instruction   */

#define W25xFLASH_CMD_REPD  	0xAB        /* Release Power down instruction   */
#define W25xFLASH_CMD_ID		0xAB        /* Device ID instruction   */
#define W25xFLASH_CMD_MFID  	0x90        /* Manufacturer(???ìéì) Device ID instruction   */
#define W25xFLASH_CMD_JEDECID	0x9F        /* JEDEC ID identification  ±è??è?μ?ID*/

#define W25xFLASH_CMD_DUMMY   0xFF        /* send a dummy data(?TD§êy?Y￡?Dé?aêy?Y) ?éò??aè?òa?μ   */

/* Private define ------------------------------------------------------------
* W25Q128 128MBit = 16MByte，即存储空间16M
* 地址范围: 0x000000~ 0xFFFFFF 16M
* 共256块(Block), 1 Block= 64KByte
* 共4096个扇区(Sector), 1 Sector= 4KByte 
* 关系：1 Block = 16 Sector
* 最小擦除单位为 1个扇区
*/
#define W25xFLASH_PageSize     0x100     /* W25Q64 页大小 每次写入最多只能写入一页 即1page=256bytes */
#define W25xFLASH_BlockSector(x,y)   ((uint32_t)x<<16 | (uint32_t)y<<12)  //x<=127Block ,y<=15Sector 
#define AddrStatues        0x000000

//功能区域(x,y)定义：
//使用方法为:W25xFLASH_BlockSector(Block,Sector)

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
#define BlockTempBuff       (120) //flash用于缓冲区的block地址

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
#define SectorTempBuff       (0) //flash用于缓冲区的block地址


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
// #define AddrTempBuff       (120,0) //flash用于缓冲区的block地址



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

typedef __packed struct   //packed指定不需要对齐
{
//-----------------基本消息		
	//产品ID 每5-10s连接一次
	uint8_t ProductId[12];  //201701010000
	//软件版本,直接是数字大小
	//  [0]       [1]         
	//Bootload    APP
	uint8_t version[2];
	//2个字节的程序状态信息,0x5A/0xA5表示需要更新
	//  [0]                       [1]         
	//Bootload是否更新(0x5A)    APP是否更新(0xA5)
	uint8_t UpdateStatus[2];	
	//升级文件大小,顺序-高位~低位:[3]~[0]
	uint8_t BootloadSize[4]; 
	uint8_t APPSize[4];	
	//程序启动时，APP的ROM地址,ROM(共64页-stm32f0c8t6 共64k)默认14页即预留14k给bootload,剩余50k给app 这里默认值14
	uint8_t APPStartPage;  
		
//-------------文件完整性（是否存在）
	//字库状态信息0-7位表示字库是否存在，如存在1，否则为0，默认擦除后位0xff
	//  [0]      [1]       [2]      [3]       [4]       [5]      [6]      [7]
	// ZK5X7_1,ZK5X7_2,ZKSymb_1,ZKSymb_2,ZKSymb_User,ZK7X14_1,ZK7X14_2 , null
	uint8_t ZKStatus;
	//系统音频状态信息0~40bit:24Hours+16Msg 是否存在，如存在1，否则为0
	uint8_t SYSWavStatus[5];		
	//用户音频状态信息0~40bit:24Hours+16Msg 是否存在，如存在1，否则为0
	uint8_t UserWavStatus[5];
//-----------------------系统配置信息
	//显示设置5位
	//  [0]       [1]        [2]        [3]       [4]   
	//时间字体，消息字体，显示布局，显示效果     预留
	uint8_t DisplaySet[5];	
	//消息设置3位
	//消息提醒配置
	// [1]      [2]       [3]    
	//提示次数 提示周期s  预留
	uint8_t MessageSet[3];	
	//日程提醒配置3位
	// [1]     [2]       [3]   
	//提示次数 提示周期s  预留
	uint8_t SchedualSet[3];
	//声音配置： [0] 系统配置声音0/用户个性声音1 [1]关0/开1
	// [0]: bit0        bit1         bit2        bit3     bit4    bit5 ~ bit7
	// 报时语言    系统提示语音      null        null     null     null
	// [1]: bit0        bit1         bit2        bit3     bit4    bit5  ~ bit7
	//报时开关     提醒开关     日程提醒开关   闹钟开关  null     null
	uint8_t SoundSet[2];	
	
//----------未读留言语言消息,按时间存放，循环存储---------
	uint8_t VoiceNumbers;//留言总数，听过一次后清空[1-7]
	uint8_t VoiceCounter;//最新一条留言位置 [0~6]	
	//8字节时间格式： [0][1]  [2][3]  [4][5] [6][7]  /......
	//                 月      日       时     分
	uint8_t VoiceTime[7][8];	
//-------------日程，采用插入式，按日期时间排序-------		
	//日程设置 -最多设置16条，每条消息占一个扇区
	int8_t ScheduleNum;//失效一条减少1条	
	//8字节时间格式： [0][1]  [2][3]  [4][5] [6][7]  /......
	//                 月      日       时     分
	uint8_t SchedulTime[16][8];
	
} W25xFLASH_AddrDef;

extern W25xFLASH_AddrDef W25xFLASH_AddrTab;
	
uint16_t W25xFLASH_ReadID(void);
void W25xFLASH_ReadBuffer( uint32_t ReadAddr,uint8_t* pBuffer, uint16_t NumByteToRead);
	
void W25xFLASH_EraseSector(uint8_t blc, uint8_t sec, uint8_t num);	//擦除blc块的以sec开始的num个扇区
void W25xFLASH_EraseBlock(uint8_t blc, uint8_t num);//擦除以who开始的num个块
void W25xFLASH_EraseBulk(void);//擦除整片flash
	
void W25xFLASH_OnlyWritePage( uint32_t WriteAddr,uint8_t* pBuffer, uint16_t NumByteToWrite);//写入前必须先擦除
void W25xFLASH_OnlyWriteBuffer(uint32_t WriteAddr, uint8_t* pBuffer,  uint16_t NumByteToWrite);//写入前必须先擦除

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
