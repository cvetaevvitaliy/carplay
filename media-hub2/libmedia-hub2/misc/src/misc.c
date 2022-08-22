/*
 * =====================================================================================
 *
 *       Filename:  misc.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/03/2014 11:38:44 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <mh_misc.h>
#include <string.h>
#include <mh_item.h>
#include <mh_contents.h>
#include <stdlib.h>
#include <tag_c.h>
#include <mh_api.h>
#include <mh_carplay.h>
#include <iconv.h>
#include <errno.h>
static char * music[]={ 
			"mp3",
			"wma",	
			"aac",
			"wav",
			"ogg",
			"mp2"
};
static char * movie[]={
			"mp4",
			"3gp",
			"avi",
			"mkv",
			"mpg",
			"mpeg",
			"wmv",
			"vob",
			"flv",
			"swf",
			"mov",
			"dat"
};
static char * picture[]={
			"jpg",
			"png",
			"bmp",
			"jpeg",
			"jfif",
			"jpe",
			"ico",
			"gif",
			"tiff",
			"tif"
};

static char * subtitile[]={
			"srt",
			"smi",
			"ssa"
};
static char * playlist[]={
			"m3u"
};
static GHashTable * typeHash	=	NULL;

static MHItemType musicType	=	MH_ITEM_MUSIC;
static MHItemType movieType	=	MH_ITEM_MOVIE;
static MHItemType pictureType	=	MH_ITEM_PICTURE;
static MHItemType playlistType	=	MH_ITEM_PLAYLIST;

static MiscIapDeviceMode iapMode	=  MISC_IAP_CARPLAY;
static MiscIapDeviceMode localIapMode = MISC_IAP_CARPLAY;
static char * bluetoothids	=	NULL;
static MHPb * pb_handle	=	NULL;
static uint8_t * init_modes = NULL;
static uint32_t rightHand = 0;

///* 
// * ===  FUNCTION  ======================================================================
// *         Name:  mh_misc_init
// *  Description:  
// * =====================================================================================
// */
//void mh_misc_init()
//{
//	int i;
//
//	typeHash	=	g_hash_table_new( g_str_hash, g_str_equal );
//
//
//	for( i = 0; i < sizeof( music ) / sizeof ( char * ); i ++ )
//	{
//		g_hash_table_insert( typeHash, music[ i ], &musicType );
//	}
//
//	for( i = 0; i < sizeof( movie ) / sizeof ( char * ); i ++ )
//	{
//		g_hash_table_insert( typeHash, movie[ i ], &movieType );
//	}
//
//	for( i = 0; i < sizeof( picture ) / sizeof ( char * ); i ++ )
//	{
//		g_hash_table_insert( typeHash, picture[ i ], &pictureType );
//	}
//	for( i = 0; i < sizeof( playlist ) /sizeof ( char * );i ++)
//	{
//		g_hash_table_insert( typeHash, playlist[ i ], &playlistType );
//	}
//}		/* -----  end of function mh_misc_init  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_file_type
 *  Description:  
 * =====================================================================================
 */
MHItemType mh_misc_get_file_type( gchar * name )
{
	MHItemType _result	=	MH_ITEM_NONE;

	gchar * _ext	=	g_strrstr( name, "." );

	if( _ext != NULL )
	{
		MHItemType * _type;

		_ext	=	g_ascii_strdown( _ext + 1, strlen( _ext + 1 ));

		_type	=	g_hash_table_lookup( typeHash, _ext );

		g_free( _ext );

		if( _type != NULL )
		{
			_result	=	 * _type ;
		}
	}

	return _result;
}		/* -----  end of function mh_misc_get_file_type  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_ext_type
 *  Description:  
 * =====================================================================================
 */
MHItemType mh_misc_get_ext_type(gchar * ext)
{
	MHItemType _result	=	MH_ITEM_NONE;

	gchar * _ext;
	if( ext != NULL )
	{
		MHItemType * _type;

		_ext	=	g_ascii_strdown( ext , strlen( ext));

		_type	=	g_hash_table_lookup( typeHash, _ext );

		if( _type != NULL )
		{
			_result	=	 * _type ;
		}
		g_free( _ext );
	}

	return _result;
}		/* -----  end of function mh_misc_get_ext_type  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_filter
 *  Description:  
 * =====================================================================================
 */
static MHResult _set_filter(MHMiscFilterType type, const char * filter)
{
	MHResult _res	=	MH_OK;
	char * _p;
	char * _key;
	char * _filter;
	MHItemType *_type;
	
	switch( type)
	{
		case  MH_MISC_FILTER_MUSIC:
			_type	=	&musicType;
			break;
		case MH_MISC_FILTER_MOVIE:
			_type	=	&movieType;
			break;
		case MH_MISC_FILTER_PICTURE:
			_type	=	&pictureType;
			break;
		case MH_MISC_FILTER_PLAYLIST:
			_type	=	&playlistType;
			break;
		default:
			_res	=	MH_INVALID_PARAM;
			return _res;
	}

	_filter	=	g_strdup( filter );

	_p	=	g_strrstr( _filter, ";");
	while( _p != NULL)
	{
		_key	=	g_ascii_strdown( _p + 1, strlen( _p +1 ));	

		g_hash_table_insert( typeHash, _key, _type);

		* _p	=	'\0';
		
		_p	=	g_strrstr( _filter, ";");
	}
	_key	=	g_ascii_strdown( _filter, strlen( _filter ));

	g_hash_table_insert( typeHash, _key, _type);

	g_free( _filter);
	return _res;
}		/* -----  end of static function _set_filter  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_filter
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_filter( MHMiscFilterType type, const char * filter, ... )
{
	MHResult _res	=	MH_OK;

	va_list _ap;
	void * _param;
	MHMiscFilterType _type;
	char * _filter;

	if( typeHash	==	NULL)
	{
		typeHash	=	g_hash_table_new( g_str_hash, g_str_equal );

	}
	_res	=	_set_filter( type, filter);
	if( _res == MH_OK)
	{
		va_start( _ap, filter );

		while(( _param	=	va_arg( _ap, char *)) != NULL)
		{
			_type	=	(MHMiscFilterType)_param;
			_filter	=	(char *)va_arg( _ap, char *);
			_res	=	_set_filter( _type, _filter);
			g_message("_set_filter return %d", _res);
			if( _res	!= 	MH_OK)
			{
				break;
			}
		}

		va_end( _ap );
	}
	return _res;

}		/* -----  end of function mh_misc_set_filter  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_reset
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_reset()
{
	MHResult _res	=	MH_OK;

	char * _path, *_db_path;
	char * _cmd;
	MHContents * _contents;

	_contents	=	mh_contents_instance();

	if(mh_contents_deinit( _contents )	==	TRUE)
	{

		_path	=	getenv("MH_DB_PATH");

		_path	=	_path ? _path : "/var";

		_db_path	=	g_strdup_printf("%s/mediahub.db", _path);

		if( remove((const char *) _db_path) != 0)
		{
			g_message("remove %s error", _db_path);
		}
		g_free( _db_path);

	}
	else
	{
		g_message("mh_contents_deinit failed");
	}
	return _res;

}		/* -----  end of function mh_misc_reset  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_db_restore
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_db_restore()
{
	MHResult _res	=	MH_OK;
	MHContents * _contents;

	_contents	=	mh_contents_instance();
	
	mh_contents_db_restore( _contents );
	return _res;
	
}		/* -----  end of function mh_misc_db_restore  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_db_delete_by_serial_number
 *  Description: 
 * =====================================================================================
 */
MHResult mh_misc_db_delete_by_serial_number(const char * serialNum) //del db by deviceId
{
	g_message("%s server", __func__);
	MHResult _res	=	MH_OK;
	MHContents * _contents;
	gint64 _deviceId = 0;
	char * serial = NULL;
	_contents	=	mh_contents_instance();
	serial = g_strdup(serialNum);
	_deviceId	=	mh_contents_get_device( _contents, serial);
	if (_deviceId != -1)
	{
		mh_contents_delete_device( _contents, _deviceId );
		g_message("%s _deviceId = %d  delete ok", __func__, _deviceId);
		
	}
	else
	{
		g_message("%s serialNum = %s not find", __func__, serialNum);
		_res = MH_INVALID_PARAM;
	}
	if (NULL != serial)
		free(serial);
	return _res;
}		/* -----  end of function mh_misc_db_delete_by_serial_number  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  filter_foreach
 *  Description:  
 * =====================================================================================
 */
static void filter_foreach( gpointer key, gpointer value, gpointer user_data)
{
	MHFilterData * _filter;
	_filter	=	(MHFilterData *)user_data;
	_filter->data[_filter->size].filter	=	g_strdup( (char *)key);
	_filter->data[_filter->size].type	=	*(MHItemType *)value;
	_filter->size ++;
}		/* -----  end of static function filter_foreach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_filter
 *  Description:  
 * =====================================================================================
 */
MHFilterData * mh_misc_get_filter()
{
	guint _size;
	MHFilterData * _filter;
	int i;

	_size	=	g_hash_table_size( typeHash );
	_filter	=	(MHFilterData *)g_malloc0(sizeof( MHFilterData) );
	_filter->data	=	( MHFilterType *)g_malloc0(sizeof( MHFilterType)*_size);
	_filter->size = 0;
	g_hash_table_foreach( typeHash, filter_foreach, _filter);
	return _filter;
}		/* -----  end of function mh_misc_get_filter  ----- */
bool is_magiced_by_c2c3(const char str[], uint32_t len)
{
	uint32_t i;
	if(len >= 2)
	{
		for(i = 0; i < len - 2; i++) 
		{
	
			if((uint8_t)str[i] < 0x7F) 
			{
			}
			else if((((uint8_t)str[i] == 0xC2U) || ((uint8_t)str[i] == 0xC3U))
					&& (((uint8_t)str[i + 2] == 0xC2U)|| ((uint8_t)str[i + 2] == 0xC3U)) )
			{
					return true;
			}
		}
	}
	return false;
}

static char * fix_c2c3_string( char * string )
{
	char * _revert	=	string;
	if(is_magiced_by_c2c3(string,strlen(string))== true)
	{
		_revert	=	g_convert( string, strlen( string ),  "iso-8859-1", "utf-8", NULL, NULL, NULL );

		g_free( string );

		if(g_utf8_validate(_revert, -1, NULL))
		{
		}
		else
		{
			iconv_t cd = iconv_open("utf-8", "gb18030");
			 if((iconv_t)-1 == cd)
			 {
				 printf("** Message: [%s : %d]-----iconv_open failed!!!!!\n","gstmisc.c", __LINE__);
			 }
			 else
			 {
				 printf("** Message: [%s : %d]-----iconv_open success!!!!!\n","gstmisc.c", __LINE__);
				 int _inbytes_left = strlen(_revert);
				 int _outbytes_left;
				 if(_inbytes_left < 128)
				 {
					 _outbytes_left = _inbytes_left * 2;
				 }
				 else
				 {
					 _outbytes_left = 256;
				 }
				 printf("** Message: [%s : %d]-----make iconv args!!!!![_inbytes_left=%d, _outbytes_left=%d]\n","gstmisc.c", __LINE__, _inbytes_left, _outbytes_left);
				 gchar *_inbuf = _revert;
				 gchar *_outstr = g_malloc0(_outbytes_left);
				 gchar *_outbuf = _outstr;
				 int ret = iconv(cd, &_inbuf, &_inbytes_left, &_outbuf, &_outbytes_left);
				 if(-1 == ret)
				 {
					 printf("** Message: [%s : %d]-----iconv() failed!!!!![%s]\n","gstmisc.c", __LINE__, strerror(errno));
					 g_free(_revert);
					 g_free(_outstr);
					 _revert = NULL;
					 _outstr = NULL;
				 }
				 else
				 {
					 printf("** Message: [%s : %d]-----iconv() success!!!!![%s]\n","gstmisc.c", __LINE__, _outstr);
					 g_free(_revert);
					 _revert = _outstr;
					 iconv_close(cd);
				 }
			 }
		}
	}
	return _revert;



}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_file_get_tag
 *  Description:  
 * =====================================================================================
 */
MHTagInfo * mh_file_get_tag(char * file_path)
{
	TagLib_File * _file;
	TagLib_Tag	* _tag;
	const TagLib_AudioProperties	* _properties;
	MHTagInfo	* _res	=	g_new0( MHTagInfo, 1);

	_file	=	taglib_file_new( file_path);
	if( _file != 	NULL)
	{
		_tag	=	taglib_file_tag( _file);
		_properties	=	taglib_file_audioproperties( _file );
		if(_tag	!=	NULL)
		{
			_res->title	=	g_strdup(taglib_tag_title( _tag));
			_res->album	=	g_strdup( taglib_tag_album( _tag ));
			_res->artist=	g_strdup( taglib_tag_artist( _tag));
			_res->genre   =   g_strdup( taglib_tag_genre( _tag ));
			
			_res->title	=	fix_c2c3_string( _res->title );
			_res->album	=	fix_c2c3_string( _res->album );
			_res->artist	=	fix_c2c3_string( _res->artist );
			_res->genre   =   fix_c2c3_string( _res->genre );
			
			_res->year    =   taglib_tag_year( _tag );
			_res->track   =   taglib_tag_track( _tag );

//			g_message("\n\n\nmh_file_get_tag:title:%s, album:%s, artist:%s", _res->title,
//					_res->album, _res->artist);
		}
		if( _properties != NULL)
		{
			_res->duration	=	taglib_audioproperties_length( _properties );
//			g_message("duration:%d\n\n\n", _res->duration);
		}
		
		taglib_tag_free_strings();
		taglib_file_free( _file );

	}
	return _res;
}		/* -----  end of function mh_file_get_tag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_iap_device_mode( MiscIapDeviceMode mode )
{
	g_message("%s Device mode = [%d]\n",__func__, mode );

	MHResult _res	=	MH_OK;

	if( mode != iapMode )
	{
		iapMode	=	mode;
		localIapMode = mode;
	}

	return _res;
}		/* -----  end of function mh_misc_set_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
MiscIapDeviceMode mh_misc_get_iap_device_mode()
{
	return iapMode;
}		/* -----  end of function mh_misc_get_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_local_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_local_iap_device_mode( MiscIapDeviceMode mode )
{
	g_message("%s Local Device mode = [%d]\n",__func__, mode );

	MHResult _res	=	MH_OK;

	if( mode != localIapMode )
		localIapMode	=	mode;

	return _res;
}		/* -----  end of function mh_misc_set_local_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_local_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
MiscIapDeviceMode mh_misc_get_local_iap_device_mode()
{
	return localIapMode;
}		/* -----  end of function mh_misc_get_local_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_bluetoothids()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_bluetoothids( const char * ids )
{
	MHResult _res	=	MH_OK;

	if( bluetoothids == NULL )	
	{
		bluetoothids =  g_strdup( ids );	
		g_message("%s bluetoothids = %s\n", __func__, bluetoothids );
	}else{
		g_message("bluetoothids is NULL!\n");
	}

	return _res;
}		/* -----  end of function mh_misc_set_bluetoothids  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_bluetoothids()
 *  Description:  
 * =====================================================================================
 */
char *  mh_misc_get_bluetoothids()
{
	return bluetoothids;
}		/* -----  end of function mh_misc_get_bluetoothids  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_save_pb()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_save_pb( MHPb * pb )
{
	g_message("%s pb = [%p]\n",__func__, pb );

	MHResult _res	=	MH_OK;

	if( pb != NULL )	
		pb_handle	=	pb;	

	return _res;
}		/* -----  end of function mh_misc_save_pb  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_pb_handle()
 *  Description:  
 * =====================================================================================
 */
MHPb * mh_misc_get_pb_handle()
{
	return pb_handle;
}		/* -----  end of function mh_misc_get_pb_handle  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_carplay_init_modes()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_carplay_init_modes( const uint8_t * modes )
{
	g_message("%s\n",__func__);

	MHResult _res	=	MH_OK;
		
	char _buf[20];

	sprintf( _buf, "init:%d%d%d%d%d%d%d%d%d%d%d", modes[1],
			modes[2], modes[3], modes[4], modes[5], modes[6],
			modes[7], modes[8], modes[9], modes[10], modes[11]);

	g_message("%s modes = [%s]\n", __func__, _buf);

	init_modes	=	g_malloc0( modes[0]);
	memcpy( init_modes, modes, modes[0]);

	return _res;
}		/* -----  end of function mh_misc_carplay_init_modes  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_read_carplay_init_modes()
 *  Description:  
 * =====================================================================================
 */
void mh_misc_read_carplay_init_modes( uint8_t ** data )
{
	if( init_modes == NULL )
	{
		g_message("app don't set init modes\n");

		init_modes	=	g_malloc0( 12 );
		
		init_modes[0]	=	12;
		init_modes[1] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
		init_modes[2] 	=   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
		init_modes[3] 	=  	MH_CARPLAY_CONSTRAINT_USER_INIT  & 0xFF;	
		init_modes[4] 	=   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
		init_modes[5] 	=   MH_CARPLAY_TRANSFERTYPE_TAKE & 0xFF;	
		init_modes[6] 	=   MH_CARPLAY_TRANSFERPRIORITY_USER_INIT & 0xFF;	
		init_modes[7] 	=   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
		init_modes[8] 	=   MH_CARPLAY_CONSTRAINT_USER_INIT & 0xFF;	
		init_modes[9] 	=   MH_CARPLAY_APPMODE_SPEECH_NOAPP & 0xFF;	
		init_modes[10] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
		init_modes[11] 	=   MH_CARPLAY_APPMODE_FALSE & 0xFF;	
	}

	*data	=	init_modes;
}		/* -----  end of function mh_misc_read_carplay_init_modes  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_righthand()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_righthand( uint32_t status )
{
	MHResult _res	=	MH_OK;

	g_message(" %s status = %d\n", __func__, status );
	rightHand	=	status;

	return _res;
}		/* -----  end of function mh_misc_set_righthand  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_get_righthand()
 *  Description:  
 * =====================================================================================
 */
uint32_t mh_misc_get_righthand()
{
	return rightHand;
}		/* -----  end of function mh_misc_get_righthand  ----- */


