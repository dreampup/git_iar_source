#include "stdlib.h"
#include "stdio.h"
#include "string.h"

/*****************************************
需要执行的函数的头文件添加到这里,并在function_table[]中
按照格式填写(void *)xxx,"void xxx(uint8_t tp,uint16_t frq);"
******************************************/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2s.h"
#include "rtc.h"
#include "crc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "display.h"
#include "wifi.h"
#include "w25xflash.h"
#include "temperature.h"
#include "delay.h"
#include "iap.h"
#include "wav.h"
#define STRING_ERR 0xff  //字符串处理错误

typedef struct
{
	void* func;			//函数指针
	char* name;		//函数名(查找串)	 
}tFuncTab;

typedef struct
{
	char num;
	char type;   //函数的返回值类型_0:void 1:数据 2:指针
	char* fname;
	char** param; //需初始化内存
}tFuncParam;

tFuncTab function_table[]=
{  
  //基本函数
  (void *)delay_ms,"void delay_ms(__IO uint32_t Delay)",
  (void *)Usart_Printf_Len,"void Usart_Printf_Len(UART_HandleTypeDef* uartHandle, uint8_t *buffer,uint16_t len)",
  
 //flash控制函数
  (void *)W25xFLASH_ReadID,"uint16_t W25xFLASH_ReadID(void)",
  (void *)W25xFLASH_ReadBuffer,"void W25xFLASH_ReadBuffer( uint32_t ReadAddr,uint8_t* pBuffer, uint16_t NumByteToRead)",
  (void *)W25xFLASH_SetProductId,"void W25xFLASH_SetProductId(uint8_t* pId,uint8_t versionBoot, uint8_t versionApp)",
  //显示函数 
  (void *)DisplayTest,"void DisplayTest(void)",
	(void *)DisplayInvert,"void DisplayInvert(uint8_t dir)",
  (void *)DisplayLightAdjust,"void DisplayLightAdjust(uint8_t level)",
  (void *)DisplayWordSet,"void DisplayWordSet(uint8_t tp,uint16_t frq)",
  (void *)DisplayShift,"void DisplayShift(uint8_t x,uint8_t y,uint8_t dir)",
	(void *)DisplayUpDown,"void DisplayUpDown(uint8_t xStart,uint8_t xEnd,uint8_t dir)",
  (void *)DisplayClean,"void DisplayClean(uint8_t x,uint8_t y, uint8_t ch)",
  (void *)DisplayString7X14,"void DisplayString7X14(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len)",
  (void *)DisplaySymbol,"void DisplaySymbol(uint8_t x,uint8_t *ch)",
	(void *)DisplayString5X7,"void DisplayString5X7(uint8_t xStar,uint8_t xEnd,uint8_t *chSrc,uint8_t len,uint8_t line)",
	
//IAP更新函数
  (void *)IAP_BoatloadUpdate,"void IAP_BoatloadUpdate(void)",    
  (void *)IAP_APPUpdate,"void IAP_APPUpdate(void)", 
  //RTC函数
  (void *)RTC_SetTime,"void RTC_SetTime(uint8_t *pDate,uint8_t* pTime)", //RTC_SetTime("1703293","151620")
  (void *)RTC_ResetAlarm,"void RTC_ResetAlarm(void)", 
  (void *)RTC_StopAlarm,"void RTC_StopAlarm(void)", 
  //温湿度函数
  (void *)printf_TH,"void printf_TH(void)",
  //光照传感器
  (void *)LightTest,"void LightTest(void)",
  //wav函数
  (void *)Wav_info,"uint32_t Wav_info(void)", 
  (void *)Wav_Play,"void Wav_Play(uint8_t deep)", 
  
  //wifi函数
  (void *)WiFi_Send,"void WiFi_Send(uint8_t *fmt, ...)",
  (void *)WiFi_Status,"uint8_t WiFi_Status(void)", 
  (void *)WiFi_SmartConfig,"uint8_t WiFi_SmartConfig(void)", 
  (void *)WiFi_StartTCPConnect,"uint8_t WiFi_StartTCPConnect(uint8_t* server_addr, uint16_t server_port)", 
  (void *)WiFi_TCPSend,"uint8_t WiFi_TCPSend(uint8_t* msg, uint16_t len)",
  (void *)WiFi_EndConnect,"uint8_t WiFi_EndConnect(void)",  
  (void *)WiFi_UpdateFile,"void WiFi_UpdateFile(uint8_t *fType, uint8_t *fName)",  

};


//去除首位空格
char* strtrim(char*str)
{
	int i = 0;
	char *begin = str;
	while(*str++ != '\0')
	{
		if(*str != ' ')	break;
		else begin++;
	}
	for(i = strlen(str)-1; i >= 0;  i--)
	{
		if(*(str+i) != ' ') break;
		else *(str + i) = '\0';
	}
	return begin;
}
//显示得到的函数参数
void show_function(tFuncParam* fc)
{
	char i;
    Usart_Printf(&huart1,"You put:\r\n", fc->num);
    Usart_Printf(&huart1,"function name:%s\r\n", fc->fname);
	Usart_Printf(&huart1,"param num:%d\r\n", fc->num);
	Usart_Printf(&huart1,"return type:%d\r\n", fc->type);
	
	for (i = 0; i < fc->num; i++)
	{
		Usart_Printf(&huart1,"param[%d]:%s\r\n", i, fc->param[i]);
	}
}
void free_function(tFuncParam* fc)
{
	unsigned char i;
	if (fc->fname != NULL)
	{
		free(fc->fname);
//		Usart_Printf(&huart1,"free fname\r\n");
	}
	for (i = 0; i < fc->num; i++)
	{
		if (fc->param[i] != NULL)
		{
			free(fc->param[i]);
	//		Usart_Printf(&huart1,"free param[%d]\r\n", i);
		}
	}
	free(fc->param);
}

/**
 * 返回值  0：分析成功ok
 *      0xff:STRING_ERR字符串处理错误
 */
unsigned char smart_uart_analyse(tFuncParam *fcp,char*str)
{
	unsigned char i=0;
	char *p, *buf;

	fcp->num = 0;
	fcp->type = 0;//默认返回类型为void型
	buf = malloc(strlen(str)+1);
	strcpy(buf, str);
	p = strtok((char*)buf, "(");  //功能标志
	if (p == NULL) {
		free(buf);
		return STRING_ERR;
	}
	strtrim(p);
	fcp->fname = malloc(strlen(p) + 1);
	strcpy(fcp->fname, p);

	while(1)
	{
		p = strtok(NULL, "(,);"); //..这里可用补充二级函数
		if (p != NULL)
		{
			fcp->num ++; //记录参数个数  
		}
		else break;
	} 		

	fcp->param =(char **) malloc(fcp->num *sizeof(char *)); //参数个数 32位的申请的地址位4字节存储
	strcpy(buf, str);
	i = 0;
	p = strtok((char*)buf, "(");  //功能标志
	while (1)
	{
		p = strtok(NULL, "(,);");
		if (p != NULL)
		{
			strtrim(p);
			fcp->param[i]= malloc(strlen(p) + 1);
			strcpy(fcp->param[i],p);	
			i++;
		}
		else break;
	}
	free(buf);
//	show_function(fcp);
	return 0;
}
/**
 * 返回值   0：没有找到可执行函数 
 *         >0：找到的函数ID值>0  
 *       0xff:STRING_ERR 字符串处理错误
*/
unsigned char smart_uart_search(tFuncParam* fc)
{
	char *buf, *p;
	unsigned char i, j;
	for(i=0;i<sizeof(function_table)/sizeof(tFuncTab);i++)
	{
		buf = malloc(strlen(function_table[i].name)+1);
		strcpy(buf, function_table[i].name);
		p=strtok(buf,"(");   //先检查返回值是否为字符串（*指针类型）
		if (p == NULL)
		{
			free(buf);
			return STRING_ERR;  //函数配置function_table不正确
		}
		for (j = 0; j < strlen(p); j++)
		{
			if (p[j] == '*') break;
		}
		//判断函数的返回值
		if(j<strlen(p)) fc->type=2;  //该函数有返回值,且为指针类型
		else fc->type = 0; //清零

		 //buf已经被截取
		p=strtok((char*)buf," *");  //取列表中函数名称和返回值类型
		if (p == NULL)
		{
			free(buf);
			return STRING_ERR;
		}
		strtrim(p);
		if((strcmp(p,"void")!=0) && (fc->type==0)) fc->type=1;  //为数据型

		p=strtok(NULL," *");  //获得列表中的函数名
		if (p == NULL)
		{
			free(buf);
			return STRING_ERR;
		}	
		strtrim(p);
		
		if (strcmp(fc->fname, p) == 0)
		{
			free(buf);
			break;
		}
		free(buf);
	}
	if (i > sizeof(function_table) / sizeof(tFuncTab) - 1)
	{
//		Usart_Printf(&huart1,"cannot found function!\r\n", i);
		return 0;//没有找到该函数	
	}
	return i+1;//记录函数的ID值
}
void smart_uart_exe(tFuncParam* fc,unsigned char idFc)
{
	unsigned char i;
	unsigned long re;
	unsigned long *p;//参数转录指针
	p = malloc(fc->num);

	for (i = 0; i < fc->num; i++)
	{
		if (fc->param[i][0] == '\"') //参数为字符串 字符如"hello"
		{
			p[i] =(unsigned long) (fc->param[i] + 1); //跳过第一个'\"' p[i]为地址值
			fc->param[i][strlen(fc->param[i]) - 1] = 0;//将末尾'\"'修改为'\0'结束
		}
		else  //参数为十六进制 字符如0x8000; 或参数为数字 
		{
			p[i] = strtol(fc->param[i],NULL,10);
		}
	}

	////参数统计 数值型个数m，指针型个数n
	//param_data = malloc(m* sizeof(unsigned long));
	//param_point = malloc(m * sizeof(unsigned long));
	switch (fc->num)
	{
	case 0:re = (*(unsigned long(*)())function_table[idFc].func)(); break;
	case 1:re = (*(unsigned long(*)())function_table[idFc].func)(p[0]); break;
	case 2:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1]); break;
	case 3:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2]); break;
	case 4:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2], p[3]); break;
	case 5:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2], p[3], p[4]); break;
	case 6:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2], p[3], p[4], p[5]); break;
	case 7:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]); break;
	case 8:re = (*(unsigned long(*)())function_table[idFc].func)(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]); break;
	}
	//fc->type返回值类型  0:void 1 : 数据 2 : 指针
	if (fc->type==1)//数据
	{
		Usart_Printf(&huart1,"return value:%ld",re);
	}
	else if (fc->type == 2)//指针地址值
	{
		Usart_Printf(&huart1,"return value:%s", (char*)re);
	}	
	free(p);
 }

 unsigned char *uartBuf = NULL;  
 tFuncParam uart_fcp;
void smart_uart_scanf(void)
{
  char i;

  if(Usart_Scanf(&recBuff1,&uartBuf,500)>0)
  {		
	smart_uart_analyse(&uart_fcp,(char*) uartBuf);
	show_function(&uart_fcp);
	i=smart_uart_search(&uart_fcp);
	if (i > 0)
	{
      Usart_Printf(&huart1,"Function id:%d\r\n", i);
      smart_uart_exe(&uart_fcp,i-1);
	}
    free_function(&uart_fcp);
    FreeScanBuf(&uartBuf);
  }
}
