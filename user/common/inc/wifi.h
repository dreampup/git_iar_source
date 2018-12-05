/**
  ******************************************************************************
  * @file    hal_wifi.h
  * @author  Luo
  * @version V1.0.1
  * @date    16-February-2016
  * @brief   Header file for stm8s_wifi.c module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WIFI_H
#define __WIFI_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
//---����������ĵ�ַ�Ͷ˿�
#define SERVICE ("120.78.71.108")
#define PORT (19090)


//---------------ATָ���-------------------
//-----------��������
#define WiFiCmd_Test ("AT\r\n")//����AT
#define WiFiCmd_Rst ("AT+RST\r\n")//����
#define WiFiCmd_GMR ("AT+GMR\r\n")//��ð汾��Ϣ
#define WiFiCmd_ATE(P1) ("ATE"#P1"\r\n")//���ػ��� 0/1 ��/��
#define WiFiCmd_RESTORE ("AT+RESTORE\r\n")//�ָ���������
#define WiFiCmd_UART_DEF(P1) ("AT+UART_DEF="#P1",8,1,0,0\r\n")//��������uart  
#define WiFiCmd_CWAUTOCONN(P1) ("AT+CWAUTOCONN="#P1"\r\n") //�ϵ��Ƿ��Զ�����AP�� 0/1  ���Զ�/�Զ�
#define WiFiCmd_CWMODE(P1) ("AT+CWMODE_DEF="#P1"\r\n")// ��������ģʽ: 1,2,3 --Station/SoftAP/AP+Station --����OK
#define WiFiCmd_CIPSTATUS  ("AT+CIPSTATUS\r\n")//��ѯ��������״̬������STATUS:<stat>

//-----------��������������
#define WiFiCmd_STARTSMART(P1) ("AT+CWSTARTSMART="#P1"\r\n")// Station ģʽ�¿�����������:1,2,3 -- ESP-TOUCH/Airkiss/esp&airkiss
//--����OK--Smart get WiFi info--Smartconfig connected WiFi
#define WiFiCmd_STOPSMART ("AT+CWSTOPSMART\r\n")//ֹͣ������������      --����OK
//���ӳɹ���(����ָ��� Smartconfig connected Wi-Fi��Ӧ���ô������ͷ��ڴ� )
//#define WiFiCmd_WX_STARTDISC(P1,P2,P3) ("AT+CWSTARTDISCOVER=\"P1\",\"P2\",P3\r\n")//�����ɱ���������΢��̽��ģʽ,������ſ���ʹ��     --����OK
//#define WiFiCmd_WX_STOPDISC ("AT+CWSTOPDISCOVER\r\n")//ʹ��΢�ŷ��ֹ���    -����OK/ERROR
//#define WiFiCmd_WPS ("AT+WPS=1\r\n")//ʹ��wps stationģʽ��


#define WiFiCmd_CWJAP(P1,P2) ("AT+CWJAP_DEF=\""#P1"\",\""#P2"\"\r\n")// ����AP
#define WiFiCmd_CWQAP ("AT+CWQAP\r\n")// �Ͽ�����AP
#define WiFiCmd_CWLAP ("AT+CWLAP\r\n")// �г����õ�AP
#define WiFiCmd_CIFSR ("AT+CIFSR\r\n")// ��ѯ����IP

//----------APģʽ��������
#define WiFiCmd_CWSAP(P1,P2) ("AT+CWSAP_DEF=\""#P1"\",\""#P2"\",5,3\r\n")// ����AP����
#define WiFiCmd_CWLIF ("AT+CWLIF\r\n")// ��ѯ���ӵ���AP����Ϣ

#define WiFiCmd_CWDHCP(P1,P2) ("AT+CWDHCP_DEF="#P1","#P1"\r\n")// ����AP STATION ģʽ��DHCP����

//-----------TCP/IP����
#define WiFiCmd_CIPDOMAIN(P1) ("AT+CIPDOMAIN=\""#P1"\"\r\n")//�������� iot.espressif.cn
#define WiFiCmd_CIPSTART(P1,P2,P3) ("AT+CIPSTART=\""#P1"\",\""#P2"\","#P3"\r\n")//TCP����"TCP\",\"192.168.31.101\",9960\r\n,UDP���� ---����OK/ERROR/ALREADY CONNECT
//#define WiFiCmd_CIPSTART ("AT+CIPSTART=\"UDP\",\"192.168.31.101\",9960,10000\r\n")//UDP���� 10000ΪUPD����ʱ���ñ��ض˿ڿ���ʡ��

#define WiFiCmd_CIPMODE(P1) ("AT+CIPMODE="#P1"\r\n")//͸������  =1͸�� =0��ͨ
#define WiFiCmd_CIPSEND ("AT+CIPSEND\r\n")   //�������ݴ��� ͸��ÿ�����2048��20ms������� --AT+CIPSEND=P1,����ERROR/SEND OK

#define WiFiCmd_STOPTX ("+++")         //�ر����ݴ��� ���ټ��1s�ٷ�������ATָ��
#define WiFiCmd_CIPCLOSE ("AT+CIPCLOSE\r\n") //TCP����

#define WiFiCmd_CIPMUX(P1) ("AT+CIPMUX="#P1"\r\n")// Ĭ��Ϊ������ P1:0,1  ������/������
#define WiFiCmd_CIPSERVER(P1,P2) ("AT+CIPSERVER="#P1","#P2"\r\n")  // ����TCP���� ���ڶ�����ģʽ�� AT+CIPMUX=1
#define WiFiCmd_CIPSTO(P1) ("AT+CIPSTO="#P1"\r\n") //�Ͽ���ʱ�Ŀͻ���, TCP ��������Ͽ�һֱ��ͨ��ֱ����ʱ���˵� TCP �ͻ������� P1:��λ��

#define WiFiCmd_PING(P1) ("AT+PING=\""#P1"\"\r\n")// ping www.baidu.com
#define WiFiCmd_CIUPDATE ("AT+CIUPDATE\r\n")// ����wifiģ��

//-----��ȡNTP������ʱ��---Ŀǰwifi��֧��
#define WiFiCmd_TimeAreaSet ("AT+CIPSNTPCFG=1,8\r\n")  //����: AT+CIPSNTPCFG=1,8,"cn.ntp.org.cn","ntp.sjtu.edu.cn","us.pool.ntp.org"
#define WiFiCmd_GetTime   ("AT+CIPSNTPTIME?\r\n")   //����: +CIPSNTPTIME:Thu Aug 04 14:48:05 2018    �س�OK


extern uint8_t dateStr[13];
extern uint8_t timeStr[9];

extern uint8_t todayDayWeather[8];
extern uint8_t todayNightWeather[8];
extern uint8_t morrowDayWeather[8];
extern uint8_t morrowNightWeather[8];
extern uint8_t afterDayWeather[8];
extern uint8_t afterNightWeather[8];

extern uint8_t todayDayTemp[4];
extern uint8_t todayNightTemp[4];
extern uint8_t morrowDayTemp[4];
extern uint8_t morrowNightTemp[4];
extern uint8_t afterDayTemp[4];
extern uint8_t afterNightTemp[4];

extern uint8_t msgtext[30]; //������Ϣbuf�����֧��30�ֽ�,15����
extern uint8_t newWeather, newClock;//��Ϣ��־λ
extern uint8_t newMsg;//�Ƿ�õ���ʱ��Ϣ 1:�õ���ʱ������Ϣ 2:�õ���ʱ������Ϣ 0:û����Ϣ




void WiFi_SendLenght(uint8_t *msg, uint16_t len);
void WiFi_Send(uint8_t *fmt, ...);
uint8_t WiFi_Status(void);

uint8_t WiFi_InitConfig(void);
uint8_t WiFi_TCPModeSet(uint8_t mode) ;

uint8_t WiFi_Setup(void);
uint8_t WiFi_SmartConfig(void);

uint8_t WiFi_StartTCPConnect(uint8_t* server_addr, uint16_t server_port);
uint8_t WiFi_EndConnect(void);

//uint8_t WiFi_TCPSend(uint8_t *fmt, ...);
uint8_t WiFi_TCPSend(uint8_t* msg, uint16_t len);
//uint8_t WiFi_StartConnectSend(void);
//uint8_t WiFi_GetTime(void);
uint8_t WiFi_DetectReceive(uint8_t **jsonMsg);

// void WiFi_TcpCommunication(uint8_t *cmd);
void WiFi_JsonDecode(uint8_t *msg);

void WiFi_RenewTime(void);
void WiFi_UpdateFile(uint8_t *fType, uint8_t *fName);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/