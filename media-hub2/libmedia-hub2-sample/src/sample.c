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
//#include <mh_extern_misc.h>
MHDev * global_dev ;
static GMainLoop *test_loop;
MHFilter * _folder_filter;
MHFilter * _all_filter;
MHFilter * _music_filter;
MHFilter * _movie_filter;
MHFilter * _picture_filter;
static int displayIndex;
static int folderCount;
MHCol * _col;
bool usb_flag;
MHPb * _pb;
MHPlaylist * playlist	=	NULL;
MHFilter * _global_filter;
bool _ppm_play_flag;
void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data );
static MHItem ** _get_children( MHFolder * self,MHFilter * filter, MHFolderPosition pos,int * count );
void FirstFileFunc( MHItemType type,MHItem * item, void * user_data)
{

}
MHFolder * music_folder;
MHPlaylist * scan_playlist;
static int playlist_count=1;
static void scan_folder( MHFolder * folder );
MHCol * _favorite;
bool playlist_favorite_del( void * data, void * user_data)
{
	
	printf("-------------------------------------------------->\n");
	MHItemData * _data	=	(MHItemData *)data;
	printf("type:%d\nuri:%s\nname:%s\nsize:%lld\n,enable:%d\n,id:%lld\n",_data->type,_data->uri,_data->name,(long long int)_data->size,_data->valid, (long long int)_data->uniqueId);
	printf("title:%s\n",_data->metadata.music.title);
	printf("mediaType:%d\n",_data->metadata.music.mediaType);
	printf("duration--------------->%d\n", _data->metadata.music.duration);
	mh_col_filter_clear( _favorite);
	mh_col_add_filter(_favorite, COL_FILTER_NAME, _data->name, COL_FILTER_TITLE, _data->metadata.music.title,
			COL_FILTER_ARTIST, _data->metadata.music.artist,  COL_FILTER_DURATION, _data->metadata.music.duration, NULL);
	mh_col_set_favorite( _favorite, MH_ITEM_MUSIC,false);

	return false;
}

bool playlist_favorite( void * data, void * user_data)
{
	
	printf("-------------------------------------------------->\n");
	MHItemData * _data	=	(MHItemData *)data;
	printf("type:%d\nuri:%s\nname:%s\nsize:%lld\n,enable:%d\n,id:%lld\n",_data->type,_data->uri,_data->name,(long long int)_data->size,_data->valid, (long long int)_data->uniqueId);
	printf("title:%s\n",_data->metadata.music.title);
	printf("mediaType:%d\n",_data->metadata.music.mediaType);
	printf("duration--------------->%d\n", _data->metadata.music.duration);
	mh_col_filter_clear( _favorite);
	mh_col_add_filter(_favorite, COL_FILTER_NAME, _data->name, COL_FILTER_TITLE, _data->metadata.music.title,
			COL_FILTER_ARTIST, _data->metadata.music.artist,  COL_FILTER_DURATION, _data->metadata.music.duration, NULL);
	mh_col_set_favorite( _favorite, MH_ITEM_MUSIC, true);

	return false;
}

static bool favorite_test( MHItem * item, MHItemData * data, void * user_data )
{
//	printf("*************favorite_test********************************************%d\n",item_count++);
	switch( data->type )
	{
		case MH_ITEM_FOLDER	:
			printf( "[ %d ](Folder): %s\n", displayIndex ++, data->name );
			folderCount	++;
			break;
		case MH_ITEM_MUSIC :
			printf( "[ %d ](Music): %s-------->Favorite:%d\n", displayIndex ++, data->name, data->favorite );
			break;
		case MH_ITEM_MOVIE :
			printf( "[ %d ](Movie): %s\n", displayIndex ++, data->name );
			break;
		case MH_ITEM_PICTURE :
			printf( "[ %d ](Picture): %s\n", displayIndex ++, data->name );
			break;
		case MH_ITEM_NONE:
			printf( "[ %d ](Not a Media): %s\n", displayIndex ++, data->name );
			break;
		default:
			printf( "[ %d ](Unknown Type): %s\n", displayIndex ++, data->name );
			break;
	}

	return false;
}		/* -----  end of static function _each_item  ----- */
static bool list_item( MHItem * item,  MHItemData * data, void * user_data)
{
	printf("%s--->%s\n", __func__, data->uri);
	return false;
}

bool playlist_foreach_func( void * data, void * user_data)
{
	printf("-------------------------------------------------->\n");
	MHItemData * _data	=	(MHItemData *)data;
	printf("type:%d\nuri:%s\nname:%s\nsize:%lld\n,enable:%d\n,id:%lld\n",_data->type,_data->uri,_data->name,(long long int)_data->size,_data->valid, (long long int)_data->uniqueId);
	printf("title:%s\n",_data->metadata.music.title);
	printf("mediaType:%d\n",_data->metadata.music.mediaType);
	printf("-------------------->Favorite:%d\n", _data->favorite);
	printf("duration--------------->%d", _data->metadata.music.duration);
//	MHItem * _item	=	mh_dev_get_item_by_uri( global_dev, _data->uri);
//	MHFolder * _folder;
//	MHItem ** _items;
//	int count	=	-1;
//	mh_object_get_properties( ( MHObject * )_item, "parent", &_folder, NULL );
//	if( _folder == NULL )
//		mh_object_get_properties( ( MHObject * )global_dev, "base", &_folder, NULL );
//
//	_items	=	_get_children( _folder, _music_filter, FOLDER_BEGIN, &count);
//	printf("-------------------------------------------------->\n\n\n\n");
//	char * _cover_path;
//	if( MISC_OK == mh_file_get_cover(_data->uri, "/tmp", 4194304,  &_cover_path))//4M=4194304字节
//	{
//		printf("_cover_path:%s\n", _cover_path);
//	}
	return false;
}


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
	mh_playlist_foreach( playlist,0, -1, playlist_foreach_func,NULL);

	MHPlaylistInfo * _info	=	mh_dev_get_playlist_info( global_dev);
	int i=0;
	bool _exist = false;
	if( _info != NULL)
	{

		while(_info[i].playlistid !=0 )
		{
			if(g_strcmp0(_info[i].name, "_last_playlist")==0)
			{
				_exist	=	true;
				mh_dev_update_playlist(global_dev, "_last_playlist",playlist, _info[i].playlistid);

	
				break;
			}

			i++;
		}
		i	=	0;
		while(_info[i].playlistid !=0 )
		{
			free( _info[i].name );
			i++;
		}

		free( _info );
	}
	if( _exist	==	false)
	{
		mh_dev_save_playlist( global_dev, "_last_playlist", playlist);
	}
	mh_object_unref((MHObject *)playlist);
	playlist 	=	NULL;
	mh_object_unref((MHObject *)dev);

}		/* -----  end of static function _detach_event  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _scan_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_event(MHDev * dev,  MHDevScanCbType scan_type, MHItemType item_type, void *  data, uint32_t pefrsent,void * user_data)
{
	MHFolder * _folder;
	MHItem ** _items;
	int _count	=	-1;
	int error_count;

	if(scan_type	==	MH_DEV_FIRST_FILE)
	{
	
//		MHFilter * _filter	=	_music_filter;
//		if(item_type	==	MH_ITEM_MUSIC && _ppm_play_flag==false)
//		{
//			int count	=	-1;
//
//			MHFolder * _folder	=	(MHFolder *) data;
//
//			_items	=	_get_children( _folder, _filter, FOLDER_BEGIN, & _count);
//			free( _items);
//			if( playlist !=	NULL)
//			{
//				mh_object_unref((MHObject *)playlist);
//				playlist	=	NULL;
//			}
//
//			playlist	=	mh_folder_create_playlist( _folder, _filter, false);
//			mh_object_get_properties(( MHObject * )playlist, "count", &count,"error_count",&error_count, NULL );
//
//			mh_pb_play_by_list( _pb, playlist, 0 );
////			mh_playlist_foreach( playlist,0, -1, playlist_foreach_func,NULL);
//
//		}



		if(item_type	==	MH_ITEM_MOVIE )
		{
			MHFilter * _filter	=	_movie_filter;
			int count	=	-1;

			MHFolder * _folder	=	(MHFolder *) data;

			_items	=	_get_children( _folder, _filter, FOLDER_BEGIN, & _count);
			char * path;
			mh_object_get_properties((MHObject *)_items[0], "uri", &path, NULL);
			printf("path:%d\n\n", path);
			char * cover_path;
			int64_t duration	=	0;
			mh_file_get_video_artwork(path , "/tmp", 1, &duration, & cover_path);
			printf("duration=%lld\n", duration);



		}

	}
	
	else
	if( scan_type == MH_DEV_FINISH )
	{
//		g_message("-------------------------->MH_DEV_FINISH");
//		int i	=	0, _count;
//		MHPlaylistInfo * _info	=	mh_dev_get_playlist_info( dev );
//		if( _info	!=	NULL)
//		{
//			while( _info[i].playlistid != 0 )
//			{
//				if( playlist !=	NULL)
//				{
//					mh_object_unref((MHObject *)playlist);
//					playlist = 	NULL;
//				}
//
//				playlist = mh_dev_restore_playlist(dev, _info[i].playlistid);
//
//				mh_object_get_properties( (MHObject *)playlist, "count", &_count, NULL );
//	
//				printf( "%s, count: %d\n", _info[i].name, _count );
//	
//				mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func, NULL );
//
//				free( _info[i].name );
//	
//				i	++;
//			}
//			free( _info );
//		}
//
//		mh_dev_request_app_launch( dev, "com.apple.Music" );
		}
								
	

}		/* -----  end of static function _register_scan_callback  ----- */

void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data )
{
	MHPbInfoData * _info;
	int i=0;
	MHTagInfo * _res	=	NULL;
	
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

				_res = mh_file_get_tag( ( char * )_info->track_info.uri);
				mh_pb_resize(pb,0,0,800,480); 
				
				printf("mh_file_get_tag:title:%s, album:%s, artist:%s genre:%s duration:%d year:%d track:%d\n", 
					_res->title,_res->album, _res->artist, _res->genre ,_res->duration, _res->year, _res->track);
				free(_res->title);
				free(_res->album);
				free(_res->artist);
				free(_res->genre);
				free(_res);
			}
			break;
		case MH_PB_INFO_PLAYLIST_CHANGE:
			printf("sample data->playlist   =  [%p]\n ", _info->playlist);
			mh_playlist_foreach( _info->playlist,0, -1, playlist_foreach_func,NULL);
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
		case MH_PB_IP_INFO_PLAYBACK_SPEED:
			printf("###    ipod tongxsh speed = [%d]\n", _info->speed );
			break;
		case MH_PB_FREQUENCY_ANALYSIS_RESULT:
			printf("###    _info->frequency_analysis_result.band = [%d]\n", _info->frequency_analysis_result.band );
			for (i=0; i<_info->frequency_analysis_result.band; i++)
			{
				printf("###    _info->frequency_analysis_result.bands[%d] = [%lf]\n", i, _info->frequency_analysis_result.bands[i] );
				printf("###    _info->frequency_analysis_result.amplitudes[%d] = [%lf]\n", i, _info->frequency_analysis_result.amplitudes[i] );
			}
			break;
		default:
			break;			
	}
}

void _pbs_status_callback (MHPb * pb, MHPbStatusType type, void * user_data )
{
	switch (type)
	{
		case MH_PB_STATE_READY:
			printf( "$$$$$$$Current state : ready!\n" );
			break;
		case MH_PB_STATE_PLAYING:
			printf( "$$$$$$$Current state : playing!\n" );
			break;
		case MH_PB_STATE_PAUSE:
			printf( "$$$$$$$Current state : pause!\n" );
			break;
		case MH_PB_STATE_SWITCHING:
			printf( "$$$$$$$Current state : switching!\n" );
			break;
		case MH_PB_STATE_SEEKING:
			printf( "$$$$$$$Current state : seeking!\n" );
			break;
		default:
			break;			
	}
}
static int __count=0;

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	printf("-------------------------------------------------------------------->count:%d\n",__count++);
	char *_serial, *_type, *_entry, *_dev_type, *_dev_path;
	MHFolder * _folder;
	int iapType;
	MHDevEventsListener _event_listener		=
	{
		.callback	=	_dev_event,
		.user_data	=	NULL
	};

	printf( "Device[ %p ] status[ %d ]\n", dev, event );

	
	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);
	
	if ( _dev_type && !g_strcmp0(_dev_type, "mtp"))
	{
		printf( "find a mtp device\n" );
		mh_object_get_properties( ( MHObject * )dev, "serial", &_serial, NULL );
		printf( "device properties:\nserial:\t%s\n", _serial );
		free( _serial );

		mh_dev_register_events_listener( dev, & _event_listener);
		mh_dev_start_scan( dev,SCAN_TAG);
	}
	else if ( _dev_type && !g_strcmp0(_dev_type, "iap2"))
	{
		printf("device type:%s \n", _dev_type);
		mh_object_get_properties(( MHObject * )dev, "iapType", &iapType, "path", &_dev_path, NULL);
		printf("iap2 iapType type:%d path = %s \n", iapType, _dev_path);
		free(_dev_path);
		if (iapType == MISC_IAP)
		{
			mh_object_ref((MHObject *)dev);
			mh_dev_register_events_listener( dev, & _event_listener);
			MHDevDetachListener _detach_listener	=	
			{
				.callback	=	_detach_event,
				.user_data	=	NULL
			};
			mh_dev_register_detach_listener( dev, & _detach_listener);
			sleep(3);
			mh_dev_start_scan( dev,SCAN_TAG);
			printf("############################################   [%p]\n",_pb);
#ifndef HS7 // for nagivi
			mh_object_set_properties( ( MHObject * )_pb, "audio_sink", "pulsesink","buffer_time", 200000, NULL );
#else // for hs 7
			mh_object_set_properties( ( MHObject * )_pb, "audio_sink", "ahsink","buffer_time", 200000, NULL );
#endif
			mh_dev_attach_pb(dev, _pb);
		}
	}
	else if ( _dev_type && !g_strcmp0(_dev_type, "iap1"))
	{
		printf("device type:%s isn't supported now\n", _dev_type);
		
		mh_object_ref((MHObject *)dev);
		mh_dev_register_events_listener( dev, & _event_listener);

		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
		mh_dev_register_detach_listener( dev, & _detach_listener);

		
	}
	else if( _dev_type && !g_strcmp0( _dev_type, "carlife"))
	{
		printf("device type:%s plug in\n", _dev_type);
	}
	else if( _dev_type && !g_strcmp0( _dev_type, "storage"))
	{
		mh_object_ref((MHObject *)dev);
		mh_object_get_properties( ( MHObject * )dev, "serial", &_serial, "fs_type", &_type, "entry", &_entry, "path", &_dev_path, NULL );
		printf( "device properties:\nserial:\t%s\nfs_type:\t%s\nentry:\t%s\n", _serial, _type, _entry );
		printf("_dev_path = %s \n", _dev_path);

		uint64_t _total, _free;		
		mh_object_get_properties( ( MHObject * )dev, "total", &_total,"free", &_free, NULL);
		printf("_total:%lld,free:%lld\n",(long long int)_total, (long long int)_free);
		free( _serial );
		free( _type );
		free( _entry );
		free( _dev_path );
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
	    mh_dev_register_detach_listener( dev, & _detach_listener);
		mh_dev_register_events_listener( dev, & _event_listener);

		MHPlaylistInfo * _info	=	mh_dev_get_playlist_info( dev);
		int i=0;
		_ppm_play_flag = false;
//		if( _info != NULL)
//		{
//			while(_info[i].playlistid !=0 )
//			{
//				if(g_strcmp0(_info[i].name, "_last_playlist")==0)
//				{
//					uint32_t _count, _ptime, _index, _repeat, _shuffle;
//
//					_ppm_play_flag=	true;
//					if( playlist !=	NULL)
//					{
//						mh_object_unref((MHObject *)playlist);
//						playlist =	NULL;
//					}
//
//					playlist = mh_dev_restore_playlist(dev, _info[i].playlistid);
//					mh_object_get_properties(( MHObject * )playlist, "count", &_count, "shuffle", &_shuffle, "repeat", &_repeat,
//						"index", &_index, "ptime", &_ptime, NULL );
//					mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func,NULL);
//
//					mh_pb_play_by_list( _pb, playlist, _index );
//
//					break;
//				}
//		
//				i++;
//			}
//
//			i	=	0;
//
//			while(_info[i].playlistid !=0 )
//			{
//				free( _info[i].name );
//				i++;
//			}
//			free( _info );
//		}
//		mh_dev_start_scan( dev,SCAN_TAG);
#ifndef HS7 // for nagivi
		mh_object_set_properties( ( MHObject * )_pb, "audio_sink", "pulsesink","buffer_time", 200000, NULL );
#else //for hs7
		mh_object_set_properties( ( MHObject * )_pb, "audio_sink", "ahsink","buffer_time", 200000, NULL );
#endif

		mh_dev_start_scan( dev, SCAN_FOLDER);
		_favorite	=	mh_col_create( dev);
	}
	else if ( _dev_type && !g_strcmp0(_dev_type, "folder"))
	{
		mh_object_ref((MHObject *)dev);
		mh_object_get_properties( ( MHObject * )dev, "serial", &_serial,  "entry", &_entry, NULL );
		printf("\n\n\nfolder-->serial:%s, entry:%s\n\n", _serial, _entry);
	
	}

	else
	{
		printf(" I don't know the device:%s\n", _dev_type);
	}

	free( _dev_type );
	
	global_dev=dev;
	_col	=	mh_col_create( global_dev);
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
	if( g_strcmp0( type, "folder")==0)
		{
			mh_misc_create_dev("folder", "abc", "/tmp");
			mh_misc_create_dev("folder", "123", "/tmp/abc");
			mh_misc_create_dev("folder", "456", "/tmp");
		}

		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		printf( "Failed load a plugin\n" );
		break;
	default:
		break;
	}
}



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
static int item_count=1;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _each_item
 *  Description:  
 * =====================================================================================
 */
static bool _each_item( MHItem * item, MHItemData * data, void * user_data )
{
//	printf("*********************************************************%d\n",item_count++);
	switch( data->type )
	{
		case MH_ITEM_FOLDER	:
			printf( "[ %d ](Folder): %s\n", displayIndex ++, data->name );
			folderCount	++;
			break;
		case MH_ITEM_MUSIC :
			printf( "[ %d ](Music): %s\n", displayIndex ++, data->name );
			break;
		case MH_ITEM_MOVIE :
			printf( "[ %d ](Movie): %s\n", displayIndex ++, data->name );
			break;
		case MH_ITEM_PICTURE :
			printf( "[ %d ](Picture): %s\n", displayIndex ++, data->name );
			break;
		case MH_ITEM_NONE:
			printf( "[ %d ](Not a Media): %s\n", displayIndex ++, data->name );
			break;
		default:
			printf( "[ %d ](Unknown Type): %s\n", displayIndex ++, data->name );
			break;
	}

	return false;
}		/* -----  end of static function _each_item  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _get_children
 *  Description:  
 * =====================================================================================
 */
static MHItem ** _get_children( MHFolder * self,MHFilter * filter, MHFolderPosition pos,int * count )
{
	MHItem ** _items	=	NULL;
	_items	=	mh_folder_get_children( self,filter, FOLDER_BEGIN, count, MH_ITEM_ORDER_BY_ALPHABET_FOR_NAGIVI );
	printf("count = %d\n",*count);
	int i	=	0;
	MHItemType _type;
	char * name;

	folderCount	=	0;
	displayIndex	=	1;

	if( *count > 0 )
	mh_item_foreach( _items, * count, _each_item, NULL );
//	for( i=0 ; i < *count ;i++)
//	{
//		mh_object_get_properties( ( MHObject * )_items[i], "name", &name, "type", &_type, NULL);
//
//		switch( _type)
//		{
//			case MH_ITEM_FOLDER	:
//				printf("[ %d ] folder :%s\n", i+1, name);
//				folderCount	++;
//			break;
//
//			case MH_ITEM_MUSIC :
//				printf("[ %d ] music :%s\n", i+1, name);
//			break;
//
//			case MH_ITEM_MOVIE :
//				printf("[ %d ] movie :%s\n", i+1, name);
//			break;
//
//			case MH_ITEM_PICTURE :
//				printf("[ %d ] picture :%s\n", i+1, name);
//			break;
//			case MH_ITEM_NONE:
//				printf( "[ %d ] Not Media: %s\n", i + 1, name );
//				break;
//			default:
//				printf( "[ %d ] Unknown Type: %s\n", i + 1, name );
//				break;
//		}
//		}

	return _items;
}		/* -----  end of static function _get_children  ----- */

void print_help()
{	

	printf("\t[f]browse in the type of folder\n");
	printf("\t[c]start scan of db\n");
	printf("\t[r]review of artist in db\n");
	printf("\t[s]search artist,title,album in db\n");
	printf( "\t[h]display the operation Help\n" );
	printf( "\t[>]Play next file\n" );
	printf( "\t[<]Play previous file\n" );
	printf( "\t[p]Play\n" );
	printf( "\t[b]Stop\n" );
	printf( "\t[a]Pause\n" );
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _playlist_item
 *  Description:  
 * =====================================================================================
 */
static void _playlist_item( void * data, void * user_data )
		{
	MHItemData * _item	=	( MHItemData * )data;
			
	printf( "uri: %s\nname: %s\n", _item->uri, _item->name );
}		/* -----  end of static function _playlist_item  ----- */
void col_func(MHItem * item, void * user_data)
{
	g_message("col_func is called\n");
}

gpointer input_thread( gpointer data)
{
	char input[256];
	MHFolder * _folder;
	int i ;
	int index;
	MHItemType _type;
	char * name;
	char * filter	= "mp3";
	//;wma;mp4;wmv;mov;MP3
	char * filter_get;
	char * path ;
	MHItem ** _items;
	//	MHPlaylist * playlist2;
	int count	=	-1;
	int _uniqueid = 0;
	print_help();
	bool _first_enter	=	true;
	MHFolder * _base;
	MHPlaylist * _allSong;
	char *_serial;
	char **  res;
	int  countTmp;
	int musicCnt = 0; 
	int movieCnt = 0;
	
	while(1)
	{
		if(scanf("%s", input)!=0)
		{
			printf( "runing\n" );
			switch( input[0] )
			{
				case '0':
				{
					mh_dev_start_scan( global_dev,SCAN_TAG);
				}
				break;
						
				case '1':
				{
					res = mh_dev_get_radiolist(global_dev,& countTmp);
					printf("countTmp = %d \n", countTmp);
					for(i = 0; i < countTmp; i++)
					{
						printf("res = %s \n", res[i]);
					}
					
					for(i = 0; i < countTmp; i++)
					{
						free(res[i]);
					}
					free(res);
					res = NULL;
				}
				break;
				
				case '2':
				{
					printf( "play radio api test\n");
					//mh_pb_play_radio_by_index(global_dev, _pb, 2);
					mh_object_set_properties(( MHObject * )global_dev, "searchType", BREADTH_FIRST, NULL);
				}
				break;
				
				case '3':
				{
					mh_object_get_properties( ( MHObject * )global_dev, "serial", &_serial, NULL );
					printf( "device properties serial: %s\n", _serial);
					mh_misc_db_delete_by_serial_number(_serial);
					free( _serial );
				}
				break;
				case '8':
				{
					printf("please input search string:\n");
					char _string[100];
//					gets(_string);
					scanf("%s",_string);
//					scanf("%s", _string);
					MHItem **  _items;
					gint _count	=	-1;
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _string, &_count); 
					if( _items != NULL)
					{
						g_message("call mh_item_foreach");
						mh_item_foreach( _items, _count, list_item,NULL );	
					}
					else
					{
						g_message("_items == NULL");
						
					}

					g_free( _items);
				}
				break;	
				case '9':
				{
					char * _str	=	g_strdup("********");
					g_message("%s", _str);

					MHItem **  _items;
					gint _count	=	-1;

					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}

					g_free( _str);

					_str	=	g_strdup("***刘华");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}

					g_free( _str);

					_str	=	g_strdup("***刘华*****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("刘华");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("刘华*****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("****刘 华*****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("刘 华*****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("**刘 华");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("刘 华");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("刘*h");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);
					_str	=	g_strdup("******刘*h*********");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("liu");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);
					_str	=	g_strdup("***liu****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

					_str	=	g_strdup("***liu");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);
					_str	=	g_strdup("liu****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);


					_str	=	g_strdup("l*华");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);
					_str	=	g_strdup("******l*华*********");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);


					_str	=	g_strdup("***l*h****");
					g_message("%s", _str);
					_items	=	mh_dev_search_name( global_dev,MH_ITEM_MUSIC, _str, &_count); 
					if( _items != NULL)
					{
						mh_item_foreach( _items, _count, list_item,NULL ); 
						g_free( _items);
					}
					g_free( _str);

				}
				break;
				case 'f':
					mh_object_get_properties( ( MHObject * )global_dev, "base", &_folder, NULL );
					_base 	=	_folder;
					mh_object_get_properties( ( MHObject * )_folder, "name", &name, NULL);
					printf( "name %s\n", name );
					mh_object_get_properties( (MHObject *)_folder, "uri", &path, NULL );
					printf("current path:%s\n",path);
					mh_object_get_properties( (MHObject *)_folder, "unique-id", &_uniqueid, NULL );
					printf("unique-id path:%d\n", _uniqueid);
					_items	=	_get_children( _folder, _music_filter, FOLDER_BEGIN, &count);
					free(path);
					while(1)
					{
						int inputnum;
						if(scanf("%d", &inputnum) !=0 )
						{
							if( inputnum	==	0)
							{
								count	=	-1;

								mh_object_get_properties( ( MHObject * )_folder, "parent", &_folder, NULL );
								if( _folder == NULL )
									mh_object_get_properties( ( MHObject * )global_dev, "base", &_folder, NULL );

								if( _items !=	NULL)
								{
									free( _items );
									_items	=	NULL;
								}
								_items	=	_get_children( _folder, _music_filter, FOLDER_BEGIN, &count);
							}
							else
							{
								char * _name;

								if( _items == NULL || inputnum > count+1 )
								{
									printf( "no media items or index is greater than items count, %p & %d & %d\n", _items, inputnum, count );
								continue;
							}

							mh_object_get_properties( ( MHObject * )_items[inputnum - 1], "type", &_type, "name", &_name, NULL);
							g_message( "item name: %s", _name );
							if( _type	==	MH_ITEM_FOLDER)
							{
								_folder	=	( MHFolder * )_items[inputnum-1];
								count	=	-1;
							
								mh_object_get_properties( (MHObject *)_folder, "unique-id", &_uniqueid, NULL );
								printf("unique-id :%d\n", _uniqueid);

								
								printf("inputnum is %d\n",inputnum);
								mh_object_get_properties( ( MHObject * )_folder, "music_count", &musicCnt, "movie_count", &movieCnt, NULL);
								printf("--------------music = %d, movie = %d ----------\n", musicCnt, movieCnt);
								
							if( _items !=	NULL)
							{
								free( _items );
								_items	=	NULL;
							}
				DEBUG_TIME_BEGIN();
				DEBUG_TIME_DUR_BEGIN();
								_items	=	_get_children(_folder,_music_filter, FOLDER_BEGIN, &count );	
				DEBUG_TIME_DUR_END();
				DEBUG_TIME_END();					
							}
							else
							{
								if( _items !=	NULL)
								{
									free( _items );
									_items	=	NULL;
								}
								printf("music playlist create\n");
								if( playlist !=	NULL)
								{
									mh_object_unref((MHObject *)playlist);
									playlist = 	NULL;
								}
								playlist	=	mh_folder_create_playlist( _folder,_music_filter, false )	;
								mh_object_get_properties(( MHObject * )playlist, "count", &count, NULL );
								printf( "get playlist: %p, count is: %d\n", playlist, count );
								index = inputnum;
								index	=	index - folderCount-1;
								printf("index= %d\n",index );
//								mh_object_set_properties(( MHObject * )_pb, "video_sink", "glimagesink", NULL );
#ifndef HS7 // for nagivi
								mh_object_set_properties(( MHObject * )_pb, "video_sink", "v4l2sink", "surfaceid", 888, "streamid", "abc", NULL );
#else
								mh_object_set_properties(( MHObject * )_pb, "video_sink", "glimagesink", "surfaceid", 888, "streamid", "abc", NULL );
#endif
								printf("---------------------------------->index=%d,folderCount=%d\n",index, folderCount);
								mh_pb_play_by_list( _pb, playlist, index );
//								mh_playlist_foreach( playlist,0, -1, playlist_foreach_func,NULL);
//								////////////////////test Favorite///////////////
//								mh_playlist_foreach( playlist, 0, -1, playlist_favorite, NULL);
//
//								_allSong	=	mh_folder_create_playlist( _base, _music_filter, true);
//
//								printf("\n\nallsong  favorite\n\n\n");
//
//								mh_playlist_foreach(_allSong, 0, -1, playlist_foreach_func, NULL);
//								printf("\n\nfolder favorite\n\n\n");		
//
//								count = -1;
//								_items	=	mh_folder_get_children( _folder, _music_filter,  FOLDER_BEGIN, &count, MH_ITEM_ORDER_BY_DEFAULT );
//								mh_item_foreach( _items, count, favorite_test,NULL );	
//
//								//////delete favorite song /////
//								mh_playlist_foreach( playlist, 0, 1, playlist_favorite_del, NULL);
//								count = -1;
//								_items	=	mh_folder_get_children( _folder, _music_filter,  FOLDER_BEGIN, &count, MH_ITEM_ORDER_BY_DEFAULT );
//								mh_item_foreach( _items, count, favorite_test,NULL );	
								///////////////////////////////end Favorite/////////////////

								while( scanf( "%s", input ))
								{
									bool _break	=	false;

									switch( input[ 0 ] )
									{
									case '>':
										printf( "### mh_pb_next ###\n" );
										mh_pb_next( _pb );
										break;
									case '<':
										printf( "### mh_pb_previous ###\n" );
										mh_pb_previous( _pb );
										break;
									case 'p': // Play.
										mh_pb_play( _pb );
										break;
									case 'b': // Stop.
										mh_pb_stop( _pb );
										break;
									case 'a': // Pause.
										mh_pb_pause( _pb );
										break;
									case 'e':
										{
										int seeknum;
										if(scanf("%d", &seeknum) !=0 )
										mh_pb_seek( _pb, seeknum );
										}
										break;
									case 'c':
										mh_pb_forward( _pb );
										break;
									case 'w':
										mh_pb_backward( _pb );
										break;
									case '1':
										mh_pb_forward_done( _pb );
										break;
									case '2':
										mh_pb_backward_done( _pb );
										break;
									case 'o':{
										mh_object_set_properties(( MHObject * )_pb, "repeat", MH_PB_REPEAT_MODE_ONE, NULL);
										//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ONE );
										break;}
									case 't':
										mh_object_set_properties(( MHObject * )_pb, "repeat", MH_PB_REPEAT_MODE_ALL, NULL);
										//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
										break;
									case 'i':
										{
											int index;
											if(scanf("%d", &index) !=0 ){
												mh_object_set_properties(( MHObject * )_pb, "index", index, NULL);
											}
										}
										break;
									case 'm':
										mh_object_set_properties(( MHObject * )_pb, "shuffle", MH_PB_SHUFFLE_ALL, NULL);
										//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
										break;
									case 'n':
										mh_object_set_properties(( MHObject * )_pb, "shuffle", MH_PB_SHUFFLE_OFF, NULL);
										//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
										break;
									case '0':
										printf("### breaking ###\n");
										_break	=	true;

										break;
									case '3':
										{
											int index;
											if(scanf("%d", &index) !=0 ){
													mh_object_set_properties(( MHObject * )_pb, "audio_track", index, NULL);
											}
										}
										break;
									case '4':
										{
											MHPbTrackInfo	* _info	=	NULL;
										//	mh_object_get_properties(( MHObject * )_pb, "track_name", &_info, NULL);
											_info	=	mh_pb_get_track_info( _pb );
											g_message("info:%p",_info);
											printf("track total count = [%d],current count	=	[%d]\n",_info->total_count,_info->current_count);
											int i;
											for( i = 0; i < _info->total_count; i++ )
											{
												printf("name = [%s]\n",_info->track_name[i]);
												free( _info->track_name[i] );
											}

											free( _info );
										}
										break;
									case '5':
										{
											int index;
											if(scanf("%d", &index) !=0 ){
													mh_object_set_properties(( MHObject * )_pb, "subtitle", index, NULL);
											}
										}
										break;
									case '6':
										{
											MHPbSubtitleInfo	* _info	=	NULL;
											_info	=	mh_pb_get_subtitle_info( _pb );
											printf("track total count = [%d],current count	=	[%d]\n",_info->total_count,_info->current_count);
											int i;
											for( i = 0; i < _info->total_count; i++ )
											{
												printf("name = [%s]\n",_info->subtitle_name[i]);
												free( _info->subtitle_name[i] );
											}

											free( _info );
										}
										break;
									default:
										break;
									}

									if( _break ) break;
								}
							}
						}
					}
				}

				break;
			//artist: artist list->album list->song
			case 'r':
				{
					char ** _result;
					MHAlbumInfo * _result2;
					int count=0;
					mh_col_set_retrieve_key( _col, COL_FILTER_ARTIST);	
					mh_col_set_order_type( _col, MH_ITEM_ORDER_BY_PINYIN);
					_result	=	mh_col_retrieve_data( _col,MH_ITEM_MUSIC,MH_MEDIA_MUSIC, &count, false);
					int i=0;
					for(i;i<count;i++)
					{
						printf("_result[%d]=%s\n",i,_result[i]);
					}
					while(1)
					{
						int inputnum;
						if(scanf("%d", &inputnum) !=0 )
						{
							if( inputnum > count || inputnum <1 )
							{
								g_message("wrong number\n");
							}
							else
							{
								int col_count=0;
							
								mh_col_add_filter( _col, COL_FILTER_ARTIST, _result[inputnum], NULL);
								_result2	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC ,&count, false);
								mh_col_filter_clear( _col );

								int i = 0;
								for( i; i < count; i++)
								{
									printf("_result2[%d]:album_title:%s---->album_artist:%s---->album_compliation:%d\n", i, _result2[i].album_title,
											_result2[i].album_artist, _result2[i].album_compliation);
								}
								while(1)
								{
									int mem	=	inputnum;
									if(scanf("%d", &inputnum)!=0)
									{
										////////////test///////////////

										//test-end////////////////
										MHPlaylist * _col_playlist;
										mh_col_filter_clear( _col);
										mh_col_add_filter( _col , COL_FILTER_ARTIST, _result[mem], COL_FILTER_ALBUM, &_result2[inputnum],NULL);
										_col_playlist	=	mh_col_create_playlist( _col, MH_ITEM_MUSIC,MH_MEDIA_MUSIC,false);

										mh_playlist_sort( _col_playlist, MH_SORT_TRACKID, MH_ITEM_ORDER_BY_TRACKID);

										mh_pb_play_by_list(	_pb, _col_playlist, 0 );	
										mh_playlist_foreach( _col_playlist, 0, -1, playlist_foreach_func,NULL);
										
										printf("\n\n##################################\n\n");

										mh_playlist_append_playlist(_col_playlist, _col_playlist);


										mh_playlist_foreach( _col_playlist, 0, -1, playlist_foreach_func,NULL);

										printf("\n\n#############################\n\n");
										while( scanf( "%s", input ))
									{
										bool _break	=	false;
										switch( input[ 0 ] )
										{
										case '>':
											printf( "### mh_pb_next ###\n" );
											mh_pb_next( _pb );
											break;
										case '<':
											printf( "### mh_pb_previous ###\n" );
											mh_pb_previous( _pb );
											break;
										case 'p': // Play.
											mh_pb_play( _pb );
											break;
										case 'b': // Stop.
											mh_pb_stop( _pb );
											break;
										case 'a': // Pause.
											mh_pb_pause( _pb );
											break;
										case 'e':
											{
											int seeknum;
											if(scanf("%d", &seeknum) !=0 )
											mh_pb_seek( _pb, seeknum );
											}
											break;
										case 'c':
											mh_pb_forward( _pb );
											break;
										case 'w':
											mh_pb_backward( _pb );
											break;
										case '1':
											mh_pb_forward_done( _pb );
											break;
										case '2':
											mh_pb_backward_done( _pb );
											break;
										case 'o':{
											mh_object_set_properties(( MHObject * )_pb, "repeat", MH_PB_REPEAT_MODE_ONE, NULL);
											//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ONE );
											break;}
										case 't':
											mh_object_set_properties(( MHObject * )_pb, "repeat", MH_PB_REPEAT_MODE_ALL, NULL);
											//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
											break;
										case 'i':
											{
												int index;
												if(scanf("%d", &index) !=0 ){
													mh_object_set_properties(( MHObject * )_pb, "index", index, NULL);
												}
											}
											break;
										case 'm':
											mh_object_set_properties(( MHObject * )_pb, "shuffle", MH_PB_SHUFFLE_ALL, NULL);
											//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
											break;
										case 'n':
											mh_object_set_properties(( MHObject * )_pb, "shuffle", MH_PB_SHUFFLE_OFF, NULL);
											//mh_pb_set_repeat( _pb, MH_PB_REPEAT_MODE_ALL );
											break;
										case '0':
											printf("### breaking ###\n");
											_break	=	true;

											break;
										default:
											break;
										}

										if( _break ) break;
									}

									}

								}
							}
						}
					}

				}
				break;
			//genre: genre list->artist list->album list->song
			case 'e':
				{///genre list	
					char ** _result;
					char ** _result2;
					MHAlbumInfo * _result3;

					int count=0;
					mh_col_set_retrieve_key( _col, COL_FILTER_GENRE);	
					mh_col_set_order_type( _col, MH_ITEM_ORDER_BY_PINYIN);
					_result	=	mh_col_retrieve_data( _col,MH_ITEM_MUSIC,MH_MEDIA_MUSIC, &count, false);
					int i=0;
					for(i;i<count;i++)
					{
						printf("_result[%d]=%s\n",i,_result[i]);
					}
					
					while(1)
					{
						g_message("while 1\n");
						int inputnum;
						if(scanf("%d", &inputnum) !=0 )
						{//artist list
							if( inputnum > count || inputnum <1 )
							{
								g_message("wrong number\n");
							}
							else
							{
								int col_count=0;
								count = 0;
								mh_col_set_retrieve_key( _col, COL_FILTER_ALBUM_ARTIST);
								mh_col_add_filter( _col, COL_FILTER_GENRE, _result[inputnum], NULL);

								_result2	=	mh_col_retrieve_data( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &count, false);
							//	mh_col_filter_clear( _col );
								for( i=0; i < count; i++)
								{
									printf("_result2[%d]=%s\n", i, _result2[i]);
								}
								while(1)
								{//album list 
									if( scanf("%d", &inputnum) != 0)
									{
										if( inputnum > count || inputnum <0 )
										{
											g_message("wrong number\n");
										}
										else
										{
											int col_count=0;
											
											mh_col_add_filter( _col, COL_FILTER_ALBUM_ARTIST, _result2[inputnum], NULL);

											_result3	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &count, false );
									//		mh_col_filter_clear( _col );
											i = 0;
											for( i; i < count; i++)

											{
												printf("_result3[%d]:album_title%s--->album_artist:%s---->album_compliation:%d\n",
														i, _result3[i].album_title,_result3[i].album_artist,_result3[i].album_compliation);
											}

											while(1)
											{//song
												if( scanf("%d",&inputnum )!= 0)
												{
													if( inputnum > count || inputnum <0 )
													{
														g_message("wrong number\n");
													}
													else
													{
														MHPlaylist * _col_playlist;
														mh_col_add_filter( _col, COL_FILTER_ALBUM, &_result3[inputnum],NULL);
														_col_playlist	=	mh_col_create_playlist( _col, MH_ITEM_MUSIC,MH_MEDIA_MUSIC, false);

														mh_pb_play_by_list(	_pb, _col_playlist, 0 );	
														mh_playlist_foreach( _col_playlist, 0, -1, playlist_foreach_func,NULL);
													}

												}
											}
										}

									}

								}
							}
						}
					}
	
				}
				break;
			case 't':
				{
				//	MHPlaylist * _col_playlist;
					int col_count=0;
					int playlist_count =0 ;
					int inputnum;

				//	mh_col_set_filter( _col, MH_ITEM_MUSIC,MH_MEDIA_MUSIC, COL_FILTER_NONE, NULL, NULL);
					if( playlist !=	NULL)
					{
						mh_object_unref((MHObject *)playlist);
						playlist = NULL;
					}

					playlist	=	mh_col_create_playlist( _col, MH_ITEM_MOVIE,MH_MEDIA_MUSIC, false);
					MHCol * _col	=	mh_col_create( global_dev );

					mh_object_get_properties((MHObject * )_col, "count", &col_count, NULL);
					g_message( "col count: %d", col_count );

			
					mh_object_get_properties( ( MHObject * )playlist, "count", &playlist_count, NULL );
					g_message("playlist count:%d\n",playlist_count);
					mh_playlist_foreach( playlist,0, -1, playlist_foreach_func,NULL);

					if(scanf("%d", &inputnum) !=0 )
					{
						if( inputnum > playlist_count || inputnum <1 )
						{
							g_message("wrong number\n");
						}
						else
						{
							mh_pb_play_by_list(	_pb, playlist, inputnum-1 );	
						}
					}

				}

				break;
			//s--->compliation->album->song
			case 's':
				{
					MHPlaylist * _col_playlist;

					int inputnum;
					MHAlbumInfo * _result;
					int i;
					MHCol * _col	=	mh_col_create( global_dev );
					mh_col_add_filter( _col, COL_FILTER_COMPLIATION, 1);
					mh_col_set_retrieve_key( _col, COL_FILTER_ALBUM);
					_result	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &count, false );	
					for( i = 0; i < count; i++)
					{
						printf("_result[%d]:album_title%s--->album_artist:%s---->album_compliation:%d\n",
							i, _result[i].album_title,_result[i].album_artist,_result[i].album_compliation);
					}
					mh_col_filter_clear( _col );
					if( scanf( "%d", &inputnum) != 0)
					{
						mh_col_add_filter( _col, COL_FILTER_ALBUM, &_result[inputnum],NULL);
						_col_playlist = mh_col_create_playlist( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, false);
						mh_playlist_foreach( _col_playlist,0, -1, playlist_foreach_func,NULL);

						mh_pb_play_by_list( _pb, _col_playlist, 0 );				
					}
				}
				break;
			case 'a':
				{//all song
					MHPlaylist * _col_playlist;
					mh_col_filter_clear( _col);
					_col_playlist = mh_col_create_playlist( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC,false );
					mh_playlist_foreach( _col_playlist,0, -1, playlist_foreach_func,NULL);

					mh_pb_play_by_list( _pb, _col_playlist, 0 );		
				}
				break;
			case 'b':
				{//composer
					char ** _result1;
					int _count = 0;
					int i = 0 ;
					int inputnum;
					MHAlbumInfo * _result2;
					MHPlaylist * _col_playlist;
					mh_col_set_retrieve_key( _col, COL_FILTER_COMPOSER);
					_result1 = mh_col_retrieve_data( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &_count, false );
					for( i = 0; i < _count; i++)
					{
						printf("_result1[%d]:%s\n", i, _result1[i]);
					}
					if(scanf("%d", &inputnum )	!=	0)
					{
						mh_col_add_filter( _col, COL_FILTER_COMPOSER, _result1[inputnum], NULL);
						_result2	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &_count, false);
						for( i = 0; i < _count; i++)
						{
							printf("[%d]album_title:%s-------album_artist:%s------album_compliation:%d\n",i,
								_result2[i].album_title, _result2[i].album_artist, _result2[i].album_compliation);
						}
						if( scanf("%d", &inputnum) != 0)
						{
							mh_col_add_filter( _col, COL_FILTER_ALBUM, &_result2[inputnum], NULL);
							_col_playlist	=	mh_col_create_playlist( _col,  MH_ITEM_MUSIC, MH_MEDIA_MUSIC, false);
							mh_playlist_foreach( _col_playlist,0, -1, playlist_foreach_func,NULL);

							mh_pb_play_by_list( _pb, _col_playlist, 0 );	

						}
					}




				}
				break;
			case 'y':
				{//有声读物
					MHAlbumInfo * _result1;
					int _count = 0;
					int i = 0;
					int inputnum;
					MHPlaylist * _col_playlist;

					_result1	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_AUDIOBOOK, &_count, false);
					for( i = 0; i < _count; i++)
					{
						printf("[%d]album_title:%s-------album_artist:%s------album_compliation:%d\n",i,
								_result1[i].album_title, _result1[i].album_artist, _result1[i].album_compliation);
					}
					if( scanf("%d", &inputnum) != 0)
					{
						mh_col_add_filter( _col, COL_FILTER_ALBUM, &_result1[inputnum], NULL);
						_col_playlist	=	mh_col_create_playlist( _col,  MH_ITEM_MUSIC, MH_MEDIA_AUDIOBOOK, false);
						mh_playlist_foreach( _col_playlist,0, -1, playlist_foreach_func,NULL);

						mh_pb_play_by_list( _pb, _col_playlist, 0 );	
					}
				}
				break;
			case 'd':
				{//folder all song
					int count = 0;
					mh_object_get_properties( ( MHObject * )global_dev, "base", &_folder, NULL );
					playlist = mh_folder_create_playlist(_folder, _music_filter, true); 
					mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func, NULL);
					mh_object_get_properties( (MHObject *)playlist, "count", &count, NULL);
					printf("\n\n\nplaylist--->count:%d\n\n\n",count);
				}
				break;
			case 'n':
				{//search file name
					printf("\n\nname\n\n");
					mh_col_add_filter(_col, COL_FILTER_NAME, "City", NULL);
					MHPlaylist * _col_playlist;
					_col_playlist	=		mh_col_create_playlist( _col, MH_ITEM_MUSIC,MH_MEDIA_NONE, false);
					mh_playlist_foreach(_col_playlist, 0, -1, playlist_foreach_func, NULL);


				}
				break;
			case 'c':
				mh_dev_scan_abort(global_dev);
				mh_misc_db_restore();
				break;
			case 'k':
			{
//				int inputnum;
//				int i=0;
//				MHAlbumInfo * _info;
//				MHPlaylist * _col_playlist;
//							int _count;
//				_col	=	mh_col_create( global_dev );
//				_info	=	mh_col_retrieve_album( _col, MH_ITEM_MUSIC, MH_MEDIA_MUSIC, &_count, false);
//				for( i; i < _count; i++)
//				{
//					printf("[%d]album_title:%s-------album_artist:%s------album_compliation:%d\n", i, _info[i].album_title, _info[i].album_artist, _info[i].album_compliation);
//				}
//				if(scanf("%d", &inputnum )	!=	0)
//				{
//					printf("[%d]album_title:%s-------album_artist:%s------album_compliation:%d",inputnum,
//					_info[inputnum].album_title, _info[inputnum].album_artist, _info[inputnum].album_compliation);
//					mh_col_add_filter( _col, COL_FILTER_ALBUM, &_info[inputnum],NULL);
//					_col_playlist	=	mh_col_create_playlist( _col,  MH_ITEM_MUSIC, MH_MEDIA_MUSIC, false);
//					mh_playlist_foreach( _col_playlist,0, -1, playlist_foreach_func,NULL);
//
//					mh_pb_play_by_list( _pb, _col_playlist, 0 );	
//				}
				mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY );
			}
				break;
			case 'w':
				{
//					MHPlaylist * scan_playlist;

//					scan_playlist	=	mh_dev_create_empty_playlist( global_dev );
//					
//					MHFolder * _folder;
//					mh_object_get_properties( ( MHObject * )global_dev, "base", &_folder, NULL );
//					scan_folder( _folder);	
//					mh_playlist_foreach( scan_playlist,0,-1, playlist_foreach_func, NULL);
					mh_misc_set_iap_device_mode( MISC_IAP );
				}
				break;
			case 'h':
				mh_pb_set_pipeline_status(_pb, 1);
//				print_help();
			default:
				break;
			}

		}
	}
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  item_foreach
 *  Description:  
 * =====================================================================================
 */
static bool item_foreach( MHItem * item, MHItemData * data, void * user_data)
{
	MHItem ** _items;
	int _count	=	-1;
	if( data->type == 	MH_ITEM_FOLDER)
	{
		_items	=	mh_folder_get_children( (MHFolder *)item, _music_filter, FOLDER_BEGIN, &_count, MH_ITEM_ORDER_BY_DEFAULT);
		if( _items != NULL)
		{
			mh_item_foreach( _items, _count, item_foreach,NULL );		
		}

	}	
	else
	{
		mh_playlist_append( scan_playlist, &item, 1);
		return true;
	}
	return false;
}		/* -----  end of static function item_foreach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  scan_folder
 *  Description:  
 * =====================================================================================
 */
static void scan_folder(  MHFolder * folder )
{
	int _count = -1;
	int i = 0;
	MHItem ** _items	=	mh_folder_get_children( folder, _music_filter, FOLDER_BEGIN, &_count, MH_ITEM_ORDER_BY_DEFAULT);
	if( _items != NULL)
	{
		mh_item_foreach( _items, _count, item_foreach, NULL);		
	}


}		/* -----  end of static function scan_folder  ----- */
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
	mh_misc_set_filter(MH_MISC_FILTER_MUSIC,"mp3;wma;aac;wav;ogg;mp2",
					   MH_MISC_FILTER_MOVIE,"mp4;3gp;avi;mkv;mpg;mpeg;wmv;vob;flv;swf;mov;dat",
					   MH_MISC_FILTER_PICTURE,"jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif",NULL);

	_all_filter	=	NULL;
	_music_filter	=	mh_filter_create("mp3;wma;aac;wav;ogg;mp2;mp4;mpg;mpeg");
//	_music_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
	_movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
	_picture_filter	=	mh_filter_create("jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif");
	_folder_filter	=	mh_filter_create("");

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );
	mh_misc_set_iap_device_mode( MISC_IAP );
	mh_core_start();

	 _pb = mh_pb_create();
	MHPbEventsListener _listener =
	{
		.callback = _pbs_callback,
		.user_data = NULL  
	};
	mh_pb_register_events_listener( _pb, &_listener );

	MHPbStatusListener _status_listener =
	{
		.callback = _pbs_status_callback,
		.user_data = NULL  
	};
	mh_pb_register_status_listener( _pb, &_status_listener );

	g_thread_new( "mh_test_input", input_thread, NULL );
	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

