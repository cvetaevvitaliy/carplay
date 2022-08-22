/*
	File:    	AirPlayReceiverIPCClient.c
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

// The IPC client wrappers for HIDUtils and ScreenUtils should never be mangled.
#undef HIDUTILS_IPC
#undef SCREENUTILS_IPC

#include "AirPlayCommon.h"
#include "AirPlayReceiverIPCCommon.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayReceiverSession.h"
#include "AudioUtils.h"
#include "IPCUtils.h"
#include "HIDUtils.h"
#include "ScreenUtils.h"

//===========================================================================================================================
//	Mappings
//===========================================================================================================================

#define AirPlayReceiverServerCreate_ipc						AirPlayReceiverServerCreate
#define AirPlayReceiverServerSetDelegate_ipc				AirPlayReceiverServerSetDelegate
#define AirPlayReceiverServerControl_ipc					AirPlayReceiverServerControl

#define AirPlayReceiverSessionSetDelegate_ipc				AirPlayReceiverSessionSetDelegate
#define AirPlayReceiverSessionControl_ipc					AirPlayReceiverSessionControl
#define AirPlayReceiverSessionChangeAppState_ipc			AirPlayReceiverSessionChangeAppState
#define AirPlayReceiverSessionChangeModes_ipc				AirPlayReceiverSessionChangeModes
#define AirPlayReceiverSessionChangeResourceMode_ipc		AirPlayReceiverSessionChangeResourceMode
#define AirPlayReceiverSessionForceKeyFrame_ipc				AirPlayReceiverSessionForceKeyFrame
#define AirPlayReceiverSessionRequestSiriAction_ipc			AirPlayReceiverSessionRequestSiriAction
#define AirPlayReceiverSessionRequestUI_ipc					AirPlayReceiverSessionRequestUI
#define AirPlayReceiverSessionSetLimitedUI_ipc				AirPlayReceiverSessionSetLimitedUI
#define AirPlayReceiverSessionSetNightMode_ipc				AirPlayReceiverSessionSetNightMode

//===========================================================================================================================
//	Proxy
//===========================================================================================================================

#define kProxyType_Other		0
#define kProxyType_Server		1
#define kProxyType_Session		2

typedef struct RemoteProxyPrivate *		RemoteProxyRef;

typedef void ( *RemoteProxyFinalize_f )( CFTypeRef inCF );

typedef struct
{
	RemoteProxyFinalize_f		finalizer_f;
	
}	RemoteProxyDelegate;

#define RemoteProxyDelegateInit( PTR )		memset( (PTR), 0, sizeof( RemoteProxyDelegate ) )

struct RemoteProxyPrivate
{
	CFRuntimeBase			base;		// CF type info. Must be first.
	RemoteProxyDelegate		delegate;	// Delegate used to configure and implement customizations.
	uint32_t				objectID;	// Identifier to access the remote object.
	uint32_t				type;		// Type of object this proxy represents.
	union
	{
		struct
		{
			AirPlayReceiverServerDelegate		delegate;
		
		}	server;
		
		struct
		{
			AirPlayReceiverSessionDelegate		delegate;
		
		}	session;
		
	}	u;
};

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static OSStatus	_EnsureInitialized( void );
static void		_Initialize( void *inContext );
static OSStatus	_HandleMessage( IPCClientRef inClient, IPCMessageRef inMsg );

static void		_AirPlayReceiverServerFinalize( CFTypeRef inCF );
static void		_Client_AirPlayReceiverServerDelegate_Control( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverServerDelegate_CopyProperty( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverServerDelegate_SetProperty( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverServerDelegate_SessionCreated( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverServerDelegate_SessionFailed( IPCClientRef inClient, IPCMessageRef inMsg );

static void		_Client_AirPlayReceiverSessionDelegate_Finalize( IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_Control( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_CopyProperty( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_SetProperty( IPCClientRef inClient, IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_ModesChanged( IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_RequestUI( IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_DuckAudio( IPCMessageRef inMsg );
static void		_Client_AirPlayReceiverSessionDelegate_UnduckAudio( IPCMessageRef inMsg );

static void		_HIDDeviceFinalize( CFTypeRef inCF );
static void		_ScreenFinalize( CFTypeRef inCF );

static CFTypeID	RemoteProxyGetTypeID( void );
static void		_RemoteProxyGetTypeID( void *inContext );
static OSStatus	RemoteProxyCreate( RemoteProxyRef *outProxy, const RemoteProxyDelegate *inDelegate, uint32_t inObjectID );
static void		_RemoteProxyFinalize( CFTypeRef inObj );

static RemoteProxyRef	_FindProxyByID( uint32_t inObjectID, uint32_t inType, OSStatus *outErr );
static OSStatus
	_SendMessageOneShot( 
		uint32_t		inOpcode, 
		uint32_t		inFlags, 
		OSStatus		inStatus, 
		const void *	inBody, 
		size_t			inLen );

static OSStatus
	_SendMessageWithReplyOneShot( 
		uint32_t		inOpcode, 
		uint32_t		inFlags, 
		OSStatus		inStatus, 
		const void *	inRequestPtr, 
		size_t			inRequestLen, 
		void *			inReplyPtr, 
		size_t			inReplyLen );

static OSStatus
	_SendPlistWithReplyPlistOneShot( 
		uint32_t			inOpcode, 
		uint32_t			inFlags, 
		OSStatus			inStatus, 
		CFDictionaryRef		inPlist, 
		CFDictionaryRef *	outPlist );
static OSStatus	_SendPlistReply( IPCClientRef inClient, IPCMessageRef inMsg, CFPropertyListRef inPlist );
static OSStatus	_SendSimpleReply( IPCClientRef inClient, IPCMessageRef inMsg, OSStatus inStatus, const void *inData, size_t inLen );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static dispatch_once_t			gRemoteProxyInitOnce = 0;
static CFTypeID					gRemoteProxyTypeID = _kCFRuntimeNotATypeID;
static const CFRuntimeClass		kRemoteProxyClass = 
{
	0,						// version
	"RemoteProxy",			// className
	NULL,					// init
	NULL,					// copy
	_RemoteProxyFinalize,	// finalize
	NULL,					// equal -- NULL means pointer equality.
	NULL,					// hash  -- NULL means pointer hash.
	NULL,					// copyFormattingDesc
	NULL,					// copyDebugDesc
	NULL,					// reclaim
	NULL					// refcount
};

static dispatch_once_t				gInitOnce;
static OSStatus						gInitErr	= kNoErr;
static IPCClientRef					gIPCClient	= NULL;
static CFMutableDictionaryRef		gProxyMap	= NULL;

//===========================================================================================================================
//	_EnsureInitialized
//===========================================================================================================================

static OSStatus	_EnsureInitialized( void )
{
	dispatch_once_f( &gInitOnce, NULL, _Initialize );
	return( gInitErr );
}

static void	_Initialize( void *inContext )
{
	OSStatus				err;
	IPCClientDelegate		delegate;
	
	(void) inContext;
	
	gProxyMap = CFDictionaryCreateMutable( NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks );
	require_action( gProxyMap, exit, err = kNoMemoryErr );
	
	IPCClientDelegateInit( &delegate );
	delegate.handleMessage_f = _HandleMessage;
	err = IPCClientCreate( &gIPCClient, &delegate );
	require_noerr( err, exit );
	
	err = IPCClientStart( gIPCClient );
	require_noerr( err, exit );
	
exit:
	gInitErr = err;
}

//===========================================================================================================================
//	_HandleMessage
//===========================================================================================================================

static OSStatus	_HandleMessage( IPCClientRef inClient, IPCMessageRef inMsg )
{
	switch( inMsg->header.opcode )
	{
		// Server
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_Control:
			_Client_AirPlayReceiverServerDelegate_Control( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_CopyProperty:
			_Client_AirPlayReceiverServerDelegate_CopyProperty( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SetProperty:
			_Client_AirPlayReceiverServerDelegate_SetProperty( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionCreated:
			_Client_AirPlayReceiverServerDelegate_SessionCreated( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionFailed:
			_Client_AirPlayReceiverServerDelegate_SessionFailed( inClient, inMsg );
			break;
		
		// Session
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Finalize:
			_Client_AirPlayReceiverSessionDelegate_Finalize( inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Control:
			_Client_AirPlayReceiverSessionDelegate_Control( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_CopyProperty:
			_Client_AirPlayReceiverSessionDelegate_CopyProperty( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_SetProperty:
			_Client_AirPlayReceiverSessionDelegate_SetProperty( inClient, inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_ModesChanged:
			_Client_AirPlayReceiverSessionDelegate_ModesChanged( inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_RequestUI:
			_Client_AirPlayReceiverSessionDelegate_RequestUI( inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_DuckAudio:
			_Client_AirPlayReceiverSessionDelegate_DuckAudio( inMsg );
			break;
		
		case kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_UnduckAudio:
			_Client_AirPlayReceiverSessionDelegate_UnduckAudio( inMsg );
			break;
		
		default:
			break;
	}
	return( kNoErr );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AirPlayReceiverServerCreate
//===========================================================================================================================

OSStatus	AirPlayReceiverServerCreate_ipc( AirPlayReceiverServerRef *outServer )
{
	OSStatus				err;
	uint32_t				objectID;
	RemoteProxyDelegate		delegate;
	RemoteProxyRef			proxy;
	
	err = _SendMessageWithReplyOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverServerCreate, 0, kNoErr, NULL, 0, 
		&objectID, sizeof( objectID ) );
	require_noerr( err, exit );
	
	RemoteProxyDelegateInit( &delegate );
	delegate.finalizer_f = _AirPlayReceiverServerFinalize;
	err = RemoteProxyCreate( &proxy, &delegate, objectID );
	require_noerr( err, exit );
	proxy->type = kProxyType_Server;
	
	*outServer = (AirPlayReceiverServerRef) proxy;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_AirPlayReceiverServerFinalize
//===========================================================================================================================

static void	_AirPlayReceiverServerFinalize( CFTypeRef inCF )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inCF;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverServerRelease, 0, kNoErr, 
		&proxy->objectID, sizeof( proxy->objectID ) );
}

//===========================================================================================================================
//	AirPlayReceiverServerSetDelegate
//===========================================================================================================================

void	AirPlayReceiverServerSetDelegate_ipc( AirPlayReceiverServerRef inServer, const AirPlayReceiverServerDelegate *inDelegate )
{
	RemoteProxyRef const						proxy = (RemoteProxyRef) inServer;
	AirPlayReceiverServerSetDelegateIPCData		data;
	
	proxy->u.server.delegate = *inDelegate;
	
	memset( &data, 0, sizeof( data ) );
	data.objectID = proxy->objectID;
	if( inDelegate->control_f )			data.control_f			= true;
	if( inDelegate->copyProperty_f )	data.copyProperty_f		= true;
	if( inDelegate->setProperty_f )		data.setProperty_f		= true;
	if( inDelegate->sessionCreated_f )	data.sessionCreated_f	= true;
	if( inDelegate->sessionFailed_f )	data.sessionFailed_f	= true;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverServerSetDelegate, 0, kNoErr, &data, sizeof( data ) );
}

//===========================================================================================================================
//	AirPlayReceiverServerControl
//===========================================================================================================================

OSStatus
	AirPlayReceiverServerControl_ipc( 
		CFTypeRef			inServer, 
		uint32_t			inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outParams )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inServer;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inFlags;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Command ), inCommand );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inParams )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inParams );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverServerControl, 0, kNoErr, 
		request, outParams );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverServerDelegate_Control
//===========================================================================================================================

static void	_Client_AirPlayReceiverServerDelegate_Control( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	CFStringRef			command;
	CFTypeRef			qualifier;
	CFDictionaryRef		request;
	CFDictionaryRef		response;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Server, &err );
	require_noerr( err, exit );
	
	command = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	request = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	require_action( proxy->u.server.delegate.control_f, exit, err = kNotHandledErr );
	err = proxy->u.server.delegate.control_f( (AirPlayReceiverServerRef) proxy, command, qualifier, request, &response, 
		proxy->u.server.delegate.context );
	require_noerr_quiet( err, exit );
	
	if( response )
	{
		err = _SendPlistReply( inClient, inMsg, response );
		CFRelease( response );
		require_noerr( err, exit );
	}
	else
	{
		err = _SendSimpleReply( inClient, inMsg, kNoErr, NULL, 0 );
		require_noerr( err, exit );
	}
	
exit:
	CFReleaseNullSafe( params );
	if( err ) _SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverServerDelegate_CopyProperty
//===========================================================================================================================

static void	_Client_AirPlayReceiverServerDelegate_CopyProperty( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus					err;
	CFDictionaryRef				params;
	uint32_t					objectID;
	RemoteProxyRef				proxy;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFMutableDictionaryRef		reply;
	CFTypeRef					value = NULL;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Server, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	
	require_action( proxy->u.server.delegate.copyProperty_f, exit, err = kNotHandledErr );
	value = proxy->u.server.delegate.copyProperty_f( (AirPlayReceiverServerRef) proxy, property, qualifier, &err, 
		proxy->u.server.delegate.context );
	require_noerr_quiet( err, exit );
	
	reply = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( reply, exit, err = kNoMemoryErr );
	if( value ) CFDictionarySetValue( reply, CFSTR( kAirPlayKey_Value ), value );
	
	err = _SendPlistReply( inClient, inMsg, reply );
	CFRelease( reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	if( err ) _SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverServerDelegate_SetProperty
//===========================================================================================================================

static void	_Client_AirPlayReceiverServerDelegate_SetProperty( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	CFStringRef			property;
	CFTypeRef			qualifier;
	CFTypeRef			value;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Server, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	value = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	require_action( proxy->u.server.delegate.setProperty_f, exit, err = kNotHandledErr );
	err = proxy->u.server.delegate.setProperty_f( (AirPlayReceiverServerRef) proxy, property, qualifier, value, 
		proxy->u.server.delegate.context );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	_SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverServerDelegate_SessionCreated
//===========================================================================================================================

static void	_Client_AirPlayReceiverServerDelegate_SessionCreated( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus				err;
	CFDictionaryRef			params;
	uint32_t				objectID;
	RemoteProxyRef			proxy;
	RemoteProxyRef			sessionProxy;
	RemoteProxyDelegate		delegate;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Server, &err );
	require_noerr( err, exit );
	
	// Set up the proxy for the session.
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID2, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	RemoteProxyDelegateInit( &delegate );
	err = RemoteProxyCreate( &sessionProxy, &delegate, objectID );
	require_noerr( err, exit );
	sessionProxy->type = kProxyType_Session;
	
	require_action( proxy->u.server.delegate.sessionCreated_f, exit, err = kNotHandledErr );
	proxy->u.server.delegate.sessionCreated_f( (AirPlayReceiverServerRef) proxy, (AirPlayReceiverSessionRef) sessionProxy, 
		proxy->u.server.delegate.context );
	
exit:
	CFReleaseNullSafe( params );
	_SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverServerDelegate_SessionFailed
//===========================================================================================================================

static void	_Client_AirPlayReceiverServerDelegate_SessionFailed( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus				err;
	CFDictionaryRef			params;
	uint32_t				objectID;
	RemoteProxyRef			proxy;
	OSStatus				reason;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Server, &err );
	require_noerr( err, exit );
	
	// Set up the proxy for the session.
	
	reason = (OSStatus) CFDictionaryGetInt64( params, CFSTR( kAirPlayKey_ReasonCode ), &err );
	require_noerr( err, exit );
	
	require_action( proxy->u.server.delegate.sessionFailed_f, exit, err = kNotHandledErr );
	proxy->u.server.delegate.sessionFailed_f( (AirPlayReceiverServerRef) proxy, reason, 
		proxy->u.server.delegate.context );
	
exit:
	CFReleaseNullSafe( params );
	_SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AirPlayReceiverSessionSetDelegate
//===========================================================================================================================

void	AirPlayReceiverSessionSetDelegate_ipc( AirPlayReceiverSessionRef inSession, const AirPlayReceiverSessionDelegate *inDelegate )
{
	RemoteProxyRef const							proxy = (RemoteProxyRef) inSession;
	AirPlayReceiverSessionSetDelegateIPCData		data;
	
	proxy->u.session.delegate = *inDelegate;
	
	memset( &data, 0, sizeof( data ) );
	data.objectID = proxy->objectID;
	if( inDelegate->initialize_f )		data.initialize_f	= true;
	if( inDelegate->finalize_f )		data.finalize_f		= true;
	if( inDelegate->control_f )			data.control_f		= true;
	if( inDelegate->copyProperty_f )	data.copyProperty_f	= true;
	if( inDelegate->setProperty_f )		data.setProperty_f	= true;
	if( inDelegate->modesChanged_f )	data.modesChanged_f	= true;
	if( inDelegate->requestUI_f )		data.requestUI_f	= true;
	if( inDelegate->duckAudio_f )		data.duckAudio_f	= true;
	if( inDelegate->unduckAudio_f )		data.unduckAudio_f	= true;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetDelegate, 0, kNoErr, &data, sizeof( data ) );
}

//===========================================================================================================================
//	AirPlayReceiverSessionControl
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionControl_ipc( 
		CFTypeRef			inSession, 
		uint32_t			inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outParams )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inFlags;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Command ), inCommand );
	if( inQualifier )	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inParams )		CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inParams );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionControl, 0, kNoErr, 
		request, outParams );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionChangeAppState
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionChangeAppState_ipc( 
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
	
	err = AirPlayReceiverSessionChangeModes_ipc( inSession, &changes, inReason, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionChangeModes
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionChangeModes_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		const AirPlayModeChanges *					inChanges, 
		CFStringRef									inReason, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetData( request, kAirPlayReceiverIPCKey_ModeChanges, inChanges, sizeof( *inChanges ) );
	if( inReason ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_ReasonStr ), inReason );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionChangeModes, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionChangeResourceMode
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionChangeResourceMode_ipc( 
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
	
	err = AirPlayReceiverSessionChangeModes_ipc( inSession, &changes, inReason, inCompletion, inContext );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionForceKeyFrame
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionForceKeyFrame_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionForceKeyFrame, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionRequestSiriAction
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionRequestSiriAction_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		AirPlaySiriAction							inSiriAction, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetInt64( request, CFSTR( kAirPlayKey_SiriAction ), (int64_t) inSiriAction );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestSiriAction, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionRequestUI
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionRequestUI_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		CFStringRef									inURL, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	if( inURL ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_URL ), inURL );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestUI, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetLimitedUI
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionSetLimitedUI_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		Boolean										inLimitedUI, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetBoolean( request, CFSTR( kAirPlayKey_LimitedUI ), inLimitedUI );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetLimitedUI, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayReceiverSessionSetNightMode
//===========================================================================================================================

OSStatus
	AirPlayReceiverSessionSetNightMode_ipc( 
		AirPlayReceiverSessionRef					inSession, 
		Boolean										inNightMode, 
		AirPlayReceiverSessionCommandCompletionFunc	inCompletion, 
		void *										inContext )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inSession;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inContext;
	require_action( inCompletion == NULL, exit, err = kUnsupportedErr );
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetBoolean( request, CFSTR( kAirPlayKey_NightMode ), inNightMode );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetNightMode, 0, kNoErr, 
		request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_Finalize
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_Finalize( IPCMessageRef inMsg )
{
	OSStatus			err;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	if( proxy->u.session.delegate.finalize_f ) proxy->u.session.delegate.finalize_f( (AirPlayReceiverSessionRef) proxy, proxy->u.session.delegate.context );

	CFRelease( proxy );
	
exit:
	return;
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_Control
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_Control( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	CFStringRef			command;
	CFTypeRef			qualifier;
	CFDictionaryRef		request;
	CFDictionaryRef		response;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	command = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	request = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	require_action( proxy->u.session.delegate.control_f, exit, err = kNotHandledErr );
	err = proxy->u.session.delegate.control_f( (AirPlayReceiverSessionRef) proxy, command, qualifier, request, &response, 
		proxy->u.session.delegate.context );
	require_noerr_quiet( err, exit );
	
	if( response )
	{
		err = _SendPlistReply( inClient, inMsg, response );
		CFRelease( response );
		require_noerr( err, exit );
	}
	else
	{
		err = _SendSimpleReply( inClient, inMsg, kNoErr, NULL, 0 );
		require_noerr( err, exit );
	}
	
exit:
	CFReleaseNullSafe( params );
	if( err ) _SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_CopyProperty
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_CopyProperty( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus					err;
	CFDictionaryRef				params;
	uint32_t					objectID;
	RemoteProxyRef				proxy;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFMutableDictionaryRef		reply;
	CFTypeRef					value = NULL;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Property ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	
	require_action( proxy->u.session.delegate.copyProperty_f, exit, err = kNotHandledErr );
	value = proxy->u.session.delegate.copyProperty_f( (AirPlayReceiverSessionRef) proxy, property, qualifier, &err, 
		proxy->u.session.delegate.context );
	require_noerr_quiet( err, exit );
	
	reply = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( reply, exit, err = kNoMemoryErr );
	if( value ) CFDictionarySetValue( reply, CFSTR( kAirPlayKey_Value ), value );
	
	err = _SendPlistReply( inClient, inMsg, reply );
	CFRelease( reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	if( err ) _SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_SetProperty
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_SetProperty( IPCClientRef inClient, IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	CFStringRef			property;
	CFTypeRef			qualifier;
	CFTypeRef			value;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	property = CFDictionaryGetCFString( params, CFSTR( kAirPlayKey_Command ), &err );
	require_noerr( err, exit );
	
	qualifier = CFDictionaryGetValue( params, CFSTR( kAirPlayKey_Qualifier ) );
	value = CFDictionaryGetCFDictionary( params, CFSTR( kAirPlayKey_Params ), NULL );
	
	require_action( proxy->u.session.delegate.setProperty_f, exit, err = kNotHandledErr );
	err = proxy->u.session.delegate.setProperty_f( (AirPlayReceiverSessionRef) proxy, property, qualifier, value, 
		proxy->u.session.delegate.context );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( params );
	_SendSimpleReply( inClient, inMsg, err, NULL, 0 );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_ModesChanged
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_ModesChanged( IPCMessageRef inMsg )
{
	OSStatus										err;
	AirPlayReceiverSessionModesChangedIPCData		ipcData;
	RemoteProxyRef									proxy;
	
	require_action( inMsg->bodyLen == sizeof( ipcData ), exit, err = kSizeErr );
	memcpy( &ipcData, inMsg->bodyPtr, sizeof( ipcData ) );
	require_action( ipcData.objectID != 0, exit, err = kIDErr );
	proxy = _FindProxyByID( ipcData.objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	require_action( proxy->u.session.delegate.modesChanged_f, exit, err = kNotHandledErr );
	proxy->u.session.delegate.modesChanged_f( (AirPlayReceiverSessionRef) proxy, &ipcData.state, 
		proxy->u.session.delegate.context );
	
exit:
	return;
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_RequestUI
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_RequestUI( IPCMessageRef inMsg )
{
	OSStatus			err;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	
	require_action( inMsg->bodyLen == sizeof( objectID ), exit, err = kSizeErr );
	objectID = ReadHost32( inMsg->bodyPtr );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	require_action( proxy->u.session.delegate.requestUI_f, exit, err = kNotHandledErr );
	proxy->u.session.delegate.requestUI_f( (AirPlayReceiverSessionRef) proxy, proxy->u.session.delegate.context );
	
exit:
	return;
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_DuckAudio
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_DuckAudio( IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	double				duration;
	double				volume;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	duration = CFDictionaryGetDouble( params, CFSTR( kAirPlayKey_DurationMs ), &err );
	require_noerr( err, exit );
	
	volume = CFDictionaryGetDouble( params, CFSTR( kAirPlayKey_Decibels ), &err );
	require_noerr( err, exit );
	
	require_action( proxy->u.session.delegate.duckAudio_f, exit, err = kNotHandledErr );
	proxy->u.session.delegate.duckAudio_f( (AirPlayReceiverSessionRef) proxy, duration, volume, proxy->u.session.delegate.context );
	
exit:
	CFReleaseNullSafe( params );
}

//===========================================================================================================================
//	_Client_AirPlayReceiverSessionDelegate_UnduckAudio
//===========================================================================================================================

static void	_Client_AirPlayReceiverSessionDelegate_UnduckAudio( IPCMessageRef inMsg )
{
	OSStatus			err;
	CFDictionaryRef		params;
	uint32_t			objectID;
	RemoteProxyRef		proxy;
	double				duration;
	
	params = CFDictionaryCreateWithBytes( inMsg->bodyPtr, inMsg->bodyLen, &err );
	require_noerr( err, exit );
	
	objectID = (uint32_t) CFDictionaryGetInt64( params, kAirPlayReceiverIPCKey_ObjectID, &err );
	require_noerr( err, exit );
	require_action( objectID != 0, exit, err = kIDErr );
	
	proxy = _FindProxyByID( objectID, kProxyType_Session, &err );
	require_noerr( err, exit );
	
	duration = CFDictionaryGetDouble( params, CFSTR( kAirPlayKey_DurationMs ), &err );
	require_noerr( err, exit );
	
	require_action( proxy->u.session.delegate.unduckAudio_f, exit, err = kNotHandledErr );
	proxy->u.session.delegate.unduckAudio_f( (AirPlayReceiverSessionRef) proxy, duration, proxy->u.session.delegate.context );
	
exit:
	CFReleaseNullSafe( params );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	HIDDeviceCreateVirtual
//===========================================================================================================================

OSStatus	HIDDeviceCreateVirtual( HIDDeviceRef *outDevice, CFDictionaryRef inProperties )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDataRef					data;
	uint32_t					objectID;
	RemoteProxyDelegate			delegate;
	RemoteProxyRef				proxy;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	if( inProperties ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inProperties );
	
	data = CFPropertyListCreateData( NULL, request, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	CFRelease( request );
	require_action( data, exit, err = kUnknownErr );
	
	err = _SendMessageWithReplyOneShot( kAirPlayReceiverIPCOpcode_HIDDeviceCreateVirtual, 0, kNoErr, 
		CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ), &objectID, sizeof( objectID ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	RemoteProxyDelegateInit( &delegate );
	delegate.finalizer_f = _HIDDeviceFinalize;
	err = RemoteProxyCreate( &proxy, &delegate, objectID );
	require_noerr( err, exit );
	
	*outDevice = (HIDDeviceRef) proxy;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_HIDDeviceFinalize
//===========================================================================================================================

static void	_HIDDeviceFinalize( CFTypeRef inCF )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inCF;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_HIDDeviceRelease, 0, kNoErr, &proxy->objectID, sizeof( proxy->objectID ) );
}

//===========================================================================================================================
//	HIDDeviceCopyProperty
//===========================================================================================================================

CFTypeRef	HIDDeviceCopyProperty( HIDDeviceRef inDevice, CFStringRef inProperty, CFTypeRef inQualifier, OSStatus *outErr )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inDevice;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDictionaryRef				reply = NULL;
	CFTypeRef					value = NULL;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_HIDDeviceCopyProperty, 0, kNoErr, request, &reply );
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
//	HIDDeviceSetProperty
//===========================================================================================================================

OSStatus	HIDDeviceSetProperty( HIDDeviceRef inDevice, CFStringRef inProperty, CFTypeRef inQualifier, CFTypeRef inValue )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inDevice;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inValue ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Value ), inValue );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_HIDDeviceSetProperty, 0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDDevicePostReport
//===========================================================================================================================

OSStatus	HIDDevicePostReport( HIDDeviceRef inDevice, const void *inReportPtr, size_t inReportLen )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inDevice;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetData( request, CFSTR( kAirPlayKey_HIDReport ), inReportPtr, inReportLen );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_HIDDevicePostReport, 0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDPostReport
//===========================================================================================================================

OSStatus	HIDPostReport( CFStringRef inUUID, const void *inReportPtr, size_t inReportLen )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_UUID ), inUUID );
	CFDictionarySetData( request, CFSTR( kAirPlayKey_HIDReport ), inReportPtr, inReportLen );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_HIDPostReport, 0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDRegisterDevice
//===========================================================================================================================

OSStatus	HIDRegisterDevice( HIDDeviceRef inDevice )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inDevice;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_HIDRegisterDevice, 0, kNoErr, &proxy->objectID, sizeof( proxy->objectID ) );
	return( kNoErr );
}

//===========================================================================================================================
//	HIDDeviceStop
//===========================================================================================================================

void	HIDDeviceStop( HIDDeviceRef inDevice )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inDevice;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_HIDDeviceStop, 0, kNoErr, &proxy->objectID, sizeof( proxy->objectID ) );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	ScreenCreate
//===========================================================================================================================

OSStatus	ScreenCreate( ScreenRef *outScreen, CFDictionaryRef inProperties )
{
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDataRef					data;
	uint32_t					objectID;
	RemoteProxyDelegate			delegate;
	RemoteProxyRef				proxy;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	if( inProperties ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Params ), inProperties );
	
	data = CFPropertyListCreateData( NULL, request, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	CFRelease( request );
	require_action( data, exit, err = kUnknownErr );
	
	err = _SendMessageWithReplyOneShot( kAirPlayReceiverIPCOpcode_ScreenCreate, 0, kNoErr, 
		CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ), &objectID, sizeof( objectID ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	RemoteProxyDelegateInit( &delegate );
	delegate.finalizer_f = _ScreenFinalize;
	err = RemoteProxyCreate( &proxy, &delegate, objectID );
	require_noerr( err, exit );
	
	*outScreen = (ScreenRef) proxy;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ScreenFinalize
//===========================================================================================================================

static void	_ScreenFinalize( CFTypeRef inCF )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inCF;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_ScreenRelease, 0, kNoErr, &proxy->objectID, sizeof( proxy->objectID ) );
}

//===========================================================================================================================
//	_ScreenCopyProperty
//===========================================================================================================================

CFTypeRef
	_ScreenCopyProperty( 
		CFTypeRef		inObject, // Must be a ScreenRef
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inObject;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	CFDictionaryRef				reply = NULL;
	CFTypeRef					value = NULL;
	
	(void) inFlags;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_ScreenCopyProperty, 0, kNoErr, request, &reply );
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
//	_ScreenSetProperty
//===========================================================================================================================

OSStatus
	_ScreenSetProperty( 
		CFTypeRef		inObject, // Must be a ScreenRef
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inObject;
	OSStatus					err;
	CFMutableDictionaryRef		request;
	
	(void) inFlags;
	
	request = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( request, exit, err = kNoMemoryErr );
	CFDictionarySetInt64( request, kAirPlayReceiverIPCKey_ObjectID, proxy->objectID );
	CFDictionarySetValue( request, CFSTR( kAirPlayKey_Property ), inProperty );
	if( inQualifier ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Qualifier ), inQualifier );
	if( inValue ) CFDictionarySetValue( request, CFSTR( kAirPlayKey_Value ), inValue );
	
	err = _SendPlistWithReplyPlistOneShot( kAirPlayReceiverIPCOpcode_ScreenSetProperty, 0, kNoErr, request, NULL );
	CFRelease( request );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	ScreenRegister
//===========================================================================================================================

OSStatus	ScreenRegister( ScreenRef inScreen )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inScreen;
	
	_SendMessageOneShot( kAirPlayReceiverIPCOpcode_ScreenRegister, 0, kNoErr, &proxy->objectID, sizeof( proxy->objectID ) );
	return( kNoErr );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	RemoteProxyGetTypeID
//===========================================================================================================================

static CFTypeID	RemoteProxyGetTypeID( void )
{
	dispatch_once_f( &gRemoteProxyInitOnce, NULL, _RemoteProxyGetTypeID );
	return( gRemoteProxyTypeID );
}

static void _RemoteProxyGetTypeID( void *inContext )
{
	(void) inContext;
	
	gRemoteProxyTypeID = _CFRuntimeRegisterClass( &kRemoteProxyClass );
	check( gRemoteProxyTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	RemoteProxyCreate
//===========================================================================================================================

static OSStatus	RemoteProxyCreate( RemoteProxyRef *outProxy, const RemoteProxyDelegate *inDelegate, uint32_t inObjectID )
{
	OSStatus			err;
	RemoteProxyRef		obj;
	size_t				extraLen;
	
	extraLen = sizeof( *obj ) - sizeof( obj->base );
	obj = (RemoteProxyRef) _CFRuntimeCreateInstance( NULL, RemoteProxyGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( obj, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) obj ) + sizeof( obj->base ), 0, extraLen );
	
	obj->delegate = *inDelegate;
	obj->objectID = inObjectID;
	CFDictionarySetValue( gProxyMap, (const void *)(uintptr_t) inObjectID, obj );
	
	*outProxy = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_RemoteProxyFinalize
//===========================================================================================================================

static void	_RemoteProxyFinalize( CFTypeRef inObj )
{
	RemoteProxyRef const		proxy = (RemoteProxyRef) inObj;
	
	if( proxy->delegate.finalizer_f ) proxy->delegate.finalizer_f( inObj );
	CFDictionaryRemoveValue( gProxyMap, (const void *)(uintptr_t) proxy->objectID );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_FindProxyByID
//===========================================================================================================================

static RemoteProxyRef	_FindProxyByID( uint32_t inObjectID, uint32_t inType, OSStatus *outErr )
{
	RemoteProxyRef		proxy = NULL;
	RemoteProxyRef		obj;
	OSStatus			err;
	
	obj = (RemoteProxyRef)( gProxyMap ? CFDictionaryGetValue( gProxyMap, (void *)(uintptr_t) inObjectID ) : NULL );
	require_action( obj, exit, err = kNotFoundErr );
	require_action( obj->type == inType, exit, err = kTypeErr );
	proxy = obj;
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( proxy );
}

//===========================================================================================================================
//	_SendMessageOneShot
//===========================================================================================================================

static OSStatus
	_SendMessageOneShot( 
		uint32_t		inOpcode, 
		uint32_t		inFlags, 
		OSStatus		inStatus, 
		const void *	inBody, 
		size_t			inLen )
{
	OSStatus			err;
	IPCMessageRef		request	= NULL;
	
	err = _EnsureInitialized();
	require_noerr( err, exit );
	
	err = IPCMessageCreate( &request );
	require_noerr( err, exit );
	
	err = IPCMessageSetContent( request, inOpcode, inFlags, inStatus, inBody, inLen );
	require_noerr( err, exit );
	
	err = IPCClientSendMessage( gIPCClient, request );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( request );
	return( err );
}

//===========================================================================================================================
//	_SendMessageOneShot
//===========================================================================================================================

static OSStatus
	_SendMessageWithReplyOneShot( 
		uint32_t		inOpcode, 
		uint32_t		inFlags, 
		OSStatus		inStatus, 
		const void *	inRequestPtr, 
		size_t			inRequestLen, 
		void *			inReplyPtr, 
		size_t			inReplyLen )
{
	OSStatus			err;
	IPCMessageRef		request	= NULL;
	IPCMessageRef		reply	= NULL;
	
	err = _EnsureInitialized();
	require_noerr( err, exit );
	
	err = IPCMessageCreate( &request );
	require_noerr( err, exit );
	
	err = IPCMessageSetContent( request, inOpcode, inFlags, inStatus, inRequestPtr, inRequestLen );
	require_noerr( err, exit );
	
	err = IPCClientSendMessageWithReplySync( gIPCClient, request, &reply );
	require_noerr( err, exit );
	require_action( reply->bodyLen == inReplyLen, exit, err = kSizeErr );
	memcpy( inReplyPtr, reply->bodyPtr, inReplyLen );
	
exit:
	CFReleaseNullSafe( request );
	CFReleaseNullSafe( reply );
	return( err );
}

//===========================================================================================================================
//	_SendPlistWithReplyPlistOneShot
//===========================================================================================================================

static OSStatus
	_SendPlistWithReplyPlistOneShot( 
		uint32_t			inOpcode, 
		uint32_t			inFlags, 
		OSStatus			inStatus, 
		CFDictionaryRef		inPlist, 
		CFDictionaryRef *	outPlist )
{
	OSStatus			err;
	IPCMessageRef		request	= NULL;
	IPCMessageRef		reply	= NULL;
	CFDictionaryRef		plist   = NULL;
	CFDataRef			data;
	
	err = _EnsureInitialized();
	require_noerr( err, exit );
	
	err = IPCMessageCreate( &request );
	require_noerr( err, exit );
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	err = IPCMessageSetContent( request, inOpcode, inFlags, inStatus, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
	if( outPlist )
	{
		err = IPCClientSendMessageWithReplySync( gIPCClient, request, &reply );
		require_noerr( err, exit );
		require_noerr_action_quiet( reply->header.status, exit, err = reply->header.status );
		
		if( reply->bodyLen > 0 )
		{
			plist = CFDictionaryCreateWithBytes( reply->bodyPtr, reply->bodyLen, &err );
			require_noerr( err, exit );
		}
		
		*outPlist = plist;
		plist = NULL;
	}
	else
	{
		err = IPCClientSendMessage( gIPCClient, request );
		require_noerr( err, exit );
	}
	
exit:
	CFReleaseNullSafe( plist );
	CFReleaseNullSafe( request );
	CFReleaseNullSafe( reply );
	return( err );
}

//===========================================================================================================================
//	_SendPlistReply
//===========================================================================================================================

static OSStatus	_SendPlistReply( IPCClientRef inClient, IPCMessageRef inMsg, CFPropertyListRef inPlist )
{
	OSStatus		err;
	CFDataRef		data;
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	
	err = _SendSimpleReply( inClient, inMsg, kNoErr, CFDataGetBytePtr( data ), (size_t) CFDataGetLength( data ) );
	CFRelease( data );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SendSimpleReply
//===========================================================================================================================

static OSStatus	_SendSimpleReply( IPCClientRef inClient, IPCMessageRef inMsg, OSStatus inStatus, const void *inData, size_t inLen )
{
	OSStatus			err;
	IPCMessageRef		reply = NULL;
	
	err = _EnsureInitialized();
	require_noerr( err, exit );
	
	err = IPCMessageCreate( &reply );
	require_noerr( err, exit );	
	
	err = IPCMessageSetContent( reply, inMsg->header.opcode, kIPCFlag_Reply, inStatus, inData, inLen );
	require_noerr( err, exit );	
	reply->header.xid = inMsg->header.xid;
	
	err = IPCClientSendMessage( inClient, reply );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( reply );
	return( err );
}
