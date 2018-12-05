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
//--ͨ�÷���
const uint8_t WiFiReturn_OK[]= {"OK\r\n"};
const uint8_t WiFiReturn_ERR[]= {"ERROR\r\n"};
const uint8_t WiFiReturn_BUSY[]= {"busy p.."};
//--����AP����--�ϵ�Ҳ�����
const uint8_t WiFiReturn_FAIL[]= {"FAIL\r\n"};
//�ɹ���������
const uint8_t WiFiReturn_CONN[]= {"WIFI CONNECTED"};
const uint8_t WiFiReturn_GOTIP[]= {"WIFI GOT IP"};

const uint8_t WiFiReturn_DISCONN[]= {"WIFI DISCONNECT"};
//--����״̬
const uint8_t WiFiReturn_STATUS[]= {"STATUS:"};//�����ź����1λ����Ϊ2,3,4,5�ֱ����ͬ������״̬��WiFi_Status()����

//---airkiss���÷���
const uint8_t WiFiReturn_SMART_FIND[]= {"Smart get wifi info"};//��Сд�����ԣ���ͬ�汾��һ��
const uint8_t WiFiReturn_SMART_CONN[]= {"smartconfig connected wifi"};//��Сд�����ԣ���ͬ�汾��һ��
//---tcp����
const uint8_t WiFiReturn_TCP_CLOSED[]= {"CLOSED\r\n"};
const uint8_t WiFiReturn_TCP_CONN[]= {"CONNECT\r\n"};
const uint8_t WiFiReturn_TCP_ALCONN[]= {"ALREADY CONNECTED\r\n"};

const uint8_t WiFiReturn_SEND_START[]= {"\r\n>"};//���Կ�ʼ����������
const uint8_t WiFiReturn_SEND_OK[]= {"SEND OK"}; //��͸��ģʽʱTCP����������

const uint8_t WiFiReturn_TCP_GetData[]= {"+IPD"};    //��͸��ģʽʱTCP���յ����ݣ�+IPD,19:asdfasdfasdfadfasdf


uint8_t dateStr[13]= {"2017-10-17 5"};
uint8_t timeStr[9]= {"20:01:50"};

uint8_t todayDayWeather[8]= {"����"};
uint8_t todayNightWeather[8]= {"����"};
uint8_t morrowDayWeather[8]= {"����"};
uint8_t morrowNightWeather[8]= {"����"};
uint8_t afterDayWeather[8]= {"����"};
uint8_t afterNightWeather[8]= {"����"};

uint8_t todayDayTemp[4]= {"22"};
uint8_t todayNightTemp[4]= {"12"};
uint8_t morrowDayTemp[4]= {"22"};
uint8_t morrowNightTemp[4]= {"12"};
uint8_t afterDayTemp[4]= {"22"};
uint8_t afterNightTemp[4]= {"12"};

uint8_t msgtext[30] = {0}; //������Ϣbuf�����֧��30�ֽ�,15����
uint8_t newWeather=0, newClock = 0;//��Ϣ��־λ
uint8_t newMsg = 0;//�Ƿ�õ���ʱ��Ϣ 1:�õ���ʱ������Ϣ 2:�õ���ʱ������Ϣ 0:û����Ϣ


void WiFi_RenewTime(void)
{
	uint8_t *buf=NULL;
	WiFi_TCPSend("{\"message\":{\"command\":\"get_clock\"}}",strlen("{\"message\":{\"command\":\"get_clock\"}}"));
	if(WiFi_DetectReceive(&buf)>0)
	{
		WiFi_JsonDecode(buf); //�������		
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
		WiFi_JsonDecode(buf); //�������
		FreeScanBuf(&buf);	
	}
}	
		

/*
��wifi���յ���Ҫ������w25xflash���ݴ洢����
*����:
	ftype: �ļ�����
	code: ��������
	fsize:�ļ�����
*����ֵ��> 0����
		 0 �ɹ�
*/
uint8_t WiFi_UpdateFlash(uint8_t ftype, uint8_t *code, uint32_t fsize)
{
	uint8_t *buf=NULL,sendBUf[40]={0};
	uint8_t num=0;
	uint16_t len=0;
	uint32_t tempLength=0;
	uint32_t w25xFlashAdd=0;
	uint32_t w25xFlashAddTemp=W25xFLASH_BlockSector(BlockTempBuff,SectorTempBuff);
	
	W25xFLASH_EraseBlock(BlockTempBuff,8);//����flash��Temp��(8��block)
	//��ʱ��TCP�Ѿ�������
	//--����͸��ģʽ\n
	WiFi_TCPModeSet(1);	//--���Ϳ�ʼ����
	Usart_Printf_Len(&huart1,"[update:0%]\n",12);  
	delay_ms(20);
	WiFi_SendLenght("{\"message\":{\"transfer\":0}}",26);
	ClearReceiveDef(&recBuff2);
	delay_ms(20);
	//�ȴ���temp(w25xFlashAddTemp)��ַ��Ȼ������crcУ����/����ܵ��˹涨���Ⱥ�ת�浽�ض���ַ��
	//��ò�Ҫֱ�ӷ�,�����粻�ȶ������ص��¸���ʧ��
	while(1)
	{
		len = Usart_Scanf(&recBuff2,&buf,2000);
		if(len !=512)
		{
			if(fsize == tempLength + len )//�������
			{
				W25xFLASH_OnlyWriteBuffer(w25xFlashAddTemp+tempLength,buf,len);	
				FreeScanBuf(&buf);
				break;
			}
			else{        
				FreeScanBuf(&buf);				
				Usart_Printf_Len(&huart1,"[update:err]\n",13);	
				delay_ms(20);				
				WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//�������յ�����512�ı������ݵĻ���رմ򿪵��ļ�     
				delay_ms(20);				
				WiFi_TCPModeSet(0);	//��ͨģʽ
				return 0xF0;
			}
		}
		W25xFLASH_OnlyWriteBuffer(w25xFlashAddTemp+tempLength,buf,len);	
		FreeScanBuf(&buf);//�ͷ��ڴ�
		tempLength +=len;	
		if(tempLength == fsize) break;
		sprintf((char*)sendBUf,"{\"message\":{\"transfer\":%d}}",tempLength);
		WiFi_SendLenght(sendBUf,strlen((char*)sendBUf));
		delay_ms(20);
		sprintf((char*)sendBUf,"[update:%d%%]\n",(uint8_t)(tempLength*100/fsize));
		Usart_Printf_Len(&huart1,sendBUf,strlen((char*)sendBUf));
		delay_ms(20);
	}
	
	//--������Ӧ�ĵ�ַ��������д�����
	if(ftype == 1)//bootload-Bootload
	{
		//--addrΪcrc32�ַ���		
		W25xFLASH_EraseBlock(BlockBootload,1);//������Ӧ����--1��block
		w25xFlashAdd = W25xFLASH_BlockSector(BlockBootload,SectorBootload);
		if(CRC32CheckAndMove(code, w25xFlashAdd, w25xFlashAddTemp, fsize)){//ת¼��У��ɹ�			
			W25xFLASH_AddrTab.UpdateStatus[0]=0x5a; //��λ��־	
			W25xFLASH_AddrTab.BootloadSize[3] = 0xff&(fsize>>24);
			W25xFLASH_AddrTab.BootloadSize[2] = 0xff&(fsize>>16);
			W25xFLASH_AddrTab.BootloadSize[1] = 0xff&(fsize>>8);
			W25xFLASH_AddrTab.BootloadSize[0] = 0xff&fsize;
			W25xFLASH_RenewConfig();//���±���flash������Ϣ			
		}
		else { //--У�鲻�ɹ�		
			WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//�������յ�����512�ı������ݵĻ���رմ򿪵��ļ�
			delay_ms(20);
			Usart_Printf_Len(&huart1,"[update:err]\n",13);
			delay_ms(20);
			WiFi_TCPModeSet(0);	//��ͨģʽ
			return 0xF1;	
		}
	}
	else if(ftype == 2)//app-User
	{
		W25xFLASH_EraseBlock(BlockAPP,1);//������Ӧ����--1��block
		w25xFlashAdd = W25xFLASH_BlockSector(BlockAPP,SectorAPP);	
		if(CRC32CheckAndMove(code, w25xFlashAdd, w25xFlashAddTemp, fsize)){//ת¼��У��ɹ�
			W25xFLASH_AddrTab.UpdateStatus[1]=0xa5; //��λ��־	
			W25xFLASH_AddrTab.APPSize[3] = 0xff&(fsize>>24);
			W25xFLASH_AddrTab.APPSize[2] = 0xff&(fsize>>16);
			W25xFLASH_AddrTab.APPSize[1] = 0xff&(fsize>>8);
			W25xFLASH_AddrTab.APPSize[0] = 0xff&fsize;
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else { //--У�鲻�ɹ�		
			WiFi_SendLenght("{\"message\":{\"transfer\":513}}",28);//�������յ�����512�ı������ݵĻ���رմ򿪵��ļ�
			delay_ms(20);
			Usart_Printf_Len(&huart1,"[update:err]\n",13);
			delay_ms(20);
			WiFi_TCPModeSet(0);	//��ͨģʽ
			return 0xF2;	
		}
	}
	else if(ftype == 3)//ziku
	{
		if(!memcmp(code,"ZK5X7_1",7))//ASCII���ֿ�1
		{
			W25xFLASH_EraseSector(BlockZK5X7_1,SectorZK5X7_1,1);//������Ӧ����--1��sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK5X7_1,SectorZK5X7_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=0x01;
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZK5X7_2",7))//ASCII���ֿ�2
		{
			W25xFLASH_EraseSector(BlockZK5X7_2,SectorZK5X7_2,1);//������Ӧ����--1��sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK5X7_2,SectorZK5X7_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<1);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZKSYMB_1",8))//�������ֿ�1
		{
			W25xFLASH_EraseSector(BlockZKSymb_1,SectorZKSymb_1,1);//������Ӧ����--1��sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_1,SectorZKSymb_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<2);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZKSYMB_2",8))//�������ֿ�2
		{
			W25xFLASH_EraseSector(BlockZKSymb_2,SectorZKSymb_2,1);//������Ӧ����--1��sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_2,SectorZKSymb_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<3);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZKSYMB_U",8))//Ϳѻ�ֿ�
		{
			W25xFLASH_EraseSector(BlockZKSymb_Usr,SectorZKSymb_Usr,11);//������Ӧ����--11��sector
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZKSymb_Usr,SectorZKSymb_Usr);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<4);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZK7X14_1",8))//�����ֿ�1
		{
			W25xFLASH_EraseBlock(BlockZK7X14_1,4);//������Ӧ����--4��block
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK7X14_1,SectorZK7X14_1);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<5);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"ZK7X14_2",8))//�����ֿ�2
		{
			W25xFLASH_EraseBlock(BlockZK7X14_2,4);//������Ӧ����--4��block
			w25xFlashAdd = W25xFLASH_BlockSector(BlockZK7X14_2,SectorZK7X14_2);	
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.ZKStatus |=(0x01<<6);
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}

	}
	else if(ftype == 4)//wav��SCLK00-SCLK23/SSYS00-SSYS15  UCLK00-UCLK23/USYS00-USYS15
	{
		if(!memcmp(code,"SCLK",4))//24���û���ʱ����
		{
			num = (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockSysWav+num,1);//������Ӧ����--1��block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockSysWav+num,SectorSysWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.SYSWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//���±���flash������Ϣ			
		}
		else if(!memcmp(code,"SSYS",4))//16��ϵͳ����
		{
			num = 24+ (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockSysWav+num,1);//������Ӧ����--1��block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockSysWav+num,SectorSysWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.SYSWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"UCLK",4))//24���û���ʱ����
		{
			num = (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockUsrWav+num,1);//������Ӧ����--1��block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockUsrWav+num,SectorUsrWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.UserWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		}
		else if(!memcmp(code,"USYS",4)) //16���û�ϵͳ����
		{
			num = 24 + (code[4] -'0')*10 + (code[5]-'0');
			W25xFLASH_EraseBlock(BlockUsrWav+num,1);//������Ӧ����--1��block
			w25xFlashAdd=W25xFLASH_BlockSector(BlockUsrWav+num,SectorUsrWav);
			W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
			W25xFLASH_AddrTab.UserWavStatus[num/8] |=(0x01<<(num%8));
			W25xFLASH_RenewConfig();//���±���flash������Ϣ			
		}
	}
	else if(ftype == 5)//voice, Ĭ�ϴ洢�û�7��������Ϣ��ѭ������,addrΪʱ����Ϣ����ʱ��[09300800]
	{
		
		if(++ W25xFLASH_AddrTab.VoiceCounter > 6)
		{
			W25xFLASH_AddrTab.VoiceCounter = 0;
			W25xFLASH_AddrTab.VoiceNumbers = 7; //�����ﵽ���ֵ
		}
		
		num = W25xFLASH_AddrTab.VoiceCounter;
		memcpy(W25xFLASH_AddrTab.VoiceTime[num],code, 8);  //������Ϣʱ��
		
		W25xFLASH_EraseBlock(BlockLeaveMSG+num*4,4);//������Ӧ����--4��block���滮7��
		w25xFlashAdd=W25xFLASH_BlockSector(BlockLeaveMSG+num*4,SectorLeaveMSG);
		W25xFLASH_MoveData(w25xFlashAdd, w25xFlashAddTemp, fsize);//ת¼
		W25xFLASH_RenewConfig();//���±���flash������Ϣ	
		newMsg =2;		
		/* 
		if(!memcmp(code,"LEAVE",5))
		{
			num = (code[5] -'0');
		
		}
		else if(!memcmp(code,"INSTANT",7))
		{
			//--���Ǹû�����,����Ҫ����
			newMsg =2;
		} 
		*/
	}	
	sprintf((char*)sendBUf,"{\"message\":{\"transfer\":%d}}",fsize);
	WiFi_SendLenght(sendBUf,strlen((char*)sendBUf));
	delay_ms(20); //�������յ���2��ָ��֮������Ҫ��ʱ20ms
	Usart_Printf_Len(&huart1,"[update:100%]\n",14);	
	delay_ms(20);
	WiFi_TCPModeSet(0);	//��ͨģʽ
	return 0;	
}
/*
����wifi���յ�Э������
����˵����
	msg: wifi�յ�������
*/
void WiFi_JsonDecode(uint8_t *msg)
{
	uint8_t len,i;
	uint8_t fType=0,fCode[9]= {0}; //fType:1/2/3/4/5  bootload/app/ziku/wav/voice
	// uint8_t scheduleBuf[8+32]={0};//���磺"06101200xxx" ��ʽ������(06/10)+ʱ��(12:00)+����(xxx���16������̫С)
	uint32_t fSize=0;

	cJSON *js = NULL;
	cJSON *value = NULL;
	cJSON *subValue = NULL;
	cJSON *thrValue = NULL;

	js = cJSON_Parse((char*)msg);
	//-----1���õ�������Ϣ-----������ʧ----------
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
		subValue = cJSON_GetObjectItem(value, "tomorrow"); //�˴�Ϊ����ṹ
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

		subValue = cJSON_GetObjectItem(value, "after"); //�˴�Ϊ����ṹ
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
	//-----2���õ�ʱ����Ϣ-----������ʧ----------
	else if(value = cJSON_GetObjectItem(js, "clock"))
	{
		Usart_Printf(&huart1,"renew clock!\n");
		subValue = cJSON_GetObjectItem(value, "date");
		memcpy(dateStr, subValue->valuestring, strlen(subValue->valuestring));
		subValue = cJSON_GetObjectItem(value, "time");
		memcpy(timeStr, subValue->valuestring, strlen(subValue->valuestring));
		newClock = 1;
	}
	//-----3���õ�������Ϣ-----������ʧ----------
	else if(value = cJSON_GetObjectItem(js, "text"))
	{
		Usart_Printf(&huart1,"renew message!\n");
		subValue = cJSON_GetObjectItem(value, "message");//��ʱ��Ϣ
		if (subValue)
		{
			thrValue = cJSON_GetObjectItem(subValue, "content");
			memcpy(msgtext, thrValue->valuestring, strlen(thrValue->valuestring));
			newMsg =1;
		}
		subValue = cJSON_GetObjectItem(value, "schedule");//Ҳ���Ե�����������{"text":{"schedule": [{"time": "06101200", "content": "xxx"}, {"time": "08101100", "content": "xxx"}] }}
		if (subValue)
		{
			W25xFLASH_AddrTab.ScheduleNum=cJSON_GetArraySize(subValue); //�������
			for( i=0; i<W25xFLASH_AddrTab.ScheduleNum ;i++)
			{
				thrValue = cJSON_GetObjectItem(subValue+i, "time");
				memcpy(W25xFLASH_AddrTab.SchedulTime[i], thrValue->valuestring, strlen(thrValue->valuestring));
				
				// thrValue = cJSON_GetObjectItem(subValue+i, "content");
				// memcpy(scheduleBuf, thrValue->valuestring, strlen(thrValue->valuestring));
				
				// W25xFLASH_EraseSector(BlockSchedul,SectorSchedul+i,1); //������Ӧ�洢��	
				// W25xFLASH_OnlyWriteBuffer(W25xFLASH_BlockSector(BlockSchedul,SectorSchedul+i),scheduleBuf,strlen((char*)scheduleBuf));//д��	
				
				thrValue = cJSON_GetObjectItem(subValue+i, "content");
				W25xFLASH_EraseSector(BlockSchedul,SectorSchedul+i,1); //������Ӧ�洢��		
				W25xFLASH_OnlyWriteBuffer(W25xFLASH_BlockSector(BlockSchedul,SectorSchedul+i),(uint8_t*)thrValue->valuestring, strlen(thrValue->valuestring));//д��					
			}
			
			//W25xFLASH_WriteSchedule(scheduleBuf);
			W25xFLASH_RenewConfig();//����flash������Ϣ			
		}		
	}
	//-----4���õ�������Ϣ-----flash����----------
	else if(value = cJSON_GetObjectItem(js, "setting"))
	{
		Usart_Printf(&huart1,"renew setting!\n");
		subValue = cJSON_GetObjectItem(value, "display");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //����0
				W25xFLASH_AddrTab.DisplaySet[i] = thrValue->valueint;
			}
		}
		subValue = cJSON_GetObjectItem(value, "message");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //����0
				W25xFLASH_AddrTab.MessageSet[i] = thrValue->valueint;
			}
		}
		subValue = cJSON_GetObjectItem(value, "schedule");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //����0
				W25xFLASH_AddrTab.SchedualSet[i] = thrValue->valueint;
			}
		}	
		subValue = cJSON_GetObjectItem(value, "sound");
		if(subValue)
		{
			len=cJSON_GetArraySize(subValue);
			for(i=0; i<len; i++)
			{
				thrValue = cJSON_GetArrayItem(subValue, i); //����0
				W25xFLASH_AddrTab.SoundSet[i] = thrValue->valueint;
			}
		}		
		W25xFLASH_RenewConfig();//����flash������Ϣ
	}
	//-----5���õ�������Ϣ-----flash����----------
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
				subValue = cJSON_GetObjectItem(value, "vision");	//�汾��Ϣ
				W25xFLASH_AddrTab.version[0] = subValue->valueint;	
				fType=1;				
			}
			else if(!memcmp(subValue->valuestring,"app",strlen(subValue->valuestring)))
			{
				subValue = cJSON_GetObjectItem(value, "vision");	//�汾��Ϣ
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
			thrValue = cJSON_GetArrayItem(subValue, 0); //����0,��2�ֽ�
			fSize = (thrValue->valueint)<<16; //��2�ֽ�
			thrValue = cJSON_GetArrayItem(subValue, 1); //����1,��2�ֽ�
			fSize |= thrValue->valueint;      
		}		
	}

	//��JSON�ṹ��ռ�õ����ݿռ��ͷ�,ֻ���ͷ�js
	cJSON_Delete(js);   //ֻ��Ҫ�ͷ���һ��
  
	if(fSize>0)
	{
		WiFi_UpdateFlash(fType,fCode,fSize);
	}
		
}


/**
  *��͸��ģʽ�£�TCPͨ���յ����ݣ��õ��������ݣ�������ʽ: +IPD,6:xxxxxx
  *������
      **jsonMsg:�洢ת�����buf
  *����ֵ��json���ݵĳ���
*/
uint8_t WiFi_DetectReceive(uint8_t **jsonMsg)
{
	uint8_t *numBuf = NULL, *buf=NULL;
	uint16_t i=0,j=0 ,num=0,len1=0,len2=0,len3=0;	
	for(i=0;i<4;i++)  //ѭ�����4��,Ŀ����Ҫ
	{
		len1 = Usart_Scanf(&recBuff2,&buf,500);
		if(len1 > 0)  //��������������ݷ��أ�
		{
			len2 = CheckIsEquel(buf,len1,(uint8_t*)WiFiReturn_TCP_GetData,strlen((char*)WiFiReturn_TCP_GetData));//���"+IPD"
			if(len2 >0)
			{
				len3 = CheckIsEquel(buf+len2-1,len1+1-len2,":",1);//���":"
				if(len3 > 0)
				{					
					num = len3-5-1; //�������λ��	
					numBuf = (uint8_t*)malloc(num+1); //����ַ�����ʽ������
					for(j = 0; j< num; j++)
					{
						numBuf[j] = buf[len2+4 + j];
					}
					numBuf[j] = 0;
					num = atoi((char*)numBuf);  //ת����� ������
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
			/* else //�ȴ�����
			{
				ClearReceiveDef(&recBuff2);
			}	 */		
		}
    
		FreeScanBuf(&buf);
	}	
	return num;
}



/**
  *TCPͨ�Ŵ����� ������TCPͨ��Э�飬�����������������������
  *������
      cmd:json��ʽ��ͨ��Э��
	  len����ʽ����
  *����ֵ��null
*/
/* void WiFi_TcpCommunication(uint8_t *cmd)
{
	uint8_t i=0;
	uint8_t *buf=NULL;
	//uint16_t len=0;
	//len = strlen((char*)cmd);
	//if(WiFi_TCPSend(cmd ,len) > 0 ) return;
	if(WiFi_TCPSend(cmd) > 0 ) return;
	//��ⷵ�ص�����Ҫ����json��ʽ������	
	if(WiFi_DetectReceive(&buf)>0)
	{
		WiFi_JsonDecode(buf); //�������
		FreeScanBuf(&buf);	
	}		
}
 */
/**
  *����ָ�����ȵ�ATָ��
  *������
      msg:��������
	  len:����
  *����ֵ��null
*/

void WiFi_SendLenght(uint8_t *msg, uint16_t len)
{
	Usart_Printf_Len(&huart2,msg,len);
}

/**
  *����ATָ�� ���ܳ���100�ֽ�
  *������
      fmt:ATָ��
  *����ֵ��null
*/
void WiFi_Send(uint8_t *fmt, ...)
{
	uint8_t buffer[151]= {0};
	uint8_t len;

	va_list arg_ptr; //Define convert parameters variable
	va_start(arg_ptr, fmt); //Init variable ָ��ָ��fmt��ĵ�һ������
	len = vsnprintf((char*)buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
	va_end(arg_ptr);
	if(len > 0)
	{
		Usart_Printf_Len(&huart2,buffer,len);
	}	
	delay_ms(20);
}
/**
 *ATָ��ͺ���
 *������
    @ *cmd: ATָ��
    @ dtimes: ָ�������ʱ���ٸ�500ms ���ܹ���ʱ�����ʱdtimes*500
    @ num :������������<8��
    @ ...  �ɸ��������
 *����:
    @ʧ�ܣ�0 ��ʱ
    @�ɹ���
      0x01 ��һ��������⵽
      0x03 ��1,2��������⵽
      0x05 ��1,3��������⵽
      0x07 ��1,2,3��������⵽
      �Դ�����,ֻҪ����һ����⵽����>0
*/
//uint8_t WiFi_ATCommand(uint8_t *cmd,uint8_t dtimes,uint8_t method,uint8_t num,...)
uint8_t WiFi_ATCommand(uint8_t *cmd,uint8_t dtimes,uint8_t num,...)
{
	uint8_t i=0,j=0,re=0;
	uint8_t *p=NULL,*buf=NULL;
	uint16_t len=0;
	va_list vl;
	WiFi_SendLenght(cmd,strlen((char*)cmd));          //����ָ��
	for(j=0; j< dtimes; j++)
	{		
		len=Usart_Scanf(&recBuff2,&buf,500);
		if(len>0)  //��ⷵ��
		{
			//Usart_Printf_Len(&huart1,buf,len ); //��ӡ��Ϣ
			//----------����������ȡ����ֻ��ѭ��1��! �����va_start,va_end��������
			va_start(vl,num);
			for(i=0; i<num; i++)
			{
				if((p=va_arg( vl, uint8_t*))!=NULL)
				{
					if( re & (0x01<<i))
					{
						//�Ѿ������ľͲ���Ҫ���ظ������
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
		FreeScanBuf(&buf); //�ͷ�buf
//    if(method>0)  //���в�����Ҫ��⵽
//    {
		if( re == (1<< num)-1 )  //���еĲ������ܼ�⵽
			break; //����ѭ��
//    }
//    else{
//        if( re >0 ) //��⵽1�����ϱ�־
//         break; //����ѭ��
//    }
	}

	return re;
}


/**
 *��ȡWIFI����״̬
 *��������
 *����: ״ֵ̬��
		��2��-������AP�����IP
		��3��-�ѽ���TCP/UDP����
		��4���Ͽ���������
		��5��-δ����AP
 */
uint8_t WiFi_Status(void)
{
	uint8_t j=0,status=0;
	uint8_t *buf=NULL;
	uint16_t len1=0,len2=0;

	WiFi_SendLenght(WiFiCmd_CIPSTATUS,strlen((char*)WiFiCmd_CIPSTATUS));  //����ָ��
	for(j=0; j< 2; j++)
	{	
		len1=Usart_Scanf(&recBuff2,&buf,500);
		if(len1>0)  //��ⷵ��
		{
			len2 = CheckIsEquel(buf,len1,(uint8_t*)WiFiReturn_STATUS,strlen((char*)WiFiReturn_STATUS));
			if(len2 >0)
			{
				status = buf[len2+6]; //ȡ��״̬�ֺ�������֣��緵��ֵ��STATUS:2
				FreeScanBuf(&buf); //�ͷ�buf
				break;
			}
		}
		FreeScanBuf(&buf); //�ͷ�buf
	}
	return status;
}

/**
 *WiFi������⣬������TCP����
 *��������    
 *����:
      ��3��TCP�Ϸ�����,�����������շ�������
       '2������4��������wifi ��û��TCP�Ϸ�����,��������
       '5��û��������wifi 
*/
uint8_t  WiFi_Setup(void)
{
  uint8_t state=0;
  ButtonState btStatus=BT_NON;
//  ButtonState btStatus=BT_NON;
//====================1��wifi״̬���=============================
//�ϵ�����û������wifi,�͵ȴ��û���������
//�û���2������ѡ��: ���� �� ������   
   state=WiFi_Status();
	 delay_ms(50);
   if(state == '2')//������wifi
   { 
       Usart_Printf(&huart1,"->wifi������\n");  
			 DisplayMsg("wifi������"); 
      
       if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//����TCP����
       {   
         	state = '3'; //TCP���ӳɹ�
					DisplayMsg("������������");
       }
   }
   else if(state == '3')
   { //�ѽ���TCP����
			Usart_Printf(&huart1,"������������!\n");  
			DisplayMsg("������������");
   }
   else if(state == '4')
   {//�ѶϿ�TCP����
       Usart_Printf(&huart1,"δ���ӷ�����!\n");
			 DisplayMsg("δ���ӷ�����");
       if(WiFi_StartTCPConnect(SERVICE,PORT)==0) //��������
       {   
        	state = '3'; //TCP���ӳɹ�
					DisplayMsg("������������");
       }
   }    
   else if(state == '5')
   {//δ��wifi

       Usart_Printf(&huart1,"δ����WIFI,����KEY1��ʼ��������!\n");
	     DisplayMsg("δ��WIFI,��1����������");	
       //�ȴ��û���������
		while(1)
		{
			btStatus = GPIO_ButtonDect();
			if(BT_SET ==btStatus)    //��ʼ΢����������
			{
				if(WiFi_SmartConfig() ==0) //�����ɹ�
				{               
					Usart_Printf(&huart1,"WIFI������!\n");  
					DisplayMsg("wifi������");
					state = '2';
					break; //�ɹ���������
				}
			}
			else if(BT_SUB ==btStatus) //����΢������
			{
				Usart_Printf(&huart1,"��������,��������!\n");  
				DisplayMsg("��������,��������");
				state = '5';
				break;
			}
			delay_ms(50);       
		}
		if(state == '2')
		{
			delay_ms(50);
			if(WiFi_StartTCPConnect(SERVICE,PORT)==0)//����TCP����
			{   
				state = '3'; //TCP���ӳɹ�  
				DisplayMsg("������������");
			}
		} 

    }   
   return state;  
}

/**
 *WIFIģʽ���ã�����smart����
 *������
    @ mode: 0,1,2
    @ dtimes: ָ�������ʱ���ʱ��,��λms
    @ num :������������<8��
    @ ...  �ɸ��������
 *����:
    0�����óɹ�
	>0:�������
 */
uint8_t WiFi_SmartConfig(void)
{
	uint8_t status=0;
	//---�û���΢��ɨ���ע,����˵��,��������,��ʼ���� �������������ð���ʵ��
	ClearReceiveDef(&recBuff2);
	//step1:����Airkiss �Զ�����ģʽ ���û�20s(40*0.5s)ʱ������,��Ϊ��ҪС�ڿ��Ź�26s��λ
	Usart_Printf(&huart1,"step1:����Airkiss \n");
	DisplayString7X14(0,65,"��ʼ΢������wifi",16);
	status=WiFi_ATCommand(WiFiCmd_STARTSMART(3),40,1,WiFiReturn_SMART_CONN);
	DisplayString7X14(0,65,"�ر�΢������",12);
	Usart_Printf(&huart1,"step2:�ر�Airkiss \n");
	WiFi_ATCommand(WiFiCmd_STOPSMART,4,1,WiFiReturn_OK); //���ܳɹ��������Ҫֹͣ
	if(status==0){
		//Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf3;
	}
	//step2:��ѯwifi״̬
	status=WiFi_Status();//����'2'��ʾ������AP���õ�IP
	if(status != '2'){
		Usart_Printf(&huart1,"wifiδ�ɹ�����!\n");
		DisplayString7X14(0,65,"����WIFIʧ��",12);
		return 0xf4; //�������
	}
	Usart_Printf(&huart1,"wifi�����ɹ�!\n");
	DisplayString7X14(0,65,"����WIFI�ɹ�",12);	
// Ŀǰwifi��֧�� //step4:����ʱ��
//  status=WiFi_ATCommand(WiFiCmd_TimeAreaSet,5,1,WiFiReturn_OK);
//  if(status==0) return 0xf3; //�������
//  //�������ʱ��
//  status=WiFi_GetTime();
//  if(status==0) return 0xf4; //�������
	return 0;
}
/**
 *����TCP����
 *������
    @ server_addr: ��������ַ
    @ server_port: �������˿�

 *����:
    0�����óɹ�
	>0:�������
 */
uint8_t WiFi_StartTCPConnect(uint8_t* server_addr, uint16_t server_port)
{
	uint8_t status=0;
	uint8_t tcpCmd[60]= {0};
  
	sprintf((char*)tcpCmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",server_addr, server_port);
	ClearReceiveDef(&recBuff2);
	//step1:TCP����
	Usart_Printf(&huart1,"step1:��ʼTCP����\n");
	DisplayMsg("��ʼTCP����");
	status=WiFi_ATCommand(tcpCmd,6,3,WiFiReturn_TCP_CONN,WiFiReturn_TCP_ALCONN,WiFiReturn_OK);
	if(status <1) { //���ټ�⵽2��
		Usart_Printf(&huart1,"����ʧ��\n");
		DisplayMsg("����ʧ��");
		return 0xf1; //�������,ֻҪ����һ����⵽����>0
	}
	
	return 0;
}

/**
 *�ر�TCP����,ֹͣ����
 *������

 *����:
    0�����óɹ�
	>0:�������
 */
uint8_t WiFi_EndConnect(void)
{
	uint8_t status=0;
	//step1:�ر�TCP����
	ClearReceiveDef(&recBuff2);	
//	Usart_Printf(&huart1,"step1:�ر�TCP����\n");
	//������close ����error ��������û�������򷵻ص���ERROR
	status=WiFi_ATCommand(WiFiCmd_CIPCLOSE,3,2,WiFiReturn_TCP_CLOSED,WiFiReturn_ERR);
//	if(status==0) {
//		Usart_Printf(&huart1,"TCP�ر�ʧ��!\n");
//		DisplayMsg("TCP�ر�ʧ��");
//		return 0xf1; //�������
//	}
	return 0;
}
/**
 *TCPģʽ����
 *������
    @ mod : 
          =0 ��ͨ����
          =1 ͸��ģʽ����
 *����:
    0�����óɹ�
	>0:�������
 */
uint8_t WiFi_TCPModeSet(uint8_t mode)  
{
  uint8_t status=0;
	ClearReceiveDef(&recBuff2);
	if( mode)
	{
		//step1:͸��ģʽ
		Usart_Printf(&huart1,"step1:����͸��ģʽ\n");
		status=WiFi_ATCommand(WiFiCmd_CIPMODE(1),4,1,WiFiReturn_OK);
		if(status==0) {
		  Usart_Printf(&huart1,"����ʧ��!\n");
		  return 0xf1;
		}
		//step2:������������
		Usart_Printf(&huart1,"step2:������������\n");
		status=WiFi_ATCommand(WiFiCmd_CIPSEND,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);
		if(status == 0){
		  Usart_Printf(&huart1,"����ʧ��!\n");
		  return 0xf2;
		}
	}
	else
	{
		//step1:����͸������ģʽ,
		Usart_Printf(&huart1,"step1:����͸��\n");
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(1200);//��������Ҫ��ʱ1s�����ָ��ſ��Է���

		//step2:��ͨģʽ
		Usart_Printf(&huart1,"step2:��ͨģʽ\n");
		status=WiFi_ATCommand(WiFiCmd_CIPMODE(0),4,1,WiFiReturn_OK);
		if(status==0) {
			Usart_Printf(&huart1,"����ʧ��!\n");
			return 0xf1;
		} 
    }
	return 0;
}

/**
 *��������,��͸��ģʽ��ʹ��
 *������
 *����:
    0�����óɹ�
	>0:�������
 */
uint8_t WiFi_InitConfig(void)
{
  uint8_t status=0;
	ClearReceiveDef(&recBuff2);	
	
	//step0:����wifi�Ƿ���ָ��ģʽ
	Usart_Printf(&huart1,"1>AT����\n");
	DisplayMsg("1>AT����");
	status=WiFi_ATCommand(WiFiCmd_Test,2,1,WiFiReturn_OK);
	if(status==0) {	
		//step2:����͸������ģʽ,
		Usart_Printf(&huart1,"AT�޻�Ӧ,���Խ���͸��\n");
		DisplayMsg("AT�޻�Ӧ");
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(200);
		Usart_Printf_Len(&huart2,WiFiCmd_STOPTX,3);
		delay_ms(1500);//��������Ҫ��ʱ1s�����ָ��ſ��Է���
		status=WiFi_ATCommand(WiFiCmd_Test,2,1,WiFiReturn_OK);
		if(status==0) 
		{
		  Usart_Printf(&huart1,"wifiӲ������\n");
			DisplayMsg("wifiӲ������");
			//--����Ӳ����λ����ָ��
			
			return 0xf0; //������� 
		}
	}
	
  delay_ms(200);
	DisplayMsg("2>�ر�SMART");
  //step1:����wifi�Ƿ���ָ��ģʽ,��ֹ����ʱ�ͻ�������
  //Usart_Printf(&huart1,"step1:���Թر�smart����\n");
	WiFi_ATCommand(WiFiCmd_STOPSMART,2,1,WiFiReturn_OK); //���ܳɹ��������Ҫֹͣ

  delay_ms(200);
	DisplayMsg("3>��׼ģʽ");
	//step1:����wifiΪstationģʽ
	//Usart_Printf(&huart1,"step1:����wifiΪstationģʽ\n");
	status=WiFi_ATCommand(WiFiCmd_CWMODE(1),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf1; //������� 
	}
	delay_ms(200);
	DisplayMsg("4>�Զ�����AP");
	//step2:ʹ���ϵ��Զ�����AP
//	Usart_Printf(&huart1,"step2:ʹ���ϵ��Զ�����AP\n");
	status=WiFi_ATCommand(WiFiCmd_CWAUTOCONN(1),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf2; //�������
	}
	delay_ms(200);
	DisplayMsg("5>�رջ���");
	//step3:�رջ���
//	Usart_Printf(&huart1,"step3:�رջ���\n");	
	status=WiFi_ATCommand(WiFiCmd_ATE(0),2,1,WiFiReturn_OK); 
	if(status==0) {
//		Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf3; //�������
	}
	delay_ms(200);
	DisplayMsg("6>��ͨ����");
	//step4:TCP��ͨģʽ
//	Usart_Printf(&huart1,"step4:TCP��ͨģʽ\n");
	WiFi_ATCommand(WiFiCmd_CIPMODE(0),2,1,WiFiReturn_OK);
	if(status==0) {
//		Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf4; //�������
	}
	delay_ms(200);
	DisplayMsg("7>�ر�TCP");
//	WiFi_ATCommand(WiFiCmd_CIPCLOSE,3,2,WiFiReturn_TCP_CLOSED,WiFiReturn_ERR);
	WiFi_EndConnect();//Ĭ�Ϲ�1��TCP���ӣ���ֹ�������
	delay_ms(200);
 return 0;	
}


/**
 *��������,��͸��ģʽ��ʹ��
 *������
 *����:
    0�����óɹ�
	>0:�������
 */

uint8_t WiFi_TCPSend(uint8_t* msg, uint16_t len)
{
	uint8_t status=0;
	uint8_t tcpCmd[20]= {0};
	ClearReceiveDef(&recBuff2);
	sprintf((char*)tcpCmd, "AT+CIPSEND=%d\r\n",len);	
	status=WiFi_ATCommand(tcpCmd,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);
	if(status == 0){
		Usart_Printf(&huart1,"����ʧ��!\n");
		return 0xf1;
	} 
	/* status=WiFi_ATCommand(msg,4,1,WiFiReturn_SEND_OK); //2����������
	if(status == 0){
		Usart_Printf(&huart1,"��������ʧ��!\n");
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
	va_start(arg_ptr, fmt); //Init variable ָ��ָ��fmt��ĵ�һ������
	len = vsnprintf((char*)buffer, 150,(char*) fmt, arg_ptr); //parameters list format to buffer
	if(len > 0)
	{
		ClearReceiveDef(&recBuff2);
		sprintf((char*)tcpCmd, "AT+CIPSEND=%d\r\n",len);	
		status=WiFi_ATCommand(tcpCmd,4,2,WiFiReturn_OK,WiFiReturn_SEND_START);//1���ȷ�����
		if(status == 0){
			Usart_Printf(&huart1,"���ͳ���ʧ��!\n");
			va_end(arg_ptr);
			return 0xf1;
		}
 		status=wifi_atcommand(buffer,4,1,wifireturn_send_ok); //2����������
		if(status == 0){
			usart_printf(&huart1,"��������ʧ��!\n");
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

// *��ȡ����ָ��
// *������
//    @ *timeStr: ʱ���ַ���
// *����:
//    @ʧ�ܣ�0 ��ʱ
//    @�ɹ���1
uint8_t WiFi_GetTime(void)
{
 uint8_t i=0;
 uint8_t *buf=NULL,*p=NULL;
 uint16_t len=0;

 WiFi_Send(WiFiCmd_GetTime);
 delay_ms(5000);     //�ȴ���ʱ
 len=Usart_Scanf(&recBuff2,&buf,500);
 if(len>0)  //��ⷵ��
 {
   //����: +CIPSNTPTIME:Thu Aug 04 14:48:05 2018    �س�OK
   p=strstr(buf,"+CIPSNTPTIME");
   if(!p) return 0;
   strtok(p, " :");//�ո���,��һ���ַ���Ϊ+CIPSNTPTIME:
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

   FreeScanBuf(&buf); //�ͷ�buf
 }
 return 1;
}


void changeTime(uint8_t *timeNow)
{
	//uint32_t year2016=1450656000;
	// time_t cur_time=(time_t)atol((char *)buf_Time);
	// struct tm* tm_t = localtime(&cur_time);
	// strftime ((char*)timeNow,13,"%S%M%H%w%d%m%_PARA2_",tm_t); //--��,��,ʱ,��,��,��,��--������-1λ����������2λ
//	uint16_t day;
//	uint8_t year;
        uint8_t hour,minute,second;
	uint32_t cur_time= atol((char *)buf_Time)- 1450656000;//����16�굽1970�������
//	year =cur_time/31536000+16;   //1��31536000��
	cur_time=cur_time%31536000;

//	day=cur_time/86400;  //1��24*3600=86400�� ��1��1���������
	cur_time=cur_time%86400;

	hour=cur_time/3600+8;  //1Сʱ3600��  UTC+8Сʱ
	cur_time=cur_time%3600;

	minute=cur_time/60;  //1����60��
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
//buf_Aqi[4]: aqi 0��50��51��100��101��150��151��200��201��300�ʹ���300����
//                 ��     ��      �����Ⱦ  �ж���Ⱦ  �ض���Ⱦ  ������Ⱦ
//uint8_t GetWebWeather_Aqi(uint8_t *buf)
//{
//	if(storeAqi)
//	{
//		storeAqi=0;
//		return 0;
//	}
//	return 1;
//}
// buf_Daily[11]:��������24(3λ��)+��������28(3λ��)+�����62(2λ��)+�����66(2λ��)
//100:��
//101������
//104: ��

//300:����
//302:������
//305:С��
//306:����
//307:����
//310:����

//400:Сѩ
//401:��ѩ
//402:��ѩ
//403:��ѩ
uint8_t GetWebWeather_Daily(uint8_t *buf)
{	uint8_t i=0;
	if(storeDaily)
	{
		storeDaily=0;
                                //�¶ȷ�Χ
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
                  if(buf_Daily[2]=='0') //��
                  {
                    buf[9]='B';
                    buf[10]='C';
                  buf[11]='\0';
                  }
                  else if(buf_Daily[2]=='4')//��
                  {
                    buf[9]='H';
                    buf[10]='I';
                   buf[11]='\0';
                  }else if(buf_Daily[2]=='1') //����
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
                    if(buf_Daily[1]=='0') //����
                    {
                    buf[9]='P';
                    buf[10]='Q';
                   }else if(buf_Daily[1]=='1') //����
                   {
                    buf[9]='R';
                    buf[10]='S';
                   }
                  buf[11]='T';
                    buf[12]='U';
                  }
                  else if(buf_Daily[2]=='5')//С��
                  {
                    buf[9]='J';
                    buf[10]='K';
                    buf[11]='T';
                    buf[12]='U';
                  }else if(buf_Daily[2]=='6')//����
                  {
                    buf[9]='L';
                    buf[10]='M';
                    buf[11]='T';
                    buf[12]='U';
                  }else if(buf_Daily[2]=='7')//����
                  {
                    buf[9]='N';
                    buf[10]='O';
                   buf[11]='T';
                    buf[12]='U';
                  }
                  else if(buf_Daily[2]=='2') //������
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
                  if(buf_Daily[2]=='0') //Сѩ
                  {
                    buf[9]='J';
                    buf[10]='K';
                  }
                  else if(buf_Daily[2]=='1')//��ѩ
                  {
                    buf[9]='L';
                    buf[10]='M';
                  }else if(buf_Daily[2]=='2') //��ѩ
                  {
                    buf[9]='N';
                    buf[10]='O';
                  }else if(buf_Daily[2]=='3') //��ѩ
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
// ��ǰʪ��17(2λ��)+��ǰ�¶�29(2λ��)
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
