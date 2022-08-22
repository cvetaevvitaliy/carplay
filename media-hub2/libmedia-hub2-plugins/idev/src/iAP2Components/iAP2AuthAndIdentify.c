/*
 * =====================================================================================
 *
 *       Filename:  iAP2AuthAndIdentify.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 11:46:08 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "iAP2AuthAndIdentify.h"
#include "iAP2I2c.h"

#include <dev_iap2.h>
#include <debug.h>
#include <mh_misc.h>

#define ACCESSORY_NAME_ID		0
#define ACCESSORY_NAME			"Media-Hub v2.0"
#define MODEL_IDENTIFIER_ID		1
#define MODEL_IDENTIFIER		"Neusoft Middleware Model"
#define MANUFACTURE_ID			2
#define MANUFACTURE				"CoC of Neusoft Automotive"
#define SN_ID					3
#define SERIAL_NUMBER			"201302055196"
#define FM_VER_ID				4
#define FIRMWARE_VER			"0.0.1"
#define HW_VER_ID				5
#define HARDWARE_VER			"0.1"

#define MSG_ACCESSORY_ID		6

//Media Library Access
uint8_t msgMediaLibraryInfo[] = {
	0x4C, 0x00, //StartMediaLibraryInformation
	0x4C, 0x02, //StopMediaLibraryInformation
	0x4C, 0x03, //StartMediaLibraryUpdates
	0x4C, 0x05, //StopMediaLibraryUpdates
	0x4C, 0x06, //PlayMediaLibraryCurrentSelection
	0x4C, 0x07, //PlayMediaLibraryItems
	0x4C, 0x08, //PlayMediaLibraryCollection
	0x4c, 0x09, //PlayMediaLibrarySpecial
};
//Power
uint8_t msgPowerUpdate[] = {
	0xAE, 0x00, //StartPowerUpdates
	0xAE, 0x02, //StopPowerUpdates
	0xAE, 0x03, //PowerSourceUpdate
};
//wif Power
uint8_t msgWifiPowerUpdate[] = {
	0xAE, 0x00, //StartPowerUpdates
	0xAE, 0x02, //StopPowerUpdates
};

//Now Playing
uint8_t msgNowPlaying[] = {
	0x50, 0x00, //StartNowPlayingUpdates
	0x50, 0x02, //StopNowPlayingUpdates
	0x50, 0x03, //SetNowPlayingInformation
};
//Human Interface Device
uint8_t msgHID[] = {
	0x68, 0x00, //StartHID
	0x68, 0x02, //AccessoryHIDReport
	0x68, 0x03, //StopHID
};
//App Launch
uint8_t msgAppLanch[] = { 
	0xEA, 0x02,  //RequestAppLaunch
};

uint8_t msgLocationInfo[] = {
	0xFF, 0xFB, //LocationInformation
};

uint8_t msgVehicleStatus[] = {
	0xA1, 0x01, //VehicleStatusUpdate
};

uint8_t msgListUpdates[] = {
	0x41, 0x70,  //StartListUpdates
	0x41, 0x72,  //StopListUpdates
};

uint8_t msgCallState[] = {
	0x41, 0x54,  //StartCallStateUpdates
	0x41, 0x56,  //StopCallStateUpdates
};

uint8_t msgAccessoryD[]			=	{
	//Media Library Access
	0x4C, 0x00,		//StartMediaLibraryInformation 
	0x4C, 0x02,		//StopMediaLibraryInformation 
	0x4C, 0x03,		//StartMediaLibraryUpdates 
	0x4C, 0x05,		//StopMediaLibraryUpdates 
	0x4C, 0x06, 	//PlayMediaLibraryCurrentSelection
	0x4C, 0x07,		//PlayMediaLibraryItems 
	0x4C, 0x08, 	//PlayMediaLibraryCollection
	0x4C, 0x09,		//PlayMediaLibrarySpecial

	//Now Playing
	0x50, 0x00,		//StartNowPlayingUpdates 
	0x50, 0x02,		//StopNowPlayingUpdates 
	0x50, 0x03,		//SetNowPlayingInformation 

	//USB Device Mode Audio
	0xDA, 0x00, 	//StartUSBDeviceModeAudio
	0xDA, 0x02,		//StopUSBDeviceModeAudio

	//Human Interface Device
	0x68, 0x00,		//StartHID 
	0x68, 0x02,		//AccessoryHIDReport 
	0x68, 0x03,		//StopHID

	//App Launch
	0xEA, 0x02,		//RequestAppLaunch

	//Power
	0xAE, 0x00, 	//StartPowerUpdates
	0xAE, 0x02,		//StopPowerUpdates 
	0xAE, 0x03,		//PowerSourceUpdate
};

uint8_t msgAccessoryLvH[]			=	{
	//Media Library Access
	0x4C, 0x00,		//StartMediaLibraryInformation 
	0x4C, 0x02,		//StopMediaLibraryInformation 
	0x4C, 0x03,		//StartMediaLibraryUpdates 
	0x4C, 0x05,		//StopMediaLibraryUpdates 
	0x4C, 0x06, 	//PlayMediaLibraryCurrentSelection
	0x4C, 0x07,		//PlayMediaLibraryItems 
	0x4C, 0x08, 	//PlayMediaLibraryCollection

	//Power
	0xAE, 0x00, 	//StartPowerUpdates
	0xAE, 0x02,		//StopPowerUpdates 
	0xAE, 0x03,		//PowerSourceUpdate

	//Now Playing
	0x50, 0x00,		//StartNowPlayingUpdates 
	0x50, 0x02,		//StopNowPlayingUpdates 

	//Human Interface Device
	0x68, 0x00,		//StartHID 
	0x68, 0x02,		//AccessoryHIDReport 
	0x68, 0x03,		//StopHID

	//App Launch
	0xEA, 0x02,		//RequestAppLaunch
};

uint8_t msgAccessoryLvD[]			=	{
	//Media Library Access
	0x4C, 0x00,		//StartMediaLibraryInformation 
	0x4C, 0x02,		//StopMediaLibraryInformation 
	0x4C, 0x03,		//StartMediaLibraryUpdates 
	0x4C, 0x05,		//StopMediaLibraryUpdates 
	0x4C, 0x06, 	//PlayMediaLibraryCurrentSelection
	0x4C, 0x07,		//PlayMediaLibraryItems 
	0x4C, 0x08, 	//PlayMediaLibraryCollection

	//Now Playing
	0x50, 0x00,		//StartNowPlayingUpdates 
	0x50, 0x02,		//StopNowPlayingUpdates 

	//Human Interface Device
	0x68, 0x00,		//StartHID 
	0x68, 0x02,		//AccessoryHIDReport 
	0x68, 0x03,		//StopHID

	//USB Device Mode Audio
	0xDA, 0x00, 	//StartUSBDeviceModeAudio
	0xDA, 0x02,		//StopUSBDeviceModeAudio

	//Power
	0xAE, 0x00, 	//StartPowerUpdates
	0xAE, 0x02,		//StopPowerUpdates 
	0xAE, 0x03,		//PowerSourceUpdate

	//App Launch
	0xEA, 0x02,		//RequestAppLaunch
};

//AccessoryWiFiConfigurationInformation
uint8_t msgWifiConfInfo[] = { 
	0x57, 0x03,  //AccessoryWiFiConfigurationInformation
};

#define MSG_DEVICE_ID			7
uint8_t msgMediaLibraryRev[] = {
	0x4C, 0x01, //MediaLibraryInformation
	0x4C, 0x04, //MediaLibraryUpdate
};

uint8_t msgDeviceNotificationsRev[] = {
	0x4E, 0x09, //DeviceInformationUpdate
	0x4E, 0x0A, //DeviceLanguageUpdate
};

uint8_t msgDeviceNotificationsOnlyCarplayRev[] = {
	0x4E,0x0C,
//	0x4E,0x0E,
};

uint8_t msgNowPlayingRev[] = {
	0x50, 0x01, //NowPlayingUpdate
};

uint8_t msgHIDRev[] = {
	0x68, 0x01, //DeviceHIDReport
};

uint8_t msgEARev[] = {
	0xEA, 0x00, //StartExternalAccessoryProtocolSession
	0xEA, 0x01,	//StopExternalAccessoryProtocolSession
};

uint8_t msgPowerUpdateRev[] = {
	0xAE, 0x01, //PowerUpdate
};

uint8_t msgLocationInfoRev[] = {
	0xFF, 0xFA, //StartLocationInformation
	0xFF, 0xFC, //StopLocationInformation
};

uint8_t msgVehicleStatusRev[] = {
	0xA1, 0x00, //StartVehicleStatusUpdates
	0xA1, 0x02  //StopVehicleStatusUpdates
};

uint8_t msgListUpdatesRev[] = {
	0x41, 0x71,  //ListUpdate
};

uint8_t msgCallStateUpdatesRev[] = {
	0x41, 0x55,  //ListUpdate
};

uint8_t msgDeviceD[]				=	{
	0x4C, 0x01, 0x4C, 0x04, 					/* MediaLibraryInformation & MediaLibraryUpdate*/
	0x4E, 0x09, 0x4E, 0x0A,                     /* DeviceInformationUpdate & DeviceLanguangeUpdate */
	0x50, 0x01,									/* NowPlayingUpdate */ 
	0xDA, 0x01,									/* USBDeviceModeAudioInformation */
	0x68, 0x01,									/* DeviceHIDReport */
	0xEA, 0x00, 0xEA, 0x01,						/* StartExternalAccessoryProtocolSession & StopExternalAccessoryProtocolSession */
	0xAE, 0x01,									/* PowerUpdate */
};

uint8_t msgWifiRevInfo[] = {
	0x57, 0x02, //RequestAccessoryWiFiConfigurationInformation
	0x4E, 0x0D, //WirelessCarPlayUpdate
};

#define POWER_SOURCE_ID			8
uint8_t powerSource[]			=	{0x02};
uint8_t powerSourceN[]			=	{0x00};
#define MAX_CURR_DRAWN_ID		9
uint8_t maxCurrDrawn[]			=	{0x00, 0x00};

#define SUPPORT_EA_PROTO		10
//uint8_t eaProtoDataH[]	=	{
//	0x00, 0x05, 0x00, 0x00, 0x5A,
//	0x00, 0x13, 0x00, 0x01, 'c', 'o', 'm', '.', 'n', 'e', 'u',
//		's', 'o', 'f', 't', '.', 'e', 'a', '\0',
//	0x00, 0x05, 0x00, 0x02, 0x01,
//};
//uint8_t eaProtoDataD[]	=	{
//	0x00, 0x05, 0x00, 0x00, 0x5A,
//	0x00, 0x13, 0x00, 0x01, 'c', 'o', 'm', '.', 'n', 'e', 'u',
//		's', 'o', 'f', 't', '.', 'e', 'a', '\0',
//	0x00, 0x05, 0x00, 0x02, 0x00,
//};
#define CUR_LANG_ID				12
#define CURRENT_LANG			"zh"
#define SUPPORT_LANG_ID			13
#define SUPPORT_LANG_EN			"en"
#define SUPPORT_LANG_ZH			"zh"

#define USB_DEV_TSC_ID			15
uint8_t usbDevTSCData[]			=	{
	0x00, 0x06, 0x00, 0x00, 0x01, 0x00, 
//	0x00, 0x09, 0x00, 0x01, 'i', 'A', 'P', '2', '\0',
	0x00, 0x09, 0x00, 0x01, 'B', 'a', 'i', 'd', 'u', ' ', 'C', 'a', 'r', 'l', 'i', 'f', 'e', '\0',
//	0x00, 0x04, 0x00, 0x02,
//	0x00, 0x05, 0x00, 0x03, 0x06,
//	0x00, 0x05, 0x00, 0x03, 0x07,
//	0x00, 0x05, 0x00, 0x03, 0x08
};

#define USB_HOST_TSC_ID			16
uint8_t usbHostTSCData[]			=	{
	0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x12, 0x00, 0x01, 'B', 'a', 'i', 'd', 'u', ' ', 'C', 'a', 'r', 'l', 'i', 'f', 'e', '\0',
	0x00, 0x04, 0x00, 0x02
//	0x00, 0x05, 0x00, 0x03, 0x00,
};
uint8_t usbHostTSCData1[]			=	{
	0x00, 0x06, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x09, 0x00, 0x01, 'i', 'A', 'P', '2', '\0',
	0x00, 0x04, 0x00, 0x02,
//	0x00, 0x05, 0x00, 0x03, 0x00,
};

#define USB_BT_TSC_ID         	17
#define HID_COMPONENT_ID		18
uint8_t hidCompontentData[]		=	{
	0x00, 0x06, 0x00, 0x00, 0x01, 0x01,
	0x00, 0x0B, 0x00, 0x01, 'R', 'e', 'm', 'o', 't', 'e', '\0',
	0x00, 0x05, 0x00, 0x02, 0x01
};

#define VEHICLEINFO_COMPONENT_ID	20
#define VEHICLESTATUS_COMPONENT_ID	21
#define LOCATIONINFORMATION_COMPONENT_ID	22
#define WIRELESS_COMPONENT_ID               24
uint8_t wireLessTscData[] = {
	0x00, 0x06, 0x00, 0x00, 0x05, WIRELESS_COMPONENT_ID, 	//TransportComponentIdentifier
	0x00, 0x09, 0x00, 0x01, 'i', 'A', 'P', '2', '\0',		//TransportComponentName
	0x00, 0x04, 0x00, 0x02, 								//TransportSupportsiAP2Connection
	0x00, 0x04, 0x00, 0x04  								//TransportSupportsCarPlay
};

//extern MHDevIap2 * iAP2Object;

uint8_t  statusType;
uint8_t  gnssMode;
uint8_t  nowPlayingFlag;
uint8_t  callStateUpdate;
uint8_t  listUpdates;
uint8_t flagLoadConf = 0;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _iap2GetIdentifyParam
 *  Description:  
 * =====================================================================================
 */
void _iap2GetIdentifyParam(IDENTIFY_PARAM idParam[])
{
	GKeyFile * config;
	gchar *str;
	char * _path	= NULL;
	char * _db_path;
	
//	_path	=	getenv("MH_DB_PATH");
	_path	=	_path ? _path : "/etc";
	_db_path	=	g_strdup_printf("%s/mh_conf.ini", _path);
	
	config = g_key_file_new();
	if (!g_key_file_load_from_file(config, _db_path, 0, NULL))
	{
		g_warning("Load config error!");
		flagLoadConf = 0;
		return;
	}
	str = g_key_file_get_string(config,"iap2_conf","MH_ACCESSORY_NAME",NULL);
	idParam[0].param = str;
	idParam[0].length =	strlen(str) + 1;
	
	str = g_key_file_get_string(config,"iap2_conf","MH_MODEL_IDENTIFIER",NULL);
	idParam[1].param = str;
	idParam[1].length =	strlen(str) + 1;
	
	str = g_key_file_get_string(config,"iap2_conf","MH_MANUFACTURE",NULL);
	idParam[2].param = str;
	idParam[2].length =	strlen(str) + 1;

	str = g_key_file_get_string(config,"iap2_conf","MH_SERIAL_NUMBER",NULL);
	idParam[3].param = str;
	idParam[3].length =	strlen(str) + 1;

	str = g_key_file_get_string(config,"iap2_conf","MH_FIRMWARE_VER",NULL);
	idParam[4].param = str;
	idParam[4].length =	strlen(str) + 1;

	str = g_key_file_get_string(config,"iap2_conf","MH_HARDWARE_VER",NULL);
	idParam[5].param = str;
	idParam[5].length =	strlen(str) + 1;

	g_key_file_free(config);
	
	flagLoadConf = 1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _iap2freeIdentifyParam
 *  Description:  
 * =====================================================================================
 */
void _iap2FreeIdentifyParam(IDENTIFY_PARAM idParam[])
{
	int i = 0;
	if (0 == flagLoadConf)
		return;
	for (i = 0; i < MSG_ACCESSORY_ID; i++)
		free(idParam[i].param);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetListUpdatesFlag
 *  Description:  
 * =====================================================================================
 */
uint8_t iAP2GetListUpdatesFlag()
{
	return listUpdates;
}		/* -----  end of function iAP2GetListUpdatesFlag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetCallStateUpdateFlag
 *  Description:  
 * =====================================================================================
 */
uint8_t iAP2GetCallStateUpdateFlag()
{
	return callStateUpdate;
}		/* -----  end of function iAP2GetCallStateUpdateFlag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetNowPlayingUpdateFlag
 *  Description:  
 * =====================================================================================
 */
uint8_t iAP2GetNowPlayingUpdateFlag()
{
	return nowPlayingFlag;
}		/* -----  end of function iAP2GetNowPlayingUpdateFlag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetParamFlag
 *  Description:  
 * =====================================================================================
 */
void iAP2GetParamFlag()
{
	const char * _vehicle	=	getenv("MH_IAP_VEHICLE_SUPPORT_STATUS");
	if( _vehicle	!=	NULL )
	{
		statusType = ( uint8_t )atoi( _vehicle );
	}else{
		statusType = 0xFF;
	}

	const char * _location	=	getenv("MH_IAP_LOCATION_INFO_COMPONENT");
	if( _location	!=	NULL )
	{
		gnssMode = ( uint8_t )atoi( _location );
	}else{
		gnssMode = 0xFF;
	}

	const char * _nowPlaying	=	getenv("MH_IAP_NOW_PLAYING_UPDATE");
	_nowPlaying	=	_nowPlaying ? _nowPlaying : "0";
	nowPlayingFlag = ( uint8_t )atoi( _nowPlaying );

	const char * _callState	=	getenv("MH_IAP_CALL_STATE_UPDATE");
	_callState	=	_callState ? _callState : "0";
	callStateUpdate = ( uint8_t )atoi( _callState );

	const char * _listState	=	getenv("MH_IAP_LIST_UPDATES");
	_listState	=	_listState ? _listState : "0";
	listUpdates = ( uint8_t )atoi( _listState );

	g_message("nowPlayingFlag = %d, callStateUpdate = %d, listUpdates=%d",nowPlayingFlag, callStateUpdate, listUpdates);
}		/* -----  end of function iAP2GetParamFlag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetBTComponentParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetBTComponentParam ( uint8_t ** idParams, int * paramsLen )
{
	int i = 0;
	int j = 0;
	uint8_t BtComponentData[] = {
		0x00, 0x06, 0x00, 0x00, 0x05, USB_BT_TSC_ID, 			//TransportComponentIdentifier
		0x00, 0x09, 0x00, 0x01, 'i', 'A', 'P', '2', '\0',		//TransportComponentName
		0x00, 0x04, 0x00, 0x02, 								//TransportSupportsiAP2Connection
		0x00, 0x0A, 0x00, 0x03,0x00,0x00,0x00,0x00,0x00,0x00 	//BluetoothTransportMediaAccessControlAddress
	};

	uint8_t * btData_p =  BtComponentData + sizeof(BtComponentData)-6;
	uint8_t * strid = ( uint8_t * )mh_misc_get_bluetoothids();;
	uint8_t  tmpbyte[2]; // save one AscII character
	int btAddresslen = strlen((const char*)strid);
	if( btAddresslen == 17){
		for(i = 0; i < 6; i++){
			for(j=0;j<2;j++){
				if( (strid[j] >= '0') && (strid[j] <= '9')){
					tmpbyte[j] = strid[j] - '0';
				}
				else if( (strid[j] >= 'a') && (strid[j] <= 'f')){
					tmpbyte[j] = strid[j] - 'a' + 10;
				}
				else if( (strid[j] >= 'A') && (strid[j] <= 'F')){
					tmpbyte[j] = strid[j] - 'A' + 10;
				}
				else{
					tmpbyte[j] = 0;
					g_message("CarPlay BluetoothTransportComponent btaddress err[%s]",strid);
				}
			}

			*btData_p = tmpbyte[0]*16 + tmpbyte[1];
			g_message("CarPlay btaddress [%X]",*btData_p);

			strid += 3;
			btData_p++;
		}
	}
	else{
		g_message("CarPlay BluetoothTransportComponent btaddress length err[%d]",btAddresslen);
	}

	uint8_t * _paramBuf	=	NULL;
	int _len	=	sizeof(BtComponentData);
	_paramBuf	=	g_malloc0( _len );

	memcpy( _paramBuf, BtComponentData, _len );

	* idParams	=	_paramBuf;
	* paramsLen	=	_len;
}		/* -----  end of function iAP2GetUsbHostTSCParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetUsbHostTSCParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetUsbHostTSCParam ( MHDevIap2 * _iAP2Object, uint8_t ** idParams, int * paramsLen )
{
	uint8_t _usbHostTSCInterfaceNum[] = {
		0x00, 0x05, 0x00, 0x03, 0x03
	};	
	uint8_t _usbHostTSCCarplayInterfaceNum[] = {
		0x00, 0x05, 0x00, 0x03, 0x00,
		0x00, 0x04, 0x00, 0x04
	};	

	uint8_t   _usbHostTSCData[255];
	uint8_t   _usbHostTSCLen;
	uint8_t * _usbHostTSCData_p;

	memset( _usbHostTSCData, 0, sizeof( _usbHostTSCData ));
	_usbHostTSCLen 	  = 0;
	_usbHostTSCData_p = _usbHostTSCData;

	if( _iAP2Object->mode	==	MISC_IAP_CARLIFE)
	{
		memcpy( _usbHostTSCData_p, usbHostTSCData, sizeof( usbHostTSCData ));
		_usbHostTSCLen    += sizeof( usbHostTSCData );
		_usbHostTSCData_p += sizeof( usbHostTSCData );
	}
	else
	{
		memcpy( _usbHostTSCData_p, usbHostTSCData1, sizeof( usbHostTSCData1 ));
		_usbHostTSCLen    += sizeof( usbHostTSCData1 );
		_usbHostTSCData_p += sizeof( usbHostTSCData1 );

	}


	if( _iAP2Object->mode == MISC_IAP )
	{
		memcpy( _usbHostTSCData_p, _usbHostTSCInterfaceNum, sizeof( _usbHostTSCInterfaceNum ));
		_usbHostTSCLen    += sizeof( _usbHostTSCInterfaceNum );
		_usbHostTSCData_p += sizeof( _usbHostTSCInterfaceNum );
	}
	else if( _iAP2Object->mode	==	MISC_IAP_CARLIFE)
	{
//		memcpy( _usbHostTSCData_p, _usbHostTSCInterfaceNum, sizeof( _usbHostTSCInterfaceNum ));
//		_usbHostTSCLen    += sizeof( _usbHostTSCInterfaceNum );
//		_usbHostTSCData_p += sizeof( _usbHostTSCInterfaceNum );

	}

	else
	{
		memcpy( _usbHostTSCData_p, _usbHostTSCCarplayInterfaceNum, sizeof( _usbHostTSCCarplayInterfaceNum ));
		_usbHostTSCLen    += sizeof( _usbHostTSCCarplayInterfaceNum );
		_usbHostTSCData_p += sizeof( _usbHostTSCCarplayInterfaceNum );
	}

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _usbHostTSCLen );

	memcpy( _paramBuf, _usbHostTSCData, _usbHostTSCLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_usbHostTSCLen;
}		/* -----  end of function iAP2GetUsbHostTSCParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetMessagesReceivedFromDeviceParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetMessagesReceivedFromDeviceParam( MHDevIap2 * _iAP2Object, uint8_t ** idParams, int * paramsLen )
{
	uint8_t _msgAccessory [255];
	uint8_t _msgLen = 0;

	if( _iAP2Object->mode == MISC_IAP )
	{
		if( _iAP2Object->transportType ==  MH_DEV_BT_IAP )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgEARev, sizeof( msgEARev ));
			_msgLen += sizeof( msgEARev );
		}
		else
		{
			if( _iAP2Object->hostMode == TRUE)
			{
				memcpy( &_msgAccessory[ _msgLen ], msgMediaLibraryRev, sizeof( msgMediaLibraryRev ));
				_msgLen += sizeof( msgMediaLibraryRev );

				memcpy( &_msgAccessory[ _msgLen ], msgHIDRev, sizeof( msgHIDRev ));
				_msgLen += sizeof( msgHIDRev );

				memcpy( &_msgAccessory[ _msgLen ], msgDeviceNotificationsRev, sizeof( msgDeviceNotificationsRev ));
				_msgLen += sizeof( msgDeviceNotificationsRev );

				memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdateRev, sizeof( msgPowerUpdateRev ));
				_msgLen += sizeof( msgPowerUpdateRev );

				memcpy( &_msgAccessory[ _msgLen ], msgNowPlayingRev, sizeof( msgNowPlayingRev ));
				_msgLen += sizeof( msgNowPlayingRev );
//
				memcpy( &_msgAccessory[ _msgLen ], msgEARev, sizeof( msgEARev ));
				_msgLen += sizeof( msgEARev );

//				if ( statusType != 0xFF )
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgVehicleStatusRev, sizeof( msgVehicleStatusRev ));
//					_msgLen += sizeof( msgVehicleStatusRev );
//				}
//
//				if ( gnssMode != 0xFF )
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgLocationInfoRev, sizeof( msgLocationInfoRev ));
//					_msgLen += sizeof( msgLocationInfoRev );
//				}
//
//				if( callStateUpdate == 1)
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgCallStateUpdatesRev, sizeof( msgCallStateUpdatesRev ));
//					_msgLen += sizeof( msgCallStateUpdatesRev );
//				}
//
//				if( listUpdates == 1)
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgListUpdatesRev, sizeof( msgListUpdatesRev ));
//					_msgLen += sizeof( msgListUpdatesRev );
//				}
			}else{
				memcpy( &_msgAccessory[ _msgLen ], msgDeviceD, sizeof( msgDeviceD ));
				_msgLen += sizeof( msgDeviceD );
			}
		}
	}
	else if( _iAP2Object->mode	==	MISC_IAP_CARLIFE)
	{
//		if( _iAP2Object->hostMode == TRUE)
//		{
//		
//			g_message("\n\n\n\n\n MISC_IAP_CARLIFE--->hostMode\n\n\n\n");
//		
//			memcpy( &_msgAccessory[ _msgLen ], msgDeviceNotificationsRev, sizeof( msgDeviceNotificationsRev ));
//			_msgLen += sizeof( msgDeviceNotificationsRev );
//
			memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdateRev, sizeof( msgPowerUpdateRev ));
			_msgLen += sizeof( msgPowerUpdateRev );
//		
//		}
//		else
//		{
//			memcpy( &_msgAccessory[ _msgLen ], msgDeviceD, sizeof( msgDeviceD ));
//			_msgLen += sizeof( msgDeviceD );
//		}

	}
	else if( _iAP2Object->mode == MISC_IAP_CARPLAY || _iAP2Object->mode	== MISC_IAP_CARPLAY_CARLIFE )
	{
		switch( _iAP2Object->transportType )
		{
			case MH_DEV_USB_IAP:
#ifdef NAGIVI
		memcpy( &_msgAccessory[ _msgLen ], msgEARev, sizeof( msgEARev ));
		_msgLen += sizeof( msgEARev );
#endif
			case MH_DEV_WIFI_IAP:
		memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdateRev, sizeof( msgPowerUpdateRev ));
		_msgLen += sizeof( msgPowerUpdateRev );

		memcpy( &_msgAccessory[ _msgLen ], msgDeviceNotificationsRev, sizeof( msgDeviceNotificationsRev ));
		_msgLen += sizeof( msgDeviceNotificationsRev );
		
		memcpy( &_msgAccessory[ _msgLen ], msgDeviceNotificationsOnlyCarplayRev, sizeof( msgDeviceNotificationsOnlyCarplayRev ));
		_msgLen += sizeof( msgDeviceNotificationsOnlyCarplayRev );

		if( nowPlayingFlag == 1 )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgNowPlayingRev, sizeof( msgNowPlayingRev ));
			_msgLen += sizeof( msgNowPlayingRev );
		}

		if ( statusType != 0xFF )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgVehicleStatusRev, sizeof( msgVehicleStatusRev ));
			_msgLen += sizeof( msgVehicleStatusRev );
		}

		if ( gnssMode != 0xFF )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgLocationInfoRev, sizeof( msgLocationInfoRev ));
			_msgLen += sizeof( msgLocationInfoRev );
		}

		if( callStateUpdate == 1)
		{
			memcpy( &_msgAccessory[ _msgLen ], msgCallStateUpdatesRev, sizeof( msgCallStateUpdatesRev ));
			_msgLen += sizeof( msgCallStateUpdatesRev );
		}

		if( listUpdates == 1)
		{
			memcpy( &_msgAccessory[ _msgLen ], msgListUpdatesRev, sizeof( msgListUpdatesRev ));
			_msgLen += sizeof( msgListUpdatesRev );
		}
				break;
			case MH_DEV_BT_IAP:
				memcpy( &_msgAccessory[ _msgLen ], msgWifiRevInfo, sizeof( msgWifiRevInfo ));
				_msgLen += sizeof( msgWifiRevInfo );
				break;
			default:
				break;
		}

	}else{
		g_message("device mode is error!\n");
		* idParams	= NULL;
		* paramsLen	= 0;
		return;
	}

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _msgLen );

	memcpy( _paramBuf, _msgAccessory, _msgLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_msgLen;

}		/* -----  end of function iAP2GetMessagesReceivedFromDeviceParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetMessagesSentByAccessoryParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetMessagesSentByAccessoryParam( MHDevIap2 * _iAP2Object, uint8_t ** idParams, int * paramsLen )
{
	uint8_t _msgAccessory [255];
	uint8_t _msgLen = 0;

	if( _iAP2Object->mode == MISC_IAP )
	{
		if( _iAP2Object->transportType ==  MH_DEV_BT_IAP )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgAppLanch, sizeof( msgAppLanch ));
			_msgLen += sizeof( msgAppLanch );
		}
		else
		{
			if( _iAP2Object->hostMode == TRUE )
			{
				memcpy( &_msgAccessory[ _msgLen ], msgMediaLibraryInfo, sizeof( msgMediaLibraryInfo ));
				_msgLen += sizeof( msgMediaLibraryInfo );

				memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdate, sizeof( msgPowerUpdate ));
				_msgLen += sizeof( msgPowerUpdate );

				memcpy( &_msgAccessory[ _msgLen ], msgNowPlaying, sizeof( msgNowPlaying ));
				_msgLen += sizeof( msgNowPlaying );

				memcpy( &_msgAccessory[ _msgLen ], msgHID, sizeof( msgHID ));
				_msgLen += sizeof( msgHID );

				memcpy( &_msgAccessory[ _msgLen ], msgAppLanch, sizeof( msgAppLanch ));
				_msgLen += sizeof( msgAppLanch );

//				if ( statusType != 0xFF )
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgVehicleStatus, sizeof( msgVehicleStatus ));
//					_msgLen += sizeof( msgVehicleStatus );
//				}
//
//				if ( gnssMode != 0xFF )
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgLocationInfo, sizeof( msgLocationInfo ));
//					_msgLen += sizeof( msgLocationInfo );
//				}
//
//				if( callStateUpdate == 1)
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgCallState, sizeof( msgCallState ));
//					_msgLen += sizeof( msgCallState );
//				}
//
//				if( listUpdates == 1)
//				{
//					memcpy( &_msgAccessory[ _msgLen ], msgListUpdates, sizeof( msgListUpdates ));
//					_msgLen += sizeof( msgListUpdates );
//				}
			}else{
				memcpy( &_msgAccessory[ _msgLen ], msgAccessoryD, sizeof( msgAccessoryD ));
				_msgLen += sizeof( msgAccessoryD );
			}
		}
	}
	else if( _iAP2Object->mode	==	MISC_IAP_CARLIFE)
	{
		if( _iAP2Object->hostMode == TRUE )
		{

			memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdate, sizeof( msgPowerUpdate ));
			_msgLen += sizeof( msgPowerUpdate );


			memcpy( &_msgAccessory[ _msgLen ], msgAppLanch, sizeof( msgAppLanch ));
			_msgLen += sizeof( msgAppLanch );




		}
//		else
//		{
//			memcpy( &_msgAccessory[ _msgLen ], msgAccessoryD, sizeof( msgAccessoryD ));
//			_msgLen += sizeof( msgAccessoryD );
//		}

	}
        else if( _iAP2Object->mode == MISC_IAP_CARPLAY || _iAP2Object->mode	==	MISC_IAP_CARPLAY_CARLIFE )
	{
		switch( _iAP2Object->transportType )
		{
			case MH_DEV_USB_IAP:
			case MH_DEV_WIFI_IAP:
				if( _iAP2Object->transportType == MH_DEV_WIFI_IAP )
				{
					memcpy( &_msgAccessory[ _msgLen ], msgWifiPowerUpdate, sizeof( msgWifiPowerUpdate ));
					_msgLen += sizeof( msgWifiPowerUpdate );
				}
				else
				{
					memcpy( &_msgAccessory[ _msgLen ], msgPowerUpdate, sizeof( msgPowerUpdate ));
					_msgLen += sizeof( msgPowerUpdate );
				}

		if( nowPlayingFlag == 1 )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgNowPlaying, sizeof( msgNowPlaying ));
			_msgLen += sizeof( msgNowPlaying );
		}

		if ( statusType != 0xFF )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgVehicleStatus, sizeof( msgVehicleStatus ));
			_msgLen += sizeof( msgVehicleStatus );
		}

		if ( gnssMode != 0xFF )
		{
			memcpy( &_msgAccessory[ _msgLen ], msgLocationInfo, sizeof( msgLocationInfo ));
			_msgLen += sizeof( msgLocationInfo );
		}

		if( callStateUpdate == 1)
		{
			memcpy( &_msgAccessory[ _msgLen ], msgCallState, sizeof( msgCallState ));
			_msgLen += sizeof( msgCallState );
		}

		if( listUpdates == 1)
		{
			memcpy( &_msgAccessory[ _msgLen ], msgListUpdates, sizeof( msgListUpdates ));
			_msgLen += sizeof( msgListUpdates );
		}
				break;
			case MH_DEV_BT_IAP:
				memcpy( &_msgAccessory[ _msgLen ], msgWifiConfInfo, sizeof( msgWifiConfInfo ));
				_msgLen += sizeof( msgWifiConfInfo );
				break;
			default:
				break;
		}
	}else{
		g_message("device mode is error!\n");
		* idParams	= NULL;
		* paramsLen	= 0;
		return;
	}

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _msgLen );

	memcpy( _paramBuf, _msgAccessory, _msgLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_msgLen;
	
}		/* -----  end of function iAP2GetMessagesSentByAccessoryParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetVehicleStatusIndentifyParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetVehicleStatusIndentifyParam ( uint8_t ** idParams, int * paramsLen )
{
	uint8_t _vehicleStatusCompontentDataHead[] = {
		0x00, 0x06, 0x00, 0x00, 0x01, 0x04,
		0x00, 0x09, 0x00, 0x01, 'A', 'm', 'a', 'U', '\0',
	};	

	uint8_t _vehicleStatusCompontent[] = {
		0x00, 0x04, 0x00, 0x00
	};

	uint8_t   _vehicleCompontentData[255];
	uint8_t   _vehicleCompontentLen;
	uint8_t * _vehicleCompontentData_p;

	g_message("CarPlay get vehicle status [%d] ", statusType );

	memset( _vehicleCompontentData, 0, sizeof( _vehicleCompontentData ));
	_vehicleCompontentLen 	 = 0;
	_vehicleCompontentData_p = _vehicleCompontentData;

	//Add VehicleStatusCompontentDataHead
	memcpy( _vehicleCompontentData_p, _vehicleStatusCompontentDataHead, sizeof( _vehicleStatusCompontentDataHead ));
	_vehicleCompontentLen 	 += sizeof( _vehicleStatusCompontentDataHead );
	_vehicleCompontentData_p += sizeof( _vehicleStatusCompontentDataHead );

	if(( statusType & 0x1 ) != 0 ){	  //Add Range
		_vehicleStatusCompontent[3] = 3;
		memcpy( _vehicleCompontentData_p, _vehicleStatusCompontent, sizeof( _vehicleStatusCompontent ));
		_vehicleCompontentLen    += sizeof( _vehicleStatusCompontent );
		_vehicleCompontentData_p += sizeof( _vehicleStatusCompontent );
	}

	if(( statusType & 0x2 ) != 0 ){	  //Add OutsideTemperature
		_vehicleStatusCompontent[3] = 4;
		memcpy( _vehicleCompontentData_p, _vehicleStatusCompontent, sizeof( _vehicleStatusCompontent ));
		_vehicleCompontentLen    += sizeof( _vehicleStatusCompontent );
		_vehicleCompontentData_p += sizeof( _vehicleStatusCompontent );
	}

	if(( statusType & 0x4 ) != 0 ){	  //Add RangeWarning
		_vehicleStatusCompontent[3] = 6;
		memcpy( _vehicleCompontentData_p, _vehicleStatusCompontent, sizeof( _vehicleStatusCompontent ));
		_vehicleCompontentLen    += sizeof( _vehicleStatusCompontent );
		_vehicleCompontentData_p += sizeof( _vehicleStatusCompontent );
	}

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _vehicleCompontentLen );

	memcpy( _paramBuf, _vehicleCompontentData, _vehicleCompontentLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_vehicleCompontentLen;
}		/* -----  end of function iAP2GetVehicleStatusIndentifyParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetVehicleInfoIndentifyParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetVehicleInfoIndentifyParam ( uint8_t ** idParams, int * paramsLen )
{
	const char * _component_type	=	getenv("MH_IAP_VEHICLE_ENGINE_TYPE");
	_component_type	=	_component_type ? _component_type : "0";
	uint8_t _engineType = ( uint8_t )atoi( _component_type );

	const char * _component_name	=	getenv("MH_IAP_VEHICLE_DISPLAY_NAME");
	_component_name	=	_component_name ? _component_name : "Neusoft";
	
	uint8_t _vehicleInfoCompontentDataHead[] = {
		0x00, 0x06, 0x00, 0x00, 0x01, 0x03,
		0x00, 0x0A, 0x00, 0x01, 'B', 'o', 'N', 'A', 'm', '\0',
	};	
	uint8_t _vehicleInfoCompontentEngineType[] = {
		0x00, 0x05, 0x00, 0x02, 0x00 
	};

	g_message("CarPlay get vehicle engine type [%d] ", _engineType );
	g_message("CarPlay get vehicle display name [%s] ", _component_name );

	_vehicleInfoCompontentEngineType[4]	=	_engineType;

	uint8_t   _vehiclenCompontentData[255];
	uint8_t   _vehicleCompontentLen;
	uint8_t * _vehicleCompontentData_p;

	memset( _vehiclenCompontentData, 0, sizeof( _vehiclenCompontentData ));
	_vehicleCompontentLen    = 0;
	_vehicleCompontentData_p = _vehiclenCompontentData;

	//Add VehicleInfoCompontentDataHead
	memcpy( _vehicleCompontentData_p, _vehicleInfoCompontentDataHead, sizeof( _vehicleInfoCompontentDataHead ));
	_vehicleCompontentLen    += sizeof( _vehicleInfoCompontentDataHead );
	_vehicleCompontentData_p += sizeof( _vehicleInfoCompontentDataHead );

	//Add VehicleInfoCompontentEngineType
	memcpy( _vehicleCompontentData_p, _vehicleInfoCompontentEngineType, sizeof( _vehicleInfoCompontentEngineType ));
	_vehicleCompontentLen    += sizeof( _vehicleInfoCompontentEngineType );
	_vehicleCompontentData_p += sizeof( _vehicleInfoCompontentEngineType );

	uint8_t _dataLen	=	4 + strlen( _component_name ) + 1;
	uint8_t _displayNameData[ _dataLen ];
	_displayNameData[0]	=	0x00;
	_displayNameData[1]	=	_dataLen;
	_displayNameData[2]	=	0x00;
	_displayNameData[3]	=	0x06;
	memcpy( _displayNameData + 4, _component_name, strlen( _component_name ));
//	memcpy( _displayNameData + 4, _component_name, sizeof( _component_name ));
	_displayNameData[_dataLen - 1]	=	'\0';

	memcpy( _vehicleCompontentData_p, _displayNameData, _dataLen );
	_vehicleCompontentLen    += _dataLen;
	_vehicleCompontentData_p += _dataLen;

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _vehicleCompontentLen );

	memcpy( _paramBuf, _vehiclenCompontentData, _vehicleCompontentLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_vehicleCompontentLen;
}		/* -----  end of function iAP2GetVehicleInfoIndentifyParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetLocationIndentifyParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetLocationIndentifyParam ( uint8_t ** idParams, int * paramsLen )
{
	uint8_t _gnssModeData;

	if( gnssMode	== 0xFF )
	{
		_gnssModeData = 7;
	}else{
		_gnssModeData = gnssMode;
	}

	uint8_t _locCompontentDataHead[] = {
		0x00, 0x06, 0x00, 0x00, 0x01, 0x02,
		0x00, 0x09, 0x00, 0x01, 'G', 'N', 'S', 'S', '\0',
	};	
	uint8_t _locCompontentNMEA[] = {
		0x00, 0x04, 0x00, 0x00,  // NMEA  sentences
	};

	uint8_t   _locationCompontentData[255];
	uint8_t   _locationCompontentLen;
	uint8_t * _locationCompontentData_p;

	if(( _gnssModeData & 0x7F ) == 0 )
	{
		* idParams	=	NULL;
		* paramsLen	=	0;
		return;
	}

	g_message( "CarPlay get GNSSMode[%d]",_gnssModeData );

	memset( _locationCompontentData, 0, sizeof( _locationCompontentData ));
	_locationCompontentLen 	  = 0;
	_locationCompontentData_p = _locationCompontentData;

	//Add LocCompontentDataHead
	memcpy( _locationCompontentData_p, _locCompontentDataHead, sizeof( _locCompontentDataHead ));
	_locationCompontentLen    += sizeof( _locCompontentDataHead );
	_locationCompontentData_p += sizeof( _locCompontentDataHead );

	if(( _gnssModeData & 0x1 ) != 0 ){	  //Add GPGGA
		_locCompontentNMEA[3] = 17;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}

	if(( _gnssModeData & 0x2 ) != 0 ){	 //Add GPRMC
		_locCompontentNMEA[3] = 18;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}

	if(( _gnssModeData & 0x4 ) != 0 ){	  //Add GPGSV
		_locCompontentNMEA[3] = 19;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}
	  
	if(( _gnssModeData & 0x8 ) != 0 ){  	  //Add PASCD
		_locCompontentNMEA[3] = 20;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}
  
	if(( _gnssModeData & 0x10 ) != 0 ){	//Add PAGCD
		_locCompontentNMEA[3] = 21;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}
	
	if(( _gnssModeData & 0x20 ) != 0 ){	//Add PAACD
		_locCompontentNMEA[3] = 22;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}
	
	if(( _gnssModeData & 0x40 ) != 0 ){  //Add GPHDT
		_locCompontentNMEA[3] = 23;
		memcpy( _locationCompontentData_p, _locCompontentNMEA, sizeof( _locCompontentNMEA ));
		_locationCompontentLen    += sizeof( _locCompontentNMEA );
		_locationCompontentData_p += sizeof( _locCompontentNMEA );
	}

	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _locationCompontentLen );

	memcpy( _paramBuf, _locationCompontentData, _locationCompontentLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_locationCompontentLen;
}		/* -----  end of function iAP2GetLocationIndentifyParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetEAIndentifyParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetEAIndentifyParam ( MHDevIap2 * _iAP2Object, const char * _eaProName, int _ProtocolIdentifier, uint8_t ** idParams, int * paramsLen )
{
//	const char * _eaProName	=	getenv("MH_EA_PROTOCOL_NAME");
//	_eaProName	=	_eaProName ? _eaProName : "com.neusot.ea";
//	//_eaProName	=	_eaProName ? "com.ford.sync.prot0" : "com.neusot.ea";
	uint32_t _eaProNameLen	=	strlen( _eaProName );
	g_message("_eaProName = %s _ProtocolIdentifier = %d \n", _eaProName,_ProtocolIdentifier);
#ifdef NAGIVI
	uint32_t _eaProDataLen	=	5 + 4 + _eaProNameLen + 1 + 5 + 4;
#else
	uint32_t _eaProDataLen	=	5 + 4 + _eaProNameLen + 1 + 5 + 6;
#endif
	uint8_t eaProtoData[ _eaProDataLen ];
	int i;
	//ExternalAccessoryProtocolIdentifier
	eaProtoData[0]	=	0x00;
	eaProtoData[1]	=	0x05;
	eaProtoData[2]	=	0x00;
	eaProtoData[3]	=	0x00;
	//eaProtoData[4]	=	0x5A;
	eaProtoData[4]	=	_ProtocolIdentifier;

	//ExternalAccessoryProtocolName
	eaProtoData[5]	=	0x00;
	eaProtoData[6]	=	5 + _eaProNameLen;
	eaProtoData[7]	=	0x00;
	eaProtoData[8]	=	0x01;
	for( i = 0; i < _eaProNameLen; i++)
	{
		eaProtoData[9+i]	=	_eaProName[i];
	}
	eaProtoData[9+i]	=	'\0';

	//ExternalAccessoryProtocolMatchAction
	eaProtoData[10+i]	=	0x00;
	eaProtoData[11+i]	=	0x05;
	eaProtoData[12+i]	=	0x00;
	eaProtoData[13+i]	=	0x02;
//	if( _iAP2Object->hostMode == TRUE )
//	{
//		eaProtoData[14+i]	=	0x01;
//	}else{
	eaProtoData[14+i]	=	0x01;
//	}
	
#ifdef NAGIVI
	if(( _iAP2Object->hostMode == TRUE )&&(_iAP2Object->mode == MISC_IAP_CARPLAY)&&
	( _iAP2Object->transportType == MH_DEV_USB_IAP )) 
	{
		eaProtoData[15+i]	=	0x00;
		eaProtoData[16+i]	=	0x04;
		eaProtoData[17+i]	=	0x00;
		eaProtoData[18+i]	=	0x04;
	}
	else
	{
		_eaProDataLen = _eaProDataLen - 4;
	}
#else
	if(( _iAP2Object->hostMode == TRUE )&&( _iAP2Object->transportType == MH_DEV_USB_IAP )) 
	{
		eaProtoData[15+i]	=	0x00;
		eaProtoData[16+i]	=	0x06;
		eaProtoData[17+i]	=	0x00;
		eaProtoData[18+i]	=	0x03;
		eaProtoData[19+i]	=	0x00;
		eaProtoData[20+i]	=	0x00;
	}
	else
	{
		_eaProDataLen = _eaProDataLen - 6;
	}

#endif
	uint8_t * _paramBuf	=	NULL;

	_paramBuf	=	g_malloc0( _eaProDataLen );

	memcpy( _paramBuf, eaProtoData, _eaProDataLen );

	* idParams	=	_paramBuf;
	* paramsLen	=	_eaProDataLen;
}		/* -----  end of function iAP2GetEAIndentifyParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetIndentifyParam
 *  Description:  
 * =====================================================================================
 */
void iAP2GetIndentifyParam ( MHDevIap2 * _iAP2Object, uint8_t ** idParams, int * paramsLen )
{
	iAP2GetParamFlag();

	uint8_t * _eaIndentifyParam			=	NULL;
	uint8_t * _eaIndentifyParam_other			=	NULL;
	int 	  _eaIndentifyParamLen		=	0;
	int 	  _eaIndentifyParamLen_other		=	0;

	uint8_t * _msgAccessoryParam		=	NULL;
	int 	  _msgAccessoryParamLen		=	0;
	iAP2GetMessagesSentByAccessoryParam( _iAP2Object, &_msgAccessoryParam, &_msgAccessoryParamLen );
	DEBUG_HEX_DISPLAY( _msgAccessoryParam, _msgAccessoryParamLen );
	
	uint8_t * _msgDeviceParam			=	NULL;
	int 	  _msgDeviceParamLen		=	0;
	iAP2GetMessagesReceivedFromDeviceParam(_iAP2Object, &_msgDeviceParam, &_msgDeviceParamLen );
	DEBUG_HEX_DISPLAY( _msgDeviceParam, _msgDeviceParamLen );

	uint8_t * _usbHostTSCParam			=	NULL;
	int 	  _usbHostTSCParamLen		=	0;

	//locationInformation
	uint8_t * _locationParam			=	NULL;
	int 	  _locationParamLen			=	0;
	
	//VehicleInformationComponent
	uint8_t * _vehicleInfoParam			=	NULL;
	int 	  _vehicleInfoParamLen		=	0;

	//VehicleInformationComponent
	uint8_t * _vehicleStatusParam		=	NULL;
	int 	  _vehicleStatusParamLen	=	0;

	//BTTSCComponent
	uint8_t * _btTscParam		=	NULL;
	int 	  _btTscParamLen	=	0;

	IDENTIFY_PARAM _devModeParams[26]	=	{
		{
			.id		=	ACCESSORY_NAME_ID,
			.param	=	( uint8_t * )ACCESSORY_NAME,
			.length	=	sizeof( ACCESSORY_NAME )
		},

		{
			.id		=	MODEL_IDENTIFIER_ID,
			.param	=	( uint8_t * )MODEL_IDENTIFIER,
			.length	=	sizeof( MODEL_IDENTIFIER )
		},

		{
			.id		=	MANUFACTURE_ID,
			.param	=	( uint8_t * )MANUFACTURE,
			.length	=	sizeof( MANUFACTURE )
		},

		{
			.id		=	SN_ID,
			.param	=	( uint8_t * )SERIAL_NUMBER,
			.length	=	sizeof( SERIAL_NUMBER )
		},

		{
			.id		=	FM_VER_ID,
			.param	=	( uint8_t * )FIRMWARE_VER,
			.length	=	sizeof( FIRMWARE_VER )
		},

		{
			.id		=	HW_VER_ID,
			.param	=	( uint8_t * )HARDWARE_VER,
			.length	=	sizeof( HARDWARE_VER )
		},

		{
			.id		=	MSG_ACCESSORY_ID,
			.param	=	_msgAccessoryParam,
			.length	=	_msgAccessoryParamLen
		},

		{
			.id		=	MSG_DEVICE_ID,
			.param	=	_msgDeviceParam,
			.length	=	_msgDeviceParamLen
		},

		{
			.id		=	POWER_SOURCE_ID,
			.param	=	powerSource,
			.length	=	sizeof( powerSource )
		},

		{
			.id		=	MAX_CURR_DRAWN_ID,
			.param	=	maxCurrDrawn,
			.length	=	sizeof( maxCurrDrawn )
		},

		{
			.id		=	CUR_LANG_ID,
			.param	=	( uint8_t * )CURRENT_LANG,
			.length	=	sizeof( CURRENT_LANG )
		},

		{
			.id		=	SUPPORT_LANG_ID,
			.param	=	( uint8_t * )SUPPORT_LANG_EN,
			.length	=	sizeof( SUPPORT_LANG_EN )
		},

		{
			.id		=	SUPPORT_LANG_ID,
			.param	=	( uint8_t * )SUPPORT_LANG_ZH,
			.length	=	sizeof( SUPPORT_LANG_ZH )
		},

		{
			.id		=	HID_COMPONENT_ID,
			.param	=	hidCompontentData,
			.length	=	sizeof( hidCompontentData )
		}
	};
	
	_iap2GetIdentifyParam(_devModeParams);
	g_message("_devModeParams[0].param:%s \t length:%d", _devModeParams[0].param, _devModeParams[0].length);
	g_message("_devModeParams[1].param:%s \t length:%d", _devModeParams[1].param, _devModeParams[1].length);
	g_message("_devModeParams[2].param:%s \t length:%d", _devModeParams[2].param, _devModeParams[2].length);
	g_message("_devModeParams[3].param:%s \t length:%d", _devModeParams[3].param, _devModeParams[3].length);
	g_message("_devModeParams[4].param:%s \t length:%d", _devModeParams[4].param, _devModeParams[4].length);
	g_message("_devModeParams[5].param:%s \t length:%d", _devModeParams[5].param, _devModeParams[5].length);

	uint8_t _devModeParamsLen	=	14;	
	const char * _eaProName	=	getenv("MH_EA_PROTOCOL_NAME");
	_eaProName	=	_eaProName ? _eaProName : "com.neusoft.ea";
	
	const char * _eaProName_01	=	getenv("MH_EA_PROTOCOL_NAME_01");
//	_eaProName_01	=	_eaProName_01 ? _eaProName_01 : "com.ford.sync.prot0";

	if( _iAP2Object->mode != MISC_IAP_CARPLAY )
	{
		g_message("SUPPORT_EA_PROTO IAP2");

		iAP2GetEAIndentifyParam( _iAP2Object, _eaProName, 0x00, &_eaIndentifyParam, &_eaIndentifyParamLen );
		_devModeParams[_devModeParamsLen].id		=	SUPPORT_EA_PROTO;
		_devModeParams[_devModeParamsLen].param		=	_eaIndentifyParam;
		_devModeParams[_devModeParamsLen].length	=	_eaIndentifyParamLen;
		_devModeParamsLen ++;
		if (NULL != _eaProName_01)
		{
			iAP2GetEAIndentifyParam( _iAP2Object, _eaProName_01, 0x5B, &_eaIndentifyParam_other, &_eaIndentifyParamLen_other );
			_devModeParams[_devModeParamsLen].id		=	SUPPORT_EA_PROTO;
			_devModeParams[_devModeParamsLen].param		=	_eaIndentifyParam_other;
			_devModeParams[_devModeParamsLen].length	=	_eaIndentifyParamLen_other;
			_devModeParamsLen ++;
		}
	}else{
		g_message("_iAP2Object->transportType = %d", _iAP2Object->transportType);
#ifdef NAGIVI
		if (NULL != _eaProName && ( _iAP2Object->transportType == MH_DEV_USB_IAP ))
		{
			g_message("SUPPORT_EA_PROTO CARPLAY");
			iAP2GetEAIndentifyParam( _iAP2Object, _eaProName, 0x5A, &_eaIndentifyParam, &_eaIndentifyParamLen );
			_devModeParams[_devModeParamsLen].id		=	SUPPORT_EA_PROTO;
			_devModeParams[_devModeParamsLen].param 	=	_eaIndentifyParam;
			_devModeParams[_devModeParamsLen].length	=	_eaIndentifyParamLen;
			_devModeParamsLen ++;
			if (NULL != _eaProName_01)
			{
				iAP2GetEAIndentifyParam( _iAP2Object, _eaProName_01, 0x5B, &_eaIndentifyParam_other, &_eaIndentifyParamLen_other );
				_devModeParams[_devModeParamsLen].id		=	SUPPORT_EA_PROTO;
				_devModeParams[_devModeParamsLen].param		=	_eaIndentifyParam_other;
				_devModeParams[_devModeParamsLen].length	=	_eaIndentifyParamLen_other;
				_devModeParamsLen ++;
			}
		}
#endif
		iAP2GetVehicleInfoIndentifyParam( &_vehicleInfoParam, &_vehicleInfoParamLen );

		_devModeParams[_devModeParamsLen].id		=	VEHICLEINFO_COMPONENT_ID;
		_devModeParams[_devModeParamsLen].param		=	_vehicleInfoParam;
		_devModeParams[_devModeParamsLen].length	=	_vehicleInfoParamLen;
		_devModeParamsLen ++;
		if( statusType != 0xFF )
		{
			iAP2GetVehicleStatusIndentifyParam( &_vehicleStatusParam, &_vehicleStatusParamLen );

			_devModeParams[_devModeParamsLen].id		=	VEHICLESTATUS_COMPONENT_ID;
			_devModeParams[_devModeParamsLen].param		=	_vehicleStatusParam;
			_devModeParams[_devModeParamsLen].length	=	_vehicleStatusParamLen;
			_devModeParamsLen ++;
		}
		if (( gnssMode != 0xFF )&& ( _iAP2Object->transportType	!=	MH_DEV_BT_IAP ))
		{
			iAP2GetLocationIndentifyParam( &_locationParam, &_locationParamLen );

			_devModeParams[_devModeParamsLen].id		=	LOCATIONINFORMATION_COMPONENT_ID;
			_devModeParams[_devModeParamsLen].param		=	_locationParam;
			_devModeParams[_devModeParamsLen].length	=	_locationParamLen;
			_devModeParamsLen ++;
		}

	}

	if( _iAP2Object->hostMode == TRUE )
	{
		g_message("-----------iAP2GetIndentifyParam00000-------------%d", __LINE__);
		iAP2GetUsbHostTSCParam( _iAP2Object, &_usbHostTSCParam, &_usbHostTSCParamLen );
		DEBUG_HEX_DISPLAY( _usbHostTSCParam, _usbHostTSCParamLen );
		_devModeParams[_devModeParamsLen].id		=	USB_HOST_TSC_ID;
		_devModeParams[_devModeParamsLen].param		=	_usbHostTSCParam;
		_devModeParams[_devModeParamsLen].length	=	_usbHostTSCParamLen;
		_devModeParamsLen ++;
	}else{
		g_message("-----------iAP2GetIndentifyParam11111-------------%d", __LINE__);
		_devModeParams[_devModeParamsLen].id		=	USB_DEV_TSC_ID;
		_devModeParams[_devModeParamsLen].param		=	usbDevTSCData;
		_devModeParams[_devModeParamsLen].length	=	sizeof( usbDevTSCData );
		_devModeParamsLen ++;
	}




	if( _iAP2Object->indentifyFlag	==	TRUE )
	{
		if( _iAP2Object->hostMode	==	FALSE )
		{
			_devModeParams[6].param		=	msgAccessoryLvD;
			_devModeParams[6].length	=	sizeof(msgAccessoryLvD);
		}else{
			_devModeParams[6].param		=	msgAccessoryLvH;
			_devModeParams[6].length	=	sizeof(msgAccessoryLvH);
		}
	}

	if(( _iAP2Object->transportType	==	MH_DEV_BT_IAP )
		|| ( _iAP2Object->transportType	==	MH_DEV_WIFI_IAP ))
	{
		_devModeParams[8].param		=	powerSourceN;

		_devModeParams[_devModeParamsLen].id		=	WIRELESS_COMPONENT_ID;
		_devModeParams[_devModeParamsLen].param		=	wireLessTscData;
		_devModeParams[_devModeParamsLen].length	=	sizeof( wireLessTscData );

		_devModeParamsLen ++;
	}

	if( _iAP2Object->transportType	==	MH_DEV_BT_IAP )
	{
		iAP2GetBTComponentParam( &_btTscParam, &_btTscParamLen );

		_devModeParams[_devModeParamsLen].id		=	USB_BT_TSC_ID;
		_devModeParams[_devModeParamsLen].param		=	_btTscParam;
		_devModeParams[_devModeParamsLen].length	=	_btTscParamLen;
		_devModeParamsLen ++;
	}

	int i;
	int _len	=	0, _offset	=	0;
	IDENTIFY_PARAM * _params;
	int _count;
	uint8_t * _paramBuf	=	NULL;

	_params	=	_devModeParams;
//	_count	=	sizeof(_devModeParams) / sizeof(IDENTIFY_PARAM);
	_count 	= _devModeParamsLen;

	for(i = 0; i < _count; i ++)
	{
		_len	+=	_params[i].length + 4;
		
		_paramBuf	=	(uint8_t *)realloc(_paramBuf, _len);

		_paramBuf[_offset + 0]	=	IAP2_HI_BYTE(_params[i].length + 4);
		_paramBuf[_offset + 1]	=	IAP2_LO_BYTE(_params[i].length + 4);
		_paramBuf[_offset + 2]	=	IAP2_HI_BYTE(_params[i].id);
		_paramBuf[_offset + 3]	=	IAP2_LO_BYTE(_params[i].id);

		memcpy(_paramBuf + _offset + 4, _params[i].param, _params[i].length);

		_offset	+=	_params[i].length + 4;
	}

	* idParams	=	_paramBuf;
	* paramsLen	=	_len;
	
	if( _eaIndentifyParamLen != 0 )
		g_free( _eaIndentifyParam );

	if( _eaIndentifyParamLen_other != 0 )
		g_free( _eaIndentifyParam_other );

	if( _msgAccessoryParamLen != 0 )
		g_free( _msgAccessoryParam );

	if( _msgDeviceParamLen != 0 )
		g_free( _msgDeviceParam );

	if( _usbHostTSCParamLen != 0 )
		g_free( _usbHostTSCParam );

	if( _locationParamLen != 0 ) 
		g_free( _locationParam );

	if( _vehicleInfoParamLen != 0 )
		g_free( _vehicleInfoParam );

	if( _vehicleStatusParamLen != 0 )
		g_free( _vehicleStatusParam );

	if( _btTscParamLen != 0 )
		g_free( _btTscParam );

	_iap2FreeIdentifyParam(_devModeParams);
}		/* -----  end of function iAP2GetIndentifyParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthReadCertData
 *  Description:  
 * =====================================================================================
 */
int16_t iAP2AuthReadCertData( uint8_t ** data )
{
	static uint8_t * certData	=	NULL;
	static int16_t certLen	=	0;
	uint8_t _buf[2];
	int i;
	int _fd;

	const char * _addr	=	getenv( "MH_APPLE_AUTH_COPROCESSOR_ADDRESS" );

	if( _addr == NULL )
		_addr	=	"/dev/i2c-0";

	if( certData == NULL )
	{
		_fd	=	open( _addr, O_RDWR );

		if( _fd <= 0 )
		{
			g_critical(" open %s failed  _fd	= [%d]", _addr, _fd);
		}

		//	if(( certData == NULL )&&( _fd > 0 ))
		if( _fd > 0 )
		{
			if( iAP2I2cReadBlock( _fd, 0x30, _buf, 2 ) > 0 )
			{
				certLen	=	_buf[0] << 8 | _buf[1];
				certData	=	g_malloc0( certLen );

				for( i = 0; i < certLen / 128; i ++ )
				{
					iAP2I2cReadBlock( _fd, 0x31 + i, certData + i * 128, 128 );
				}

				if( certLen > i * 128 )
				{
					iAP2I2cReadBlock( _fd, 0x31 + i, certData + i * 128, certLen - i * 128 );
				}
			}else{
				g_critical(" iAP2I2cReadBlock failed! ");
			}

			close( _fd );
		}
	}
	* data	=	certData;

	return certLen;
}		/* -----  end of function iAP2AuthReadCertData  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthWriteChallengeData
 *  Description:  
 * =====================================================================================
 */
gboolean iAP2AuthWriteChallengeData( uint8_t * data, uint16_t len )
{
	uint8_t _buf[2]	=	{0};
	gboolean _result	=	FALSE;
	int _fd;
	
	const char * _addr	=	getenv( "MH_APPLE_AUTH_COPROCESSOR_ADDRESS" );

	if( _addr == NULL )
		_addr	=	"/dev/i2c-0";

	_fd	=	open( _addr, O_RDWR );

	if( _fd <= 0 )
	{
		g_critical(" open /dev/i2c-1 failed  _fd	= [%d]", _fd);

		return _result;
	}

	_buf[0]	=	len >> 8;
	_buf[1]	=	len & 0xFF;


	do{
		if( iAP2I2cWriteBlock( _fd, 0x20, _buf, 2 ) <= 0 )
		{
			g_critical(" %s  iAP2I2cWriteBlock( _fd, 0x20, _buf, 2 )",__func__);
			break;
		}
		if( iAP2I2cWriteBlock( _fd, 0x21, data, len ) <= 0 )
		{
			g_critical(" %s  iAP2I2cWriteBlock( _fd, 0x21, data, len )",__func__);
			break;
		}

		_buf[0]	=	1;

		if( iAP2I2cWriteBlock( _fd, 0x10, _buf, 1 ) <= 0 )
		{
			g_critical(" %s  iAP2I2cWriteBlock( _fd, 0x10, _buf, 1 )",__func__);
			break;
		}

		if( iAP2I2cReadBlock( _fd, 0x10, _buf, 1 ) <= 0 )
		{
			g_critical(" %s  iAP2I2cReadBlock( _fd, 0x10, _buf, 1 )",__func__);
			break;
		}

		if( _buf[0] & 0xF0 == 0x10 )
			_result	=	TRUE;
	}while( 0 );

	close( _fd );

	return _result;
}		/* -----  end of function iAP2AuthWriteChallengeData  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthReadChallengeResponse
 *  Description:  
 * =====================================================================================
 */
int16_t iAP2AuthReadChallengeResponse( uint8_t ** data )
{
	uint8_t _buf[2];
	int16_t _resLen	=	0;
	uint8_t * _data = NULL;
	int _fd;
	
	const char * _addr	=	getenv( "MH_APPLE_AUTH_COPROCESSOR_ADDRESS" );

	if( _addr == NULL )
		_addr	=	"/dev/i2c-0";

	_fd	=	open( _addr, O_RDWR );

	do{
		if( iAP2I2cReadBlock( _fd, 0x11, _buf, 2 ) <= 0 )
		{
			g_critical(" %s  iAP2I2cReadBlock( _fd, 0x11, _buf, 2 )",__func__);
			break;
		}

		_resLen	=	_buf[0] << 8 | _buf[1];
		_data	=	g_malloc0( _resLen );

		if( iAP2I2cReadBlock( _fd, 0x12, _data, _resLen ) <= 0 )
		{
			g_critical(" %s  iAP2I2cReadBlock( _fd, 0x12, _data, _resLen )",__func__);
			break;
		}

		* data	=	_data;
	}while( 0 );

	close( _fd );

	return _resLen;
}		/* -----  end of function iAP2AuthReadChallengeResponse  ----- */

