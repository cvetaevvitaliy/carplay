/*
	File:    	CFCompat.c
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

#include "APSCommonServices.h" // Include early for TARGET_* conditionals.
#include "APSDebugServices.h"  // Include early for DEBUG* conditionals.

#if( TARGET_HAS_STD_C_LIB )
	#include <string.h>
#endif

#include "CFCompat.h"
#include "StringUtils.h"

#if( CFLITE_ENABLED )

#include "CFLite.h"
#include "TimeUtils.h"

#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	#include "utfconv.h"
#endif
#if( CFL_BINARY_PLISTS )
	#include "CFLiteBinaryPlist.h"
#endif

#if 0
#pragma mark == Base ==
#endif

//===========================================================================================================================
//	CFGetTypeID
//===========================================================================================================================

CFTypeID	CFGetTypeID( CFTypeRef inObj )
{
	CFTypeID		id;
	
	id = 0;
	CFLGetTypeID( inObj, &id );
	return( id );
}

//===========================================================================================================================
//	CFGetRetainCount
//===========================================================================================================================

CFIndex	CFGetRetainCount( CFTypeRef inObj )
{
	CFIndex		n;
	
	n = 0;
	CFLGetRetainCount( inObj, &n );
	return( n );
}

//===========================================================================================================================
//	CFRetain
//===========================================================================================================================

CFTypeRef	CFRetain( CFTypeRef inObject )
{
	return( CFLRetain( inObject ) );
}

//===========================================================================================================================
//	CFRelease
//===========================================================================================================================

void	CFRelease( CFTypeRef inObject )
{
	CFLRelease( inObject );
}

//===========================================================================================================================
//	CFEqual
//===========================================================================================================================

Boolean	CFEqual( CFTypeRef inLeft, CFTypeRef inRight )
{
	return( CFLEqual( inLeft, inRight ) );
}

//===========================================================================================================================
//	CFHash
//===========================================================================================================================

CFHashCode	CFHash( CFTypeRef inObject )
{
	return( CFLHash( inObject ) );
}

//===========================================================================================================================
//	CFPropertyListCreateDeepCopy
//===========================================================================================================================

CFPropertyListRef
	CFPropertyListCreateDeepCopy( 
		CFAllocatorRef 		inAllocator, 
		CFPropertyListRef	inPropertyList, 
		CFOptionFlags		inMutabilityOption )
{
	CFLObjectRef		obj;
	
	(void) inAllocator;			// Unused
	(void) inMutabilityOption;	// Unused
	
	obj = NULL;
	CFLCopy( inPropertyList, &obj );
	return( obj );
}

#if 0
#pragma mark -
#pragma mark == Array ==
#endif

//===========================================================================================================================
//	CFArrayGetTypeID
//===========================================================================================================================

CFTypeID	CFArrayGetTypeID( void )
{
	return( CFLArrayGetTypeID() );
}

//===========================================================================================================================
//	CFArrayCreate
//===========================================================================================================================

CFArrayRef
	CFArrayCreate( 
		CFAllocatorRef				inAllocator, 
		const void **				inValues, 
		CFIndex						inCount, 
		const CFArrayCallBacks *	inCallBacks )
{
	CFArrayRef		obj;
	CFArrayRef		tmp;
	CFIndex			i;
	OSStatus		err;
	
	obj = NULL;
	tmp = NULL;
	require_action( inValues || ( inCount == 0 ), exit, err = kParamErr );
	
	err = CFLArrayCreate( inAllocator, inCallBacks, &tmp );
	require_noerr( err, exit );
	
	for( i = 0; i < inCount; ++i )
	{
		err = CFLArrayAppendValue( tmp, inValues[ i ] );
		require_noerr( err, exit );
	}
	
	obj = tmp;
	tmp = NULL;
	
exit:
	if( tmp ) CFLRelease( tmp );
	return( obj );	
}

//===========================================================================================================================
//	CFArrayCreateCopy
//===========================================================================================================================

CFArrayRef	CFArrayCreateCopy( CFAllocatorRef inAllocator, CFArrayRef inArray )
{
	CFArrayRef		obj;
	
	obj = NULL;
	CFLArrayCreateCopy( inAllocator, inArray, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFArrayCreateMutable
//===========================================================================================================================

CFMutableArrayRef	CFArrayCreateMutable( CFAllocatorRef inAllocator, CFIndex inCapacity, const CFArrayCallBacks *inCallBacks )
{
	CFArrayRef		obj;
	
	(void) inCapacity; // Unused
	
	obj = NULL;
	CFLArrayCreate( inAllocator, inCallBacks, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFArrayCreateMutableCopy
//===========================================================================================================================

CFMutableArrayRef	CFArrayCreateMutableCopy( CFAllocatorRef inAllocator, CFIndex inCapacity, CFArrayRef inArray )
{
	CFArrayRef		obj;
	
	(void) inCapacity; // Unused
	
	obj = NULL;
	CFLArrayCreateCopy( inAllocator, inArray, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFArrayGetCount
//===========================================================================================================================

CFIndex	CFArrayGetCount( CFArrayRef inObject )
{
	CFIndex		n;
	
	n = 0;
	CFLArrayGetCount( inObject, &n );
	return( n );
}

//===========================================================================================================================
//	CFArrayGetFirstIndexOfValue
//===========================================================================================================================

CFIndex	CFArrayGetFirstIndexOfValue( CFArrayRef inArray, CFRange inRange, const void *inValue )
{
	CFIndex			i, n;
	CFIndex			maxN;
	CFTypeRef		obj;
	
	i = inRange.location;
	n = i + inRange.length;
	maxN = CFArrayGetCount( inArray );
	require( ( i >= 0 ) && ( i <= n ), exit );
	require( ( n >= 0 ) && ( n <= maxN ), exit );
	
	for( ; i < n; ++i )
	{
		obj = CFArrayGetValueAtIndex( inArray, i );
		if( CFEqual( obj, inValue ) )
		{
			return( i );
		}
	}
	
exit:
	return( -1 );
}

//===========================================================================================================================
//	CFArrayGetValues
//===========================================================================================================================

void	CFArrayGetValues( CFArrayRef inArray, CFRange inRange, const void **inValues )
{
	CFLArrayGetValues( inArray, inRange.location, inRange.length, inValues );
}

//===========================================================================================================================
//	CFArrayGetValueAtIndex
//===========================================================================================================================

void *	CFArrayGetValueAtIndex( CFArrayRef inObject, CFIndex inIndex )
{
	void *		value;
	
	value = NULL;
	CFLArrayGetValueAtIndex( inObject, inIndex, &value );
	return( value );
}

//===========================================================================================================================
//	CFArraySetValueAtIndex
//===========================================================================================================================

void	CFArraySetValueAtIndex( CFMutableArrayRef inObject, CFIndex inIndex, const void *inValue )
{
	CFLArraySetValueAtIndex( inObject, inIndex, inValue );
}

//===========================================================================================================================
//	CFArrayInsertValueAtIndex
//===========================================================================================================================

void	CFArrayInsertValueAtIndex( CFMutableArrayRef inObject, CFIndex inIndex, const void *inValue )
{
	CFLArrayInsertValueAtIndex( inObject, inIndex, inValue );
}

//===========================================================================================================================
//	CFArrayAppendValue
//===========================================================================================================================

void	CFArrayAppendValue( CFMutableArrayRef inObject, const void *inValue )
{
	CFLArrayAppendValue( inObject, inValue );
}

//===========================================================================================================================
//	CFArrayRemoveValueAtIndex
//===========================================================================================================================

void	CFArrayRemoveValueAtIndex( CFMutableArrayRef inObject, CFIndex inIndex )
{
	CFLArrayRemoveValueAtIndex( inObject, inIndex );
}

//===========================================================================================================================
//	CFArrayRemoveAllValues
//===========================================================================================================================

void	CFArrayRemoveAllValues( CFMutableArrayRef inObject )
{
	CFLArrayRemoveAllValues( inObject );
}

//===========================================================================================================================
//	CFArrayContainsValue
//===========================================================================================================================

Boolean	CFArrayContainsValue( CFArrayRef inArray, CFRange inRange, const void *inValue )
{
	return( CFLArrayContainsValue( inArray, inRange.location, inRange.location + inRange.length, inValue ) );
}

//===========================================================================================================================
//	CFArrayAppendArray
//===========================================================================================================================

void	CFArrayAppendArray( CFMutableArrayRef inDstArray, CFArrayRef inSrcArray, CFRange inSrcRange )
{
	CFIndex			srcCount;
	CFIndex			srcIndex, endIndex;
	
	srcCount = CFArrayGetCount( inSrcArray );
	srcIndex = inSrcRange.location;
	endIndex = srcIndex + inSrcRange.length;
	require( ( srcIndex >= 0 ) && ( srcIndex <= srcCount ), exit );
	require( ( endIndex >= 0 ) && ( endIndex <= srcCount ), exit );
	
	for( ; srcIndex < endIndex; ++srcIndex )
	{
		CFArrayAppendValue( inDstArray, CFArrayGetValueAtIndex( inSrcArray, srcIndex ) );
	}
	
exit:
	return;
}

#if 0
#pragma mark -
#pragma mark == Boolean ==
#endif

//===========================================================================================================================
//	CFBooleanGetTypeID
//===========================================================================================================================

CFTypeID	CFBooleanGetTypeID( void )
{
	return( CFLBooleanGetTypeID() );
}

//===========================================================================================================================
//	CFBooleanGetValue
//===========================================================================================================================

Boolean	CFBooleanGetValue( CFBooleanRef inBoolean )
{
	return( (Boolean)( inBoolean == kCFBooleanTrue ) );
}

#if 0
#pragma mark -
#pragma mark == Data ==
#endif

//===========================================================================================================================
//	CFDataGetTypeID
//===========================================================================================================================

CFTypeID	CFDataGetTypeID( void )
{
	return( CFLDataGetTypeID() );
}

//===========================================================================================================================
//	CFDataCreate
//===========================================================================================================================

CFDataRef	CFDataCreate( CFAllocatorRef inAllocator, const uint8_t *inData, CFIndex inSize )
{
	CFDataRef		obj;
	
	obj = NULL;
	CFLDataCreate( inAllocator, inData, (size_t) inSize, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFDataCreateMutable
//===========================================================================================================================

CFMutableDataRef	CFDataCreateMutable( CFAllocatorRef inAllocator, CFIndex inCapacity )
{
	CFMutableDataRef		obj;
	
	(void) inCapacity; // Unused
	
	obj = NULL;
	CFLDataCreate( inAllocator, NULL, 0, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFDataCreateMutableCopy
//===========================================================================================================================

CFMutableDataRef	CFDataCreateMutableCopy( CFAllocatorRef inAllocator, CFIndex inCapacity, CFDataRef inData )
{
	CFMutableDataRef		obj;
	
	(void) inCapacity; // Unused
	
	obj = NULL;
	CFLDataCreate( inAllocator, CFDataGetBytePtr( inData ), (size_t) CFDataGetLength( inData ), &obj );
	return( obj );
}

//===========================================================================================================================
//	CFDataGetLength
//===========================================================================================================================

CFIndex	CFDataGetLength( CFDataRef inObject )
{
	size_t		size;
	
	size = 0;
	CFLDataGetDataPtr( inObject, NULL, &size );
	return( (CFIndex) size );
}

//===========================================================================================================================
//	CFDataGetBytePtr
//===========================================================================================================================

const uint8_t *	CFDataGetBytePtr( CFDataRef inObject )
{
	uint8_t *		p;
	
	p = NULL;
	CFLDataGetDataPtr( inObject, &p, NULL );
	return( p );
}

//===========================================================================================================================
//	CFDataGetBytes
//===========================================================================================================================

void	CFDataGetBytes( CFDataRef inObject, CFRange inRange, uint8_t *inBuffer )
{
	uint8_t *		p;
	size_t			n;
	
	p = NULL;
	n = 0;
	CFLDataGetDataPtr( inObject, &p, &n );
	require( p && ( n >= ( (size_t)( inRange.location + inRange.length ) ) ), exit );
	
	memmove( inBuffer, p + inRange.location, (size_t) inRange.length );
	
exit:
	return;
}

//===========================================================================================================================
//	CFDataGetMutableBytePtr
//===========================================================================================================================

uint8_t *	CFDataGetMutableBytePtr( CFDataRef inObject )
{
	uint8_t *		p;
	
	p = NULL;
	CFLDataGetDataPtr( inObject, &p, NULL );
	return( p );
}

//===========================================================================================================================
//	CFDataAppendBytes
//===========================================================================================================================

void	CFDataAppendBytes( CFMutableDataRef inObject, const uint8_t *inData, CFIndex inSize )
{
	CFLDataAppendData( inObject, inData, (size_t) inSize );
}

#if 0
#pragma mark -
#pragma mark == Date ==
#endif

//===========================================================================================================================
//	CFDateGetTypeID
//===========================================================================================================================

CFTypeID	CFDateGetTypeID( void )
{
	return( CFLDateGetTypeID() );
}

//===========================================================================================================================
//	CFDateCreate
//===========================================================================================================================

CFDateRef	CFDateCreate( CFAllocatorRef inAllocator, CFAbsoluteTime inTime )
{
	CFDateRef				date;
	CFLDateComponents		comps;
	
	inTime += ( ( (int64_t) kDaysToCoreFoundationEpoch ) * kSecondsPerDay );
	SecondsToYMD_HMS( (int64_t) inTime, &comps.year, &comps.month, &comps.day, &comps.hour, &comps.minute, &comps.second );
	date = NULL;
	CFLDateCreate( inAllocator, &comps, &date );
	return( date );
}

//===========================================================================================================================
//	CFDateCreateWithComponents
//===========================================================================================================================

CFDateRef
	CFDateCreateWithComponents( 
		CFAllocatorRef	inAllocator, 
		int				inYear, 
		int				inMonth, 
		int				inDay, 
		int				inHour, 
		int				inMinute, 
		int				inSecond )
{
	CFDateRef				date;
	CFLDateComponents		components = { inYear, inMonth, inDay, inHour, inMinute, inSecond };
	
	date = NULL;
	CFLDateCreate( inAllocator, &components, &date );
	return( date );
}

//===========================================================================================================================
//	CFDateGetAbsoluteTime
//===========================================================================================================================

CFAbsoluteTime	CFDateGetAbsoluteTime( CFDateRef inDate )
{
	CFAbsoluteTime			at;
	OSStatus				err;
	CFLDateComponents		d;
	
	at = 0;
	err = CFLDateGetDate( inDate, &d );
	require_noerr( err, exit );
	
	at = (CFAbsoluteTime) YMD_HMStoSeconds( d.year, d.month, d.day, d.hour, d.minute, d.second, kDaysToCoreFoundationEpoch );
	
exit:
	return( at );
}

//===========================================================================================================================
//	CFDateCompare
//===========================================================================================================================

CFComparisonResult	CFDateCompare( CFDateRef inA, CFDateRef inB, void *inContext )
{
	CFLDateComponents		c;
	int64_t					a, b;
	
	(void) inContext;
	
	CFLDateGetDate( inA, &c );
	a = YMD_HMStoSeconds( c.year, c.month, c.day, c.hour, c.minute, c.second, kDaysToCoreFoundationEpoch );
	
	CFLDateGetDate( inB, &c );
	b = YMD_HMStoSeconds( c.year, c.month, c.day, c.hour, c.minute, c.second, kDaysToCoreFoundationEpoch );
	
	return( ( a < b ) ? -1 : ( a > b ) ? 1 : 0 );
}

#if 0
#pragma mark -
#pragma mark == Dictionary ==
#endif

//===========================================================================================================================
//	CFDictionaryGetTypeID
//===========================================================================================================================

CFTypeID	CFDictionaryGetTypeID( void )
{
	return( CFLDictionaryGetTypeID() );
}

//===========================================================================================================================
//	CFDictionaryCreate
//===========================================================================================================================

CFDictionaryRef
	CFDictionaryCreate(
		CFAllocatorRef 						inAllocator,
		const void **						inKeys,
		const void **						inValues,
		CFIndex								inCount,
		const CFDictionaryKeyCallBacks *	inKeyCallBacks,
		const CFDictionaryValueCallBacks *	inValueCallBacks )
{
	CFMutableDictionaryRef		obj;
	CFIndex						i;
	
	obj = CFDictionaryCreateMutable( inAllocator, inCount, inKeyCallBacks, inValueCallBacks );
	require( obj, exit );
	
	for( i = 0; i < inCount; ++i )
	{
		OSStatus		err;
		
		err = CFLDictionarySetValue( obj, inKeys[ i ], inValues[ i ] );
		check_noerr( err );
		if( err )
		{
			CFRelease( obj );
			obj = NULL;
			goto exit;
		}
	}
	
exit:
	return( obj );
}

//===========================================================================================================================
//	CFDictionaryCreateMutable
//===========================================================================================================================

CFMutableDictionaryRef
	CFDictionaryCreateMutable( 
		CFAllocatorRef 						inAllocator, 
		CFIndex								inCapacity, 
		const CFDictionaryKeyCallBacks *	inKeyCallBacks, 
		const CFDictionaryValueCallBacks *	inValueCallBacks )
{
	CFMutableDictionaryRef		obj;
	
	obj = NULL;
	CFLDictionaryCreate( inAllocator, inCapacity, inKeyCallBacks, inValueCallBacks, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFDictionaryGetCount
//===========================================================================================================================

CFIndex	CFDictionaryGetCount( CFDictionaryRef inObject )
{
	CFIndex		n;
	
	n = 0;
	CFLDictionaryGetCount( inObject, &n );
	return( n );
}

//===========================================================================================================================
//	CFDictionaryGetValue
//===========================================================================================================================

const void *	CFDictionaryGetValue( CFDictionaryRef inObject, const void *inKey )
{
	void *		value;
	
	value = NULL;
	CFLDictionaryGetValue( inObject, inKey, &value );
	return( value );
}

//===========================================================================================================================
//	CFDictionarySetValue
//===========================================================================================================================

void	CFDictionarySetValue( CFMutableDictionaryRef inObject, const void *inKey, const void *inValue )
{
	CFLDictionarySetValue( inObject, inKey, inValue );
}

//===========================================================================================================================
//	CFDictionaryAddValue
//===========================================================================================================================

void	CFDictionaryAddValue( CFMutableDictionaryRef inObject, const void *inKey, const void *inValue )
{
	CFLDictionaryAddValue( inObject, inKey, inValue );
}

//===========================================================================================================================
//	CFDictionaryRemoveValue
//===========================================================================================================================

void	CFDictionaryRemoveValue( CFMutableDictionaryRef inObject, const void *inKey )
{
	CFLDictionaryRemoveValue( inObject, inKey );
}

//===========================================================================================================================
//	CFDictionaryRemoveAllValues
//===========================================================================================================================

void	CFDictionaryRemoveAllValues( CFMutableDictionaryRef inObject )
{
	CFLDictionaryRemoveAllValues( inObject );
}

//===========================================================================================================================
//	CFDictionaryGetKeysAndValues
//===========================================================================================================================

void	CFDictionaryGetKeysAndValues( CFDictionaryRef inObject, const void **ioKeys, const void **ioValues )
{
	CFLDictionaryGetKeysAndValues( inObject, ioKeys, ioValues );
}

//===========================================================================================================================
//	CFDictionaryApplyFunction
//===========================================================================================================================

void	CFDictionaryApplyFunction( CFDictionaryRef inDict, CFDictionaryApplierFunction inApplier, void *inContext )
{
	CFLDictionaryApplyFunction( inDict, inApplier, inContext );
}

#if 0
#pragma mark -
#pragma mark == Number ==
#endif

//===========================================================================================================================
//	CFNumberGetTypeID
//===========================================================================================================================

CFTypeID	CFNumberGetTypeID( void )
{
	return( CFLNumberGetTypeID() );
}

//===========================================================================================================================
//	CFNumberCreate
//===========================================================================================================================

CFNumberRef CFNumberCreate( CFAllocatorRef inAllocator, CFNumberType inType, const void *inValuePtr )
{
	CFNumberRef		obj;
	OSStatus		err;
	
	obj = NULL;
	err = CFLNumberCreate( inAllocator, inType, inValuePtr, &obj );
	require_noerr( err, exit );
	
exit:
	return( obj );
}

//===========================================================================================================================
//	CFNumberGetValue
//===========================================================================================================================

Boolean CFNumberGetValue( CFNumberRef inNumber, CFNumberType inType, void *outValue )
{
	return( CFLNumberGetValue( inNumber, inType, outValue ) == kNoErr );
}

#if 0
#pragma mark -
#pragma mark == String ==
#endif

//===========================================================================================================================
//	CFStringGetTypeID
//===========================================================================================================================

CFTypeID	CFStringGetTypeID( void )
{
	return( CFLStringGetTypeID() );
}

//===========================================================================================================================
//	CFStringCreateMutable
//===========================================================================================================================

CFMutableStringRef	CFStringCreateMutable( CFAllocatorRef inAllocator, CFIndex inMaxSize )
{
	CFStringRef		obj;
	
	(void) inMaxSize; // Unused
	
	obj = NULL;
	CFLStringCreateWithText( inAllocator, NULL, 0, &obj );
	return( obj );
}

//===========================================================================================================================
//	CFStringCreateMutableCopy
//===========================================================================================================================

CFMutableStringRef	CFStringCreateMutableCopy( CFAllocatorRef inAllocator, CFIndex inMaxLength, CFStringRef inString )
{
	CFStringRef			obj;
	OSStatus			err;
	const char *		p;
	size_t				n;
	
	(void) inMaxLength;	// Unused
	
	obj = NULL;
	
	err = CFLStringGetCStringPtr( inString, &p, &n );
	require_noerr( err, exit );
	
	err = CFLStringCreateWithText( inAllocator, p, n, &obj );
	require_noerr( err, exit );
	
exit:
	return( obj );
}

//===========================================================================================================================
//	CFStringCreateWithBytes
//===========================================================================================================================

CFStringRef
	CFStringCreateWithBytes( 
		CFAllocatorRef 		inAllocator, 
		const uint8_t *		inBytes, 
		CFIndex				inSize, 
		CFStringEncoding	inEncoding, 
		Boolean				inIsExternal )
{
	CFStringRef		obj;
	OSStatus		err;
#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	uint8_t *		utf8Ptr;
	size_t			utf8Len;
#endif
	
	(void) inIsExternal;	// Unused
	
	obj = NULL;

#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	utf8Ptr	= NULL;
	if( ( inEncoding == kCFStringEncodingWindowsLatin1 ) || 
		( inEncoding == kCFStringEncodingISOLatin1 ) )
	{
		err = latin1_to_utf8_copy( inBytes, (size_t) inSize, &utf8Ptr, &utf8Len, '/', 0 );
		require_noerr( err, exit );
		
		inBytes = utf8Ptr;
		inSize  = (CFIndex) utf8Len;
	}
	else if( ( inEncoding == kCFStringEncodingUTF16 )   || 
			 ( inEncoding == kCFStringEncodingUTF16BE ) || 
			 ( inEncoding == kCFStringEncodingUTF16LE ) )
	{
		int		flags;
		
		if(      inEncoding == kCFStringEncodingUTF16BE )	flags = UTF_BIG_ENDIAN;
		else if( inEncoding == kCFStringEncodingUTF16LE )	flags = UTF_LITTLE_ENDIAN;
		else												flags = 0;
		err = utf8_encodestr_copy( (const uint16_t *) inBytes, inSize, &utf8Ptr, &utf8Len, '/', flags );
		require_noerr( err, exit );
		
		inBytes = utf8Ptr;
		inSize  = (CFIndex) utf8Len;
	}
#else
	require_action( ( inEncoding == kCFStringEncodingUTF8 ) ||
					( inEncoding == kCFStringEncodingASCII ), exit, err = kUnsupportedErr );
#endif
	
	err = CFLStringCreateWithText( inAllocator, inBytes, (size_t) inSize, &obj );
	require_noerr( err, exit );
	
exit:
#if( CFCOMPAT_HAS_UNICODE_SUPPORT )
	if( utf8Ptr ) utffree( utf8Ptr );
#endif
	return( obj );
}

//===========================================================================================================================
//	CFStringCreateWithCString
//===========================================================================================================================

CFStringRef	CFStringCreateWithCString( CFAllocatorRef inAllocator, const char *inString, CFStringEncoding inEncoding )
{
	return( CFStringCreateWithBytes( inAllocator, (const uint8_t *) inString, (CFIndex) strlen( inString ), inEncoding, false ) );
}

//===========================================================================================================================
//	CFStringCreateWithFormat
//===========================================================================================================================

CFStringRef
	CFStringCreateWithFormat( 
		CFAllocatorRef	inAllocator, 
		CFDictionaryRef	inFormatOptions, 
		CFStringRef		inFormat, 
		... )
{
	CFStringRef			resultString;
	OSStatus			err;
	const char *		formatCStr;
	va_list				args;
	char *				str;
	int					n;
	
	resultString = NULL;
	require( inFormatOptions == NULL, exit ); // We don't support format options yet.
	
	err = CFLStringGetCStringPtr( inFormat, &formatCStr, NULL );
	require_noerr( err, exit );
	
	va_start( args, inFormat );
	n = VASPrintF( &str, formatCStr, args );
	va_end( args );
	require( n >= 0, exit );
	
	resultString = CFStringCreateWithCString( inAllocator, str, kCFStringEncodingUTF8 );
	free( str );
	require( resultString, exit );
	
exit:
	return( resultString );
}

//===========================================================================================================================
//	CFStringGetLength
//===========================================================================================================================

CFIndex	CFStringGetLength( CFStringRef inObject )
{
	CFIndex		n;
	
	n = 0;
	CFLStringGetLength( inObject, &n );
	return( n );
}

//===========================================================================================================================
//	CFStringGetMaximumSizeForEncoding
//===========================================================================================================================

CFIndex	CFStringGetMaximumSizeForEncoding( CFIndex inSize, CFStringEncoding inEncoding )
{
	(void) inEncoding; // Unused
	
	return( inSize * 4 ); // Max spec'd by Unicode is 4 bytes even though UTF-8 itself can support 6 bytes.
}

//===========================================================================================================================
//	CFStringGetCStringPtr
//===========================================================================================================================

const char *	CFStringGetCStringPtr( CFStringRef inString, CFStringEncoding inEncoding )
{
	const char *		s;
	
	s = NULL;
	require( inEncoding == kCFStringEncodingUTF8, exit );
	
	CFLStringGetCStringPtr( inString, &s, NULL );
	
exit:
	return( s );
}

//===========================================================================================================================
//	CFStringGetCString
//===========================================================================================================================

Boolean	CFStringGetCString( CFStringRef inString, char *outBuffer, CFIndex inMaxSize, CFStringEncoding inEncoding )
{
	OSStatus			err;
	const char *		str;
	size_t				size;
	
	(void ) inEncoding;					// Unused
	
	err = CFLStringGetCStringPtr( inString, &str, &size );
	require_noerr( err, exit );
	require_action_quiet( size < (size_t) inMaxSize, exit, err = kSizeErr );
	
	memcpy( outBuffer, str, size + 1 );
	
exit:
	return( (Boolean)( err ? false : true ) );
}

//===========================================================================================================================
//	CFStringGetPascalString
//===========================================================================================================================

Boolean	CFStringGetPascalString( CFStringRef inString, unsigned char *inBuffer, CFIndex inMaxSize, CFStringEncoding inEncoding )
{
	OSStatus			err;
	const char *		str;
	size_t				size;
	
	(void ) inEncoding;					// Unused
	
	err = CFLStringGetCStringPtr( inString, &str, &size );
	require_noerr( err, exit );
	require_action_quiet( size < (size_t) inMaxSize, exit, err = kSizeErr );
	
	inBuffer[ 0 ] = (uint8_t) size;
	memcpy( &inBuffer[ 1 ], str, size );
	
exit:
	return( (Boolean)( err ? false : true ) );
}

//===========================================================================================================================
//	CFStringGetBytes
//===========================================================================================================================

CFIndex
	CFStringGetBytes(
		CFStringRef			inString,
		CFRange				inRange,
		CFStringEncoding	inEncoding,
		UInt8				inLossByte,
		Boolean				inIsExternalRepresentation,
		UInt8 *				inBuffer,
		CFIndex				inMaxBufLen,
		CFIndex *			outUsedBufLen )
{
	CFIndex				nConverted;
	OSStatus			err;
	const char *		str;
	size_t				size;
	
	(void ) inEncoding;					// Unused
	(void ) inLossByte;					// Unused
	(void ) inIsExternalRepresentation; // Unused
	
	nConverted = 0;
	
	err = CFLStringGetCStringPtr( inString, &str, &size );
	require_noerr( err, exit );
	require( inRange.location <= ( (CFIndex) size ), exit );
	require( inRange.length <= ( (CFIndex) size ), exit );
	require( ( inRange.location + inRange.length ) <= ( (CFIndex) size ), exit );
	
	if( inRange.length > inMaxBufLen ) inRange.length = inMaxBufLen;
	if( inBuffer ) memmove( inBuffer, str + inRange.location, inRange.length );
	nConverted = inRange.length;
	
exit:
	if( outUsedBufLen ) *outUsedBufLen = nConverted;
	return( nConverted );
}

//===========================================================================================================================
//	CFStringCompare
//===========================================================================================================================

CFComparisonResult	CFStringCompare( CFStringRef inS1, CFStringRef inS2, CFOptionFlags inOptions )
{
	CFComparisonResult		cmp;
	OSStatus				err;
	const char *			s1;
	size_t					len1;
	const char *			s2;
	size_t					len2;
	int						n;
	Boolean					ignoreCase;
	
	cmp = kCFCompareLessThan;
	
	err = CFLStringGetCStringPtr( inS1, &s1, &len1 );
	require_noerr( err, exit );
	
	err = CFLStringGetCStringPtr( inS2, &s2, &len2 );
	require_noerr( err, exit );
	
	ignoreCase = (Boolean)( inOptions & kCFCompareCaseInsensitive );
	if( inOptions & kCFCompareNumerically ) n = TextCompareNatural( s1, len1, s2, len2, ignoreCase );
	else if( ignoreCase )					n = stricmp( s1, s2 );
	else									n = strcmp( s1, s2 );
	if(      n < 0 ) cmp = kCFCompareLessThan;
	else if( n > 0 ) cmp = kCFCompareGreaterThan;
	else			 cmp = kCFCompareEqualTo;
	
exit:
	return( cmp );
}

//===========================================================================================================================
//	CFStringGetIntValue
//===========================================================================================================================

int32_t	CFStringGetIntValue( CFStringRef inObject )
{
	int32_t				val;
	OSStatus			err;
	const char *		str;
	size_t				len;
	
	val = 0;
	
	err = CFLStringGetCStringPtr( inObject, &str, &len );
	require_noerr( err, exit );
	
	val = TextToInt32( str, len, 10 );
	
exit:
	return( val );
}

//===========================================================================================================================
//	CFStringFindAndReplace
//===========================================================================================================================

#if( TARGET_HAS_STD_C_LIB )
CFIndex
	CFStringFindAndReplace( 
		CFMutableStringRef	inString, 
		CFStringRef			inStringToFind, 
		CFStringRef			inReplacementString, 
		CFRange				inRangeToSearch, 
		CFOptionFlags		inCompareOptions )
{
	return( CFLStringFindAndReplace( inString, inStringToFind, inReplacementString, 
		inRangeToSearch.location, inRangeToSearch.length, inCompareOptions ) );
}
#endif

//===========================================================================================================================
//	CFStringFindWithOptions
//===========================================================================================================================

#if( TARGET_HAS_STD_C_LIB )
Boolean
	CFStringFindWithOptions(
		CFStringRef		inStringToSearch, 
		CFStringRef		inStringToFind, 
		CFRange			inRangeToSearch, 
		CFOptionFlags	inSearchOptions,
		CFRange *		outRange )
{
	Boolean				found;
	OSStatus			err;
	CFIndex				endOffset;
	const char *		sourceStr;
	const char *		sourcePtr;
	size_t				sourceLen;
	const char *		searchStr;
	size_t				searchLen;
	const char *		foundPtr;
	
	found = false;
	
	err = CFLStringGetCStringPtr( inStringToSearch, &sourceStr, &sourceLen );
	require_noerr( err, exit );
	require_action( inRangeToSearch.location >= 0, exit, err = kRangeErr );
	require_action( inRangeToSearch.location <= ( (CFIndex) sourceLen ), exit, err = kRangeErr );
	endOffset = inRangeToSearch.location + inRangeToSearch.length;
	require_action( endOffset >= inRangeToSearch.location, exit, err = kRangeErr );
	require_action( endOffset <= ( (CFIndex) sourceLen ), exit, err = kRangeErr );
	
	err = CFLStringGetCStringPtr( inStringToFind, &searchStr, &searchLen );
	require_noerr( err, exit );
	require_quiet( searchLen > 0, exit ); // The real CF seems to never match an empty search string.
	
	sourcePtr = sourceStr + inRangeToSearch.location;
	sourceLen = (size_t) inRangeToSearch.length;
	if( inSearchOptions & kCFCompareCaseInsensitive )	foundPtr = strncasestr( sourcePtr, searchStr, sourceLen );
	else												foundPtr = strnstr( sourcePtr, searchStr, sourceLen );
	if( foundPtr )
	{
		if( outRange )
		{
			outRange->location	= (CFIndex)( foundPtr - sourceStr );
			outRange->length	= (CFIndex) searchLen;
		}
		found = true;
	}
	
exit:
	return( found );
}
#endif

//===========================================================================================================================
//	CFStringAppendCString
//===========================================================================================================================

void	CFStringAppendCString( CFMutableStringRef inCFStr, const char *inCStr, CFStringEncoding inEncoding )
{
	OSStatus		err;
	
	require_action( inEncoding == kCFStringEncodingUTF8, exit, err = kUnsupportedErr );
	
	err = CFLStringAppendText( inCFStr, inCStr, kSizeCString );
	require_noerr( err, exit );
	
exit:
	return;
}

#if 0
#pragma mark -
#pragma mark == Plists ==
#endif

//===========================================================================================================================
//	CFPropertyListCreateData
//===========================================================================================================================

CFDataRef
	CFPropertyListCreateData( 
		CFAllocatorRef			inAllocator, 
		CFPropertyListRef		inPropertyList, 
		CFPropertyListFormat	inFormat, 
		CFOptionFlags			inOptions, 
		CFErrorRef *			outError )
{
	CFDataRef		data = NULL;
	
	(void) inOptions;
	
	if( 0 ) {}
#if( CFL_BINARY_PLISTS )
	else if( inFormat == kCFPropertyListBinaryFormat_v1_0 )
	{
		data = CFBinaryPlistV0CreateData( inPropertyList, NULL );
		require( data, exit );
	}
	else if( inFormat == kCFPropertyListBinaryFormat_CFLite )
	{
		CFBinaryPlistCreateData( inAllocator, inPropertyList, &data );
		require( data, exit );
	}
#endif
#if( CFL_XML )
	else if( inFormat == kCFPropertyListXMLFormat_v1_0 )
	{
		data = CFPropertyListCreateXMLData( inAllocator, inPropertyList );
		require( data, exit );
	}
#endif
	else
	{
		dlogassert( "Unsupported plist format: %d\n", (int) inFormat );
		goto exit;
	}
	
exit:
	if( outError ) *outError = NULL;
	return( data );
}

//===========================================================================================================================
//	CFPropertyListCreateWithData
//===========================================================================================================================

CFPropertyListRef
	CFPropertyListCreateWithData( 
		CFAllocatorRef			inAllocator, 
		CFDataRef				inData, 
		CFOptionFlags			inOptions, 
		CFPropertyListFormat *	outFormat, 
		CFErrorRef *			outError )
{
	CFPropertyListRef		plist = NULL;
	
#if( CFL_BINARY_PLISTS )
	const uint8_t *			ptr;
	size_t					len;
	
	ptr = CFDataGetBytePtr( inData );
	len = (size_t) CFDataGetLength( inData );
	plist = CFBinaryPlistV0CreateWithData( ptr, len, NULL );
	if( plist )
	{
		if( outFormat ) *outFormat = kCFPropertyListBinaryFormat_v1_0;
		goto exit;
	}
	
	plist = CFBinaryPlistCreateFromData( inAllocator, ptr, len, NULL );
	if( plist )
	{
		if( outFormat ) *outFormat = kCFPropertyListBinaryFormat_CFLite;
		goto exit;
	}
#else
	(void) inOptions;
#endif
	
#if( CFL_XML_PARSING )
	plist = CFPropertyListCreateFromXMLData( inAllocator, inData, inOptions, NULL );
	if( plist )
	{
		if( outFormat ) *outFormat = kCFPropertyListXMLFormat_v1_0;
		goto exit;
	}
#else
	(void) inOptions;
#endif
	
exit:
	if( outError ) *outError = NULL;
	return( plist );
}

#if 0
#pragma mark -
#pragma mark == Runtime ==
#endif

//===========================================================================================================================
//	_CFRuntimeRegisterClass
//===========================================================================================================================

CFTypeID	_CFRuntimeRegisterClass( const CFRuntimeClass * const inClass )
{
	CFTypeID			typeID;
	CFLRuntimeClass		c;
	
	c.name		= inClass->className;
	c.freeObj	= inClass->finalize;
	c.equal		= inClass->equal;
	c.hash		= inClass->hash;
	
	typeID = _kCFRuntimeNotATypeID;
	CFLRuntimeRegisterClass( &c, &typeID );
	return( typeID );
}

//===========================================================================================================================
//	_CFRuntimeCreateInstance
//===========================================================================================================================

CFTypeRef
		_CFRuntimeCreateInstance( 
			CFAllocatorRef	inAllocator, 
			CFTypeID		inTypeID, 
			CFIndex 		inExtraBytes, 
			unsigned char *	inCategory )
{
	CFTypeRef		obj;
	
	(void) inCategory; // Unused
	
	obj = NULL;
	CFLRuntimeCreateInstance( inAllocator, inTypeID, (size_t) inExtraBytes, (void *) &obj );
	return( obj );
}

#endif	// CFLITE_ENABLED

#if 0
#pragma mark -
#pragma mark == Utilities ==
#endif

//===========================================================================================================================
//	CFDateGetComponents
//===========================================================================================================================

#if( CFLITE_ENABLED )
OSStatus
	CFDateGetComponents( 
		CFDateRef	inDate, 
		int *		outYear, 
		int *		outMonth, 
		int *		outDay, 
		int *		outHour, 
		int *		outMinute, 
		int *		outSecond, 
		int *		outMicros )
{
	CFLDateComponents		components;
	
	CFLDateGetDate( inDate, &components );
	*outYear	= components.year;
	*outMonth	= components.month;
	*outDay		= components.day;
	*outHour	= components.hour;
	*outMinute	= components.minute;
	*outSecond	= components.second;
	if( outMicros ) *outMicros = 0;
	return( kNoErr );
}
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

