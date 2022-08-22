/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/14/2015 11:40:56 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <mh_dbus_streaming.h>
#include <gst/gst.h>
#include <mh_client.h>
Mediahub2Streaming * streamingClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_add_media_mapping
 *  Description:  
 * =====================================================================================
 */
bool mh_streaming_add_media_mapping( const char * uri,gboolean shared)
{
	g_return_val_if_fail( uri != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;
	gboolean _result	=	FALSE;

	mediahub2_streaming_call_add_media_mapping_sync( streamingClient, uri, (guint)shared,&_result, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__, _error->message );

		g_error_free( _error );

		return MH_IPC_ERROR;
	}

	return _result;
}		/* -----  end of function mh_streaming_add_media_mapping  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_remove_media_mapping
 *  Description:  
 * =====================================================================================
 */
bool mh_streaming_remove_media_mapping( const char * uri )
{
	g_return_val_if_fail( uri != NULL, MH_INVALID_PARAM );

	GError * _error	=	NULL;
	gboolean _result;

	mediahub2_streaming_call_remove_media_mapping_sync( streamingClient, uri, &_result, NULL, &_error );

	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__, _error->message );

		g_error_free( _error );

		return MH_IPC_ERROR;
	}

	return _result;
}		/* -----  end of function mh_streaming_remove_media_mapping  ----- */

void mh_ipc_streaming_client_init( MHIPCConnection * conn )
{
	IPCConnect * _conn	=	( IPCConnect *)conn;
	GDBusConnection * _dbus_conn	=	_conn->dbus_conn;
	GError * _error	=	NULL;

	gst_init( NULL, NULL );

	streamingClient	=	mediahub2_streaming_proxy_new_sync( _dbus_conn, G_DBUS_PROXY_FLAGS_NONE,
			NULL, "/streaming", NULL, &_error );

	if( _error )
	{
		g_message( "Loading Media-Hub v2.0 Streaming Client Library failed:[ %s ]", _error->message );

		g_error_free( _error );

		return;
	}

	g_message( "Media-Hub v2.0 Streaming Client Library has been loaded successfully" );
}
