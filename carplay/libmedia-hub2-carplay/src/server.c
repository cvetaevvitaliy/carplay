/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/26/2015 09:43:39 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <mh_dbus_carplay.h>
#include <mh_carplay.h>

extern GList * engineList;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_test_string
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_test_string( Mediahub2Carplay *object, GDBusMethodInvocation *invocation,
		const gchar *arg_uri)
{
	mediahub2_carplay_complete_test_string( object, invocation, TRUE );

	return TRUE;
}		/* -----  end of static function handle_test_string  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_close_connection
 *  Description:  
 * =====================================================================================
 */
static void on_close_connection( GDBusConnection * connection, gboolean remote_peer_vanished,
		GError * error, gpointer user_data )
{
	g_message( "The CarPlay connection [ %p ] was closed", connection );
}		/* -----  end of static function on_close_connection  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  load_carplay_engine
 *  Description:  
 * =====================================================================================
 */
static void load_carplay_engine( GDBusServer *server, GDBusConnection *connection,
		gpointer user_data )
{
	GError * _error	=	NULL;
	Mediahub2Carplay * _object;
	Mediahub2CarplayIface * _iface;

	_object	=	mediahub2_carplay_skeleton_new();

	_iface	=	MEDIAHUB2_CARPLAY_GET_IFACE( _object );

	_iface->handle_test_string	=	handle_test_string;

	g_dbus_interface_skeleton_export( G_DBUS_INTERFACE_SKELETON( _object ),
			connection, "/carplay", &_error );

	g_signal_connect( connection, "closed", G_CALLBACK( on_close_connection ), _object );

	if( _error != NULL )
	{
		g_warning( "Create new CarPlay connection failed: [ %s ]", _error->message );

		g_error_free( _error );
	}
	else
	{
		g_message( "A new CarPlay connection [ %p ] was created", connection );
	}}		/* -----  end of static function load_carplay_engine  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_ipc_start_carplay_engine
 *  Description:  
 * =====================================================================================
 */
void mh_ipc_start_carplay_engine( MHIPCServer * server )
{
	engineList	=	g_list_append( engineList, load_carplay_engine );

	g_message( "Media-Hub v2.0 CarPlay Engine Started" );
}		/* -----  end of function mh_ipc_start_carplay_engine  ----- */
