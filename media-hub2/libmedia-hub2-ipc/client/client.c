/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/06/2014 11:16:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "dns_sd.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib.h>
#include <dlfcn.h>
#include <mh_client.h>
//#ifdef __x86_64__
//	#include<mh_dbus_x64.h>
//#else
	#include <mh_dbus.h>
//#endif
Mediahub2Dbus * dbusClient;
IPCConnect * dbusConnect;

static void * dnsHandle;

static DNSServiceErrorType  (*_DNSServiceBrowse)(DNSServiceRef *sdRef, DNSServiceFlags flags,
    uint32_t  interfaceIndex, const char *regtype, const char *domain, DNSServiceBrowseReply callBack, void *context);

static DNSServiceErrorType (* _DNSServiceProcessResult)(DNSServiceRef sdRef);

static void (* _DNSServiceRefDeallocate)(DNSServiceRef sdRef);

static DNSServiceErrorType (* _DNSServiceResolve)(DNSServiceRef *sdRef, DNSServiceFlags flags,uint32_t interfaceIndex,
		const char *name, const char *regtype, const char *domain, DNSServiceResolveReply callBack, void *context);

static DNSServiceErrorType (*_DNSServiceGetAddrInfo)(DNSServiceRef *sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceProtocol protocol,
		const char *hostname, DNSServiceGetAddrInfoReply callBack, void *context);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getaddr_reply
 *  Description:  
 * =====================================================================================
 */
static void getaddr_reply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, 
		DNSServiceErrorType errorCode, const char * hostname, const struct sockaddr * address,
		uint32_t ttl, void * context )
{

	IPCConnect * _conn	=	(IPCConnect *)context;	

	char _addr[ INET6_ADDRSTRLEN];
	struct sockaddr_in6 * _sockaddr6;
	unsigned char * _p;

	if( address && address->sa_family == AF_INET6 )
	{

		_sockaddr6	=	( struct sockaddr_in6 * )address;

		_p	=	( unsigned char * )&(_sockaddr6->sin6_addr);

		if( NULL == inet_ntop( AF_INET6, _p, _addr, INET6_ADDRSTRLEN))
		{
			perror( "inet_ntop\n");
		}

		_conn->connect_name = g_strdup(_addr);

		_conn->connect_scope_id	=	_sockaddr6->sin6_scope_id;

	}

}		/* -----  end of static function getaddr_reply  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getaddr_reply_ipv4
 *  Description:  
 * =====================================================================================
 */
static void getaddr_reply_ipv4( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, 
		DNSServiceErrorType errorCode, const char * hostname, const struct sockaddr * address,
		uint32_t ttl, void * context )
{
	
	IPCConnect * _conn	=	(IPCConnect *)context;	

	char _addr[INET_ADDRSTRLEN];
	struct sockaddr_in * _sockaddr4;
	unsigned char * _p;

	if( address && address->sa_family == AF_INET )
	{
		_sockaddr4	=	( struct sockaddr_in * )address;
		_p	=	( unsigned char * )&(_sockaddr4->sin_addr);
		if( NULL == inet_ntop( AF_INET, _p, _addr, INET_ADDRSTRLEN))
		{
			perror( "inet_ntop\n");
		}

		_conn->connect_name	= g_strdup( _addr );
	}

}		/* -----  end of static function getaddr_reply_ipv4  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  resolve_reply
 *  Description:  
 * =====================================================================================
 */
static void resolve_reply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, 
		DNSServiceErrorType errorCode, const char * fullname, const char * hosttarget,
		uint16_t port, uint16_t textLen, const unsigned char * txtRecord, void * context )
{

	DNSServiceRef _gRef;

	IPCConnect * _conn	=	(IPCConnect *)context;


	_conn->connect_port	=	htons(port);

	printf("DNSServiceGetAddrInfo:%d\n",_DNSServiceGetAddrInfo( &_gRef, 0, interfaceIndex, kDNSServiceProtocol_IPv6, hosttarget, getaddr_reply, context ));

	_DNSServiceProcessResult( _gRef );

	_DNSServiceRefDeallocate( _gRef );


}		/* -----  end of static function resolve_reply  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  browse_reply
 *  Description:  
 * =====================================================================================
 */
static void browse_reply( DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, 
		DNSServiceErrorType errorCode, const char * serviceName, const char * regtype,
		const char * replyDomain, void * context )
{
	DNSServiceRef _resRef;

	g_message( "%d", _DNSServiceResolve( &_resRef, 0, interfaceIndex, serviceName,
				regtype, replyDomain, resolve_reply, context));
	_DNSServiceProcessResult( _resRef );

	_DNSServiceRefDeallocate( _resRef );

}		/* -----  end of static function browse_reply  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  client_func
 *  Description:  
 * =====================================================================================
 */
static gpointer client_func( gpointer user_data )
{
	GMainLoop * _loop	=	NULL;

	_loop	=	g_main_loop_new( NULL, FALSE );

	g_main_loop_run( _loop );

	g_main_loop_unref( _loop );

	return NULL;
}		/* -----  end of static function client_func  ----- */

MHIPCConnection *  mh_ipc_connection_create()
{
	IPCConnect * _conn	= (IPCConnect *)g_malloc0( sizeof( IPCConnect));

	GError * _error	=	NULL;

	gchar * _connect;

	const char * _streaming_enble = getenv( "MH_STREAMING_ENABLED");

	if( _streaming_enble == NULL)
	{
		_conn->dbus_conn	=	g_dbus_connection_new_for_address_sync( "unix:abstract=mediahub2",
				G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &_error );

		g_message("unix:abstract=mediahub2");
	
	}
	else
	{
		dnsHandle	=	dlopen("libdns_sd.so", RTLD_LAZY);

		DNSServiceRef _sdRef;
		_DNSServiceBrowse	=	dlsym( dnsHandle, "DNSServiceBrowse");

		_DNSServiceProcessResult	=	dlsym( dnsHandle, "DNSServiceProcessResult");

		_DNSServiceRefDeallocate	=	dlsym( dnsHandle, "DNSServiceRefDeallocate");

		_DNSServiceResolve	=	dlsym( dnsHandle, "DNSServiceResolve");

		_DNSServiceGetAddrInfo	=	dlsym( dnsHandle, "DNSServiceGetAddrInfo");

		g_message( "%d", _DNSServiceBrowse( &_sdRef, 0, 0, "_Media-Hub-v2._tcp", NULL, browse_reply, _conn ));

		_DNSServiceProcessResult( _sdRef );

		_DNSServiceRefDeallocate( _sdRef );

		_connect = g_strdup_printf("tcp:host=%s%%25%d,port=%d",_conn->connect_name,_conn->connect_scope_id, _conn->connect_port );
        printf("tcp:host=%s%%25%d,port=%d",_conn->connect_name,_conn->connect_scope_id, _conn->connect_port);

		g_message("\n\nconnect:%s\n\n", _connect);

		_conn->dbus_conn	=	g_dbus_connection_new_for_address_sync( _connect,
				G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &_error );

		g_free( _connect);

		dlclose(dnsHandle);
	
	}
	if( _error )
	{
		g_message( "Loading Media-Hub v2.0 IPC Client Library failed:[ %s ]", _error->message );

		g_error_free( _error );

		return NULL;
	}
	dbusConnect	= _conn;
	return ( MHIPCConnection * )_conn;

}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_close_connection
 *  Description:  
 * =====================================================================================
 */
static void on_close_connection( GDBusConnection * connection, gboolean remote_peer_vanished,
		GError * error, gpointer user_data )
{
	g_message("on_close_connection");

	IPCConnect * _conn	=	(IPCConnect *)user_data;

	GList * _list	=	g_list_first( _conn->handle_id);

	while( _list != NULL)
	{
		g_signal_handler_disconnect( _conn->dbus_conn, *(uint64_t *)(_list->data));

		g_free(_list->data);

		_list	=	_list->next;
	}

	g_list_free( _conn->handle_id);

	g_free( _conn );

}		/* -----  end of static function on_close_connection  ----- */



MHResult mh_ipc_media_client_init( MHIPCConnection * conn )
{
	g_return_val_if_fail( conn != NULL, MH_INVALID_PARAM); 	

	MHResult _res	=	MH_OK;

	g_message("%s", __func__);

	IPCConnect * _conn	=	(IPCConnect *)conn;

	GDBusConnection * _dbus_conn	=	_conn->dbus_conn;

	GError * _error	=	NULL;

	dbusClient	=	mediahub2_dbus_proxy_new_sync( _dbus_conn, G_DBUS_PROXY_FLAGS_NONE,
			NULL, "/media", NULL, &_error );

	if( _error )
	{
		g_message( "Loading Media-Hub v2.0 Media Client Library failed:[ %s ]", _error->message );

		g_error_free( _error );
		
		_res	=	MH_IPC_ERROR;
	}
	else
	{
		g_thread_new( "ipc-client", client_func, NULL );
		g_signal_connect( _dbus_conn, "closed", G_CALLBACK( on_close_connection), conn);
		g_message( "Media-Hub v2.0 Media Client Library has been loaded successfully" );
	}
	return _res;
}
