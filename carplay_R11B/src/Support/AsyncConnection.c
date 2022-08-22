/*
	File:    	AsyncConnection.c
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
	
	Copyright (C) 2010-2014 Apple Inc. All Rights Reserved.
*/

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#include "AsyncConnection.h"

#include <dns_sd.h>
#include <errno.h>

#include "APSDebugServices.h"
#include "CFUtils.h"
#include "NetUtils.h"
#include "StringUtils.h"
#include "URLUtils.h"

#if( TARGET_OS_POSIX )
	#include <net/if.h>
	#include <netinet/tcp.h>
#endif

//===========================================================================================================================
//	Internals
//===========================================================================================================================

typedef struct AsyncConnectionOperation *		AsyncConnectionOperationRef;

struct AsyncConnection
{
	int32_t							refCount;
	char *							destination;
	int								defaultPort;
	AsyncConnectionFlags			flags;
	uint64_t						timeoutNanos;
	int								socketSendBufferSize;
	int								socketRecvBufferSize;
	dispatch_source_t				timerSource;
	AsyncConnectionOperationRef		operationList;
	AsyncConnectionProgressFunc		progressFunc;
	void *							progressArg;
	dispatch_queue_t				handlerQueue;
	AsyncConnectionHandlerFunc		handlerFunc;
	void *							handlerArg;
};

struct AsyncConnectionOperation
{
	int32_t							refCount;
	AsyncConnectionOperationRef		next;
	AsyncConnectionRef				connection;
	sockaddr_ip						addr;
	uint32_t						ifIndex;
	int								defaultPort;
	SocketRef						nativeSock;
	DNSServiceRef					service;
	dispatch_source_t				readSource;
	dispatch_source_t				writeSource;
};

static void	_AsyncConnection_Release( void *inArg );
static void	_AsyncConnection_UserRelease( void *inArg );
static void	_AsyncConnection_Complete( AsyncConnectionRef inConnection, SocketRef inSock, OSStatus inError );
static void	_AsyncConnection_ReleaseOperation( AsyncConnectionOperationRef inOperation );
static void	_AsyncConnection_Connect( void *inArg );
static OSStatus
	_AsyncConnection_StartSRVQuery( 
		AsyncConnectionRef	inConnection, 
		const char *		inDestination, 
		int					inDefaultPort );
static void DNSSD_API
	_AsyncConnection_SRVCallBack(
		DNSServiceRef		inRef,
		DNSServiceFlags		inFlags,
		uint32_t			inIfIndex,
		DNSServiceErrorType	inErrorCode,
		const char *		inFullName,    
		uint16_t			inRRType,
		uint16_t			inRRClass,
		uint16_t			inRDataSize,
		const void *		inRData,
		uint32_t			inTTL,
		void *				inContext );
static OSStatus
	_AsyncConnection_StartDNSResolve( 
		AsyncConnectionRef	inConnection, 
		const char *		inDestination, 
		int					inDefaultPort );
static void DNSSD_API
	_AsyncConnection_DNSCallBack(
		DNSServiceRef			inRef,
		DNSServiceFlags			inFlags,
		uint32_t				inIfIndex,
		DNSServiceErrorType		inErrorCode,
		const char *			inHostName,
		const struct sockaddr *	inAddr,
		uint32_t				inTTL,
		void *					inContext );

static OSStatus
	_AsyncConnection_StartConnect( 
		AsyncConnectionRef	inConnection, 
		const void *		inAddr, 
		uint32_t			inIfIndex, 
		int					inDefaultPort );
static void	_AsyncConnection_ConnectHandler( AsyncConnectionOperationRef inOperation );
static void	_AsyncConnection_EventHandler( void *inContext );
static void	_AsyncConnection_CancelHandler( void *inContext );
static void	_AsyncConnection_ErrorHandler( AsyncConnectionOperationRef inOperation, OSStatus inError );
static void	_AsyncConnection_TimeoutHandler( void *inArg );

#define AsyncConnectionProgress( CNX, PHASE, DETAILS ) \
	do { if( (CNX)->progressFunc ) (CNX)->progressFunc( (PHASE), (DETAILS), (CNX)->progressArg ); } while( 0 )

ulog_define( AsyncConnection, kLogLevelNotice, kLogFlags_Default, "AsyncConnection",  NULL );
#define ac_ulog( LEVEL, ... )		ulog( &log_category_from_name( AsyncConnection ), (LEVEL), __VA_ARGS__ )

//===========================================================================================================================
//	Stuff from the Darwin mDNSResponder project
//===========================================================================================================================

// RFC 1034/1035 specify that a domain label consists of a length byte plus up to 63 characters
#define MAX_DOMAIN_LABEL 63
typedef struct { uint8_t c[ 64]; } domainlabel;		// One label: length byte and up to 63 characters

// RFC 1034/1035 specify that a domain name, including length bytes, data bytes, and terminating zero, may be up to 255 bytes long
#define MAX_DOMAIN_NAME 255
typedef struct { uint8_t c[256]; } domainname;		// Up to 255 bytes of length-prefixed domainlabels

// Convert native format domainlabel or domainname back to C string format
// IMPORTANT:
// When using ConvertDomainLabelToCString, the target buffer must be MAX_ESCAPED_DOMAIN_LABEL (254) bytes long
// to guarantee there will be no buffer overrun. It is only safe to use a buffer shorter than this in rare cases
// where the label is known to be constrained somehow (for example, if the label is known to be either "_tcp" or "_udp").
// Similarly, when using ConvertDomainNameToCString, the target buffer must be MAX_ESCAPED_DOMAIN_NAME (1005) bytes long.
// See definitions of MAX_ESCAPED_DOMAIN_LABEL and MAX_ESCAPED_DOMAIN_NAME for more detailed explanation.
static char    *ConvertDomainLabelToCString_withescape(const domainlabel *const name, char *cstring, char esc);
#define         ConvertDomainLabelToCString_unescaped(D,C) ConvertDomainLabelToCString_withescape((D), (C), 0)
#define         ConvertDomainLabelToCString(D,C)           ConvertDomainLabelToCString_withescape((D), (C), '\\')
static char    *ConvertDomainNameToCString_withescape(const domainname *const name, char *cstring, char esc);
#define         ConvertDomainNameToCString_unescaped(D,C) ConvertDomainNameToCString_withescape((D), (C), 0)
#define         ConvertDomainNameToCString(D,C)           ConvertDomainNameToCString_withescape((D), (C), '\\')

// Other

static OSStatus
	ParseDestination( 
		const char *	inDestination, 
		char *			inNameBuf, 
		size_t			inNameMaxLen, 
		uint32_t *		outIndex, 
		int *			outPort );

//===========================================================================================================================
//	AsyncConnection_Connect
//===========================================================================================================================

OSStatus
	AsyncConnection_Connect( 
		AsyncConnectionRef *			outConnection, 
		const char *					inDestination, 
		int								inDefaultPort, 
		AsyncConnectionFlags			inFlags, 
		uint64_t						inTimeoutNanos, 
		int								inSocketSendBufferSize, 
		int								inSocketRecvBufferSize, 
		AsyncConnectionProgressFunc		inProgressFunc, 
		void *							inProgressArg, 
		AsyncConnectionHandlerFunc		inHandlerFunc, 
		void *							inHandlerArg, 
		dispatch_queue_t				inHandlerQueue )
{
	OSStatus				err;
	AsyncConnectionRef		obj = NULL;
	const char *			ptr;
	URLComponents			urlComps;
	size_t					destinationLen;
	
	require_action( *inDestination != '\0', exit, err = kParamErr );
	ptr = strchr( inDestination, ':' );
	if( ptr && ( ptr[ 1 ] == '/' ) && ( ptr[ 2 ] == '/' ) ) // If it looks like a URL.
	{
		err = URLParseComponents( inDestination, NULL, &urlComps, NULL );
		require_noerr( err, exit );
		
		inDestination  = urlComps.hostPtr;
		destinationLen = urlComps.hostLen;
	}
	else
	{
		destinationLen = strlen( inDestination );
	}
	
	obj = (AsyncConnectionRef) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->refCount = 1;
	
	obj->destination = strndup( inDestination, destinationLen );
	require_action( obj->destination, exit, err = kNoMemoryErr );
	obj->defaultPort			= inDefaultPort;
	obj->flags					= inFlags;
	obj->timeoutNanos			= ( inTimeoutNanos != 0 ) ? inTimeoutNanos : DISPATCH_TIME_FOREVER;
	obj->socketSendBufferSize	= inSocketSendBufferSize;
	obj->socketRecvBufferSize	= inSocketRecvBufferSize;
	obj->progressFunc			= inProgressFunc;
	obj->progressArg			= inProgressArg;
	
	dispatch_retain( inHandlerQueue );
	obj->handlerQueue	= inHandlerQueue;
	obj->handlerFunc	= inHandlerFunc;
	obj->handlerArg		= inHandlerArg;
	
	dispatch_async_f( inHandlerQueue, obj, _AsyncConnection_Connect );
	
	*outConnection = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _AsyncConnection_Release( obj );
	return( err );
}

static void	_AsyncConnection_Connect( void *inArg )
{
	AsyncConnectionRef const		connection = (AsyncConnectionRef) inArg;
	OSStatus						err;
	sockaddr_ip						sip;
	
	err = StringToSockAddr( connection->destination, &sip, sizeof( sip ), NULL );
	if( err == kNoErr )
	{
		{
			err = _AsyncConnection_StartConnect( connection, &sip, 0, connection->defaultPort );
			require_noerr( err, exit );
		}
	}
	else if( stristr( connection->destination, "._tcp."  ) )
	{
		err = _AsyncConnection_StartSRVQuery( connection, connection->destination, connection->defaultPort );
		require_noerr( err, exit );
	}
	else
	{
		err = _AsyncConnection_StartDNSResolve( connection, connection->destination, connection->defaultPort );
		require_noerr( err, exit );
	}
	
	if( connection->timeoutNanos != DISPATCH_TIME_FOREVER )
	{
		connection->timerSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, connection->handlerQueue );
		require_action( connection->timerSource, exit, err = kUnknownErr );
		dispatch_set_context( connection->timerSource, connection );
		dispatch_source_set_event_handler_f( connection->timerSource, _AsyncConnection_TimeoutHandler );
		dispatch_source_set_timer( connection->timerSource, dispatch_time( DISPATCH_TIME_NOW, connection->timeoutNanos ), 
			connection->timeoutNanos, 100 * kNanosecondsPerMillisecond );
		dispatch_resume( connection->timerSource );
	}
	
exit:
	if( err ) _AsyncConnection_Complete( connection, kInvalidSocketRef, err );
}

//===========================================================================================================================
//	AsyncConnection_Release
//===========================================================================================================================

void	AsyncConnection_Release( AsyncConnectionRef inConnection )
{
	dispatch_async_f( inConnection->handlerQueue, inConnection, _AsyncConnection_UserRelease );
}

static void	_AsyncConnection_Release( void *inArg )
{
	AsyncConnectionRef const		connection = (AsyncConnectionRef) inArg;
	
	if( --connection->refCount == 0 )
	{
		_AsyncConnection_Complete( connection, kInvalidSocketRef, kCanceledErr );
		dispatch_forget( &connection->handlerQueue );
		ForgetMem( &connection->destination );
		free( connection );
	}
}

static void	_AsyncConnection_UserRelease( void *inArg )
{
	_AsyncConnection_Complete( (AsyncConnectionRef) inArg, kInvalidSocketRef, kCanceledErr );
	_AsyncConnection_Release( (AsyncConnectionRef) inArg );
}

//===========================================================================================================================
//	_AsyncConnection_Complete
//===========================================================================================================================

static void	_AsyncConnection_Complete( AsyncConnectionRef inConnection, SocketRef inSock, OSStatus inError )
{
	AsyncConnectionHandlerFunc		handlerFunc;
	AsyncConnectionOperationRef		operation;
	
	handlerFunc = inConnection->handlerFunc;
	inConnection->handlerFunc = NULL;
	if( handlerFunc ) handlerFunc( inSock, inError, inConnection->handlerArg );
	
	dispatch_source_forget( &inConnection->timerSource );
	while( ( operation = inConnection->operationList ) != NULL )
	{
		inConnection->operationList = operation->next;
		_AsyncConnection_ReleaseOperation( operation );
	}
}

//===========================================================================================================================
//	_AsyncConnection_ReleaseOperation
//===========================================================================================================================

static void	_AsyncConnection_ReleaseOperation( AsyncConnectionOperationRef inOperation )
{
	dispatch_source_forget( &inOperation->readSource );
	dispatch_source_forget( &inOperation->writeSource );
	DNSServiceForget( &inOperation->service );
	if( --inOperation->refCount == 0 )
	{
		ForgetSocket( &inOperation->nativeSock );
		_AsyncConnection_Release( inOperation->connection );
		free( inOperation );
	}
}

//===========================================================================================================================
//	_AsyncConnection_StartSRVQuery
//===========================================================================================================================

static OSStatus
	_AsyncConnection_StartSRVQuery( 
		AsyncConnectionRef	inConnection, 
		const char *		inDestination, 
		int					inDefaultPort )
{
	OSStatus						err;
	char							host[ kDNSServiceMaxDomainName ];
	uint32_t						ifindex;
	AsyncConnectionOperationRef		obj;
	int								port;
	
	obj = (AsyncConnectionOperationRef) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	++inConnection->refCount;
	obj->refCount	= 1;
	obj->connection = inConnection;
	obj->nativeSock = kInvalidSocketRef;
	
	port = 0;
	err = ParseDestination( inDestination, host, sizeof( host ), &ifindex, &port );
	require_noerr( err, exit );
	obj->defaultPort = ( ( port == 0 ) || ( inDefaultPort < 0 ) ) ? inDefaultPort : port;
	
	ac_ulog( kLogLevelVerbose, "Querying SRV  %s\n", inDestination );
	AsyncConnectionProgress( inConnection, kAsyncConnectionPhase_QueryingSRV, inDestination );
	err = DNSServiceQueryRecord( &obj->service, kDNSServiceFlagsUnicastResponse_compat, ifindex, host, 
		kDNSServiceType_SRV, kDNSServiceClass_IN, _AsyncConnection_SRVCallBack, obj );
	require_noerr( err, exit );
	
#if( _DNS_SD_LIBDISPATCH )
	DNSServiceSetDispatchQueue( obj->service, inConnection->handlerQueue );
#else
	obj->readSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, DNSServiceRefSockFD( obj->service ), 0, 
		inConnection->handlerQueue );
	require_action( obj->readSource, exit, err = kUnknownErr );
	dispatch_set_context( obj->readSource, obj );
	dispatch_source_set_event_handler_f( obj->readSource, _AsyncConnection_EventHandler );
	dispatch_source_set_cancel_handler_f( obj->readSource, _AsyncConnection_CancelHandler );
	dispatch_resume( obj->readSource );
	++obj->refCount;
#endif
	
	obj->next = inConnection->operationList;
	inConnection->operationList = obj;
	obj = NULL;
	
exit:
	if( obj ) _AsyncConnection_ReleaseOperation( obj );
	return( err );
}

//===========================================================================================================================
//	_AsyncConnection_SRVCallBack
//===========================================================================================================================

static void DNSSD_API
	_AsyncConnection_SRVCallBack(
		DNSServiceRef		inRef,
		DNSServiceFlags		inFlags,
		uint32_t			inIfIndex,
		DNSServiceErrorType	inErrorCode,
		const char *		inFullName,    
		uint16_t			inRRType,
		uint16_t			inRRClass,
		uint16_t			inRDataSize,
		const void *		inRData,
		uint32_t			inTTL,
		void *				inContext )
{
	AsyncConnectionOperationRef const		operation = (AsyncConnectionOperationRef) inContext;
	OSStatus								err;
	const uint8_t *							ptr;
	int										port;
	char									host[ kDNSServiceMaxDomainName + 64 ];
	size_t									len;
	
	(void) inRef;
	(void) inFullName;
	(void) inRRType;
	(void) inRRClass;
	(void) inTTL;
	
	require_noerr_action( inErrorCode, exit, err = inErrorCode );
	require_action_quiet( inFlags & kDNSServiceFlagsAdd, exit, err = kNoErr );
	
	// Parse the SRV data. It's in the format: <2:priority> <2:weight> <2:port> <n:dnsName>.
	// Note: mDNSResponder has already validated the packet so we rely on it not giving us malformed data.
	
	require_action( inRDataSize > 6, exit, err = kMalformedErr );
	ptr = (const uint8_t *) inRData;
	port = ( ptr[ 4 ] << 8 ) | ptr[ 5 ];
	ConvertDomainNameToCString( (domainname *) &ptr[ 6 ], host );
	
	ac_ulog( kLogLevelVerbose, "SRV resolved  %s -> %s port %d, If %u, Flags 0x%X, TTL %u\n", 
		inFullName, host, port, inIfIndex, inFlags, inTTL );
	
	// Start resolving the DNS name.
	
	len = strlen( host );
	snprintf( &host[ len ], sizeof( host ) - len, "%%%u", inIfIndex );
	if( ( port == 0 ) || ( operation->defaultPort < 0 ) ) port = operation->defaultPort;
	err = _AsyncConnection_StartDNSResolve( operation->connection, host, port );
	require_noerr( err, exit );
	
exit:
	if( err ) _AsyncConnection_ErrorHandler( operation, err );
}

//===========================================================================================================================
//	_AsyncConnection_StartDNSResolve
//===========================================================================================================================

static OSStatus
	_AsyncConnection_StartDNSResolve( 
		AsyncConnectionRef	inConnection, 
		const char *		inDestination, 
		int					inDefaultPort )
{
	OSStatus						err;
	char							host[ kDNSServiceMaxDomainName ];
	uint32_t						ifindex;
	AsyncConnectionOperationRef		obj;
	int								port;
	DNSServiceFlags					flags;
	
	obj = (AsyncConnectionOperationRef) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	++inConnection->refCount;
	obj->refCount	= 1;
	obj->connection = inConnection;
	obj->nativeSock = kInvalidSocketRef;
	
	port = 0;
	err = ParseDestination( inDestination, host, sizeof( host ), &ifindex, &port );
	require_noerr( err, exit );
	obj->defaultPort = ( ( port == 0 ) || ( inDefaultPort < 0 ) ) ? inDefaultPort : port;
	
	ac_ulog( kLogLevelVerbose, "Resolving     %s\n", inDestination );
	AsyncConnectionProgress( inConnection, kAsyncConnectionPhase_ResolvingDNS, inDestination );
	flags = 0;
	if( inConnection->flags & kAsyncConnectionFlag_SuppressUnusable ) flags |= kDNSServiceFlagsSuppressUnusable;
	err = DNSServiceGetAddrInfo( &obj->service, flags, ifindex, 0, host, _AsyncConnection_DNSCallBack, obj );
	require_noerr( err, exit );
	
#if( _DNS_SD_LIBDISPATCH )
	DNSServiceSetDispatchQueue( obj->service, inConnection->handlerQueue );
#else
	obj->readSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, DNSServiceRefSockFD( obj->service ), 0, 
		inConnection->handlerQueue );
	require_action( obj->readSource, exit, err = kUnknownErr );
	dispatch_set_context( obj->readSource, obj );
	dispatch_source_set_event_handler_f( obj->readSource, _AsyncConnection_EventHandler );
	dispatch_source_set_cancel_handler_f( obj->readSource, _AsyncConnection_CancelHandler );
	dispatch_resume( obj->readSource );
	++obj->refCount;
#endif
	
	obj->next = inConnection->operationList;
	inConnection->operationList = obj;
	obj = NULL;
	
exit:
	if( obj ) _AsyncConnection_ReleaseOperation( obj );
	return( err );
}

//===========================================================================================================================
//	_AsyncConnection_DNSCallBack
//===========================================================================================================================

static void DNSSD_API
	_AsyncConnection_DNSCallBack(
		DNSServiceRef			inRef,
		DNSServiceFlags			inFlags,
		uint32_t				inIfIndex,
		DNSServiceErrorType		inErrorCode,
		const char *			inHostName,
		const struct sockaddr *	inAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	AsyncConnectionOperationRef const		operation  = (AsyncConnectionOperationRef) inContext;
	AsyncConnectionRef const				connection = operation->connection;
	OSStatus								err;
	
	(void) inRef;
	(void) inHostName;
	(void) inTTL;
	
	require_noerr_action_quiet( inErrorCode, exit, err = inErrorCode );
	require_action_quiet( inFlags & kDNSServiceFlagsAdd, exit, err = kNoErr );
	
	ac_ulog( kLogLevelVerbose, "Resolved      %s -> %##a, Flags 0x%X, If %u, TTL %u\n", 
		inHostName, inAddr, inFlags, inIfIndex, inTTL );
	
	err = _AsyncConnection_StartConnect( connection, inAddr, inIfIndex, operation->defaultPort );
	require_noerr_quiet( err, exit );
	
exit:
	return;
}

//===========================================================================================================================
//	_AsyncConnection_StartConnect
//===========================================================================================================================

static OSStatus
	_AsyncConnection_StartConnect( 
		AsyncConnectionRef	inConnection, 
		const void *		inAddr, 
		uint32_t			inIfIndex, 
		int					inDefaultPort )
{
	OSStatus						err;
	AsyncConnectionOperationRef		obj;
	int								tempInt;
	
	obj = (AsyncConnectionOperationRef) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	++inConnection->refCount;
	obj->refCount	= 1;
	obj->connection	= inConnection;
	SockAddrCopy( inAddr, &obj->addr );
	if(      inDefaultPort < 0 )					SockAddrSetPort( &obj->addr, -inDefaultPort );
	else if( SockAddrGetPort( &obj->addr ) == 0 )	SockAddrSetPort( &obj->addr,  inDefaultPort );
	obj->defaultPort = inDefaultPort;
	
	obj->nativeSock = socket( obj->addr.sa.sa_family, SOCK_STREAM, IPPROTO_TCP );
	err = map_socket_creation_errno( obj->nativeSock );
	require_noerr( err, exit );
	
	err = SocketMakeNonBlocking( obj->nativeSock );
	require_noerr( err, exit );
	
	if( inConnection->flags & kAsyncConnectionFlag_P2P ) SocketSetP2P( obj->nativeSock, true );
	
#if( defined( SO_NOSIGPIPE ) )
	// Disable SIGPIPE for this socket so it returns an EPIPE error instead of terminating the process with SIGPIPE.
	
	setsockopt( obj->nativeSock, SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, (socklen_t) sizeof( int ) );
#endif
	
	// Set the bound interface to work around issues such IPv4 devices on different interaces with the same IP.
	// This also works around captive network setup since that may prevent automatic interface selection/routing.
	
	if( ( inIfIndex != 0 ) && ( inConnection->flags & kAsyncConnectionFlag_BoundInterface ) )
	{
		SocketSetBoundInterface( obj->nativeSock, obj->addr.sa.sa_family, inIfIndex );
	}
	
	// Disable nagle so responses we send are not delayed. Code should coalesce writes to minimize small writes instead.
	
	tempInt = 1;
	setsockopt( obj->nativeSock, IPPROTO_TCP, TCP_NODELAY, (char *) &tempInt, (socklen_t) sizeof( tempInt ) );
	
	// Configure socket buffer before connect because TCP needs it for its window size negotiation during connect.
	
	SocketSetBufferSize( obj->nativeSock, SO_SNDBUF, inConnection->socketSendBufferSize );
	SocketSetBufferSize( obj->nativeSock, SO_RCVBUF, inConnection->socketRecvBufferSize );
	
	// Start the connection process. If this didn't succeed or fail immediately, schedule completion callbacks.
	
	ac_ulog( kLogLevelVerbose, "Connecting to %##a, interface %d\n", &obj->addr, 
		( inConnection->flags & kAsyncConnectionFlag_BoundInterface ) ? inIfIndex : 0 );
	AsyncConnectionProgress( inConnection, kAsyncConnectionPhase_Connecting, &obj->addr );
	err = connect( obj->nativeSock, &obj->addr.sa, SockAddrGetSize( &obj->addr ) );
	err = map_socket_noerr_errno( obj->nativeSock, err );
	if( err == 0 )				{ _AsyncConnection_ConnectHandler( obj ); goto exit; }
	if( err != EINPROGRESS )	{ ac_ulog( kLogLevelNotice, "### Connect 1 to %##a failed: %#m\n", &obj->addr, err ); goto exit; }
	
	obj->readSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, obj->nativeSock, 0, inConnection->handlerQueue );
	require_action( obj->readSource, exit, err = kUnknownErr );
	dispatch_set_context( obj->readSource, obj );
	dispatch_source_set_event_handler_f( obj->readSource, _AsyncConnection_EventHandler );
	dispatch_source_set_cancel_handler_f( obj->readSource, _AsyncConnection_CancelHandler );
	dispatch_resume( obj->readSource );
	++obj->refCount;
	
	obj->writeSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_WRITE, obj->nativeSock, 0, inConnection->handlerQueue );
	require_action( obj->writeSource, exit, err = kUnknownErr );
	dispatch_set_context( obj->writeSource, obj );
	dispatch_source_set_event_handler_f( obj->writeSource, _AsyncConnection_EventHandler );
	dispatch_source_set_cancel_handler_f( obj->writeSource, _AsyncConnection_CancelHandler );
	dispatch_resume( obj->writeSource );
	++obj->refCount;
	
	obj->next = inConnection->operationList;
	inConnection->operationList = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _AsyncConnection_ReleaseOperation( obj );
	return( err );
}

//===========================================================================================================================
//	_AsyncConnection_ConnectHandler
//===========================================================================================================================

static void	_AsyncConnection_ConnectHandler( AsyncConnectionOperationRef inOperation )
{
	OSStatus		err;
	int				tempInt;
	socklen_t		len;
	SocketRef		sock;
	
	// Check if the connection was successful by checking the current pending socket error. Some sockets 
	// implementations return an error from getsockopt itself to signal an error so handle that case as well.
	
	tempInt = 0;
	len = (socklen_t) sizeof( tempInt );
	sock = inOperation->nativeSock;
	err = getsockopt( sock, SOL_SOCKET, SO_ERROR, (char *) &tempInt, &len );
	err = map_socket_noerr_errno( sock, err );
	if( err == 0 ) err = tempInt;
	if( err ) goto exit;
	
	ac_ulog( kLogLevelVerbose, "Connected to  %##a\n", &inOperation->addr );
	AsyncConnectionProgress( inOperation->connection, kAsyncConnectionPhase_Connected, &inOperation->addr );
	inOperation->nativeSock = kInvalidSocketRef;
	_AsyncConnection_Complete( inOperation->connection, sock, kNoErr );
	
exit:
	if( err ) _AsyncConnection_ErrorHandler( inOperation, err );
}

//===========================================================================================================================
//	_AsyncConnection_EventHandler
//===========================================================================================================================

static void	_AsyncConnection_EventHandler( void *inContext )
{
	AsyncConnectionOperationRef const		operation  = (AsyncConnectionOperationRef) inContext;
	OSStatus								err;
	int										tmp;
	socklen_t								len;
	
	if( operation->service )
	{
		err = DNSServiceProcessResult( operation->service );
		if( err )
		{
			ac_ulog( kLogLevelAssert, "DNSServiceProcessResult failed: %#m...mDNSResponder probably crashed\n", err );
			_AsyncConnection_ErrorHandler( operation, err );
		}
	}
	else
	{
		err = connect( operation->nativeSock, &operation->addr.sa, SockAddrGetSize( &operation->addr ) );
		err = map_socket_noerr_errno( operation->nativeSock, err );
		if( ( err == 0 ) || ( err == EISCONN ) )
		{
			_AsyncConnection_ConnectHandler( operation );
		}
		else
		{
			if( err == EINVAL )
			{
				tmp = 0;
				len = (socklen_t) sizeof( tmp );
				err = getsockopt( operation->nativeSock, SOL_SOCKET, SO_ERROR, (char *) &tmp, &len );
				err = map_socket_noerr_errno( operation->nativeSock, err );
				if( !err ) err = tmp;
				if( !err ) err = EINVAL;
			}
			ac_ulog( kLogLevelInfo, "### Connect 2 to %##a failed: %#m\n", &operation->addr, err );
			_AsyncConnection_ErrorHandler( operation, err );
		}
	}
}

//===========================================================================================================================
//	_AsyncConnection_CancelHandler
//===========================================================================================================================

static void	_AsyncConnection_CancelHandler( void *inContext )
{
	_AsyncConnection_ReleaseOperation( (AsyncConnectionOperationRef) inContext );
}

//===========================================================================================================================
//	_AsyncConnection_ErrorHandler
//===========================================================================================================================

static void	_AsyncConnection_ErrorHandler( AsyncConnectionOperationRef inOperation, OSStatus inError )
{
	AsyncConnectionRef const			connection = inOperation->connection;
	AsyncConnectionOperationRef *		next;
	AsyncConnectionOperationRef			curr;
	
	for( next = &connection->operationList; ( curr = *next ) != NULL; next = &curr->next )
	{
		if( curr == inOperation )
		{
			*next = curr->next;
			break;
		}
	}
	if( connection->operationList == NULL )
	{
		_AsyncConnection_Complete( connection, kInvalidSocketRef, inError );
	}
	if( curr )
	{
		_AsyncConnection_ReleaseOperation( curr );
	}
}

//===========================================================================================================================
//	_AsyncConnection_TimeoutHandler
//===========================================================================================================================

static void	_AsyncConnection_TimeoutHandler( void *inArg )
{
	_AsyncConnection_Complete( (AsyncConnectionRef) inArg, kInvalidSocketRef, kTimeoutErr );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	Stuff from the Darwin mDNSResponder project
//===========================================================================================================================

static char *ConvertDomainLabelToCString_withescape(const domainlabel *const label, char *ptr, char esc)
	{
	const uint8_t *      src = label->c;						// Domain label we're reading
	const uint8_t        len = *src++;							// Read length of this (non-null) label
	const uint8_t *const end = src + len;						// Work out where the label ends
	if (len > MAX_DOMAIN_LABEL) return(NULL);					// If illegal label, abort
	while (src < end)											// While we have characters in the label
		{
		uint8_t c = *src++;
		if (esc)
			{
			if (c == '.' || c == esc)							// If character is a dot or the escape character
				*ptr++ = esc;									// Output escape character
			else if (c <= ' ')									// If non-printing ascii,
				{												// Output decimal escape sequence
				*ptr++ = esc;
				*ptr++ = (char)   ('0' + (c / 100)     );
				*ptr++ = (char)   ('0' + (c /  10) % 10);
				c      = (uint8_t)('0' + (c      ) % 10);
				}
			}
		*ptr++ = (char)c;										// Copy the character
		}
	*ptr = 0;													// Null-terminate the string
	return(ptr);												// and return
	}

// Note: To guarantee that there will be no possible overrun, cstring must be at least MAX_ESCAPED_DOMAIN_NAME (1005 bytes)
static char *ConvertDomainNameToCString_withescape(const domainname *const name, char *ptr, char esc)
	{
	const uint8_t *src         = name->c;							// Domain name we're reading
	const uint8_t *const max   = name->c + MAX_DOMAIN_NAME;			// Maximum that's valid

	if (*src == 0) *ptr++ = '.';									// Special case: For root, just write a dot

	while (*src)													// While more characters in the domain name
		{
		if (src + 1 + *src >= max) return(NULL);
		ptr = ConvertDomainLabelToCString_withescape((const domainlabel *)src, ptr, esc);
		if (!ptr) return(NULL);
		src += 1 + *src;
		*ptr++ = '.';												// Write the dot after the label
		}

	*ptr++ = 0;														// Null-terminate the string
	return(ptr);													// and return
	}

//===========================================================================================================================
//	ParseDestination
//===========================================================================================================================

static OSStatus
	ParseDestination( 
		const char *	inDestination, 
		char *			inNameBuf, 
		size_t			inNameMaxLen, 
		uint32_t *		outIndex, 
		int *			outPort )
{
	OSStatus			err;
	const char *		ptr;
	const char *		ptr2;
	const char *		end;
	char				ifname[ IF_NAMESIZE + 1 ];
	unsigned int		ifindex;
	int					hasPort;
	int					port;
	int					n;
	size_t				len;
	
	hasPort = 0;
	port = 0;
	ptr = strchr( inDestination, '%' );
	if( ptr )
	{
		ptr2 = ptr + 1;
		end = strchr( ptr2, ':' );
		if( end )
		{
			n = sscanf( end + 1, "%d", &port );
			require_action( n == 1, exit, err = kMalformedErr );
			hasPort = 1;
		}
		else
		{
			end = ptr2 + strlen( ptr );
		}
		len = (size_t)( end - ptr2 );
		require_action( len < sizeof( ifname ), exit, err = kMalformedErr );
		memcpy( ifname, ptr2, len );
		ifname[ len ] = '\0';
		
		ifindex = if_nametoindex( ifname );
		if( ifindex == 0 )
		{
			n = sscanf( ifname, "%u", &ifindex );
			require_action( n == 1, exit, err = kMalformedErr );
		}
		len = (size_t)( ptr - inDestination );
	}
	else
	{
		ptr = strchr( inDestination, ':' );
		if( ptr )
		{
			n = sscanf( ptr + 1, "%d", &port );
			require_action( n == 1, exit, err = kMalformedErr );
			hasPort = 1;
			len = (size_t)( ptr - inDestination );
		}
		else
		{
			len = strlen( inDestination );
		}
		ifindex = kDNSServiceInterfaceIndexAny;
	}
	require_action( len < inNameMaxLen, exit, err = kSizeErr );
	memcpy( inNameBuf, inDestination, len );
	inNameBuf[ len ] = '\0';
	
	*outIndex = ifindex;
	if( hasPort ) *outPort = port;
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AsyncConnection_ConnectSync
//
//	Yes, I realize the irony of a synchronous version of an AsyncConnection object. It's just a helper for legacy code.
//===========================================================================================================================

typedef struct
{
	dispatch_queue_t			queue;
	dispatch_semaphore_t		doneSem;
	OSStatus					error;
	SocketRef					sock;
	
}	AsyncConnection_ConnectSyncContext;

static void	_AsyncConnection_ConnectSyncHandler( SocketRef inSock, OSStatus inError, void *inArg );

OSStatus
	AsyncConnection_ConnectSync( 
		const char *				inDestination, 
		int							inDefaultPort, 
		AsyncConnectionFlags		inFlags, 
		uint64_t					inTimeoutNanos, 
		int							inSocketSendBufferSize, 
		int							inSocketRecvBufferSize, 
		AsyncConnectionProgressFunc	inProgressFunc, 
		void *						inProgressArg, 
		SocketRef *					outSock )
{
	return( AsyncConnection_ConnectSyncEx( inDestination, inDefaultPort, inFlags, inTimeoutNanos, 
		inSocketSendBufferSize, inSocketRecvBufferSize, inProgressFunc, inProgressArg, NULL, NULL, outSock ) );
}

OSStatus
	AsyncConnection_ConnectSyncEx( 
		const char *				inDestination, 
		int							inDefaultPort, 
		AsyncConnectionFlags		inFlags, 
		uint64_t					inTimeoutNanos, 
		int							inSocketSendBufferSize, 
		int							inSocketRecvBufferSize, 
		AsyncConnectionProgressFunc	inProgressFunc, 
		void *						inProgressArg, 
		AsyncConnectionWaitFunc		inWaitCallBack, 
		void *						inWaitContext, 
		SocketRef *					outSock )
{
	AsyncConnection_ConnectSyncContext		context;
	AsyncConnectionRef						connection;
	OSStatus								err;
	Boolean									released;
	dispatch_time_t							timeout;
	
	context.doneSem = NULL;
	context.queue	= NULL;
	require_action( *inDestination != '\0', exit, err = kParamErr );
	
	context.queue = dispatch_queue_create( inDestination, NULL );
	require_action( context.queue, exit, err = kUnknownErr );
	
	context.doneSem = dispatch_semaphore_create( 0 );
	require_action( context.doneSem, exit, err = kUnknownErr );
	
	context.error = kUnknownErr;
	context.sock  = kInvalidSocketRef;
	
	err = AsyncConnection_Connect( &connection, inDestination, inDefaultPort, inFlags, inTimeoutNanos, 
		inSocketSendBufferSize, inSocketRecvBufferSize, inProgressFunc, inProgressArg, 
		_AsyncConnection_ConnectSyncHandler, &context, context.queue );
	require_noerr( err, exit );
	
	released = false;
	if( inWaitCallBack )
	{
		for( ;; )
		{
			timeout = dispatch_time( DISPATCH_TIME_NOW, 250 * UINT64_C_safe( kNanosecondsPerMillisecond ) );
			if( !dispatch_semaphore_wait( context.doneSem, timeout ) ) break;
			
			if( !released )
			{
				err = inWaitCallBack( inWaitContext );
				if( err )
				{
					ac_ulog( kLogLevelNotice, "### Canceling connect to %s\n", inDestination );
					
					released = true;
					AsyncConnection_Release( connection );
				}
			}
		}
	}
	else
	{
		dispatch_semaphore_wait( context.doneSem, DISPATCH_TIME_FOREVER );
	}
	if( !released ) AsyncConnection_Release( connection );
	
	err = context.error;
	require_noerr_quiet( err, exit );
	
	*outSock = context.sock;
	
exit:
	if( context.doneSem ) dispatch_release( context.doneSem );
	if( context.queue )   dispatch_release( context.queue );
	return( err );
}

static void	_AsyncConnection_ConnectSyncHandler( SocketRef inSock, OSStatus inError, void *inArg )
{
	AsyncConnection_ConnectSyncContext * const		context = (AsyncConnection_ConnectSyncContext *) inArg;
	
	context->error = inError;
	if( !inError ) context->sock = inSock;
	dispatch_semaphore_signal( context->doneSem );
}

#if 0
#pragma mark -
#endif

#if( DEBUG )

typedef struct
{
	dispatch_semaphore_t		doneSem;
	OSStatus					error;
	SocketRef					sock;
	
}	AsyncConnection_TestContext;

void	AsyncConnection_TestHandler( SocketRef inSock, OSStatus inError, void *inArg );

//===========================================================================================================================
//	AsyncConnection_Test
//===========================================================================================================================

OSStatus	AsyncConnection_Test( void )
{
	OSStatus						err;
	AsyncConnection_TestContext		context;
	dispatch_queue_t				queue;
	AsyncConnectionRef				connection;
	char							tempStr[ kDNSServiceMaxDomainName + 64 ];
	
	connection		= NULL;
	context.doneSem	= NULL;
	context.sock	= kInvalidSocketRef;
	
	queue = dispatch_queue_create( __ROUTINE__, NULL );
	require_action( queue, exit, err = -1 );
	
	context.doneSem = dispatch_semaphore_create( 0 );
	require_action( context.doneSem, exit, err = -1 );
	
	// IP
	
	context.error = kUnknownErr;
	err = AsyncConnection_Connect( &connection, "http://10.0.20.1", 5009, kAsyncConnectionFlag_P2P, 
		10 * UINT64_C_safe( kNanosecondsPerSecond ),  kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, 
		NULL, NULL, AsyncConnection_TestHandler, &context, queue );
	require_noerr( err, exit );
	dispatch_semaphore_wait( context.doneSem, DISPATCH_TIME_FOREVER );
	err = context.error;
	require_noerr( err, exit );
	ForgetSocket( &context.sock );
	AsyncConnection_Release( connection );
	connection = NULL;
	ac_ulog( kLogLevelVerbose | kLogLevelFlagContinuation, "\n" );
	
	// DNS
	
	context.error = kUnknownErr;
	err = AsyncConnection_Connect( &connection, "www.apple.com", 80, kAsyncConnectionFlag_P2P, 
		10 * UINT64_C_safe( kNanosecondsPerSecond ), kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, 
		NULL, NULL, AsyncConnection_TestHandler, &context, queue );
	require_noerr( err, exit );
	dispatch_semaphore_wait( context.doneSem, DISPATCH_TIME_FOREVER );
	err = context.error;
	require_noerr( err, exit );
	ForgetSocket( &context.sock );
	AsyncConnection_Release( connection );
	connection = NULL;
	ac_ulog( kLogLevelVerbose | kLogLevelFlagContinuation, "\n" );
	
	// Bonjour 1
	
	DNSServiceConstructFullName( tempStr, "bb30home", "_airport._tcp.", "local." );
	context.error = kUnknownErr;
	err = AsyncConnection_Connect( &connection, tempStr, 0, kAsyncConnectionFlag_P2P, 
		10 * UINT64_C_safe( kNanosecondsPerSecond ), kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, 
		NULL, NULL, AsyncConnection_TestHandler, &context, queue );
	require_noerr( err, exit );
	dispatch_semaphore_wait( context.doneSem, DISPATCH_TIME_FOREVER );
	err = context.error;
	require_noerr( err, exit );
	ForgetSocket( &context.sock );
	AsyncConnection_Release( connection );
	connection = NULL;
	ac_ulog( kLogLevelVerbose | kLogLevelFlagContinuation, "\n" );
	
	// Bonjour 2
	
	DNSServiceConstructFullName( tempStr, "Brother HL-5370DW series", "_http._tcp.", "local." );
	context.error = kUnknownErr;
	err = AsyncConnection_Connect( &connection, tempStr, 0, kAsyncConnectionFlag_P2P, 
		10 * UINT64_C_safe( kNanosecondsPerSecond ), kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, 
		NULL, NULL, AsyncConnection_TestHandler, &context, queue );
	require_noerr( err, exit );
	dispatch_semaphore_wait( context.doneSem, DISPATCH_TIME_FOREVER );
	err = context.error;
	require_noerr( err, exit );
	ForgetSocket( &context.sock );
	AsyncConnection_Release( connection );
	connection = NULL;
	ac_ulog( kLogLevelVerbose | kLogLevelFlagContinuation, "\n" );
	
exit:
	if( connection )		AsyncConnection_Release( connection );
	if( context.doneSem )	dispatch_release( context.doneSem );
	if( queue )				dispatch_release( queue );
	dlog( kLogLevelMax, "%###s: %s\n", __ROUTINE__, !err ? "PASSED" : "FAILED" );
	return( err );
}

//===========================================================================================================================
//	AsyncConnection_TestHandler
//===========================================================================================================================

void	AsyncConnection_TestHandler( SocketRef inSock, OSStatus inError, void *inArg )
{
	AsyncConnection_TestContext * const		context = (AsyncConnection_TestContext *) inArg;
	
	context->error = inError;
	context->sock  = inSock;
	dispatch_semaphore_signal( context->doneSem );	
}
#endif // DEBUG
