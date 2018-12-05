/**
  ******************************************************************************
  * @file    hal_wifi.c
  * @author  Luo
  * @version V1.0.1
  * @date    16-February-2016
  * @brief   wifi function configue file.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "delay.h"
#include "wifi.h"
#include "usart.h"
#include "stdarg.h"
#include "w25xflash.h"
#include "cJSON.h"
#include "crc.h"
#include "comment.h"
#include "gpio.h"
#include "display.h"
//--通用返回
const uint8_t WiFiReturn_OK[]= {"OK\r\n"};
const uint8_t WiFiReturn_ERR[]= {"ERROR\r\n"};
const uint8_t WiFiReturn_BUSY[]= {"busy p.."};
//--连接AP返回--上电也会出现
const uint8_t WiFiReturn_FAIL[]= {"FAIL\r\n"};
//成功入网返回
const uint8_t WiFiReturn_CONN[]= {"WIFI CONNECTED"};
const uint8_t WiFiReturn_GOTIP[]= {"WIFI GOT IP"};

const uint8_t WiFiReturn_DISCONN[]= {"WIFI DISCONNECT"};
//--网络状态
const uint8_t WiFiReturn_STATUS[]= {"STATUS:"};//紧接着后面的1位数字为2,3,4,5分别代表不同的网络状态见WiFi_Status()函数

//---airkiss配置返回
const uint8_t WiFiReturn_SMART_FIND[]= {"Smart get wifi info"};//大小写都可以，不同版本不一样
const uint8_t WiFiReturn_SMART_CONN[]= {"smartconfig connected wifi"};//大小写都可以，不同版本不一样
//---tcp返回
const uint8_t WiFiReturn_TCP_CLOSED[]= {"CLOSED\r\n"};
const uint8_t WiFiReturn_TCP_CONN[]= {"CONNECT\r\n"};
const uint8_t WiFiReturn_TCP_ALCONN[]= {"ALREADY CONNECTED\r\n"};

const uint8_t WiFiReturn_SEND_START[]= {"\r\n>"};//可以开始发送数据了
const uint8_t WiFiReturn_SEND_OK[]= {"SEND OK"}; //非透传模式时TCP发送完数据

const uint8_t WiFiReturn_TCP_GetData[]= {"+IPD"};    //非透传模式时TCP接收到数据：+IPD,19:asdfasdfasdfadfasdf


uint8_t dateStr[13]= {"2017-10-17 5"};
uint8_t timeStr[9]= {"20:01:50"};

uint8_t todayDayWeather[8]= {"多云"};
uint8_t todayNightWeather[8]= {"多云"};
uint8_t morrowDayWeather[8]= {"多云"};
uint8_t morrowNightWeather[8]= {"多云"};
uint8_t afterDayWeather[8]= {"多云"};
uint8_t afterNightWeather[8]= {"多云"};

uint8_t todayDayTemp[4]= {"22"};
uint8_t todayNightTemp[4]= {"12"};
uint8_t morrowDayTemp[4]= {"22"};
uint8_t morrowNightTemp[4]= {"12"};
uint8_t afterDayTemp[4]= {"22"};
uint8_t afterNightTemp[4]= {"12"};

uint8_t msgtext[30] = {0}; //文字消息buf，最多支持30字节,15汉字
uint8_t newWeather=0, newClock = 0;//消息标志位
uint8_t newMsg = 0;//是否得到及时消息 1:得到及时文字消息 2:得到及时语音消息 0:没有消息


void WiFi_RenewTime(void)
{
	uint8_t *buf=NULL;
	WiFi_TCPSend("{\"message\":{\"command\":\"get_clock\"}}",strlen("{\"message\":{\"command\":\"get_clock\"}}"));
	if(WiFi_DetectReceive(&buf)>0)
	{
		WiFi_JsonDecode(buf); //命令解析		
		FreeScanBuf(&buf);	
	}
}

void WiFi_UpdateFile(uint8_t *fType, uint8_t *fName)
{
	uint8_t *buf=NULL;
	uint8_t jsonSendBuf[50]={0};
	sprintf((char*)jsonSendBuf,"{\"message\":{\"update\":{\"%s\":\"%s\"}}}",fType,fName);
	WiFi_TCPSend(jsonSendBuf,strlen((char*)jsonSendBuf));
	if(WiFi_DetectReceive(&buf)>0)
	{
		WiFi_JsonDecode(buf); //命令解析
		FreeScanBuf(&buf);	
	}
}	
		

/*
将wifi接收的需要保存在w25xflash数据存储下来
*参数:
	ftype: 文件类型
	code: 代码类型
	fsize:文件长度
*返回值：> 0错误
		 0 成功
*/
uint8_t WiFi_UpdateFlash(uint8_t ftype, uint8_t *code, uint32_t fsize)
{
	uint8_t *buf=NULL,sendBUf[40]={0};
	uint8_t num=0;
	uint16_t len=0;
	uint32_t tempLength=0;
	uint32_t w25xFlashAdd=0;
	uint32_t w25xFlashAddTemp=W25xFLASH_BlockSector(BlockTempBuff,SectorTempBuff);
	
	W25xFLASH_EraseBlock(BlockTempBuff,8);//擦除flash的Temp区(8个block)
	//此时，TCP已经连接了
	//--开启透传模式\n
	WiFi_TCPModeSet(1);	//--发送开始命令
	Usart_Printf_Len(&huart1,"[update:0%]\n",12);  
	delay_ms(20);
	WiFi_SendLenght("{\"message\":{\"transfer\":0}}",26);
	ClearReceiveDef(&recBuff2);
	delay_ms(20);
	//先存于temp(w25xFlashAddTemp)地址，然后计算出crc校验码/或接受到了规定长度后，转存到特定地址，
	//最好不要直接放,怕网络不稳定等因素导致更新失败
	while(1)
	{
		len = Usart_Scanf(&recBuff2,&buf,2000);
		if(len !=512)
		{
			if(fsize == tempLength + len )//接收完成
			{
				W25xFLASH_OnlyWriteBuffer(w25xFlashAddTemp+tempLength,buf,len);	
				FreeScanBuf(&buf);
				break;
			}
			else{        
				FreeScanBuf(&buf);				
				Usart_Printf_Len(&huart1,"[update:err]\n",13);	
				delay_ms(20);				
				WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//服务器收到不是512的倍数数据的话会关闭打开的文件     
				delay_ms(20);				
				WiFi_TCPModeSet(0);	//普通模式
				return 0xF0;
			}
		}
		W25xFLASH_OnlyWriteBuffer(w25xFlashAddTemp+tempLength,buf,len);	
		FreeScanBuf(&buf);//释放内存
		tempLength +=len;	
		if(tempLength == fsize) break;
		sprintf((char*)sendBUf,"{\"message\":{\"transfer\":%d}}",tempLength);
		WiFi_SendLenght(sendBUf,strlen((char*)sendBUf));
		delay_ms(20);
		sprintf((char*)sendBUf,"[update:%d%%]\n",(uint8_t)(tempLength*100/fsize));
		Usart_Printf_Len(&huart1,sendBUf,strlen((char*)sendBUf));
		delay_ms(20);
	}
	
	//--擦除相应的地址区，进行写入操作
	if(ftype == 1)//bootload-Bootload
	{
		//--addr为crc32字符串		
		W25xFLASH_EraseBlock(BlockBootload,1);//擦除相应区域--1个block
		w25xFlashAdd = W25xFLASH_BlockSector(BlockBootload,SectorBootload);
		if(CRC32CheckAndMove(code, w25xFlashAdd, w25xFlashAddTemp, fsize)){//转录并校验成功			
			W25xFLASH_AddrTab.UpdateStatus[0]=0x5a; //置位标志	
			W25xFLASH_AddrTab.BootloadSize[3] = 0xff&(fsize>>24);
			W25xFLASH_AddrTab.BootloadSize[2] = 0xff&(fsize>>16);
			W25xFLASH_AddrTab.BootloadSize[1] = 0xff&(fsize>>8);
			W25xFLASH_AddrTab.BootloadSize[0] = 0xff&fsize;
			W25xFLASH_RenewConfig();//更新保存flash配置信息			
		}
		else { //--校验不成功		
			WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//服务器收到不是512的倍数数据的话会关闭打开的文件
			delay_ms(20);
			Usart_Printf_Len(&huart1,"[update:err]\n",13);
			delay_ms(20);
			WiFi_TCPModeSet(0);	//普通模式
			return 0xF1;	
		}
	}
	else if(ftype == 2)//app-User
	{
		W25xFLASH_EraseBlock(BlockAPP,1);//擦除相应区域--1个block
		w25xFlashAdd = W25xFLASH_BlockSector(BlockAPP,SectorAPP);	
		if(CRC32CheckAndMove(code, w25xFlashAdd, w25xFlashAddTemp, fsize)){//转录并校验成功
			W25xFLASH_AddrTab.UpdateStatus[1]=0xa5; //置位标志	
			W25xFLASH_AddrTab.APPSize[3] = 0xff&(fsize>>24);
			W25xFLASH_AddrTab.APPSize[2] = 0xff&(fsize>>16);
			W25xFLASH_AddrTab.APPSize[1] = 0xff&(fsize>>8);
			W25xFLASH_AddrTab.APPSize[0] = 0xff&fsize;
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else { //--校验不成功		
			WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//服务器收到不是512的倍数数据的话会关闭打开的文件
			delay_ms(20);
			Usart_Printf_Len(&huart1,"[update:err]\n",13);
			delay_ms(20);
			WiFi_TCPModeSet(0);	//普通模式
			return 0xF2;	
		}
	}
	else if(ftype == 3)//ziku
	{
		if(!memcmp(code,"ZK5X7_1",7))//ASCII码字库1
		{
			W25xFLASH_EraseSector(BlockZK5X7_1,SectorZK5X7_1,1);//擦除相应区域--1个sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=0x01;
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZK5X7_2",7))//ASCII码字库2
		{
			W25xFLASH_EraseSector(BlockZK5X7_2,SectorZK5X7_2,1);//擦除相应区域--1个sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<1);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZKSYMB_1",8))//符号码字库1
		{
			W25xFLASH_EraseSector(BlockZKSymb_1,SectorZKSymb_1,1);//擦除相应区域--1个sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_1,SectorZKSymb_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<2);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZKSYMB_2",8))//符号码字库2
		{
			W25xFLASH_EraseSector(BlockZKSymb_2,SectorZKSymb_2,1);//擦除相应区域--1个sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_2,SectorZKSymb_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<3);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZKSYMB_U",8))//涂鸦字库
		{
			W25xFLASH_EraseSector(BlockZKSymb_Usr,SectorZKSymb_Usr,11);//擦除相应区域--11个sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_Usr,SectorZKSymb_Usr);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<4);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZK7X14_1",8))//汉字字库1
		{
			W25xFLASH_EraseBlock(BlockZK7X14_1,4);//擦除相应区域--4个block
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK7X14_1,SectorZK7X14_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<5);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"ZK7X14_2",8))//汉字字库2
		{
			W25xFLASH_EraseBlock(BlockZK7X14_2,4);//擦除相应区域--4个block
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK7X14_2,SectorZK7X14_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<6);
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}

	}
	else if(ftype == 4)//wav：SCLK00-SCLK23/SSYS00-SSYS15  UCLK00-UCLK23/USYS00-USYS15
	{
		if(!memcmp(code,"SCLK",4))//24个用户报时声音
		{
			num = (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockSysWav+num,1);//擦除相应区域--1个block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockSysWav+num,SectorSysWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.SYSWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//更新保存flash配置信息			
		}
		else if(!memcmp(code,"SSYS",4))//16个系统声音
		{
			num = 24+ (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockSysWav+num,1);//擦除相应区域--1个block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockSysWav+num,SectorSysWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.SYSWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"UCLK",4))//24个用户报时声音
		{
			num = (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockUsrWav+num,1);//擦除相应区域--1个block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockUsrWav+num,SectorUsrWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.UserWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//更新保存flash配置信息	
		}
		else if(!memcmp(code,"USYS",4)) //16个用户系统声音
		{
			num = 24 + (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockUsrWav+num,1);//擦除相应区域--1个block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockUsrWav+num,SectorUsrWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
			W25xFLASH_AddrTab.UserWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//更新保存flash配置信息			
		}
	}
	else if(ftype == 5)//voice, 默认存储用户7个留言信息，循环缓冲,addr为时间信息月日时分[09300800]
	{
		
		if(++ W25xFLASH_AddrTab.VoiceCounter > 6)
		{
			W25xFLASH_AddrTab.VoiceCounter = 0;
			W25xFLASH_AddrTab.VoiceNumbers = 7; //数量达到最大值
		}
		
		num = W25xFLASH_AddrTab.VoiceCounter;
		memcpy(W25xFLASH_AddrTab.VoiceTime[num],code, 8);  //更新消息时间
		
		W25xFLASH_EraseBlock(BlockLeaveMSG+num*4,4);//擦除相应区域--4个block，规划7条
		w25xFlashAdd=W25xFLASH_BlockSector(BlockLeaveMSG+num*4,SectorLeaveMSG);
		W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//转录
		W25xFLASH_RenewConfig();//更新保存flash配置信息	
		newMsg =2;		
		/* 
		if(!memcmp(code,"LEAVE",5))
		{
			num = (code[5] -'0');
		
		}
		else if(!memcmp(code,"INSTANT",7))
		{
			//--就是该缓冲区,不需要缓冲
			newMsg =2;
		} 
		*/
	}	
	sprintf((char*)sendBUf,"{\"message\":{\"transfer\":%d}}",fsize);
	WiFi_SendLenght(sendBUf,strlen((char*)sendBUf));
	delay_ms(20); //服务器收到的2条指令之间至少要延时20ms
	Usart_Printf_Len(&huart1,"[update:100%]\n",14);	
	delay_ms(20);
	WiFi_TCPModeSet(0);	//普通模式
	return 0;	
}
/*
解析wifi接收的协议数据
参数说明：
	msg: wifi收到的数据
*/
void WiFi_JsonDecode(uint8_t *msg)
{
	uint8_t len,i;
	uint8_t fType=0,fCode[9]= {0}; //fType:1/2/3/4/5  bootload/app/ziku/wav/voice
	// uint8_t scheduleBuf[8+32]={0};//例如："06101200xxx" 格式：日期(06/10)+时间(12:00)+内容(xxx最大16汉字屏太小)
	uint32_t fSize=0;

	cJSON *js = NULL;
	cJSON *value = NULL;
	cJSON *subValue = NULL;
	cJSON *thrValue = NULL;

	js = cJSON_Parse((char*)msg);
	//-----1、得到天气信息-----掉电易失----------
	if (value = cJSON_GetObjectItem(js, "weather"))
	{
		Usart_Printf(&huart1,"renew weather!\n");
		subValue = cJSON_GetObjectItem(value, "today");
		if (subValue)
		{
			thrValue = cJSON_GetObjectItem(subValue, "day_weather");
			memcpy(todayDayWeather,thrValue->valuestring,strlen(thrValue->valuestring));

			thrValue = cJSON_GetObjectItem(subValue, "night_weather");
			memcpy(todayNightWeather, thrValue->valuestring, strlen(thrValue->valuestring));

			thrValue = cJSON_GetObjectItem(subValue, "night_temperature");
			memcpy(todayNightTemp, thrValue->valuestring, strlen(thrValue->valuestring));

			thrValue = cJSON_GetObjectItem(subValue, "day_temperature");
			memcpy(todayDayTemp, thrValue->valuestring, strlen(thrValue->valuestring));
			
		}
		subValue = cJSON_GetObjectItem(value, "tomorrow"); //此处为数组结构
		if (subValue)
		{
			thrValue = cJSON_GetObjectItem(subValue, "day_weather");
			memcpy(morrowDayWeather, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "night_weather");
			memcpy(morrowNightWeather, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "night_temperature");
			memcpy(morrowNightTemp, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "day_temperature");
			memcpy(morrowDayTemp, thrValue->valuestring, strlen(thrValue->valuestring));
		}

		subValue = cJSON_GetObjectItem(value, "after"); //此处为数组结构
		if (subValue)
		{
			thrValue = cJSON_GetObjectItem(subValue, "day_weather");
			memcpy(afterDayWeather, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "night_weather");
			memcpy(afterNightWeather, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "night_temperature");
			memcpy(afterNightTemp, thrValue->valuestring, strlen(thrValue->valuestring));
			thrValue = cJSON_GetObjectItem(subValue, "day_temperature");
			memcpy(afterDayTemp, thrValue->valuestring, strlen(thrValue->valuestring));
		}
		newWeather =1;
	}
	//-----2、得到时钟信息-----掉电易失----------
	else if(value = cJSON_GetObjectItem(js, "clock"))
	{
		Usart_Printf(&huart1,"renew clock!\n");
		subValue = cJSON_GetObjectItem(value, "date");
		memcpy(dateStr, subValue->valuestring, strlen(subValue->valuestring));
		subValue = cJSON_GetObjectItem(value, "time");
		memcpy(timeStr, subValue->valuestring, strlen(subValue->valuestring));
		newClock = 1;
	}
	//-----3、得到文字信息-----掉电易失----------
	else if(value = cJSON_GetObjectItem(js, "text"))
	{
		Usart_Printf(&huart1,"renew message!\n");
		subValue = cJSON_GetObjectItem(value, "message");//及时消息
		if (subValue)
		{
			thrValue = cJSON_GetObjectItem(subValue, "content");
			memcpy(msgtext, thrValue->valuestring, strlen(thrValue->valuestring));
			newMsg =1;
		}
		subValue = cJSON_GetObjectItem(value, "schedule");//也可以当成留言文字{"text":{"schedule": [{"time": "06101200", "content": "xxx"}, {"time": "08101100", "content": "xxx"}] }}
		if (subValue)
		{
			W25xFLASH_AddrTab.ScheduleNum=cJSON_GetArraySize(subValue); //数组深度
			for( i=0; i<W25xFLASH_AddrTab.ScheduleNum ;i++)
			{
				thrValue = cJSON_GetObjectItem(subValue+i, "time");
				memcpy(W25xFLASH_AddrTab.SchedulTime[i], thrValue->valuestring, strlen(thrValue->valuestring));
				
				// thrValue = cJSON_GetObjectItem(subValue+i, "content");
				// memcpy(scheduleBuf, thrValue->valuestring, strlen(thrValue->valuestring));
				
				// W25xFLASH_EraseSector(BlockSchedul,SectorSchedul+i,1); //擦除对应存储区	
				// W25xFLASH_OnlyWriteBuffer(W25xFLASH_BlockSector(BlockSchedul,SectorSchedul+i),scheduleBuf,strlen((char*)scheduleBuf));//写入	
				
				thrValue = cJSON_GetObjectItem(subValue+i, "content");
				W25xFLASH_EraseSector(BlockSchedul,SectorSchedul+i,1); //擦除对应存储区		
				W25xFLASH_OnlyWriteBuffer(W25xFLASH_BlockSector(BlockSchedul,SectorSchedul+i),(uint8_t*)thrValue->valuestring, strlen(thrValue->valuestring));//写入					
			}
			
			//W25xFLASH_WriteSchedule(scheduleBuf);
			W25xFLASH_RenewConfig();//保存flash配置信息			
		}		
	}
	//-----4、得到设置信息-----flash保存----------
	else if(value = cJSON_GetObjectItem(js, "setting"))
	{
		Usart_Printf(&huart1,"renew setting!\n");
		subValue = cJSON_GetObjectItem(value, "display");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //数组0
				W25xFLASH_AddrTab.DisplaySet[i] = thrValue->valueint;
			}
		}
		subValue = cJSON_GetObjectItem(value, "message");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //数组0
				W25xFLASH_AddrTab.MessageSet[i] = thrValue->valueint;
			}
		}
		subValue = cJSON_GetObjectItem(value, "schedule");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //数组0
				W25xFLASH_AddrTab.SchedualSet[i] = thrValue->valueint;
			}
		}	
		subValue = cJSON_GetObjectItem(value, "sound");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //数组0
				W25xFLASH_AddrTab.SoundSet[i] = thrValue->valueint;
			}
		}		
		W25xFLASH_RenewConfig();//保存flash配置信息
	}
	//-----5、得到升级信息-----flash保存----------
	else if(value = cJSON_GetObjectItem(js, "update"))
	{
		/*
		{
		"update":
		{
		"type":"bootload",
		"vision":1,
		"code":"04C11DB7"
		"size":[0,1024]
		}
		}
		*/
		Usart_Printf(&huart1,"update file!\n");
		subValue = cJSON_GetObjectItem(value, "type");
		if(subValue)
		{
			if(!memcmp(subValue->valuestring,"bootload",strlen(subValue->valuestring)))
			{
				subValue = cJSON_GetObjectItem(value, "vision");	//版本信息
				W25xFLASH_AddrTab.version[0] = subValue->valueint;	
				fType=1;				
			}
			else if(!memcmp(subValue->valuestring,"app",strlen(subValue->valuestring)))
			{
				subValue = cJSON_GetObjectItem(value, "vision");	//版本信息
				W25xFLASH_AddrTab.version[1] = subValue->valueint;	
				fType=2;
			}
			else if(!memcmp(subValue->valuestring,"ziku",strlen(subValue->valuestring)))
			{
				fType=3;
			}
			else if(!memcmp(subValue->valuestring,"wav",strlen(subValue->valuestring)))
			{
				fType=4;
			}
			else if(!memcmp(subValue->valuestring,"voice",strlen(subValue->valuestring)))
			{
				fType=5;
			}
		}		
		subValue = cJSON_GetObjectItem(value, "code");
		if(subValue)
		{
			memcpy(fCode,subValue->valuestring,strlen(subValue->valuestring));
		}
		subValue = cJSON_GetObjectItem(value, "size");
		if(subValue)
		{
			thrValue = cJSON_GetArrayItem(subValue, 0); //数组0,高2字节
			fSize = (thrValue->valueint)<<16; //高2字节
			thrValue = cJSON_GetArrayItem(subValue, 1); //数组1,低2字节
			fSize |= thrValue->valueint;      
		}		
	}

	//将JSON结构所占用的数据空间释放,只能释放js
	cJSON_Delete(js);   //只需要释放这一个
  
	if(fSize>0)
	{
		WiFi_UpdateFlash(fType,fCode,fSize);
	}
		
}


/**
  *非透传模式下，TCP通信收到数据，得到数据内容，即检测格式: +IPD,6:xxxxxx
  *参数：
      **jsonMsg:存储转换后的buf
  *返回值：json数据的长度
*/
uint8_t WiFi_DetectReceive(uint8_t **jsonMsg)
{
	uint8_t *numBuf = NULL, *buf=NULL;
	uint16_t i=0,j=0 ,num=0,len1=0,len2=0,len3=0;	
	for(i=0;i<4;i++)  //循环检测4次,目标是要
	{
		len1 = Usart_Scanf(&recBuff2,&buf,500);
		if(len1 > 0)  //如果服务器有数据返回，
		{
			len2 = CheckIsEquel(buf,len1,(uint8_t*)WiFiReturn_TCP_GetData,strlen((char*)WiFiReturn_TCP_GetData));//检测"+IPD"
			if(len2 >0)
			{
				len3 = CheckIsEquel(buf+len2-1,len1+1-len2,":",1);//检测":"
				if(len3 > 0)
				{					
					num = len3-5-1; //获得数字位数	
					numBuf = (uint8_t*)malloc(num+1); //获得字符串格式的数字
					for(j = 0; j< num; j++)
					{
						numBuf[j] = buf[len2+4 + j];
					}
					numBuf[j] = 0;
					num = atoi((char*)numBuf);  //转换后的 数字量
					free(numBuf);
					*jsonMsg = (uint8_t*)malloc(num+1); //
					for(j = 0; j< num; j++)
					{
						(*jsonMsg)[j] = buf[len2+len3-1 + j];
					}
					(*jsonMsg)[j] = 0;
          
					FreeScanBuf(&buf);
          
					break;
				}				
			}
			/* else //等待返回
			{
				ClearReceiveDef(&recBuff2);
			}	 */		
		}
    
		FreeScanBuf(&buf);
	}	
	return num;
}



/**
  *TCP通信处理函数 ，发送TCP通信协议，并处理服务器返回来的数据
  *参数：
      cmd:json格式的通信协议
	  len：格式长度
  *返回值：null
*/
/* void WiFi_TcpCommunication(uint8_t *cmd)
{
	uint8_t i=0;
	uint8_t *buf=NULL;
	//uint16_t len=0;
	//len = strlen((char*)cmd);
	//if(WiFi_TCPSend(cmd ,len) > 0 ) return;
	if(WiFi_TCPSend(cmd) > 0 ) return;
	//检测返回的数据要满足json格式的数据	
	if(WiFi_DetectReceive(&buf)>0)
	{
		WiFi_JsonDecode(buf); //命令解析
		FreeScanBuf(&buf);	
	}		
}
 */
/**
  *发送指定长度的AT指令
  *参数：
      msg:发送内容
	  len:长度
  *返回值：null
*/

void WiFi_SendLenght(uint8_t *msg, uint16_t len)
{
	Usart_Printf_Len(&huart2,msg,len);
}

/**
  *发送AT指令 不能超过100字节
  *参数：
      fmt:AT指令
  *返回值：null
*/
void WiFi_Send(uint8_t *fmt, ...)
{
	uint8_t buffer[151]= {0};
	uint8_t len;

	va_list arg_ptr; //Define convert parameters variable
	va_start(arg_ptr, fmt); //Init variable 指针指向fmt后的第一个参数
	len = vsnprintf((char*)buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
	va_end(arg_ptr);
	if(len > 0)
	{
		Usart_Printf_Len(&huart2,buffer,len);
	}	
	delay_ms(20);
}
/**
 *AT指令发送函数
 *参数：
    @ *cmd: AT指令
    @ dtimes: 指令发出后延时多少个500ms 即总共延时最多延时dtimes*500
    @ num :检测参数个数，<8：
    @ ...  可跟多个参数
 *返回:
    @失败：0 超时
    @成功：
      0x01 第一个参数检测到
      0x03 第1,2个参数检测到
      0x05 第1,3个参数检测到
      0x07 第1,2,3个参数检测到
      以此类推,只要其中一个检测到都会>0
*/
//uint8_t WiFi_ATCommand(uint8_t *cmd,uint8_t dtimes,uint8_t method,uint8_t num,...)
uint8_t WiFi_ATCommand(uint8_t *cmd,uint8_t dtimes,uint8_t num,...)
{
	uint8_t i=0,j=0,re=0;
	uint8_t *p=NULL,*buf=NULL;
	uint16_t len=0;
	va_list vl;
	WiFi_SendLenght(cmd,strlen((char*)cmd));          //发送指令
	for(j=0; j< dtimes; j++)
	{		
		len=Usart_Scanf(&recBuff2,&buf,500);
		if(len>0)  //检测返回
		{
			//Usart_Printf_Len(&huart1,buf,len ); //打印消息
			//----------问题出在这里，取参数只能循环1遍! 解决：va_start,va_end放在这里
			va_start(vl,num);
			for(i=0; i<num; i++)
			{
				if((p=va_arg( vl, uint8_t*))!=NULL)
				{
					if( re & (0x01<<i))
					{
						//已经检测过的就不需要再重复检测了
					}
					else if(CheckIsEquel(buf,len,p,strlen((char*) p))>0)
					{
						re |=(0x01<<i);
						// Usart_Printf(&huart1,"DETECTED:%s\n",p);
					}
				}
			}
			va_end(vl);			
		}   
		FreeScanBuf(&buf); //释放buf
//    if(method>0)  //所有参数都要检测到
//    {
		if( re == (1<< num)-1 )  //所有的参数都能检测到
			break; //跳出循环
//    }
//    else{
//        if( re >0 ) //检测到1个以上标志
//         break; //跳出循环
//    }
	}

	return re;
}


/**
 *获取WIFI网络状态
 *参数：无
 *返回: 状态值：
		‘2’-已连接AP，获得IP
		‘3’-已建立TCP/UDP连接
		‘4’断开网络连接
		‘5’-未连接AP
 */
uint8_t WiFi_Status(void)
{
	uint8_t j=0,status=0;
	uint8_t *buf=NULL;
	uint16_t len1=0,len2=0;

	WiFi_SendLenght(WiFiCmd_CIPSTATUS,strlen((char*)WiFiCmd_CIPSTATUS));  //发送指令
	for(j=0; j< 2; j++)
	{	
		len1=Usart_Scanf(&recBuff2,&buf,500);
		if(len1>0)  //检测返回
		{
			len2 = CheckIsEquel(buf,len1,(uint8_t*)WiFiReturn_STATUS,strlen((char*)WiFiReturn_STATUS));
			if(len2 >0)
			{
				status = buf[len2+6]; //取得状态字后面的数字，如返回值：STATUS:2
				FreeScanBuf(&buf); //释放buf
				break;
			}
		}
		FreeScanBuf(&buf); //释放buf
	}
	return status;
}

/**
 *WiFi上网检测，并配置TCP连接
 *参数：无    
 *返回:
      ‘3’TCP上服务器,接下来可以收发数据了
       '2’、‘4’连上了wifi 但没有TCP上服务器,外网问题
       '5’没有连上了wifi 
*/
uint8_t  WiFi_Setup(void)
{
  uint8_t state=0;
  ButtonState btStatus=BT_NON;
//  ButtonState btStatus=BT_NON;
//====================1、wifi状态检测=============================
//上电后，如果没有连接wifi,就等待用户配置网络
//用户有2个按键选择: 配网 和 不配网   
   state=WiFi_Status();
	 delay_ms(50);
   if(state == '2')//连上了wifi
   { 
       Usart_Printf(&huart1,"->wifi已连网\n");  
			 DisplayMsg("wifi已连网"); 
      
       if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//开启TCP连接
       {   
         	state = '3'; //TCP连接成功
					DisplayMsg("服务器已连接");
       }
   }
   else if(state == '3')
   { //已建立TCP连接
			Usart_Printf(&huart1,"服务器已连接!\n");  
			DisplayMsg("服务器已连接");
   }
   else if(state == '4')
   {//已断开TCP连接
       Usart_Printf(&huart1,"未连接服务器!\n");
			 DisplayMsg("未连接服务器");
       if(WiFi_StartTCPConnect(SERVICE,PORT)==0) //重新连接
       {   
        	state = '3'; //TCP连接成功
					DisplayMsg("服务器已连接");
       }
   }    
   else if(state == '5')
   {//未连wifi

       Usart_Printf(&huart1,"未连接WIFI,按下KEY1开始配置网络!\n");
	     DisplayMsg("未连WIFI,按1键配置网络");	
       //等待用户配置网络
		while(1)
		{
			btStatus = GPIO_ButtonDect();
			if(BT_SET ==btStatus)    //开始微信配置网络
			{
				if(WiFi_SmartConfig() ==0) //配网成功
				{               
					Usart_Printf(&huart1,"WIFI已连接!\n");  
					DisplayMsg("wifi已连网");
					state = '2';
					break; //成功配置网络
				}
			}
			else if(BT_SUB ==btStatus) //放弃微信配网
			{
				Usart_Printf(&huart1,"放弃联网,离线运行!\n");  
				DisplayMsg("放弃联网,离线运行");
				state = '5';
				break;
			}
			delay_ms(50);       
		}
		if(state == '2')
		{
			delay_ms(50);
			if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//开启TCP连接
			{   
				state = '3'; //TCP连接成功  
				DisplayMsg("服务器已连接");
			}
		} 

    }   
   return state;  
}

/**
 *WIFI模式配置，开启smart入网
 *参数：
    @ mode: 0,1,2
    @ dtimes: 指令发出后延时检测时间,单位ms
    @ num :检测参数个数，<8：
    @ ...  可跟多个参数
 *返回:
    0：配置成功
	>0:错误代码
 */
uint8_t WiFi_SmartConfig(void)
{
	uint8_t status=0;
	//---用户打开微信扫码关注,根据说明,长按按键,开始配网 在主函数中设置按键实现
	ClearReceiveDef(&recBuff2);
	//step1:开启Airkiss 自动配网模式 给用户20s(40*0.5s)时间配置,因为需要小于看门狗26s复位
	Usart_Printf(&huart1,"step1:开启Airkiss \n");
	DisplayString7X14(0,65,"开始微信配置wifi",16);
	status=WiFi_ATCommand(WiFiCmd_STARTSMART(3),40,1,WiFiReturn_SMART_CONN);
	DisplayString7X14(0,65,"关闭微信配网",12);
	Usart_Printf(&huart1,"step2:关闭Airkiss \n");
	WiFi_ATCommand(WiFiCmd_STOPSMART,4,1,WiFiReturn_OK); //不管成功配置与否都要停止
	if(status==0){
		//Usart_Printf(&huart1,"设置失败!\n");
		return 0xf3;
	}
	//step2:查询wifi状态
	status=WiFi_Status();//返回'2'表示连接上AP，得到IP
	if(status != '2'){
		Usart_Printf(&huart1,"wifi未成功连接!\n");
		DisplayString7X14(0,65,"接入WIFI失败",12);
		return 0xf4; //错误代码
	}
	Usart_Printf(&huart1,"wifi入网成功!\n");
	DisplayString7X14(0,65,"接入WIFI成功",12);	
// 目前wifi不支持 //step4:设置时区
//  status=WiFi_ATCommand(WiFiCmd_TimeAreaSet,5,1,WiFiReturn_OK);
//  if(status==0) return 0xf3; //错误代码
//  //获得网络时间
//  status=WiFi_GetTime();
//  if(status==0) return 0xf4; //错误代码
	return 0;
}
/**
 *开启TCP连接
 *参数：
    @ server_addr: 服务器地址
    @ server_port: 服务器端口

 *返回:
    0：配置成功
	>0:错误代码
 */
uint8_t WiFi_StartTCPConnect(uint8_t* server_addr, uint16_t server_port)
{
	uint8_t status=0;
	uint8_t tcpCmd[60]= {0};
  
	sprintf((char*)tcpCmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",server_addr, server_port);
	ClearReceiveDef(&recBuff2);
	//step1:TCP连接
	Usart_Printf(&huart1,"step1:开始TCP连接\n");
	DisplayMsg("开始TCP连接");
	status=WiFi_ATCommand(tcpCmd,6,3,WiFiReturn_TCP_CONN,WiFiReturn_TCP_ALCONN,WiFiReturn_OK);
	if(status <1) { //至少检测到2个
		Usart_Printf(&huart1,"连接失败\n");
		DisplayMsg("连接失败");
		return 0xf1; //错误代码,只要其中一个检测到都会>0
	}
	
	return 0;
}

/**
 *关闭TCP连接,停止发送
 *参数：

 *返回:
    0：配置成功
	>0:错误代码
 */
uint8_t WiFi_EndConnect(void)
{
	uint8_t status=0;
	//step1:关闭TCP连接
	ClearReceiveDef(&recBuff2);	
//	Usart_Printf(&huart1,"step1:关闭TCP连接\n");
	//这里检测close 或者error 如果本身就没有连接则返回的是ERROR
	status=WiFi_ATCommand(WiFiCmd_CIPCLOSE,3,2,WiFiReturn_TCP_CLOSED,WiFiReturn_ERR);
//	if(status==0) {
//		Usart_Printf(&huart1,"TCP关闭失败!\n");
//		DisplayMsg("TCP关闭失败");
//		return 0xf1; //错误代码
//	}
	return 0;
}
/**
 *TCP模式设置
 *参数：
    @ mod : 
          =0 普通发送
          =1 透传模式发送
 *返回:
    0：配置成功
	>0:错误代码
 */
uint8_t WiFi_TCPModeSet(uint8_t mode)  
{
  uint8_t status=0;
	ClearReceiveDef(&recBuff2);
	if( mode)
	{
		//step1:透传模式
		Usart_Printf(&huart1,"step1:开启透传模式\n");
		status=WiFi_ATCommand(WiFiCmd_CIPMODE(1),4,1,WiFiReturn_OK);
		if(status==0) {
		  Usart_Printf(&huart1,"设置失败!\n");
		  return 0xf1;
		}
		//step2:启动发送数据
		Usart_Printf(&huart1,"step2:启动发送数据\n");
		status=WiFi_ATCommand(WiFiCmd_CIPSEND,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);
		if(status == 0){
		  Usart_Printf(&huart1,"启动失败!\n");
		  return 0xf2;
		}
	}
	else
	{
		//step1:结束透传发送模式,
		Usart_Printf(&huart1,"step1:结束透传\n");
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(1200);//这里至少要延时1s下面的指令才可以发出

		//step2:普通模式
		Usart_Printf(&huart1,"step2:普通模式\n");
		status=WiFi_ATCommand(WiFiCmd_CIPMODE(0),4,1,WiFiReturn_OK);
		if(status==0) {
			Usart_Printf(&huart1,"设置失败!\n");
			return 0xf1;
		} 
    }
	return 0;
}

/**
 *发送配置,非透传模式中使用
 *参数：
 *返回:
    0：配置成功
	>0:错误代码
 */
uint8_t WiFi_InitConfig(void)
{
  uint8_t status=0;
	ClearReceiveDef(&recBuff2);	
	
	//step0:测试wifi是否处于指令模式
	Usart_Printf(&huart1,"1>AT测试\n");
	DisplayMsg("1>AT测试");
	status=WiFi_ATCommand(WiFiCmd_Test,2,1,WiFiReturn_OK);
	if(status==0) {	
		//step2:结束透传发送模式,
		Usart_Printf(&huart1,"AT无回应,尝试结束透传\n");
		DisplayMsg("AT无回应");
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(200);
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(1500);//这里至少要延时1s下面的指令才可以发出
		status=WiFi_ATCommand(WiFiCmd_Test,2,1,WiFiReturn_OK);
		if(status==0) 
		{
		  Usart_Printf(&huart1,"wifi硬件错误\n");
			DisplayMsg("wifi硬件错误");
			//--补充硬件复位控制指令
			
			return 0xf0; //错误代码 
		}
	}
	
  delay_ms(200);
	DisplayMsg("2>关闭SMART");
  //step1:测试wifi是否处于指令模式,防止配置时客户重启了
  //Usart_Printf(&huart1,"step1:尝试关闭smart配置\n");
	WiFi_ATCommand(WiFiCmd_STOPSMART,2,1,WiFiReturn_OK); //不管成功配置与否都要停止

  delay_ms(200);
	DisplayMsg("3>标准模式");
	//step1:设置wifi为station模式
	//Usart_Printf(&huart1,"step1:设置wifi为station模式\n");
	status=WiFi_ATCommand(WiFiCmd_CWMODE(1),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"设置失败!\n");
		return 0xf1; //错误代码 
	}
	delay_ms(200);
	DisplayMsg("4>自动连接AP");
	//step2:使能上电自动连接AP
//	Usart_Printf(&huart1,"step2:使能上电自动连接AP\n");
	status=WiFi_ATCommand(WiFiCmd_CWAUTOCONN(1),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"设置失败!\n");
		return 0xf2; //错误代码
	}
	delay_ms(200);
	DisplayMsg("5>关闭回显");
	//step3:关闭回显
//	Usart_Printf(&huart1,"step3:关闭回显\n");	
	status=WiFi_ATCommand(WiFiCmd_ATE(0),2,1,WiFiReturn_OK); 
	if(status==0) {
//		Usart_Printf(&huart1,"设置失败!\n");
		return 0xf3; //错误代码
	}
	delay_ms(200);
	DisplayMsg("6>普通传输");
	//step4:TCP普通模式
//	Usart_Printf(&huart1,"step4:TCP普通模式\n");
	WiFi_ATCommand(WiFiCmd_CIPMODE(0),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"设置失败!\n");
		return 0xf4; //错误代码
	}
	delay_ms(200);
	DisplayMsg("7>关闭TCP");
//	WiFi_ATCommand(WiFiCmd_CIPCLOSE,3,2,WiFiReturn_TCP_CLOSED,WiFiReturn_ERR);
	WiFi_EndConnect();//默认关1次TCP连接，防止多个连接
	delay_ms(200);
 return 0;	
}


/**
 *发送配置,非透传模式中使用
 *参数：
 *返回:
    0：配置成功
	>0:错误代码
 */

uint8_t WiFi_TCPSend(uint8_t* msg, uint16_t len)
{
	uint8_t status=0;
	uint8_t tcpCmd[20]= {0};
	ClearReceiveDef(&recBuff2);
	sprintf((char*)tcpCmd, "AT+CIPSEND=%d\r\n",len);	
	status=WiFi_ATCommand(tcpCmd,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);
	if(status == 0){
		Usart_Printf(&huart1,"启动失败!\n");
		return 0xf1;
	} 
	/* status=WiFi_ATCommand(msg,4,1,WiFiReturn_SEND_OK); //2、发送内容
	if(status == 0){
		Usart_Printf(&huart1,"发送内容失败!\n");
		return 0xf2;
	}  */
	Usart_Printf_Len(&huart2,msg,len);
	return 0;
}
  /*
uint8_t WiFi_TCPSend(uint8_t *fmt, ...)
{
	uint8_t status=0;
	uint8_t tcpCmd[20]= {0};	
	uint8_t buffer[151]= {0};
	uint8_t len;

	va_list arg_ptr; //Define convert parameters variable
	va_start(arg_ptr, fmt); //Init variable 指针指向fmt后的第一个参数
	len = vsnprintf((char*)buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
	if(len > 0)
	{
		ClearReceiveDef(&recBuff2);
		sprintf((char*)tcpCmd, "AT+CIPSEND=%d\r\n",len);	
		status=WiFi_ATCommand(tcpCmd,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);//1、先发长度
		if(status == 0){
			Usart_Printf(&huart1,"发送长度失败!\n");
			va_end(arg_ptr);
			return 0xf1;
		}
 		status=wifi_atcommand(buffer,4,1,wifireturn_send_ok); //2、发送内容
		if(status == 0){
			usart_printf(&huart1,"发送内容失败!\n");
			va_end(arg_ptr);
			return 0xf2;
		}  		
	}
	va_end(arg_ptr);		
	Usart_Printf_Len(&huart2,buffer,len);
	return 0;
}
*/


/*

// *获取数据指令
// *参数：
//    @ *timeStr: 时间字符串
// *返回:
//    @失败：0 超时
//    @成功：1
uint8_t WiFi_GetTime(void)
{
 uint8_t i=0;
 uint8_t *buf=NULL,*p=NULL;
 uint16_t len=0;

 WiFi_Send(WiFiCmd_GetTime);
 delay_ms(5000);     //等待延时
 len=Usart_Scanf(&recBuff2,&buf,500);
 if(len>0)  //检测返回
 {
   //返回: +CIPSNTPTIME:Thu Aug 04 14:48:05 2018    回车OK
   p=strstr(buf,"+CIPSNTPTIME");
   if(!p) return 0;
   strtok(p, " :");//空格拆分,第一个字符串为+CIPSNTPTIME:
   while((p = strtok(NULL, " :")))
   {
     i++;
     switch(i)
     {
     case 1:
       if(strcmp(p,"Mon")==0)
         week[0]='1';
       else if(strcmp(p,"Tue")==0)
         week[0]='2';
       else if(strcmp(p,"Wed")==0)
         week[0]='3';
       else if(strcmp(p,"Thu")==0)
         week[0]='4';
       else if(strcmp(p,"Fri")==0)
         week[0]='5';
       else if(strcmp(p,"Sat")==0)
         week[0]='6';
       else if(strcmp(p,"Sun")==0)
         week[0]='7';
       break;
     case 2:
       if(strcmp(p,"Jan")==0)
       {  month[0]='0';        month[1]='1';}
       else if(strcmp(p,"Feb")==0)
       { month[0]='0';          month[1]='2';}
       else if(strcmp(p,"Mar")==0)
       { month[0]='0';          month[1]='3';}
       else if(strcmp(p,"Apr")==0)
        { month[0]='0';          month[1]='4';}
       else if(strcmp(p,"May")==0)
        { month[0]='0';          month[1]='5';}
       else if(strcmp(p,"Jun")==0)
        { month[0]='0';          month[1]='6';}
       else if(strcmp(p,"Jul")==0)
        { month[0]='0';          month[1]='7';}
       else if(strcmp(p,"Aug")==0)
        { month[0]='0';          month[1]='8';}
       else if(strcmp(p,"Sep")==0)
         {month[0]='0';          month[1]='9';}
       else if(strcmp(p,"Oct")==0)
        { month[0]='1';          month[1]='0';}
       else if(strcmp(p,"Nov")==0)
        { month[0]='1';          month[1]='1';}
       else if(strcmp(p,"Dec")==0)
        { month[0]='1';          month[1]='2';}
       break;
     case 3:
       { day[0]=p[0];         day[1]=p[1]; }
       break;
     case 4:
       { hour[0]=p[0];         hour[1]=p[1]; }
       break;
     case 5:
       { minute[0]=p[0];         minute[1]=p[1]; }
       break;
     case 6:
       { second[0]=p[0];         second[1]=p[1];}
       break;
     case 7:
       { year[0]=p[2];         year[1]=p[3]; }
       break;
     }
     if(i==7) break;
   }

   FreeScanBuf(&buf); //释放buf
 }
 return 1;
}


void changeTime(uint8_t *timeNow)
{
	//uint32_t year2016=1450656000;
	// time_t cur_time=(time_t)atol((char *)buf_Time);
	// struct tm* tm_t = localtime(&cur_time);
	// strftime ((char*)timeNow,13,"%S%M%H%w%d%m%_PARA2_",tm_t); //--秒,分,时,周,日,月,年--其中周-1位，其他的是2位
//	uint16_t day;
//	uint8_t year;
        uint8_t hour,minute,second;
	uint32_t cur_time= atol((char *)buf_Time)- 1450656000;//计算16年到1970年的秒数
//	year =cur_time/31536000+16;   //1年31536000秒
	cur_time=cur_time%31536000;

//	day=cur_time/86400;  //1天24*3600=86400秒 从1月1日起的天数
	cur_time=cur_time%86400;

	hour=cur_time/3600+8;  //1小时3600秒  UTC+8小时
	cur_time=cur_time%3600;

	minute=cur_time/60;  //1分钟60秒
	second=cur_time%60;

	timeNow[0]=second;
	timeNow[1] =minute;
	timeNow[2] =hour;


        // timeNow[0] =second/10+'0';
	// timeNow[1] =second%10+'0';
        // timeNow[2] =minute/10+'0';
	// timeNow[3] =minute%10+'0';
        // timeNow[4] =hour/10+'0';
	// timeNow[5] =hour%10+'0';
 }
//buf_Aqi[4]: aqi 0－50、51－100、101－150、151－200、201－300和大于300六档
//                 优     良      轻度污染  中度污染  重度污染  严重污染
//uint8_t GetWebWeather_Aqi(uint8_t *buf)
//{
//	if(storeAqi)
//	{
//		storeAqi=0;
//		return 0;
//	}
//	return 1;
//}
// buf_Daily[11]:白天天气24(3位数)+晚上天气28(3位数)+最高温62(2位数)+最低温66(2位数)
//100:晴
//101：多云
//104: 阴

//300:阵雨
//302:雷阵雨
//305:小雨
//306:中雨
//307:大雨
//310:暴雨

//400:小雪
//401:中雪
//402:大雪
//403:暴雪
uint8_t GetWebWeather_Daily(uint8_t *buf)
{	uint8_t i=0;
	if(storeDaily)
	{
		storeDaily=0;
                                //温度范围
                buf[0]='<';
                buf[1]='=';
                buf[2]='@';
                buf[3]='A';
                buf[4]=buf_Daily[8];
                buf[5]=buf_Daily[9];
                buf[6]=':';
                buf[7]=buf_Daily[6];
                buf[8]=buf_Daily[7];


                if(buf_Daily[0]=='1' )
                {
                  if(buf_Daily[2]=='0') //晴
                  {
                    buf[9]='B';
                    buf[10]='C';
                  buf[11]='\0';
                  }
                  else if(buf_Daily[2]=='4')//阴
                  {
                    buf[9]='H';
                    buf[10]='I';
                   buf[11]='\0';
                  }else if(buf_Daily[2]=='1') //多云
                  {
                    buf[9]='D';
                    buf[10]='E';
                    buf[11]='F';
                    buf[12]='G';
                  buf[13]='\0';
                  }

                }else if(buf_Daily[0]=='3')
                {
                  if(buf_Daily[2]=='0')
                  {
                    if(buf_Daily[1]=='0') //阵雨
                    {
                    buf[9]='P';
                    buf[10]='Q';
                   }else if(buf_Daily[1]=='1') //暴雨
                   {
                    buf[9]='R';
                    buf[10]='S';
                   }
                  buf[11]='T';
                    buf[12]='U';
                  }
                  else if(buf_Daily[2]=='5')//小雨
                  {
                    buf[9]='J';
                    buf[10]='K';
                    buf[11]='T';
                    buf[12]='U';
                  }else if(buf_Daily[2]=='6')//中雨
                  {
                    buf[9]='L';
                    buf[10]='M';
                    buf[11]='T';
                    buf[12]='U';
                  }else if(buf_Daily[2]=='7')//大雨
                  {
                    buf[9]='N';
                    buf[10]='O';
                   buf[11]='T';
                    buf[12]='U';
                  }
                  else if(buf_Daily[2]=='2') //雷阵雨
                  {
                    buf[9]='X';
                    buf[10]='Y';
                    buf[11]='P';
                    buf[12]='Q';
                    buf[13]='T';
                    buf[14]='U';
                  }



                }
                else if(buf_Daily[0]=='4' )
                {
                  if(buf_Daily[2]=='0') //小雪
                  {
                    buf[9]='J';
                    buf[10]='K';
                  }
                  else if(buf_Daily[2]=='1')//中雪
                  {
                    buf[9]='L';
                    buf[10]='M';
                  }else if(buf_Daily[2]=='2') //大雪
                  {
                    buf[9]='N';
                    buf[10]='O';
                  }else if(buf_Daily[2]=='3') //暴雪
                  {
                    buf[9]='R';
                    buf[10]='S';
                  }
                  buf[11]='V';
                  buf[12]='W';
                  buf[13]='\0';
                }

		return 0;
	}
	return 1;
}
// 当前湿度17(2位数)+当前温度29(2位数)
//uint8_t GetWebWeather_Now(uint8_t *buf)
//{
//	if(storeNow)
//	{
//		storeNow=0;
//                buf[0]='>';
//                buf[1]='?';
//                buf[2]='@';
//                buf[3]='A';
//                buf[4]=buf_Now[0];
//                buf[5]=buf_Now[1];
//
//                buf[6]='<';
//                buf[7]='=';
//                buf[8]='@';
//                buf[9]='A';
//                buf[10]=buf_Now[2];
//                buf[11]=buf_Now[3];
//                buf[12]='\0';
//		return 0;
//	}
//	return 1;
//}


uint8_t GetWebTime(uint8_t *myTime)
{
	if(storeTime)
	{
		changeTime(myTime);
		storeTime=0;
		return 0;
	}
	return 1;
}

*/







/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
