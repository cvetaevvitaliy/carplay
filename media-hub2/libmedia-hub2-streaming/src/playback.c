/*
 * =====================================================================================
 *
 *       Filename:  playback.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/15/2015 10:20:20 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_streaming_pb.h>
#include <mh_client.h>
#include <gst/gst.h>
#include <glib.h>
#include <string.h>
#include <mh_dev.h>

typedef struct _MHStreamingPbPrivate MHStreamingPbPrivate;

#define SHUFFLE_NUM 2

struct _MHStreamingPbPrivate
{
	gint dummy;

	GstElement * pipeline;
	GstBus * bus;
	gint watchid;
	GSource * time_source;

	int count;
	gint64 duration;

	MHPbRepeatMode repeat;
	MHPbShuffleMode shuffle;

	uint32_t shuffle_start_index;
	gchar * path;
	TagInfo * taginfo;

	uint32_t window_layer;	
	gboolean force_aspect_ratio;
	uint32_t disp_x;
	uint32_t disp_y;
	uint32_t disp_width;
	uint32_t disp_height;

	MHDev * mhDev;
};

enum {
/* Ios */

	/* Signals */
	STREAMING_PB_EVENTS,
	STREAMING_STATUS_UPDATE,

	N_SIGNALS
};

static guint signals[ N_SIGNALS ] = {0};

enum
{
	PROP_0,

	PROP_STREAMING_PB_VIDEO_SINK,
	PROP_STREAMING_PB_AUDIO_SINK,
	PROP_STREAMING_PB_INDEX,
	PROP_STREAMING_PB_REPEAT,
	PROP_STREAMING_PB_SHUFFLE,
	PROP_STREAMING_PB_WINDOW_LAYER,
	PROP_STREAMING_PB_FORCE_ASPECT_RATIO,
	PROP_STREAMING_PB_WINDOW_X,
	PROP_STREAMING_PB_WINDOW_Y,
	PROP_STREAMING_PB_WINDOW_WIDTH,
	PROP_STREAMING_PB_WINDOW_HEIGHT,
	PROP_STREAMING_PB_AUDIO_TRACK,
	PROP_STREAMING_PB_SUBTITLE,
	PROP_STREAMING_PB_SHARED,

	N_PROPERTIES
};

typedef enum {
	GST_PLAY_FLAG_VIDEO         = (1 << 0),
	GST_PLAY_FLAG_AUDIO         = (1 << 1),
	GST_PLAY_FLAG_TEXT          = (1 << 2),
	GST_PLAY_FLAG_VIS           = (1 << 3),
	GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
	GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
	GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
	GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
	GST_PLAY_FLAG_BUFFERING     = (1 << 8),
	GST_PLAY_FLAG_DEINTERLACE   = (1 << 9)
} GstPlayFlags;

typedef struct _MHStreamingPbParam 
{
	MHStreamingPb * self;
	uint32_t index;	
} MHStreamingPbParam;	

static GParamSpec * pbProperties[ N_PROPERTIES ]	=	{ NULL, };
G_DEFINE_TYPE_WITH_PRIVATE( MHStreamingPb, mh_streaming_pb, MH_TYPE_IO )

static gboolean vpuflag = false;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_path
 *  Description:  
 * =====================================================================================
 */
static bool get_path( void * data, void * user_data )
{
	MHItemData * _item	=	( MHItemData * )data;
	PathInfo ** _pathInfo	=	( PathInfo ** )user_data;

	g_message( "%s %s", __func__, _item->uri );

	(*_pathInfo)->path	=	g_strdup( _item->uri );
	(*_pathInfo)->name	=	g_strdup( _item->name );
	(*_pathInfo)->type	=	_item->type;

	return false;
}		/* -----  end of static function get_path  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _state_change
 *  Description:  
 * =====================================================================================
 */
//static gboolean _state_change(GstState sRecState, GstElement * elem )
//{
//	GstStateChangeReturn _result = GST_STATE_CHANGE_FAILURE;
//	GstState _current;
//	gchar * _ele_name = gst_element_get_name( elem );
//	_result = gst_element_set_state( elem, sRecState );
//	if(_result == GST_STATE_CHANGE_FAILURE)
//	{
//		if( NULL != _ele_name )
//		{
//			g_free( _ele_name );
//		}
//		return FALSE;
//	}
//	int _count = 0;
//	while( TRUE )
//	{
//		if (gst_element_get_state( elem, &_current, NULL, GST_SECOND)!=GST_STATE_CHANGE_FAILURE)
//		{
//			if( sRecState == _current || _current == GST_STATE_NULL )
//			{
//				break;
//			}
//			if( _count == 3)
//			{
//				if( NULL != _ele_name )
//				{
//					g_free( _ele_name );
//				}
//				return FALSE;
//			}
//			usleep( 1000000 );
//
//			_count	++;
//		}else{
//			return FALSE;
//		}
//	}
//	if( NULL != _ele_name )
//	{
//		g_free( _ele_name );
//	}
//	return TRUE;
//}      /*  -----  end of static function _state_change  ----- */ 


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_media_uri
 *  Description:  
 * =====================================================================================
 */
static void _set_media_uri( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gchar * _uri;

	PathInfo * _pathInfo	=	g_slice_new( PathInfo );

	mh_playlist_foreach( pb->playlist, pb->seq[ pb->index ], 1, get_path, &_pathInfo );

	MHPbInfoData * _info = g_slice_new( MHPbInfoData );

	_info->track_info.index = pb->seq[ pb->index ];	
	_info->track_info.uri = g_strdup( _pathInfo->path );
	_info->track_info.name =  g_strdup( _pathInfo->name );

	g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_TRACK_TOP, _info );

	_priv->duration	=	-1;

	if( _pathInfo->type	==	MH_ITEM_MUSIC )
	{
		MHTagInfo * _taginfo =	mh_file_get_tag( _pathInfo->path );

		if( _taginfo	!= NULL )
		{
//			MHPbInfoData *_info = g_slice_new( MHPbInfoData );
//			_info->tag_info.title	=	g_strdup( _taginfo->title ? _taginfo->title: "");
//			_info->tag_info.artist	=	g_strdup( _taginfo->artist ? _taginfo->artist: "");
//			_info->tag_info.album	=	g_strdup( _taginfo->album ? _taginfo->album: "");
//
//			g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_TAG, _info );
//
//			g_free( _info->tag_info.title );
//			g_free( _info->tag_info.artist );
//			g_free( _info->tag_info.album );
//
//			g_slice_free( MHPbInfoData, _info);

			g_message("title = [%s]",_taginfo->title);
			g_message("album = [%s]",_taginfo->album);
			g_message("artist = [%s]",_taginfo->artist);
			g_message("duration = [%d]",_taginfo->duration);

//			MHPbInfoData *_info = g_slice_new( MHPbInfoData );
//			_info->tag_info.title	=	g_strdup( _taginfo->title ? _taginfo->title: "");
//			_info->tag_info.artist	=	g_strdup( _taginfo->artist ? _taginfo->artist: "");
//			_info->tag_info.album	=	g_strdup( _taginfo->album ? _taginfo->album: "");
//
//			g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_TAG, _info );
//
//			g_free( _info->tag_info.title );
//			g_free( _info->tag_info.artist );
//			g_free( _info->tag_info.album );
//
//			g_slice_free( MHPbInfoData, _info);

			if( _taginfo->title != NULL ) 
				g_free( _taginfo->title );
			if( _taginfo->album	!= NULL )
				g_free( _taginfo->album );
			if( _taginfo->artist	!= NULL )
				g_free( _taginfo->artist );
			if( _taginfo->genre	!= NULL )
				g_free( _taginfo->genre );
			if( _taginfo->duration	!= 0 )
				_priv->duration	=	(gint64)(_taginfo->duration * 1000 * GST_MSECOND);

			g_free(  _taginfo);
		}else{
			g_message("Get tag info failed!");
		}
	}
	if( _priv->path	!= NULL )
	{
		g_free( _priv->path );
		_priv->path	=	NULL;
	}
	
	_priv->path	=	g_filename_to_uri( _pathInfo->path, NULL, NULL );

	mh_streaming_add_media_mapping( _pathInfo->path, pb->shared );
//	_uri	=	g_strconcat( "rtsp://10.1.205.161:8554", _path, NULL );
	_uri	=	g_strdup_printf("rtsp://[%s%%%d]:8554%s", ((IPCConnect *)pb->connect)->connect_name, ((IPCConnect *)pb->connect)->connect_scope_id, _priv->path + 7 );

	g_object_set( _priv->pipeline, "uri", _uri, NULL );

	g_free( _uri );
	g_free( _pathInfo->path );
	g_free( _pathInfo->name );
	g_free( _info->track_info.uri );
	g_free( _info->track_info.name );
	g_slice_free( PathInfo, _pathInfo);
	g_slice_free( MHPbInfoData, _info);

}		/* -----  end of function _set_media_uri  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _stop
 *  Description:  
 * =====================================================================================
 */
static void _stop( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	uint32_t _index;

	PathInfo * _pathInfo	=	g_slice_new( PathInfo );

	gst_element_set_state( _priv->pipeline, GST_STATE_READY );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_source_set_ready_time( _priv->time_source, -1 );

	mh_object_get_properties( ( MHObject * )pb->playlist, "index", &_index, NULL );

	mh_playlist_foreach( pb->playlist, _index, 1, get_path, &_pathInfo );

	/* Unknown warnings */
	//	mh_streaming_remove_media_mapping( _pathInfo->path );
	vpuflag = false;

	g_free( _pathInfo->path );
	g_free( _pathInfo->name );
	g_slice_free( PathInfo, _pathInfo);
}		/* -----  end of function _stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_register_events_listener
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_register_events_listener( MHStreamingPb * self, MHPbEventsListener * listener )
{
	g_message( "%s",__func__ );

	g_signal_connect( self, "streaming_pb_events", G_CALLBACK( listener->callback ), listener->user_data );
}		/* -----  end of function mh_streaming_pb_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_register_events_listener
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_register_status_listener( MHStreamingPb * self, MHPbStatusListener * listener )
{
	g_message( "%s",__func__ );

	g_signal_connect( self, "streaming_status_update", G_CALLBACK( listener->callback ), listener->user_data );
}		/* -----  end of function mh_streaming_pb_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _send_repeat_signal
 *  Description:  
 * =====================================================================================
 */
void _send_repeat_signal( MHStreamingPb * pb, MHPbRepeatMode repeat )
{
	MHPbInfoData * _info = g_slice_new( MHPbInfoData );

	_info->repeat_mode	=	repeat;

	g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_IP_INFO_REPEAT_MODE, _info );
	
	g_slice_free( MHPbInfoData, _info);
}		/* -----  end of function _send_repeat_signal  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _send_shuffle_signal
 *  Description:  
 * =====================================================================================
 */
void _send_shuffle_signal( MHStreamingPb * pb, MHPbShuffleMode shuffle )
{
	MHPbInfoData * _info = g_slice_new( MHPbInfoData );

	_info->shuffle_mode	=	shuffle;

	g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_IP_INFO_SHUFFLE_MODE, _info );
	
	g_slice_free( MHPbInfoData, _info);
}		/* -----  end of function _send_shuffle_signal  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ptimeDispatch
 *  Description:  
 * =====================================================================================
 */
static gboolean ptimeDispatch( GSource * source, GSourceFunc callback, gpointer user_data )
{
	GstFormat _fmt = GST_FORMAT_TIME;
	gint64 _pos = -1;
	MHStreamingPb * pb	=	MH_STREAMING_PB( user_data );
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	if( gst_element_query_position( _priv->pipeline, _fmt, &_pos )) 
	{
		if( _pos != -1 && _fmt == GST_FORMAT_TIME ) 
		{	
			MHPbInfoData *_info = g_slice_new( MHPbInfoData );

			_info->time_info.current_time	 = _pos / GST_MSECOND;

			g_source_set_ready_time( _priv->time_source, g_source_get_time( _priv->time_source ) + 
					(( _info->time_info.current_time / 1000 + 1 ) * 1000 + 10 - _pos / GST_MSECOND ) * 1000 );

			gint64 _duration	=	-1;

			if( gst_element_query_duration( GST_ELEMENT( _priv->pipeline ),_fmt, &_duration ))
			{
				if( _duration != -1 )
					_priv->duration = _duration ;
			}
			_info->time_info.duration = _priv->duration / GST_MSECOND;

			g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_PTIME_CHANGE, _info );
		}
	}
	else {
		g_message("could not get position\n");

		g_source_set_ready_time( _priv->time_source, g_source_get_time( _priv->time_source ) + 
				500 * 1000 );
	}

	return G_SOURCE_CONTINUE;
}		/*  -----  end of static function ptimeDispatch  ----- */

static GSourceFuncs ptimeFuncs  =
{
		.dispatch   =   ptimeDispatch
};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_ptime_source
 *  Description:  
 * =====================================================================================
 */
static void _set_ptime_source( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	_priv->time_source	=	g_source_new( &ptimeFuncs, sizeof( GSource ));

	g_source_set_callback( _priv->time_source, NULL, pb, NULL );

	g_source_set_ready_time( _priv->time_source, -1 );

	mh_io_dispatch( MH_IO( pb ), _priv->time_source );

//	g_source_unref( _priv->time_source );
}      /*  -----  end of static function _set_ptime_source  ----- */ 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ExistOrNot
 *  Description:
 * =====================================================================================
 */
static gboolean ExistOrNot(uint32_t * seq, uint32_t start_index, uint32_t num )
{
	uint32_t _i;

	for( _i = start_index; _i < SHUFFLE_NUM; _i++ )
	{
		if( seq[_i] == num )
		{
			return TRUE;
		}
	}

	return FALSE; 
}      /*  -----  end of static function ExistOrNot  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _recover_seq 
 *  Description:
 * =====================================================================================
 */
static void _recover_seq( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	uint32_t _i;

	for( _i = 0; _i < ( _priv->count ); _i++ )
	{
		pb->seq[_i] = _i; 
	}
}      /*  -----  end of static function _recover_seq  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _printf_seq 
 *  Description:
 * =====================================================================================
 */
static void _printf_seq( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	uint32_t i = 0;

	for( i = 0; i < _priv->count; i++ )
	{
		g_message("seq[%d] = %d\n", i, pb->seq[i]);
	}
}      /*  -----  end of static function _printf_seq  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _avoid_repeat_file
 *  Description:
 * =====================================================================================
 */
static void _avoid_repeat_file( MHStreamingPb * pb, uint32_t * old_seq )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	int _i, _rand_num, _tmp;

	for( _i = 0; _i < SHUFFLE_NUM; _i ++ )       //judge last three file whether repeat                                              
	{                                                                                 
		if( ExistOrNot( old_seq, _i, pb->seq[_i]) == TRUE )
		{                                                                             
			while( TRUE )
			{
				_rand_num = g_random_int_range( _i, _priv->count);
				if( ExistOrNot( old_seq, _i, pb->seq[_rand_num]) == FALSE )break;		
			}
			_tmp = pb->seq[_i];
			pb->seq[_i] = pb->seq[_rand_num];
			pb->seq[_rand_num] = _tmp;
		}                                                                             
	}  
}      /*  -----  end of static function _avoid_repeat_file  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _create_shuffle_seq 
 *  Description:
 * =====================================================================================
 */
static void _create_shuffle_seq( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	uint32_t _i = _priv->count - 1;	
	uint32_t _tmp, _rand_num;

	for( _i; _i > 1; _i-- )
	{
		_rand_num = g_random_int_range( 0,_i+1 );

		_tmp = pb->seq[_i]; 
		pb->seq[_i] = pb->seq[_rand_num]; 
		pb->seq[_rand_num] = _tmp; 
	}
}      /*  -----  end of static function _create_shuffle_seq  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _set_playlist_seq 
 *  Description:
 * =====================================================================================
 */
static void _set_playlist_seq( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	GVariant * _seq;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));

	int i;
	for( i = 0; i < _priv->count; i++)
	{
		g_variant_builder_add( _builder, "u", pb->seq[i] );
	}
	_seq	=	g_variant_builder_end( _builder);

	GVariant * _var = g_variant_new("v",_seq );
	mh_object_set_properties((MHObject *)pb->playlist, "seq",_var,NULL );

	g_variant_unref( _seq );
	g_variant_unref( _var );
	g_variant_builder_unref( _builder );
}      /*  -----  end of static function _set_playlist_seq  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _get_playlist_seq 
 *  Description:
 * =====================================================================================
 */
static void _get_playlist_seq( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	pb->seq	=	g_renew( guint, pb->seq, _priv->count );

	int i = 0;
	GVariant * _var;
	mh_object_get_properties((MHObject *)pb->playlist, "seq",&_var,NULL );
	GVariantIter * _it;
	guint _val;

	g_variant_get(_var, "au", &_it);
	while( g_variant_iter_loop( _it, "u", &_val ))
	{
		pb->seq[i]	=	_val;
		g_message("seq:%d", _val);
		i ++;
	}

	g_variant_unref( _var );
	g_variant_iter_free( _it );

}      /*  -----  end of static function _get_playlist_seq  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_index
 *  Description:  
 * =====================================================================================
 */
//static void _set_index( MHStreamingPb * pb, uint32_t index )
static gboolean _set_index( gpointer user_data )
{
	MHStreamingPbParam * _pbParam = ( MHStreamingPbParam * )user_data;
	MHStreamingPb * pb	=	_pbParam->self;
	uint32_t index =	_pbParam->index; 
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( _pbParam->self );
	
	if( index <= _priv->count )
	{
		g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_SWITCHING );

		_stop( pb );

		if( _priv->shuffle == MH_PB_SHUFFLE_ALL )
		{
			uint32_t _i;

			for( _i = 0; _i < ( _priv->count ); _i++ )
			{
				if( pb->seq[_i] == index )
				{
					pb->index = _i;
					break;
				}
			}
		}else{
			pb->index	=	index;
		}

		_set_media_uri( pb );

		gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
		gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

		g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

		g_source_set_ready_time( _priv->time_source, 0 );

		mh_object_set_properties( ( MHObject * )pb->playlist, "index", index, NULL );

	}else{
		g_message( "index more than the playlist length !");
	}

	return G_SOURCE_REMOVE;
}		/* -----  end of function _set_index  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_repeat
 *  Description:  
 * =====================================================================================
 */
static void _set_repeat( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	if( pb->playlist	==	NULL )
	{
		g_message( "%s : playlist is NULL\n",__func__ );	
	}

	mh_object_set_properties( ( MHObject * )pb->playlist, "repeat", _priv->repeat, NULL );

	_send_repeat_signal( pb, _priv->repeat );

}		/* -----  end of function _set_repeat  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_shuffle
 *  Description:  
 * =====================================================================================
 */
static void _set_shuffle( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	if( pb->playlist	==	NULL )
	{
		g_message( "%s : playlist is NULL\n",__func__ );	
	}

	uint32_t _i, _index;

	mh_object_set_properties( ( MHObject * )pb->playlist, "shuffle", _priv->shuffle, NULL );

	mh_object_get_properties( ( MHObject * )pb->playlist, "index", &_index, NULL );

	if( _priv->shuffle == MH_PB_SHUFFLE_ALL )
	{
		_create_shuffle_seq( pb );

		for( _i = 0; _i < ( _priv->count); _i++ )
		{
			if( _index == pb->seq[_i] )
			{
				_priv->shuffle_start_index = _i;
				pb->index = _i;
				g_message( "pb->shuffle_start_index = [%d]\n",_priv->shuffle_start_index);
				break;
			}
		}

	}else{
		pb->index = _index;

		_recover_seq( pb );
	}

	mh_object_set_properties( ( MHObject * )pb->playlist, "index", pb->index, NULL );

	_printf_seq( pb );

	_set_playlist_seq( pb );

	_send_shuffle_signal( pb, _priv->shuffle );
}		/* -----  end of function _set_shuffle  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_subtitle
 *  Description:  
 * =====================================================================================
 */
static void _set_subtitle( MHStreamingPb * pb, guint subtitle )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	GstTagList *tags;

	gint _flags, _total_number, _current_number;

	g_object_get( G_OBJECT( _priv->pipeline ), "n-text", &_total_number, NULL );

	g_object_get( G_OBJECT( _priv->pipeline ), "current-text", &_current_number, NULL );	

	g_message("_total_number	=	[%d], _current_number	=	[%d]\n", _total_number, _current_number );

	if( subtitle !=	_current_number )
	{
		g_object_get( G_OBJECT( _priv->pipeline ), "flags", &_flags, NULL );

		if( !( _flags & GST_PLAY_FLAG_TEXT ))
		{
			_flags |= GST_PLAY_FLAG_TEXT;
		}

		g_object_set( G_OBJECT( _priv->pipeline ),"flags", _flags, "current-text", subtitle, NULL);

		g_object_get( G_OBJECT( _priv->pipeline ), "current-text", &subtitle, NULL);	

		g_signal_emit_by_name( G_OBJECT( _priv->pipeline ), "get-text-tags", subtitle, &tags);
	}
}      /*  -----  end of static function _set_subtitle  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_audio_track
 *  Description:  
 * =====================================================================================
 */
static void _set_audio_track( MHStreamingPb * pb, guint track )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gint _current_audio_index = -1;
	gint _total_audio_number = 0;

	GstTagList *tags;

	g_object_get( G_OBJECT( _priv->pipeline ), "n-audio", &_total_audio_number, NULL );
	g_object_get( G_OBJECT( _priv->pipeline ), "current-audio", &_current_audio_index, NULL );

	g_message(" _total_audio_number = [%d] _current_audio_index = [%d]\n",_total_audio_number,_current_audio_index);

	if( track != _current_audio_index )
	{
		gint _flags;

		g_object_get( G_OBJECT( _priv->pipeline ), "flags", &_flags, NULL );

		if( !( _flags & GST_PLAY_FLAG_AUDIO ))
		{
			_flags |= GST_PLAY_FLAG_AUDIO;
		}

		g_object_set( G_OBJECT( _priv->pipeline ),"flags", _flags, "current-audio", track, NULL);

		g_object_get( G_OBJECT( _priv->pipeline ), "current-audio", &_current_audio_index, NULL );

		g_signal_emit_by_name( G_OBJECT( _priv->pipeline), "get-audio-tags", track, &tags);

		g_message( "_current_audio_index	=	[%d]\n",_current_audio_index);

//		gint64 position;  
//
//		GstFormat format = GST_FORMAT_TIME;  
//
//		if (gst_element_query_position ( self->playbin2, &format, &position)) 
//		{
//			g_message("position	=	[%lld]\n",position);	
//
//			GstSeekFlags _seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE;
//
//			GstEvent *_seek_event = NULL;
//
//			_seek_event = gst_event_new_seek( 1.0, GST_FORMAT_TIME,
//					_seek_flags,
//					GST_SEEK_TYPE_SET,position,
//					GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );
//
//			if( gst_element_send_event( self->playbin2, _seek_event ) == FALSE)
//			{
//				g_message( " Seek Failer: send seek event failed!\n" );
//			}
//		}
	}
}      /*  -----  end of static function _set_audio_track  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _repeat_one_play
 *  Description:  
 * =====================================================================================
 */
static gboolean _repeat_one_play(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_SWITCHING );

	gst_element_set_state( _priv->pipeline, GST_STATE_READY );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_source_set_ready_time( _priv->time_source, -1 );

	gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

	g_source_set_ready_time( _priv->time_source, 0 );

	return G_SOURCE_REMOVE;
}      /*  -----  end of static function _repeat_one_play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _previous
 *  Description:  
 * =====================================================================================
 */
static gboolean _previous(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_SWITCHING );

	_stop( pb );

	if( pb->index == 0 )
		pb->index	=	_priv->count;

	pb->index	--;

	if(( _priv->shuffle == MH_PB_SHUFFLE_ALL ) && ( _priv->count > 3))
	{
		uint32_t _reset_index	=	0;

		if( _priv->shuffle_start_index == 0)
		{
			_reset_index	=	_priv->count - 1; 
		}else{
			_reset_index	=	_priv->shuffle_start_index - 1;
		}

		if( pb->index == _reset_index )
		{
			uint32_t * old_seq = g_new0( uint32_t, SHUFFLE_NUM );                                               
			int32_t _i;    

			for( _i = 1; _i >= 0; _i-- )           //save last three seq id                                                
			{                                                                                      
				old_seq[_i] = pb->seq[_priv->shuffle_start_index];                        
				g_message(" old_seq[ %d ] = %d\n ",_i, old_seq[_i] );

				if( _priv->shuffle_start_index < _priv->count - 1 )                                                
				{                                                                                  
					_priv->shuffle_start_index ++;                    
				}else{                                                                             
					_priv->shuffle_start_index = 0;	
				}                                                                                  
			}     

			_recover_seq( pb );

			pb->index = 0;
			_priv->shuffle_start_index = 0;

			_create_shuffle_seq( pb );

			_avoid_repeat_file( pb, old_seq );

			_set_playlist_seq( pb );

			_printf_seq( pb );

			g_free( old_seq );            
		}
	}

	_set_media_uri( pb );

	gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

	g_source_set_ready_time( _priv->time_source, 0 );

	mh_object_set_properties( ( MHObject * )pb->playlist, "index", pb->seq[ pb->index ], NULL );

	return G_SOURCE_REMOVE;
}      /*  -----  end of static function _repeat_one_play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _next
 *  Description:  
 * =====================================================================================
 */
//static void _next( MHStreamingPb * pb )
static gboolean _next(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_SWITCHING );

	_stop( pb );

	pb->index	++;

	if( pb->index == _priv->count )
		pb->index	=	0;

	if(( _priv->shuffle == MH_PB_SHUFFLE_ALL ) && ( _priv->count > 3 ))
	{
		if( pb->index == _priv->shuffle_start_index )
		{
			uint32_t * old_seq = g_new0( uint32_t, SHUFFLE_NUM );                                               
			int32_t _i;    

			for( _i = 1; _i >= 0; _i-- )           //save last three seq id                                                
			{                                                                                      
				if( _priv->shuffle_start_index == 0 )                                                
				{                                                                                  
					_priv->shuffle_start_index =  _priv->count - 1;                     
				}else{                                                                             
					_priv->shuffle_start_index--;                                                    
				}                                                                                  
				old_seq[_i] = pb->seq[_priv->shuffle_start_index];                        
				g_message("old_seq[%d]	=	%d\n", _i, old_seq[_i]);
			}                                                                                      

			_recover_seq( pb );

			pb->index = 0;
			_priv->shuffle_start_index = 0;

			_create_shuffle_seq( pb );

			_avoid_repeat_file( pb, old_seq );

			_set_playlist_seq( pb );

			_printf_seq( pb );

			g_free( old_seq );                                                                
		}
	}

	_set_media_uri( pb );

	gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

	g_source_set_ready_time( _priv->time_source, 0 );

	mh_object_set_properties( ( MHObject * )pb->playlist, "index", pb->seq[ pb->index ], NULL );

	return G_SOURCE_REMOVE;
}		/* -----  end of function _next  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _pb_stop
 *  Description:  
 * =====================================================================================
 */
static gboolean _pb_stop(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	g_source_set_ready_time( _priv->time_source, -1 );

	gst_element_set_state( _priv->pipeline, GST_STATE_READY );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_READY );

	return G_SOURCE_REMOVE;
}		/* -----  end of function _pb_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _play
 *  Description:  
 * =====================================================================================
 */
static gboolean _play(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

	g_source_set_ready_time( _priv->time_source, 0 );

	return G_SOURCE_REMOVE;
}		/* -----  end of function _play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _pause
 *  Description:  
 * =====================================================================================
 */
static gboolean _pause(  gpointer user_data )
{
	MHStreamingPb * pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gst_element_set_state( _priv->pipeline, GST_STATE_PAUSED );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PAUSE );

	g_source_set_ready_time( _priv->time_source, -1 );

	return G_SOURCE_REMOVE;
}		/* -----  end of function _pause  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  is_isink
 *  Description:
 * =====================================================================================
 */
static gboolean is_isink( GstElement * video_sink )
{
	gchar * _ele_name = NULL;
	gboolean _result = FALSE;

	_ele_name = gst_element_get_name( video_sink );

	if( g_strrstr( _ele_name, "isink" )) 
	{
		_result = TRUE;
	}

	if( NULL != _ele_name ) 
	{
		g_free( _ele_name );
	}

	return _result;
}      /*  -----  end of static function is_isink  ----- */ 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  is_glimagesink
 *  Description:
 * =====================================================================================
 */
static gboolean is_glimagesink( GstElement * video_sink )
{
	gchar * _ele_name = NULL;
	gboolean _result = FALSE;

	_ele_name = gst_element_get_name( video_sink );

	if( g_strrstr( _ele_name, "glimagesink" )) 
	{
		_result = TRUE;
	}

	if( NULL != _ele_name ) 
	{
		g_free( _ele_name );
	}

	return _result;
}      /*  -----  end of static function is_glimagesink  ----- */ 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  update_glimagesink_size
 *  Description:
 * =====================================================================================
 */
static gboolean update_glimagesink_size( MHStreamingPb * pb, GstElement * video_sink, gint offsetx, gint offsety, gint width, gint height )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gboolean _result = TRUE;
	uint32_t _offsetx = 0;
	uint32_t _offsety = 0;
	uint32_t _disp_width = 0;
	uint32_t _disp_height = 0;

	gchar * _ele_name = gst_element_get_name( video_sink );

	/*   glimagesink window_x=0 window_y=0 window_width=768 window_height=424 window_layer=1 force-aspect-ratio=true */
	g_object_get( G_OBJECT( video_sink ), "window_x", &_offsetx, NULL );
	g_object_get( G_OBJECT( video_sink ), "window_y", &_offsety, NULL );
	g_object_get( G_OBJECT( video_sink ), "window_width", &_disp_width, NULL );
	g_object_get( G_OBJECT( video_sink ), "window_height", &_disp_height, NULL );

	if(( _priv->disp_x != offsetx ) || ( _priv->disp_y != offsety ) 
			|| ( _priv->disp_width != width ) || ( _priv->disp_height != height )) 
	{
		_priv->disp_x = offsetx;
		_priv->disp_y = offsety;
		_priv->disp_width = width;
		_priv->disp_height = height;

		g_object_set(G_OBJECT( video_sink ),
				"window_x", _priv->disp_x,
				"window_y", _priv->disp_y,
				"window_width", _priv->disp_width,
				"window_height", _priv->disp_height,
				NULL );
	}

	if( NULL != _ele_name ) 
	{
		g_free( _ele_name );
	}

	return _result ;
}      /*  -----  end of static function update_glimagesink_size  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  update_mfw_isink_size
 *  Description:
 * =====================================================================================
 */
static gboolean update_mfw_isink_size( MHStreamingPb * pb, GstElement * video_sink, gint offsetx, gint offsety, gint width, gint height )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gboolean _result = TRUE;
	uint32_t _offsetx = 0;
	uint32_t _offsety = 0;
	uint32_t _disp_width = 0;
	uint32_t _disp_height = 0;

	gchar * _ele_name = gst_element_get_name( video_sink );

	g_object_get( G_OBJECT( video_sink ), "axis-left", &_offsetx, NULL );
	g_object_get( G_OBJECT( video_sink ), "axis-top", &_offsety, NULL );
	g_object_get( G_OBJECT( video_sink ), "disp-width", &_disp_width, NULL );
	g_object_get( G_OBJECT( video_sink ), "disp-height", &_disp_height, NULL );

	if(( _priv->disp_x != offsetx ) || ( _priv->disp_y != offsety ) 
			|| ( _priv->disp_width != width ) || ( _priv->disp_height != height )) 
	{
		gint _cnt = 0;
		gint _paraset;

		_priv->disp_x = offsetx;
		_priv->disp_y = offsety;
		_priv->disp_width = width;
		_priv->disp_height = height;

		g_object_set( G_OBJECT( video_sink ),
				"axis-left", _priv->disp_x,
				"axis-top", _priv->disp_y,
				"disp-width", _priv->disp_width,
				"disp-height", _priv->disp_height,
				NULL );

		g_object_set( G_OBJECT( video_sink ), "disp-config", 1, NULL );
		g_object_get( G_OBJECT( video_sink ), "disp-config", &_paraset, NULL );

		while(( _paraset ) && _cnt < 100 ) 
		{
			usleep( 20000 );
			_cnt++;
			g_object_get( G_OBJECT( video_sink ), "disp-config", &_paraset, NULL );
		}

		if( _cnt >= 100 ) 
		{
			g_message( "%s(): reconfig %s failure!\n",__func__, _ele_name );
			_result = false;
		}else {
			g_message( "%s(): reconfig %s successfully!\n",__func__, _ele_name );
		}

		g_object_get( G_OBJECT( video_sink ), "axis-left", &_offsetx, NULL );
		g_object_get( G_OBJECT( video_sink ), "axis-top", &_offsety, NULL );
		g_object_get( G_OBJECT( video_sink ), "disp-width", &_disp_width, NULL );
		g_object_get( G_OBJECT( video_sink ), "disp-height", &_disp_height, NULL );
	}

	if( NULL != _ele_name ) 
	{
		g_free( _ele_name );
	}

	return _result ;
}      /*  -----  end of static function update_mfw_isink_size  ----- */ 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _is_invalid_utf8_tag_string_c2c3
 *  Description:
 * =====================================================================================
 */
static gboolean _is_invalid_utf8_tag_string_c2c3(const gchar *str)
{
	gint i;
	gint len = strlen(str);

	if(len >= 2) {
		for(i = 0; i < len - 2; i++) {

			if((guchar)str[i] < 0x7F) {
			}
			else if((((guchar)str[i] == 0xC2) || ((guchar)str[i] == 0xC3))
					&& (((guchar)str[i + 2] == 0xC2) || ((guchar)str[i + 2] == 0xC3))
				   ) {
				return TRUE;
			}
		}
	}

	return FALSE;
}		/*  -----  end of static function _is_invalid_utf8_tag_string_c2c3  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _text_invaild_UTF8_chang_loc_cn
 *  Description:
 * =====================================================================================
 */
gchar * _text_invaild_UTF8_chang_loc_cn(const gchar *string)
{
	gchar *temp_string;
	gchar *conv_string;
	gsize temp_size;

	if( _is_invalid_utf8_tag_string_c2c3( string )) 
	{
		temp_string = g_convert(string, strlen(string), "ISO-8859-1", "UTF-8", NULL, &temp_size, NULL);

		conv_string = g_convert(temp_string, temp_size, "UTF-8", "GB2312", NULL, NULL, NULL);

		if(conv_string == NULL)
		{
			conv_string = g_convert(string, strlen(string), "UTF-8", "GBK", NULL, NULL, NULL);
		}

		if( conv_string	==	NULL )
		{
			temp_string = g_convert(string, strlen(string), "GB18030", "UTF-8", NULL, &temp_size, NULL);

			conv_string = g_convert(temp_string, temp_size, "UTF-8", "GB18030", NULL, NULL, NULL);
		}

		g_free(temp_string);
	}
	else {
		conv_string = g_strdup_printf("%s", string);
	}

	return conv_string;
}		/*  -----  end of static function _text_invaild_UTF8_chang_loc_cn  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _tag_func
 *  Description:  
 * =====================================================================================
 */
static void _tag_func( const GstTagList * list, const gchar * tag, gpointer user_data )
{
	GValue _value	=	G_VALUE_INIT;
	const gchar * _string;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( MH_STREAMING_PB( user_data ) );
	gchar * _cnv_string = NULL;

	if( gst_tag_list_copy_value( &_value, list, tag ))
	{
		switch( G_VALUE_TYPE( &_value ))
		{
			case G_TYPE_STRING:
				_string	=	g_value_get_string( &_value );		
//				g_message( "tag: [ %s ] value: [ %s ]", tag, _string );
				if( _string && *_string != '\0' && g_utf8_validate(_string, -1, NULL)) 
				{
					_cnv_string = _text_invaild_UTF8_chang_loc_cn(_string);
					g_message("cnv_string [%s]\n",_cnv_string);
				}
				else {
					_cnv_string = NULL;
				}

				if( _cnv_string == NULL ) break;
				if(( strncmp(gst_tag_get_nick(tag), "title", 5) == 0 )&&( _priv->taginfo->title == NULL ))
				{
					_priv->taginfo->title = g_strdup( _cnv_string );
				}
				if(( strncmp(gst_tag_get_nick(tag), "artist", 6) == 0 )&&( _priv->taginfo->artist == NULL ))
				{
					_priv->taginfo->artist = g_strdup( _cnv_string );
				}
				if(( strcmp(gst_tag_get_nick(tag), "album") == 0 )&&( _priv->taginfo->album == NULL ))
				{	
					_priv->taginfo->album = g_strdup( _cnv_string );
				}
				break;								
			case G_TYPE_UINT:				
				g_message( "tag: [ %s ] value: [ %u ]", tag, g_value_get_uint( &_value ));
				break;
			case G_TYPE_BOOLEAN:
				g_message( "tag: [ %s ] value: [ %s ]", tag, g_value_get_boolean( &_value ) ? "TRUE" : "FALSE" );
				break;
			case G_TYPE_UINT64:
				g_message( "tag: [ %s ] value: [ %llu ]", tag, g_value_get_uint64( &_value ));
				break;
			default:
				break;
		}	
		g_value_unset( &_value );
	}
	if( _cnv_string != NULL)
		g_free( _cnv_string );
}		/*   -----  end of static function _tag_func  ----- */ 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_dispose
 *  Description:
 * =====================================================================================
 */
static void mh_streaming_pb_dispose( GObject * object )
{
	g_message("%s",__func__);

	MHStreamingPb * _self	=	MH_STREAMING_PB( object );

	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( MH_STREAMING_PB( object ));

	if( _self->video_sink_name != NULL )
		g_free( _self->video_sink_name );

	if( _self->audio_sink_name != NULL )
		g_free( _self->audio_sink_name );

	if( _self->seq != NULL )
		g_free( _self->seq );

	if( _priv->bus )
		gst_object_unref ( _priv->bus );
	
	if( _priv->watchid )
	{
		g_source_remove( _priv->watchid );
		_priv->watchid = 0;
	}

	if( _priv->pipeline	!=	NULL)
	{
		g_source_set_ready_time( _priv->time_source, -1 );
		gst_element_set_state( _priv->pipeline, GST_STATE_NULL );
		gst_object_unref ( GST_OBJECT( _priv->pipeline ));
		_priv->pipeline	=	NULL;
	}

	if( _priv->time_source != NULL )
		g_source_unref( _priv->time_source );

	if( _self->playlist != NULL ) 
	{
		mh_object_unref( (MHObject *)_self->playlist );
		_self->playlist	=	NULL;
	}

	if( _priv->mhDev != NULL ) 
	{
		mh_object_unref((MHObject *)_priv->mhDev );
		_priv->mhDev	=	NULL;
	}

	if( _priv->path	!= NULL )
	{
		g_free( _priv->path );
		_priv->path	=	NULL;
	}

	G_OBJECT_CLASS( mh_streaming_pb_parent_class )->dispose( object );

}		/*   -----  end of static function mh_streaming_pb_dispose  ----- */ 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_finalize
 *  Description:
 * =====================================================================================
 */
static void mh_streaming_pb_finalize( GObject * object )
{
	g_message("%s",__func__);

	MHStreamingPb * _self	=	MH_STREAMING_PB( object );

	G_OBJECT_CLASS( mh_streaming_pb_parent_class )->finalize( object );
}		/*   -----  end of static function mh_streaming_pb_finalize  ----- */ 

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_unref
 *  Description:
 * =====================================================================================
 */
void mh_streaming_pb_unref( MHStreamingPb * pb )
{
	g_object_unref( (GObject *) pb );
}		/*   -----  end of function mh_streaming_pb_unref  ----- */ 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _busFunc
 *  Description:  
 * =====================================================================================
 */
static gboolean _busFunc( GstBus * bus, GstMessage * message, gpointer user_data )
{
	g_return_val_if_fail( MH_IS_STREAMING_PB( user_data ),FALSE);
	GError * _error;
	gchar * _debugInfo;
	GstTagList * _tagList =	NULL;	
	GstClock * _clock;
	GstStreamStatusType _type;
	GstElement * _owner;
	GstFormat _format;
	gint64 _duration;
	MHStreamingPb * pb	=	MH_STREAMING_PB( user_data );
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

//	g_message ("Got [ %s ] message from element [ %s ]", GST_MESSAGE_TYPE_NAME( message ), gst_element_get_name( message->src ));

	if(( strncmp( gst_element_get_name( message->src ), "vpudec", 6 ) == 0 )&&(vpuflag == false))
	{
		g_object_set( message->src, "low-latency", TRUE, NULL );
		vpuflag = true;
	}

	switch( GST_MESSAGE_TYPE( message ))
	{
		case GST_MESSAGE_UNKNOWN:

			break;
		case GST_MESSAGE_EOS:
			g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_EOS, NULL );

			if ( MH_PB_REPEAT_MODE_ONE == _priv->repeat )
			{
				GSource * _source	=	g_idle_source_new();

				g_source_set_callback( _source, _repeat_one_play, pb, NULL );

				mh_io_dispatch( MH_IO( pb ), _source );

				g_source_unref( _source );

//				g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_SWITCHING );
//
//				gst_element_set_state( _priv->pipeline, GST_STATE_READY );
//				gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );
//
//				g_source_set_ready_time( _priv->time_source, -1 );
//
//				gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
//				gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );
//
//				g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );
//
//				g_source_set_ready_time( _priv->time_source, 0 );

			}else{
//				_next( pb );			
				GSource * _source	=	g_idle_source_new();

				g_source_set_callback( _source, _next, pb, NULL );

				mh_io_dispatch( MH_IO( pb ), _source );

				g_source_unref( _source );
			}
			break;
		case GST_MESSAGE_ERROR:
//			gst_message_parse_error( message, &_error, &_debugInfo );
//			g_warning ( "Error from element[ %s ]\n: %s\n", GST_OBJECT_NAME( message->src ), 
//					_debugInfo ? _debugInfo : "none" );
//			g_message("Error message[ %s ]\n",_error->message);
//
//			gst_bus_set_flushing( bus, TRUE );
//			if( strncmp( _error->message, "Resource not found.", 19 ) == 0 )
//			{
//				_set_media_info( _player, _player->pb, MH_PLAYER_ERROR_NOT_EXIST, NULL );
//			}else{
//				_set_media_info( _player, _player->pb, MH_PLAYER_ERROR, NULL );
//			}
//			g_error_free( _error );
//			g_free( _debugInfo );
			g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_ERROR );
			break;
		case GST_MESSAGE_WARNING:
			gst_message_parse_warning (message, &_error, &_debugInfo);

			g_warning ( "Warning from element[ %s ]: %s", GST_OBJECT_NAME( message->src ), 
					_debugInfo ? _debugInfo : "none" );
			g_error_free( _error );
			g_free( _debugInfo );
			break;
		case GST_MESSAGE_INFO:

			break;
		case GST_MESSAGE_TAG:
//			{
//				gchar *ele_name =	gst_element_get_name( message->src );
//				if(( strncmp( ele_name, "id3demux", 8 ) == 0 )
//						|| ( strncmp( ele_name, "aiurdemux", 9 ) == 0 )) 
//				{		
//					if( g_strcmp0(_priv->path, _priv->old_path ) != 0 ) 
//					{
//						if( _priv->old_path != NULL )
//							g_free( _priv->old_path );
//						_priv->old_path	=	g_strdup( _priv->path );
//						if( _priv->taginfo	!= NULL )
//						{
//							g_free( _priv->taginfo->title );
//							g_free( _priv->taginfo->artist );
//							g_free( _priv->taginfo->album );
//							g_slice_free( TagInfo, _priv->taginfo);
//							_priv->taginfo->title	=	NULL;
//							_priv->taginfo->artist	=	NULL;
//							_priv->taginfo->album	=	NULL;
//							_priv->taginfo	=	NULL;
//						}
//						_priv->taginfo	=	g_slice_new( TagInfo );
//					}
//					gst_message_parse_tag( message, &_tagList );
//					gst_tag_list_foreach( _tagList, _tag_func, MH_STREAMING_PB( user_data ));
//					gst_tag_list_free( _tagList );     
//					if(  _priv->taginfo->title != "\0") 
//					{
//						MHPbInfoData *_info = g_slice_new( MHPbInfoData );
//						_info->tag_info.title	=	g_strdup( _priv->taginfo->title ? _priv->taginfo->title : "");
//						_info->tag_info.artist	=	g_strdup( _priv->taginfo->artist ? _priv->taginfo->artist : "");
//						_info->tag_info.album	=	g_strdup( "");
//
//						g_signal_emit( pb, signals[ STREAMING_PB_EVENTS ], 0, MH_PB_INFO_TAG, _info );
//
//						g_free( _info->tag_info.title );
//						g_free( _info->tag_info.artist );
//						g_free( _info->tag_info.album );
//
//						g_slice_free( MHPbInfoData, _info);
//					}
//				}
//				if( ele_name != NULL)
//					g_free( ele_name );
//			}
			break;
		case GST_MESSAGE_BUFFERING:
//		{
//			gint percent = 0;
//	
//			gst_message_parse_buffering (message, &percent);
//
//			g_message ("Buffering (%3d%%)\r", percent);  
//		}
			break;
		case GST_MESSAGE_STATE_CHANGED:
			{
//				GstState _old_state, _new_state, _pending;
//				gst_message_parse_state_changed( message, &_old_state, &_new_state, &_pending );
//
//				if( _old_state == _new_state)
//					break;
//				if(( _old_state == GST_STATE_PAUSED )&&( _new_state == GST_STATE_PLAYING ))
//				{
//					if( _priv->time_source != NULL )
//						g_source_set_ready_time( _priv->time_source, 0 );
//				}
//				g_message( "%s -> %s, pending: %s", gst_element_state_get_name( _old_state ),
//						gst_element_state_get_name( _new_state ), gst_element_state_get_name( _pending ));
			}
			break;
		case GST_MESSAGE_STATE_DIRTY:

			break;
		case GST_MESSAGE_STEP_DONE:

			break;
		case GST_MESSAGE_CLOCK_PROVIDE:

			break;
		case GST_MESSAGE_CLOCK_LOST:

			break;
		case GST_MESSAGE_NEW_CLOCK:
			gst_message_parse_new_clock( message, &_clock );

			g_message( "%llu", gst_clock_get_time( _clock ));
			break;
		case GST_MESSAGE_STRUCTURE_CHANGE:

			break;
		case GST_MESSAGE_STREAM_STATUS:
			gst_message_parse_stream_status( message, &_type, &_owner );

			break;
		case GST_MESSAGE_APPLICATION:

			break;
		case GST_MESSAGE_ELEMENT:

			break;
		case GST_MESSAGE_SEGMENT_START:

			break;
		case GST_MESSAGE_SEGMENT_DONE:

			break;
		case GST_MESSAGE_DURATION:
	
			break;
		case GST_MESSAGE_LATENCY:

			break;
		case GST_MESSAGE_ASYNC_START:

			break;
		case GST_MESSAGE_ASYNC_DONE:
			g_message( "[ GST_MESSAGE_ASYNC_DONE ]" );
			break;
		case GST_MESSAGE_REQUEST_STATE:

			break;
		case GST_MESSAGE_STEP_START:

			break;
		case GST_MESSAGE_QOS:

			break;
		case GST_MESSAGE_PROGRESS:

			break;
		case GST_MESSAGE_ANY:

			break;
		default:
			g_message( "un-catched message" );
			/*   unhandled message */
			break;
	}
	return TRUE;
}		/*  -----  end of static function _busFunc  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: _create_pipeline
 *  Description:
 * =====================================================================================
 */
static gboolean _create_pipeline( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	_priv->pipeline	=	gst_element_factory_make( "playbin2", "player" );

	if( _priv->pipeline	== NULL )
	{
		g_message("create pipeline Failed!");
		return	FALSE;
	}

	_priv->bus = gst_pipeline_get_bus(GST_PIPELINE( _priv->pipeline ));

	if( _priv->bus	== NULL )
	{
		g_message("create bus Failed!");
		return	FALSE;
	}

	_priv->watchid	=	gst_bus_add_watch( _priv->bus, _busFunc, pb );

	if( pb->video_sink_name != NULL )
	{
		GstElement	* _sink;

		_sink	=	gst_element_factory_make( pb->video_sink_name, NULL );

		if( g_strcmp0( pb->video_sink_name, "glimagesink" ) == 0 )
		{
			if( _priv->disp_x	!=	0 )
				g_object_set( _sink, "window_x", _priv->disp_x, NULL);
			if( _priv->disp_y	!=	0 )
				g_object_set( _sink, "window_y", _priv->disp_y, NULL);
			if( _priv->disp_width	!=	0 )
				g_object_set( _sink, "window_width", _priv->disp_width, NULL);
			if( _priv->disp_height	!=	0 )
				g_object_set( _sink, "window_height", _priv->disp_height, NULL);

			if( _priv->window_layer	!=	0 )
				g_object_set( _sink, "window_layer", _priv->window_layer, NULL);
			if( _priv->force_aspect_ratio	!=	FALSE )
				g_object_set( _sink, "force_aspect_ratio", _priv->force_aspect_ratio, NULL);
		}

		g_object_set( _priv->pipeline, "video_sink", _sink, NULL);
	}

	if( pb->audio_sink_name != NULL )
	{
		GstElement	* _sink;

		_sink	=	gst_element_factory_make( pb->audio_sink_name, NULL );

		g_object_set( _priv->pipeline, "audio_sink", _sink, NULL);
	}

	return TRUE;
}      /*  -----  end of static function _create_pipeline  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dispose_callback
 *  Description:  
 * =====================================================================================
 */
static gboolean _dispose_callback( gpointer user_data )
{
	g_message("streaming dev dispose callback");

	MHStreamingPb * _pb	=	( MHStreamingPb * )user_data;
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( _pb );

	if( _priv->bus )
		gst_object_unref ( _priv->bus );
	
	if( _priv->watchid )
	{
		g_source_remove( _priv->watchid );
		_priv->watchid = 0;
	}

	if( _priv->pipeline	!=	NULL)
	{
		g_source_set_ready_time( _priv->time_source, -1 );
		gst_element_set_state( _priv->pipeline, GST_STATE_NULL );
		gst_object_unref ( GST_OBJECT( _priv->pipeline ));
		_priv->pipeline	=	NULL;
	}

	if( _priv->time_source != NULL )
		g_source_unref( _priv->time_source );

	if( _pb->playlist != NULL ) 
	{
		mh_object_unref((MHObject *)_pb->playlist );
		_pb->playlist	=	NULL;
	}

	if( _priv->mhDev != NULL ) 
	{
		mh_object_unref((MHObject *)_priv->mhDev );
		_priv->mhDev	=	NULL;
	}

	if( _priv->path	!= NULL )
	{
		g_free( _priv->path );
		_priv->path	=	NULL;
	}

	return G_SOURCE_REMOVE;
}		/* -----  end of static function _dispose_callback  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name: pipeline_dispose
 *  Description:
 * =====================================================================================
 */
static void pipeline_dispose( MHDev * self,  void * user_data )
{
	MHStreamingPb * _pb	=	( MHStreamingPb * )user_data;

	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _dispose_callback, user_data, NULL );

	mh_io_dispatch( MH_IO( _pb ), _source );

	g_source_unref( _source );
}		/* -----  end of static function player_dispose  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_play_by_list
 *  Description:  
 * =====================================================================================
 */
bool mh_streaming_pb_play_by_list( MHStreamingPb * pb, MHPlaylist * playlist, uint32_t index )
{
	g_message( "%s",__func__ );

	gchar * _uri;

	uint32_t _count, _ptime, _index, _repeat, _shuffle;	
	
	gboolean _ret = FALSE;

	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	if( playlist	==	NULL )
	{
		g_message( "%s playlist	is NULL!\n",__func__);
		return FALSE;
	}

	if( pb->playlist != NULL )
	{
		mh_object_unref( (MHObject *)pb->playlist );
		pb->playlist	=	NULL;
	}
	pb->playlist	=	( MHPlaylist * )mh_object_ref( ( MHObject * )playlist );

	if( pb->playlist	==	NULL ) return FALSE;

	if( _priv->pipeline	==	NULL )
	{
		if(_create_pipeline( pb ) == FALSE )
		{
			g_message("create pipeline Failed");
			return FALSE;
		}

		MHDev * _dev;
		mh_object_get_properties(( MHObject * )playlist, "dev", &_dev, NULL );

		if( _dev	==	NULL )  return FALSE;

		_priv->mhDev 	=	( MHDev * )mh_object_ref(( MHObject * )_dev) ;
		
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	pipeline_dispose,
			.user_data	=	pb
		};
		mh_dev_register_detach_listener( _priv->mhDev, & _detach_listener);

		_set_ptime_source( pb );

	}else{
		_stop( pb );	
	}

	mh_object_get_properties(( MHObject * )playlist, "count", &_count, "shuffle", &_shuffle, "repeat", &_repeat,
			"index", &_index, "ptime", &_ptime, NULL );

	if( _count	==	0 )
	{
		g_message( "%s playlist	count is zero!\n",__func__);
		return FALSE;
	}

	g_message(" _count = [ %d ] index = [ %d ]", _count, index );

	if( _count <= index )
	{
		index	=	0;
		_ptime	=	0;

		mh_object_set_properties(( MHObject * )playlist, "index", &index, "ptime", _ptime, NULL );
	}

	_priv->repeat	=	_repeat;
	_send_repeat_signal( pb, _priv->repeat );	

	_priv->shuffle	=	_shuffle;
	_send_shuffle_signal( pb, _priv->shuffle );

	_priv->count	=	_count;

	_get_playlist_seq( pb );

	if( _shuffle == MH_PB_SHUFFLE_ALL )
	{
		uint32_t _j;
		for( _j = 0; _j < _count; _j++ )
		{
			if( pb->seq[_j] == index )
			{
				pb->index	=	_j;
				_priv->shuffle_start_index	=	_j;
				g_message("shuffle mode playlist index	=	%d",pb->index);
				break;
			}
		}
	}else{
		pb->index	=	index;	
	}

	_set_media_uri( pb );

	if( _ptime	!= 0 )
	{
		gst_element_set_state( _priv->pipeline, GST_STATE_PAUSED );
		gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

//		_seek();
	}

	gst_element_set_state( _priv->pipeline, GST_STATE_PLAYING );
	gst_element_get_state( _priv->pipeline, NULL, NULL, -1 );

	g_signal_emit_by_name( pb, "streaming_status_update", MH_PB_STATE_PLAYING );

	g_source_set_ready_time( _priv->time_source, 0 );

	mh_object_set_properties( ( MHObject * )playlist, "index", index, NULL );

	return TRUE;
}		/* -----  end of function mh_streaming_pb_play_by_list  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_next
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_next( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );
	
//	_next( pb );
	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _next, pb, NULL );

	mh_io_dispatch( MH_IO( pb ), _source );

	g_source_unref( _source );
}		/* -----  end of function mh_streaming_pb_next  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_previous
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_previous( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );

	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _previous, pb, NULL );

	mh_io_dispatch( MH_IO( pb ), _source );

	g_source_unref( _source );

}		/* -----  end of function mh_streaming_pb_previous  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_stop
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_stop( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );

	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _pb_stop, pb, NULL );

	mh_io_dispatch( MH_IO( pb ), _source );

	g_source_unref( _source );

}		/* -----  end of function mh_streaming_pb_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_play
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_play( MHStreamingPb * pb )
{	
	g_message( "%s",__func__ );

	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _play, pb, NULL );

	mh_io_dispatch( MH_IO( pb ), _source );

	g_source_unref( _source );

}		/* -----  end of function mh_streaming_pb_play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_pause
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_pause( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );

	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _pause, pb, NULL );

	mh_io_dispatch( MH_IO( pb ), _source );

	g_source_unref( _source );

}		/* -----  end of function mh_streaming_pb_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_forward
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_forward( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );
}		/* -----  end of function mh_streaming_pb_forward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_backward
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_backward( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );
}		/* -----  end of function mh_streaming_pb_backward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_forward_done
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_forward_done( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );
}		/* -----  end of function mh_streaming_pb_forward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_forward_done
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_backward_done( MHStreamingPb * pb )
{
	g_message( "%s",__func__ );
}		/* -----  end of function mh_streaming_pb_backward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_seek
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_seek( MHStreamingPb * pb, uint32_t second )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	g_message( "%s",__func__ );

	GstEvent *_seek_event = NULL;
	guint64 _seekpos;
	GstSeekFlags _seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_KEY_UNIT;
//	GstSeekFlags _seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT;
	gboolean _ret;

	_seekpos = ( guint64 )second * GST_MSECOND;

	g_message("_seekpos = [%lld]\n",_seekpos );

	gst_element_seek (_priv->pipeline, 1.0, GST_FORMAT_TIME,
			_seek_flags, GST_SEEK_TYPE_SET, _seekpos, GST_SEEK_TYPE_NONE, GST_SEEK_TYPE_NONE);

	/*  and block for the seek to complete */
//	GST_INFO ("done seeking %d", res);
	gst_element_get_state (_priv->pipeline, NULL, NULL, -1);

//	_seek_event = gst_event_new_seek( 1.0, GST_FORMAT_TIME,
//			_seek_flags,
//			GST_SEEK_TYPE_SET,_seekpos,
//			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );
//
//	if( gst_element_send_event( _priv->pipeline, _seek_event ) == FALSE )
//	{
//		g_message( " Seek Failer: send seek event failed!\n" );
////		return FALSE;
//	}
}		/* -----  end of function mh_streaming_pb_seek  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_playlist_change
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_playlist_change( MHStreamingPb * pb, MHPlaylist * playlist )
{
	g_message( "%s",__func__ );

	if( playlist != NULL )
	{
		MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

		if( pb->playlist != NULL ) 
			g_object_unref( pb->playlist );
		pb->playlist	=	g_object_ref(( MHPlaylist *)pb->playlist ) ;

		uint32_t _count, _index, _repeat, _shuffle;	

		mh_object_get_properties(( MHObject * )playlist, "count", &_count, "shuffle", &_shuffle, "repeat", &_repeat,
				"index", &_index, NULL );
		_priv->count	=	_count;
		_priv->repeat	=	_repeat;
		_priv->shuffle	=	_shuffle;
		if( _shuffle == MH_PB_SHUFFLE_ALL )
		{
			uint32_t _i;
			for( _i = 0; _i < _count; _i++ )
			{
				if( pb->seq[_i] == _index )
				{
					pb->index	=	_i;
					_priv->shuffle_start_index	=	_i;
					g_message("shuffle mode playlist index	=	%d",pb->index);
					break;
				}
			}
		}else{
			pb->index	=	_index;	
		}
		
		_get_playlist_seq( pb );

	}else{
		g_message( "playlist is NULL!\n");
	}
}		/* -----  end of function mh_streaming_pb_playlist_change  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_resize
 *  Description:  
 * =====================================================================================
 */
void mh_streaming_pb_resize( MHStreamingPb * pb, uint32_t offsetx, uint32_t offsety, uint32_t width, uint32_t height )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	GstElement * _video_sink = NULL;

	gboolean _result = FALSE;

	g_object_get( G_OBJECT( _priv->pipeline ), "video-sink", &_video_sink, NULL );

	g_message( "offsetx = [%d] offsety = [%d] width = [%d] height = [%d]\n", offsetx, offsety, width, height );

	if( _video_sink != NULL )
	{
		if( TRUE == is_isink( _video_sink ))
		{
			_result = update_mfw_isink_size( pb, _video_sink , offsetx, offsety, width, height );
		}
		else if ( TRUE == is_glimagesink( _video_sink )) 
		{
			_result = update_glimagesink_size( pb, _video_sink , offsetx, offsety, width, height);
		}
//		if( _result	==	TRUE )
//		{
//			GstState _current;
//
//			gst_element_get_state( self->playbin2, &_current, NULL, GST_SECOND );
//
//			if( _current != GST_STATE_PLAYING )
//			{
//				GstFormat _fmt = GST_FORMAT_TIME;
//				gint64 _pos = -1;
//
//				GstEvent *_seek_event = NULL;
//				GstSeekFlags _seek_flags = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE;
//
//				gst_element_query_position( _video_sink, &_fmt, &_pos ); 
//
//				_seek_event = gst_event_new_seek( 1.0, GST_FORMAT_TIME,
//						_seek_flags,
//						GST_SEEK_TYPE_SET, _pos,
//						GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );
//
//				if( gst_element_send_event( self->playbin2, _seek_event ) == FALSE )
//				{
//					g_message( " Seek Failer: send seek event failed!\n" );
//					return FALSE;
//				}
//			}
//		}
	}else{
		g_message("Get video sink Error");
	}
}		/* -----  end of function mh_streaming_pb_resize  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_get_subtitle_info
 *  Description:  
 * =====================================================================================
 */
MHPbSubtitleInfo * mh_streaming_pb_get_subtitle_info( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gint i;
	gchar *str;
	GstTagList *tags;

	MHPbSubtitleInfo	* _info	=	(MHPbSubtitleInfo *)g_malloc0(sizeof( MHPbSubtitleInfo));

	g_object_get( G_OBJECT( _priv->pipeline ), "n-text", &_info->total_count, NULL );
	g_object_get( G_OBJECT( _priv->pipeline ), "current-text", &_info->current_count, NULL );

	_info->subtitle_name	=	g_new0( char *, _info->total_count );

	g_message("total_count = %d, current_count	=	%d\n",_info->total_count,_info->current_count);

	for (i = 0; i < _info->total_count; i++) {  
		tags = NULL;  
		/*   Retrieve the stream's audio tags */  
		g_signal_emit_by_name ( _priv->pipeline, "get-text-tags", i, &tags );  

		if (tags) {  

			if (gst_tag_list_get_string (tags, GST_TAG_LANGUAGE_CODE, &str)) {  
				g_message ("  language: %s\n", str);  
				_info->subtitle_name[i] =	g_strdup( str );
				g_free (str);  
			}  

			gst_tag_list_free (tags);  
		}else{
			g_message(" no tags found\n");
		}
	}

	return _info;
}		/* -----  end of function mh_streaming_pb_get_subtitle_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_get_track_info
 *  Description:  
 * =====================================================================================
 */
MHPbTrackInfo * mh_streaming_pb_get_track_info( MHStreamingPb * pb )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( pb );

	gint i;
	gchar *str;
	GstTagList *tags;

	MHPbTrackInfo	* _info	=	(MHPbTrackInfo *)g_malloc0(sizeof( MHPbTrackInfo));

	g_object_get( G_OBJECT( _priv->pipeline ), "n-audio", &_info->total_count, NULL );
	g_object_get( G_OBJECT( _priv->pipeline ), "current-audio", &_info->current_count, NULL );

	_info->track_name	=	g_new0( char *, _info->total_count );

	for ( i = 0; i < _info->total_count; i++ ) 
	{  
		tags = NULL;  
		/*   Retrieve the stream's audio tags */  
		g_signal_emit_by_name ( _priv->pipeline, "get-audio-tags", i, &tags );  

		if ( tags ) 
		{  
			if ( gst_tag_list_get_string ( tags, GST_TAG_LANGUAGE_CODE, &str )) 
			{  
				g_message ("  language: %s\n", str);  
				_info->track_name[i] =	g_strdup( str );
				g_free (str);  
			}  

			gst_tag_list_free (tags);  
		}  
	}

	return _info;
}		/* -----  end of function mh_streaming_pb_get_track_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_create
 *  Description:  
 * =====================================================================================
 */
MHStreamingPb * mh_streaming_pb_create(MHIPCConnection * conn )
{
	MHStreamingPb * _pb	=	g_object_new( MH_TYPE_STREAMING_PB, "io-name", "streaming-pb", NULL );
	g_message("unique_name:%s", g_dbus_connection_get_unique_name( (GDBusConnection*) conn));
	_pb->connect		=	conn;
	return _pb;
}		/* -----  end of function mh_streaming_pb_create  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_property
 *  Description:
 * =====================================================================================
 */
static void _set_property( GObject * object, guint property_id, const GValue * value,
		GParamSpec * spec)
{
	MHStreamingPb * _self  =   MH_STREAMING_PB( object );
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( _self );

	switch( property_id )
	{
		case PROP_STREAMING_PB_VIDEO_SINK:
			_self->video_sink_name = g_strdup( g_value_get_string( value ));
			g_message( " video_sink_name = [ %s ]\n", _self->video_sink_name );
			break;

		case PROP_STREAMING_PB_AUDIO_SINK:
			_self->audio_sink_name = g_strdup( g_value_get_string( value ));
			g_message( " audio_sink_name = [ %s ]\n", _self->audio_sink_name );
			break;

		case PROP_STREAMING_PB_INDEX:
			{
			g_message( " set index  = [ %d ]\n", g_value_get_uint( value ) );
//				_set_index( _self, g_value_get_uint( value ) );
				GSource * _source	=	g_idle_source_new();

				MHStreamingPbParam *_pbParam = g_new( MHStreamingPbParam, 1 );
				_pbParam->self = _self;
				_pbParam->index = g_value_get_uint( value );

				g_source_set_callback( _source, _set_index, _pbParam, g_free );

				mh_io_dispatch( MH_IO( _self ), _source );

				g_source_unref( _source );
			}
			break;

		case PROP_STREAMING_PB_REPEAT:
			_priv->repeat	=	g_value_get_uint( value );
			g_message( "repeat	=	%d\n",_priv->repeat );
			_set_repeat( _self );
			break;

		case PROP_STREAMING_PB_SHUFFLE:
			_priv->shuffle	=	g_value_get_uint( value );
			g_message( "shuffle	=	%d\n",_priv->shuffle );
			_set_shuffle( _self );
			break;

		case PROP_STREAMING_PB_WINDOW_LAYER:
			_priv->window_layer	=	g_value_get_uint( value );
			g_message( " window_layer = [ %d ]\n", _priv->window_layer );
			break;

		case PROP_STREAMING_PB_FORCE_ASPECT_RATIO:
			_priv->force_aspect_ratio	=	g_value_get_boolean( value );
			g_message( "force_aspect_ratio = [ %d ]\n", _priv->force_aspect_ratio );
			break;

		case PROP_STREAMING_PB_WINDOW_X:
			_priv->disp_x	=	g_value_get_uint( value );
			g_message( "window_x	=	%d\n", _priv->disp_x );
			break;

		case PROP_STREAMING_PB_WINDOW_Y:
			_priv->disp_y	=	g_value_get_uint( value );
			g_message( "window_y	=	%d\n", _priv->disp_y );
			break;

		case PROP_STREAMING_PB_WINDOW_WIDTH:
			_priv->disp_width=	g_value_get_uint( value );
			g_message( "window_width	=	%d\n", _priv->disp_width );
			break;

		case PROP_STREAMING_PB_WINDOW_HEIGHT:
			_priv->disp_height=	g_value_get_uint( value );
			g_message( "window_width	=	%d\n", _priv->disp_height );
			break;

		case PROP_STREAMING_PB_AUDIO_TRACK:
			_set_audio_track( _self, g_value_get_uint( value ));
			g_message( "audio track	=	%d\n", g_value_get_uint( value ) );
			break;

		case PROP_STREAMING_PB_SUBTITLE:
			_set_subtitle( _self, g_value_get_uint( value ));
			g_message( "subtitle number	=	%d\n", g_value_get_uint( value ) );
			break;
		case PROP_STREAMING_PB_SHARED:
			_self->shared	=	g_value_get_boolean( value );
			g_message( "shared = [ %d ]\n", _self->shared );
			break;

		default:
			break;
	}
}       /*   -----  end of static function _set_property  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _get_property
 *  Description:
 * =====================================================================================
 */
static void _get_property( GObject * object, guint property_id, GValue * value,
		GParamSpec * spec)
{
	MHStreamingPb * _self  =   MH_STREAMING_PB( object );

	switch( property_id )
	{
		case PROP_STREAMING_PB_VIDEO_SINK:
			g_value_set_string( value, _self->video_sink_name );
			break;
		case PROP_STREAMING_PB_AUDIO_SINK:
			g_value_set_string( value, _self->audio_sink_name );
			break;
		default:
			break;
	}

}       /*   -----  end of static function _get_property  ----- */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_init
 *  Description:
 * =====================================================================================
 */
static void mh_streaming_pb_init( MHStreamingPb * self )
{
	MHStreamingPbPrivate * _priv	=	mh_streaming_pb_get_instance_private( self );

	self->video_sink_name	=	NULL;
	self->audio_sink_name	=	NULL;
	self->seq	=	NULL;
	self->index	=	0;
	self->playlist	=	NULL;
	self->shared	=	FALSE;
	_priv->pipeline	=	NULL;
	_priv->count	=	0;
	_priv->time_source	=	NULL;
	_priv->shuffle	=	MH_PB_SHUFFLE_OFF;
	_priv->repeat	=	MH_PB_REPEAT_MODE_OFF;
	_priv->path	=	NULL;
	_priv->taginfo	=	NULL;
	_priv->window_layer	=	0;
	_priv->force_aspect_ratio	=	FALSE;
	_priv->disp_x = 0;
	_priv->disp_y = 0;
	_priv->disp_width = 0;
	_priv->disp_height = 0;
	_priv->watchid	=	0;
}       /* -----  end of static function mh_streaming_pb_init  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_class_init
 *  Description:
 * =====================================================================================
 */
static void mh_streaming_pb_class_init( MHStreamingPbClass * klass )
{
	GObjectClass * _gobjectClass	=	G_OBJECT_CLASS( klass );

	_gobjectClass->set_property =   _set_property;
	_gobjectClass->get_property =   _get_property;

	_gobjectClass->dispose	=	mh_streaming_pb_dispose;
	_gobjectClass->finalize	=	mh_streaming_pb_finalize;

	signals[ STREAMING_PB_EVENTS ]	=
		g_signal_new( "streaming_pb_events",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				2	
				, G_TYPE_UINT, G_TYPE_POINTER );

	signals[ STREAMING_STATUS_UPDATE ]	=
		g_signal_new( "streaming_status_update",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				1
				, G_TYPE_UINT );

	pbProperties[ PROP_STREAMING_PB_VIDEO_SINK ]	=	
		g_param_spec_string( "video_sink", "MHStreamingPb property", "video sink name",
				"", G_PARAM_READWRITE );
	pbProperties[ PROP_STREAMING_PB_AUDIO_SINK ]	=	
		g_param_spec_string( "audio_sink", "MHStreamingPb property", "audio sink name",
				"", G_PARAM_READWRITE );
	pbProperties[ PROP_STREAMING_PB_INDEX ]	=	
		g_param_spec_uint( "index", "MHStreamingPb property", "Index of the playing item in the playlist",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_REPEAT ]	=	
		g_param_spec_uint( "repeat", "MHStreamingPb property", "Repeat mode of the playback",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_SHUFFLE ]	=	
		g_param_spec_uint( "shuffle", "MHStreamingPb property", "Shuffle mode of the playback",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE  );
	pbProperties[ PROP_STREAMING_PB_WINDOW_LAYER ]	=	
		g_param_spec_uint( "window_layer", "MHStreamingPb property", "window layer",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_FORCE_ASPECT_RATIO ]	=	
		g_param_spec_boolean( "force-aspect-ratio", "MHStreamingPb property", "window force aspect ratio",
				FALSE, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_SHARED ]	=	
		g_param_spec_boolean( "shared", "MHStreamingPb property", "media shared",
				FALSE, G_PARAM_WRITABLE );


	pbProperties[ PROP_STREAMING_PB_WINDOW_X]	=	
		g_param_spec_uint( "window_x", "MHStreamingPb property", "window x",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_WINDOW_Y]	=	
		g_param_spec_uint( "window_y", "MHStreamingPb property", "window y",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_WINDOW_WIDTH]	=	
		g_param_spec_uint( "window_width", "MHStreamingPb property", "window width",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_WINDOW_HEIGHT]	=	
		g_param_spec_uint( "window_height", "MHStreamingPb property", "window height",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );

	pbProperties[ PROP_STREAMING_PB_AUDIO_TRACK ]	=	
		g_param_spec_uint( "audio_track", "MHStreamingPb property", "audio track",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );
	pbProperties[ PROP_STREAMING_PB_SUBTITLE ]	=	
		g_param_spec_uint( "subtitle", "MHStreamingPb property", "video subtitle",
				0, G_MAXUINT, 0, G_PARAM_WRITABLE );

	g_object_class_install_properties( _gobjectClass, N_PROPERTIES, pbProperties );
}       /* -----  end of static function mh_pb_class_init  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_get_properties
 *  Description:  
 * =====================================================================================
 */
bool mh_streaming_pb_get_properties( MHStreamingPb * pb, const char * first_property_name, ...)
{
	return 0;
}		/* -----  end of static function mh_streaming_pb_get_properties  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_pb_set_properties
 *  Description:  
 * =====================================================================================
 */
bool mh_streaming_pb_set_properties( MHStreamingPb * pb, const char * first_property_name, ...)
{
	va_list _varArgs;

	g_return_val_if_fail( G_IS_OBJECT( pb ), false );

	va_start( _varArgs, first_property_name );

	g_object_set_valist( G_OBJECT( pb ), first_property_name, _varArgs );

	va_end( _varArgs );

	return true;

}		/* -----  end of static function mh_streaming_pb_set_properties  ----- */
