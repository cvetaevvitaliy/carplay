/*
	File:    	AudioUtilsStub.c
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

#include "AudioUtils.h"

#include <errno.h>
#include <stdlib.h>

#include <gst/gst.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#include <TickUtils.h>
#include <mh_carplay.h>

#include CF_HEADER
#include LIBDISPATCH_HEADER

#if( !AUDIO_STREAM_DLL )
	#include CF_RUNTIME_HEADER
#endif

//===========================================================================================================================
//	AudioStream
//===========================================================================================================================

#if( AUDIO_STREAM_DLL )
typedef struct AudioStreamImp *			AudioStreamImpRef;
struct AudioStreamImp
#else
typedef struct AudioStreamPrivate *		AudioStreamImpRef;
struct AudioStreamPrivate
#endif
{
#if( !AUDIO_STREAM_DLL )
	CFRuntimeBase					base;					// CF type info. Must be first.
#endif
	Boolean							prepared;				// True if AudioStreamPrepare has been called (and stop hasn't yet).
	
	AudioStreamInputCallback_f		inputCallbackPtr;		// Function to call to write input audio.
	void *							inputCallbackCtx;		// Context to pass to audio input callback function.
	AudioStreamOutputCallback_f		outputCallbackPtr;		// Function to call to read audio to output.
	void *							outputCallbackCtx;		// Context to pass to audio output callback function.
	Boolean							input;					// Enable audio input.
	AudioStreamBasicDescription		format;					// Format of the audio data.
	uint32_t						preferredLatencyMics;	// Max latency the app can tolerate.
	uint32_t						streamType;				// AirPlay Stream type (e.g. main, alt).

	GstElement * pipeline, * src;
	uint64_t clock, running;
	GMainLoop * mainloop;
	GMainContext * context;
	GSource * source;
	uint32_t count;
	GThread * thread;
	
	void * sc;
	gboolean inputing;
};

#if( AUDIO_STREAM_DLL )
	#define _AudioStreamGetImp( STREAM )		( (AudioStreamImpRef) AudioStreamGetContext( (STREAM) ) )
#else
	#define _AudioStreamGetImp( STREAM )		(STREAM)
#endif

#if( !AUDIO_STREAM_DLL )
	static void		_AudioStreamGetTypeID( void *inContext );
	static void		_AudioStreamFinalize( CFTypeRef inCF );
#endif

#if( !AUDIO_STREAM_DLL )
static dispatch_once_t			gAudioStreamInitOnce = 0;
static CFTypeID					gAudioStreamTypeID = _kCFRuntimeNotATypeID;
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
#endif

//===========================================================================================================================
//	Logging
//===========================================================================================================================
ulog_define( AudioStream, kLogLevelTrace, kLogFlags_Default, "AudioStream", NULL );
#define as_dlog( LEVEL, ... )		dlogc( &log_category_from_name( AudioStream ), (LEVEL), __VA_ARGS__ )
#define as_ulog( LEVEL, ... )		ulog( &log_category_from_name( AudioStream ), (LEVEL), __VA_ARGS__ )

#include <assert.h>
#if( !AUDIO_STREAM_DLL )
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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _enoughtData
 *  Description:  
 * =====================================================================================
 */
static void _enoughData( GstElement * src, gpointer user_data )
{
	g_message( "audio %s", __func__ );
}		/* -----  end of static function _enoughtData  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  setupSrc
 *  Description:  
 * =====================================================================================
 */
static void setupSrc( GstElement * object, GstElement * arg0, gpointer user_data )
{
	AudioStreamRef const		me = (AudioStreamRef) user_data;
	GstCaps * _caps;

	g_object_set( arg0, "stream-type", 0, "is-live", TRUE, NULL );

	me->src	=	arg0;

	_caps	=   gst_caps_new_simple( "audio/x-raw-int",
			"endianness", G_TYPE_INT, 1234,
			"signed", G_TYPE_BOOLEAN, TRUE,
			"width", G_TYPE_INT, 16,
			"depth", G_TYPE_INT, 16,
			"rate", G_TYPE_INT, me->format.mSampleRate,
			"channels", G_TYPE_INT, me->format.mChannelsPerFrame,
			NULL );

	g_object_set( arg0, "caps", _caps, NULL );

	gst_caps_unref( _caps );

	g_signal_connect( arg0, "enough-data", G_CALLBACK( _enoughData ), user_data );

}		/* -----  end of static function setupSrc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  pcm_output
 *  Description:  
 * =====================================================================================
 */
static gpointer pcm_output( gpointer user_data )
{
	AudioStreamRef const		me = (AudioStreamRef) user_data;

	g_main_loop_run( me->mainloop );

	return 0;
}		/* -----  end of static function pcm_output  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  pushDispatch
 *  Description:  
 * =====================================================================================
 */
static gboolean pushDispatch( GSource * source, GSourceFunc callback, gpointer user_data )
{
	AudioStreamRef const		me = (AudioStreamRef) user_data;
	GstBuffer * _buf;
	GstFlowReturn _ret;
//	static FILE * _fp	=	NULL;
//
//	if( _fp == NULL )
//		_fp	=	fopen( "/tmp/a.pcm", "w" );

	_buf	=	gst_buffer_new_and_alloc( me->format.mSampleRate * me->format.mBytesPerFrame / 10 );

	me->outputCallbackPtr( me->count, me->clock, GST_BUFFER_DATA( _buf ), 
			me->format.mSampleRate * me->format.mBytesPerFrame / 10, me->outputCallbackCtx );
	GST_BUFFER_PTS( _buf )	=	me->running;

//	fwrite( GST_BUFFER_DATA( _buf ), me->format.mSampleRate * me->format.mBytesPerFrame / 10, 1, _fp );
//	fflush( _fp );

	g_signal_emit_by_name( me->src, "push-buffer", _buf, &_ret );

	gst_buffer_unref( _buf );

	me->count	+=	me->format.mSampleRate / 10;
	me->clock	+=	100000000;
	me->running	+=	100000000;

	g_source_set_ready_time( source, me->clock / 1000 );

	return G_SOURCE_CONTINUE;
}		/* -----  end of static function pushDispatch  ----- */

static GSourceFuncs pushFuncs	=	
{
	.dispatch	=	pushDispatch
};

//===========================================================================================================================
//	AudioStreamCreate
//===========================================================================================================================

OSStatus	AudioStreamCreate( CFDictionaryRef inOptions, AudioStreamRef *outStream )
{
	OSStatus			err;
	AudioStreamRef		me;
	size_t				extraLen;
	
	(void) inOptions;
	
	extraLen = sizeof( *me ) - sizeof( me->base );
	me = (AudioStreamRef) _CFRuntimeCreateInstance( NULL, AudioStreamGetTypeID(), (CFIndex) extraLen, NULL );
	require_action( me, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) me ) + sizeof( me->base ), 0, extraLen );
	
	// $$$ TODO: Other initialization goes here.
	// This function is only called when AudioUtils is compiled into the AirPlay library.

	if( me->pipeline != NULL ) assert( 0 );

	me->pipeline	=	gst_element_factory_make( "playbin2", "apipeline" );
	g_signal_connect( me->pipeline, "source-setup", G_CALLBACK( setupSrc ), me );
	g_object_set( me->pipeline, "uri", "appsrc://", NULL );

	me->context	=	g_main_context_new();
	me->mainloop	=	g_main_loop_new( me->context, FALSE );

	me->thread	=	g_thread_new( "pcm_output", pcm_output, me );

	me->source	=	g_source_new( &pushFuncs, sizeof( GSource ));
	g_source_set_callback( me->source, NULL, me, NULL );
	g_source_set_ready_time( me->source, -1 );
	g_source_attach( me->source, me->context );

	g_source_unref( me->source );

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
	
	// $$$ TODO: Last chance to free any resources allocated by this object.
	// This function is called when AudioUtils is compiled into the AirPlay library, when the retain count of an AudioStream 
	// object goes to zero.
	if( me->input )
	{
	}

	g_main_loop_quit( me->mainloop );

	g_main_loop_unref( me->mainloop );
	g_main_context_unref( me->context );

	g_thread_join( me->thread );

	g_thread_unref( me->thread );
	gst_element_set_state( me->pipeline, GST_STATE_NULL );
}
#endif // !AUDIO_STREAM_DLL

#if( AUDIO_STREAM_DLL )
//===========================================================================================================================
//	AudioStreamInitialize
//===========================================================================================================================

OSStatus	AudioStreamInitialize( AudioStreamRef inStream, CFDictionaryRef inOptions )
{
	OSStatus				err;
	AudioStreamImpRef		me;
	
	assert( 0 );
	(void) inOptions;
	
	require_action( AudioStreamGetContext( inStream ) == NULL, exit, err = kAlreadyInitializedErr );
	
	me = (AudioStreamImpRef) calloc( 1, sizeof( *me ) );
	require_action( me, exit, err = kNoMemoryErr );
	
	// $$$ TODO: Other initialization goes here.
	// This function is called (instead of AudioStreamCreate()) when AudioUtils is built as a standalone shared object
	// that is loaded dynamically by AirPlay at runtime, so the initialization code should look very similar
	// to that in AudioStreamCreate().
	
	AudioStreamSetContext( inStream, me );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AudioStreamFinalize
//===========================================================================================================================

void	AudioStreamFinalize( AudioStreamRef inStream )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	
	if( !me ) return;
	
	// $$$ TODO: Last chance to free any resources allocated by this object.
	// This function is called (instead of _AudioStreamFinalize()) when AudioUtils is built as a standalone shared object
	// that is loaded dynamically by AirPlay at runtime, so the finalization code should look very similar to that in
	// _AudioStreamFinalize().
	// It is automatically invoked, when the retain count of an AudioStream object goes to zero.

	free( me );
	AudioStreamSetContext( inStream, NULL );
}
#endif

//===========================================================================================================================
//	AudioStreamSetInputCallback
//===========================================================================================================================

void	AudioStreamSetInputCallback( AudioStreamRef inStream, AudioStreamInputCallback_f inFunc, void *inContext )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	
	me->inputCallbackPtr = inFunc;
	me->inputCallbackCtx = inContext;
}

//===========================================================================================================================
//	AudioStreamSetOutputCallback
//===========================================================================================================================

void	AudioStreamSetOutputCallback( AudioStreamRef inStream, AudioStreamOutputCallback_f inFunc, void *inContext )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	
	me->outputCallbackPtr = inFunc;
	me->outputCallbackCtx = inContext;
}

//===========================================================================================================================
//	_AudioStreamCopyProperty
//===========================================================================================================================

CFTypeRef
	_AudioStreamCopyProperty( 
		CFTypeRef		inObject,
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		OSStatus *		outErr )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( (AudioStreamRef) inObject );
	OSStatus					err;
	CFTypeRef					value = NULL;
	
	assert( 0 );
	(void) inFlags;
	(void) inQualifier;
	
	if( 0 ) {}
	
	// AudioType
	else if( CFEqual( inProperty, kAudioStreamProperty_AudioType ) )
	{
		// $$$ TODO: Return the current audio type.
	}
	
	// Format

	else if( CFEqual( inProperty, kAudioStreamProperty_Format ) )
	{
		value = CFDataCreate( NULL, (const uint8_t *) &me->format, sizeof( me->format ) );
		require_action( value, exit, err = kNoMemoryErr );
	}
	
	// Input

	else if( CFEqual( inProperty, kAudioStreamProperty_Input ) )
	{
		value = me->input ? kCFBooleanTrue : kCFBooleanFalse;
		CFRetain( value );
	}
	
	// PreferredLatency
	
	else if( CFEqual( inProperty, kAudioStreamProperty_PreferredLatency ) )
	{
		value = CFNumberCreateInt64( me->preferredLatencyMics );
		require_action( value, exit, err = kNoMemoryErr );
	}
	
	// StreamType
	
	else if( CFEqual( inProperty, kAudioStreamProperty_StreamType ) )
	{
		value = CFNumberCreateInt64( me->streamType );
		require_action( value, exit, err = kNoMemoryErr );
	}
	
	// ThreadName
	
	else if( CFEqual( inProperty, kAudioStreamProperty_ThreadName ) )
	{
		// $$$ TODO: If your implementation uses a helper thread, return its name here.
	}
	
	// ThreadPriority
	
	else if( CFEqual( inProperty, kAudioStreamProperty_ThreadPriority ) )
	{
		// $$$ TODO: If your implementation uses a helper thread, return its priority here.
	}
	
	// Other
	
	else
	{
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_AudioStreamSetProperty
//===========================================================================================================================

OSStatus
	_AudioStreamSetProperty( 
		CFTypeRef		inObject,
		CFObjectFlags	inFlags, 
		CFStringRef		inProperty, 
		CFTypeRef		inQualifier, 
		CFTypeRef		inValue )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( (AudioStreamRef) inObject );
	OSStatus					err;
	
	(void) inFlags;
	(void) inQualifier;
	
	// Properties may only be set before AudioStreamPrepare is called.
	
	require_action( !me->prepared, exit, err = kStateErr );

	if( 0 ) {}
	
	// AudioType

	else if( CFEqual( inProperty, kAudioStreamProperty_AudioType ) )
	{
		// $$$ TODO: Use the audio type to enable certain types of audio processing.
		// For example, if the audio type is "telephony", echo cancellation should be enabled;
		// if the audio type is "speech recognition", non-linear processing algorithms should be disabled.
	}
	
	// Format

	else if( CFEqual( inProperty, kAudioStreamProperty_Format ) )
	{
		CFGetData( inValue, &me->format, sizeof( me->format ), NULL, &err );
		require_noerr( err, exit );
	}
	
	// Input

	else if( CFEqual( inProperty, kAudioStreamProperty_Input ) )
	{
		me->input = CFGetBoolean( inValue, NULL );
	}
	
	// PreferredLatency
	
	else if( CFEqual( inProperty, kAudioStreamProperty_PreferredLatency ) )
	{
		me->preferredLatencyMics = (uint32_t) CFGetInt64( inValue, &err );
		require_noerr( err, exit );
	}
	
	// StreamType
	
	else if( CFEqual( inProperty, kAudioStreamProperty_StreamType ) )
	{
		me->streamType = (uint32_t) CFGetInt64( inValue, &err );
		require_noerr( err, exit );
	}
	
	// ThreadName
	
	else if( CFEqual( inProperty, kAudioStreamProperty_ThreadName ) )
	{
		// $$$ TODO: If your implementation uses a helper thread, set the name of the thread to the string passed in
		// to this property.  See SetThreadName().
	}
	
	// ThreadPriority
	
	else if( CFEqual( inProperty, kAudioStreamProperty_ThreadPriority ) )
	{
		// $$$ TODO: If your implementation uses a helper thread, set the priority of the thread to the string passed in
		// to this property.  See SetCurrentThreadPriority().
	}
	
	// Other
	
	else
	{
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AudioStreamRampVolume
//===========================================================================================================================

OSStatus
	AudioStreamRampVolume( 
		AudioStreamRef		inStream, 
		double				inFinalVolume, 
		double				inDurationSecs, 
		dispatch_queue_t	inQueue )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	OSStatus					err;
	
	assert( 0 );
	// $$$ TODO: The volume of the audio should be ramped to inFinalVolume over inDurationSecs.
	// To be consistent when the rest of the accessory's user experience, inFinalVolume may be replaced with a more
	// canonical value where appropriate (i.e., when this routine is called to perform the audio duck).
	(void) inFinalVolume;
	(void) inDurationSecs;
	(void) inQueue;
	(void) me;

	err = kNoErr;
	
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AudioStreamPrepare
//===========================================================================================================================

OSStatus	AudioStreamPrepare( AudioStreamRef inStream )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	OSStatus					err;
	
	// $$$ TODO: This is where the audio processing chain should be set up based on the properties previously set on the
	// AudioStream object:
	//	me->format specifies the sample rate, channel count, and bit-depth.
	//	me->input specifies whether or not the processing chain should be set up to record audio from the accessory's
	//	          microphone(s).
	// Audio output should always be prepared.
	// If the audio processing chain is successfully set up, me->prepared should be set to true.

	me->prepared = true;
	err = kNoErr;
	
	if( err ) AudioStreamStop( inStream, false );
	return( err );
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  pcm_input
 *  Description:  
 * =====================================================================================
 */
static gpointer pcm_input( gpointer user_data )
{
	AudioStreamImpRef const	me	=	_AudioStreamGetImp( user_data );
	gint64 _clock;
	gint32 _count	=	0;
	gint32 _frame	=	me->format.mSampleRate / 10;

	CFLRetain( me );

	me->sc	=	carplay_stub_open_pcm_device();

	if( me->sc == NULL )
	{
		g_warning( "carplay_stub_open_pcm_device return NULL" );

		goto OPEN_FAILED;
	}

	g_message( "--->%d %d %d %d %d %d", 
			me->format.mSampleRate,
			me->format.mBytesPerPacket,
			me->format.mFramesPerPacket,
			me->format.mBytesPerFrame,
			me->format.mChannelsPerFrame,
			me->format.mBitsPerChannel );

	if( !carplay_stub_set_pcm_params( me->sc, me->format.mSampleRate, me->format.mChannelsPerFrame ))
	{
		g_warning( "carplay_stub_set_pcm_params return FALSE" );

		goto PREPARE_FAILED;
	}

	_clock	=	UpTicks();

	while( 1 )
	{
		guint8 _buf[ _frame * 4 ];
		carplay_stub_read_pcm_data( me->sc, _buf, _frame );

		if( !me->inputing ) break;

		if( me->inputCallbackPtr != NULL )
		{
			me->inputCallbackPtr( _count, _clock, _buf, _frame * 4, me->inputCallbackCtx );
		}

		_clock	+=	100000000;
		_count	+=	_frame;
	}

PREPARE_FAILED:
	carplay_stub_close_pcm_device( me->sc );

OPEN_FAILED:

	CFLRelease( me );

	return NULL;
}		/* -----  end of static function pcm_input  ----- */

//===========================================================================================================================
//	AudioStreamStart
//===========================================================================================================================

OSStatus	AudioStreamStart( AudioStreamRef inStream )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );
	OSStatus					err;

	if( !me->prepared )
	{
		err = AudioStreamPrepare( inStream );
		require_noerr( err, exit );
	}
	
	// $$$ TODO: This is where the audio processing chain should be started.
	//
	// me->outputCallbackPtr should be invoked periodically to retrieve a continuous stream of samples to be output.
	// When calling me->outputCallbackPtr(), a buffer is provided for the caller to write into.  Equally important
	// is the inSampleTime and inHostTime arguments.  It is important that accurate { inSampleTime, inHostTime } pairs
	// be provided to the caller.  inSampleTime should be a (reasonably) current running total of the number of samples
	// that have hit the speaker since AudioStreamStart() was called.  inHostTime is the system time, in units of ticks,
	// corresponding to inSampleTime (see TickUtils.h).  This information will be returned to the controller and is
	// a key piece in allowing the controller to derive the relationship between the controller's system clock and the
	// accessory's audio (DAC) clock for A/V sync.
	//
	// If input has been requested (me->input == true), then me->inputCallbackPtr should also be invoked periodically
	// to provide a continuous stream of samples from the accessory's microphone (possibly with some processing, depending
	// on the audio type, see kAudioStreamProperty_AudioType).  If no audio samples are available for whatever reason,
	// the me->inputCallbackPtr should be called with a buffer of zeroes.

	if( me->input )
	{
		me->inputing	=	TRUE;
		g_thread_unref( g_thread_new( "pcm_input", pcm_input, me ));
	}

	me->clock	=	UpTicks();//g_source_get_time( me->source );
	me->running	=	300000000;

	gst_element_set_state( me->pipeline, GST_STATE_PLAYING );

	g_source_set_ready_time( me->source, 0 );

	err = kNoErr;
	
exit:
	if( err ) AudioStreamStop( inStream, false );
	return( err );
}

//===========================================================================================================================
//	AudioStreamStop
//===========================================================================================================================

void	AudioStreamStop( AudioStreamRef inStream, Boolean inDrain )
{
	AudioStreamImpRef const		me = _AudioStreamGetImp( inStream );

	// $$$ TODO: This is where the audio processing chain should be stopped, and the audio processing chain torn down.
	// When AudioStreamStop() returns, the object should return to the state similar to before AudioStreamPrepare()
	// was called, so this function is responsible for undoing any resource allocation performed in AudioStreamPrepare().
	(void) inDrain;

	if( me->input )
	{
		me->inputing	=	FALSE;
	}

	gst_element_set_state( me->pipeline, GST_STATE_READY );
	g_source_set_ready_time( me->source, -1 );

	me->prepared = false;
}

#if 0
#pragma mark -
#endif

static AudioSessionEventHandler_f audioSessionEventHandler;

//===========================================================================================================================
//	AudioSessionSetEventHandler
//===========================================================================================================================

void AudioSessionSetEventHandler( AudioSessionEventHandler_f inHandler, void *inContext )
{
	(void) inHandler;
	(void) inContext;
	
	g_message( "%s %d", __func__, __LINE__ );
	// This implementation should remain empty.
	audioSessionEventHandler	=	inHandler;
}

//===========================================================================================================================
//	_CreateLatencyDictionary
//===========================================================================================================================

static CFDictionaryRef
	_CreateLatencyDictionary(
		AudioStreamType inStreamType,
		CFStringRef inAudioType,
		uint32_t inSampleRate,
		uint32_t inSampleSize,
		uint32_t inChannels,
		uint32_t inInputLatency,
		uint32_t inOutputLatency,
		OSStatus *outErr )
{
	CFDictionaryRef						result = NULL;
	OSStatus							err;
	CFMutableDictionaryRef				latencyDict;
	
	latencyDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( latencyDict, exit, err = kNoMemoryErr );
	
	if( inStreamType != kAudioStreamType_Invalid )	CFDictionarySetInt64( latencyDict, kAudioSessionKey_Type, inStreamType );
	if( inAudioType )		CFDictionarySetValue( latencyDict, kAudioSessionKey_AudioType, inAudioType );
	if( inSampleRate > 0 )	CFDictionarySetInt64( latencyDict, kAudioSessionKey_SampleRate, inSampleRate );
	if( inSampleSize > 0 )	CFDictionarySetInt64( latencyDict, kAudioSessionKey_SampleSize, inSampleSize );
	if( inChannels > 0 )	CFDictionarySetInt64( latencyDict, kAudioSessionKey_Channels, inChannels );
	CFDictionarySetInt64( latencyDict, kAudioSessionKey_InputLatencyMicros, inInputLatency );
	CFDictionarySetInt64( latencyDict, kAudioSessionKey_OutputLatencyMicros, inOutputLatency );
	
	result = latencyDict;
	latencyDict = NULL;
	err = kNoErr;
exit:
	CFReleaseNullSafe( latencyDict );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
// AudioSessionCopyLatencies
//===========================================================================================================================

CFArrayRef	AudioSessionCopyLatencies( OSStatus *outErr )
{
	CFArrayRef							result = NULL;
	OSStatus							err;
	
	g_message( "%s %d", __func__, __LINE__ );
	// $$$ TODO: obtain audio latencies for all audio formats and audio types supported by the underlying hardware.
	// Audio latencies are reported as an ordered array of dictionaries (from least restrictive to the most restrictive).
	// Each dictionary contains the following keys:
	//		[kAudioSessionKey_Type] - if not specified, then latencies are good for all stream types
	//		[kAudioSessionKey_AudioType] - if not specified, then latencies are good for all audio types
	//		[kAudioSessionKey_SampleRate] - if not specified, then latencies are good for all sample rates
	//		[kAudioSessionKey_SampleSize] - if not specified, then latencies are good for all sample sizes
	//		[kAudioSessionKey_Channels] - if not specified, then latencies are good for all channel counts
	//		[kAudioSessionKey_CompressionType] - if not specified, then latencies are good for all compression types
	//		kAudioSessionKey_InputLatencyMicros
	//		kAudioSessionKey_OutputLatencyMicros
	
	CFMutableArrayRef					audioLatenciesArray;
	CFDictionaryRef						dict = NULL;
	
	audioLatenciesArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( audioLatenciesArray, exit, err = kNoMemoryErr );
	
	// default latencies - $$$ TODO set real latencies
	
	dict = _CreateLatencyDictionary(
		kAudioStreamType_Invalid,					// inStreamType
		NULL,										// inAudioType
		0,											// inSampleRate,
		0,											// inSampleSize,
		0,											// inChannels,
		0,											// inInputLatency,
		0,											// inOutputLatency,
		&err );
	require_noerr( err, exit );
	
	CFArrayAppendValue( audioLatenciesArray, dict );
	ForgetCF( &dict );
	
	// telephony latencies - set 0 latencies for now - $$$ TODO set real latencies
	
	dict = _CreateLatencyDictionary(
		kAudioStreamType_MainAudio,					// inStreamType
		kAudioStreamAudioType_Telephony,			// inAudioType
		0,											// inSampleRate,
		0,											// inSampleSize,
		0,											// inChannels,
		0,											// inInputLatency,
		0,											// inOutputLatency,
		&err );
	require_noerr( err, exit );
	
	CFArrayAppendValue( audioLatenciesArray, dict );
	ForgetCF( &dict );
	
	// SpeechRecognition latencies - set 0 latencies for now - $$$ TODO set real latencies
	
	dict = _CreateLatencyDictionary(
		kAudioStreamType_MainAudio,					// inStreamType
		kAudioStreamAudioType_SpeechRecognition,	// inAudioType
		0,											// inSampleRate,
		0,											// inSampleSize,
		0,											// inChannels,
		0,											// inInputLatency,
		0,											// inOutputLatency,
		&err );
	require_noerr( err, exit );
	
	CFArrayAppendValue( audioLatenciesArray, dict );
	ForgetCF( &dict );
	
	// $$$ TODO add more latencies dictionaries as needed
	
	result = audioLatenciesArray;
	audioLatenciesArray = NULL;
	err = kNoErr;

exit:
	CFReleaseNullSafe( dict );
	CFReleaseNullSafe( audioLatenciesArray );
	if( outErr ) *outErr = err;
	return( result );
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
	(void) inHasInput;
	(void) inPreferredSystemSampleRate;
	(void) inPreferredSystemBufferSizeMicros;

	g_message( "%s %d", __func__, __LINE__ );
	// This implementation should remain empty.
}

//===========================================================================================================================
//	AudioSessionEnsureTornDown
//===========================================================================================================================

void AudioSessionEnsureTornDown( void )
{
	g_message( "%s %d", __func__, __LINE__ );
	// This implementation should remain empty.
}

//===========================================================================================================================
//	AudioSessionGetSupportedInputFormats
//===========================================================================================================================

AudioSessionAudioFormat	AudioSessionGetSupportedInputFormats( AudioStreamType inStreamType )
{
	AudioSessionAudioFormat		formats;
	
	(void) inStreamType;
	
	g_message( "%s %d", __func__, __LINE__ );
	formats = 0;
	formats |= kAudioSessionAudioFormat_PCM_16KHz_16Bit_Stereo;//kAudioSessionAudioFormat_PCM_8KHz_16Bit_Mono;

	// $$$ TODO: This is where the accessory provides a list of audio input formats it supports in hardware.
	// It is important that, at a minimum, all sample rates required by the specification are included here.
	
	return( formats );
}

//===========================================================================================================================
//	AudioSessionGetSupportedOutputFormats
//===========================================================================================================================

AudioSessionAudioFormat	AudioSessionGetSupportedOutputFormats( AudioStreamType inStreamType )
{
	AudioSessionAudioFormat		formats;
	
	g_message( "%s %d", __func__, __LINE__ );
	formats = AudioSessionGetSupportedInputFormats( inStreamType );

	// $$$ TODO: This is where the accessory provides a list of audio output formats it supports in hardware.
	// It is expected that the list of supported audio output formats is a superset of the supported audio
	// input formats.  As with input formats, it is important that, at a minimum, all sample rates required
	// by the specification are included here.

	formats |= kAudioSessionAudioFormat_PCM_48KHz_16Bit_Stereo;

	return formats;
}

#if 0
#pragma mark -
#endif

