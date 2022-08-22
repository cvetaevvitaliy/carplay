/*
	File:    	Base64Utils.c
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
	
	Copyright (C) 2002-2014 Apple Inc. All Rights Reserved.
*/

#include "APSCommonServices.h"	// Include early for TARGET_*, etc. definitions.
#include "APSDebugServices.h"	// Include early for DEBUG_*, etc. definitions.

#if( TARGET_HAS_STD_C_LIB )
	#include <ctype.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#include "Base64Utils.h"

/*===========================================================================================================================
	Base64 is documented in RFC 4648. The following is a paraphrase of RFC 4648:
	
	Base64 is a 65-character subset of US-ASCII, enabling 6 bits to be represented per printable character. The extra 
	65th character, "=", is used as a pad character to round the output to an even multiple of 4 bytes.
	
	The encoding process represents 24-bit groups of input bits as output strings of 4 encoded characters. Proceeding 
	from left to right, a 24-bit input group is formed by concatenating 3 8-bit input groups. These 24 bits are then 
	treated as 4 concatenated 6-bit groups, each of which is translated into a single digit in the base64 alphabet.

	Each 6-bit group is used as an index into an array of 64 printable characters. The character referenced by the index 
	is placed in the output string.

	Base64 Alphabet

	Value Encoding  Value Encoding  Value Encoding  Value Encoding
	    0 A            17 R            34 i            51 z
	    1 B            18 S            35 j            52 0
	    2 C            19 T            36 k            53 1
	    3 D            20 U            37 l            54 2
	    4 E            21 V            38 m            55 3
	    5 F            22 W            39 n            56 4
	    6 G            23 X            40 o            57 5
	    7 H            24 Y            41 p            58 6
	    8 I            25 Z            42 q            59 7
	    9 J            26 a            43 r            60 8
	   10 K            27 b            44 s            61 9
	   11 L            28 c            45 t            62 +
	   12 M            29 d            46 u            63 /
	   13 N            30 e            47 v
	   14 O            31 f            48 w         (pad) =
	   15 P            32 g            49 x
	   16 Q            33 h            50 y

	Special processing is performed if fewer than 24 bits are available at the end of the data being encoded. A full 
	encoding quantum is always completed at the end of a quantity. When fewer than 24 input bits are available in an 
	input group, zero bits are added (on the right) to form an integral number of 6-bit groups. Padding at the end of 
	the data is performed using the '=' character.
	
	base64url Alphabet:
	
	Same as the normal Base64 alphabet except:
	
	Value Encoding
	   62 - (minus)
	   63 _ (underline)
*/

//===========================================================================================================================
//	Constants
//===========================================================================================================================

static const uint8_t		kBase64EncodeTable[]		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uint8_t		kBase64EncodeTable_URL[]	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const uint8_t		kBase64DecodeTable[] =
{
	// Values 0x00-0x3F (0-63 inclusive) are valid Base64 values.
	// Value 0x40 is a special flag meaning the Base64 pad character (=).
	// Value 0x80 is a special flag meaning to ignore the character (i.e. whitespace).
	// Value 0xFF is a special flag meaning invalid (e.g. non-Base64 and non-whitespace).
	
//	0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0xFF,	// 0x00-0x0F /   0 -  15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0x10-0x1F /  15 -  31
	0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0x3E, 0xFF, 0x3F,	// 0x20-0x2F /  32 -  47
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF,	// 0x30-0x3F /  48 -  63
	0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,	// 0x40-0x4F /  64 -  79
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,	// 0x50-0x5F /  80 -  95
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,	// 0x60-0x6F /  96 - 111
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0x70-0x7F / 112 - 127
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0x80-0x8F / 128 - 143
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0x90-0x9F / 144 - 159
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0xA0-0xAF / 160 - 175
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0xB0-0xBF / 176 - 191
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0xC0-0xCF / 192 - 207
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0xD0-0xDF / 208 - 223
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// 0xE0-0xEF / 224 - 239
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF	// 0xF0-0xFF / 240 - 255
};

#define	kBase64PadFlag				0x40
#define	kBase64IgnoreFlag			0x80
#define	kBase64InvalidFlag			0xFF

#define	kBase64MaxLineSize			72		// Evenly divisible by 4 for 3->4 Base64 encoding.

//===========================================================================================================================
//	Base64EncodeEx
//===========================================================================================================================

OSStatus
	Base64EncodeEx( 
		const void *	inSourceData, 
		size_t			inSourceSize, 
		Base64Flags		inFlags, 
		void *			inEncodedDataBuffer, 
		size_t			inEncodedDataBufferSize, 
		size_t *		outEncodedSize )
{
	OSStatus			err;
	const uint8_t *		table;
	const uint8_t *		src;
	const uint8_t *		end;
	uint8_t *			dst;
	size_t				encodedSize;
	
	check( inSourceData );
	check( inEncodedDataBuffer );
	if( inSourceSize == kSizeCString) inSourceSize = strlen( (const char *) inSourceData );
	
	if( inFlags & kBase64Flag_URLCharSet )	table = kBase64EncodeTable_URL;
	else									table = kBase64EncodeTable;
	
	src = (const uint8_t *) inSourceData;
	end = src + inSourceSize;
	dst = (uint8_t *) inEncodedDataBuffer;
	
	encodedSize = Base64EncodedMaxSize( inSourceSize );
	require_action( encodedSize >= inSourceSize, exit, err = kSizeErr ); // Detect wrap due to overflow.
	require_action_quiet( encodedSize <= inEncodedDataBufferSize, exit, err = kOverrunErr );
	
	// Process all 3 byte chunks into 4 byte encoded chunks.
	
	while( ( end - src ) >= 3 )
	{
		dst[ 0 ] = table[     src[ 0 ]          >> 2 ];
		dst[ 1 ] = table[ ( ( src[ 0 ] & 0x03 ) << 4 ) + ( src[ 1 ] >> 4 ) ];
		dst[ 2 ] = table[ ( ( src[ 1 ] & 0x0F ) << 2 ) + ( src[ 2 ] >> 6 ) ];
		dst[ 3 ] = table[     src[ 2 ] & 0x3F ];
		src		+= 3;
		dst 	+= 4;
	}
    
	// Process any remaining 1 or 2 bytes into a 4 byte chunk, padding with 1 or 2 '=' characters as needed.
	
	switch( end - src )
	{
		case 1:
			*dst++ = table[   src[ 0 ]          >> 2 ];
			*dst++ = table[ ( src[ 0 ] & 0x03 ) << 4 ];
			if( !( inFlags & kBase64Flag_NoPadding ) )
			{
				*dst++ = '=';
				*dst++ = '=';
			}
			break;
			
		case 2:
			*dst++ = table[     src[ 0 ]          >> 2 ];
			*dst++ = table[ ( ( src[ 0 ] & 0x03 ) << 4 ) + ( src[ 1 ] >> 4 ) ];
			*dst++ = table[   ( src[ 1 ] & 0x0F ) << 2 ];
			if( !( inFlags & kBase64Flag_NoPadding ) )
			{
				*dst++ = '=';
			}
			break;
			
		default:
			check( ( end - src ) == 0 );
			break;
	}
	check( ( (size_t)( dst - ( (uint8_t *) inEncodedDataBuffer ) ) ) <= encodedSize );
	err = kNoErr;
	
exit:
	if( outEncodedSize ) *outEncodedSize = (size_t)( dst - ( (uint8_t *) inEncodedDataBuffer ) );
	return( err );
}

//===========================================================================================================================
//	Base64EncodeCopy
//===========================================================================================================================

OSStatus
	Base64EncodeCopyEx( 
		const void *	inSourceData, 
		size_t			inSourceSize, 
		Base64Flags		inFlags, 
		void *			outEncodedData, 
		size_t *		outEncodedSize )
{
	OSStatus		err;
	size_t			n;
	uint8_t *		dst = NULL;
	
	check( inSourceData );
	check( outEncodedData );
	if( inSourceSize == kSizeCString) inSourceSize = strlen( (const char *) inSourceData );
	
	// Allocate a buffer big enough to hold the Base64 encoded data and a null terminator.
	
	n = Base64EncodedMaxSize( inSourceSize );
	require_action( n >= inSourceSize, exit, err = kSizeErr ); // Detect wrap due to overflow.
	require_action( n < SIZE_MAX, exit, err = kSizeErr );
	
	dst = (uint8_t *) malloc( n + 1 );
	require_action( dst, exit, err = kNoMemoryErr );
	
	// Encode the data into the buffer and null terminate the result.
	
	err = Base64EncodeEx( inSourceData, inSourceSize, inFlags, dst, n, &n );
	require_noerr( err, exit );
	dst[ n ] = '\0';
	
	*( (uint8_t **) outEncodedData ) = dst;
	dst = NULL;
	if( outEncodedSize ) *outEncodedSize = n;
	err = kNoErr;
	
exit:
	if( dst ) free( dst );
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	Base64EncodedLinesMaxSize
//===========================================================================================================================

size_t	Base64EncodedLinesMaxSize( size_t inSize, size_t inIndentCount, const char *inLineEnding )
{
	size_t				size;
	size_t				lines;
	const char *		p;
	
	check( inLineEnding );
	
	// Calculate the normal Base64 size. Every 3 bytes are turned into 4 bytes and +2 to account for padding.
	
	size = ( ( inSize + 2 ) / 3 ) * 4;
	
	// Calculate the number of lines.
	
	lines = size / kBase64MaxLineSize;
	if( ( size % kBase64MaxLineSize ) != 0 ) lines += 1;
	
	// Add the line indent and line ending size for each line.
	
	size += ( lines * inIndentCount );
	for( p = inLineEnding; *p != '\0'; ++p ) {}
	size += ( lines * (size_t)( p - inLineEnding ) );
	
	return( size );
}

//===========================================================================================================================
//	Base64EncodeLines
//===========================================================================================================================

OSStatus
	Base64EncodeLinesEx( 
		const void *	inSourceData, 
		size_t 			inSourceSize, 
		size_t 			inIndentCount, 
		const char *	inLineEnding, 
		Base64Flags		inFlags, 
		void *			inEncodedDataBuffer, 
		size_t			inEncodedDataBufferSize, 
		size_t *		outEncodedSize )
{
	OSStatus			err;
	const uint8_t *		table;
	const uint8_t *		src;
	const uint8_t *		end;
	uint8_t *			dst;
	size_t				lineSize;
	size_t				encodedSize;
	const char *		p;
	size_t				i;
	
	check( inSourceData );
	check( inLineEnding );
	check( inEncodedDataBuffer );
	if( inSourceSize == kSizeCString) inSourceSize = strlen( (const char *) inSourceData );
	
	if( inFlags & kBase64Flag_URLCharSet )	table = kBase64EncodeTable_URL;
	else									table = kBase64EncodeTable;
	
	src			= (const uint8_t *) inSourceData;
	end			= src + inSourceSize;
	dst			= (uint8_t *) inEncodedDataBuffer;
	lineSize	= 0;
	
	encodedSize = Base64EncodedLinesMaxSize( inSourceSize, inIndentCount, inLineEnding );
	require_action( encodedSize >= inSourceSize, exit, err = kSizeErr ); // Detect wrap due to overflow.
	require_action_quiet( encodedSize <= inEncodedDataBufferSize, exit, err = kOverrunErr );
	
	// Indent the first line up front so main loop only needs to handle continuation line indents.
	
	for( i = 0; i < inIndentCount; ++i ) *dst++ = '\t';
	
	// Process all 3 byte chunks into 4 byte encoded chunks.
	
	if( ( end - src ) >= 3 )
	{
		while( ( end - src ) >= 3 )
		{
			// Add a new-line then indent if we've hit the max line size.
			
			if( lineSize >= kBase64MaxLineSize )
			{
				for( p = inLineEnding; *p != '\0'; ++p ) *dst++ = (uint8_t) *p;
				for( i = 0; i < inIndentCount; ++i )	 *dst++ = '\t';
				lineSize = 0;
			}
			
			// Encode the 3-byte chunk into a 4-byte encoded chunk.
			
			dst[ 0 ]  = table[     src[ 0 ]          >> 2 ];
			dst[ 1 ]  = table[ ( ( src[ 0 ] & 0x03 ) << 4 ) + ( src[ 1 ] >> 4 ) ];
			dst[ 2 ]  = table[ ( ( src[ 1 ] & 0x0F ) << 2 ) + ( src[ 2 ] >> 6 ) ];
			dst[ 3 ]  = table[     src[ 2 ] & 0x3F ];
			src 	 += 3;
			dst		 += 4;
			lineSize += 4;
		}
    }
    
    // Add a new-line and indent if we have more data and we've hit the max line size. This is to handle the 
    // case where the last processed chunk ended exactly on the max line size.
	
	if( ( ( end - src ) > 0 ) && ( lineSize >= kBase64MaxLineSize ) )
	{
		for( p = inLineEnding; *p != '\0'; ++p ) *dst++ = (uint8_t) *p;
		for( i = 0; i < inIndentCount; ++i )	 *dst++ = '\t';
	}
	
	// Process any remaining 1 or 2 bytes into a 4 byte chunk, padding with 1 or 2 '=' characters as needed.
	
	switch( end - src )
	{
		case 1:
			*dst++ = table[   src[ 0 ]          >> 2 ];
			*dst++ = table[ ( src[ 0 ] & 0x03 ) << 4 ];
			if( !( inFlags & kBase64Flag_NoPadding ) )
			{
				*dst++ = '=';
				*dst++ = '=';
			}
			break;
			
		case 2:
			*dst++ = table[     src[ 0 ]          >> 2 ];
			*dst++ = table[ ( ( src[ 0 ] & 0x03 ) << 4 ) + ( src[ 1 ] >> 4 ) ];
			*dst++ = table[   ( src[ 1 ] & 0x0F ) << 2 ];
			if( !( inFlags & kBase64Flag_NoPadding ) )
			{
				*dst++ = '=';
			}
			break;
			
		default:
			check( ( end - src ) == 0 );
			break;
	}
	
	// Add a final new-line if we encoded any data.
	
	if( dst != ( (uint8_t *) inEncodedDataBuffer ) )
	{
		for( p = inLineEnding; *p != '\0'; ++p ) *dst++ = (uint8_t) *p;
	}
	check( ( (size_t)( dst - ( (uint8_t *) inEncodedDataBuffer ) ) ) <= encodedSize );
	err = kNoErr;
	
exit:
	if( outEncodedSize ) *outEncodedSize = (size_t)( dst - ( (uint8_t *) inEncodedDataBuffer ) );
	return( err );
}

//===========================================================================================================================
//	Base64EncodeLinesCopy
//===========================================================================================================================

OSStatus
	Base64EncodeLinesCopyEx( 
		const void *	inSourceData, 
		size_t 			inSourceSize, 
		size_t 			inIndentCount, 
		const char *	inLineEnding, 
		Base64Flags		inFlags, 
		void *			outEncodedData, 
		size_t *		outEncodedSize )
{
	OSStatus		err;
	size_t			n;
	uint8_t *		dst = NULL;
	
	check( inSourceData );
	check( inLineEnding );
	check( outEncodedData );
	if( inSourceSize == kSizeCString) inSourceSize = strlen( (const char *) inSourceData );
	
	// Allocate a buffer big enough to hold the Base64 encoded data and a null terminator.
	
	n = Base64EncodedLinesMaxSize( inSourceSize, inIndentCount, inLineEnding );
	require_action( n >= inSourceSize, exit, err = kSizeErr ); // Detect wrap due to overflow.
	require_action( n < SIZE_MAX, exit, err = kSizeErr );
	
	dst = (uint8_t *) malloc( n + 1 );
	require_action( dst, exit, err = kNoMemoryErr );
	
	// Encode the data into the buffer and null terminate the result.
	
	err = Base64EncodeLinesEx( inSourceData, inSourceSize, inIndentCount, inLineEnding, inFlags, dst, n, &n );
	require_noerr( err, exit );
	dst[ n ] = '\0';
	
	*( (uint8_t **) outEncodedData ) = dst;
	dst = NULL;
	if( outEncodedSize ) *outEncodedSize = n;
	err = kNoErr;
	
exit:
	if( dst ) free( dst );
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	Base64DecodedSize
//===========================================================================================================================

size_t	Base64DecodedSize( const void *inEncodedData, size_t inEncodedSize )
{
	size_t				n;
	const uint8_t *		src;
	const uint8_t *		end;
	int					state;
	uint8_t				b;
	
	check( inEncodedData );
	if( inEncodedSize == kSizeCString) inEncodedSize = strlen( (const char *) inEncodedData );
	
	n		= 0;
	src		= (const uint8_t *) inEncodedData;
	end		= src + inEncodedSize;
	state	= 0;
	
	// Process chunks of 4 encoded characters into 3 decoded bytes.
	
	while( src < end )
	{
		b = kBase64DecodeTable[ *src++ ];
		if( b == kBase64InvalidFlag ) goto exit;
		if( b == kBase64IgnoreFlag )  continue;
		if( b == kBase64PadFlag )     break;
		
		switch( state )
		{
			case 0: state = 1; break;
			case 1: state = 2; break;	
			case 2: state = 3; break;
			case 3: state = 0; n += 3; break;
			default:
				dlogassert( "impossible state (%d)", state );
				goto exit;
		}
	}
	
	// Process any remaining data and pad characters.

	switch( state )
	{
		case 0: break;
		case 1: break;
		case 2: n += 1; break;
		case 3: n += 2; break;
		default:
			dlogassert( "impossible pad state (%d)", state );
			goto exit;
	}
	
exit:
	return( n );
}

//===========================================================================================================================
//	Base64Decode
//===========================================================================================================================

OSStatus
	Base64Decode( 
		const void *	inEncodedData, 
		size_t 			inEncodedSize, 
		void *			inDecodedDataBuffer, 
		size_t			inDecodedDataBufferSize, 
		size_t *		outDecodedSize )
{
	OSStatus			err;
	const uint8_t *		src;
	const uint8_t *		end;
	uint8_t *			dst;
	uint8_t *			lim;
	uint8_t				buf[ 3 ];
	int					state;
	uint8_t				b;
	
	check( inEncodedData );
	check( inDecodedDataBuffer );
	if( inEncodedSize == kSizeCString) inEncodedSize = strlen( (const char *) inEncodedData );
	
	src		= (const uint8_t *) inEncodedData;
	end		= src + inEncodedSize;
	dst		= (uint8_t *) inDecodedDataBuffer;
	lim		= dst + inDecodedDataBufferSize;
	state	= 0;
	
	// Initialize to workaround GCC 4 warning bug. See <rdar://problem/4028403>.
	
	buf[ 0 ] = 0;
	buf[ 1 ] = 0;
	buf[ 2 ] = 0;
	
	// Process chunks of 4 encoded characters into 3 decoded bytes.
	
	while( src < end )
	{
		b = kBase64DecodeTable[ *src++ ];
		require_action_quiet( b != kBase64InvalidFlag, exit, err = kIntegrityErr );
		if( b == kBase64IgnoreFlag ) continue;
		if( b == kBase64PadFlag )    break;
		
		switch( state )
		{
			case 0:
				buf[ 0 ] = (uint8_t)( b << 2 );
				state	 = 1;
				break;
			
			case 1:
				buf[ 0 ] |= (uint8_t)(   b          >> 4 );
				buf[ 1 ]  = (uint8_t)( ( b & 0x0F ) << 4 );
				state	  = 2;
				break;
				
			case 2:
				buf[ 1 ] |= (uint8_t)(   b          >> 2 );
				buf[ 2 ]  = (uint8_t)( ( b & 0x03 ) << 6 );
				state	  = 3;
				break;
			
			case 3:
				require_action_quiet( ( lim - dst ) >= 3, exit, err = kOverrunErr );
				
				buf[ 2 ] |= b;
				dst[ 0 ]  = buf[ 0 ];
				dst[ 1 ]  = buf[ 1 ];
				dst[ 2 ]  = buf[ 2 ];
				dst		 += 3;
				state	  = 0;
				break;
			
			default:
				dlogassert( "impossible state (%d)", state );
				err = kStateErr;
				goto exit;
		}
	}
	
	// Process any remaining data and pad characters. Note: this is intentionally pretty liberal in what it considers 
	// valid. For example, it does not return an error if the data did not have proper padding, if there was extraneous 
	// padding, or if there were extra characters beyond the padding. This is to work around non-compliant encoders.
	
	switch( state )
	{
		case 0:
		case 1:
			break;
		
		case 2:
			
			// Write the final byte. Make sure the unneeded partial decode byte didn't contain non-zero data.
			
			require_action_quiet( ( lim - dst ) >= 1, exit, err = kOverrunErr );
			
			*dst++ = buf[ 0 ];
			check( buf[ 1 ] == 0 );
			break;
						
		case 3:
			
			// Write the final 2 bytes. Make sure the unneeded partial decode byte didn't contain non-zero data.
			
			require_action_quiet( ( lim - dst ) >= 2, exit, err = kOverrunErr );
			
			dst[ 0 ] = buf[ 0 ];
			dst[ 1 ] = buf[ 1 ];
			dst += 2;
			check( buf[ 2 ] == 0 );
			break;
		
		default:
			dlogassert( "impossible pad state (%d)", state );
			err = kStateErr;
			goto exit;
	}
	err = kNoErr;
	
exit:
	if( outDecodedSize ) *outDecodedSize = (size_t)( dst - ( (uint8_t *) inDecodedDataBuffer ) );
	return( err );
}

//===========================================================================================================================
//	Base64DecodeCopy
//===========================================================================================================================

OSStatus	Base64DecodeCopy( const void *inEncodedData, size_t inEncodedSize, void *outDecodedData, size_t *outDecodedSize )
{
	OSStatus		err;
	size_t			n;
	uint8_t *		dst = NULL;
	
	dst = NULL;
	check( inEncodedData );
	check( outDecodedData );
	if( inEncodedSize == kSizeCString) inEncodedSize = strlen( (const char *) inEncodedData );
	
	// Allocate a buffer big enough to hold the decoded data and a null terminator.
	
	n = inEncodedSize + 3;
	require_action( n > inEncodedSize, exit, err = kSizeErr ); // Detect wrap due to overflow.
	n = ( n / 4 ) * 3;
	require_action( n < SIZE_MAX, exit, err = kSizeErr );
	
	dst = (uint8_t *) malloc( n + 1 );
	require_action( dst, exit, err = kNoMemoryErr );
	
	// Decode the data into the buffer and null terminate it for convenience when dealing with string data.
	
	err = Base64Decode( inEncodedData, inEncodedSize, dst, n, &n );
	require_noerr( err, exit );
	dst[ n ] = '\0';
	
	*( (uint8_t **) outDecodedData ) = dst;
	dst = NULL;
	if( outDecodedSize ) *outDecodedSize = n;
	err = kNoErr;
	
exit:
	if( dst ) free( dst );
	return( err );
}

#if 0
#pragma mark -
#endif

