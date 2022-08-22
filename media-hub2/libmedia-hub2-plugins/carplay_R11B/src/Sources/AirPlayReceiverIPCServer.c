/*
	File:    	AirPlayReceiverIPCServer.c
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
	
	Copyright (C) 2013-2014 Apple Inc. All Rights Reserved.
*/

#include "AirPlayCommon.h"
#include "AirPlayReceiverIPCCommon.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayReceiverServerPriv.h"
#include "AirPlayReceiverSession.h"
#include "AirPlayReceiverSessionPriv.h"
#include "APSCommonServices.h"
#include "HIDUtils.h"
#include "IPCUtils.h"
#include "ScreenUtils.h"

//===========================================================================================================================
//	Internals
//===========================================================================================================================

typedef struct
{
	IPCServerConnectionRef		cnx;
	CFMutableDictionaryRef		objectMap;	// Key'd by objectID, value is the object.
	
}	AirPlayReceiverIPCServerContext;

static OSStatus	_Server_InitConnection( IPCServerConnectionRef inCnx );
static void		_Server_FreeConnection( IPCServerConnectionRef inCnx );
static void		_Server_FreeObjectApplier( const void *inKey, const void *inValue, void *inContext );
static OSStatus	_Server_HandleMessage( IPCServerConnectionRef inCnx, IPCMessageRef inMsg );

// AirPlayReceiverServer

static OSStatus	_Server_AirPlayReceiverServerCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static void		_Server_AirPlayReceiverServerInvalidate( void *inContext );
static OSStatus	_Server_AirPlayReceiverServerSetDelegate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_AirPlayReceiverServerControl( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );

static OSStatus
	_Server_AirPlayReceiverServerDelegate_Control( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		CFDictionaryRef *			outParams, 
		void *						inContext );
static CFTypeRef
	_Server_AirPlayReceiverServerDelegate_CopyProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext );
static OSStatus
	_Server_AirPlayReceiverServerDelegate_SetProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		CFTypeRef					inValue, 
		void *						inContext );
static void
	_Server_AirPlayReceiverServerDelegate_SessionCreated( 
		AirPlayReceiverServerRef	inServer, 
		AirPlayReceiverSessionRef	inSession, 
		void *						inContext );
static void
	_Server_AirPlayReceiverServerDelegate_SessionFailed( 
		AirPlayReceiverServerRef	inServer, 
		OSStatus					inReason, 
		void *						inContext );

// AirPlayReceiverSession

static void		_Server_AirPlayReceiverSessionInvalidate( void *inContext );
static OSStatus	_Server_AirPlayReceiverSessionSetDelegate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_AirPlayReceiverSessionControl( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_AirPlayReceiverSessionChangeModes( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_AirPlayReceiverSessionForceKeyFrame( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus _Server_AirPlayReceiverSessionRequestSiriAction( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_AirPlayReceiverSessionRequestUI( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
	static OSStatus _Server_AirPlayReceiverSessionSetLimitedUI( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
	static OSStatus _Server_AirPlayReceiverSessionSetNightMode( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );

static void		_Server_AirPlayReceiverSessionDelegate_Finalize( AirPlayReceiverSessionRef inSession, void *inContext );
static OSStatus
	_Server_AirPlayReceiverSessionDelegate_Control( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		CFDictionaryRef *			outParams, 
		void *						inContext );
static CFTypeRef
	_Server_AirPlayReceiverSessionDelegate_CopyProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext );
static OSStatus
	_Server_AirPlayReceiverSessionDelegate_SetProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		CFTypeRef					inValue, 
		void *						inContext );
static void
	_Server_AirPlayReceiverSessionDelegate_ModesChanged( 
		AirPlayReceiverSessionRef 	inSession, 
		const AirPlayModeState *	inState, 
		void *						inContext );
static void
	_Server_AirPlayReceiverSessionDelegate_RequestUI( 
		AirPlayReceiverSessionRef 	inSession, 
		void *						inContext );
static void
	_Server_AirPlayReceiverSessionDelegate_DuckAudio( 
		AirPlayReceiverSessionRef 	inSession, 
		double						inDurationSecs,
		double						inVolume,
		void *						inContext );
static void
	_Server_AirPlayReceiverSessionDelegate_UnduckAudio( 
		AirPlayReceiverSessionRef 	inSession, 
		double						inDurationSecs,
		void *						inContext );

// HIDDevice

static OSStatus	_Server_HIDDeviceCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_HIDDeviceCopyProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_HIDDeviceSetProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_HIDDevicePostReport( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_HIDPostReport( IPCMessageRef inMsg );
static OSStatus	_Server_HIDRegisterDevice( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus _Server_HIDDeviceStop( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );

// Screen

static OSStatus	_Server_ScreenCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_ScreenCopyProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_ScreenSetProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );
static OSStatus	_Server_ScreenRegister( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg );

// Utils

static OSStatus		_AddObject( AirPlayReceiverIPCServerContext *ctx, CFTypeRef inObject, uint32_t *outID );
static void			_RemoveObjectByID( AirPlayReceiverIPCServerContext *ctx, uint32_t inObjectID, Boolean release );
static OSStatus		_RemoveObjectByMsg( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg, CFTypeID inTypeID, Boolean release );
static CFTypeRef	_FindObject( AirPlayReceiverIPCServerContext *ctx, uint32_t inObjectID, CFTypeID inTypeID, OSStatus *outErr );
static OSStatus		_SendPlistRequest( IPCServerConnectionRef inCnx, uint32_t inOpcode, CFPropertyListRef inPlist );
static OSStatus		_SendSimpleRequest( IPCServerConnectionRef inCnx, uint32_t inOpcode, const void *inData, size_t inLen );
static OSStatus		_SendPlistReply( IPCServerConnectionRef	inCnx, IPCMessageRef inMsg, CFPropertyListRef inPlist );
static OSStatus
	_SendSimpleReply( 
		IPCServerConnectionRef	inCnx, 
		IPCMessageRef			inMsg, 
		OSStatus				inStatus, 
		const void *			inData, 
		size_t					inLen );
static OSStatus
	_SendPlistWithReplyPlistOneShot( 
		IPCServerConnectionRef	inCnx, 
		uint32_t				inOpcode, 
		uint32_t				inFlags, 
		OSStatus				inStatus, 
		CFDictionaryRef			inPlist, 
		CFDictionaryRef *		outPlist );

static IPCServerRef		gIPCServer		= NULL;
static uint32_t			gLastObjectID	= 0;

ulog_define( AirPlayReceiverIPCServer, kLogLevelNotice, kLogFlags_Default, "AirPlayReceiverIPCServer", NULL );
#define aprs_ipc_dlog( LEVEL, ... )		dlogc( &log_category_from_name( AirPlayReceiverIPCServer ), (LEVEL), __VA_ARGS__ )
#define aprs_ipc_ulog( LEVEL, ... )		ulog( &log_category_from_name( AirPlayReceiverIPCServer ), (LEVEL), __VA_ARGS__ )

//===========================================================================================================================
//	AirPlayReceiverIPCServerInitialize
//===========================================================================================================================

OSStatus	AirPlayReceiverIPCServerInitialize( void )
{
	OSStatus				err;
	IPCServerDelegate		delegate;
	
	IPCServerDelegateInit( &delegate );
	delegate.initConnection_f	= _Server_InitConnection;
	delegate.freeConnection_f	= _Server_FreeConnection;
	delegate.handleMessage_f	= _Server_HandleMessage;
	
	err = IPCServerCreate( &gIPCServer, &delegate );
	require_noerr( err, exit );
	
	err = IPCServerStart( gIPCServer );
	require_noerr( err, exit );
	
exit:
	if( err ) IPCServerForget( &gIPCServer );
	return( err );
}

//===========================================================================================================================
//	_Server_InitConnection
//===========================================================================================================================

static OSStatus	_Server_InitConnection( IPCServerConnectionRef inCnx )
{
	OSStatus								err;
	AirPlayReceiverIPCServerContext *		ctx;
	
	ctx = (AirPlayReceiverIPCServerContext *) calloc( 1, sizeof( *ctx ) );
	require_action( ctx, exit, err = kNoMemoryErr );
	
	ctx->cnx = inCnx;
	inCnx->delegateCtx = ctx;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_FreeConnection
//===========================================================================================================================

static void	_Server_FreeConnection( IPCServerConnectionRef inCnx )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inCnx->delegateCtx;
	
	if( !ctx ) return;
	
	if( ctx->objectMap )
	{
		CFDictionaryApplyFunction( ctx->objectMap, _Server_FreeObjectApplier, ctx );
		CFRelease( ctx->objectMap );
		ctx->objectMap = NULL;
	}
	inCnx->delegateCtx = NULL;
	free( ctx );
}

//===========================================================================================================================
//	_Server_FreeObjectApplier
//===========================================================================================================================

static void	_Server_FreeObjectApplier( const void *inKey, const void *inValue, void *inContext )
{
	(void) inKey;
	(void) inContext;
	
	if( CFGetTypeID( inValue ) == AirPlayReceiverServerGetTypeID() )
	{
		AirPlayReceiverServerRef const		server = (AirPlayReceiverServerRef) inValue;
		
		dispatch_sync_f( AirPlayReceiverServerGetDispatchQueue( server ), server, _Server_AirPlayReceiverServerInvalidate );
	}
	else if( CFGetTypeID( inValue ) == AirPlayReceiverSessionGetTypeID() )
	{
		AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inValue;
		
		dispatch_sync_f( AirPlayReceiverSessionGetDispatchQueue( session ), session, _Server_AirPlayReceiverSessionInvalidate );
	}
	
	CFRelease( inValue );
}

//===========================================================================================================================
//	_Server_HandleMessage
//===========================================================================================================================

static OSStatus	_Server_HandleMessage( IPCServerConnectionRef inCnx, IPCMessageRef inMsg )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inCnx->delegateCtx;
	OSStatus									err;
	
	aprs_ipc_ulog( kLogLevelVerbose, "Message opcode %u, xid %u, length %u bytes\n", 
		inMsg->header.opcode, inMsg->header.xid, inMsg->header.length );
	
	switch( inMsg->header.opcode )
	{
		// AirPlayReceiverServer
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerCreate:
			err = _Server_AirPlayReceiverServerCreate( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerRelease:
			err = _RemoveObjectByMsg( ctx, inMsg, AirPlayReceiverServerGetTypeID(), true );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerSetDelegate:
			err = _Server_AirPlayReceiverServerSetDelegate( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerControl:
			err = _Server_AirPlayReceiverServerControl( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		// AirPlayReceiverSession
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetDelegate:
			err = _Server_AirPlayReceiverSessionSetDelegate( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionControl:
			err = _Server_AirPlayReceiverSessionControl( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionChangeModes:
			err = _Server_AirPlayReceiverSessionChangeModes( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionForceKeyFrame:
			err = _Server_AirPlayReceiverSessionForceKeyFrame( ctx, inMsg );
			require_noerr( err, exit );
			break;

		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestSiriAction:
			err = _Server_AirPlayReceiverSessionRequestSiriAction( ctx, inMsg );
			require_noerr( err, exit );
			break;
			
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestUI:
			err = _Server_AirPlayReceiverSessionRequestUI( ctx, inMsg );
			require_noerr( err, exit );
			break;
			
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetLimitedUI:
			err = _Server_AirPlayReceiverSessionSetLimitedUI( ctx, inMsg );
			require_noerr( err, exit );
			break;
			
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetNightMode:
			err = _Server_AirPlayReceiverSessionSetNightMode( ctx, inMsg );
			require_noerr( err, exit );
			break;
			
		// HID
		
		case kAirPlayReceiverIPCOpcode_HIDDeviceCreateVirtual:
			err = _Server_HIDDeviceCreate( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDDeviceRelease:
			err = _RemoveObjectByMsg( ctx, inMsg, HIDDeviceGetTypeID(), true );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDDeviceCopyProperty:
			err = _Server_HIDDeviceCopyProperty( ctx, inMsg );
			require_noerr_quiet( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDDeviceSetProperty:
			err = _Server_HIDDeviceSetProperty( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDRegisterDevice:
			err = _Server_HIDRegisterDevice( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDDevicePostReport:
			err = _Server_HIDDevicePostReport( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDPostReport:
			err = _Server_HIDPostReport( inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_HIDDeviceStop:
			err = _Server_HIDDeviceStop( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		// Screen
		
		case kAirPlayReceiverIPCOpcode_ScreenCreate:
			err = _Server_ScreenCreate( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_ScreenRelease:
			err = _RemoveObjectByMsg( ctx, inMsg, HIDDeviceGetTypeID(), true );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_ScreenCopyProperty:
			err = _Server_ScreenCopyProperty( ctx, inMsg );
			require_noerr_quiet( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_ScreenSetProperty:
			err = _Server_ScreenSetProperty( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		case kAirPlayReceiverIPCOpcode_ScreenRegister:
			err = _Server_ScreenRegister( ctx, inMsg );
			require_noerr( err, exit );
			break;
		
		default:
			aprs_ipc_ulog( kLogLevelNotice, "### Unhandled message: %u\n", inMsg->header.opcode );
			err = kNotHandledErr;
			goto exit;
	}
	err = kNoErr;
	
exit:
	if( err ) _SendSimpleReply( inCnx, inMsg, err, NULL, 0 );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == AirPlayReceiverServer ==
#endif

//===========================================================================================================================
//	_Server_AirPlayReceiverServerCreate
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverServerCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	AirPlayReceiverServerRef		server = NULL;
	uint32_t						objectID;
	
	err = AirPlayReceiverServerCreate( &server );
	require_noerr( err, exit );
	
	err = _AddObject( ctx, server, &objectID );
	require_noerr( err, exit );
	
	err = _SendSimpleReply( ctx->cnx, inMsg, kNoErr, &objectID, sizeof( objectID ) );
	if( err ) _RemoveObjectByID( ctx, objectID, false );
	require_noerr( err, exit );
	
exit:
	if( err ) CFReleaseNullSafe( server );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerInvalidate
//===========================================================================================================================

static void	_Server_AirPlayReceiverServerInvalidate( void *inContext )
{
	AirPlayReceiverServerRef const		server = (AirPlayReceiverServerRef) inContext;
	AirPlayReceiverServerDelegate		delegate;
		
	AirPlayReceiverServerDelegateInit( &delegate );
	AirPlayReceiverServerSetDelegate( server, &delegate );
	AirPlayReceiverServerStop( server );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerSetDelegate
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverServerSetDelegate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus									err;
	AirPlayReceiverServerSetDelegateIPCData		ipcData;
	AirPlayReceiverServerRef					server;
	AirPlayReceiverServerDelegate				delegate;
	
	require_action( inMsg->bodyLen == sizeof( ipcData ), exit, err = kSizeErr );
	memcpy( &ipcData, inMsg->bodyPtr, sizeof( ipcData ) );
	
	server = (AirPlayReceiverServerRef) _FindObject( ctx, ipcData.objectID, AirPlayReceiverServerGetTypeID(), &err );
	require_noerr( err, exit );
	
	AirPlayReceiverServerDelegateInit( &delegate );
	delegate.context = ctx;
	delegate.context2 = (void *)(uintptr_t) ipcData.objectID;
	if( ipcData.control_f )			delegate.control_f			= _Server_AirPlayReceiverServerDelegate_Control;
	if( ipcData.copyProperty_f )	delegate.copyProperty_f		= _Server_AirPlayReceiverServerDelegate_CopyProperty;
	if( ipcData.setProperty_f )		delegate.setProperty_f		= _Server_AirPlayReceiverServerDelegate_SetProperty;
	if( ipcData.sessionCreated_f )	delegate.sessionCreated_f	= _Server_AirPlayReceiverServerDelegate_SessionCreated;
	if( ipcData.sessionFailed_f )	delegate.sessionFailed_f	= _Server_AirPlayReceiverServerDelegate_SessionFailed;
	AirPlayReceiverServerSetDelegate( server, &delegate );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerControl
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverServerControl( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverServerRef		server;
	CFStringRef						command;
	CFTypeRef						qualifier;
	CFDictionaryRef					request;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	server = (AirPlayReceiverServerRef) _FindObject( ctx, objectID, AirPlayReceiverServerGetTypeID(), &err );
	require_noerr( err, exit );
	
	command = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	request = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	// Note: This is async with no response since we don't currently use responses and synchronously waiting would deadlock.
	// This should be revisited to figure out a better solution if responses end up being needed in the future.
	
	err = CFObjectControlAsync( server, AirPlayReceiverServerGetDispatchQueue( server ), AirPlayReceiverServerControl, 0, 
		command, qualifier, request, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	err = _SendSimpleReply( ctx->cnx, inMsg, kNoErr, NULL, 0 );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerDelegate_Control
//===========================================================================================================================

static OSStatus
	_Server_AirPlayReceiverServerDelegate_Control( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		CFDictionaryRef *			outParams, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inServer->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Command ), inCommand );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inParams )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inParams );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_Control, 
		0, kNoErr, request, outParams );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerDelegate_CopyProperty
//===========================================================================================================================

static CFTypeRef
	_Server_AirPlayReceiverServerDelegate_CopyProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	CFDictionaryRef								reply = NULL;
	CFTypeRef									value = NULL;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inServer->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_CopyProperty, 
		0, kNoErr, request, &reply );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
	value = CFDictionaryGetValue( reply, CFSTR( kAirPlayKey_Value ) );
	if( value ) CFRetain( value );
	
exit:
	CFReleaseNullSafe( reply );
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerDelegate_SetProperty
//===========================================================================================================================

static OSStatus
	_Server_AirPlayReceiverServerDelegate_SetProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		CFTypeRef					inValue, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inServer->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inValue )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Value ), inValue );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SetProperty, 
		0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerDelegate_SessionCreated
//===========================================================================================================================

static void
	_Server_AirPlayReceiverServerDelegate_SessionCreated( 
		AirPlayReceiverServerRef	inServer, 
		AirPlayReceiverSessionRef	inSession, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	uint32_t									sessionObjectID;
	AirPlayReceiverSessionDelegate				delegate;
	CFMutableDictionaryRef						request;
	
	err = _AddObject( ctx, inSession, &sessionObjectID );
	require_noerr( err, exit );
	
	AirPlayReceiverSessionDelegateInit( &delegate );
	delegate.context	= ctx;
	delegate.context2	= (void *)(uintptr_t) sessionObjectID;
	delegate.finalize_f = _Server_AirPlayReceiverSessionDelegate_Finalize;
	AirPlayReceiverSessionSetDelegate( inSession, &delegate );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inServer->delegate.context2 );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID2, sessionObjectID );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionCreated, 
		0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return;
}

//===========================================================================================================================
//	_Server_AirPlayReceiverServerDelegate_SessionFailed
//===========================================================================================================================

static void
	_Server_AirPlayReceiverServerDelegate_SessionFailed( 
		AirPlayReceiverServerRef	inServer, 
		OSStatus					inReason, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
		
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inServer->delegate.context2 );
	CFDictionarySetInt64( request, CFSTR( kAirPlayKey_ReasonCode ), inReason );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionFailed, 
		0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return;
}

#if 0
#pragma mark -
#pragma mark == AirPlayReceiverSession ==
#endif

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionInvalidate
//===========================================================================================================================

static void	_Server_AirPlayReceiverSessionInvalidate( void *inContext )
{
	AirPlayReceiverSessionRef const		session = (AirPlayReceiverSessionRef) inContext;
	AirPlayReceiverSessionDelegate		delegate;
		
	AirPlayReceiverSessionDelegateInit( &delegate );
	AirPlayReceiverSessionSetDelegate( session, &delegate );
	AirPlayReceiverSessionTearDown( session, NULL, kConnectionErr, NULL );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionSetDelegate
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionSetDelegate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus										err;
	AirPlayReceiverSessionSetDelegateIPCData		ipcData;
	AirPlayReceiverSessionRef						session;
	AirPlayReceiverSessionDelegate					delegate;
	
	require_action( inMsg->bodyLen == sizeof( ipcData ), exit, err = kSizeErr );
	memcpy( &ipcData, inMsg->bodyPtr, sizeof( ipcData ) );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, ipcData.objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	AirPlayReceiverSessionDelegateInit( &delegate );
	delegate.context = ctx;
	delegate.context2 = (void *)(uintptr_t) ipcData.objectID;
	delegate.finalize_f											= _Server_AirPlayReceiverSessionDelegate_Finalize;
	if( ipcData.control_f )			delegate.control_f			= _Server_AirPlayReceiverSessionDelegate_Control;
	if( ipcData.copyProperty_f )	delegate.copyProperty_f		= _Server_AirPlayReceiverSessionDelegate_CopyProperty;
	if( ipcData.setProperty_f )		delegate.setProperty_f		= _Server_AirPlayReceiverSessionDelegate_SetProperty;
	if( ipcData.modesChanged_f )	delegate.modesChanged_f		= _Server_AirPlayReceiverSessionDelegate_ModesChanged;
	if( ipcData.requestUI_f )		delegate.requestUI_f		= _Server_AirPlayReceiverSessionDelegate_RequestUI;
	if( ipcData.duckAudio_f )		delegate.duckAudio_f		= _Server_AirPlayReceiverSessionDelegate_DuckAudio;
	if( ipcData.unduckAudio_f )		delegate.unduckAudio_f		= _Server_AirPlayReceiverSessionDelegate_UnduckAudio;
	AirPlayReceiverSessionSetDelegate( session, &delegate );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionControl
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionControl( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	CFStringRef						command;
	CFTypeRef						qualifier;
	CFDictionaryRef					request;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	command = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	request = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	// Note: This is async with no response since we don't currently use responses and synchronously waiting would deadlock.
	// This should be revisited to figure out a better solution if responses end up being needed in the future.
	
	err = CFObjectControlAsync( session, AirPlayReceiverSessionGetDispatchQueue( session ), AirPlayReceiverSessionControl, 
		0, command, qualifier, request, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	err = _SendSimpleReply( ctx->cnx, inMsg, kNoErr, NULL, 0 );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionChangeModes
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionChangeModes( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	AirPlayModeChanges				changes;
	size_t							len;
	CFStringRef						reasonStr;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	CFDictionaryGetData( params, kAirPlayReceiverIPCKey_ModeChanges, &changes, sizeof( changes ), &len, &err );
	require_noerr( err, exit );
	require_action( len == sizeof( changes ), exit, err = kSizeErr );
	
	reasonStr = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_ReasonStr ), NULL );
	
	err = AirPlayReceiverSessionChangeModes( session, &changes, reasonStr, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionForceKeyFrame
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionForceKeyFrame( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
		
	err = AirPlayReceiverSessionForceKeyFrame( session, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionRequestSiriAction
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionRequestSiriAction( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	int64_t							siriAction;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	siriAction = CFDictionaryGetInt64( params, CFSTR( kAirPlayKey_SiriAction ), NULL );
	
	err = AirPlayReceiverSessionRequestSiriAction( session, (AirPlaySiriAction) siriAction, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionRequestUI
//===========================================================================================================================

static OSStatus	_Server_AirPlayReceiverSessionRequestUI( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	CFStringRef						url;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	url = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_URL ), NULL );
	
	err = AirPlayReceiverSessionRequestUI( session, url, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionSetLimitedUI
//===========================================================================================================================

static OSStatus _Server_AirPlayReceiverSessionSetLimitedUI( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	Boolean							limitedUI;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	limitedUI = CFDictionaryGetBoolean( params, CFSTR( kAirPlayKey_LimitedUI ), NULL );
	
	err = AirPlayReceiverSessionSetLimitedUI( session, limitedUI, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionSetNightMode
//===========================================================================================================================

static OSStatus _Server_AirPlayReceiverSessionSetNightMode( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus						err;
	CFDictionaryRef					params;
	uint32_t						objectID;
	AirPlayReceiverSessionRef		session;
	Boolean							nightMode;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	session = (AirPlayReceiverSessionRef) _FindObject( ctx, objectID, AirPlayReceiverSessionGetTypeID(), &err );
	require_noerr( err, exit );
	
	nightMode = CFDictionaryGetBoolean( params, CFSTR( kAirPlayKey_NightMode ), NULL );
	
	err = AirPlayReceiverSessionSetNightMode( session, nightMode, NULL, NULL );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_Finalize
//===========================================================================================================================

static void	_Server_AirPlayReceiverSessionDelegate_Finalize( AirPlayReceiverSessionRef inSession, void *inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	uint32_t									objectID;
	
	objectID = (uint32_t)(uintptr_t) inSession->delegate.context2;
	_RemoveObjectByID( ctx, objectID, true );
	
	_SendSimpleRequest( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Finalize, 
		&objectID, sizeof( objectID ) );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_Control
//===========================================================================================================================

static OSStatus
	_Server_AirPlayReceiverSessionDelegate_Control( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		CFDictionaryRef *			outParams, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uint32_t)(uintptr_t) inSession->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Command ), inCommand );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inParams )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inParams );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Control, 
		0, kNoErr, request, outParams );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_CopyProperty
//===========================================================================================================================

static CFTypeRef
	_Server_AirPlayReceiverSessionDelegate_CopyProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	CFDictionaryRef								reply = NULL;
	CFTypeRef									value = NULL;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inSession->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_CopyProperty, 
		0, kNoErr, request, &reply );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
	value = CFDictionaryGetValue( reply, CFSTR( kAirPlayKey_Value ) );
	if( value ) CFRetain( value );
	err = (OSStatus) CFDictionaryGetInt64( reply, CFSTR( kAirPlayKey_Status ), NULL );
	
exit:
	CFReleaseNullSafe( reply );
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_SetProperty
//===========================================================================================================================

static OSStatus
	_Server_AirPlayReceiverSessionDelegate_SetProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		CFTypeRef					inValue, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inSession->delegate.context2 );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inValue )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Value ), inValue );
	
	err = _SendPlistWithReplyPlistOneShot( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_SetProperty, 
		0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_ModesChanged
//===========================================================================================================================

static void
	_Server_AirPlayReceiverSessionDelegate_ModesChanged( 
		AirPlayReceiverSessionRef 	inSession, 
		const AirPlayModeState *	inState, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const			ctx = (AirPlayReceiverIPCServerContext *) inContext;
	AirPlayReceiverSessionModesChangedIPCData		ipcData;
	
	ipcData.objectID = (uint32_t)(uintptr_t) inSession->delegate.context2;
	ipcData.state = *inState;
	_SendSimpleRequest( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_ModesChanged, 
		&ipcData, sizeof( ipcData ) );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_RequestUI
//===========================================================================================================================

static void
	_Server_AirPlayReceiverSessionDelegate_RequestUI( 
		AirPlayReceiverSessionRef 	inSession, 
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	uint32_t									objectID;
	
	objectID = (uint32_t)(uintptr_t) inSession->delegate.context2;
	_SendSimpleRequest( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_RequestUI, 
		&objectID, sizeof( objectID ) );
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_DuckAudio
//===========================================================================================================================

static void
	_Server_AirPlayReceiverSessionDelegate_DuckAudio( 
		AirPlayReceiverSessionRef 	inSession, 
		double						inDurationSecs,
		double						inVolume,
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inSession->delegate.context2 );
	CFDictionarySetDouble( request, CFSTR( kAirPlayKey_DurationMs ), inDurationSecs );
	CFDictionarySetDouble( request, CFSTR( kAirPlayKey_Decibels ), inVolume );
	
	err = _SendPlistRequest( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_DuckAudio, request );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return;
}

//===========================================================================================================================
//	_Server_AirPlayReceiverSessionDelegate_UnduckAudio
//===========================================================================================================================

static void
	_Server_AirPlayReceiverSessionDelegate_UnduckAudio( 
		AirPlayReceiverSessionRef 	inSession, 
		double						inDurationSecs,
		void *						inContext )
{
	AirPlayReceiverIPCServerContext * const		ctx = (AirPlayReceiverIPCServerContext *) inContext;
	OSStatus									err;
	CFMutableDictionaryRef						request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, (uintptr_t) inSession->delegate.context2 );
	CFDictionarySetDouble( request, CFSTR( kAirPlayKey_DurationMs ), inDurationSecs );
	
	err = _SendPlistRequest( ctx->cnx, kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_UnduckAudio, request );
	CFRelease( request );
	require_noerr_quiet( err, exit );
	
exit:
	return;
}

#if 0
#pragma mark -
#pragma mark == HID Device ==
#endif

//===========================================================================================================================
//	_Server_HIDDeviceCreate
//===========================================================================================================================

static OSStatus	_Server_HIDDeviceCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params = NULL;
	HIDDeviceRef		hid = NULL;
	uint32_t			objectID;
	
	if( inMsg->bodyLen > 0 )
	{
		params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
		require_noerr( err, exit );
	}
	
	err = HIDDeviceCreateVirtual( &hid, params );
	CFReleaseNullSafe( params );
	require_noerr( err, exit );
	
	err = _AddObject( ctx, hid, &objectID );
	require_noerr( err, exit );
	
	err = _SendSimpleReply( ctx->cnx, inMsg, kNoErr, &objectID, sizeof( objectID ) );
	if( err ) _RemoveObjectByID( ctx, objectID, false );
	require_noerr( err, exit );
	
exit:
	if( err ) CFReleaseNullSafe( hid );
	return( err );
}

//===========================================================================================================================
//	_Server_HIDDeviceCopyProperty
//===========================================================================================================================

static OSStatus	_Server_HIDDeviceCopyProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus					err;
	CFDictionaryRef				params;
	uint32_t					objectID;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFTypeRef					value = NULL;
	CFMutableDictionaryRef		reply;
	HIDDeviceRef				hid;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	hid = (HIDDeviceRef) _FindObject( ctx, objectID, HIDDeviceGetTypeID(), &err );
	require_noerr( err, exit );
	
	value = HIDDeviceCopyProperty( hid, property, qualifier, &err );
	require_noerr_quiet( err, exit );
	
	reply = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( reply, exit, err = kNoMemoryErr );
	if( value ) CFDictionarySetValue( reply, CFSTR( kAirPlayKey_Value ), value );
	
	err = _SendPlistReply( ctx->cnx, inMsg, reply );
	CFRelease( reply );
	require_noerr( err, exit );

exit:
	CFReleaseNullSafe( params );
	CFReleaseNullSafe( value );
	return( err );
}

//===========================================================================================================================
//	_Server_HIDDeviceSetProperty
//===========================================================================================================================

static OSStatus	_Server_HIDDeviceSetProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	CFStringRef			property;
	CFTypeRef			qualifier;
	CFTypeRef			value;
	HIDDeviceRef		hid;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	value = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Value ) );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	hid = (HIDDeviceRef) _FindObject( ctx, objectID, HIDDeviceGetTypeID(), &err );
	require_noerr( err, exit );
	
	err = HIDDeviceSetProperty( hid, property, qualifier, value );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_HIDDevicePostReport
//===========================================================================================================================

static OSStatus	_Server_HIDDevicePostReport( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	CFDataRef			report;
	HIDDeviceRef		hid;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	report = CFDictionaryGetCFData( params, CFSTR( kAirPlayKey_HIDReport ), &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	hid = (HIDDeviceRef) _FindObject( ctx, objectID, HIDDeviceGetTypeID(), &err );
	require_noerr( err, exit );
	
	err = HIDDevicePostReport( hid, CFDataGetBytePtr( report ), (size_t) CFDataGetLength( report ) );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_HIDPostReport
//===========================================================================================================================

static OSStatus	_Server_HIDPostReport( IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	CFStringRef			uuid;
	CFDataRef			report;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	uuid = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_UUID ), &err );
	require_noerr( err, exit );
	
	report = CFDictionaryGetCFData( params, CFSTR( kAirPlayKey_HIDReport ), &err );
	require_noerr( err, exit );
		
	err = HIDPostReport( uuid, CFDataGetBytePtr( report ), (size_t) CFDataGetLength( report ) );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_HIDRegisterDevice
//===========================================================================================================================

static OSStatus	_Server_HIDRegisterDevice( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	uint32_t			objectID;
	HIDDeviceRef		hid;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	hid = (HIDDeviceRef) _FindObject( ctx, objectID, HIDDeviceGetTypeID(), &err );
	require_noerr( err, exit );
	
	err = HIDRegisterDevice( hid );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Server_HIDDeviceStop
//===========================================================================================================================

static OSStatus	_Server_HIDDeviceStop( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	uint32_t			objectID;
	HIDDeviceRef		hid;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	hid = (HIDDeviceRef) _FindObject( ctx, objectID, HIDDeviceGetTypeID(), &err );
	require_noerr( err, exit );
	
	HIDDeviceStop( hid );
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Screen ==
#endif

//===========================================================================================================================
//	_Server_ScreenCreate
//===========================================================================================================================

static OSStatus	_Server_ScreenCreate( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params = NULL;
	ScreenRef			screen = NULL;
	uint32_t			objectID;
	
	if( inMsg->bodyLen > 0 )
	{
		params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
		require_noerr( err, exit );
	}
	
	err = ScreenCreate( &screen, params );
	CFReleaseNullSafe( params );
	require_noerr( err, exit );
	
	err = _AddObject( ctx, screen, &objectID );
	require_noerr( err, exit );
	
	err = _SendSimpleReply( ctx->cnx, inMsg, kNoErr, &objectID, sizeof( objectID ) );
	if( err ) _RemoveObjectByID( ctx, objectID, false );
	require_noerr( err, exit );
	
exit:
	if( err ) CFReleaseNullSafe( screen );
	return( err );
}

//===========================================================================================================================
//	_Server_ScreenCopyProperty
//===========================================================================================================================

static OSStatus	_Server_ScreenCopyProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus					err;
	CFDictionaryRef				params;
	uint32_t					objectID;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFTypeRef					value = NULL;
	CFMutableDictionaryRef		reply;
	ScreenRef					screen;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	screen = (ScreenRef) _FindObject( ctx, objectID, ScreenGetTypeID(), &err );
	require_noerr( err, exit );
	
	value = ScreenCopyProperty( screen, property, qualifier, &err );
	require_noerr_quiet( err, exit );
	
	reply = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( reply, exit, err = kNoMemoryErr );
	if( value ) CFDictionarySetValue( reply, CFSTR( kAirPlayKey_Value ), value );

	err = _SendPlistReply( ctx->cnx, inMsg, reply );
	CFRelease( reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	CFReleaseNullSafe( value );
	return( err );
}

//===========================================================================================================================
//	_Server_ScreenSetProperty
//===========================================================================================================================

static OSStatus	_Server_ScreenSetProperty( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	CFStringRef			property;
	CFTypeRef			qualifier;
	CFTypeRef			value;
	ScreenRef			screen;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	value = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Value ) );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	screen = (ScreenRef) _FindObject( ctx, objectID, ScreenGetTypeID(), &err );
	require_noerr( err, exit );
	
	err = ScreenSetProperty( screen, property, qualifier, value );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	return( err );
}

//===========================================================================================================================
//	_Server_ScreenRegister
//===========================================================================================================================

static OSStatus	_Server_ScreenRegister( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg )
{
	OSStatus		err;
	uint32_t		objectID;
	ScreenRef		screen;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	screen = (ScreenRef) _FindObject( ctx, objectID, ScreenGetTypeID(), &err );
	require_noerr( err, exit );
	
	err = ScreenRegister( screen );
	require_noerr( err, exit );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Utils ==
#endif

//===========================================================================================================================
//	_AddObject
//===========================================================================================================================

static OSStatus	_AddObject( AirPlayReceiverIPCServerContext *ctx, CFTypeRef inObject, uint32_t *outID )
{
	OSStatus		err;
	uint32_t		objectID;
	
	if( !ctx->objectMap )
	{
		// Do not retain values so as to prevent objects that are added internally (e.g. not explicitly created by clients)
		//  from never being explicitly released.  Instead, callers should "leak" their reference until _RemoveObjectXXX is
		//  called (passing 'true' for the 'release' parameter to "unleak" said reference) <rdar:15933063>
		ctx->objectMap = CFDictionaryCreateMutable( NULL, 0, NULL, NULL );
		require_action( ctx->objectMap, exit, err = kNoMemoryErr );
	}
	
	objectID = gLastObjectID + 1;
	if( objectID == 0 ) ++objectID;
	gLastObjectID = objectID;
	CFDictionarySetValue( ctx->objectMap, (const void *)(uintptr_t) objectID, inObject );
	
	*outID = objectID;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_RemoveObjectByID
//===========================================================================================================================

static void	_RemoveObjectByID( AirPlayReceiverIPCServerContext *ctx, uint32_t inObjectID, Boolean release )
{
	CFTypeRef		obj;
	
	if( ctx->objectMap )
	{
		obj = CFDictionaryGetValue( ctx->objectMap, (const void *)(uintptr_t) inObjectID );
		if( obj )
		{
			CFDictionaryRemoveValue( ctx->objectMap, (const void *)(uintptr_t) inObjectID );
			if( release ) CFRelease( obj );
		}
	}
}

//===========================================================================================================================
//	_RemoveObjectByMsg
//===========================================================================================================================

static OSStatus	_RemoveObjectByMsg( AirPlayReceiverIPCServerContext *ctx, IPCMessageRef inMsg, CFTypeID inTypeID, Boolean release )
{
	OSStatus		err;
	uint32_t		objectID;
	CFTypeRef		obj;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	require_action( ctx->objectMap, exit, err = kIDErr );
	obj = CFDictionaryGetValue( ctx->objectMap, (const void *)(uintptr_t) objectID );
	require_action( obj, exit, err = kNotFoundErr );
	require_action( CFGetTypeID( obj ) == inTypeID, exit, err = kTypeErr );
	
	CFDictionaryRemoveValue( ctx->objectMap, (const void *)(uintptr_t) objectID );
	if( release ) CFRelease( obj );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_FindObject
//===========================================================================================================================

static CFTypeRef	_FindObject( AirPlayReceiverIPCServerContext *ctx, uint32_t inObjectID, CFTypeID inTypeID, OSStatus *outErr )
{
	CFTypeRef		result = NULL;
	CFTypeRef		obj;
	OSStatus		err;
	
	require_action( ctx->objectMap, exit, err = kNotFoundErr );
	obj = CFDictionaryGetValue( ctx->objectMap, (const void *)(uintptr_t) inObjectID );
	require_action( obj, exit, err = kNotFoundErr );
	require_action( CFGetTypeID( obj ) == inTypeID, exit, err = kTypeErr );
	
	result = obj;
	err = kNoErr;
exit:
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	_SendPlistRequest
//===========================================================================================================================

static OSStatus		_SendPlistRequest( IPCServerConnectionRef inCnx, uint32_t inOpcode, CFPropertyListRef inPlist )
{
	OSStatus		err;
	CFDataRef		data;
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	
	err = _SendSimpleRequest( inCnx, inOpcode, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SendSimpleRequest
//===========================================================================================================================

static OSStatus	_SendSimpleRequest( IPCServerConnectionRef inCnx, uint32_t inOpcode, const void *inData, size_t inLen )
{
	OSStatus			err;
	IPCMessageRef		reply = NULL;
	
	err = IPCMessageCreate( &reply );
	require_noerr( err, exit );	
	
	err = IPCMessageSetContent( reply, inOpcode, kIPCFlags_None, kNoErr, inData, inLen );
	require_noerr( err, exit );	
	
	err = IPCServerConnectionSendMessage( inCnx, reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( reply );
	return( err );
}

//===========================================================================================================================
//	_SendPlistReply
//===========================================================================================================================

static OSStatus	_SendPlistReply( IPCServerConnectionRef	inCnx, IPCMessageRef inMsg, CFPropertyListRef inPlist )
{
	OSStatus		err;
	CFDataRef		data;
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	
	err = _SendSimpleReply( inCnx, inMsg, kNoErr, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SendSimpleReply
//===========================================================================================================================

static OSStatus
	_SendSimpleReply( 
		IPCServerConnectionRef	inCnx, 
		IPCMessageRef			inMsg, 
		OSStatus				inStatus, 
		const void *			inData, 
		size_t					inLen )
{
	OSStatus			err;
	IPCMessageRef		reply = NULL;
	
	err = IPCMessageCreate( &reply );
	require_noerr( err, exit );	
	
	err = IPCMessageSetContent( reply, inMsg->header.opcode, kIPCFlag_Reply, inStatus, inData, inLen );
	require_noerr( err, exit );	
	reply->header.xid = inMsg->header.xid;
	
	err = IPCServerConnectionSendMessage( inCnx, reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( reply );
	return( err );
}

//===========================================================================================================================
//	_SendPlistWithReplyPlistOneShot
//===========================================================================================================================

static OSStatus
	_SendPlistWithReplyPlistOneShot( 
		IPCServerConnectionRef	inCnx, 
		uint32_t				inOpcode, 
		uint32_t				inFlags, 
		OSStatus				inStatus, 
		CFDictionaryRef			inPlist, 
		CFDictionaryRef *		outPlist )
{
	OSStatus			err;
	IPCMessageRef		request	= NULL;
	IPCMessageRef		reply	= NULL;
	CFDictionaryRef		plist   = NULL;
	CFDataRef			data;
	
	err = IPCMessageCreate( &request );
	require_noerr( err, exit );
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	err = IPCMessageSetContent( request, inOpcode, inFlags, inStatus, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	if( outPlist )
	{
		err = IPCServerConnectionSendMessageWithReplySync( inCnx, request, &reply );
		require_noerr( err, exit );
		
		if( reply->bodyLen > 0 )
		{
			plist = CFDictionaryCreateWithBytes( reply->bodyPtr, reply->bodyLen, &err );
			require_noerr( err, exit );
		}
		
		*outPlist = plist;
		plist = NULL;
		err = reply->header.status;
	}
	else
	{
		err = IPCServerConnectionSendMessage( inCnx, request );
		require_noerr( err, exit );
	}
	
exit:
	CFReleaseNullSafe( plist );
	CFReleaseNullSafe( request );
	CFReleaseNullSafe( reply );
	return( err );
}
