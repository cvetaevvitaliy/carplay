/*
	File:    	AirPlayUtils.c
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

#include "AirPlayUtils.h"

#include "AirPlayCommon.h"
#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFUtils.h"
#include "StringUtils.h"
#include "TickUtils.h"

#include COREAUDIO_HEADER
#include SHA_HEADER

//===========================================================================================================================
//	Internals
//===========================================================================================================================

#define RTPJitterBufferLock( CTX )			dispatch_semaphore_wait( (CTX)->nodeLock, DISPATCH_TIME_FOREVER )
#define RTPJitterBufferUnlock( CTX )		dispatch_semaphore_signal( (CTX)->nodeLock )

#define RTPJitterBufferBufferedSamples( CTX )	( (uint32_t)( \
	( ctx->busyList->prev->pkt.pkt.rtp.header.ts - ctx->busyList->next->pkt.pkt.rtp.header.ts ) + \
	( ctx->busyList->prev->pkt.len / ctx->bytesPerFrame ) ) )

#define RTPJitterBufferSamplesToMs( CTX, X )	( ( ( 1000 * (X) ) + ( (CTX)->sampleRate / 2 ) ) / (CTX)->sampleRate )
#define RTPJitterBufferMsToSamples( CTX, X )	( ( (X) * (CTX)->sampleRate ) / 1000 )
#define RTPJitterBufferBufferedMs( CTX )		RTPJitterBufferSamplesToMs( (CTX), RTPJitterBufferBufferedSamples( (CTX) ) )

static void	_RTPJitterBufferReleaseBusyNode( RTPJitterBufferContext *ctx, RTPPacketNode *inNode );

ulog_define( AirPlayJitterBuffer, kLogLevelNotice, kLogFlags_Default, "AirPlay", "AirPlayJitterBuffer:rate=3;1000" );
#define ap_jitter_ulog( LEVEL, ... )		ulog( &log_category_from_name( AirPlayJitterBuffer ), (LEVEL), __VA_ARGS__ )
#define ap_jitter_label( CTX )				( (CTX)->label ? (CTX)->label : "Default" )

//===========================================================================================================================
//	AirPlayAudioFormatToASBD
//===========================================================================================================================

OSStatus
	AirPlayAudioFormatToASBD( 
		AirPlayAudioFormat				inFormat, 
		AudioStreamBasicDescription *	outASBD, 
		uint32_t *						outBitsPerChannel )
{
	switch( inFormat )
	{
		case kAirPlayAudioFormat_PCM_8KHz_16Bit_Mono:		ASBD_FillPCM( outASBD,  8000, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_8KHz_16Bit_Stereo:		ASBD_FillPCM( outASBD,  8000, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono:		ASBD_FillPCM( outASBD, 16000, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_16KHz_16Bit_Stereo:	ASBD_FillPCM( outASBD, 16000, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono:		ASBD_FillPCM( outASBD, 24000, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_24KHz_16Bit_Stereo:	ASBD_FillPCM( outASBD, 24000, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_32KHz_16Bit_Mono:		ASBD_FillPCM( outASBD, 32000, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_32KHz_16Bit_Stereo:	ASBD_FillPCM( outASBD, 32000, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_44KHz_16Bit_Mono:		ASBD_FillPCM( outASBD, 44100, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo:	ASBD_FillPCM( outASBD, 44100, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_44KHz_24Bit_Mono:		ASBD_FillPCM( outASBD, 44100, 24, 24, 1 ); break;
		case kAirPlayAudioFormat_PCM_44KHz_24Bit_Stereo:	ASBD_FillPCM( outASBD, 44100, 24, 24, 2 ); break;
		case kAirPlayAudioFormat_PCM_48KHz_16Bit_Mono:		ASBD_FillPCM( outASBD, 48000, 16, 16, 1 ); break;
		case kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo:	ASBD_FillPCM( outASBD, 48000, 16, 16, 2 ); break;
		case kAirPlayAudioFormat_PCM_48KHz_24Bit_Mono:		ASBD_FillPCM( outASBD, 48000, 24, 24, 1 ); break;
		case kAirPlayAudioFormat_PCM_48KHz_24Bit_Stereo:	ASBD_FillPCM( outASBD, 48000, 24, 24, 2 ); break;
		#if( AIRPLAY_ALAC )
		case kAirPlayAudioFormat_ALAC_44KHz_16Bit_Stereo:	ASBD_FillALAC( outASBD, 44100, 16, 2 ); break;
		case kAirPlayAudioFormat_ALAC_44KHz_24Bit_Stereo:	ASBD_FillALAC( outASBD, 44100, 24, 2 ); break;
		case kAirPlayAudioFormat_ALAC_48KHz_16Bit_Stereo:	ASBD_FillALAC( outASBD, 48000, 16, 2 ); break;
		case kAirPlayAudioFormat_ALAC_48KHz_24Bit_Stereo:	ASBD_FillALAC( outASBD, 48000, 24, 2 ); break;
		#endif
		default: return( kUnsupportedErr );
	}
	if( outBitsPerChannel )
	{
		if( ( inFormat == kAirPlayAudioFormat_ALAC_44KHz_16Bit_Stereo )	||
			( inFormat == kAirPlayAudioFormat_ALAC_48KHz_16Bit_Stereo )	||
			( inFormat == kAirPlayAudioFormat_AAC_LC_44KHz_Stereo )		||
			( inFormat == kAirPlayAudioFormat_AAC_LC_48KHz_Stereo )		||
			( inFormat == kAirPlayAudioFormat_AAC_ELD_44KHz_Stereo )	||
			( inFormat == kAirPlayAudioFormat_AAC_ELD_48KHz_Stereo ) )
		{
			*outBitsPerChannel = 16;
		}
		else if( ( inFormat == kAirPlayAudioFormat_ALAC_44KHz_24Bit_Stereo ) ||
				 ( inFormat == kAirPlayAudioFormat_ALAC_48KHz_24Bit_Stereo ) )
		{
			*outBitsPerChannel = 24;
		}
		else
		{
			*outBitsPerChannel = outASBD->mBitsPerChannel;
		}
	}
	return( kNoErr );
}

//===========================================================================================================================
//	AirPlayAudioFormatToPCM
//===========================================================================================================================

OSStatus	AirPlayAudioFormatToPCM( AirPlayAudioFormat inFormat, AudioStreamBasicDescription *outASBD )
{
	OSStatus						err;
	AudioStreamBasicDescription		asbd;
	uint32_t						bitsPerChannel;
	
	err = AirPlayAudioFormatToASBD( inFormat, &asbd, &bitsPerChannel );
	require_noerr_quiet( err, exit );
	
	if( asbd.mFormatID == kAudioFormatLinearPCM )
	{
		memcpy( outASBD, &asbd, sizeof( asbd ) );
	}
	else
	{
		ASBD_FillPCM( outASBD, asbd.mSampleRate, bitsPerChannel, RoundUp( bitsPerChannel, 8 ), asbd.mChannelsPerFrame );
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	AirPlayCreateModesDictionary
//===========================================================================================================================

EXPORT_GLOBAL
CFDictionaryRef	AirPlayCreateModesDictionary( const AirPlayModeChanges *inChanges, CFStringRef inReason, OSStatus *outErr )
{
	CFDictionaryRef				result		= NULL;
	Boolean const				useStrings	= false;
	CFMutableDictionaryRef		params;
	CFMutableArrayRef			appStates	= NULL;
	CFMutableArrayRef			resources	= NULL;
	CFMutableDictionaryRef		tempDict	= NULL;
	CFStringRef					key;
	OSStatus					err;
	
	params = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( params, exit, err = kNoMemoryErr );
	
	// AppState: PhoneCall
	
	if( inChanges->phoneCall != kAirPlayTriState_NotApplicable )
	{
		tempDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( tempDict, exit, err = kNoMemoryErr );
		if( useStrings )
		{
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_AppStateID ), 
				AirPlayAppStateIDToCFString( kAirPlayAppStateID_PhoneCall ) );
		}
		else
		{
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_AppStateID ), kAirPlayAppStateID_PhoneCall );
		}
		CFDictionarySetBoolean( tempDict, CFSTR( kAirPlayKey_State ), inChanges->phoneCall != kAirPlayTriState_False );
		
		err = CFArrayEnsureCreatedAndAppend( &appStates, tempDict );
		CFRelease( tempDict );
		tempDict = NULL;
		require_noerr( err, exit );
	}
	
	// AppState: Speech
	
	if( inChanges->speech != kAirPlayEntity_NotApplicable )
	{
		tempDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( tempDict, exit, err = kNoMemoryErr );
		if( useStrings )
		{
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_AppStateID ), 
				AirPlayAppStateIDToCFString( kAirPlayAppStateID_Speech ) );
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_SpeechMode ), AirPlaySpeechModeToCFString( inChanges->speech ) );
		}
		else
		{
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_AppStateID ), kAirPlayAppStateID_Speech );
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_SpeechMode ), inChanges->speech );
		}
		
		err = CFArrayEnsureCreatedAndAppend( &appStates, tempDict );
		CFRelease( tempDict );
		tempDict = NULL;
		require_noerr( err, exit );
	}
	
	// AppState: TurnByTurn
	
	if( inChanges->turnByTurn != kAirPlayTriState_NotApplicable )
	{
		tempDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( tempDict, exit, err = kNoMemoryErr );
		if( useStrings )
		{
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_AppStateID ), 
				AirPlayAppStateIDToCFString( kAirPlayAppStateID_TurnByTurn ) );
		}
		else
		{
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_AppStateID ), kAirPlayAppStateID_TurnByTurn );
		}
		CFDictionarySetBoolean( tempDict, CFSTR( kAirPlayKey_State ), inChanges->turnByTurn != kAirPlayTriState_False );
		
		err = CFArrayEnsureCreatedAndAppend( &appStates, tempDict );
		CFRelease( tempDict );
		tempDict = NULL;
		require_noerr( err, exit );
	}
	
	// Resource: Screen
	
	if( inChanges->screen.type != kAirPlayTransferType_NotApplicable )
	{
		tempDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( tempDict, exit, err = kNoMemoryErr );
		
		if( useStrings )
		{
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_ResourceID ), 
				AirPlayResourceIDToCFString( kAirPlayResourceID_MainScreen ) );
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TransferType ), 
				AirPlayTransferTypeToCFString( inChanges->screen.type ) );
			if( inChanges->screen.priority != kAirPlayTransferPriority_NotApplicable )
			{
				CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TransferPriority ), 
					AirPlayTransferPriorityToCFString( inChanges->screen.priority ) );
			}
			if( inChanges->screen.takeConstraint != kAirPlayConstraint_NotApplicable )
			{
				CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TakeConstraint ), 
					AirPlayConstraintToCFString( inChanges->screen.takeConstraint ) );
			}
			if( inChanges->screen.borrowOrUnborrowConstraint != kAirPlayConstraint_NotApplicable )
			{
				if(      inChanges->screen.type == kAirPlayTransferType_Take )   key = CFSTR( kAirPlayKey_BorrowConstraint );
				else if( inChanges->screen.type == kAirPlayTransferType_Borrow ) key = CFSTR( kAirPlayKey_UnborrowConstraint );
				else { dlogassert( "Bad borrow/unborrow constraint" ); err = kParamErr; goto exit; }
				CFDictionarySetValue( tempDict, key, AirPlayConstraintToCFString( inChanges->screen.borrowOrUnborrowConstraint ) );
			}
		}
		else
		{
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_ResourceID ), kAirPlayResourceID_MainScreen );
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TransferType ), inChanges->screen.type );
			if( inChanges->screen.priority != kAirPlayTransferPriority_NotApplicable )
			{
				CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TransferPriority ), inChanges->screen.priority );
			}
			if( inChanges->screen.takeConstraint != kAirPlayConstraint_NotApplicable )
			{
				CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TakeConstraint ), inChanges->screen.takeConstraint );
			}
			if( inChanges->screen.borrowOrUnborrowConstraint != kAirPlayConstraint_NotApplicable )
			{
				if(      inChanges->screen.type == kAirPlayTransferType_Take )   key = CFSTR( kAirPlayKey_BorrowConstraint );
				else if( inChanges->screen.type == kAirPlayTransferType_Borrow ) key = CFSTR( kAirPlayKey_UnborrowConstraint );
				else { dlogassert( "Bad borrow/unborrow constraint" ); err = kParamErr; goto exit; }
				CFDictionarySetInt64( tempDict, key, inChanges->screen.borrowOrUnborrowConstraint );
			}
		}
		
		err = CFArrayEnsureCreatedAndAppend( &resources, tempDict );
		CFRelease( tempDict );
		tempDict = NULL;
		require_noerr( err, exit );
	}
	
	// Resource: MainAudio
	
	if( inChanges->mainAudio.type != kAirPlayTransferType_NotApplicable )
	{
		tempDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( tempDict, exit, err = kNoMemoryErr );
		
		if( useStrings )
		{
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_ResourceID ), 
				AirPlayResourceIDToCFString( kAirPlayResourceID_MainAudio ) );
			CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TransferType ), 
				AirPlayTransferTypeToCFString( inChanges->mainAudio.type ) );
			if( inChanges->mainAudio.priority != kAirPlayTransferPriority_NotApplicable )
			{
				CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TransferPriority ), 
					AirPlayTransferPriorityToCFString( inChanges->mainAudio.priority ) );
			}
			if( inChanges->mainAudio.takeConstraint != kAirPlayConstraint_NotApplicable )
			{
				CFDictionarySetValue( tempDict, CFSTR( kAirPlayKey_TakeConstraint ), 
					AirPlayConstraintToCFString( inChanges->mainAudio.takeConstraint ) );
			}
			if( inChanges->mainAudio.borrowOrUnborrowConstraint != kAirPlayConstraint_NotApplicable )
			{
				if(      inChanges->mainAudio.type == kAirPlayTransferType_Take )   key = CFSTR( kAirPlayKey_BorrowConstraint );
				else if( inChanges->mainAudio.type == kAirPlayTransferType_Borrow ) key = CFSTR( kAirPlayKey_UnborrowConstraint );
				else { dlogassert( "Bad borrow/unborrow constraint" ); err = kParamErr; goto exit; }
				CFDictionarySetValue( tempDict, key, AirPlayConstraintToCFString( inChanges->mainAudio.borrowOrUnborrowConstraint ) );
			}
		}
		else
		{
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_ResourceID ), kAirPlayResourceID_MainAudio );
			CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TransferType ), inChanges->mainAudio.type );
			if( inChanges->mainAudio.priority != kAirPlayTransferPriority_NotApplicable )
			{
				CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TransferPriority ), inChanges->mainAudio.priority );
			}
			if( inChanges->mainAudio.takeConstraint != kAirPlayConstraint_NotApplicable )
			{
				CFDictionarySetInt64( tempDict, CFSTR( kAirPlayKey_TakeConstraint ), inChanges->mainAudio.takeConstraint );
			}
			if( inChanges->mainAudio.borrowOrUnborrowConstraint != kAirPlayConstraint_NotApplicable )
			{
				if(      inChanges->mainAudio.type == kAirPlayTransferType_Take )   key = CFSTR( kAirPlayKey_BorrowConstraint );
				else if( inChanges->mainAudio.type == kAirPlayTransferType_Borrow ) key = CFSTR( kAirPlayKey_UnborrowConstraint );
				else { dlogassert( "Bad borrow/unborrow constraint" ); err = kParamErr; goto exit; }
				CFDictionarySetInt64( tempDict, key, inChanges->mainAudio.borrowOrUnborrowConstraint );
			}
		}
		
		err = CFArrayEnsureCreatedAndAppend( &resources, tempDict );
		CFRelease( tempDict );
		tempDict = NULL;
		require_noerr( err, exit );
	}
	
	if( appStates )	CFDictionarySetValue( params, CFSTR( kAirPlayKey_AppStates ), appStates );
	if( resources )	CFDictionarySetValue( params, CFSTR( kAirPlayKey_Resources ), resources );
	if( inReason )	CFDictionarySetValue( params, CFSTR( kAirPlayKey_ReasonStr ), inReason );
	result = params;
	params = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( params );
	CFReleaseNullSafe( appStates );
	CFReleaseNullSafe( resources );
	CFReleaseNullSafe( tempDict );
	if( outErr ) *outErr = err;
	return( result );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AirPlay_DeriveAESKeySHA512
//===========================================================================================================================

void
	AirPlay_DeriveAESKeySHA512(
		const void *		inMasterKeyPtr,
		size_t				inMasterKeyLen,
		const void *		inKeySaltPtr,
		size_t				inKeySaltLen,
		const void *		inIVSaltPtr,
		size_t				inIVSaltLen,
		uint8_t				outKey[ 16 ],
		uint8_t				outIV[ 16 ] )
{
	SHA512_CTX		shaCtx;
	uint8_t			buf[ 64 ];
	
	if( outKey )
	{
		SHA512_Init( &shaCtx );
		SHA512_Update( &shaCtx, inKeySaltPtr, inKeySaltLen );
		SHA512_Update( &shaCtx, inMasterKeyPtr, inMasterKeyLen );
		SHA512_Final( buf, &shaCtx );
		memcpy( outKey, buf, 16 );
	}
	if( outIV )
	{
		SHA512_Init( &shaCtx );
		SHA512_Update( &shaCtx, inIVSaltPtr, inIVSaltLen );
		SHA512_Update( &shaCtx, inMasterKeyPtr, inMasterKeyLen );
		SHA512_Final( buf, &shaCtx );
		memcpy( outIV, buf, 16 );
	}
	MemZeroSecure( buf, sizeof( buf ) );
}

//===========================================================================================================================
//	AirPlay_DeriveAESKeySHA512ForScreen
//===========================================================================================================================

void
	AirPlay_DeriveAESKeySHA512ForScreen(
		const void *		inMasterKeyPtr,
		size_t				inMasterKeyLen,
		uint64_t			inScreenStreamConnectionID,
		uint8_t				outKey[ 16 ],
		uint8_t				outIV[ 16 ] )
{

	char *screenStreamKeySalt = NULL, *screenStreamIVSalt = NULL;
	size_t screenStreamKeySaltLen, screenStreamIVSaltLen;
				
	screenStreamKeySaltLen = ASPrintF( &screenStreamKeySalt, "%s%llu", kAirPlayEncryptionStreamPrefix_Key, inScreenStreamConnectionID );
	screenStreamIVSaltLen = ASPrintF( &screenStreamIVSalt, "%s%llu", kAirPlayEncryptionStreamPrefix_IV, inScreenStreamConnectionID );
				
	AirPlay_DeriveAESKeySHA512( inMasterKeyPtr, inMasterKeyLen, screenStreamKeySalt, screenStreamKeySaltLen, screenStreamIVSalt, screenStreamIVSaltLen, outKey, outIV );

	MemZeroSecure( screenStreamKeySalt, screenStreamKeySaltLen );
	MemZeroSecure( screenStreamIVSalt, screenStreamIVSaltLen );

	free( screenStreamKeySalt );
	free( screenStreamIVSalt );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	RTPJitterBufferInit
//===========================================================================================================================

OSStatus
	RTPJitterBufferInit( 
		RTPJitterBufferContext *ctx, 
		uint32_t				inSampleRate, 
		uint32_t				inBytesPerFrame, 
		uint32_t				inBufferMs )
{
	OSStatus		err;
	size_t			i, n;
	
	memset( ctx, 0, sizeof( *ctx ) );
	
	ctx->nodeLock = dispatch_semaphore_create( 1 );
	require_action( ctx->nodeLock, exit, err = kNoResourcesErr );
	
	n = 50; // ~400 ms at 352 samples per packet and 44100 Hz.
	ctx->packets = (RTPPacketNode *) malloc( n * sizeof( *ctx->packets ) );
	require_action( ctx->packets, exit, err = kNoMemoryErr );
	
	n -= 1;
	for( i = 0; i < n; ++i )
	{
		ctx->packets[ i ].next = &ctx->packets[ i + 1 ];
	}
	ctx->packets[ i ].next	= NULL;
	ctx->freeList			= ctx->packets;
	ctx->sentinel.prev		= &ctx->sentinel;
	ctx->sentinel.next		= &ctx->sentinel;
	ctx->busyList			= &ctx->sentinel;
	ctx->sampleRate			= inSampleRate;
	ctx->bytesPerFrame		= inBytesPerFrame;
	ctx->bufferMs			= inBufferMs;
	ctx->buffering			= true;
	err = kNoErr;
	
exit:
	if( err ) RTPJitterBufferFree( ctx );
	return( err );
}

//===========================================================================================================================
//	RTPJitterBufferFree
//===========================================================================================================================

void	RTPJitterBufferFree( RTPJitterBufferContext *ctx )
{
	if( ( ctx->nLate > 0 ) || ( ctx->nGaps > 0 ) || ( ctx->nSkipped > 0 ) || ( ctx->nRebuffer > 0 ) )
	{
		ap_jitter_ulog( kLogLevelNotice | kLogLevelFlagDontRateLimit, 
			"### %s: Buffering issues during session: Late=%u Missing=%u Gaps=%u Rebuffers=%u\n", 
			ap_jitter_label( ctx ), ctx->nLate, ctx->nGaps, ctx->nSkipped, ctx->nRebuffer );
	}
	ctx->nLate		= 0;
	ctx->nGaps		= 0;
	ctx->nSkipped	= 0;
	ctx->nRebuffer	= 0;
	
	dispatch_forget( &ctx->nodeLock );
	ForgetMem( &ctx->packets );
}

//===========================================================================================================================
//	RTPJitterBufferReset
//===========================================================================================================================

void	RTPJitterBufferReset( RTPJitterBufferContext *ctx )
{
	RTPPacketNode *		stop;
	RTPPacketNode *		node;
	RTPPacketNode *		next;
	
	RTPJitterBufferLock( ctx );
	
	stop = ctx->busyList;
	for( node = stop->next; node != stop; node = next )
	{
		next = node->next;
		_RTPJitterBufferReleaseBusyNode( ctx, node );
	}
	ctx->buffering = true;
	
	RTPJitterBufferUnlock( ctx );
}

//===========================================================================================================================
//	RTPJitterBufferGetFreeNode
//===========================================================================================================================

OSStatus	RTPJitterBufferGetFreeNode( RTPJitterBufferContext *ctx, RTPPacketNode **outNode )
{
	OSStatus			err;
	RTPPacketNode *		node;
	RTPPacketNode *		stop;
	
	RTPJitterBufferLock( ctx );
	node = ctx->freeList;
	if( node )
	{
		ctx->freeList = node->next;
	}
	else
	{
		// No free nodes so steal the oldest node.
		
		stop = ctx->busyList;
		node = stop->next;
		require_action( node != stop, exit, err = kInternalErr );
		node->next->prev = node->prev;
		node->prev->next = node->next;
	}
	*outNode = node;
	err = kNoErr;
	
exit:
	RTPJitterBufferUnlock( ctx );
	return( err );
}

//===========================================================================================================================
//	RTPJitterBufferPutFreeNode
//===========================================================================================================================

void	RTPJitterBufferPutFreeNode( RTPJitterBufferContext *ctx, RTPPacketNode *inNode )
{
	RTPJitterBufferLock( ctx );
	inNode->next = ctx->freeList;
	ctx->freeList = inNode;
	RTPJitterBufferUnlock( ctx );
}

//===========================================================================================================================
//	RTPJitterBufferPutBusyNode
//===========================================================================================================================

OSStatus	RTPJitterBufferPutBusyNode( RTPJitterBufferContext *ctx, RTPPacketNode *inNode )
{
	uint32_t const		ts = inNode->pkt.pkt.rtp.header.ts;
	OSStatus			err;
	RTPPacketNode *		stop;
	RTPPacketNode *		node;
	
	RTPJitterBufferLock( ctx );
	
	// Drop the node if we're not enabled.
	
	if( ctx->disabled )
	{
		inNode->next = ctx->freeList;
		ctx->freeList = inNode;
		err = kNoErr;
		goto exit;
	}
	
	// Insert the new node in timestamp order. It's most likely to be a later timestamp so start at the end.
	
	stop = ctx->busyList;
	for( node = stop->prev; ( node != stop ) && Mod32_GT( node->pkt.pkt.rtp.header.ts, ts ); node = node->prev ) {}
	require_action( ( node == stop ) || ( node->pkt.pkt.rtp.header.ts != ts ), exit, err = kDuplicateErr );
	
	inNode->prev		= node;
	inNode->next		= node->next;
	inNode->next->prev	= inNode;
	inNode->prev->next	= inNode;
	
	// If this is the first packet after we started buffering then schedule audio to start after the buffer window.
	
	if( ctx->buffering && ( ctx->startTicks == 0 ) )
	{
		ctx->startTicks = UpTicks() + MillisecondsToUpTicks( ctx->bufferMs );
		ap_jitter_ulog( kLogLevelNotice | kLogLevelFlagDontRateLimit, "%s: Starting audio in %u ms\n", 
			ap_jitter_label( ctx ), ctx->bufferMs );
	}
	err = kNoErr;
	
exit:
	RTPJitterBufferUnlock( ctx );
	return( err );
}

//===========================================================================================================================
//	_RTPJitterBufferReleaseBusyNode
//===========================================================================================================================

static void	_RTPJitterBufferReleaseBusyNode( RTPJitterBufferContext *ctx, RTPPacketNode *inNode )
{
	inNode->next->prev	= inNode->prev;
	inNode->prev->next	= inNode->next;
	inNode->next		= ctx->freeList;
	ctx->freeList		= inNode;
}

//===========================================================================================================================
//	RTPJitterBufferRead
//===========================================================================================================================

OSStatus	RTPJitterBufferRead( RTPJitterBufferContext *ctx, void *inBuffer, size_t inLen )
{
	uint32_t const		lenTS = (uint32_t)( inLen / ctx->bytesPerFrame );
	uint8_t *			dst   = (uint8_t *) inBuffer;
	RTPPacketNode *		stop;
	RTPPacketNode *		node;
	RTPPacketNode *		next;
	uint32_t			nowTS, limTS, srcTS, endTS, delta;
	size_t				len;
	Boolean				cap;
	uint64_t			ticks;
	
	RTPJitterBufferLock( ctx );
	
	if( ctx->buffering )
	{
		ticks = UpTicks();
		if( ( ctx->startTicks == 0 ) || ( ticks < ctx->startTicks ) )
		{
			memset( inBuffer, 0, inLen );
			goto exit;
		}
		nowTS = ctx->busyList->next->pkt.pkt.rtp.header.ts;
		ctx->buffering = false;
		ap_jitter_ulog( kLogLevelNotice | kLogLevelFlagDontRateLimit, "%s: Buffering complete, %d ms (%d), %llu ticks late\n", 
			ap_jitter_label( ctx ), RTPJitterBufferBufferedMs( ctx ), RTPJitterBufferBufferedSamples( ctx ), 
			ticks - ctx->startTicks );
	}
	else
	{
		nowTS = ctx->nextTS;
	}
	limTS = nowTS + lenTS;
	
	stop = ctx->busyList;
	for( node = stop->next; node != stop; node = next )
	{
		srcTS = node->pkt.pkt.rtp.header.ts;
		if( Mod32_GE( srcTS, limTS ) ) break;
		
		// If the node is before the timing window, it's too late so go to the next one.
		
		endTS = (uint32_t)( srcTS + ( node->pkt.len / ctx->bytesPerFrame ) );
		if( Mod32_LE( endTS, nowTS ) )
		{
			++ctx->nLate;
			ap_jitter_ulog( kLogLevelNotice, "%s: Late: %d ms (%u total)\n", ap_jitter_label( ctx ), 
				RTPJitterBufferSamplesToMs( ctx, (int)( nowTS - endTS ) ), ctx->nLate );
			goto next;
		}
		
		// If the node has samples before the timing window, they're late so skip them.
		
		if( Mod32_LT( srcTS, nowTS ) )
		{
			++ctx->nSkipped;
			ap_jitter_ulog( kLogLevelNotice, "%s: Skip: %d ms (%u total)\n", ap_jitter_label( ctx ), 
				RTPJitterBufferSamplesToMs( ctx, (int)( nowTS - srcTS ) ), ctx->nSkipped );
			
			delta = nowTS - srcTS;
			len   = delta * ctx->bytesPerFrame;
			node->ptr += len;
			node->pkt.len -= len;
			node->pkt.pkt.rtp.header.ts += delta;
			srcTS = nowTS;
		}
		
		// If the node starts after the beginning of the timing window, there's a gap so fill in silence.
		
		else if( Mod32_GT( srcTS, nowTS ) )
		{
			++ctx->nGaps;
			ap_jitter_ulog( kLogLevelNotice, "%s: Gap:  %d ms (%u total)\n", ap_jitter_label( ctx ), 
				RTPJitterBufferSamplesToMs( ctx, (int)( srcTS - nowTS ) ), ctx->nGaps );
			
			delta = srcTS - nowTS;
			len   = delta * ctx->bytesPerFrame;
			memset( dst, 0, len );
			dst   += len;
			nowTS += delta;
		}
		
		// Copy into the playout buffer.
		
		cap = Mod32_GT( endTS, limTS );
		if( cap ) endTS = limTS;
		
		delta = endTS - srcTS;
		len   = delta * ctx->bytesPerFrame;
		memcpy( dst, node->ptr, len );
		dst   += len;
		nowTS += delta;
		if( cap )
		{
			node->ptr     += len;
			node->pkt.len -= len;
			node->pkt.pkt.rtp.header.ts	+= delta;
			break;
		}
		
	next:
		next = node->next;
		_RTPJitterBufferReleaseBusyNode( ctx, node );
	}
	
	// If more samples are needed for this timing window then we've run dry. If it's prolonged then re-buffer.
	
	if( Mod32_LT( nowTS, limTS ) )
	{
		++ctx->nRebuffer;
		ap_jitter_ulog( kLogLevelNotice | kLogLevelFlagDontRateLimit, 
			"%s: Re-buffering: %d ms buffered (%d), %d ms missing (%d), %u total)\n", ap_jitter_label( ctx ), 
			RTPJitterBufferBufferedMs( ctx ), RTPJitterBufferBufferedSamples( ctx ), 
			RTPJitterBufferSamplesToMs( ctx, (int)( limTS - nowTS ) ), (int)( limTS - nowTS ), ctx->nRebuffer );
		
		delta = limTS - nowTS;
		len = delta * ctx->bytesPerFrame;
		memset( dst, 0, len );
		ctx->buffering = true;
		ctx->startTicks = 0;
		nowTS = limTS;
	}
	ctx->nextTS = nowTS;
	
exit:
	RTPJitterBufferUnlock( ctx );
	return( kNoErr );
}
