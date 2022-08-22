/*
	File:    	AirPlayNTPClient.c
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

#include "AirPlayNTPClient.h"

#include <errno.h>
#include <float.h>
#include <math.h>

#include "AirPlayCommon.h"
#include "AtomicUtils.h"
#include "MathUtils.h"
#include "NetUtils.h"
#include "NTPUtils.h"
#include "TickUtils.h"

#include LIBDISPATCH_HEADER

//===========================================================================================================================
//	Internals
//===========================================================================================================================

#define kAirPlayNTP_MaxSlew		INT64_C( 4294967296 ) // 100 ms = (100 * 2^32) / 1000 = 429496729.6 rounded down.

struct AirPlayNTPClient
{
	dispatch_queue_t		internalQueue;
	sockaddr_ip				serverAddr;
	sockaddr_ip				selfAddr;
	SocketRef				ntpSocket;
	dispatch_source_t		ntpSource;
	dispatch_source_t		timerSource;
	Boolean					qosDisabled;
	
	// NTP protocol.
	
	uint64_t				ntpNextRequestUpTicks;	// Next UpTicks when we should send an NTP request.
	uint32_t				ntpLastTransmitTimeHi;	// Upper 32 bits of transmit time of last NTP request we sent.
	uint32_t				ntpLastTransmitTimeLo;	// Lower 32 bits of transmit time of last NTP request we sent.	
	uint32_t				ntpRequestCount;		// Number of NTP requests we've received.
	uint32_t				ntpResponseCount;		// Number of valid NTP responses we've received.
	Boolean					ntpResetStats;			// True if the clock offset stats need to be reset.
	uint32_t				ntpStepCount;			// Number of times the clock had to be stepped.
	
	double					rttMin;					// Min round-trip time.
	double					rttMax;					// Max round-trip time.
	double					rttAvg;					// Avg round-trip time.
	double					rttLeastBad;			// Best RTT we've received that was still considered "bad".
	uint32_t				rttBadCount;			// Number of consecutive bad RTT's we've received.
	
	double					clockOffsets[ 3 ];		// Last N NTP clock offsets between us and the server.
	size_t					clockOffsetIndex;		// Circular index into the clock offset history array.
	double					clockOffsetMin;			// Minimum clock offset.
	double					clockOffsetMax;			// Maximum clock offset.
	double					clockOffsetAvg;			// Moving average of clock offsets.
	double					clockOffsetLeastBad;	// Best clock offset we've received that was still considered "bad".
	
	// Local clock.
	
	volatile uint32_t		localGeneration;		// Generation number for detecting changes.
	Time96					localTime;				// Current time as of the last update.
	uint32_t				localLastCount;			// Last raw value from the clock hardware.
	uint64_t				localFrequency;			// Number of ticks per second of the local clock.
	uint64_t				localScale;				// 1/2^64 units to scale UpTicks to the disciplined rate.
};

DEBUG_STATIC void		_AirPlayNTPClient_Delete( void *inContext );
DEBUG_STATIC void		_AirPlayNTPClient_Finalizer( void *inContext );

DEBUG_STATIC void		_AirPlayNTPClient_Stop( AirPlayNTPClientRef inClient );
DEBUG_STATIC OSStatus	_AirPlayNTPClient_Negotiate( AirPlayNTPClientRef inClient );
DEBUG_STATIC OSStatus	_AirPlayNTPClient_SetupSocket( AirPlayNTPClientRef inClient );

DEBUG_STATIC void		_AirPlayNTPClient_ReadHandler( void *inContext );
DEBUG_STATIC void		_AirPlayNTPClient_CancelHandler( void *inContext );
DEBUG_STATIC OSStatus	_AirPlayNTPClient_SendRequest( AirPlayNTPClientRef inClient );
DEBUG_STATIC OSStatus	_AirPlayNTPClient_ReadPacket( AirPlayNTPClientRef inClient );
DEBUG_STATIC OSStatus	_AirPlayNTPClient_ProcessResponse( AirPlayNTPClientRef inClient, NTPPacket *inPkt, uint64_t inTicks );

DEBUG_STATIC Boolean	_AirPlayNTPClient_AdjustClock( AirPlayNTPClientRef inClient, Q32x32 inAdjust, Boolean inReset );
DEBUG_STATIC void		_AirPlayNTPClient_ClockTimer( void *inContext );

ulog_define( AirPlayNTPClientCore, kLogLevelNotice, kLogFlags_Default, "AirPlayNTPClientCore",  NULL );
#define antpc_ulog( LEVEL, ... )		ulog( &log_category_from_name( AirPlayNTPClientCore ), (LEVEL), __VA_ARGS__ )

ulog_define( AirPlayNTPClientStat, kLogLevelOff, kLogFlags_Default, "AirPlayNTPClientStat",  NULL );
#define antpcstat_ulog( LEVEL, ... )	ulog( &log_category_from_name( AirPlayNTPClientStat ), (LEVEL), __VA_ARGS__ )

//===========================================================================================================================
//	AirPlayNTPClient_Create
//===========================================================================================================================

OSStatus	AirPlayNTPClient_Create( AirPlayNTPClientRef *outClient, const void *inServerAddr )
{
	OSStatus				err;
	AirPlayNTPClientRef		client;
	size_t					i;
	
	client = (AirPlayNTPClientRef) calloc( 1, sizeof( *client ) );
	require_action( client, exit, err = kNoMemoryErr );
	
	SockAddrCopy( inServerAddr, &client->serverAddr );
	client->ntpSocket = kInvalidSocketRef;
	
	client->rttAvg		= 0.0;
	client->rttMin		= DBL_MAX;
	client->rttMax		= DBL_MIN;
	client->rttLeastBad	= DBL_MAX;
	
	for( i = 0; i < countof( client->clockOffsets ); ++i )
	{
		client->clockOffsets[ i ] = 0.0;
	}
	client->clockOffsetMin		= DBL_MAX;
	client->clockOffsetMax		= DBL_MIN;
	client->clockOffsetAvg		= 0.0;
	client->clockOffsetLeastBad	= 0.0;
	
	client->localFrequency	= UpTicksPerSecond();
	client->localScale		= UINT64_C( 0xFFFFFFFFFFFFFFFF ) / client->localFrequency;
	client->localGeneration	= 1;
	
	client->internalQueue = dispatch_queue_create( "AirPlayNTPClient", NULL );
	require_action( client->internalQueue, exit, err = kNoMemoryErr );
	dispatch_set_context( client->internalQueue, client );
	dispatch_set_finalizer_f( client->internalQueue, _AirPlayNTPClient_Finalizer );
	
	antpc_ulog( kLogLevelTrace, "Created   %p to %##a\n", client, &client->serverAddr );
	*outClient = client;
	client = NULL;
	err = kNoErr;
	
exit:
	if( client ) _AirPlayNTPClient_Delete( client );
	return( err );
}

//===========================================================================================================================
//	AirPlayNTPClient_Delete
//===========================================================================================================================

void	AirPlayNTPClient_Delete( AirPlayNTPClientRef inClient )
{
	antpc_ulog( kLogLevelTrace, "Deleting  %p\n", inClient );
	dispatch_async_f( inClient->internalQueue, inClient, _AirPlayNTPClient_Delete );
}

DEBUG_STATIC void	_AirPlayNTPClient_Delete( void *inContext )
{
	AirPlayNTPClientRef const		client = (AirPlayNTPClientRef) inContext;
	
	_AirPlayNTPClient_Stop( client );
	if( client->internalQueue ) dispatch_release( client->internalQueue );
	else						_AirPlayNTPClient_Finalizer( client );
}

DEBUG_STATIC void	_AirPlayNTPClient_Finalizer( void *inContext )
{
	AirPlayNTPClientRef const		client = (AirPlayNTPClientRef) inContext;
	
	check( !IsValidSocket( client->ntpSocket ) );
	check( client->ntpSource == NULL );
	check( client->timerSource == NULL );
	
	antpc_ulog( kLogLevelTrace, "Finalized %p\n", client );
	free( client );
}

//===========================================================================================================================
//	AirPlayNTPClient_SetQoSDisabled
//===========================================================================================================================

void	AirPlayNTPClient_SetQoSDisabled( AirPlayNTPClientRef inClient, Boolean inQoSDisabled )
{
	inClient->qosDisabled = inQoSDisabled;
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AirPlayNTPClient_Start
//===========================================================================================================================

OSStatus	AirPlayNTPClient_Start( AirPlayNTPClientRef inClient )
{
	OSStatus		err;
	
	err = _AirPlayNTPClient_SetupSocket( inClient );
	require_noerr( err, exit );
	
	inClient->ntpNextRequestUpTicks = kUpTicksForever;
	inClient->localLastCount = UpTicks32();
	
	inClient->timerSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, inClient->internalQueue );
	require_action( inClient->timerSource, exit, err = kUnknownErr );
	dispatch_set_context( inClient->timerSource, inClient );
	dispatch_source_set_event_handler_f( inClient->timerSource, _AirPlayNTPClient_ClockTimer );
	dispatch_source_set_timer( inClient->timerSource, DISPATCH_TIME_NOW, kNanosecondsPerSecond / 10, 
		10 * kNanosecondsPerMillisecond );
	dispatch_resume( inClient->timerSource );
	
	antpc_ulog( kLogLevelTrace, "Started   %p to %##a from %##a\n", inClient, &inClient->serverAddr, &inClient->selfAddr );
	
	err = _AirPlayNTPClient_Negotiate( inClient );
	require_noerr_quiet( err, exit );
	
	inClient->ntpNextRequestUpTicks = UpTicks() + ( 3 * inClient->localFrequency );
	
	// Setup a callback to process packets as they arrive.
	
	inClient->ntpSource = dispatch_source_create( DISPATCH_SOURCE_TYPE_READ, inClient->ntpSocket, 0, 
		inClient->internalQueue );
	require_action( inClient->ntpSource, exit, err = kUnknownErr );
	
	dispatch_set_context( inClient->ntpSource, inClient );
	dispatch_source_set_event_handler_f(  inClient->ntpSource, _AirPlayNTPClient_ReadHandler );
	dispatch_source_set_cancel_handler_f( inClient->ntpSource, _AirPlayNTPClient_CancelHandler );
	dispatch_resume( inClient->ntpSource );
	
exit:
	if( err )
	{
		_AirPlayNTPClient_Stop( inClient );
		antpc_ulog( kLogLevelWarning, "### NTP client start with %##a failed: %#m\n", &inClient->serverAddr, err );
	}
	return( err );
}

//===========================================================================================================================
//	_AirPlayNTPClient_Stop
//===========================================================================================================================

void	_AirPlayNTPClient_Stop( AirPlayNTPClientRef inClient )
{
	dispatch_socket_forget( inClient->ntpSource, &inClient->ntpSocket, false );
	dispatch_source_forget( &inClient->timerSource );
}

//===========================================================================================================================
//	_AirPlayNTPClient_Negotiate
//===========================================================================================================================

DEBUG_STATIC OSStatus	_AirPlayNTPClient_Negotiate( AirPlayNTPClientRef inClient )
{
	OSStatus			err;
	int					nFailure;
	int					nSuccess;
	int					nTimeout;
	int					nSendError, nTotalSendError;
	int					nRecvError;
	fd_set				readSet;
	int					n;
	struct timeval		timeout;
	
	antpc_ulog( kLogLevelTrace, "Negotiating time with %##a\n", &inClient->serverAddr );
	nFailure		= 0;
	nSuccess		= 0;
	nTimeout		= 0;
	nRecvError		= 0;
	nTotalSendError	= 0;
	FD_ZERO( &readSet );
	for( ;; )
	{
		// Send request.
		
		nSendError = 0;
		for( ;; )
		{
			err = _AirPlayNTPClient_SendRequest( inClient );
			if( !err ) break;
			antpc_ulog( kLogLevelInfo, "### Negotiate send to %##a failed (%d): %#m\n", 
				&inClient->serverAddr, nSendError, err );
			usleep( 10000 );
			++nTotalSendError;
			if( ++nSendError >= 64 ) goto exit;
		}
		
		// Read and process response.
		
		for( ;; )
		{
			do
			{
				FD_SET( inClient->ntpSocket, &readSet );
				timeout.tv_sec  = 0;
				timeout.tv_usec = 500 * 1000;
				n = select( inClient->ntpSocket + 1, &readSet, NULL, NULL, &timeout );
				err = select_errno( n );
			
			}	while( err == EINTR );
			if( err )
			{
				if( err == kTimeoutErr ) ++nTimeout;
				++nFailure;
				antpc_ulog( kLogLevelWarning, "### Negotiate wait for %##a failed (%d, %d): %#m\n", 
					&inClient->serverAddr, nFailure, nTimeout, err );
				break;
			}
			
			err = _AirPlayNTPClient_ReadPacket( inClient );
			if( err )
			{
				++nRecvError;
				++nFailure;
				antpc_ulog( kLogLevelInfo, "### Negotiate recv failed (%d, %d): %#m\n", nFailure, nRecvError, err );
				if( err == kDuplicateErr ) DrainUDPSocket( inClient->ntpSocket, 500, NULL );
			}
			else
			{
				++nSuccess;
			}
			break;
		}
		if( nSuccess >= 3 ) break;
		if( nFailure >= 64 )
		{
			antpc_ulog( kLogLevelError, "### Negotiate with %##a failed: %d, %d, %d, %d\n", 
				&inClient->serverAddr, nFailure, nSuccess, nRecvError, nTimeout );
			err = kTimeoutErr;
			goto exit;
		}
	}
	antpc_ulog( kLogLevelInfo, "Successfully negotiated time with %##a\n", &inClient->serverAddr );
	
exit:
	if( err ) antpc_ulog( kLogLevelWarning, "### Negotiate with %##a failed: G=%d, B=%d, S=%d, R=%d, T=%d\n", 
		&inClient->serverAddr, nSuccess, nFailure, nTotalSendError, nRecvError, nTimeout );
	return( err );
}

//===========================================================================================================================
//	_AirPlayNTPClient_SetupSocket
//===========================================================================================================================

DEBUG_STATIC OSStatus	_AirPlayNTPClient_SetupSocket( AirPlayNTPClientRef inClient )
{
	OSStatus		err;
	int				family;
	SocketRef		sock;
	sockaddr_ip		sip;
	socklen_t		len;
		
	family = SockAddrGetFamily( &inClient->serverAddr );
	sock = socket( family, SOCK_DGRAM, IPPROTO_UDP );
	err = map_socket_creation_errno( sock );
	require_noerr( err, exit );
	
	SocketMakeNonBlocking( sock );
	SocketSetPacketTimestamps( sock, true );
	if( !inClient->qosDisabled ) SocketSetQoS( sock, kSocketQoS_NTP );
	
	// Start listening for packets. If a fixed port is already in use then try a dynamic port.
	
	if( family == AF_INET )
	{
		memset( &sip.v4, 0, sizeof( sip.v4 ) );
		SIN_LEN_SET( &sip.v4 );
		sip.v4.sin_family		= AF_INET;
		sip.v4.sin_port		= htons( kAirPlayPort_ScreenNTPClient );
		sip.v4.sin_addr.s_addr	= htonl( INADDR_ANY );
		len = sizeof( sip.v4 );
	}
	else if( family == AF_INET6 )
	{
		memset( &sip.v6, 0, sizeof( sip.v6 ) );
		SIN6_LEN_SET( &sip.v6 );
		sip.v6.sin6_family		= AF_INET6;
		sip.v6.sin6_port		= htons( kAirPlayPort_ScreenNTPClient );
		sip.v6.sin6_flowinfo	= 0;
		sip.v6.sin6_addr		= in6addr_any;
		sip.v6.sin6_scope_id	= 0;
		len = sizeof( sip.v6 );
	}
	else
	{
		dlogassert( "Bad family: %##a", &inClient->serverAddr );
		err = kUnsupportedErr;
		goto exit;
	}
	err = bind( sock, &sip.sa, len );
	err = map_socket_noerr_errno( sock, err );
	if( err && ( kAirPlayPort_ScreenNTPClient != 0 ) )
	{
		SockAddrSetPort( &sip, 0 );
		err = bind( sock, &sip.sa, len );
		err = map_socket_noerr_errno( sock, err );
	}
	require_noerr( err, exit );
	
	len = (socklen_t) sizeof( inClient->selfAddr );
	getsockname( sock, &inClient->selfAddr.sa, &len );
	
	// Connect to the server address so we receive ICMP errors if sends fail.
	
	err = connect( sock, &inClient->serverAddr.sa, SockAddrGetSize( &inClient->serverAddr ) );
	err = map_socket_noerr_errno( sock, err );
	require_noerr( err, exit );
	
	inClient->ntpSocket = sock;
	sock = kInvalidSocketRef;
	err = kNoErr;
	
exit:
	ForgetSocket( &sock );
	return( err );
}

//===========================================================================================================================
//	_AirPlayNTPClient_ReadHandler
//===========================================================================================================================

DEBUG_STATIC void	_AirPlayNTPClient_ReadHandler( void *inContext )
{
	_AirPlayNTPClient_ReadPacket( (AirPlayNTPClientRef) inContext );
}

//===========================================================================================================================
//	_AirPlayNTPClient_CancelHandler
//===========================================================================================================================

DEBUG_STATIC void	_AirPlayNTPClient_CancelHandler( void *inContext )
{
	AirPlayNTPClientRef const		client = (AirPlayNTPClientRef) inContext;
	
	antpc_ulog( kLogLevelTrace, "Canceled  %d\n", client->ntpSocket );
	ForgetSocket( &client->ntpSocket );
	client->ntpSource = NULL;
}

//===========================================================================================================================
//	_AirPlayNTPClient_SendRequest
//===========================================================================================================================

DEBUG_STATIC OSStatus	_AirPlayNTPClient_SendRequest( AirPlayNTPClientRef inClient )
{
	OSStatus		err;
	NTPPacket		pkt;
	Time96			now;
	ssize_t			n;
	
	NTPPacket_SetLeap_Version_Mode( &pkt, kNTPLeap_NoWarning, kNTPVersion4, kNTPMode_Client );
	pkt.stratum			= 0;
	pkt.poll			= 0;
	pkt.precision		= 0;
	pkt.rootDelay		= 0;
	pkt.rootDispersion	= 0;
	pkt.referenceID		= 0;
	pkt.referenceTimeHi	= 0;
	pkt.referenceTimeLo	= 0;
	pkt.originateTimeHi	= 0;
	pkt.originateTimeLo	= 0;
	pkt.receiveTimeHi	= 0;
	pkt.receiveTimeLo	= 0;
	
	AirPlayNTPClient_GetSynchronizedTime( inClient, &now );
	inClient->ntpLastTransmitTimeHi = (uint32_t) now.secs;
	inClient->ntpLastTransmitTimeLo = (uint32_t)( now.frac >> 32 );
	pkt.transmitTimeHi = htonl( inClient->ntpLastTransmitTimeHi );
	pkt.transmitTimeLo = htonl( inClient->ntpLastTransmitTimeLo );
	
	n = send( inClient->ntpSocket, &pkt, sizeof( pkt ), 0 );
	err = map_socket_value_errno( inClient->ntpSocket, n == (ssize_t) sizeof( pkt ), n );
	return( err );
}

//===========================================================================================================================
//	_AirPlayNTPClient_ReadPacket
//===========================================================================================================================

DEBUG_STATIC OSStatus	_AirPlayNTPClient_ReadPacket( AirPlayNTPClientRef inClient )
{
	OSStatus			err;
	NTPPacket			pkt;
	size_t				len;
	sockaddr_ip			sip;
	uint64_t			ticks;
	
	err = SocketRecvFrom( inClient->ntpSocket, &pkt, sizeof( pkt ), &len, &sip, sizeof( sip ), NULL, &ticks, NULL, NULL );
	if( err ) usleep( 100000 );
	require_noerr_quiet( err, exit );
	require_action( len == sizeof( pkt ), exit, err = kSizeErr );
	
	if( NTPPacket_GetMode( &pkt ) == kNTPMode_Server )
	{
		err = _AirPlayNTPClient_ProcessResponse( inClient, &pkt, ticks );
	}
	else if( ( pkt.stratum == 0 ) && ( ntohl( pkt.referenceID ) == 0x5857414B ) ) // 'XWAK' Awake packet.
	{
		err = _AirPlayNTPClient_SendRequest( inClient );
	}
	else
	{
		err = kUnsupportedErr;
		goto exit;
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	_AirPlayNTPClient_ProcessResponse
//===========================================================================================================================

DEBUG_STATIC OSStatus	_AirPlayNTPClient_ProcessResponse( AirPlayNTPClientRef inClient, NTPPacket *inPkt, uint64_t inTicks )
{
	OSStatus		err;
	Time96			syncedTime;
	uint64_t		t1;
	uint64_t		t2;
	uint64_t		t3;
	uint64_t		t4;
	double			offset;
	double			rtt, rttAbs;
	double			median;
	size_t			i;
	
	AirPlayNTPClient_GetSynchronizedTimeNearUpTicks( inClient, &syncedTime, inTicks );
		
	// Make sure this response is for the last request we made and is not a duplicate response.
	
	inPkt->originateTimeHi = ntohl( inPkt->originateTimeHi );
	inPkt->originateTimeLo = ntohl( inPkt->originateTimeLo );
	if( ( inPkt->originateTimeHi != inClient->ntpLastTransmitTimeHi ) || 
		( inPkt->originateTimeLo != inClient->ntpLastTransmitTimeLo ) )
	{
		antpc_ulog( kLogLevelInfo, "### Wrong NTP response. Expected time 0x%08x:%08x, but got 0x%08x:%08x\n", 
			inClient->ntpLastTransmitTimeHi, inClient->ntpLastTransmitTimeLo, 
			inPkt->originateTimeHi, inPkt->originateTimeLo );
		err = kDuplicateErr;
		goto exit;
	}
	inClient->ntpLastTransmitTimeHi = 0; // Zero so we don't try to process a duplicate.
	inClient->ntpLastTransmitTimeLo = 0;
	
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
	
	t1 = ( ( (uint64_t) inPkt->originateTimeHi )			<< 32 ) | inPkt->originateTimeLo;
	t2 = ( ( (uint64_t) ntohl( inPkt->receiveTimeHi ) )		<< 32 ) | ntohl( inPkt->receiveTimeLo );
	t3 = ( ( (uint64_t) ntohl( inPkt->transmitTimeHi ) )	<< 32 ) | ntohl( inPkt->transmitTimeLo );
	t4 = ( ( (uint64_t) syncedTime.secs )					<< 32 ) | ( syncedTime.frac >> 32 );
	
	offset = ( ( ( (double)( (int64_t)( t2 - t1 ) ) ) * kNTPFractionalUnit_FP ) + 
			   ( ( (double)( (int64_t)( t3 - t4 ) ) ) * kNTPFractionalUnit_FP ) ) / 2;
	rtt =	   ( ( (double)( (int64_t)( t4 - t1 ) ) ) * kNTPFractionalUnit_FP ) - 
			   ( ( (double)( (int64_t)( t3 - t2 ) ) ) * kNTPFractionalUnit_FP );
	
	// Update round trip time stats. If the RTT is > 50 ms, it's probably a spurious error.
	// If we're not getting any good RTT's, use the best one we've received in the last 10 tries.
	// This avoids the initial time sync negotiate from failing entirely if RTT's are really high.
	// It also avoids time sync from drifting too much if RTT's become really high later.
	
	if( rtt < inClient->rttMin ) inClient->rttMin = rtt;
	if( rtt > inClient->rttMax ) inClient->rttMax = rtt;
	rttAbs = fabs( rtt );
	if( rttAbs > 0.050 )
	{
		antpc_ulog( kLogLevelInfo, "### Large NTP RTT %f, RTT avg %f, RTT min %f, RTT max %f, offset %f\n", 
			rtt, inClient->rttAvg, inClient->rttMin, inClient->rttMax, offset );
		
		if( rttAbs < fabs( inClient->rttLeastBad ) )
		{
			inClient->rttLeastBad			= rtt;
			inClient->clockOffsetLeastBad	= offset;
		}
		if( ++inClient->rttBadCount < 5 )
		{
			err = kRangeErr;
			goto exit;
		}
		
		antpc_ulog( kLogLevelInfo, "### Replacing large NTP RTT %f, offset %f with least bad RTT %f, offset %f\n", 
			rtt, offset, inClient->rttLeastBad, inClient->clockOffsetLeastBad );
		rtt		= inClient->rttLeastBad;
		offset	= inClient->clockOffsetLeastBad;
	}
	if( inClient->ntpResponseCount == 0 ) inClient->rttAvg = rtt;
	inClient->rttAvg				= ( ( ( 10.0 - 1.0 ) * inClient->rttAvg ) + rtt ) * ( 1.0 / 10.0 );
	inClient->rttBadCount			= 0;
	inClient->rttLeastBad			= +1000000.0;
	inClient->clockOffsetLeastBad	= 0.0;
	
	// Update clock offset stats. If this is first time ever or the first time after a clock step, reset the stats.
	
	if( inClient->ntpResetStats || ( inClient->ntpResponseCount == 0 ) )
	{
		for( i = 0; i < countof( inClient->clockOffsets ); ++i )
		{
			inClient->clockOffsets[ i ] = offset;
		}
		inClient->clockOffsetIndex	= 0;
		inClient->clockOffsetAvg	= offset;
		inClient->clockOffsetMin	= offset;
		inClient->clockOffsetMax	= offset;
		median = offset;
	}
	else
	{
		inClient->clockOffsets[ inClient->clockOffsetIndex++ ] = offset;
		if( inClient->clockOffsetIndex >= countof( inClient->clockOffsets ) )
		{
			inClient->clockOffsetIndex = 0;
		}
		
		check_compile_time_code( countof( inClient->clockOffsets ) == 3 );
		median = median_of_3( inClient->clockOffsets[ 0 ], inClient->clockOffsets[ 1 ], inClient->clockOffsets[ 2 ] );
		inClient->clockOffsetAvg = ( ( ( 10.0 - 1.0 ) * inClient->clockOffsetAvg ) + offset ) * ( 1.0 / 10.0 );
	}
	if( offset < inClient->clockOffsetMin ) inClient->clockOffsetMin = offset;
	if( offset > inClient->clockOffsetMax ) inClient->clockOffsetMax = offset;
	
	// Sync our local clock to the server's clock. Use median to reject outliers. If this is the first sync, always step.
	
	inClient->ntpResetStats = _AirPlayNTPClient_AdjustClock( inClient, (int64_t)( median * 4294967296.0 ), 
		inClient->ntpResponseCount == 0 );
	if( inClient->ntpResetStats ) ++inClient->ntpStepCount;
	++inClient->ntpResponseCount;
	
	if( ( inClient->ntpResponseCount % 20 ) == 1 )
	{
		antpcstat_ulog( kLogLevelInfo | kLogLevelFlagContinuation, "\n" );
		antpcstat_ulog( kLogLevelInfo, "#    RTTLast   RTTMin    RTTMax    RTTAvg    CLKLast   CLKMin    CLKMax    CLKAvg    Rate\n" );
		antpcstat_ulog( kLogLevelInfo, "---- --------- --------- --------- --------- --------- --------- --------- --------- ----------\n" );
	}
	antpcstat_ulog( kLogLevelInfo, 
		"%-4u " 
		"%+1.6f %+1.6f %+1.6f %+1.6f "
		"%+1.6f %+1.6f %+1.6f %+1.6f "
		"%+1.6f"
		"\n", 
		inClient->ntpResponseCount, 
		rtt, inClient->rttMin, inClient->rttMax, inClient->rttAvg, 
		offset, inClient->clockOffsetMin, inClient->clockOffsetMax, inClient->clockOffsetAvg, 
		( (double) inClient->localFrequency ) * 
		( ( (double) inClient->localScale ) / ( (double) UINT64_C( 0xFFFFFFFFFFFFFFFF ) ) ) );
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AirPlayNTPClient_GetSynchronizedNTPTime
//===========================================================================================================================

uint64_t	AirPlayNTPClient_GetSynchronizedNTPTime( AirPlayNTPClientRef inClient )
{
	Time96		t96;
	
	AirPlayNTPClient_GetSynchronizedTime( inClient, &t96 );
	return( Time96ToNTP( &t96 ) );
}

//===========================================================================================================================
//	AirPlayNTPClient_GetSynchronizedTime
//===========================================================================================================================

void	AirPlayNTPClient_GetSynchronizedTime( AirPlayNTPClientRef inClient, Time96 *outTime )
{
	uint32_t		gen;
	uint32_t		delta;
	
	do
	{
		gen = inClient->localGeneration;
		delta = UpTicks32() - inClient->localLastCount;
		*outTime = inClient->localTime;
		Time96_AddFrac( outTime, delta * inClient->localScale );
		atomic_read_write_barrier();
		
	}	while( ( gen == 0 ) || ( gen != inClient->localGeneration ) );
}

//===========================================================================================================================
//	AirPlayNTPClient_GetSynchronizedTimeNearUpTicks
//===========================================================================================================================

void	AirPlayNTPClient_GetSynchronizedTimeNearUpTicks( AirPlayNTPClientRef inClient, Time96 *outTime, uint64_t inTicks )
{
	uint32_t		gen;
	uint64_t		ticks;
	uint32_t		delta;
	uint64_t		scale;
	Time96			deltaTime;
	
	do
	{
		gen = inClient->localGeneration;
		ticks = UpTicks();
		delta = ( (uint32_t)( ticks & UINT32_C( 0xFFFFFFFF ) ) ) - inClient->localLastCount;
		scale = inClient->localScale;
		*outTime = inClient->localTime;
		Time96_AddFrac( outTime, delta * scale );
		atomic_read_write_barrier();
		
	}	while( ( gen == 0 ) || ( gen != inClient->localGeneration ) );
	
	ticks -= inTicks;
	deltaTime.secs = (int32_t)( ticks / inClient->localFrequency ); // Note: unscaled, but delta expected to be < 1 sec.
	deltaTime.frac = scale *  ( ticks % inClient->localFrequency );
	Time96_Sub( outTime, &deltaTime );
}

//===========================================================================================================================
//	AirPlayNTPClient_GetUpTicksNearSynchronizedNTPTime
//===========================================================================================================================

uint64_t	AirPlayNTPClient_GetUpTicksNearSynchronizedNTPTime( AirPlayNTPClientRef inClient, uint64_t inNTPTime )
{
	uint64_t		nowNTP;
	uint64_t		ticks;
	
	nowNTP = AirPlayNTPClient_GetSynchronizedNTPTime( inClient );
	if( inNTPTime >= nowNTP )	ticks = UpTicks() + NTPtoUpTicks( inNTPTime - nowNTP );
	else						ticks = UpTicks() - NTPtoUpTicks( nowNTP    - inNTPTime );
	return( ticks );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_AirPlayNTPClient_AdjustClock
//===========================================================================================================================

DEBUG_STATIC Boolean	_AirPlayNTPClient_AdjustClock( AirPlayNTPClientRef inClient, Q32x32 inAmount, Boolean inReset )
{
	if( inReset || ( inAmount < -kAirPlayNTP_MaxSlew ) || ( inAmount > kAirPlayNTP_MaxSlew ) )
	{
		uint32_t	gen;
		Time96		temp;
		
		gen = inClient->localGeneration + 1;
		if( gen == 0 ) gen = 1;			// Generation 0 is reserved to mean invalid so skip to 1.
		inClient->localGeneration = 0;	// Mark generation invalid while updating (so readers retry).
		
		temp.secs = (int32_t)(  inAmount >> 32 ); // Note: preserves sign so -0.25 -> -1 here.
		temp.frac = (uint64_t)( inAmount << 32 ); // Note: preserves sign so -0.25 -> 0xC0... instead of 0x40...
		Time96_Add( &inClient->localTime, &temp );
		atomic_read_write_barrier();
		
		inClient->localGeneration = gen;
		inReset = true;
		
		antpc_ulog( kLogLevelInfo, "Stepped clock by %d.%09llu seconds\n", temp.secs, Time96FracToNanoseconds( temp.frac ) );
	}
	else
	{
		uint32_t		gen;
		int64_t			adjust;
		uint64_t		scale;
		
		adjust = inAmount;
		Q32x32_RightShift( adjust, 3 ); // Divide by 8 for the amount of adjust over the next 8 seconds.
		adjust <<= 32; // Convert Q32x32 to Q64 units.
		
		gen = inClient->localGeneration + 1;
		if( gen == 0 ) gen = 1;				// Generation 0 is reserved to mean invalid so skip to 1.
		inClient->localGeneration = 0;		// Mark generation invalid while updating (so readers retry).
		
		scale = UINT64_C( 1 ) << 63;		// Use 2^63 to avoid overflow when the adjustment is added.
		scale += adjust;
		scale /= inClient->localFrequency;
		inClient->localScale = scale * 2;	// Scale back to compensate for using 2^63 instead of 2^64 above.
		
		inClient->localGeneration = gen;
	}
	return( inReset );
}

//===========================================================================================================================
//	_AirPlayNTPClient_ClockTimer
//===========================================================================================================================

DEBUG_STATIC void	_AirPlayNTPClient_ClockTimer( void *inContext )
{
	AirPlayNTPClientRef const		client = (AirPlayNTPClientRef) inContext;
	OSStatus						err;
	uint32_t						gen;
	uint64_t						ticks;
	uint32_t						count;
	uint32_t						delta;
	
	gen = client->localGeneration + 1;
	if( gen == 0 ) gen = 1;			// Generation 0 is reserved to mean invalid so skip to 1.
	client->localGeneration = 0;	// Mark generation invalid while updating (so readers retry).
	
	ticks = UpTicks();
	count = (uint32_t)( ticks & UINT32_C( 0xFFFFFFFF ) );
	delta = count - client->localLastCount;
	client->localLastCount = count;
	Time96_AddFrac( &client->localTime, delta * client->localScale );
	
	client->localGeneration = gen;
	
	if( ticks >= client->ntpNextRequestUpTicks )
	{
		err = _AirPlayNTPClient_SendRequest( client );
		if( err ) antpc_ulog( kLogLevelWarning, "### Send NTP to %##a failed: %#m\n", &client->serverAddr, err );
		
		ticks += ( 2 * client->localFrequency ) + ( Random32() % ( client->localFrequency + 1 ) );
		client->ntpNextRequestUpTicks = ticks;
	}
}
