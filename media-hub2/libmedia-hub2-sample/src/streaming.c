/*
 * =====================================================================================
 *
 *       Filename:  streaming.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2015 05:57:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifdef USE_MH2_IPC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mh_streaming.h>
MHFilter * _folder_filter;
MHFilter * _all_filter;
MHFilter * _music_filter;
MHFilter * _movie_filter;
MHFilter * _picture_filter;
static MHPlaylist * playlist	=	NULL;
static MHStreamingPb * pb;
MHDev * global_dev;
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

void streaming_pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data )
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
		default:
			break;			
	}
}

bool playlist_foreach_func( void * data, void * user_data)
{
	printf("-------------------------------------------------->\n");
	MHItemData * _data	=	(MHItemData *)data;
	printf("type:%d\nuri:%s\nname:%s\nsize:%lld\n,enable:%d\n,id:%llX\n",_data->type,_data->uri,_data->name,_data->size,_data->valid,_data->uniqueId);
	printf("title:%s\n",_data->metadata.music.title);
	printf("mediaType:%d\n",_data->metadata.music.mediaType);
//	char * _cover_path;
//	if( MISC_OK == mh_file_get_cover(_data->uri, COVER_SAVE_PATH, COVER_MAX_SIZE,  (&_cover_path)))//4M=4194304字节
//	{
//		printf("_cover_path:%s\n", _cover_path);
//	}
	return false;
}


static void _dev_event(MHDev * dev,  MHDevScanCbType scan_type, MHItemType item_type, uint32_t data, uint32_t persent,void * user_data)
{
	if( scan_type == MH_DEV_FIRST_FILE )
	{
		if( item_type == MH_ITEM_MUSIC)
		{
			MHFolder * _folder	=	( MHFolder * )data;
			MHFilter * _music_filter	=	mh_filter_create("mp3;wma;aac;wav;ogg;mp2");

			playlist	=	mh_folder_create_playlist( _folder, _music_filter, false );
			mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func, NULL);
			mh_streaming_pb_play_by_list( pb, playlist, 0 );

		}
//		if( item_type == MH_ITEM_MOVIE )
//		{
//			MHFolder * _folder	=	( MHFolder * )data;
//			MHFilter * _movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
//			
////			mh_streaming_pb_set_properties( pb, "video_sink", "glimagesink", NULL);
//			playlist	=	mh_folder_create_playlist( _folder, _movie_filter, false );
//			mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func, NULL);
//			mh_streaming_pb_play_by_list( pb, playlist, 0 );
//		}
	}
}

static void _detach_event(	MHDev * dev, void * user_data)
{
	
}

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	global_dev 	=	dev;
	MHDevEventsListener _event_listener		=
	{
		.callback	=	_dev_event,
		.user_data	=	NULL
	};
	MHDevDetachListener _detach_listener	=	
	{
		.callback	=	_detach_event,
		.user_data	=	NULL
	};
	char *_serial, *_type, *_entry, *_dev_type;

	mh_object_ref( ( MHObject * )dev );

	printf( "Device[ %p ] status[ %d ]\n", dev, event );

	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);

	if( _dev_type && strcmp( _dev_type, "storage" ) == 0 )
	{
	    mh_dev_register_detach_listener( dev, & _detach_listener);
		mh_dev_register_events_listener( dev, & _event_listener);

		mh_dev_start_scan( dev, SCAN_FOLDER );
	}
}
static int folderCount;
static int displayIndex;
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

static MHItem ** _get_children( MHFolder * self,MHFilter * filter, MHFolderPosition pos,int * count )
{
	MHItem ** _items	=	NULL;
	_items	=	mh_folder_get_children( self,filter, FOLDER_BEGIN, count, MH_ITEM_ORDER_BY_PINYIN );
	printf("count = %d\n",*count);
	int i	=	0;
	MHItemType _type;
	char * name;

	folderCount	=	0;
	displayIndex	=	1;

	if( *count > 0 )
	mh_item_foreach( _items, * count, _each_item, NULL );
	return _items;
}
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

	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	MHFolder * _folder;	
	char * _name, * _path;
	MHItem ** _items;
	int count	=	-1;
	MHItemType _type;
	pb	=	mh_streaming_pb_create(_conn);
	MHPbEventsListener _listener =
	{
		.callback = streaming_pbs_callback,
		.user_data = NULL  
	};
	mh_streaming_pb_register_events_listener( pb, &_listener );

	mh_streaming_pb_set_properties( pb, "shared", true, NULL);
	mh_streaming_pb_set_properties( pb, "video_sink", "glimagesink", "window_layer", 1,
			"window_x", 20, "window_y", 20, "window_width", 200, "window_height", 200, NULL);

	mh_ipc_media_client_init( _conn );
	mh_ipc_streaming_client_init( _conn );

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );

	mh_misc_set_filter(MH_MISC_FILTER_MUSIC,"mp3;wma;aac;wav;ogg;mp2",
					   MH_MISC_FILTER_MOVIE,"mp4;3gp;avi;mkv;mpg;mpeg;wmv;vob;flv;swf;mov;dat",
					   MH_MISC_FILTER_PICTURE,"jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif",NULL);
	_all_filter	=	NULL;
	_music_filter	=	mh_filter_create("mp3;wma;aac;wav;ogg;mp2");
	_movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
	_picture_filter	=	mh_filter_create("jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif");
	_folder_filter	=	mh_filter_create("");

	mh_core_start();

	while( true )
	{
		int _cmd	=	getchar();

		printf( "$" );

		fflush( stdout );

		switch( _cmd )
		{
			case 'f':
			{
				mh_object_get_properties( (MHObject *)global_dev, "base", &_folder, NULL);
				mh_object_get_properties( ( MHObject * )_folder, "name", &_name, NULL);
				printf( "name %s\n", _name );
				mh_object_get_properties( (MHObject *)_folder, "uri", &_path, NULL );
				printf("current path:%s\n",_path);
				_items	=	_get_children( _folder, _movie_filter, FOLDER_BEGIN, &count);
				free( _path );
				free( _name );
				while( 1 )
				{
					int inputnum;
					scanf("%d", &inputnum); 	
					if( inputnum == 0)
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
						_items	=	_get_children( _folder, _movie_filter, FOLDER_BEGIN, &count);
					}
					else
					{
						mh_object_get_properties((MHObject *)_items[inputnum - 1], "type", &_type, "name", &_name, NULL);
						if( _type	==	MH_ITEM_FOLDER)
						{
							_folder	=	( MHFolder * )_items[inputnum-1];
							count	=	-1;

							printf("inputnum is %d\n",inputnum);
							if( _items !=	NULL)
							{
								free( _items );
								_items	=	NULL;
							}
							_items	=	_get_children(_folder,_movie_filter, FOLDER_BEGIN, &count );	
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
							playlist	=	mh_folder_create_playlist( _folder,_movie_filter, false )	;
							mh_object_get_properties(( MHObject * )playlist, "count", &count, NULL );
							printf( "get playlist: %p, count is: %d\n", playlist, count );
							int index = inputnum;
							index	=	index - folderCount-1;
							printf("index= %d\n",index );
							printf("---------------------------------->index=%d,folderCount=%d\n",index, folderCount);
							mh_playlist_foreach( playlist, 0, -1, playlist_foreach_func, NULL);
							mh_streaming_pb_play_by_list( pb, playlist, index );

						}


					}

				}
			}

			break;
		case 'x':
			goto LOOP_END;
			break;
		case '>':
			mh_streaming_pb_next( pb );
			break;
		case '<':
			mh_streaming_pb_previous( pb );
			break;
		case 'p': // Play.
			mh_streaming_pb_play( pb );
			break;
		case 'b': // Stop.
			mh_streaming_pb_stop( pb );
			break;
		case 'a': // Pause.
			mh_streaming_pb_pause( pb );
			break;
		case 'i':
			{
				int index;
				if(scanf("%d", &index) !=0 ){
					mh_streaming_pb_set_properties( pb, "index", index, NULL);
				}
			}
			break;
		case 'o':
			mh_streaming_pb_set_properties( pb, "repeat", MH_PB_REPEAT_MODE_ONE, NULL);
			break;
		case 't':
			mh_streaming_pb_set_properties( pb, "repeat", MH_PB_REPEAT_MODE_ALL, NULL);
			break;
		case 'm':
			mh_streaming_pb_set_properties( pb, "shuffle", MH_PB_SHUFFLE_ALL, NULL );
			break;
		case 'n':
			mh_streaming_pb_set_properties( pb, "shuffle", MH_PB_SHUFFLE_OFF, NULL );
			break;
		case 'r':
			mh_streaming_pb_resize( pb, 20,20,400,400);
		default:
			break;
		}
	}

LOOP_END:
	return 0;
}				/* ----------  end of function main  ---------- */
#else

#include <stdio.h>
#include <stdlib.h>
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	printf( "This program can only be used under USE_MH2_IPC definition.\n" );

	return 0;
}				/* ----------  end of function main  ---------- */
#endif
