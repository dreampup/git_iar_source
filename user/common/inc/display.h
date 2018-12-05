#ifndef __DISPLAY_H
#define __DISPLAY_H
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"


#define CMD_SYS_DIS     0x00   //关闭时钟  默认
#define CMD_SYS_EN      0x01     
#define CMD_LED_OFF     0x02   //关闭LED扫描周期   默认
#define CMD_LED_ON      0x03
#define CMD_BLINK_OFF   0x08   //关闭LED闪烁  默认
#define CMD_BLINK_ON    0x09
#define CMD_SLAVE_MODE  0x10   //作为从设备
#define CMD_MASTER_RC   0x18  //内部时钟主设备   默认
#define CMD_MASTER_EXT  0x1C  //外部时钟主设备 
#define CMD_COM_N8      0x20    //N-MOS驱动 8com口  COM低电平，ROW高电平   默认
#define CMD_COM_N16     0x24    //N-MOS驱动 16com口  COM低电平，ROW高电平 
#define CMD_COM_P8      0x28    //P-MOS驱动 8com口  COM高电平，ROW低电平 
#define CMD_COM_P16     0x2C    //P-MOS驱动 16com口  COM高电平，ROW低电平 
#define CMD_PWM_DUTY1   0xA0    //PWM 1/16占空比
#define CMD_PWM_DUTY2   0xA1    //PWM 2/16占空比
#define CMD_PWM_DUTY3   0xA2    //PWM 3/16占空比
#define CMD_PWM_DUTY4   0xA3    //PWM 4/16占空比
#define CMD_PWM_DUTY5   0xA4    //PWM 5/16占空比
#define CMD_PWM_DUTY6   0xA5    //PWM 6/16占空比uint8_t CMD_PWM_DUTY  0xA0    //PWM 1/16占空比
#define CMD_PWM_DUTY7   0xA6    //PWM 7/16占空比uint8_t CMD_PWM_DUTY  0xA0    //PWM 1/16占空比
#define CMD_PWM_DUTY8   0xA7    //PWM 8/16占空比
#define CMD_PWM_DUTY9   0xA8    //PWM 9/16占空比
#define CMD_PWM_DUTY10  0xA9    //PWM 10/16占空比
#define CMD_PWM_DUTY11  0xAA    //PWM 11/16占空比
#define CMD_PWM_DUTY12  0xAB    //PWM 12/16占空比
#define CMD_PWM_DUTY13  0xAC    //PWM 13/16占空比
#define CMD_PWM_DUTY14  0xAD    //PWM 14/16占空比
#define CMD_PWM_DUTY15  0xAE    //PWM 15/16占空比
#define CMD_PWM_DUTY16  0xAF    //PWM 16/16占空比  最亮     默认 

#define CMD_MODE ((uint8_t)0x04)
#define WR_MODE  ((uint8_t)0x05)   //包括读写模式
#define RD_MODE  ((uint8_t)0x06)

#define DIS_MAX 65

//配置: 阴码 逐列式 逆向(低位在前) 十六进制 点阵14 像素大小10   代码每行点阵14，索引每行14
//字体：Terminal 字宽14 字高14 
// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~

void DisplayConfig(void);
void DisplayLightAdjust(uint8_t level);
void DisplayTest(void);
void DisplayWordSet(uint8_t tp,uint16_t frq);
void DisplayOneWord(uint8_t x,uint8_t *buf,uint8_t len );
void DisplayClean(uint8_t x,uint8_t y, uint8_t ch);
void DisplayShift(uint8_t x,uint8_t y,uint8_t dir);
void DisplayInvert(uint8_t dir);

void DisplayUpOrDown(uint8_t xStart,uint16_t *buf,uint8_t len,uint8_t dir);
void DisplayUpDown(uint8_t xStart,uint8_t xEnd,uint8_t dir);
void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len);

void DisplayOneNumber(uint8_t xStart,uint8_t *buf,uint8_t len,uint8_t line);
void DisplayString5X7(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len,uint8_t line);

void DisplaySymbol(uint8_t x,uint8_t *ch);

void DisplayTime(void );
void DisplayMsg(uint8_t* msg);


#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/