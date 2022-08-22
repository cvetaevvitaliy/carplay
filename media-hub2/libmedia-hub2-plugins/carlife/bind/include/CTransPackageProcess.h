/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_TRANS_PACKAGE_PROCESS
#define C_TRANS_PACKAGE_PROCESS
#ifdef __cplusplus
extern "C"
{
#endif
#include "CCarLifeLibWrapper_c.h"
/********************************************************
DEFINE
*********************************************************/
#define DEFAULT_HEAD_LEN 12
#define DEFAULT_DATA_SIZE 10*1024

#define IOS_CMD_HEAD_LEN 8
#define IOS_CMD_DATA_SIZE 40*1024//MSG_IOS_CMD_IOS_MEDIA_INFO<32k

#define IOS_VIDEO_HEAD_LEN 12
#define IOS_VIDEO_DATA_SIZE 600*1024//normally IOS_VIDEO_DATA_SIZE<300X1024

#define IOS_MEDIA_HEAD_LEN 12
#define IOS_MEDIA_DATA_SIZE 100*1024

#define TTS_HEAD_LEN 12
#define TTS_DATA_SIZE 50*1024//normally TTS_DATA_SIZE<10x1024

#define VR_HEAD_LEN 12
#define VR_DATA_SIZE 50*1024//normally VR_DATA_SIZE<10x1024

#define CTRL_HEAD_LEN 8
#define CTRL_DATA_SIZE 10*1024


/**********************************************************
ENUM
***********************************************************/
typedef enum _ChannelType{
	IOS_CMD_CHANNEL=1,
	IOS_VIDEO_CHANNEL=2,
	IOS_MEDIA_CHANNEL=3,
	IOS_TTS_CHANNEL=4,
	IOS_VR_CHANNEL=5,
	IOS_CTRL_CHANNEL=6,
}ChannelType;

typedef enum _PackageHeadType{
	//default invailid head type
	IOS_DEFAULT_HEAD_TYPE=0x00000000,
	//--------------------Command channel-------------------------
       //command channel relative
	//--interactive message for initialization
	IOS_CMD_HU_PROTOCOL_VERSION = 0x00018001,		//finish
	IOS_CMD_PROTOCOL_VERSION_MATCH_STATUS = 0x00010002,  //finish
	IOS_CMD_HU_INFO = 0x00018003,
	IOS_CMD_MD_INFO = 0x00010004,	//finish
	IOS_CMD_HU_BT_PAIR_INFO = 0x00018005,
	IOS_CMD_MD_BT_PAIR_INFO = 0x00010006,

	//--video relative
	IOS_CMD_VIDEO_ENCODER_INIT = 0x00018007,	//finish
	IOS_CMD_VIDEO_ENCODER_INIT_DONE = 0x00010008,//finish
	IOS_CMD_VIDEO_ENCODER_START = 0x00018009,	//finish
	IOS_CMD_VIDEO_ENCODER_PAUSE = 0x0001800A,	//finish
	IOS_CMD_VIDEO_ENCODER_RESET = 0x0001800B,	//finish
	IOS_CMD_VIDEO_ENCODER_FRAME_RATE_CHANGE = 0x0001800C,
	IOS_CMD_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE = 0x0001000D,

	//--audio relative
	IOS_CMD_PAUSE_IOS_MEDIA = 0x0001800E,

	//--vehicle infromation
	IOS_CMD_CAR_VELOCITY = 0x0001800F,		//finish
	IOS_CMD_CAR_GPS = 0x00018010,			//finish
	IOS_CMD_CAR_GYROSCOPE = 0x00018011,		//finish
	IOS_CMD_CAR_ACCELERATION = 0x00018012,	//finish
	IOS_CMD_CAR_OIL = 0x00018013,			//finish

	//--phone status
	IOS_CMD_TELE_STATE_CHANGE_INCOMING = 0x00010014,
	IOS_CMD_TELE_STATE_CHANGE_OUTGOING = 0x00010015,
	IOS_CMD_TELE_STATE_CHANGE_IDLE = 0x00010016,
	IOS_CMD_TELE_STATE_CHANGE_INCALLING = 0x00010017,

	//--mobile device status
	IOS_CMD_SCREEN_ON = 0x00010018,
	IOS_CMD_SCREEN_OFF = 0x00010019,
	IOS_CMD_SCREEN_USERPRESENT = 0x0001001A,
	IOS_CMD_FOREGROUND = 0x0001001B,	//finish
	IOS_CMD_BACKGROUND = 0x0001001C,	//finish
	
	//--launch mode
	IOS_CMD_LAUNCH_MODE_NORMAL = 0x0001801D,	//finish
	IOS_CMD_LAUNCH_MODE_PHONE = 0x0001801E,		//finish
	IOS_CMD_LAUNCH_MODE_MAP = 0x0001801F,		//finish
	IOS_CMD_LAUNCH_MODE_MUSIC = 0x00018020,		//finish
	IOS_CMD_GO_TO_DESKTOP = 0x00010021,
	
	//--vr relative
	IOS_CMD_MIC_RECORD_WAKEUP_START = 0x00010022,	//finish
	IOS_CMD_MIC_RECORD_END = 0x00010023,			//finish
	IOS_CMD_MIC_RECORD_RECOG_START = 0x00010024,	//finish

	IOS_CMD_GO_TO_FOREGROUND = 0x00018025,
	IOS_CMD_MODULE_STATUS = 0x00010026,		//finish
	IOS_CMD_STATISTIC_INFO = 0x00018027,	//finish
	IOS_CMD_MODULE_CONTROL = 0x00018028,	//finish

	//navi info
	IOS_CMD_GEAR_INFO=0x00010029,
	IOS_CMD_NAVI_NEXT_TURN_INFO=0x00010030,
	IOS_CMD_CAR_DATA_SUBSCRIBE=0x00010031,
	IOS_CMD_CAR_DATA_SUBSCRIBE_DONE=0x00010032,
	IOS_CMD_CAR_DATA_SUBSCRIBE_START=0x00010033,
	IOS_CMD_CAR_DATA_SUBSCRIBE_STOP=0x00010034,

    //music relative
    IOS_CMD_MEDIA_INFO = 0x00010035,	//finish
    IOS_CMD_MEDIA_PROGRESS_BAR = 0x00010036,	//finish

    IOS_CMD_CONNECT_EXCEPTION = 0x00010037,
    IOS_CMD_REQUEST_GO_TO_FOREGROUND = 0x00010038,

    IOS_CMD_UI_ACTION_SOUND = 0x00010039,	//finish

	IOS_CMD_BT_HFP_REQUEST=0x00010040,
	IOS_CMD_BT_HFP_INDICATION=0x00018041,
    IOS_CMD_BT_HFP_CONNECTION=0x00018042,

	IOS_CMD_CARLIFE_DATA_SUBSCRIBE=0x00018043,
	IOS_CMD_CARLIFE_DATA_SUBSCRIBE_DONE=0x00010044,
	IOS_CMD_CARLIFE_DATA_SUBSCRIBE_START=0x00018045,
	IOS_CMD_CARLIFE_DATA_SUBSCRIBE_STOP=0x00018046,
	IOS_CMD_NAVI_ASSITANTGUIDE_INFO=0x00010047,
	IOS_CMD_HU_AUTHEN_REQUEST=0x00018048,
	IOS_CMD_MD_AUTHEN_RESPONSE=0x00010049,
	IOS_CMD_HU_AUTHEN_RESULT=0x0001804A,
	IOS_CMD_MD_AUTHEN_RESULT=0x0001004B,	//finish
	IOS_CMD_GO_TO_FOREGROUND_RESPONSE=0x0001004C,
	IOS_CMD_START_BT_AUTOPAIR_REQUEST=0x0001004D,
	IOS_CMD_BT_HFP_RESPONSE=0x0001804E,
	IOS_CMD_BT_HFP_STATUS_REQUEST=0x0001004F,
	IOS_CMD_BT_HFP_STATUS_RESPONSE=0x00018050,

	IOS_CMD_MD_FEATURE_CONFIG_REQUEST=0x00010051,  //finish
    IOS_CMD_HU_FEATURE_CONFIG_RESPONSE=0x00018052,	//finish
	IOS_CMD_BT_START_IDENTIFY_REQ=0x00018053,
	IOS_CMD_BT_IDENTIFY_RESULT_IND=0x00010054,
	IOS_CMD_ERROR_CODE=0x00018055,

	IOS_CMD_IOS_VIDEO_ENCODER_JPEG=0x00018056,
	IOS_CMD_IOS_VIDEO_ENCODER_JPEG_ACK=0x00010057,
	IOS_CMD_BT_HFP_CALL_STATUS_COVER=0x00010058,
	IOS_CMD_MD_EXIT=0x00010059,

    //------------------------Video channel--------------------------------
	//video channel relative
	IOS_VIDEO_DATA = 0x00020001,	//finish
	IOS_VIDEO_HEARTBEAT = 0x00020002,	//finish:hu->md  not finish:md->hu

	//------------------------Media channel-------------------------------
	//media channel relative
	IOS_MEDIA_INIT = 0x00030001,	//finish
	IOS_MEDIA_STOP = 0x00030002,
	IOS_MEDIA_PAUSE = 0x00030003,	//finish
	IOS_MEDIA_RESUME_PLAY = 0x00030004,
	IOS_MEDIA_SEEK_TO = 0x00030005,
	IOS_MEDIA_DATA = 0x00030006,	//finish

	 //-------------------Navi TTS channel-----------------------------
	//tts channel relative
	IOS_TTS_INIT = 0x00040001,		//finish
	IOS_TTS_STOP = 0x00040002,		//finish
	IOS_TTS_DATA = 0x00040003,		//finish

	//-------------------VR channel----------------------
	//vr channel relative
	IOS_VR_MIC_DATA = 0x00058001,	//finish
	IOS_VR_INIT = 0x00050002,		//finish
	IOS_VR_DATA = 0x00050003,		//finish
	IOS_VR_STOP = 0x00050004,		//finish

	//-----------------Control channel--------------------
	//control channel relative
	IOS_TOUCH_ACTION = 0x00068001,			//finish
	IOS_TOUCH_ACTION_DOWN = 0x00068002,		//finish
	IOS_TOUCH_ACTION_UP = 0x00068003,		//finish
	IOS_TOUCH_ACTION_MOVE = 0x00068004,		//finish
	IOS_TOUCH_SINGLE_CLICK = 0x00068005,	//finish
	IOS_TOUCH_DOUBLE_CLICK = 0x00068006,	//finish
	IOS_TOUCH_LONG_PRESS = 0x00068007,		//finish
	IOS_TOUCH_CAR_HARD_KEY_CODE = 0x00068008,	//finish
	IOS_TOUCH_UI_ACTION_SOUND = 0x00060009,

	//hard key value
	IOS_KEYCODE_HOME = 0x00000001,			//finish
	IOS_KEYCODE_PHONE_CALL = 0x00000002,	//finish
	IOS_KEYCODE_PHONE_END = 0x00000003,		//finish
	IOS_KEYCODE_PHONE_END_MUTE = 0x00000004,//finish
	IOS_KEYCODE_HFP = 0x00000005,			//finish
	IOS_KEYCODE_SELECTOR_NEXT = 0x00000006,	//finish
	IOS_KEYCODE_SELECTOR_PREVIOUS = 0x00000007,	//finish
	IOS_KEYCODE_SETTING = 0x00000008,			//finish
	IOS_KEYCODE_MEDIA = 0x00000009,				//finish
	IOS_KEYCODE_RADIO = 0x0000000A,				//finish
	IOS_KEYCODE_NAVI = 0x0000000B,				//finish
	IOS_KEYCODE_SRC = 0x0000000C,				//finish
	IOS_KEYCODE_MODE = 0x0000000D,				//finish
	IOS_KEYCODE_BACK = 0x0000000E,				//finish
	IOS_KEYCODE_SEEK_SUB = 0x0000000F,			//finish
	IOS_KEYCODE_SEEK_ADD = 0x00000010,			//finish	
	IOS_KEYCODE_VOLUME_SUB = 0x00000011,		//finish
	IOS_KEYCODE_VOLUME_ADD = 0x00000012,		//finish
	IOS_KEYCODE_MUTE = 0x00000013,				//finish
	IOS_KEYCODE_OK = 0x00000014,				//finish
	IOS_KEYCODE_MOVE_LEFT = 0x00000015,			//finish
	IOS_KEYCODE_MOVE_RIGHT = 0x00000016,		//finish
	IOS_KEYCODE_MOVE_UP = 0x00000017,			//finish
	IOS_KEYCODE_MOVE_DOWN = 0x00000018,			//finish
	IOS_KEYCODE_MOVE_UP_LEFT = 0x00000019,		//finish
	IOS_KEYCODE_MOVE_UP_RIGHT = 0x0000001A,		//finish
	IOS_KEYCODE_MOVE_DOWN_LEFT = 0x0000001B,	//finish
	IOS_KEYCODE_MOVE_DOWN_RIGHT = 0x0000001C,	//finish
	IOS_KEYCODE_TEL = 0x0000001D,				//finish
	IOS_KEYCODE_MAIN = 0x0000001E,				//finish
}PackageHeadType;

void * packageDataAnalysis( PackageHeadType type, const uint8_t * data, uint32_t len); 
void ios_sendCmdVideoEncoderInit( S_VIDEO_ENCODER_INIT_t * initParam, uint8_t * data, uint32_t *len);
void ios_sendCmdHUProtoclVersion( S_HU_PROTOCOL_VERSION_t* version, uint8_t * data, uint32_t * len);
void ios_sendCmdStatisticInfo(S_STATISTICS_INFO_t * info, uint8_t * data, uint32_t *len); 
void ios_sendCmdVideoHeartbeat( uint8_t * data, uint32_t *len, uint32_t timestamp);
void ios_sendCmdFeatureConfigResponse( S_FEATURE_CONFIG_LIST_t * list, uint8_t * data, uint32_t * len);
void ios_sendCmdVideoEncoderStart(uint8_t * data, uint32_t *len);
void ios_sendCmdVideoEncoderPause(uint8_t * data, uint32_t *len);
void ios_sendCmdVideoEncoderReset(uint8_t * data, uint32_t *len);
void ios_sendTouch(MHTouchType type, uint32_t x, uint32_t y, uint8_t *data, uint32_t *len);
void ios_sendCmdLaunchMode( MHCarlifeLaunchMode mode, uint8_t * data, uint32_t * len);
void ios_sendCmdHardkey( S_TOUCH_CAR_HARD_KEY_CODE_t * _key, uint8_t * data, uint32_t * len);
uint32_t ios_sendVRRecordData( uint8_t * data,uint32_t pbLen, uint32_t timestamp); 
void ios_sendCarVelocity(S_CAR_VELOCITY_t * velocity, uint8_t * data, uint32_t * len);
void ios_sendCarGPS(S_CAR_GPS_t * gps, uint8_t * data, uint32_t * len);
void ios_sendCarGyroscope(S_CAR_GYROSCOPE_t * cyro, uint8_t * data, uint32_t * len);
void ios_sendCarAcceleration(S_CAR_ACCELERATION_t * acceleration, uint8_t * data, uint32_t * len);
void ios_sendCarOil(S_CAR_OIL_t * oil, uint8_t * data, uint32_t * len);
void ios_sendModuleControl(S_MODULE_STATUS_CONTROL_t * status, uint8_t * data, uint32_t * len);

#ifdef __cplusplus
}
#endif
#endif

