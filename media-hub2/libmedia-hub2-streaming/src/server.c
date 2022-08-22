/*
 * =====================================================================================
 *
 *       Filename:  streaming.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2015 05:00:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdint.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-media-factory-uri.h>

#include <mh_dbus_streaming.h>
#include <mh_streaming.h>

static GMainContext * context;
static GMainLoop * mainloop;
static GstRTSPMountPoints * mounts;

static GHashTable * uriHash;

extern GList * engineList;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  streaming_task
 *  Description:  
 * =====================================================================================
 */
static gpointer streaming_task( gpointer user_data )
{
	g_main_loop_run( mainloop );

	g_main_loop_unref( mainloop );
	g_main_context_unref( context );

	return NULL;
}		/* -----  end of static function streaming_task  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_add_media_mapping
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_add_media_mapping( Mediahub2Streaming * object, GDBusMethodInvocation * invocation,
		const gchar * arg_uri, guint arg_shared )
{
	GstRTSPMediaFactoryURI *factory;
	gchar * _uri;
	gchar * _tmp_uri;
	int * _count;
	gchar * _key;
	gboolean _result	=	FALSE;

	_uri	=	g_filename_to_uri( arg_uri, NULL, NULL );

	_tmp_uri = g_strdup( _uri + 7);  	

	if( _uri != NULL )
	{
		if( ( _count = g_hash_table_lookup( uriHash, _uri )) != NULL )
		{
			* _count +=	1;
		}
		else
		{
			factory	=	gst_rtsp_media_factory_uri_new(); 

			gst_rtsp_media_factory_uri_set_uri( factory, _uri );
			if( arg_shared == TRUE)
			{
				gst_rtsp_media_factory_set_shared ( GST_RTSP_MEDIA_FACTORY (factory), TRUE);	
			}
			gst_rtsp_mount_points_add_factory( mounts, _tmp_uri, GST_RTSP_MEDIA_FACTORY( factory ) );

			_key	=	g_strdup( _uri );
			_count	=	g_new0( gint, 1 );

			* _count	=	1;

			g_hash_table_insert( uriHash, _key, _count );

			g_message( "streaming server added mounts: %s", _uri );
		}

		g_free( _tmp_uri );
		g_free( _uri );

		_result	=	TRUE;
	}
	else
	{
		g_message( "streaming server added mounts: %s failed", arg_uri );
	}

	mediahub2_streaming_complete_add_media_mapping( object, invocation, _result );

	return TRUE;
}		/* -----  end of static function handle_add_media_mapping  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_remove_media_mapping
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_remove_media_mapping( Mediahub2Streaming * object, GDBusMethodInvocation * invocation,
		const gchar * arg_uri )
{
	gboolean _result	=	FALSE;
	int * _count;
	gchar * _uri;

	_uri	=	g_filename_to_uri( arg_uri, NULL, NULL );

	if( _uri != NULL )
	{
		if( ( _count = g_hash_table_lookup( uriHash, _uri )) != NULL )
		{
			* _count -=	1;
		}

		if( * _count == 0 )
		{
			gst_rtsp_mount_points_remove_factory( mounts, arg_uri );

			g_hash_table_remove( uriHash, _uri );
		}

		g_free( _uri );

		_result	=	TRUE;
	}

	mediahub2_streaming_complete_remove_media_mapping( object, invocation, _result );

	g_message( "streaming server removed mounts: %s", arg_uri );

	return TRUE;
}		/* -----  end of static function handle_remove_media_mapping  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_close_connection
 *  Description:  
 * =====================================================================================
 */
static void on_close_connection( GDBusConnection * connection, gboolean remote_peer_vanished,
		GError * error, gpointer user_data )
{
	g_message( "The streaming connection [ %p ] was closed", connection );
}		/* -----  end of static function on_close_connection  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  load_streaming_engine
 *  Description:  
 * =====================================================================================
 */
static void load_streaming_engine( GDBusServer *server, GDBusConnection *connection,
		gpointer user_data )
{
	GError * _error	=	NULL;
	Mediahub2Streaming * _object;
	Mediahub2StreamingIface * _iface;

	_object	=	mediahub2_streaming_skeleton_new();

	_iface	=	MEDIAHUB2_STREAMING_GET_IFACE( _object );

	_iface->handle_add_media_mapping	=	handle_add_media_mapping;
	_iface->handle_remove_media_mapping	=	handle_remove_media_mapping;

	g_dbus_interface_skeleton_export( G_DBUS_INTERFACE_SKELETON( _object ),
			connection, "/streaming", &_error );

//	g_signal_connect( connection, "closed", G_CALLBACK( on_close_connection ), _object );

	if( _error != NULL )
	{
		g_warning( "Create new streaming connection failed: [ %s ]", _error->message );

		g_error_free( _error );
	}
	else
	{
		g_message( "A new streaming connection [ %p ] was created", connection );
	}
}		/* -----  end of static function load_streaming_engine  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_streaming_start
 *  Description:  
 * =====================================================================================
 */
void mh_ipc_start_streaming_engine( MHIPCServer * server )
{
	GstRTSPServer * _server;

	uriHash	=	g_hash_table_new_full( g_str_hash, g_str_equal, g_free, g_free );

	context	=	g_main_context_new();
	mainloop	=	g_main_loop_new( context, FALSE );

	_server	=	gst_rtsp_server_new();

	g_object_set( _server, "address", "::0", NULL);

	mounts	=	gst_rtsp_server_get_mount_points( _server );

	gst_rtsp_server_attach( _server, context );

	engineList	=	g_list_append( engineList, load_streaming_engine );

	g_thread_new( "streaming", streaming_task, NULL );

	g_message( "Media-Hub v2.0 Steaming Engine Started" );
}		/* -----  end of function mh_streaming_start  ----- */
