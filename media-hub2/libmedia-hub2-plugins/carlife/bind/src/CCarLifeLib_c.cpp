
/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#include<CCarLifeLibWrapper.h>
#include"CCarLifeLibWrapper_c.h"
#include <glib.h>
//#include"CCarLifeLibWrapper_c.h"
//#include<pthread.h>
//#include<unistd.h>
//#include "CCarLifeLib.h"
//CCarLifeLib* getInstance(){
//	return CCarLifeLib::getInstance();
//}
//
using namespace CCarLifeLibH;

typedef void (* ProtocolVersionMatchStatus_cb) (S_PROTOCOL_VERSION_MATCH_SATUS_t*);
typedef void (* MDInfro_cb) ( S_MD_INFO_t *);
typedef void (* MDBTPairInfro_cb)( S_BT_PAIR_INFO_t *);
typedef void (* VideoEncoderInitDone_cb)(S_VIDEO_ENCODER_INIT_DONE_t*);
typedef void (* VideoEncoderFrameRateChangeDone_cb)(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE_t*);
typedef void (* ModuleStatus_cb)(S_MODULE_STATUS_LIST_MOBILE_t*);
typedef void (* NaviNextTurnInfo_cb)(S_NAVI_NEXT_TURN_INFO_t*);
typedef void (* CarDataSubscribe_cb )(S_VEHICLE_INFO_LIST_t*);
typedef void (* CarDataSubscribeStart_cb )(S_VEHICLE_INFO_LIST_t*);
typedef void (* CarDataSubscribeStop_cb )(S_VEHICLE_INFO_LIST_t*);
typedef void (* MediaInfo_cb )(S_MEDIA_INFO_t*);
typedef void (* MediaProgressBar_cb)(S_MEDIA_PROGRESS_BAR_t*);
typedef void (* ConnectException_cb)(S_CONNECTION_EXCEPTION_t*);
typedef void (* BtHfpRequest_cb)(S_BT_HFP_REQUEST_t*);
typedef void (* CarLifeDataSubscribeDone_cb)(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*);
typedef void (* NaviAssistantGuideInfo_cb)(S_NAVI_ASSITANT_GUIDE_INFO_t*);
typedef void (* MdAuthenResponse_cb)(S_AUTHEN_RESPONSE_t*);
typedef void (* MdAuthenResult_cb)(S_MD_AUTHEN_RESULT_t*);

typedef void (*	CmdStartBtAutoPairRequest_cb)(S_BT_START_PAIR_REQ_t*);
typedef void (* CmdBTHfpStatusRequest_cb)(S_BT_HFP_STATUS_REQUEST_t*);
typedef void (* CmdBTIdentifyResultInd_cb)(S_BT_INDENTIFY_RESULT_IND_t*);
typedef void (* VideoDataReceive_cb)(u8 *data, u32 len);
typedef void (* MediaInit_cb)(S_AUDIO_INIT_PARAMETER_t*);
typedef void (* MediaNormalData_cb)(u8 *data, u32 len);
	
typedef void (* TTSInit_cb)(S_AUDIO_INIT_PARAMETER_t*);
typedef void (* TTSNormalData_cb)(u8 *data, u32 len);


typedef void (* VRInit_cb)(S_AUDIO_INIT_PARAMETER_t*);
typedef void (* VRNormalData_cb)(u8 *data, u32 len);


typedef struct _CCarLibFuncCB 
{
	ProtocolVersionMatchStatus_cb ProtocolVersionMatchStatus;
	MDInfro_cb	MDInfro;
	MDBTPairInfro_cb	MDBTPairInfro;
	VideoEncoderInitDone_cb		VideoEncoderInitDone; 
	VideoEncoderFrameRateChangeDone_cb	 VideoEncoderFrameRateChangeDone;
	ModuleStatus_cb		ModuleStatus;
	NaviNextTurnInfo_cb		NaviNextTurnInfo;
	CarDataSubscribe_cb     CarDataSubscribe;
	CarDataSubscribeStart_cb CarDataSubscribeStart;
	CarDataSubscribeStop_cb		CarDataSubscribeStop;
	MediaInfo_cb	MediaInfo;
	MediaProgressBar_cb		MediaProgressBar;
	ConnectException_cb     ConnectException;
	BtHfpRequest_cb			BtHfpRequest;
	CarLifeDataSubscribeDone_cb		CarLifeDataSubscribeDone;
	NaviAssistantGuideInfo_cb	NaviAssistantGuideInfo;
	MdAuthenResponse_cb			MdAuthenResponse;
	MdAuthenResult_cb			MdAuthenResult;
	CmdStartBtAutoPairRequest_cb 	CmdStartBtAutoPairRequest;
 	CmdBTHfpStatusRequest_cb	 CmdBTHfpStatusRequest;
	CmdBTIdentifyResultInd_cb    CmdBTIdentifyResultInd;
	VideoDataReceive_cb   VideoDataReceive;
	MediaInit_cb	MediaInit;
	MediaNormalData_cb	MediaNormalData;
 	TTSInit_cb  TTSInit;
 	TTSNormalData_cb	TTSNormalData;
 	VRInit_cb	VRInit;
 	VRNormalData_cb   VRNormalData;
} CCarLibFuncCB;				/* ----------  end of struct CCarLibFuncCB  ---------- */
static CCarLibFuncCB	_CarLibFuncCB;
//////////////callback////////
void _ProtocolVersionMatchStatus(S_PROTOCOL_VERSION_MATCH_SATUS * data )
{
//	g_message("bind--->%s", __func__);
	_CarLibFuncCB.ProtocolVersionMatchStatus( (S_PROTOCOL_VERSION_MATCH_SATUS_t *)data);
}
void _MDInfro(S_MD_INFO * data)
{
//	g_message("bind--->%s", __func__);
	S_MD_INFO_t * _data	=	g_new0( S_MD_INFO_t, 1);
	_data->os	=	g_strdup(data->os.c_str());
	_data->board	=	g_strdup(data->board.c_str());
	_data->bootloader	=	g_strdup(data->bootloader.c_str());
	_data->brand	=	g_strdup(data->brand.c_str());
	_data->cpu_abi	=	g_strdup(data->cpu_abi.c_str());
	_data->cpu_abi2	=	g_strdup(data->cpu_abi2.c_str());
	_data->device	=	g_strdup(data->device.c_str());
	_data->display	=	g_strdup(data->display.c_str());

	_data->fingerprint	=	g_strdup(data->fingerprint.c_str());
	_data->hardware	=	g_strdup(data->hardware.c_str());
	_data->host	=	g_strdup(data->host.c_str());
	_data->cid	=	g_strdup(data->cid.c_str());
	_data->manufacturer	=	g_strdup(data->manufacturer.c_str());
	_data->model	=	g_strdup(data->model.c_str());
	_data->product	=	g_strdup(data->product.c_str());
	_data->serial	=	g_strdup(data->serial.c_str());
	
	_data->codename	=	g_strdup(data->codename.c_str());
	_data->incremental	=	g_strdup(data->incremental.c_str());
	_data->release	=	g_strdup(data->release.c_str());
	_data->sdk	=	g_strdup(data->sdk.c_str());
	_data->sdk_int	=	data->sdk_int;

	_data->token	=	g_strdup( data->token.c_str());
	_data->btaddress	=	g_strdup( data->btaddress.c_str());

	_CarLibFuncCB.MDInfro( _data);
	if( _data->os != NULL)
	{
		g_free( _data->os);
	}
	if( _data->board != NULL)
	{
		g_free( _data->board);
	}
	if( _data->bootloader != NULL)
	{
		g_free( _data->bootloader);
	}
	if( _data->brand != NULL)
	{
		g_free( _data->brand);
	}
	if( _data->cpu_abi != NULL)
	{
		g_free( _data->cpu_abi);
	}
	if( _data->cpu_abi2 != NULL)
	{
		g_free( _data->cpu_abi2);
	}
	if( _data->device != NULL)
	{
		g_free( _data->device);
	}
	if( _data->display != NULL)
	{
		g_free( _data->display);
	}
	if( _data->fingerprint != NULL)
	{
		g_free( _data->fingerprint);
	}
	if( _data->hardware != NULL)
	{
		g_free( _data->hardware);
	}
	if( _data->host != NULL)
	{
		g_free( _data->host);
	}
	if( _data->cid != NULL)
	{
		g_free( _data->cid);
	}
	if( _data->manufacturer != NULL)
	{
		g_free( _data->manufacturer);
	}
	if( _data->model != NULL)
	{
		g_free( _data->model);
	}
	if( _data->product != NULL)
	{
		g_free( _data->product);
	}
	if( _data->serial != NULL)
	{
		g_free( _data->serial);
	}

	if( _data->codename != NULL)
	{
		g_free( _data->codename);
	}
	if( _data->incremental != NULL)
	{
		g_free( _data->incremental);
	}
	if( _data->release != NULL)
	{
		g_free( _data->release);
	}
	if( _data->sdk != NULL)
	{
		g_free( _data->sdk);
	}
	if( _data->token != NULL)
	{
		g_free( _data->token);
	}
	if( _data->btaddress != NULL)
	{
		g_free( _data->btaddress);
	}
	g_free( _data);
	///////////need free when have time
}

void  _MDBTPairInfro( S_BT_PAIR_INFO * data )
{
	g_message("bind--->%s", __func__);
	S_BT_PAIR_INFO_t * _data	=	g_new0(S_BT_PAIR_INFO_t, 1);

	_data->address	=	g_strdup( data->address.c_str());
	_data->passKey	=	g_strdup( data->passKey.c_str());
	_data->hash	=	g_strdup( data->hash.c_str());
	_data->randomizer	=	g_strdup( data->randomizer.c_str());
	_data->uuid	=	g_strdup( data->uuid.c_str());
	_data->name	=	g_strdup( data->name.c_str());
	_data->status	=	data->status;


	_CarLibFuncCB.MDBTPairInfro( _data );


	if( _data->address)
	{
		g_free( _data->address);
	}
	if( _data->passKey )
	{
		g_free( _data->passKey);
	}
	if( _data->hash)
	{
		g_free( _data->hash);
	}
	if(_data->randomizer)
	{
		g_free( _data->randomizer);
	}
	if( _data->uuid)
	{
		g_free( _data->uuid);
	}
	if( _data->name)
	{
		g_free( _data->name);
	};
	g_free( _data);


}
void  _VideoEncoderInitDone(S_VIDEO_ENCODER_INIT_DONE* data)
{
	_CarLibFuncCB.VideoEncoderInitDone( (S_VIDEO_ENCODER_INIT_DONE_t *) data);
}
void  _VideoEncoderFrameRateChangeDone(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE* data)
{
	_CarLibFuncCB.VideoEncoderFrameRateChangeDone( (S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE_t *) data);
}
void  _ModuleStatus(S_MODULE_STATUS_LIST_MOBILE* data )
{
	S_MODULE_STATUS_LIST_MOBILE_t _data;
	S_MODULE_STATUS_MOBILE * _p	=	data->moduleStatus;

	_data.cnt	=	data->cnt;
	_data.moduleStatus	=	g_new0( S_MODULE_STATUS_MOBILE_t, _data.cnt);

	int i	=	0;
	
	for(i ; i < _data.cnt; i++)
	{

		_data.moduleStatus[i].moduleID	=	_p->moduleID;
		_data.moduleStatus[i].statusID	=	_p->statusID;
		_p	=	_p->next;
		g_message("_data[%d]%d %d", i,_data.moduleStatus[i].moduleID, _data.moduleStatus[i].statusID );
	}
	_CarLibFuncCB.ModuleStatus( &_data);
	g_free( _data.moduleStatus);
}

void  _NaviNextTurnInfo(S_NAVI_NEXT_TURN_INFO* data)
{
	_CarLibFuncCB.NaviNextTurnInfo( (S_NAVI_NEXT_TURN_INFO_t *) data);
}

void  _CarDataSubscribe (S_VEHICLE_INFO_LIST* data)
{
	_CarLibFuncCB.CarDataSubscribe( (S_VEHICLE_INFO_LIST_t *) data);
}

void  _CarDataSubscribeStart(S_VEHICLE_INFO_LIST* data )
{
	_CarLibFuncCB.CarDataSubscribeStart( (S_VEHICLE_INFO_LIST_t *) data);
}

void  _CarDataSubscribeStop (S_VEHICLE_INFO_LIST* data )
{
	_CarLibFuncCB.CarDataSubscribeStop( (S_VEHICLE_INFO_LIST_t *) data);
}

void  _MediaInfo (S_MEDIA_INFO* data )
{
	S_MEDIA_INFO_t * _info	=	g_new0( S_MEDIA_INFO_t, 1);
	_info->source	=	g_strdup( data->source.c_str());
	_info->song		=	g_strdup( data->song.c_str());
	_info->artist	=	g_strdup( data->artist.c_str());
	_info->album	=	g_strdup( data->album.c_str());
	_info->albumArt	=	g_strdup( data->albumArt.c_str());
	_info->duration	=	data->duration;
	_info->playlistNum	=	data->playlistNum;
	_info->songId	=	g_strdup( data->songId.c_str());
	_info->mode		=	data->mode;

	_CarLibFuncCB.MediaInfo( _info	);
	if( _info->source)
	{
		g_free( _info->source);
	}
	if( _info->song)
	{
		g_free( _info->song);
	}
	if( _info->artist)
	{
		g_free( _info->artist);
	}
	if( _info->album)
	{
		g_free( _info->album);
	}
	if( _info->albumArt)
	{
		g_free( _info->albumArt);
	}
	if( _info->songId)
	{
		g_free(  _info->songId);
	}
	g_free( _info);
}

void  _MediaProgressBar(S_MEDIA_PROGRESS_BAR* data)
{
	_CarLibFuncCB.MediaProgressBar( (S_MEDIA_PROGRESS_BAR_t *) data);

}
void  _ConnectException(S_CONNECTION_EXCEPTION * data)
{
	_CarLibFuncCB.ConnectException( (S_CONNECTION_EXCEPTION_t *) data);

}
void  _BtHfpRequest(S_BT_HFP_REQUEST * data)
{
	g_message("bind--->%s", __func__);
	S_BT_HFP_REQUEST_t _data;
	_data.command	=	data->command;
	_data.phoneNum	=	g_strdup( data->phoneNum.c_str());
	_data.dtmfCode	=	data->dtmfCode;

	_CarLibFuncCB.BtHfpRequest( &_data);
	g_free( _data.phoneNum);


}
void  _CarLifeDataSubscribeDone(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST * data)
{
	_CarLibFuncCB.CarLifeDataSubscribeDone( (S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t *) data);

}
void  _NaviAssistantGuideInfo(S_NAVI_ASSITANT_GUIDE_INFO* data)
{
	_CarLibFuncCB.NaviAssistantGuideInfo( (S_NAVI_ASSITANT_GUIDE_INFO_t *) data);

}
void  _MdAuthenResponse(S_AUTHEN_RESPONSE* data)
{
	_CarLibFuncCB.MdAuthenResponse((S_AUTHEN_RESPONSE_t *)data);
}
void  _MdAuthenResult(S_MD_AUTHEN_RESULT * data)
{
	_CarLibFuncCB.MdAuthenResult( (S_MD_AUTHEN_RESULT_t*) data);
}

void  _CmdStartBtAutoPairRequest(S_BT_START_PAIR_REQ * data)
{
	g_message("CCarLifeLib_c.cpp:%s", __func__);
	S_BT_START_PAIR_REQ_t * _data	=	g_new0( S_BT_START_PAIR_REQ_t, 1);

	_data->osType	=	data->osType;
	_data->address	=	g_strdup( data->address.c_str());	

	_CarLibFuncCB.CmdStartBtAutoPairRequest( _data);

	g_free( _data->address);
	g_free( _data);
}
void  _CmdBTHfpStatusRequest(S_BT_HFP_STATUS_REQUEST * data)
{
	_CarLibFuncCB.CmdBTHfpStatusRequest( (S_BT_HFP_STATUS_REQUEST_t*) data);
}
void  _CmdBTIdentifyResultInd(S_BT_INDENTIFY_RESULT_IND * data)
{
	S_BT_INDENTIFY_RESULT_IND_t * _data	=	g_new0( S_BT_INDENTIFY_RESULT_IND_t, 1);
	_data->status	=	data->status;
	_data->address	=	g_strdup( data->address.c_str());
	_CarLibFuncCB.CmdBTIdentifyResultInd( _data );

	g_free( _data->address);
	g_free( _data);
}
void  _VideoDataReceive(u8 *data, u32 len)
{
	_CarLibFuncCB.VideoDataReceive( (uint8_t*) data, (uint32_t)len);
}
void  _MediaInit(S_AUDIO_INIT_PARAMETER * data )
{
	g_message("%s", __func__);
	_CarLibFuncCB.MediaInit( (S_AUDIO_INIT_PARAMETER_t*) data);
}
void  _MediaNormalData(u8 *data, u32 len)
{
//	g_message("%s", __func__);
	_CarLibFuncCB.MediaNormalData( (uint8_t*) data, (uint32_t) len);
}

void  _TTSInit(S_AUDIO_INIT_PARAMETER* data)
{
	_CarLibFuncCB.TTSInit( (S_AUDIO_INIT_PARAMETER_t*) data);
}
void  _TTSNormalData(u8 *data, u32 len)
{
	_CarLibFuncCB.TTSNormalData( (uint8_t*) data, (uint32_t)len);
}


void  _VRInit(S_AUDIO_INIT_PARAMETER* data )
{
	_CarLibFuncCB.VRInit( (S_AUDIO_INIT_PARAMETER_t*) data);
}
void  _VRNormalData(u8 *data, u32 len)
{
	_CarLibFuncCB.VRNormalData( (uint8_t*) data, (uint32_t)len);
}

int C_carLifeLibInit(){
	return CCarLifeLib::getInstance()->carLifeLibInit();
}
void C_carLifeLibDestory()
{
	return CCarLifeLib::getInstance()->carLifeLibDestory();
}
void C_connection_disconnect()
{	
	CCarLifeLib::getInstance()->disconnect();
}

int C_connectionSetup(){
	return CCarLifeLib::getInstance()->connectionSetup();
}

int C_connectionSetupIp(const char * mdIP){
	return CCarLifeLib::getInstance()->connectionSetup(string(mdIP));
}

int C_connectionSetupIpName(const char * mdIP, const char * interfaceName){
	return CCarLifeLib::getInstance()->connectionSetup(string(mdIP), string(interfaceName));
}

//
int C_cmdHUProtoclVersion(S_HU_PROTOCOL_VERSION_t * version){
	return CCarLifeLib::getInstance()->cmdHUProtoclVersion((S_HU_PROTOCOL_VERSION *)version);
}
int C_cmdHUInfro(S_HU_INFO_t* huInfo){
	g_message("CCarLifeLib_c.cpp->%s->btaddress:%s", __func__, huInfo->btaddress);
	S_HU_INFO _info;
	_info.os	=	( huInfo->os) ;
	_info.board	=	( huInfo->board) ;
	_info.bootloader	=	( huInfo->bootloader) ;
	_info.brand	=	( huInfo->brand) ;
	_info.cpu_abi	=	( huInfo->cpu_abi) ;
	_info.cpu_abi2	=	( huInfo->cpu_abi2) ;
	_info.device	=	( huInfo->device) ;
	_info.display	=	( huInfo->display) ;
	_info.fingerprint	=	( huInfo->fingerprint) ;
	_info.hardware	=	( huInfo->hardware) ;
	_info.host	=	( huInfo->host) ;
	_info.cid	=	( huInfo->os) ;
	_info.manufacturer	=	( huInfo->manufacturer) ;
	_info.model	=	( huInfo->model) ;
	_info.product	=	( huInfo->product) ;
	_info.serial	=	( huInfo->serial) ;
	_info.codename	=	( huInfo->codename) ;
	_info.incremental	=	( huInfo->incremental) ;
	_info.release	=	( huInfo->release) ;
	_info.sdk	=	( huInfo->sdk) ;
	_info.sdk_int	=	huInfo->sdk_int;
	_info.token	=	( huInfo->token) ;
	_info.btaddress	=	( huInfo->btaddress) ;
	

	return CCarLifeLib::getInstance()->cmdHUInfro(&_info);
}
int C_cmdHUBTPairInfro(S_BT_PAIR_INFO_t* info){
	int _res	=	0;
	S_BT_PAIR_INFO _info;
	if(info->address != NULL)
	{
		_info.address	=	(info->address);
	}
	if(info->passKey != NULL)
	{
		_info.passKey	=	(info->passKey);
	}
	if(info->hash != NULL)
	{
		_info.hash	=	(info->hash);
	}
	if(info->randomizer != NULL)
	{
		_info.randomizer	=	(info->randomizer);
	}
	if(info->uuid != NULL)
	{
		_info.uuid	=	(info->uuid);
	}
	if(info->name != NULL)
	{
		_info.name	=	(info->name);
	}

	_info.status	=	info->status;

	_res	=	CCarLifeLib::getInstance()->cmdHUBTPairInfro(&_info);

//	g_free( _info);
	return _res;
}
int C_cmdVideoEncoderInit(S_VIDEO_ENCODER_INIT_t* initParam){
	return CCarLifeLib::getInstance()->cmdVideoEncoderInit((S_VIDEO_ENCODER_INIT *)initParam);
}
int C_cmdVideoEncoderStart(){
	return CCarLifeLib::getInstance()->cmdVideoEncoderStart();
}
int C_cmdVideoEncoderPause(){
	return CCarLifeLib::getInstance()->cmdVideoEncoderPause();
}
int C_cmdVideoEncoderReset(){
	return CCarLifeLib::getInstance()->cmdVideoEncoderReset();
}
int C_cmdVideoEncoderFrameRateChange(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_t* videoParam){
	return CCarLifeLib::getInstance()->cmdVideoEncoderFrameRateChange((S_VIDEO_ENCODER_FRAME_RATE_CHANGE * )videoParam);
}
int C_cmdPauseMedia(){
	return CCarLifeLib::getInstance()->cmdPauseMedia();
}
int C_cmdCarVelocity(S_CAR_VELOCITY_t* carVelocity){
	return CCarLifeLib::getInstance()->cmdCarVelocity( (S_CAR_VELOCITY *)carVelocity);
}
int C_cmdCarGPS(S_CAR_GPS_t* cps){
	return CCarLifeLib::getInstance()->cmdCarGPS( (S_CAR_GPS *)cps);
}
int C_cmdCarGyroscope(S_CAR_GYROSCOPE_t* cyro){
	return CCarLifeLib::getInstance()->cmdCarGyroscope((S_CAR_GYROSCOPE *)cyro);
}
int C_cmdCarAcceleration(S_CAR_ACCELERATION_t* acceleration){
	return CCarLifeLib::getInstance()->cmdCarAcceleration((S_CAR_ACCELERATION *)acceleration);
}
int C_cmdCarOil( S_CAR_OIL_t * oil){
	return CCarLifeLib::getInstance()->cmdCarOil( (S_CAR_OIL*)oil);
}
int C_cmdLaunchModeNormal(){
	return CCarLifeLib::getInstance()->cmdLaunchModeNormal();
}
int C_cmdLaunchModePhone(){
	return CCarLifeLib::getInstance()->cmdLaunchModePhone();
}
int C_cmdLaunchModeMap(){
	return CCarLifeLib::getInstance()->cmdLaunchModeMap();
}
int C_cmdLaunchModeMusic(){
	return CCarLifeLib::getInstance()->cmdLaunchModeMusic();
}

int C_cmdReceiveOperation(){
	return CCarLifeLib::getInstance()->cmdReceiveOperation();
}
void C_cmdRegisterProtocolVersionMatchStatus(void (*pFunc)(S_PROTOCOL_VERSION_MATCH_SATUS_t*)){
	_CarLibFuncCB.ProtocolVersionMatchStatus	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterProtocolVersionMatchStatus(_ProtocolVersionMatchStatus);
}


void C_cmdRegisterMDInfro(void (*pFunc)(S_MD_INFO_t*)){
	_CarLibFuncCB.MDInfro	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMDInfro(_MDInfro);
}

void C_cmdRegisterMDBTPairInfro(void (*pFunc)(S_BT_PAIR_INFO_t*)){
	_CarLibFuncCB.MDBTPairInfro	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMDBTPairInfro(_MDBTPairInfro);
}

void C_cmdRegisterVideoEncoderInitDone(void (*pFunc)(S_VIDEO_ENCODER_INIT_DONE_t*)){
	_CarLibFuncCB.VideoEncoderInitDone	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterVideoEncoderInitDone(_VideoEncoderInitDone);
}

void C_cmdRegisterVideoEncoderFrameRateChangeDone(void (*pFunc)(S_VIDEO_ENCODER_FRAME_RATE_CHANGE_DONE_t*)){
	_CarLibFuncCB.VideoEncoderFrameRateChangeDone	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterVideoEncoderFrameRateChangeDone(_VideoEncoderFrameRateChangeDone);
}

void C_cmdRegisterTelStateChangeIncoming(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterTelStateChangeIncoming(pFunc);
}

void C_cmdRegisterTelStateChangeOutGoing(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterTelStateChangeOutGoing(pFunc);
}

void C_cmdRegisterTelStateChangeIdle(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterTelStateChangeIdle(pFunc);
}

void C_cmdRegisterTelStateChangeInCalling(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterTelStateChangeInCalling(pFunc);
}

void C_cmdRegisterScreenOn(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterScreenOn(pFunc);
}

void C_cmdRegisterScreenOff(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterScreenOff(pFunc);
}

void C_cmdRegisterScreenUserPresent(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterScreenUserPresent(pFunc);
}

void C_cmdRegisterForeground(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterForeground(pFunc);
}

void C_cmdRegisterBackground(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterBackground(pFunc);
}

void C_cmdRegisterGoToDeskTop(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterGoToDeskTop(pFunc);
}

void C_cmdRegisterMicRecordWakeupStart(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterMicRecordWakeupStart(pFunc);
}
void C_cmdRegisterMicRecordEnd(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterMicRecordEnd(pFunc);
}

void C_cmdRegisterMicRecordRecogStart(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterMicRecordRecogStart(pFunc);
}

int C_videoReceiveOperation(){
	return CCarLifeLib::getInstance()->videoReceiveOperation();

}

void C_videoRegisterDataReceive(void (*pFunc)(u8 *data, u32 len)){
	_CarLibFuncCB.VideoDataReceive	=	pFunc;
	CCarLifeLib::getInstance()->videoRegisterDataReceive(_VideoDataReceive);
}

void C_videoRegisterHeartBeat(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->videoRegisterHeartBeat(pFunc);
}

int C_mediaReceiveOperation(){
//	cout<<"----------->CCarLifeLib_c.cpp---->C_mediaReceiveOperation"<<endl;
	return CCarLifeLib::getInstance()->mediaReceiveOperation();
}

void C_mediaRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*)){
	_CarLibFuncCB.MediaInit	=	pFunc;
	CCarLifeLib::getInstance()->mediaRegisterInit(_MediaInit);
}

void C_mediaRegisterNormalData(void (*pFunc)(u8 *data, u32 len)){
	_CarLibFuncCB.MediaNormalData	=	pFunc;
	CCarLifeLib::getInstance()->mediaRegisterNormalData(_MediaNormalData);
}

void C_mediaRegisterStop(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->mediaRegisterStop(pFunc);
}
void C_mediaRegisterPause(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->mediaRegisterPause(pFunc);
}
void C_mediaRegisterResume(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->mediaRegisterResume(pFunc);
}
void C_mediaRegisterSeek(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->mediaRegisterSeek(pFunc);
}

int C_ttsReceiveOperation(){
	return CCarLifeLib::getInstance()->ttsReceiveOperation();
}

void C_ttsRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*)){
	_CarLibFuncCB.TTSInit	=	pFunc;
	CCarLifeLib::getInstance()->ttsRegisterInit(_TTSInit);
}

void C_ttsRegisterNormalData(void (*pFunc)(u8 *data, u32 len)){
	_CarLibFuncCB.TTSNormalData	=	pFunc;
	CCarLifeLib::getInstance()->ttsRegisterNormalData(_TTSNormalData);
}

void C_ttsRegisterStop(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->ttsRegisterStop(pFunc);
}

int C_sendVRRecordData(u8* data, u32 size, u32 timeStamp){
	return CCarLifeLib::getInstance()->sendVRRecordData(data, size, timeStamp);
}

int C_vrReceiveOperation(){
	return CCarLifeLib::getInstance()->vrReceiveOperation();
}

void C_vrRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER_t*)){
	_CarLibFuncCB.VRInit	=	pFunc;
	CCarLifeLib::getInstance()->vrRegisterInit(_VRInit);
}

void C_vrRegisterNormalData(void (*pFunc)(u8 *data, u32 len)){
	_CarLibFuncCB.VRNormalData	=	pFunc;
	CCarLifeLib::getInstance()->vrRegisterNormalData(_VRNormalData);
}

void C_vrRegisterStop(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->vrRegisterStop(pFunc);
}

//int C_ctrlTouchAction(S_TOUCH_ACTION_t* touchAction){
//	return CCarLifeLib::getInstance()->ctrlTouchAction((S_TOUCH_ACTION *)touchAction);
//}

int C_ctrlTouchActionDown(S_TOUCH_ACTION_DOWN_t* touchActionDown){
	return CCarLifeLib::getInstance()->ctrlTouchActionDown((S_TOUCH_ACTION_DOWN *)touchActionDown);
}

int C_ctrlTouchActionUp(S_TOUCH_ACTION_UP_t* touchActionUp){
	return CCarLifeLib::getInstance()->ctrlTouchActionUp((S_TOUCH_ACTION_UP *)touchActionUp);
}

int C_ctrlTouchActionMove(S_TOUCH_ACTION_MOVE_t* touchActionMove){
	return CCarLifeLib::getInstance()->ctrlTouchActionMove((S_TOUCH_ACTION_MOVE *)touchActionMove);
}

int C_ctrlTouchSigleClick(S_TOUCH_SIGNAL_CLICK_t* touchSingleClick){
	return CCarLifeLib::getInstance()->ctrlTouchSigleClick((S_TOUCH_SIGNAL_CLICK *)touchSingleClick);
}

int C_ctrlTouchDoubleClick(S_TOUCH_DOUBLE_CLICK_t* touchDoubleClick){
	return CCarLifeLib::getInstance()->ctrlTouchDoubleClick((S_TOUCH_DOUBLE_CLICK *)touchDoubleClick);
}

int C_ctrlTouchLongPress(S_TOUCH_LONG_PRESS_t* touchLongPress){
	return CCarLifeLib::getInstance()->ctrlTouchLongPress((S_TOUCH_LONG_PRESS *)touchLongPress);
}

int C_ctrlTouchCarHardKeyCode(S_TOUCH_CAR_HARD_KEY_CODE_t* touchCarHardKeyCode){
	return CCarLifeLib::getInstance()->ctrlTouchCarHardKeyCode((S_TOUCH_CAR_HARD_KEY_CODE *)touchCarHardKeyCode);
}

int C_ctrlReceiveOperation(){
	return CCarLifeLib::getInstance()->ctrlReceiveOperation();
}
	
void C_ctrlRegisterUIActionSound(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->ctrlRegisterUIActionSound(pFunc);
}

//added on 9th Semptember 2015
//0x00018025
int C_cmdGoToForeground(){
	return CCarLifeLib::getInstance()->cmdGoToForeground();
}

//0x00010026
void C_cmdRegisterModuleStatus(void (*pFunc)(S_MODULE_STATUS_LIST_MOBILE_t*)){
	_CarLibFuncCB.ModuleStatus	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterModuleStatus(_ModuleStatus);
}

//0x00018027
int C_cmdStatisticInfo(S_STATISTICS_INFO_t* info){
//	S_STATISTICS_INFO * _info	=	g_new0( S_STATISTICS_INFO, 1);
//	_info->cuid	=	( info->cuid) ;
//	_info->versionName	=	(info->versionName);
//	_info->versionCode	=	info->versionCode;
//	_info->channel	=	string( (const char *)(info->channel));
//	_info->connectCount	=	info->connectCount;
//	_info->connectSuccessCount	=	info->connectSuccessCount;
//	_info->connectTime	=	info->connectTime;
//	_info->crashLog	=	( info->crashLog);
	S_STATISTICS_INFO _info;
	_info.cuid	=	( info->cuid) ;
	_info.versionName	=	(info->versionName);
	_info.versionCode	=	info->versionCode;
	_info.channel	=	string( (const char *)(info->channel));
	_info.connectCount	=	info->connectCount;
	_info.connectSuccessCount	=	info->connectSuccessCount;
	_info.connectTime	=	info->connectTime;
	_info.crashLog	=	( info->crashLog);

	return CCarLifeLib::getInstance()->cmdStatisticInfo( &_info);
}

//0x00018028
int C_cmdModuleControl(S_MODULE_STATUS_CONTROL_t* control){
	 return CCarLifeLib::getInstance()->cmdModuleControl( (S_MODULE_STATUS_CONTROL *)control);
}

//0x00018029
int C_cmdCarDataGear(S_GEAR_INFO_t* info){
	return CCarLifeLib::getInstance()->cmdCarDataGear( ( S_GEAR_INFO *)info);
}

//0x00010030
void C_cmdRegisterNaviNextTurnInfo(void (*pFunc)(S_NAVI_NEXT_TURN_INFO_t*)){
	_CarLibFuncCB.NaviNextTurnInfo	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterNaviNextTurnInfo(_NaviNextTurnInfo);
}

//0x00010031
void C_cmdRegisterCarDataSubscribe(void (*pFunc)(S_VEHICLE_INFO_LIST_t*)){
	_CarLibFuncCB.CarDataSubscribe	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterCarDataSubscribe(_CarDataSubscribe);
}

//0x00018032
int C_cmdCarDataSubscribeDone(S_VEHICLE_INFO_LIST_t* list){
	return CCarLifeLib::getInstance()->cmdCarDataSubscribeDone( (S_VEHICLE_INFO_LIST *)list);
}

//0x00010033
void C_cmdRegisterCarDataSubscribeStart(void (*pFunc)(S_VEHICLE_INFO_LIST_t*)){
	_CarLibFuncCB.CarDataSubscribeStart	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterCarDataSubscribeStart(_CarDataSubscribeStart);
}

//0x00010034
void C_cmdRegisterCarDataSubscribeStop(void (*pFunc)(S_VEHICLE_INFO_LIST_t*)){
	_CarLibFuncCB.CarDataSubscribeStop	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterCarDataSubscribeStop(_CarDataSubscribeStop);
}

//0x00010035
void C_cmdRegisterMediaInfo(void (*pFunc)(S_MEDIA_INFO_t*)){
	_CarLibFuncCB.MediaInfo	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMediaInfo(_MediaInfo);
}

//0x00010036
void C_cmdRegisterMediaProgressBar(void (*pFunc)(S_MEDIA_PROGRESS_BAR_t*)){
	_CarLibFuncCB.MediaProgressBar	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMediaProgressBar(_MediaProgressBar);
}

//0x00010037
void C_cmdRegisterConnectException(void (*pFunc)(S_CONNECTION_EXCEPTION_t*)){
	_CarLibFuncCB.ConnectException	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterConnectException(_ConnectException);
}

//0x00010038
void C_cmdRegisterRequestGoToForeground(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterRequestGoToForeground(pFunc);
}

//0x00010039
void C_cmdRegisterUIActionSound(void (*pFunc)(void)){

	CCarLifeLib::getInstance()->cmdRegisterUIActionSound(pFunc);
}

//added on 5th January 2016
//0x00010040
void C_cmdRegisterBtHfpRequest(void (*pFunc)(S_BT_HFP_REQUEST_t*)){

	g_message("%s", __func__);
	_CarLibFuncCB.BtHfpRequest	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterBtHfpRequest(_BtHfpRequest);
}

//0x00018041
int C_cmdBtHfpIndication(S_BT_HFP_INDICATION_t* indication){

	S_BT_HFP_INDICATION _indi;

	_indi.state	=	indication->state;

	if( indication->phoneNum != NULL &&  g_strcmp0( indication->phoneNum, "") != 0)
	{
		_indi.phoneNum	=	string((const char*)indication->phoneNum);
	}
	if( indication->name != NULL &&  g_strcmp0( indication->name, "") != 0)
	{

		_indi.name	=	indication->name;
	}
	if( indication->address != NULL &&  g_strcmp0( indication->address, "") != 0)
	{
		_indi.address	=	indication->address;
	}

	return CCarLifeLib::getInstance()->cmdBtHfpIndication( &_indi);
}

//0x00018042
int C_cmdBtHfpConnection(S_BT_HFP_CONNECTION_t* connection){
	int _res		=	0;

	S_BT_HFP_CONNECTION _conn;

	_conn.state	=	connection->state;
	g_message("\n\nC_cmdBtHfpConnection11111111-----[%d][%s][%s]\n\n", _conn.state, _conn.address.c_str(), _conn.name.c_str());
	g_message("\n\nC_cmdBtHfpConnection22222222-----[%d][%s][%s]\n\n", connection->state, connection->address, connection->name);

	if( connection->address != NULL && g_strcmp0( connection->address, "") != 0)
	{
		_conn.address	=	string(connection->address);
	}
	if( connection->name !=	NULL && g_strcmp0( connection->name, "") != 0)
	{
		_conn.name	=	string(connection->name);
	}

	_res	=	CCarLifeLib::getInstance()->cmdBtHfpConnection( &_conn);

//	g_free( _conn );

	return _res;

}

//0x00018043 
int C_cmdCarLifeDataSubscribe(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t* list){
	return CCarLifeLib::getInstance()->cmdCarLifeDataSubscribe( (S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST *)list);
}

//0x00010044 
void C_cmdRegisterCarLifeDataSubscribeDone(void (*pFunc)(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t*)){

	_CarLibFuncCB.CarLifeDataSubscribeDone	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterCarLifeDataSubscribeDone(_CarLifeDataSubscribeDone);
}

//0x00018045 
int C_cmdCarLifeDataSubscribeStart(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t* list){
	return CCarLifeLib::getInstance()->cmdCarLifeDataSubscribeStart( (S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST *)list);
}

//0x00018046 
int C_cmdCarLifeDataSubscribeStop(S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST_t* list){
	return CCarLifeLib::getInstance()->cmdCarLifeDataSubscribeStop( (S_SUBSCRIBE_MOBILE_CARLIFE_INFO_LIST *)list);
}

//0x00010047 
void C_cmdRegisterNaviAssistantGuideInfo(void (*pFunc)(S_NAVI_ASSITANT_GUIDE_INFO_t*)){

	_CarLibFuncCB.NaviAssistantGuideInfo	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterNaviAssistantGuideInfo(_NaviAssistantGuideInfo);
}

//0x00018048 
int C_cmdHuAuthenRequest(S_AUTHEN_REQUEST_t* req){
	return CCarLifeLib::getInstance()->cmdHuAuthenRequest( (S_AUTHEN_REQUEST *)req);
}

//0x00010049 
void C_cmdRegisterMdAuthenResponse(void (*pFunc)(S_AUTHEN_RESPONSE_t*)){

	_CarLibFuncCB.MdAuthenResponse	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMdAuthenResponse(_MdAuthenResponse);
}

//0x0001804A 
int C_cmdHuAuthenResult(S_HU_AUTHEN_RESULT_t* result){
	return CCarLifeLib::getInstance()->cmdHuAuthenResult((S_HU_AUTHEN_RESULT *)result);
}

//0x0001804B
void C_cmdRegisterMdAuthenResult(void (*pFunc)(S_MD_AUTHEN_RESULT_t*)){

	_CarLibFuncCB.MdAuthenResult	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterMdAuthenResult(_MdAuthenResult);
}

//0x0001004C 
void C_cmdRegisterGotoForgroundResponse(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterGotoForgroundResponse(pFunc);
}

//0x0001004D 
void C_cmdRegisterStartBtAutoPairRequest(void (*pFunc)(S_BT_START_PAIR_REQ_t*)){

	_CarLibFuncCB.CmdStartBtAutoPairRequest	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterStartBtAutoPairRequest(_CmdStartBtAutoPairRequest);
}

//0x0001804E
int C_cmdBTHfpResponse(S_BT_HFP_RESPONSE_t* rep){
	return CCarLifeLib::getInstance()->cmdBTHfpResponse( (S_BT_HFP_RESPONSE *)rep);
}

//0x0001004F
void C_cmdRegisterBTHfpStatusRequest(void (*pFunc)(S_BT_HFP_STATUS_REQUEST_t*)){

	_CarLibFuncCB.CmdBTHfpStatusRequest	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterBTHfpStatusRequest(_CmdBTHfpStatusRequest);
}

//0x00018050
int C_cmdBTHfpStatusResponse(S_BT_HFP_STATUS_RESPONSE_t* rep){
	return CCarLifeLib::getInstance()->cmdBTHfpStatusResponse((S_BT_HFP_STATUS_RESPONSE *)rep);
}

//added on 3th March 2016
//0x00010051
void C_cmdRegisterFeatureConfigRequest(void (*pFunc)(void)){
	CCarLifeLib::getInstance()->cmdRegisterFeatureConfigRequest(pFunc);
}

//0x00018052
int C_cmdFeatureConfigResponse(S_FEATURE_CONFIG_LIST_t* list){
	g_message("\n\nbind-->C_cmdFeatureConfigResponse[%d]\n\n", list->cnt);

	int _res	=	0;
	S_FEATURE_CONFIG_LIST _list;
	_list.cnt	=	list->cnt;
	S_FEATURE_CONFIG_t * _next_t;
	_next_t	=	list->pFeatureConfig;
	if(list->cnt <= 7)
	{
		S_FEATURE_CONFIG _p[list->cnt];
		S_FEATURE_CONFIG* _last = NULL;
		int i = 0;
		while(_next_t != NULL)
		{
			if( _last != NULL)
			{
				_last->pNext	=	&_p[i];

			}
			else
			{
				_list.pFeatureConfig	=	&_p[i];
			}

			_p[i].key.assign(_next_t->key);

			_p[i].value	=	_next_t->value;

			_last	=	&_p[i];

			_next_t	=	_next_t->pNext;
			i++;
		}
		_res	=	CCarLifeLib::getInstance()->cmdFeatureConfigResponse( &_list);
	}
	else
	{
		g_message("\n\nbind-->C_cmdFeatureConfigResponse input param cnt error!!!!![%d]\n\n", list->cnt);
		return -1;
	}

//	S_FEATURE_CONFIG_LIST * _list	=	new S_FEATURE_CONFIG_LIST;
//
//
//	S_FEATURE_CONFIG * _p, *_last, * _next;
//
//
//
//	_last	=	NULL;
//
//	while( _next_t != NULL )
//	{
//		_p	=	new S_FEATURE_CONFIG;
//		if( _last != NULL)
//		{
//			_last->pNext	=	_p;
//
//		}
//		else
//		{
//			_list->pFeatureConfig	=	_p;
//		}
//
//		_p->key.assign(_next_t->key);
//
//		_p->value	=	_next_t->value;
//
//		_last	=	_p;
//
//		_next_t	=	_next_t->pNext;
//
//	}
//
//	_next	=	_list->pFeatureConfig;
//
//	while( _next != NULL)
//	{
//		_p	=	_next;
//
//		_next	=	_next->pNext;
//		_p->key.clear();
//		delete _p;
//	}
//	delete _list;
	return _res;
}

//0x00018053
int C_cmdBTStartIdentifyReq(S_BT_START_IDENTIFY_REQ_t* req){
	S_BT_START_IDENTIFY_REQ	_req;

	_req.address	=	(req->address);

	return CCarLifeLib::getInstance()->cmdBTStartIdentifyReq( &_req);
}

//0x00010054
void C_cmdRegisterBTIdentifyResultInd(void (*pFunc)(S_BT_INDENTIFY_RESULT_IND_t*)){

	_CarLibFuncCB.CmdBTIdentifyResultInd	=	pFunc;
	CCarLifeLib::getInstance()->cmdRegisterBTIdentifyResultInd(_CmdBTIdentifyResultInd);
}

//0x00018055
int C_cmdErrorCode(S_ERROR_CODE_t* errorCode){
	return CCarLifeLib::getInstance()->cmdErrorCode( (S_ERROR_CODE *)errorCode);
}
//0x00010059
void C_cmdRegisterMdExit(void (*pFunc)(void))
{
	return CCarLifeLib::getInstance()->cmdRegisterMdExit( pFunc);
}




