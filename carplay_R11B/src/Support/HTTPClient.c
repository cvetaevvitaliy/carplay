/*
	File:    	HTTPClient.c
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
	
	Copyright (C) 2011-2014 Apple Inc. All Rights Reserved.
*/

#include "HTTPClient.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "AsyncConnection.h"
#include "HTTPNetUtils.h"
#include "NetUtils.h"

#include CF_RUNTIME_HEADER
#include LIBDISPATCH_HEADER

//===========================================================================================================================
//	Structures
//===========================================================================================================================

typedef enum
{
	kHTTPClientStateIdle			= 0, 
	kHTTPClientStateConnecting		= 1, 
	kHTTPClientStateWritingRequest	= 2, 
	kHTTPClientStateReadingResponse	= 3, 
	kHTTPClientStateWaitingForClose	= 4, 
	kHTTPClientStateError			= 5
	
}	HTTPClientState;

struct HTTPClientPrivate
{
	CFRuntimeBase			base;			// CF type info. Must be first.
	dispatch_queue_t		queue;			// Queue to perform all operations on.
	
	// Configuration
	
	char *					destination;	// Destination hostname, IP address, URL, etc. of the the server to talk to.
	int						defaultPort;	// Default port to connect to if not provided in the destination string.
	HTTPClientFlags			flags;			// Flags affecting operation.
	LogCategory *			ucat;			// Log category to use for logging.
	
	// Networking
	
	HTTPClientState			state;			// State of the client: connecting, reading, writing, etc.
	AsyncConnectionRef		connector;		// Used for connecting. Only non-NULL while connecting.
	SocketRef				sock;			// Socket used for I/O to the server.
	int						sockRefCount;	// Number of references to the socket.
	dispatch_source_t		readSource;		// GCD source for readability notification.
	Boolean					readSuspended;	// True if GCD read source has been suspended.
	dispatch_source_t		writeSource;	// GCD source for writability notification.
	Boolean					writeSuspended;	// True if GCD write source has been suspended.
	dispatch_source_t		timerSource;	// GCD source for handling timeouts.
	
	// Messages
	
	HTTPMessageRef			messageList;	// List of messages to send.
	HTTPMessageRef *		messageNext;	// Ptr to append next message to send.
};

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static void _HTTPClientGetTypeID( void *inContext );
static void	_HTTPClientFinalize( CFTypeRef inCF );
static void	_HTTPClientInvalidate( void *inContext );
static void	_HTTPClientRunStateMachine( HTTPClientRef me );
static void	_HTTPClientSendMessage( void *inContext );
static void	_HTTPClientConnectHandler( SocketRef inSock, OSStatus inError, void *inArg );
static void	_HTTPClientReadHandler( void *inContext );
static void	_HTTPClientWriteHandler( void *inContext );
static void	_HTTPClientCancelHandler( void *inContext );
static void	_HTTPClientErrorHandler( HTTPClientRef me, OSStatus inError );
static void	_HTTPClientCompleteMessage( HTTPClientRef me, HTTPMessageRef inMsg, OSStatus inStatus );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static dispatch_once_t			gHTTPClientInitOnce = 0;
static CFTypeID					gHTTPClientTypeID = _kCFRuntimeNotATypeID;
static const CFRuntimeClass		kHTTPClientClass = 
{
	0,						// version
	"HTTPClient",			// className
	NULL,					// init
	NULL,					// copy
	_HTTPClientFinalize,	// finalize
	NULL,					// equal -- NULL means pointer equality.
	NULL,					// hash  -- NULL means pointer hash.
	NULL,					// copyFormattingDesc
	NULL,					// copyDebugDesc
	NULL,					// reclaim
	NULL					// refcount
};

ulog_define( HTTPClientCore, kLogLevelNotice, kLogFlags_Default, "HTTPClient",  NULL );

//===========================================================================================================================
//	HTTPClientGetTypeID
//===========================================================================================================================

CFTypeID	HTTPClientGetTypeID( void )
{
	dispatch_once_f( &gHTTPClientInitOnce, NULL, _HTTPClientGetTypeID );
	return( gHTTPClientTypeID );
}

static void _HTTPClientGetTypeID( void *inContext )
{
	(void) inContext;
	
	gHTTPClientTypeID = _CFRuntimeRegisterClass( &kHTTPClientClass );
	check( gHTTPClientTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	HTTPClientCreate
//===========================================================================================================================

OSStatus	HTTPClientCreate( HTTPClientRef *outClient )
{
	OSStatus			err;
	HTTPClientRef		me;
	size_t				extraLen;
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (HTTPClientRef) _CFRuntimeCreateInstance( NULL, HTTPClientGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	me->sock = kInvalidSocketRef;
	me->ucat = &log_category_from_name( HTTPClientCore );
	HTTPClientSetDispatchQueue( me, NULL ); // Default to the main queue.
	me->messageNext = &me->messageList;
	
	*outClient = me;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPClientCreateWithSocket
//===========================================================================================================================

OSStatus	HTTPClientCreateWithSocket( HTTPClientRef *outClient, SocketRef inSock )
{
	OSStatus			err;
	HTTPClientRef		client;
	
	err = HTTPClientCreate( &client );
	require_noerr( err, exit );
	
	client->sock = inSock;
	*outClient = client;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_HTTPClientFinalize
//===========================================================================================================================

static void	_HTTPClientFinalize( CFTypeRef inCF )
{
	HTTPClientRef const		me = (HTTPClientRef) inCF;
	
	dispatch_forget( &me->queue );
	ForgetMem( &me->destination );
    check( me->sockRefCount == 0 );
    ForgetSocket( &me->sock );
}

//===========================================================================================================================
//	HTTPClientInvalidate
//===========================================================================================================================

void	HTTPClientInvalidate( HTTPClientRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _HTTPClientInvalidate );
}

static void	_HTTPClientInvalidate( void *inContext )
{
	HTTPClientRef const		me = (HTTPClientRef) inContext;
	
	_HTTPClientErrorHandler( me, kCanceledErr );
	CFRelease( me );
}

//===========================================================================================================================
//	HTTPClientGetPeerAddress
//===========================================================================================================================

OSStatus	HTTPClientGetPeerAddress( HTTPClientRef inClient, void *inSockAddr, size_t inMaxLen, size_t *outLen )
{
	OSStatus		err;
	socklen_t		len;
	
	len = (socklen_t) inMaxLen;
	err = getpeername( inClient->sock, (struct sockaddr *) inSockAddr, &len );
	err = map_socket_noerr_errno( inClient->sock, err );
	require_noerr( err, exit );
	
	if( outLen ) *outLen = len;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPClientSetDestination
//===========================================================================================================================

OSStatus	HTTPClientSetDestination( HTTPClientRef me, const char *inDestination, int inDefaultPort )
{
	OSStatus		err;
	char *			tempStr;
	
	tempStr = strdup( inDestination );
	require_action( tempStr, exit, err = kNoMemoryErr );
	
	if( me->destination ) free( me->destination );
	me->destination = tempStr;
	me->defaultPort = inDefaultPort;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HTTPClientSetDispatchQueue
//===========================================================================================================================

void	HTTPClientSetDispatchQueue( HTTPClientRef me, dispatch_queue_t inQueue )
{
	ReplaceDispatchQueue( &me->queue, inQueue );
}

//===========================================================================================================================
//	HTTPClientSetFlags
//===========================================================================================================================

void	HTTPClientSetFlags( HTTPClientRef me, HTTPClientFlags inFlags, HTTPClientFlags inMask )
{
	me->flags = ( me->flags & ~inMask ) | ( inFlags & inMask );
}

//===========================================================================================================================
//	HTTPClientSetLogging
//===========================================================================================================================

void	HTTPClientSetLogging( HTTPClientRef me, LogCategory *inLogCategory )
{
	me->ucat = inLogCategory;
}

//===========================================================================================================================
//	_HTTPClientRunStateMachine
//===========================================================================================================================

static void	_HTTPClientRunStateMachine( HTTPClientRef me )
{
	OSStatus			err;
	HTTPMessageRef		msg;
	
	for( ;; )
	{
		if( me->state == kHTTPClientStateIdle )
		{
			AsyncConnectionFlags		flags;
			
			msg = me->messageList;
			if( !msg ) break;
			
			if( IsValidSocket( me->sock ) )
			{
				if( me->readSource )
				{
					me->state = kHTTPClientStateWritingRequest;
					continue;
				}
				CFRetain( me );
				_HTTPClientConnectHandler( me->sock, kNoErr, me );
				break;
			}
			
			check( !me->connector );
			flags = 0;
			if( me->flags & kHTTPClientFlag_P2P )				flags |= kAsyncConnectionFlag_P2P;
			if( me->flags & kHTTPClientFlag_SuppressUnusable )	flags |= kAsyncConnectionFlag_SuppressUnusable;
			if( me->flags & kHTTPClientFlag_Reachability )		flags |= kAsyncConnectionFlag_Reachability;
			if( me->flags & kHTTPClientFlag_BoundInterface )	flags |= kAsyncConnectionFlag_BoundInterface;
			
			err = AsyncConnection_Connect( &me->connector, me->destination, me->defaultPort, flags, msg->timeoutNanos, 
				kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, NULL, NULL, _HTTPClientConnectHandler, me, me->queue );
			require_noerr( err, exit );
			
			CFRetain( me );
			me->state = kHTTPClientStateConnecting;
			break;
		}
		else if( me->state == kHTTPClientStateWritingRequest )
		{
			msg = me->messageList;
			err = SocketWriteData( me->sock, &msg->iop, &msg->ion );
			if( err == EWOULDBLOCK )
			{
				dispatch_resume_if_suspended( me->writeSource, &me->writeSuspended );
				break;
			}
			require_noerr_quiet( err, exit );
			LogHTTP( me->ucat, me->ucat, msg->header.buf, msg->header.len, msg->bodyPtr, msg->bodyLen );
			
			HTTPMessageReset( msg );
			me->state = kHTTPClientStateReadingResponse;
		}
		else if( me->state == kHTTPClientStateReadingResponse )
		{
			msg = me->messageList;
			err = HTTPMessageReadMessage( me->sock, msg );
			if( err == EWOULDBLOCK )
			{
				dispatch_resume_if_suspended( me->readSource, &me->readSuspended );
				break;
			}
			require_noerr_quiet( err, exit );
			LogHTTP( me->ucat, me->ucat, msg->header.buf, msg->header.len, msg->bodyPtr, msg->bodyLen );
			
			if( msg->closeAfterRequest )
			{
				// Note: the shutdown() is after the response is read because some servers, such as Akamai's, 
				// treat a TCP half-close as a client abort and will disconnect instead of sending a response.
				
				shutdown( me->sock, SHUT_WR_COMPAT );
				me->state = kHTTPClientStateWaitingForClose;
			}
			else
			{
				_HTTPClientCompleteMessage( me, msg, kNoErr );
				me->state = kHTTPClientStateIdle;
			}
		}
		else if( me->state == kHTTPClientStateWaitingForClose )
		{
			ssize_t		n;
			uint8_t		buf[ 16 ];
			
			n = recv( me->sock, (char *) buf, sizeof( buf ), 0 );
			err = map_socket_value_errno( me->sock, ( n >= 0 ), n );
			if( err == EWOULDBLOCK )
			{
				dispatch_resume_if_suspended( me->readSource, &me->readSuspended );
				break;
			}
			if(  n > 0 )   ulog( me->ucat, kLogLevelNotice, "### Read %zd bytes after connection close\n", n );
			else if( err ) ulog( me->ucat, kLogLevelNotice, "### Error on connection close: %#m\n", err );
			
			msg = me->messageList;
			_HTTPClientCompleteMessage( me, msg, kNoErr );
			_HTTPClientErrorHandler( me, kEndingErr );
			me->state = kHTTPClientStateIdle;
		}
		else if( me->state == kHTTPClientStateError )
		{
			dlogassert( "Running state machine in error state" );
			err = kStateErr;
			goto exit;
		}
		else
		{
			dlogassert( "Bad state %d", me->state );
			err = kInternalErr;
			goto exit;
		}
	}
	err = kNoErr;
	
exit:
	if( err ) _HTTPClientErrorHandler( me, err );
}

//===========================================================================================================================
//	_HTTPClientConnectHandler
//===========================================================================================================================

static void	_HTTPClientConnectHandler( SocketRef inSock, OSStatus inError, void *inArg )
{
	HTTPClientRef const		me = (HTTPClientRef) inArg;
	OSStatus				err;
	
	require_noerr_action_quiet( inError, exit, err = inError );
	
	// Set up a source to notify us when the socket is readable.
	
	me->readSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, inSock, 0, me->queue );
	require_action( me->readSource, exit, err = kUnknownErr );
	++me->sockRefCount;
	CFRetain( me );
	dispatch_set_context( me->readSource, me );
	dispatch_source_set_event_handler_f( me->readSource, _HTTPClientReadHandler );
	dispatch_source_set_cancel_handler_f( me->readSource, _HTTPClientCancelHandler );
	dispatch_resume( me->readSource );
	
	// Set up a source to notify us when the socket is writable.
	
	me->writeSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_WRITE, inSock, 0, me->queue );
	require_action( me->writeSource, exit, err = kUnknownErr );
	++me->sockRefCount;
	CFRetain( me );
	dispatch_set_context( me->writeSource, me );
	dispatch_source_set_event_handler_f( me->writeSource, _HTTPClientWriteHandler );
	dispatch_source_set_cancel_handler_f( me->writeSource, _HTTPClientCancelHandler );
	me->writeSuspended = true; // Don't resume until we get EWOULDBLOCK.
	
	me->sock = inSock;
	me->state = kHTTPClientStateWritingRequest;
	_HTTPClientRunStateMachine( me );
	err = kNoErr;
	
exit:
	if( err )
	{
		if( me->sockRefCount == 0 ) ForgetSocket( &inSock );
		_HTTPClientErrorHandler( me, err );
	}
	CFRelease( me );
}

//===========================================================================================================================
//	_HTTPClientReadHandler
//===========================================================================================================================

static void	_HTTPClientReadHandler( void *inContext )
{
	HTTPClientRef const		me = (HTTPClientRef) inContext;
	
	check( !me->readSuspended );
	dispatch_suspend( me->readSource ); // Disable readability notification until we get another EWOULDBLOCK.
	me->readSuspended = true;
	
	_HTTPClientRunStateMachine( me );
}

//===========================================================================================================================
//	_HTTPClientWriteHandler
//===========================================================================================================================

static void	_HTTPClientWriteHandler( void *inContext )
{
	HTTPClientRef const		me = (HTTPClientRef) inContext;
	
	check( !me->writeSuspended );
	dispatch_suspend( me->writeSource ); // Disable writability notification until we get another EWOULDBLOCK.
	me->writeSuspended = true;
	
	_HTTPClientRunStateMachine( me );
}

//===========================================================================================================================
//	_HTTPClientCancelHandler
//===========================================================================================================================

static void	_HTTPClientCancelHandler( void *inContext )
{
	HTTPClientRef const		me = (HTTPClientRef) inContext;
	
	if( --me->sockRefCount == 0 )
	{
		ForgetSocket( &me->sock );
	}
	CFRelease( me );
}

//===========================================================================================================================
//	_HTTPClientErrorHandler
//===========================================================================================================================

static void	_HTTPClientErrorHandler( HTTPClientRef me, OSStatus inError )
{
	HTTPMessageRef		msg;
	
	me->state = kHTTPClientStateError;
	if( me->connector )
	{
		AsyncConnection_Release( me->connector );
		me->connector = NULL;
	}
	dispatch_source_forget_ex( &me->readSource,  &me->readSuspended );
	dispatch_source_forget_ex( &me->writeSource, &me->writeSuspended );
	dispatch_source_forget( &me->timerSource );
	
	while( ( msg = me->messageList ) != NULL )
	{
		_HTTPClientCompleteMessage( me, msg, inError );
	}
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_HTTPClientCompleteMessage
//===========================================================================================================================

static void	_HTTPClientCompleteMessage( HTTPClientRef me, HTTPMessageRef inMsg, OSStatus inStatus )
{
	if( ( me->messageList = inMsg->next ) == NULL )
		  me->messageNext = &me->messageList;
	inMsg->status = inStatus;
	if( inMsg->completion ) inMsg->completion( inMsg );
	CFRelease( inMsg );
}

//===========================================================================================================================
//	HTTPClientSendMessage
//===========================================================================================================================

OSStatus	HTTPClientSendMessage( HTTPClientRef me, HTTPMessageRef inMsg )
{
	OSStatus		err;
	
	if( inMsg->closeAfterRequest )
	{
		HTTPHeader_SetField( &inMsg->header, "Connection", "close" );
	}
	
	err = HTTPHeader_Commit( &inMsg->header );
	require_noerr( err, exit );
	
	inMsg->iov[ 0 ].iov_base = inMsg->header.buf;
	inMsg->iov[ 0 ].iov_len  = inMsg->header.len;
	inMsg->ion = 1;
	if( inMsg->bodyLen > 0 )
	{
		inMsg->iov[ 1 ].iov_base = (char *) inMsg->bodyPtr;
		inMsg->iov[ 1 ].iov_len  = inMsg->bodyLen;
		inMsg->ion = 2;
	}
	inMsg->iop = inMsg->iov;
	
	CFRetain( inMsg );
	CFRetain( me );
	inMsg->httpContext1 = me;
	dispatch_async_f( me->queue, inMsg, _HTTPClientSendMessage );
	err = kNoErr;
	
exit:
	return( err );
}

static void	_HTTPClientSendMessage( void *inContext )
{
	HTTPMessageRef const		msg = (HTTPMessageRef) inContext;
	HTTPClientRef const			me  = (HTTPClientRef) msg->httpContext1;
	
	msg->next = NULL;
	*me->messageNext = msg;
	 me->messageNext = &msg->next;
	_HTTPClientRunStateMachine( me );
	CFRelease( me );
}

//===========================================================================================================================
//	HTTPClientSendMessageSync
//===========================================================================================================================

static void	_HTTPClientSendMessageSyncCompletion( HTTPMessageRef inMsg );

OSStatus	HTTPClientSendMessageSync( HTTPClientRef me, HTTPMessageRef inMsg )
{
	OSStatus					err;
	dispatch_semaphore_t		sem;
	
	sem = dispatch_semaphore_create( 0 );
	require_action( sem, exit, err = kNoMemoryErr );
	
	inMsg->httpContext2 = sem;
	inMsg->completion   = _HTTPClientSendMessageSyncCompletion;
	
	err = HTTPClientSendMessage( me, inMsg );
	require_noerr( err, exit );
	
	dispatch_semaphore_wait( sem, DISPATCH_TIME_FOREVER );
	err = inMsg->status;
	if( !err && !IsHTTPStatusCode_Success( inMsg->header.statusCode ) )
	{
		err = HTTPStatusToOSStatus( inMsg->header.statusCode );
	}
	
exit:
	if( sem ) dispatch_release( sem );
	return( err );
}

static void	_HTTPClientSendMessageSyncCompletion( HTTPMessageRef inMsg )
{
	dispatch_semaphore_signal( (dispatch_semaphore_t) inMsg->httpContext2 );
}

#if 0
#pragma mark -
#endif

