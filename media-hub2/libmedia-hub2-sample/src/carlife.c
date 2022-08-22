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
//#include <mh_extern_misc.h>
MHDev * global_dev ;
static GMainLoop *test_loop;

MHPb * _pb;
MHDevStatusListener * _carlife_status_listener;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_callback(MHDev * self, MHDevStatus status, void * user_data)
{
	switch( status )
	{
		case	CARLIFE_CONNECT_SUCCESS:
			printf("recevive CARLIFE_CONNECT_SUCCESS\n");
			break;
		case 	CARLIFE_VIDEO_INIT_DONE:
			printf("recevive CARLIFE_VIDEO_INIT_DONE\n");
			break;
		case 	CARLIFE_FOREGROUND:
			printf("recevive CARLIFE_FOREGROUND\n");
			break;
		case	CARLIFE_BACKGROUND:
			printf("recevive CARLIFE_BACKGROUND\n");
			break;
		case 	CARLIFE_SCREEN_ON:
			printf("recevive CARLIFE_SCREEN_ON\n");
			break;
		case	CARLIFE_SCREEN_OFF:
			printf("recevive CARLIFE_SCREE_OFF\n");
			break;
		case 	CARLIFE_USER_PRESENT:
			printf("recevive CARLIFE_USER_PRESENT\n");
		mh_dev_start( global_dev );
			break;
		case 	CARLIFE_APP_EXIT:
			printf("recevive CARLIFE_APP_EXIT\n");
			break;
		default:
			printf("recevive default\n");
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
	printf("sample---------------------------------------->_detach_event\n");
	global_dev	=	dev;
	if( _carlife_status_listener->user_data	==	dev)
	{
		g_free( _carlife_status_listener);
	}
	
	mh_object_unref((MHObject *)dev);

}		/* -----  end of static function _detach_event  ----- */

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char   *_dev_type;
	printf( "Device[ %p ] status[ %d ]\n", dev, event );
	
	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);
	g_message("1111111111111carlife.c---->_dev_type:%p----->%s", _dev_type, _dev_type);	
	if ( _dev_type && !g_strcmp0(_dev_type, "carlife"))
	{
		printf("the carlife device plug in\n");
		bool _exist;
		mh_object_get_properties((MHObject *)dev, "exist", &_exist, NULL);
		if( _exist != 0)
		{
			mh_object_set_properties((MHObject *)dev, "width", 800, "height", 480 , "framerate", 24, NULL);
	g_message("22222222222222carlife.c---->_dev_type:%p----->%s", _dev_type, _dev_type);	
			mh_dev_attach_pb( dev, _pb);
			_carlife_status_listener	=	(MHDevStatusListener*)g_new0( MHDevStatusListener, 1);
			_carlife_status_listener->callback	=	_dev_status_callback;	
			_carlife_status_listener->user_data	=	dev;
			mh_dev_register_status_listener( dev, _carlife_status_listener);

			mh_dev_start( dev );
		}
		else
		{
			printf("the device have no carlife!!!\n\n\n");
		}
	}	
	else
	{
		printf("the device type is not carlife device\n");

	}

	g_message("333333333333333carlife.c---->_dev_type:%p----->%s", _dev_type, _dev_type);	
	//_dev_type is change , continue to check
//	free( _dev_type );
	
	global_dev=dev;
}

void event_arrived( MHCore * core, MHCoreEvent event,const char * type, void * user_data )
{
	switch( event )
	{
	case MH_CORE_STARTED:
		printf( "Media-Hub v2.0 has been started\n" );

		break;
	case MH_CORE_PLUGIN_INVALID:
		printf( "Invalid Media-Hub v2.0 plugin be found\n" );

		break;
	case MH_CORE_PLUGIN_NOT_FOUND:
		printf( "No plugins be found\n" );

		break;
	case MH_CORE_PLUGIN_LOAD_SUCCESS:
		printf( "Success load a plugin---------------->%s\n", type );
		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		printf( "Failed load a plugin\n" );
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
	printf("**********TIME MEASURING BEGIN**********\n");

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
	printf("***********TIME MEASURING END (%d)***********\n", _count); \
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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
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

	printf( "Media-Hub v2.0 Built Time: %s %s\n", mh_built_date(), mh_built_time() );
	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );
	mh_core_start();


	_pb	=	mh_pb_create();	
	mh_object_set_properties( (MHObject *)_pb, "video_sink", "v4l2sink", "surfaceid", 888, NULL);


	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

