/*
	File:    	CFLiteBinaryPlist.c
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
	
	Copyright (C) 2006-2014 Apple Inc. All Rights Reserved.

	To Do:
		
		- Add stack depth limit when parsing to avoid stack overruns.
*/

#include "CFLiteBinaryPlist.h"

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFCompat.h"

#if( TARGET_HAS_STD_C_LIB )
	#include <limits.h>
	#include <stdarg.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#include CF_HEADER

#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	#include "utfconv.h"
#endif

//===========================================================================================================================
//	Constants
//===========================================================================================================================

#define	kCFLBinaryPlistHeaderSignature			"CFB0"
#define	kCFLBinaryPlistHeaderSignatureSize		sizeof_string( kCFLBinaryPlistHeaderSignature )
#define	kCFLBinaryPlistTrailerSignature			"END!"
#define	kCFLBinaryPlistTrailerSignatureSize		sizeof_string( kCFLBinaryPlistTrailerSignature )

#define	kCFLBinaryPlistMarkerNull				0x00
#define	kCFLBinaryPlistMarkerFalse				0x08
#define	kCFLBinaryPlistMarkerTrue				0x09
#define	kCFLBinaryPlistMarkerFill				0x0F
#define	kCFLBinaryPlistMarkerInt				0x10
#define	kCFLBinaryPlistMarkerReal				0x20
#define	kCFLBinaryPlistMarkerDate				0x30
#define	kCFLBinaryPlistMarkerDateInteger		0x30
#define	kCFLBinaryPlistMarkerDateFloat			0x33
#define	kCFLBinaryPlistMarkerData				0x40
#define	kCFLBinaryPlistMarkerASCIIString		0x50
#define	kCFLBinaryPlistMarkerUnicodeString		0x60
#define	kCFLBinaryPlistMarkerUTF8String			0x70
#define	kCFLBinaryPlistMarkerUID				0x80
#define	kCFLBinaryPlistMarkerArray				0xA0
#define	kCFLBinaryPlistMarkerDictionary			0xD0

//===========================================================================================================================
//	Types
//===========================================================================================================================

typedef struct
{
	CFMutableDataRef			data;
	uint8_t						stringBuf[ 256 ];
	CFMutableArrayRef			array;
	CFMutableDictionaryRef		uniqueDict;
	CFIndex						uniqueCount;
	size_t						bytesWritten;
	uint8_t						objectRefSize;
	uint8_t						offsetIntSize;
	size_t						offsetTableOffset;
	
}	CFBinaryPlistContext;

#define CFBinaryPlistContextInit( CTX ) \
	do \
	{ \
		(CTX)->data					= NULL; \
		(CTX)->array				= NULL; \
		(CTX)->uniqueDict			= NULL; \
		(CTX)->uniqueCount			= 0; \
		(CTX)->bytesWritten			= 0; \
		(CTX)->objectRefSize		= 0; \
		(CTX)->offsetIntSize		= 0; \
		(CTX)->offsetTableOffset	= 0; \
		\
	}	while( 0 )

#define CFBinaryPlistContextFree( CTX ) \
	do \
	{ \
		ForgetCF( &(CTX)->data ); \
		ForgetCF( &(CTX)->array ); \
		ForgetCF( &(CTX)->uniqueDict ); \
		\
	}	while( 0 )

typedef struct
{
	uint8_t			unused[ 5 ];
	uint8_t			sortVersion;
	uint8_t			offsetIntSize;
	uint8_t			objectRefSize;
	uint64_t		numObjects;
	uint64_t		topObject;
	uint64_t		offsetTableOffset;
	
}	CFBinaryPlistTrailer;

check_compile_time( offsetof( CFBinaryPlistTrailer, unused )			==  0 );
check_compile_time( offsetof( CFBinaryPlistTrailer, sortVersion )		==  5 );
check_compile_time( offsetof( CFBinaryPlistTrailer, offsetIntSize )		==  6 );
check_compile_time( offsetof( CFBinaryPlistTrailer, objectRefSize )		==  7 );
check_compile_time( offsetof( CFBinaryPlistTrailer, numObjects )		==  8 );
check_compile_time( offsetof( CFBinaryPlistTrailer, topObject )			== 16 );
check_compile_time( offsetof( CFBinaryPlistTrailer, offsetTableOffset )	== 24 );
check_compile_time( sizeof(   CFBinaryPlistTrailer )					== 32 );

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static void		_GlobalEnsureInitialized( void );

static OSStatus	_WriteObject( CFBinaryPlistContext *inContext, CFTypeRef inObj );
static OSStatus	_WriteInteger( CFBinaryPlistContext *inStream, uint64_t inValue );
static OSStatus	_WriteBytes( CFBinaryPlistContext *inContext, const void *inData, size_t inSize );

static OSStatus	_ReadObject( CFAllocatorRef inAllocator, const uint8_t **ioPtr, const uint8_t *inEnd, CFTypeRef *outObj );
static OSStatus	_ReadInteger( const uint8_t **ioPtr, const uint8_t *inEnd, uint64_t *outValue );

static void		_FlattenPlist( CFBinaryPlistContext *ctx, CFTypeRef inObj );
static void		_FlattenArray( const void *inValue, void *inContext );
static void		_FlattenDictionaryKey( const void *inKey, const void *inValue, void *inContext );
static void		_FlattenDictionaryValue( const void *inKey, const void *inValue, void *inContext );
static Boolean	_ObjectsExactlyEqual( const void *a, const void *b );
static void		_WriteV0ArrayValue( const void *inValue, void *inContext );
static void		_WriteV0DictionaryKey( const void *inKey, const void *inValue, void *inContext );
static void		_WriteV0DictionaryValue( const void *inKey, const void *inValue, void *inContext );
static OSStatus	_WriteV0Object( CFBinaryPlistContext *inContext, CFTypeRef inObj );
static OSStatus	_WriteV0Number( CFBinaryPlistContext *ctx, CFNumberRef inNum );
static OSStatus	_WriteV0String( CFBinaryPlistContext *ctx, CFStringRef inStr );

static CFTypeRef
	_ReadV0Object( 
		CFBinaryPlistContext *	ctx, 
		const uint8_t *			inSrc, 
		const uint8_t *			inEnd, 
		size_t					inOffset, 
		OSStatus *				outErr ) CF_RETURNS_RETAINED;
static OSStatus
	_ReadRefOffset( 
		CFBinaryPlistContext *	ctx, 
		const uint8_t *			inSrc, 
		const uint8_t *			inEnd, 
		const uint8_t **		ioPtr, 
		size_t *				outOffset );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static CFTypeID		gCFArrayType		= (CFTypeID) -1;
static CFTypeID		gCFBooleanType		= (CFTypeID) -1;
static CFTypeID		gCFDataType			= (CFTypeID) -1;
static CFTypeID		gCFDateType			= (CFTypeID) -1;
static CFTypeID		gCFDictionaryType	= (CFTypeID) -1;
static CFTypeID		gCFNumberType		= (CFTypeID) -1;
static CFTypeID		gCFStringType		= (CFTypeID) -1;

//===========================================================================================================================
//	_GlobalEnsureInitialized
//===========================================================================================================================

static void	_GlobalEnsureInitialized( void )
{
	if( gCFStringType == ( (CFTypeID) -1 ) )
	{
		gCFArrayType		= CFArrayGetTypeID();
		gCFBooleanType		= CFBooleanGetTypeID();
		gCFDataType			= CFDataGetTypeID();
		gCFDateType			= CFDateGetTypeID();
		gCFDictionaryType	= CFDictionaryGetTypeID();
		gCFNumberType		= CFNumberGetTypeID();
		gCFStringType		= CFStringGetTypeID();
	}
}

#if 0
#pragma mark -
#pragma mark == Streamed ==
#endif

//===========================================================================================================================
//	CFBinaryPlistCreateData
//===========================================================================================================================

OSStatus	CFBinaryPlistCreateData( CFAllocatorRef inAllocator, CFTypeRef inObj, CFDataRef *outData )
{
	OSStatus					err;
	CFBinaryPlistContext		ctx;
	
	_GlobalEnsureInitialized();
	CFBinaryPlistContextInit( &ctx );
	
	ctx.data = CFDataCreateMutable( inAllocator, 0 );
	require_action( ctx.data, exit, err = kNoMemoryErr );
	
	err = _WriteBytes( &ctx, "CFB0", 4 );
	require_noerr( err, exit );
	
	err = _WriteObject( &ctx, inObj );
	require_noerr( err, exit );
	
	err = _WriteBytes( &ctx, "END!", 4 );
	require_noerr( err, exit );
	
	*outData = ctx.data;
	ctx.data = NULL;
	
exit:
	CFBinaryPlistContextFree( &ctx );
	return( err );
}

//===========================================================================================================================
//	_WriteObject
//===========================================================================================================================

static OSStatus	_WriteObject( CFBinaryPlistContext *inContext, CFTypeRef inObj )
{
	OSStatus		err;
	CFTypeID		type;
	uint8_t			marker;
	CFRange			range;
	CFIndex			i;
	CFIndex			n;
	CFIndex			size;
	CFIndex			used;
	uint64_t		u64;
	CFTypeRef		obj;
	CFTypeRef *		keysAndValues;
	CFIndex			nTotal;
	
	keysAndValues = NULL;
	
	type = CFGetTypeID( inObj );
	if( type == gCFStringType )				// String
	{
		marker = kCFLBinaryPlistMarkerUTF8String;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
			
		range.location	= 0;
		range.length	= CFStringGetLength( (CFStringRef) inObj );
		while( range.length > 0 )
		{
			size = (CFIndex) sizeof( inContext->stringBuf );
			n = CFStringGetBytes( (CFStringRef) inObj, range, kCFStringEncodingUTF8, 0, false, inContext->stringBuf, size, &used );
			require_action( n > 0, exit, err = kUnsupportedErr );
			
			err = _WriteBytes( inContext, inContext->stringBuf, (size_t) used );
			require_noerr( err, exit );
			
			range.location	+= n;
			range.length	-= n;
		}
		
		err = _WriteBytes( inContext, "", 1 );
		require_noerr( err, exit );
	}
	else if( type == gCFNumberType )		// Number
	{
		CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &u64 );
		err = _WriteInteger( inContext, u64 );
		require_noerr( err, exit );
	}
	else if( type == gCFBooleanType )		// Boolean
	{
		if( inObj == kCFBooleanTrue ) marker = kCFLBinaryPlistMarkerTrue;
		else						  marker = kCFLBinaryPlistMarkerFalse;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
	}
	else if( type == gCFDataType )			// Data
	{
		n = CFDataGetLength( (CFDataRef) inObj );
		if( n < 15 ) marker = (uint8_t)( kCFLBinaryPlistMarkerData | n );
		else		 marker = kCFLBinaryPlistMarkerData | 0xF;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
		
		if( n >= 15 )
		{
			err = _WriteInteger( inContext, (uint64_t) n );
			require_noerr( err, exit );
		}
		
		err = _WriteBytes( inContext, CFDataGetBytePtr( (CFDataRef) inObj ), (size_t) n );
		require_noerr( err, exit );
	}
	else if( type == gCFDateType )			// Date
	{
		int			year, month, day, hour, minute, second;
		uint8_t		dateBuf[ 6 ];
		
		// Year		- 14 bits (0-16383)
		// Month	-  4 bits (0-11)
		// Day		-  5 bits (0-30)
		// Hour		-  5 bits (0-23)
		// Minute	-  6 bits (0-59)
		// Second	-  6 bits (0-61 double leap seconds)
		
		CFDateGetComponents( (CFDateRef) inObj, &year, &month, &day, &hour, &minute, &second, NULL );
		dateBuf[ 0 ]  = kCFLBinaryPlistMarkerDateInteger;
		dateBuf[ 1 ]  = (uint8_t)( ( year   >> 6 ) & 0xFF );
		dateBuf[ 2 ]  = (uint8_t)( ( year   << 2 ) & 0xFC );
		dateBuf[ 2 ] |= (uint8_t)( ( month  >> 2 ) & 0x03 );
		dateBuf[ 3 ]  = (uint8_t)( ( month  << 6 ) & 0xC0 );
		dateBuf[ 3 ] |= (uint8_t)( ( day    << 1 ) & 0x3E );
		dateBuf[ 3 ] |= (uint8_t)( ( hour   >> 4 ) & 0x01 );
		dateBuf[ 4 ]  = (uint8_t)( ( hour   << 4 ) & 0xF0 );
		dateBuf[ 4 ] |= (uint8_t)( ( minute >> 2 ) & 0x0F );
		dateBuf[ 5 ]  = (uint8_t)( ( minute << 6 ) & 0xC0 );
		dateBuf[ 5 ] |= (uint8_t)(   second        & 0x3F );
		
		err = _WriteBytes( inContext, dateBuf, 6 );
		require_noerr( err, exit );
	}
	else if( type == gCFDictionaryType )	// Dictionary
	{
		marker = kCFLBinaryPlistMarkerDictionary;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
		
		n = CFDictionaryGetCount( (CFDictionaryRef) inObj );
		if( n > 0 )
		{
			nTotal = n * 2;
			keysAndValues = (CFTypeRef *) malloc( nTotal * sizeof( *keysAndValues ) );
			require_action( keysAndValues, exit, err = kNoMemoryErr );
			
			CFDictionaryGetKeysAndValues( (CFDictionaryRef) inObj, &keysAndValues[ 0 ], &keysAndValues[ n ] );
			for( i = 0; i < n; ++i )
			{
				err = _WriteObject( inContext, keysAndValues[ 0 + i ] );
				require_noerr( err, exit );
				
				err = _WriteObject( inContext, keysAndValues[ n + i ] );
				require_noerr( err, exit );
			}
			free( (void *) keysAndValues ); keysAndValues = NULL;
		}
		marker = kCFLBinaryPlistMarkerNull;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
	}
	else if( type == gCFArrayType )			// Array
	{
		marker = kCFLBinaryPlistMarkerArray;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
		
		n = CFArrayGetCount( (CFArrayRef) inObj );
		for( i = 0; i < n; ++i )
		{
			obj = CFArrayGetValueAtIndex( (CFArrayRef) inObj, i );
			err = _WriteObject( inContext, obj );
			require_noerr( err, exit );
		}
		
		marker = kCFLBinaryPlistMarkerNull;
		err = _WriteBytes( inContext, &marker, 1 );
		require_noerr( err, exit );
	}
	else
	{
		dlogassert( "Unknown object type: %u", type );
		err = kUnsupportedErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( keysAndValues ) free( (void *) keysAndValues );
	return( err );
}

//===========================================================================================================================
//	_WriteInteger
//===========================================================================================================================

static OSStatus	_WriteInteger( CFBinaryPlistContext *inContext, uint64_t inValue )
{
	OSStatus		err;
	uint8_t			marker;
	uint8_t *		p;
	uint8_t			n;
	
	if( inValue <= UINT64_C( 0xFF ) )
	{
		n = 1;
		marker = kCFLBinaryPlistMarkerInt | 0;
	}
	else if( inValue <= UINT64_C( 0xFFFF ) )
	{
		n = 2;
		marker = kCFLBinaryPlistMarkerInt | 1;
	}
	else if( inValue <= UINT64_C( 0xFFFFFFFF ) )
	{
		n = 4;
		marker = kCFLBinaryPlistMarkerInt | 2;
	}
	else
	{
		n = 8;
		marker = kCFLBinaryPlistMarkerInt | 3;
	}
	err = _WriteBytes( inContext, &marker, 1 );
	require_noerr( err, exit );
	
	inValue = hton64( inValue );
	p = ( ( (uint8_t *) &inValue ) + sizeof( inValue ) ) - n;
	err = _WriteBytes( inContext, p, n );
	require_noerr( err, exit );

exit:
	return( err );
}

//===========================================================================================================================
//	_WriteBytes
//===========================================================================================================================

static OSStatus	_WriteBytes( CFBinaryPlistContext *inContext, const void *inData, size_t inSize )
{
	CFDataAppendBytes( inContext->data, (const UInt8 *) inData, (CFIndex) inSize );
	inContext->bytesWritten += inSize;
	return( kNoErr );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFBinaryPlistCreateFromData
//===========================================================================================================================

CFTypeRef	CFBinaryPlistCreateFromData( CFAllocatorRef inAllocator, const void *inData, size_t inSize, OSStatus *outErr )
{
	CFTypeRef			result = NULL;
	CFTypeRef			obj = NULL;
	OSStatus			err;
	const uint8_t *		p;
	const uint8_t *		q;
	
	_GlobalEnsureInitialized();
	
	p = (const uint8_t *) inData;
	q = p + inSize;
	require_action_quiet( inSize > kCFLBinaryPlistHeaderSignatureSize, exit, err = kSizeErr );
	require_action_quiet( memcmp( p, kCFLBinaryPlistHeaderSignature, kCFLBinaryPlistHeaderSignatureSize ) == 0, exit, err = kSignatureErr );
	p += kCFLBinaryPlistHeaderSignatureSize;
	
	err = _ReadObject( inAllocator, &p, q, &obj );
	require_noerr_quiet( err, exit );
	require_action_quiet( obj, exit, err = kMalformedErr );
	
	require_action_quiet( ( q - p ) >= (ptrdiff_t) kCFLBinaryPlistTrailerSignatureSize, exit, err = kSizeErr );
	require_action_quiet( memcmp( p, kCFLBinaryPlistTrailerSignature, kCFLBinaryPlistTrailerSignatureSize ) == 0, exit, err = kSignatureErr );
	
	result = obj;
	obj = NULL;
	
exit:
	CFReleaseNullSafe( obj );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	_ReadObject
//===========================================================================================================================

static OSStatus	_ReadObject( CFAllocatorRef inAllocator, const uint8_t **ioPtr, const uint8_t *inEnd, CFTypeRef *outObj )
{
	OSStatus					err;
	const uint8_t *				p;
	const uint8_t *				r;
	uint8_t						marker;
	uint64_t					u64;
	CFMutableArrayRef			array;
	CFMutableDictionaryRef		dict;
	CFTypeRef					key;
	CFTypeRef					value;
	
	array	= NULL;
	dict	= NULL;
	key		= NULL;
	value	= NULL;
	
	p = *ioPtr;
	require_action_quiet( p < inEnd, exit, err = kUnderrunErr );
	
	marker = *p++;
	switch( marker & 0xF0 )
	{
		case 0:
			switch( marker )
			{
				case kCFLBinaryPlistMarkerNull:
					*outObj = NULL;
					break;
				
				case kCFLBinaryPlistMarkerFalse:
					*outObj = CFRetain( kCFBooleanFalse );
					break;
				
				case kCFLBinaryPlistMarkerTrue:
					*outObj = CFRetain( kCFBooleanTrue );
					break;
				
				default:
					err = kUnsupportedErr;
					goto exit;
			}
			break;
		
		case kCFLBinaryPlistMarkerInt:
		{
			CFNumberRef		num;
			
			p -= 1;
			err = _ReadInteger( &p, inEnd, &u64 );
			require_noerr_quiet( err, exit );
			
			num = CFNumberCreate( inAllocator, kCFNumberSInt64Type, &u64 );
			require_action( num, exit, err = kNoMemoryErr );
			
			*outObj = num;
			break;
		}
		
		case kCFLBinaryPlistMarkerDateInteger:
		{
			int				year, month, day, hour, minute, second;
			CFDateRef		date;
			
			require_action_quiet( (int)( inEnd - p ) >= 5, exit, err = kSizeErr );
			
			// Year		- 14 bits (0-16383)
			// Month	-  4 bits (0-11)
			// Day		-  5 bits (0-30)
			// Hour		-  5 bits (0-23)
			// Minute	-  6 bits (0-59)
			// Second	-  6 bits (0-61 double leap seconds)
			
			year	= (   p[ 0 ] << 6 )          | ( p[ 1 ] >> 2 );
			month	= ( ( p[ 1 ] << 2 ) & 0x0C ) | ( p[ 2 ] >> 6 );
			day		= (   p[ 2 ] >> 1 ) & 0x1F;
			hour	= ( ( p[ 2 ] << 4 ) & 0x10 ) | ( p[ 3 ] >> 4 );
			minute	= ( ( p[ 3 ] << 2 ) & 0x3C ) | ( p[ 4 ] >> 6 );
			second	=     p[ 4 ]        & 0x3F;
			p += 5;
			
			date = CFDateCreateWithComponents( inAllocator, year, month, day, hour, minute, second );
			require_action( date, exit, err = kNoMemoryErr );
			
			*outObj = date;
			break;
		}
		
		case kCFLBinaryPlistMarkerData:
		{
			CFDataRef		data;
			
			u64 = marker & 0x0F;
			if( u64 == 0xF )
			{
				err = _ReadInteger( &p, inEnd, &u64 );
				require_noerr_quiet( err, exit );
				require_action_quiet( u64 <= 0x7FFFFFFF, exit, err = kRangeErr );
			}
			require_action_quiet( (ptrdiff_t) u64 <= ( inEnd - p ) , exit, err = kUnderrunErr );
			
			data = CFDataCreate( inAllocator, p, (CFIndex) u64 );
			require_action( data, exit, err = kNoMemoryErr );
			p += u64;
			
			*outObj = data;
			break;
		}
		
		case kCFLBinaryPlistMarkerUTF8String:
		{
			CFStringRef		str;
			char *			asciiBuf;
			const char *	asciiSrc;
			char *			asciiDst;
			char			c;
			
			for( r = p; ( r < inEnd ) && *r; ++r ) {}
			require_action_quiet( r < inEnd, exit, err = kNotFoundErr );
			
			// Strings in CFLite binary plists should always be UTF-8, but code may generate UTF-16, Latin-1, etc.
			// So try UTF-8 then Windows Latin-1 then UTF-16LE, and finally strip non-ASCII characters and use that.
			
			str = CFStringCreateWithCString( inAllocator, (const char *) p, kCFStringEncodingUTF8 );
			if( !str )
			{
				dlogassert( "Bad UTF-8 string, trying Windows Latin-1 with:\n%1.1H", 
					p, (int)( r - p ), (int)( r - p ) );
				
				str = CFStringCreateWithCString( inAllocator, (const char *) p, kCFStringEncodingWindowsLatin1 );
				if( !str )
				{
					dlogassert( "Bad Windows Latin-1E, stripping non-ASCII" );
					
					asciiBuf = strdup( (const char *) p );
					require_action( asciiBuf, exit, err = kNoMemoryErr );
					
					asciiDst = asciiBuf;
					for( asciiSrc = asciiBuf; ( c = *asciiSrc ) != '\0'; ++asciiSrc )
					{
						if( c & 0x80 ) continue;
						*asciiDst++ = c;
					}
					*asciiDst = '\0';
					
					str = CFStringCreateWithCString( inAllocator, asciiBuf, kCFStringEncodingUTF8 );
					free( asciiBuf );
					require_action( str, exit, err = kMalformedErr );
				}
			}
			p = r + 1;
			
			*outObj = str;
			break;
		}
		
		case kCFLBinaryPlistMarkerArray:
			array = CFArrayCreateMutable( inAllocator, 0, &kCFTypeArrayCallBacks );
			require_action( array, exit, err = kNoMemoryErr );
			
			for( ;; )
			{
				err = _ReadObject( inAllocator, &p, inEnd, &value );
				require_noerr_quiet( err, exit );
				if( !value ) break;
				
				CFArrayAppendValue( array, value );
				CFRelease( value );
				value = NULL;
			}
			
			*outObj = array; array = NULL;
			break;
		
		case kCFLBinaryPlistMarkerDictionary:
			dict = CFDictionaryCreateMutable( inAllocator, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require_action( dict, exit, err = kNoMemoryErr );
			
			for( ;; )
			{
				err = _ReadObject( inAllocator, &p, inEnd, &key );
				require_noerr_quiet( err, exit );
				if( !key ) break;
				
				err = _ReadObject( inAllocator, &p, inEnd, &value );
				require_noerr_quiet( err, exit );
				require_action_quiet( value, exit, err = kUnderrunErr );
				
				CFDictionarySetValue( dict, key, value );
				CFRelease( key );	key		= NULL;
				CFRelease( value );	value	= NULL;
			}
			
			*outObj = dict; dict = NULL;
			break;
		
		default:
			err = kUnsupportedErr;
			goto exit;
	}
	*ioPtr = p;
	err = kNoErr;
	
exit:
	if( key )	CFRelease( key );
	if( value )	CFRelease( value );
	if( array ) CFRelease( array );
	if( dict )  CFRelease( dict );
	return( err );
}

//===========================================================================================================================
//	_ReadInteger
//===========================================================================================================================

static OSStatus	_ReadInteger( const uint8_t **ioPtr, const uint8_t *inEnd, uint64_t *outValue )
{
	OSStatus			err;
	const uint8_t *		p;
	uint8_t				marker;
	int					n;
	uint64_t			x;
	
	p = *ioPtr;
	require_action_quiet( p < inEnd, exit, err = kUnderrunErr );
	
	marker = *p++;
	require_action_quiet( ( marker & 0xF0 ) == kCFLBinaryPlistMarkerInt, exit, err = kTypeErr );
	
	n = 1 << ( marker & 0x0F );
	require_action_quiet( n <= (int)( inEnd - p ), exit, err = kSizeErr );
	
	x = 0;
	while( n-- > 0 ) x = ( x << 8 ) | *p++;
	
	*ioPtr = p;
	*outValue = x;
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Version 0 ==
#endif

//===========================================================================================================================
//	CFBinaryPlistV0CreateData
//===========================================================================================================================

CFDataRef	CFBinaryPlistV0CreateData( CFTypeRef inObj, OSStatus *outErr )
{
	CFDataRef						result = NULL;
	OSStatus						err;
	CFBinaryPlistContext			ctx;
	CFDictionaryKeyCallBacks		dictCallbacks;
	CFBinaryPlistTrailer			trailer;
	uint64_t *						offsets = NULL;
	CFIndex							i;
	CFTypeRef						obj;
	uint8_t							buf[ 8 ];
	
	_GlobalEnsureInitialized();
	CFBinaryPlistContextInit( &ctx );
	
	// Flatten the plist to an array of unique objects.
	
	ctx.data = CFDataCreateMutable( NULL, 0 );
	require_action( ctx.data, exit, err = kNoMemoryErr );
	
	ctx.array = CFArrayCreateMutable( NULL, 0, NULL );
	require_action( ctx.array, exit, err = kNoMemoryErr );
	
	dictCallbacks			= kCFTypeDictionaryKeyCallBacks;
	dictCallbacks.equal		= _ObjectsExactlyEqual;
	dictCallbacks.retain	= NULL;
	dictCallbacks.release	= NULL;
	ctx.uniqueDict = CFDictionaryCreateMutable( NULL, 0, &dictCallbacks, NULL );
	require_action( ctx.uniqueDict, exit, err = kNoMemoryErr );
	
	_FlattenPlist( &ctx, inObj );
	check( ctx.uniqueCount > 0 );
	
	// Write the header and object table.
	
	err = _WriteBytes( &ctx, "bplist00", 8 );
	require_noerr( err, exit );
	
	memset( &trailer, 0, sizeof( trailer ) );
	trailer.numObjects		= hton64( ctx.uniqueCount );
	trailer.objectRefSize	= MinPowerOf2BytesForValue( ctx.uniqueCount );
	ctx.objectRefSize		= trailer.objectRefSize;
	
	offsets = (uint64_t *) malloc( ctx.uniqueCount * sizeof( *offsets ) );
	require_action( offsets, exit, err = kNoMemoryErr );
	for( i = 0; i < ctx.uniqueCount; ++i )
	{
		offsets[ i ] = ctx.bytesWritten;
		obj = CFArrayGetValueAtIndex( ctx.array, i );
		err = _WriteV0Object( &ctx, obj );
		require_noerr( err, exit );
	}
	
	// Write the offsets table and trailer.
	
	trailer.offsetTableOffset	= hton64( ctx.bytesWritten );
	trailer.offsetIntSize		= MinPowerOf2BytesForValue( ctx.bytesWritten );
	for( i = 0; i < ctx.uniqueCount; ++i )
	{
		WriteBig64( buf, offsets[ i ] );
		err = _WriteBytes( &ctx, buf + ( sizeof( *offsets ) - trailer.offsetIntSize ), trailer.offsetIntSize );
		require_noerr( err, exit );
	}
	err = _WriteBytes( &ctx, &trailer, sizeof( trailer ) );
	require_noerr( err, exit );
	
	result = ctx.data;
	ctx.data = NULL;
	
exit:
	if( offsets ) free( offsets );
	CFBinaryPlistContextFree( &ctx );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	_FlattenPlist
//===========================================================================================================================

static void	_FlattenPlist( CFBinaryPlistContext *ctx, CFTypeRef inObj )
{
	CFTypeID		type;
	CFIndex			oldCount;
	
	// Add the object and if the count doesn't change, it means the object was already there so we can skip it.
	// This doesn't unique arrays or dictionaries because it's slow and they have poor hash functions.
	
	oldCount = CFDictionaryGetCount( ctx->uniqueDict );
	CFDictionaryAddValue( ctx->uniqueDict, inObj, (const void *)(uintptr_t) ctx->uniqueCount );
	
	type = CFGetTypeID( inObj );
	if( ( type == gCFStringType )  || 
		( type == gCFNumberType )  ||
		( type == gCFBooleanType ) || 
		( type == gCFDataType )    || 
		( type == gCFDateType ) )
	{
		if( CFDictionaryGetCount( ctx->uniqueDict ) == oldCount )
		{
			return;
		}
	}
	CFArrayAppendValue( ctx->array, inObj );
	++ctx->uniqueCount;
	
	if( type == gCFDictionaryType )
	{
		CFDictionaryApplyFunction( (CFDictionaryRef) inObj, _FlattenDictionaryKey, ctx );
		CFDictionaryApplyFunction( (CFDictionaryRef) inObj, _FlattenDictionaryValue, ctx );
	}
	else if( type == gCFArrayType )
	{
		CFArrayApplyFunction( (CFArrayRef) inObj, CFRangeMake( 0, CFArrayGetCount( (CFArrayRef) inObj ) ), _FlattenArray, ctx );
	}
}

static void	_FlattenArray( const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	
	_FlattenPlist( ctx, inValue );
}

static void	_FlattenDictionaryKey( const void *inKey, const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	
	(void) inValue;
	
	_FlattenPlist( ctx, inKey );
}

static void	_FlattenDictionaryValue( const void *inKey, const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	
	(void) inKey;
	
	_FlattenPlist( ctx, inValue );
}

//===========================================================================================================================
//	_ObjectsExactlyEqual
//
//	This is needed because we need exact matches to avoid lossy roundtrip conversions:
//
//	1. CFEqual will treat kCFBooleanFalse == CFNumber(0) and kCFBooleanTrue == CFNumber(1).
//	2. CFEqual will treat CFNumber( 1.0 ) == CFNumber( 1 ).
//===========================================================================================================================

static Boolean	_ObjectsExactlyEqual( const void *a, const void *b )
{
	CFTypeID const		aType = CFGetTypeID( a );
	CFTypeID const		bType = CFGetTypeID( b );
	
	if( ( aType == bType ) && CFEqual( a, b ) )
	{
		if( aType != gCFNumberType )
		{
			return( true );
		}
		if( CFNumberIsFloatType( (CFNumberRef) a ) == CFNumberIsFloatType( (CFNumberRef) b ) )
		{
			return( true );
		}
	}
	return( false );
}

//===========================================================================================================================
//	_WriteObject
//===========================================================================================================================

static OSStatus	_WriteV0Object( CFBinaryPlistContext *ctx, CFTypeRef inObj )
{
	OSStatus		err;
	CFTypeID		type;
	uint8_t			marker;
	Value64			v;
	CFIndex			count;
	
	type = CFGetTypeID( inObj );
	if( type == gCFStringType )
	{
		err = _WriteV0String( ctx, (CFStringRef) inObj );
		require_noerr( err, exit );
	}
	else if( type == gCFNumberType )
	{
		err = _WriteV0Number( ctx, (CFNumberRef) inObj );
		require_noerr( err, exit );
	}
	else if( type == gCFBooleanType )
	{
		marker = ( inObj == kCFBooleanTrue ) ? kCFLBinaryPlistMarkerTrue : kCFLBinaryPlistMarkerFalse;
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
	}
	else if( type == gCFDataType )
	{
		count = CFDataGetLength( (CFDataRef) inObj );
		marker = (uint8_t)( kCFLBinaryPlistMarkerData | ( ( count < 15 ) ? count : 0xF ) );
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
		if( count >= 15 )
		{
			err = _WriteInteger( ctx, (uint64_t) count );
			require_noerr( err, exit );
		}
		err = _WriteBytes( ctx, CFDataGetBytePtr( (CFDataRef) inObj ), (size_t) count );
		require_noerr( err, exit );
	}
	else if( type == gCFDictionaryType )
	{
		count = CFDictionaryGetCount( (CFDictionaryRef) inObj );
		marker = (uint8_t)( kCFLBinaryPlistMarkerDictionary | ( ( count < 15 ) ? count : 0xF ) );
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
		if( count >= 15 )
		{
			err = _WriteInteger( ctx, (uint64_t) count );
			require_noerr( err, exit );
		}
		CFDictionaryApplyFunction( (CFDictionaryRef) inObj, _WriteV0DictionaryKey, ctx );
		CFDictionaryApplyFunction( (CFDictionaryRef) inObj, _WriteV0DictionaryValue, ctx );
	}
	else if( type == gCFArrayType )
	{
		count = CFArrayGetCount( (CFArrayRef) inObj );
		marker = (uint8_t)( kCFLBinaryPlistMarkerArray | ( ( count < 15 ) ? count : 0xF ) );
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
		if( count >= 15 )
		{
			err = _WriteInteger( ctx, (uint64_t) count );
			require_noerr( err, exit );
		}
		CFArrayApplyFunction( (CFArrayRef) inObj, CFRangeMake( 0, count ), _WriteV0ArrayValue, ctx );
	}
	else if( type == gCFDateType )
	{
		marker = kCFLBinaryPlistMarkerDateFloat;
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
		
		v.f64 = CFDateGetAbsoluteTime( (CFDateRef) inObj );
		v.u64 = hton64( v.u64 );
		err = _WriteBytes( ctx, &v.u64, 8 );
		require_noerr( err, exit );
	}
	else if( inObj == ( (CFTypeRef) kCFNull ) )
	{
		marker = kCFLBinaryPlistMarkerNull;
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
	}
	else
	{
		dlogassert( "Unsupported object type: %u", type );
		err = kUnsupportedDataErr;
		goto exit;
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	_WriteV0ArrayValue
//===========================================================================================================================

static void	_WriteV0ArrayValue( const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	uint32_t							refnum;
	uint8_t								buf[ 4 ];
	
	refnum = (uint32_t)(uintptr_t) CFDictionaryGetValue( ctx->uniqueDict, inValue );
	WriteBig32( buf, refnum );
	_WriteBytes( ctx, buf + ( 4 - ctx->objectRefSize ), ctx->objectRefSize );
}

//===========================================================================================================================
//	_WriteV0DictionaryKey
//===========================================================================================================================

static void	_WriteV0DictionaryKey( const void *inKey, const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	uint32_t							refnum;
	uint8_t								buf[ 4 ];
	
	(void) inValue;
	
	refnum = (uint32_t)(uintptr_t) CFDictionaryGetValue( ctx->uniqueDict, inKey );
	WriteBig32( buf, refnum );
	_WriteBytes( ctx, buf + ( 4 - ctx->objectRefSize ), ctx->objectRefSize );
}

//===========================================================================================================================
//	_WriteV0DictionaryValue
//===========================================================================================================================

static void	_WriteV0DictionaryValue( const void *inKey, const void *inValue, void *inContext )
{
	CFBinaryPlistContext * const		ctx = (CFBinaryPlistContext *) inContext;
	uint32_t							refnum;
	uint8_t								buf[ 4 ];
	
	(void) inKey;
	
	refnum = (uint32_t)(uintptr_t) CFDictionaryGetValue( ctx->uniqueDict, inValue );
	WriteBig32( buf, refnum );
	_WriteBytes( ctx, buf + ( 4 - ctx->objectRefSize ), ctx->objectRefSize );
}

//===========================================================================================================================
//	_WriteV0Number
//===========================================================================================================================

static OSStatus	_WriteV0Number( CFBinaryPlistContext *ctx, CFNumberRef inNum )
{
	OSStatus		err;
	Value64			v;
	uint8_t			marker;
	uint8_t			n;
	
	if( CFNumberIsFloatType( inNum ) )
	{
		if( CFNumberGetByteSize( inNum ) <= ( (CFIndex) sizeof( Float32 ) ) )
		{
			CFNumberGetValue( inNum, kCFNumberFloat32Type, &v.f32 );
			v.u32[ 0 ] = htonl( v.u32[ 0 ] );
			marker = kCFLBinaryPlistMarkerReal | 2;
			n = 4;
		}
		else
		{
			CFNumberGetValue( inNum, kCFNumberFloat64Type, &v.f64 );
			v.u64 = hton64( v.u64 );
			marker = kCFLBinaryPlistMarkerReal | 3;
			n = 8;
		}
		
		err = _WriteBytes( ctx, &marker, 1 );
		require_noerr( err, exit );
		
		err = _WriteBytes( ctx, v.u8, n );
		require_noerr( err, exit );
	}
	else
	{
		CFNumberGetValue( inNum, kCFNumberSInt64Type, &v.u64 );
		err = _WriteInteger( ctx, v.u64 );
		require_noerr( err, exit );
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	_WriteV0String
//===========================================================================================================================

static OSStatus	_WriteV0String( CFBinaryPlistContext *ctx, CFStringRef inStr )
{
	OSStatus			err;
	const uint8_t *		src;
	uint8_t *			utf8  = NULL;
#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	uint16_t *			utf16 = NULL;
#endif
	size_t				len, i;
	uint8_t				marker;
	size_t				count;
	
	src = (const uint8_t *) CFStringGetCStringPtr( inStr, kCFStringEncodingUTF8 );
	if( src )
	{
		len = strlen( (const char *) src );
	}
	else
	{
		CFRange		range;
		CFIndex		nBytes;
		
		range = CFRangeMake( 0, CFStringGetLength( inStr ) );
		nBytes = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
		
		utf8 = (uint8_t *) malloc( (size_t)( nBytes + 1 ) );
		require_action( utf8, exit, err = kNoMemoryErr );
		
		range.location = CFStringGetBytes( inStr, range, kCFStringEncodingUTF8, 0, false, utf8, nBytes, &nBytes );
		require_action( range.location == range.length, exit, err = kUnknownErr );
		utf8[ nBytes ] = '\0';
		src = utf8;
		len = nBytes;
	}
	
	// Check if the string is only ASCII. If it's then we can write it out directly.
	
	for( i = 0; ( i < len ) && !( src[ i ] & 0x80 ); ++i ) {}
	if( i == len )
	{
		marker = kCFLBinaryPlistMarkerASCIIString;
		count  = len;
	}
	else
	{
		#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
			err = utf8_decodestr_copy( src, len, &utf16, &len, 0, UTF_BIG_ENDIAN );
			require_noerr( err, exit );
			
			marker = kCFLBinaryPlistMarkerUnicodeString;
			src    = (const uint8_t *) utf16;
			count  = len / 2;
		#else
			dlogassert( "UTF-16 required, but conversion code stripped out" );
			err = kUnsupportedDataErr;
			goto exit;
		#endif
	}
	
	marker |= ( ( count < 15 ) ? count : 0xF );
	err = _WriteBytes( ctx, &marker, 1 );
	require_noerr( err, exit );
	if( count >= 15 )
	{
		err = _WriteInteger( ctx, count );
		require_noerr( err, exit );
	}
	err = _WriteBytes( ctx, src, len );
	require_noerr( err, exit );
	
exit:
	if( utf8 )  free( utf8 );
#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	if( utf16 ) utffree( utf16 );
#endif
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFBinaryPlistV0CreateWithData
//===========================================================================================================================

CFPropertyListRef	CFBinaryPlistV0CreateWithData( const void *inPtr, size_t inLen, OSStatus *outErr )
{
	const uint8_t *					src   = (const uint8_t *) inPtr;
	const uint8_t *					end   = src + inLen;
	CFPropertyListRef				plist = NULL;
	CFBinaryPlistContext			ctx;
	OSStatus						err;
	CFDictionaryValueCallBacks		dictCallbacks;
	CFBinaryPlistTrailer			trailer;
	
	CFBinaryPlistContextInit( &ctx );
	
	// Sanity check the header/trailer up front to speed up batch checking of arbitrary files.
	
	require_action_quiet( src < end, exit, err = kSizeErr );
	require_action_quiet( inLen > ( 8 + sizeof( trailer ) ), exit, err = kSizeErr );
	require_action_quiet( memcmp( src, "bplist00", 8 ) == 0, exit, err = kFormatErr );
	memcpy( &trailer, end - sizeof( trailer ), sizeof( trailer ) );
	end -= sizeof( trailer );
	
	require_action_quiet( 
		( trailer.offsetIntSize == 1 ) || 
		( trailer.offsetIntSize == 2 ) ||
		( trailer.offsetIntSize == 4 ) ||
		( trailer.offsetIntSize == 8 ), exit, err = kMalformedErr );
	require_action_quiet( 
		( trailer.objectRefSize == 1 ) || 
		( trailer.objectRefSize == 2 ) ||
		( trailer.objectRefSize == 4 ) ||
		( trailer.objectRefSize == 8 ), exit, err = kMalformedErr );
	
	trailer.numObjects = ntoh64( trailer.numObjects );
	require_action_quiet( trailer.numObjects > 0, exit, err = kCountErr );
	
	trailer.offsetTableOffset = ntoh64( trailer.offsetTableOffset );
	require_action_quiet( trailer.offsetTableOffset >= 9, exit, err = kMalformedErr );
	require_action_quiet( trailer.offsetTableOffset < ( inLen - sizeof( trailer ) ), exit, err = kMalformedErr );
	require_action_quiet( trailer.numObjects <= 
		( ( (size_t)( end - ( src + trailer.offsetTableOffset ) ) ) / trailer.offsetIntSize ), exit, err = kCountErr );
	
	// Read the root object (and any objects it contains).
	
	_GlobalEnsureInitialized();
	ctx.uniqueCount			= (CFIndex) trailer.numObjects;
	ctx.offsetIntSize		= trailer.offsetIntSize;
	ctx.objectRefSize		= trailer.objectRefSize;
	ctx.offsetTableOffset	= (size_t) trailer.offsetTableOffset;
	
	dictCallbacks = kCFTypeDictionaryValueCallBacks;
	dictCallbacks.equal = _ObjectsExactlyEqual;
	ctx.uniqueDict = CFDictionaryCreateMutable( NULL, 0, NULL, &dictCallbacks );
	require_action( ctx.uniqueDict, exit, err = kNoMemoryErr );
	
	plist = _ReadV0Object( &ctx, src, end, 8, &err );
	require_noerr_quiet( err, exit );
	
exit:
	CFBinaryPlistContextFree( &ctx );
	if( outErr ) *outErr = err;
	return( plist );
}

//===========================================================================================================================
//	_ReadV0Object
//===========================================================================================================================

static CFTypeRef
	_ReadV0Object( 
		CFBinaryPlistContext *	ctx, 
		const uint8_t *			inSrc, 
		const uint8_t *			inEnd, 
		size_t					inOffset, 
		OSStatus *				outErr )
{
	CFTypeRef					obj = NULL;
	OSStatus					err;
	const uint8_t *				ptr;
	uint8_t						marker;
	uint64_t					count;
	uint32_t					u32;
	Value64						v64;
	size_t						offset;
	CFMutableArrayRef			array = NULL;
	CFMutableDictionaryRef		dict = NULL;
	CFTypeRef					key = NULL, value = NULL;
	const uint8_t *				keyPtr;
	const uint8_t *				valuePtr;
	
	require_action_quiet( inOffset < ( (size_t)( inEnd - inSrc ) ), exit, err = kRangeErr );
	
	obj = CFDictionaryGetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset );
	if( obj )
	{
		CFRetain( obj );
		err = kNoErr;
		goto exit;
	}
	
	ptr = inSrc + inOffset;
	marker = *ptr++;
	switch( marker & 0xF0 )
	{
		case 0:
			switch( marker )
			{
				case kCFLBinaryPlistMarkerNull:
					obj = CFRetain( kCFNull );
					break;
				
				case kCFLBinaryPlistMarkerFalse:
					obj = CFRetain( kCFBooleanFalse );
					break;
				
				case kCFLBinaryPlistMarkerTrue:
					obj = CFRetain( kCFBooleanTrue );
					break;
				
				default:
					err = kTypeErr;
					goto exit;
			}
			break;
		
		case kCFLBinaryPlistMarkerInt:
			u32 = 1 << ( marker & 0x0F );
			require_action_quiet( u32 <= 8, exit, err = kSizeErr );
			require_action_quiet( u32 <= ( (size_t)( inEnd - ptr ) ), exit, err = kSizeErr );
			v64.u64 = 0;
			while( u32-- > 0 ) v64.u64 = ( v64.u64 << 8 ) | *ptr++;
			obj = CFNumberCreate( NULL, kCFNumberSInt64Type, &v64.u64 );
			require_action( obj, exit, err = kNoMemoryErr );
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerReal:
			switch( marker & 0x0F )
			{
				case 2:
					require_action_quiet( ( inEnd - ptr ) >= 4, exit, err = kSizeErr );
					v64.u32[ 0 ] = ReadBig32( ptr );
					obj = CFNumberCreate( NULL, kCFNumberFloat32Type, &v64.f32 );
					require_action( obj, exit, err = kNoMemoryErr );
					break;
				
				case 3:
					require_action_quiet( ( inEnd - ptr ) >= 8, exit, err = kSizeErr );
					v64.u64 = ReadBig64( ptr );
					obj = CFNumberCreate( NULL, kCFNumberFloat64Type, &v64.f64 );
					require_action( obj, exit, err = kNoMemoryErr );
					break;
				
				default:
					err = kSizeErr;
					goto exit;
			}
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerDate:
			require_action_quiet( marker == kCFLBinaryPlistMarkerDateFloat, exit, err = kTypeErr );
			require_action_quiet( ( inEnd - ptr ) >= 8, exit, err = kSizeErr );
			v64.u64 = ReadBig64( ptr );
			obj = CFDateCreate( NULL, v64.f64 );
			require_action( obj, exit, err = kNoMemoryErr );
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerData:
			count = marker & 0x0F;
			if( count == 0xF )
			{
				err = _ReadInteger( &ptr, inEnd, &count );
				require_noerr_quiet( err, exit );
			}
			require_action_quiet( count <= ( (size_t)( inEnd - ptr ) ), exit, err = kSizeErr );
			
			obj = CFDataCreate( NULL, ptr, (CFIndex) count );
			require_action( obj, exit, err = kNoMemoryErr );
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerASCIIString:
			count = marker & 0x0F;
			if( count == 0xF )
			{
				err = _ReadInteger( &ptr, inEnd, &count );
				require_noerr_quiet( err, exit );
			}
			require_action_quiet( count <= ( (size_t)( inEnd - ptr ) ), exit, err = kSizeErr );
			
			obj = CFStringCreateWithBytes( NULL, ptr, (CFIndex) count, kCFStringEncodingASCII, false );
			require_action( obj, exit, err = kNoMemoryErr );
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerUnicodeString:
			count = marker & 0x0F;
			if( count == 0xF )
			{
				err = _ReadInteger( &ptr, inEnd, &count );
				require_noerr_quiet( err, exit );
			}
			count *= 2;
			require_action_quiet( count <= ( (size_t)( inEnd - ptr ) ), exit, err = kSizeErr );
			
			obj = CFStringCreateWithBytes( NULL, ptr, (CFIndex) count, kCFStringEncodingUTF16BE, false );
			require_action( obj, exit, err = kNoMemoryErr );
			CFDictionarySetValue( ctx->uniqueDict, (const void *)(uintptr_t) inOffset, obj );
			break;
		
		case kCFLBinaryPlistMarkerArray:
			count = marker & 0x0F;
			if( count == 0xF )
			{
				err = _ReadInteger( &ptr, inEnd, &count );
				require_noerr_quiet( err, exit );
			}
			
			array = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			require_action( array, exit, err = kNoMemoryErr );
			
			while( count-- > 0 )
			{
				err = _ReadRefOffset( ctx, inSrc, inEnd, &ptr, &offset );
				require_noerr_quiet( err, exit );
				value = _ReadV0Object( ctx, inSrc, inEnd, offset, &err );
				require_noerr_quiet( err, exit );
				
				CFArrayAppendValue( array, value );
				CFRelease( value );
				value = NULL;
			}
			obj = array;
			array = NULL;
			break;
		
		case kCFLBinaryPlistMarkerDictionary:
			count = marker & 0x0F;
			if( count == 0xF )
			{
				err = _ReadInteger( &ptr, inEnd, &count );
				require_noerr_quiet( err, exit );
			}
			require_action_quiet( count <= ( ( (size_t)( inEnd - ptr ) ) / ctx->objectRefSize ), exit, err = kCountErr );
			
			dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require_action( dict, exit, err = kNoMemoryErr );
			
			keyPtr   = ptr;
			valuePtr = ptr + ( count * ctx->objectRefSize );
			while( count-- > 0 )
			{
				err = _ReadRefOffset( ctx, inSrc, inEnd, &keyPtr, &offset );
				require_noerr_quiet( err, exit );
				key = _ReadV0Object( ctx, inSrc, inEnd, offset, &err );
				require_noerr_quiet( err, exit );
				
				err = _ReadRefOffset( ctx, inSrc, inEnd, &valuePtr, &offset );
				require_noerr_quiet( err, exit );
				value = _ReadV0Object( ctx, inSrc, inEnd, offset, &err );
				require_noerr_quiet( err, exit );
				
				CFDictionarySetValue( dict, key, value );
				CFRelease( key );   key   = NULL;
				CFRelease( value ); value = NULL;
			}
			obj = dict;
			dict = NULL;
			break;
		
		default:
			err = kTypeErr;
			goto exit;
	}
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( array );
	CFReleaseNullSafe( dict );
	CFReleaseNullSafe( key );
	CFReleaseNullSafe( value );
	if( outErr ) *outErr = err;
	return( obj );
}

//===========================================================================================================================
//	_ReadRefOffset
//===========================================================================================================================

static OSStatus
	_ReadRefOffset( 
		CFBinaryPlistContext *	ctx, 
		const uint8_t *			inSrc, 
		const uint8_t *			inEnd, 
		const uint8_t **		ioPtr, 
		size_t *				outOffset )
{
	const uint8_t *		ptr = *ioPtr;
	OSStatus			err;
	uint64_t			refnum;
	const uint8_t *		offsetPtr;
	uint64_t			offset;
	
	require_action_quiet( ctx->objectRefSize < ( (size_t)( inEnd - ptr ) ), exit, err = kUnderrunErr );
	switch( ctx->objectRefSize )
	{
		case 1: refnum = Read8( ptr ); break;
		case 2: refnum = ReadBig16( ptr ); break;
		case 4: refnum = ReadBig32( ptr ); break;
		case 8: refnum = ReadBig64( ptr ); break;
		default: err = kInternalErr; goto exit;
	}
	*ioPtr = ptr + ctx->objectRefSize;
	
	require_action_quiet( refnum < ( (uint64_t) ctx->uniqueCount ), exit, err = kRangeErr );
	offsetPtr = inSrc + ctx->offsetTableOffset + ( refnum * ctx->offsetIntSize );
	switch( ctx->offsetIntSize )
	{
		case 1: offset = Read8( offsetPtr ); break;
		case 2: offset = ReadBig16( offsetPtr ); break;
		case 4: offset = ReadBig32( offsetPtr ); break;
		case 8: offset = ReadBig64( offsetPtr ); break;
		default: err = kInternalErr; goto exit;
	}
	require_action_quiet( offset <= SIZE_MAX, exit, err = kSizeErr );
	*outOffset = (size_t) offset;
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

#if 0
#pragma mark -
#pragma mark == Format ==
#endif

/*===========================================================================================================================
	Binary Property List Format
	---------------------------
	
	HEADER
		signature ("CFB0") for streamed binary plists.
		or "bplist00" for version 0 binary plists (normal CF).
	
	ROOT OBJECT
		Object Formats (marker byte followed by additional info in some cases)
		null	0000 0000
		bool	0000 1000							// false
		bool	0000 1001							// true
		fill	0000 1111							// fill byte
		int		0001 nnnn	...						// # of bytes is 2^nnnn, big-endian bytes
		real	0010 nnnn	...						// # of bytes is 2^nnnn, big-endian bytes
		date	0011 0000	...						// 5 byte encoded date
		date	0011 0011	...						// 8 byte float follows, big-endian bytes
		data	0100 nnnn	[int]	...				// nnnn is number of bytes unless 1111 then int count follows, followed by bytes
		string	0101 nnnn	[int]	...				// ASCII string, nnnn is # of chars, else 1111 then int count, then bytes
		string	0110 nnnn	[int]	...				// Unicode string, nnnn is # of chars, else 1111 then int count, then big-endian 2-byte uint16_t
		string	0111 0000	...						// UTF-8 string with a null terminator.
		uid		1000 nnnn	...						// nnnn+1 is # of bytes
				1001 xxxx							// unused
		array	1010 0000	objects					// A null object indicates the end.
		array	1010 nnnn	[int]	objref*			// nnnn is count, unless '1111', then int count follows
				1011 xxxx							// unused
				1100 xxxx							// unused
		dict	1101 0000	key/value object pairs	// a single null object indicates the end.
		dict	1101 nnnn	[int]	keyref* objref*	// nnnn is count, unless '1111', then int count follows
				1110 xxxx							// unused
				1111 xxxx							// unused
	
	OFFSET TABLE (for version 0 binary plists)
		list of ints, byte size of which is given in trailer
		-- these are the byte offsets into the file
		-- number of these is in the trailer
	
	TRAILER
		signature ("END!") for streamed binary plists.
		OR for version 0 binary plists:
			byte size of offset ints in offset table
			byte size of object refs in arrays and dicts
			number of offsets in offset table (also is number of objects)
			element # in offset table which is top level object
			offset table offset
===========================================================================================================================*/
