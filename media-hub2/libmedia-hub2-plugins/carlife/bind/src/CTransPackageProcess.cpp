
/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/

#include"CCarLifeLibWrapper_c.h"
#include "CTransPackageProcess.h"
#include <glib.h>
#include <google/protobuf/io/coded_stream.h>
#include "CarlifeAccelerationProto.pb.h"
#include "CarlifeVideoEncoderInfoProto.pb.h"
#include "CarlifeMusicInitProto.pb.h"
#include "CarlifeBTPairInfoProto.pb.h"
#include "CarlifeCallRecordsListProto.pb.h"
#include "CarlifeCallRecordsProto.pb.h"
#include "CarlifeCarGpsProto.pb.h"
#include "CarlifeCarHardKeyCodeProto.pb.h"
#include "CarlifeCarSpeedProto.pb.h"
#include "CarlifeContactsListProto.pb.h"
#include "CarlifeContactsProto.pb.h"
#include "CarlifeDeviceInfoProto.pb.h"
#include "CarlifeGyroscopeProto.pb.h"
#include "CarlifeMediaInfoListProto.pb.h"
#include "CarlifeMediaInfoProto.pb.h"
#include "CarlifeProtocolVersionProto.pb.h"
#include "CarlifeTouchActionProto.pb.h"
#include "CarlifeTouchEventAllDeviceProto.pb.h"
#include "CarlifeTouchEventDeviceProto.pb.h"
#include "CarlifeTouchEventProto.pb.h"
#include "CarlifeTouchFlingProto.pb.h"
#include "CarlifeTouchScrollProto.pb.h"
#include "CarlifeTouchSinglePointProto.pb.h"
#include "CarlifeTTSInitProto.pb.h"
#include "CarlifeVideoFrameRateProto.pb.h"
#include "CarlifeProtocolVersionMatchStatusProto.pb.h"
#include "CarlifeStatisticsInfoProto.pb.h"
#include "CarlifeModuleStatusProto.pb.h"
#include "CarlifeGearInfoProto.pb.h"
#include "CarlifeVehicleInfoProto.pb.h"
#include "CarlifeVehicleInfoListProto.pb.h"
#include "CarlifeModuleStatusProto.pb.h"
#include "CarlifeModuleStatusListProto.pb.h"
#include "CarlifeNaviNextTurnInfoProto.pb.h"
#include "CarlifeMediaInfoProto.pb.h"
#include "CarlifeMediaProgressBarProto.pb.h"
#include "CarlifeConnectExceptionProto.pb.h"

#include "CarlifeAuthenRequestProto.pb.h"
#include "CarlifeAuthenResponseProto.pb.h"
#include "CarlifeAuthenResultProto.pb.h"
#include "CarlifeBTHfpConnectionProto.pb.h"
#include "CarlifeBTHfpIndicationProto.pb.h"
#include "CarlifeBTHfpRequestProto.pb.h"
#include "CarlifeBTHfpResponseProto.pb.h"
#include "CarlifeBTHfpStatusRequestProto.pb.h"
#include "CarlifeBTHfpStatusResponseProto.pb.h"
#include "CarlifeBTStartPairReqProto.pb.h"
#include "CarlifeNaviAssitantGuideInfoProto.pb.h"
#include "CarlifeSubscribeMobileCarLifeInfoListProto.pb.h"
#include "CarlifeSubscribeMobileCarLifeInfoProto.pb.h"

#include "CarlifeBTIdentifyResultIndProto.pb.h"
#include "CarlifeBTStartIdentifyReqProto.pb.h"
#include "CarlifeErrorCodeProto.pb.h"
#include "CarlifeFeatureConfigListProto.pb.h"
#include "CarlifeFeatureConfigProto.pb.h"
#include "CarlifeOilProto.pb.h"
#include "debug.h"
using namespace std;
using com::baidu::carlife::protobuf::CarlifeAcceleration;
using com::baidu::carlife::protobuf::CarlifeVideoEncoderInfo;
using com::baidu::carlife::protobuf::CarlifeMusicInit;
using com::baidu::carlife::protobuf::CarlifeBTPairInfo;
using com::baidu::carlife::protobuf::CarlifeCallRecordsList;
using com::baidu::carlife::protobuf::CarlifeCallRecords;
using com::baidu::carlife::protobuf::CarlifeCarGps;
using com::baidu::carlife::protobuf::CarlifeCarHardKeyCode;
using com::baidu::carlife::protobuf::CarlifeCarSpeed;
using com::baidu::carlife::protobuf::CarlifeContactsList;
using com::baidu::carlife::protobuf::CarlifeContacts;
using com::baidu::carlife::protobuf::CarlifeDeviceInfo;
using com::baidu::carlife::protobuf::CarlifeGyroscope;
using com::baidu::carlife::protobuf::CarlifeProtocolVersion;
using com::baidu::carlife::protobuf::CarlifeTouchAction;
using com::baidu::carlife::protobuf::CarlifeTouchEventAllDevice;
using com::baidu::carlife::protobuf::CarlifeTouchEventDevice;
using com::baidu::carlife::protobuf::CarlifeTouchEvent;
using com::baidu::carlife::protobuf::CarlifeTouchFling;
using com::baidu::carlife::protobuf::CarlifeTouchScroll;
using com::baidu::carlife::protobuf::CarlifeTouchSinglePoint;
using com::baidu::carlife::protobuf::CarlifeTTSInit;
using com::baidu::carlife::protobuf::CarlifeVideoFrameRate;
using com::baidu::carlife::protobuf::CarlifeProtocolVersionMatchStatus;

using com::baidu::carlife::protobuf::CarlifeStatisticsInfo;
using com::baidu::carlife::protobuf::CarlifeModuleStatus;
using com::baidu::carlife::protobuf::CarlifeGearInfo;
using com::baidu::carlife::protobuf::CarlifeVehicleInfo;
using com::baidu::carlife::protobuf::CarlifeVehicleInfoList;
using com::baidu::carlife::protobuf::CarlifeModuleStatus;
using com::baidu::carlife::protobuf::CarlifeModuleStatusList;
using com::baidu::carlife::protobuf::CarlifeNaviNextTurnInfo;
using com::baidu::carlife::protobuf::CarlifeMediaInfo;
using com::baidu::carlife::protobuf::CarlifeMediaProgressBar;
using com::baidu::carlife::protobuf::CarlifeConnectException;

using com::baidu::carlife::protobuf::CarlifeBTHfpRequest;
using com::baidu::carlife::protobuf::CarlifeBTHfpIndication;
using com::baidu::carlife::protobuf::CarlifeBTHfpConnection;
using com::baidu::carlife::protobuf::CarlifeSubscribeMobileCarLifeInfo;
using com::baidu::carlife::protobuf::CarlifeSubscribeMobileCarLifeInfoList;
using com::baidu::carlife::protobuf::CarlifeNaviAssitantGuideInfo;
using com::baidu::carlife::protobuf::CarlifeAuthenRequest;
using com::baidu::carlife::protobuf::CarlifeAuthenResponse;
using com::baidu::carlife::protobuf::CarlifeAuthenResult;
using com::baidu::carlife::protobuf::CarlifeBTStartPairReq;
using com::baidu::carlife::protobuf::CarlifeBTHfpResponse;
using com::baidu::carlife::protobuf::CarlifeBTHfpStatusRequest;
using com::baidu::carlife::protobuf::CarlifeBTHfpStatusResponse;

using com::baidu::carlife::protobuf::CarlifeBTIdentifyResultInd;
using com::baidu::carlife::protobuf::CarlifeBTStartIdentifyReq;
using com::baidu::carlife::protobuf::CarlifeErrorCode;
using com::baidu::carlife::protobuf::CarlifeFeatureConfigList;
using com::baidu::carlife::protobuf::CarlifeFeatureConfig;
using com::baidu::carlife::protobuf::CarlifeOil;

void * packageDataAnalysis( PackageHeadType type, const uint8_t * data, uint32_t len) 
{
//	g_message("%s--->len=%d", __func__, len);
//	DEBUG_HEX_DISPLAY( data, len);
	void * _res	=	NULL;
	switch( type )
	{
		case IOS_CMD_MD_INFO:
		{
			S_MD_INFO_t * _info	=	g_new0( S_MD_INFO_t, 1);
			CarlifeDeviceInfo deviceInfo;
			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);
			deviceInfo.MergePartialFromCodedStream(&input);
			_info->os	=	g_strdup(deviceInfo.os().c_str());
			_info->board	=	g_strdup(deviceInfo.board().c_str());
			_info->bootloader =	g_strdup(deviceInfo.bootloader().c_str());
			_info->brand=	g_strdup(deviceInfo.brand().c_str());
			_info->cpu_abi=	g_strdup(deviceInfo.cpu_abi().c_str());
			_info->cpu_abi2 =	g_strdup(deviceInfo.cpu_abi2().c_str());
			_info->device =	g_strdup(deviceInfo.device().c_str());
			_info->display =	g_strdup(deviceInfo.display().c_str());
			_info->fingerprint =	g_strdup(deviceInfo.fingerprint().c_str());
			_info->hardware =	g_strdup(deviceInfo.hardware().c_str());
			_info->host =	g_strdup(deviceInfo.host().c_str());
			_info->cid =	g_strdup(deviceInfo.cid().c_str());
			_info->manufacturer =	g_strdup(deviceInfo.manufacturer().c_str());
			_info->model =	g_strdup(deviceInfo.model().c_str());
			_info->product =	g_strdup(deviceInfo.product().c_str());
			_info->serial=	g_strdup(deviceInfo.serial().c_str());
			_info->codename =	g_strdup(deviceInfo.codename().c_str());
			_info->incremental=	g_strdup(deviceInfo.incremental().c_str());
			_info->release =	g_strdup(deviceInfo.release().c_str());
			_info->sdk =	g_strdup(deviceInfo.sdk().c_str());
			_info->sdk_int	=	deviceInfo.sdk_int();
			_info->token=	g_strdup(deviceInfo.token().c_str());
			_info->btaddress=	g_strdup(deviceInfo.btaddress().c_str());
			_res	=	(void *)_info;

		}
		break;

		case IOS_CMD_PROTOCOL_VERSION_MATCH_STATUS:
		{
			S_PROTOCOL_VERSION_MATCH_SATUS_t * _status	=	g_new0( S_PROTOCOL_VERSION_MATCH_SATUS_t, 1);

			CarlifeProtocolVersionMatchStatus protocolVersionMatchStatus;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);

			protocolVersionMatchStatus.MergePartialFromCodedStream(&input);

			_status->matchStatus=protocolVersionMatchStatus.matchstatus();

			_res	=	(void *)_status;		

		}
		break;
		case IOS_CMD_MODULE_STATUS:
		{
			
			S_MODULE_STATUS_LIST_MOBILE_t * _mobileList	=	g_new0( S_MODULE_STATUS_LIST_MOBILE_t, 1);


			CarlifeModuleStatusList carlifeModuleStatusList;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);

			carlifeModuleStatusList.MergePartialFromCodedStream(&input);

			_mobileList->cnt	=	carlifeModuleStatusList.cnt();

			_mobileList->moduleStatus	=	g_new0( S_MODULE_STATUS_MOBILE_t, _mobileList->cnt);


			int i	=	0;

			g_message("%s--->_mobileList->cnt:%d", __func__, _mobileList->cnt);
			for(i = 0 ; i < _mobileList->cnt ; i++)
			{
				_mobileList->moduleStatus[i].moduleID	=	carlifeModuleStatusList.modulestatus(i).moduleid();
				_mobileList->moduleStatus[i].statusID	=	carlifeModuleStatusList.modulestatus(i).statusid();

			}
			
			_res	=	(void *)_mobileList;
		}
		break;
		case IOS_CMD_MD_AUTHEN_RESULT:
		{
			S_MD_AUTHEN_RESULT_t * _result	=	g_new0( S_MD_AUTHEN_RESULT_t, 1);

			CarlifeAuthenResult carlifeAuthenResult;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);

			carlifeAuthenResult.MergePartialFromCodedStream(&input);

			_result->authenResult=carlifeAuthenResult.authenresult();

			_res	=	(void *)_result;

		}
		break;
		case IOS_CMD_VIDEO_ENCODER_INIT_DONE:
		{
			S_VIDEO_ENCODER_INIT_DONE_t *	_result	=	g_new0( S_VIDEO_ENCODER_INIT_DONE_t, 1);

			CarlifeVideoEncoderInfo carLifeVideoEncoderInfo;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);

			carLifeVideoEncoderInfo.MergePartialFromCodedStream(&input);
			
			_result->width=carLifeVideoEncoderInfo.width();
			_result->height=carLifeVideoEncoderInfo.height();
			_result->frameRate=carLifeVideoEncoderInfo.framerate();

			_res	=	(void *)_result;


		}
		break;
		case  IOS_CMD_MEDIA_INFO:
		{
			S_MEDIA_INFO_t * mediaInfo	=	g_new0( S_MEDIA_INFO_t, 1);

			CarlifeMediaInfo carlifeMediaInfo;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);
			carlifeMediaInfo.MergePartialFromCodedStream(&input);


			mediaInfo->source	=	g_strdup( carlifeMediaInfo.source().c_str());
			mediaInfo->song	=	g_strdup( carlifeMediaInfo.song().c_str() );
			mediaInfo->artist	=	g_strdup( carlifeMediaInfo.artist().c_str() );
			mediaInfo->album	=	g_strdup( carlifeMediaInfo.album().c_str() );
			mediaInfo->albumArt	=	g_strdup( carlifeMediaInfo.albumart().c_str() );
			mediaInfo->duration	=	carlifeMediaInfo.duration();
			mediaInfo->playlistNum	=	carlifeMediaInfo.playlistnum();
			mediaInfo->songId	=	g_strdup( carlifeMediaInfo.songid().c_str());
			mediaInfo->mode	=	carlifeMediaInfo.mode();
			
			_res	=	(void *) mediaInfo;


		}
		break;
		case IOS_MEDIA_INIT:
		{
			S_AUDIO_INIT_PARAMETER_t * audioInitParameter	=	g_new0( S_AUDIO_INIT_PARAMETER_t, 1);

			CarlifeMusicInit carLifeMusicInit;
			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);
			carLifeMusicInit.MergePartialFromCodedStream(&input);

			audioInitParameter->sampleRate=carLifeMusicInit.samplerate();
			audioInitParameter->channelConfig=carLifeMusicInit.channelconfig();
			audioInitParameter->sampleFormat=carLifeMusicInit.sampleformat();

			_res	=	(void *) audioInitParameter;



		}
		break;
		case IOS_CMD_MEDIA_PROGRESS_BAR:
		{
			S_MEDIA_PROGRESS_BAR_t * mediaProgressBar	=	g_new0( S_MEDIA_PROGRESS_BAR_t, 1);

			CarlifeMediaProgressBar carlifeMediaProgressBar;

			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);

			carlifeMediaProgressBar.MergePartialFromCodedStream(&input);


			mediaProgressBar->progressBar=carlifeMediaProgressBar.progressbar();
			 _res	=	(void *)mediaProgressBar;


		}
		break;
		case IOS_VR_INIT:
		{
			S_AUDIO_INIT_PARAMETER_t * vrInitParameter	=	g_new0( S_AUDIO_INIT_PARAMETER_t, 1);

			CarlifeMusicInit carLifeMusicInit;
			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);
			carLifeMusicInit.MergePartialFromCodedStream(&input);

			vrInitParameter->sampleRate=carLifeMusicInit.samplerate();
			vrInitParameter->channelConfig=carLifeMusicInit.channelconfig();
			vrInitParameter->sampleFormat=carLifeMusicInit.sampleformat();

			_res	=	(void *) vrInitParameter;

		}
		break;

		case IOS_TTS_INIT:
		{
			S_AUDIO_INIT_PARAMETER_t * ttsInit	=	g_new0( S_AUDIO_INIT_PARAMETER_t, 1);

			CarlifeMusicInit carLifeMusicInit;
			::google::protobuf::io::CodedInputStream input((const ::google::protobuf::uint8*)data, len);
			carLifeMusicInit.MergePartialFromCodedStream(&input);

			ttsInit->sampleRate=carLifeMusicInit.samplerate();
			ttsInit->channelConfig=carLifeMusicInit.channelconfig();
			ttsInit->sampleFormat=carLifeMusicInit.sampleformat();
			_res	=	( void *)ttsInit;

		}
		break;
	}
	return _res;
}
static void cmdPackageHead( ChannelType channel, PackageHeadType type,uint8_t * data, uint32_t pbLen, uint32_t msgLen)
{
	if( channel == IOS_CMD_CHANNEL)
	{
		data[0]	=	0x00;
		data[1]	=	0x00;
		data[2]	=	0x00;
		data[3]	=	0x01;
	}
	else if( channel	==	IOS_CTRL_CHANNEL)
	{
		data[0]	=	0x00;
		data[1]	=	0x00;
		data[2]	=	0x00;
		data[3]	=	0x06;

	}
	else
	{
		g_message("%s->channel %d is not IOS_CMD_CHANNEL or IOS_CTRL_CHANNEL", __func__, channel);
		return;
	}
	data[4] = ( uint8_t ) ((msgLen >> 24) & 0xff);
	data[5] = ( uint8_t ) ((msgLen >> 16) & 0xff);
	data[6] = ( uint8_t ) ((msgLen >> 8) & 0xff);
	data[7] = ( uint8_t ) (msgLen & 0xff);

	data[8]	=	(uint8_t ) ((pbLen >>8)& 0xff);
	data[9]	=	(uint8_t ) (pbLen & 0xff);

//reserved is 2 bytes 12~13
	data[10] =	0x00;
	data[11] =	0x00;

	data[12] = ( uint8_t ) ((type >> 24) & 0xff);
	data[13] = ( uint8_t ) ((type >> 16) & 0xff);
	data[14]	=	( uint8_t ) ((type >> 8) & 0xff);
	data[15] = ( uint8_t ) (type & 0xff);



}
void ios_sendCmdVideoEncoderInit( S_VIDEO_ENCODER_INIT_t * initParam, uint8_t * data, uint32_t *len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeVideoEncoderInfo VideoEncoderInfo;
	VideoEncoderInfo.set_width(initParam->width);
	VideoEncoderInfo.set_height(initParam->height);
	VideoEncoderInfo.set_framerate(initParam->frameRate);

	_pbLen	=	VideoEncoderInfo.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_VIDEO_ENCODER_INIT, data, _pbLen, _msgLen);

	VideoEncoderInfo.SerializeWithCachedSizesToArray( (uint8_t *)(data + 16));

	*len	=	_msgLen	+ 8;

}

void ios_sendCmdHUProtoclVersion( S_HU_PROTOCOL_VERSION_t* version, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen,  _msgLen;
	CarlifeProtocolVersion protocolVersion;

	protocolVersion.set_majorversion(version->majorVersion);
	protocolVersion.set_minorversion(version->minorVersion);

	_pbLen	=	protocolVersion.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_HU_PROTOCOL_VERSION, data, _pbLen, _msgLen);
//service type 4 bytes 14~17

	//set package data filed
	protocolVersion.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));
	
	*len	=	_msgLen + 8;

}
void ios_sendCmdStatisticInfo(S_STATISTICS_INFO_t * info, uint8_t * data, uint32_t *len) 
{
	uint32_t _pbLen,  _msgLen;
	CarlifeStatisticsInfo carlifeStatisticsInfo;

	carlifeStatisticsInfo.set_cuid(info->cuid);
	carlifeStatisticsInfo.set_versionname(info->versionName);
	carlifeStatisticsInfo.set_versioncode(info->versionCode);
	carlifeStatisticsInfo.set_channel(info->channel);
	carlifeStatisticsInfo.set_connectcount(info->connectCount);
	carlifeStatisticsInfo.set_connectsuccesscount(info->connectSuccessCount);
	carlifeStatisticsInfo.set_connecttime(info->connectTime);
	carlifeStatisticsInfo.set_crashlog(info->crashLog);

	_pbLen	=	carlifeStatisticsInfo.ByteSize();

	_msgLen	=	_pbLen + 8;


	cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_STATISTIC_INFO, data, _pbLen, _msgLen);

	carlifeStatisticsInfo.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));
	
	*len	=	_msgLen + 8;

}

static void mediaPackageHead( ChannelType channel, PackageHeadType type,uint8_t * data, uint32_t pbLen, uint32_t msgLen, uint32_t timestamp)
{
	switch( channel)
	{
		case IOS_VIDEO_CHANNEL:
			{
				data[0]	=	0x00;
				data[1]	=	0x00;
				data[2]	=	0x00;
				data[3]	=	0x02;

			}
			break;
		case IOS_MEDIA_CHANNEL:
			{
				data[0]	=	0x00;
				data[1]	=	0x00;
				data[2]	=	0x00;
				data[3]	=	0x03;

			}
			break;
		case IOS_TTS_CHANNEL:
			{
				data[0]	=	0x00;
				data[1]	=	0x00;
				data[2]	=	0x00;
				data[3]	=	0x04;

			}
			break;
		case IOS_VR_CHANNEL:
			{
				data[0]	=	0x00;
				data[1]	=	0x00;
				data[2]	=	0x00;
				data[3]	=	0x05;

			}
			break;
		default:
			g_message("%s->channel %d is not IOS_VIDEO_CHANNEL ,IOS_MEDIA_CHANNEL,IOS_TTS_CHANNEL,\
				IOS_VR_CHANNEL", __func__, channel);
			break;
	}
	data[4] = ( uint8_t ) ((msgLen >> 24) & 0xff);
	data[5] = ( uint8_t ) ((msgLen >> 16) & 0xff);
	data[6] = ( uint8_t ) ((msgLen >> 8) & 0xff);
	data[7] = ( uint8_t ) (msgLen & 0xff);

	data[8] = ( uint8_t ) ((pbLen >> 24) & 0xff);
	data[9] = ( uint8_t ) ((pbLen >> 16) & 0xff);
	data[10] = ( uint8_t ) ((pbLen >> 8) & 0xff);
	data[11] = ( uint8_t ) (pbLen & 0xff);

	data[12] = ( uint8_t ) ((timestamp >> 24) & 0xff);
	data[13] = ( uint8_t ) ((timestamp >> 16) & 0xff);
	data[14] = ( uint8_t ) ((timestamp >> 8) & 0xff);
	data[15] = ( uint8_t ) (timestamp & 0xff);

	data[16] = ( uint8_t ) ((type >> 24) & 0xff);
	data[17] = ( uint8_t ) ((type >> 16) & 0xff);
	data[18] = ( uint8_t ) ((type >> 8) & 0xff);
	data[19] = ( uint8_t ) (type & 0xff);


}
void ios_sendCmdVideoHeartbeat( uint8_t * data, uint32_t *len,uint32_t timestamp )
{
	uint32_t _pbLen, _msgLen;

	_pbLen	=	0;
	_msgLen	=	12;

	mediaPackageHead(IOS_VIDEO_CHANNEL, IOS_VIDEO_HEARTBEAT, data, _pbLen, _msgLen, timestamp); 

	* len	=	_msgLen + 8;

	
}

void ios_sendCmdFeatureConfigResponse( S_FEATURE_CONFIG_LIST_t * list, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen,  _msgLen;
	int i = 0;

	CarlifeFeatureConfig *carlifeFeatureConfig;

	CarlifeFeatureConfigList carlifeFeatureConfigList;

	carlifeFeatureConfigList.set_cnt(list->cnt);

	S_FEATURE_CONFIG_t *pFeatureConfig=list->pFeatureConfig;	

	for( i; i<list->cnt; i++){
		carlifeFeatureConfig=carlifeFeatureConfigList.add_featureconfig();

		carlifeFeatureConfig->set_key(pFeatureConfig->key);

		carlifeFeatureConfig->set_value(pFeatureConfig->value);

		pFeatureConfig=pFeatureConfig->pNext;
	}

	_pbLen	=	carlifeFeatureConfig->ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CMD_CHANNEL,IOS_CMD_HU_FEATURE_CONFIG_RESPONSE, data, _pbLen, _msgLen);

	carlifeFeatureConfigList.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	* len	=	_msgLen +	8;

}

void ios_sendCmdVideoEncoderStart(uint8_t * data, uint32_t *len)
{
	uint32_t _pbLen, _msgLen;

	_pbLen	=	0;

	_msgLen	=	8;

	cmdPackageHead(IOS_CMD_CHANNEL, IOS_CMD_VIDEO_ENCODER_START, data, _pbLen, _msgLen);

	* len	=	_msgLen + 8;
}
void ios_sendCmdVideoEncoderPause(uint8_t * data, uint32_t *len)
{
	uint32_t _pbLen, _msgLen;

	_pbLen	=	0;

	_msgLen	=	8;

	cmdPackageHead(IOS_CMD_CHANNEL, IOS_CMD_VIDEO_ENCODER_PAUSE, data, _pbLen, _msgLen);

	* len	=	_msgLen + 8;
}
void ios_sendCmdVideoEncoderReset(uint8_t * data, uint32_t *len)
{
	uint32_t _pbLen, _msgLen;

	_pbLen	=	0;

	_msgLen	=	8;

	cmdPackageHead(IOS_CMD_CHANNEL, IOS_CMD_VIDEO_ENCODER_RESET, data, _pbLen, _msgLen);

	* len	=	_msgLen + 8;
}

void ios_sendTouch(MHTouchType type, uint32_t x, uint32_t y, uint8_t *data, uint32_t *len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeTouchSinglePoint action;

	action.set_x( x );
	action.set_y( y );

	_pbLen	=	action.ByteSize();

	_msgLen	=	_pbLen + 8;


	switch( type)
	{
		case TOUCH_DOWN:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_ACTION_DOWN, data, _pbLen, _msgLen);

			break;

		case TOUCH_UP:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_ACTION_UP, data, _pbLen, _msgLen);

			break;

		case TOUCH_MOVE:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_ACTION_MOVE, data, _pbLen, _msgLen); 

			break;
		case TOUCH_SIGNAL_CLICK:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_SINGLE_CLICK, data, _pbLen, _msgLen); 

			break;

		case TOUCH_DOUBLE_CLICK:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_DOUBLE_CLICK, data, _pbLen, _msgLen); 

			break;

		case TOUCH_LONG_PRESS:

			cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_LONG_PRESS, data, _pbLen, _msgLen); 

			break;

		default:
			g_message("invalid press type");
			break;
			
	}

	action.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	* len	=	_msgLen + 8;
	
}

void ios_sendCmdLaunchMode( MHCarlifeLaunchMode mode, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	_pbLen	=	0;

	_msgLen	=	8;

	switch(mode)
	{
		case CARLIFE_LAUNCH_NORMAL:
			cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_LAUNCH_MODE_NORMAL, data, _pbLen, _msgLen);
			break;

		case CARLIFE_LAUNCH_PHONE:
			cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_LAUNCH_MODE_PHONE, data, _pbLen, _msgLen);
			break;

		case CARLIFE_LAUNCH_MAP:
			cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_LAUNCH_MODE_MAP, data, _pbLen, _msgLen);
			break;

		case CARLIFE_LAUNCH_MUSIC:
			cmdPackageHead( IOS_CMD_CHANNEL, IOS_CMD_LAUNCH_MODE_MUSIC, data, _pbLen, _msgLen);
			break;
	}

	* len	=	_msgLen + 8;
}

void ios_sendCmdHardkey( S_TOUCH_CAR_HARD_KEY_CODE_t * key, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeCarHardKeyCode hardKey;

	hardKey.set_keycode(key->keycode);
	
	_pbLen	=	hardKey.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_TOUCH_CAR_HARD_KEY_CODE, data, _pbLen, _msgLen);


	//set package data filed
	hardKey.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;

}

uint32_t ios_sendVRRecordData(  uint8_t * data, uint32_t pbLen, uint32_t timestamp) 
{
	mediaPackageHead( IOS_VR_CHANNEL, IOS_VR_MIC_DATA, data, pbLen, pbLen + VR_HEAD_LEN, timestamp);
	return 20; 
	
}
void ios_sendCarVelocity(S_CAR_VELOCITY_t * velocity, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeCarSpeed carSpeed;

	carSpeed.set_speed(velocity->speed);

	carSpeed.set_timestamp(velocity->timeStamp);

	_pbLen	=	carSpeed.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_CAR_VELOCITY, data, _pbLen, _msgLen);

	carSpeed.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;
}

void ios_sendCarGPS(S_CAR_GPS_t * gps, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeCarGps carGps;
	carGps.set_antennastate(gps->antennaState);
	carGps.set_signalquality(gps->signalQuality);
	carGps.set_latitude(gps->latitude);
	carGps.set_longitude(gps->longitude);
	carGps.set_height(gps->height);
	carGps.set_speed(gps->speed);
	carGps.set_heading(gps->heading);
	carGps.set_year(gps->year);
	carGps.set_month(gps->month);
	carGps.set_day(gps->day);
	carGps.set_hrs(gps->hrs);
	carGps.set_min(gps->min);
	carGps.set_sec(gps->sec);
	carGps.set_fix(gps->fix);
	carGps.set_hdop(gps->hdop);
	carGps.set_pdop(gps->pdop);
	carGps.set_vdop(gps->vdop);
	carGps.set_satsused(gps->satsUsed);
	carGps.set_satsvisible(gps->satsVisible);
	carGps.set_horposerror(gps->horPosError);
	carGps.set_vertposerror(gps->vertPosError);
	carGps.set_northspeed(gps->northSpeed);
	carGps.set_eastspeed(gps->eastSpeed);
	carGps.set_vertspeed(gps->vertSpeed);
	carGps.set_timestamp(gps->timeStamp);


	_pbLen	=	carGps.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_CAR_GPS, data, _pbLen, _msgLen);

	carGps.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;

}
void ios_sendCarGyroscope(S_CAR_GYROSCOPE_t * cyro, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeGyroscope cyroScope;

	cyroScope.set_gyrotype(cyro->gyroType);
	cyroScope.set_gyrox(cyro->gyroX);
	cyroScope.set_gyroy(cyro->gyroY);
	cyroScope.set_gyroz(cyro->gyroZ);
	cyroScope.set_timestamp(cyro->timeStamp);

	_pbLen	=	cyroScope.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_CAR_GYROSCOPE, data, _pbLen, _msgLen);

	cyroScope.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;

}

void ios_sendCarAcceleration(S_CAR_ACCELERATION_t * acceleration, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeAcceleration carlifeAcceleration;

	carlifeAcceleration.set_accx(acceleration->accX);
	carlifeAcceleration.set_accy(acceleration->accY);
	carlifeAcceleration.set_accz(acceleration->accZ);
	carlifeAcceleration.set_timestamp(acceleration->timeStamp);

	_pbLen	=	carlifeAcceleration.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_CAR_ACCELERATION, data, _pbLen, _msgLen);

	carlifeAcceleration.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;



}
void ios_sendCarOil(S_CAR_OIL_t * oil, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeOil carlifeOil;

	carlifeOil.set_level(oil->level);
	carlifeOil.set_range(oil->range);
	carlifeOil.set_lowfulewarning(oil->lowFullWarning);

	_pbLen	=	carlifeOil.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_CAR_OIL, data, _pbLen, _msgLen);

	carlifeOil.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;


}

void ios_sendModuleControl(S_MODULE_STATUS_CONTROL_t * status, uint8_t * data, uint32_t * len)
{
	uint32_t _pbLen, _msgLen;

	CarlifeModuleStatus carlifeModuleStatus;

	carlifeModuleStatus.set_moduleid(status->moduleID);
	carlifeModuleStatus.set_statusid(status->statusID);

	_pbLen	=	carlifeModuleStatus.ByteSize();

	_msgLen	=	_pbLen + 8;

	cmdPackageHead( IOS_CTRL_CHANNEL, IOS_CMD_MODULE_CONTROL, data, _pbLen, _msgLen);

	carlifeModuleStatus.SerializeWithCachedSizesToArray((uint8_t *)(data + 16));

	*len	=	_msgLen + 8;

}
