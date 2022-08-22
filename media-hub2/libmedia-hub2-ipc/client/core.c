/*
 * =====================================================================================
 *
 *       Filename:  core.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/06/2014 11:07:17 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <mh_client.h>

#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif

extern Mediahub2Dbus * dbusClient;
extern IPCConnect * dbusConnect;



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_core_register_events_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_core_register_events_listener( MHEventsListener * listener )
{
	g_return_val_if_fail( listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;
	uint64_t * _handleId	=	g_new0( uint64_t, 1);

	* _handleId	=	g_signal_connect( dbusClient, "core_events", G_CALLBACK( listener->callback ), 
			listener->user_data );

	mediahub2_dbus_call_core_register_events_listener_sync( dbusClient, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "Register events listener falied: [ %s ]", _error->message );

		g_error_free( _error );

		return MH_IPC_ERROR;
	}

	dbusConnect->handle_id = g_list_append( dbusConnect->handle_id, _handleId);


	return 0;
}		/* -----  end of function mh_core_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_core_register_devices_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_core_register_devices_listener( MHDevicesListener * listener )
{
	g_return_val_if_fail( listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;

	uint64_t * _handleId	=	g_new0( uint64_t, 1);

	* _handleId = g_signal_connect( dbusClient, "core_devices", G_CALLBACK( listener->callback ), 
			listener->user_data );

	mediahub2_dbus_call_core_register_devices_listener_sync( dbusClient, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "Register devices listener falied: [ %s ]", _error->message );

		g_error_free( _error );

		return MH_IPC_ERROR;
	}
	dbusConnect->handle_id = g_list_append( dbusConnect->handle_id, _handleId);

	return 0;
}		/* -----  end of function mh_core_register_devices_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_core_start
 *  Description:  
 * =====================================================================================
 */
MHResult mh_core_start()
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	
	mediahub2_dbus_call_core_start_sync( dbusClient, (gint *)&_res, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "mh_core_start falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_core_start  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_core_stop()
 *  Description:  
 * =====================================================================================
 */
MHResult mh_core_stop()
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	
	mediahub2_dbus_call_core_stop_sync( dbusClient, (gint *)&_res, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "mh_core_stop falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_core_stop()  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_core_find_dev
 *  Description:  
 * =====================================================================================
 */
MHResult mh_core_find_dev( MHDevParam * devParam )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	GVariant * _param;
	GVariant * _param_p;
	_param	=	g_variant_new( "(usu)", devParam->type,
			devParam->mac_addr ? devParam->mac_addr :"", devParam->connect_status );

	_param_p   =  g_variant_new( "v", _param );

	mediahub2_dbus_call_core_find_dev_sync( dbusClient, _param_p, (gint *)&_res, NULL, &_error );

	g_variant_unref( _param );
	g_variant_unref( _param_p );

	if( _error != NULL )
	{
		g_warning( "mh_core_find_dev falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_core_find_dev  ----- */

