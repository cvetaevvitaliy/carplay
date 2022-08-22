/*
	File:    	NetPerf.c
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
	
	Copyright (C) 2012-2014 Apple Inc. All Rights Reserved.
*/

#include "NetPerf.h"

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFUtils.h"
#include "HTTPClient.h"
#include "HTTPMessage.h"
#include "MathUtils.h"
#include "NetUtils.h"
#include "TickUtils.h"
#include "TimeUtils.h"

#include CF_RUNTIME_HEADER
#include LIBDISPATCH_HEADER

//===========================================================================================================================
//	Constants
//===========================================================================================================================

#define kNetPerfPort_Control		5000 // TCP port for the HTTP server for control.
#define kNetPerfPort_DataClient		6100 // Base TCP or UDP port for receiving ACKs from a performance testing server.
#define kNetPerfPort_DataServer		6200 // Base TCP or UDP port for receiving data from a performance testing client.

#define kNetPerfURL					"/perf"

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static void		_NetPerfFinalize( CFTypeRef inCF );

static OSStatus	_NetPerfClientStart( NetPerfRef me, CFDictionaryRef inParams, CFDictionaryRef *outResponse );
static void		_NetPerfClientStop( NetPerfRef me );
static void		_NetPerfClientRunStateMachine( NetPerfRef me );
static void		_NetPerfClientErrorHandler( NetPerfRef me, OSStatus inError );
static OSStatus	_NetPerfClientSendControlMessage( NetPerfRef me, CFStringRef inCommand, CFDictionaryRef inParams );
static void		_NetPerfClientHandleControlCompletion( HTTPMessageRef inMsg );
static OSStatus	_NetPerfClientHandleSetUpServerCompletion( NetPerfRef me, CFDictionaryRef inResponse );
static OSStatus	_NetPerfClientStartData( NetPerfRef me );
static void *	_NetPerfClientReceiveThread( void *inArg );
static void		_NetPerfClientReceivePacket( NetPerfRef me, SocketRef inSock );
static void *	_NetPerfClientSendThread( void *inArg );

static OSStatus	_NetPerfServerSessionSetUp( NetPerfRef me, CFDictionaryRef inParams, CFDictionaryRef *outResponse );
static void		_NetPerfServerSessionTearDown( NetPerfRef me );
static void *	_NetPerfServerDataThread( void *inArg );
static OSStatus	_NetPerfServerDataHandler( NetPerfRef me, SocketRef inSock );

static OSStatus	_PostEventF( NetPerfRef me, uint32_t inType, const char *inFormat, ... );
static void		_PostEventOnEventQueue( void *inArg );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

CF_CLASS_DEFINE( NetPerf );

//===========================================================================================================================
//	NetPerfCreate
//===========================================================================================================================

OSStatus	NetPerfCreate( NetPerfRef *outPerf )
{
	OSStatus		err;
	NetPerfRef		me;
	
	CF_OBJECT_CREATE( NetPerf, me, err, exit );
	
	me->clientDataCmdSock = kInvalidSocketRef;
	me->clientDataSock    = kInvalidSocketRef;
	
	me->serverDataCmdSock = kInvalidSocketRef;
	me->serverDataSockV4  = kInvalidSocketRef;
	me->serverDataSockV6  = kInvalidSocketRef;
	
	me->queue = dispatch_queue_create( "NetPerfControl", NULL );
	require_action( me->queue, exit, err = kNoMemoryErr );
	
	ReplaceDispatchQueue( &me->eventQueue, NULL );
	
	*outPerf = me;
	me = NULL;
	err = kNoErr;
	
exit:
	if( me ) CFRelease( me );
	return( err );
}

//===========================================================================================================================
//	_NetPerfFinalize
//===========================================================================================================================

static void	_NetPerfFinalize( CFTypeRef inCF )
{
	NetPerfRef const		me = (NetPerfRef) inCF;
	
	dispatch_forget( &me->queue );
	dispatch_forget( &me->eventQueue );
	ForgetCF( &me->config );
	
	dlog( kLogLevelNotice, "NetPerf finalized\n" );
}

//===========================================================================================================================
//	NetPerfSetEventHandler
//===========================================================================================================================

void	NetPerfSetEventHandler( NetPerfRef me, NetPerfEventHandlerFunc inHandler, void *inContext, dispatch_queue_t inQueue )
{
	me->eventHandler = inHandler;
	me->eventContext = inContext;
	ReplaceDispatchQueue( &me->eventQueue, inQueue );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_NetPerfControl
//===========================================================================================================================

OSStatus
	_NetPerfControl( 
		CFTypeRef			inObject, 
		CFObjectFlags		inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outResponse )
{
	NetPerfRef const		me = (NetPerfRef) inObject;
	OSStatus				err;
	
	(void) inFlags;
	(void) inQualifier;
	
	if( 0 ) {} // Empty if it simplify else if's below.
	
	else if( CFStringCompare( inCommand, kNetPerfCommand_StartClientSession, 0 ) == 0 )
	{
		err = _NetPerfClientStart( me, inParams, outResponse );
		require_noerr( err, exit );
	}
	else if( CFStringCompare( inCommand, kNetPerfCommand_StopClientSession, 0 ) == 0 )
	{
		_NetPerfClientStop( me );
	}
	else if( CFStringCompare( inCommand, kNetPerfCommand_SetUpServerSession, 0 ) == 0 )
	{
		err = _NetPerfServerSessionSetUp( me, inParams, outResponse );
		require_noerr( err, exit );
	}
	else if( CFStringCompare( inCommand, kNetPerfCommand_TearDownServerSession, 0 ) == 0 )
	{
		_NetPerfServerSessionTearDown( me );
	}
	else if( CFStringCompare( inCommand, kNetPerfCommand_StartServerSession, 0 ) == 0 )
	{
		dlog( kLogLevelNotice, "NetPerf server starting\n" );
		require_action( me->serverSessionSetUp, exit, err = kNotPreparedErr );
		require_action( !me->serverSessionStarted, exit, err = kAlreadyInUseErr );
		me->serverSessionStarted = true;
		dlog( kLogLevelNotice, "NetPerf server started\n" );
	}
	else
	{
		dlogassert( "Unsupported command %@", inCommand );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_NetPerfCopyProperty
//===========================================================================================================================

CFTypeRef
	_NetPerfCopyProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr )
{
	NetPerfRef const		me = (NetPerfRef) inObject;
	CFTypeRef				value;
	OSStatus				err;
	
	(void) me;
	(void) inFlags;
	(void) inProperty;
	(void) inQualifier;
	
	value = NULL;
	if( 0 ) {} // Empty if it simplify else if's below.
	
	// Other
	
	else
	{
		dlogassert( "Get of unsupported property %@", inProperty );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_NetPerfSetProperty
//===========================================================================================================================

OSStatus
	_NetPerfSetProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue )
{
	NetPerfRef const		me = (NetPerfRef) inObject;
	OSStatus				err;
	
	(void) me;
	(void) inFlags;
	(void) inProperty;
	(void) inQualifier;
	(void) inValue;
	
	if( 0 ) {} // Empty if it simplify else if's below.
	
	// Other
	
	else
	{
		dlogassert( "Set of unsupported property %@", inProperty );
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_NetPerfClientStart
//===========================================================================================================================

static OSStatus	_NetPerfClientStart( NetPerfRef me, CFDictionaryRef inParams, CFDictionaryRef *outResponse )
{
	OSStatus		err;
	char *			tempStr;
	
	(void) outResponse;
	
	dlog( kLogLevelNotice, "NetPerf client starting\n" );
	require_action( me->clientState == kNetPerfClientState_Stopped, exit2, err = kAlreadyInUseErr );
	require_action( inParams, exit, err = kParamErr );
	
	me->minRTT				= DBL_MAX;
	me->maxRTT				= DBL_MIN;
	me->avgRTT				= -1;
	me->nextLogTicks		= 0;
	me->statCount			= 0;
	me->logIntervalTicks	= SecondsToUpTicks( 1 );
	
	me->packetMaxLen = 1500;
	me->packetBuffer = (uint8_t *) calloc( 1, me->packetMaxLen );
	require_action( me->packetBuffer, exit, err = kNoMemoryErr );
	
	err = HTTPClientCreate( &me->httpClient );
	require_noerr( err, exit );
	
	HTTPClientSetDispatchQueue( me->httpClient, me->queue );
	HTTPClientSetFlags( me->httpClient, kHTTPClientFlag_P2P, kHTTPClientFlag_P2P );
	
	tempStr = CFDictionaryCopyCString( inParams, kNetPerfKey_Destination, &err );
	require_noerr( err, exit );
	err = HTTPClientSetDestination( me->httpClient, tempStr, kNetPerfPort_Control );
	free( tempStr );
	require_noerr( err, exit );
	
	err = HTTPMessageCreate( &me->httpMessage );
	require_noerr( err, exit );
	me->httpMessage->userContext1 = me;
	me->httpMessage->completion = _NetPerfClientHandleControlCompletion;
	
	dlog( kLogLevelNotice, "NetPerf client started\n" );
	
	CFRetain( me );
	me->clientState = kNetPerfClientState_Starting;
	_NetPerfClientRunStateMachine( me );
	err = kNoErr;
	
exit:
	if( err ) _NetPerfClientStop( me );
	
exit2:
	return( err );
}

//===========================================================================================================================
//	_NetPerfClientStop
//===========================================================================================================================

static void	_NetPerfClientStop( NetPerfRef me )
{
	OSStatus		err;
	
	DEBUG_USE_ONLY( err );
	
	dlog( kLogLevelNotice, "NetPerf client stopping\n" );
	
	if( me->clientSendThreadPtr )
	{
		err = SendSelfConnectedLoopbackMessage( me->clientDataCmdSock, "q", 1 );
		check_noerr( err );
		
		err = pthread_join( me->clientSendThread, NULL );
		check_noerr( err );
		
		me->clientSendThreadPtr = NULL;
	}
	if( me->clientReceiveThreadPtr )
	{
		err = SendSelfConnectedLoopbackMessage( me->clientDataCmdSock, "q", 1 );
		check_noerr( err );
		
		err = pthread_join( me->clientReceiveThread, NULL );
		check_noerr( err );
		
		me->clientReceiveThreadPtr = NULL;
	}
	if( me->httpClient )
	{
		HTTPClientInvalidate( me->httpClient );
		CFRelease( me->httpClient );
		me->httpClient = NULL;
	}
	ForgetCF( &me->httpMessage );
	ForgetCF( &me->clientResponse );
	ForgetSocket( &me->clientDataCmdSock );
	ForgetSocket( &me->clientDataSock );
	ForgetMem( &me->packetBuffer );
	
	if( me->clientState != kNetPerfClientState_Stopped )
	{
		_PostEventF( me, kNetPerfEvent_Stopped, NULL );
		me->clientState = kNetPerfClientState_Stopped;
		CFRelease( me );
	}
	
	dlog( kLogLevelNotice, "NetPerf client stopped\n" );
}

//===========================================================================================================================
//	_NetPerfClientRunStateMachine
//===========================================================================================================================

static void	_NetPerfClientRunStateMachine( NetPerfRef me )
{
	OSStatus		err;
	
	for( ;; )
	{
		if( me->clientState == kNetPerfClientState_Starting )
		{
			dlog( kLogLevelNotice, "NetPerf client setting up server\n" );
			err = _NetPerfClientSendControlMessage( me, kNetPerfCommand_SetUpServerSession, NULL );
			require_noerr( err, exit );
			me->clientState = kNetPerfClientState_SettingUpServer;
			break;
		}
		else if( me->clientState == kNetPerfClientState_SettingUpServer )
		{
			err = _NetPerfClientHandleSetUpServerCompletion( me, me->clientResponse );
			require_noerr( err, exit );
			
			dlog( kLogLevelNotice, "NetPerf client starting server\n" );
			err = _NetPerfClientSendControlMessage( me, kNetPerfCommand_StartServerSession, NULL );
			require_noerr( err, exit );
			me->clientState = kNetPerfClientState_StartingServer;
			break;
		}
		else if( me->clientState == kNetPerfClientState_StartingServer )
		{
			dlog( kLogLevelNotice, "NetPerf client starting data\n" );
			err = _NetPerfClientStartData( me );
			require_noerr( err, exit );
			me->clientState = kNetPerfClientState_Running;
		}
		else if( me->clientState == kNetPerfClientState_Running )
		{
			dlog( kLogLevelNotice, "NetPerf client running\n" );
			break;
		}
		else
		{
			dlogassert( "Bad state %d", me->clientState );
			err = kInternalErr;
			goto exit;
		}
	}
	err = kNoErr;
	
exit:
	if( err ) _NetPerfClientErrorHandler( me, err );
}

//===========================================================================================================================
//	_NetPerfClientErrorHandler
//===========================================================================================================================

static void	_NetPerfClientErrorHandler( NetPerfRef me, OSStatus inError )
{
	(void) me;
	(void) inError;
}

//===========================================================================================================================
//	_NetPerfClientSendControlMessageMessage
//===========================================================================================================================

static OSStatus	_NetPerfClientSendControlMessage( NetPerfRef me, CFStringRef inCommand, CFDictionaryRef inParams )
{
	HTTPMessageRef const		msg = me->httpMessage;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDataRef					data;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, kNetPerfKey_ControlCommand, inCommand );
	if( inParams ) CFDictionarySetValue( request, kNetPerfKey_ControlParams, inParams );
	
	data = CFPropertyListCreateData( NULL, request, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	CFRelease( request );
	require_action( data, exit, err = kUnknownErr );
	
	HTTPHeader_InitRequest( &msg->header, "POST", kNetPerfURL, "HTTP/1.1" );
	err = HTTPMessageSetBody( msg, kMIMEType_AppleBinaryPlist, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	err = HTTPClientSendMessage( me->httpClient, msg );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_NetPerfClientHandleControlCompletion
//===========================================================================================================================

static void	_NetPerfClientHandleControlCompletion( HTTPMessageRef inMsg )
{
	NetPerfRef const		me = (NetPerfRef) inMsg->userContext1;
	OSStatus				err;
	
	require_noerr_action_quiet( inMsg->status, exit, err = inMsg->status );
	
	ForgetCF( &me->clientResponse );
	if( inMsg->bodyLen > 0 )
	{
		CFDataRef		data;
		
		data = CFDataCreate( NULL, inMsg->bodyPtr, (CFIndex) inMsg->bodyLen );
		require_action( data, exit, err = kNoMemoryErr );
		
		me->clientResponse = (CFDictionaryRef) CFPropertyListCreateWithData( NULL, data, 0, NULL, NULL );
		CFRelease( data );
		require_action( me->clientResponse, exit, err = kUnknownErr );
		require_action( CFIsType( me->clientResponse, CFDictionary ), exit, err = kTypeErr );
	}
	
	_NetPerfClientRunStateMachine( me );
	err = kNoErr;
	
exit:
	if( err ) _NetPerfClientErrorHandler( me, err );
}

//===========================================================================================================================
//	_NetPerfClientHandleSetUpServerCompletion
//===========================================================================================================================

static OSStatus	_NetPerfClientHandleSetUpServerCompletion( NetPerfRef me, CFDictionaryRef inResponse )
{
	OSStatus		err;
	int				port;
	sockaddr_ip		sip;
	size_t			len;
	
	err = HTTPClientGetPeerAddress( me->httpClient, &sip, sizeof( sip ), &len );
	require_noerr( err, exit );
	
	err = ServerSocketOpen( sip.sa.sa_family, SOCK_DGRAM, IPPROTO_UDP, -kNetPerfPort_DataClient, NULL, 
		kSocketBufferSize_DontSet, &me->clientDataSock );
	require_noerr( err, exit );
	
	SocketSetP2P( me->clientDataSock, true );
	SocketSetPacketTimestamps( me->clientDataSock, true );
	
	require_action( inResponse, exit, err = kParamErr );
	port = (int) CFDictionaryGetInt64( inResponse, kNetPerfKey_Port, &err );
	require_noerr( err, exit );
	
	SockAddrSetPort( &sip, port );
	err = connect( me->clientDataSock, &sip.sa, (socklen_t) len );
	err = map_socket_noerr_errno( me->clientDataSock, err );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_NetPerfClientStartData
//===========================================================================================================================

static OSStatus	_NetPerfClientStartData( NetPerfRef me )
{
	OSStatus		err;
	
	err = OpenSelfConnectedLoopbackSocket( &me->clientDataCmdSock );
	require_noerr( err, exit );
	
	err = pthread_create( &me->clientReceiveThread, NULL, _NetPerfClientReceiveThread, me );
	require_noerr( err, exit );
	me->clientReceiveThreadPtr = &me->clientReceiveThread;
	
	err = pthread_create( &me->clientSendThread, NULL, _NetPerfClientSendThread, me );
	require_noerr( err, exit );
	me->clientSendThreadPtr = &me->clientSendThread;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_NetPerfClientReceiveThread
//===========================================================================================================================

static void *	_NetPerfClientReceiveThread( void *inArg )
{
	NetPerfRef const		me			= (NetPerfRef) inArg;
	SocketRef const			dataSock	= me->clientDataSock;
	SocketRef const			cmdSock		= me->clientDataCmdSock;
	fd_set					readSet;
	int						maxFd;
	int						n;
	OSStatus				err;
	
	SetCurrentThreadPriority( 62 );
	
	FD_ZERO( &readSet );
	maxFd = -1;
	if( dataSock > maxFd ) maxFd = dataSock;
	if( cmdSock  > maxFd ) maxFd = cmdSock;
	maxFd += 1;
	for( ;; )
	{
		FD_SET( dataSock, &readSet );
		FD_SET( cmdSock,  &readSet );
		n = select( maxFd, &readSet, NULL, NULL, NULL );
		err = select_errno( n );
		if( err == EINTR ) continue;
		if( err ) { dlogassert( "Client data thread select() error: %#m", err ); sleep( 1 ); continue; }
		
		if( FD_ISSET( dataSock, &readSet ) ) _NetPerfClientReceivePacket( me, dataSock );
		if( FD_ISSET( cmdSock,  &readSet ) ) break; // The only event is quit so break if anything is pending.
	}
	return( NULL );
}

//===========================================================================================================================
//	_NetPerfClientReceivePacket
//===========================================================================================================================

static void	_NetPerfClientReceivePacket( NetPerfRef me, SocketRef inSock )
{
	NetPerfUDPHeader * const		pkt = (NetPerfUDPHeader *) me->packetBuffer;
	OSStatus						err;
	size_t							len;
	uint64_t						ticks;
	uint64_t						t1, t2, t3, t4;
	double							rtt, fullRTT;
	Boolean							bad;
	
	err = SocketRecvFrom( inSock, pkt, me->packetMaxLen, &len, NULL, 0, NULL, &ticks, NULL, NULL );
	require_noerr( err, exit );
	++me->statCount;
	
	// Calculate the relative offset between clocks.
	//
	// Client:  T1           T4
	// ----------------------------->
	//           \           ^
	//            \         /
	//             v       /
	// ----------------------------->
	// Server:     T2     T3
	// 
	// Clock offset in seconds		= ((T2 - T1) + (T3 - T4)) / 2.
	// Round-trip delay in seconds	=  (T4 - T1) - (T3 - T2)
	
	t1 = ntoh64( pkt->originateTime );
	t2 = ntoh64( pkt->receiveTime );
	t3 = ntoh64( pkt->transmitTime );
	t4 = UpTicksToNTP( ticks );
	
	fullRTT = ( (double)( (int64_t)( t4 - t1 ) ) ) * kNTPFraction;
	rtt = fullRTT - ( ( (double)( (int64_t)( t3 - t2 ) ) ) * kNTPFraction );
	if( rtt < me->minRTT )	me->minRTT = rtt;
	if( rtt > me->maxRTT )	me->maxRTT = rtt;
	if( me->avgRTT < 0 )	me->avgRTT = rtt;
	else					me->avgRTT = MovingAverageF( me->avgRTT, rtt, 0.1 );
	bad = ( rtt > ( 3 * me->avgRTT ) );
	
	if( bad || ( ticks >= me->nextLogTicks ) )
	{
		fprintf( stdout, "%4llu: Min RTT %10f ms, Max RTT %10f, Avg RTT %10f, Recent RTT %10f Lost %u%s\n", 
			me->statCount, 1000 * me->minRTT, 1000 * me->maxRTT, 1000 * me->avgRTT, 1000 * rtt, ntohl( pkt->lost ),
			bad ? " (bad)" : "" );
		if( !bad ) me->nextLogTicks = ticks + me->logIntervalTicks;
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_NetPerfClientSendThread
//===========================================================================================================================

static void *	_NetPerfClientSendThread( void *inArg )
{
	NetPerfRef const		me			= (NetPerfRef) inArg;
	SocketRef const			dataSock	= me->clientDataSock;
	OSStatus				err;
	uint64_t				nextSendTicks, intervalTicks;
	NetPerfUDPHeader		pkt;
	uint32_t				seqNum;
	ssize_t					n;
	
	SetCurrentThreadPriority( 62 );
	
	nextSendTicks = UpTicks();
	intervalTicks = UpTicksPerSecond() / 60;
	seqNum = 1;
	for( ;; )
	{
		pkt.seqNum			= htonl( seqNum );
		pkt.flags			= 0;
		pkt.originateTime	= 0;
		pkt.receiveTime		= 0;
		pkt.transmitTime	= UpNTP();
		pkt.transmitTime	= hton64( pkt.transmitTime );
		n = send( dataSock, &pkt, sizeof( pkt ), 0 );
		err = map_socket_value_errno( dataSock, n == ( (ssize_t) sizeof( pkt ) ), n );
		require_noerr( err, exit );
		
		seqNum += 1;
		nextSendTicks += intervalTicks;
		SleepUntilUpTicks( nextSendTicks );
	}
	
exit:
	return( NULL );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_NetPerfServerSessionSetUp
//===========================================================================================================================

static OSStatus	_NetPerfServerSessionSetUp( NetPerfRef me, CFDictionaryRef inParams, CFDictionaryRef *outResponse )
{
	OSStatus					err;
	CFMutableDictionaryRef		response = NULL;
	int							port;
	
	(void) inParams;
	
	dlog( kLogLevelNotice, "NetPerf server setting up\n" );
	require_action( !me->serverSessionSetUp, exit, err = kAlreadyInitializedErr );
	
	// Set up the data path.
	
	me->lastSeqNum		= 0;
	me->statCount		= 0;
	me->statOutOfOrder	= 0;
	me->statLost		= 0;
	me->statDup			= 0;
	
	me->packetMaxLen = 1500;
	me->packetBuffer = (uint8_t *) calloc( 1, me->packetMaxLen );
	require_action( me->packetBuffer, exit, err = kNoMemoryErr );
	
	err = UDPServerSocketPairOpen( 0, &port, &me->serverDataSockV4, &me->serverDataSockV6 );
	require_noerr( err, exit );
	if( IsValidSocket( me->serverDataSockV4 ) )
	{
		SocketSetP2P( me->serverDataSockV4, true );
		SocketSetPacketTimestamps( me->serverDataSockV4, true );
	}
	if( IsValidSocket( me->serverDataSockV6 ) )
	{
		SocketSetP2P( me->serverDataSockV6, true );
		SocketSetPacketTimestamps( me->serverDataSockV6, true );
	}
	
	err = OpenSelfConnectedLoopbackSocket( &me->serverDataCmdSock );
	require_noerr( err, exit );
	
	err = pthread_create( &me->serverDataThread, NULL, _NetPerfServerDataThread, me );
	require_noerr( err, exit );
	me->serverDataThreadPtr = &me->serverDataThread;
	
	// Report how the server is set up.
	
	response = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( response, exit, err = kNoMemoryErr );
	
	CFDictionarySetInt64( response, kNetPerfKey_Port, port );
	
	me->serverSessionSetUp = true;
	CFRetain( me );
	*outResponse = response;
	response = NULL;
	dlog( kLogLevelNotice, "NetPerf server set up\n" );
	
exit:
	if( response )	CFRelease( response );
	if( err )		_NetPerfServerSessionTearDown( me );
	return( err );
}

//===========================================================================================================================
//	_NetPerfServerSessionTearDown
//===========================================================================================================================

static void	_NetPerfServerSessionTearDown( NetPerfRef me )
{
	OSStatus		err;
	
	DEBUG_USE_ONLY( err );
	
	dlog( kLogLevelNotice, "NetPerf server tearing down\n" );
	if( me->serverDataThreadPtr )
	{
		err = SendSelfConnectedLoopbackMessage( me->serverDataCmdSock, "q", 1 );
		check_noerr( err );
		
		err = pthread_join( me->serverDataThread, NULL );
		check_noerr( err );
		
		me->serverDataThreadPtr = NULL;
	}
	ForgetSocket( &me->serverDataCmdSock );
	ForgetSocket( &me->serverDataSockV4 );
	ForgetSocket( &me->serverDataSockV6 );
	ForgetMem( &me->packetBuffer );
	if( me->serverSessionSetUp )
	{
		me->serverSessionSetUp = false;
		CFRelease( me );
	}
	dlog( kLogLevelNotice, "NetPerf server torn down\n" );
}

//===========================================================================================================================
//	_NetPerfServerDataThread
//===========================================================================================================================

static void *	_NetPerfServerDataThread( void *inArg )
{
	NetPerfRef const		me			= (NetPerfRef) inArg;
	SocketRef const			dataSockV4	= me->serverDataSockV4;
	SocketRef const			dataSockV6	= me->serverDataSockV6;
	SocketRef const			cmdSock		= me->serverDataCmdSock;
	fd_set					readSet;
	int						maxFd;
	int						n;
	OSStatus				err;
	
	SetCurrentThreadPriority( 62 );
	
	FD_ZERO( &readSet );
	maxFd = -1;
	if( dataSockV4 > maxFd ) maxFd = dataSockV4;
	if( dataSockV6 > maxFd ) maxFd = dataSockV6;
	if( cmdSock    > maxFd ) maxFd = cmdSock;
	maxFd += 1;
	for( ;; )
	{
		if( IsValidSocket( dataSockV4 ) ) FD_SET( dataSockV4, &readSet );
		if( IsValidSocket( dataSockV6 ) ) FD_SET( dataSockV6, &readSet );
		FD_SET( cmdSock,  &readSet );
		n = select( maxFd, &readSet, NULL, NULL, NULL );
		err = select_errno( n );
		if( err == EINTR ) continue;
		if( err ) { dlogassert( "Server data thread select() error: %#m", err ); sleep( 1 ); continue; }
		
		if( IsValidSocket( dataSockV4 ) && FD_ISSET( dataSockV4, &readSet ) ) _NetPerfServerDataHandler( me, dataSockV4 );
		if( IsValidSocket( dataSockV6 ) && FD_ISSET( dataSockV6, &readSet ) ) _NetPerfServerDataHandler( me, dataSockV6 );
		if( FD_ISSET( cmdSock,  &readSet ) ) break; // The only event is quit so break if anything is pending.
	}
	return( NULL );
}

//===========================================================================================================================
//	_NetPerfServerDataHandler
//===========================================================================================================================

static OSStatus	_NetPerfServerDataHandler( NetPerfRef me, SocketRef inSock )
{
	NetPerfUDPHeader * const		pkt = (NetPerfUDPHeader *) me->packetBuffer;
	OSStatus						err;
	size_t							len;
	sockaddr_ip						sip;
	size_t							sipLen;
	uint64_t						ticks;
	uint64_t						receiveTime;
	size_t							n;
	uint32_t						seqCurr, seqNext, seqLoss;
	
	err = SocketRecvFrom( inSock, pkt, me->packetMaxLen, &len, &sip, sizeof( sip ), &sipLen, &ticks, NULL, NULL );
	require_noerr( err, exit );
	receiveTime = UpTicksToNTP( ticks );
	
	seqCurr = ntohl( pkt->seqNum );
	seqNext = me->lastSeqNum + 1;
	if( seqCurr == seqNext )
	{
		me->lastSeqNum = seqCurr;
	}
	else if( Mod32_GT( seqCurr, seqNext ) )
	{
		seqLoss = seqCurr - seqNext;
		me->statLost += seqLoss;
		me->lastSeqNum = seqCurr;
	}
	else if( seqCurr == me->lastSeqNum )
	{
		me->statDup += 1;
	}
	else
	{
		me->statOutOfOrder += 1;
	}
	me->statCount += 1;
	
	pkt->originateTime	= pkt->transmitTime;
	pkt->receiveTime	= hton64( receiveTime );
	pkt->transmitTime	= UpNTP();
	pkt->transmitTime	= hton64( pkt->transmitTime );
	pkt->lost			= htonl( me->statLost );
	pkt->outOfOrder		= htonl( me->statOutOfOrder );
	pkt->dup			= htonl( me->statDup );
	
	n = sendto( inSock, pkt, sizeof( *pkt ), 0, &sip.sa, (socklen_t) sipLen );
	err = map_socket_value_errno( inSock, n == (ssize_t) sizeof( *pkt ), n );
	require_noerr( err, exit );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_PostEventF
//===========================================================================================================================

typedef struct
{
	uint32_t					type;
	CFDictionaryRef				details;
	NetPerfEventHandlerFunc		handler;
	void *						context;
	
}	PostEventParams;

static OSStatus	_PostEventF( NetPerfRef me, uint32_t inType, const char *inFormat, ... )
{
	OSStatus					err;
	va_list						args;
	CFMutableDictionaryRef		details = NULL;
	PostEventParams *			params;
	
	if( !me->eventHandler ) { err = kNoErr; goto exit; }
	
	if( inFormat )
	{
		va_start( args, inFormat );
		err = CFPropertyListCreateFormattedVAList( NULL, &details, inFormat, args );
		va_end( args );
		require_noerr( err, exit );
	}
	
	params = (PostEventParams *) malloc( sizeof( *params ) );
	require_action( params, exit, err = kNoMemoryErr );
	
	params->type	= inType;
	params->details	= details;
	params->handler	= me->eventHandler;
	params->context	= me->eventContext;
	
	dispatch_async_f( me->eventQueue, params, _PostEventOnEventQueue );
	static_analyzer_malloc_freed( params );
	details = NULL;
	err = kNoErr;
	
exit:
	if( details ) CFRelease( details );
	return( err );
}

static void	_PostEventOnEventQueue( void *inArg )
{
	PostEventParams * const		params = (PostEventParams *) inArg;
	
	params->handler( params->type, params->details, params->context );
	if( params->details ) CFRelease( params->details );
	free( params );
}
