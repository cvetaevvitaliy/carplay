/*
	File:    	AirPlayReceiverSession.c
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
	
	Copyright (C) 2005-2014 Apple Inc. All Rights Reserved.
*/

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#if 0
#pragma mark == Includes ==
#endif

#include "AirPlayReceiverSession.h"
#include "AirPlayReceiverSessionPriv.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "AESUtils.h"
#include "AirPlayCommon.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayReceiverServerPriv.h"
#include "AirPlaySettings.h"
#include "AirPlayUtils.h"
#include "AirTunesClock.h"
#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFUtils.h"
#include "HTTPClient.h"
#include "HTTPUtils.h"
#include "MathUtils.h"
#include "NetUtils.h"
#include "NTPUtils.h"
#include "RandomNumberUtils.h"
#include "StringUtils.h"
#include "SystemUtils.h"
#include "TickUtils.h"
#include "TimeUtils.h"
#include "UUIDUtils.h"

#include CF_HEADER

#if( TARGET_OS_POSIX )
	#include <sys/types.h>
	
	#include <arpa/inet.h>
	#include <netinet/in.h>
		#include <netinet/in_systm.h>
		#include <netinet/ip.h>
	#include <netinet/tcp.h>
	#include <pthread.h>
	#include <sched.h>
	#include <sys/socket.h>
	#include <linux/sysctl.h>
#endif

#if 0
#pragma mark == Constants ==
#endif

//===========================================================================================================================
//	Constants
//===========================================================================================================================

	#define kAirPlayInputRingSize				65536	// 371 millieconds at 44100 Hz.

#if 0
#pragma mark == Prototypes ==
#endif

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

#define	AirTunesFreeBufferNode( SESSION, NODE )				\
	do														\
	{														\
		( NODE )->next->prev	= ( NODE )->prev;			\
		( NODE )->prev->next	= ( NODE )->next;			\
		( NODE )->next			= ( SESSION )->freeList;	\
		( SESSION )->freeList	= ( NODE );					\
		--( SESSION )->busyNodeCount;						\
		debug_sub( gAirTunesDebugBusyNodeCount, 1 );		\
															\
	}	while( 0 )

#define NanosecondsToMilliseconds32( NANOS )	\
	( (uint32_t)( ( (NANOS) == UINT64_MAX ) ? 0 : ( (NANOS) / kNanosecondsPerMillisecond ) ) )

// General

static void		_GetTypeID( void *inContext );
static void		_Finalize( CFTypeRef inCF );
static void		_PerformPeriodTasks( void *inContext );
static OSStatus	_UpdateFeedback( AirPlayReceiverSessionRef inSession, CFDictionaryRef inInput, CFDictionaryRef *outOutput );

// Control/Events

static OSStatus
	_ControlSetup( 
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inRequestParams, 
		CFMutableDictionaryRef		inResponseParams );
static void		_ControlTearDown( AirPlayReceiverSessionRef inSession );
static OSStatus	_ControlStart( AirPlayReceiverSessionRef inSession );

// MainAudio

static OSStatus
	_MainAltAudioSetup(
		AirPlayReceiverSessionRef	inSession, 
		AirPlayStreamType			inType, 
		CFDictionaryRef				inRequestStreamDesc, 
		CFMutableDictionaryRef		inResponseParams );
static void *	_MainAltAudioThread( void *inArg );
static void		_MainAltAudioProcessPacket( AirPlayAudioStreamContext * const ctx );

// Timing

static OSStatus	_TimingInitialize( AirPlayReceiverSessionRef inSession );
static OSStatus	_TimingFinalize( AirPlayReceiverSessionRef inSession );
static OSStatus	_TimingNegotiate( AirPlayReceiverSessionRef inSession );
static void *	_TimingThread( void *inArg );
static OSStatus	_TimingSendRequest( AirPlayReceiverSessionRef inSession );
static OSStatus	_TimingReceiveResponse( AirPlayReceiverSessionRef inSession, SocketRef inSock );
static OSStatus	_TimingProcessResponse( AirPlayReceiverSessionRef inSession, RTCPTimeSyncPacket *inPkt, const AirTunesTime *inTime );

// Screen

static OSStatus
	_ScreenSetup(
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inStreamDesc,
		CFMutableDictionaryRef		inResponseParams );

static void		_ScreenTearDown( AirPlayReceiverSessionRef inSession );
static OSStatus	_ScreenStart( AirPlayReceiverSessionRef inSession );
static void *	_ScreenThread( void *inArg );
static void
	_ScreenHandleEvent(
		AirPlayReceiverSessionScreenRef		inSession,
		CFStringRef							inEventName,
		CFDictionaryRef						inEventData,
		void *								inUserData );
static uint64_t _ScreenGetSynchronizedNTPTime( void *inContext );
static uint64_t _ScreenGetUpTicksNearSynchronizedNTPTime( void *inContext, uint64_t inNTPTime );

// Utils

static OSStatus	_AddResponseStream( CFMutableDictionaryRef inResponseParams, CFDictionaryRef inStreamDesc );

static void	_LogStarted( AirPlayReceiverSessionRef inSession, AirPlayReceiverSessionStartInfo *inInfo, OSStatus inStatus );
static void	_LogEnded( AirPlayReceiverSessionRef inSession, OSStatus inReason );
static void	_LogUpdate( AirPlayReceiverSessionRef inSession, uint64_t inTicks, Boolean inForce );
static void	_TearDownStream( AirPlayReceiverSessionRef inSession, AirPlayAudioStreamContext * const ctx );
static void	_UpdateEstimatedRate( AirPlayAudioStreamContext *ctx, uint32_t inSampleTime, uint64_t inHostTime );

// Debugging

#if( DEBUG )
	#define	airtunes_record_clock_offset( X )																			\
		do																												\
		{																												\
			gAirTunesClockOffsetHistory[ gAirTunesClockOffsetIndex++ ] = ( X );											\
			if( gAirTunesClockOffsetIndex >= countof( gAirTunesClockOffsetHistory ) ) gAirTunesClockOffsetIndex = 0;	\
			if( gAirTunesClockOffsetCount < countof( gAirTunesClockOffsetHistory ) ) ++gAirTunesClockOffsetCount;		\
																														\
		}	while( 0 )
#else
	#define	airtunes_record_clock_offset( X )
#endif

#if( DEBUG )
	#define	airtunes_record_clock_rtt( X )																		\
		do																										\
		{																										\
			gAirTunesClockRTTHistory[ gAirTunesClockRTTIndex++ ] = ( X );										\
			if( gAirTunesClockRTTIndex >= countof( gAirTunesClockRTTHistory ) ) gAirTunesClockRTTIndex = 0;		\
			if( gAirTunesClockRTTCount < countof( gAirTunesClockRTTHistory ) ) ++gAirTunesClockRTTCount;		\
																												\
		}	while( 0 )
#else
	#define	airtunes_record_clock_rtt( X )
#endif

#if( DEBUG )
	#define	airtunes_record_rtp_offset( X )																			\
		do																											\
		{																											\
			gAirTunesRTPOffsetHistory[ gAirTunesRTPOffsetIndex++ ] = ( X );											\
			if( gAirTunesRTPOffsetIndex >= countof( gAirTunesRTPOffsetHistory ) ) gAirTunesRTPOffsetIndex = 0;		\
			if( gAirTunesRTPOffsetCount < countof( gAirTunesRTPOffsetHistory ) ) ++gAirTunesRTPOffsetCount;			\
																													\
		}	while( 0 )
#else
	#define	airtunes_record_rtp_offset( X )
#endif

#if( DEBUG )
	#define	airtunes_record_retransmit( RT_NODE, REASON, FINAL_NANOS )												\
		do																											\
		{																											\
			AirTunesRetransmitHistoryNode *		rthNode;															\
			size_t								rthI;																\
																													\
			rthNode = &gAirTunesRetransmitHistory[ gAirTunesRetransmitIndex++ ];									\
			rthNode->reason 	= ( REASON );																		\
			rthNode->seq		= ( RT_NODE )->seq;																	\
			rthNode->tries		= ( RT_NODE )->tries;																\
			rthNode->finalNanos	= ( FINAL_NANOS );																	\
			for( rthI = 0; rthI < countof( rthNode->tryNanos ); ++rthI )											\
			{																										\
				rthNode->tryNanos[ rthI ] = ( RT_NODE )->tryNanos[ rthI ];											\
			}																										\
			if( gAirTunesRetransmitIndex >= countof( gAirTunesRetransmitHistory ) ) gAirTunesRetransmitIndex = 0;	\
			if( gAirTunesRetransmitCount < countof( gAirTunesRetransmitHistory ) ) ++gAirTunesRetransmitCount;		\
																													\
		}	while( 0 )
#else
	#define	airtunes_record_retransmit( RT_NODE, REASON, FINAL_NANOS )
#endif

ulog_define( AirPlayReceiverCore,		kLogLevelNotice, kLogFlags_Default, "AirPlay",  NULL );
#define atr_ucat()						&log_category_from_name( AirPlayReceiverCore )
#define atr_ulog( LEVEL, ... )			ulog( atr_ucat(), (LEVEL), __VA_ARGS__ )

ulog_define( AirPlayReceiverEvents,	kLogLevelNotice, kLogFlags_Default, "AirPlay",  NULL );
#define atr_events_ucat()				&log_category_from_name( AirPlayReceiverEvents )

ulog_define( AirPlayReceiverStats,		kLogLevelNotice, kLogFlags_Default, "AirPlay", "AirPlayReceiverStats:rate=5;3000" );
#define atr_stats_ucat()				&log_category_from_name( AirPlayReceiverStats )
#define atr_stats_ulog( LEVEL, ... )	ulog( atr_stats_ucat(), (LEVEL), __VA_ARGS__ )

#if 0
#pragma mark == Globals ==
#endif

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static const CFRuntimeClass		kAirPlayReceiverSessionClass = 
{
	0,							// version
	"AirPlayReceiverSession",	// className
	NULL,						// init
	NULL,						// copy
	_Finalize,					// finalize
	NULL,						// equal -- NULL means pointer equality.
	NULL,						// hash  -- NULL means pointer hash.
	NULL,						// copyFormattingDesc
	NULL,						// copyDebugDesc
	NULL,						// reclaim
	NULL						// refcount
};

static dispatch_once_t		gAirPlayReceiverSessionInitOnce	= 0;
static CFTypeID				gAirPlayReceiverSessionTypeID	= _kCFRuntimeNotATypeID;
static int32_t				gAirTunesRelativeTimeOffset		= 0;		// Custom adjustment to the real offset for fine tuning.

AirPlayReceiverSessionRef	gAirTunes = NULL;
AirPlayAudioStats			gAirPlayAudioStats;

// Debugging

#if( DEBUG )
	FILE *					gAirTunesFile									= NULL;
	
	// Control Variables
	
	int						gAirTunesDropMinRate							= 0;
	int						gAirTunesDropMaxRate							= 0;
	int						gAirTunesDropRemaining							= 0;
	int						gAirTunesSkipMinRate							= 0;
	int						gAirTunesSkipMaxRate							= 0;
	int						gAirTunesSkipRemaining							= 0;
	int						gAirTunesLateDrop								= 0;
	int						gAirTunesDebugNoSkewSlew						= 0;
	int						gAirTunesDebugLogAllSkew						= 0;
	int						gAirTunesDebugNoRetransmits						= 0;
	int						gAirTunesDebugPrintPerf							= 0;
	int						gAirTunesDebugPerfMode							= 0;
	int						gAirTunesDebugRetransmitTiming					= 1;
	
	// Stats
	
	unsigned int			gAirTunesDebugBusyNodeCount						= 0;
	unsigned int			gAirTunesDebugBusyNodeCountLast					= 0;
	unsigned int			gAirTunesDebugBusyNodeCountMax					= 0;
	
	uint64_t				gAirTunesDebugSentByteCount						= 0;
	uint64_t				gAirTunesDebugRecvByteCount						= 0;
	uint64_t				gAirTunesDebugRecvRTPOriginalByteCount			= 0;
	uint64_t				gAirTunesDebugRecvRTPOriginalByteCountLast		= 0;
	uint64_t				gAirTunesDebugRecvRTPOriginalBytesPerSecAvg		= 0;
	uint64_t				gAirTunesDebugRecvRTPRetransmitByteCount		= 0;
	uint64_t				gAirTunesDebugRecvRTPRetransmitByteCountLast	= 0;
	uint64_t				gAirTunesDebugRecvRTPRetransmitBytesPerSecAvg	= 0;
	unsigned int			gAirTunesDebugIdleTimeoutCount					= 0;
	unsigned int			gAirTunesDebugStolenNodeCount					= 0;
	unsigned int			gAirTunesDebugOldDiscardCount					= 0;
	unsigned int			gAirTunesDebugConcealedGapCount					= 0;
	unsigned int			gAirTunesDebugConcealedEndCount					= 0;
	unsigned int			gAirTunesDebugLateDropCount						= 0;
	unsigned int			gAirTunesDebugSameTimestampCount				= 0;
	unsigned int			gAirTunesDebugLossCounts[ 10 ]					= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int			gAirTunesDebugTotalLossCount					= 0;
	unsigned int			gAirTunesDebugMaxBurstLoss						= 0;
	unsigned int			gAirTunesDebugDupCount							= 0;
	unsigned int			gAirTunesDebugMisorderedCount					= 0;
	unsigned int			gAirTunesDebugUnrecoveredPacketCount			= 0;
	unsigned int			gAirTunesDebugUnexpectedRTPOffsetResetCount 	= 0;
	unsigned int			gAirTunesDebugHugeSkewResetCount				= 0;
	unsigned int			gAirTunesDebugGlitchCount			 			= 0;
	unsigned int			gAirTunesDebugTimeSyncHugeRTTCount				= 0;
	uint64_t				gAirTunesDebugTimeAnnounceMinNanos				= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	uint64_t				gAirTunesDebugTimeAnnounceMaxNanos				= 0;
	unsigned int			gAirTunesDebugRetransmitActiveCount				= 0;
	unsigned int			gAirTunesDebugRetransmitActiveMax				= 0;
	unsigned int			gAirTunesDebugRetransmitSendCount				= 0;
	unsigned int			gAirTunesDebugRetransmitSendLastCount			= 0;
	unsigned int			gAirTunesDebugRetransmitSendPerSecAvg			= 0;
	unsigned int			gAirTunesDebugRetransmitRecvCount				= 0;
	unsigned int			gAirTunesDebugRetransmitBigLossCount			= 0;
	unsigned int			gAirTunesDebugRetransmitAbortCount				= 0;
	unsigned int			gAirTunesDebugRetransmitFutileAbortCount		= 0;
	unsigned int			gAirTunesDebugRetransmitNoFreeNodesCount		= 0;
	unsigned int			gAirTunesDebugRetransmitNotFoundCount			= 0;
	unsigned int			gAirTunesDebugRetransmitPrematureCount			= 0;
	unsigned int			gAirTunesDebugRetransmitMaxTries				= 0;
	uint64_t				gAirTunesDebugRetransmitMinNanos				= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	uint64_t				gAirTunesDebugRetransmitMaxNanos				= 0;
	uint64_t				gAirTunesDebugRetransmitAvgNanos				= 0;
	uint64_t				gAirTunesDebugRetransmitRetryMinNanos			= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	uint64_t				gAirTunesDebugRetransmitRetryMaxNanos			= 0;
	
	double					gAirTunesClockOffsetHistory[ 512 ];
	unsigned int			gAirTunesClockOffsetIndex 						= 0;
	unsigned int			gAirTunesClockOffsetCount 						= 0;
	
	double					gAirTunesClockRTTHistory[ 512 ];
	unsigned int			gAirTunesClockRTTIndex 							= 0;
	unsigned int			gAirTunesClockRTTCount 							= 0;
	
	uint32_t				gAirTunesRTPOffsetHistory[ 1024 ];
	unsigned int			gAirTunesRTPOffsetIndex 						= 0;
	unsigned int			gAirTunesRTPOffsetCount 						= 0;
	
	// Transients
	
	uint64_t				gAirTunesDebugLastPollTicks						= 0;
	uint64_t				gAirTunesDebugPollIntervalTicks					= 0;
	uint64_t				gAirTunesDebugSentByteCountLast					= 0;
	uint64_t				gAirTunesDebugRecvByteCountLast					= 0;
	uint16_t				gAirTunesDebugHighestSeqLast					= 0;
	uint32_t				gAirTunesDebugTotalLossCountLast				= 0;
	uint32_t				gAirTunesDebugRecvCountLast						= 0;
	uint32_t				gAirTunesDebugGlitchCountLast					= 0;
#endif

#if 0
#pragma mark == General ==
#endif

//===========================================================================================================================
//	AirPlayReceiverSessionGetTypeID
//===========================================================================================================================

CFTypeID	AirPlayReceiverSessionGetTypeID( void )
{
	dispatch_once_f( &gAirPlayReceiverSessionInitOnce, NULL, _GetTypeID );
	return( gAirPlayReceiverSessionTypeID );
}

//===========================================================================================================================
//	AirPlayReceiverSessionCreate
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionCreate( 
		AirPlayReceiverSessionRef *					outSession, 
		const AirPlayReceiverSessionCreateParams *	inParams )
{
	AirPlayReceiverServerRef const		server = inParams->server;
	OSStatus							err;
	AirPlayReceiverSessionRef			me;
	size_t								extraLen;
	uint64_t							ticksPerSec;
	uint64_t							ticks;
	AirTunesSource *					ats;
	
	ticksPerSec = UpTicksPerSecond();
	ticks = UpTicks();
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (AirPlayReceiverSessionRef) _CFRuntimeCreateInstance( NULL, AirPlayReceiverSessionGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	me->queue = AirPlayReceiverServerGetDispatchQueue( server );
	dispatch_retain( me->queue );
	
	CFRetain( server );
	me->server = server;
	
	// Initialize variables to a good state so we can safely clean up if something fails during init.
	
	me->transportType			= inParams->transportType;
	me->peerAddr				= *inParams->peerAddr;
	UUIDGet( me->sessionUUID );
	me->startStatus				= kNotInitializedErr;
	me->clientDeviceID			= inParams->clientDeviceID;
	me->clientSessionID			= inParams->clientSessionID;
	me->clientVersion			= inParams->clientVersion;
	
	memcpy( me->clientIfMACAddr, inParams->clientIfMACAddr, sizeof( inParams->clientIfMACAddr ) );
	
	me->sessionTicks			= ticks;
	me->useEvents				= inParams->useEvents;
	me->eventSock				= kInvalidSocketRef;
	me->mainAudioCtx.cmdSock	= kInvalidSocketRef;
	me->mainAudioCtx.dataSock	= kInvalidSocketRef;
	me->altAudioCtx.cmdSock		= kInvalidSocketRef;
	me->altAudioCtx.dataSock	= kInvalidSocketRef;
	me->timingSock				= kInvalidSocketRef;
	me->timingCmdSock			= kInvalidSocketRef;
	me->screenSock				= kInvalidSocketRef;
	
	err = AirPlayReceiverSessionScreen_Create( &me->screenSession );
	require_noerr( err, exit );
	
	AirPlayReceiverSessionScreen_SetEventHandler( me->screenSession, _ScreenHandleEvent, me, NULL );
	
	AirPlayReceiverSessionScreen_SetOverscanOverride( me->screenSession, me->server->overscanOverride );
	
	AirPlayReceiverSessionScreen_SetClientIfMACAddr( me->screenSession, (uint8_t *) inParams->clientIfMACAddr, sizeof( inParams->clientIfMACAddr ) );
	
	AirPlayReceiverSessionScreen_SetIFName( me->screenSession, (char *) inParams->ifName );
	
	AirPlayReceiverSessionScreen_SetTransportType( me->screenSession, me->transportType );
	
	// Finish initialization.
	
	ats								= &me->source;
	ats->lastActivityTicks			= ticks;
	ats->maxIdleTicks				= server->timeoutDataSecs * ticksPerSec;
	ats->perSecTicks				= 1 * ticksPerSec;
	ats->perSecLastTicks			= 0; // Explicit 0 to note that it's intentional to force an immmediate update.
	ats->lastIdleLogTicks			= ticks;
	ats->idleLogIntervalTicks		= 10 * ticksPerSec;
	
	ats->rtcpTIClockRTTMin			= +1000000.0;
	ats->rtcpTIClockRTTMax			= -1000000.0;
	ats->rtcpTIClockRTTLeastBad		= +1000000.0;
	ats->rtcpTIClockOffsetMin		= +1000000.0;
	ats->rtcpTIClockOffsetMax		= -1000000.0;
	
	if( server->delegate.sessionCreated_f ) server->delegate.sessionCreated_f( server, me, server->delegate.context );
	err = AirPlayReceiverSessionPlatformInitialize( me );
	require_noerr( err, exit );
	if( me->delegate.initialize_f )
	{
		err = me->delegate.initialize_f( me, me->delegate.context );
		require_noerr( err, exit );
	}
	
	*outSession = me;
	me = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( me );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionGetDispatchQueue
//===========================================================================================================================

dispatch_queue_t	AirPlayReceiverSessionGetDispatchQueue( AirPlayReceiverSessionRef inSession )
{
	return( inSession->queue );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetDelegate
//===========================================================================================================================

EXPORT_GLOBAL
void	AirPlayReceiverSessionSetDelegate( AirPlayReceiverSessionRef inSession, const AirPlayReceiverSessionDelegate *inDelegate )
{
	inSession->delegate = *inDelegate;
}

//===========================================================================================================================
//	AirPlayReceiverSessionControl
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionControl( 
		CFTypeRef			inSession, 
		uint32_t			inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outParams )
{
	AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inSession;
	OSStatus							err;
	
	if( 0 ) {}
		
	// ModesChanged
	
	else if( session->delegate.modesChanged_f && CFEqual( inCommand, CFSTR( kAirPlayCommand_ModesChanged ) ) )
	{
		AirPlayModeState		modeState;
		
		require_action( inParams, exit, err = kParamErr );
		err = AirPlayReceiverSessionMakeModeStateFromDictionary( session, inParams, &modeState );
		require_noerr( err, exit );
		
		atr_ulog( kLogLevelNotice, "Modes changed: screen %s, mainAudio %s, speech %s (%s), phone %s, turns %s\n", 
			AirPlayEntityToString( modeState.screen ), AirPlayEntityToString( modeState.mainAudio ), 
			AirPlayEntityToString( modeState.speech.entity ), AirPlaySpeechModeToString( modeState.speech.mode ), 
			AirPlayEntityToString( modeState.phoneCall ), AirPlayEntityToString( modeState.turnByTurn ) );
		session->delegate.modesChanged_f( session, &modeState, session->delegate.context );
	}
	
	// RequestUI
	
	else if( session->delegate.requestUI_f && CFEqual( inCommand, CFSTR( kAirPlayCommand_RequestUI ) ) )
	{
		atr_ulog( kLogLevelNotice, "Request accessory UI\n" );
		session->delegate.requestUI_f( session, session->delegate.context );
	}
	
	// UpdateFeedback
	
	else if( CFEqual( inCommand, CFSTR( kAirPlayCommand_UpdateFeedback ) ) )
	{
		err = _UpdateFeedback( session, inParams, outParams );
		require_noerr( err, exit );
	}
	
	// SetHIDInputMode
	
	else if( session->delegate.control_f && CFEqual( inCommand, CFSTR( kAirPlayCommand_HIDSetInputMode ) ) )
	{
		atr_ulog( kLogLevelNotice, "Set HIDInputMode\n" );
		session->delegate.control_f( session, inCommand, inQualifier, inParams, outParams, session->delegate.context );
	}
	
	// Other
	
	else
	{
		err = AirPlayReceiverSessionPlatformControl( session, inFlags, inCommand, inQualifier, inParams, outParams );
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionCopyProperty
//===========================================================================================================================

EXPORT_GLOBAL
CFTypeRef
	AirPlayReceiverSessionCopyProperty( 
		CFTypeRef	inSession, 
		uint32_t	inFlags, 
		CFStringRef	inProperty, 
		CFTypeRef	inQualifier, 
		OSStatus *	outErr )
{
	AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inSession;
	OSStatus							err;
	CFTypeRef							value = NULL;
	
	(void) inFlags;
	(void) inQualifier;
	
	if( 0 ) {}
	
	// Unknown
	
	else
	{
		value = AirPlayReceiverSessionPlatformCopyProperty( session, inFlags, inProperty, inQualifier, &err );
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetProperty
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionSetProperty( 
		CFTypeRef	inSession, 
		uint32_t	inFlags, 
		CFStringRef	inProperty, 
		CFTypeRef	inQualifier, 
		CFTypeRef	inValue )
{
	AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inSession;
	OSStatus							err;
	
	if( 0 ) {}
	
	// TimelineOffset
	
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_TimelineOffset ) ) )
	{
		int32_t		offset;
		
		offset = (int32_t) CFGetInt64( inValue, NULL );
		require_action( ( offset >= -250 ) && ( offset <= 250 ), exit, err = kRangeErr );
		gAirTunesRelativeTimeOffset = offset;
	}
	
	// Other
	
	else
	{
		err = AirPlayReceiverSessionPlatformSetProperty( session, inFlags, inProperty, inQualifier, inValue );
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetSecurityInfo
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionSetSecurityInfo( 
		AirPlayReceiverSessionRef	inSession, 
		const uint8_t				inKey[ 16 ], 
		const uint8_t				inIV[ 16 ] )
{
	OSStatus		err;
	
	AES_CBCFrame_Final( &inSession->decryptorStorage );
	inSession->decryptor = NULL;
	
	err = AES_CBCFrame_Init( &inSession->decryptorStorage, inKey, inIV, false );
	require_noerr( err, exit );
	inSession->decryptor = &inSession->decryptorStorage;
	
	memcpy( inSession->aesSessionKey, inKey, sizeof( inSession->aesSessionKey ) );
	memcpy( inSession->aesSessionIV, inIV, sizeof( inSession->aesSessionIV ) );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSendCommand
//===========================================================================================================================

static void	_AirPlayReceiverSessionSendCommandCompletion( HTTPMessageRef inMsg );

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionSendCommand( 
		AirPlayReceiverSessionRef					inSession, 
		CFDictionaryRef								inRequest, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	OSStatus			err;
	HTTPMessageRef		msg = NULL;
	CFDataRef			data;
	
	require_action_quiet( inSession->eventClient, exit, err = kUnsupportedErr );
	
	err = HTTPMessageCreate( &msg );
	require_noerr( err, exit );
	
	err = HTTPHeader_InitRequest( &msg->header, "POST", kAirPlayCommandPath, kAirTunesHTTPVersionStr );
	require_noerr( err, exit );
	
	data = CFPropertyListCreateData( NULL, inRequest, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	err = HTTPMessageSetBody( msg, kMIMEType_AppleBinaryPlist, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	if( inCompletion )
	{
		CFRetain( inSession );
		msg->userContext1 = inSession;
		msg->userContext2 = (void *)(uintptr_t) inCompletion;
		msg->userContext3 = inContext;
		msg->completion   = _AirPlayReceiverSessionSendCommandCompletion;
	}
	
	err = HTTPClientSendMessage( inSession->eventClient, msg );
	if( err && inCompletion ) CFRelease( inSession );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( msg );
	return( err );
}

static void	_AirPlayReceiverSessionSendCommandCompletion( HTTPMessageRef inMsg )
{
	AirPlayReceiverSessionRef const							session		= (AirPlayReceiverSessionRef) inMsg->userContext1;
	AirPlayReceiverSessionCommandCompletionFunc const		completion	= (AirPlayReceiverSessionCommandCompletionFunc)(uintptr_t) inMsg->userContext2;
	OSStatus												err;
	CFDictionaryRef											response;
	
	response = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	err = (OSStatus) CFDictionaryGetInt64( response, CFSTR( kAirPlayKey_Status ), NULL );
	require_noerr_quiet( err, exit );
	
exit:
	if( completion ) completion( err, err ? NULL : response, inMsg->userContext3 );
	CFRelease( session );
	CFReleaseNullSafe( response );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetup
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionSetup( 
		AirPlayReceiverSessionRef	me,
		CFDictionaryRef				inRequestParams, 
		CFDictionaryRef *			outResponseParams )
{
	OSStatus					err;
	CFMutableDictionaryRef		responseParams;
	CFArrayRef					requestStreams;
	CFDictionaryRef				requestStreamDesc;
	CFIndex						streamIndex, streamCount;
	char						clientOSBuildVersion[ 32 ], minClientOSBuildVersion[ 32 ];
	CFStringRef					tempCFStr;
	AirPlayStreamType			type;
	
	atr_ulog( kLogLevelTrace, "Setting up session %llu with %##a %?@\n", 
		me->clientSessionID, &me->peerAddr, log_category_enabled( atr_ucat(), kLogLevelVerbose ), inRequestParams );
	
	responseParams = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( responseParams, exit, err = kNoMemoryErr );
	
	if( !me->controlSetup )
	{
		err = _ControlSetup( me, inRequestParams, responseParams );
		require_noerr( err, exit );
	}
	
	// Perform minimum version check
	
	err = AirPlayGetMinimumClientOSBuildVersion( minClientOSBuildVersion, sizeof( minClientOSBuildVersion ) );
	if( !err )
	{
		CFDictionaryGetCString( inRequestParams, CFSTR( kAirPlayKey_OSBuildVersion ), clientOSBuildVersion, sizeof( clientOSBuildVersion ), &err );
		if( !err )
		{
			if( CompareOSBuildVersionStrings( minClientOSBuildVersion, clientOSBuildVersion ) > 0 )
				require_noerr( err = kVersionErr, exit );
		}
	}
		
	// Save off client info.
	
		tempCFStr = CFDictionaryGetCFString( inRequestParams, CFSTR( kAirPlayKey_ModelCode ), &err );
		if( tempCFStr ) AirPlayReceiverSessionScreen_SetClientModelCode( me->screenSession, tempCFStr );
		
		tempCFStr = CFDictionaryGetCFString( inRequestParams, CFSTR( kAirPlayKey_UDID ), &err );
		if( tempCFStr ) AirPlayReceiverSessionScreen_SetClientDeviceUDID( me->screenSession, tempCFStr );
		
		tempCFStr = CFDictionaryGetCFString( inRequestParams, CFSTR( kAirPlayKey_OSBuildVersion ), &err );
		if( tempCFStr ) AirPlayReceiverSessionScreen_SetClientOSBuildVersion( me->screenSession, tempCFStr );
	
	// Set up each stream.
	
	requestStreams = CFDictionaryGetCFArray( inRequestParams, CFSTR( kAirPlayKey_Streams ), &err );
	streamCount = requestStreams ? CFArrayGetCount( requestStreams ) : 0;
	for( streamIndex = 0; streamIndex < streamCount; ++streamIndex )
	{
		requestStreamDesc = CFArrayGetCFDictionaryAtIndex( requestStreams, streamIndex, &err );
		require_noerr( err, exit );
		
		type = (AirPlayStreamType) CFDictionaryGetInt64( requestStreamDesc, CFSTR( kAirPlayKey_Type ), NULL );
		switch( type )
		{
			
			case kAirPlayStreamType_MainAudio:
			case kAirPlayStreamType_AltAudio:
				err = _MainAltAudioSetup( me, type, requestStreamDesc, responseParams );
				require_noerr( err, exit );
				break;
			
			case kAirPlayStreamType_Screen:
				err = _ScreenSetup( me, requestStreamDesc, responseParams );
				require_noerr( err, exit );
				break;
			
			default:
				atr_ulog( kLogLevelNotice, "### Unsupported stream type: %d\n", type );
				break;
		}
	}
	
	// Set up the platform.
	
	err = AirPlayReceiverSessionPlatformControl( me, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SetUpStreams ), NULL, 
		inRequestParams, NULL );
	require_noerr( err, exit );
	
	*outResponseParams = responseParams;
	responseParams = NULL;
	gAirTunes = me;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( responseParams );
	if( err )
	{
		atr_ulog( kLogLevelNotice, "### Set up session %llu with %##a failed: %#m %@\n", 
			me->clientSessionID, &me->peerAddr, err, inRequestParams );

		if( me->server->delegate.sessionFailed_f )
			me->server->delegate.sessionFailed_f( me->server, err, me->server->delegate.context );

		AirPlayReceiverSessionTearDown( me, inRequestParams, err, NULL );
	}
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionTearDown
//===========================================================================================================================

static void
	_AirPlayReceiverSessionReplaceEventHandlerSynchronouslyOnSessionScreen( void *inArg )
{
	AirPlayReceiverSessionRef inSession = (AirPlayReceiverSessionRef) inArg;
	AirPlayReceiverSessionScreen_ReplaceEventHandlerSynchronously( inSession->screenSession, NULL, NULL, NULL );
	CFRelease( inSession );
}

void
	AirPlayReceiverSessionTearDown( 
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inParams, 
		OSStatus					inReason, 
		Boolean *					outDone )
{
	OSStatus				err;
	CFArrayRef				streams;
	CFIndex					streamIndex, streamCount;
	CFDictionaryRef			streamDesc;
	AirPlayStreamType		streamType;
	
	atr_ulog( kLogLevelTrace, "Tearing down session %llu with %##a %?@\n", 
		inSession->clientSessionID, &inSession->peerAddr, log_category_enabled( atr_ucat(), kLogLevelVerbose ), inParams );
	
	AirPlayReceiverSessionPlatformControl( inSession, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_TearDownStreams ), NULL, 
		inParams, NULL );
	
	streams = inParams ? CFDictionaryGetCFArray( inParams, CFSTR( kAirPlayKey_Streams ), NULL ) : NULL;
	streamCount = streams ? CFArrayGetCount( streams ) : 0;
	for( streamIndex = 0; streamIndex < streamCount; ++streamIndex )
	{
		streamDesc = CFArrayGetCFDictionaryAtIndex( streams, streamIndex, &err );
		require_noerr( err, exit );
		
		streamType = (AirPlayStreamType) CFDictionaryGetInt64( streamDesc, CFSTR( kAirPlayKey_Type ), NULL );
		switch( streamType )
		{
			case kAirPlayStreamType_MainAudio:
				_TearDownStream( inSession, &inSession->mainAudioCtx );
				break;
			
			case kAirPlayStreamType_AltAudio:
				_TearDownStream( inSession, &inSession->altAudioCtx );
				break;
			
			case kAirPlayStreamType_Screen:
				_ScreenTearDown( inSession );
				if( inSession->screenSession != NULL ) {
					CFRetain( inSession );
                    dispatch_async_f( dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 ), inSession, _AirPlayReceiverSessionReplaceEventHandlerSynchronouslyOnSessionScreen );
                }
				break;
			
			default:
				atr_ulog( kLogLevelNotice, "### Unsupported stream type: %d\n", streamType );
				break;
		}
	}
	if( streamCount > 0 ) goto exit;
	
	_LogEnded( inSession, inReason );
	gAirTunes = NULL;
	
	_ScreenTearDown( inSession );
	_TearDownStream( inSession, &inSession->altAudioCtx );
	_TearDownStream( inSession, &inSession->mainAudioCtx );
	_ControlTearDown( inSession );
	_TimingFinalize( inSession );
	AirTunesClock_Finalize( inSession->airTunesClock );
	inSession->airTunesClock = NULL;
	dispatch_source_forget( &inSession->periodicTimer );
	
exit:
	if( outDone ) *outDone = ( streamCount == 0 );
}

//===========================================================================================================================
//	AirPlayReceiverSessionStart
//===========================================================================================================================

OSStatus	AirPlayReceiverSessionStart( AirPlayReceiverSessionRef inSession, AirPlayReceiverSessionStartInfo *inInfo )
{
	OSStatus						err;
	AirPlayAudioStreamContext *		ctx;
	dispatch_source_t				source;
	uint64_t						nanos;
	
	inSession->playTicks = UpTicks();
	
	if( IsValidSocket( inSession->eventSock ) )
	{
		err = _ControlStart( inSession );
		require_noerr( err, exit );
	}
	
	ctx = &inSession->mainAudioCtx;
	
	if( ( ctx->type == kAirPlayStreamType_MainAudio ) && !ctx->threadPtr )
	{
		err = pthread_create( &ctx->thread, NULL, _MainAltAudioThread, ctx );
		require_noerr( err, exit );
		ctx->threadPtr = &ctx->thread;
	}
	
	ctx = &inSession->altAudioCtx;
	if( ( ctx->type == kAirPlayStreamType_AltAudio ) && !ctx->threadPtr )
	{
		err = pthread_create( &ctx->thread, NULL, _MainAltAudioThread, ctx );
		require_noerr( err, exit );
		ctx->threadPtr = &ctx->thread;
	}
	
	if( IsValidSocket( inSession->screenSock ) )
	{
		err = _ScreenStart( inSession );
		require_noerr( err, exit );
	}

	err = _TimingNegotiate( inSession );
	require_noerr( err, exit );
	
	err = AirPlayReceiverSessionPlatformControl( inSession, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_StartSession ), 
		NULL, NULL, NULL );
	require_noerr_quiet( err, exit );
	inSession->sessionStarted = true;
	
	// Start a timer to service things periodically.
	
	inSession->source.lastIdleLogTicks = inSession->playTicks;
	inSession->periodicTimer = source = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, inSession->queue );
	require_action( source, exit, err = kUnknownErr );
	dispatch_set_context( source, inSession );
	dispatch_source_set_event_handler_f( source, _PerformPeriodTasks );
	nanos = 250 * kNanosecondsPerMillisecond;
	dispatch_source_set_timer( source, dispatch_time( DISPATCH_TIME_NOW, nanos ), nanos, nanos );
	dispatch_resume( source );
	
exit:
	_LogStarted( inSession, inInfo, err );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionReadAudio
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionReadAudio( 
		AirPlayReceiverSessionRef	inSession, 
		AirPlayStreamType			inType, 
		uint32_t					inSampleTime, 
		uint64_t					inHostTime, 
		void *						inBuffer, 
		size_t						inLen )
{
	OSStatus		err;
	
	switch( inType )
	{
		
		case kAirPlayStreamType_MainAudio:
			_UpdateEstimatedRate( &inSession->mainAudioCtx, inSampleTime, inHostTime );
			err = RTPJitterBufferRead( &inSession->mainAudioCtx.jitterBuffer, inBuffer, inLen );
			require_noerr( err, exit );
			break;
		
		case kAirPlayStreamType_AltAudio:
			_UpdateEstimatedRate( &inSession->altAudioCtx, inSampleTime, inHostTime );
			err = RTPJitterBufferRead( &inSession->altAudioCtx.jitterBuffer, inBuffer, inLen );
			require_noerr( err, exit );
			break;
		
		default:
			dlogassert( "Bad stream type: %u", inType );
			err = kParamErr;
			goto exit;
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionWriteAudio
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionWriteAudio( 
		AirPlayReceiverSessionRef	inSession, 
		AirPlayStreamType			inType, 
		uint32_t					inSampleTime, 
		uint64_t					inHostTime, 
		const void *				inBuffer, 
		size_t						inLen )
{
	AirPlayAudioStreamContext * const		ctx		= &inSession->mainAudioCtx;
	MirroredRingBuffer * const				ring	= &ctx->inputRing;
	uint16_t								seq		= ctx->inputSeqNum;
	uint32_t								ts		= inSampleTime;
	uint32_t								spp;
	RTPSavedPacket							pkt;
	size_t									avail, len;
	ssize_t									n;
	OSStatus								err;
	
	(void) inHostTime;
	
	len = MirroredRingBufferGetBytesFree( ring );
	require_action_quiet( len >= inLen, exit, err = kNoSpaceErr; 
		atr_ulog( kLogLevelInfo, "### Audio input buffer full: %zu > %zu\n", inLen, len ) );
	memcpy( MirroredRingBufferGetWritePtr( ring ), inBuffer, inLen );
	MirroredRingBufferWriteAdvance( ring, inLen );
	
	pkt.pkt.rtp.header.v_p_x_cc	= RTPHeaderInsertVersion( 0, kRTPVersion );
	pkt.pkt.rtp.header.m_pt		= RTPHeaderInsertPayloadType( 0, inType );
	pkt.pkt.rtp.header.ssrc		= 0;
	for( ;; )
	{
		avail = MirroredRingBufferGetBytesUsed( ring );
		if( avail > kAirTunesMaxPayloadSizeUDP ) avail = kAirTunesMaxPayloadSizeUDP;
		spp = (uint32_t)( avail / ctx->bytesPerUnit );
		if( spp == 0 ) break;
		
		pkt.pkt.rtp.header.seq	= htons( seq );
		pkt.pkt.rtp.header.ts	= htonl( ts );
		HostToBig16Mem( MirroredRingBufferGetReadPtr( ring ), avail, pkt.pkt.rtp.payload );
		
		len = kRTPHeaderSize + avail;
		n = sendto( ctx->dataSock, (const char *) &pkt, len, 0, &ctx->inputAddr.sa, ctx->inputAddrLen );
		err = map_socket_value_errno( inSession->audioInputSock, n == (ssize_t) len, n );
		if( err )
		{
			increment_saturate( ctx->sendErrors, UINT32_MAX );
			atr_stats_ulog( kLogLevelNotice, "### Audio audio send error (%u total): %#m\n", ctx->sendErrors, err );
		}
		
		MirroredRingBufferReadAdvance( ring, avail );
		seq += 1;
		ts  += spp;
	}
	ctx->inputSeqNum = seq;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetUserVersion
//===========================================================================================================================

void AirPlayReceiverSessionSetUserVersion( AirPlayReceiverSessionRef inSession, uint32_t userVersion )
{
	AirPlayReceiverSessionScreen_SetUserVersion( inSession->screenSession, userVersion );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_GetTypeID
//===========================================================================================================================

static void _GetTypeID( void *inContext )
{
	(void) inContext;
	
	gAirPlayReceiverSessionTypeID = _CFRuntimeRegisterClass( &kAirPlayReceiverSessionClass );
	check( gAirPlayReceiverSessionTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	_Finalize
//===========================================================================================================================

static void	_Finalize( CFTypeRef inCF )
{
	AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inCF;
	
	gAirTunes = NULL;
	if( session->delegate.finalize_f ) session->delegate.finalize_f( session, session->delegate.context );
	AirPlayReceiverSessionPlatformFinalize( session );
	
	_ScreenTearDown( session );
	_TearDownStream( session, &session->altAudioCtx );
	_TearDownStream( session, &session->mainAudioCtx );
	_ControlTearDown( session );
	_TimingFinalize( session );
	AirTunesClock_Finalize( session->airTunesClock );
	session->airTunesClock = NULL;
	dispatch_source_forget( &session->periodicTimer );
	
	AES_CBCFrame_Final( &session->decryptorStorage );
	AirPlayReceiverSessionScreen_Forget( &session->screenSession );
	ForgetCF( &session->server );
	dispatch_forget( &session->queue );
}

//===========================================================================================================================
//	_PerformPeriodTasks
//===========================================================================================================================

static void	_PerformPeriodTasks( void *inContext )
{
	AirPlayReceiverSessionRef const		me  = (AirPlayReceiverSessionRef) inContext;
	AirTunesSource * const				ats = &me->source;
	uint64_t							ticks, idleTicks;
	
	// Check activity.
	
	ticks = UpTicks();
	if( ( ats->receiveCount == ats->lastReceiveCount ) && ( ats->activityCount == ats->lastActivityCount ) )
	{
		// If we've been idle for a while then log it.
		
		idleTicks = ticks - ats->lastActivityTicks;
		if( ( ticks - ats->lastIdleLogTicks ) > ats->idleLogIntervalTicks )
		{
			atr_ulog( kLogLevelInfo, "### Idle for %llu seconds\n", idleTicks / UpTicksPerSecond() );
			ats->lastIdleLogTicks = ticks;
		}
		
		// If there hasn't been activity in long time, fail the session.
		
		if( idleTicks > ats->maxIdleTicks )
		{
			debug_increment_saturate( gAirTunesDebugIdleTimeoutCount, UINT_MAX );
			atr_ulog( kLogLevelError, "Idle timeout after %d seconds with no audio\n", me->server->timeoutDataSecs );
			AirPlayReceiverServerControl( me->server, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SessionDied ), me, NULL, NULL );
			goto exit;
		}
	}
	else
	{
		ats->lastReceiveCount	= ats->receiveCount;
		ats->lastActivityCount	= ats->activityCount;
		ats->lastActivityTicks	= ticks;
		ats->lastIdleLogTicks	= ticks;
	}
	
	// Update stats.
	
	_LogUpdate( me, ticks, false );
	
exit:
	return;
}

//===========================================================================================================================
//	_UpdateFeedback
//===========================================================================================================================

static OSStatus	_UpdateFeedback( AirPlayReceiverSessionRef inSession, CFDictionaryRef inInput, CFDictionaryRef *outOutput )
{
	OSStatus					err;
	CFMutableDictionaryRef		feedback = NULL;
	CFMutableArrayRef			streams  = NULL;
	CFMutableDictionaryRef		stream   = NULL;
	
	(void) inInput;
	
	require_action_quiet( inSession->mainAudioCtx.session || inSession->altAudioCtx.session, exit, err = kNoErr );
	require_action_quiet( outOutput, exit, err = kNoErr );
	
	feedback = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( feedback, exit, err = kNoMemoryErr );
	
	streams = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( streams, exit, err = kNoMemoryErr );
	CFDictionarySetValue( feedback, CFSTR( kAirPlayKey_Streams ), streams );
	
	if( inSession->mainAudioCtx.session )
	{
		stream = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( stream, exit, err = kNoMemoryErr );
		
		CFDictionarySetInt64( stream, CFSTR( kAirPlayKey_Type ), kAirPlayStreamType_MainAudio );
		CFDictionarySetDouble( stream, CFSTR( kAirPlayKey_SampleRate ), inSession->mainAudioCtx.rateAvg );
		
		CFArrayAppendValue( streams, stream );
		CFRelease( stream );
		stream = NULL;
	}
	if( inSession->altAudioCtx.session )
	{
		stream = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( stream, exit, err = kNoMemoryErr );
		
		CFDictionarySetInt64( stream, CFSTR( kAirPlayKey_Type ), kAirPlayStreamType_AltAudio );
		CFDictionarySetDouble( stream, CFSTR( kAirPlayKey_SampleRate ), inSession->altAudioCtx.rateAvg );
		
		CFArrayAppendValue( streams, stream );
		CFRelease( stream );
		stream = NULL;
	}
	
	*outOutput = feedback;
	feedback = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( stream );
	CFReleaseNullSafe( streams );
	CFReleaseNullSafe( feedback );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Control/Events ==
#endif

//===========================================================================================================================
//	_ControlSetup
//===========================================================================================================================

static OSStatus
	_ControlSetup( 
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inRequestParams, 
		CFMutableDictionaryRef		inResponseParams )
{
	OSStatus		err;
	
	require_action( !inSession->controlSetup, exit2, err = kAlreadyInitializedErr );
	
	inSession->timingPortRemote = (int) CFDictionaryGetInt64( inRequestParams, CFSTR( kAirPlayKey_Port_Timing ), NULL );
	if( inSession->timingPortRemote <= 0 ) inSession->timingPortRemote = kAirPlayFixedPort_TimeSyncLegacy;
	
	err = AirTunesClock_Create( &inSession->airTunesClock );
	require_noerr( err, exit );
	
	err = _TimingInitialize( inSession );
	require_noerr( err, exit );
	CFDictionarySetInt64( inResponseParams, CFSTR( kAirPlayKey_Port_Timing ), inSession->timingPortLocal );
	
	if( inSession->useEvents )
	{
		err = ServerSocketOpen( inSession->peerAddr.sa.sa_family, SOCK_STREAM, IPPROTO_TCP, -kAirPlayPort_RTSPEvents,
			&inSession->eventPort, kSocketBufferSize_DontSet, &inSession->eventSock );
		require_noerr( err, exit );
		
		CFDictionarySetInt64( inResponseParams,  CFSTR( kAirPlayKey_Port_Event ), inSession->eventPort );
		
		atr_ulog( kLogLevelTrace, "Events set up on port %d\n", inSession->eventPort );
	}
	inSession->controlSetup = true;
	
exit:
	if( err ) _ControlTearDown( inSession );
	
exit2:
	if( err ) atr_ulog( kLogLevelWarning, "### Control setup failed: %#m\n", err );
	return( err );
}

//===========================================================================================================================
//	_ControlTearDown
//===========================================================================================================================

static void	_ControlTearDown( AirPlayReceiverSessionRef inSession )
{
	HTTPClientForget( &inSession->eventClient );
	ForgetSocket( &inSession->eventSock );
	if( inSession->controlSetup )
	{
		inSession->controlSetup = false;
		atr_ulog( kLogLevelTrace, "Control torn down\n" );
	}
}

//===========================================================================================================================
//	_ControlStart
//===========================================================================================================================

static OSStatus	_ControlStart( AirPlayReceiverSessionRef inSession )
{
	OSStatus		err;
	SocketRef		newSock = kInvalidSocketRef;
	sockaddr_ip		sip;
	
	err = SocketAccept( inSession->eventSock, kAirPlayConnectTimeoutSecs, &newSock, &sip );
	require_noerr( err, exit );
	ForgetSocket( &inSession->eventSock );
	
	err = HTTPClientCreateWithSocket( &inSession->eventClient, newSock );
	require_noerr( err, exit );
	newSock = kInvalidSocketRef;
	
	HTTPClientSetLogging( inSession->eventClient, atr_events_ucat() );
	
	atr_ulog( kLogLevelTrace, "Events started on port %d to port %d\n", inSession->eventPort, SockAddrGetPort( &sip ) );
	
exit:
	ForgetSocket( &newSock );
	if( err ) atr_ulog( kLogLevelWarning, "### Event start failed: %#m\n", err );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == MainAltAudio ==
#endif

//===========================================================================================================================
//	_MainAudioSetup
//===========================================================================================================================

static OSStatus
	_MainAltAudioSetup(
		AirPlayReceiverSessionRef	inSession, 
		AirPlayStreamType			inType, 
		CFDictionaryRef				inRequestStreamDesc, 
		CFMutableDictionaryRef		inResponseParams )
{
	OSStatus						err;
	AirPlayAudioStreamContext *		ctx;
	const char *					label;
	CFMutableDictionaryRef			responseStreamDesc = NULL;
	AirPlayAudioFormat				format;
	AudioStreamBasicDescription		asbd;
	int								receivePort, sendPort = 0;
	int64_t							bufferMs;
	
	switch( inType )
	{
		case kAirPlayStreamType_MainAudio:
			ctx = &inSession->mainAudioCtx;
			label = "Main";
			break;
		
		case kAirPlayStreamType_AltAudio:
			ctx = &inSession->altAudioCtx;
			label = "Alt";
			break;
		
		default:
			dlogassert( "Bad stream type: %u", inType );
			label = "<< Bad >>";
			err = kParamErr;
			goto exit2;
	}
	require_action( !ctx->session, exit2, err = kAlreadyInitializedErr );
	ctx->type  = inType;
	ctx->label = label;
	
	responseStreamDesc = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( responseStreamDesc, exit, err = kNoMemoryErr );
	
	err = OpenSelfConnectedLoopbackSocket( &ctx->cmdSock );
	require_noerr( err, exit );
	
	// Set up to receive audio from the sender.
	
	format = (AirPlayAudioFormat) CFDictionaryGetInt64( inRequestStreamDesc, CFSTR( kAirPlayKey_AudioFormat ), NULL );
	if( format == kAirPlayAudioFormat_Invalid ) format = kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo;
	err = AirPlayAudioFormatToASBD( format, &asbd, &ctx->bitsPerSample );
	require_noerr( err, exit );
	ctx->sampleRate		= (uint32_t) asbd.mSampleRate;
	ctx->channels		= asbd.mChannelsPerFrame;
	ctx->bytesPerUnit	= asbd.mBytesPerFrame;
	if( ctx->bytesPerUnit == 0 ) ctx->bytesPerUnit = ( RoundUp( ctx->bitsPerSample, 8 ) * ctx->channels ) / 8;
	
	ctx->rateUpdateNextTicks		= 0;
	ctx->rateUpdateIntervalTicks	= SecondsToUpTicks( 1 );
	ctx->rateUpdateCount			= 0;
	ctx->rateAvg					= (Float32) ctx->sampleRate;
	
	bufferMs = CFDictionaryGetInt64( inRequestStreamDesc, CFSTR( kAirPlayKey_AudioLatencyMs ), &err );
	if( err || ( bufferMs < 0 ) )
	{
		if( inSession->transportType == kNetTransportType_DirectLink )
		{
			bufferMs = CFDictionaryGetInt64( inRequestStreamDesc, CFSTR( kAirPlayPrefKey_AudioBufferMainAltWiredMs ), &err );
			if( err || ( bufferMs < 0 ) ) bufferMs = kAirPlayAudioBufferMainAltWiredMs;
		}
		else
		{
			bufferMs = CFDictionaryGetInt64( inRequestStreamDesc, CFSTR( kAirPlayPrefKey_AudioBufferMainAltWiFiMs ), &err );
			if( err || ( bufferMs < 0 ) ) bufferMs = kAirPlayAudioBufferMainAltWiFiMs;
		}
	}
	err = RTPJitterBufferInit( &ctx->jitterBuffer, ctx->sampleRate, ctx->bytesPerUnit, (uint32_t) bufferMs );
	require_noerr( err, exit );
	ctx->jitterBuffer.label = label;
	
	err = ServerSocketOpen( inSession->peerAddr.sa.sa_family, SOCK_DGRAM, IPPROTO_UDP, kSocketPort_Auto, &receivePort, 
		kSocketBufferSize_DontSet, &ctx->dataSock );
	require_noerr( err, exit );
	
	if( !inSession->server->qosDisabled )	SocketSetQoS( ctx->dataSock, kSocketQoS_Voice );
	
	CFDictionarySetInt64( responseStreamDesc, CFSTR( kAirPlayKey_Type ), inType );
	CFDictionarySetInt64( responseStreamDesc, CFSTR( kAirPlayKey_Port_Data ), receivePort );
	
	// If the sender provided its own port number then set up to send input audio to it.
	
	if( inType == kAirPlayStreamType_MainAudio )
	{
		sendPort = (int) CFDictionaryGetInt64( inRequestStreamDesc, CFSTR( kAirPlayKey_Port_Data ), NULL );
		if( sendPort > 0 )
		{
			err = MirroredRingBufferInit( &ctx->inputRing, kAirPlayInputRingSize, true );
			require_noerr( err, exit );
			
			RandomBytes( &ctx->inputSeqNum, sizeof( ctx->inputSeqNum ) );
			
			SockAddrCopy( &inSession->peerAddr, &ctx->inputAddr );
			SockAddrSetPort( &ctx->inputAddr, sendPort );
			ctx->inputAddrLen = (socklen_t) SockAddrGetSize( &ctx->inputAddr );
		}
	}
	
	err = _AddResponseStream( inResponseParams, responseStreamDesc );
	require_noerr( err, exit );
	
	ctx->session = inSession;
	atr_ulog( kLogLevelTrace, "%s audio set up for %s on receive port %d, send port %d\n", 
		ctx->label, AirPlayAudioFormatToString( format ), receivePort, sendPort );
	
	// If the session's already started then immediately start the thread process it.
	
	if( inSession->sessionStarted && !ctx->threadPtr )
	{
		err = pthread_create( &ctx->thread, NULL, _MainAltAudioThread, ctx );
		require_noerr( err, exit );
		ctx->threadPtr = &ctx->thread;
	}
	
exit:
	CFReleaseNullSafe( responseStreamDesc );
	if( err ) _TearDownStream( inSession, ctx );
	
exit2:
	if( err ) atr_ulog( kLogLevelWarning, "### %s audio setup failed: %#m\n", label, err );
	return( err );
}

//===========================================================================================================================
//	_MainAltAudioThread
//===========================================================================================================================

static void *	_MainAltAudioThread( void *inArg )
{
	AirPlayAudioStreamContext * const		ctx			= (AirPlayAudioStreamContext *) inArg;
	SocketRef const							dataSock	= ctx->dataSock;
	SocketRef const							cmdSock		= ctx->cmdSock;
	fd_set									readSet;
	int										maxFd;
	int										n;
	OSStatus								err;
	
	SetThreadName( "AirPlayAudioReceiver" );
	SetCurrentThreadPriority( kAirPlayThreadPriority_AudioReceiver );
    atr_ulog( kLogLevelTrace, "MainAltAudio thread starting\n" );
	
	FD_ZERO( &readSet );
	maxFd = -1;
	if( (int) dataSock > maxFd ) maxFd = dataSock;
	if( (int) cmdSock  > maxFd ) maxFd = cmdSock;
	maxFd += 1;
	for( ;; )
	{
		FD_SET( dataSock, &readSet );
		FD_SET( cmdSock,  &readSet );
		n = select( maxFd, &readSet, NULL, NULL, NULL );
		err = select_errno( n );
		if( err == EINTR ) continue;
		if( err ) { dlogassert( "select() error: %#m", err ); usleep( 100000 ); continue; }
		
		if( FD_ISSET( dataSock, &readSet ) ) _MainAltAudioProcessPacket( ctx );
		if( FD_ISSET( cmdSock,  &readSet ) ) break; // The only event is quit so break if anything is pending.
	}
	atr_ulog( kLogLevelTrace, "%s audio thread exit\n", ctx->label );
	return( NULL );
}

//===========================================================================================================================
//	_MainAltAudioProcessPacket
//===========================================================================================================================

static void	_MainAltAudioProcessPacket( AirPlayAudioStreamContext * const ctx )
{
	OSStatus			err;
	RTPPacketNode *		node = NULL;
	size_t				len;
	
	err = RTPJitterBufferGetFreeNode( &ctx->jitterBuffer, &node );
	require_noerr( err, exit );
	
	err = SocketRecvFrom( ctx->dataSock, node->pkt.pkt.bytes, sizeof( node->pkt.pkt.bytes ), &len, 
		NULL, 0, NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	require_action( len >= kRTPHeaderSize, exit, err = kSizeErr );
	
	node->pkt.pkt.rtp.header.seq	= ntohs( node->pkt.pkt.rtp.header.seq );
	node->pkt.pkt.rtp.header.ts		= ntohl( node->pkt.pkt.rtp.header.ts );
	node->pkt.pkt.rtp.header.ssrc	= ntohl( node->pkt.pkt.rtp.header.ssrc );
	node->pkt.len					= len - kRTPHeaderSize;
	node->ptr						= node->pkt.pkt.rtp.payload;
	BigToHost16Mem( node->ptr, node->pkt.len, node->ptr );
	
	err = RTPJitterBufferPutBusyNode( &ctx->jitterBuffer, node );
	require_noerr( err, exit );
	node = NULL;
	
exit:
	if( node )	RTPJitterBufferPutFreeNode( &ctx->jitterBuffer, node );
	if( err )	atr_ulog( kLogLevelNotice, "### Process main audio error: %#m\n", err );
}

#if 0
#pragma mark -
#pragma mark == Timing ==
#endif

//===========================================================================================================================
//	_TimingInitialize
//===========================================================================================================================

static OSStatus	_TimingInitialize( AirPlayReceiverSessionRef inSession )
{
	OSStatus		err;
	sockaddr_ip		sip;
	
	// Set up a socket to send and receive timing-related info.
	
	SockAddrCopy( &inSession->peerAddr, &sip );
	err = ServerSocketOpen( sip.sa.sa_family, SOCK_DGRAM, IPPROTO_UDP, -kAirPlayPort_TimeSyncClient, 
		&inSession->timingPortLocal, kSocketBufferSize_DontSet, &inSession->timingSock );
	require_noerr( err, exit );
	
	SocketSetPacketTimestamps( inSession->timingSock, true );
	if( !inSession->server->qosDisabled ) SocketSetQoS( inSession->timingSock, kSocketQoS_NTP );
	
	// Connect to the server address to avoid the IP stack doing a temporary connect on each send. 
	// Using connect also allows us to receive ICMP errors if the server goes away.
	
	SockAddrSetPort( &sip, inSession->timingPortRemote );
	inSession->timingRemoteAddr = sip;
	inSession->timingRemoteLen  = SockAddrGetSize( &sip );
	err = connect( inSession->timingSock, &sip.sa, inSession->timingRemoteLen );
	err = map_socket_noerr_errno( inSession->timingSock, err );
	if( err ) dlog( kLogLevelNotice, "### Timing connect UDP to %##a failed (using sendto instead): %#m\n", &sip, err );
	inSession->timingConnected = !err;
	
	// Set up a socket for sending commands to the thread.
	
	err = OpenSelfConnectedLoopbackSocket( &inSession->timingCmdSock );
	require_noerr( err, exit );
	
	atr_ulog( kLogLevelTrace, "Timing set up on port %d to port %d\n", inSession->timingPortLocal, inSession->timingPortRemote );
	
exit:
	if( err )
	{
		atr_ulog( kLogLevelWarning, "### Timing setup failed: %#m\n", err );
		_TimingFinalize( inSession );
	}
	return( err );
}

//===========================================================================================================================
//	_TimingFinalize
//===========================================================================================================================

static OSStatus	_TimingFinalize( AirPlayReceiverSessionRef inSession )
{
	OSStatus		err;
	Boolean			wasStarted;
	
	DEBUG_USE_ONLY( err );
	wasStarted = IsValidSocket( inSession->timingSock );
	
	// Signal the thread to quit and wait for it to signal back that it exited.
	
	if( inSession->timingThreadPtr )
	{
		err = SendSelfConnectedLoopbackMessage( inSession->timingCmdSock, "q", 1 );
		check_noerr( err );
		
		err = pthread_join( inSession->timingThread, NULL );
		check_noerr( err );
		inSession->timingThreadPtr = NULL;
	}
	
	// Clean up resources.
	
	ForgetSocket( &inSession->timingCmdSock );
	ForgetSocket( &inSession->timingSock );
	if( wasStarted ) atr_ulog( kLogLevelTrace, "Timing finalized\n" );
	return( kNoErr );
}

//===========================================================================================================================
//	_TimingNegotiate
//===========================================================================================================================

static OSStatus	_TimingNegotiate( AirPlayReceiverSessionRef inSession )
{
	SocketRef const		timingSock = inSession->timingSock;
	OSStatus			err;
	int					nFailure;
	int					nSuccess;
	int					nTimeouts;
	int					nSendErrors;
	int					nRecvErrors;
	OSStatus			lastSendError;
	OSStatus			lastRecvError;
	fd_set				readSet;
	int					n;
	struct timeval		timeout;
	
	require_action( inSession->timingThreadPtr == NULL, exit, err = kAlreadyInitializedErr );
	
	nFailure		= 0;
	nSuccess		= 0;
	nTimeouts		= 0;
	nRecvErrors		= 0;
	lastSendError	= kNoErr;
	lastRecvError	= kNoErr;
	FD_ZERO( &readSet );
	for( ;; )
	{
		// Send a request.
		
		nSendErrors = 0;
		for( ;; )
		{
			err = _TimingSendRequest( inSession );
			if( !err ) break;
			atr_ulog( kLogLevelWarning, "### Time sync send error: %#m\n", err );
			usleep( 100000 );
			if( err != lastSendError )
			{
				atr_ulog( kLogLevelNotice, "Time negotiate send error: %d\n", (int) err );
				lastSendError = err;
			}
			if( ++nSendErrors >= 64 )
			{
				atr_ulog( kLogLevelError, "Too many time negotiate send failures: %d\n", (int) err );
				goto exit;
			}
		}
		
		// Receive and process the response.
		
		for( ;; )
		{
			FD_SET( timingSock, &readSet );
			timeout.tv_sec  = 0;
			timeout.tv_usec = 500 * 1000;
			n = select( timingSock + 1, &readSet, NULL, NULL, &timeout );
			err = select_errno( n );
			if( err )
			{
				atr_ulog( kLogLevelWarning, "### Time sync select() error: %#m\n", err );
				#if( TARGET_OS_POSIX )
				if( err == EINTR ) continue;
				#endif
				++nTimeouts;
				++nFailure;
				if( err != lastRecvError )
				{
					atr_ulog( kLogLevelNotice, "Time negotiate receive error: %d\n", (int) err );
					lastRecvError = err;
				}
				break;
			}
			
			err = _TimingReceiveResponse( inSession, timingSock );
			if( err )
			{
				++nRecvErrors;
				++nFailure;
				atr_ulog( kLogLevelWarning, "### Time sync receive error: %#m\n", err );
				if( err != lastRecvError )
				{
					atr_ulog( kLogLevelNotice, "Time negotiate receive error: %d\n", (int) err );
					lastRecvError = err;
				}
				if( err == kDuplicateErr )
				{
					DrainUDPSocket( timingSock, 500, NULL );
				}
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
			atr_ulog( kLogLevelError, "Too many time negotiate failures: G=%d B=%d R=%d T=%d\n", 
				nSuccess, nFailure, nRecvErrors, nTimeouts );
			err = kTimeoutErr;
			goto exit;
		}
	}
	
	// Start the timing thread to keep our clock sync'd.
	
	err = pthread_create( &inSession->timingThread, NULL, _TimingThread, inSession );
	require_noerr( err, exit );
	inSession->timingThreadPtr = &inSession->timingThread;
	
	atr_ulog( kLogLevelTrace, "Timing started\n" );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_TimingThread
//===========================================================================================================================

static void *	_TimingThread( void *inArg )
{
	AirPlayReceiverSessionRef const		session	= (AirPlayReceiverSessionRef) inArg;
	SocketRef const						sock	= session->timingSock;
	SocketRef const						cmdSock	= session->timingCmdSock;
	fd_set								readSet;
	int									maxFd;
	struct timeval						timeout;
	int									n;
	OSStatus							err;
	
	SetThreadName( "AirPlayTimeSyncClient" );
	SetCurrentThreadPriority( kAirPlayThreadPriority_TimeSyncClient );
    atr_ulog( kLogLevelTrace, "Timing thread starting\n" );
		
	FD_ZERO( &readSet );
	maxFd = -1;
	if( (int) sock    > maxFd ) maxFd = sock;
	if( (int) cmdSock > maxFd ) maxFd = cmdSock;
	maxFd += 1;
	for( ;; )
	{
		FD_SET( sock, &readSet );
		FD_SET( cmdSock, &readSet );
		timeout.tv_sec  = 2;
		timeout.tv_usec = Random32() % 1000000;
		n = select( maxFd, &readSet, NULL, NULL, &timeout );
		err = select_errno( n );
		if( err == EINTR )					continue;
		if( err == kTimeoutErr )			{ _TimingSendRequest( session ); continue; }
		if( err )							{ dlogassert( "select() error: %#m", err ); usleep( 100000 ); continue; }
		if( FD_ISSET( sock,    &readSet ) ) _TimingReceiveResponse( session, sock );
		if( FD_ISSET( cmdSock, &readSet ) ) break; // The only event is quit so break if anything is pending.
	}
	atr_ulog( kLogLevelTrace, "Timing thread exit\n" );
	return( NULL );
}

//===========================================================================================================================
//	_TimingSendRequest
//
//	Note: This function does not need the AirTunes lock because it only accesses variables from a single thread at a time.
//		  These variables are only accessed once by the RTSP thread during init and then only by the timing thread.
//===========================================================================================================================

static OSStatus	_TimingSendRequest( AirPlayReceiverSessionRef inSession )
{
	OSStatus				err;
	AirTunesSource *		src;
	RTCPTimeSyncPacket		pkt;
	AirTunesTime			now;
	ssize_t					n;
	
	src = &inSession->source;
	
	// Build and send the request. The response is received asynchronously.
	
	pkt.v_p_m			= RTCPHeaderInsertVersion( 0, kRTPVersion );
	pkt.pt				= kRTCPTypeTimeSyncRequest;
	pkt.length			= htons( ( sizeof( pkt ) / 4 ) - 1 );
	pkt.rtpTimestamp	= 0;
	pkt.ntpOriginateHi	= 0;
	pkt.ntpOriginateLo	= 0;
	pkt.ntpReceiveHi	= 0;
	pkt.ntpReceiveLo	= 0;
	
	AirTunesClock_GetSynchronizedTime( inSession->airTunesClock, &now );
	src->rtcpTILastTransmitTimeHi = now.secs + kNTPvsUnixSeconds;
	src->rtcpTILastTransmitTimeLo = (uint32_t)( now.frac >> 32 );
	pkt.ntpTransmitHi	= htonl( src->rtcpTILastTransmitTimeHi );
	pkt.ntpTransmitLo	= htonl( src->rtcpTILastTransmitTimeLo );
	
	if( inSession->timingConnected )
	{
		n = send( inSession->timingSock, (char *) &pkt, sizeof( pkt ), 0 );
	}
	else
	{
		n = sendto( inSession->timingSock, (char *) &pkt, sizeof( pkt ), 0, &inSession->timingRemoteAddr.sa, 
			inSession->timingRemoteLen );
	}
	err = map_socket_value_errno( inSession->timingSock, n == (ssize_t) sizeof( pkt ), n );
	require_noerr_quiet( err, exit );
	debug_add( gAirTunesDebugSentByteCount, n );
	increment_wrap( src->rtcpTISendCount, 1 );
	
exit:
	if( err ) atr_ulog( kLogLevelNotice, "### NTP send request failed: %#m\n", err );
	return( err );
}

//===========================================================================================================================
//	_TimingReceiveResponse
//===========================================================================================================================

static OSStatus	_TimingReceiveResponse( AirPlayReceiverSessionRef inSession, SocketRef inSock )
{
	OSStatus			err;
	RTCPPacket			pkt;
	size_t				len;
	uint64_t			ticks;
	int					tmp;
	AirTunesTime		recvTime;
	
	err = SocketRecvFrom( inSock, &pkt, sizeof( pkt ), &len, NULL, 0, NULL, &ticks, NULL, NULL );
	if( err == EWOULDBLOCK ) goto exit;
	require_noerr( err, exit );
	debug_add( gAirTunesDebugRecvByteCount, len );
	if( len < sizeof( pkt.header ) )
	{
		dlogassert( "Bad size: %zu < %zu", sizeof( pkt.header ), len );
		err = kSizeErr;
		goto exit;
	}
	
	tmp = RTCPHeaderExtractVersion( pkt.header.v_p_c );
	if( tmp != kRTPVersion )
	{
		dlogassert( "Bad version: %d", tmp );
		err = kVersionErr;
		goto exit;
	}
	if( pkt.header.pt != kRTCPTypeTimeSyncResponse )
	{
		dlogassert( "Wrong packet type: %d", pkt.header.pt );
		err = kTypeErr;
		goto exit;
	}
	
	require_action( len >= sizeof( pkt.timeSync ), exit, err = kSizeErr );
	AirTunesClock_GetSynchronizedTimeNearUpTicks( inSession->airTunesClock, &recvTime, ticks );
	err = _TimingProcessResponse( inSession, &pkt.timeSync, &recvTime );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_TimingProcessResponse
//
//	Note: This function does not need the AirTunes lock because it only accesses variables from a single thread at a time.
//		  These variables are only accessed once by the RTSP thread during init and then only by the timing thread.
//===========================================================================================================================

static OSStatus	_TimingProcessResponse( AirPlayReceiverSessionRef inSession, RTCPTimeSyncPacket *inPkt, const AirTunesTime *inTime )
{
	AirTunesSource * const	src = &inSession->source;
	OSStatus				err;
	uint64_t				t1;
	uint64_t				t2;
	uint64_t				t3;
	uint64_t				t4;
	double					offset;
	double					rtt;
	double					rttAbs;
	unsigned int			i;
	double					median;
	Boolean					forceStep;
	
	inPkt->rtpTimestamp		= ntohl( inPkt->rtpTimestamp );
	inPkt->ntpOriginateHi	= ntohl( inPkt->ntpOriginateHi );
	inPkt->ntpOriginateLo	= ntohl( inPkt->ntpOriginateLo );
	inPkt->ntpReceiveHi		= ntohl( inPkt->ntpReceiveHi );
	inPkt->ntpReceiveLo		= ntohl( inPkt->ntpReceiveLo );
	inPkt->ntpTransmitHi	= ntohl( inPkt->ntpTransmitHi );
	inPkt->ntpTransmitLo	= ntohl( inPkt->ntpTransmitLo );
	
	// Make sure this response is for the last request we made and is not a duplicate response.
	
	if( ( inPkt->ntpOriginateHi != src->rtcpTILastTransmitTimeHi ) || 
		( inPkt->ntpOriginateLo != src->rtcpTILastTransmitTimeLo ) )
	{
		err = kDuplicateErr;
		goto exit;
	}
	src->rtcpTILastTransmitTimeHi = 0; // Zero so we don't try to process a duplicate.
	src->rtcpTILastTransmitTimeLo = 0;
	
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
	// Clock offset in NTP units     = ((T2 - T1) + (T3 - T4)) / 2.
	// Round-trip delay in NTP units =  (T4 - T1) - (T3 - T2)
	
	t1 = ( ( (uint64_t) inPkt->ntpOriginateHi )	<< 32 ) | inPkt->ntpOriginateLo;
	t2 = ( ( (uint64_t) inPkt->ntpReceiveHi )	<< 32 ) | inPkt->ntpReceiveLo;
	t3 = ( ( (uint64_t) inPkt->ntpTransmitHi )	<< 32 ) | inPkt->ntpTransmitLo;
	t4 = ( ( (uint64_t)( inTime->secs + kNTPvsUnixSeconds ) ) << 32 ) + ( inTime->frac >> 32 );
	
	offset = 0.5 * ( ( ( (double)( (int64_t)( t2 - t1 ) ) ) * kNTPFraction ) + 
					 ( ( (double)( (int64_t)( t3 - t4 ) ) ) * kNTPFraction ) );
	rtt = ( ( (double)( (int64_t)( t4 - t1 ) ) ) * kNTPFraction ) - 
		  ( ( (double)( (int64_t)( t3 - t2 ) ) ) * kNTPFraction );
	airtunes_record_clock_offset( offset );
	airtunes_record_clock_rtt( rtt );
	
	// Update round trip time stats. If the RTT is > 70 ms, it's probably a spurious error.
	// If we're not getting any good RTT's, use the best one we've received in the last 5 tries.
	// This avoids the initial time sync negotiate from failing entirely if RTT's are really high.
	// It also avoids time sync from drifting too much if RTT's become really high later.
	
	if( rtt < src->rtcpTIClockRTTMin ) src->rtcpTIClockRTTMin = rtt;
	if( rtt > src->rtcpTIClockRTTMax ) src->rtcpTIClockRTTMax = rtt;
	rttAbs = fabs( rtt );
	if( rttAbs > 0.070 )
	{
		++src->rtcpTIClockRTTOutliers;
		debug_increment_saturate( gAirTunesDebugTimeSyncHugeRTTCount, UINT_MAX );
		atr_ulog( kLogLevelVerbose, "### Large clock sync RTT %f, Avg %f, Min %f, Max %f, Offset %f\n", 
			rtt, src->rtcpTIClockRTTAvg, src->rtcpTIClockRTTMin, src->rtcpTIClockRTTMax, offset );
		
		if( rttAbs < fabs( src->rtcpTIClockRTTLeastBad ) )
		{
			src->rtcpTIClockRTTLeastBad		= rtt;
			src->rtcpTIClockOffsetLeastBad	= offset;
		}
		if( ++src->rtcpTIClockRTTBadCount < 5 )
		{
			err = kRangeErr;
			goto exit;
		}
		rtt		= src->rtcpTIClockRTTLeastBad;
		offset	= src->rtcpTIClockOffsetLeastBad;
		
		atr_ulog( kLogLevelInfo, "Replaced large clock sync RTT with %f, offset %f\n", rtt, offset );
	}
	if( src->rtcpTIResponseCount == 0 ) src->rtcpTIClockRTTAvg = rtt;
	src->rtcpTIClockRTTAvg			= ( ( 15.0 * src->rtcpTIClockRTTAvg ) + rtt ) * ( 1.0 / 16.0 );
	src->rtcpTIClockRTTBadCount		= 0;
	src->rtcpTIClockRTTLeastBad		= +1000000.0;
	src->rtcpTIClockOffsetLeastBad	= 0.0;
	
	// Update clock offset stats. If this is first time ever or the first time after a clock step, reset the stats.
	
	if( src->rtcpTIResetStats || ( src->rtcpTIResponseCount == 0 ) )
	{
		for( i = 0; i < countof( src->rtcpTIClockOffsetArray ); ++i )
		{
			src->rtcpTIClockOffsetArray[ i ] = offset;
		}
		src->rtcpTIClockOffsetIndex = 0;
		src->rtcpTIClockOffsetAvg = offset;
		src->rtcpTIClockOffsetMin = offset;
		src->rtcpTIClockOffsetMax = offset;
		median = offset;
	}
	else
	{
		src->rtcpTIClockOffsetArray[ src->rtcpTIClockOffsetIndex++ ] = offset;
		if( src->rtcpTIClockOffsetIndex >= countof( src->rtcpTIClockOffsetArray ) )
		{
			src->rtcpTIClockOffsetIndex = 0;
		}
		
		check_compile_time_code( countof( src->rtcpTIClockOffsetArray ) == 3 );
		median = median_of_3( src->rtcpTIClockOffsetArray[ 0 ], 
							  src->rtcpTIClockOffsetArray[ 1 ], 
							  src->rtcpTIClockOffsetArray[ 2 ] );
		
		src->rtcpTIClockOffsetAvg = ( ( 15.0 * src->rtcpTIClockOffsetAvg ) + offset ) * ( 1.0 / 16.0 );
		if( offset < src->rtcpTIClockOffsetMin ) src->rtcpTIClockOffsetMin = offset;
		if( offset > src->rtcpTIClockOffsetMax ) src->rtcpTIClockOffsetMax = offset;
	}
	
	// Sync our local clock to the server's clock. Use median to reject outliers. If this is the first sync, always step.
	
	forceStep = ( src->rtcpTIResponseCount == 0 );
	src->rtcpTIResetStats = AirTunesClock_Adjust( inSession->airTunesClock, (int64_t)( median * 1E9 ), forceStep );
	if( src->rtcpTIResetStats && !forceStep ) ++src->rtcpTIStepCount;
	++src->rtcpTIResponseCount;
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Screen ==
#endif

//===========================================================================================================================
//	Screen
//===========================================================================================================================

#define kAirPlayScreenReceiver_MaxFrameCount		8
#define kAirPlayScreenReceiver_MaxFrameSize			( 160 * 1024 )
#define kAirPlayScreenReceiver_SocketBufferSize		( ( 110 * kAirPlayScreenReceiver_MaxFrameSize ) / 100 ) // 10% extra

typedef struct
{
	AirPlayReceiverServerRef	server;
	CFArrayRef					timestampInfo;
	
}	ScreenTimestampInfoParams;

//===========================================================================================================================
//	_ScreenSetup
//===========================================================================================================================

static OSStatus
	_ScreenSetup(
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inStreamDesc,
		CFMutableDictionaryRef		inResponseParams )
{
	OSStatus						err;
	CFMutableDictionaryRef			responseStreamDesc = NULL;
	CFArrayRef						timestampInfo = NULL;
	int								receivePort;
	uint64_t						streamConnectionID = 0;
	uint8_t							aesScreenKey[ 16 ];
	uint8_t							aesScreenIV[ 16 ];
				
	require_action( !inSession->screenInitialized, exit2, err = kAlreadyInitializedErr );
	
	responseStreamDesc = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( responseStreamDesc, exit, err = kNoMemoryErr );
	
	err = AirPlayReceiverSessionScreen_Setup( inSession->screenSession, inStreamDesc, (uint32_t) inSession->clientSessionID );
	require_noerr( err, exit );
	
	streamConnectionID = (uint64_t) CFDictionaryGetInt64( inStreamDesc, CFSTR( kAirPlayKey_StreamConnectionID ), NULL );
	require_action( streamConnectionID, exit, err = kVersionErr );
	
	AirPlay_DeriveAESKeySHA512ForScreen( inSession->aesSessionKey, kAirTunesAESKeyLen, streamConnectionID, aesScreenKey, aesScreenIV );
	err = AirPlayReceiverSessionScreen_SetSecurityInfo( inSession->screenSession, aesScreenKey, aesScreenIV );
	MemZeroSecure( aesScreenKey, sizeof( aesScreenKey ) );
	MemZeroSecure( aesScreenIV, sizeof( aesScreenIV ) );
	require_noerr( err, exit );

	err = ServerSocketOpen( inSession->peerAddr.sa.sa_family, SOCK_STREAM, IPPROTO_TCP, -kAirPlayPort_RTPScreen, 
		&receivePort, -kAirPlayScreenReceiver_SocketBufferSize, &inSession->screenSock );
	require_noerr( err, exit );
	
	if( !inSession->server->qosDisabled )	SocketSetQoS( inSession->screenSock, kSocketQoS_AirPlayScreenVideo );
	
	CFDictionarySetInt64( responseStreamDesc, CFSTR( kAirPlayKey_Type ), kAirPlayStreamType_Screen );
	CFDictionarySetInt64( responseStreamDesc, CFSTR( kAirPlayKey_Port_Data ), receivePort );
	
	err = _AddResponseStream( inResponseParams, responseStreamDesc );
	require_noerr( err, exit );
	
	inSession->screenInitialized = true;
	atr_ulog( kLogLevelTrace, "screen receiver set up on port %d\n", receivePort );

	// If the session's already started then immediately start the thread process it.
	
	if( inSession->sessionStarted )
	{
		err = _ScreenStart( inSession );
		require_noerr( err, exit );
	}
	
exit:
	CFReleaseNullSafe( responseStreamDesc );
	CFReleaseNullSafe( timestampInfo );
	if( err ) _ScreenTearDown( inSession );
	
exit2:
	if( err ) atr_ulog( kLogLevelWarning, "### screen receiver setup failed: %#m\n", err );
	return( err );
}

//===========================================================================================================================
//	_ScreenTearDown
//===========================================================================================================================

static void	_ScreenTearDown( AirPlayReceiverSessionRef inSession )
{
	OSStatus		err;
	
	DEBUG_USE_ONLY( err );
	
	if( inSession->screenThreadPtr )
	{
		err = AirPlayReceiverSessionScreen_SendCommand( inSession->screenSession, kAirPlayReceiverSessionScreenCommand_Quit, NULL, 0 );
		check_noerr( err );

		err = pthread_join( inSession->screenThread, NULL );
		check_noerr( err );
		inSession->screenThreadPtr = NULL;
	}
	
	ForgetSocket( &inSession->screenSock );
	
	if( inSession->screenInitialized ) atr_ulog( kLogLevelTrace, "screen receiver torn down\n" );
	inSession->screenInitialized = false;
}

//===========================================================================================================================
//	_ScreenStart
//===========================================================================================================================

static OSStatus	_ScreenStart( AirPlayReceiverSessionRef inSession )
{
	OSStatus			err;
	
	require_action_quiet( !inSession->screenThreadPtr, exit, err = kNoErr );

	err = pthread_create( &inSession->screenThread, NULL, _ScreenThread, inSession );
	require_noerr( err, exit );
	inSession->screenThreadPtr = &inSession->screenThread;

exit:
	if( err ) atr_ulog( kLogLevelWarning, "### screen start failed: %#m\n", err );
	return( err );
}

//===========================================================================================================================
//	_ScreenThread
//===========================================================================================================================

static void *	_ScreenThread( void *inArg )
{
	AirPlayReceiverSessionRef const					session = (AirPlayReceiverSessionRef) inArg;
	OSStatus										err;
	NetSocketRef									netSock = NULL;
	AirPlayReceiverSessionScreenTimeSynchronizer	timeSynchronizer;
	CFMutableDictionaryRef							params = NULL;
	SocketRef										newSock = kInvalidSocketRef;
	
	SetThreadName( "AirPlayScreenReceiver" );
	SetCurrentThreadPriority( kAirPlayThreadPriority_ScreenReceiver );
	
	// Wait for the client to connect and then replace the acceptor socket with the new data socket.
	
	err = SocketAccept( session->screenSock, kAirPlayConnectTimeoutSecs, &newSock, NULL );
	require_noerr( err, exit );
	
	ForgetSocket( &session->screenSock ); // don't need listening socket anymore.
	
	atr_ulog( kLogLevelTrace, "screen receiver started\n" );

	AirPlayReceiverSessionScreen_SetSessionUUID( session->screenSession, session->sessionUUID );
	AirPlayReceiverSessionScreen_SetClientDeviceID( session->screenSession, session->clientDeviceID );

	err = NetSocket_CreateWithNative( &netSock, newSock );
	require_noerr( err, exit );
	
	newSock = kInvalidSocketRef; // netSock now owns newSock.
	
	timeSynchronizer.context								= session;
	timeSynchronizer.getSynchronizedNTPTimeFunc				= _ScreenGetSynchronizedNTPTime;
	timeSynchronizer.getUpTicksNearSynchronizedNTPTimeFunc	= _ScreenGetUpTicksNearSynchronizedNTPTime;

	AirPlayReceiverSessionScreen_SetTimeSynchronizer( session->screenSession, &timeSynchronizer );
	
	AirPlayReceiverSessionScreen_SetReceiveEndTime( session->screenSession, CFAbsoluteTimeGetCurrent() );
	AirPlayReceiverSessionScreen_SetAuthEndTime( session->screenSession, CFAbsoluteTimeGetCurrent() );
	AirPlayReceiverSessionScreen_SetNTPEndTime( session->screenSession, CFAbsoluteTimeGetCurrent() );

	err = AirPlayReceiverSessionScreen_StartSession( session->screenSession, session->server->screenStreamOptions );
	require_noerr( err, exit );
	
	params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( params, exit, err = kNoMemoryErr );
	
	AirPlayReceiverSessionScreen_LogStarted( session->screenSession, params, session->transportType );
	
	err = AirPlayReceiverSessionScreen_ProcessFrames( session->screenSession, netSock, session->server->timeoutDataSecs );
	require_noerr( err, exit );
	
exit:
	AirPlayReceiverSessionScreen_LogEnded( session->screenSession, err );
	AirPlayReceiverSessionScreen_StopSession( session->screenSession );
	NetSocket_Forget( &netSock );
	ForgetSocket( &newSock );
	CFReleaseNullSafe( params );
    atr_ulog( kLogLevelTrace, "Screen thread exit\n" );
	return( NULL );
}

//===========================================================================================================================
//	_ScreenHandleEvent
//===========================================================================================================================
static void
	_ScreenHandleEvent(
		AirPlayReceiverSessionScreenRef		inSession,
		CFStringRef							inEventName,
		CFDictionaryRef						inEventData,
		void *								inUserData )
{
	AirPlayReceiverSessionRef const		me = (AirPlayReceiverSessionRef) inUserData;
	OSStatus							err;
	uint32_t							frameCount;
	CFArrayRef							metrics;
	CFMutableDictionaryRef				request;
	
	(void) inSession;
	
	if( 0 ) {} // Empty if to simplify else if's below.
	
	// ForceKeyFrame
	
	else if( CFEqual( inEventName, CFSTR( kAirPlayReceiverSessionScreenEvent_ForceKeyFrameNeeded ) ) )
	{
		request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require( request, exit );
		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_ForceKeyFrame ) );
		err = AirPlayReceiverSessionSendCommand( me, request, NULL, NULL );
		CFRelease( request );
		require_noerr( err, exit );
	}
	
	// TimestampsUpdated
	
	else if( CFEqual( inEventName, CFSTR( kAirPlayReceiverSessionScreenEvent_TimestampsUpdated ) ) )
	{
		frameCount = (uint32_t) CFDictionaryGetInt64( inEventData, CFSTR( "frameCount" ), NULL );
		metrics = CFDictionaryGetCFArray( inEventData, CFSTR( "metrics" ), NULL );
		if( metrics )
		{
			AirPlayReceiverServerControlAsyncF( me->server, CFSTR( kAirPlayCommand_UpdateTimestamps ), NULL,
				NULL, NULL, NULL, 
				"{"
					"%kO=%i"
					"%kO=%O"
				"}", 
				CFSTR( "frameCount" ),	frameCount, 
				CFSTR( "metrics" ),		metrics );
		}
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ScreenGetSynchronizedNTPTime
//===========================================================================================================================

static uint64_t _ScreenGetSynchronizedNTPTime( void *inContext )
{
	AirPlayReceiverSessionRef me =  (AirPlayReceiverSessionRef) inContext;
	
	return AirTunesClock_GetSynchronizedNTPTime( me->airTunesClock );
}

//===========================================================================================================================
//	_ScreenGetUpTicksNearSynchronizedNTPTime
//===========================================================================================================================

static uint64_t _ScreenGetUpTicksNearSynchronizedNTPTime( void *inContext, uint64_t inNTPTime )
{
	AirPlayReceiverSessionRef me =  (AirPlayReceiverSessionRef) inContext;

	return AirTunesClock_GetUpTicksNearSynchronizedNTPTime( me->airTunesClock, inNTPTime );
}

#if 0
#pragma mark -
#pragma mark == Utils ==
#endif

//===========================================================================================================================
//	_AddResponseStream
//===========================================================================================================================

static OSStatus	_AddResponseStream( CFMutableDictionaryRef inResponseParams, CFDictionaryRef inStreamDesc )
{
	OSStatus				err;
	CFMutableArrayRef		responseStreams;
	
	responseStreams = (CFMutableArrayRef) CFDictionaryGetCFArray( inResponseParams, CFSTR( kAirPlayKey_Streams ), NULL );
	if( !responseStreams )
	{
		responseStreams = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action( responseStreams, exit, err = kNoMemoryErr );
		CFArrayAppendValue( responseStreams, inStreamDesc );
		CFDictionarySetValue( inResponseParams, CFSTR( kAirPlayKey_Streams ), responseStreams );
		CFRelease( responseStreams );
	}
	else
	{
		CFArrayAppendValue( responseStreams, inStreamDesc );
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_LogStarted
//===========================================================================================================================

static void	_LogStarted( AirPlayReceiverSessionRef inSession, AirPlayReceiverSessionStartInfo *inInfo, OSStatus inStatus )
{
	inSession->startStatus = inStatus;
	inInfo->recordMs = (uint32_t)( UpTicksToMilliseconds( UpTicks() - inSession->playTicks ) );
	
	atr_ulog( kLogLevelNotice,
		"AirPlay session started: From=%s D=0x%012llx A=%##a "
		"T=%s C=%s Bonjour=%u ms Conn=%u ms Auth=%u ms Ann=%u ms Setup=%u ms %s%?u%sRec=%u ms: %#m\n",
		inInfo->clientName, inSession->clientDeviceID, &inSession->peerAddr,
		NetTransportTypeToString( inInfo->transportType ), AirPlayCompressionTypeToString( inSession->compressionType ),
		inInfo->bonjourMs, inInfo->connectMs, inInfo->authMs, inInfo->announceMs, inInfo->setupAudioMs,
		inSession->screen ? "Scr=" : "", inSession->screen, inInfo->setupScreenMs, inSession->screen ? " ms " : "",
		inInfo->recordMs, inStatus );
	
}

//===========================================================================================================================
//	_LogEnded
//===========================================================================================================================

static void	_LogEnded( AirPlayReceiverSessionRef inSession, OSStatus inReason )
{
	DataBuffer							db;
#if( TARGET_OS_POSIX )
	char								buf[ 2048 ];
#else
	char								buf[ 512 ];
#endif
	uint32_t const						durationSecs			= (uint32_t) UpTicksToSeconds( UpTicks() - inSession->sessionTicks );
	
	DataBuffer_Init( &db, buf, sizeof( buf ), 10000 );
	DataBuffer_AppendF( &db, "AirPlay session ended: Dur=%{dur} Reason=%#m\n", durationSecs, inReason );
	atr_ulog( kLogLevelNotice, "%.*s\n", (int) DataBuffer_GetLen( &db ), DataBuffer_GetPtr( &db ) );
	DataBuffer_Free( &db );
	
}

//===========================================================================================================================
//	_LogUpdate
//===========================================================================================================================

static void	_LogUpdate( AirPlayReceiverSessionRef inSession, uint64_t inTicks, Boolean inForce )
{
	(void) inSession;
	(void) inTicks;
	(void) inForce;
}

//===========================================================================================================================
//	_TearDownStream
//===========================================================================================================================

static void	_TearDownStream( AirPlayReceiverSessionRef inSession, AirPlayAudioStreamContext * const ctx )
{
	OSStatus		err;
	
	DEBUG_USE_ONLY( err );
	
	if( ctx->threadPtr )
	{
		err = SendSelfConnectedLoopbackMessage( ctx->cmdSock, "q", 1 );
		check_noerr( err );
		
		err = pthread_join( ctx->thread, NULL );
		check_noerr( err );
		ctx->threadPtr = NULL;
	}
	ForgetSocket( &ctx->cmdSock );
	ForgetSocket( &ctx->dataSock );
	RTPJitterBufferFree( &ctx->jitterBuffer );
	MirroredRingBufferFree( &ctx->inputRing );
	
	(void) inSession;
	ctx->session = NULL;
	if( ctx->type != kAirPlayStreamType_Invalid )
	{
		ctx->type = kAirPlayStreamType_Invalid;
		atr_ulog( kLogLevelTrace, "%s audio torn down\n", ctx->label );
	}
}

//===========================================================================================================================
//	_UpdateEstimatedRate
//===========================================================================================================================

static void	_UpdateEstimatedRate( AirPlayAudioStreamContext *ctx, uint32_t inSampleTime, uint64_t inHostTime )
{
	uint32_t					oldCount, newCount;
	AirPlayTimestampTuple *		newSample;
	AirPlayTimestampTuple *		oldSample;
	AirTunesTime				atTime;
	double						scale, rate;
	AirTunesClockRef			airTunesClock = ctx->session ? ctx->session->airTunesClock : NULL;
	
	if( inHostTime >= ctx->rateUpdateNextTicks )
	{
		oldCount = ctx->rateUpdateCount;
		newCount = oldCount + 1;
		AirTunesClock_GetSynchronizedTimeNearUpTicks( airTunesClock, &atTime, inHostTime );
		newSample				= &ctx->rateUpdateSamples[ oldCount % countof( ctx->rateUpdateSamples ) ];
		newSample->hostTime		= AirTunesTime_ToNTP( &atTime );
		newSample->sampleTime	= inSampleTime;
		if( newCount >= 8 )
		{
			oldSample = &ctx->rateUpdateSamples[ newCount % Min( newCount, countof( ctx->rateUpdateSamples ) ) ];
			scale = ( newSample->hostTime - oldSample->hostTime ) * kNTPUnits_FP;
			if( scale > 0 )
			{
				rate = ( newSample->sampleTime - oldSample->sampleTime ) / scale;
				ctx->rateAvg = (Float32) MovingAverageF( ctx->rateAvg, rate, 0.125 );
				atr_stats_ulog( ( kLogLevelVerbose + 1 ) | kLogLevelFlagDontRateLimit, "%s: Estimated rate: %.3f\n", 
					ctx->label, ctx->rateAvg );
			}
		}
		ctx->rateUpdateCount = newCount;
		ctx->rateUpdateNextTicks = inHostTime + ctx->rateUpdateIntervalTicks;
	}
}

#if 0
#pragma mark -
#pragma mark == Helpers ==
#endif

//===========================================================================================================================
//	AirPlayReceiverSessionChangeModes
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionChangeModes( 
		AirPlayReceiverSessionRef					inSession, 
		const AirPlayModeChanges *					inChanges, 
		CFStringRef									inReason, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDictionaryRef				params;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_ChangeModes ) );
	
	params = AirPlayCreateModesDictionary( inChanges, inReason, &err );
	require_noerr( err, exit );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), params );
	CFRelease( params );
	
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionChangeAppState
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionChangeAppState( 
		AirPlayReceiverSessionRef					inSession, 
		AirPlaySpeechMode							inSpeechMode, 
		AirPlayTriState								inPhoneCall, 
		AirPlayTriState								inTurnByTurn, 	
		CFStringRef									inReason, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	OSStatus				err;
	AirPlayModeChanges		changes;
	
	AirPlayModeChangesInit( &changes );
	if( inSpeechMode != kAirPlaySpeechMode_NotApplicable )	changes.speech		= inSpeechMode;
	if( inPhoneCall  != kAirPlaySpeechMode_NotApplicable )	changes.phoneCall	= inPhoneCall;
	if( inTurnByTurn != kAirPlaySpeechMode_NotApplicable )	changes.turnByTurn	= inTurnByTurn;
	
	err = AirPlayReceiverSessionChangeModes( inSession, &changes, inReason, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionChangeResourceMode
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionChangeResourceMode( 
		AirPlayReceiverSessionRef					inSession, 
		AirPlayResourceID							inResourceID, 
		AirPlayTransferType							inType, 
		AirPlayTransferPriority						inPriority, 
		AirPlayConstraint							inTakeConstraint, 
		AirPlayConstraint							inBorrowOrUnborrowConstraint, 
		CFStringRef									inReason, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	OSStatus				err;
	AirPlayModeChanges		changes;
	
	AirPlayModeChangesInit( &changes );
	if( inResourceID == kAirPlayResourceID_MainScreen )
	{
		changes.screen.type								= inType;
		changes.screen.priority							= inPriority;
		changes.screen.takeConstraint					= inTakeConstraint;
		changes.screen.borrowOrUnborrowConstraint		= inBorrowOrUnborrowConstraint;
	}
	else if( inResourceID == kAirPlayResourceID_MainAudio )
	{
		changes.mainAudio.type							= inType;
		changes.mainAudio.priority						= inPriority;
		changes.mainAudio.takeConstraint				= inTakeConstraint;
		changes.mainAudio.borrowOrUnborrowConstraint	= inBorrowOrUnborrowConstraint;
	}
	else
	{
		dlogassert( "Bad resource ID: %d\n", inResourceID );
		err = kParamErr;
		goto exit;
	}
	
	err = AirPlayReceiverSessionChangeModes( inSession, &changes, inReason, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionMakeModeStateFromDictionary
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionMakeModeStateFromDictionary( 
		AirPlayReceiverSessionRef	inSession, 
		CFDictionaryRef				inDict, 
		AirPlayModeState *			outModes )
{
	OSStatus			err;
	CFArrayRef			array;
	CFIndex				i, n;
	CFDictionaryRef		dict;
	CFStringRef			cfstr;
	int					x;
	
	(void) inSession;
	
	AirPlayModeStateInit( outModes );
	
	// AppStates
	
	array = CFDictionaryGetCFArray( inDict, CFSTR( kAirPlayKey_AppStates ), NULL );
	n = array ? CFArrayGetCount( array ) : 0;
	for( i = 0; i < n; ++i )
	{
		dict = CFArrayGetCFDictionaryAtIndex( array, i, &err );
		require_noerr( err, exit );
		
		cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_AppStateID ), NULL );
		if( cfstr )	x = AirPlayAppStateIDFromCFString( cfstr );
		else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_AppStateID ), NULL );
		switch( x )
		{
			case kAirPlayAppStateID_PhoneCall:
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_Entity ), &err );
				if( cfstr )	x = AirPlayEntityFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_Entity ), &err );
				require_noerr( err, exit );
				outModes->phoneCall = x;
				break;
			
			case kAirPlayAppStateID_Speech:
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_Entity ), &err );
				if( cfstr )	x = AirPlayEntityFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_Entity ), &err );
				require_noerr( err, exit );
				outModes->speech.entity = x;
				
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_SpeechMode ), &err );
				if( cfstr )	x = AirPlaySpeechModeFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_SpeechMode ), &err );
				require_noerr( err, exit );
				outModes->speech.mode = x;
				break;
			
			case kAirPlayAppStateID_TurnByTurn:
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_Entity ), &err );
				if( cfstr )	x = AirPlayEntityFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_Entity ), &err );
				require_noerr( err, exit );
				outModes->turnByTurn = x;
				break;
			
			case kAirPlayAppStateID_NotApplicable:
				break;
			
			default:
				atr_ulog( kLogLevelNotice, "### Ignoring unknown app state %@\n", dict );
				break;
		}
	}
	
	// Resources
	
	array = CFDictionaryGetCFArray( inDict, CFSTR( kAirPlayKey_Resources ), NULL );
	n = array ? CFArrayGetCount( array ) : 0;
	for( i = 0; i < n; ++i )
	{
		dict = CFArrayGetCFDictionaryAtIndex( array, i, &err );
		require_noerr( err, exit );
		
		cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_ResourceID ), NULL );
		if( cfstr )	x = AirPlayResourceIDFromCFString( cfstr );
		else		x = (AirPlayAppStateID) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_ResourceID ), NULL );
		switch( x )
		{
			case kAirPlayResourceID_MainScreen:
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_Entity ), &err );
				if( cfstr )	x = AirPlayEntityFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_Entity ), &err );
				require_noerr( err, exit );
				outModes->screen = x;
				break;
			
			case kAirPlayResourceID_MainAudio:
				cfstr = CFDictionaryGetCFString( dict, CFSTR( kAirPlayKey_Entity ), &err );
				if( cfstr )	x = AirPlayEntityFromCFString( cfstr );
				else		x = (int) CFDictionaryGetInt64( dict, CFSTR( kAirPlayKey_Entity ), &err );
				require_noerr( err, exit );
				outModes->mainAudio = x;
				break;
			
			case kAirPlayResourceID_NotApplicable:
				break;
			
			default:
				atr_ulog( kLogLevelNotice, "### Ignoring unknown resource state %@\n", dict );
				break;
		}
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionForceKeyFrame
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionForceKeyFrame(
		AirPlayReceiverSessionRef					inSession,
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion,
		void *										inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_ForceKeyFrame ) );
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionRequestSiriAction
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionRequestSiriAction(
		AirPlayReceiverSessionRef					inSession,
		AirPlaySiriAction							inAction,
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion,
		void *										inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFMutableDictionaryRef		params;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_RequestSiri ) );
	
	params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( params, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( params, CFSTR( kAirPlayKey_SiriAction ), (int64_t) inAction );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), params );
	CFRelease( params );
	
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionRequestUI
//===========================================================================================================================

EXPORT_GLOBAL
OSStatus
	AirPlayReceiverSessionRequestUI( 
		AirPlayReceiverSessionRef					inSession, 
		CFStringRef									inURL, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFMutableDictionaryRef		params;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_RequestUI ) );
	
	if( inURL )
	{
		params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( params, exit, err = kNoMemoryErr );
		CFDictionarySetValue( params, CFSTR( kAirPlayKey_URL ), inURL );
		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), params );
		CFRelease( params );
	}
	
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetNightMode
//===========================================================================================================================

OSStatus
AirPlayReceiverSessionSetNightMode(
	AirPlayReceiverSessionRef					inSession,
	Boolean										inNightMode,
	AirPlayReceiverSessionCommandCompletionFunc	inCompletion,
	void *										inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFMutableDictionaryRef		params;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_SetNightMode ) );
	
	params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( params, exit, err = kNoMemoryErr );
	CFDictionarySetValue( params, CFSTR( kAirPlayKey_NightMode ), inNightMode ? kCFBooleanTrue : kCFBooleanFalse );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), params );
	CFRelease( params );
	
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetLimitedUI
//===========================================================================================================================

OSStatus
AirPlayReceiverSessionSetLimitedUI(
	AirPlayReceiverSessionRef						inSession,
	Boolean											inLimitUI,
	AirPlayReceiverSessionCommandCompletionFunc		inCompletion,
	void *											inContext )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFMutableDictionaryRef		params;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Type ), CFSTR( kAirPlayCommand_SetLimitedUI ) );
	
	params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( params, exit, err = kNoMemoryErr );
	CFDictionarySetValue( params, CFSTR( kAirPlayKey_LimitedUI ), inLimitUI ? kCFBooleanTrue : kCFBooleanFalse );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), params );
	CFRelease( params );
	
	err = AirPlayReceiverSessionSendCommand( inSession, request, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

//===========================================================================================================================
//	AirTunesDebugControl
//===========================================================================================================================

#if( DEBUG )

void	AirTunesDebugControl_Tweak( DataBuffer *inDB, const char *inCmd );

OSStatus	AirTunesDebugControl( const char *inCmd, CFStringRef *outOutput )
{
	OSStatus			err;
	DataBuffer			db;
	const char *		key;
	size_t				keyLen;
	const char *		value;
	int					n;
	
	DataBuffer_Init( &db, NULL, 0, SIZE_MAX );
	
	if( *inCmd == '\0' ) inCmd = "help";
	key = inCmd;
	for( value = inCmd; ( *value != '\0' ) && ( *value != '=' ); ++value ) {}
	keyLen = (size_t)( value - key );
	if( *value != '\0' ) ++value;
	
	// drop -- simulate dropped packets.
	
	else if( strncmpx( key, keyLen, "drop" ) == 0 )
	{
		int		minDropRate, maxDropRate, minSkipRate, maxSkipRate;
		
		n = sscanf( value, "%d,%d,%d,%d", &minDropRate, &maxDropRate, &minSkipRate, &maxSkipRate );
		if( n == 4 )
		{
			gAirTunesDropMinRate = minDropRate;
			gAirTunesDropMaxRate = maxDropRate;
			gAirTunesSkipMinRate = minSkipRate;
			gAirTunesSkipMaxRate = maxSkipRate;
			gAirTunesDropRemaining = 0;
			gAirTunesSkipRemaining = 0;
			
			DataBuffer_AppendF( &db, "Starting simulated packet loss: drop %d-%d packets every %d-%d packets\n", 
				minDropRate, maxDropRate, minSkipRate, maxSkipRate );
		}
		else
		{
			gAirTunesDropMinRate = 0;
			DataBuffer_AppendF( &db, "Stopped simulated packet loss\n" );
		}
	}
	else if( strncmpx( key, keyLen, "latedrop" ) == 0 )
	{
		gAirTunesLateDrop = 0;
		sscanf( value, "%d", &gAirTunesLateDrop );
		if( gAirTunesLateDrop > 0 ) DataBuffer_AppendF( &db, "Late dropping every %d packets\n", gAirTunesLateDrop );
		else						DataBuffer_AppendF( &db, "Disabling late drop\n" );
	}
	
	// rs -- Resets all debug stats.
	
	else if( strncmpx( key, keyLen, "rs" ) == 0 )
	{
		AirTunesDebugControl_ResetDebugStats();
		DataBuffer_AppendF( &db, "Reset debug stats\n" );
	}
	
	// file -- Save audio to a file.
	
#if( TARGET_HAS_C_LIB_IO )
	else if( strncmpx( key, keyLen, "file" ) == 0 )
	{
		char		path[ PATH_MAX + 1 ];
		
		if( gAirTunesFile )
		{
			fclose( gAirTunesFile );
			gAirTunesFile = NULL;
		}
		
		if( *value != '\0' )
		{
			snprintf( path, sizeof( path ), "%s", value );
			gAirTunesFile = fopen( path, "wb" );
			err = map_global_value_errno( gAirTunesFile, gAirTunesFile );
			require_noerr( err, exit );
			
			DataBuffer_AppendF( &db, "starting record of audio at file \"%s\"\n", value );
		}
	}
#endif
	
	// Help
	
	else if( strncmpx( key, keyLen, "help" ) == 0 )
	{
		DataBuffer_AppendF( &db, "-- AirTunes Debug Control Commands --\n" );
		DataBuffer_AppendF( &db, "    aro                    -- Adjust relative RTP offset.\n" );
		DataBuffer_AppendF( &db, "    rs                     -- Reset all debug stats.\n" );
		DataBuffer_AppendF( &db, "    perf                   -- Start/stop performance monitoring.\n" );
		DataBuffer_AppendF( &db, "    perfMode               -- Set performance monitoring mode.\n" );
		DataBuffer_AppendF( &db, "    hwskew <force>,<skew>  -- Set performance monitoring mode.\n" );
		DataBuffer_AppendF( &db, "\n" );
	}
	
	// Unknown
	
	else
	{
		DataBuffer_AppendF( &db, "### unknown command: '%s'\n", inCmd );
		goto exitOutput;
	}
	
exitOutput:
		
	// Return as a CFString.
	
	if( outOutput )
	{
		CFStringRef		output;
		
		output = CFStringCreateWithBytes( NULL, db.bufferPtr, (CFIndex) db.bufferLen, kCFStringEncodingUTF8, false );
		require_action( output, exit, err = kNoMemoryErr );
		*outOutput = output;
	}
	else
	{
		dlog( kLogLevelMax, "%.*s", (int) db.bufferLen, db.bufferPtr );
	}
	err = kNoErr;
	
exit:
	DataBuffer_Free( &db );
	return( err );
}
#endif

//===========================================================================================================================
//	AirTunesDebugControl_ResetDebugStats
//===========================================================================================================================

#if( DEBUG )
void	AirTunesDebugControl_ResetDebugStats( void )
{
	gAirTunesDebugBusyNodeCountMax					= 0;
	
	gAirTunesDebugSentByteCount						= 0;
	gAirTunesDebugRecvByteCount						= 0;
	gAirTunesDebugRecvRTPOriginalByteCount			= 0;
	gAirTunesDebugRecvRTPOriginalByteCountLast		= 0;
	gAirTunesDebugRecvRTPOriginalBytesPerSecAvg		= 0;
	gAirTunesDebugRecvRTPRetransmitByteCount		= 0;
	gAirTunesDebugRecvRTPRetransmitByteCountLast	= 0;
	gAirTunesDebugRecvRTPRetransmitBytesPerSecAvg	= 0;
	gAirTunesDebugIdleTimeoutCount					= 0;
	gAirTunesDebugStolenNodeCount					= 0;
	gAirTunesDebugOldDiscardCount					= 0;
	gAirTunesDebugConcealedGapCount					= 0;
	gAirTunesDebugConcealedEndCount					= 0;
	gAirTunesDebugLateDropCount						= 0;
	gAirTunesDebugSameTimestampCount				= 0;
	memset( gAirTunesDebugLossCounts, 0, sizeof( gAirTunesDebugLossCounts ) );
	gAirTunesDebugTotalLossCount					= 0;
	gAirTunesDebugMaxBurstLoss						= 0;
	gAirTunesDebugDupCount							= 0;
	gAirTunesDebugMisorderedCount					= 0;
	gAirTunesDebugUnrecoveredPacketCount			= 0;
	gAirTunesDebugUnexpectedRTPOffsetResetCount 	= 0;
	gAirTunesDebugHugeSkewResetCount				= 0;
	gAirTunesDebugGlitchCount						= 0;
	gAirTunesDebugTimeSyncHugeRTTCount				= 0;
	gAirTunesDebugTimeAnnounceMinNanos				= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	gAirTunesDebugTimeAnnounceMaxNanos				= 0;
	gAirTunesDebugRetransmitActiveCount				= 0;
	gAirTunesDebugRetransmitActiveMax				= 0;
	gAirTunesDebugRetransmitSendCount				= 0;
	gAirTunesDebugRetransmitSendLastCount			= 0;
	gAirTunesDebugRetransmitSendPerSecAvg			= 0;
	gAirTunesDebugRetransmitRecvCount				= 0;
	gAirTunesDebugRetransmitBigLossCount			= 0;
	gAirTunesDebugRetransmitAbortCount				= 0;
	gAirTunesDebugRetransmitFutileAbortCount		= 0;
	gAirTunesDebugRetransmitNoFreeNodesCount		= 0;
	gAirTunesDebugRetransmitNotFoundCount			= 0;
	gAirTunesDebugRetransmitPrematureCount			= 0;
	gAirTunesDebugRetransmitMaxTries				= 0;
	gAirTunesDebugRetransmitMinNanos				= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	gAirTunesDebugRetransmitMaxNanos				= 0;
	gAirTunesDebugRetransmitAvgNanos				= 0;
	gAirTunesDebugRetransmitRetryMinNanos			= UINT64_C( 0xFFFFFFFFFFFFFFFF );
	gAirTunesDebugRetransmitRetryMaxNanos			= 0;
	
	gAirTunesClockOffsetIndex 						= 0;
	gAirTunesClockOffsetCount 						= 0;
	
	gAirTunesClockRTTIndex 							= 0;
	gAirTunesClockRTTCount 							= 0;
	
	gAirTunesRTPOffsetIndex 						= 0;
	gAirTunesRTPOffsetCount 						= 0;
	
	dlog( kLogLevelMax, "AirPlay debugging stats reset\n" );
}
#endif

//===========================================================================================================================
//	AirTunesDebugControl_Tweak
//===========================================================================================================================

#if( DEBUG )
void	AirTunesDebugControl_Tweak( DataBuffer *inDB, const char *inCmd )
{
	const char *		name;
	size_t				nameLen;
	const char *		value;
	
	name = inCmd;
	for( value = inCmd; ( *value != '\0' ) && ( *value != '=' ); ++value ) {}
	if( *value != '=' )
	{
		DataBuffer_AppendF( inDB, "### malformed tweak: '%s'\n", inCmd );
		goto exit;
	}
	nameLen = (size_t)( value - name );
	++value;
	
	if( 0 ) {}
	else
	{
		(void) nameLen;
		DataBuffer_AppendF( inDB, "### unknown tweak: '%s'\n", inCmd );
	}
	
exit:
	return;
}
#endif

//===========================================================================================================================
//	AirTunesDebugShow - Console show routine.
//===========================================================================================================================

OSStatus	AirTunesDebugShow( const char *inCmd, CFStringRef *outOutput )
{
	OSStatus		err;
	DataBuffer		dataBuf;
	CFStringRef		output;
	
	DataBuffer_Init( &dataBuf, NULL, 0, SIZE_MAX );
	
	err = AirTunesDebugAppendShowData( inCmd, &dataBuf );
	require_noerr( err, exit );
		
	if( outOutput )
	{
		output = CFStringCreateWithBytes( NULL, dataBuf.bufferPtr, (CFIndex) dataBuf.bufferLen, kCFStringEncodingUTF8, false );
		require_action( output, exit, err = kNoMemoryErr );
		*outOutput = output;
	}
	else
	{
		dlog( kLogLevelMax, "%.*s", (int) dataBuf.bufferLen, dataBuf.bufferPtr );
	}
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}

//===========================================================================================================================
//	AirTunesDebugAppendShowData - Console show routine.
//===========================================================================================================================

OSStatus	AirTunesDebugAppendShowData( const char *inCmd, DataBuffer *inDB )
{
	OSStatus				err;
	uint32_t				runningSecs;
	AirTunesSource *		src;
#if( DEBUG )
	unsigned int			i;
#endif
	Boolean					b;
	
	// globals -- Show globals.
	
	if( !inCmd || ( *inCmd == '\0' ) || ( strcmp( inCmd, "globals" ) == 0 ) )
	{
		DataBuffer_AppendF( inDB, "\n" );
		DataBuffer_AppendF( inDB, "+-+ AirPlay Audio Statistics +-+\n" );
		if( gAirTunes )
		{
			b = (Boolean)( gAirTunes->compressionType != kAirPlayCompressionType_PCM );
			DataBuffer_AppendF( inDB, "    Encoding Mode            = %s\n", 
				( gAirTunes->decryptor && b )	? "EC"	:
				  gAirTunes->decryptor			? "Ec"	:
				  b								? "eC"	:
												  "ec" );
			runningSecs = (uint32_t) UpTicksToSeconds( UpTicks() - gAirTunes->sessionTicks );
			DataBuffer_AppendF( inDB, "    Running Time             = %u seconds (%{dur})\n", runningSecs, runningSecs );
			DataBuffer_AppendF( inDB, "    timing                   = sock %d, local port %d, remote port %d\n", 
				gAirTunes->timingSock, gAirTunes->timingPortLocal, gAirTunes->timingPortRemote );
			src = &gAirTunes->source;
			DataBuffer_AppendF( inDB, "    receiveCount             = %u\n", src->receiveCount );
			DataBuffer_AppendF( inDB, "    rtcpTISend, Resp         = %u, %u, %u\n", src->rtcpTISendCount, src->rtcpTIResponseCount );
			DataBuffer_AppendF( inDB, "    rtcpTIStepCount          = %u\n", src->rtcpTIStepCount );
			DataBuffer_AppendF( inDB, "    rtcpTIClockRTTOutliers   = %u\n", src->rtcpTIClockRTTOutliers );
			DataBuffer_AppendF( inDB, "    rtcpTIClockRTTAvg        = %f\n", src->rtcpTIClockRTTAvg );
			DataBuffer_AppendF( inDB, "    rtcpTIClockRTTMin        = %f\n", src->rtcpTIClockRTTMin );
			DataBuffer_AppendF( inDB, "    rtcpTIClockRTTMax        = %f\n", src->rtcpTIClockRTTMax );
			DataBuffer_AppendF( inDB, "    rtcpTIClockOffsetArray   = [%f, %f, %f]\n", 
				src->rtcpTIClockOffsetArray[ 0 ], src->rtcpTIClockOffsetArray[ 1 ], src->rtcpTIClockOffsetArray[ 2 ] );
			DataBuffer_AppendF( inDB, "    rtcpTIClockOffsetAvg     = %f\n", src->rtcpTIClockOffsetAvg );
			DataBuffer_AppendF( inDB, "    rtcpTIClockOffsetMin     = %f\n", src->rtcpTIClockOffsetMin );
			DataBuffer_AppendF( inDB, "    rtcpTIClockOffsetMax     = %f\n", src->rtcpTIClockOffsetMax );
			DataBuffer_AppendF( inDB, "\n" );
		}
	}
	
	// clockOffsets -- Show clock offset history.
	
	else if( strcmp( inCmd, "clockOffsets" ) == 0 )
	{
	#if( DEBUG )
		DataBuffer_AppendF( inDB, "+-+ AirTunes Clock Offset History (%u total) +-+\n", gAirTunesClockOffsetCount );
		for( i = 0; i < gAirTunesClockOffsetCount; ++i )
		{
			if( i == 0 )
			{
				DataBuffer_AppendF( inDB, "%+f\n", gAirTunesClockOffsetHistory[ i ] );
			}
			else
			{
				DataBuffer_AppendF( inDB, "%+f\t%+f\n", gAirTunesClockOffsetHistory[ i ], 
					gAirTunesClockOffsetHistory[ i ] - gAirTunesClockOffsetHistory[ i - 1 ] );
			}
		}
		DataBuffer_AppendF( inDB, "\n" );
	#endif
	}
	
	// rtt -- Show RTT history.
	
	else if( strcmp( inCmd, "rtt" ) == 0 )
	{
	#if( DEBUG )
		DataBuffer_AppendF( inDB, "+-+ AirTunes Clock RTT History (%u total) +-+\n", gAirTunesClockRTTCount );
		for( i = 0; i < gAirTunesClockRTTCount; ++i )
		{
			if( i == 0 )
			{
				DataBuffer_AppendF( inDB, "%+f\n", gAirTunesClockRTTHistory[ i ] );
			}
			else
			{
				DataBuffer_AppendF( inDB, "%+f\t%+f\n", gAirTunesClockRTTHistory[ i ], 
					gAirTunesClockRTTHistory[ i ] - gAirTunesClockRTTHistory[ i - 1 ] );
			}
		}
		DataBuffer_AppendF( inDB, "\n" );
	#endif
	}
	
	// Help
	
	else if( strcmp( inCmd, "help" ) == 0 )
	{
	#if( DEBUG )
		DataBuffer_AppendF( inDB, "+-+ AirTunes Debug Show Commands +-+\n" );
		DataBuffer_AppendF( inDB, "    globals       -- Show globals.\n" );
		DataBuffer_AppendF( inDB, "    stats         -- Show persistent stats.\n" );
		DataBuffer_AppendF( inDB, "    tweak         -- Show tweakable values.\n" );
		DataBuffer_AppendF( inDB, "    mem           -- Show memory usage.\n" );
		DataBuffer_AppendF( inDB, "    clockOffsets  -- Show clock offset history.\n" );
		DataBuffer_AppendF( inDB, "\n" );
	#endif
	}
	
	// Unknown
	
	else
	{
		dlog( kLogLevelError, "### Unknown command: \"%s\"\n", inCmd );
		err = kUnsupportedErr;
		goto exit;
	}
	goto exitWithOutput;
	
exitWithOutput:
	err = kNoErr;
	
exit:
	return( err );
}

