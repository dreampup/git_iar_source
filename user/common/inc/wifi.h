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
//---定义服务器的地址和端口
#define SERVICE ("120.78.71.108")
#define PORT (19090)


//---------------AT指令表-------------------
//-----------基本配置
#define WiFiCmd_Test ("AT\r\n")//测试AT
#define WiFiCmd_Rst ("AT+RST\r\n")//重启
#define WiFiCmd_GMR ("AT+GMR\r\n")//获得版本信息
#define WiFiCmd_ATE(P1) ("ATE"#P1"\r\n")//开关回显 0/1 关/开
#define WiFiCmd_RESTORE ("AT+RESTORE\r\n")//恢复出厂设置
#define WiFiCmd_UART_DEF(P1) ("AT+UART_DEF="#P1",8,1,0,0\r\n")//永久配置uart  
#define WiFiCmd_CWAUTOCONN(P1) ("AT+CWAUTOCONN="#P1"\r\n") //上点是否自动连接AP： 0/1  非自动/自动
#define WiFiCmd_CWMODE(P1) ("AT+CWMODE_DEF="#P1"\r\n")// 设置入网模式: 1,2,3 --Station/SoftAP/AP+Station --返回OK
#define WiFiCmd_CIPSTATUS  ("AT+CIPSTATUS\r\n")//查询网络连接状态：返回STATUS:<stat>

//-----------入网及配置命令
#define WiFiCmd_STARTSMART(P1) ("AT+CWSTARTSMART="#P1"\r\n")// Station 模式下空中配置命令:1,2,3 -- ESP-TOUCH/Airkiss/esp&airkiss
//--返回OK--Smart get WiFi info--Smartconfig connected WiFi
#define WiFiCmd_STOPSMART ("AT+CWSTOPSMART\r\n")//停止空中配置命令      --返回OK
//连接成功后(上条指令返回 Smartconfig connected Wi-Fi后，应调用此命令释放内存 )
//#define WiFiCmd_WX_STARTDISC(P1,P2,P3) ("AT+CWSTARTDISCOVER=\"P1\",\"P2\",P3\r\n")//开启可被局域网内微信探测模式,入网后才可用使用     --返回OK
//#define WiFiCmd_WX_STOPDISC ("AT+CWSTOPDISCOVER\r\n")//使能微信发现功能    -返回OK/ERROR
//#define WiFiCmd_WPS ("AT+WPS=1\r\n")//使能wps station模式下


#define WiFiCmd_CWJAP(P1,P2) ("AT+CWJAP_DEF=\""#P1"\",\""#P2"\"\r\n")// 连接AP
#define WiFiCmd_CWQAP ("AT+CWQAP\r\n")// 断开连接AP
#define WiFiCmd_CWLAP ("AT+CWLAP\r\n")// 列出可用的AP
#define WiFiCmd_CIFSR ("AT+CIFSR\r\n")// 查询本地IP

//----------AP模式参数配置
#define WiFiCmd_CWSAP(P1,P2) ("AT+CWSAP_DEF=\""#P1"\",\""#P2"\",5,3\r\n")// 配置AP参数
#define WiFiCmd_CWLIF ("AT+CWLIF\r\n")// 查询连接到本AP的信息

#define WiFiCmd_CWDHCP(P1,P2) ("AT+CWDHCP_DEF="#P1","#P1"\r\n")// 设置AP STATION 模式下DHCP开关

//-----------TCP/IP配置
#define WiFiCmd_CIPDOMAIN(P1) ("AT+CIPDOMAIN=\""#P1"\"\r\n")//域名解析 iot.espressif.cn
#define WiFiCmd_CIPSTART(P1,P2,P3) ("AT+CIPSTART=\""#P1"\",\""#P2"\","#P3"\r\n")//TCP连接"TCP\",\"192.168.31.101\",9960\r\n,UDP见下 ---返回OK/ERROR/ALREADY CONNECT
//#define WiFiCmd_CIPSTART ("AT+CIPSTART=\"UDP\",\"192.168.31.101\",9960,10000\r\n")//UDP连接 10000为UPD连接时设置本地端口可以省略

#define WiFiCmd_CIPMODE(P1) ("AT+CIPMODE="#P1"\r\n")//透传命令  =1透传 =0普通
#define WiFiCmd_CIPSEND ("AT+CIPSEND\r\n")   //开启数据传输 透传每包最大2048，20ms间隔发送 --AT+CIPSEND=P1,返回ERROR/SEND OK

#define WiFiCmd_STOPTX ("+++")         //关闭数据传输 至少间隔1s再发送下条AT指令
#define WiFiCmd_CIPCLOSE ("AT+CIPCLOSE\r\n") //TCP连接

#define WiFiCmd_CIPMUX(P1) ("AT+CIPMUX="#P1"\r\n")// 默认为单连接 P1:0,1  单连接/多连接
#define WiFiCmd_CIPSERVER(P1,P2) ("AT+CIPSERVER="#P1","#P2"\r\n")  // 开启TCP服务 需在多连接模式下 AT+CIPMUX=1
#define WiFiCmd_CIPSTO(P1) ("AT+CIPSTO="#P1"\r\n") //断开超时的客户端, TCP 服务器会断开一直不通信直至超时了了的 TCP 客户端连接 P1:单位秒

#define WiFiCmd_PING(P1) ("AT+PING=\""#P1"\"\r\n")// ping www.baidu.com
#define WiFiCmd_CIUPDATE ("AT+CIUPDATE\r\n")// 升级wifi模块

//-----获取NTP互联网时间---目前wifi不支持
#define WiFiCmd_TimeAreaSet ("AT+CIPSNTPCFG=1,8\r\n")  //事例: AT+CIPSNTPCFG=1,8,"cn.ntp.org.cn","ntp.sjtu.edu.cn","us.pool.ntp.org"
#define WiFiCmd_GetTime   ("AT+CIPSNTPTIME?\r\n")   //返回: +CIPSNTPTIME:Thu Aug 04 14:48:05 2018    回车OK


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

extern uint8_t msgtext[30]; //文字消息buf，最多支持30字节,15汉字
extern uint8_t newWeather, newClock;//消息标志位
extern uint8_t newMsg;//是否得到及时消息 1:得到及时文字消息 2:得到及时语音消息 0:没有消息




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