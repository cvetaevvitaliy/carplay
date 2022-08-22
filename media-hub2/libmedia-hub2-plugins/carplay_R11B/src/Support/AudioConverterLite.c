/*
	File:    	AudioConverterLite.c
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

#include "AudioConverterLite.h"

#if( AUDIO_CONVERTER_LITE_ENABLED )

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#if( !defined( AUDIO_CONVERTER_ALAC ) )
	#define AUDIO_CONVERTER_ALAC		1
#endif

#if( AUDIO_CONVERTER_ALAC )
	#include "AppleLosslessDecoder.h"
	#include "BitUtilities.h"
#endif

//===========================================================================================================================
//	Internals
//===========================================================================================================================

typedef struct AudioConverterPrivate *		AudioConverterPrivateRef;
struct AudioConverterPrivate
{
#if( AUDIO_CONVERTER_ALAC )
	AppleLosslessDecoderRef		alacDecoder;
#endif
};

//===========================================================================================================================
//	AudioConverterNew_compat
//===========================================================================================================================

OSStatus
	AudioConverterNew_compat( 
		const AudioStreamBasicDescription *	inSourceFormat,
		const AudioStreamBasicDescription *	inDestinationFormat,
		AudioConverterRef *					outAudioConverter )
{
	OSStatus						err;
	AudioConverterPrivateRef		me;
	
#if( !AUDIO_CONVERTER_ALAC )	
	(void) inSourceFormat;
#endif
	
	me = (AudioConverterPrivateRef) calloc( 1, sizeof( *me ) );
	require_action( me, exit, err = kNoMemoryErr );
	
	if( 0 ) {}
#if( AUDIO_CONVERTER_ALAC )
	else if( inSourceFormat->mFormatID == kAudioFormatAppleLossless )
	{
		err = AppleLosslessDecoder_Create( &me->alacDecoder );
		require_noerr( err, exit );
	}
#endif
	else
	{
		err = kUnsupportedErr;
		goto exit;
	}
	
	require_action_quiet( inDestinationFormat->mFormatID == kAudioFormatLinearPCM, exit, err = kUnsupportedErr );
	
	*outAudioConverter = (AudioConverterRef) me;
	me = NULL;
	
exit:
	if( me ) AudioConverterDispose_compat( (AudioConverterRef) me );
	return( err );
}

//===========================================================================================================================
//	AudioConverterDispose_compat
//===========================================================================================================================

OSStatus	AudioConverterDispose_compat( AudioConverterRef inConverter )
{
	AudioConverterPrivateRef const		me = (AudioConverterPrivateRef) inConverter;
	
#if( AUDIO_CONVERTER_ALAC )
	if( me->alacDecoder )
	{
		AppleLosslessDecoder_Delete( me->alacDecoder );
		me->alacDecoder = NULL;
	}
#endif
	free( me );
	return( kNoErr );
}

//===========================================================================================================================
//	AudioConverterReset_compat
//===========================================================================================================================

OSStatus	AudioConverterReset_compat( AudioConverterRef me )
{
	(void) me;
	
	// Nothing to do here.
	
	return( kNoErr );
}

//===========================================================================================================================
//	AudioConverterSetProperty_compat
//===========================================================================================================================

OSStatus
	AudioConverterSetProperty_compat( 
		AudioConverterRef			inConverter,
		AudioConverterPropertyID	inPropertyID,
		UInt32						inSize,
		const void *				inData )
{
	AudioConverterPrivateRef const		me = (AudioConverterPrivateRef) inConverter;
	OSStatus							err;
	
#if( !AUDIO_CONVERTER_ALAC )
	(void) me;
	(void) inPropertyID;
	(void) inSize;
	(void) inData;
#endif
	
	if( 0 ) {}
#if( AUDIO_CONVERTER_ALAC )
	else if( inPropertyID == kAudioConverterDecompressionMagicCookie )
	{
		const ALACParams * const		alacParams = (const ALACParams *) inData;
		
		require_action( inSize == sizeof( *alacParams ), exit, err = kSizeErr );
		require_action( me->alacDecoder, exit, err = kIncompatibleErr );
		
		err = AppleLosslessDecoder_Configure( me->alacDecoder, alacParams );
		require_noerr( err, exit );
	}
#endif
	else
	{
		err = kUnsupportedErr;
		goto exit;
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	AudioConverterFillComplexBuffer_compat
//===========================================================================================================================

OSStatus
	AudioConverterFillComplexBuffer_compat( 
		AudioConverterRef					inConverter,
		AudioConverterComplexInputDataProc	inInputDataProc,
		void *								inInputDataProcUserData,
		UInt32 *							ioOutputDataPacketSize,
		AudioBufferList *					outOutputData,
		AudioStreamPacketDescription *		outPacketDescription )
{
#if( AUDIO_CONVERTER_ALAC )
	AudioConverterPrivateRef const		me = (AudioConverterPrivateRef) inConverter;
	OSStatus							err;
	BitBuffer							bits;
	AudioBufferList						bufferList;
	UInt32								packetCount;
	AudioStreamPacketDescription *		packetDesc;
	uint32_t							sampleCount;
	
	(void) outPacketDescription;
	
	packetCount = 1;
	packetDesc  = NULL;
	err = inInputDataProc( inConverter, &packetCount, &bufferList, &packetDesc, inInputDataProcUserData );
	require_noerr_quiet( err, exit );
	
	BitBufferInit( &bits, (uint8_t *) bufferList.mBuffers[ 0 ].mData, bufferList.mBuffers[ 0 ].mDataByteSize );
	sampleCount = *ioOutputDataPacketSize;
	err = AppleLosslessDecoder_Decode( me->alacDecoder, &bits, (uint8_t *) outOutputData->mBuffers[ 0 ].mData, 
		sampleCount, 2, &sampleCount );
	require_noerr_quiet( err, exit );
	
	*ioOutputDataPacketSize = sampleCount;
	
exit:
	return( err );
#else
	(void) inConverter;
	(void) inInputDataProc;
	(void) inInputDataProcUserData;
	(void) ioOutputDataPacketSize;
	(void) outOutputData;
	(void) outPacketDescription;
	
	return( kUnsupportedErr );
#endif
}

#endif // AUDIO_CONVERTER_LITE_ENABLED
