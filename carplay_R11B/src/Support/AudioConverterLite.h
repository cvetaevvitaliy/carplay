/*
	File:    	AudioConverterLite.h
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

#ifndef	__AudioConverterLite_h__
#define	__AudioConverterLite_h__

#include "APSCommonServices.h"

#if( AUDIO_CONVERTER_LITE_ENABLED )

// If we have the AudioToolbox framework, include it to get all its types then though we won't use its APIs.
// This is needed to avoid conflicts if we defined our own versions then somebody else included AudioToolbox.h.

#if( __has_include( <AudioToolbox/AudioToolbox.h> ) )
	#include <AudioToolbox/AudioToolbox.h>
	
	#define HAS_AUDIO_TOOLBOX		1
#endif

#ifdef	__cplusplus
extern "C" {
#endif

//===========================================================================================================================
//	Types
//===========================================================================================================================

#if( !HAS_AUDIO_TOOLBOX )

typedef struct AudioConverterPrivate *		AudioConverterRef;
typedef UInt32								AudioConverterPropertyID;

typedef struct
{
	UInt32		mNumberChannels;
	UInt32		mDataByteSize;
	void *		mData;
	
}	AudioBuffer;

typedef struct
{
	UInt32			mNumberBuffers;
	AudioBuffer		mBuffers[ 1 ];
	
}	AudioBufferList;

typedef struct
{
    int64_t		mStartOffset;
    UInt32		mVariableFramesInPacket;
    UInt32		mDataByteSize;
    
}	AudioStreamPacketDescription;

typedef OSStatus
	( *AudioConverterComplexInputDataProc )( 
		AudioConverterRef				inAudioConverter,
		UInt32 *						ioNumberDataPackets,
		AudioBufferList *				ioData,
		AudioStreamPacketDescription **	outDataPacketDescription,
		void *							inUserData );

#endif // HAS_AUDIO_TOOLBOX

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

OSStatus
	AudioConverterNew_compat( 
		const AudioStreamBasicDescription *	inSourceFormat,
		const AudioStreamBasicDescription *	inDestinationFormat,
		AudioConverterRef *					outAudioConverter );
OSStatus	AudioConverterDispose_compat( AudioConverterRef inAudioConverter );
OSStatus	AudioConverterReset_compat( AudioConverterRef inAudioConverter );
OSStatus
	AudioConverterSetProperty_compat( 
		AudioConverterRef			inAudioConverter,
		AudioConverterPropertyID	inPropertyID,
		UInt32						inSize,
		const void *				inData );
OSStatus
	AudioConverterFillComplexBuffer_compat( 
		AudioConverterRef					inAudioConverter,
		AudioConverterComplexInputDataProc	inInputDataProc,
		void *								inInputDataProcUserData,
		UInt32 *							ioOutputDataPacketSize,
		AudioBufferList *					outOutputData,
		AudioStreamPacketDescription *		outPacketDescription );

// The AudioToolbox headers are available, map our compat functions to real functions so they'll call the lite versions.
// Otherwise, map the real functions to the compat functions so the people can use the real versions in both cases.

#if( HAS_AUDIO_TOOLBOX )
	#define AudioConverterNew_compat					AudioConverterNew
	#define AudioConverterDispose_compat				AudioConverterDispose
	#define AudioConverterReset_compat					AudioConverterReset
	#define AudioConverterSetProperty_compat			AudioConverterSetProperty
	#define AudioConverterFillComplexBuffer_compat		AudioConverterFillComplexBuffer
#else
	#define AudioConverterNew							AudioConverterNew_compat
	#define AudioConverterDispose						AudioConverterDispose_compat
	#define AudioConverterReset							AudioConverterReset_compat
	#define AudioConverterSetProperty					AudioConverterSetProperty_compat
	#define AudioConverterFillComplexBuffer				AudioConverterFillComplexBuffer_compat
#endif

#endif // AUDIO_CONVERTER_LITE_ENABLED

#ifdef __cplusplus
}
#endif

#endif	// __AudioConverterLite_h__
