/*
 * =====================================================================================
 *
 *       Filename:  carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/29/2015 03:03:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef HS7 // for nagivi
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mh_api.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <mh_carplay.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <glib.h>
//#include <ilm/ilm_control.h>
//#include <ilm_common.h>
#include <libudev.h>
#include <sys/epoll.h>

uint64_t signals[20];
MHDev * carplay, * iap2;
//
//t_ilm_uint layer;
//t_ilm_uint screen;
static int number_of_surfaces;
//t_ilm_uint screenWidth;
//t_ilm_uint screenHeight;
static int tmpx = 0;

MHPb * _pb;
void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data );
pthread_t write_location_tid;
pthread_t write_status_tid;
MHDevStatusListener * _iap_status_listener;

static void * touch_task_epoll( void * user_data );

static void request_ui( void * _, MHObject * object, void * user_data )
{
	/* Shows HMI */
	uint8_t _mode[5]  =   {0};	
	tmpx = 0;
	printf( "Show HMI.tmpx=%d\n",tmpx );
	_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
	_mode[1] =   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
	_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
	_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
	_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
	mh_dev_carplay_change_resource_mode( carplay, _mode );
}

static void modes_changed( void * _, MHDev * object, const char * screen, const char * main_audio, const char * speech_entity, 
		const char * speech_mode, const char * phone, const char * turns, void * user_data )
{
	/* Parse Modes Changed */
	printf( "Modes Changed: screen [ %s ], main_audio [ %s ], speech_entity [ %s ], speech_mode [ %s ], phone [ %s ], turns[ %s ]\n",
			screen, main_audio, speech_entity, speech_mode, phone, turns );
}

static void dev_detach( void * _, MHDev * dev, void * user_data )
{
	/* Device has been removed */
	printf( "CarPlay Device has been removed\n" );

	mh_object_unref((MHObject *)carplay);
	mh_object_unref((MHObject *)iap2);
	g_free( _iap_status_listener);
	
	mh_object_signal_disconnect( signals[0] );
	mh_object_signal_disconnect( signals[1] );
	mh_object_signal_disconnect( signals[2] );
	mh_object_signal_disconnect( signals[3] );

	mh_object_signal_disconnect( signals[4] );
	mh_object_signal_disconnect( signals[5] );

	mh_object_signal_disconnect( signals[6] );
	mh_object_signal_disconnect( signals[7] );

	mh_object_signal_disconnect( signals[8] );
	mh_object_signal_disconnect( signals[9] );

	if( write_location_tid != 0 ) pthread_cancel( write_location_tid );
	if( write_status_tid != 0 ) pthread_cancel( write_status_tid );
}

static void disable_bluetooth( void * _, MHDev * object, const char * device_id, void * user_data )
{
	printf( "Disable Bluetooth: %s\n", device_id );
}

static void duck_audio( void * _, MHDev * object, double durationms, double volume, void * user_data )
{
	printf( "duck_audio: %f %f\n", durationms, volume );
}
static void unduck_audio( void * _, MHDev * object, double durationms, void * user_data )
{
	printf( "unduck_audio: %f \n", durationms );
}

static void audio_info( void * _, MHDev * object, guint status, guint type, guint rate, guint channel, void * user_data )
{
	printf( "audio_info: %d %d %d %d\n", status, type, rate, channel );
}

void * write_location( void * user_data )
{
//	while( 1 )
//	{
////		mh_dev_write_location_data( iap2, "$GPGGA,225833.00,0039.9096,N,0116.397228,E,7,09,0.6,44.4,M,-27.0,M,,*5F" );
//		mh_dev_write_location_data( iap2, "$GPRMC,225833.00,A,3719.951324,N,12201.808796,W,0.0,26.1,060813,0.0,E,A*2F");
//		mh_dev_write_location_data( iap2, "$GPGGA,225833.00,3719.951324,N,12201.808796,W,1,09,0.6,69.8,M,-27.0,M,,*5F");
//		
//		printf("__________ddd\n");
//		usleep( 1000000 );
//	}

	return NULL;
}

void start_location_info( void * _, MHDev * dev, uint8_t location_id, void * user_data )
{
	printf( "location started: %d \n", location_id );
	if( location_id & 0x1)
		printf("GPGGA is support\n");
	if( location_id & 0x2)
		printf("GPRMC is support\n");
	if( location_id & 0x4)
		printf("GPGSV is support\n");
	if( location_id & 0x8)
		printf("PASCD is support\n");
	if( location_id & 0x10)
		printf("PAGCD is support\n");
	if( location_id & 0x20)
		printf("PAACD is support\n");
	if( location_id & 0x40)
		printf("GPHDT is support\n");

//	pthread_create( &write_location_tid, NULL, write_location, NULL );
}

void stop_location_info( void * _, MHDev * dev, void * user_data )
{
	printf( "location stopped\n" );
	pthread_cancel( write_location_tid );

	write_location_tid	=	0;
}

void * write_vehicle_status( void * user_data )
{
	while( 1 )
	{
		mh_dev_send_vehicle_status(iap2, 300, 0xFFFF, 0 );
		sleep( 1 );
	}

	return NULL;
}

void start_vehicle_status_updates( void * _, MHDev * dev, uint8_t status_id, void * user_data )
{
	printf( "status_id started: %d \n", status_id );
	if( status_id & 0x1)
		printf("Range is support\n");
	if( status_id & 0x2)
		printf("OutsideTemperature is support\n");
	if( status_id & 0x4)
		printf("RangeWarning is support\n");

//	pthread_create( &write_status_tid, NULL, write_vehicle_status, NULL );
}

void stop_vehicle_status_updates( void * _, MHDev * dev, void * user_data )
{
	printf( "vehicle status stopped\n" );
	pthread_cancel( write_status_tid );

	write_status_tid	=	0;
}

void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data )
{
	MHPbInfoData * _info;
	if ( pdata != NULL )
	{
		_info = ( MHPbInfoData * )pdata;
	}
				
	switch (type)
	{
		case MH_PB_INFO_PTIME_CHANGE:
			printf( "###     ptime = [%d] duration = [%d]\r", _info->time_info.current_time,_info->time_info.duration );
			fflush( stdout );
			break;
		case MH_PB_INFO_TAG:
			{
				printf( "###     Title = [%s]\n",( char * )_info->tag_info.title );
				printf( "###     Artist = [%s]\n",( char * )_info->tag_info.artist );
				printf( "###     Album = [%s]\n",( char * )_info->tag_info.album );
			}
			break;
		case MH_PB_INFO_EOS:
			printf( "###     MH_PB_INFO_EOS\n" );
			break;
		case MH_PB_INFO_ERROR:
			printf( "###     MH_PB_INFO_ERROR\n" );
			break;
		case MH_PB_INFO_ERROR_NOT_EXIST:
			printf( "###     MH_PB_INFO_ERROR_NOT_EXIST\n" );
			break;
		case MH_PB_INFO_TRACK_TOP:
			{
				printf( "###     index = [%d]\n", _info->track_info.index );
				printf( "###     uri = [%s]\n", ( char * )_info->track_info.uri );
				printf( "###     name = [%s]\n", ( char * )_info->track_info.name );								
			}
			break;
		case MH_PB_INFO_PLAYLIST_CHANGE:
			printf("sample data->playlist   =  [%p]\n ", _info->playlist);
			break;

		case MH_PB_IP_INFO_PTIME_CHANGE:
			printf("###    ipod ptime = [%d]\r", _info->ptime );	
			fflush(stdout);
			break;
		case MH_PB_IP_INFO_QUEUE_INDEX:
			printf("###    ipod index = [%d]\n", _info->index );
			break;
		case MH_PB_IP_INFO_MEDIA:

			printf("###    ipod title = [%s]\n", _info->media_info.title );
			printf("###    ipod rating = [%d]\n", _info->media_info.rating );
			printf("###    ipod duration = [%d]\n", _info->media_info.duration );
			printf("###    ipod album_title = [%s]\n", _info->media_info.album_title );
			printf("###    ipod track = [%d]\n", _info->media_info.track );
			printf("###    ipod track_count = [%d]\n", _info->media_info.track_count );
			printf("###    ipod disc = [%d]\n", _info->media_info.disc );
			printf("###    ipod disc_count = [%d]\n", _info->media_info.disc_count );
			printf("###    ipod artist = [%s]\n", _info->media_info.artist );
			printf("###    ipod album_artist = [%s]\n", _info->media_info.album_artist );
			printf("###    ipod genre = [%s]\n", _info->media_info.genre );
			printf("###    ipod composer = [%s]\n", _info->media_info.composer );
			break;
		case MH_PB_IP_INFO_REPEAT_MODE:
			printf("###    ipod repeat_mode = [%d]\n", _info->repeat_mode );
			break;
		case MH_PB_IP_INFO_SHUFFLE_MODE:
			printf("###    ipod shuffle_mode = [%d]\n", _info->shuffle_mode );
			break;
		case MH_PB_IP_INFO_COVER_PATH:
			printf("###    ipod cover_path = [%s]\n", _info->cover_path );
			break;
		case MH_PB_IP_INFO_APP_NAME:
			printf("###    ipod app_name = [%s]\n", _info->app_name );
			break;
		case MH_PB_IP_INFO_DEVICE_NAME:
			printf("###    ipod device_name = [%s]\n", _info->device_name );
			break;
		case MH_PB_IP_INFO_SHUFFLE_LIST:
			{
				printf("###    shuffle list count = [%d]\n", _info->sf_list_info.list_count );
				int i;
				for(i = 0; i< _info->sf_list_info.list_count; i++)
				{
					printf("_info->sf_list_info.shuffle_seq[%d] = [%d] \n",i,_info->sf_list_info.shuffle_seq[i]);
				}
			}
		case MH_PB_IP_INFO_CALL_STATE_UPDATE:
			printf("\tRemoteID[%s]\n",   		_info->call_state_info.remoteID);
			printf("\tDisplayName[%s]\n", 		_info->call_state_info.displayName);
			printf("\tStatus[%d]\n", 			_info->call_state_info.status);
			printf("\tdirection[%d]\n", 		_info->call_state_info.direction);
			printf("\tcallUUID[%s]\n", 			_info->call_state_info.callUUID);
			printf("\taddressBookID[%s]\n", 	_info->call_state_info.addressBookID);
			printf("\tlabel[%s]\n", 			_info->call_state_info.label);
			printf("\tservice[%d]\n", 			_info->call_state_info.service);
			printf("\tisConferenced[%d]\n", 	_info->call_state_info.isConferenced);
			printf("\tconferenceGroup[%s]\n", 	_info->call_state_info.conferenceGroup);
			printf("\tdisconnectReason[%d]\n", 	_info->call_state_info.disconnectReason);
			break;
		case MH_PB_IP_INFO_RECENTS_LIST_UPDATES:
			{
				printf("\t RecentsListAvailable = [%d]\n",	_info->recentslist_updates.recentsListAvailable);
				printf("\t RecentsListCount = [%d]\n",		_info->recentslist_updates.recentsListCount);
				int i;
				for( i=0; i<_info->recentslist_updates.recentsListCount; i++ )
				{
					printf("_____________________ %d ___________________\n",i);
					printf("\t Index[%d]\n", 			_info->recentslist_updates.recentList[i].index);
					printf("\t RemoteID[%s]\n", 		_info->recentslist_updates.recentList[i].remoteID);	
					printf("\t DisplayName[%s]\n", 		_info->recentslist_updates.recentList[i].displayName);
					printf("\t Label[%s]\n", 			_info->recentslist_updates.recentList[i].label);
					printf("\t AddressBookID[%s]\n", 	_info->recentslist_updates.recentList[i].addressBookID);
					printf("\t Service[%d]\n", 			_info->recentslist_updates.recentList[i].service);
					printf("\t Type[%d]\n", 			_info->recentslist_updates.recentList[i].type);
					printf("\t UnixTimestamp[%lld]\n",	_info->recentslist_updates.recentList[i].unixTimestamp);
					printf("\t Duration[%d]\n", 		_info->recentslist_updates.recentList[i].duration);
					printf("\t Occurrences[%d]\n",		_info->recentslist_updates.recentList[i].occurrences);	
				}
			}
			break;
		case MH_PB_IP_INFO_FAVORITES_LIST_UPDATES:
			{
				printf("\t favoritesListAvailable = [%d]\n",_info->favoriteslist_updates.favoritesListAvailable);
				printf("\t favoritesListCount = [%d]\n",	_info->favoriteslist_updates.favoritesListCount);
				int i;
				for( i=0; i<_info->favoriteslist_updates.favoritesListCount; i++ )
				{
					printf("_____________________ %d ___________________\n",i);
					printf("\t Index[%d]\n", 			_info->favoriteslist_updates.favoritesList[i].index);
					printf("\t RemoteID[%s]\n", 		_info->favoriteslist_updates.favoritesList[i].remoteID);	
					printf("\t DisplayName[%s]\n", 		_info->favoriteslist_updates.favoritesList[i].displayName);
					printf("\t Label[%s]\n", 			_info->favoriteslist_updates.favoritesList[i].label);
					printf("\t AddressBookID[%s]\n", 	_info->favoriteslist_updates.favoritesList[i].addressBookID);
					printf("\t Service[%d]\n", 			_info->favoriteslist_updates.favoritesList[i].service);
				}
			}
			break;
		default:
			break;			
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_callback( MHDev * dev, MHDevStatus status, void * user_data )
{
	switch( status )
	{
		case IAP_AUTH_SUCCESS:
			printf("recevive IAP2_AUTH_SUCCESS\n");
			mh_dev_attach_pb(dev, _pb);
			signals[4]  =   mh_object_signal_connect( ( MHObject * )dev, "start_location_info", ( void * )start_location_info, NULL );
			signals[5]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_location_info", ( void * )stop_location_info, NULL );
			signals[6]  =   mh_object_signal_connect( ( MHObject * )dev, "start_vehicle_status_updates", ( void * )start_vehicle_status_updates, NULL );
			signals[7]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_vehicle_status_updates", ( void * )stop_vehicle_status_updates, NULL );
			break;
		default:
			printf("recevive default\n");
			break;
	}
}		/* -----  end of static function _dev_status_callback  ----- */

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char * _dev_type;
	int iapType;

	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );
	printf("_dev_type = %s\n",_dev_type);
	if( _dev_type && strcmp( _dev_type, "carplay" ) == 0 )
	{
		void * _videoSink	=	NULL;
		mh_object_ref( (MHObject *)dev );

		carplay	=	dev;

		printf( "find a carplay device\n" );

		/* Register CarPlay events listener */
		signals[0]	=	mh_object_signal_connect( ( MHObject * )dev, "request_ui", ( void * )request_ui, NULL );
		signals[1]  =   mh_object_signal_connect( ( MHObject * )dev, "modes_changed", ( void * )modes_changed, NULL );
		signals[2]  =   mh_object_signal_connect( ( MHObject * )dev, "dev_detach", ( void * )dev_detach, NULL );
		signals[3]  =   mh_object_signal_connect( ( MHObject * )dev, "disable_bluetooth", ( void * )disable_bluetooth, NULL );
		signals[8]  =   mh_object_signal_connect( ( MHObject * )dev, "duck_audio", ( void * )duck_audio, NULL );
		signals[9]  =   mh_object_signal_connect( ( MHObject * )dev, "unduck_audio", ( void * )unduck_audio, NULL );
		
		signals[17]  =	 mh_object_signal_connect( ( MHObject * )dev, "audio_info", ( void * )audio_info, NULL );

		mh_object_get_properties( ( MHObject * )dev, "video-sink", &_videoSink, NULL );
	}
	else
	if( _dev_type && strcmp( _dev_type, "iap2" ) == 0 )
	{
		mh_object_get_properties(( MHObject * )dev, "iapType", &iapType, NULL);
		printf("carplay iapType type:%d \n", iapType);
		if (iapType == MISC_IAP_CARPLAY)
		{
			mh_object_ref( (MHObject *)dev );

			iap2	=	dev;

			mh_object_set_properties( (MHObject *)_pb, "video_sink", "v4l2sink", "surfaceid", 888,
//					"window_x",0,"window_y",0,"window_width",800,"window_height",480,
					"streamid","media","siri_streamid","siri","tele_streamid","tele","alt_streamid","alt",NULL);
			mh_misc_set_righthand(0);

			mh_misc_save_pb(_pb);

			uint8_t _mode[12]  =   {0};	
			_mode[0]	=	12;
			_mode[1] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
			_mode[2] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
			_mode[3] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
			_mode[4] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
			_mode[5] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
			_mode[6] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
			_mode[7] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
			_mode[8] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
			_mode[9] 	=   MH_CARPLAY_APPMODE_SPEECH_NOAPP & 0xFF;	
			_mode[10] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
			_mode[11] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	

			mh_misc_carplay_init_modes( _mode );

			_iap_status_listener	=	(MHDevStatusListener*)g_new0( MHDevStatusListener, 1);
			_iap_status_listener->callback	=	_dev_status_callback;	
			_iap_status_listener->user_data	=	dev;
			mh_dev_register_status_listener( dev, _iap_status_listener);
		}
	}
}

void event_arrived( MHCore * core, MHCoreEvent event, const char * type, void * user_data )
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
		printf( "Success load a plugin\n" );
		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		printf( "Failed load a plugin\n" );
		break;
	default:
		break;
	}
}

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

		printf( "%s\n", _path );
		if( g_strrstr( _path, "i2c-0")!= NULL)
		{
			_udevDev    =   udev_device_new_from_syspath( _udevCtx, _path );
			_properties =   udev_device_get_properties_list_entry( _udevDev );
			_model	=	udev_list_entry_get_by_name( _properties, "DEVNAME");
			if( _model)
			{
				_res	=	g_strdup( udev_list_entry_get_value( _model));
				printf("DEVNAME:%s\n", _res);
				break;
			}

		
		}
	}
	return _res;

}		/* -----  end of static function check_input_devices  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  touch_task
 *  Description:  
 * =====================================================================================
 */
static void * touch_task( void * user_data )
{
	int _fd	=	0;
	int _press, _x, _y;
	
//	if( _fd == 0 )
//		_fd	=	open( "/dev/input/event1", O_RDWR );
	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			printf("can't get input path\n");
			return NULL;
		}
		printf("_path:%s\n", _path);
		_fd	=	open( _path, O_RDWR );
		g_free( _path);
	}

	while( 1 )
	{
		struct input_event _event;
		ssize_t _ret	=	read( _fd, &_event, sizeof( struct input_event ));

		if( _ret < 0 )
			perror( "read" );

//		printf( "%d %04X %04X %04X %04X %04X %04X %04X %04X\n", _ret, _event.time.tv_sec & 0xFFFF,
//				_event.time.tv_sec >> 16 , _event.time.tv_usec & 0xFFFF, _event.time.tv_usec >> 16,
//				_event.type, _event.code, _event.value & 0xFFFF, _event.value >> 16 );

		switch( _event.type )
		{
		case EV_ABS:
			switch( _event.code )
			{
			case ABS_MT_TOUCH_MAJOR:
				break;
			case ABS_MT_POSITION_X:
				printf( "x %d", _event.value );
//				_x	=	( _event.value - 32 );
				_x	=	( _event.value  );
				_x	=	(_x * 1280)/720;	
				break;
			case ABS_MT_POSITION_Y:
				printf( "y %d\n", _event.value );
//				_y	=	( _event.value - 32 );
				_y	=	( _event.value );
				_y	=	(_y * 720)/1280;
				break;
			case ABS_MT_TRACKING_ID:
				break;
			case ABS_MT_PRESSURE:
				break;
			default:
				printf( "Uncached ABS event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_SYN:
			switch( _event.code )
			{
			case SYN_REPORT:
				mh_dev_carplay_send_signal_touch( carplay, _press, _x, _y );

				break;
			case SYN_MT_REPORT:
				break;
			default:
				printf( "Uncached SYN event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_KEY:
			switch( _event.code )
			{
			case BTN_TOUCH:
				_press	=	_event.value;
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
 *         Name:  touch_task_epoll
 *  Description:  
 * =====================================================================================
 */
static void * touch_task_epoll( void * user_data )
{
	int _fd	=	0, epfd;
	int _press, _x, _y;
	struct epoll_event event;
	
	printf("touch_task_epoll start\n");

	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			printf("can't get input path\n");
			return NULL;
		}
		printf("_path:%s\n", _path);
		_fd	=	open( _path, O_RDWR | O_NONBLOCK );
		g_free( _path);
	}

	epfd = epoll_create1(EPOLL_CLOEXEC);
	event.events = EPOLLIN;
	event.data.fd = _fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, _fd, &event) < 0) {
		printf("epoll_ctl error\n");
	}

	while( tmpx )
	{

		struct input_event _event;
		ssize_t _ret;

		struct epoll_event ep[32];
		int i, count;

		count = epoll_wait(epfd, ep, (sizeof(ep)/sizeof(ep)[0]), 0);
//		printf("epoll_wait count = %d\n",count);
		if (count < 0)
		{
			printf("epoll_wait error\n");
		}
		else if (count == 0)
		{
		}
		else
		{
	//		printf("epoll_wait count = %d\n",count);
			for(i = 0; i < count; i++)
			{
				if (ep[i].events & EPOLLIN)
				{
					_ret = read( _fd, &_event, sizeof( struct input_event ));

					if( _ret < 0 )
					{
						perror( "read" );
					}
					else{
	//					printf("[EVENT]type = %d, code = %d, value = %d\n", _event.type, _event.code, _event.value);
						switch( _event.type )
						{
							case EV_ABS:
								switch( _event.code )
								{
									case ABS_MT_TOUCH_MAJOR:
										break;
									case ABS_MT_POSITION_X:
	//									printf( "x %d\n", _event.value );
										_x	=	( _event.value  );
								//		_x	=	(_x * 800)/480;	
										break;
									case ABS_MT_POSITION_Y:
	//									printf( "y %d\n", _event.value );
										_y	=	( _event.value );
								//		_y	=	(_y * 480)/800;
										break;
									case ABS_MT_TRACKING_ID:
										if(_event.value<0) {
											_press = 0; // press up
										} else {
											_press = 1; // press down
										}
										break;
									case ABS_MT_PRESSURE:
										break;
									default:
										printf( "Uncached ABS event 0x%02X\n", _event.code );
										break;
								}
								break;
							case EV_SYN:
								switch( _event.code )
								{
									case SYN_REPORT:
										mh_dev_carplay_send_signal_touch( carplay, _press, _x, _y );

										break;
									case SYN_MT_REPORT:
										break;
									default:
										printf( "Uncached SYN event 0x%02X\n", _event.code );
										break;
								}
								break;
							case EV_KEY:
								switch( _event.code )
								{
									case BTN_TOUCH:
							//			_press	=	_event.value;
										break;
									default:
										break;
								}
								break;
						}

					}
				}

			}
		}

	}
	printf("touch_task_epoll close\n");
	close(epfd);
	close(_fd);

	return NULL;
}		/* -----  end of static function touch_task_epoll  ----- */

//static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
//{
//    ilm_surfaceSetDestinationRectangle(id, 0, 0, width, height);
//    printf("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
//	ilm_surfaceSetSourceRectangle(id, 0, 0, width,height);
//
//    printf("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
//    ilm_surfaceSetVisibility(id, ILM_TRUE);
//    printf("SetVisibility          : surface ID (%d), ILM_TRUE\n", id);
//    ilm_layerAddSurface(layer,id);
//    printf("layerAddSurface        : surface ID (%d) is added to layer ID (%d)\n", id, layer);
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
//    struct ilmSurfaceProperties sp;
//
//    if (object == ILM_SURFACE) {
//        if (created) {
//
//
////            if (number_of_surfaces > 0) {
//			if( id == 888){
//                number_of_surfaces--;
//                printf("surface                : %d created\n",id);
//                ilm_getPropertiesOfSurface(id, &sp);
//                if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
//                {   // surface is already configured
//                    configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
//                } else {
//                    // wait for configured event
//                    ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
//                    ilm_commitChanges();
//                }
//            }
//        }
//        else if(!created)
//            printf("surface: %d destroyed\n",id);
//    } else if (object == ILM_LAYER) {
//        if (created)
//            printf("layer: %d created\n",id);
//        else if(!created)
//            printf("layer: %d destroyed\n",id);
//    }
//}
//

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
//	layer	=	888;
//	number_of_surfaces	=	888;
//	 struct ilmScreenProperties screenProperties;
//    struct ilmScreenExtendedProperties screenExtendedProperties;
//
//    ilm_getPropertiesOfScreen(screen, &screenProperties);
//    ilm_getExtendedPropertiesOfScreen(screen, &screenExtendedProperties);
//
//    screenWidth = screenProperties.screenWidth;
//    screenHeight = screenProperties.screenHeight;
//    ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
//    printf("CreateWithDimension          : layer ID (%d), Width (%u), Height (%u)\n", layer, screenWidth, screenHeight);
//    ilm_layerSetDestinationRectangle(layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
//    printf("layerSetDestinationRectangle : layer ID (%d), X (%u), Y (%u), Width (%u), Height (%u)\n", layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
//    ilm_layerSetVisibility(layer,ILM_TRUE);
//    printf("SetVisibility                : layer ID (%d), ILM_TRUE\n", layer);
//
//    t_ilm_int i, length = 0;
//    t_ilm_layer* pArray = NULL;
//    ilm_getLayerIDs(&length, &pArray);
//
//    t_ilm_layer renderOrder[length + 1];
//    renderOrder[length] = layer;

//    for(i=0;i<length;i++) {
//        renderOrder[i] = pArray[i];
//    }
//
//    ilm_displaySetRenderOrder(0,renderOrder,length + 1);
//    ilm_commitChanges();
//    ilm_registerNotification(callbackFunction, NULL);

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
	pthread_t _touch;

	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	mh_ipc_media_client_init( _conn );

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );

	mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY );
	mh_core_start();

	mh_misc_set_bluetoothids("11:22:33:44:55:66");

	tmpx = 1;
	pthread_create( &_touch, NULL, touch_task, NULL );
	pthread_create( &_touch, NULL, touch_task_epoll, NULL );

	_pb = mh_pb_create();

	MHPbEventsListener _listener =
	{
		.callback = _pbs_callback,
		.user_data = NULL  
	};
	mh_pb_register_events_listener( _pb, &_listener );

	uint8_t _mode[12]  =   {0};
	_mode[0]	=	12;
	_mode[1] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;
	_mode[2] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;
	_mode[3] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;
	_mode[4] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;
	_mode[5] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;
	_mode[6] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;
	_mode[7] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;
	_mode[8] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;
	_mode[9] 	=   MH_CARPLAY_APPMODE_SPEECH_NOAPP & 0xFF;
	_mode[10] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;
	_mode[11] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;

	mh_misc_carplay_init_modes( _mode );

	while( 1 )
	{
		char _cmd[128]	=	{0};
		uint8_t _buf[1024]	=	{0};

		read( 0, _cmd, sizeof( _cmd ));

		_cmd[ strlen( _cmd ) - 1 ]	=	0;

		printf("\n\n\n   _cmd = [%s]\n\n\n",_cmd);
		if( strcmp( _cmd, "take screen" ) == 0 )
		{
			g_message("____________________ take screen\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "untake screen" ) == 0 )
		{
			g_message("____________________ untake screen\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_UNTAKE & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "borrow screen" ) == 0 )
		{
			g_message("____________________ borrow screen\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_BORROW & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_NOAPP & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "unborrow screen" ) == 0 )
		{
			g_message("____________________ unborrow screen\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_UNBORROW & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_NOAPP & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "take audio" ) == 0 )
		{
			g_message("____________________ take audio\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_AUDIO & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "untake audio" ) == 0 )
		{
			g_message("____________________ untake audio\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_AUDIO & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_UNTAKE & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "borrow audio" ) == 0 )
		{
			g_message("____________________ borrow audio\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_AUDIO & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_BORROW & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_NOAPP & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "unborrow audio" ) == 0 )
		{
			g_message("____________________ unborrow audio\n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_AUDIO & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_UNBORROW & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_NOAPP & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "iap2" ) == 0 )
		{
			mh_misc_set_iap_device_mode( MISC_IAP );
		}
		else
		if( strcmp( _cmd, "carplay" ) == 0 )
		{
			mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY );
		}
		else
		if( strcmp( _cmd, "start" ) == 0 )
		{
			tmpx = 1;
			pthread_create( &_touch, NULL, touch_task_epoll, NULL );
		}
		else
		if( strcmp( _cmd, "end" ) == 0 )
		{
			tmpx = 0;
		}
		else
		if( strcmp( _cmd, "ui" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "" );
		}
		else
		if( strcmp( _cmd, "ui maps" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "maps:" );
		}
		else
		if( strcmp( _cmd, "ui mobilephone" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "mobilephone:" );
		}
		else
		if( strcmp( _cmd, "ui music" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "music:" );
		}
		else
		if( strcmp( _cmd, "ui nowplaying" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "nowplaying:" );
		}
		else
		if( strncmp( _cmd, "ui tel:", 7 ) == 0 )
		{
			printf("ui tel = %s\n",_cmd + 3);
			mh_dev_carplay_request_ui( carplay, _cmd + 3 );
		}
		else
		if( strcmp( _cmd, "siri prewarm" ) == 0 )
		{
			mh_dev_carplay_request_siri_prewarm( carplay );
		}
		else
		if( strcmp( _cmd, "siri down" ) == 0 )
		{
			mh_dev_carplay_request_siri_button_down( carplay );
		}
		else
		if( strcmp( _cmd, "siri up" ) == 0 )
		{
			mh_dev_carplay_request_siri_button_up( carplay );
		}
		else
		if( strcmp( _cmd, "keyframe" ) == 0 )
		{
			mh_dev_carplay_force_key_frame( carplay );
		}
		else
		if( strcmp( _cmd, "night mode" ) == 0 )
		{
			mh_dev_carplay_set_night_mode( carplay, true );
		}
		else
		if( strcmp( _cmd, "night mode f" ) == 0 )
		{
			mh_dev_carplay_set_night_mode( carplay, false );
		}
		else
		if( strcmp( _cmd, "change app" ) == 0 )
		{
			uint8_t _mode[3]  =   {0};	
			_mode[0] =   MH_CARPLAY_APPMODE_SPEECH_NONE & 0xFF;	
			_mode[1] =   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
			_mode[2] =   MH_CARPLAY_APPMODE_TRUE & 0xFF;		
			mh_dev_carplay_change_app_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "media btn" ) == 0 )
		{
			mh_dev_carplay_send_media_button( carplay, MH_CARPLAY_BTN_PLAY );
		}
		else
		if( strcmp( _cmd, "phone btn" ) == 0 )
		{
			mh_dev_carplay_send_phone_button( carplay,  MH_CARPLAY_PHONE_KEY_ZERO );
			mh_dev_carplay_send_phone_button( carplay,  0 );
		}
		else
		{
			printf( "unknown command: %s\n", _cmd );
		}
	}
	return 0;
}				/* ----------  end of function main  ---------- */

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mh_api.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <mh_carplay.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <glib.h>
#include <ilm/ilm_control.h>
#include <ilm_common.h>
#include <libudev.h>
#include <sys/epoll.h>
#include "fundef.h"

#include <sys/types.h>
//#include <dbus/dbus.h>

uint64_t signals[20];
MHDev * carplay, * iap2, * iap2_bt, * iap2_wifi;
char * serial;
int wifi_connect = 0;

t_ilm_uint layer;
t_ilm_uint screen;
static int number_of_surfaces;
t_ilm_uint screenWidth;
t_ilm_uint screenHeight;

MHPb * _pb;
void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data );
pthread_t write_location_tid;
pthread_t write_status_tid;
MHDevStatusListener * _iap_status_listener;

bool g_bInquiry 			=	false;
bool g_bPairing 			=	false;
bool g_bPBDownloding[2] 	=	{false};
bool g_bSMSDownloding[2]	= 	{false};
bool g_bRunPowerOffProcess	= 	false;
bool g_bBTPowerStatus 		= 	false;
SPP_CONNECTION g_SPPConnectInfo[4];

int g_HFPConnectStatus[2] 	= 	{0};
int g_A2DPConnectStatus 	=	0;
int g_HIDConnectStatus		=	0;
int g_SCOConnectStatus[2]	=	{0};
int g_AVRCPBrowsingConnectStatus = 0;

int   mSearchCarPlayDev = 0;
ANW_BD_ADDR  mCarPlayMacAddr[2];
uint32_t devCount = 0;

unsigned int mCarPlayIndex;

#define DEBUG_HEX_DISPLAY(D, L) \
{ \
	char _tmpChar[100] = {0}; \
	int _cc, i; \
\
	printf( "%lld\n",(long long int) (L) );\
	fprintf(stdout, "*******HEX DISPLAY BEGIN (%03lld bytes)******\n",(long long int) (L));\
\
	for(_cc = 0; _cc < (L); _cc += 16) \
	{ \
		memset(_tmpChar, 0, sizeof(_tmpChar)); \
\
		for(i = 0; i < 16 && i + _cc < (L); i ++) \
		{ \
			_tmpChar[i * 3]     =   ((D)[_cc + i] >> 4)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 1] =   ((D)[_cc + i] & 0x0F)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 2] =   ' '; \
		} \
\
		fprintf(stdout, "\t%s\n",  _tmpChar); \
	} \
	fprintf(stdout, "************HEX DISPLAY END*************\n");\
}

//static DBusConnection *connection;


//void * station_print(void * user_data)
//static int station_print(DBusMessageIter *iter)
//{
////	DBusMessageIter  * iter = ( DBusMessageIter  *)user_data;
//
//	DBusMessageIter array;
//
//	dbus_message_iter_recurse(iter, &array);
//
//	int _status = 0;
//	int wifi_iap2 = wifi_connect;	
//
//	g_message("%s  ------------------------------------------scan start\n",__func__);
//	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRUCT) {
//		
//		g_message("%s  ------------------------------------------find\n",__func__);
//		DBusMessageIter entry, dict, key_value;
//
//		dbus_message_iter_recurse(&array, &entry);
//
//		dbus_message_iter_recurse(&entry, &dict);
//
//		{// print dict entry
//			DBusMessageIter* dict_p = &dict;
//			DBusMessageIter* key_value_p = &key_value;
//
//			fprintf(stdout, "*Station\n");
//
//			while (dbus_message_iter_get_arg_type(dict_p) != DBUS_TYPE_INVALID)
//			{
//				dbus_message_iter_recurse(dict_p, key_value_p);
//				{
//					char *str;
//					dbus_message_iter_get_basic(key_value_p, &str);
//					fprintf(stdout, "  %s = ", str);
//					dbus_message_iter_next(key_value_p);
//					if(!strcmp(str, "IP"))
//					{
//						DBusMessageIter value;
//						DBusMessageIter* value_p = &value;
//						dbus_message_iter_recurse(key_value_p, value_p);
//						{
//							unsigned int ip;
//							dbus_message_iter_get_basic(value_p, &ip);
//							fprintf(stdout, "%d.%d.%d.%d\n", ((ip & 0xff000000) >> 24), ((ip & 0x00ff0000) >> 16), ((ip & 0x0000ff00) >> 8), (ip & 0x000000ff));
//							dbus_message_iter_next(value_p);
//						}
//						dbus_message_iter_next(key_value_p);
//					}
//					else if(!strcmp(str, "Expire"))
//					{
//						DBusMessageIter value;
//						DBusMessageIter* value_p = &value;
//						dbus_message_iter_recurse(key_value_p, value_p);
//						{
//							time_t expire;
//							dbus_message_iter_get_basic(value_p, &expire);
//							fprintf(stdout, "%s", ctime(&expire));
//							dbus_message_iter_next(value_p);
//						}
//						dbus_message_iter_next(key_value_p);
//					}
//					else if(!strcmp(str, "Mac"))
//					{
//						DBusMessageIter value;
//						DBusMessageIter* value_p = &value;
//						dbus_message_iter_recurse(key_value_p, value_p);
//						{
//							char *mac_string;
//							dbus_message_iter_get_basic(value_p, &mac_string);
//							fprintf(stdout, "%s\n", mac_string);
//							dbus_message_iter_next(value_p);
//
//							if( wifi_connect == 1 )
//							{
//								g_message("mac_string = %s serial 11 = %s\n",mac_string ,serial);
//								if( strcmp(mac_string, serial) == 0)
//								{
//									g_message("wifi connected");
//									_status	=	1;
//								}
//							}
//						}
//						dbus_message_iter_next(key_value_p);
//					}
//					else if(!strcmp(str, "ID"))
//					{
//						DBusMessageIter value;
//						DBusMessageIter* value_p = &value;
//						dbus_message_iter_recurse(key_value_p, value_p);
//						{
//							char *id_string;
//							dbus_message_iter_get_basic(value_p, &id_string);
//							fprintf(stdout, "%s\n", id_string);
//							dbus_message_iter_next(value_p);
//						}
//						dbus_message_iter_next(key_value_p);
//					}
//					else
//					{
//					}
//				}
//				dbus_message_iter_next(dict_p);
//			}
//		}
//
//		fprintf(stdout, "\n");
//		dbus_message_iter_next(&array);
//	}
//	if(( _status == 0 )&&( wifi_iap2 == 1 ))
//	{
////		iap2_wifi =	NULL;
//		wifi_connect	=	0;
//		g_message("_______________detech wifi carplay_________________________\n");	
//		MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
//		_devParam->type				=	MH_DEV_WIFI_IAP;
//		_devParam->mac_addr			=	g_strdup( serial );
//		_devParam->connect_status	=	0;	
//
//		mh_core_find_dev( _devParam );
//
//		g_free( _devParam->mac_addr );
//		g_free( _devParam );
//	}
//
//	g_message("%s  ------------------------------------------end\n",__func__);
//
//	return 0;
//}

static void request_ui( void * _, MHObject * object, void * user_data )
{
	/* Shows HMI */
	printf( "Request UI\n" );
}

static void modes_changed( void * _, MHDev * object, const char * screen, const char * main_audio, const char * speech_entity, 
		const char * speech_mode, const char * phone, const char * turns, void * user_data )
{
	/* Parse Modes Changed */
	printf( "Modes Changed: screen [ %s ], main_audio [ %s ], speech_entity [ %s ], speech_mode [ %s ], phone [ %s ], turns[ %s ]\n",
			screen, main_audio, speech_entity, speech_mode, phone, turns );
}
static void _detach_event(	MHDev * dev, void * user_data)
{
	printf( "iap2 Device has been removed\n" );

	char * _dev_type;
	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );

	printf("detach iap2 dev type  = %s\n",_dev_type);

	if( _dev_type && strcmp( _dev_type, "iap2_bt" ) == 0 )
{
		char * _mac;
		mh_object_get_properties( ( MHObject * )dev, "bt_mac_address", &_mac, NULL );
		g_message("detach dev mac = %s\n",_mac);
		g_message("detach dev iap2_bt = %p\n",iap2_bt);
		mh_object_unref((MHObject *)iap2_bt);

		mh_object_signal_disconnect( signals[10] );
		mh_object_signal_disconnect( signals[11] );
		mh_object_signal_disconnect( signals[12] );
	}

	if(( _dev_type && strcmp( _dev_type, "iap2_wifi" ) == 0 )
			|| ( _dev_type && strcmp( _dev_type, "iap2" ) == 0 ))
	{
		if(strcmp( _dev_type, "iap2" ) == 0 )
		{
			g_message("detach dev usb %p\n",iap2);
	mh_object_unref((MHObject *)iap2);

	mh_object_signal_disconnect( signals[4] );
	mh_object_signal_disconnect( signals[5] );

	mh_object_signal_disconnect( signals[6] );
	mh_object_signal_disconnect( signals[7] );
		}
		
		if(strcmp( _dev_type, "iap2_wifi" ) == 0 )
		{
			g_message("detach dev wifi %p\n",iap2_wifi);
			mh_object_unref((MHObject *)iap2_wifi );
			mh_object_signal_disconnect( signals[13] );
			mh_object_signal_disconnect( signals[14] );

			mh_object_signal_disconnect( signals[15] );
			mh_object_signal_disconnect( signals[16] );
		}

//		g_free( _iap_status_listener);

	if( write_location_tid != 0 ) pthread_cancel( write_location_tid );
	if( write_status_tid != 0 ) pthread_cancel( write_status_tid );
}

	if(( _dev_type && strcmp( _dev_type, "carplay" ) == 0 )
			|| ( _dev_type && strcmp( _dev_type, "carplay_wifi" ) == 0 ))
	{
		g_message("detach dev carplay %p\n",carplay);
		mh_object_unref((MHObject *)carplay);

		mh_object_signal_disconnect( signals[0] );
		mh_object_signal_disconnect( signals[1] );
//		mh_object_signal_disconnect( signals[2] );
		mh_object_signal_disconnect( signals[3] );

		mh_object_signal_disconnect( signals[8] );
		mh_object_signal_disconnect( signals[9] );
	}	
}

//static void dev_detach( void * _, MHDev * dev, void * user_data )
//{
//	/* Device has been removed */
//	printf( "CarPlay Device has been removed\n" );
//
//	char * _dev_type;
//	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );
//	printf("detach dev type  = %s\n",_dev_type);
//	
//	if( _dev_type && strcmp( _dev_type, "carplay" ) == 0 )
//	{
//		mh_object_unref((MHObject *)carplay);
//
//		mh_object_signal_disconnect( signals[0] );
//		mh_object_signal_disconnect( signals[1] );
//		mh_object_signal_disconnect( signals[2] );
//		mh_object_signal_disconnect( signals[3] );
//
//		mh_object_signal_disconnect( signals[8] );
//		mh_object_signal_disconnect( signals[9] );
//	}	
//}

static void disable_bluetooth( void * _, MHDev * object, const char * device_id, void * user_data )
{
	printf( "Disable Bluetooth: %s\n", device_id );
	PL_BTPower(0);
}

static void duck_audio( void * _, MHDev * object, double durationms, double volume, void * user_data )
{
	printf( "duck_audio: %f %f\n", durationms, volume );
}
static void unduck_audio( void * _, MHDev * object, double durationms, void * user_data )
{
	printf( "unduck_audio: %f \n", durationms );
}

static void audio_info( void * _, MHDev * object, guint status, guint type, guint rate, guint channel, void * user_data )
{
	printf( "audio_info: %d %d %d %d\n", status, type, rate, channel );
}

void * write_location( void * user_data )
{
//	while( 1 )
//	{
////		mh_dev_write_location_data( iap2, "$GPGGA,225833.00,0039.9096,N,0116.397228,E,7,09,0.6,44.4,M,-27.0,M,,*5F" );
//		mh_dev_write_location_data( iap2, "$GPRMC,225833.00,A,3719.951324,N,12201.808796,W,0.0,26.1,060813,0.0,E,A*2F");
//		mh_dev_write_location_data( iap2, "$GPGGA,225833.00,3719.951324,N,12201.808796,W,1,09,0.6,69.8,M,-27.0,M,,*5F");
//		
//		printf("__________ddd\n");
//		usleep( 1000000 );
//	}

	return NULL;
}

void start_location_info( void * _, MHDev * dev, uint8_t location_id, void * user_data )
{
	printf( "location started: %d \n", location_id );
	if( location_id & 0x1)
		printf("GPGGA is support\n");
	if( location_id & 0x2)
		printf("GPRMC is support\n");
	if( location_id & 0x4)
		printf("GPGSV is support\n");
	if( location_id & 0x8)
		printf("PASCD is support\n");
	if( location_id & 0x10)
		printf("PAGCD is support\n");
	if( location_id & 0x20)
		printf("PAACD is support\n");
	if( location_id & 0x40)
		printf("GPHDT is support\n");

//	pthread_create( &write_location_tid, NULL, write_location, NULL );
}

void stop_location_info( void * _, MHDev * dev, void * user_data )
{
	printf( "location stopped\n" );
	pthread_cancel( write_location_tid );

	write_location_tid	=	0;
}

void * write_vehicle_status( void * user_data )
{
	while( 1 )
	{
		mh_dev_send_vehicle_status(iap2_wifi, 300, 0xFFFF, 0 );
		sleep( 1 );
	}

	return NULL;
}

void start_vehicle_status_updates( void * _, MHDev * dev, uint8_t status_id, void * user_data )
{
	printf( "status_id started: %d \n", status_id );
	if( status_id & 0x1)
		printf("Range is support\n");
	if( status_id & 0x2)
		printf("OutsideTemperature is support\n");
	if( status_id & 0x4)
		printf("RangeWarning is support\n");

//	pthread_create( &write_status_tid, NULL, write_vehicle_status, NULL );
}

void stop_vehicle_status_updates( void * _, MHDev * dev, void * user_data )
{
	printf( "vehicle status stopped\n" );
	pthread_cancel( write_status_tid );

	write_status_tid	=	0;
}

void req_accessory_wifi_conf_info( void * _, MHDev * dev, void * user_data )
{
	printf( "req_Accessory_wifi_conf_info\n" );	

	uint8_t _ssid[] = {0x06, 'A', 'P', '1', '2', '3'};	
	uint8_t _pass[] = {0x09, 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a' };

	mh_dev_send_wifi_conf_info( iap2_bt, _ssid, _pass, 2 , 149 );
}

void wifi_carplay_update( void * _, MHDev * dev, uint8_t status_id, void * user_data )
{
	printf( "wifi carplay status_id : %d \n", status_id );
//	pthread_create( &write_status_tid, NULL, write_vehicle_status, NULL );
}

void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data )
{
	MHPbInfoData * _info;
	if ( pdata != NULL )
	{
		_info = ( MHPbInfoData * )pdata;
	}
				
	switch (type)
	{
		case MH_PB_INFO_PTIME_CHANGE:
			printf( "###     ptime = [%d] duration = [%d]\r", _info->time_info.current_time,_info->time_info.duration );
			fflush( stdout );
			break;
		case MH_PB_INFO_TAG:
			{
				printf( "###     Title = [%s]\n",( char * )_info->tag_info.title );
				printf( "###     Artist = [%s]\n",( char * )_info->tag_info.artist );
				printf( "###     Album = [%s]\n",( char * )_info->tag_info.album );
			}
			break;
		case MH_PB_INFO_EOS:
			printf( "###     MH_PB_INFO_EOS\n" );
			break;
		case MH_PB_INFO_ERROR:
			printf( "###     MH_PB_INFO_ERROR\n" );
			break;
		case MH_PB_INFO_ERROR_NOT_EXIST:
			printf( "###     MH_PB_INFO_ERROR_NOT_EXIST\n" );
			break;
		case MH_PB_INFO_TRACK_TOP:
			{
				printf( "###     index = [%d]\n", _info->track_info.index );
				printf( "###     uri = [%s]\n", ( char * )_info->track_info.uri );
				printf( "###     name = [%s]\n", ( char * )_info->track_info.name );								
			}
			break;
		case MH_PB_INFO_PLAYLIST_CHANGE:
			printf("sample data->playlist   =  [%p]\n ", _info->playlist);
			break;

		case MH_PB_IP_INFO_PTIME_CHANGE:
			printf("###    ipod ptime = [%d]\r", _info->ptime );	
			fflush(stdout);
			break;
		case MH_PB_IP_INFO_QUEUE_INDEX:
			printf("###    ipod index = [%d]\n", _info->index );
			break;
		case MH_PB_IP_INFO_MEDIA:

			printf("###    ipod title = [%s]\n", _info->media_info.title );
			printf("###    ipod rating = [%d]\n", _info->media_info.rating );
			printf("###    ipod duration = [%d]\n", _info->media_info.duration );
			printf("###    ipod album_title = [%s]\n", _info->media_info.album_title );
			printf("###    ipod track = [%d]\n", _info->media_info.track );
			printf("###    ipod track_count = [%d]\n", _info->media_info.track_count );
			printf("###    ipod disc = [%d]\n", _info->media_info.disc );
			printf("###    ipod disc_count = [%d]\n", _info->media_info.disc_count );
			printf("###    ipod artist = [%s]\n", _info->media_info.artist );
			printf("###    ipod album_artist = [%s]\n", _info->media_info.album_artist );
			printf("###    ipod genre = [%s]\n", _info->media_info.genre );
			printf("###    ipod composer = [%s]\n", _info->media_info.composer );
			break;
		case MH_PB_IP_INFO_REPEAT_MODE:
			printf("###    ipod repeat_mode = [%d]\n", _info->repeat_mode );
			break;
		case MH_PB_IP_INFO_SHUFFLE_MODE:
			printf("###    ipod shuffle_mode = [%d]\n", _info->shuffle_mode );
			break;
		case MH_PB_IP_INFO_COVER_PATH:
			printf("###    ipod cover_path = [%s]\n", _info->cover_path );
			break;
		case MH_PB_IP_INFO_APP_NAME:
			printf("###    ipod app_name = [%s]\n", _info->app_name );
			break;
		case MH_PB_IP_INFO_DEVICE_NAME:
			printf("###    ipod device_name = [%s]\n", _info->device_name );
			break;
		case MH_PB_IP_INFO_SHUFFLE_LIST:
			{
				printf("###    shuffle list count = [%d]\n", _info->sf_list_info.list_count );
				int i;
				for(i = 0; i< _info->sf_list_info.list_count; i++)
				{
					printf("_info->sf_list_info.shuffle_seq[%d] = [%d] \n",i,_info->sf_list_info.shuffle_seq[i]);
				}
			}
		case MH_PB_IP_INFO_CALL_STATE_UPDATE:
			printf("\tRemoteID[%s]\n",   		_info->call_state_info.remoteID);
			printf("\tDisplayName[%s]\n", 		_info->call_state_info.displayName);
			printf("\tStatus[%d]\n", 			_info->call_state_info.status);
			printf("\tdirection[%d]\n", 		_info->call_state_info.direction);
			printf("\tcallUUID[%s]\n", 			_info->call_state_info.callUUID);
			printf("\taddressBookID[%s]\n", 	_info->call_state_info.addressBookID);
			printf("\tlabel[%s]\n", 			_info->call_state_info.label);
			printf("\tservice[%d]\n", 			_info->call_state_info.service);
			printf("\tisConferenced[%d]\n", 	_info->call_state_info.isConferenced);
			printf("\tconferenceGroup[%s]\n", 	_info->call_state_info.conferenceGroup);
			printf("\tdisconnectReason[%d]\n", 	_info->call_state_info.disconnectReason);
			break;
		case MH_PB_IP_INFO_RECENTS_LIST_UPDATES:
			{
				printf("\t RecentsListAvailable = [%d]\n",	_info->recentslist_updates.recentsListAvailable);
				printf("\t RecentsListCount = [%d]\n",		_info->recentslist_updates.recentsListCount);
				int i;
				for( i=0; i<_info->recentslist_updates.recentsListCount; i++ )
				{
					printf("_____________________ %d ___________________\n",i);
					printf("\t Index[%d]\n", 			_info->recentslist_updates.recentList[i].index);
					printf("\t RemoteID[%s]\n", 		_info->recentslist_updates.recentList[i].remoteID);	
					printf("\t DisplayName[%s]\n", 		_info->recentslist_updates.recentList[i].displayName);
					printf("\t Label[%s]\n", 			_info->recentslist_updates.recentList[i].label);
					printf("\t AddressBookID[%s]\n", 	_info->recentslist_updates.recentList[i].addressBookID);
					printf("\t Service[%d]\n", 			_info->recentslist_updates.recentList[i].service);
					printf("\t Type[%d]\n", 			_info->recentslist_updates.recentList[i].type);
					printf("\t UnixTimestamp[%lld]\n",	_info->recentslist_updates.recentList[i].unixTimestamp);
					printf("\t Duration[%d]\n", 		_info->recentslist_updates.recentList[i].duration);
					printf("\t Occurrences[%d]\n",		_info->recentslist_updates.recentList[i].occurrences);	
				}
			}
			break;
		case MH_PB_IP_INFO_FAVORITES_LIST_UPDATES:
			{
				printf("\t favoritesListAvailable = [%d]\n",_info->favoriteslist_updates.favoritesListAvailable);
				printf("\t favoritesListCount = [%d]\n",	_info->favoriteslist_updates.favoritesListCount);
				int i;
				for( i=0; i<_info->favoriteslist_updates.favoritesListCount; i++ )
				{
					printf("_____________________ %d ___________________\n",i);
					printf("\t Index[%d]\n", 			_info->favoriteslist_updates.favoritesList[i].index);
					printf("\t RemoteID[%s]\n", 		_info->favoriteslist_updates.favoritesList[i].remoteID);	
					printf("\t DisplayName[%s]\n", 		_info->favoriteslist_updates.favoritesList[i].displayName);
					printf("\t Label[%s]\n", 			_info->favoriteslist_updates.favoritesList[i].label);
					printf("\t AddressBookID[%s]\n", 	_info->favoriteslist_updates.favoritesList[i].addressBookID);
					printf("\t Service[%d]\n", 			_info->favoriteslist_updates.favoritesList[i].service);
				}
			}
			break;
		default:
			break;			
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_callback( MHDev * dev, MHDevStatus status, void * user_data )
{
	switch( status )
	{
		case IAP_AUTH_SUCCESS:
			printf("recevive IAP2_AUTH_SUCCESS\n");
			char * _dev_type;
			mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );
			printf("_dev_status_callback _dev_type = %s\n",_dev_type);

		if(  strcmp( _dev_type, "iap2_wifi" ) != 0 )
		{
			mh_dev_attach_pb(dev, _pb);
			signals[4]  =   mh_object_signal_connect( ( MHObject * )dev, "start_location_info", ( void * )start_location_info, NULL );
			signals[5]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_location_info", ( void * )stop_location_info, NULL );
			signals[6]  =   mh_object_signal_connect( ( MHObject * )dev, "start_vehicle_status_updates", ( void * )start_vehicle_status_updates, NULL );
			signals[7]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_vehicle_status_updates", ( void * )stop_vehicle_status_updates, NULL );
		}else{
			mh_dev_attach_pb(dev, _pb);
			signals[13]  =   mh_object_signal_connect( ( MHObject * )dev, "start_location_info", ( void * )start_location_info, NULL );
			signals[14]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_location_info", ( void * )stop_location_info, NULL );
			signals[15]  =   mh_object_signal_connect( ( MHObject * )dev, "start_vehicle_status_updates", ( void * )start_vehicle_status_updates, NULL );
			signals[16]  =   mh_object_signal_connect( ( MHObject * )dev, "stop_vehicle_status_updates", ( void * )stop_vehicle_status_updates, NULL );
		}
//			signals[13]  =   mh_object_signal_connect( ( MHObject * )dev, "dev_detach", ( void * )dev_detach, NULL );
//			signals[10]  =   mh_object_signal_connect( ( MHObject * )dev, "req_accessory_wifi_conf_info", ( void * )req_accessory_wifi_conf_info, NULL );
//			signals[11]  =   mh_object_signal_connect( ( MHObject * )dev, "wifi_carplay_update", ( void * )wifi_carplay_update, NULL );
			break;
		case IAP_AUTH_FAILED:
			g_message("recevive IAP_AUTH_FAILED\n");		
			break;
		default:
			printf("recevive default\n");
			break;
	}
}		/* -----  end of static function _dev_status_callback  ----- */


void bt_data( void * _, MHDev * dev, GVariant * var, void * user_data )
{
	GVariant * _var	=	g_variant_get_variant( var );
	const uint8_t * _buf;
	gsize _len	=	0;
	int nWritten =0;
	
	_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( uint8_t ));
	uint8_t * _bt_buf;	

	if( _len > 0x28A )
	{
		int _len_p = 0;
		int _last_len = _len%0x28A; 

		for( int i = 0; i< _len/0x28A; i++  )
		{
			_bt_buf	=	g_malloc0( 0x28A );

			memcpy( _bt_buf, _buf + _len_p, 0x28A);
			g_message("bt_data  ************************** for start \n");
			DEBUG_HEX_DISPLAY( _bt_buf,  0x28A);
			g_message("bt_data  ************************** for end \n");
			
			ANW_SPPWrite(mCarPlayIndex, _bt_buf, 0x28A , &nWritten);
			g_free( _bt_buf );
	
			_len_p += 0x28A; 
		}
		
		if( _last_len != 0 )
		{
			_bt_buf	=	g_malloc0( _last_len );

			memcpy( _bt_buf, _buf + _len_p, _last_len);
			g_message("bt_data  ************************** if start \n");
			DEBUG_HEX_DISPLAY( _bt_buf,  _last_len);
			g_message("bt_data  ************************** if end \n");

			ANW_SPPWrite(mCarPlayIndex, _bt_buf, _last_len , &nWritten);
			g_free( _bt_buf );
		}

	}else{
		_bt_buf	=	g_malloc0( _len );

		memcpy( _bt_buf, _buf, _len);
		g_message("bt_data  ************************** start _len = %d  %d\n",_len,sizeof( _bt_buf));
		DEBUG_HEX_DISPLAY( _bt_buf,  _len);
		g_message("bt_data  ************************** end\n");
		ANW_SPPWrite(mCarPlayIndex, _bt_buf, _len , &nWritten);

		g_free( _bt_buf );
	}
	g_variant_unref( _var );
}

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char * _dev_type;

	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );
	printf("_dev_type = %s\n",_dev_type);
	if(( _dev_type && strcmp( _dev_type, "carplay" ) == 0 )
			|| ( _dev_type && strcmp( _dev_type, "carplay_wifi" ) == 0 ))
	{
		void * _videoSink	=	NULL;
		mh_object_ref( (MHObject *)dev );

		carplay	=	dev;

		printf( "find a carplay device\n" );

		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
		mh_dev_register_detach_listener( dev, & _detach_listener);

		/* Register CarPlay events listener */
		signals[0]	=	mh_object_signal_connect( ( MHObject * )dev, "request_ui", ( void * )request_ui, NULL );
		signals[1]  =   mh_object_signal_connect( ( MHObject * )dev, "modes_changed", ( void * )modes_changed, NULL );
//		signals[2]  =   mh_object_signal_connect( ( MHObject * )dev, "dev_detach", ( void * )dev_detach, NULL );
		signals[3]  =   mh_object_signal_connect( ( MHObject * )dev, "disable_bluetooth", ( void * )disable_bluetooth, NULL );
		signals[8]  =   mh_object_signal_connect( ( MHObject * )dev, "duck_audio", ( void * )duck_audio, NULL );
		signals[9]  =   mh_object_signal_connect( ( MHObject * )dev, "unduck_audio", ( void * )unduck_audio, NULL );

		signals[17]  =   mh_object_signal_connect( ( MHObject * )dev, "audio_info", ( void * )audio_info, NULL );

		mh_object_get_properties( ( MHObject * )dev, "video-sink", &_videoSink, NULL );
	}
	else
	if(( _dev_type && strcmp( _dev_type, "iap2" ) == 0 )
		|| ( _dev_type && strcmp( _dev_type, "iap2_bt" ) == 0 )
		|| ( _dev_type && strcmp( _dev_type, "iap2_wifi" ) == 0 ))
	{
		mh_object_ref( (MHObject *)dev );
		
//		iap2	=	dev;
		
		if(  strcmp( _dev_type, "iap2" ) == 0 )
		iap2	=	dev;

		if( strcmp( _dev_type, "iap2_bt" ) == 0 )
		{
			char * _mac;
			mh_object_get_properties( ( MHObject * )dev, "bt_mac_address", &_mac, NULL );
			g_message("_mac = %s\n",_mac);

			iap2_bt	=	dev;
		
			signals[10]  =   mh_object_signal_connect( ( MHObject * )dev, "req_accessory_wifi_conf_info", ( void * )req_accessory_wifi_conf_info, NULL );
			signals[11]  =   mh_object_signal_connect( ( MHObject * )dev, "wifi_carplay_update", ( void * )wifi_carplay_update, NULL );
			signals[12]  =	 mh_object_signal_connect( ( MHObject * )dev, "bt_data", bt_data, NULL );
		}

		if(  strcmp( _dev_type, "iap2_wifi" ) == 0 )
		{
			iap2_wifi	=	dev;
			wifi_connect	=	1;
			mh_object_get_properties( ( MHObject * )dev, "serial", &serial, NULL );
			printf( "wifi properties:\nserial:\t%s\n", serial );
		}
		if(( strcmp( _dev_type, "iap2_bt" ) == 0 )
				|| ( strcmp( _dev_type, "iap2" ) == 0 ))
		{
		mh_object_set_properties( (MHObject *)_pb, "video_sink", "glimagesink", "surfaceid", 888,
				"window_x",0,"window_y",0,"window_width",1280,"window_height",720,
				"streamid","media","siri_streamid","siri","tele_streamid","tele","alt_streamid","alt",NULL);

		mh_misc_set_righthand(1);

		mh_misc_save_pb(_pb);

		uint8_t _mode[12]  =   {0};	
		_mode[0]	=	12;
		_mode[1] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
		_mode[2] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
		_mode[3] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
		_mode[4] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
		_mode[5] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
		_mode[6] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
		_mode[7] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
		_mode[8] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
		_mode[9] 	=   MH_CARPLAY_APPMODE_SPEECH_NOAPP & 0xFF;	
		_mode[10] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
		_mode[11] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	

		mh_misc_carplay_init_modes( _mode );
		}
		if( strcmp( _dev_type, "iap2_bt" ) != 0 )
		{
		_iap_status_listener	=	(MHDevStatusListener*)g_new0( MHDevStatusListener, 1);
		_iap_status_listener->callback	=	_dev_status_callback;	
		_iap_status_listener->user_data	=	dev;
		mh_dev_register_status_listener( dev, _iap_status_listener);
	}

		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
		mh_dev_register_detach_listener( dev, & _detach_listener);
	}
}

void event_arrived( MHCore * core, MHCoreEvent event, const char * type, void * user_data )
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
		printf( "Success load a plugin\n" );
		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		printf( "Failed load a plugin\n" );
		break;
	default:
		break;
	}
}

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

		printf( "%s\n", _path );
		if( g_strrstr( _path, "i2c-0")!= NULL)
		{
			_udevDev    =   udev_device_new_from_syspath( _udevCtx, _path );
			_properties =   udev_device_get_properties_list_entry( _udevDev );
			_model	=	udev_list_entry_get_by_name( _properties, "DEVNAME");
			if( _model)
			{
				_res	=	g_strdup( udev_list_entry_get_value( _model));
				printf("DEVNAME:%s\n", _res);
				break;
			}

		
		}
	}
	return _res;

}		/* -----  end of static function check_input_devices  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  touch_task
 *  Description:  
 * =====================================================================================
 */
static void * touch_task( void * user_data )
{
	int _fd	=	0;
	int _press, _x, _y;
	
//	if( _fd == 0 )
//		_fd	=	open( "/dev/input/event1", O_RDWR );
	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			printf("can't get input path\n");
			return NULL;
		}
		printf("_path:%s\n", _path);
		_fd	=	open( _path, O_RDWR );
		g_free( _path);
	}

	while( 1 )
	{
		struct input_event _event;
		ssize_t _ret	=	read( _fd, &_event, sizeof( struct input_event ));

		if( _ret < 0 )
			perror( "read" );

//		printf( "%d %04X %04X %04X %04X %04X %04X %04X %04X\n", _ret, _event.time.tv_sec & 0xFFFF,
//				_event.time.tv_sec >> 16 , _event.time.tv_usec & 0xFFFF, _event.time.tv_usec >> 16,
//				_event.type, _event.code, _event.value & 0xFFFF, _event.value >> 16 );

		switch( _event.type )
		{
		case EV_ABS:
			switch( _event.code )
			{
			case ABS_MT_TOUCH_MAJOR:
				break;
			case ABS_MT_POSITION_X:
				printf( "x %d", _event.value );
//				_x	=	( _event.value - 32 );
							_x	=	( _event.value  );
				_x	=	(_x * 1280)/720;	
				break;
			case ABS_MT_POSITION_Y:
				printf( "y %d\n", _event.value );
//				_y	=	( _event.value - 32 );
					_y	=	( _event.value );
				_y	=	(_y * 720)/1280;
				break;
			case ABS_MT_TRACKING_ID:
				break;
			case ABS_MT_PRESSURE:
				break;
			default:
				printf( "Uncached ABS event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_SYN:
			switch( _event.code )
			{
			case SYN_REPORT:
				mh_dev_carplay_send_signal_touch( carplay, _press, _x, _y );

				break;
			case SYN_MT_REPORT:
				break;
			default:
				printf( "Uncached SYN event 0x%02X\n", _event.code );
				break;
			}
			break;
		case EV_KEY:
			switch( _event.code )
			{
			case BTN_TOUCH:
				_press	=	_event.value;
				break;
			default:
				break;
			}
			break;
		}
	}

	return NULL;
}		/* -----  end of static function touch_task  ----- */

int tmpx = 0;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  touch_task_epoll
 *  Description:  
 * =====================================================================================
 */
static void * touch_task_epoll( void * user_data )
{
	int _fd	=	0, epfd;
	int _press, _x, _y;
	struct epoll_event event;
	
	printf("touch_task_epoll start\n");

	if( _fd == 0 )
	{
		char * _path	=	NULL;
		_path	=	check_input_devices();
		if( _path	==	NULL)
		{
			printf("can't get input path\n");
			return NULL;
		}
		printf("_path:%s\n", _path);
		_fd	=	open( _path, O_RDWR | O_NONBLOCK );
		g_free( _path);
	}

	epfd = epoll_create1(EPOLL_CLOEXEC);
	event.events = EPOLLIN;
	event.data.fd = _fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, _fd, &event) < 0) {
		printf("epoll_ctl error\n");
	}

	while( tmpx )
	{

		struct input_event _event;
		ssize_t _ret;

		struct epoll_event ep[32];
		int i, count;

		count = epoll_wait(epfd, ep, (sizeof(ep)/sizeof(ep)[0]), 0);
//		printf("epoll_wait count = %d\n",count);
		if (count < 0)
		{
			printf("epoll_wait error\n");
		}
		else if (count == 0)
		{
		}
		else
		{
			printf("epoll_wait count = %d\n",count);
			for(i = 0; i < count; i++)
			{
				if (ep[i].events & EPOLLIN)
				{
					_ret = read( _fd, &_event, sizeof( struct input_event ));

					if( _ret < 0 )
					{
						perror( "read" );
					}
					else{
						printf("_event.type = %d <===> _event.code = %d\n",_event.type,_event.code);
						switch( _event.type )
						{
							case EV_ABS:
								switch( _event.code )
								{
									case ABS_MT_TOUCH_MAJOR:
										break;
									case ABS_MT_POSITION_X:
										printf( "x %d", _event.value );
										//				_x	=	( _event.value - 32 );
										_x	=	( _event.value  );
										_x	=	(_x * 1280)/720;	
										break;
									case ABS_MT_POSITION_Y:
										printf( "y %d\n", _event.value );
										//				_y	=	( _event.value - 32 );
										_y	=	( _event.value );
										_y	=	(_y * 720)/1280;
										break;
									case ABS_MT_TRACKING_ID:
										break;
									case ABS_MT_PRESSURE:
										break;
									default:
										printf( "Uncached ABS event 0x%02X\n", _event.code );
										break;
								}
								break;
							case EV_SYN:
								switch( _event.code )
								{
									case SYN_REPORT:
										mh_dev_carplay_send_signal_touch( carplay, _press, _x, _y );

										break;
									case SYN_MT_REPORT:
										break;
									default:
										printf( "Uncached SYN event 0x%02X\n", _event.code );
										break;
								}
								break;
							case EV_KEY:
								switch( _event.code )
								{
									case BTN_TOUCH:
										_press	=	_event.value;
										break;
									default:
										break;
								}
								break;
						}

					}
				}

			}
		}

	}
	printf("touch_task_epoll close\n");
	close(epfd);
	close(_fd);

	return NULL;
}		/* -----  end of static function touch_task_epoll  ----- */

static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
{
	g_message("--------------------%s----------------------\n",__func__);
	
    ilm_surfaceSetDestinationRectangle(id, 0, 0, width, height);
    printf("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
	ilm_surfaceSetSourceRectangle(id, 0, 0, width,height);

    printf("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
    ilm_surfaceSetVisibility(id, ILM_TRUE);
    printf("SetVisibility          : surface ID (%d), ILM_TRUE\n", id);
    ilm_layerAddSurface(layer,id);
    printf("layerAddSurface        : surface ID (%d) is added to layer ID (%d)\n", id, layer);
    ilm_commitChanges();
}

static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
	g_message("--------------------%s----------------------\n",__func__);
    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
    {
        configure_ilm_surface(id,sp->origSourceWidth,sp->origSourceHeight);

    }
}
static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
	g_message("--------------------%s----------------------\n",__func__);
    (void)user_data;
    struct ilmSurfaceProperties sp;

    if (object == ILM_SURFACE) {
        if (created) {


//            if (number_of_surfaces > 0) {
			if( id == 888){
                number_of_surfaces--;
                printf("surface                : %d created\n",id);
                ilm_getPropertiesOfSurface(id, &sp);
                if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
                {   // surface is already configured
                    configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
                } else {
                    // wait for configured event
                    ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
                    ilm_commitChanges();
                }
            }
        }
        else if(!created)
            printf("surface: %d destroyed\n",id);
    } else if (object == ILM_LAYER) {
        if (created)
            printf("layer: %d created\n",id);
        else if(!created)
            printf("layer: %d destroyed\n",id);
    }
}

ANW_uint8 mAccessoryCarPlayUUID[] = { 0x48, 0x43, 0x88, 0xEC,
									  0x41, 0xCD,
									  0xA2, 0x40,
									  0x97, 0x27, 0x57, 0x5D, 0x50, 0xBF, 0x1F, 0xD3
									};

ANW_uint8 mAccessoryIAP2UUID[] =  { 0x00, 0x00, 0x00, 0x00,
								   0xCA, 0xDE,
								   0xDE, 0xFA,
								   0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF
								 };

//iPhone CarPlay UUID 0x2D8D2466E14D451C88BC7301ABEA291A
ANW_uint8 mAppleCarPlayUUID[] =  { 0x66, 0x24, 0x8d, 0x2d,
								   0x4d, 0xe1,
								   0x1c, 0x45,
								   0x88, 0xbc, 0x73, 0x01, 0xab, 0xea, 0x29, 0x1a
								 };
//iPhone iAP2 UUID,0xFECACADEAFDECADEDEFACADE00000000
ANW_uint8 mAppleIAP2UUID[] = { 0x00, 0x00, 0x00, 0x00,
							   0xCA, 0xDE,
							   0xDE, 0xFA,
							   0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFE
							 };

int PairStatusCallback(PAIRSTATUS_TYPE nType,unsigned int data,ANW_BD_ADDR* bd_addr,int cod)
{
	printf( "\033[1;32;40m PairStatusCallback(%d) \033[0m\n", nType);
	printf("\n\n\n\n%s--->nType:%d, data:%d, address:%2x%2x%2x%2x%2x%2x, cod:%d\n\n\n\n", 
			__func__, nType, data, bd_addr->bytes[0], bd_addr->bytes[1], bd_addr->bytes[2], bd_addr->bytes[3],
			bd_addr->bytes[4], bd_addr->bytes[5], cod);
	switch(nType)
	{
		case PAIR_INCOMING_REQUEST:
			g_bPairing = 1;
			ANW_DeviceInversePair(*bd_addr,"0000");
			break;
		case SSP_PAIR_INCOMING_REQUEST:
			g_bPairing = 1;
			ANW_DeviceSSPAccept(*bd_addr);
			break;
		case PAIR_STATUS:
			if(bd_addr)
			{
				if(data == 1)
					printf( "\033[1;32;40m Device pair success(%02x:%02x:%02x:%02x:%02x:%02x) \033[0m\n",bd_addr->bytes[5],bd_addr->bytes[4],bd_addr->bytes[3],bd_addr->bytes[2],bd_addr->bytes[1],bd_addr->bytes[0]);
				else
					printf( "\033[1;32;40m Device pair fail(%02x:%02x:%02x:%02x:%02x:%02x) \033[0m\n",bd_addr->bytes[5],bd_addr->bytes[4],bd_addr->bytes[3],bd_addr->bytes[2],bd_addr->bytes[1],bd_addr->bytes[0]);
			}
			else
			{
				if(data == 1)
					printf( "\033[1;32;40m Device pair success \033[0m\n");
				else
					printf( "\033[1;32;40m  Device pair fail \033[0m\n");
			}
			g_bPairing = 0;
			
			break;
	}

	return 1;
}

int ConnectStatusCallback(int nIndex,Connect_Channel Channel,int nStatus, ANW_BD_ADDR bd_addr)
{
	char szProfile[256];
	char szConnectStatus[256];
	szProfile[0]='\0';
	szConnectStatus[0] = '\0';
	if(nStatus == 1)
		sprintf(szConnectStatus,"connected");
	else
	{
	if(nStatus ==-1)
		sprintf(szConnectStatus,"disconnected : linklost");
	else
		sprintf(szConnectStatus,"disconnected");
	}
	switch(Channel)
	{
		case HF_CHANNEL: // HF
			if(nIndex>=0 && nIndex<2)
			g_HFPConnectStatus[nIndex] = nStatus;
			sprintf(szProfile,"HFP");
			break;
		case PIMDATA_CHANNEL: // Data
			sprintf(szProfile,"PIM");
			break;
		case AUDIO_STREAM_CHANNEL: // A2DP
			g_A2DPConnectStatus =  nStatus;
			sprintf(szProfile,"A2DP");
			break;
		case AUDIO_CONTROL_CHANNEL: // AVRCP
			sprintf(szProfile,"AVRCP");
			break;
		case SPP_CHANNEL: 
			mSearchCarPlayDev = 0;
			sprintf(szProfile,"SPP");
			break;			
		case HID_CONTROL_CHANNEL: 
			g_HIDConnectStatus = nStatus;
			sprintf(szProfile,"HID Control");
			break;			
		case HID_INTERRUPT_CHANNEL: 
			sprintf(szProfile,"HID interrupt");
			break;			
		case SCO_CHANNEL: 
			sprintf(szProfile,"SCO");
			if(nIndex>=0 && nIndex<2)
			g_SCOConnectStatus[nIndex]= nStatus;
			break;			
		case AVRCP_BROWSING_CHANNEL: 
			g_AVRCPBrowsingConnectStatus = nStatus;
			sprintf(szProfile,"AVRCP browsing");
			break;	
		case PHONEBOOK_CHANNEL: 
			sprintf(szProfile,"Phonebook");
			break;						
		case SMS_CHANNEL: 
			sprintf(szProfile,"SMS");
			break;	
		default:
			break;
	}
	printf( "\033[1;32;40m  %s (%02x:%02x:%02x:%02x:%02x:%02x) %s -index =%d \033[0m\n",szProfile,bd_addr.bytes[0],bd_addr.bytes[1],bd_addr.bytes[2],bd_addr.bytes[3],bd_addr.bytes[4],bd_addr.bytes[5],szConnectStatus,nIndex);

	return 1;
}
int ServiceConnectRequestCallBack(Connect_Channel nType,ANW_BD_ADDR bd_addr,ANW_CHAR* szdeviceName,int nIndex)
{
	printf( "\033[1;32;40m  nIndex = %d ServiceConnectRequestCallBack(%d) \033[0m\n",nIndex,nType);
	switch(nType)
	{
		case HF_CHANNEL:
			ANW_HFAcceptConnection(nIndex);
			break;
		case HID_CONTROL_CHANNEL:
			ANW_HidAccept();
			break;
		case AUDIO_STREAM_CHANNEL:
			ANW_AudioAcceptConnection(TYPE_A2DP);
			break;
		case AUDIO_CONTROL_CHANNEL:
			ANW_AudioAcceptConnection(TYPE_AVRCP);
			break;
	}
	return 1;
}


ANW_BOOL SPPEventCallBack(SPP_EVEN_TTYPE nEventType,void* pEventData,int nEventdataSize)
{
	switch(nEventType)
	{
		case SPP_EVENT_CONNECT_STATE:
			{
				SPP_CONNECTSTATE_EVENTDATA *pData =(SPP_CONNECTSTATE_EVENTDATA*) pEventData;
				if(pData->nIndex >=0 && pData->nIndex<4)
//				if( pData->nIndex == mCarPlayIndex )
				{
					memcpy(&g_SPPConnectInfo[pData->nIndex].bt_address,&pData->bt_address,sizeof(ANW_BD_ADDR));
					g_SPPConnectInfo[pData->nIndex].nConnectStatus = pData->nConnectStatus;
					printf( "\033[1;32;40m **SPP connect report :%02x:%02x:%02x:%02x:%02x:%02x index = %d Connect status = %d \033[0m\n",
					pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0],pData->nIndex,pData->nConnectStatus);
			
					if( pData->nConnectStatus == 0 )
					{
						int i,j,k = 0;
						int find_dev = 1;
						for( j = 0; j < devCount; j++ )	
						{
//							g_message("--------------------mac---------------------");
//							DEBUG_HEX_DISPLAY(mCarPlayMacAddr[j].bytes, 6);
//							DEBUG_HEX_DISPLAY(pData->bt_address.bytes, 6);

							for( i = 0; i < 6; i++ )// Detects whether the mac address is a connected device
							{
								if( pData->bt_address.bytes[i] != mCarPlayMacAddr[j].bytes[i] )	
								{
//									g_message("--------------------------------------\n");
									find_dev = 0;
									break;
								}
							}
							if( find_dev == 0 )
							{
								continue;	
							}else{
								k = j;
//								g_message("-------------------------------------- 11\n");
								break;
							}
						}

//						g_message("--------------------------------------22 k = %d  %02x\n", k, mCarPlayMacAddr[k].bytes[0]);
						if(( iap2_bt != NULL )&& ( find_dev == 1)&& ( mCarPlayMacAddr[k].bytes[0] != 0))// send detach device 
						{
							memset( mCarPlayMacAddr[k].bytes, 0, 6 );

							char _address[17];
							
							sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",
									pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0]);
							g_message(" _address = %s \n",_address );
					
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_BT_IAP;
							_devParam->mac_addr			=	g_strdup( _address );
							_devParam->connect_status	=	0;	

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );
						}
					}

				}
			}
			break;
		case SPP_EVENT_DATA_IND:
			{
				SPP_DATAIND_EVENTDATA *pData = (SPP_DATAIND_EVENTDATA*) pEventData;
//				if(pData->nIndex >=0 && pData->nIndex<4 && pData->nDataSize >0)
				if( pData->nIndex == mCarPlayIndex )//If the data is connected to the device, send data to mediahub
				{
					int i=0;
					char szTemp1[256];
					char szTemp2[256];
					szTemp1[0]='\0';
					szTemp2[0]='\0';
					printf( "\033[1;32;40m **SPP Receive data : index = %d ,size = %d \033[0m\n",pData->nIndex,pData->nDataSize);

					uint8_t * _buf;
					_buf = g_malloc0( pData->nDataSize );
					for(i=0 ;i<pData->nDataSize;i++)
					{
						sprintf(szTemp2,"%02x ",*(pData->pDataBuffer+i));
						strcat(szTemp1,szTemp2);
							_buf[i]	=	*(pData->pDataBuffer+i);
						if(i%16 ==0 && i!=0)
						{
							printf( "\033[1;32;40m szTemp1 = %s \033[0m\n", szTemp1);
							szTemp1[0]='\0';
						}	
					
					}
					printf( "\033[1;32;40m szTemp1 = %s \033[0m\n", szTemp1);
					g_message("spp callback *********************start\n");
					DEBUG_HEX_DISPLAY( _buf, pData->nDataSize);
					g_message("spp callback *********************end\n");
					mh_dev_write_bt_data(iap2_bt, _buf, pData->nDataSize );
					g_free(_buf);

				}
			}
			break;
		case SPP_EVENT_CONNECT_REQUEST:
			{
				SPP_CONNECTREQUEST_EVENTDATA *pData = (SPP_CONNECTREQUEST_EVENTDATA*) pEventData;

				printf( "\033[1;32;40m **SPP channel connect with %s  %02x:%02x:%02x:%02x:%02x:%02x \033[0m\n",
						pData->szdeviceName,pData->bt_address.bytes[0],pData->bt_address.bytes[1],pData->bt_address.bytes[2],pData->bt_address.bytes[3],
							pData->bt_address.bytes[4],pData->bt_address.bytes[5]);

				int i,j,k;
				int connect = 1;
				for( j = 0; j < devCount; j++ )	
				{
					for( i = 0; i < 6; i++ )// Detects whether the mac address is a connected device
					{
						if( pData->bt_address.bytes[i] != mCarPlayMacAddr[j].bytes[i] )	
						{
							connect = 0;
							break;
						}
					}
					if( connect == 0 )
					{
						continue;	
					}else{
						k = j;
						break;
					}
				}

				if( connect == 1 )
				{
					ANW_SPPAccept(pData->nIndex);
					mCarPlayIndex	=	pData->nIndex;
					char _address[17];
					
					sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",
						pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0]);
					g_message(" _address = %s \n",_address );

					MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
					_devParam->type				=	MH_DEV_BT_IAP;
					_devParam->mac_addr			=	g_strdup( _address );
					_devParam->connect_status	=	1;	//connect

					mh_core_find_dev( _devParam );

					g_free( _devParam->mac_addr );
					g_free( _devParam );
				}else{
					printf( "\033[1;32;40m **SPP channel connect Failed  \033[0m\n");
				}
			}
			break;
	}
	return TRUE;
}

unsigned char InquiryResponse(BLUETOOTH_DEVICE device, ANW_BOOL bComplete)
{
	if(bComplete ==0)
	{
		printf( "\033[1;32;40m InquiryResponse %s  %02x:%02x:%02x:%02x:%02x:%02x \033[0m\n",device.bd_name,
				device.bt_address.bytes[5],device.bt_address.bytes[4],device.bt_address.bytes[3],device.bt_address.bytes[2],device.bt_address.bytes[1],device.bt_address.bytes[0]);

		if( (device.EIRData.ad_mask & ANW_BT_ADV_BIT_SERVICE) != 0  )
		{	
			int i = 0;
			devCount	=	0;
			for (i = 0; i < device.EIRData.service_count; i++) 
			{
				int isEq2	=	0;

				//Check Apple CarPlay EIR
				if (( device.EIRData.service[i].service_type == ANW_SERVICE_128BITS )
						||  ( device.EIRData.service[i].service_type == ANW_SERVICE_SOL_128BITS )) {
					g_message ( "find service_type: %d\n", device.EIRData.service[i].service_type);

					if(( device.EIRData.service[i].service_uuid.uuid_128b.service_uuid	==	0x00000000 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data2	==	0xDECA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data3	==	0xFADE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[0]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[1]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[2]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[3]	==	0xAF )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[4]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[5]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[6]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[7]	==	0xFE ))
					{
						g_message ("Find Apple iAP2 EIR:[%d] = %s\n" ,i, device.bt_address.bytes);
					}else{
						g_message ("Don't Find Apple iAP2 EIR\n");								
						g_message ("service_uuid = %08X\n", device.EIRData.service[i].service_uuid.uuid_128b.service_uuid);	
						g_message ("data2 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data2);
						g_message ("data3 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data3);
						g_message ("data4[0] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[0]);
						g_message ("data4[1] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[1]);
						g_message ("data4[2] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[2]);
						g_message ("data4[3] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[3]);
						g_message ("data4[4] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[4]);
						g_message ("data4[5] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[5]);
						g_message ("data4[6] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[6]);
						g_message ("data4[7] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[7]);
					}

					if(( device.EIRData.service[i].service_uuid.uuid_128b.service_uuid	==	0x2D8D2466 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data2	==	0xE14D )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data3	==	0x451C )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[0]	==	0x88 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[1]	==	0xBC )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[2]	==	0x73 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[3]	==	0x01 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[4]	==	0xAB )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[5]	==	0xEA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[6]	==	0x29 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[7]	==	0x1A ))
					{
	
						g_message ("Find Apple carplay EIR:[%d] = %02x:%02x:%02x:%02x:%02x:%02x\n" ,i,
								device.bt_address.bytes[5],device.bt_address.bytes[4],device.bt_address.bytes[3],device.bt_address.bytes[2],device.bt_address.bytes[1],device.bt_address.bytes[0]);

						//							mSearchCarPlayDev = 0;
						mCarPlayMacAddr[devCount].bytes[0] = device.bt_address.bytes[0];
						mCarPlayMacAddr[devCount].bytes[1] = device.bt_address.bytes[1];
						mCarPlayMacAddr[devCount].bytes[2] = device.bt_address.bytes[2];
						mCarPlayMacAddr[devCount].bytes[3] = device.bt_address.bytes[3];
						mCarPlayMacAddr[devCount].bytes[4] = device.bt_address.bytes[4];
						mCarPlayMacAddr[devCount].bytes[5] = device.bt_address.bytes[5];
						devCount++;
						//							mCarPlayCod = cod;
						//							mCarPlaySupport = 1;
					}else{
						g_message ("Don't Find Apple carplay EIR\n");								
						g_message ("service_uuid = %08X\n", device.EIRData.service[i].service_uuid.uuid_128b.service_uuid);	
						g_message ("data2 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data2);
						g_message ("data3 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data3);
						g_message ("data4[0] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[0]);
						g_message ("data4[1] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[1]);
						g_message ("data4[2] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[2]);
						g_message ("data4[3] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[3]);
						g_message ("data4[4] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[4]);
						g_message ("data4[5] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[5]);
						g_message ("data4[6] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[6]);
						g_message ("data4[7] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[7]);
					}
				}
			}
		}
	}
	else
	{
		printf( "\033[1;32;40m   Inquiry Completed \033[0m\n");
		g_bInquiry =0;
	}
	return 1;
}

int PL_BTPower(bool bOn)
{
	int nRet = Anw_SUCCESS ;

	if(bOn == true && g_bBTPowerStatus == false)
	{
		system("echo 0 > /sys/class/gpio/gpio449/value");
		system("echo 449 > /sys/class/gpio/unexport");
		usleep(500*1000);
		system("echo 449 > /sys/class/gpio/export");
		system("echo out > /sys/class/gpio/gpio449/direction");
		system("echo 1 > /sys/class/gpio/gpio449/value");
		printf("system cmd is finish\n");

		//BT Module Power on & initial
		nRet	=	ANW_ManagerInitialEx("/media/ivi-data/anw");

		if( nRet != Anw_SUCCESS )
		{
			printf("ANW_ManagerInitialEx is Failed!\n\n");
			return nRet;
		}

		nRet	=	ANW_DeviceInitial();

		if( nRet != Anw_SUCCESS )
		{
			printf("ANW_DeviceInitial is Failed!\n\n");
			return nRet;
		}

		ANW_EnableLog(1);

		g_bBTPowerStatus = true;
		//Set callback function
		ANW_SetConnectStatusCallBack(ConnectStatusCallback);
		ANW_SetPairStatusCallBack(PairStatusCallback);
		ANW_SetServiceConnectRequestCallBack(ServiceConnectRequestCallBack); 
//		ANW_SetHFIndicatorCallBack(HFIndicatorCallback);
//		ANW_SetIncomingframeCallBack(IncomingCallCallback, IncomingSMSCallback);
//		ANW_AudioControlSetEventCallBack(AudioControlEventCallback);

		//Profile initial
		memset(&g_SPPConnectInfo,0,sizeof(SPP_CONNECTION)*4);
		{

			ANW_BT_ADV_DATA EIRData ;
			EIRData.ad_mask = ANW_BT_ADV_BIT_DEV_NAME | ANW_BT_ADV_BIT_SERVICE;
			EIRData.bt_name[0]	= 'H';
			EIRData.bt_name[1]	= 'S';
			EIRData.bt_name[1]	= '7';
			EIRData.service_count = 2;
			EIRData.service[0].service_type = ANW_SERVICE_128BITS;
	
			EIRData.service[0].service_uuid.uuid_128b.service_uuid	=	0x484388EC;
			EIRData.service[0].service_uuid.uuid_128b.data2	=	0x41CD;
			EIRData.service[0].service_uuid.uuid_128b.data3	=	0xA240;

			EIRData.service[0].service_uuid.uuid_128b.data4[0]	=	0x97;
			EIRData.service[0].service_uuid.uuid_128b.data4[1]	=	0x27;
			EIRData.service[0].service_uuid.uuid_128b.data4[2]	=	0x57;
			EIRData.service[0].service_uuid.uuid_128b.data4[3]	=	0x5D;
			EIRData.service[0].service_uuid.uuid_128b.data4[4]	=	0x50;
			EIRData.service[0].service_uuid.uuid_128b.data4[5]	=	0xBF;
			EIRData.service[0].service_uuid.uuid_128b.data4[6]	=	0x1F;
			EIRData.service[0].service_uuid.uuid_128b.data4[7]	=	0xD3;

			EIRData.service[1].service_uuid.uuid_128b.service_uuid	=	0x00000000;
			EIRData.service[1].service_uuid.uuid_128b.data2	=	0xCADE;
			EIRData.service[1].service_uuid.uuid_128b.data3	=	0xDEFA;

			EIRData.service[1].service_uuid.uuid_128b.data4[0]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[1]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[2]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[3]	=	0xAF;
			EIRData.service[1].service_uuid.uuid_128b.data4[4]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[5]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[6]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[7]	=	0xFF;

			ANW_SetEIRData(EIRData);

			ANW_SPPInit( SPPEventCallBack, mAccessoryCarPlayUUID, 16 );

			ANW_SPPAddService ( mAccessoryIAP2UUID, 16 );

			mSearchCarPlayDev++;
		}

		ANW_DeviceInquiry(InquiryResponse);

	}
	else if(bOn == false && g_bBTPowerStatus == true)
	{
		g_bRunPowerOffProcess = true;
		//Stop all processing
		if(g_bInquiry ==1)
		{
			ANW_DeviceInquiryStop();
			
			while(g_bInquiry ==1)
				sleep(1);
		}
		if(g_bPBDownloding[0] == 1 || g_bSMSDownloding[0] == 1 ||g_bPBDownloding[1] == 1 || g_bSMSDownloding[1] == 1)
		{			
			while(g_bPBDownloding[0] == 1 || g_bSMSDownloding[0] == 1 ||g_bPBDownloding[1] == 1 || g_bSMSDownloding[1] == 1)
				sleep(1);
		}		
		//Profile deinit
		ANW_SPPDenit();

		//BT module deinit & power of
		nRet = ANW_DeviceClose();
		g_bBTPowerStatus = false;
		g_bRunPowerOffProcess = false;
	}
	return nRet;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  touch_task_epoll
 *  Description:  
 * =====================================================================================
 */
//static void * wifi_scan( void * user_data )
//{
//	DBusError dbus_err;
//
//	dbus_error_init(&dbus_err);
//	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_err);
//
//	if (dbus_error_is_set(&dbus_err)) {
//		fprintf(stderr, "Error: %s\n", dbus_err.message);
//		dbus_error_free(&dbus_err);
//		return NULL;
//	}
//
//	while( tmpx )
//	{
//		fprintf(stdout, "Call Method\n");
//		{
//			DBusMessage *message, *reply;
//			message = dbus_message_new_method_call("net.connman",
//					"/", "net.connman.Manager", "GetStations");
//
//			reply = dbus_connection_send_with_reply_and_block(connection, message, -1, NULL);
//			dbus_message_unref(message);
//
//			if (reply) {
//				fprintf(stdout, "Reply\n");
//				if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
//					DBusError err;
//
//					dbus_error_init(&err);
//					dbus_set_error_from_message(&err, reply);
//
//					if (err.message) {
//						fprintf(stderr, "Error: %s\n", err.message);
//					}
//
//					dbus_error_free(&err);
//				}
//				else
//				{
//					DBusMessageIter iter;
//					dbus_message_iter_init(reply, &iter);
//					station_print(&iter);
//				}
//
//				dbus_message_unref(reply);
//			}
//		}
//		sleep(1);
//	}
//	dbus_connection_unref(connection);
//	return NULL;
//}		/* -----  end of static function touch_task_epoll  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	ilm_init();
	screen	=	1;
	layer	=	888;
	number_of_surfaces	=	888;
	 struct ilmScreenProperties screenProperties;
    struct ilmScreenExtendedProperties screenExtendedProperties;

    ilm_getPropertiesOfScreen(screen, &screenProperties);
    ilm_getExtendedPropertiesOfScreen(screen, &screenExtendedProperties);

    screenWidth = screenProperties.screenWidth;
    screenHeight = screenProperties.screenHeight;
    ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
    printf("CreateWithDimension          : layer ID (%d), Width (%u), Height (%u)\n", layer, screenWidth, screenHeight);
    ilm_layerSetDestinationRectangle(layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
    printf("layerSetDestinationRectangle : layer ID (%d), X (%u), Y (%u), Width (%u), Height (%u)\n", layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
    ilm_layerSetVisibility(layer,ILM_TRUE);
    printf("SetVisibility                : layer ID (%d), ILM_TRUE\n", layer);

    t_ilm_int i, length = 0;
    t_ilm_layer* pArray = NULL;
    ilm_getLayerIDs(&length, &pArray);

    t_ilm_layer renderOrder[length + 1];
    renderOrder[length] = layer;

    for(i=0;i<length;i++) {
        renderOrder[i] = pArray[i];
    }

    ilm_displaySetRenderOrder(0,renderOrder,length + 1);
    ilm_commitChanges();
    ilm_registerNotification(callbackFunction, NULL);

	PL_BTPower(true);	//bt power on 

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
	pthread_t _touch;

	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	mh_ipc_media_client_init( _conn );

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );

	mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY );
	mh_core_start();

	mh_misc_set_bluetoothids("C5:F5:B3:FD:FE:4C");

	tmpx = 1;
//	pthread_create( &_touch, NULL, touch_task, NULL );
	pthread_create( &_touch, NULL, touch_task_epoll, NULL );

	_pb = mh_pb_create();

	MHPbEventsListener _listener =
	{
		.callback = _pbs_callback,
		.user_data = NULL  
	};
	mh_pb_register_events_listener( _pb, &_listener );

//	uint8_t _mode[12]  =   {0};	
//	_mode[0]	=	12;
//	_mode[1] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
//	_mode[2] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
//	_mode[3] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
//	_mode[4] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
//	_mode[5] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
//	_mode[6] 	=   MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE & 0xFF;	
//	_mode[7] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
//	_mode[8] 	=   MH_CARPLAY_CONSTRAINT_ANYTIME & 0xFF;	
//	_mode[9] 	=   MH_CARPLAY_APPMODE_SPEECH_NOAPP & 0xFF;	
//	_mode[10] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
//	_mode[11] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
//
//	mh_misc_carplay_init_modes( _mode );

//	pthread_t _wifi_status;
//
//	pthread_create( &_touch, NULL, wifi_scan, NULL );

	while( 1 )
	{
		char _cmd[128]	=	{0};
		uint8_t _buf[1024]	=	{0};

		read( 0, _cmd, sizeof( _cmd ));

		_cmd[ strlen( _cmd ) - 1 ]	=	0;

		printf("\n\n\n   _cmd = [%s]\n\n\n",_cmd);
		if( strcmp( _cmd, "2" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_USB_CARPLAY;
			_devParam->mac_addr			=	"28:e1:4c:75:ba:81";
			_devParam->connect_status	=	1;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		else
		if( strcmp( _cmd, "3" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_USB_CARPLAY;
			_devParam->mac_addr			=	"28:e1:4c:75:ba:81";
			_devParam->connect_status	=	0;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		if( strcmp( _cmd, "8" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_USB_CARPLAY;
			_devParam->mac_addr			=	"d0:25:98:8c:04:12";
			_devParam->connect_status	=	1;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		else
		if( strcmp( _cmd, "9" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_USB_CARPLAY;
			_devParam->mac_addr			=	"d0:25:98:8c:04:12";
			_devParam->connect_status	=	0;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		if( strcmp( _cmd, "4" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_CARPLAY;
			_devParam->mac_addr			=	"d0:25:98:8c:04:12";
			_devParam->connect_status	=	1;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		else
		if( strcmp( _cmd, "5" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_CARPLAY;
			_devParam->mac_addr			=	"d0:25:98:8c:04:12";
			_devParam->connect_status	=	0;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		if( strcmp( _cmd, "6" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_CARPLAY;
			_devParam->mac_addr			=	"a8:88:08:51:39:1a";
			_devParam->connect_status	=	1;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
		else
		if( strcmp( _cmd, "7" ) == 0 )
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_CARPLAY;
			_devParam->mac_addr			=	"a8:88:08:51:39:1a";
			_devParam->connect_status	=	0;	

			mh_core_find_dev( _devParam );

			g_free( _devParam );
		}
	
		if( strcmp( _cmd, "take screen" ) == 0 )
		{
			mh_dev_carplay_take_screen( carplay );
		}
		else
		if( strcmp( _cmd, "untake screen" ) == 0 )
		{
			mh_dev_carplay_untake_screen( carplay );
		}
		else
		if( strcmp( _cmd, "borrow screen" ) == 0 )
		{
			mh_dev_carplay_borrow_screen( carplay );
		}
		else
		if( strcmp( _cmd, "unborrow screen" ) == 0 )
		{
			mh_dev_carplay_unborrow_screen( carplay );
		}
		else
		if( strcmp( _cmd, "take audio" ) == 0 )
		{
			mh_dev_carplay_take_main_audio( carplay );
		}
		else
		if( strcmp( _cmd, "untake audio" ) == 0 )
		{
			mh_dev_carplay_untake_main_audio( carplay );
		}
		else
		if( strcmp( _cmd, "borrow audio" ) == 0 )
		{
			mh_dev_carplay_borrow_main_audio( carplay );
		}
		else
		if( strcmp( _cmd, "unborrow audio" ) == 0 )
		{
			mh_dev_carplay_unborrow_main_audio( carplay );
		}
		else
		if( strcmp( _cmd, "start" ) == 0 )
		{
			tmpx = 1;
			pthread_create( &_touch, NULL, touch_task_epoll, NULL );
		}
		else
		if( strcmp( _cmd, "end" ) == 0 )
		{
			tmpx = 0;
		}
		else
		if( strcmp( _cmd, "ui" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "" );
		}
		else
		if( strcmp( _cmd, "ui maps" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "maps:" );
		}
		else
		if( strcmp( _cmd, "ui mobilephone" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "mobilephone:" );
		}
		else
		if( strcmp( _cmd, "ui music" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "music:" );
		}
		else
		if( strcmp( _cmd, "ui nowplaying" ) == 0 )
		{
			mh_dev_carplay_request_ui( carplay, "nowplaying:" );
		}
		else
		if( strncmp( _cmd, "ui tel:", 7 ) == 0 )
		{
			printf("ui tel = %s\n",_cmd + 3);
			mh_dev_carplay_request_ui( carplay, _cmd + 3 );
		}
		else
		if( strcmp( _cmd, "siri prewarm" ) == 0 )
		{
			mh_dev_carplay_request_siri_prewarm( carplay );
		}
		else
		if( strcmp( _cmd, "siri down" ) == 0 )
		{
			mh_dev_carplay_request_siri_button_down( carplay );
		}
		else
		if( strcmp( _cmd, "siri up" ) == 0 )
		{
			mh_dev_carplay_request_siri_button_up( carplay );
		}
		else
		if( strcmp( _cmd, "keyframe" ) == 0 )
		{
			mh_dev_carplay_force_key_frame( carplay );
		}
		else
		if( strcmp( _cmd, "night mode" ) == 0 )
		{
			mh_dev_carplay_set_night_mode( carplay, true );
		}
		else
		if( strcmp( _cmd, "night mode f" ) == 0 )
		{
			mh_dev_carplay_set_night_mode( carplay, false );
		}
		else
		if( strcmp( _cmd, "change res" ) == 0 )
		{
g_message("____________________adsadaaaaasssdsda \n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "1" ) == 0 )
		{
			g_message("____________________adsada \n");
			uint8_t _mode[5]  =   {0};	
			_mode[0] =   MH_CARPLAY_RESOURCE_MAIN_SCREEN & 0xFF;	
			_mode[1] =   MH_CARPLAY_TRANSFERTYPE_BORROW & 0xFF;	
			_mode[2] =   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
			_mode[3] =   MH_CARPLAY_CONSTRAINT_NERVER & 0xFF;	
			_mode[4] =   MH_CARPLAY_CONSTRAINT_NERVER & 0xFF;	

			mh_dev_carplay_change_resource_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "change app" ) == 0 )
		{
			uint8_t _mode[3]  =   {0};	
			_mode[0] =   MH_CARPLAY_APPMODE_SPEECH_NONE & 0xFF;	
			_mode[1] =   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
			_mode[2] =   MH_CARPLAY_APPMODE_TRUE & 0xFF;		
			mh_dev_carplay_change_app_mode( carplay, _mode );
		}
		else
		if( strcmp( _cmd, "media btn" ) == 0 )
		{
			mh_dev_carplay_send_media_button( carplay, MH_CARPLAY_BTN_PLAY );
		}
		else
		if( strcmp( _cmd, "phone btn" ) == 0 )
		{
			mh_dev_carplay_send_phone_button( carplay,  MH_CARPLAY_PHONE_KEY_ZERO );
			mh_dev_carplay_send_phone_button( carplay,  0 );
		}
		else
		{
			printf( "unknown command: %s\n", _cmd );
		}
	}
	return 0;
}				/* ----------  end of function main  ---------- */
#endif
