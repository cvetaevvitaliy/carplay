/*
	File:    	AppleLosslessDecoder.c
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
	
	Copyright (C) 2004-2014 Apple Inc. All Rights Reserved.
*/

#include "AppleLosslessDecoder.h"

#include "aglib.h"
#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "BitUtilities.h"
#include "dplib.h"
#include "matrixlib.h"

//===========================================================================================================================
//	Internals
//===========================================================================================================================

enum
{
	ID_SCE = 0x0,	// single channel element
	ID_CPE,			// channel_pair_element
	ID_CCE,			// coupling_channel_element
	ID_LFE,			// lfe_channel_element
	ID_DSE,			// data_stream_element
	ID_PCE,			// program_config_element
	ID_FIL,			// fill_element
	ID_END			// terminator
};

struct AppleLosslessDecoder
{
	ALACParams		config;
	int32_t *		mixBufferU;
	int32_t *		mixBufferV;
	int32_t *		predictor;
	uint16_t *		shiftBuffer;
};

static OSStatus	DataStreamElement( BitBuffer *inBits );
static OSStatus	FillElement( BitBuffer *inBits );

static void	Zero16( int16_t *buffer, uint32_t numItems, uint32_t stride );
static void	Zero24( uint8_t *buffer, uint32_t numItems, uint32_t stride );
static void	Zero32( int32_t *buffer, uint32_t numItems, uint32_t stride );

//===========================================================================================================================
//	AppleLosslessDecoder_Create
//===========================================================================================================================

OSStatus	AppleLosslessDecoder_Create( AppleLosslessDecoderRef *outDecoder )
{
	OSStatus					err;
	AppleLosslessDecoderRef		obj;
	
	obj = (AppleLosslessDecoderRef) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	*outDecoder = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AppleLosslessDecoder_Delete
//===========================================================================================================================

void	AppleLosslessDecoder_Delete( AppleLosslessDecoderRef inDecoder )
{
	if( inDecoder->mixBufferU )		free( inDecoder->mixBufferU );
	if( inDecoder->mixBufferV )		free( inDecoder->mixBufferV );
	if( inDecoder->predictor )		free( inDecoder->predictor );
	free( inDecoder );
}

//===========================================================================================================================
//	AppleLosslessDecoder_Configure
//===========================================================================================================================

OSStatus	AppleLosslessDecoder_Configure( AppleLosslessDecoderRef inDecoder, const ALACParams *inParams )
{
	OSStatus		err;

	inDecoder->config				= *inParams;
	inDecoder->config.frameLength	= ntohl( inDecoder->config.frameLength );
	inDecoder->config.maxRun		= ntohs( inDecoder->config.maxRun );
	inDecoder->config.maxFrameBytes	= ntohl( inDecoder->config.maxFrameBytes );
	inDecoder->config.avgBitRate	= ntohl( inDecoder->config.avgBitRate );
	inDecoder->config.sampleRate	= ntohl( inDecoder->config.sampleRate );
	
	ForgetMem( &inDecoder->mixBufferU );
	inDecoder->mixBufferU = (int32_t *) malloc( inDecoder->config.frameLength * sizeof( int32_t ) );
	require_action( inDecoder->mixBufferU, exit, err = kNoMemoryErr );
	
	ForgetMem( &inDecoder->mixBufferV );
	inDecoder->mixBufferV = (int32_t *) malloc( inDecoder->config.frameLength * sizeof( int32_t ) );
	require_action( inDecoder->mixBufferV, exit, err = kNoMemoryErr );
	
	ForgetMem( &inDecoder->predictor );
	inDecoder->predictor = (int32_t *) malloc( inDecoder->config.frameLength * sizeof( int32_t ) );
	require_action( inDecoder->predictor, exit, err = kNoMemoryErr );
	inDecoder->shiftBuffer = (uint16_t *) inDecoder->predictor; // Shares memory with predictor buffer.
	
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	AppleLosslessDecoder_Decode
//===========================================================================================================================

OSStatus
	AppleLosslessDecoder_Decode( 
		AppleLosslessDecoderRef	inDecoder, 
		struct BitBuffer *		inBits, 
		uint8_t *				sampleBuffer, 
		uint32_t				numSamples, 
		uint32_t				numChannels, 
		uint32_t *				outNumSamples )
{
	int32_t * const		predictor   = inDecoder->predictor;
	int32_t * const		mixBufferU  = inDecoder->mixBufferU;
	int32_t * const		mixBufferV  = inDecoder->mixBufferV;
	uint16_t * const	shiftBuffer = inDecoder->shiftBuffer;
	BitBuffer			shiftBits;
	unsigned int		bits1, bits2;
	uint8_t				tag;
	AGParamRec			agParams;
	uint32_t			channelIndex;
	int16_t				coefsU[32];		// max possible size is 32 although NUMCOEPAIRS is the current limit
	int16_t				coefsV[32];
	uint8_t				numU, numV;
	uint8_t				mixBits;
	int8_t				mixRes;
	uint16_t			unusedHeader;
	uint8_t				escapeFlag;
	uint32_t			chanBits;
	uint8_t				bytesShifted;
	uint32_t			shift;
	uint8_t				modeU, modeV;
	uint32_t			denShiftU, denShiftV;
	uint16_t			pbFactorU, pbFactorV;
	uint16_t			pb;
	int16_t *			out16;
	uint8_t *			out20;
	uint8_t *			out24;
	int32_t *			out32;
	uint8_t				headerByte;
	uint8_t				partialFrame;
	uint32_t			extraBits;
	int32_t				val;
	uint32_t			i, j;
	OSStatus			status;
	
	channelIndex	= 0;
	status = kNoErr;
	*outNumSamples = numSamples;

	while ( status == kNoErr )
	{
		// bail if we ran off the end of the buffer
    	require_action( inBits->cur < inBits->end, exit, status = kFormatErr );

		// copy global decode params for this element
		pb = inDecoder->config.pb;

		// read element tag
		tag = BitBufferReadSmall( inBits, 3 );
		switch ( tag )
		{
			case ID_SCE:
			case ID_LFE:
			{
				// mono/LFE channel
				BitBufferReadSmall( inBits, 4 );

				// read the 12 unused header bits
				unusedHeader = (uint16_t) BitBufferRead( inBits, 12 );
				require_action( unusedHeader == 0, exit, status = kFormatErr );

				// read the 1-bit "partial frame" flag, 2-bit "shift-off" flag & 1-bit "escape" flag
				headerByte = (uint8_t) BitBufferRead( inBits, 4 );
				
				partialFrame = headerByte >> 3;
				
				bytesShifted = (headerByte >> 1) & 0x3u;
				require_action( bytesShifted != 3, exit, status = kFormatErr );;

				shift = bytesShifted * 8;

				escapeFlag = headerByte & 0x1;

				chanBits = inDecoder->config.bitDepth - shift;
				
				// check for partial frame to override requested numSamples
				if ( partialFrame != 0 )
				{
					numSamples  = BitBufferRead( inBits, 16 ) << 16;
					numSamples |= BitBufferRead( inBits, 16 );
				}

				if ( escapeFlag == 0 )
				{
					// compressed frame, read rest of parameters
					mixBits	= (uint8_t) BitBufferRead( inBits, 8 );
					mixRes	= (int8_t) BitBufferRead( inBits, 8 );
					check( (mixBits == 0) && (mixRes == 0) );		// no mixing for mono

					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					modeU		= headerByte >> 4;
					denShiftU	= headerByte & 0xfu;
					
					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					pbFactorU	= headerByte >> 5;
					numU		= headerByte & 0x1fu;

					for ( i = 0; i < numU; i++ )
						coefsU[i] = (int16_t) BitBufferRead( inBits, 16 );
					
					// if shift active, skip the the shift buffer but remember where it starts
					if ( bytesShifted != 0 )
					{
						shiftBits = *inBits;
						BitBufferAdvance( inBits, shift * numSamples ); 
					}

					// decompress
					set_ag_params( &agParams, inDecoder->config.mb, (pb * pbFactorU) / 4, inDecoder->config.kb, numSamples, numSamples, inDecoder->config.maxRun );
					status = dyn_decomp( &agParams, inBits, predictor, numSamples, chanBits, &bits1 );
					require_noerr( status, exit );

					if ( modeU == 0 )
					{
						unpc_block( predictor, mixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block( predictor, predictor, numSamples, NULL, 31, chanBits, 0 );
						unpc_block( predictor, mixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
				}
				else
				{
					check( bytesShifted == 0 );

					// uncompressed frame, copy data into the mix buffer to use common output code
					shift = 32 - chanBits;
					if ( chanBits <= 16 )
					{
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( inBits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mixBufferU[i] = val;
						}
					}
					else
					{
						// BitBufferRead() can't read more than 16 bits at a time so break up the reads
						extraBits = chanBits - 16;
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( inBits, 16 );
							val = (val << 16) >> shift;
							mixBufferU[i] = val | BitBufferRead( inBits, (uint8_t) extraBits );
						}
					}

					bits1 = chanBits * numSamples;
					bytesShifted = 0;
				}

				// now read the shifted values into the shift buffer
				if ( bytesShifted != 0 )
				{
					shift = bytesShifted * 8;
					check( shift <= 16 );

					for ( i = 0; i < numSamples; i++ )
						shiftBuffer[i] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
				}

				// convert 32-bit integers into output buffer
				switch ( inDecoder->config.bitDepth )
				{
					case 16:
						out16 = &((int16_t *)sampleBuffer)[channelIndex];
						for ( i = 0, j = 0; i < numSamples; i++, j += numChannels )
							out16[j] = (int16_t) mixBufferU[i];
						break;
					case 20:
						out20 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						copyPredictorTo20( mixBufferU, out20, numChannels, numSamples );
						break;
					case 24:
						out24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						if ( bytesShifted != 0 )
							copyPredictorTo24Shift( mixBufferU, shiftBuffer, out24, numChannels, numSamples, bytesShifted );
						else
							copyPredictorTo24( mixBufferU, out24, numChannels, numSamples );							
						break;
					case 32:
						out32 = &((int32_t *)sampleBuffer)[channelIndex];
						if ( bytesShifted != 0 )
							copyPredictorTo32Shift( mixBufferU, shiftBuffer, out32, numChannels, numSamples, bytesShifted );
						else
							copyPredictorTo32( mixBufferU, out32, numChannels, numSamples);
						break;
				}
				
				channelIndex += 1;
				*outNumSamples = numSamples;
				break;
			}

			case ID_CPE:
			{
				// if decoding this pair would take us over the max channels limit, bail
				if ( (channelIndex + 2) > numChannels )
					goto NoMoreChannels;

				// stereo channel pair
				BitBufferReadSmall( inBits, 4 );

				// read the 12 unused header bits
				unusedHeader = (uint16_t) BitBufferRead( inBits, 12 );
				require_action( unusedHeader == 0, exit, status = kFormatErr );

				// read the 1-bit "partial frame" flag, 2-bit "shift-off" flag & 1-bit "escape" flag
				headerByte = (uint8_t) BitBufferRead( inBits, 4 );
				
				partialFrame = headerByte >> 3;
				
				bytesShifted = (headerByte >> 1) & 0x3u;
				require_action( bytesShifted != 3, exit, status = kFormatErr );

				shift = bytesShifted * 8;

				escapeFlag = headerByte & 0x1;

				chanBits = inDecoder->config.bitDepth - shift + 1;
				
				// check for partial frame length to override requested numSamples
				if ( partialFrame != 0 )
				{
					numSamples  = BitBufferRead( inBits, 16 ) << 16;
					numSamples |= BitBufferRead( inBits, 16 );
				}

				if ( escapeFlag == 0 )
				{
					// compressed frame, read rest of parameters
					mixBits		= (uint8_t) BitBufferRead( inBits, 8 );
					mixRes		= (int8_t) BitBufferRead( inBits, 8 );

					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					modeU		= headerByte >> 4;
					denShiftU	= headerByte & 0xfu;
					
					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					pbFactorU	= headerByte >> 5;
					numU		= headerByte & 0x1fu;
					for ( i = 0; i < numU; i++ )
						coefsU[i] = (int16_t) BitBufferRead( inBits, 16 );

					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					modeV		= headerByte >> 4;
					denShiftV	= headerByte & 0xfu;
					
					headerByte	= (uint8_t) BitBufferRead( inBits, 8 );
					pbFactorV	= headerByte >> 5;
					numV		= headerByte & 0x1fu;
					for ( i = 0; i < numV; i++ )
						coefsV[i] = (int16_t) BitBufferRead( inBits, 16 );

					// if shift active, skip the interleaved shifted values but remember where they start
					if ( bytesShifted != 0 )
					{
						shiftBits = *inBits;
						BitBufferAdvance( inBits, shift * 2 * numSamples );
					}

					// decompress and run predictor for "left" channel
					set_ag_params( &agParams, inDecoder->config.mb, (pb * pbFactorU) / 4, inDecoder->config.kb, numSamples, numSamples, inDecoder->config.maxRun );
					status = dyn_decomp( &agParams, inBits, predictor, numSamples, chanBits, &bits1 );
					require_noerr( status, exit );

					if ( modeU == 0 )
					{
						unpc_block( predictor, mixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block( predictor, predictor, numSamples, NULL, 31, chanBits, 0 );
						unpc_block( predictor, mixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}

					// decompress and run predictor for "right" channel
					set_ag_params( &agParams, inDecoder->config.mb, (pb * pbFactorV) / 4, inDecoder->config.kb, numSamples, numSamples, inDecoder->config.maxRun );
					status = dyn_decomp( &agParams, inBits, predictor, numSamples, chanBits, &bits2 );
					require_noerr( status, exit );

					if ( modeV == 0 )
					{
						unpc_block( predictor, mixBufferV, numSamples, &coefsV[0], numV, chanBits, denShiftV );
					}
					else
					{
						// the special "numActive == 31" mode can be done in-place
						unpc_block( predictor, predictor, numSamples, NULL, 31, chanBits, 0 );
						unpc_block( predictor, mixBufferV, numSamples, &coefsV[0], numV, chanBits, denShiftV );
					}
				}
				else
				{
					check( bytesShifted == 0 );

					// uncompressed frame, copy data into the mix buffers to use common output code
					chanBits = inDecoder->config.bitDepth;
					shift = 32 - chanBits;
					if ( chanBits <= 16 )
					{
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( inBits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mixBufferU[i] = val;

							val = (int32_t) BitBufferRead( inBits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mixBufferV[i] = val;
						}
					}
					else
					{
						// BitBufferRead() can't read more than 16 bits at a time so break up the reads
						extraBits = chanBits - 16;
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( inBits, 16 );
							val = (val << 16) >> shift;
							mixBufferU[i] = val | BitBufferRead( inBits, (uint8_t)extraBits );

							val = (int32_t) BitBufferRead( inBits, 16 );
							val = (val << 16) >> shift;
							mixBufferV[i] = val | BitBufferRead( inBits, (uint8_t)extraBits );
						}
					}

					bits1 = chanBits * numSamples;
					bits2 = chanBits * numSamples;
					mixBits = mixRes = 0;
					bytesShifted = 0;
				}

				// now read the shifted values into the shift buffer
				if ( bytesShifted != 0 )
				{
					shift = bytesShifted * 8;
					check( shift <= 16 );

					for ( i = 0; i < (numSamples * 2); i += 2 )
					{
						shiftBuffer[i + 0] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
						shiftBuffer[i + 1] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
					}
				}

				// un-mix the data and convert to output format
				// - note that mixRes = 0 means just interleave so we use that path for uncompressed frames
				switch ( inDecoder->config.bitDepth )
				{
					case 16:
						out16 = &((int16_t *)sampleBuffer)[channelIndex];
						unmix16( mixBufferU, mixBufferV, out16, numChannels, numSamples, mixBits, mixRes );
						break;
					case 20:
						out20 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						unmix20( mixBufferU, mixBufferV, out20, numChannels, numSamples, mixBits, mixRes );
						break;
					case 24:
						out24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						unmix24( mixBufferU, mixBufferV, out24, numChannels, numSamples,
									mixBits, mixRes, shiftBuffer, bytesShifted );
						break;
					case 32:
						out32 = &((int32_t *)sampleBuffer)[channelIndex];
						unmix32( mixBufferU, mixBufferV, out32, numChannels, numSamples,
									mixBits, mixRes, shiftBuffer, bytesShifted );
						break;
				}
				
				channelIndex += 2;
				*outNumSamples = numSamples;
				break;
			}

			case ID_CCE:
			case ID_PCE:
			{
				// unsupported element, bail
				check_noerr( tag );
				status = kFormatErr;
				break;
			}

			case ID_DSE:
			{
				// data stream element -- parse but ignore
				status = DataStreamElement( inBits );
				break;
			}
			
			case ID_FIL:
			{
				// fill element -- parse but ignore
				status = FillElement( inBits );
				break;
			}

			case ID_END:
			{
				// frame end, all done so byte align the frame and check for overruns
				BitBufferByteAlign( inBits, false );
				check( inBits->cur == inBits->end );
				goto exit;
			}
		}

		// if we've decoded all of our channels, bail (but not in debug b/c we want to know if we're seeing bad bits)
		// - this also protects us if the config does not match the bitstream or crap data bits follow the audio bits
		if ( channelIndex >= numChannels )
			break;
	}

NoMoreChannels:

	// if we get here and haven't decoded all of the requested channels, fill the remaining channels with zeros
	for ( ; channelIndex < numChannels; channelIndex++ )
	{
		switch ( inDecoder->config.bitDepth )
		{
			case 16:
			{
				int16_t *	fill16 = &((int16_t *)sampleBuffer)[channelIndex];
				Zero16( fill16, numSamples, numChannels );
				break;
			}
			case 24:
			{
				uint8_t *	fill24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
				Zero24( fill24, numSamples, numChannels );
				break;
			}
			case 32:
			{
				int32_t *	fill32 = &((int32_t *)sampleBuffer)[channelIndex];
				Zero32( fill32, numSamples, numChannels );
				break;
			}
		}
	}

exit:
	return status;
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	DataStreamElement -- we don't care about data stream elements so just skip them
//===========================================================================================================================

static OSStatus	DataStreamElement( BitBuffer *inBits )
{
	Boolean		data_byte_align_flag;
	uint16_t	count;
	
	// the tag associates this data stream element with a given audio element
	(void) BitBufferReadSmall( inBits, 4 );
	
	data_byte_align_flag = BitBufferReadOne( inBits );

	// 8-bit count or (8-bit + 8-bit count) if 8-bit count == 255
	count = BitBufferReadSmall( inBits, 8 );
	if ( count == 255 )
		count += BitBufferReadSmall( inBits, 8 );

	// the align flag means the bitstream should be byte-aligned before reading the following data bytes
	if ( data_byte_align_flag )
		BitBufferByteAlign( inBits, false );

	// skip the data bytes
	BitBufferAdvance( inBits, count * 8 );

	require( inBits->cur <= inBits->end, error );

	return kNoErr;

error:
	return kFormatErr;
}

//===========================================================================================================================
//	FillElement -- they're just filler so we don't need 'em.
//===========================================================================================================================

static OSStatus	FillElement( BitBuffer *inBits )
{
	int16_t		count;
	
	// 4-bit count or (4-bit + 8-bit count) if 4-bit count == 15
	// - plus this weird -1 thing I still don't fully understand
	count = BitBufferReadSmall( inBits, 4 );
	if ( count == 15 )
		count += (int16_t) BitBufferReadSmall( inBits, 8 ) - 1;

	BitBufferAdvance( inBits, count * 8 );

	require( inBits->cur <= inBits->end, error );

	return kNoErr;

error:
	return kFormatErr;
}

//===========================================================================================================================
//	ZeroN -- helper routines to clear out output channel buffers when decoding fewer channels than requested
//===========================================================================================================================

static void	Zero16( int16_t *buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * sizeof(int16_t) );
	}
	else
	{
		size_t		i, n;
		
		n = numItems * stride;
		for ( i = 0; i < n; i += stride )
			buffer[i] = 0;
	}
}

static void	Zero24( uint8_t *buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * 3 );
	}
	else
	{
		size_t		i, n;
		
		stride *= 3;
		n = numItems * stride;
		for ( i = 0; i < n; i += stride )
		{
			buffer[i + 0] = 0;
			buffer[i + 1] = 0;
			buffer[i + 2] = 0;
		}
	}
}

static void	Zero32( int32_t *buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * sizeof(int32_t) );
	}
	else
	{
		size_t		i, n;
		
		n = numItems * stride;
		for ( i = 0; i < n; i += stride )
			buffer[i] = 0;
	}
}
