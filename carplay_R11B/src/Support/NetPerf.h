/*
	File:    	NetPerf.h
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

#ifndef	__NetPerf_h__
#define	__NetPerf_h__

#include "APSCommonServices.h"
#include "CFUtils.h"
#include "HTTPClient.h"
#include "HTTPMessage.h"
#include "ThreadUtils.h"

#include CF_RUNTIME_HEADER
#include LIBDISPATCH_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#pragma mark == Creation ==
#endif

//===========================================================================================================================
/*!	@group		Creation
	@abstract	Creating a netperf object.
*/

typedef struct NetPerfPrivate *		NetPerfRef;

CFTypeID	NetPerfGetTypeID( void );
OSStatus	NetPerfCreate( NetPerfRef *outPerf );

#if 0
#pragma mark -
#pragma mark == Commands ==
#endif

//===========================================================================================================================
/*!	@group		Commands
	@abstract	Commands for controlling the server.
*/

// Start a client session.
// Input:  None.
// Output: None.
#define kNetPerfCommand_StartClientSession			CFSTR( "startClientSession" )

// Stop a client session.
// Input:  None.
// Output: None.
#define kNetPerfCommand_StopClientSession			CFSTR( "stopClientSession" )

// Set up the server for a perf session.
// Input:  Requested session configuration info.
// Output: Details about how the session is actually set up (e.g. port numbers, etc.).
// Notes:  Must be balanced with a tearDownServerSession command.
#define kNetPerfCommand_SetUpServerSession			CFSTR( "setUpServerSession" )

// Tear down the server session.
// Input:  None.
// Output: None.
// Note:   Safe to multiple times or if the server has never been set up (for easier cleanup).
#define kNetPerfCommand_TearDownServerSession		CFSTR( "tearDownServerSession" )

// Starts a session with the server.
// Input:  None.
// Output: None.
// Notes:  Must have been previously set up via kNetPerfCommand_SetUpServerSession.
#define kNetPerfCommand_StartServerSession			CFSTR( "startServerSession" )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NetPerfControl
	@abstract	Controls the server
	@discussion	For details on the format-string variants, see the HeaderDoc for CFPropertyListCreateFormatted.
*/

#define NetPerfControlSync( OBJ, COMMAND, PARAMS, RESPONSE_PTR ) \
	CFObjectControlSync( (OBJ), (OBJ)->queue, _NetPerfControl, 0, (COMMAND), NULL, (PARAMS), (RESPONSE_PTR) )

#define NetPerfControlSyncF( OBJ, FLAGS, COMMAND, ... ) \
	CFObjectControlSyncF( (OBJ), (OBJ)->queue, _NetPerfControl, 0, (COMMAND), NULL, (RESPONSE_PTR), __VA_ARGS__ )

#define NetPerfControlSyncV( OBJ, FLAGS, COMMAND, FORMAT, ARGS ) \
	CFObjectControlSyncV( (OBJ), (OBJ)->queue, _NetPerfControl, 0, (COMMAND), NULL, (RESPONSE_PTR), (FORMAT), (ARGS) )

OSStatus
	_NetPerfControl( 
		CFTypeRef			inObject, 
		CFObjectFlags		inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outResponse );

#if 0
#pragma mark -
#pragma mark == Events ==
#endif

//===========================================================================================================================
/*!	@group		Events
	@abstract	Event notification.
*/

// Test completed (success or failure). Details dictionary contains an error code if it failed.
#define kNetPerfEvent_Completed		1

// Non-fatal error occurred. Details dictionary contains an error code, etc.
#define kNetPerfEvent_Error			2

// Progress update.
#define kNetPerfEvent_Progress		3

// Posted after a client session has been completed stopped. No further events will be posted after thing unless restarted.
// Details: None.
#define kNetPerfEvent_Stopped		4

typedef void ( *NetPerfEventHandlerFunc )( uint32_t inType, CFDictionaryRef inDetails, void *inContext );
void	NetPerfSetEventHandler( NetPerfRef inPerf, NetPerfEventHandlerFunc inHandler, void *inContext, dispatch_queue_t inQueue );

#if 0
#pragma mark -
#pragma mark == Properties ==
#endif

//===========================================================================================================================
/*!	@group		Properties
	@abstract	Properties to get/set.
*/

// Configuration parameters.
#define kNetPerfProperty_Config		CFSTR( "config" ) // [Dictionary]

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NetPerfCopyProperty
	@abstract	Copies a property.
*/

#define NetPerfCopyProperty( OBJ, FLAGS, PROPERTY, QUALIFIER, OUT_ERROR ) \
	CFObjectCopyProperty( (OBJ), (OBJ)->queue, _NetPerfCopyProperty, (FLAGS), (PROPERTY), (QUALIFIER), (OUT_ERROR) )

#define NetPerfGetPropertyBoolean( OBJ, FLAGS, PROPERTY, QUALIFIER, OUT_ERROR ) \
	( ( NetPerfGetPropertyInt64( (OBJ), (FLAGS), (PROPERTY), (QUALIFIER), (OUT_ERROR) ) != 0 ) ? true : false )

#define NetPerfGetPropertyCString( OBJ, FLAGS, PROPERTY, QUALIFIER, BUF, MAX_LEN, OUT_ERROR ) \
	CFObjectGetPropertyCStringSync( (OBJ), (OBJ)->queue, _NetPerfCopyProperty, (FLAGS), (PROPERTY), (QUALIFIER), \
		(BUF), (MAX_LEN), (OUT_ERROR) )

#define NetPerfGetPropertyInt64( OBJ, FLAGS, PROPERTY, QUALIFIER, OUT_ERROR ) \
	CFObjectGetPropertyInt64Sync( (OBJ), (OBJ)->queue, _NetPerfCopyProperty, (FLAGS), (PROPERTY), (QUALIFIER), \
		(OUT_ERROR) )

CFTypeRef
	_NetPerfCopyProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NetPerfSetProperty
	@abstract	Sets a property.
	@discussion	For details on the format-string variants, see the HeaderDoc for CFPropertyListCreateFormatted.
*/

#define NetPerfSetProperty( OBJ, FLAGS, PROPERTY, QUALIFIER, VALUE ) \
	CFObjectSetProperty( (OBJ), (OBJ)->queue, _NetPerfSetProperty, (FLAGS) | kCFObjectFlagAsync, \
		(PROPERTY), (QUALIFIER), (VALUE) )

#define NetPerfSetPropertyF( OBJ, FLAGS, PROPERTY, QUALIFIER, ... ) \
	CFObjectSetPropertyF( (OBJ), (OBJ)->queue, _NetPerfSetProperty, (FLAGS) | kCFObjectFlagAsync, \
		(PROPERTY), (QUALIFIER), __VA_ARGS__ )

#define NetPerfSetPropertyV( OBJ, FLAGS, PROPERTY, QUALIFIER, FORMAT, ARGS ) \
	CFObjectSetPropertyV( (OBJ), (OBJ)->queue, _NetPerfSetProperty, (FLAGS) | kCFObjectFlagAsync, \
		(PROPERTY), (QUALIFIER), (FORMAT), (ARGS) )

OSStatus
	_NetPerfSetProperty( 
		CFTypeRef		inObject, 
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue );

#if 0
#pragma mark -
#pragma mark == Keys ==
#endif

//===========================================================================================================================
/*!	@group		Keys
	@abstract	Keys for the configuration plist.
*/

// Command to control the object.
// [String]
#define kNetPerfKey_ControlCommand				CFSTR( "controlCommand" )

// Parameters for a command.
// [Dictionary]
#define kNetPerfKey_ControlParams				CFSTR( "controlParams" )

// Numeric value describing an error.
// [Number:OSStatus]
#define kNetPerfKey_ErrorCode					CFSTR( "errorCode" )

// Destination server to run a performance test with. Should be a DNS name, IP address, or URL.
// [String]
#define kNetPerfKey_Destination					CFSTR( "destination" )

// TCP or UDP port number.
// [Number:int]
#define kNetPerfKey_Port						CFSTR( "port" )

// Estimate of test progress. 0.0 means it just started. 1.0 means it has completed. 0.5 means halfway done.
// [Number:double;0.0-1.0]
#define kNetPerfKey_Progress					CFSTR( "progress" )

// Quality of service (QoS).
// [Number:int]
#define kNetPerfKey_QoS							CFSTR( "qos" )

// Socket receive buffer size in bytes.
// [Number:int]
#define kNetPerfKey_SocketReceiveBufferSize		CFSTR( "socketReceiveBufferSize" )

// Socket send buffer size in bytes.
// [Number:int]
#define kNetPerfKey_SocketSendBufferSize		CFSTR( "socketSendBufferSize" )

// Summarizes an operation. It may be one or more lines of text.
// For progress updates, this indicates the current operation.
// [String]
#define kNetPerfKey_Summary						CFSTR( "summary" )

#if 0
#pragma mark -
#pragma mark == Packets ==
#endif

//===========================================================================================================================
//	Packets
//===========================================================================================================================

typedef struct
{
	uint32_t		seqNum;			// [ 0] Sequence number of packet.
	uint32_t		flags;			// [ 4] Flags.
	uint64_t		originateTime;	// [ 8] NTP timestamp when client sent request.
	uint64_t		receiveTime;	// [16] NTP timestamp when server received request. 0 for client.
	uint64_t		transmitTime;	// [24] NTP timestamp when client sent request.
	uint32_t		lost;			// [32] Number of lost packets so far.
	uint32_t		outOfOrder;		// [36] Number of out of order packets so far.
	uint32_t		dup;			// [40] Number of duplicate packets so far.
	uint32_t		pad;			// [44]
									// [48] Total
}	NetPerfUDPHeader;

check_compile_time( offsetof( NetPerfUDPHeader, seqNum )		==  0 );
check_compile_time( offsetof( NetPerfUDPHeader, flags )			==  4 );
check_compile_time( offsetof( NetPerfUDPHeader, originateTime )	==  8 );
check_compile_time( offsetof( NetPerfUDPHeader, receiveTime )	== 16 );
check_compile_time( offsetof( NetPerfUDPHeader, transmitTime )	== 24 );
check_compile_time( sizeof(   NetPerfUDPHeader )				== 48 );

#if 0
#pragma mark -
#pragma mark == Internals ==
#endif

//===========================================================================================================================
//	Internals
//===========================================================================================================================

typedef enum
{
	kNetPerfClientState_Stopped			= 0, 
	kNetPerfClientState_Starting		= 1, 
	kNetPerfClientState_SettingUpServer	= 2, 
	kNetPerfClientState_StartingServer	= 3, 
	kNetPerfClientState_Running			= 4, 
	kNetPerfClientState_TearDownServer	= 4
	
}	NetPerfClientState;

struct NetPerfPrivate
{
	CFRuntimeBase				base;					// CF type info. Must be first.
	dispatch_queue_t			queue;					// Queue to serialize internal operations.
	NetPerfEventHandlerFunc		eventHandler;			// Function to call to handle events.
	void *						eventContext;			// Context to pass to event handler function.
	dispatch_queue_t			eventQueue;				// Queue to call event handler on.
	CFDictionaryRef				config;					// Configuration parameters.
	
	HTTPClientRef				httpClient;				// Client for talking to the server.
	HTTPMessageRef				httpMessage;			// For sending control messages.
	CFDictionaryRef				clientResponse;			// Response plist from the server.
	NetPerfClientState			clientState;			// State of the client session.
	SocketRef					clientDataCmdSock;		// Loopback socket for signaling the data thread.
	SocketRef					clientDataSock;			// Socket for send data.
	pthread_t					clientSendThread;		// Thread for sending data.
	pthread_t *					clientSendThreadPtr;	// Ptr to send thread (for NULL testing).
	pthread_t					clientReceiveThread;	// Thread for receiving data.
	pthread_t *					clientReceiveThreadPtr;	// Ptr to receive thread (for NULL testing).
	
	Boolean						serverSessionSetUp;		// True if the server has been set up.
	Boolean						serverSessionStarted;	// True if the server has been started.
	pthread_t					serverDataThread;		// Thread for processing data.
	pthread_t *					serverDataThreadPtr;	// Ptr to data thread (for NULL testing).
	SocketRef					serverDataCmdSock;		// Loopback socket for signaling the data thread.
	SocketRef					serverDataSockV4;		// IPv4 socket for receiving data.
	SocketRef					serverDataSockV6;		// IPv6 socket for receiving data.
	
	uint8_t *					packetBuffer;			// Buffer to receive packets.
	size_t						packetMaxLen;			// Max bytes in a packet.
	uint32_t					lastSeqNum;				// Last in-order sequence number we received.
	uint64_t					statCount;				// Total number of packets received.
	uint32_t					statOutOfOrder;			// Number of out-of-order packets.
	uint32_t					statLost;				// Number of lost packets.
	uint32_t					statDup;				// Number of duplicate packets.
	uint64_t					nextLogTicks;
	uint64_t					logIntervalTicks;
	double						minRTT, maxRTT, avgRTT;
};

#ifdef __cplusplus
}
#endif

#endif	// __NetPerf_h__
