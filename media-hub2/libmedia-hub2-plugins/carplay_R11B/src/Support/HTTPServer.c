/*
	File:    	HTTPServer.c
	Package: 	Apple CarPlay Communication Plug-in.
	Abstract: 	n/a 
	Version: 	210.81
	
	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple Inc. ("Apple"), in your
	capacity as a current, and in good standing, Licensee in the MFi Licensing Program. Use of this
	Apple software is governed by and subject to the terms and conditions of your MFi License,
	including, but not limited to, the restrictions specified in the provision entitled ”Public 
	Software”, and is further subject to your agreement to the following additional terms, and your 
	agreement that the use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these additional terms. If you do not agree with these additional terms,
	please do not use, install, modify or redistribute this Apple software.
	
	Subject to all of these terms and in consideration of your agreement to abide by them, Apple grants
	you, for as long as you are a current and in good-standing MFi Licensee, a personal, non-exclusive 
	license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use, 
	reproduce, and modify the Apple Software in source form, and to use, reproduce, modify, and 
	redistribute the Apple Software, with or without modifications, in binary form. While you may not 
	redistribute the Apple Software in source form, should you redistribute the Apple Software in binary
	form, you must retain this notice and the following text and disclaimers in all such redistributions
	of the Apple Software. Neither the name, trademarks, service marks, or logos of Apple Inc. may be
	used to endorse or promote products derived from the Apple Software without specific prior written
	permission from Apple. Except as expressly stated in this notice, no other rights or licenses, 
	express or implied, are granted by Apple herein, including but not limited to any patent rights that
	may be infringed by your derivative works or by other works in which the Apple Software may be 
	incorporated.  
	
	Unless you explicitly state otherwise, if you provide any ideas, suggestions, recommendations, bug 
	fixes or enhancements to Apple in connection with this software (“Feedback”), you hereby grant to
	Apple a non-exclusive, fully paid-up, perpetual, irrevocable, worldwide license to make, use, 
	reproduce, incorporate, modify, display, perform, sell, make or have made derivative works of,
	distribute (directly or indirectly) and sublicense, such Feedback in connection with Apple products 
	and services. Providing this Feedback is voluntary, but if you do provide Feedback to Apple, you 
	acknowledge and agree that Apple may exercise the license granted above without the payment of 
	royalties or further consideration to Participant.
	
	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
	IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
	AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR
	IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION 
	AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
	(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2011-2013 Apple Inc. All Rights Reserved.
	
	To Do:
	
	- Add support for chunked encodings.
	- Flag to close the connection after a send has completed (Connection: close in response?).
	- Timeout support for reading and writing over the network and for connections.
	- Think about delegate way to pause writing the body after it has already started it (e.g. wait for user).
	- Allow delegate to have request body read in chunks into delegate-provided buffer(s) for DAAP PUTs.
	- Figure out a way to handle readability while in a writing state. For example, if we get a peer close while waiting
	  for the delegate to process a request, the socket should be closed. Another case is if we support pipelined 
	  requests since we'll then have constant readability hogging the CPU while we're trying to write.
	- Read headers then read body separately via user buffer/callback (RTSP binary data for AirTunes or DAAP PUT).
	- Wait indefinitely before sending response (DAAP update response only occurs on a database revision change).
	- Send response headers then send response body in chunks via callback (DAAP item data or big content blocks).
*/

#include "HTTPServer.h"

#include <errno.h>
#if( TARGET_OS_POSIX )
	#include <netinet/tcp.h>
#endif

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFUtils.h"
#include "HTTPMessage.h"
#include "HTTPNetUtils.h"
#include "HTTPUtils.h"
#include "NetUtils.h"
#include "RandomNumberUtils.h"

#include CF_RUNTIME_HEADER
#include LIBDISPATCH_HEADER

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static void		_HTTPServerGetTypeID( void *inContext );
static void		_HTTPServerFinalize( CFTypeRef inCF );
static OSStatus	_HTTPServerStart( HTTPServerRef me );
static void		_HTTPServerStop( HTTPServerRef me );
static void		_HTTPServerAcceptConnection( void *inContext );
static void		_HTTPServerCloseConnection( HTTPConnectionRef inCnx );
static void		_HTTPServerListenerCanceled( void *inContext );

static void		_HTTPConnectionGetTypeID( void *inContext );
static void		_HTTPConnectionFinalize( CFTypeRef inCF );
static void		_HTTPConnectionReadHandler( void *inContext );
static void		_HTTPConnectionWriteHandler( void *inContext );
static void		_HTTPConnectionCancelHandler( void *inContext );
static void		_HTTPConnectionRunStateMachine( HTTPConnectionRef inCnx );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static dispatch_once_t			gHTTPServerInitOnce = 0;
static CFTypeID					gHTTPServerTypeID = _kCFRuntimeNotATypeID;
static const CFRuntimeClass		kHTTPServerClass = 
{
	0,							// version
	"HTTPServer",				// className
	NULL,						// init
	NULL,						// copy
	_HTTPServerFinalize,		// finalize
	NULL,						// equal -- NULL means pointer equality.
	NULL,						// hash  -- NULL means pointer hash.
	NULL,						// copyFormattingDesc
	NULL,						// copyDebugDesc
	NULL,						// reclaim
	NULL						// refcount
};

static dispatch_once_t			gHTTPConnectionInitOnce = 0;
static CFTypeID					gHTTPConnectionTypeID = _kCFRuntimeNotATypeID;
static const CFRuntimeClass		kHTTPConnectionClass = 
{
	0,							// version
	"HTTPConnection",			// className
	NULL,						// init
	NULL,						// copy
	_HTTPConnectionFinalize,	// finalize
	NULL,						// equal -- NULL means pointer equality.
	NULL,						// hash  -- NULL means pointer hash.
	NULL,						// copyFormattingDesc
	NULL,						// copyDebugDesc
	NULL,						// reclaim
	NULL						// refcount
};

ulog_define( HTTPServerCore, kLogLevelNotice, kLogFlags_Default, "HTTPServer",  NULL );

//===========================================================================================================================
//	HTTPServerGetTypeID
//===========================================================================================================================

CFTypeID	HTTPServerGetTypeID( void )
{
	dispatch_once_f( &gHTTPServerInitOnce, NULL, _HTTPServerGetTypeID );
	return( gHTTPServerTypeID );
}

static void _HTTPServerGetTypeID( void *inContext )
{
	(void) inContext;
	
	gHTTPServerTypeID = _CFRuntimeRegisterClass( &kHTTPServerClass );
	check( gHTTPServerTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	HTTPServerCreate
//===========================================================================================================================

OSStatus	HTTPServerCreate( HTTPServerRef *outServer, const HTTPServerDelegate *inDelegate )
{
	OSStatus			err;
	HTTPServerRef		me;
	size_t				extraLen;
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (HTTPServerRef) _CFRuntimeCreateInstance( NULL, HTTPServerGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	HTTPServerSetDispatchQueue( me, NULL ); // Default to the main queue.
	me->ucat		= &log_category_from_name( HTTPServerCore );
	me->delegate	= *inDelegate;
	
	if( me->delegate.initializeServer_f )
	{
		err = me->delegate.initializeServer_f( me );
		require_noerr( err, exit );
	}
	
	*outServer = me;
	me = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( me );
	return( err );
}

//===========================================================================================================================
//	_HTTPServerFinalize
//===========================================================================================================================

static void	_HTTPServerFinalize( CFTypeRef inCF )
{
	HTTPServerRef const		me = (HTTPServerRef) inCF;
	
	if( me->delegate.finalizeServer_f ) me->delegate.finalizeServer_f( me );
	dispatch_forget( &me->queue );
	ForgetMem( &me->password );
	ForgetMem( &me->realm );
}

//===========================================================================================================================
//	HTTPServerSetDispatchQueue
//===========================================================================================================================

void	HTTPServerSetDispatchQueue( HTTPServerRef me, dispatch_queue_t inQueue )
{
	ReplaceDispatchQueue( &me->queue, inQueue );
}

//===========================================================================================================================
//	_HTTPServerControl
//===========================================================================================================================

OSStatus
	_HTTPServerControl( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inCommand, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inParams )
{
	HTTPServerRef const		me = (HTTPServerRef) inObject;
	OSStatus				err;
	
	if( 0 ) {}
	
	// StartServer
	
	else if( CFEqual( inCommand, kHTTPServerCommand_StartServer ) )
	{
		err = _HTTPServerStart( me );
		require_noerr( err, exit );
	}
	
	// StopServer
	
	else if( CFEqual( inCommand, kHTTPServerCommand_StopServer ) )
	{
		_HTTPServerStop( me );
	}
	
	// Other
	
	else
	{
		if( me->delegate.control_f ) return( me->delegate.control_f( me, inFlags, inCommand, inQualifier, inParams ) );
		dlogassert( "Unsupported command %@", inCommand );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_HTTPServerCopyProperty
//===========================================================================================================================

CFTypeRef
	_HTTPServerCopyProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr )
{
	HTTPServerRef const		me		= (HTTPServerRef) inObject;
	CFTypeRef				value	= NULL;
	OSStatus				err;
	
	if( 0 ) {}
	
	// Other
	
	else
	{
		if( me->delegate.copyProperty_f ) return( me->delegate.copyProperty_f( me, inFlags, inProperty, inQualifier, outErr ) );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_HTTPServerSetProperty
//===========================================================================================================================

OSStatus
	_HTTPServerSetProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue )
{
	HTTPServerRef const		me = (HTTPServerRef) inObject;
	OSStatus				err;
	char *					utf8;
	
	if( 0 ) {}
	
	// P2P
	
	else if( CFEqual( inProperty, kHTTPServerProperty_P2P ) )
	{
		me->allowP2P = CFGetBoolean( inValue, NULL );
		if( me->listenerV4 && IsValidSocket( me->listenerV4->sock ) ) SocketSetP2P( me->listenerV4->sock, me->allowP2P );
		if( me->listenerV6 && IsValidSocket( me->listenerV6->sock ) ) SocketSetP2P( me->listenerV6->sock, me->allowP2P );
	}
	
	// Password
	
	else if( CFEqual( inProperty, kHTTPServerProperty_Password ) )
	{
		require_action( !inValue || CFIsType( inValue, CFString ), exit, err = kTypeErr );
		
		utf8 = NULL;
		if( inValue && ( CFStringGetLength( (CFStringRef) inValue ) > 0 ) )
		{
			err = CFStringCopyUTF8CString( (CFStringRef) inValue, &utf8 );
			require_noerr( err, exit );
		}
		if( me->password ) free( me->password );
		me->password = utf8;
	}
	
	// Realm
	
	else if( CFEqual( inProperty, kHTTPServerProperty_Realm ) )
	{
		require_action( !inValue || CFIsType( inValue, CFString ), exit, err = kTypeErr );
		
		utf8 = NULL;
		if( inValue && ( CFStringGetLength( (CFStringRef) inValue ) > 0 ) )
		{
			err = CFStringCopyUTF8CString( (CFStringRef) inValue, &utf8 );
			require_noerr( err, exit );
		}
		if( me->realm ) free( me->realm );
		me->realm = utf8;
	}
	
	// Other
	
	else
	{
		if( me->delegate.setProperty_f ) return( me->delegate.setProperty_f( me, inFlags, inProperty, inQualifier, inValue ) );
		dlogassert( "Set of unsupported property %@", inProperty );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_HTTPServerStart
//===========================================================================================================================

static OSStatus	_HTTPServerStart( HTTPServerRef me )
{
	OSStatus					err;
	SocketRef					sockV4 = kInvalidSocketRef;
	SocketRef					sockV6 = kInvalidSocketRef;
	HTTPListenerContext *		listener;
	dispatch_source_t			source;
	
	err = RandomBytes( me->timedNonceKey, sizeof( me->timedNonceKey ) );
	require_noerr( err, exit );
	
	// Set up listener sockets.
	
	err = TCPServerSocketPairOpen( me->listenPort, &me->listeningPort, kSocketBufferSize_DontSet, &sockV4, &sockV6 );
	require_noerr( err, exit );
	if( IsValidSocket( sockV4 ) )
	{
		if( me->allowP2P ) SocketSetP2P( sockV4, true );
		
		listener = (HTTPListenerContext *) calloc( 1, sizeof( *listener ) );
		require_action( listener, exit, err = kNoMemoryErr );
		
		source = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, sockV4, 0, me->queue );
		if( !source ) free( listener );
		require_action( source, exit, err = kUnknownErr );
		
		CFRetain( me );
		listener->source	= source;
		listener->sock		= sockV4;
		listener->server	= me;
		me->listenerV4		= listener;
		dispatch_set_context( source, listener );
		dispatch_source_set_event_handler_f( source, _HTTPServerAcceptConnection );
		dispatch_source_set_cancel_handler_f( source, _HTTPServerListenerCanceled );
		dispatch_resume( source );
		sockV4 = kInvalidSocketRef;
	}
	if( IsValidSocket( sockV6 ) )
	{
		if( me->allowP2P ) SocketSetP2P( sockV6, true );
		
		listener = (HTTPListenerContext *) calloc( 1, sizeof( *listener ) );
		require_action( listener, exit, err = kNoMemoryErr );
		
		source = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, sockV6, 0, me->queue );
		if( !source ) free( listener );
		require_action( source, exit, err = kUnknownErr );
		
		CFRetain( me );
		listener->source	= source;
		listener->sock		= sockV6;
		listener->server	= me;
		me->listenerV6		= listener;
		dispatch_set_context( source, listener );
		dispatch_source_set_event_handler_f( source, _HTTPServerAcceptConnection );
		dispatch_source_set_cancel_handler_f( source, _HTTPServerListenerCanceled );
		dispatch_resume( source );
		sockV6 = kInvalidSocketRef;
	}
	
	// Start the delegate.
	
	if( me->delegate.control_f )
	{
		err = me->delegate.control_f( me, 0, kHTTPServerCommand_StartServer, NULL, NULL );
		require_noerr( err, exit );
	}
	
	CFRetain( me );
	me->started = true;
	http_server_ulog( me, kLogLevelInfo, "Listening on port %d\n", me->listeningPort );
	
exit:
	ForgetSocket( &sockV4 );
	ForgetSocket( &sockV6 );
	if( err ) _HTTPServerStop( me );
	return( err );
}

//===========================================================================================================================
//	_HTTPServerStop
//===========================================================================================================================

static void	_HTTPServerStop( HTTPServerRef me )
{
	if( me->listenerV4 )
	{
		dispatch_source_forget( &me->listenerV4->source );
		me->listenerV4 = NULL;
	}
	if( me->listenerV6 )
	{
		dispatch_source_forget( &me->listenerV6->source );
		me->listenerV6 = NULL;
	}
	while( me->connections )
	{
		_HTTPServerCloseConnection( me->connections );
	}
	if( me->started )
	{
		if( me->delegate.control_f ) me->delegate.control_f( me, 0, kHTTPServerCommand_StopServer, NULL, NULL );
		me->started = false;
		CFRelease( me );
	}
}

//===========================================================================================================================
//	_HTTPServerAcceptConnection
//===========================================================================================================================

static void	_HTTPServerAcceptConnection( void *inContext )
{
	HTTPListenerContext * const		listener	= (HTTPListenerContext *) inContext;
	HTTPServerRef const				server		= listener->server;
	OSStatus						err;
	SocketRef						newSock		= kInvalidSocketRef;
	HTTPConnectionDelegate			delegate;
	HTTPConnectionRef				cnx			= NULL;
	
	newSock = accept( listener->sock, NULL, NULL );
	err = map_socket_creation_errno( newSock );
	require_noerr( err, exit );
	
	HTTPConnectionDelegateInit( &delegate );
	delegate.httpProtocol			= server->delegate.httpProtocol;
	delegate.initializeConnection_f	= server->delegate.initializeConnection_f;
	delegate.finalizeConnection_f	= server->delegate.finalizeConnection_f;
	delegate.closeConnection_f		= _HTTPServerCloseConnection;
	delegate.requiresAuth_f			= server->delegate.requiresAuth_f;
	delegate.handleMessage_f		= server->delegate.handleMessage_f;
	delegate.initResponse_f			= server->delegate.initResponse_f;
	
	err = HTTPConnectionCreate( &cnx, &delegate, newSock, server );
	require_noerr( err, exit );
	newSock = kInvalidSocketRef;
	
	HTTPConnectionSetDispatchQueue( cnx, server->queue );
	
	cnx->next = server->connections;
	server->connections = cnx;
	
	err = HTTPConnectionStart( cnx );
	require_noerr( err, exit );
	
	http_server_ulog( server, kLogLevelInfo, "Accepted connection from %##a to %##a\n", &cnx->peerAddr, &cnx->selfAddr );
	cnx = NULL;
	
exit:
	ForgetSocket( &newSock );
	if( cnx ) _HTTPServerCloseConnection( cnx );
	if( err ) http_server_ulog( server, kLogLevelWarning, "### Accept connection failed: %#m\n", err );
}

//===========================================================================================================================
//	_HTTPServerCloseConnection
//===========================================================================================================================

static void	_HTTPServerCloseConnection( HTTPConnectionRef inCnx )
{
	HTTPServerRef const		server = inCnx->server;
	HTTPConnectionRef *		next;
	HTTPConnectionRef		curr;
	
	for( next = &server->connections; ( curr = *next ) != NULL; next = &curr->next )
	{
		if( curr == inCnx )
		{
			*next = curr->next;
			break;
		}
	}
	HTTPConnectionStop( inCnx );
	if( inCnx->selfAddr.sa.sa_family != AF_UNSPEC )
	{
		http_server_ulog( server, kLogLevelInfo, "Closing  connection from %##a to %##a\n", 
			&inCnx->peerAddr, &inCnx->selfAddr );
	}
	CFRelease( inCnx );
}

//===========================================================================================================================
//	_HTTPServerListenerCanceled
//===========================================================================================================================

static void	_HTTPServerListenerCanceled( void *inContext )
{
	HTTPListenerContext * const		listener = (HTTPListenerContext *) inContext;
	
	ForgetSocket( &listener->sock );
	CFRelease( listener->server );
	free( listener );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	HTTPConnectionGetTypeID
//===========================================================================================================================

CFTypeID	HTTPConnectionGetTypeID( void )
{
	dispatch_once_f( &gHTTPConnectionInitOnce, NULL, _HTTPConnectionGetTypeID );
	return( gHTTPConnectionTypeID );
}

static void _HTTPConnectionGetTypeID( void *inContext )
{
	(void) inContext;
	
	gHTTPConnectionTypeID = _CFRuntimeRegisterClass( &kHTTPConnectionClass );
	check( gHTTPConnectionTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	HTTPConnectionCreate
//===========================================================================================================================

OSStatus
	HTTPConnectionCreate( 
		HTTPConnectionRef *				outCnx, 
		const HTTPConnectionDelegate *	inDelegate, 
		SocketRef						inSock, 
		HTTPServerRef					inServer )
{
	OSStatus				err;
	HTTPConnectionRef		me;
	size_t					extraLen;
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (HTTPConnectionRef) _CFRuntimeCreateInstance( NULL, HTTPConnectionGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	me->delegate = *inDelegate;
	if( !me->delegate.httpProtocol ) me->delegate.httpProtocol = "HTTP/1.1";
	me->ucat = inServer ? inServer->ucat : &log_category_from_name( HTTPServerCore );
	me->sock = kInvalidSocketRef;
	
	if( inServer )
	{
		CFRetain( inServer );
		me->server = inServer;
	}
	
	err = HTTPMessageCreate( &me->requestMsg );
	require_noerr( err, exit );
	
	err = HTTPMessageCreate( &me->responseMsg );
	require_noerr( err, exit );
	
	me->sock = inSock;
	if( inDelegate->initializeConnection_f )
	{
		err = inDelegate->initializeConnection_f( me );
		if( err ) me->sock = kInvalidSocketRef;
		require_noerr( err, exit );
	}
	
	*outCnx = me;
	me = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( me );
	return( err );
}

//===========================================================================================================================
//	_HTTPConnectionFinalize
//===========================================================================================================================

static void	_HTTPConnectionFinalize( CFTypeRef inCF )
{
	HTTPConnectionRef const		me = (HTTPConnectionRef) inCF;
	
	if( me->delegate.finalizeConnection_f ) me->delegate.finalizeConnection_f( me );
	ForgetCF( &me->server );
	dispatch_forget( &me->queue );
	check( me->readSource == NULL );
	check( me->writeSource == NULL );
	ForgetSocket( &me->sock );
	ForgetCF( &me->requestMsg );
	ForgetCF( &me->responseMsg );
}

//===========================================================================================================================
//	HTTPConnectionSetDispatchQueue
//===========================================================================================================================

void	HTTPConnectionSetDispatchQueue( HTTPConnectionRef inCnx, dispatch_queue_t inQueue )
{
	ReplaceDispatchQueue( &inCnx->queue, inQueue );
}

//===========================================================================================================================
//	HTTPConnectionStart
//===========================================================================================================================

OSStatus	HTTPConnectionStart( HTTPConnectionRef inCnx )
{
	OSStatus		err;
	socklen_t		len;
	int				option;
	
	if( !inCnx->queue ) HTTPConnectionSetDispatchQueue( inCnx, dispatch_get_main_queue() );
	
	// Disable SIGPIPE on this socket so we get EPIPE errors instead of terminating the process.
	
#if( defined( SO_NOSIGPIPE ) )
	setsockopt( inCnx->sock, SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, (socklen_t) sizeof( int ) );
#endif
	
	err = SocketMakeNonBlocking( inCnx->sock );
	check_noerr( err );
	
	// Get addresses for both sides and interface info.
	
	len = (socklen_t) sizeof( inCnx->selfAddr );
	err = getsockname( inCnx->sock, &inCnx->selfAddr.sa, &len );
	err = map_socket_noerr_errno( inCnx->sock, err );
	check_noerr( err );
	
	len = (socklen_t) sizeof( inCnx->peerAddr );
	err = getpeername( inCnx->sock, &inCnx->peerAddr.sa, &len );
	err = map_socket_noerr_errno( inCnx->sock, err );
	check_noerr( err );
	
#if( TARGET_OS_POSIX )
	SocketGetInterfaceInfo( inCnx->sock, NULL, inCnx->ifName, &inCnx->ifIndex, inCnx->ifMACAddress, &inCnx->ifMedia, &inCnx->ifFlags,
		&inCnx->ifExtendedFlags, NULL, &inCnx->transportType );
#endif
	if( !NetTransportTypeIsP2P( inCnx->transportType ) ) SocketSetP2P( inCnx->sock, false ); // Clear if P2P was inherited.
	
	// Disable nagle so responses we send are not delayed. We coalesce writes to minimize small writes anyway.
	
	option = 1;
	err = setsockopt( inCnx->sock, IPPROTO_TCP, TCP_NODELAY, (char *) &option, (socklen_t) sizeof( option ) );
	err = map_socket_noerr_errno( inCnx->sock, err );
	check_noerr( err );
	
	// Set up a source to notify us when the socket is readable.
	
	inCnx->readSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, inCnx->sock, 0, inCnx->queue );
	require_action( inCnx->readSource, exit, err = kUnknownErr );
	CFRetain( inCnx );
	dispatch_set_context( inCnx->readSource, inCnx );
	dispatch_source_set_event_handler_f( inCnx->readSource, _HTTPConnectionReadHandler );
	dispatch_source_set_cancel_handler_f( inCnx->readSource, _HTTPConnectionCancelHandler );
	dispatch_resume( inCnx->readSource );
	
	// Set up a source to notify us when the socket is writable.
	
	inCnx->writeSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_WRITE, inCnx->sock, 0, inCnx->queue );
	require_action( inCnx->writeSource, exit, err = kUnknownErr );
	CFRetain( inCnx );
	dispatch_set_context( inCnx->writeSource, inCnx );
	dispatch_source_set_event_handler_f( inCnx->writeSource, _HTTPConnectionWriteHandler );
	dispatch_source_set_cancel_handler_f( inCnx->writeSource, _HTTPConnectionCancelHandler );
	inCnx->writeSuspended = true; // Don't resume until we get EWOULDBLOCK.
	
exit:
	if( err ) HTTPConnectionStop( inCnx );
	return( err );
}

//===========================================================================================================================
//	HTTPConnectionStop
//===========================================================================================================================

void	HTTPConnectionStop( HTTPConnectionRef inCnx )
{
	dispatch_source_forget_ex( &inCnx->readSource,  &inCnx->readSuspended );
	dispatch_source_forget_ex( &inCnx->writeSource, &inCnx->writeSuspended );
}

//===========================================================================================================================
//	_HTTPConnectionReadHandler
//===========================================================================================================================

static void	_HTTPConnectionReadHandler( void *inContext )
{
	HTTPConnectionRef const		cnx = (HTTPConnectionRef) inContext;
	
	check( !cnx->readSuspended );
	dispatch_suspend( cnx->readSource ); // Disable readability notification until we get another EWOULDBLOCK.
	cnx->readSuspended = true;
	
	_HTTPConnectionRunStateMachine( cnx );
}

//===========================================================================================================================
//	_HTTPConnectionWriteHandler
//===========================================================================================================================

static void	_HTTPConnectionWriteHandler( void *inContext )
{
	HTTPConnectionRef const		cnx = (HTTPConnectionRef) inContext;
	
	check( !cnx->writeSuspended );
	dispatch_suspend( cnx->writeSource ); // Disable writability notification until we get another EWOULDBLOCK.
	cnx->writeSuspended = true;
	
	_HTTPConnectionRunStateMachine( cnx );
}

//===========================================================================================================================
//	_HTTPConnectionCancelHandler
//===========================================================================================================================

static void	_HTTPConnectionCancelHandler( void *inContext )
{
	HTTPConnectionRef const		cnx = (HTTPConnectionRef) inContext;
	
	CFRelease( cnx );
}

//===========================================================================================================================
//	_HTTPConnectionRunStateMachine
//===========================================================================================================================

static void	_HTTPConnectionRunStateMachine( HTTPConnectionRef inCnx )
{
	OSStatus			err;
	HTTPMessageRef		msg;
	
	for( ;; )
	{
		if( inCnx->state == kHTTPConnectionStateReadingRequest )
		{
			msg = inCnx->requestMsg;
			err = HTTPMessageReadMessage( inCnx->sock, msg );
			if( err == EWOULDBLOCK ) { dispatch_resume_if_suspended( inCnx->readSource, &inCnx->readSuspended ); break; }
			require_noerr_quiet( err, exit );
			LogHTTP( inCnx->ucat, inCnx->ucat, msg->header.buf, msg->header.len, msg->bodyPtr, msg->bodyLen );
			
			err = inCnx->delegate.handleMessage_f( inCnx, msg );
			require_noerr_quiet( err, exit );
		}
		else if( inCnx->state == kHTTPConnectionStateWritingResponse )
		{
			err = SocketWriteData( inCnx->sock, &inCnx->responseMsg->iop, &inCnx->responseMsg->ion );
			if( err == EWOULDBLOCK ) { dispatch_resume_if_suspended( inCnx->writeSource, &inCnx->writeSuspended ); break; }
			require_noerr_quiet( err, exit );
			require_action_quiet( inCnx->requestMsg->header.persistent, exit, err = kEndingErr );
			
			HTTPMessageReset( inCnx->requestMsg );
			HTTPMessageReset( inCnx->responseMsg );
			inCnx->state = kHTTPConnectionStateReadingRequest;
		}
		else
		{
			dlogassert( "Bad state %d", inCnx->state );
			err = kInternalErr;
			goto exit;
		}
	}
	err = kNoErr;
	
exit:
	if( err )
	{
		HTTPConnectionStop( inCnx );
		if( inCnx->delegate.closeConnection_f ) inCnx->delegate.closeConnection_f( inCnx );
	}
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	HTTPConnectionGetNextURLSegmentEx
//===========================================================================================================================

Boolean
	HTTPConnectionGetNextURLSegmentEx( 
		HTTPConnectionRef	inCnx, 
		HTTPMessageRef		inMsg, 
		Boolean				inDontSendResponse, 
		const char **		outPtr, 
		size_t *			outLen, 
		OSStatus *			outErr )
{
	HTTPHeader * const		hdr = &inMsg->header;
	Boolean					good;
	OSStatus				err;
	const char *			src;
	const char *			ptr;
	const char *			end;
	
	src = hdr->url.segmentPtr;
	end = hdr->url.segmentEnd;
	for( ptr = src; ( ptr < end ) && ( *ptr != '/' ); ++ptr ) {}
	good = (Boolean)( ptr != src );
	if( good )
	{
		*outPtr = src;
		*outLen = (size_t)( ptr - src );
		hdr->url.segmentPtr = ( ptr < end ) ? ( ptr + 1 ) : ptr;
	}
	else if( !inDontSendResponse )
	{
		ulog( inCnx->ucat, kLogLevelWarning, "### Bad URL segment: '%.*s'\n", (int) hdr->urlLen, hdr->urlPtr );
		err = HTTPConnectionSendStatusResponse( inCnx, kHTTPStatus_BadRequest );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	*outErr = err;
	return( good );
}

//===========================================================================================================================
//	HTTPConnectionInitResponse
//===========================================================================================================================

OSStatus	HTTPConnectionInitResponse( HTTPConnectionRef inCnx, HTTPStatus inStatusCode )
{
	OSStatus			err;
	char				str[ 64 ];
	const char *		ptr;
	
	HTTPHeader_InitResponse( &inCnx->responseMsg->header, inCnx->delegate.httpProtocol, inStatusCode, NULL );
	ptr = HTTPMakeDateString( time( NULL ), str, sizeof( str ) );
	if( *ptr != '\0' ) HTTPHeader_SetField( &inCnx->responseMsg->header, "Date", "%s", ptr );
	if( inCnx->delegate.initResponse_f )
	{
		err = inCnx->delegate.initResponse_f( inCnx, inCnx->responseMsg );
		require_noerr( err, exit );
	}
	inCnx->responseMsg->bodyLen = 0;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPConnectionSendResponse
//
//	Must be called from the connection queue.
//===========================================================================================================================

OSStatus	HTTPConnectionSendResponse( HTTPConnectionRef inCnx )
{
	HTTPMessageRef const		msg = inCnx->responseMsg;
	OSStatus					err;
	
	err = HTTPHeader_Commit( &msg->header );
	require_noerr( err, exit );
	LogHTTP( inCnx->ucat, inCnx->ucat, msg->header.buf, msg->header.len, msg->bodyPtr, msg->bodyLen );
	
	msg->iov[ 0 ].iov_base = msg->header.buf;
	msg->iov[ 0 ].iov_len  = msg->header.len;
	msg->ion = 1;
	if( msg->bodyLen > 0 )
	{
		msg->iov[ 1 ].iov_base = msg->bodyPtr;
		msg->iov[ 1 ].iov_len  = msg->bodyLen;
		msg->ion = 2;
	}
	msg->iop = msg->iov;
	inCnx->state = kHTTPConnectionStateWritingResponse;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPConnectionSendSimpleResponse
//
//	Must be called from the connection queue.
//===========================================================================================================================

OSStatus
	HTTPConnectionSendSimpleResponse( 
		HTTPConnectionRef	inCnx, 
		HTTPStatus			inStatus, 
		const char *		inContentType, 
		const void *		inBodyPtr, 
		size_t				inBodyLen )
{
	OSStatus		err;
	
	err = HTTPConnectionInitResponse( inCnx, inStatus );
	require_noerr( err, exit );
	
	err = HTTPMessageSetBody( inCnx->responseMsg, inContentType, inBodyPtr, inBodyLen );
	require_noerr( err, exit );
	
	err = HTTPConnectionSendResponse( inCnx );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPConnectionVerifyAuth
//===========================================================================================================================

OSStatus	HTTPConnectionVerifyAuth( HTTPConnectionRef inCnx, HTTPMessageRef inMsg, Boolean *outAllow )
{
	HTTPServerRef const			server = inCnx->server;
	OSStatus					err;
	Boolean						allow = false;
	HTTPAuthorizationInfo		authInfo;
	HTTPStatus					status;
	char						nonceStr[ 64 ];
	
	if( inCnx->authorized || !server->password )
	{
		inCnx->authorized = true;
		allow = true;
		err = kNoErr;
		goto exit;
	}
	if( inCnx->delegate.requiresAuth_f && !inCnx->delegate.requiresAuth_f( inCnx, inMsg ) )
	{
		allow = true;
		err = kNoErr;
		goto exit;
	}
	
	memset( &authInfo, 0, sizeof( authInfo ) );
	authInfo.serverScheme			= kHTTPAuthorizationScheme_Digest;
	authInfo.serverPassword			= server->password;
	authInfo.serverTimedNonceKeyPtr	= server->timedNonceKey;
	authInfo.serverTimedNonceKeyLen	= sizeof( server->timedNonceKey );
	authInfo.headerPtr				= inMsg->header.buf;
	authInfo.headerLen				= inMsg->header.len;
	authInfo.requestMethodPtr		= inMsg->header.methodPtr;
	authInfo.requestMethodLen		= inMsg->header.methodLen;
	authInfo.requestURLPtr			= inMsg->header.urlPtr;
	authInfo.requestURLLen			= inMsg->header.urlLen;
	status = HTTPVerifyAuthorization( &authInfo );
	if( status == kHTTPStatus_OK )
	{
		inCnx->authorized = true;
		allow = true;
		err = kNoErr;
		goto exit;
	}
	else if( status == kHTTPStatus_Unauthorized )
	{
		// Some HTTP clients use separate connections for each auth attempt so we have to use a timed nonce instead 
		// of a stronger per-connection nonce.
		
		err = HTTPMakeTimedNonce( kHTTPTimedNonceETagPtr, kHTTPTimedNonceETagLen, 
			server->timedNonceKey, sizeof( server->timedNonceKey ), 
			nonceStr, sizeof( nonceStr ), NULL );
		require_noerr_action( err, statusExit, status = kHTTPStatus_InternalServerError );
		require_action( server->realm, statusExit, status = kHTTPStatus_InternalServerError );
		
		err = HTTPConnectionInitResponse( inCnx, kHTTPStatus_Unauthorized );
		require_noerr( err, exit );
		
		HTTPHeader_SetField( &inCnx->responseMsg->header, "Content-Length", "0" );
		HTTPHeader_SetField( &inCnx->responseMsg->header, "WWW-Authenticate", "Digest realm=\"%s\", nonce=\"%s\"", 
			server->realm, nonceStr );
		
		err = HTTPConnectionSendResponse( inCnx );
		require_noerr( err, exit );
		goto exit;
	}
	
statusExit:
	err = HTTPConnectionSendStatusResponse( inCnx, status );
	require_noerr( err, exit );

exit:
	*outAllow = allow;
	return( err );
}

#if 0
#pragma mark -
#endif

