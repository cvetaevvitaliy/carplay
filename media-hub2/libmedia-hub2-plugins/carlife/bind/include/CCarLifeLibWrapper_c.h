/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
//this file is combined with CCarLifeLib.h and CommonUtil.h

#ifndef C_CARLIFE_LIB_WRAPPER_C_H
#define C_CARLIFE_LIB_WRAPPER_C_H
#ifdef __cplusplus
extern "C"
{
#endif
#include<stdint.h>
#include<stdbool.h>
#include<mh_api.h>
#include <mh_carlife.h>
//using namespace std;

/*******************************************************
STRUCT
*******************************************************/
//struct for C_cmd channel
typedef struct C_HUProtocolVersion{
	uint32_t majorVersion;
    uint32_t minorVersion;

}S_HU_PROTOCOL_VERSION_t;

typedef struct C_ProtocolVersionMatchStatus{
	uint32_t matchStatus;
}S_PROTOCOL_VERSION_MATCH_SATUS_t;

typedef struct C_HUInfo{
	char * os;
	char * board ;
	char * bootloader ;
	char * brand;
	char * cpu_abi;
	char * cpu_abi2 ;
	char * device ;
	char * display ;
	char * fingerprint ;
	char * hardware ;
	char * host ;
	char * cid ;
	char * manufacturer ;
	char * model ;
	char * product ;
	char * serial;
	char * codename ;
	char * incremental;
	char * release ;
	char * sdk ;
	uint32_t sdk_int;
	char * token;

	char * btaddress;

}S_HU_INFO_t;

typedef struct C_MDInfo{
	char * os;
	char * board ;
	char * bootloader ;
	char * brand;
	char * cpu_abi;
	char * cpu_abi2 ;
	char * device ;
	char * display ;
	char * fingerprint ;
	char * hardware ;
	char * host ;
	char * cid ;
	char * manufacturer ;
	char * model ;
	char * product ;
	char * serial;
	char * codename ;
	char * incremental;
	char * release ;
	char * sdk ;
	uint32_t sdk_int;
	char * token;
	char * btaddress;

}S_MD_INFO_t;

typedef struct C_BTPairInfo{
	char * address ;
	char * passKey ;
	char * hash ;
	char * randomizer;
	char * uuid;
	char * name;
	uint32_t status;
}S_BT_PAIR_INFO_t;

typedef struct C_VideoEncoderInit{
	uint32_t width ;
	uint32_t height ;
	uint32_t frameRate;
}S_VIDEO_ENCODER_INIT_t;

typedef struct C_VideoEncoderInitDone{
	uint32_t width ;
	uint32_t height ;
	uint32_t frameRate;
}S_VIDEO_ENCODER_INIT_DONE_t;

typedef struct C_VideoEncoderFrameRateChange{
	uint32_t frameRate;
}S_VIDEO_ENCODER_FRAME_RATE_CHANGE_t;

typedef struct C_VideoEncoderFrameRateChangeDone{
	uint32_t frameRate;
}S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE_t;

typedef struct C_CarVelocity{
	uint32_t speed;
	uint64_t timeStamp; 
}S_CAR_VELOCITY_t;

typedef struct C_CarGPS{
	uint32_t antennaState;
	uint32_t signalQuality;
	// dBHz
	uint32_t latitude;
	// 1/1,000,000 degrees
	uint32_t longitude;
	// 1/1,000,000 degrees
	uint32_t height ;
	// 0.1 meters
	uint32_t speed ;
	// 0.01 km/h units
	uint32_t heading;
	// 0.1 degrees
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t hrs ;
	uint32_t min ;
	uint32_t sec ;
	uint32_t fix;
	uint32_t hdop;
	// units 0.1
	uint32_t pdop ;
	// units 0.1
	uint32_t vdop ;
	// units 0.1
	uint32_t satsUsed ;
	uint32_t satsVisible;
	uint32_t horPosError ;
	uint32_t vertPosError;
	uint32_t northSpeed;
	// 0.01m/s
	uint32_t eastSpeed;
	// 0.01m/s
	uint32_t vertSpeed; 
	// 0.01m/s
	
	uint64_t timeStamp;
	
}S_CAR_GPS_t;

typedef struct C_CarGyroscope{
	int32_t	gyroType;
	double gyroX;
	double gyroY;
	double gyroZ;
	uint64_t timeStamp;
}S_CAR_GYROSCOPE_t;

typedef struct C_CarAcceleration{
	double accX ;
	double accY ;
	double accZ ;
	uint64_t timeStamp ;

}S_CAR_ACCELERATION_t;

typedef struct C_CarOil{
	int32_t level;
	int32_t range;
	bool lowFullWarning;
}S_CAR_OIL_t;

//struct for media && tts channel
typedef struct C_AudioInitParameter{
	uint32_t sampleRate;
	uint32_t channelConfig;
	uint32_t sampleFormat;
}S_AUDIO_INIT_PARAMETER_t;

//struct for ctrl channel
//typedef struct C_TouchAction{
//	uint32_t action;
//	uint32_t x ;
//	uint32_t y ;
//
//}S_TOUCH_ACTION_t;

typedef struct C_TouchActionDown{
	uint32_t x ;
	uint32_t y ;
}S_TOUCH_ACTION_DOWN_t;

typedef struct C_TouchActionUp{
	uint32_t x ;
	uint32_t y ;
}S_TOUCH_ACTION_UP_t;

typedef struct C_TouchActionMove{
	uint32_t x ;
	uint32_t y ;
}S_TOUCH_ACTION_MOVE_t;

typedef struct C_TouchSingleClick{
	uint32_t x;
	uint32_t y;
}S_TOUCH_SIGNAL_CLICK_t;

typedef struct C_TouchDoubleClick{
	uint32_t x;
	uint32_t y;
}S_TOUCH_DOUBLE_CLICK_t;

typedef struct C_TouchLongPress{
	uint32_t x;
	uint32_t y;
}S_TOUCH_LONG_PRESS_t;

typedef struct C_TouchCarHardkeyCode{
	uint32_t keycode;
}S_TOUCH_CAR_HARD_KEY_CODE_t;

typedef struct C_TouchUIActionSound{
	//reserved
}S_TOUCH_UI_ACTION_SOUND_t;

//0x00018025

//0x00010026
typedef struct C_ModuleStatusMobile{
	uint32_t moduleID;
	uint32_t statusID;
}S_MODULE_STATUS_MOBILE_t;

typedef struct C_ModuleStatusListMobile{
	uint32_t	cnt ;
	S_MODULE_STATUS_MOBILE_t* moduleStatus;
}S_MODULE_STATUS_LIST_MOBILE_t;

//0x00018027
typedef struct C_StatisticsInfo{
	const char * cuid;
	const char * versionName;
	uint32_t versionCode;
	const char * channel;
	uint32_t connectCount;
	uint32_t connectSuccessCount;
	uint32_t connectTime;
	const char * crashLog;
}S_STATISTICS_INFO_t;

//0x00018028
typedef struct C_ModuleStatusControl{
	uint32_t moduleID;
    uint32_t statusID;
}S_MODULE_STATUS_CONTROL_t;

//0x00018029
typedef struct C_GearInfo
{
	uint32_t gear;
}S_GEAR_INFO_t;

//0x00010030
typedef struct C_NaviNextTurnInfo
{
	uint32_t action ;
	uint32_t nextTurn;
	char * roadName ;
	uint32_t totalDistance ;
	uint32_t remainDistance ;
	uint32_t time;
}S_NAVI_NEXT_TURN_INFO_t;

//0x00010031
//0x00010032
//0x00010033
//0x00010034
typedef struct C_VehicleInfo{
	uint32_t moduleID;
	int32_t flag;
	uint32_t frequency;

	struct VehicleInfo* pNext;
}S_VEHICLE_INFO_t;

typedef struct C_VehicleInfoList{
	uint32_t	cnt ;
	S_VEHICLE_INFO_t * pVehicleInfo;
}S_VEHICLE_INFO_LIST_t;

//0x00010035
typedef struct C_MediaInfo
{
	char * source ;
	char * song ;
	char * artist ;
	char * album ;
	char * albumArt ;
	uint32_t duration ;
	uint32_t playlistNum ;
	char * songId ;
	uint32_t mode ;
}S_MEDIA_INFO_t;

//0x00010036
typedef struct C_MediaProgressBar{
	uint32_t progressBar;
}S_MEDIA_PROGRESS_BAR_t;

//0x00010037
typedef struct C_ConnectException
{
    uint32_t exceptionType;
}S_CONNECTION_EXCEPTION_t;	

//0x00010038

//0x00010039

//0x00010040
typedef struct C_BTHfpRequest{
	uint32_t command;
	char * phoneNum;
	uint32_t dtmfCode;
}S_BT_HFP_REQUEST_t;

//0x00018041
typedef struct C_BTHfpIndication{
	uint32_t state;
	char * phoneNum;
	char * name;
	char * address;
}S_BT_HFP_INDICATION_t;

//0x00018042
typedef struct C_BTHfpConnection{
	uint32_t state;
	char * address;
	char * name;
}S_BT_HFP_CONNECTION_t;

//0x00018043
//0x00010044
//0x00018045
//0x00018046
typedef struct C_MobileCarLifeInfo{
	uint32_t moduleID;
	bool supportFlag;
	uint32_t frequency;

	struct MobileCarLifeInfo *pNext; 
}S_MOBILE_CARLIFE_INFO_t;

typedef struct C_SubscribeMobileCarLifeInfoList{
	uint32_t cnt;
	S_MOBILE_CARLIFE_INFO_t *pMobileCarLifeInfo;
}S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t;

//0x00010047
typedef struct C_NaviAssitantGuideInfo{
	uint32_t action;
	uint32_t assistantType;
	uint32_t trafficSignType;
	uint32_t totalDistance;
	uint32_t remainDistance;
	uint32_t cameraSpeed;
}S_NAVI_ASSITANT_GUIDE_INFO_t;

//0x00018048
typedef struct C_AuthenRequest{
	char * randomValue;
}S_AUTHEN_REQUEST_t;

//0x00010049
typedef struct C_AuthenResponse{
	char * encryptValue;
}S_AUTHEN_RESPONSE_t;

//0x0001004A
typedef struct C_HUAuthenResult{
	bool authenResult;
}S_HU_AUTHEN_RESULT_t;

//0x0001804B
typedef struct C_MDAuthenResult{
	bool authenResult;
}S_MD_AUTHEN_RESULT_t;

//0x0001004C
//none

//0x0001004D
typedef struct C_BTStartPairReq{
	uint32_t osType;
	char * address;
}S_BT_START_PAIR_REQ_t;

//0x0001804E
typedef struct C_BTHfpResponse{
	uint32_t status;
	uint32_t C_cmd;
	uint32_t dtmfCode;
}S_BT_HFP_RESPONSE_t;

//0x0001004F
typedef struct C_BTHfpStatusRequest{
	uint32_t type;
}S_BT_HFP_STATUS_REQUEST_t;

//0x00018050
typedef struct C_BTHfpStatusResponse{
	uint32_t status;
	uint32_t type;
}S_BT_HFP_STATUS_RESPONSE_t;

//0x00018051
//none

//0x00018052
typedef struct C_FeatureConfig{
	char * key;
	uint32_t value;

	struct C_FeatureConfig *pNext;
}S_FEATURE_CONFIG_t;

typedef struct C_FeatureConfigList{
	uint32_t cnt;

	S_FEATURE_CONFIG_t *pFeatureConfig;
}S_FEATURE_CONFIG_LIST_t;

//0x00018053
typedef struct C_BTStartIdentifyReq{
	char * address;
}S_BT_START_IDENTIFY_REQ_t;
//0x00010054
typedef struct C_BTIdentifyResultInd{
	uint32_t status;
	char * address;
}S_BT_INDENTIFY_RESULT_IND_t;
//0x00018055
typedef struct C_ErrorCode{
	char * errorCode;
}S_ERROR_CODE_t;


/*==============================================
				initialization process
===============================================*/
//CCarLifeLib* getInstance();
int C_carLifeLibInit();
void C_carLifeLibDestory();
void C_connection_disconnect();
	/*=====================================================
	  connection between head unit and mobile phone
=======================================================*/
int C_connectionSetup();
int C_connectionSetupIp(const char * mdIP);
int C_connectionSetupIpName(const char * mdIP, const char * interfaceName);

/*==============================================================
		command channel: send method && receive call back register method
===============================================================*/
int C_cmdHUProtoclVersion(S_HU_PROTOCOL_VERSION_t* version);
int C_cmdHUInfro(S_HU_INFO_t* huInfo);
int C_cmdHUBTPairInfro(S_BT_PAIR_INFO_t* info);
int C_cmdVideoEncoderInit(S_VIDEO_ENCODER_INIT_t* initParam);
int C_cmdVideoEncoderStart();
int C_cmdVideoEncoderPause();
int C_cmdVideoEncoderReset();
int C_cmdVideoEncoderFrameRateChange(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_t* videoParam);
int C_cmdPauseMedia();
int C_cmdCarVelocity(S_CAR_VELOCITY_t* carVelocity);
int C_cmdCarGPS(S_CAR_GPS_t* cps);
int C_cmdCarGyroscope(S_CAR_GYROSCOPE_t* cyro);
int C_cmdCarAcceleration(S_CAR_ACCELERATION_t* acceleration);
int C_cmdCarOil( S_CAR_OIL_t *);
int C_cmdLaunchModeNormal();
int C_cmdLaunchModePhone();
int C_cmdLaunchModeMap();
int C_cmdLaunchModeMusic();
int C_cmdReceiveOperation();
void C_cmdRegisterProtocolVersionMatchStatus(void (*pFunc)(S_PROTOCOL_VERSION_MATCH_SATUS_t*));
void C_cmdRegisterMDInfro(void (*pFunc)(S_MD_INFO_t*));
void C_cmdRegisterMDBTPairInfro(void (*pFunc)(S_BT_PAIR_INFO_t*));
void C_cmdRegisterVideoEncoderInitDone(void (*pFunc)(S_VIDEO_ENCODER_INIT_DONE_t*));
void C_cmdRegisterVideoEncoderFrameRateChangeDone(void (*pFunc)(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE_t*));
void C_cmdRegisterTelStateChangeIncoming(void (*pFunc)(void));
void C_cmdRegisterTelStateChangeOutGoing(void (*pFunc)(void));
void C_cmdRegisterTelStateChangeIdle(void (*pFunc)(void));
void C_cmdRegisterTelStateChangeInCalling(void (*pFunc)(void));
void C_cmdRegisterScreenOn(void (*pFunc)(void));
void C_cmdRegisterScreenOff(void (*pFunc)(void));
void C_cmdRegisterScreenUserPresent(void (*pFunc)(void));
void C_cmdRegisterForeground(void (*pFunc)(void));
void C_cmdRegisterBackground(void (*pFunc)(void));
void C_cmdRegisterGoToDeskTop(void (*pFunc)(void));
void C_cmdRegisterMicRecordWakeupStart(void (*pFunc)(void));
void C_cmdRegisterMicRecordEnd(void (*pFunc)(void));
void C_cmdRegisterMicRecordRecogStart(void (*pFunc)(void));
	//added on 9th Semptember 2015
	//0x00018025
int C_cmdGoToForeground();
	//0x00010026
void C_cmdRegisterModuleStatus(void (*pFunc)(S_MODULE_STATUS_LIST_MOBILE_t*));
	//0x00018027
int C_cmdStatisticInfo(S_STATISTICS_INFO_t*);
	//0x00018028
int C_cmdModuleControl(S_MODULE_STATUS_CONTROL_t*);
	//0x00018029
int C_cmdCarDataGear(S_GEAR_INFO_t*);
	//0x00010030
void C_cmdRegisterNaviNextTurnInfo(void (*pFunc)(S_NAVI_NEXT_TURN_INFO_t*));
	//0x00010031
void C_cmdRegisterCarDataSubscribe(void (*pFunc)(S_VEHICLE_INFO_LIST_t*));
	//0x00018032
int C_cmdCarDataSubscribeDone(S_VEHICLE_INFO_LIST_t*);
	//0x00010033
void C_cmdRegisterCarDataSubscribeStart(void (*pFunc)(S_VEHICLE_INFO_LIST_t*));
	//0x00010034
void C_cmdRegisterCarDataSubscribeStop(void (*pFunc)(S_VEHICLE_INFO_LIST_t*));
	//0x00010035
void C_cmdRegisterMediaInfo(void (*pFunc)(S_MEDIA_INFO_t*));
	//0x00010036
void C_cmdRegisterMediaProgressBar(void (*pFunc)(S_MEDIA_PROGRESS_BAR_t*));
	//0x00010037
void C_cmdRegisterConnectException(void (*pFunc)(S_CONNECTION_EXCEPTION_t*));
	//0x00010038
void C_cmdRegisterRequestGoToForeground(void (*pFunc)(void));
	//0x00010039
void C_cmdRegisterUIActionSound(void (*pFunc)(void));
	//added on 5th January 2016
	//0x00010040
void C_cmdRegisterBtHfpRequest(void (*pFunc)(S_BT_HFP_REQUEST_t*));
	//0x00018041
int C_cmdBtHfpIndication(S_BT_HFP_INDICATION_t*);
	//0x00018042
int C_cmdBtHfpConnection(S_BT_HFP_CONNECTION_t*);
	//0x00018043 
int C_cmdCarLifeDataSubscribe(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*);
	//0x00010044 
void C_cmdRegisterCarLifeDataSubscribeDone(void (*pFunc)(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*));
	//0x00018045 
int C_cmdCarLifeDataSubscribeStart(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*);
	//0x00018046 
int C_cmdCarLifeDataSubscribeStop(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*);
	//0x00010047 
void C_cmdRegisterNaviAssistantGuideInfo(void (*pFunc)(S_NAVI_ASSITANT_GUIDE_INFO_t*));
	//0x00018048 
int C_cmdHuAuthenRequest(S_AUTHEN_REQUEST_t*);
	//0x00010049 
void C_cmdRegisterMdAuthenResponse(void (*pFunc)(S_AUTHEN_RESPONSE_t*));
	//0x0001804A 
int C_cmdHuAuthenResult(S_HU_AUTHEN_RESULT_t*);
	//0x0001804B
void C_cmdRegisterMdAuthenResult(void (*pFunc)(S_MD_AUTHEN_RESULT_t*));

	//0x0001004C 
void C_cmdRegisterGotoForgroundResponse(void (*pFunc)(void));
	//0x0001004D 
void C_cmdRegisterStartBtAutoPairRequest(void (*pFunc)(S_BT_START_PAIR_REQ_t*));
	//0x0001804E
int C_cmdBTHfpResponse(S_BT_HFP_RESPONSE_t*);
	//0x0001004F
void C_cmdRegisterBTHfpStatusRequest(void (*pFunc)(S_BT_HFP_STATUS_REQUEST_t*));
	//0x00018050
int C_cmdBTHfpStatusResponse(S_BT_HFP_STATUS_RESPONSE_t*);
	//added on 3th March 2016
	//0x00010051
void C_cmdRegisterFeatureConfigRequest(void (*pFunc)(void));
	//0x00018052
int C_cmdFeatureConfigResponse(S_FEATURE_CONFIG_LIST_t*);
	//0x00018053
int C_cmdBTStartIdentifyReq(S_BT_START_IDENTIFY_REQ_t*);
	//0x00010054
void C_cmdRegisterBTIdentifyResultInd(void (*pFunc)(S_BT_INDENTIFY_RESULT_IND_t*));
	//0x00018055
int C_cmdErrorCode(S_ERROR_CODE_t*);
	//0x00010059
void C_cmdRegisterMdExit(void (*pFunc)(void));
/*=============================================================
				video channel: receive call back register method
==============================================================*/
int C_videoReceiveOperation();
void C_videoRegisterDataReceive(void (*pFunc)(uint8_t *data, uint32_t len));
void C_videoRegisterHeartBeat(void (*pFunc)(void));

/*=============================================================
				media channel: receive call back register method
==============================================================*/
int C_mediaReceiveOperation();
void C_mediaRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*));
void C_mediaRegisterNormalData(void (*pFunc)(uint8_t *data, uint32_t len));
void C_mediaRegisterStop(void (*pFunc)(void));
void C_mediaRegisterPause(void (*pFunc)(void));
void C_mediaRegisterResume(void (*pFunc)(void));
void C_mediaRegisterSeek(void (*pFunc)(void));
	
/*=============================================================
				tts channel: receive call back register method
==============================================================*/
int C_ttsReceiveOperation();
void C_ttsRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*));
void C_ttsRegisterNormalData(void (*pFunc)(uint8_t *data, uint32_t len));
void C_ttsRegisterStop(void (*pFunc)(void));

/*=============================================================
				vr channel: send method
==============================================================*/
int C_sendVRRecordData(uint8_t* data, uint32_t size, uint32_t timeStamp);
int C_vrReceiveOperation();
void C_vrRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*));
void C_vrRegisterNormalData(void (*pFunc)(uint8_t *data, uint32_t len));
void C_vrRegisterStop(void (*pFunc)(void));

/*=============================================================
				control channel: send method
==============================================================*/
//int C_ctrlTouchAction(S_TOUCH_ACTION_t* touchAction);
int C_ctrlTouchActionDown(S_TOUCH_ACTION_DOWN_t* touchActionDown);
int C_ctrlTouchActionUp(S_TOUCH_ACTION_UP_t* touchActionUp);
int C_ctrlTouchActionMove(S_TOUCH_ACTION_MOVE_t* touchActionMove);
int C_ctrlTouchSigleClick(S_TOUCH_SIGNAL_CLICK_t* touchSingleClick);
int C_ctrlTouchDoubleClick(S_TOUCH_DOUBLE_CLICK_t* touchDoubleClick);
int C_ctrlTouchLongPress(S_TOUCH_LONG_PRESS_t* touchLongPress);
int C_ctrlTouchCarHardKeyCode(S_TOUCH_CAR_HARD_KEY_CODE_t* touchCarHardKeyCode);
int C_ctrlReceiveOperation();
void C_ctrlRegisterUIActionSound(void (*pFunc)(void));

#ifdef __cplusplus
}
#endif

#endif













































