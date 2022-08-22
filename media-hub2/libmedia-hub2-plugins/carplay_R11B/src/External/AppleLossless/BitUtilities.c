/*
	File:    	BitUtilities.c
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
	
	Bit parsing/writing utilities.
*/

#include "BitUtilities.h"

	#include "APSDebugServices.h"
	
	#define Assert( a )		check( a )
	#define DebugStop( a )	dlog( kLogLevelError | kLogLevelFlagDebugBreak, a )
	
	#define	RequireAction(X, ACTION) \
		do { \
			if( !( X ) ) { \
				dlog( kLogLevelAssert, "ASSERT FAILED: " #X " %s, line %d\n", __FILE__, __LINE__ ); \
				ACTION \
			} \
		} while( 0 )
	
	#define RequireActionSilent( X, ACTION ) \
		do { \
			if( !( X ) ) { \
				ACTION \
			} \
		} while( 0 )
	
	#if( !defined( nil ) )
		#define nil		NULL
	#endif

// BitArrayTestBit
//
Boolean BitArrayTestBit(const uint8_t *bitArray, uint32_t bitIndex)
{
	Assert(bitArray != nil);

	if (bitArray[bitIndex >> 3] & (0x80 >> (bitIndex & 7)))
		return true;
	else
		return false;
}

// BitArraySetBit
//
void BitArraySetBit(uint8_t *bitArray, uint32_t bitIndex)
{
	Assert(bitArray != nil);

	bitArray[bitIndex >> 3] |= (0x80 >> (bitIndex & 7));
}

// BitArraySetBits
//
void BitArraySetBits(uint8_t *bitArray, uint32_t bitIndexStart, uint32_t bitIndexEnd)
{
	uint32_t		bitIndex;

	Assert(bitArray != nil);
	Assert(bitIndexStart <= bitIndexEnd);

	for (bitIndex = bitIndexStart; bitIndex <= bitIndexEnd; bitIndex++)
	{
		bitArray[bitIndex >> 3] |= (0x80 >> (bitIndex & 7));
	}
}

// BitArrayClearBit
//
void BitArrayClearBit(uint8_t *bitArray, uint32_t bitIndex)
{
	Assert(bitArray != nil);

	bitArray[bitIndex >> 3] &= ~(0x80 >> (bitIndex & 7));
}

// BitArrayClearBits
//
void BitArrayClearBits(uint8_t *bitArray, uint32_t bitIndexStart, uint32_t bitIndexEnd)
{
	uint32_t		bitIndex;

	Assert(bitArray != nil);
	Assert(bitIndexStart <= bitIndexEnd);

	for (bitIndex = bitIndexStart; bitIndex <= bitIndexEnd; bitIndex++)
	{
		bitArray[bitIndex >> 3] &= ~(0x80 >> (bitIndex & 7));
	}
}

#if PRAGMA_MARK
#pragma mark -
#endif

// BitBufferInit
//
void BitBufferInit( BitBuffer * bits, uint8_t * buffer, uint32_t byteSize )
{
	bits->cur		= buffer;
	bits->end		= bits->cur + byteSize;
	bits->bitIndex	= 0;
	bits->byteSize	= byteSize;
}

// BitBufferRead
//
uint32_t BitBufferRead( BitBuffer * bits, uint8_t numBits )
{
	uint32_t		returnBits;
	
	Assert( numBits <= 16 );

	returnBits = ((uint32_t)bits->cur[0] << 16) | ((uint32_t)bits->cur[1] << 8) | ((uint32_t)bits->cur[2]);
	returnBits = returnBits << bits->bitIndex;
	returnBits &= 0x00FFFFFF;
	
	bits->bitIndex += numBits;
	
	returnBits = returnBits >> (24 - numBits);
	
	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;
	
	Assert( bits->cur <= bits->end );
	
	return returnBits;
}

// BitBufferReadSmall
//
uint8_t BitBufferReadSmall( BitBuffer * bits, uint8_t numBits )
{
	uint16_t		returnBits;
	
	Assert( numBits <= 8 );
	
	returnBits = (uint16_t)((bits->cur[0] << 8) | bits->cur[1]);
	returnBits = (uint16_t)(returnBits << bits->bitIndex);
	
	bits->bitIndex += numBits;
	
	returnBits = returnBits >> (16 - numBits);
	
	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;
	
	Assert( bits->cur <= bits->end );
	
	return (uint8_t) returnBits;
}

// BitBufferReadOne
//
uint8_t BitBufferReadOne( BitBuffer * bits )
{
	uint8_t		returnBits;

	returnBits = (bits->cur[0] >> (7 - bits->bitIndex)) & 1;

	bits->bitIndex++;
	
	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;
	
	Assert( bits->cur <= bits->end );
	
	return returnBits;
}

// BitBufferPeek
//
uint32_t BitBufferPeek( BitBuffer * bits, uint8_t numBits )
{
	return ((((((uint32_t) bits->cur[0] << 16) | ((uint32_t) bits->cur[1] << 8) |
			((uint32_t) bits->cur[2])) << bits->bitIndex) & 0x00FFFFFF) >> (24 - numBits));
}

// BitBufferPeekOne
//
uint32_t BitBufferPeekOne( BitBuffer * bits )
{
	return ((bits->cur[0] >> (7 - bits->bitIndex)) & 1);
}

// BitBufferUnpackBERSize
//
uint32_t BitBufferUnpackBERSize( BitBuffer * bits )
{
	uint32_t		size;
	uint8_t		tmp;
	
	for ( size = 0, tmp = 0x80u; tmp & 0x80u; size = (size << 7u) | (tmp & 0x7fu) )
		tmp = (uint8_t) BitBufferReadSmall( bits, 8 );
	
	return size;
}

// BitBufferGetPosition
//
uint32_t BitBufferGetPosition( BitBuffer * bits )
{
	uint8_t *		begin;
	
	begin = bits->end - bits->byteSize;
	
	return (uint32_t)(((bits->cur - begin) * 8) + bits->bitIndex);
}

// BitBufferByteAlign
//
void BitBufferByteAlign( BitBuffer * bits, Boolean addZeros )
{
	// align bit buffer to next byte boundary, writing zeros if requested
	if ( bits->bitIndex == 0 )
		return;

	if ( addZeros )
		BitBufferWrite( bits, 0, 8 - bits->bitIndex );
	else	
		BitBufferAdvance( bits, 8 - bits->bitIndex );	
}

// BitBufferAdvance
//
void BitBufferAdvance( BitBuffer * bits, uint32_t numBits )
{
	if ( numBits )
	{
		bits->bitIndex += numBits;
		bits->cur += (bits->bitIndex >> 3);
		bits->bitIndex &= 7;
	}
}

// BitBufferRewind
//
void BitBufferRewind( BitBuffer * bits, uint32_t numBits )
{
	uint32_t	numBytes;
	
	if ( numBits == 0 )
		return;
	
	if ( bits->bitIndex >= numBits )
	{
		bits->bitIndex -= numBits;
		return;
	}
	
	numBits -= bits->bitIndex;
	bits->bitIndex = 0;

	numBytes	= numBits / 8;
	numBits		= numBits % 8;
	
	bits->cur -= numBytes;
	
	if ( numBits > 0 )
	{
		bits->bitIndex = 8 - numBits;
		bits->cur--;
	}
	
	if ( bits->cur < (bits->end - bits->byteSize) )
	{
		DebugStop("BitBufferRewind: Rewound too far.");

		bits->cur		= (bits->end - bits->byteSize);
		bits->bitIndex	= 0;
	}
}

// BitBufferWrite
//
void BitBufferWrite( BitBuffer * bits, uint32_t bitValues, uint32_t numBits )
{
	uint32_t				invBitIndex;
	
	RequireAction( bits != nil, return; );
	RequireActionSilent( numBits > 0, return; );

	invBitIndex = 8 - bits->bitIndex;

	while ( numBits > 0 )
	{
		uint32_t	tmp;
		uint8_t		shift;
		uint8_t		mask;
		uint32_t	curNum;

		curNum = Min( invBitIndex, numBits );

		tmp = bitValues >> (numBits - curNum);

		shift  = (uint8_t) (invBitIndex - curNum);
		mask   = 0xffu >> (8 - curNum);		// must be done in two steps to avoid compiler sequencing ambiguity
		mask <<= shift;

		bits->cur[0] = (bits->cur[0] & ~mask) | (((uint8_t) tmp << shift)  & mask);
		numBits -= curNum;

		// increment to next byte if need be
		invBitIndex -= curNum;
		if ( invBitIndex == 0 )
		{
			invBitIndex = 8;
			bits->cur++;
		}
	}

	bits->bitIndex = 8 - invBitIndex;
	Assert( bits->cur <= bits->end );
}

#if PRAGMA_MARK
#pragma mark -
#endif

#if UNIT_TESTS

#if PRAGMA_MARK
#pragma mark -
#endif

OSStatus TestBitUtilities( void );

OSStatus TestBitUtilities( void )
{
	BitBuffer			bits;
	BitStream			stream;
	uint8_t *			ptr;
	uint32_t			value;
	uint32_t			numBits;
	uint32_t			size;
	uint8_t				outBuf[1024];
	uint32_t			count;
	OSStatus			status;
	
	// BitBuffer tests
	MemClear( &bits, sizeof(bits) );

	BitBufferInit( &bits, outBuf, sizeof(outBuf) );
	
	for ( count = 0; count < (sizeof(outBuf) / sizeof(uint32_t)); count++ )
		BitBufferWrite( &bits, count * 4194304ul, 32 );
	
	BitBufferInit( &bits, outBuf, sizeof(outBuf) );
	
	for ( count = 0; count < (sizeof(outBuf) / sizeof(uint32_t)); count++ )
	{
		value  = BitBufferRead( &bits, 16 ) << 16;
		value |= BitBufferRead( &bits, 16 );
		
		Assert( value == (count * 4194304ul) );
	}
	
	for ( count = 0; count < 8; count++ )
	{
		BitBufferInit( &bits, outBuf, sizeof(outBuf) );
		bits.bitIndex = count;
		
		MemClear( &bits.cur[0], 32 );

		BitBufferWrite( &bits, 0xffff5555, 32 );
		
		BitBufferInit( &bits, outBuf, sizeof(outBuf) );
		bits.bitIndex = count;
		
		value  = BitBufferRead( &bits, 16 ) << 16;
		value |= BitBufferRead( &bits, 16 );
		
		Assert( value == 0xffff5555ul );
	}

	for ( count = 0; count < 8; count++ )
	{
		BitBufferInit( &bits, outBuf, sizeof(outBuf) );
		bits.bitIndex = count;
		
		MemClear( &bits.cur[0], 32 );

		BitBufferWrite( &bits, 0xe5, 8 );
		
		BitBufferInit( &bits, outBuf, sizeof(outBuf) );
		bits.bitIndex = count;
		
		value  = BitBufferReadSmall( &bits, 8 );
		
		Assert( value == 0xe5ul );
	}
	
	// BitStream tests
	MemClear( &stream, sizeof(stream) );

	status = BitStreamOpen( &stream, 256 );
	RequireNoErr( status, goto Exit; );

	count	= 0;
	ptr		= nil;

#if 1
	for ( count = 0; count < 253; count++ )
	{
		numBits = (uint32_t)Random() & 0x1f;	// 0 - 31
		value	= (uint32_t)(Random() << 16) | (uint32_t)Random();

		status = BitStreamWrite( &stream, value, numBits );
		RequireNoErr( status, goto Exit; );
	}
#else
	value 	= 0x6359;
	numBits	= 16;

	status = BitStreamWrite( &stream, value, numBits );
	RequireNoErr( status, goto Exit; );

	ptr = stream.curBuf.end - stream.curBuf.byteSize;
	DebugDumpBytes( ptr, (stream.totalBitIndex + 7) / 8 );

	value	= 0x33;
	numBits = 20;

	status = BitStreamWrite( &stream, value, numBits );
	RequireNoErr( status, goto Exit; );

	ptr = stream.curBuf.end - stream.curBuf.byteSize;
	DebugDumpBytes( ptr, (stream.totalBitIndex + 7) / 8 );
#endif

	DebugMsg( "total bits = %lu\n", stream.totalBitIndex );
	
	size = BitStreamByteSize( &stream );
	status = BitStreamFlatten( &stream, outBuf, size );
	AssertNoErr( status );
	
	DebugDumpBytes( outBuf, size );

Exit:
	BitStreamClose( &stream );

	return status;
}

#endif
