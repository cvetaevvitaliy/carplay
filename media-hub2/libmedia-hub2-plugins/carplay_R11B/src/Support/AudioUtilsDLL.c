/*
	File:    	AudioUtilsDLL.c
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
	
	AudioStream adapter that delegates functionality to a DLL.
	
	This defaults to loading the DLL from "libAudioStream.so".
	These can be overridden in the makefile with the following:
	
	CFLAGS += -DAUDIO_STREAM_DLL_PATH\"/some/other/path/libAudioStream.so\"
*/

#include "AudioUtils.h"

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#include CF_HEADER
#include CF_RUNTIME_HEADER
#include LIBDISPATCH_HEADER

//===========================================================================================================================
//	AudioUtils DLL
//===========================================================================================================================

#if( defined( AUDIO_STREAM_DLL_PATH ) )
	#define kAudioStreamDLLPath					AUDIO_STREAM_DLL_PATH
#else
	#define kAudioStreamDLLPath					"libAudioStream.so"
#endif

#define FIND_SYM_HANDLE( HANDLE, NAME )		(NAME ## _f)(uintptr_t) dlsym( HANDLE, # NAME );

static void	_AudioUtilsDLLInitialize( void *inContext );

static dispatch_once_t	gAudioUtilsDLLInitOnce	= 0;

//===========================================================================================================================
//	AudioStream
//===========================================================================================================================

struct AudioStreamPrivate
{
	CFRuntimeBase							base;		// CF type info. Must be first.
	void *									context;	// Context for DLLs.
};

static void		_AudioStreamGetTypeID( void *inContext );
static void		_AudioStreamFinalize( CFTypeRef inCF );

static const CFRuntimeClass		kAudioStreamClass = 
{
	0,						// version
	"AudioStream",			// className
	NULL,					// init
	NULL,					// copy
	_AudioStreamFinalize,	// finalize
	NULL,					// equal -- NULL means pointer equality.
	NULL,					// hash  -- NULL means pointer hash.
	NULL,					// copyFormattingDesc
	NULL,					// copyDebugDesc
	NULL,					// reclaim
	NULL					// refcount
};

static dispatch_once_t					gAudioStreamInitOnce			= 0;
static CFTypeID							gAudioStreamTypeID				= _kCFRuntimeNotATypeID;

static AudioStreamInitialize_f			gAudioStreamInitialize_f		= NULL;
static AudioStreamFinalize_f			gAudioStreamFinalize_f			= NULL;
static AudioStreamSetInputCallback_f	gAudioStreamSetInputCallback_f	= NULL;
static AudioStreamSetOutputCallback_f	gAudioStreamSetOutputCallback_f	= NULL;
static _AudioStreamCopyProperty_f		gAudioStreamCopyProperty_f		= NULL;
static _AudioStreamSetProperty_f		gAudioStreamSetProperty_f		= NULL;
static AudioStreamGetVolume_f			gAudioStreamGetVolume_f			= NULL;
static AudioStreamSetVolume_f			gAudioStreamSetVolume_f			= NULL;
static AudioStreamRampVolume_f			gAudioStreamRampVolume_f		= NULL;
static AudioStreamPrepare_f				gAudioStreamPrepare_f			= NULL;
static AudioStreamStart_f				gAudioStreamStart_f				= NULL;
static AudioStreamStop_f				gAudioStreamStop_f				= NULL;

//===========================================================================================================================
//	AudioSession
//===========================================================================================================================

static AudioSessionSetEventHandler_f			gAudioSessionSetEventHandler_f				= NULL;
static AudioSessionCopyLatencies_f				gAudioSessionCopyLatencies_f				= NULL;
static AudioSessionEnsureSetup_f				gAudioSessionEnsureSetup_f					= NULL;
static AudioSessionEnsureTornDown_f				gAudioSessionEnsureTornDown_f				= NULL;
static AudioSessionGetSupportedInputFormats_f	gAudioSessionGetSupportedInputFormats_f		= NULL;
static AudioSessionGetSupportedOutputFormats_f	gAudioSessionGetSupportedOutputFormats_f	= NULL;

//===========================================================================================================================
//	AudioStreamGetTypeID
//===========================================================================================================================

CFTypeID	AudioStreamGetTypeID( void )
{
	dispatch_once_f( &gAudioStreamInitOnce, NULL, _AudioStreamGetTypeID );
	return( gAudioStreamTypeID );
}

static void _AudioStreamGetTypeID( void *inContext )
{
	(void) inContext;
	
	gAudioStreamTypeID = _CFRuntimeRegisterClass( &kAudioStreamClass );
	check( gAudioStreamTypeID != _kCFRuntimeNotATypeID );
}

//===========================================================================================================================
//	AudioStreamCreate
//===========================================================================================================================

OSStatus	AudioStreamCreate( CFDictionaryRef inOptions, AudioStreamRef *outStream )
{
	OSStatus			err;
	AudioStreamRef		me;
	size_t				extraLen;
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (AudioStreamRef) _CFRuntimeCreateInstance( NULL, AudioStreamGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );
	
	if( gAudioStreamInitialize_f )
	{
		err = gAudioStreamInitialize_f( me, inOptions );
		require_noerr( err, exit );
	}
	
	*outStream = me;
	me = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( me );
	return( err );
}

//===========================================================================================================================
//	_AudioStreamFinalize
//===========================================================================================================================

static void	_AudioStreamFinalize( CFTypeRef inCF )
{
	AudioStreamRef const		me = (AudioStreamRef) inCF;
	
	if( gAudioStreamFinalize_f ) gAudioStreamFinalize_f( me );
}

//===========================================================================================================================
//	AudioStreamGetContext
//===========================================================================================================================

void *	AudioStreamGetContext( AudioStreamRef me )
{
	return( me->context );
}

//===========================================================================================================================
//	AudioStreamSetContext
//===========================================================================================================================

void	AudioStreamSetContext( AudioStreamRef me, void *inContext )
{
	me->context = inContext;
}

//===========================================================================================================================
//	AudioStreamSetInputCallback
//===========================================================================================================================

void	AudioStreamSetInputCallback( AudioStreamRef me, AudioStreamInputCallback_f inFunc, void *inContext )
{
	if( gAudioStreamSetInputCallback_f ) gAudioStreamSetInputCallback_f( me, inFunc, inContext );
}

//===========================================================================================================================
//	AudioStreamSetOutputCallback
//===========================================================================================================================

void	AudioStreamSetOutputCallback( AudioStreamRef me, AudioStreamOutputCallback_f inFunc, void *inContext )
{
	if( gAudioStreamSetOutputCallback_f ) gAudioStreamSetOutputCallback_f( me, inFunc, inContext );
}

//===========================================================================================================================
//	_AudioStreamCopyProperty
//===========================================================================================================================

CFTypeRef
	_AudioStreamCopyProperty(
		CFTypeRef		inObject, // Must be a AudioStreamRef
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr )
{
	if( gAudioStreamCopyProperty_f ) return( gAudioStreamCopyProperty_f( inObject, inFlags, inProperty, inQualifier, outErr ) );
	if( outErr ) *outErr = kUnsupportedErr;
	return( NULL );
}

//===========================================================================================================================
//	_AudioStreamSetProperty
//===========================================================================================================================

OSStatus
	_AudioStreamSetProperty(
		CFTypeRef		inObject, // Must be a AudioStreamRef
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue )
{
	return( gAudioStreamSetProperty_f ? gAudioStreamSetProperty_f( inObject, inFlags, inProperty, inQualifier, inValue ) : kUnsupportedErr );
}

//===========================================================================================================================
//	AudioStreamGetVolume
//===========================================================================================================================

double	AudioStreamGetVolume( AudioStreamRef me, OSStatus *outErr )
{
	if( gAudioStreamGetVolume_f ) return( gAudioStreamGetVolume_f( me, outErr ) );
	if( outErr ) *outErr = kUnsupportedErr;
	return( 0 );
}

//===========================================================================================================================
//	AudioStreamSetVolume
//===========================================================================================================================

OSStatus	AudioStreamSetVolume( AudioStreamRef me, double inVolume )
{
	return( gAudioStreamSetVolume_f ?gAudioStreamSetVolume_f( me, inVolume ) : kUnsupportedErr );
}

//===========================================================================================================================
//	AudioStreamRampVolume
//===========================================================================================================================

OSStatus
	AudioStreamRampVolume( 
		AudioStreamRef		me, 
		double				inFinalVolume, 
		double				inDurationSecs, 
		dispatch_queue_t	inQueue )
{
	return( gAudioStreamRampVolume_f ? gAudioStreamRampVolume_f( me, inFinalVolume, inDurationSecs, inQueue ) : kUnsupportedErr );
}

//===========================================================================================================================
//	AudioStreamPrepare
//===========================================================================================================================

OSStatus	AudioStreamPrepare( AudioStreamRef me )
{
	return( gAudioStreamPrepare_f ? gAudioStreamPrepare_f( me ) : kUnsupportedErr );
}

//===========================================================================================================================
//	AudioStreamStart
//===========================================================================================================================

OSStatus	AudioStreamStart( AudioStreamRef me )
{
	return( gAudioStreamStart_f ? gAudioStreamStart_f( me ) : kUnsupportedErr );
}

//===========================================================================================================================
//	AudioStreamStop
//===========================================================================================================================

void	AudioStreamStop( AudioStreamRef me, Boolean inDrain )
{
	return( gAudioStreamStop_f ? gAudioStreamStop_f( me, inDrain ) : kUnsupportedErr );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AudioSessionSetEventHandler
//===========================================================================================================================

void AudioSessionSetEventHandler( AudioSessionEventHandler_f inHandler, void *inContext )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );

	if( gAudioSessionSetEventHandler_f ) gAudioSessionSetEventHandler_f( inHandler, inContext );
}

//===========================================================================================================================
//	AudioSessionCopyLatencies
//===========================================================================================================================

CFArrayRef	AudioSessionCopyLatencies( OSStatus *outErr )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );
	
	if( gAudioSessionCopyLatencies_f ) return gAudioSessionCopyLatencies_f( outErr );
	
	return 0;
}

//===========================================================================================================================
//	AudioSessionEnsureSetup
//===========================================================================================================================

void
	AudioSessionEnsureSetup(
		Boolean		inHasInput,
		uint32_t	inPreferredSystemSampleRate,
		uint32_t	inPreferredSystemBufferSizeMicros )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );

	if( gAudioSessionEnsureSetup_f ) gAudioSessionEnsureSetup_f( inHasInput, inPreferredSystemSampleRate, inPreferredSystemBufferSizeMicros );
}

//===========================================================================================================================
//	AudioSessionEnsureTornDown
//===========================================================================================================================

void AudioSessionEnsureTornDown( void )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );

	if( gAudioSessionEnsureTornDown_f ) gAudioSessionEnsureTornDown_f();
}

//===========================================================================================================================
//	AudioSessionGetSupportedInputFormats
//===========================================================================================================================

AudioSessionAudioFormat	AudioSessionGetSupportedInputFormats( AudioStreamType inStreamType )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );

	if( gAudioSessionGetSupportedInputFormats_f ) return gAudioSessionGetSupportedInputFormats_f( inStreamType );
	
	return 0;
}

//===========================================================================================================================
//	AudioSessionGetSupportedOutputFormats
//===========================================================================================================================

AudioSessionAudioFormat	AudioSessionGetSupportedOutputFormats( AudioStreamType inStreamType )
{
	dispatch_once_f( &gAudioUtilsDLLInitOnce, NULL, _AudioUtilsDLLInitialize );

	if( gAudioSessionGetSupportedOutputFormats_f ) return gAudioSessionGetSupportedOutputFormats_f( inStreamType );
	
	return 0;
}

//===========================================================================================================================
//	_AudioUtilsDLLInitialize
//===========================================================================================================================

static void _AudioUtilsDLLInitialize( void *inContext )
{
	void *		dllHandle;

	(void) inContext;
	
	dllHandle = dlopen( kAudioStreamDLLPath, RTLD_LAZY | RTLD_LOCAL );
	if( dllHandle )
	{
		gAudioSessionSetEventHandler_f				= FIND_SYM_HANDLE( dllHandle, AudioSessionSetEventHandler );
		gAudioSessionCopyLatencies_f				= FIND_SYM_HANDLE( dllHandle, AudioSessionCopyLatencies );
		gAudioSessionEnsureSetup_f					= FIND_SYM_HANDLE( dllHandle, AudioSessionEnsureSetup );
		gAudioSessionEnsureTornDown_f				= FIND_SYM_HANDLE( dllHandle, AudioSessionEnsureTornDown );
		gAudioSessionGetSupportedInputFormats_f		= FIND_SYM_HANDLE( dllHandle, AudioSessionGetSupportedInputFormats );
		gAudioSessionGetSupportedOutputFormats_f	= FIND_SYM_HANDLE( dllHandle, AudioSessionGetSupportedOutputFormats );
		
		gAudioStreamInitialize_f					= FIND_SYM_HANDLE( dllHandle, AudioStreamInitialize );
		gAudioStreamFinalize_f						= FIND_SYM_HANDLE( dllHandle, AudioStreamFinalize );
		gAudioStreamSetInputCallback_f				= FIND_SYM_HANDLE( dllHandle, AudioStreamSetInputCallback );
		gAudioStreamSetOutputCallback_f				= FIND_SYM_HANDLE( dllHandle, AudioStreamSetOutputCallback );
		gAudioStreamCopyProperty_f					= FIND_SYM_HANDLE( dllHandle, _AudioStreamCopyProperty );
		gAudioStreamSetProperty_f					= FIND_SYM_HANDLE( dllHandle, _AudioStreamSetProperty );
		gAudioStreamGetVolume_f						= FIND_SYM_HANDLE( dllHandle, AudioStreamGetVolume );
		gAudioStreamSetVolume_f						= FIND_SYM_HANDLE( dllHandle, AudioStreamSetVolume );
		gAudioStreamRampVolume_f					= FIND_SYM_HANDLE( dllHandle, AudioStreamRampVolume );
		gAudioStreamPrepare_f						= FIND_SYM_HANDLE( dllHandle, AudioStreamPrepare );
		gAudioStreamStart_f							= FIND_SYM_HANDLE( dllHandle, AudioStreamStart );
		gAudioStreamStop_f							= FIND_SYM_HANDLE( dllHandle, AudioStreamStop );
	}
}

#if 0
#pragma mark -
#endif

