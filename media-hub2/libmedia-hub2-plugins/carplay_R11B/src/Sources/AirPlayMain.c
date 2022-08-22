/*
	File:    	AirPlayMain.c
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

#include "AirPlayMain.h"

#include "AirPlayCommon.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayVersion.h"
#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "RandomNumberUtils.h"
#include "StringUtils.h"
#include "SystemUtils.h"
#include "ThreadUtils.h"
#include "AirPlayReceiverSession.h"

#include <mh_core.h>
#include <dev_carplay.h>

#if( TARGET_OS_POSIX )
	#include <errno.h>
	#include <syslog.h>
#endif

MHDevCarplay * carplayObject;

//===========================================================================================================================
//	Internals
//===========================================================================================================================

#if( AIRPLAY_THREADED_MAIN )
	static void *	AirPlayMainThread( void *inArg );
	
	static pthread_t		gAirPlayMainThread;
	static pthread_t *		gAirPlayMainThreadPtr = NULL;
#endif
#include <assert.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _AirPlayHandleServerControl
 *  Description:  
 * =====================================================================================
 */
static OSStatus _AirPlayHandleServerControl( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		CFDictionaryRef *			outParams, 
		void *						inContext )
{
	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inCommand, kCFStringEncodingUTF8 ));
	return kNoErr;
}		/* -----  end of static function _AirPlayHandleServerControl  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _AirPlayHandleServerSetProperty
 *  Description:  
 * =====================================================================================
 */
static OSStatus _AirPlayHandleServerSetProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		CFTypeRef					inValue, 
		void *						inContext )
{
	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));
	assert( 0 );
	return kNoErr;
}		/* -----  end of static function _AirPlayHandleServerSetProperty  ----- */

static CFTypeRef
_AirPlayHandleServerCopyProperty(
		AirPlayReceiverServerRef    inServer,
		CFStringRef                 inProperty,
		CFTypeRef                   inQualifier,
		OSStatus *                  outErr,
		void *                      inContext )
{
	CFTypeRef       value = NULL;
	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));

	if( CFEqual( inProperty, CFSTR( kAirPlayKey_Features )))
	{
		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_FirmwareRevision )))
	{
		value	=	CFSTR( "Media-Hub v2.0" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_PlaybackAllowed )))
	{
		value	=	kCFBooleanTrue;

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_BluetoothIDs )))
	{
		value	=	CFSTR( "What is the BluetoothIDs?" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_HardwareRevision )))
	{
		value	=	CFSTR( "T-Aflus-2wk" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_HIDLanguages )))
	{
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_LimitedUIElements )))
	{
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_LimitedUI )))
	{
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_Manufacturer )))
	{
		value	=	CFSTR( "Neusoft" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_NightMode )))
	{
		value	=	kCFBooleanTrue;

		* outErr	=	kNoErr;
	}
	else
	{
		* outErr	=	kNotHandledErr;
//		assert( 0 );
	}

	return( value );
}

//===========================================================================================================================
//  _AirPlayHandleSessionInitialized
//===========================================================================================================================

static OSStatus _AirPlayHandleSessionInitialized( AirPlayReceiverSessionRef inSession, void *inContext )
{
	printf( "\033[1;32;40mAirPlay session initialized\033[0m\n" );

	return kNoErr;
}

//===========================================================================================================================
//  _AirPlayHandleSessionFinalized
//===========================================================================================================================

static void _AirPlayHandleSessionFinalized( AirPlayReceiverSessionRef inSession, void *inContext )
{
	printf( "\033[1;32;40mAirPlay session ended\033[0m\n" );
}

//===========================================================================================================================
//  _AirPlayHandleSessionSetProperty
//===========================================================================================================================

static OSStatus
_AirPlayHandleSessionSetProperty(
		AirPlayReceiverSessionRef   inSession,
		CFStringRef                 inProperty,
		CFTypeRef                   inQualifier,
		CFTypeRef                   inValue,
		void *                      inContext )
{
	CFTypeRef       value = NULL;
	OSStatus        err;

	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));

		assert( 0 );
	if( CFEqual( inProperty, CFSTR( kAirPlayProperty_Modes )))
	{
		
	}
	else
	{
		assert( 0 );
	}

	return( kNoErr );
}


//===========================================================================================================================
//  _AirPlayHandleSessionCopyProperty
//===========================================================================================================================

static CFTypeRef
_AirPlayHandleSessionCopyProperty(
		AirPlayReceiverSessionRef   inSession,
		CFStringRef                 inProperty,
		CFTypeRef                   inQualifier,
		OSStatus *                  outErr,
		void *                      inContext )
{
	CFTypeRef       value = NULL;
	OSStatus        err;
	Boolean	_done;

	if( CFEqual( inProperty, CFSTR( kAirPlayProperty_Modes )))
	{
		printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));
		value	=	carplayObject->modes;
	}
	else
	{
		assert( 0 );
	}

	return( value );
}

//===========================================================================================================================
//  _AirPlayHandleModesChanged
//===========================================================================================================================

static void
_AirPlayHandleModesChanged(
		AirPlayReceiverSessionRef   inSession,
		const AirPlayModeState *    inState,
		void *                      inContext )
{
//	printf( "\033[1;32;40mModes changed: screen [ %s ], mainAudio [ %s ], speech [ %s %s ], phone [ %s ], turns [ %s ]\033[0m\n",
//			AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ),
//			AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ),
//			AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ) );

	g_signal_emit_by_name( carplayObject, "modes_changed", AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ),
			AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ),
			AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ) );
}

//===========================================================================================================================
//  _AirPlayHandleRequestUI
//===========================================================================================================================

static void _AirPlayHandleRequestUI( AirPlayReceiverSessionRef inSession, void *inContext )
{
	printf( "\033[1;32;40mRequest accessory UI\033[0m\n" );

	g_signal_emit_by_name( carplayObject, "request_ui", NULL );
}

//===========================================================================================================================
//  _AirPlayHandleDuckAudio
//===========================================================================================================================

static void _AirPlayHandleDuckAudio(
		AirPlayReceiverSessionRef	inSession,
		double						inDurationSecs,
		double						inVolume,
		void *						inContext )
{
	printf( "\033[1;32;40mDuck Audio\033[0m\n" );
	assert( 0 );
}

//===========================================================================================================================
//  _AirPlayHandleUnduckAudio
//===========================================================================================================================

static void _AirPlayHandleUnduckAudio(
		AirPlayReceiverSessionRef	inSession,
		double						inDurationSecs,
		void *						inContext )
{
	printf( "\033[1;32;40mUnduck Audio\033[0m\n" );
	assert( 0 );
}

//===========================================================================================================================
//  _AirPlayHandleSessionControl
//===========================================================================================================================

	static OSStatus
_AirPlayHandleSessionControl(
		AirPlayReceiverSessionRef   inSession,
		CFStringRef                 inCommand,
		CFTypeRef                   inQualifier,
		CFDictionaryRef             inParams,
		CFDictionaryRef *           outParams,
		void *                      inContext )
{
	OSStatus err;

	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inCommand, kCFStringEncodingUTF8 ));

	if( CFEqual( inCommand, CFSTR( kAirPlayCommand_DisableBluetooth )))
	{
		g_signal_emit_by_name( carplayObject, "disable_bluetooth", "test id", NULL );
	}
	else
	{
		assert( 0 );
	}

	return( kNoErr );
}

static void
_AirPlayHandleSessionCreated(
		AirPlayReceiverServerRef    inServer,
		AirPlayReceiverSessionRef   inSession,
		void *                      inContext )
{
	AirPlayReceiverSessionDelegate      delegate;

	(void) inServer;
	(void) inContext;

	printf( "\033[1;32;40mAirPlay session created\033[0m\n" );

	// Register ourself as a delegate to receive session-level events, such as modes changes.

	AirPlayReceiverSessionDelegateInit( &delegate );

	delegate.initialize_f	=	_AirPlayHandleSessionInitialized;
	delegate.finalize_f     =	_AirPlayHandleSessionFinalized;
	delegate.control_f      =	_AirPlayHandleSessionControl;
	delegate.copyProperty_f =	_AirPlayHandleSessionCopyProperty;
	delegate.setProperty_f	=	_AirPlayHandleSessionSetProperty;
	delegate.modesChanged_f =	_AirPlayHandleModesChanged;
	delegate.requestUI_f    =	_AirPlayHandleRequestUI;
	delegate.duckAudio_f	=	_AirPlayHandleDuckAudio;
	delegate.unduckAudio_f	=	_AirPlayHandleUnduckAudio;

	carplayObject	=	g_object_new( MH_TYPE_DEV_CARPLAY, "io-name", "carplay", NULL );

	carplayObject->inSession	=	inSession;
	carplayObject->inContext	=	inContext;

	mh_core_attach_dev( MH_DEV( carplayObject ));

	AirPlayReceiverSessionSetDelegate( inSession, &delegate );
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _AirPlayHandleSessionFailed
 *  Description:  
 * =====================================================================================
 */
static void _AirPlayHandleSessionFailed(
		AirPlayReceiverServerRef	inServer,
		OSStatus					inReason,
		void *						inContext )
{
	printf( "\033[1;32;40m%s %d\033[0m\n", __func__, inReason );
	assert( 0 );
}		/* -----  end of static function _AirPlayHandleSessionFailed  ----- */

//===========================================================================================================================
//	main
//===========================================================================================================================

int	AirPlayMain( int argc, const char **argv )
{
	OSStatus						err;
	AirPlayReceiverServerRef		server = NULL;
	int								argi;
	const char *					arg;
	const char *					ifname = NULL;
	AirPlayReceiverServerDelegate       delegate;
	
	err = AirPlayReceiverServerCreate( &server );
	require_noerr( err, exit );

	AirPlayReceiverServerDelegateInit( &delegate );

	delegate.control_f			=	_AirPlayHandleServerControl;
	delegate.copyProperty_f		=	_AirPlayHandleServerCopyProperty;
	delegate.setProperty_f		=	_AirPlayHandleServerSetProperty;
	delegate.sessionCreated_f	=	_AirPlayHandleSessionCreated;
	delegate.sessionFailed_f	=	_AirPlayHandleSessionFailed;

	AirPlayReceiverServerSetDelegate( server, &delegate );
	
	if( ifname ) AirPlayReceiverServerSetCString( server, CFSTR( kAirPlayProperty_InterfaceName ), NULL, ifname, kSizeCString );
	
	err = AirPlayReceiverServerStart( server );
	require_noerr( err, exit );
	
	CFRunLoopRun();
	
	AirPlayReceiverServerStop( server );
	
exit:
	CFReleaseNullSafe( server );
	return( err ? 1 : 0 );
}

//===========================================================================================================================
//	AirPlayStartMain
//===========================================================================================================================

OSStatus	AirPlayStartMain( void )
{
	OSStatus		err;

	require_action( gAirPlayMainThreadPtr == NULL, exit, err = kAlreadyInUseErr );
	
	err = pthread_create( &gAirPlayMainThread, NULL, AirPlayMainThread, NULL );
	require_noerr( err, exit );
	gAirPlayMainThreadPtr = &gAirPlayMainThread;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayStopMain
//===========================================================================================================================

OSStatus	AirPlayStopMain( void )
{
	OSStatus		err;
	
	require_action( gAirPlayMainThreadPtr, exit, err = kNotInUseErr );
	
	CFRunLoopStop( CFRunLoopGetMain() );
	
	err = pthread_join( gAirPlayMainThread, NULL );
	check_noerr( err );
	gAirPlayMainThreadPtr = NULL;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayMainThread
//===========================================================================================================================

static void *	AirPlayMainThread( void *inArg )
{
	(void) inArg;
	
	pthread_setname_np_compat( "AirPlayMain" );
	AirPlayMain( 0, NULL );
	return( NULL );
}
