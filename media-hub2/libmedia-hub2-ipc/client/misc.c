/*
 * =====================================================================================
 *
 *       Filename:  misc.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/22/2015 11:48:45 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_filter
 *  Description:  
 * =====================================================================================
 */
#include <mh_api.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif

extern Mediahub2Dbus * dbusClient;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_filter
 *  Description:  
 * =====================================================================================
 */
static MHResult _set_filter( MHMiscFilterType type, const char * filter, va_list varArgs)
{
	MHResult _res	=	MH_OK;
	void * _param;
	MHMiscFilterType _type;
	char * _filter;

	GVariant * _data	= 	NULL;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(us)" ));

	g_variant_builder_add( _builder,"(us)", type, filter);

	while(( _param = va_arg(varArgs, char *)) != NULL)
	{
		_type	=	( MHMiscFilterType )_param;
		_filter	=	(char *)va_arg( varArgs, char *);

		g_variant_builder_add( _builder, "(us)", _type, _filter);
	
	}

	_data	=	g_variant_builder_end( _builder );

	g_variant_builder_unref( _builder );

	if(mediahub2_dbus_call_misc_set_filter_sync( dbusClient, _data, (gint *)&_res, NULL,NULL))
	{

	}
	else
	{
		_res	=	MH_IPC_ERROR;
	}
	return _res;

}		/* -----  end of static function _set_filter  ----- */
MHResult mh_misc_set_filter( MHMiscFilterType type, const char * filter, ... )
{
	MHResult _res	=	MH_OK;
	va_list _varArgs;

	va_start( _varArgs, filter);

	_res	=	_set_filter( type, filter, _varArgs);

	va_end( _varArgs );

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
	if( !mediahub2_dbus_call_misc_reset_sync( dbusClient, (gint *)&_res, NULL, NULL ))
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
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
	if( !mediahub2_dbus_call_misc_db_restore_sync( dbusClient, (gint *)&_res, NULL, NULL ))
	{
		_res	=	MH_IPC_ERROR;
		g_message("%s:ipc error\n",__func__);
	}
	return _res;
}		/* -----  end of function mh_misc_db_restore  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_db_delete_by_serial_number
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_db_delete_by_serial_number(const char * serialNum)//del db by deviceId
{
	MHResult _res	=	MH_OK;
	if( !mediahub2_dbus_call_misc_db_delete_by_serial_number_sync( dbusClient, serialNum, NULL, NULL ))
	{
		_res	=	MH_IPC_ERROR;
		g_message("%s:ipc error\n",__func__);
	}
	return _res;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_file_get_tag
 *  Description:  
 * =====================================================================================
 */
MHTagInfo * mh_file_get_tag(char * file_path)
{
	MHTagInfo * _res	=	NULL;
	GVariant * _tag;
	char * _title, *_album, *_artist, *_genre;
	int _duration, _year, _track;
	if( !mediahub2_dbus_call_file_get_tag_sync( dbusClient, file_path, &_tag, NULL,NULL ))
	{
		g_message("%s:ipc error\n",__func__);
	}
	else
	{
		_res	=	(MHTagInfo *)g_new0(MHTagInfo, 1);
		g_variant_get( _tag, "(ssssuuu)",&_title, &_album, &_artist, &_genre, &_duration, &_year, &_track);
		_res->title	=	g_strdup(_title);
		_res->album	=	g_strdup(_album);
		_res->artist	=	g_strdup( _artist);
		_res->genre	=	g_strdup( _genre);
		_res->duration	=	_duration;
		_res->year	=	_year;
		_res->track	=	_track;
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
	MHResult _res	=	MH_OK;

	if( !mediahub2_dbus_call_misc_set_iap_device_mode_sync( dbusClient, (guint)mode, (gint *)&_res, NULL, NULL ))
	{
		_res	=	MH_IPC_ERROR;
		g_message("%s:ipc error\n",__func__);
	}

	return _res;
}		/* -----  end of function mh_misc_set_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_bluetoothids()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_bluetoothids( const char * ids )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	
	mediahub2_dbus_call_misc_set_bluetoothids_sync( dbusClient, ids, (gint *)&_res, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "mh_misc_set_bluetoothids falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_misc_set_bluetoothids()  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_save_pb()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_save_pb( MHPb * pb )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	
#ifdef __x86_64__
	mediahub2_dbus_call_misc_save_pb_sync( dbusClient, ( guint64 )pb, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_misc_save_pb_sync( dbusClient, ( guint )pb, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_misc_save_pb falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_misc_save_pb()  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_carplay_init_modes
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_carplay_init_modes( const uint8_t * modes )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;

	GVariant * _var;
	
	_var	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, modes, modes[0], sizeof( uint8_t ));

	_var	=	g_variant_new_variant( _var );

	mediahub2_dbus_call_misc_carplay_init_modes_sync( dbusClient, _var, (gint *)&_res, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "mh_misc_carplay_init_modes falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}

	return _res;
}		/* -----  end of function mh_misc_carplay_init_modes  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_misc_set_righthand()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_misc_set_righthand( uint32_t status )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	
	mediahub2_dbus_call_misc_set_righthand_sync( dbusClient, status, (gint *)&_res, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "mh_misc_set_righthand falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_misc_set_righthand()  ----- */
