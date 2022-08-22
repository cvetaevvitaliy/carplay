#ifndef _FUNDEF_H
#define _FUNDEF_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "anwmanagerfun.h"
#include <wchar.h>
#include <locale.h>
#include <string.h>

enum{
	CLR_RED =0,
	CLR_GREEN,
	CLR_YELLOW,
	CLR_BLUE,
	CLR_MAGENTA,
	CLR_CYAN,
	CLR_GRAY
};
/*
enum
{
	AVRCP_CONTROL_ACT_PRESS			= 0x00,
	AVRCP_CONTROL_ACT_RELEASE		= 0x01
};*/
#define MENU_CLR  CLR_CYAN
typedef struct
{
	ANW_BD_ADDR		bt_address; 
	int			nConnectStatus; //0: disconnect , 1: connect
}SPP_CONNECTION;
/*
typedef enum PlayerAttributes {
    	PLAYER_ATTR_EQUALIZER		= 0x01,
   	PLAYER_ATTR_REPEATMODE		= 0x02,
	PLAYER_ATTR_SHUFFLE		= 0x03,
	PLAYER_ATTR_SCAN		= 0x04
} PlayerAttributes;*/
//callback function
int ConnectStatusCallback(int nIndex,Connect_Channel Channel,int nStatus, ANW_BD_ADDR bd_addr);
int PairStatusCallback(PAIRSTATUS_TYPE nType,unsigned int data,ANW_BD_ADDR* bd_addr,int cod);
int ServiceConnectRequestCallBack(Connect_Channel nType,ANW_BD_ADDR bd_addr,ANW_CHAR* szdeviceName,int nIndex);
int HFIndicatorCallback(int nHFDeviceIndex,ANW_UINT IndKey, ANW_UINT IndValue);
int IncomingCallCallback(int nIndex,GSM_MemoryEntry entry,int nCallStstus);
int IncomingSMSCallback(int nIndex,SMS_Data_Strc sms);
int AudioControlEventCallback(CONTROL_CALLBACK_EVENT control_event);
ANW_BOOL SPPEventCallBack(SPP_EVEN_TTYPE nEventType,void* pEventData,int nEventdataSize);

int color_print (int nclr, const char * format, ... );

bool PL_GetBTAddress(char* str1,char* str2,ANW_BD_ADDR  *btaddr);
void PL_GetEntryNameNumber(GSM_MemoryEntry entry,char *szName,char * szNumber);
bool PL_CheckBTPowerStatus();
bool PL_CheckHFPConnectStatus(int nIndex);

int PL_BTPower(bool bon);
void PL_SettingMenu();
void PL_HFPMenu();
void PL_PhonebookMenu();
void PL_SMSMenu();
void PL_A2DPMenu();
void PL_HIDMenu();
void PL_SPPMenu();
#endif