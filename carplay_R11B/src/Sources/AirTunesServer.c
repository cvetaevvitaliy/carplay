/*
	File:    	AirTunesServer.c
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
	
	Copyright (C) 2007-2014 Apple Inc. All Rights Reserved.
*/

#include "AirTunesServer.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "AirPlayCommon.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayReceiverServerPriv.h"
#include "AirPlayReceiverSession.h"
#include "AirPlayReceiverSessionPriv.h"
#include "AirPlaySettings.h"
#include "AirPlayVersion.h"
#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "Base64Utils.h"
#include "CFUtils.h"
#include "HTTPServer.h"
#include "HTTPUtils.h"
#include "NetUtils.h"
#include "RandomNumberUtils.h"
#include "StringUtils.h"
#include "SystemUtils.h"
#include "ThreadUtils.h"
#include "TickUtils.h"
#include "URLUtils.h"

#if( TARGET_OS_BSD )
	#include <net/if_media.h>
	#include <net/if.h>
#endif

	#include "APSMFiSAP.h"

//===========================================================================================================================
//	Internal
//===========================================================================================================================

typedef struct
{
	HTTPServerRef		httpServer;                     // HTTP server
	dispatch_queue_t	queue;                          // queue to run all operations and deliver callbacks on
	
	uint8_t				httpTimedNonceKey[ 16 ];
	
	Boolean				started;
}	AirTunesServer;

typedef struct AirTunesConnection 
{
	AirTunesServer				*atServer;
#if( TARGET_OS_POSIX )
	NetworkChangeListenerRef	networkChangeListener;
#endif
	
	uint64_t					clientDeviceID;
	uint8_t						clientInterfaceMACAddress[ 6 ];
	char						clientName[ 128 ];
	uint64_t					clientSessionID;
	uint32_t					clientVersion;
	Boolean						didAnnounce;
	Boolean						didAudioSetup;
	Boolean						didScreenSetup;
	Boolean						didRecord;
	
	char						ifName[ IF_NAMESIZE + 1 ];	// Name of the interface the connection was accepted on.
	
	Boolean						httpAuthentication_IsAuthenticated;
	
	AirPlayCompressionType		compressionType;
	uint32_t					samplesPerFrame;
	
	APSMFiSAPRef				MFiSAP;
	Boolean						MFiSAPDone;
	uint8_t						encryptionKey[ 16 ];
	uint8_t						encryptionIV[ 16 ];
	Boolean						usingEncryption;
	
	uint32_t					minLatency, maxLatency;
	
	AirPlayReceiverSessionRef	session;
}	AirTunesConnection;

AirPlayCompressionType				gAirPlayAudioCompressionType	= kAirPlayCompressionType_Undefined;
static AirTunesServer *				gAirTunesServer					= NULL;
static Boolean						gAirTunesPlaybackAllowed		= true;
static Boolean						gAirTunesDenyInterruptions		= false;

ulog_define( AirTunesServerCore, kLogLevelNotice, kLogFlags_Default, "AirPlay",  NULL );
#define ats_ucat()						&log_category_from_name( AirTunesServerCore )
#define ats_ulog( LEVEL, ... )			ulog( ats_ucat(), (LEVEL), __VA_ARGS__ )

ulog_define( AirTunesServerHTTP, kLogLevelNotice, kLogFlags_Default, "AirPlay",  NULL );
#define ats_http_ucat()					&log_category_from_name( AirTunesServerHTTP )
#define ats_http_ulog( LEVEL, ... )		ulog( ats_http_ucat(), (LEVEL), __VA_ARGS__ )

	#define PairingDescription( atCnx )				""

	#define AirTunesConnection_DidSetup( atCnx )	( (atCnx)->didAudioSetup || (atCnx)->didScreenSetup )

#if 0
#pragma mark == Prototypes ==
#endif

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static HTTPConnectionRef _AirTunesServer_FindActiveConnection( AirTunesServer *inServer );
static void _AirTunesServer_HijackConnections( AirTunesServer *inServer, HTTPConnectionRef inHijacker );
static void _AirTunesServer_RemoveAllConnections( AirTunesServer *inServer, HTTPConnectionRef inConnectionToKeep );

static OSStatus		_connectionInitialize( HTTPConnectionRef inCnx );
static void			_connectionFinalize( HTTPConnectionRef inCnx );
static void			_connectionDestroy( HTTPConnectionRef inCnx );
static OSStatus		_connectionHandleMessage( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );

	static HTTPStatus	_requestProcessAuthSetup( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessCommand( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessFeedback( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessGetLog( HTTPConnectionRef inCnx );
static HTTPStatus	_requestProcessInfo( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessOptions( HTTPConnectionRef inCnx );
static HTTPStatus	_requestProcessGetParameter( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessSetParameter( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessSetParameterText( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessSetProperty( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessRecord( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessSetup( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessSetupPlist( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );
static HTTPStatus	_requestProcessTearDown( HTTPConnectionRef inCnx, HTTPMessageRef inMsg );

static OSStatus		_requestCreateSession( HTTPConnectionRef inCnx, Boolean inUseEvents );
static OSStatus
	_requestDecryptKey(
		HTTPConnectionRef		inCnx,
		CFDictionaryRef			inRequestParams,
		AirPlayEncryptionType	inType,
		uint8_t					inKeyBuf[ 16 ],
		uint8_t					inIVBuf[ 16 ] );
static HTTPStatus
	_requestSendPlistResponse(
		HTTPConnectionRef	inCnx,
		HTTPMessageRef		inMsg,
		CFPropertyListRef	inPlist,
		OSStatus *			outErr );
#if( TARGET_OS_POSIX )
	static void			_requestNetworkChangeListenerHandleEvent( uint32_t inEvent, void *inContext );
#endif

#if 0
#pragma mark == AirTunes Control ==
#endif

//===========================================================================================================================
//	AirTunesServer_EnsureCreated
//===========================================================================================================================

OSStatus AirTunesServer_EnsureCreated( void )
{
	OSStatus err;
	
	if( !gAirTunesServer )
	{
		HTTPServerDelegate delegate;
		
		gAirTunesServer	= (AirTunesServer *) calloc( 1, sizeof( AirTunesServer ) );
		require_action( gAirTunesServer, exit, err = kNoMemoryErr );
		
		gAirTunesServer->queue = dispatch_queue_create( "AirTunesServerQueue", 0 );
		require_action( gAirTunesServer->queue, exit, err = kNoMemoryErr );
		
		gAirTunesPlaybackAllowed = true;
		
		RandomBytes( gAirTunesServer->httpTimedNonceKey, sizeof( gAirTunesServer->httpTimedNonceKey ) );
		
		HTTPServerDelegateInit( &delegate );
		delegate.initializeConnection_f = _connectionInitialize;
		delegate.handleMessage_f        = _connectionHandleMessage;
		delegate.finalizeConnection_f   = _connectionFinalize;
		
		err = HTTPServerCreate( &gAirTunesServer->httpServer, &delegate );
		require_noerr( err, exit );
		
		gAirTunesServer->httpServer->listenPort = -kAirPlayPort_RTSPControl;
		HTTPServerSetDispatchQueue( gAirTunesServer->httpServer, gAirTunesServer->queue );
		//HTTPServerSetLogging( gAirTunesServer->httpServer, ats_ucat() );
		
	}
	err = kNoErr;
	
exit:
	if ( err ) AirTunesServer_EnsureDeleted();
	return err;
}

//===========================================================================================================================
//	AirTunesServer_EnsureDeleted
//===========================================================================================================================

void	AirTunesServer_EnsureDeleted( void )
{
	if( gAirTunesServer )
	{
		
		HTTPServerForget( &gAirTunesServer->httpServer );
		dispatch_forget( &gAirTunesServer->queue );
		ForgetMem( &gAirTunesServer );
	}
}

//===========================================================================================================================
//	AirTunesServer_Start
//===========================================================================================================================

OSStatus	AirTunesServer_Start( void )
{
	OSStatus		err;
	
	require_action( gAirTunesServer && gAirTunesServer->httpServer, exit, err = kNotInitializedErr );
	require_action_quiet( !gAirTunesServer->started, exit, err = kNoErr );
	
	// Start HTTPServer synchronously
	err = HTTPServerStartSync( gAirTunesServer->httpServer );
	require_noerr( err, exit );
	
	gAirTunesServer->started = true;
	err = kNoErr;
	
exit:
	return err;
}

//===========================================================================================================================
//	AirTunesServer_GetListenPort
//===========================================================================================================================

int	AirTunesServer_GetListenPort( void )
{
	if( gAirTunesServer && gAirTunesServer->httpServer ) return( gAirTunesServer->httpServer->listeningPort );
	return( 0 );
}

//===========================================================================================================================
//	AirTunesServer_SetDenyInterruptions
//===========================================================================================================================

void	AirTunesServer_SetDenyInterruptions( Boolean inDeny )
{
	if( inDeny != gAirTunesDenyInterruptions )
	{
		gAirTunesDenyInterruptions = inDeny;
		
	}
}

//===========================================================================================================================
//	AirTunesServer_FailureOccurred
//===========================================================================================================================

void	AirTunesServer_FailureOccurred( OSStatus inError )
{
	DEBUG_USE_ONLY( inError );
	
	ats_ulog( kLogLevelNotice, "### Failure: %#m\n", inError );
	_AirTunesServer_RemoveAllConnections( gAirTunesServer, NULL );
}

//===========================================================================================================================
//	AirTunesServer_StopAllConnections
//===========================================================================================================================

void	AirTunesServer_StopAllConnections( void )
{
	_AirTunesServer_RemoveAllConnections( gAirTunesServer, NULL );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_AirTunesServer_PlaybackAllowed
//===========================================================================================================================

static Boolean _AirTunesServer_PlaybackAllowed( void )
{
	if( gAirTunesPlaybackAllowed )
	{
		OSStatus		err;
		Boolean			b;
		
		b = AirPlayReceiverServerPlatformGetBoolean( gAirPlayReceiverServer, CFSTR( kAirPlayProperty_PlaybackAllowed ),
			NULL, &err );
		if( b || ( err == kNotHandledErr ) )
		{
			return( true );
		}
	}
	return( false );
}

//===========================================================================================================================
//	AirTunesServer_AllowPlayback
//===========================================================================================================================

void	AirTunesServer_AllowPlayback( void )
{
	if( !gAirTunesPlaybackAllowed )
	{
		dlog( kLogLevelNotice, "*** ALLOW PLAYBACK ***\n" );
		gAirTunesPlaybackAllowed = true;
		
	}
}

//===========================================================================================================================
//	AirTunesServer_PreventPlayback
//===========================================================================================================================

void	AirTunesServer_PreventPlayback( void )
{
	if( gAirTunesPlaybackAllowed )
	{
		dlog( kLogLevelNotice, "*** PREVENT PLAYBACK ***\n" );
		gAirTunesPlaybackAllowed = false;
		_AirTunesServer_RemoveAllConnections( gAirTunesServer, NULL );
		
	}
}

#if 0
#pragma mark -
#endif

#if 0
#pragma mark -
#pragma mark == AirTunesServer ==
#endif

//===========================================================================================================================
//	_AirTunesServer_FindActiveConnection
//===========================================================================================================================

static HTTPConnectionRef _AirTunesServer_FindActiveConnection( AirTunesServer *inServer )
{
	HTTPConnectionRef cnx;
	
	if( !( inServer && inServer->httpServer ) ) return NULL;
	
	for( cnx = inServer->httpServer->connections; cnx; cnx = cnx->next )
	{
		AirTunesConnection *atCnx = (AirTunesConnection *) cnx->userContext;
		
		if( atCnx->didAnnounce )
		{
			return( cnx );
		}
	}
	
	return( NULL );
}

//===========================================================================================================================
//	_AirTunesServer_HijackConnections
//===========================================================================================================================

static void _AirTunesServer_HijackConnections( AirTunesServer *inServer, HTTPConnectionRef inHijacker )
{
	HTTPConnectionRef *	next;
	HTTPConnectionRef	conn;
	
	if( !( inServer && inServer->httpServer ) ) return;
	
	// Close any connections that have started audio (should be 1 connection at most).
	// This leaves connections that haven't started audio because they may just be doing OPTIONS requests, etc.
	
	next = &inServer->httpServer->connections;
	while( ( conn = *next ) != NULL )
	{
		AirTunesConnection *atConn = (AirTunesConnection *) conn->userContext;
		
		if( ( conn != inHijacker ) && atConn->didAnnounce )
		{
			ats_ulog( kLogLevelNotice, "*** Hijacking connection %##a for %##a\n", &conn->peerAddr, &inHijacker->peerAddr );
			*next = conn->next;
			_connectionDestroy( conn );
		}
		else
		{
			next = &conn->next;
		}
	}
	
}

//===========================================================================================================================
//	_AirTunesServer_RemoveAllConnections
//===========================================================================================================================

static void _AirTunesServer_RemoveAllConnections( AirTunesServer *inServer, HTTPConnectionRef inConnectionToKeep )
{
	HTTPConnectionRef *	next;
	HTTPConnectionRef	conn;
	
	if( !( inServer && inServer->httpServer ) ) return;
	
	next = &inServer->httpServer->connections;
	while( ( conn = *next ) != NULL )
	{
		if( conn == inConnectionToKeep )
		{
			next = &conn->next;
			continue;
		}
		
		*next = conn->next;
		_connectionDestroy( conn );
	}
	
	check( inServer->httpServer->connections == inConnectionToKeep );
	check( !inServer->httpServer->connections || ( inServer->httpServer->connections->next == NULL ) );
}

#if 0
#pragma mark -
#pragma mark == AirTunesConnection ==
#endif

//===========================================================================================================================
//	_connectionInitialize
//===========================================================================================================================

static OSStatus _connectionInitialize( HTTPConnectionRef inCnx )
{
	OSStatus err;
	AirTunesConnection *atCnx;
	
	atCnx = (AirTunesConnection *) calloc( 1, sizeof( AirTunesConnection ) );
	require_action( atCnx, exit, err = kNoMemoryErr );
	
	atCnx->atServer			= gAirTunesServer;
	atCnx->compressionType	= kAirPlayCompressionType_Undefined;
	inCnx->userContext		= atCnx;
	err						= kNoErr;

    SocketSetKeepAlive( inCnx->sock, kAirPlayDataTimeoutSecs / 5, 5 );

exit:
	return err;
}

//===========================================================================================================================
//	_connectionFinalize
//===========================================================================================================================

static void _connectionFinalize( HTTPConnectionRef inCnx )
{
	AirTunesConnection *atCnx;
	
	atCnx = (AirTunesConnection *) inCnx->userContext;
	if( !atCnx ) return;
	
	atCnx->atServer = NULL;
	
#if( TARGET_OS_POSIX )
	if( atCnx->networkChangeListener )
	{
		NetworkChangeListenerStop( atCnx->networkChangeListener );
		ForgetCF( &atCnx->networkChangeListener );
	}
#endif
	if( atCnx->session )
	{
		AirPlayReceiverSessionTearDown( atCnx->session, NULL, atCnx->didAnnounce ? kConnectionErr : kNoErr, NULL );
		CFRelease( atCnx->session );
		atCnx->session = NULL;
		AirPlayReceiverServerSetBoolean( gAirPlayReceiverServer, CFSTR( kAirPlayProperty_Playing ), NULL, false );
	}
	if( atCnx->MFiSAP )
	{
		APSMFiSAP_Delete( atCnx->MFiSAP );
		atCnx->MFiSAP = NULL;
		atCnx->MFiSAPDone = false;
	}
	
	ForgetMem( &inCnx->userContext );
}

//===========================================================================================================================
//	_connectionDestroy
//===========================================================================================================================

static void _connectionDestroy( HTTPConnectionRef inCnx )
{
	HTTPConnectionStop( inCnx );
	if( inCnx->selfAddr.sa.sa_family != AF_UNSPEC )
	{
		ats_ulog( kLogLevelInfo, "Closing  connection from %##a to %##a\n", &inCnx->peerAddr, &inCnx->selfAddr );
	}
	CFRelease( inCnx );
}

//===========================================================================================================================
//	_connectionHandleMessage
//===========================================================================================================================

#define GetHeaderValue( req, name, outVal, outValLen ) \
	HTTPGetHeaderField( (req)->header.buf, (req)->header.len, name, NULL, NULL, outVal, outValLen, NULL )

static OSStatus _connectionHandleMessage( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	OSStatus				err;
	HTTPMessageRef			response	= inCnx->responseMsg;
	const char * const		methodPtr	= request->header.methodPtr;
	size_t const			methodLen	= request->header.methodLen;
	const char * const		pathPtr		= request->header.url.pathPtr;
	size_t const			pathLen		= request->header.url.pathLen;
	Boolean					logHTTP		= true;
	AirTunesConnection *	atCnx		= (AirTunesConnection *) inCnx->userContext;
	const char *			httpProtocol;
	HTTPStatus				status;
	const char *			cSeqPtr		= NULL;
	size_t					cSeqLen		= 0;
	
	require_action( atCnx, exit, err = kParamErr );
	
	httpProtocol = ( strnicmp_prefix(request->header.protocolPtr, request->header.protocolLen, "HTTP" ) == 0 )
		? "HTTP/1.1" : kAirTunesHTTPVersionStr;
	
	// OPTIONS and /feedback requests are too chatty so don't log them by default.
	
	if( ( ( strnicmpx( methodPtr, methodLen, "OPTIONS" ) == 0 ) ||
		( ( strnicmpx( methodPtr, methodLen, "POST" ) == 0 ) && ( strnicmp_suffix( pathPtr, pathLen, "/feedback" ) == 0 ) ) ) &&
		!log_category_enabled( ats_http_ucat(), kLogLevelChatty ) )
	{
		logHTTP = false;
	}
	if( logHTTP ) LogHTTP( ats_http_ucat(), ats_http_ucat(), request->header.buf, request->header.len,
		request->bodyPtr, request->bodyLen );
	
	if( atCnx->session ) ++atCnx->session->source.activityCount;
	
	GetHeaderValue( request, kHTTPHeader_CSeq, &cSeqPtr, &cSeqLen );
	
	// Parse the client device's ID. If not provided (e.g. older device) then fabricate one from the IP address.
	
	HTTPScanFHeaderValue( request->header.buf, request->header.len, kAirPlayHTTPHeader_DeviceID, "%llx", &atCnx->clientDeviceID );
	if( atCnx->clientDeviceID == 0 ) atCnx->clientDeviceID = SockAddrToDeviceID( &inCnx->peerAddr );
	
	if( *atCnx->clientName == '\0' )
	{
		const char *		namePtr	= NULL;
		size_t				nameLen	= 0;
		
		GetHeaderValue( request, kAirPlayHTTPHeader_ClientName, &namePtr, &nameLen );
		if( nameLen > 0 ) TruncateUTF8( namePtr, nameLen, atCnx->clientName, sizeof( atCnx->clientName ), true );
	}
	
	// Process the request. Assume success initially, but we'll change it if there is an error.
	// Note: methods are ordered below roughly to how often they are used (most used earlier).
	
	err = HTTPHeader_InitResponse( &response->header, httpProtocol, kHTTPStatus_OK, NULL );
	require_noerr( err, exit );
	response->bodyLen = 0;
	
	if(      strnicmpx( methodPtr, methodLen, "OPTIONS" )			== 0 ) status = _requestProcessOptions( inCnx );
	else if( strnicmpx( methodPtr, methodLen, "SET_PARAMETER" )		== 0 ) status = _requestProcessSetParameter( inCnx, request );
	else if( strnicmpx( methodPtr, methodLen, "GET_PARAMETER" ) 	== 0 ) status = _requestProcessGetParameter( inCnx, request );
	else if( strnicmpx( methodPtr, methodLen, "RECORD" )			== 0 ) status = _requestProcessRecord( inCnx, request );
	else if( strnicmpx( methodPtr, methodLen, "SETUP" )				== 0 ) status = _requestProcessSetup( inCnx, request );
	else if( strnicmpx( methodPtr, methodLen, "TEARDOWN" )			== 0 ) status = _requestProcessTearDown( inCnx, request );
	else if( strnicmpx( methodPtr, methodLen, "GET" )				== 0 )
	{
		if( 0 ) {}
		else if( strnicmp_suffix( pathPtr, pathLen, "/log" )		== 0 ) status = _requestProcessGetLog( inCnx );
		else if( strnicmp_suffix( pathPtr, pathLen, "/info" )		== 0 ) status = _requestProcessInfo( inCnx, request );
		else { dlog( kLogLevelNotice, "### Unsupported GET: '%.*s'\n", (int) pathLen, pathPtr ); status = kHTTPStatus_NotFound; }
	}
	else if( strnicmpx( methodPtr, methodLen, "POST" )				== 0 )
	{
		if( 0 ) {}
		else if( strnicmp_suffix( pathPtr, pathLen, "/auth-setup" )	== 0 ) status = _requestProcessAuthSetup( inCnx, request );
		else if( strnicmp_suffix( pathPtr, pathLen, "/command" )	== 0 ) status = _requestProcessCommand( inCnx, request );
		else if( strnicmp_suffix( pathPtr, pathLen, "/diag-info" )	== 0 ) status = kHTTPStatus_OK;
		else if( strnicmp_suffix( pathPtr, pathLen, "/feedback" )	== 0 ) status = _requestProcessFeedback( inCnx, request );
		else if( strnicmp_suffix( pathPtr, pathLen, "/info" )		== 0 ) status = _requestProcessInfo( inCnx, request );
		else { dlogassert( "Bad POST: '%.*s'", (int) pathLen, pathPtr ); status = kHTTPStatus_NotFound; }
	}
	else { dlogassert( "Bad method: %.*s", (int) methodLen, methodPtr ); status = kHTTPStatus_NotImplemented; }
	goto SendResponse;
	
SendResponse:
	
	// If an error occurred, reset the response message with a new status.
	
	if( status != kHTTPStatus_OK )
	{
		err = HTTPHeader_InitResponse( &response->header, httpProtocol, status, NULL );
		require_noerr( err, exit );
		response->bodyLen = 0;
		
		err = HTTPHeader_SetField( &response->header, kHTTPHeader_ContentLength, "0" );
		require_noerr( err, exit );
	}
	
	// Server
	
	err = HTTPHeader_SetField( &response->header, kHTTPHeader_Server, "AirTunes/%s", kAirPlaySourceVersionStr );
	require_noerr( err, exit );
	
	// WWW-Authenticate
	
	if( status == kHTTPStatus_Unauthorized )
	{
		char		nonce[ 64 ];
		
		err = HTTPMakeTimedNonce( kHTTPTimedNonceETagPtr, kHTTPTimedNonceETagLen,
			atCnx->atServer->httpTimedNonceKey, sizeof( atCnx->atServer->httpTimedNonceKey ),
			nonce, sizeof( nonce ), NULL );
		require_noerr( err, exit );
		
		err = HTTPHeader_SetField( &response->header, kHTTPHeader_WWWAuthenticate, "Digest realm=\"airplay\", nonce=\"%s\"", nonce );
		require_noerr( err, exit );
	}
	
	// CSeq
	
	if( cSeqPtr )
	{
		err = HTTPHeader_SetField( &response->header, kHTTPHeader_CSeq, "%.*s", (int) cSeqLen, cSeqPtr );
		require_noerr( err, exit );
	}
	
	// Apple-Challenge
	
	if( logHTTP ) LogHTTP( ats_http_ucat(), ats_http_ucat(), response->header.buf, response->header.len,
		response->bodyPtr, response->bodyLen );
	
	err = HTTPConnectionSendResponse( inCnx );
	require_noerr( err, exit );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Request Processing ==
#endif

//===========================================================================================================================
//	_requestProcessAuthSetup
//===========================================================================================================================

static HTTPStatus _requestProcessAuthSetup( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	OSStatus				err;
	uint8_t *				outputPtr;
	size_t					outputLen;
	AirTunesConnection *	atCnx		= (AirTunesConnection *) inCnx->userContext;
	HTTPMessageRef			response	= inCnx->responseMsg;
	
	ats_ulog( kAirPlayPhaseLogLevel, "MFi\n" );
	outputPtr = NULL;
	require_action( request->bodyOffset > 0, exit, status = kHTTPStatus_BadRequest );
	
	// Let MFi-SAP process the input data and generate output data.
	
	if( atCnx->MFiSAPDone && atCnx->MFiSAP )
	{
		APSMFiSAP_Delete( atCnx->MFiSAP );
		atCnx->MFiSAP = NULL;
		atCnx->MFiSAPDone = false;
	}
	if( !atCnx->MFiSAP )
	{
		err = APSMFiSAP_Create( &atCnx->MFiSAP, kAPSMFiSAPVersion1 );
		require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	}
	
	err = APSMFiSAP_Exchange( atCnx->MFiSAP, request->bodyPtr, request->bodyOffset, &outputPtr, &outputLen, &atCnx->MFiSAPDone );
	require_noerr_action( err, exit, status = kHTTPStatus_Forbidden );
	
	// Send the MFi-SAP output data in the response.
	
	err = HTTPMessageSetBody( response, kMIMEType_Binary, outputPtr, outputLen );
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
	status = kHTTPStatus_OK;
	
exit:
	if( outputPtr ) free( outputPtr );
	return( status );
}

//===========================================================================================================================
//	_requestProcessCommand
//===========================================================================================================================

static HTTPStatus _requestProcessCommand( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	OSStatus				err;
	CFDictionaryRef			requestDict;
	CFStringRef				command = NULL;
	CFDictionaryRef			params;
	CFDictionaryRef			responseDict;
	AirTunesConnection *	atCnx = (AirTunesConnection *) inCnx->userContext;
	
	requestDict = CFDictionaryCreateWithBytes( request->bodyPtr, request->bodyOffset, &err );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	command = CFDictionaryGetCFString( requestDict, CFSTR( kAirPlayKey_Type ), NULL );
	require_action( command, exit, err = kParamErr; status = kHTTPStatus_ParameterNotUnderstood );
	
	params = CFDictionaryGetCFDictionary( requestDict, CFSTR( kAirPlayKey_Params ), NULL );
	
	// Perform the command and send its response.
	
	require_action( atCnx->session, exit, err = kNotPreparedErr; status = kHTTPStatus_SessionNotFound );
	responseDict = NULL;
	err = AirPlayReceiverSessionControl( atCnx->session, kCFObjectFlagDirect, command, NULL, params, &responseDict );
	require_noerr_action_quiet( err, exit, status = kHTTPStatus_UnprocessableEntity );
	if( !responseDict )
	{
		responseDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( responseDict, exit, err = kNoMemoryErr; status = kHTTPStatus_InternalServerError );
	}
	
	status = _requestSendPlistResponse( inCnx, request, responseDict, &err );
	CFRelease( responseDict );
	
exit:
	if( err ) ats_ulog( kLogLevelNotice, "### Command '%@' failed: %#m, %#m\n", command, status, err );
	CFReleaseNullSafe( requestDict );
	return( status );
}

//===========================================================================================================================
//	_requestProcessFeedback
//===========================================================================================================================

static HTTPStatus _requestProcessFeedback( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus					status;
	OSStatus					err;
	CFDictionaryRef				input	= NULL;
	CFMutableDictionaryRef		output	= NULL;
	CFDictionaryRef				dict;
	AirTunesConnection *		atCnx	= (AirTunesConnection *) inCnx->userContext;
	
	input = CFDictionaryCreateWithBytes( request->bodyPtr, request->bodyOffset, &err );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	output = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( output, exit, err = kNoMemoryErr; status = kHTTPStatus_InternalServerError );
	
	if( atCnx->session )
	{
		dict = NULL;
		AirPlayReceiverSessionControl( atCnx->session, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_UpdateFeedback ), NULL,
			input, &dict );
		if( dict )
		{
			CFDictionaryMergeDictionary( output, dict );
			CFRelease( dict );
		}
	}
	
	status = _requestSendPlistResponse( inCnx, request, ( CFDictionaryGetCount( output ) > 0 ) ? output : NULL, &err );
	
exit:
	if( err ) ats_ulog( kLogLevelNotice, "### Feedback failed: %#m, %#m\n", status, err );
	CFReleaseNullSafe( input );
	CFReleaseNullSafe( output );
	return( status );
}

//===========================================================================================================================
//	_requestProcessGetLog
//===========================================================================================================================

static HTTPStatus _requestProcessGetLog( HTTPConnectionRef inCnx )
{
	HTTPStatus		status;
	OSStatus		err;
	DataBuffer		dataBuf;
	uint8_t *		ptr;
	size_t			len;
	HTTPMessageRef	response = inCnx->responseMsg;
	
	DataBuffer_Init( &dataBuf, NULL, 0, 10 * kBytesPerMegaByte );
	
	{
		ats_ulog( kLogLevelNotice, "### Rejecting log request from non-internal build\n" );
		status = kHTTPStatus_NotFound;
		goto exit;
	}
	
	DataBuffer_AppendF( &dataBuf, "AirPlay Diagnostics\n" );
	DataBuffer_AppendF( &dataBuf, "===================\n" );
	AirTunesDebugAppendShowData( "globals", &dataBuf );
	AirTunesDebugAppendShowData( "stats", &dataBuf );
	AirTunesDebugAppendShowData( "rtt", &dataBuf );
	AirTunesDebugAppendShowData( "retrans", &dataBuf );
	AirTunesDebugAppendShowData( "retransDone", &dataBuf );
	
#if( TARGET_OS_POSIX )
	DataBuffer_AppendF( &dataBuf, "+-+ syslog +--\n" );
	DataBuffer_RunProcessAndAppendOutput( &dataBuf, "syslog" );
	DataBuffer_AppendFile( &dataBuf, kAirPlayPrimaryLogPath );
#endif
	
	err = DataBuffer_Commit( &dataBuf, &ptr, &len );
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
	err = HTTPMessageSetBody( response, "text/plain", ptr, len );
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
	status = kHTTPStatus_OK;
	
exit:
	DataBuffer_Free( &dataBuf );
	return( status );
}

//===========================================================================================================================
//	_requestProcessInfo
//===========================================================================================================================

static HTTPStatus _requestProcessInfo( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus					status;
	OSStatus					err;
	CFMutableDictionaryRef		requestDict;
	CFMutableArrayRef			qualifier = NULL;
	CFDictionaryRef				responseDict;
	const char *				src;
	const char *				end;
	const char *				namePtr;
	size_t						nameLen;
	char *						nameBuf;
	uint32_t					userVersion;
	AirTunesConnection *		atCnx = (AirTunesConnection *) inCnx->userContext;
	
	requestDict = CFDictionaryCreateMutableWithBytes( request->bodyPtr, request->bodyOffset, &err );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	qualifier = (CFMutableArrayRef) CFDictionaryGetCFArray( requestDict, CFSTR( kAirPlayKey_Qualifier ), NULL );
	if( qualifier ) CFRetain( qualifier );
	
	src = request->header.url.queryPtr;
	end = src + request->header.url.queryLen;
	while( ( err = URLGetOrCopyNextVariable( src, end, &namePtr, &nameLen, &nameBuf, NULL, NULL, NULL, &src ) ) == kNoErr )
	{
		err = CFArrayEnsureCreatedAndAppendCString( &qualifier, namePtr, nameLen );
		if( nameBuf ) free( nameBuf );
		require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	}
	
	HTTPScanFHeaderValue( request->header.buf, request->header.len, kAirPlayHTTPHeader_ProtocolVersion, "%u", &userVersion );
	if( atCnx->session ) AirPlayReceiverSessionSetUserVersion( atCnx->session, userVersion );
	
	responseDict = AirPlayCopyServerInfo( atCnx->session, qualifier, inCnx->ifMACAddress , &err );
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
	status = _requestSendPlistResponse( inCnx, request, responseDict, &err );
	CFReleaseNullSafe( responseDict );
	
exit:
	CFReleaseNullSafe( qualifier );
	CFReleaseNullSafe( requestDict );
	if( err ) ats_ulog( kLogLevelNotice, "### Get info failed: %#m, %#m\n", status, err );
	return( status );
}

//===========================================================================================================================
//	_requestProcessOptions
//===========================================================================================================================

static HTTPStatus _requestProcessOptions( HTTPConnectionRef inCnx )
{
	HTTPMessageRef response = inCnx->responseMsg;
	
	HTTPHeader_SetField( &response->header, kHTTPHeader_Public,
		"ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER, POST, GET" );
	
	return( kHTTPStatus_OK );
}

//===========================================================================================================================
//	_requestProcessGetParameter
//===========================================================================================================================

static HTTPStatus _requestProcessGetParameter( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	OSStatus				err;
	const char *			src;
	const char *			end;
	size_t					len;
	char					tempStr[ 256 ];
	char					responseBuf[ 256 ];
	int						n;
	HTTPMessageRef			response	= inCnx->responseMsg;
	AirTunesConnection *	atCnx		= (AirTunesConnection *) inCnx->userContext;
	
	if( !AirTunesConnection_DidSetup( atCnx ) )
	{
		dlogassert( "GET_PARAMETER not allowed before SETUP" );
		status = kHTTPStatus_MethodNotValidInThisState;
		goto exit;
	}
	
	// Check content type.
	
	err = GetHeaderValue( request, kHTTPHeader_ContentType, &src, &len );
	if( err )
	{
		dlogassert( "No Content-Type header" );
		status = kHTTPStatus_BadRequest;
		goto exit;
	}
	if( strnicmpx( src, len, "text/parameters" ) != 0 )
	{
		dlogassert( "Bad Content-Type: '%.*s'", (int) len, src );
		status = kHTTPStatus_HeaderFieldNotValid;
		goto exit;
	}
	
	// Parse parameters. Each parameter is formatted as <name>\r\n
	
	src = (const char *) request->bodyPtr;
	end = src + request->bodyOffset;
	while( src < end )
	{
		char				c;
		const char *		namePtr;
		size_t				nameLen;
		
		namePtr = src;
		while( ( src < end ) && ( ( c = *src ) != '\r' ) && ( c != '\n' ) ) ++src;
		nameLen = (size_t)( src - namePtr );
		if( ( nameLen == 0 ) || ( src >= end ) )
		{
			dlogassert( "Bad parameter: '%.*s'", (int) request->bodyOffset, request->bodyPtr );
			status = kHTTPStatus_ParameterNotUnderstood;
			goto exit;
		}
		
		// Process the parameter.
		
		if( 0 ) {}
		
		// Name
		
		else if( strnicmpx( namePtr, nameLen, "name" ) == 0 )
		{
			err = AirPlayGetDeviceName( tempStr, sizeof( tempStr ) );
			require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
			
			n = snprintf( responseBuf, sizeof( responseBuf ), "name: %s\r\n", tempStr );
			err = HTTPMessageSetBody( response, "text/parameters", responseBuf, (size_t) n );
			require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
		}
		
		// Other
		
		else
		{
			dlogassert( "Unknown parameter: '%.*s'", (int) nameLen, namePtr );
			status = kHTTPStatus_ParameterNotUnderstood;
			goto exit;
		}
		
		while( ( src < end ) && ( ( ( c = *src ) == '\r' ) || ( c == '\n' ) ) ) ++src;
	}
	
	status = kHTTPStatus_OK;
	
exit:
	return( status );
}

//===========================================================================================================================
//	_requestProcessSetParameter
//===========================================================================================================================

static HTTPStatus _requestProcessSetParameter( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	const char *			src;
	size_t					len;
	AirTunesConnection *	atCnx	= (AirTunesConnection *) inCnx->userContext;
	
	if( !AirTunesConnection_DidSetup( atCnx ) )
	{
		dlogassert( "SET_PARAMETER not allowed before SETUP" );
		status = kHTTPStatus_MethodNotValidInThisState;
		goto exit;
	}
	
	src = NULL;
	len = 0;
	GetHeaderValue( request, kHTTPHeader_ContentType, &src, &len );
	if( ( len == 0 ) && ( request->bodyOffset == 0 ) )			status = kHTTPStatus_OK;
	else if( strnicmpx( src, len, "text/parameters" ) == 0 )	status = _requestProcessSetParameterText( inCnx, request );
	else if( MIMETypeIsPlist( src, len ) )						status = _requestProcessSetProperty( inCnx, request );
	else { dlogassert( "Bad Content-Type: '%.*s'", (int) len, src ); status = kHTTPStatus_HeaderFieldNotValid; goto exit; }
	
exit:
	return( status );
}

//===========================================================================================================================
//	_requestProcessSetParameterText
//===========================================================================================================================

static HTTPStatus _requestProcessSetParameterText( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus			status;
	OSStatus			err;
	const char *		src;
	const char *		end;
	(void) inCnx;
	
	// Parse parameters. Each parameter is formatted as <name>: <value>\r\n
	
	src = (const char *) request->bodyPtr;
	end = src + request->bodyOffset;
	while( src < end )
	{
		char				c;
		const char *		namePtr;
		size_t				nameLen;
		const char *		valuePtr;
		size_t				valueLen;
		
		// Parse the name.
		
		namePtr = src;
		while( ( src < end ) && ( *src != ':' ) ) ++src;
		nameLen = (size_t)( src - namePtr );
		if( ( nameLen == 0 ) || ( src >= end ) )
		{
			dlogassert( "Bad parameter: '%.*s'", (int) request->bodyOffset, request->bodyPtr );
			status = kHTTPStatus_ParameterNotUnderstood;
			goto exit;
		}
		++src;
		while( ( src < end ) && ( ( ( c = *src ) == ' ' ) || ( c == '\t' ) ) ) ++src;
		
		// Parse the value.
		
		valuePtr = src;
		while( ( src < end ) && ( ( c = *src ) != '\r' ) && ( c != '\n' ) ) ++src;
		valueLen = (size_t)( src - valuePtr );
		
		// Process the parameter.
		
		if( 0 ) {}
		
		// Other
		
		else
		{
			(void) err;
			(void) valueLen;
			
			dlogassert( "Unknown parameter: '%.*s'", (int) nameLen, namePtr );
			status = kHTTPStatus_ParameterNotUnderstood;
			goto exit;
		}
		
		while( ( src < end ) && ( ( ( c = *src ) == '\r' ) || ( c == '\n' ) ) ) ++src;
	}
	
	status = kHTTPStatus_OK;
	
exit:
	return( status );
}

//===========================================================================================================================
//	_requestProcessSetProperty
//===========================================================================================================================

static HTTPStatus _requestProcessSetProperty( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	OSStatus				err;
	CFDictionaryRef			requestDict;
	CFStringRef				property = NULL;
	CFTypeRef				qualifier;
	CFTypeRef				value;
	AirTunesConnection *	atCnx = (AirTunesConnection *) inCnx->userContext;
	
	requestDict = CFDictionaryCreateWithBytes( request->bodyPtr, request->bodyOffset, &err );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	property = CFDictionaryGetCFString( requestDict, CFSTR( kAirPlayKey_Property ), NULL );
	require_action( property, exit, err = kParamErr; status = kHTTPStatus_BadRequest );
	
	qualifier	= CFDictionaryGetValue( requestDict, CFSTR( kAirPlayKey_Qualifier ) );
	value		= CFDictionaryGetValue( requestDict, CFSTR( kAirPlayKey_Value ) );
	
	// Set the property on the session.
	
	require_action( atCnx->session, exit, err = kNotPreparedErr; status = kHTTPStatus_SessionNotFound );
	err = AirPlayReceiverSessionSetProperty( atCnx->session, kCFObjectFlagDirect, property, qualifier, value );
	require_noerr_action_quiet( err, exit, status = kHTTPStatus_UnprocessableEntity );
	
	status = kHTTPStatus_OK;
	
exit:
	if( err ) ats_ulog( kLogLevelNotice, "### Set property '%@' failed: %#m, %#m\n", property, status, err );
	CFReleaseNullSafe( requestDict );
	return( status );
}

//===========================================================================================================================
//	_requestProcessRecord
//===========================================================================================================================

static HTTPStatus _requestProcessRecord( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus							status;
	OSStatus							err;
	const char *						src;
	const char *						end;
	size_t								len;
	const char *						namePtr;
	size_t								nameLen;
	const char *						valuePtr;
	size_t								valueLen;
	AirPlayReceiverSessionStartInfo		startInfo;
	AirTunesConnection *				atCnx = (AirTunesConnection *) inCnx->userContext;
	
	ats_ulog( kAirPlayPhaseLogLevel, "Record\n" );
	
	if( !AirTunesConnection_DidSetup( atCnx ) )
	{
		dlogassert( "RECORD not allowed before SETUP" );
		status = kHTTPStatus_MethodNotValidInThisState;
		goto exit;
	}
	
	memset( &startInfo, 0, sizeof( startInfo ) );
	startInfo.clientName	= atCnx->clientName;
	startInfo.transportType	= inCnx->transportType;
	
	// Parse session duration info.
	
	src = NULL;
	len = 0;
	GetHeaderValue( request, kAirPlayHTTPHeader_Durations, &src, &len );
	end = src + len;
	while( HTTPParseParameter( src, end, &namePtr, &nameLen, &valuePtr, &valueLen, NULL, &src ) == kNoErr )
	{
		if(      strnicmpx( namePtr, nameLen, "b" )  == 0 ) startInfo.bonjourMs		= TextToInt32( valuePtr, valueLen, 10 );
		else if( strnicmpx( namePtr, nameLen, "c" )  == 0 ) startInfo.connectMs		= TextToInt32( valuePtr, valueLen, 10 );
		else if( strnicmpx( namePtr, nameLen, "au" ) == 0 ) startInfo.authMs		= TextToInt32( valuePtr, valueLen, 10 );
		else if( strnicmpx( namePtr, nameLen, "an" ) == 0 ) startInfo.announceMs	= TextToInt32( valuePtr, valueLen, 10 );
		else if( strnicmpx( namePtr, nameLen, "sa" ) == 0 ) startInfo.setupAudioMs	= TextToInt32( valuePtr, valueLen, 10 );
		else if( strnicmpx( namePtr, nameLen, "ss" ) == 0 ) startInfo.setupScreenMs	= TextToInt32( valuePtr, valueLen, 10 );
	}
	
	// Start the session.
	
	err = AirPlayReceiverSessionStart( atCnx->session, &startInfo );
	if( AirPlayIsBusyError( err ) ) { status = kHTTPStatus_NotEnoughBandwidth; goto exit; }
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
#if( TARGET_OS_POSIX )
	err = NetworkChangeListenerCreate( &atCnx->networkChangeListener );
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	NetworkChangeListenerSetDispatchQueue( atCnx->networkChangeListener, AirPlayReceiverServerGetDispatchQueue( gAirPlayReceiverServer) );
	NetworkChangeListenerSetHandler( atCnx->networkChangeListener, _requestNetworkChangeListenerHandleEvent, inCnx );
	NetworkChangeListenerStart( atCnx->networkChangeListener );
#endif
	
	atCnx->didRecord = true;
	status = kHTTPStatus_OK;
	
exit:
	return( status );
}

//===========================================================================================================================
//	_requestProcessSetup
//===========================================================================================================================

static HTTPStatus _requestProcessSetup( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus				status;
	const char *			ptr;
	size_t					len;
	
	// Reject all requests if playback is not allowed.
	
	if( !_AirTunesServer_PlaybackAllowed() )
	{
		ats_ulog( kLogLevelNotice, "### Setup denied because playback is not allowed\n" );
		status = kHTTPStatus_NotEnoughBandwidth; // Make us look busy to the sender.
		goto exit;
	}
	
	ptr = NULL;
	len = 0;
	GetHeaderValue( request, kHTTPHeader_ContentType, &ptr, &len );
	if( MIMETypeIsPlist( ptr, len ) )
	{
		status = _requestProcessSetupPlist( inCnx, request );
		goto exit;
	}
	
	{
		dlogassert( "Bad setup URL '%.*s'", (int) request->header.urlLen, request->header.urlPtr );
		status = kHTTPStatus_BadRequest;
		goto exit;
	}
	
exit:
	return( status );
}

//===========================================================================================================================
//	_requestProcessSetupPlist
//===========================================================================================================================

static HTTPStatus _requestProcessSetupPlist( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	HTTPStatus					status;
	OSStatus					err;
	CFMutableDictionaryRef		requestParams  = NULL;
	CFDictionaryRef				responseParams = NULL;
	uint8_t						sessionUUID[ 16 ];
	char						cstr[ 64 ];
	size_t						len;
	uint64_t					u64;
	AirPlayEncryptionType		et;
	AirTunesConnection *		atCnx = (AirTunesConnection *) inCnx->userContext;
	
	ats_ulog( kAirPlayPhaseLogLevel, "Setup\n" );
	
	// If we're denying interrupts then reject if there's already an active session.
	// Otherwise, hijack any active session to start the new one (last-in-wins).
	
	if( gAirTunesDenyInterruptions )
	{
		HTTPConnectionRef activeCnx;
		
		activeCnx = _AirTunesServer_FindActiveConnection( atCnx->atServer );
		if( activeCnx && ( activeCnx != inCnx ) )
		{
			ats_ulog( kLogLevelNotice, "Denying interruption from %##a due to %##a\n", &inCnx->peerAddr, &activeCnx->peerAddr );
			status = kHTTPStatus_NotEnoughBandwidth;
			err = kNoErr;
			goto exit;
		}
	}
	_AirTunesServer_HijackConnections( atCnx->atServer, inCnx );
	
	requestParams = CFDictionaryCreateMutableWithBytes( request->bodyPtr, request->bodyOffset, &err );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	u64 = CFDictionaryGetMACAddress( requestParams, CFSTR( kAirPlayKey_DeviceID ), NULL, &err );
	if( !err ) atCnx->clientDeviceID = u64;
	
	strncpy( atCnx->ifName, inCnx->ifName, sizeof( atCnx->ifName ) );	
	
	CFDictionaryGetMACAddress( requestParams, CFSTR( kAirPlayKey_MACAddress ), atCnx->clientInterfaceMACAddress, &err );
	
	CFDictionaryGetCString( requestParams, CFSTR( kAirPlayKey_Name ), atCnx->clientName, sizeof( atCnx->clientName ), NULL );
	
	CFDictionaryGetData( requestParams, CFSTR( kAirPlayKey_SessionUUID ), sessionUUID, sizeof( sessionUUID ), &len, &err );
	if( !err )
	{
		require_action( len == sizeof( sessionUUID ), exit, err = kSizeErr; status = kHTTPStatus_BadRequest );
		atCnx->clientSessionID = ReadBig64( sessionUUID );
	}
	
	*cstr = '\0';
	CFDictionaryGetCString( requestParams, CFSTR( kAirPlayKey_SourceVersion ), cstr, sizeof( cstr ), NULL );
	if( *cstr != '\0' ) atCnx->clientVersion = TextToSourceVersion( cstr, kSizeCString );
	
	// Set up the session.
	
	if( !atCnx->session )
	{
		err = _requestCreateSession( inCnx, true );
		require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
		strlcpy( gAirPlayAudioStats.ifname, inCnx->ifName, sizeof( gAirPlayAudioStats.ifname ) );
	}
	
	et = (AirPlayEncryptionType) CFDictionaryGetInt64( requestParams, CFSTR( kAirPlayKey_EncryptionType ), &err );
	if( !err && ( et != kAirPlayEncryptionType_None ) )
	{
		uint8_t key[ 16 ], iv[ 16 ];
		
		err = _requestDecryptKey( inCnx, requestParams, et, key, iv );
		require_noerr_action( err, exit, status = kHTTPStatus_KeyManagementError );
		
		err = AirPlayReceiverSessionSetSecurityInfo( atCnx->session, key, iv );
		MemZeroSecure( key, sizeof( key ) );
		require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	}
	CFDictionaryRemoveValue( requestParams, CFSTR( kAirPlayKey_EncryptionKey ) );
	CFDictionaryRemoveValue( requestParams, CFSTR( kAirPlayKey_EncryptionIV ) );
	
	err = AirPlayReceiverSessionSetup( atCnx->session, requestParams, &responseParams );
	require_noerr_action( err, exit, status = kHTTPStatus_BadRequest );
	
	atCnx->didAnnounce		= true;
	atCnx->didAudioSetup	= true;
	atCnx->didScreenSetup	= true;
	AirPlayReceiverServerSetBoolean( gAirPlayReceiverServer, CFSTR( kAirPlayProperty_Playing ), NULL, true );
	
	status = _requestSendPlistResponse( inCnx, request, responseParams, &err );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( requestParams );
	CFReleaseNullSafe( responseParams );
	if( err ) ats_ulog( kLogLevelNotice, "### Setup session failed: %#m\n", err );
	return( status );
}

//===========================================================================================================================
//	_requestProcessTearDown
//===========================================================================================================================

static HTTPStatus _requestProcessTearDown( HTTPConnectionRef inCnx, HTTPMessageRef request )
{
	CFDictionaryRef			requestDict;
	Boolean					done = true;
	AirTunesConnection *	atCnx = (AirTunesConnection *) inCnx->userContext;
	
	requestDict = CFDictionaryCreateWithBytes( request->bodyPtr, request->bodyOffset, NULL );
	ats_ulog( kLogLevelNotice, "Teardown %?@\n", log_category_enabled( ats_ucat(), kLogLevelVerbose ), requestDict );
	
	if( atCnx->session )
	{
		AirPlayReceiverSessionTearDown( atCnx->session, requestDict, kNoErr, &done );
	}
	if( done )
	{
#if( TARGET_OS_POSIX )
		if( atCnx->networkChangeListener )
		{
			NetworkChangeListenerStop( atCnx->networkChangeListener );
			ForgetCF( &atCnx->networkChangeListener );
		}
#endif
		ForgetCF( &atCnx->session );
		AirPlayReceiverServerSetBoolean( gAirPlayReceiverServer, CFSTR( kAirPlayProperty_Playing ), NULL, false );
		gAirPlayAudioCompressionType = kAirPlayCompressionType_Undefined;
		atCnx->didAnnounce = false;
		atCnx->didAudioSetup = false;
		atCnx->didScreenSetup	= false;
		atCnx->didRecord = false;
	}
	CFReleaseNullSafe( requestDict );
	return( kHTTPStatus_OK );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_requestCreateSession
//===========================================================================================================================

static OSStatus _requestCreateSession( HTTPConnectionRef inCnx, Boolean inUseEvents )
{
	OSStatus								err;
	AirPlayReceiverSessionCreateParams		createParams;
	AirTunesConnection *					atCnx = (AirTunesConnection *) inCnx->userContext;
	
	require_action_quiet( !atCnx->session, exit, err = kNoErr );
	
	memset( &createParams, 0, sizeof( createParams ) );
	createParams.server				= gAirPlayReceiverServer;
	createParams.transportType		= inCnx->transportType;
	createParams.peerAddr			= &inCnx->peerAddr;
	createParams.clientDeviceID		= atCnx->clientDeviceID;
	createParams.clientSessionID	= atCnx->clientSessionID;
	createParams.clientVersion		= atCnx->clientVersion;
	createParams.useEvents			= inUseEvents;
	
	memcpy( createParams.clientIfMACAddr, atCnx->clientInterfaceMACAddress, sizeof( createParams.clientIfMACAddr ) );
	strncpy( createParams.ifName, atCnx->ifName, sizeof( createParams.ifName ) );
	
	err = AirPlayReceiverSessionCreate( &atCnx->session, &createParams );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_requestDecryptKey
//===========================================================================================================================

static OSStatus
	_requestDecryptKey(
		HTTPConnectionRef		inCnx,
		CFDictionaryRef			inRequestParams, 
		AirPlayEncryptionType	inType, 
		uint8_t					inKeyBuf[ 16 ], 
		uint8_t					inIVBuf[ 16 ] )
{
	OSStatus				err;
	const uint8_t *			keyPtr;
	size_t					len;
	AirTunesConnection *	atCnx = (AirTunesConnection *) inCnx->userContext;
	
	keyPtr = CFDictionaryGetData( inRequestParams, CFSTR( kAirPlayKey_EncryptionKey ), NULL, 0, &len, &err );
	require_noerr( err, exit );
	
	if( 0 ) {}
	else if( inType == kAirPlayEncryptionType_MFi_SAPv1 )
	{
		require_action( atCnx->MFiSAP, exit, err = kAuthenticationErr );
		err = APSMFiSAP_Decrypt( atCnx->MFiSAP, keyPtr, len, inKeyBuf );
		require_noerr( err, exit );
	}
	else
	{
		(void) atCnx;
		ats_ulog( kLogLevelWarning, "### Bad ET: %d\n", inType );
		err = kParamErr;
		goto exit;
	}
	
	CFDictionaryGetData( inRequestParams, CFSTR( kAirPlayKey_EncryptionIV ), inIVBuf, 16, &len, &err );
	require_noerr( err, exit );
	require_action( len == 16, exit, err = kSizeErr );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_requestSendPlistResponse
//===========================================================================================================================

static HTTPStatus _requestSendPlistResponse( HTTPConnectionRef inCnx, HTTPMessageRef request, CFPropertyListRef inPlist, OSStatus *outErr )
{
	HTTPStatus			status;
	OSStatus			err;
	const char *		httpProtocol;
	CFDataRef			data = NULL;
	const uint8_t *		ptr;
	size_t				len;
	HTTPMessageRef		response = inCnx->responseMsg;
	
	if( response->header.len == 0 )
	{
		httpProtocol = ( strnicmp_prefix( request->header.protocolPtr, request->header.protocolLen, "HTTP" ) == 0 )
			? "HTTP/1.1" : kAirTunesHTTPVersionStr;
		err = HTTPHeader_InitResponse( &response->header, httpProtocol, kHTTPStatus_OK, NULL );
		require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
		response->bodyLen = 0;
	}
	
	if( inPlist )
	{
		data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
		require_action( data, exit, err = kUnknownErr; status = kHTTPStatus_InternalServerError );
		ptr = CFDataGetBytePtr( data );
		len = (size_t) CFDataGetLength( data );
		err = HTTPMessageSetBody( response, kMIMEType_AppleBinaryPlist, ptr, len );
	}
	else
	{
		err = HTTPMessageSetBody( response, NULL, NULL, 0 );
	}
	require_noerr_action( err, exit, status = kHTTPStatus_InternalServerError );
	
	status = kHTTPStatus_OK;
	
exit:
	CFReleaseNullSafe( data );
	if( outErr ) *outErr = err;
	return( status );
}

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	_requestNetworkChangeListenerHandleEvent
//===========================================================================================================================

static void	_requestNetworkChangeListenerHandleEvent( uint32_t inEvent, void *inContext )
{
	OSStatus							err;
	HTTPConnectionRef					cnx = (HTTPConnectionRef) inContext;
	AirTunesConnection *				atCnx = (AirTunesConnection *) cnx->userContext;
	uint32_t							flags;
	Boolean								sessionDied = false;
	uint64_t							otherFlags	= 0;
	
	if( inEvent == kNetworkEvent_Changed )
	{
		err = SocketGetInterfaceInfo( cnx->sock, cnx->ifName, NULL, NULL, NULL, NULL, &flags, NULL, &otherFlags, NULL );
		if( err )
		{
			ats_ulog( kLogLevelInfo, "### Can't get interface's %s info: err = %d; killing session.\n", cnx->ifName, err );
			sessionDied = true;
			goto exit;
		}
		if( !( flags & IFF_RUNNING ) )
		{
			ats_ulog( kLogLevelInfo, "### Interface %s is not running; killing session.\n", cnx->ifName );
			sessionDied = true;
			goto exit;
		}
		if( otherFlags & kNetInterfaceFlag_Inactive )
		{
			ats_ulog( kLogLevelInfo, "### Interface %s is inactive; killing session.\n", cnx->ifName );
			sessionDied = true;
			goto exit;
		}
	}
	
exit:
	if( sessionDied )
	{
		NetworkChangeListenerStop( atCnx->networkChangeListener );
		ForgetCF( &atCnx->networkChangeListener );
		AirPlayReceiverServerControl( gAirPlayReceiverServer, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SessionDied ), atCnx->session, NULL, NULL );
	}
}
#endif
