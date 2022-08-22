/*
 * =====================================================================================
 *
 *       Filename:  sample.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/17/2014 03:44:32 PM
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
#include <mh_api.h>
#include <glib.h>
#include <unistd.h>
#include <glib.h>
//#include <ilm/ilm_control.h>
//#include <ilm_common.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libudev.h>
#include<mh_carlife.h>
//#include<anwmanagerfun.h>
#include <string.h>
//#include <mh_extern_misc.h>
#include "app_rpc_if.h"

#define DEBUG_SWITCH 1
#if DEBUG_SWITCH
#define dbg_printf(format,...) \
do{\
	printf("[%s : %d][%s]-----"format, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
}while(0)
#else
#define dbg_printf(format,...)
#endif

app_rpc_handle_t *app_source_rpc_handle;
//
pthread_t _touch = 0;
char * BTAddress;
//dd:87:17:17:d4:67
char * BTName	=	"TestName";
MHDev * global_dev ;
static GMainLoop *test_loop;

MHDevStatusListener * _carlife_status_listener;
ModuleStatusListener * _carlife_module_status;
MHCarlifeBtHfpRequestListener *	_carlife_bt_hfp_request;	
//t_ilm_uint layer;
//t_ilm_uint screen;
static int number_of_surfaces;
int tmpx = 0;
//t_ilm_uint screenWidth;
//t_ilm_uint screenHeight;
static char * md_bt_address;
static char * md_bt_passkey;
static bool androidFlag;
static void * ios_touch_task( void * user_data );
static void * android_touch_task( void * user_data);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_callback(MHDev * self, MHDevStatus status, void * user_data)
{
	g_message("%s---->%p", __func__, self);
	switch( status )
	{
		case CARLIFE_ROLE_SWITCH:
			dbg_printf("\n\n\n----------%d------------------------->ios persent 10%\n\n", CARLIFE_ROLE_SWITCH);
			break;
		case CARLIFE_IAP_AUTH:
			dbg_printf("\n\n\n----------%d------------------------->ios persent 30%\n\n", CARLIFE_IAP_AUTH);
			break;
		case CARLIFE_IAP_AUTH_SUCCESS:
			dbg_printf("\n\n\n-----------%d------------------------>ios persent 60%\n\n", CARLIFE_IAP_AUTH_SUCCESS);
		mh_dev_start(global_dev);
			break;
		case CARLIFE_DEVICE_OPEN:
			dbg_printf("\n\n\n----------------------------------->ios persent 80%\n\n");
			break;

		case 	CARLIFE_GET_MD_VERSION:
			dbg_printf("receive CARLIFE_GET_MD_VERSION\n");
			dbg_printf("\n\n*****************************persent %30\n\n");
			break;
		case 	CARLIFE_RUN_BDIM:
			dbg_printf("receive CARLIFE_RUN_BDIM\n");
			dbg_printf("\n\n*****************************persent %50\n\n");
			break;
		case 	CARLIFE_LAUNCH_APP:
			dbg_printf("receive CARLIFE_LAUNCH_APP\n");
			dbg_printf("\n\n*****************************persent %60\n\n");
			break;
		case	CARLIFE_CONNECT_SUCCESS:
			if( androidFlag == 1)
			{
				dbg_printf("receive CARLIFE_CONNECT_SUCCESS\n");
				dbg_printf("\n\n*****************************persent %100\n\n");
			}
			else
			{
				dbg_printf("\n\n\n----------------------------------->ios persent 100%\n\n");
			}


//			mh_dev_module_control(self, PHONE_MODULE, PHONE_STATUS_IDLE);
//			mh_dev_module_control(self, PHONE_MODULE, PHONE_STATUS_INCOMING);
//			mh_dev_module_control(self, PHONE_MODULE, PHONE_STATUS_OUTING);
//
//			mh_dev_module_control(self, NAVI_MODULE, NAVI_STATUS_IDLE);
//			mh_dev_module_control(self, NAVI_MODULE, NAVI_STATUS_RUNNING);
//
//			mh_dev_module_control(self, MUSIC_MODULE, MUSIC_STATUS_IDLE);
//			mh_dev_module_control(self, MUSIC_MODULE, MUSIC_STATUS_RUNNING);
//
//			mh_dev_module_control(self, VR_MODULE, VR_STATUS_RECORD_IDLE);
//			mh_dev_module_control(self, VR_MODULE, VR_STATUS_RECORD_RUNNING);
//			mh_dev_module_control(self, VR_MODULE, VR_STATUS_NOT_SUPPORT);
//
//			mh_dev_module_control(self, CONNECT_MODULE, CONNECT_STATUS_ADB);
//			mh_dev_module_control(self, CONNECT_MODULE, CONNECT_STATUS_AOA);
//			mh_dev_module_control(self, CONNECT_MODULE, CONNECT_STATUS_NCM_ANDROID);
//			mh_dev_module_control(self, CONNECT_MODULE, CONNECT_STATUS_NCM_IOS);
//			mh_dev_module_control(self, CONNECT_MODULE, CONNECT_STATUS_WIFI);
//
//			mh_dev_module_control(self, MIC_MODULE, MIC_STATUS_USE_VEHICLE_MIC);		
//			mh_dev_module_control(self, MIC_MODULE, MIC_STATUS_USE_MOBILE_MIC);		
//			mh_dev_module_control(self, MIC_MODULE, MIC_STATUS_NOT_SUPPORT);		
			break;
		case 	CARLIFE_VIDEO_INIT_DONE:
			dbg_printf("receive CARLIFE_VIDEO_INIT_DONE\n");
			MHCarlifeCarInfo * car_info	=	g_new0( MHCarlifeCarInfo, 1);
			car_info->os	=	g_strdup("linux");
			car_info->manufacturer	=	g_strdup("HYUNDAI");
			car_info->btaddress	=	g_strdup(BTAddress);
			//			_info->board	=	g_strdup("aaa");
			//			_info->bootloader	=	g_strdup("aaa");
			car_info->brand	=	g_strdup("HYUNDAI");
			//			_info->cpu_abi	=	g_strdup("aaa");
			//			_info->cpu_abi2	=	g_strdup("aaa");
			//			_info->device	=	g_strdup("aaa");
			car_info->display	=	g_strdup("HYUNDAI");
			car_info->fingerprint	=	g_strdup("HYUNDAI");
			//
			//			_info->hardware	=	g_strdup("aaa");
			//			_info->host	=	g_strdup("aaa");
			//			_info->cid	=	g_strdup("aaa");
			//			_info->model	=	g_strdup("aaa");
			//			_info->serial	=	g_strdup("aaa");
			//
			//			_info->codename	=	g_strdup("aaa");
			//			_info->incremental	=	g_strdup("aaa");
			//			_info->release	=	g_strdup("aaa");
			//			_info->sdk	=	g_strdup("aaa");
			car_info->token	=	g_strdup("HYUNDAI");

			mh_dev_carlife_car_info( self, car_info);
			mh_dev_carlife_video_start( self);

			break;
		case 	CARLIFE_FOREGROUND:
			dbg_printf("receive CARLIFE_FOREGROUND\n");
			break;
		case	CARLIFE_BACKGROUND:
			dbg_printf("receive CARLIFE_BACKGROUND\n");
			break;
		case 	CARLIFE_SCREEN_ON:
			dbg_printf("receive CARLIFE_SCREEN_ON\n");
			break;
		case	CARLIFE_SCREEN_OFF:
			dbg_printf("receive CARLIFE_SCREE_OFF\n");
			break;
		case 	CARLIFE_USER_PRESENT:
			dbg_printf("receive CARLIFE_USER_PRESENT\n");
			break;
		case 	CARLIFE_APP_EXIT:
			dbg_printf("\n\n\nreceive CARLIFE_APP_EXIT\n\n\n");
			mh_dev_start(self);
			break;
		case CARLIFE_GOTO_DESKTOP:
			dbg_printf("receive CARLIFE_GOTO_DESKTOP\n");
			mh_dev_carlife_video_pause( self);
			break;
		case CARLIFE_MIC_RECORD_WAKEUP_START:
			dbg_printf("receive CARLIFE_MIC_RECORD_WAKEUP_START\n");
			break;
		case CARLIFE_MIC_RECORD_END:
			dbg_printf("receive CARLIFE_MIC_RECORD_END\n");
			break;
		case CARLIFE_MIC_RECORD_RECOG_START:
			dbg_printf("receive CARLIFE_MIC_RECORD_RECOG_START\n");
			break;
		case CARLIFE_FEATURE_CONFIG_DONE:
			{
				if( androidFlag	== 1)
				{
				dbg_printf("receive CARLIFE_FEATURE_CONFIG_DONE\n");
				MHCarlifeBtStartIdentifyReq * _req	=	g_new0( MHCarlifeBtStartIdentifyReq,  1);
				_req->address	=	g_strdup(BTAddress);
				dbg_printf("***************************BTAddress:%s\n", BTAddress);
				mh_dev_carlife_bt_start_identify_req( global_dev, _req);
				}
				else
				{
					dbg_printf("ios don't need bt identity\n");
				}
			}
			break;

		case CARLIFE_START_BT_AUTOPAIR:
			//app need disconect existing HFP connections and auto pairing
			{
				dbg_printf("receive CARLIFE_START_BT_AUTOPAIR\n");
				MHCarlifeBtPairInfo * _info	=	g_new0( MHCarlifeBtPairInfo, 1);
				_info->address	=	g_strdup( BTAddress);
				_info->passKey	=	g_strdup("");
				_info->hash	=	g_strdup("");
				_info->randomizer	=	g_strdup("");
				_info->uuid	=	g_strdup("");
				_info->name	=	g_strdup(BTName);
				_info->status	=	0;
				mh_dev_carlife_send_bt_pair_info( global_dev, _info);
			}
			break;
		case CARLIFE_BT_IDENTIFY_SUCCESS:
			dbg_printf("CARLIFE_BT_IDENTIFY_SUCCESS\n");
			//app need enable its connect option for bluetooth 
			//or unblock its native telephone app on reception of this message

			break;
		case CARLIFE_BT_IDENTIFY_FAILED:
			//app need enable its connect option for bluetooth 
			//or unblock its native telephone app on reception of this message
			dbg_printf("CARLIFE_BT_IDENTIFY_FAILED\n");
			//			{
			//				MHCarlifeBtPairInfo * _info	=	g_new0( MHCarlifeBtPairInfo, 1);
			//				_info->address	=	g_strdup(BTAddress);
			////	_info->passKey	=	g_strdup("");
			////				_info->hash	=	g_strdup("");
			//				_info->randomizer	=	g_strdup("");
			//				_info->uuid	=	g_strdup("");
			//				_info->name	=	g_strdup("");
			//				_info->status	=	2;
			//				mh_dev_carlife_send_bt_pair_info( global_dev, _info);

			//			}

			break;
		case CARLIFE_BT_RECV_MD_INFO:
			mh_object_get_properties((MHObject *)self, "md_bt_address", &md_bt_address, 
					"md_bt_passkey", &md_bt_passkey, NULL);
			dbg_printf("get md_bt_address:%s, md_bt_passkey:%s\n", md_bt_address, md_bt_passkey);
			MHCarlifeBtPairInfo * _info	=	g_new0( MHCarlifeBtPairInfo, 1);
			_info->address	=	g_strdup( BTAddress);
			_info->passKey	=	g_strdup("");
			_info->hash	=	g_strdup("");
			_info->randomizer	=	g_strdup("");
			_info->uuid	=	g_strdup("");
			_info->name	=	g_strdup(BTName);
			_info->status	=	1;
			mh_dev_carlife_send_bt_pair_info( global_dev, _info);
			break;
//		case CARLIFE_IAP_AUTH_SUCCESS:
//			dbg_printf("receive CARLIFE_IAP_AUTH_SUCCESS \n");
//			mh_dev_start(global_dev);
//			break;
//
		case CARLIFE_UI_ACTION_SOUND:
			dbg_printf("Play the key tone\n\n");
			break;
		case CARLIFE_RECV_MD_INFO:
			mh_object_get_properties((MHObject *)self, "md_bt_address", &md_bt_address, 
					"md_bt_passkey", &md_bt_passkey, NULL);
			dbg_printf("CARLIFE_RECV_MD_INFO--->get md_bt_address:%s, md_bt_passkey:%s\n", md_bt_address, md_bt_passkey);
			break;

		case CARLIFE_VR_START:
			dbg_printf("switch the channel from other to VR tts\n");
			break;
		case CARLIFE_VR_STOP:
			dbg_printf("switch the channel from VR tts to other\n");
			break;
		default:
			dbg_printf("receive default\n");
			break;
	}
}		/* -----  end of static function _dev_status_callback  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _detach_event
 *  Description:  
 * =====================================================================================
 */
static void _detach_event(	MHDev * dev, void * user_data)
{
	dbg_printf("sample---------------------------------------->_detach_event\n");
	global_dev	=	dev;
	if( _carlife_status_listener->user_data	==	dev)
	{
		g_free( _carlife_status_listener);
	}
	if( _carlife_module_status->user_data == dev);
	{
		g_free( _carlife_module_status);
	}
	if(_touch != 0)
	{
		pthread_cancel(_touch);
		pthread_join(_touch, NULL);
		_touch = 0;
		dbg_printf("sample---------------------------------------->_detach_event pthread_cancel _touch\n");
	}
	mh_object_unref((MHObject *)global_dev);
	global_dev = NULL;
	tmpx = 0;

//	system("/etc/rc.d/mdns stop");

}		/* -----  end of static function _detach_event  ----- */
void _dev_module_status_callback(MHDev *dev, ModuleStatusList * module_status, void * user_data)
{

	dbg_printf("\n\nsample.c---->%s--->%d\n", __func__, module_status->count);
	int i = 0;

	for(i; i <  module_status->count; i++)
	{
		switch(module_status->list[i].module)
		{
			case PHONE_MODULE:
			{
				if( module_status->list[i].status == PHONE_STATUS_IDLE)
				{
					dbg_printf("phone is idle\n");
				}
				else if( module_status->list[i].status == PHONE_STATUS_INCOMING)
				{
					dbg_printf("phone is incoming\n");
				}
				else if( module_status->list[i].status == PHONE_STATUS_OUTING)
				{
					dbg_printf("phone is outing\n");

				}
			}
			break;

			case NAVI_MODULE:
			{
				if( module_status->list[i].status == NAVI_STATUS_IDLE)
				{
					dbg_printf("navi is idle\n");
				}
				else if( module_status->list[i].status == NAVI_STATUS_RUNNING)
				{
					dbg_printf("navi is running\n");
				}
				else if( module_status->list[i].status	==	NAVI_STATUS_STOP)
				{
					dbg_printf("navi is stop\n");
				}
			
			}
			break;

			case MUSIC_MODULE:
			{
				if( module_status->list[i].status == MUSIC_STATUS_IDLE)
				{
					dbg_printf("music is idle\n");
				}
				else if( module_status->list[i].status == MUSIC_STATUS_RUNNING)
				{
					dbg_printf("music is running\n");
				}

			}
			break;

			case VR_MODULE:
			{
				if( module_status->list[i].status == VR_STATUS_RECORD_IDLE)
				{
					dbg_printf("vr record is idle\n");
				}
				else if( module_status->list[i].status == VR_STATUS_RECORD_RUNNING)
				{
					dbg_printf("vr record is running\n");
				}
				else if( module_status->list[i].status == VR_STATUS_NOT_SUPPORT)
				{
					dbg_printf("vr is not support\n");

				}

			}
			break;

			case CONNECT_MODULE:
			{
				if( module_status->list[i].status == CONNECT_STATUS_ADB)
				{
					dbg_printf("connect is adb\n");
				}
				else if( module_status->list[i].status == CONNECT_STATUS_AOA)
				{
					dbg_printf("connect is aoa\n");
				}
				else if( module_status->list[i].status == CONNECT_STATUS_NCM_ANDROID)
				{
					dbg_printf("connect is ncm android\n");
				}
				if( module_status->list[i].status == CONNECT_STATUS_NCM_IOS)
				{
					dbg_printf("connect is ios\n");
				}
				else if( module_status->list[i].status == CONNECT_STATUS_WIFI)
				{
					dbg_printf("connect is wifi\n");
				}
			}
			break;

			case MIC_MODULE:
			{
				if( module_status->list[i].status == MIC_STATUS_USE_VEHICLE_MIC)
				{
					dbg_printf("mic is vehicle\n");
				}
				else if( module_status->list[i].status == MIC_STATUS_USE_MOBILE_MIC)
				{
					dbg_printf("mic is mobile\n");
				}
				else if( module_status->list[i].status == MIC_STATUS_NOT_SUPPORT)
				{
					dbg_printf("mic is not support\n");
				}

			}
			break;

			case MEDIA_PCM_MODULE:
			{
				if( module_status->list[i].status	==	MEDIA_PCM_STATUS_IDLE)
				{
					dbg_printf("media pcm module is idle\n");
				}
				else if( module_status->list[i].status	==	MEDIA_PCM_STATUS_RUNNING)
				{
					dbg_printf("media pcm module is running\n");
				}
				else 
				{
					dbg_printf("media pcm module is invalid");
				}

			}
			break;
			default:
			break;

		}
	}
}
void _dev_bt_hfp_request_cb( MHDev * dev, MHCarlifeBtHfpRequest * request, void * user_data)
{
	switch( request->type)
	{
		case CARLIFE_REQUEST_START_CALL:
			dbg_printf("%s->CARLIFE_REQUEST_START_CALL\n", __func__);
			break;
		case CARLIFE_REQUEST_TERMINATE_CALL:
			dbg_printf("%s->CARLIFE_REQUEST_TERMINATE_CALL\n", __func__);
			break;
		case CARLIFE_ANSWER_CALL:
			{
//				ANW_AnswerCall(0);
				MHCarlifeBtHfpResponse _response;
				_response.status	=	BT_HFP_RESPONSE_SUCCESS;
				_response.cmd	=	BT_HFP_ANSWER_CALL;
				_response.dtmfCode	=	0;
				mh_dev_carlife_send_bt_hfp_response( dev, &_response);
				dbg_printf("%s->CARLIFE_ANSWER_CALL\n", __func__);
			}
			break;
		case CARLIFE_REJECT_CALL:
			dbg_printf("%s->CARLIFE_REJECT_CALL\n", __func__);
			break;
		case CARLIFE_DTMF_CODE:
			dbg_printf("%s->CARLIFE_DTMF_CODE\n", __func__);
			break;
		case CARLIFE_MUTE_MIC:
			dbg_printf("%s->CARLIFE_MUTE_MIC\n", __func__);
			break;
	}

}
void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char   *_dev_type;
	dbg_printf( "Device[ %p ] status[ %d ]\n", dev, event );
	
	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);

	if ( _dev_type && !g_strcmp0(_dev_type, "carlife"))
	{
	g_message("\n\n\n\ntype->%s--->dev:%p\n\n\n", _dev_type, dev);
		char * _os;
		_carlife_status_listener	=	(MHDevStatusListener*)g_new0( MHDevStatusListener, 1);
		_carlife_status_listener->callback	=	_dev_status_callback;	
		_carlife_status_listener->user_data	=	dev;
		mh_dev_register_status_listener( dev, _carlife_status_listener);

		_carlife_module_status	=	g_new0( ModuleStatusListener, 1);
		_carlife_module_status->callback	=	_dev_module_status_callback;
		_carlife_module_status->user_data	=	dev;

		mh_dev_register_module_status(dev, _carlife_module_status);

		_carlife_bt_hfp_request	=	g_new0( MHCarlifeBtHfpRequestListener, 1);
		_carlife_bt_hfp_request->callback	=	_dev_bt_hfp_request_cb;
		_carlife_bt_hfp_request->user_data	=	dev;

		mh_object_ref( (MHObject *)dev );
		dbg_printf("the carlife device plug in\n");
		bool _exist;
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
		mh_dev_register_detach_listener( dev, & _detach_listener);

		mh_object_set_properties((MHObject *)dev, "width", 800, "height", 480 ,"carlife_media_client_name", "media",
				"carlife_navi_client_name", "navi", "carlife_vrtts_client_name", "vr","carlife_video_sink", "v4l2sink",
				"carlife_surfaceid", 888,"carlife_audio_sink", "pulsesink",  "framerate", "24", NULL);

		global_dev 	=	dev;

		mh_dev_carlife_launch_mode( dev, CARLIFE_LAUNCH_NORMAL);	

		mh_object_get_properties((MHObject *)dev, "md_os", &_os, NULL);
		mh_object_get_properties((MHObject *)dev, "exist", &_exist, NULL);
		if(_os && !g_strcmp0( _os, "ios") )
		{
			dbg_printf("receive carlife device\n");
			tmpx = 1;
			pthread_create( &_touch, NULL, ios_touch_task, NULL);

			g_message("\ncarlife->ios\n");
			androidFlag	=	0;
		}
		else if( _os && ! g_strcmp0( _os, "android") && _exist != 0)
		{
			dbg_printf("\n\n*****************************persent %10\n\n");
			tmpx = 1;
			pthread_create( &_touch, NULL, android_touch_task, NULL);
			androidFlag = 1;

			mh_dev_register_bt_hfp_request( dev, _carlife_bt_hfp_request);

			mh_dev_start( dev );
		}
		else
		{
			dbg_printf("the device have no carlife!!!\n\n\n");
		}

		//change audio channel to usb

	}	
	else
	{
		dbg_printf("the device type is not carlife device\n");

	}
	

}

void event_arrived( MHCore * core, MHCoreEvent event,const char * type, void * user_data )
{
	switch( event )
	{
	case MH_CORE_STARTED:
		dbg_printf( "Media-Hub v2.0 has been started\n" );

		break;
	case MH_CORE_PLUGIN_INVALID:
		dbg_printf( "Invalid Media-Hub v2.0 plugin be found\n" );

		break;
	case MH_CORE_PLUGIN_NOT_FOUND:
		dbg_printf( "No plugins be found\n" );

		break;
	case MH_CORE_PLUGIN_LOAD_SUCCESS:
		dbg_printf( "Success load a plugin---------------->%s\n", type );
		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		dbg_printf( "Failed load a plugin\n" );
		break;
	default:
		break;
	}
}
static int folderCount;
static int displayIndex;

#include <sys/time.h>
#include <assert.h>
#include <execinfo.h>

#define DEBUG_TIME_BEGIN() \
{ \
	struct timeval _start = {0, 0}, _current = {0, 0}, _result = {0, 0}; \
	float _max = 0, _min = 10000, _avg = 0; \
	int _count = 0; \
	float _tmp; \
	dbg_printf("**********TIME MEASURING BEGIN**********\n");

#define DEBUG_TIME_DUR_BEGIN() \
	gettimeofday(&_start, NULL);

#define DEBUG_TIME_DUR_END() \
	gettimeofday(&_current, NULL); \
	timersub(&_current, &_start, &_result); \
	{ \
		_tmp    =   _result.tv_sec + (float)_result.tv_usec / 1000000; \
		if(_tmp > _max) _max =   _tmp; \
		if(_tmp < _min) _min =   _tmp; \
		_avg    +=  (_tmp - _avg) / ( ++ _count); \
		fprintf(stdout, "***CNT:%d MAX:%03fs MIN:%03fs AVG:%03fs CUR:%03f\n", _count, _max, _min, _avg, _tmp); \
	}

#define DEBUG_TIME_END() \
	dbg_printf("***********TIME MEASURING END (%d)***********\n", _count); \
}

//#include <gperftools/profiler.h>
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handler
 *  Description:  
 * =====================================================================================
 */
static void handler( int sig )
{

	exit( 0 );
}		/* -----  end of static function handler  ----- */
//static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
//{
//    ilm_surfaceSetDestinationRectangle(id, 0, 0, width, height);
//    dbg_printf("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
//	ilm_surfaceSetSourceRectangle(id, 0, 0, width,height);
//
//    dbg_printf("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
//    ilm_surfaceSetVisibility(id, ILM_TRUE);
//    dbg_printf("SetVisibility          : surface ID (%d), ILM_TRUE\n", id);
//    ilm_layerAddSurface(layer,id);
//    dbg_printf("layerAddSurface        : surface ID (%d) is added to layer ID (%d)\n", id, layer);
//    ilm_commitChanges();
//}
//
//static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
//{
//    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
//    {
//        configure_ilm_surface(id,sp->origSourceWidth,sp->origSourceHeight);
//
//    }
//}
//static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
//{
//    (void)user_data;
//	struct ilmSurfaceProperties sp;
//
//	if (object == ILM_SURFACE)
//	{
//		if (created)
//		{
//			//            if (number_of_surfaces > 0) {
//			if(id ==888)
//			{
//				number_of_surfaces--;
//				dbg_printf("surface                : %d created\n",id);
//				ilm_getPropertiesOfSurface(id, &sp);
//				if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
//				{   // surface is already configured
//					configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
//				}
//				else
//				{
//					// wait for configured event
//					ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
//					ilm_commitChanges();
//				}
//			}
//		}
//
//		else if(!created)
//			dbg_printf("surface: %d destroyed\n",id);
//
//	}
//	else if (object == ILM_LAYER)
//	{
//		if (created)
//			dbg_printf("layer: %d created\n",id);
//		else if(!created)
//			dbg_printf("layer: %d destroyed\n",id);
//	}
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  check_input_devices
 *  Description:  
 * =====================================================================================
 */
static char *  check_input_devices()
{
	char * _res	=	NULL;
	struct udev * _udevCtx;
	struct udev_enumerate * _enum;
	struct udev_list_entry * _devices, * _entry,  * _properties, * _model;

	_udevCtx	=	udev_new();
	_enum   =   udev_enumerate_new( _udevCtx );

	udev_enumerate_add_match_subsystem( _enum, "input" );
	udev_enumerate_scan_devices( _enum );

	_devices    =   udev_enumerate_get_list_entry( _enum );

	udev_list_entry_foreach( _entry, _devices )
	{
		const char * _path =   udev_list_entry_get_name( _entry );
		struct udev_device * _udevDev;

		dbg_printf( "%s\n", _path );
		if( g_strrstr( _path, "i2c-2")!= NULL)
		{
			_udevDev    =   udev_device_new_from_syspath( _udevCtx, _path );
			_properties =   udev_device_get_properties_list_entry( _udevDev );
			_model	=	udev_list_entry_get_by_name( _properties, "DEVNAME");
			if( _model)
			{
				_res	=	g_strdup( udev_list_entry_get_value( _model));
				dbg_printf("DEVNAME:%s\n", _res);
				break;
			}

		
		}
	}
	return _res;

}		/* -----  end of static function check_input_devices  ----- */

static void * ios_touch_task( void * user_data )
{
	dbg_printf("ios_touch_task IN!!!!!\n");
	int _fd	=	0;
	int _lastPress = TOUCH_UP;
	int _press, _x, _y, _fristDown;
	int _x_diff = 0, _y_diff = 0;

	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			dbg_printf("ios_touch_task can't get input path\n");
			return NULL;
		}
		dbg_printf("ios_touch_task _path:%s\n", _path);
		_fd	=	open( _path, O_RDWR );
		g_free( _path);
	}

	while( 1 )
	{
		struct input_event _event;
		ssize_t _ret	=	read( _fd, &_event, sizeof( struct input_event ));

		if( _ret < 0 )
			perror( "read" );
		if( global_dev == NULL)
		{
			dbg_printf("ios_touch_task exit 0!!!!!\n");
			close(_fd);
			_fd = 0;
			break;
		}
//		dbg_printf( "%d %04X %04X %04X %04X %04X %04X %04X %04X\n", _ret, _event.time.tv_sec & 0xFFFF,
//				_event.time.tv_sec >> 16 , _event.time.tv_usec & 0xFFFF, _event.time.tv_usec >> 16,
//				_event.type, _event.code, _event.value & 0xFFFF, _event.value >> 16 );
//		dbg_printf("ios_touch_task-----type:0x%x, code:0x%x, value:0x%x\n", _event.type, _event.code, _event.value);
		switch( _event.type )
		{
		case EV_ABS:
			switch( _event.code )
			{
			case ABS_MT_TOUCH_MAJOR:
				break;
			case ABS_MT_POSITION_X:
				if(_event.value != _x)
				{
					_x	=	( _event.value  );
					_x_diff = 1;
				}
				else
				{
					_x_diff = 0;
				}
//				_x	=	(_x * 800)/480;
				break;
			case ABS_MT_POSITION_Y:
				if(_event.value != _y)
				{
					_y	=	( _event.value );
					_y_diff = 1;
				}
				else
				{
					_y_diff = 0;
				}
//				_y	=	(_y * 480)/800;
				break;
			case ABS_MT_TRACKING_ID:
				break;
			case ABS_MT_PRESSURE:
				break;
			default:
//				dbg_printf( "Uncached ABS event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_SYN:
			switch( _event.code )
			{
			case SYN_REPORT:
				if( _press	== TOUCH_UP)
				{
					if(_lastPress == TOUCH_MOVE)
					{
						dbg_printf("\n\n----------ios_touch_task---------->_lastPress = TOUCH_UP\n\n", _x, _y);
						_lastPress = TOUCH_UP;
						mh_dev_send_signal_touch( global_dev, TOUCH_UP, _x, _y);
					}
					else
					{
						dbg_printf("\n\n----------ios_touch_task---------->send TOUCH_UP %d, %d\n\n", _x, _y);
						mh_dev_send_signal_touch( global_dev, TOUCH_SIGNAL_CLICK, _x, _y);
						_lastPress	=	TOUCH_UP;
					}
					_x_diff = 0;
					_y_diff = 0;
				}
				else if( _press	==	 TOUCH_DOWN)
				{
					if(_lastPress	== TOUCH_UP)
					{
						dbg_printf("\n\n----------ios_touch_task---------->send TOUCH_DOWN %d, %d\n\n", _x, _y);
						mh_dev_send_signal_touch( global_dev, TOUCH_DOWN, _x, _y);
						_lastPress	=	TOUCH_DOWN;
					}
					else
					{
						if(_x_diff || _y_diff)
						{
							dbg_printf("\n\n----------ios_touch_task---------->send TOUCH_MOVE %d, %d\n\n", _x, _y);
							mh_dev_send_signal_touch( global_dev, TOUCH_MOVE, _x, _y);
							_lastPress	=	TOUCH_MOVE;

						}
						else
						{
							dbg_printf("\n\n----------ios_touch_task---------->_lastPress	=	TOUCH_DOWN\n\n", _x, _y);
							_lastPress	=	TOUCH_DOWN;
						}
					}
				}
				else
				{
					;//do nothing
				}
				break;
			case SYN_MT_REPORT:
				break;
			default:
//				dbg_printf( "Uncached SYN event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_KEY:
			switch( _event.code )
			{
				case BTN_TOUCH:
					if( _event.value	==	1 )
					{
						_press	=	TOUCH_DOWN;	
					}
					else 
					{
						_press	=	TOUCH_UP;
						_fristDown	=	0;
					}
					break;
				default:
					break;
			}
			break;
		}
	}

	return NULL;
}		/* -----  end of static function touch_task  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  touch_task
 *  Description:  
 * =====================================================================================
 */
static void * android_touch_task( void * user_data )
{
	dbg_printf("android_touch_task IN!!!!!\n");
	int _fd	=	0;
	int _press, _x, _y, _fristDown = 0;

	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			dbg_printf("android_touch_task can't get input path\n");
			return NULL;
		}
		dbg_printf("android_touch_task _path:%s\n", _path);
		_fd	=	open( _path, O_RDWR );
		g_free( _path);
	}

	while( 1 )
	{
		struct input_event _event;
		ssize_t _ret	=	read( _fd, &_event, sizeof( struct input_event ));

		if( _ret < 0 )
			perror( "read" );
		if( global_dev == NULL)
		{
			dbg_printf("android_touch_task exit 0!!!!!\n");
			close(_fd);
			_fd = 0;
			break;
		}
//		dbg_printf( "%d %04X %04X %04X %04X %04X %04X %04X %04X\n", _ret, _event.time.tv_sec & 0xFFFF,
//				_event.time.tv_sec >> 16 , _event.time.tv_usec & 0xFFFF, _event.time.tv_usec >> 16,
//				_event.type, _event.code, _event.value & 0xFFFF, _event.value >> 16 );
//		dbg_printf("android_touch_task-----type:0x%x, code:0x%x, value:0x%x\n", _event.type, _event.code, _event.value);
		switch( _event.type )
		{
		case EV_ABS:
			switch( _event.code )
			{
			case ABS_MT_TOUCH_MAJOR:
				break;
			case ABS_MT_POSITION_X:
				_x	=	( _event.value  );
//				_x	=	(_x * 1280)/720;	
				break;
			case ABS_MT_POSITION_Y:
				_y	=	( _event.value );
//				_y	=	(_y * 720)/1280;
				break;
			case ABS_MT_TRACKING_ID:
				break;
			case ABS_MT_PRESSURE:
				break;
			default:
//				dbg_printf( "Uncached ABS event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_SYN:
			switch( _event.code )
			{
			case SYN_REPORT:
				if( _press	==	TOUCH_DOWN)
				{
					if( _fristDown	==	0)
					{
						mh_dev_send_signal_touch( global_dev, TOUCH_DOWN, _x, _y );
						dbg_printf("\n\n----------android_touch_task---------->send down %d, %d\n\n", _x, _y);
						_fristDown	=	1;
					}
					else
					{
						mh_dev_send_signal_touch( global_dev, TOUCH_MOVE, _x, _y);
						dbg_printf("\n\n********android_touch_task*******send move %d, %d\n\n", _x, _y);

					}
				}
				else
				{
					mh_dev_send_signal_touch( global_dev, _press, _x, _y);
					dbg_printf("\n\nsend up<----------android_touch_task------------------ %d, %d\n\n", _x, _y);
				}
				break;
			case SYN_MT_REPORT:
				break;
			default:
//				dbg_printf( "Uncached SYN event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_KEY:
			switch( _event.code )
			{
				case BTN_TOUCH:
					if( _event.value	==	1 )
					{
						dbg_printf("\n\n********android_touch_task*******_press	=	TOUCH_DOWN\n\n");
						_press	=	TOUCH_DOWN;	
					}
					else 
					{
						dbg_printf("\n\n********android_touch_task*******_press	=	TOUCH_UP\n\n");
						_press	=	TOUCH_UP;
						_fristDown	=	0;
					}
					break;
				default:
					break;
			}
			break;
		}
	}

	return NULL;
}		/* -----  end of static function touch_task  ----- */

gpointer input_thread( gpointer data)
{
	char input[256];

	while(1)
	{
		if(scanf("%s", input ) != 0)
		{
			switch(input[0])
			{
				case 'a':
					mh_dev_carlife_send_hardkey( global_dev, MH_CARLIFE_HARDKEY_MEDIA);
					break;
				case 'b':
					mh_dev_carlife_send_hardkey( global_dev, MH_CARLIFE_HARDKEY_NAVI);
					break;
				case 'c':
					mh_dev_carlife_video_pause( global_dev);
					break;
				case 'd':
					mh_dev_carlife_video_reset( global_dev);
					break;
				case 'e':
					{
						MHCarlifeCarVelocity velocity	={1};
						mh_dev_carlife_car_velocity( global_dev, &velocity);
					}
					break;
				case 'f':
					{
						MHCarlifeCarGPS gps	=	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
						mh_dev_carlife_car_GPS( global_dev, &gps);
					}
					break;
				case 'g':
					{
						MHCarlifeCarGyroscope gyroscope	=	{1,2,3,4};
						mh_dev_carlife_car_gyroscope( global_dev, &gyroscope);
					}
					break;
				case 'h':
					{
						MHCarlifeCarAcceleration acceleration	=	{1,2,3,4};
						mh_dev_carlife_car_acceleration(global_dev, &acceleration);
					}
					break;
				case 'i':
					{
						mh_dev_module_control( global_dev, MUSIC_MODULE, MUSIC_STATUS_IDLE); 
					}
					break;
				case 'j':
					{
						mh_dev_module_control( global_dev, MUSIC_MODULE, MUSIC_STATUS_RUNNING);
					}
					break;
				case 'k':
					{
						mh_dev_carlife_video_start( global_dev);
					}
					break;
				case 'l':
					{
						mh_dev_start( global_dev);
					}
					break;
				case 'm':
					{
						mh_dev_carlife_video_pause( global_dev);
					}
					break;
				case 'n':
					{
						mh_misc_set_iap_device_mode( MISC_IAP_CARLIFE);
					}
					break;
				case 'x':
					{
						mh_dev_carlife_send_hardkey(global_dev,MH_CARLIFE_HARDKEY_SEEK_ADD);
					}
					break;
				case 'z':
					{
						mh_dev_carlife_send_hardkey(global_dev,MH_CARLIFE_HARDKEY_SEEK_SUB);
					}
					break;
				default:
					break;
			}
		}
	}
}
//int g_HFPConnectStatus[2] = {0};
//int g_A2DPConnectStatus = 0;
//int g_HIDConnectStatus = 0;
//int g_SCOConnectStatus[2] ={0};
//int g_AVRCPBrowsingConnectStatus = 0;
//int ConnectStatusCallback(int nIndex,Connect_Channel Channel,int nStatus, ANW_BD_ADDR bd_addr)
//{
//	char szProfile[256];
//	char szConnectStatus[256];
//	szProfile[0]='\0';
//	szConnectStatus[0] = '\0';
//	if(nStatus == 1)
//		sprintf(szConnectStatus,"connected");
//	else
//	{
//	if(nStatus ==-1)
//		sprintf(szConnectStatus,"disconnected : linklost");
//	else
//		sprintf(szConnectStatus,"disconnected");
//	}
//	switch(Channel)
//	{
//		case HF_CHANNEL: // HF
//			if(nIndex>=0 && nIndex<2)
//			g_HFPConnectStatus[nIndex] = nStatus;
//			sprintf(szProfile,"HFP");
//			break;
//		case PIMDATA_CHANNEL: // Data
//			sprintf(szProfile,"PIM");
//			break;
//		case AUDIO_STREAM_CHANNEL: // A2DP
//			g_A2DPConnectStatus =  nStatus;
//			sprintf(szProfile,"A2DP");
//			break;
//		case AUDIO_CONTROL_CHANNEL: // AVRCP
//			sprintf(szProfile,"AVRCP");
//			break;
//		case SPP_CHANNEL:
//			sprintf(szProfile,"SPP");
//			break;
//		case HID_CONTROL_CHANNEL:
//			g_HIDConnectStatus = nStatus;
//			sprintf(szProfile,"HID Control");
//			break;
//		case HID_INTERRUPT_CHANNEL:
//			sprintf(szProfile,"HID interrupt");
//			break;
//		case SCO_CHANNEL:
//			sprintf(szProfile,"SCO");
//			if(nIndex>=0 && nIndex<2)
//			g_SCOConnectStatus[nIndex]= nStatus;
//			break;
//		case AVRCP_BROWSING_CHANNEL:
//			g_AVRCPBrowsingConnectStatus = nStatus;
//			sprintf(szProfile,"AVRCP browsing");
//			break;
//		case PHONEBOOK_CHANNEL:
//			sprintf(szProfile,"Phonebook");
//			break;
//		case SMS_CHANNEL:
//			sprintf(szProfile,"SMS");
//			break;
//		default:
//			break;
//	}
//	dbg_printf("  %s (%02x:%02x:%02x:%02x:%02x:%02x) %s -index =%d\n",szProfile,bd_addr.bytes[0],bd_addr.bytes[1],bd_addr.bytes[2],bd_addr.bytes[3],bd_addr.bytes[4],bd_addr.bytes[5],szConnectStatus,nIndex);
//
//	return 1;
//}
//int ServiceConnectRequestCallBack(Connect_Channel nType,ANW_BD_ADDR bd_addr,ANW_CHAR* szdeviceName,int nIndex)
//{
//	dbg_printf("ServiceConnectRequestCallBack:nIndex = %d ServiceConnectRequestCallBack(%d)\n",nIndex,nType);
//	switch(nType)
//	{
//		case HF_CHANNEL:
//			ANW_HFAcceptConnection(nIndex);
//			break;
//		case HID_CONTROL_CHANNEL:
//			ANW_HidAccept();
//			break;
//		case AUDIO_STREAM_CHANNEL:
//			ANW_AudioAcceptConnection(TYPE_A2DP);
//			break;
//		case AUDIO_CONTROL_CHANNEL:
//			ANW_AudioAcceptConnection(TYPE_AVRCP);
//			break;
//	}
//	return 1;
//}
//int PairStatusCallback(PAIRSTATUS_TYPE nType,unsigned int data,ANW_BD_ADDR* bd_addr,int cod)
//{
//	dbg_printf("\n\n\n\n%s--->nType:%d, data:%d, address:%2x%2x%2x%2x%2x%2x, cod:%d\n\n\n\n",
//			__func__, nType, data, bd_addr->bytes[0], bd_addr->bytes[1], bd_addr->bytes[2], bd_addr->bytes[3],
//			bd_addr->bytes[4], bd_addr->bytes[5], cod);
//	if(nType	==	SSP_PAIR_INCOMING_REQUEST)
//	{
//		ANW_DeviceSSPAccept( *bd_addr);
//	}
//	return 1;
//}
//int PL_IsPhoneNumberEntryType(GSM_EntryType entryType)
//{
//	if(entryType == PBK_Number_General ||
//		entryType == PBK_Number_Mobile ||
//		entryType == PBK_Number_Work ||
//		entryType == PBK_Number_Fax ||
//		entryType == PBK_Number_Home ||
//		entryType == PBK_Number_Pager ||
//		entryType == PBK_Number_Other ||
//		entryType == PBK_Number_Sex ||
//		entryType == PBK_Number_Light ||
//		entryType == PBK_Number_Mobile_Home ||
//		entryType == PBK_Number_Mobile_Work ||
//		entryType == PBK_Number_Fax_Home ||
//		entryType == PBK_Number_Fax_Work ||
//		entryType == PBK_Number_Pager_Home ||
//		entryType == PBK_Number_Pager_Work ||
//		entryType == PBK_Number_VideoCall ||
//		entryType == PBK_Number_VideoCall_Home ||
//		entryType == PBK_Number_VideoCall_Work ||
//		//new add
//		entryType == PBK_Number_Assistant ||
//		entryType == PBK_Number_Business ||
//		entryType == PBK_Number_Callback ||
//		entryType == PBK_Number_Car ||
//		entryType == PBK_Number_ISDN ||
//		entryType == PBK_Number_Primary ||
//		entryType == PBK_Number_Radio ||
//		entryType == PBK_Number_Telix ||
//		entryType == PBK_Number_TTYTDD	)
//		return 1;
//	return 0;
//
//}
//void PL_GetEntryNameNumber(GSM_MemoryEntry entry,char *szName,char *szNumber)
//{
//	int i=0;
//	int iFirstName = -1;
//	int iLastName = -1;
//	int iName = -1;
//	int iNumber = -1;
//
//	szName[0] ='\0';
//	szNumber[0] ='\0';
//
//	for( i = 0; i<entry.EntriesNum ; i++)
//	{
//		if(entry.Entries[i].EntryType == PBK_Text_FirstName)
//		{
//			iFirstName = i;
//		}
//		else if(entry.Entries[i].EntryType == PBK_Text_LastName)
//		{
//			iLastName = i;
//		}
//		else if(entry.Entries[i].EntryType == PBK_Text_Name)
//		{
//			iName = i;
//		}
//		else if(iNumber == -1 && PL_IsPhoneNumberEntryType(entry.Entries[i].EntryType) == 1)
//			iNumber = i;
//	}
//	if(iName !=-1)
//		sprintf(szName,"%s",entry.Entries[iName].Text);// UTF8 encode
//	else
//	{
//		if(iFirstName != -1)
//			sprintf(szName,"%s ",entry.Entries[iFirstName].Text);// UTF8 encode
//		if(iLastName != -1)
//			strcat(szName,entry.Entries[iLastName].Text);// UTF8 encode
//	}
//
//	if(iNumber!=-1)
//		sprintf(szNumber,"%s",entry.Entries[iNumber].Text);
//
//}
//int IncomingCallCallback(int uIndex, GSM_MemoryEntry entry,int nCallStatus)
//{
//dbg_printf("\n\n\n\n%s-------->nCallStstus:%d\n", __func__, nCallStatus);
//
//	MHCarlifeBtHfpIndication _indication;
//	_indication.type	=	CARLIFE_BT_INDICATION_NULL;
//
//	ANW_BD_ADDR _address;
//	char _name[100];
//
//	switch( nCallStatus)
//	{
//		case 0:
//			_indication.type	=	CARLIFE_BT_INDICATION_CALL_INACTIVE;
//			break;
//		case 1:
//			{
//				char _name[300];
//				char _number[300];
//				PL_GetEntryNameNumber(entry, _name, _number);
//				dbg_printf("_name:%s, _number:%s\n", _name, _number);
//				_indication.type	=	CARLIFE_BT_INDICATION_INCOMMING_CALL;
//				_indication.phoneNum	=	g_strdup( _number);
//				dbg_printf("PL_GetEntryNameNumber->name:%s, phoneNum:%s\n", _name, _indication.phoneNum);
//			}
//
//			break;
//		case 2:
//			_indication.type	=	CARLIFE_BT_INDICATION_CALL_ACTIVE;
//			break;
//		case 3:
//			_indication.type	=	CARLIFE_BT_INDICATION_OUTGOING_CALL;
//			break;
//		case 9:
//			_indication.type	=	CARLIFE_BT_INDICATION_MULTICALL_ACTIVE;
//			break;
//		default:
//			break;
//
//	}
//	if( _indication.type !=	CARLIFE_BT_INDICATION_NULL)
//	{
//		ANW_GetConnectedDevice( uIndex, &_address, _name);
//		_indication.address	=	g_strdup_printf("%02X:%02X:%02X:%02X:%02X:%02X", _address.bytes[5], _address.bytes[4],
//				_address.bytes[3],_address.bytes[2], _address.bytes[1],_address.bytes[0]);
//		_indication.name	=	g_strdup( _name );
//
//		dbg_printf("ANW_GetConnectedDevice->bt_address:%s, name:%s\n", _indication.address, _indication.name);
//		mh_dev_carlife_send_bt_hfp_indication(global_dev,  &_indication);
//		g_free( _indication.address);
//		g_free( _indication.name);
//		if( _indication.type == CARLIFE_BT_INDICATION_INCOMMING_CALL)
//		{
//			g_free( _indication.phoneNum);
//		}
//
//	}
//	return 1;
//}
//
//void bt_init()
//{
//	system("echo 0 > /sys/class/gpio/gpio449/value");
//	system("echo 449 > /sys/class/gpio/unexport");
//	usleep(500*1000);
//	system("echo 449 > /sys/class/gpio/export");
//	system("echo out > /sys/class/gpio/gpio449/direction");
//	system("echo 1 > /sys/class/gpio/gpio449/value");
//	dbg_printf("system cmd is finish\n");
//
//
//	dbg_printf("\n\n%s\n\n", __func__);
//	////bt
//	if(1 == ANW_ManagerInitialEx("/media/ivi-data/anw"))
//	{
//		dbg_printf("ANW_ManagerInitial is Success\n\n");
//	}
//	if( 1	==	ANW_DeviceInitial())
//	{
//		dbg_printf("ANW_DeviceInitial is Success\n\n");
//	}
////	ANW_EnableLog(1);
//
//	ANW_SetConnectStatusCallBack(ConnectStatusCallback);
//	ANW_SetPairStatusCallBack(PairStatusCallback);
//	ANW_SetServiceConnectRequestCallBack(ServiceConnectRequestCallBack);
////	ANW_SetHFIndicatorCallBack(HFIndicatorCallback);
//	ANW_SetIncomingframeCallBack(IncomingCallCallback, NULL);
//	//	ANW_AudioControlSetEventCallBack(AudioControlEventCallback);
//	ANW_BD_ADDR _pba;
//	ANW_ReadLocalAddr(&_pba);
////	BTAddress =	g_strdup_printf("93:DB:1F:88:BA:D3");
//	dbg_printf("ANW_ReadLocalAddr:%2x:%2x:%2x:%2x:%2x:%2x\n", _pba.bytes[0], _pba.bytes[1],
//			_pba.bytes[2],_pba.bytes[3], _pba.bytes[4],_pba.bytes[5] );
//	BTAddress	=	g_strdup_printf("%02X:%02X:%02X:%02X:%02X:%02X",  _pba.bytes[5] , _pba.bytes[4], _pba.bytes[3], _pba.bytes[2],  _pba.bytes[1], _pba.bytes[0]);
//	dbg_printf("\n\n\n\nBTAddress initialization:%s\n\n\n\n", BTAddress);
//	ANW_SetDeviceName("donglijuan");
//	ANW_SetDiscoveryMode(3);
//
//
//
//}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
//	ilm_init();
//	screen	=	1;
//	layer	=	1000;
	app_rpc_attach(&app_source_rpc_handle);
	rpc_snd_aslect_t asl = {ASELEC_SRC_USB, ASELEC_OUT_FL};
	app_rpc_snd_aud_selt(app_source_rpc_handle, &asl);
	number_of_surfaces	=	1000;
//	 struct ilmScreenProperties screenProperties;
//    struct ilmScreenExtendedProperties screenExtendedProperties;

//    ilm_getPropertiesOfScreen(screen, &screenProperties);
//    ilm_getExtendedPropertiesOfScreen(screen, &screenExtendedProperties);

//    screenWidth = 800//screenProperties.screenWidth;
//    screenHeight = 480//screenProperties.screenHeight;
//    ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
//    dbg_printf("CreateWithDimension          : layer ID (%d), Width (%u), Height (%u)\n", layer, screenWidth, screenHeight);
//    ilm_layerSetDestinationRectangle(layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
//    dbg_printf("layerSetDestinationRectangle : layer ID (%d), X (%u), Y (%u), Width (%u), Height (%u)\n", layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
//    ilm_layerSetVisibility(layer,ILM_TRUE);
//    dbg_printf("SetVisibility                : layer ID (%d), ILM_TRUE\n", layer);

//    t_ilm_int i, length = 0;
//    t_ilm_layer* pArray = NULL;
//    ilm_getLayerIDs(&length, &pArray);

//    t_ilm_layer renderOrder[length + 1];
//    renderOrder[length] = layer;

//    for(i=0;i<length;i++) {
//        renderOrder[i] = pArray[i];
//    }

//    ilm_displaySetRenderOrder(0,renderOrder,length + 1);
//    ilm_commitChanges();
//    ilm_registerNotification(callbackFunction, NULL);
//


	MHEventsListener _eventListener	=	
	{
		.callback	=	event_arrived,
		.user_data	=	NULL
	};

	MHDevicesListener _devListener	=	
	{
		.callback	=	dev_event,
		.user_data	=	NULL
	};

	signal( SIGINT, handler );
//	ProfilerStart( "sample.prof" );
#ifdef USE_MH2_IPC
	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	mh_ipc_media_client_init( _conn );
#endif

	dbg_printf( "Media-Hub v2.0 Built Time: %s %s\n", mh_built_date(), mh_built_time() );
	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );
	mh_core_start();
	mh_misc_set_iap_device_mode( MISC_IAP_CARLIFE);
//	bt_init();

	dbg_printf("BTAddress:%s\n", BTAddress);

	g_thread_new( "mh_test_input", input_thread, NULL );
//			pthread_create( &_touch, NULL, ios_touch_task, NULL);

	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

