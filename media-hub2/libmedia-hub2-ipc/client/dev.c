/*
 * =====================================================================================
 *
 *       Filename:  dev.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:01:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif


extern Mediahub2Dbus * dbusClient;

typedef struct _MHDevDetachDBusInfo 
{
	MHDev * dev;
	MHDevDetachListener listener;
	uint64_t	detach_id;
} MHDevDetachDBusInfo;				/* ----------  end of struct MHDevDetachDBusInfo  ---------- */

typedef struct _MHDevEventDBusInfo 
{
	MHDev * dev;
	MHDevEventsListener listener;
	uint64_t	event_id;
	uint64_t	detach_id;
} MHDevEventDBusInfo;				/* ----------  end of struct MHDevEventDBusInfo  ---------- */

typedef struct _MHDevStatusDBusInfo 
{
	MHDev * dev;
	MHDevStatusListener listener;
	uint64_t status_id;
	uint64_t detach_id;
} MHDevStatusDBusInfo;				/* ----------  end of struct MHDevStatusDBusInfo  ---------- */

typedef struct _MHDevModuleStatusInfo 
{
	MHDev * dev;
	ModuleStatusListener * listener;
	uint64_t status_id;
	uint64_t detach_id;

} MHDevModuleStatusInfo;				/* ----------  end of struct MHDevModuleStatus  ---------- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_start_scan
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_start_scan( MHDev * self, MHDevScanType type)
{
	printf("[%s : %s : %d][type : %d]\n", __FILE__, __func__, __LINE__, type);
	g_return_val_if_fail( self != NULL, MH_INVALID_PARAM);

	if( type < SCAN_FOLDER || type > SCAN_TAG)
	{
		return MH_INVALID_PARAM;
	}
	MHResult _res	=	MH_OK;
#ifdef __x86_64__
	if(mediahub2_dbus_call_dev_start_scan_sync(dbusClient, (guint64)self, type, (gint *)&_res, NULL, NULL))
	{
		;
	}
#else
	if(mediahub2_dbus_call_dev_start_scan_sync( dbusClient, (guint)self, type, (guint *)&_res, NULL, NULL))
	{
		;	
	}
#endif
	else
	{
		g_message("dbus error\n");
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_dev_start_scan  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_restore_playlist
 *  Description:  
 * =====================================================================================
 */
MHPlaylist * mh_dev_restore_playlist( MHDev * self, int64_t playlist_id )
{
	MHPlaylist * _playlist	=	NULL;

#ifdef __x86_64__
	mediahub2_dbus_call_dev_restore_playlist_sync( dbusClient, (guint64)self,
			playlist_id, (guint64 *)&_playlist, NULL, NULL);
#else
	mediahub2_dbus_call_dev_restore_playlist_sync( dbusClient, ( guint )self,
			playlist_id, ( guint * )&_playlist,	NULL, NULL );
#endif

	return _playlist;
}		/* -----  end of function mh_dev_restore_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_detach
 *  Description:  
 * =====================================================================================
 */
static void _dev_detach( Mediahub2Dbus * proxy, MHDev * dev, gpointer * user_data)
{


	MHDevDetachDBusInfo * _info	=	( MHDevDetachDBusInfo * )user_data;

	if( dev == _info->dev )
	{
		_info->listener.callback( _info->dev, _info->listener.user_data );

		g_signal_handler_disconnect(dbusClient, _info->detach_id);
	g_free(_info);	
	}
}		/* -----  end of static function _dev_detach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_register_detach_listener
 *  Description:  
 * =====================================================================================
 */
uint32_t mh_dev_register_detach_listener(MHDev * self, MHDevDetachListener * listener)
{


	g_return_val_if_fail( self != NULL && listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_dev_register_detach_listener_sync( dbusClient, (guint64) self, 
			NULL, &_error);
#else
	mediahub2_dbus_call_dev_register_detach_listener_sync( dbusClient, ( guint )self,
			NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_dev_register_detach_listener falied: [ %s ]", _error->message );

		g_error_free( _error );
	}
	else
	{
		MHDevDetachDBusInfo * _info	=	g_new0( MHDevDetachDBusInfo, 1 );

		_info->dev	=	self;
		_info->listener		=	* listener;

		_info->detach_id	=	g_signal_connect( dbusClient, "dev_detach", G_CALLBACK( _dev_detach ), _info );

	}


	return 0;

}		/* -----  end of function mh_dev_register_detach_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_event_detach
 *  Description:  
 * =====================================================================================
 */
static void _dev_event_detach( Mediahub2Dbus * proxy, MHDev * dev, gpointer * user_data)
{
	MHDevEventDBusInfo * _info	=	( MHDevEventDBusInfo * )user_data;

	g_signal_handler_disconnect(dbusClient, _info->detach_id);
	g_signal_handler_disconnect(dbusClient, _info->event_id);		
	g_free(_info);

}		/* -----  end of static function _dev_event_detach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_event
 *  Description:  
 * =====================================================================================
 */
static void _dev_event(Mediahub2Dbus * proxy,MHDev * dev , MHDevScanCbType scan_type, MHItemType item_type, void * data,uint32_t percent, gpointer user_data)
{
	MHDevEventDBusInfo * _info	=	( MHDevEventDBusInfo * )user_data;
	g_message("_dev_event start");
	if( dev == _info->dev )
	{
		g_message("_dev_event callback");
		_info->listener.callback( _info->dev, scan_type, item_type, data, percent, _info->listener.user_data );
//		if( scan_type == MH_DEV_FINISH)
//		{
//			g_signal_handler_disconnect( dbusClient, _info->event_id);
//		}
	}
	
}		/* -----  end of static function _dev_event  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_register_events_listener
 *  Description:  
 * =====================================================================================
 */
uint32_t mh_dev_register_events_listener( MHDev * self, MHDevEventsListener * listener)
{
	g_return_val_if_fail( self != NULL && listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_dev_register_events_listener_sync( dbusClient, ( guint64 )self,
			NULL, &_error );
#else
	mediahub2_dbus_call_dev_register_events_listener_sync( dbusClient, ( guint )self,
			NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_dev_register_events_listener falied: [ %s ]", _error->message );

		g_error_free( _error );
	}
	else
	{
		MHDevEventDBusInfo * _info	=	g_new0( MHDevEventDBusInfo, 1 );

		_info->dev	=	self;
		_info->listener		=	* listener;
		
		g_message("mh_dev_register_events_listener self = %p", _info->dev);
		_info->event_id		=	g_signal_connect( dbusClient, "dev_event", G_CALLBACK( _dev_event ), _info );
		_info->detach_id	=	g_signal_connect( dbusClient, "dev_detach", G_CALLBACK( _dev_event_detach ), _info );

	}


	return 0;
}		/* -----  end of function mh_dev_register_events_listener  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_get_playlist_info
 *  Description:  
 * =====================================================================================
 */
MHPlaylistInfo * mh_dev_get_playlist_info( MHDev * self)
{
	g_return_val_if_fail( self != NULL, NULL);
	GVariant * _info;
	GVariantIter * _it;
	int _size;
	int _i = 0;
	MHPlaylistInfo * _res = NULL;
	char * _pvar;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_get_playlist_info_sync( dbusClient, (guint64)self, &_info, NULL,NULL); 
#else
	_dbusRes	=	mediahub2_dbus_call_dev_get_playlist_info_sync( dbusClient, (guint)self, &_info, NULL,NULL);
#endif
	if( _dbusRes)
	{
		_size = g_variant_n_children( _info);
		if( _size > 0)
		{
			g_variant_get( _info, "a(xs)", &_it);
	
			_res	=	(MHPlaylistInfo *) g_malloc0( sizeof(MHPlaylistInfo)*(_size+1) );

			while( g_variant_iter_loop( _it,"(xs)",&_res[_i].playlistid,&_pvar))
			{
				_res[_i].name	=	g_strdup( _pvar );
				_i++;
			}
			g_variant_iter_free ( _it );
		}
		else
		{
			_res	=	(MHPlaylistInfo*)g_malloc0(sizeof(MHPlaylistInfo));
		}
	}

 	g_variant_unref (_info);

	return _res;
}		/* -----  end of function mh_dev_get_playlist_info  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_save_playlist
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_save_playlist( MHDev * self, const char * name, MHPlaylist * playlist )
{
	g_return_val_if_fail( self != NULL && name != NULL && playlist != NULL, MH_INVALID_PARAM);
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_save_playlist_sync( dbusClient, ( guint64 )self, 
				( const gchar * )name, (guint64)playlist, (gint *)&_res, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_save_playlist_sync( dbusClient, ( guint )self, 
				( const gchar * )name, (guint)playlist, (gint *)&_res, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_dev_save_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_update_playlist
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_update_playlist( MHDev * self, const char * name, MHPlaylist * playlist, int64_t playlist_id)
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_update_playlist_sync( dbusClient, (guint64)self, (const gchar *)name,
				(guint64) playlist, (gint64) playlist_id, (gint *)&_res, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_update_playlist_sync( dbusClient, (guint)self, (const gchar *)name,
				(guint) playlist, (gint64) playlist_id,(gint *)&_res, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_dev_update_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_delete_playlist
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_delete_playlist( MHDev * self, int64_t playlist_id)
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_delete_playlist_sync( dbusClient, (guint64)self, 
				(gint64)playlist_id, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_delete_playlist_sync( dbusClient, (guint)self, 
				(gint64)playlist_id, (gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_dev_delete_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_get_radiolist
 *  Description:  
 * =====================================================================================
 */
char **  mh_dev_get_radiolist( MHDev * self, int * count)
{
	GError * _error	=	NULL;

	char ** data	=	NULL;

	GVariant * _result;
	
	printf("%s \n", __func__);
	
#ifdef __x86_64__
	mediahub2_dbus_call_dev_get_radiolist_sync( dbusClient, (guint64)self,  (gint)*count, count, &_result,NULL, &_error);
#else
	mediahub2_dbus_call_dev_get_radiolist_sync( dbusClient, (guint)self,  (gint)*count, count, &_result,NULL, &_error);
#endif

	if(_error != NULL)
	{
		g_warning( "mh_dev_get_radiolist failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	else
	{
		if( *count > 0 )
		{
			int _offset	=	0;
			GVariantIter * _it;
			gchar * _val;

			data	=	g_new0( char *, * count );

			g_variant_get( _result, "a(s)", &_it );

			while( g_variant_iter_loop( _it, "(s)", &_val ))
			{
				data[ _offset ] = g_strdup( _val );
				_offset	++;
			}

			g_variant_unref( _result );
			g_variant_iter_free( _it );

		}
	}
	return data;

}		/* -----  end of function mh_dev_get_radiolist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_attach_pb
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_attach_pb( MHDev * self, MHPb * pb )
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_attach_pb_sync( dbusClient, (guint64)self, 
				(guint64)pb, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_attach_pb_sync( dbusClient, (guint)self, 
				(guint)pb, (gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;

}		/* -----  end of function mh_dev_attach_pb  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_scan_abort
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_scan_abort( MHDev * self)
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_scan_abort_sync( dbusClient, (guint64)self,(gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_scan_abort_sync( dbusClient, (guint)self,(gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;

}		/* -----  end of function mh_dev_scan_abort  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_request_app_launch
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_request_app_launch( MHDev * self, const char * app_bundle_id )
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_request_app_launch_sync( dbusClient,
			( guint64 )self, app_bundle_id, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_request_app_launch_sync( dbusClient,
			( guint )self, app_bundle_id, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}

	return _res;
}		/* -----  end of function mh_dev_request_app_launch  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_write_ea_data
 *  Description:  
 * =====================================================================================
 */
int32_t mh_dev_write_ea_data( MHDev * self, const uint8_t * buf, int32_t len )
{
	int _len	=	-1;
	GVariant * _var;
	gboolean _dbusRes;

	_var	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, buf, len, sizeof( uint8_t ));

	_var	=	g_variant_new_variant( _var );
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_write_ea_data_sync( dbusClient, ( guint64 )self, _var, &_len, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_write_ea_data_sync( dbusClient, ( guint )self, _var, &_len, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
	}

	return _len;
}		/* -----  end of function mh_dev_write_ea_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_write_bt_data
 *  Description:  
 * =====================================================================================
 */
int32_t mh_dev_write_bt_data( MHDev * self, const uint8_t * buf, int32_t len )
{
	int _len	=	-1;
	GVariant * _var;
	gboolean _dbusRes;

	_var	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, buf, len, sizeof( uint8_t ));

	_var	=	g_variant_new_variant( _var );
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_write_bt_data_sync( dbusClient, ( guint64 )self, _var, &_len, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_write_bt_data_sync( dbusClient, ( guint )self, _var, &_len, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
	}

	return _len;
}		/* -----  end of function mh_dev_write_bt_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_write_location_data
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_write_location_data( MHDev * self, const char * data )
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_write_location_data_sync( dbusClient,
			( guint64 )self, data, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_write_location_data_sync( dbusClient,
			( guint )self, data, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}

	return _res;
}		/* -----  end of function mh_dev_write_location_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_send_vehicle_status
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_send_vehicle_status( MHDev * self, uint32_t remainingRange, int32_t outsideTempreture, int32_t rangeWarning ) 
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_send_vehicle_status_sync( dbusClient,
			( guint64 )self,( guint )remainingRange,( gint )outsideTempreture, ( gint )rangeWarning, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_send_vehicle_status_sync( dbusClient,
			( guint )self, ( guint )remainingRange,( gint )outsideTempreture, ( gint )rangeWarning, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}

	return _res;
}		/* -----  end of function mh_dev_send_vehicle_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_get_item_by_uri
 *  Description:  
 * =====================================================================================
 */
MHItem * mh_dev_get_item_by_uri( MHDev * self, const char * uri)
{
	MHItem * _item 	= 	NULL;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_get_item_by_uri_sync( dbusClient, ( guint64 )self,
			uri, ( guint64 * )&_item,	NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_get_item_by_uri_sync( dbusClient, ( guint )self,
			uri, ( guint * )&_item,	NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
	}
	return _item;

}		/* -----  end of function mh_dev_get_item_by_uri  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_create_empty_playlist
 *  Description:  
 * =====================================================================================
 */
MHPlaylist * mh_dev_create_empty_playlist( MHDev * self)
{
	g_return_val_if_fail( self != NULL, NULL);
	MHPlaylist * _playlist	=	NULL;
gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_create_empty_playlist_sync( dbusClient, 
			(guint64)self, (guint64 *)&_playlist, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_create_empty_playlist_sync( dbusClient,
			(guint)self, (guint *)&_playlist, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_playlist	=	NULL;
	}

	return _playlist;
}		/* -----  end of function mh_dev_create_empty_playlist  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_send_signal_touch
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_send_signal_touch( MHDev * dev, MHTouchType action, int x, int y )
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_send_signal_touch_sync( dbusClient,
			(guint64)dev, (guint)action, (guint)x, (guint)y, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_send_signal_touch_sync( dbusClient,
			(guint)dev, (guint)action, (guint)x, (guint)y, (gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_dev_send_signal_touch  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_start
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_start(MHDev * self)
{
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_start_sync( dbusClient,
			(guint64)self, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_start_sync( dbusClient,
			(guint)self,(gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;

}		/* -----  end of function mh_dev_start  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status
 *  Description:  
 * =====================================================================================
 */

static void _dev_status(Mediahub2Dbus * proxy, MHDev * self, MHDevStatus status, void * user_data)
{
	MHDevStatusDBusInfo * _info	=	(MHDevStatusDBusInfo *)user_data;
	if( self == _info->dev)
	{
		_info->listener.callback( _info->dev, status, _info->listener.user_data);

	}
}		/* -----  end of static function _dev_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_detach
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_detach(Mediahub2Dbus * proxy, MHDev * dev, gpointer *user_data)
{
	MHDevStatusDBusInfo * _info	=	(MHDevStatusDBusInfo *)user_data;
	g_signal_handler_disconnect( dbusClient, _info->detach_id);
	g_signal_handler_disconnect( dbusClient, _info->status_id );
}		/* -----  end of static function _dev_status_detach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_register_status_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_register_status_listener( MHDev * self, MHDevStatusListener * listener)
{
	g_return_val_if_fail( listener->callback != NULL, MH_INVALID_PARAM );
	MHResult _res	=	MH_OK;

	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_register_status_listener_sync( dbusClient,
			(guint64)self, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_register_status_listener_sync( dbusClient,
			(guint)self, (gint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}

	else
	{
		MHDevStatusDBusInfo * _info	=	g_new0( MHDevStatusDBusInfo, 1);

		_info->dev	=	self;
		_info->listener	=	*listener;
		
		_info->status_id	=	g_signal_connect( dbusClient, "dev_status", G_CALLBACK( _dev_status), _info);
		_info->detach_id	=	g_signal_connect( dbusClient, "dev_detach", G_CALLBACK( _dev_status_detach), _info);
		
	}
	return _res;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_add_file
 *  Description:  
 * =====================================================================================
 */
MHItem * mh_dev_add_file(MHDev * dest_dev, MHFolder * dest, MHItem * source)
{
	if( dest_dev == NULL || dest == NULL || source == NULL)
	{
		return NULL;
	}
	MHItem * _item	=	NULL;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_add_file_sync( dbusClient, 
			(guint64)dest_dev, (guint64)dest, (guint64)source, (guint64 *)&_item, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_add_file_sync( dbusClient, 
			(guint)dest_dev, (guint)dest, (guint)source, (guint *)&_item, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
	}
	return _item;

}		/* -----  end of function mh_dev_add_file  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_delete_file
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_delete_file( MHDev * dev, MHItem * item)
{
	if( dev == NULL || item == NULL)
	{
		return MH_INVALID_PARAM;
	}
	MHResult	_res	=	MH_OK;
	gboolean _dbusRes;

#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_detete_file_sync( dbusClient, 
			(guint64)dev, (guint64)item, (guint *)_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_detete_file_sync( dbusClient, 
			(guint)dev, (guint)item, (guint *)&_res, NULL, NULL);
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
		
	}
	return _res;

}		/* -----  end of function mh_dev_delete_file  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_event_detach
 *  Description:  
 * =====================================================================================
 */
static void _dev_module_status_detach( Mediahub2Dbus * proxy, MHDev * dev, gpointer * user_data)
{
	MHDevModuleStatusInfo * _info	=	( MHDevModuleStatusInfo * )user_data;

	g_signal_handler_disconnect(dbusClient, _info->detach_id);
	g_signal_handler_disconnect(dbusClient, _info->status_id);		

}		/* -----  end of static function _dev_module_status_detach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_module_status
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static void _dev_module_status( Mediahub2Dbus * proxy, guint64 arg_dev, guint arg_cnt, GVariant * list, gpointer user_data)
#else
static void _dev_module_status( Mediahub2Dbus * proxy, guint arg_dev, guint arg_cnt, GVariant * list, gpointer user_data)
#endif
{
	g_message("client->dev.c:%s", __func__);
	MHDev * _dev	=	(MHDev *)arg_dev;
	MHDevModuleStatusInfo *_info	=	(MHDevModuleStatusInfo *)user_data;
	if( _dev == _info->dev)
	{
		ModuleStatusList * _msList	=	g_new0( ModuleStatusList ,  1);
		GVariantIter * _it;
		int i	=	0;
		_msList->count	=	arg_cnt;
		
		_msList->list	=	(ModuleStatus *)g_new0( ModuleStatus, arg_cnt+1);
		
		g_variant_get( list, "a(uu)", &_it);
		
		while( g_variant_iter_loop( _it, "(uu)", &(_msList->list[i].module), &(_msList->list[i].status)))
		{
			i++;
		}

		_info->listener->callback(_dev, _msList,  _info->listener->user_data);
	}
}		/* -----  end of static function _dev_module_status  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_register_module_status
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_register_module_status( MHDev * dev, ModuleStatusListener * listener)
{

	g_return_val_if_fail( dev != NULL && listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;

	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_register_module_status_sync( dbusClient, (guint64)dev,  (gint *)&_res,NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_register_module_status_sync( dbusClient, (guint)dev,  (gint *)&_res,NULL, NULL );
#endif
	if( !_dbusRes )
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}
	else
	{
		MHDevModuleStatusInfo * _info	=	g_new0( MHDevModuleStatusInfo, 1);
		_info->dev	=	dev;
		_info->listener	=	listener;
		_info->status_id	=	g_signal_connect( dbusClient, "dev_module_status", G_CALLBACK( _dev_module_status), _info);
		_info->detach_id	=	g_signal_connect( dbusClient, "dev_detach", G_CALLBACK( _dev_module_status_detach), _info); 
		

	}
	return _res;

}		/* -----  end of function mh_dev_register_module_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_module_control
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_module_control( MHDev * dev, uint32_t module, uint32_t status)
{
	char _buf[10];
	sprintf(_buf, "%d%d", module, status);
	mh_object_set_properties((MHObject *)dev, "module_control", _buf, NULL);

	return MH_OK;
}		/* -----  end of function mh_dev_module_control  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_send_wifi_conf_info
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_send_wifi_conf_info( MHDev * self, const uint8_t * ssid, const uint8_t * pass, uint8_t securityType, uint8_t channel ) 
{
	MHResult _res	=	MH_OK;

	GVariant * _var_ssid;
	GVariant * _var_pass;
	gboolean _dbusRes;

	_var_ssid	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, ssid, ssid[0], sizeof( uint8_t ));

	_var_ssid	=	g_variant_new_variant( _var_ssid );

	_var_pass	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, pass, pass[0], sizeof( uint8_t ));

	_var_pass	=	g_variant_new_variant( _var_pass );

#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_send_wifi_conf_info_sync( dbusClient, ( guint64 )self, _var_ssid, _var_pass, securityType, channel, (guint *)&_res, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_dev_send_wifi_conf_info_sync( dbusClient, ( guint )self, _var_ssid, _var_pass, securityType, channel, (guint *)&_res, NULL, NULL );
#endif
	if( !_dbusRes)
	{
		g_message("%s:ipc error", __func__);
		_res	=	MH_IPC_ERROR;
	}
	
	return _res;
}		/* -----  end of function mh_dev_send_wifi_conf_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_search_name
 *  Description:  
 * =====================================================================================
 */
MHItem ** mh_dev_search_name( MHDev * dev, MHItemType type, char * string, int32_t * count)
{
	g_return_val_if_fail( dev != NULL && string != NULL, NULL);

	GVariant * _result;
	 
	MHItem ** _items	=	NULL;

	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_dev_search_name_sync( dbusClient, (guint64) dev, type, string,
			*count, count, &_result, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_dev_search_name_sync( dbusClient, (guint) dev, type, string,
			*count, count, &_result, NULL, NULL);
#endif
	if( _dbusRes)
	{
		if( *count > 0)
		{
			int _offset	=	0;
			GVariantIter * _it;

			_items	=	g_new0(MHItem *, *count);

#ifdef __x86_64__
			guint64 _val;
			g_variant_get( _result, "at", &_it);


			while( g_variant_iter_loop( _it, "t", &_val))
			{
				*(_items + _offset)	=	(MHItem *)_val;

				_offset++;
			}
#else
			guint _val;

			g_variant_get( _result, "au", &_it);

			while( g_variant_iter_loop( _it, "u", &_val))
			{
				*(_items + _offset) = (MHItem *)_val;

				_offset ++;
			}
#endif
			g_variant_unref( _result);
			g_variant_iter_free( _it);

		}
		else
		{
			g_message("%s->count:%d", __func__,*count);
		}
	}
	else
	{
		g_message("%s:ipc error", __func__);

	}

	return _items;
}		/* -----  end of function mh_dev_search_name  ----- */
