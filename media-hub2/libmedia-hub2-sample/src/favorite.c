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
MHDev * storage_dev ;

static GMainLoop *test_loop;
MHFilter * _folder_filter;
MHFilter * _all_filter;
MHFilter * _music_filter;
MHFilter * _movie_filter;
MHFilter * _picture_filter;
static int displayIndex;
static int folderCount;
MHPb * _pb;
static MHItem ** _get_children( MHFolder * self,MHFilter * filter, MHFolderPosition pos,int * count );
MHPlaylist * scan_playlist;
static void scan_folder( MHFolder * folder );

MHCol * _favorite;

MHDev * emmc_dev;
MHCol *	_emmcCol;
MHPlaylist * favoritePlaylist	=	NULL;
int64_t	favoritePlaylistId;
bool _exist_favorite	=	false;
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

bool playlist_foreach_func( void * data, void * user_data)
{
	printf("-------------------------------------------------->\n");
	MHItemData * _data	=	(MHItemData *)data;
	printf("type:%d\nuri:%s\nname:%s\nsize:%lld\n,enable:%d\n,id:%lld\n",_data->type,_data->uri,_data->name,(long long int)_data->size,_data->valid, (long long int)_data->uniqueId);
	printf("title:%s\n",_data->metadata.music.title);
	printf("mediaType:%d\n",_data->metadata.music.mediaType);
	printf("-------------------->Favorite:%d\n", _data->favorite);
	printf("duration--------------->%d", _data->metadata.music.duration);
	return false;
}
bool playlist_show( void * data, void * user_data)
{
	int * num	=	(int *)user_data;
	MHItemData * _data	=	(MHItemData *)data;
	printf("[%d]-->name:%s --->title:%s  --->artist:%s \n", *num, _data->name, _data->metadata.music.title, _data->metadata.music.artist );
	*num	=	*num + 1;
	return false;
}

bool playlist_get_uri( void * data, void * user_data)
{
	char ** _path	=	(char **)user_data;
	MHItemData * _data	=	(MHItemData *)data;
	*_path	=	g_strdup( _data->uri);
	g_message("%s--->%s", __func__,  *_path);
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
	storage_dev	=	dev;

	int i=0;
	bool _exist = false;
	int64_t	_id;
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
	
	}
	
	else if( scan_type == MH_DEV_FINISH )
	{
		}

								
	

}		/* -----  end of static function _register_scan_callback  ----- */
static MHPlaylist * check_database_playlist( MHDev * dev, char * playlistName, int64_t * playlist_id)
{
	MHPlaylist * _res	=	NULL;

	int64_t playlistId	=	0;
	MHPlaylistInfo * _info	=	mh_dev_get_playlist_info( dev );
	if( _info != NULL)
	{
		int i	=	0;
		while( _info[i].playlistid != 0)
		{
			g_message("_info[%lld].playlistid:%lld, _info[%lld]:favorite_list:%s",i, _info[i].playlistid, i, _info[i].name );
			if(g_strcmp0( _info[i].name, playlistName) == 0)
			{

				_res	=	mh_dev_restore_playlist( dev, _info[i].playlistid);

				* playlist_id	=	_info[i].playlistid;

				g_message("%d-->* playlist_id:%lld",__LINE__,  * playlist_id);
		
				break;
			}
			i++;
		}
		i	=	0;
		while( _info[i].playlistid	!= 	0)
		{
			free( _info[i].name);

			i++;
		}
		free( _info);
	
	}
	return _res;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _scan_callback
 *  Description:  
 * =====================================================================================
 */
static void _emmc_event(MHDev * dev,  MHDevScanCbType scan_type, MHItemType item_type, void *  data, uint32_t pefrsent,void * user_data)
{
	if( (scan_type == MH_DEV_FIRST_FILE) && (item_type == MH_ITEM_MUSIC) )
	{

	}
	else if( scan_type	==	MH_DEV_FINISH)
	{
		int64_t	_playlist_id	=	0;
		MHPlaylist * _playlist;

		g_message("\n\n\n\nemmc scan finish\n\n\n\n");
		_playlist	=	check_database_playlist( emmc_dev, "favorite_list", & _playlist_id);

		if( _playlist !=	NULL)
		{
			favoritePlaylist	=	_playlist;
			favoritePlaylistId	=	_playlist_id;
			int * num	=	(int *)g_new0( int, 1);
			mh_playlist_foreach( favoritePlaylist, 0, -1, playlist_show, num);
			free( num);
			mh_pb_play_by_list( _pb, favoritePlaylist, 0);
			
		}
		else
		{
			favoritePlaylist	=	mh_dev_create_empty_playlist( emmc_dev);
			mh_dev_save_playlist(emmc_dev, "favorite_list", favoritePlaylist);

		}

	}



}		/* -----  end of static function _register_scan_callback  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _emmc_detach
 *  Description:  
 * =====================================================================================
 */
static void _emmc_detach(	MHDev * dev, void * user_data)
{
	g_message("%s", __func__);
}		/* -----  end of static function _emmc_detach  ----- */

static int __count=0;

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	printf("-------------------------------------------------------------------->count:%d\n",__count++);
	char *_serial, *_type, *_entry, *_dev_type;
	MHFolder * _folder;
	uint32_t _entryNum;
	printf( "Device[ %p ] status[ %d ]\n", dev, event );
	
	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);
	
	if ( _dev_type && !g_strcmp0(_dev_type, "emmc"))
	{
		mh_object_get_properties((MHObject *)dev, "entry_number", &_entryNum, NULL);
		g_message("entry_number=====%d", _entryNum);
		if( _entryNum	==	15)
		{
			g_message("\n\n\n\nright emmc devices\n\n\n");
			MHDevEventsListener _emmc_listener	=	
			{
				.callback	=	_emmc_event,
				.user_data	=	NULL
			};
			MHDevDetachListener _emmc_detach_listener	=	
			{
				.callback	=	_emmc_detach,
				.user_data	=	NULL
			};
			mh_object_ref((MHObject *)dev);
			emmc_dev	=	dev;
			mh_dev_register_events_listener( emmc_dev, &_emmc_listener);
			mh_dev_register_detach_listener( emmc_dev, &_emmc_detach_listener);
			_emmcCol	=	mh_col_create( emmc_dev);

		
			mh_dev_start_scan( emmc_dev, SCAN_TAG);

		}
	}
	else if( _dev_type && !g_strcmp0( _dev_type, "storage"))
	{
		MHDevEventsListener _event_listener		=
		{
			.callback	=	_dev_event,
			.user_data	=	NULL
		};


		mh_object_ref((MHObject *)dev);
		mh_object_get_properties( ( MHObject * )dev, "serial", &_serial, "fs_type", &_type, "entry", &_entry, NULL );
		printf( "device properties:\nserial:\t%s\nfs_type:\t%s\nentry:\t%s\n", _serial, _type, _entry );

		uint64_t _total, _free;		
		mh_object_get_properties( ( MHObject * )dev, "total", &_total,"free", &_free, NULL);
		printf("_total:%lld,free:%lld\n",(long long int)_total, (long long int)_free);
		free( _serial );
		free( _type );
		free( _entry );
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
	    mh_dev_register_detach_listener( dev, & _detach_listener);
		mh_dev_register_events_listener( dev, & _event_listener);

		MHPlaylistInfo * _info	=	mh_dev_get_playlist_info( dev);
		int i=0;
		mh_dev_start_scan( dev,SCAN_TAG);
		_favorite	=	mh_col_create( dev);
		storage_dev	=	dev;
	}
	else
	{
		printf(" I don't know the device:%s\n", _dev_type);
	}

	free( _dev_type );
	
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
	_items	=	mh_folder_get_children( self,filter, FOLDER_BEGIN, count, MH_ITEM_ORDER_BY_PINYIN );
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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  delete_item
 *  Description:  
 * =====================================================================================
 */
static bool delete_item( void * data, void * user_data)
{
	gpointer * _user_data	=	(gpointer *)user_data;
	uint64_t * _id	=	(uint64_t *)_user_data[0];
	int * _index	=	(int *)_user_data[1];
	MHItemData * _data	=	(MHItemData *)data;
	if( _data->uniqueId	==	*_id)
	{
		mh_playlist_remove( favoritePlaylist, *_index, 1);
		return true;
	}
	else
	{
		* _index =	* _index + 1;
		return false;
	}
}		/* -----  end of static function delete_item  ----- */
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
	print_help();
	bool _first_enter	=	true;
	MHFolder * _base;
	MHPlaylist * _allSong;
	while(1)
	{
		if(scanf("%s", input)!=0)
		{
			printf( "copy file to emmc\n\n" );
			switch( input[0] )
			{
				case 'f':

					mh_object_get_properties( ( MHObject * )storage_dev, "base", &_folder, NULL );
					_base 	=	_folder;
					_items	=	_get_children( _folder, _music_filter, FOLDER_BEGIN, &count);
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
									mh_object_get_properties( ( MHObject * )storage_dev, "base", &_folder, NULL );

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
								mh_object_get_properties((MHObject *)_items[inputnum-1], "type", &_type, NULL);
								if( _type	==	MH_ITEM_FOLDER)
								{
									_folder	=	(MHFolder *)_items[inputnum-1];
									count	=	-1;
									free( _items );
									_items	=	_get_children( _folder, _music_filter, FOLDER_BEGIN, &count);
									
								}
								else
								{
									char * _sourcePath, *_itemPath, *_itemName;
									char * _emmcPath;
									char * _cmd;
									MHItem * _newItem, * _oldItem;
						
									MHFolder * _emmcFolder;
									mh_object_get_properties( (MHObject *) _items[inputnum -1], "uri", &_sourcePath, "name", &_itemName, NULL); 
									mh_object_get_properties( (MHObject *) emmc_dev, "base", &_emmcFolder, NULL);

									mh_object_get_properties( (MHObject *) _emmcFolder, "uri", &_emmcPath, NULL);

									_cmd	=	g_strdup_printf("cp \"%s\" \"%s\"", _sourcePath, _emmcPath);

									_itemPath	=	g_strdup_printf("%s/%s", _emmcPath, _itemName);
									if( access( _itemPath, F_OK) == 0)
									{
										g_message("if y will cover old file");
										if( scanf("%s", input) != 0	&& input[0] == 'y' && system(_cmd) != -1)
										{
											_oldItem	=	mh_dev_get_item_by_uri( emmc_dev, _itemPath);
											uint64_t * _oldUniqueId	=	g_new0( uint64_t, 1);
											int * num	=	(int *)g_new0( int, 1);
											mh_object_get_properties( (MHObject *)_oldItem, "unique-id", _oldUniqueId, NULL);


											gpointer * user_data	=	(gpointer *)g_new0( gpointer, 2);
											user_data[0]	=	(gpointer)_oldUniqueId;
											user_data[1]	=	(gpointer)num;

											mh_playlist_foreach( favoritePlaylist, 0, -1, delete_item, user_data);
											_newItem	=	mh_dev_add_file( emmc_dev, _emmcFolder, _items[inputnum -1]);

											mh_playlist_insert( favoritePlaylist, *num, &_newItem, 1); 
											if( favoritePlaylistId == 0)
											{
												check_database_playlist( emmc_dev, "favorite_list", &favoritePlaylistId);
											}
											g_message("%d-->favoritePlaylistId:%lld",__LINE__,  favoritePlaylistId);
											mh_dev_update_playlist( emmc_dev, "favorite_list", favoritePlaylist, favoritePlaylistId);
											mh_playlist_foreach( favoritePlaylist, 0, -1,  playlist_foreach_func, NULL);

										}
										else
										{
									
										}
										break;
									}
									g_message("the item :%s will  copy to emmc:%s", _sourcePath, _emmcPath );
									g_free( _sourcePath);
									g_free( _emmcPath );
									g_message("_cmd:%s", _cmd);
									if(system( _cmd) == -1)
									{
										g_message("copy is fail");


									}
									else
									{
									
										g_message("copy is OK");
										MHItem *_item;
										_item	=	mh_dev_add_file( emmc_dev, _emmcFolder, _items[inputnum-1]);
										
										mh_playlist_append(favoritePlaylist, &_item, 1);
										if( favoritePlaylistId == 0)
										{
											check_database_playlist( emmc_dev, "favorite_list", &favoritePlaylistId);
										}
				g_message("%d-->favoritePlaylistId:%lld",__LINE__,  favoritePlaylistId);
										mh_dev_update_playlist( emmc_dev, "favorite_list", favoritePlaylist, favoritePlaylistId);
										mh_playlist_foreach( favoritePlaylist, 0, -1,  playlist_foreach_func, NULL);
										break;
										
									}

								}
							}
						}//if(scanf("%d", &inputnum)!=0
					}//while(1)
				    break;
					case 'm':
					{
						int inputnum;
						int * num	=	(int *)g_new0(int, 1);
						mh_playlist_foreach(favoritePlaylist, 0, -1, playlist_show, num );
						printf("move ");
						if(scanf("%d", &inputnum) != 0)
						{
							char * _path;
							MHItem * _item;
							mh_playlist_foreach(favoritePlaylist, inputnum, 1, playlist_get_uri, &_path);
							g_message("path:%s", _path);
							_item 	=	mh_dev_get_item_by_uri(emmc_dev, _path);
							mh_playlist_remove(favoritePlaylist, inputnum, 1);
							printf("to ");
							if( scanf("%d", &inputnum) != 0)
							{
								mh_playlist_insert(favoritePlaylist, inputnum, &_item, 1);
								if( favoritePlaylistId == 0)
								{
									check_database_playlist( emmc_dev, "favorite_list", &favoritePlaylistId);
								}

								g_message("%d-->favoritePlaylistId:%lld",__LINE__,  favoritePlaylistId);
								mh_dev_update_playlist( emmc_dev, "favorite_list", favoritePlaylist, favoritePlaylistId);
								*num=0;
								mh_playlist_foreach( favoritePlaylist, 0, -1, playlist_show, num);
							}
						}
					}

					break;
					case 'd':
					{
						int inputnum;
						int * num	=	(int *)g_new0( int, 1);
						char * _cmd;
						char * _path;
						*num	=	0;	
						mh_playlist_foreach( favoritePlaylist, 0, -1, playlist_show, num );
						printf("delete ");
						if(scanf("%d", &inputnum)!=0)
						{
							mh_playlist_foreach( favoritePlaylist, inputnum, 1, playlist_get_uri, &_path);
							_cmd	=	g_strdup_printf("rm \"%s\"", _path);
							g_message("_cmd:%s", _cmd);
							if(system( _cmd ) == -1)
							{
								g_message("%s is failed", _cmd);
							}
							else
							{
								g_message("%s is Success", _cmd);
								mh_playlist_remove( favoritePlaylist, inputnum, 1);
								if( favoritePlaylistId == 0)
								{
									check_database_playlist( emmc_dev, "favorite_list", &favoritePlaylistId);
								}

								g_message("%d-->favoritePlaylistId:%lld",__LINE__,  favoritePlaylistId);
								mh_dev_update_playlist( emmc_dev, "favorite_list", favoritePlaylist, favoritePlaylistId);

								*num	=	0;	
								mh_playlist_foreach( favoritePlaylist, 0, -1, playlist_show, num );

							}


						}

					}
					break;
					default:
						break;

				}//	switch( input[0] )


			}//	if(scanf("%s", input)!=0)

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
	_music_filter	=	mh_filter_create("mp3;wma;aac;wav;ogg;mp2");
	_movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
	_picture_filter	=	mh_filter_create("jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif");
	_folder_filter	=	mh_filter_create("");

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );
	mh_core_start();

	 _pb = mh_pb_create();


	g_thread_new( "mh_test_input", input_thread, NULL );
	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

