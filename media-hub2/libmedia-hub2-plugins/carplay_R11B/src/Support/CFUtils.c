/*
	File:    	CFUtils.c
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
	
	Copyright (C) 2000-2014 Apple Inc. All Rights Reserved.
*/

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#include "CFUtils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "MiscUtils.h"
#include "StringUtils.h"

#include CF_HEADER
#include CF_RUNTIME_HEADER

#if( CFL_BINARY_PLISTS )
	#include "CFLiteBinaryPlist.h"
#endif

#if 0
#pragma mark == Formatted Building ==
#endif

//===========================================================================================================================
//	Formatted Building
//===========================================================================================================================

// CFPropertyListStack

typedef struct	CFPropertyListStack	CFPropertyListStack;
struct	CFPropertyListStack
{
	CFPropertyListStack *		next;
	CFMutableArrayRef			array;
	CFMutableDictionaryRef		dict;
};

// Prototypes

static OSStatus
	__CFPropertyListAssociateObject( 
		CFMutableArrayRef 		inArray, 
		CFMutableDictionaryRef 	inDict, 
		CFStringRef * 			ioKey, 
		CFTypeRef 				inObj, 
		CFTypeRef *				ioTopObject );

static OSStatus
	__CFPropertyListStackPush( 
		CFPropertyListStack **	ioStack, 
		CFMutableArrayRef 		inArray, 
		CFMutableDictionaryRef 	inDict );

static OSStatus
	__CFPropertyListStackPop( 
		CFPropertyListStack **		ioStack, 
		CFMutableArrayRef *			outArray, 
		CFMutableDictionaryRef *	outDict );

static void	__CFPropertyListStackFree( CFPropertyListStack *inStack );

//===========================================================================================================================
//	CFPropertyListCreateFormatted
//===========================================================================================================================

OSStatus	CFPropertyListCreateFormatted( CFAllocatorRef inAllocator, void *outObj, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFPropertyListCreateFormattedVAList( inAllocator, outObj, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFPropertyListCreateFormattedVAList
//===========================================================================================================================

OSStatus	CFPropertyListCreateFormattedVAList( CFAllocatorRef inAllocator, void *outObj, const char *inFormat, va_list inArgs )
{
	return( CFPropertyListBuildFormatted( inAllocator, NULL, outObj, inFormat, inArgs ) );
}

//===========================================================================================================================
//	CFPropertyListAppendFormatted
//===========================================================================================================================

OSStatus	CFPropertyListAppendFormatted( CFAllocatorRef inAllocator, CFTypeRef inParent, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFPropertyListAppendFormattedVAList( inAllocator, inParent, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFPropertyListAppendFormattedVAList
//===========================================================================================================================

OSStatus
	CFPropertyListAppendFormattedVAList( 
		CFAllocatorRef	inAllocator, 
		CFTypeRef 		inParent, 
		const char *	inFormat, 
		va_list 		inArgs )
{
	return( CFPropertyListBuildFormatted( inAllocator, inParent, NULL, inFormat, inArgs ) );
}

//===========================================================================================================================
//	CFPropertyListBuildFormatted
//
//	Spec		Description									Parameters
//	-----------	-------------------------------------------	----------
//	'['			Array begin									none
//	']'			Array end									none
//	'{'			Dictionary begin							none
//	'}'			Dictionary end								none
//	
//	<key>=		Inline key string							none
//	%ks=		Key string (null terminated)				const char *key
//	%.*ks=		Key string (variable size)					int size, const char *key
//	%.nks=		Key string (n characters)					const char *key
//	%kC=		Key string (FourCharCode)					uint32_t code (e.g. 'APPL')
//	%kO=		Key string (CF object)						CFStringRef key
//	%kU=		Key string (UUID)							uint8_t uuid[ 16 ] (big endian)
//
//	<value>;	Inline value string							none
//	%s			String (null terminated)					const char *string
//	%.*s		String (variable size)						int size, const char *string
//	%.ns		String (n characters)						const char *string
//	%C			String (FourCharCode)						uint32_t code (e.g. 'APPL')
//	%i			Integer										int x
//	%lli		64-bit Integer								int64_t x
//	%f			Floating-point value						double x
//	%D			Data										const void *data, int size
//	%b			Boolean										int x (true/false)
//	%O			Object										CFPropertyListRef or NULL
//	%#O			Deep copy of object							CFPropertyListRef or NULL
//	%##O		Merge dictionary into dictionary			CFDictionaryRef
//	%.4a		String (IPv4: 1.2.3.4)						uint32_t *ipv4 (network byte order)
//	%.6a		String (MAC: 00:11:22:33:44:55)				uint8_t mac[ 6 ]
//	%.8a		String (Fibre: 00:11:22:33:44:55:66:77)		uint8_t addr[ 8 ]
//	%.16a		String (IPv6: fe80::217:f2ff:fec8:d6e7)		uint8_t ipv6[ 16 ]
//	%##a		String (IPv4, IPv6, etc. network address)	sockaddr *addr
//	%T			Date/Time									int year, int month, int day, int hour, int minute, int second
//	%U			UUID string									uint8_t uuid[ 16 ] (big endian)
//	%@			Receive current parent						CFTypeRef *outParent
//===========================================================================================================================

OSStatus
	CFPropertyListBuildFormatted( 
		CFAllocatorRef	inAllocator, 
		CFTypeRef 		inParent, 
		void *			outObj, 
		const char *	inFormat, 
		va_list 		inArgs )
{
	OSStatus					err;
	CFMutableArrayRef			array;
	CFMutableDictionaryRef		dict;
	CFPropertyListStack *		parentStack;
	CFStringRef					key;
	CFTypeRef					topObject;
	CFTypeRef					obj;
	CFTypeID					typeID;
	const unsigned char *		fmt;
	unsigned char				c;
	int							precision;
	int							altForm;
	int							longFlag;
	const unsigned char *		p;
	uint32_t					u32;
	int64_t						s64;
	double						d;
	unsigned char				buf[ 64 ];
	int							n;
	CFTypeRef *					objArg;
	
	// Initialize variables. The "array" and "dict" variables are used to track the current container. Only 1 can 
	// be valid at any time. 2 separate variables are used to avoid needing to doing more costly type ID checks.
		
	array 		= NULL;
	dict		= NULL;
	parentStack	= NULL;
	key			= NULL;
	topObject	= NULL;
	obj			= NULL;
	
	require_action( inFormat, exit, err = kParamErr );
	
	// Set up and verify the parent array or dictionary (if one was passed in).
	
	if( inParent )
	{
		typeID = CFGetTypeID( inParent );
		if(      typeID == CFArrayGetTypeID() )			array = (CFMutableArrayRef) inParent;
		else if( typeID == CFDictionaryGetTypeID() )	dict  = (CFMutableDictionaryRef) inParent;
		else
		{
			// Only an array, dictionary, or null can be the parent object.
			
			dlog( kLogLevelError, "%s: parent must be an array, dictionary, or null (typeID=%d)\n", __ROUTINE__, (int) typeID );
			err = kTypeErr;
			goto exit;
		}
	}
	
	// Parse the format string.
	
	fmt = (const unsigned char *) inFormat;
	for( c = *fmt; c != '\0'; c = *( ++fmt ) )
	{
		// Parse container specifiers.
		
		switch( c )
		{
			case '[':	// Array begin
				
				obj = CFArrayCreateMutable( inAllocator, 0, &kCFTypeArrayCallBacks );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				
				err = __CFPropertyListStackPush( &parentStack, array, dict );
				require_noerr( err, exit );
				
				array = (CFMutableArrayRef) obj;
				dict = NULL;
				continue;
			
			case ']':	// Array end
				
				err = __CFPropertyListStackPop( &parentStack, &array, &dict );
				require_noerr( err, exit );
				continue;
			
			case '{':	// Dictionary begin
				
				obj = CFDictionaryCreateMutable( inAllocator, 0, &kCFTypeDictionaryKeyCallBacks, 
					&kCFTypeDictionaryValueCallBacks );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				
				err = __CFPropertyListStackPush( &parentStack, array, dict );
				require_noerr( err, exit );
				
				array = NULL;
				dict = (CFMutableDictionaryRef) obj;
				continue;
			
			case '}':	// Dictionary end
				
				err = __CFPropertyListStackPop( &parentStack, &array, &dict );
				require_noerr( err, exit );
				continue;
			
			default:	// Non-container specifier
				break;
		}
				
		// Parse inline key/value strings.
		
		if( c != '%' )
		{
			p = fmt;
			for( ; ( c != '\0' ) && ( c != '=' ) && ( c != ';' ); c = *( ++fmt ) ) {}
			if( c == '=' )
			{
				require_action( !key, exit, err = kMalformedErr );
				require_action( dict, exit, err = kMalformedErr );
				
				key = CFStringCreateWithBytes( inAllocator, p, (CFIndex)( fmt - p ), kCFStringEncodingUTF8, false );
				require_action( key, exit, err = kNoMemoryErr );
			}
			else
			{
				obj = CFStringCreateWithBytes( inAllocator, p, (CFIndex)( fmt - p ), kCFStringEncodingUTF8, false );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				
				if( c == '\0' ) break;
			}
			continue;
		}
		
		// Parse flags.
		
		altForm  = 0;
		longFlag = 0;
		for( ;; )
		{
			c = *( ++fmt );
			if(       c == '#' ) ++altForm;
			else if ( c == 'l' ) ++longFlag;
			else break;
		}
		
		// Parse the precision.
		
		precision = -1;
		if( c == '.' )
		{
			c = *( ++fmt );
			if( c == '*' )
			{
				precision = va_arg( inArgs, int );
				require_action( precision >= 0, exit, err = kSizeErr );
				c = *( ++fmt );
			}
			else
			{
				precision = 0;
				for( ; isdigit( c ); c = *( ++fmt ) ) precision = ( precision * 10 ) + ( c - '0' );
				require_action( precision >= 0, exit, err = kSizeErr );
			}
		}
		
		// Parse key specifiers.
		
		if( c == 'k' )
		{
			require_action( !key, exit, err = kMalformedErr );
			require_action( dict, exit, err = kMalformedErr );
					
			c = *( ++fmt );
			switch( c )
			{
				case 's':	// %ks: Key String (e.g. "<key>my-key</key>")
				
					p = va_arg( inArgs, const unsigned char * );
					require_action( p, exit, err = kParamErr );
					
					if( precision >= 0 )
					{
						precision = (int) strnlen( (const char *) p, (size_t) precision );
						key = CFStringCreateWithBytes( inAllocator, p, precision, kCFStringEncodingUTF8, false );
						require_action( key, exit, err = kNoMemoryErr );
					}
					else
					{
						key = CFStringCreateWithCString( inAllocator, (const char *) p, kCFStringEncodingUTF8 );
						require_action( key, exit, err = kNoMemoryErr );
					}
					break;
				
				case 'C':	// %kC: Key FourCharCode (e.g. "<key>AAPL</key>")
					
					require_action( precision == -1, exit, err = kFormatErr );
					
					u32 = va_arg( inArgs, uint32_t );
					buf[ 0 ] = (unsigned char)( ( u32 >> 24 ) & 0xFF );
					buf[ 1 ] = (unsigned char)( ( u32 >> 16 ) & 0xFF );
					buf[ 2 ] = (unsigned char)( ( u32 >>  8 ) & 0xFF );
					buf[ 3 ] = (unsigned char)(   u32         & 0xFF );
					key = CFStringCreateWithBytes( inAllocator, buf, 4, kCFStringEncodingMacRoman, false );
					require_action( key, exit, err = kNoMemoryErr );
					break;
				
				case 'o':	// %kO: Key Object (e.g. "<key>my-key</key>")
				case 'O':
					
					require_action( precision == -1, exit, err = kFormatErr );
					
					key = va_arg( inArgs, CFStringRef );
					require_action( key, exit, err = kParamErr );
					
					CFRetain( key );
					break;
				
				case 'U':	// %kU: UUID Key UUID string (e.g. "<key>8129b4b2-86dd-4f40-951f-6be834da5b8e</key>")
					
					require_action( precision == -1, exit, err = kFormatErr );
					
					p = va_arg( inArgs, const unsigned char * );
					require_action( p, exit, err = kParamErr );
					
					key = CFStringCreateWithCString( NULL, UUIDtoCString( p, 0, buf ), kCFStringEncodingUTF8 );
					require_action( key, exit, err = kNoMemoryErr );
					break;
				
				default:
					dlog( kLogLevelError, "%s: unknown key specifier: %s\n", __ROUTINE__, fmt );
					err = kFormatErr;
					goto exit;
				
			}
			c = *( ++fmt );
			require_action( c == '=', exit, err = kFormatErr );
			continue;
		}
		
		// Parse value specifiers.
		
		switch( c )
		{
			case 's':	// %s: String (e.g. "<string>my-string</string>")
								
				p = va_arg( inArgs, const unsigned char * );
				if( !p )
				{
					// Null string so just ignore this entry and release its key.
					
					if( key )
					{
						CFRelease( key );
						key = NULL;
					}
					break;
				}
				
				if( precision >= 0 )
				{
					precision = (int) strnlen( (const char *) p, (size_t) precision );
					obj = CFStringCreateWithBytes( inAllocator, p, precision, kCFStringEncodingUTF8, false );
					require_action( obj, exit, err = kNoMemoryErr );
				}
				else
				{
					obj = CFStringCreateWithCString( inAllocator, (const char *) p, kCFStringEncodingUTF8 );
					require_action( obj, exit, err = kNoMemoryErr );
				}
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;

			case 'C':	// %C: FourCharCode (e.g. "<string>AAPL</string>")
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				u32 = va_arg( inArgs, uint32_t );
				buf[ 0 ] = (unsigned char)( ( u32 >> 24 ) & 0xFF );
				buf[ 1 ] = (unsigned char)( ( u32 >> 16 ) & 0xFF );
				buf[ 2 ] = (unsigned char)( ( u32 >>  8 ) & 0xFF );
				buf[ 3 ] = (unsigned char)(   u32         & 0xFF );
				obj = CFStringCreateWithBytes( inAllocator, buf, 4, kCFStringEncodingMacRoman, false );
				require_action( obj, exit, err = kNoMemoryErr );
								
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;

			case 'i':	// %i: Integer (e.g. "<integer>-3</integer>")
				require_action( precision == -1, exit, err = kFormatErr );
				
				check( ( longFlag == 0 ) || ( longFlag == 2 ) );
				if( longFlag == 2 )	s64 = va_arg( inArgs, int64_t );
				else				s64 = va_arg( inArgs, int );
				obj = CFNumberCreateInt64( s64 );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;
			
			case 'f':	// %f: Floating-point value (e.g. "<real>3.14</real>")
				require_action( precision == -1, exit, err = kFormatErr );
				
				d = va_arg( inArgs, double );
				obj = CFNumberCreate( inAllocator, kCFNumberDoubleType, &d );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;
			
			case 'd':	// %D: Data (e.g. "<data>ABEiMw==</data>")
			case 'D':
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				p = va_arg( inArgs, const unsigned char * );
				n = va_arg( inArgs, int );
				require_action( p || ( n == 0 ), exit, err = kParamErr );
				obj = CFDataCreate( inAllocator, p, n );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;

			case 'b':	// %b: Boolean (e.g. "<true/>")
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				n = va_arg( inArgs, int );
				err = __CFPropertyListAssociateObject( array, dict, &key, n ? kCFBooleanTrue : kCFBooleanFalse, &topObject );
				require_noerr( err, exit );
				break;

			case 'o':	// %O: Object
			case 'O':
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				obj = va_arg( inArgs, CFTypeRef );
				if( obj )
				{
					if( altForm == 1 )
					{
						obj = CFPropertyListCreateDeepCopy( inAllocator, obj, kCFPropertyListMutableContainersAndLeaves );
						require_action( obj, exit, err = kNoMemoryErr );
						
						err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
						CFRelease( obj );
						require_noerr( err, exit );
					}
					else if( altForm == 2 )
					{
						require_action( !key, exit, err = kParamErr );
						require_action( dict, exit, err = kParamErr );
						require_action( CFGetTypeID( obj ) == CFDictionaryGetTypeID(), exit, err = kTypeErr );
						
						err = CFDictionaryMergeDictionary( dict, (CFDictionaryRef) obj );
						require_noerr( err, exit );
					}
					else
					{
						err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
						require_noerr( err, exit );
					}
				}
				else
				{
					// Null object so just ignore this entry and release its key.
					
					if( key )
					{
						CFRelease( key );
						key = NULL;
					}
				}
				break;
						
			case 'a':	// %a: Address String (%.4a=IPv4, %.6a=Ethernet, %.8a=Fibre Channel)
				
				p = va_arg( inArgs, const unsigned char * );
				require_action( p, exit, err = kParamErr );
				
				if( altForm == 2 )
				{
					n = SNPrintF( (char *) buf, sizeof( buf ), "%##a", p );
					require_action( n > 0, exit, err = kUnknownErr );
				}
				else
				{
					n = SNPrintF( (char *) buf, sizeof( buf ), "%.*a", precision, p );
					require_action( n > 0, exit, err = kUnknownErr );
				}
				
				obj = CFStringCreateWithBytes( inAllocator, buf, n, kCFStringEncodingUTF8, false );
				require_action( obj, exit, err = kNoMemoryErr );
								
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;
			
			case 'T':	// %T: Date/Time (e.g. "<date>2006-05-26T02:37:16Z</date>")
			{
				int		year, month, day, hour, minute, second;
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				year	= va_arg( inArgs, int );
				month	= va_arg( inArgs, int );
				day		= va_arg( inArgs, int );
				hour	= va_arg( inArgs, int );
				minute	= va_arg( inArgs, int );
				second	= va_arg( inArgs, int );
				obj = CFDateCreateWithComponents( inAllocator, year, month, day, hour, minute, second );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;
			}
			
			case 'U':	// %U: UUID string (e.g. "<string>8129b4b2-86dd-4f40-951f-6be834da5b8e</string>")
				
				p = va_arg( inArgs, const unsigned char * );
				require_action( p, exit, err = kParamErr );
				
				obj = CFStringCreateWithCString( NULL, UUIDtoCString( p, 0, buf ), kCFStringEncodingUTF8 );
				require_action( obj, exit, err = kNoMemoryErr );
				
				err = __CFPropertyListAssociateObject( array, dict, &key, obj, &topObject );
				CFRelease( obj );
				require_noerr( err, exit );
				break;
			
			case '@':	// %@: Receive current parent
				
				require_action( precision == -1, exit, err = kFormatErr );
				
				objArg = va_arg( inArgs, CFTypeRef * );
				require_action( objArg, exit, err = kParamErr );
				
				if( array )		obj = array;
				else if( dict )	obj = dict;
				else			obj = NULL;
				*objArg = obj;
				break;
			
			default:
				dlog( kLogLevelError, "%s: unknown format char: %s\n", __ROUTINE__, fmt );
				err = kFormatErr;
				goto exit;
		}
	}
	
	// Success!
	
	if( outObj )
	{
		*( (CFTypeRef *) outObj ) = topObject;
		topObject = NULL;
	}
	err = kNoErr;
	
exit:
	if( key ) CFRelease( key );
	
	// We only need to release a non-contained object (i.e. a top-level object). If a parent was passed in, any object(s)
	// created by this routine would either be released above or attached to the parent so they don't need to be released.
	
	if( !inParent && topObject ) CFRelease( topObject );
	__CFPropertyListStackFree( parentStack );
	return( err );
}

//===========================================================================================================================
//	__CFPropertyListAssociateObject
//===========================================================================================================================

static OSStatus
	__CFPropertyListAssociateObject( 
		CFMutableArrayRef 		inArray, 
		CFMutableDictionaryRef 	inDict, 
		CFStringRef * 			ioKey, 
		CFTypeRef 				inObj, 
		CFTypeRef *				ioTopObject )
{
	OSStatus		err;
	
	check( ( !inArray && !inDict ) || ( !inArray != !inDict ) );
	check( ioKey );
	check( ( *ioKey || !inDict ) && ( !( *ioKey ) || inDict ) );
	check( inObj );
	check( ioTopObject );
	check( inArray || inDict || ( ioTopObject && !( *ioTopObject ) ) );
	
	if( inArray )
	{
		CFArrayAppendValue( inArray, inObj );
	}
	else if( inDict )
	{
		require_action( *ioKey, exit, err = kMalformedErr );
		
		CFDictionarySetValue( inDict, *ioKey, inObj );
		CFRelease( *ioKey );
		*ioKey = NULL;
	}
	else
	{
		// No parent so retain to keep the object around.
		
		CFRetain( inObj );
	}
	
	// Only set the top object if it has not been set yet.
	
	if( !( *ioTopObject ) ) *ioTopObject = inObj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__CFPropertyListStackPush
//===========================================================================================================================

static OSStatus
	__CFPropertyListStackPush( 
		CFPropertyListStack **	ioStack, 
		CFMutableArrayRef 		inArray, 
		CFMutableDictionaryRef 	inDict )
{
	OSStatus					err;
	CFPropertyListStack *		node;
	
	check( ioStack );
	
	node = (CFPropertyListStack *) calloc( 1U, sizeof( *node ) );
	require_action( node, exit, err = kNoMemoryErr );
	
	node->next 		= *ioStack;
	node->array		= inArray;
	node->dict		= inDict;
	*ioStack		= node;
	err 			= kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__CFPropertyListStackPop
//===========================================================================================================================

static OSStatus
	__CFPropertyListStackPop( 
		CFPropertyListStack **		ioStack, 
		CFMutableArrayRef *			outArray, 
		CFMutableDictionaryRef *	outDict )
{
	OSStatus					err;
	CFPropertyListStack *		node;
	
	check( ioStack );
	check( outArray );
	check( outDict );
	
	node = *ioStack;
	require_action( node, exit, err = kMalformedErr );
	
	*ioStack 	= node->next;
	*outArray 	= node->array;
	*outDict 	= node->dict;
	free( node );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__CFPropertyListStackFree
//===========================================================================================================================

static void	__CFPropertyListStackFree( CFPropertyListStack *inStack )
{
	while( inStack )
	{
		CFPropertyListStack *		nextNode;
		
		nextNode = inStack->next;
		free( inStack );
		inStack = nextNode;
	}
}

#if 0
#pragma mark -
#pragma mark == Formatted Extracting and Validation ==
#endif

//===========================================================================================================================
//	CFPropertyListExtractFormatted
//===========================================================================================================================

OSStatus	CFPropertyListExtractFormatted( CFPropertyListRef inObj, void *outResult, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFPropertyListExtractFormattedVAList( inObj, outResult, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFPropertyListExtractFormattedVAList
//
//	Spec	Purpose									Parameters
//	-------	---------------------------------------	----------
//	<key>	Lookup inline key in dictionary			none
//	%kC		Lookup FourCharCode key in dictionary	uint32_t code (e.g. 'APPL'/0x4150504C).
//	%ki		Lookup string-based integer key			uint64_t integerKey
//	%ks		Lookup c-string key in dictionary		const char *key
//	%kt		Lookup text key in dictionary			const char *key, int keySize
//	%kO		Lookup object key in dictionary			CFStringRef key
//	[n]		Lookup inline index n in array			none
//	[*]		Lookup variable index n in array		int index
//
//	Conversion Types	Purpose								Parameter				Comment
//	-------------------	-----------------------------------	-----------------------	-------
//	bool				CFBoolean to Boolean				Boolean *outBool
//	code				CFString to OSType					uint32_t *outCode
//	data*				CFData to const void *				void **outPtr			Last va_arg is size_t expected size.
//	err					CFNumber or CFString to OSStatus	OSStatus *outErr
//	int					CFNumber or CFString to int			int *outValue
//	int8				CFNumber or CFString to int8		int8_t *outValue
//	int16				CFNumber or CFString to int16		int16_t *outValue
//	int64				CFNumber or CFString to int64		int64_t *outValue
//	int*				CFNumber or CFString to integer		void *outValue			Last va_arg is size_t sizeof( integer ).
//	ipv4				CFString to IPv4 address			uint32_t *outIPv4		Returned in host byte order.
//	mac					CFData or CFString MAC address		uint8_t outMAC[ 6 ]		outMAC must be at least 6 bytes.
//	macStr				CFData or CFString MAC address		char outMACStr[ 18 ]	outMACStr must be at least 18 bytes.
//	obj					Retained CF object					CFTypeRef *outObj		Caller must CFRelease on success.
//	utf8				CFString to malloc'd UTF-8			char **outString		Caller must free on success.
//	*utf8				CFString to fixed-size UTF-8		char *inBuffer			Last va_arg is size_t inBufferSize.
//	vers				CFString to 32-bit NumVersion		uint32_t *outVersion
//	svers				CFString to 32-bit Source Version	uint32_t *outVersion
//	uuid				CFData or CFString to 16-bytes		uint8_t outUUID[ 16 ]	Returns a big endian UUID.
//	<n>					CFData to n-bytes of data			uint8_t outData[ n ]	outData must be at least n bytes.
//===========================================================================================================================

OSStatus	CFPropertyListExtractFormattedVAList( CFPropertyListRef inObj, void *outResult, const char *inFormat, va_list inArgs )
{
	OSStatus					err;
	CFPropertyListRef			obj;
	const unsigned char *		p;
	const unsigned char *		q;
	CFStringRef					key;
	CFIndex						i;
	CFTypeRef					value;
	const char *				s;
	int							n;
	uint32_t					u32;
	Boolean						good;
	uint8_t						buf[ 8 ];
	const char *				type;
	CFRange						range;
	CFIndex						size;
	CFStringRef					str;
	int64_t						s64;
	uint64_t					u64;
	char						tempStr[ 64 ];
	size_t						len;
	
	key		= NULL;
	i		= -1;
	value	= NULL;
	obj		= inObj;
	type	= NULL;
	
	while( *inFormat != '\0' )
	{
		// Parse a segment ending in a null or '.' (end of segment) or ':' start of type.
		
		p = (const unsigned char *) inFormat;
		while( ( *inFormat != '\0' ) && ( *inFormat != '.' ) && ( *inFormat != ':' ) ) ++inFormat;
		q = (const unsigned char *) inFormat;
		while( ( *inFormat != '\0' ) && ( *inFormat != '.' ) ) ++inFormat;
		
		// Note: The following code does not explicitly check for "p < q" at each step because the format string is 
		// required to be null terminated so based on that and the above parsing/checks, we know the segment ends with 
		// either a null (last segment in the format), '.' (end of segment), or ':' (start of type conversion).
		// The code is careful to check each character to avoid accessing beyond the null, '.', or ':' sentinel.
		
		if( p[ 0 ] == '%' )			// %k specifiers
		{
			if( p[ 1 ] == 'k' )
			{
				switch( p[ 2 ] )
				{
					case 's':		// %ks: Key c-string
						
						s = va_arg( inArgs, const char * );
						require_action( s, exit, err = kParamErr );
						
						key = CFStringCreateWithCString( kCFAllocatorDefault, s, kCFStringEncodingUTF8 );
						require_action( key, exit, err = kNoMemoryErr );
						break;
					
					case 't':		// %kt: Key text
						
						s = va_arg( inArgs, const char * );
						require_action( s, exit, err = kParamErr );
						n = va_arg( inArgs, int );
						
						key = CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8 *) s, n, kCFStringEncodingUTF8, false );
						require_action( key, exit, err = kNoMemoryErr );
						break;
					
					case 'C':		// %kC: Key FourCharCode
					case 'c':		// %kc: Key FourCharCode - DEPRECATED
						
						u32 = va_arg( inArgs, uint32_t );
						buf[ 0 ] = (uint8_t)( ( u32 >> 24 ) & 0xFF );
						buf[ 1 ] = (uint8_t)( ( u32 >> 16 ) & 0xFF );
						buf[ 2 ] = (uint8_t)( ( u32 >>  8 ) & 0xFF );
						buf[ 3 ] = (uint8_t)(   u32         & 0xFF );
						
						key = CFStringCreateWithBytes( kCFAllocatorDefault, buf, 4, kCFStringEncodingMacRoman, false );
						require_action( key, exit, err = kNoMemoryErr );
						break;
					
					case 'O':		// %kO: Key object
					case 'o':		// %ko: Key object - DEPRECATED
						
						key = va_arg( inArgs, CFStringRef );
						require_action( key, exit, err = kParamErr );
						
						CFRetain( key );
						break;
					
					case 'i':		// %ki: Key string-based integer
						
						u64 = va_arg( inArgs, uint64_t );
						SNPrintF( tempStr, sizeof( tempStr ), "%llu", u64 );
						
						key = CFStringCreateWithCString( NULL, tempStr, kCFStringEncodingUTF8 );
						require_action( key, exit, err = kNoMemoryErr );
						break;

					default:
						dlog( kLogLevelError, "%s: unknown %%k specifier: %.*s\n", __ROUTINE__, (int)( q - p ), p );
						err = kFormatErr;
						goto exit;
				}
				p += 3;
			}
			else
			{
				dlog( kLogLevelError, "%s: unknown specifier: %.*s\n", __ROUTINE__, (int)( q - p ), p );
				err = kFormatErr;
				goto exit;
			}
		}
		else if( p[ 0 ] == '[' )	// []: Array specifiers
		{
			if( ( p[ 1 ] == '*' ) && ( p[ 2 ] == ']' ) )	// [*]: Variable array index
			{
				i = va_arg( inArgs, int );
				require_action( i >= 0, exit, err = kRangeErr );
				p += 3;
			}
			else if( isdigit( p[ 1 ] ) )					// [n]: Inline array index
			{
				++p;
				i = 0;
				while( isdigit( *p ) ) i = ( i * 10 ) + ( *p++ - '0' );
				require_action( i >= 0, exit, err = kRangeErr );
				require_action( *p == ']', exit, err = kFormatErr );
				++p;
			}
			else
			{
				dlog( kLogLevelError, "%s: unknown [] specifier: %.*s\n", __ROUTINE__, (int)( q - p ), p );
				err = kFormatErr;
				goto exit;
			}
		}
		else if( p < q )			// Inline keys
		{
			key = CFStringCreateWithBytes( kCFAllocatorDefault, p, (CFIndex)( q - p ), kCFStringEncodingUTF8, false );
			require_action( key, exit, err = kNoMemoryErr );
			p = q;
		}
		
		// Lookup the value with the dictionary key or array index.
		
		if( key )
		{
			check( i == -1 );
			require_action_quiet( CFGetTypeID( obj ) == CFDictionaryGetTypeID(), exit, err = kTypeErr );

			value = (CFTypeRef) CFDictionaryGetValue( (CFDictionaryRef) obj, key );
			CFRelease( key );
			key = NULL;
			require_action_quiet( value, exit, err = kNotFoundErr );
		}
		else if( i >= 0 )
		{
			require_action_quiet( CFGetTypeID( obj ) == CFArrayGetTypeID(), exit, err = kTypeErr );
			require_action_quiet( i < CFArrayGetCount( (CFArrayRef) obj ), exit, err = kRangeErr );
			
			value = CFArrayGetValueAtIndex( (CFArrayRef) obj, i );
			i = -1;
		}
		else
		{
			value = obj;
		}
		
		// Parse the conversion type (if any). Conversions are only allowed in the last segment.
		
		if( *p == ':' )
		{
			type = (const char *) ++p;
			while( ( *p != '\0' ) && ( *p != '.' ) ) ++p;
			require_action( *p == '\0', exit, err = kFormatErr );
			break;
		}
		
		// Move to the next level (if any).
		
		obj = value;
		if( *inFormat != '\0' ) ++inFormat;
	}
	require_action_quiet( value, exit, err = kNotFoundErr );
	
	// Perform value conversion (if any). Note: types must be at the end (checked above) so strcmp is safe here.
	
	if( type )
	{
		if( strcmp( type, "err" ) == 0 )		// err: CFNumber or CFString to OSStatus
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			if( outResult ) *( (OSStatus *) outResult ) = (OSStatus) s64;
		}
		else if( strcmp( type, "int" ) == 0 )	// int: CFNumber or CFString to int
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			check( ( s64 >= INT_MIN ) && ( s64 <= UINT_MAX ) );
			if( outResult ) *( (int *) outResult ) = (int) s64;
		}
		else if( strcmp( type, "int8" ) == 0 )	// int8: CFNumber or CFString to int8_t
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			check( ( s64 >= SCHAR_MIN ) && ( s64 <= UCHAR_MAX ) );
			if( outResult ) *( (uint8_t *) outResult ) = (uint8_t) s64;
		}
		else if( strcmp( type, "int16" ) == 0 )	// int16: CFNumber or CFString to int16_t
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			check( ( s64 >= SHRT_MIN ) && ( s64 <= USHRT_MAX ) );
			if( outResult ) *( (int16_t *) outResult ) = (int16_t) s64;
		}
		else if( strcmp( type, "int64" ) == 0 )	// int64: CFNumber or CFString to int64_t
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			if( outResult ) *( (int64_t *) outResult ) = s64;
		}
		else if( strcmp( type, "int*" ) == 0 )	// int*: CFNumber or CFString to variable-size int
		{
			s64 = CFGetInt64( value, &err );
			require_noerr( err, exit );
			
			len = va_arg( inArgs, size_t );
			if(      len == 1 ) { check( ( s64 >=  INT8_MIN ) && ( s64 <=  UINT8_MAX ) ); *(  (int8_t *) outResult ) =  (int8_t) s64; }
			else if( len == 2 ) { check( ( s64 >= INT16_MIN ) && ( s64 <= UINT16_MAX ) ); *( (int16_t *) outResult ) = (int16_t) s64; }
			else if( len == 4 ) { check( ( s64 >= INT32_MIN ) && ( s64 <= UINT32_MAX ) ); *( (int32_t *) outResult ) = (int32_t) s64; }
			else if( len == 8 )															  *( (int64_t *) outResult ) = s64;
			else { dlogassert( "bad integer size %zu", len ); err = kSizeErr; goto exit; }
		}
		else if( strcmp( type, "utf8" ) == 0 )	// utf8: CFString to malloc'd UTF-8
		{
			uint8_t *		utf8;
			
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			
			range = CFRangeMake( 0, CFStringGetLength( (CFStringRef) value ) );
			size = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
			utf8 = (uint8_t *) malloc( (size_t)( size + 1 ) );
			require_action( utf8, exit, err = kNoMemoryErr );
			
			range.location = CFStringGetBytes( (CFStringRef) value, range, kCFStringEncodingUTF8, 0, false, utf8, size, &size );
			if( range.location == range.length )
			{
				utf8[ size ] = '\0';
				
				if( outResult ) *( (uint8_t **) outResult ) = utf8;
				else			free( utf8 );
			}
			else
			{
				free( utf8 );
				dlog( kLogLevelError, "%s: cannot convert to UTF-8: %@\n", __ROUTINE__, value );
				err = kUnexpectedErr;
				goto exit;
			}
		}
		else if( strcmp( type, "*utf8" ) == 0 )	// *utf8: CFString to fixed-sized UTF-8.
		{
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			
			size = (CFIndex) va_arg( inArgs, size_t );
			good = CFStringGetCString( (CFStringRef) value, (char *) outResult, size, kCFStringEncodingUTF8 );
			require_action( good, exit, err = kSizeErr );
		}
		else if( strcmp( type, "obj" ) == 0 )	// obj: Retained CF object
		{
			if( outResult )
			{
				CFRetain( value );
				*( (CFTypeRef *) outResult ) = value;
			}
		}
		else if( strcmp( type, "bool" ) == 0 )	// bool: CFBoolean to Boolean
		{
			require_action_quiet( CFGetTypeID( value ) == CFBooleanGetTypeID(), exit, err = kTypeErr );
			
			if( outResult ) *( (Boolean *) outResult ) = CFBooleanGetValue( (CFBooleanRef) value );
		}
		else if( strcmp( type, "mac" ) == 0 )	// mac: CFData/CFString to 6-byte MAC address array.
		{
			if( CFGetTypeID( value ) == CFDataGetTypeID() )
			{
				require_action( CFDataGetLength( (CFDataRef) value ) == 6, exit, err = kSizeErr );
				if( outResult ) memcpy( outResult, CFDataGetBytePtr( (CFDataRef) value ), 6 );
			}
			else if( CFGetTypeID( value ) == CFStringGetTypeID() )
			{
				good = CFStringGetCString( (CFStringRef) value, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingUTF8 );
				require_action( good, exit, err = kSizeErr );
				
				err = TextToMACAddress( tempStr, kSizeCString, outResult );
				require_noerr( err, exit );
			}
			else
			{
				err = kTypeErr;
				goto exit;
			}
		}
		else if( strcmp( type, "macStr" ) == 0 )	// macStr: CFData/CFString to 18-byte MAC address C string.
		{
			if( CFGetTypeID( value ) == CFDataGetTypeID() )
			{
				require_action( CFDataGetLength( (CFDataRef) value ) == 6, exit, err = kSizeErr );
				MACAddressToCString( CFDataGetBytePtr( (CFDataRef) value ), (char *) outResult );
			}
			else if( CFGetTypeID( value ) == CFStringGetTypeID() )
			{
				good = CFStringGetCString( (CFStringRef) value, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingUTF8 );
				require_action( good, exit, err = kSizeErr );
				
				err = TextToMACAddress( tempStr, kSizeCString, buf );
				require_noerr( err, exit );
				
				MACAddressToCString( buf, (char *) outResult );
			}
			else
			{
				err = kTypeErr;
				goto exit;
			}
		}
		else if( strcmp( type, "code" ) == 0 )	// code: CFString to OSType
		{
			uint32_t		code;
			
			if( CFGetTypeID( value ) == CFStringGetTypeID() )
			{
				str = (CFStringRef) value;
				range = CFRangeMake( 0, CFStringGetLength( str ) );
				require_action( range.length == 4, exit, err = kSizeErr );
			
				size = 0;
				CFStringGetBytes( str, range, kCFStringEncodingUTF8, 0, false, buf, 4, &size );
				require_action( size == 4, exit, err = kFormatErr );
			
				code = TextToFourCharCode( buf, 4 );
			}
			else if( CFGetTypeID( value ) == CFNumberGetTypeID() )
			{
				CFNumberGetValue( (CFNumberRef) value, kCFNumberSInt32Type, &code );
			}
			else
			{
				err = kTypeErr;
				goto exit;
			}
			if( outResult ) *( (uint32_t *) outResult ) = code;
		}
		else if( strcmp( type, "ipv4" ) == 0 )	// ipv4: CFString to IPv4 address
		{
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			str = (CFStringRef) value;
			
			good = CFStringGetCString( str, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingUTF8 );
			require_action( good, exit, err = kOverrunErr );
			
			err = StringToIPv4Address( tempStr, kStringToIPAddressFlagsNone, (uint32_t *) outResult, NULL, NULL, NULL, NULL );
			require_noerr( err, exit );
		}
		else if( strcmp( type, "vers" ) == 0 )	// vers: CFString to 32-bit NumVersion
		{
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			str = (CFStringRef) value;
			
			range = CFRangeMake( 0, CFStringGetLength( str ) );
			size = 0;
			CFStringGetBytes( str, range, kCFStringEncodingUTF8, 0, false, (uint8_t *) tempStr, 
				(CFIndex)( sizeof( tempStr ) - 1 ), &size );
			
			err = TextToNumVersion( tempStr, (size_t) size, (uint32_t *) outResult );
			require_noerr( err, exit );
		}
		else if( strcmp( type, "svers" ) == 0 )	// vers: CFString to 32-bit Source Version
		{
			uint32_t	sourceVersion;
			
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			str = (CFStringRef) value;
			
			range = CFRangeMake( 0, CFStringGetLength( str ) );
			size = 0;
			CFStringGetBytes( str, range, kCFStringEncodingUTF8, 0, false, (uint8_t *) tempStr, 
				(CFIndex)( sizeof( tempStr ) - 1 ), &size );
			
			sourceVersion = TextToSourceVersion( tempStr, (size_t) size );
			require_action( sourceVersion != 0, exit, err = kMalformedErr );
			
			*( (uint32_t *) outResult ) = sourceVersion;
		}
		else if( strcmp( type, "uuid" ) == 0 )	// uuid: CFData/CFString to 16-byte, big endian UUID
		{
			if( CFGetTypeID( value ) == CFDataGetTypeID() )
			{
				require_action( CFDataGetLength( (CFDataRef) value ) == 16, exit, err = kSizeErr );
				if( outResult ) memcpy( outResult, CFDataGetBytePtr( (CFDataRef) value ), 16 );
			}
			else if( CFGetTypeID( value ) == CFStringGetTypeID() )
			{
				char		uuidStr[ 64 ];
				
				good = CFStringGetCString( (CFStringRef) value, uuidStr, (CFIndex) sizeof( uuidStr ), kCFStringEncodingUTF8 );
				require_action( good, exit, err = kSizeErr );
				
				err = StringToUUID( uuidStr, kSizeCString, 0, outResult );
				require_noerr( err, exit );
			}
			else
			{
				err = kTypeErr;
				goto exit;
			}
		}
		else if( strcmp( type, "CFStringUUID" ) == 0 )	// CFStringUUID: CFString UUID validity checking.
		{
			require_action_quiet( CFGetTypeID( value ) == CFStringGetTypeID(), exit, err = kTypeErr );
			
			good = CFStringGetCString( (CFStringRef) value, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingUTF8 );
			require_action_quiet( good, exit, err = kSizeErr );
			
			err = StringToUUID( tempStr, kSizeCString, false, NULL );
			require_noerr_quiet( err, exit );
			
			if( outResult ) *( (CFTypeRef *) outResult ) = value;
		}
		else if( strncmp( type, "CF", 2 ) == 0 ) // CF*: CF object of a specific type.
		{
			CFTypeID		requiredTypeID;
			Boolean			assertOnBadType;
			
			s = strchr( type, '!' );
			if( s ) { len = (size_t)( s - type );	assertOnBadType = true; }
			else	{ len = strlen( type );			assertOnBadType = false; }
			
			if(      strncmpx( type, len, "CFArray" )		== 0 ) requiredTypeID = CFArrayGetTypeID();
			else if( strncmpx( type, len, "CFBoolean" )		== 0 ) requiredTypeID = CFBooleanGetTypeID();
			else if( strncmpx( type, len, "CFData" )		== 0 ) requiredTypeID = CFDataGetTypeID();
			else if( strncmpx( type, len, "CFDate" )		== 0 ) requiredTypeID = CFDateGetTypeID();
			else if( strncmpx( type, len, "CFDictionary" )	== 0 ) requiredTypeID = CFDictionaryGetTypeID();
			else if( strncmpx( type, len, "CFNumber" )		== 0 ) requiredTypeID = CFNumberGetTypeID();
			else if( strncmpx( type, len, "CFString" )		== 0 ) requiredTypeID = CFStringGetTypeID();
			else { dlogassert( "unknown CF type: '%s'", type ); err = kUnsupportedErr; goto exit; }
			
			if( CFGetTypeID( value ) != requiredTypeID )
			{
				if( assertOnBadType ) dlogassert( "not type %.*s:\n%1@", (int) len, type, value );
				err = kTypeErr;
				goto exit;
			}
			if( outResult )
			{
				*( (CFTypeRef *) outResult ) = value;
			}
		}
		else if( strcmp( type, "data*" ) == 0 )	// data*: CFData to const void *
		{
			require_action( CFGetTypeID( value ) == CFDataGetTypeID(), exit, err = kSizeErr );
			
			len = va_arg( inArgs, size_t );
			require_action( CFDataGetLength( (CFDataRef) value ) == ( (CFIndex) len ), exit, err = kSizeErr );
			
			if( outResult ) *( (const uint8_t **) outResult ) = CFDataGetBytePtr( (CFDataRef) value );
		}
		else
		{
			i = 0;
			for( p = (unsigned char *) type; isdigit( *p ); ++p ) i = ( i * 10 ) + ( *p - '0' );
			if( *p != '\0' )
			{
				dlog( kLogLevelError, "%s: unknown conversion type: %s\n", __ROUTINE__, type );
				err = kUnexpectedErr;
				goto exit;
			}
			require_action( i >= 0, exit, err = kRangeErr );
			require_action( CFGetTypeID( value ) == CFDataGetTypeID(), exit, err = kTypeErr );
			require_action( CFDataGetLength( (CFDataRef) value ) == i, exit, err = kSizeErr );
			
			if( outResult )
			{
				memcpy( outResult, CFDataGetBytePtr( (CFDataRef) value ), (size_t) i );
			}
		}
	}
	else
	{
		if( outResult ) *( (void **) outResult ) = (void *) value;
	}
	err = kNoErr;
	
exit:
	if( key ) CFRelease( key );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Object Accessors ==
#endif

//===========================================================================================================================
//	CFObjectControlAsync
//===========================================================================================================================

typedef struct
{
	CFTypeRef						object;
	CFObjectControlFunc				func;
	CFObjectFlags					flags;
	CFStringRef						command;
	CFTypeRef						qualifier;
	CFDictionaryRef					params;
	OSStatus						error;
	CFDictionaryRef					response;
	dispatch_queue_t				responseQueue;
	CFObjectControlResponseFunc		responseFunc;
	void *							responseContext;
	
}	CFObjectControlAsyncParams;

static void _CFObjectControlAsync( void *inArg );
static void _CFObjectControlResponse( void *inArg );

OSStatus
	CFObjectControlAsync( 
		CFTypeRef					inObject,
		dispatch_queue_t			inQueue, 
		CFObjectControlFunc			inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		CFDictionaryRef				inParams, 
		dispatch_queue_t			inResponseQueue, 
		CFObjectControlResponseFunc	inResponseFunc, 
		void *						inResponseContext )
{
	OSStatus							err;
	CFObjectControlAsyncParams *		params;
	
	params = (CFObjectControlAsyncParams *) malloc( sizeof( *params ) );
	require_action( params, exit, err = kNoMemoryErr );
	
	CFRetain( inObject );
	params->object	= inObject;
	params->func	= inFunc;
	params->flags	= inFlags;
	
	CFRetain( inCommand );
	params->command = inCommand;
	
	CFRetainNullSafe( inQualifier );
	params->qualifier = inQualifier;
	
	CFRetainNullSafe( inParams );
	params->params   = inParams;
	
	params->response = NULL;
	if( inResponseQueue ) dispatch_retain( inResponseQueue );
	params->responseQueue	= inResponseQueue;
	params->responseFunc    = inResponseFunc;
	params->responseContext = inResponseContext;
	
	dispatch_async_f( inQueue, params, _CFObjectControlAsync );
	err = kNoErr;
	
exit:
	return( err );
}

static void _CFObjectControlAsync( void *inArg )
{
	CFObjectControlAsyncParams *		params = (CFObjectControlAsyncParams *) inArg;
	
	params->error = params->func( params->object, params->flags, params->command, params->qualifier, params->params, 
		&params->response );
	CFRelease( params->object );
	CFRelease( params->command );
	CFReleaseNullSafe( params->qualifier );
	CFReleaseNullSafe( params->params );
	
	if( params->responseFunc )
	{
		if( params->responseQueue )
		{
			dispatch_async_f( params->responseQueue, params, _CFObjectControlResponse );
			params = NULL;
		}
		else
		{
			params->responseFunc( params->error, params->response, params->responseContext );
		}
	}
	else if( params->response )
	{
		dlog( kLogLevelNotice, "### Async control with no completion ignored response %@\n", params->response );
	}
	if( params )
	{
		CFReleaseNullSafe( params->response );
		if( params->responseQueue ) dispatch_release( params->responseQueue );
		free( params );
	}
}

static void _CFObjectControlResponse( void *inArg )
{
	CFObjectControlAsyncParams * const		params = (CFObjectControlAsyncParams *) inArg;
	
	params->responseFunc( params->error, params->response, params->responseContext );
	CFReleaseNullSafe( params->response );
	dispatch_release( params->responseQueue );
	free( params );
}

//===========================================================================================================================
//	CFObjectControlAsyncF
//===========================================================================================================================

OSStatus
	CFObjectControlAsyncF( 
		CFTypeRef					inObject,
		dispatch_queue_t			inQueue, 
		CFObjectControlFunc			inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		dispatch_queue_t			inResponseQueue, 
		CFObjectControlResponseFunc	inResponseFunc, 
		void *						inResponseContext, 
		const char *				inFormat, 
		... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFObjectControlAsyncV( inObject, inQueue, inFunc, inFlags, inCommand, inQualifier, 
		inResponseQueue, inResponseFunc, inResponseContext, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFObjectControlAsyncV
//===========================================================================================================================

OSStatus
	CFObjectControlAsyncV( 
		CFTypeRef					inObject,
		dispatch_queue_t			inQueue, 
		CFObjectControlFunc			inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inCommand, 
		CFTypeRef					inQualifier, 
		dispatch_queue_t			inResponseQueue, 
		CFObjectControlResponseFunc	inResponseFunc, 
		void *						inResponseContext, 
		const char *				inFormat, 
		va_list						inArgs )
{
	OSStatus					err;
	CFMutableDictionaryRef		params;
	
	params = NULL;
	err = CFPropertyListCreateFormattedVAList( NULL, &params, inFormat, inArgs );
	require_noerr( err, exit );
	
	err = CFObjectControlAsync( inObject, inQueue, inFunc, inFlags, inCommand, inQualifier, params, 
		inResponseQueue, inResponseFunc, inResponseContext );
	CFReleaseNullSafe( params );
	
exit:
	return( err );
}
#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFObjectControlSync
//===========================================================================================================================

typedef struct
{
	CFTypeRef				object;
	CFObjectControlFunc		func;
	CFObjectFlags			flags;
	CFStringRef				command;
	CFTypeRef				qualifier;
	CFDictionaryRef			params;
	CFDictionaryRef *		responsePtr;
	OSStatus				error;
	
}	CFObjectControlSyncParams;

static void	_CFObjectControlSync( void *inContext );

OSStatus
	CFObjectControlSync( 
		CFTypeRef			inObject,
		dispatch_queue_t	inQueue, 
		CFObjectControlFunc	inFunc, 
		CFObjectFlags		inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef		inParams, 
		CFDictionaryRef *	outResponse )
{
	if( !( inFlags & kCFObjectFlagDirect ) )
	{
		CFObjectControlSyncParams		params = { inObject, inFunc, inFlags, inCommand, inQualifier, inParams, outResponse, kUnknownErr };
		
		dispatch_sync_f( inQueue, &params, _CFObjectControlSync );
		return( params.error );
	}
	return( inFunc( inObject, inFlags, inCommand, inQualifier, inParams, outResponse ) );
}

static void	_CFObjectControlSync( void *inContext )
{
	CFObjectControlSyncParams * const		params = (CFObjectControlSyncParams *) inContext;
	
	params->error = params->func( params->object, params->flags, params->command, params->qualifier, params->params, 
		params->responsePtr );
}

//===========================================================================================================================
//	CFObjectControlSyncF
//===========================================================================================================================

OSStatus
	CFObjectControlSyncF( 
		CFTypeRef			inObject, 
		dispatch_queue_t	inQueue, 
		CFObjectControlFunc	inFunc, 
		CFObjectFlags		inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef *	outResponse, 
		const char *		inFormat, 
		... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFObjectControlSyncV( inObject, inQueue, inFunc, inFlags, inCommand, inQualifier, outResponse, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFObjectControlSyncV
//===========================================================================================================================

OSStatus
	CFObjectControlSyncV( 
		CFTypeRef			inObject, 
		dispatch_queue_t	inQueue, 
		CFObjectControlFunc	inFunc, 
		CFObjectFlags		inFlags, 
		CFStringRef			inCommand, 
		CFTypeRef			inQualifier, 
		CFDictionaryRef *	outResponse, 
		const char *		inFormat, 
		va_list				inArgs )
{
	OSStatus					err;
	CFMutableDictionaryRef		params;
	
	params = NULL;
	err = CFPropertyListCreateFormattedVAList( NULL, &params, inFormat, inArgs );
	require_noerr( err, exit );
	
	err = CFObjectControlSync( inObject, inQueue, inFunc, inFlags, inCommand, inQualifier, params, outResponse );
	CFRelease( params );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFObjectCopyProperty
//===========================================================================================================================

typedef struct
{
	CFTypeRef					object;
	CFObjectCopyPropertyFunc	func;
	CFObjectFlags				flags;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFTypeRef					value;
	OSStatus *					errorPtr;
	
}	CFObjectCopyPropertyParams;

static void	_CFObjectCopyProperty( void *inContext );

CFTypeRef
	CFObjectCopyProperty( 
		CFTypeRef					inObject,
		dispatch_queue_t			inQueue, 
		CFObjectCopyPropertyFunc	inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr )
{
	if( !( inFlags & kCFObjectFlagDirect ) )
	{
		CFObjectCopyPropertyParams		params = { inObject, inFunc, inFlags, inProperty, inQualifier, NULL, outErr };
		
		dispatch_sync_f( inQueue, &params, _CFObjectCopyProperty );
		return( params.value );
	}
	return( inFunc( inObject, inFlags, inProperty, inQualifier, outErr ) );
}

static void	_CFObjectCopyProperty( void *inContext )
{
	CFObjectCopyPropertyParams * const		params = (CFObjectCopyPropertyParams *) inContext;
	
	params->value = params->func( params->object, params->flags, params->property, params->qualifier, params->errorPtr );
}

//===========================================================================================================================
//	CFObjectGetPropertyCStringSync
//===========================================================================================================================

char *
	CFObjectGetPropertyCStringSync( 
		CFTypeRef					inObject, 
		dispatch_queue_t			inQueue, 
		CFObjectCopyPropertyFunc	inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		char *						inBuf, 
		size_t						inMaxLen, 
		OSStatus *					outErr )
{
	char *			value;
	CFTypeRef		cfValue;
	
	cfValue = CFObjectCopyProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, outErr );
	if( cfValue )
	{	
		value = CFGetCString( cfValue, inBuf, inMaxLen );
		CFRelease( cfValue );
		return( value );
	}
	return( NULL );
}

//===========================================================================================================================
//	CFObjectGetPropertyDoubleSync
//===========================================================================================================================

double
	CFObjectGetPropertyDoubleSync( 
		CFTypeRef					inObject, 
		dispatch_queue_t			inQueue, 
		CFObjectCopyPropertyFunc	inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr )
{
	double			value;
	CFTypeRef		cfValue;
	
	cfValue = CFObjectCopyProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, outErr );
	if( cfValue )
	{
		value = CFGetDouble( cfValue, outErr );
		CFRelease( cfValue );
		return( value );
	}
	return( 0 );
}

//===========================================================================================================================
//	CFObjectGetPropertyInt64Sync
//===========================================================================================================================

int64_t
	CFObjectGetPropertyInt64Sync( 
		CFTypeRef					inObject, 
		dispatch_queue_t			inQueue, 
		CFObjectCopyPropertyFunc	inFunc, 
		CFObjectFlags				inFlags, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr )
{
	int64_t			value;
	CFTypeRef		cfValue;
	
	cfValue = CFObjectCopyProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, outErr );
	if( cfValue )
	{
		value = CFGetInt64( cfValue, outErr );
		CFRelease( cfValue );
		return( value );
	}
	return( 0 );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFObjectSetProperty
//===========================================================================================================================

typedef struct
{
	CFTypeRef					object;
	CFObjectSetPropertyFunc		func;
	CFObjectFlags				flags;
	CFStringRef					property;
	CFTypeRef					qualifier;
	CFTypeRef					value;
	OSStatus					error;
	
}	CFObjectSetPropertyParams;

static void	_CFObjectSetProperty( void *inContext );

OSStatus
	CFObjectSetProperty( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		CFTypeRef				inValue )
{
	OSStatus		err;
	
	if( inFlags & kCFObjectFlagDirect )
	{
		err = inFunc( inObject, inFlags, inProperty, inQualifier, inValue );
	}
	else if( inFlags & kCFObjectFlagAsync )
	{
		CFObjectSetPropertyParams *		params;
		
		params = (CFObjectSetPropertyParams *) malloc( sizeof( *params ) );
		require_action( params, exit, err = kNoMemoryErr );
		
		CFRetain( inObject );
		params->object = inObject;
		
		params->func  = inFunc;
		params->flags = inFlags;
		
		CFRetain( inProperty );
		params->property = inProperty;
		
		CFRetainNullSafe( inQualifier );
		params->qualifier = inQualifier;
		
		CFRetainNullSafe( inValue );
		params->value = inValue;
		
		dispatch_async_f( inQueue, params, _CFObjectSetProperty );
		static_analyzer_malloc_freed( params );
		err = kNoErr;
	}
	else
	{
		CFObjectSetPropertyParams		localParams = { inObject, inFunc, inFlags, inProperty, inQualifier, inValue, kUnknownErr };
		
		dispatch_sync_f( inQueue, &localParams, _CFObjectSetProperty );
		err = localParams.error;
	}
	
exit:
	return( err );
}

static void	_CFObjectSetProperty( void *inContext )
{
	CFObjectSetPropertyParams * const		params = (CFObjectSetPropertyParams *) inContext;
	
	params->error = params->func( params->object, params->flags, params->property, params->qualifier, params->value );
	if( params->flags & kCFObjectFlagAsync )
	{
		CFRelease( params->object );
		CFRelease( params->property );
		CFReleaseNullSafe( params->qualifier );
		CFReleaseNullSafe( params->value );
		free( params );
	}
}

//===========================================================================================================================
//	CFObjectSetPropertyF
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyF( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		const char *			inFormat, 
		... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = CFObjectSetPropertyV( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, inFormat, args );
	va_end( args );
	return( err );
}

//===========================================================================================================================
//	CFObjectSetPropertyV
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyV( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		const char *			inFormat, 
		va_list					inArgs )
{
	OSStatus					err;
	CFMutableDictionaryRef		value = NULL;
	
	err = CFPropertyListCreateFormattedVAList( NULL, &value, inFormat, inArgs );
	require_noerr( err, exit );
	
	err = CFObjectSetProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, value );
	CFReleaseNullSafe( value );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFObjectSetPropertyCString
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyCString( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		const void *			inStr, 
		size_t					inLen )
{
	OSStatus		err;
	CFStringRef		value;
	
	if( inLen == kSizeCString )
	{
		value = CFStringCreateWithCString( NULL, (const char *) inStr, kCFStringEncodingUTF8 );
		require_action( value, exit, err = kUnknownErr );
	}
	else
	{
		value = CFStringCreateWithBytes( NULL, (const uint8_t *) inStr, (CFIndex) inLen, kCFStringEncodingUTF8, false );
		require_action( value, exit, err = kUnknownErr );
	}
	
	err = CFObjectSetProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, value );
	CFRelease( value );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFObjectSetPropertyData
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyData( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		const void *			inData, 
		size_t					inLen )
{
	OSStatus		err;
	CFDataRef		value;
	
	value = CFDataCreate( NULL, (const uint8_t *) inData, (CFIndex) inLen );
	require_action( value, exit, err = kUnknownErr );
	
	err = CFObjectSetProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, value );
	CFRelease( value );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFObjectSetPropertyDouble
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyDouble( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		double					inValue )
{
	OSStatus		err;
	CFNumberRef		value;
	
	value = CFNumberCreate( NULL, kCFNumberDoubleType, &inValue );
	require_action( value, exit, err = kNoMemoryErr );
	
	err = CFObjectSetProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, value );
	CFRelease( value );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFObjectSetPropertyInt64
//===========================================================================================================================

OSStatus
	CFObjectSetPropertyInt64( 
		CFTypeRef				inObject, 
		dispatch_queue_t		inQueue, 
		CFObjectSetPropertyFunc	inFunc, 
		CFObjectFlags			inFlags, 
		CFStringRef				inProperty, 
		CFTypeRef				inQualifier, 
		int64_t					inValue )
{
	OSStatus		err;
	CFNumberRef		value;
	
	value = CFNumberCreateInt64( inValue );
	require_action( value, exit, err = kNoMemoryErr );
	
	err = CFObjectSetProperty( inObject, inQueue, inFunc, inFlags, inProperty, inQualifier, value );
	CFRelease( value );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Utils ==
#endif

//===========================================================================================================================
//	CFArrayApplyBlock
//===========================================================================================================================

//===========================================================================================================================
//	CFDictionaryApplyBlock
//===========================================================================================================================

//===========================================================================================================================
//	CFSetApplyBlock
//===========================================================================================================================

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFArrayCreatedSortedByKeyPath
//===========================================================================================================================

//===========================================================================================================================
//	CFSortCompareKeyPath
//===========================================================================================================================

CFComparisonResult	CFSortCompareKeyPath( const void *inLeft, const void *inRight, void *inContext )
{
	const char * const		keyPath = (const char *) inContext;
	CFComparisonResult		cmp;
	OSStatus				err;
	void *					temp;
	CFTypeRef				left, right;
	CFTypeID				leftTypeID, rightTypeID;
	
	cmp = -1;
	
	err = CFPropertyListExtractFormatted( inLeft, &temp, keyPath );
	require_noerr( err, exit );
	left = (CFTypeRef) temp;

	err = CFPropertyListExtractFormatted( inRight, &temp, keyPath );
	require_noerr( err, exit );
	right = (CFTypeRef) temp;

	leftTypeID  = CFGetTypeID( left );
	rightTypeID = CFGetTypeID( right );
	require( leftTypeID == rightTypeID, exit );
	
	if( leftTypeID == CFNumberGetTypeID() )
	{
		int64_t		left64, right64;
		
		CFNumberGetValue( (CFNumberRef) left,  kCFNumberSInt64Type, &left64 );
		CFNumberGetValue( (CFNumberRef) right, kCFNumberSInt64Type, &right64 );
		if(      left64 < right64 ) cmp = -1;
		else if( left64 > right64 ) cmp =  1;
		else						cmp = 0;
	}
	else if( leftTypeID == CFStringGetTypeID() )
	{
		cmp = CFStringLocalizedStandardCompare( (CFStringRef) left, (CFStringRef) right );
	}
	else if( leftTypeID == CFDateGetTypeID() )
	{
		cmp = CFDateCompare( (CFDateRef) left, (CFDateRef) right, NULL );
	}
	else
	{
		dlogassert( "unsupport CF type: %d\n", (int) leftTypeID );
		goto exit;
	}
	
exit:
	return( cmp );
}

//===========================================================================================================================
//	CFSortLocalizedStandardCompare
//===========================================================================================================================

CFComparisonResult	CFSortLocalizedStandardCompare( const void *inLeft, const void *inRight, void *inContext )
{
	(void) inContext;
	
	return( CFStringLocalizedStandardCompare( (CFStringRef) inLeft, (CFStringRef) inRight ) );
}

//===========================================================================================================================
//	CFDictionaryCreateWithCFStringArray
//===========================================================================================================================

OSStatus	CFDictionaryCreateWithCFStringArray( CFArrayRef inArray, CFMutableDictionaryRef *outDict )
{
	OSStatus					err;
	CFMutableDictionaryRef		dict;
	CFIndex						i, n;
	CFStringRef					str;
	
	dict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( dict, exit, err = kNoMemoryErr );
	
	n = CFArrayGetCount( inArray );
	for( i = 0; i < n; ++i )
	{
		str = (CFStringRef) CFArrayGetValueAtIndex( inArray, i );
		require_action( CFGetTypeID( str ) == CFStringGetTypeID(), exit, err = kTypeErr );
		
		CFDictionarySetValue( dict, str, kCFBooleanTrue );
	}
	
	*outDict = dict;
	dict = NULL;
	err = kNoErr;
	
exit:
	if( dict ) CFRelease( dict );
	return( err );
}

//===========================================================================================================================
//	CFDictionaryCreateWithFourCharCodeArray
//===========================================================================================================================

OSStatus	CFDictionaryCreateWithFourCharCodeArray( const uint32_t inArray[], size_t inCount, CFMutableDictionaryRef *outDict )
{
	OSStatus					err;
	CFMutableDictionaryRef		dict;
	size_t						i;
	
	dict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( dict, exit, err = kNoMemoryErr );
	
	for( i = 0; i < inCount; ++i )
	{
		err = CFPropertyListAppendFormatted( NULL, dict, "%kC=%O", inArray[ i ], kCFBooleanTrue );
		require_noerr( err, exit );
	}
	
	*outDict = dict;
	dict = NULL;
	err = kNoErr;
	
exit:
	if( dict ) CFRelease( dict );
	return( err );
}

//===========================================================================================================================
//	CFDataCreateWithFilePath
//===========================================================================================================================

CFDataRef	CFDataCreateWithFilePath( const char *inPath, OSStatus *outErr )
{
	CFDataRef		result = NULL;
	OSStatus		err;
	FILE *			file;
	
	file = fopen( inPath, "rb" );
	err = map_global_value_errno( file, file );
	require_noerr_quiet( err, exit );
	
	result = CFDataCreateWithANSIFile( file, &err );
	require_noerr( err, exit );
	
exit:
	if( file ) fclose( file );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFDataCreateWithANSIFile
//===========================================================================================================================

CFDataRef	CFDataCreateWithANSIFile( FILE *inFile, OSStatus *outErr )
{
	CFDataRef				result = NULL;
	CFMutableDataRef		data;
	OSStatus				err;
	uint8_t *				tempBuf = NULL;
	size_t					tempLen;
	size_t					readSize;
	
	data = CFDataCreateMutable( NULL, 0 );
	require_action( data, exit, err = kNoMemoryErr );
	
	tempLen = 128 * 1024;
	tempBuf = (uint8_t *) malloc( tempLen );
	require_action( tempBuf, exit, err = kNoMemoryErr );
	
	for( ;; )
	{
		readSize = fread( tempBuf, 1, tempLen, inFile );
		if( readSize == 0 ) break;
		
		CFDataAppendBytes( data, tempBuf, (CFIndex) readSize );
	}
	
	result = data;
	data = NULL;
	err = kNoErr;
	
exit:
	if( tempBuf ) free( tempBuf );
	CFReleaseNullSafe( data );
	if( outErr ) *outErr = err;
	return( result );
}

#if( !CFLITE_ENABLED )
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
	CFDateRef			date	 = NULL;
	CFCalendarRef		calendar = NULL;
	CFTimeZoneRef		tz;
	Boolean				good;
	CFAbsoluteTime		t;
	
	(void) inAllocator;
	
	calendar = CFCalendarCopyCurrent();
	require( calendar, exit );
	
	tz = CFTimeZoneCreateWithName( NULL, CFSTR( "GMT" ), false );
	require( tz, exit );
	CFCalendarSetTimeZone( calendar, tz );
	CFRelease( tz );
	
	good = CFCalendarComposeAbsoluteTime( calendar, &t, "yMdHms", inYear, inMonth, inDay, inHour, inMinute, inSecond );
	require( good, exit );
	
	date = CFDateCreate( NULL, t );
	require( date, exit );
	
exit:
	CFReleaseNullSafe( calendar );
	return( date );
}

//===========================================================================================================================
//	CFDateGetComponents
//===========================================================================================================================

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
	OSStatus			err;
	CFCalendarRef		calendar = NULL;
	CFTimeZoneRef		tz;
	CFAbsoluteTime		t;
	Boolean				good;
	double				d;
	
	calendar = CFCalendarCopyCurrent();
	require_action( calendar, exit, err = kUnknownErr );
	
	tz = CFTimeZoneCreateWithName( NULL, CFSTR( "GMT" ), false );
	require_action( tz, exit, err = kUnknownErr );
	CFCalendarSetTimeZone( calendar, tz );
	CFRelease( tz );
	
	t = CFDateGetAbsoluteTime( inDate );
	good = CFCalendarDecomposeAbsoluteTime( calendar, t, "yMdHms", outYear, outMonth, outDay, outHour, outMinute, outSecond );
	require_action( good, exit, err = kUnknownErr );
	
	if( outMicros ) *outMicros = (int)( modf( t, &d ) * 1000000 );
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( calendar );
	if( err )
	{
		*outYear	= 0;
		*outMonth	= 0;
		*outDay		= 0;
		*outHour	= 0;
		*outMinute	= 0;
		*outSecond	= 0;
		if( outMicros ) *outMicros	= 0;
	}
	return( err );
}
#endif

//===========================================================================================================================
//	CFPropertyListCreateBytes
//===========================================================================================================================

OSStatus	CFPropertyListCreateBytes( CFPropertyListRef inPlist, CFPropertyListFormat inFormat, uint8_t **outPtr, size_t *outLen )
{
	OSStatus		err;
	CFDataRef		data;
	size_t			len;
	uint8_t *		ptr;
	
	data = CFPropertyListCreateData( NULL, inPlist, inFormat, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	
	len = (size_t) CFDataGetLength( data );
	ptr = (uint8_t *) malloc( len );
	require_action( ptr, exit, err = kUnknownErr );
	memcpy( ptr, CFDataGetBytePtr( data ), len );
	
	*outPtr = ptr;
	*outLen = len;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( data );
	return( err );
}

//===========================================================================================================================
//	CFPropertyListCreateFromANSIFile
//===========================================================================================================================

OSStatus	CFPropertyListCreateFromANSIFile( FILE *inFile, CFOptionFlags inOptions, CFPropertyListRef *outPlist )
{
	OSStatus				err;
	CFPropertyListRef		plist;
	CFDataRef				data;
	
	data = CFDataCreateWithANSIFile( inFile, &err );
	require_noerr_quiet( err, exit );
	
	plist = CFPropertyListCreateWithData( NULL, data, inOptions, NULL, NULL );
	CFRelease( data );
	require_action_quiet( plist, exit, err = kFormatErr );
	
	*outPlist = plist;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFPropertyListCreateFromFilePath
//===========================================================================================================================

CFTypeRef	CFPropertyListCreateFromFilePath( const char *inPath, CFOptionFlags inOptions, OSStatus *outErr )
{
	CFTypeRef		plist = NULL;
	CFDataRef		data;
	OSStatus		err;
	
	data = CFDataCreateWithFilePath( inPath, &err );
	require_noerr_quiet( err, exit );
	
	plist = CFPropertyListCreateWithData( NULL, data, inOptions, NULL, NULL );
	CFRelease( data );
	require_action_quiet( plist, exit, err = kFormatErr );
	
exit:
	if( outErr ) *outErr = err;
	return( plist );
}

//===========================================================================================================================
//	CFPropertyListWriteToFilePath / CFPropertyListWriteToANSIFile
//===========================================================================================================================

static OSStatus
	CFPropertyListWriteToFilePathEx( 
		CFPropertyListRef	inPlist, 
		const char *		inFormat, 
		const char *		inPath, 
		FILE *				inFile );

OSStatus	CFPropertyListWriteToFilePath( CFPropertyListRef inPlist, const char *inFormat, const char *inPath )
{
	return( CFPropertyListWriteToFilePathEx( inPlist, inFormat, inPath, NULL ) );
}

OSStatus	CFPropertyListWriteToANSIFile( CFPropertyListRef inPlist, const char *inFormat, FILE *inFile )
{
	return( CFPropertyListWriteToFilePathEx( inPlist, inFormat, NULL, inFile ) );
}

static OSStatus
	CFPropertyListWriteToFilePathEx( 
		CFPropertyListRef	inPlist, 
		const char *		inFormat, 
		const char *		inPath, 
		FILE *				inFile )
{
	OSStatus			err;
	CFDataRef			data;
	FILE *				file;
	CFTypeID			typeID;
	const char *		utf8;
	char *				utf8Storage;
	size_t				stringLen;
	CFIndex				dataLen;
	size_t				nWrote;
	
	file		= NULL;
	data		= NULL;
	utf8Storage = NULL;
	
	// If the format is prefixed with "raw-" and it's a type that supports it then write it out raw.
	
	if( strncmp_prefix( inFormat, SIZE_MAX, "raw-" ) == 0 )
	{
		typeID = CFGetTypeID( inPlist );
		if( typeID == CFStringGetTypeID() )
		{
			err = CFStringGetOrCopyCStringUTF8( (CFStringRef) inPlist, &utf8, &utf8Storage );
			require_noerr( err, exit );
			
			if( !inFile )
			{
				require_action( inPath, exit, err = kPathErr );
				file = fopen( inPath, "wb" );
				err = map_global_value_errno( file, file  );
				require_noerr( err, exit );
				inFile = file;
			}
			
			stringLen = strlen( utf8 );
			nWrote = fwrite( utf8, 1, stringLen, inFile );
			err = map_global_value_errno( nWrote == stringLen, inFile );
			require_noerr( err, exit );
			goto exit;
		}
		else if( typeID == CFDataGetTypeID() )
		{
			if( !inFile )
			{
				require_action( inPath, exit, err = kPathErr );
				file = fopen( inPath, "wb" );
				err = map_global_value_errno( file, file );
				require_noerr( err, exit );
				inFile = file;
			}
			
			dataLen = CFDataGetLength( (CFDataRef) inPlist );
			nWrote = fwrite( CFDataGetBytePtr( (CFDataRef) inPlist ), 1, (size_t) dataLen, inFile );
			err = map_global_value_errno( nWrote == (size_t) dataLen, inFile );
			require_noerr( err, exit );
			goto exit;
		}
		else
		{
			inFormat += sizeof_string( "raw-" ); // Skip "raw-" to point to the alternate format.
		}
	}
	if( 0 ) {} // Empty if to simplify conditionalize code below.
#if( CFL_BINARY_PLISTS )
	else if( strcmp( inFormat, "cflbinary" ) == 0 )
	{
		err = CFBinaryPlistCreateData( NULL, inPlist, &data );
		require_noerr( err, exit );
	}
#endif
#if( CFL_XML )
	else if( ( strcmp( inFormat, "xml" ) == 0 ) || ( strcmp( inFormat, "xml1" ) == 0 ) )
	{
		data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListXMLFormat_v1_0, 0, NULL );
		require_action( data, exit, err = kUnknownErr );
	}
#endif
#if( 0 || CFL_BINARY_PLISTS )
	else if( strcmp( inFormat, "binary1" ) == 0 )
	{
		data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
		require_action( data, exit, err = kUnknownErr );
	}
#endif
	else
	{
		dlogassert( "unknown format: '%s'\n", inFormat );
		err = kUnsupportedErr;
		goto exit;
	}
	
	if( !inFile )
	{
		require_action( inPath, exit, err = kPathErr );
		file = fopen( inPath, "wb" );
		err = map_global_value_errno( file, file );
		require_noerr( err, exit );
		inFile = file;
	}
	
	dataLen = CFDataGetLength( data );
	nWrote = fwrite( CFDataGetBytePtr( data ), 1, (size_t) dataLen, inFile );
	err = map_global_value_errno( nWrote == (size_t) dataLen, inFile );
	require_noerr( err, exit );
	
exit:
	if( utf8Storage )	free( utf8Storage );
	if( data )			CFRelease( data );
	if( file )			fclose( file );
	return( err );
}

//===========================================================================================================================
//	CFStringGetOrCopyCStringUTF8
//===========================================================================================================================

OSStatus	CFStringGetOrCopyCStringUTF8( CFStringRef inString, const char **outUTF8, char **outStorage )
{
	OSStatus			err;
	const char *		ptr;
	uint8_t *			storage;
	CFRange				range;
	CFIndex				size;
	
	storage = NULL;
	
	ptr = CFStringGetCStringPtr( inString, kCFStringEncodingUTF8 );
	if( !ptr )
	{
		range = CFRangeMake( 0, CFStringGetLength( inString ) );
		size = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
		
		storage = (uint8_t *) malloc( (size_t)( size + 1 ) );
		require_action( storage, exit, err = kNoMemoryErr );
		
		range.location = CFStringGetBytes( inString, range, kCFStringEncodingUTF8, 0, false, storage, size, &size );
		require_action( range.location == range.length, exit, err = kUnknownErr );
		
		storage[ size ] = '\0';
		ptr = (const char *) storage;
	}
	
	*outUTF8	= ptr;
	*outStorage = (char *) storage;
	storage		= NULL;
	err			= kNoErr;
	
exit:
	if( storage ) free( storage );
	return( err );
}

//===========================================================================================================================
//	CFStringCopyUTF8CString
//===========================================================================================================================

OSStatus	CFStringCopyUTF8CString( CFStringRef inString, char **outUTF8 )
{
	OSStatus			err;
	const char *		src;
	CFRange				range;
	CFIndex				size;
	uint8_t *			utf8;
	
	utf8 = NULL;
	
	src = CFStringGetCStringPtr( inString, kCFStringEncodingUTF8 );
	if( src )
	{
		utf8 = (uint8_t *) strdup( src );
		require_action( utf8, exit, err = kNoMemoryErr );
	}
	else
	{
		range = CFRangeMake( 0, CFStringGetLength( inString ) );
		size = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
		
		utf8 = (uint8_t *) malloc( (size_t)( size + 1 ) );
		require_action( utf8, exit, err = kNoMemoryErr );
		
		range.location = CFStringGetBytes( inString, range, kCFStringEncodingUTF8, 0, false, utf8, size, &size );
		require_action( range.location == range.length, exit, err = kUnknownErr );
		
		utf8[ size ] = '\0';
	}
	
	*outUTF8 = (char *) utf8; utf8 = NULL;
	err = kNoErr;
	
exit:
	if( utf8 ) free( utf8 );
	return( err );
}

//===========================================================================================================================
//	MapCFStringToValue
//===========================================================================================================================

int	MapCFStringToValue( CFStringRef inString, int inDefaultValue, ... )
{
	va_list			args;
	CFStringRef		str;
	int				val;
	int				x;
	
	check( inString );
	
	val = inDefaultValue;
	va_start( args, inDefaultValue );
	for( ;; )
	{
		str = va_arg( args, CFStringRef );
		if( !str ) break;
		
		x = va_arg( args, int );
		if( CFEqual( inString, str ) )
		{
			val = x;
			break;
		}
	}
	va_end( args );
	return( val );
}

//===========================================================================================================================
//	MapValueToCFString
//===========================================================================================================================

CFStringRef	MapValueToCFString( int inValue, CFStringRef inDefaultStr, ... )
{
	va_list			args;
	CFStringRef		mappedStr;
	CFStringRef		str;
	int				val;
	
	mappedStr = inDefaultStr;
	va_start( args, inDefaultStr );
	for( ;; )
	{
		str = va_arg( args, CFStringRef );
		if( !str ) break;
		
		val = va_arg( args, int );
		if( inValue == val )
		{
			mappedStr = str;
			break;
		}
	}
	va_end( args );
	return( mappedStr );
}

//===========================================================================================================================
//	StringToRangeArray
//
//	Parses a number list string (e.g. "1-5" or "1,2,3-7" or "3,2,1") to a CFArray of begin/end number CFDictionary's.
//===========================================================================================================================

OSStatus	StringToRangeArray( const char *inStr, CFArrayRef *outArray )
{
	OSStatus					err;
	CFMutableArrayRef			array;
	const unsigned char *		p;
	const unsigned char *		q;
	int							x;
	int							y;
	int							z;
	
	array = NULL;
	
	require_action( inStr, exit, err = kParamErr );
	
	array = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	require_action( array, exit, err = kNoMemoryErr );
	
	p = (const unsigned char *) inStr;
	while( isspace( *p ) ) ++p;
	while( *p != '\0' )
	{
		// Parse a number, skipping any leading or trailing whitespace.
		
		q = p;
		x = 0;
		for( ; isdigit( *p ); ++p ) x = ( x * 10 ) + ( *p - '0' );
		require_action_quiet( p > q, exit, err = kMalformedErr );
		while( isspace( *p ) ) ++p;
		
		if( *p == '-' ) // Range (e.g. 1-5).
		{
			// Parse the second number in the range, skipping any leading whitespace.
			
			++p;
			while( isspace( *p ) ) ++p;
			q = p;
			y = 0;
			for( ; isdigit( *p ); ++p ) y = ( y * 10 ) + ( *p - '0' );
			require_action_quiet( p > q, exit, err = kMalformedErr );
			
			if( x > y )	// First > second (e.g. 5-1) so swap them to put in ascending order.
			{
				z = x;
				x = y;
				y = z;
			}
		}
		else // Single number so begin=end (ranges are inclusive).
		{
			y = x;
		}
		err = CFPropertyListAppendFormatted( kCFAllocatorDefault, array, "{" "begin=%i" "end=%i" "}", x, y );
		require_noerr( err, exit );
		
		// Skip space and comma delimiters to move to the next entry.
		
		while( isspace( *p ) ) ++p;
		if( *p == ',' ) ++p;
		while( isspace( *p ) ) ++p;
	}
	
	*outArray = array; array = NULL;
	err = kNoErr;
	
exit:
	if( array ) CFRelease( array );
	return( err );
}

//===========================================================================================================================
//	RangeArrayToString
//
//	Converts a CFArray of begin/end number CFDictionary's to a string (e.g. "1, 2, 3-7").
//===========================================================================================================================

OSStatus	RangeArrayToString( CFArrayRef inArray, CFStringRef *outString )
{
	OSStatus				err;
	CFMutableStringRef		str;
	CFIndex					i;
	CFIndex					n;
	CFDictionaryRef			dict;
	CFNumberRef				num;
	int						x;
	int						y;
	Boolean					good;
	char					buf[ 64 ];
	char *					ptr;
	
	str = CFStringCreateMutable( kCFAllocatorDefault, 0 );
	require_action( str, exit, err = kNoMemoryErr );
	
	n = CFArrayGetCount( inArray );
	for( i = 0; i < n; )
	{
		// Extract the begin/end numbers.
		
		dict = (CFDictionaryRef) CFArrayGetValueAtIndex( inArray, i );
		require_action( CFGetTypeID( dict ) == CFDictionaryGetTypeID(), exit, err = kTypeErr );
		
		num = (CFNumberRef) CFDictionaryGetValue( dict, CFSTR( "begin" ) );
		require_action( num, exit, err = kNotFoundErr );
		require_action( CFGetTypeID( num ) == CFNumberGetTypeID(), exit, err = kTypeErr );
		
		good = CFNumberGetValue( num, kCFNumberIntType, &x );
		require_action( good, exit, err = kSizeErr );
		require_action( x >= 0, exit, err = kRangeErr );
		
		num = (CFNumberRef) CFDictionaryGetValue( dict, CFSTR( "end" ) );
		require_action( num, exit, err = kNotFoundErr );
		require_action( CFGetTypeID( num ) == CFNumberGetTypeID(), exit, err = kTypeErr );
		
		good = CFNumberGetValue( num, kCFNumberIntType, &y );
		require_action( good, exit, err = kSizeErr );
		require_action( y >= 0, exit, err = kRangeErr );
		
		// Append the entry to the string.
		
		ptr = buf;
		if( x == y ) ptr += snprintf( buf, sizeof( buf ), "%d", x );
		else		 ptr += snprintf( buf, sizeof( buf ), "%d-%d", x, y );
		++i;
		if( i < n )
		{
			*ptr++ = ',';
			*ptr++ = ' ';
			*ptr   = '\0';
		}
		CFStringAppendCString( str, buf, kCFStringEncodingUTF8 );
	}
	
	*outString = str; str = NULL;
	err = kNoErr;
	
exit:
	if( str ) CFRelease( str );
	return( err );
}

//===========================================================================================================================
//	ValidateRangeArrayStrings
//
//	Checks if two range array strings are compatible.
//===========================================================================================================================

OSStatus	ValidateRangeArrayStrings( const char *inStr1, const char *inStr2 )
{
	OSStatus			err;
	CFArrayRef			a1;
	CFArrayRef			a2;
	CFDictionaryRef		dict;
	CFIndex				i;
	CFIndex				n;
	int					begin;
	int					end;
	int					x;
	
	a1 = NULL;
	a2 = NULL;
	
	err = StringToRangeArray( inStr1, &a1 );
	require_noerr( err, exit );
	
	err = StringToRangeArray( inStr2, &a2 );
	require_noerr( err, exit );
	
	// Both arrays must map to the exact same number of entries.
	
	n = CFArrayGetCount( a1 );
	require_action_quiet( n == CFArrayGetCount( a2 ), exit, err = kCountErr );
	
	// Each entry must the exactly same distance between begin and end numbers.
	
	for( i = 0; i < n; ++i )
	{
		dict = (CFDictionaryRef) CFArrayGetValueAtIndex( a1, i );
		
		err = CFPropertyListExtractFormatted( dict, &begin, "%kO:int", CFSTR( "begin" ) );
		require_noerr( err, exit );
		
		err = CFPropertyListExtractFormatted( dict, &end, "%kO:int", CFSTR( "end" ) );
		require_noerr( err, exit );
		
		x = end - begin;
		
		dict = (CFDictionaryRef) CFArrayGetValueAtIndex( a2, i );
		
		err = CFPropertyListExtractFormatted( dict, &begin, "%kO:int", CFSTR( "begin" ) );
		require_noerr( err, exit );
		
		err = CFPropertyListExtractFormatted( dict, &end, "%kO:int", CFSTR( "end" ) );
		require_noerr( err, exit );
		
		require_action_quiet( x == ( end - begin ), exit, err = kRangeErr );
	}
	err = kNoErr;
	
exit:
	if( a1 ) CFRelease( a1 );
	if( a2 ) CFRelease( a2 );
	return( err );
}

//===========================================================================================================================
//	ConflictingRangeArrayStrings
//
//	Checks if two range array strings have any overlapping values. kNoErr means no conflicts.
//===========================================================================================================================

OSStatus	ConflictingRangeArrayStrings( const char *inStr1, const char *inStr2 )
{
	OSStatus		err;
	CFArrayRef		array;
	
	require_action( inStr1, exit, err = kParamErr );
	require_action( inStr2, exit, err = kParamErr );
	
	err = StringToRangeArray( inStr2, &array );
	require_noerr( err, exit );
	
	err = ConflictingRangeArrayStringAndRangeArray( inStr1, array );
	CFRelease( array );
	
exit:
	return( err );
}

//===========================================================================================================================
//	ConflictingRangeArrayStringAndRangeArray
//
//	Checks if a range array strings and a range array have any overlapping values. kNoErr means no conflicts.
//===========================================================================================================================

OSStatus	ConflictingRangeArrayStringAndRangeArray( const char *inStr, CFArrayRef inArray )
{
	OSStatus			err;
	CFArrayRef			outerArray;
	CFArrayRef			innerArray;
	CFDictionaryRef		outerDict;
	CFDictionaryRef		innerDict;
	CFIndex				outerIndex;
	CFIndex				outerCount;
	CFIndex				innerIndex;
	CFIndex				innerCount;
	int					outerBegin;
	int					outerEnd;
	int					innerBegin;
	int					innerEnd;
	
	outerArray = NULL;
	innerArray = NULL;
	
	require_action( inStr, exit, err = kParamErr );
	require_action( inArray, exit, err = kParamErr );
	
	err = StringToRangeArray( inStr, &outerArray );
	require_noerr( err, exit );
	
	innerArray = inArray;
	
	outerCount = CFArrayGetCount( outerArray );
	innerCount = CFArrayGetCount( innerArray );
	
	// For each entry, compare its begin and end against each begin and end in the other array.
	
	for( outerIndex = 0; outerIndex < outerCount; ++outerIndex )
	{
		outerDict = (CFDictionaryRef) CFArrayGetValueAtIndex( outerArray, outerIndex );
		
		err = CFPropertyListExtractFormatted( outerDict, &outerBegin, "%kO:int", CFSTR( "begin" ) );
		require_noerr( err, exit );
		
		err = CFPropertyListExtractFormatted( outerDict, &outerEnd, "%kO:int", CFSTR( "end" ) );
		require_noerr( err, exit );
		
		for( innerIndex = 0; innerIndex < innerCount; ++innerIndex )
		{
			innerDict = (CFDictionaryRef) CFArrayGetValueAtIndex( innerArray, innerIndex );
			
			err = CFPropertyListExtractFormatted( innerDict, &innerBegin, "%kO:int", CFSTR( "begin" ) );
			require_noerr( err, exit );
			
			err = CFPropertyListExtractFormatted( innerDict, &innerEnd, "%kO:int", CFSTR( "end" ) );
			require_noerr( err, exit );
			
			// Check for the actual conflicts.
			
			require_action_quiet( ( ( innerBegin < outerBegin ) && ( innerEnd < outerEnd ) ) || 
								  ( ( innerBegin > outerBegin ) && ( innerEnd > outerEnd ) ), 
								  exit, err = kAlreadyInUseErr );
		}
	}
	err = kNoErr;
	
exit:
	if( outerArray ) CFRelease( outerArray );
	return( err );
}

//===========================================================================================================================
//	CFNumberCreateInt64
//===========================================================================================================================

CFNumberRef	CFNumberCreateInt64( int64_t x )
{
	int8_t			s8;
	int16_t			s16;
	int32_t			s32;
	CFNumberType	type;
	void *			ptr;
	
	if(      ( x >= INT8_MIN )  && ( x <= INT8_MAX ) )  { s8  = (int8_t)  x; type = kCFNumberSInt8Type;  ptr = &s8; }
	else if( ( x >= INT16_MIN ) && ( x <= INT16_MAX ) ) { s16 = (int16_t) x; type = kCFNumberSInt16Type; ptr = &s16; }
	else if( ( x >= INT32_MIN ) && ( x <= INT32_MAX ) ) { s32 = (int32_t) x; type = kCFNumberSInt32Type; ptr = &s32; }
	else												{					 type = kCFNumberSInt64Type; ptr = &x; }
	
	return( CFNumberCreate( NULL, type, ptr ) );
}

//===========================================================================================================================
//	CFCreateObjectFromString
//===========================================================================================================================

OSStatus	CFCreateObjectFromString( const char *inStr, CFTypeRef *outObj )
{
	return( CFCreateObjectFromStringEx( inStr, "", outObj ) );
}

//===========================================================================================================================
//	CFCreateObjectFromStringEx
//===========================================================================================================================

OSStatus	CFCreateObjectFromStringEx( const char *inStr, const char *inType, CFTypeRef *outObj )
{
	OSStatus				err;
	size_t					len;
	int64_t					s64;
	int						n;
	int						offset;
	CFTypeRef				obj;
	CFMutableDataRef		data;
	
	data = NULL;
	
	if( ( *inType == '\0' ) || ( strcmp( inType, "--bool" ) == 0 ) )
	{
		// Boolean true.
		
		if( ( stricmp( inStr, "true" )	== 0 ) || 
			( stricmp( inStr, "yes" )	== 0 ) )
		{
			*outObj = kCFBooleanTrue;
			err = kNoErr;
			goto exit;
		}
		
		// Boolean false.
		
		if( ( stricmp( inStr, "false" )	== 0 ) || 
			( stricmp( inStr, "no" )	== 0 ) )
		{
			*outObj = kCFBooleanFalse;
			err = kNoErr;
			goto exit;
		}
	}
	
	// Number
	
	if( ( *inType == '\0' ) || ( strcmp( inType, "--integer" ) == 0 ) )
	{
		offset = -1;
		len = strlen( inStr );
		n = SNScanF( inStr, len, "%lli %n", &s64, &offset );
		if( ( n == 1 ) && ( offset == (int) len ) )
		{
			obj = CFNumberCreateInt64( s64 );
			require_action( obj, exit, err = kUnknownErr );
			
			*outObj = obj;
			err = kNoErr;
			goto exit;
		}
	}
	
	// Data
	
	if( strcmp( inType, "--hex" ) == 0 )
	{
		err = HexToData( inStr, kSizeCString, kHexToData_DefaultFlags, NULL, 0, NULL, &len, NULL );
		require_noerr_quiet( err, exit );
		
		data = CFDataCreateMutable( NULL, 0 );
		require_action( data, exit, err = kNoMemoryErr );
		
		CFDataSetLength( data, (CFIndex) len );
		require_action( CFDataGetLength( data ) == (CFIndex) len, exit, err = kNoMemoryErr );
		
		err = HexToData( inStr, kSizeCString, kHexToData_DefaultFlags, CFDataGetMutableBytePtr( data ), len, NULL, &len, NULL );
		require_noerr( err, exit );
		
		*outObj = data;
		data = NULL;
		err = kNoErr;
		goto exit;
	}
	
	// TXT Record
	
	if( strcmp( inType, "--txt" ) == 0 )
	{
		uint8_t *		txtRec;
		size_t			txtLen;
		
		err = CreateTXTRecordWithCString( inStr, &txtRec, &txtLen );
		require_noerr_quiet( err, exit );
		
		obj = CFDataCreate( NULL, txtRec, (CFIndex) txtLen );
		free( txtRec );
		require_action( obj, exit, err = kNoMemoryErr );
		
		*outObj = obj;
		err = kNoErr;
		goto exit;
	}
	
	// Array
	
	if( ( ( *inType == '\0' ) && ( strcmp( inStr, "[]" ) == 0 ) ) || ( strcmp( inType, "--array" ) == 0 ) )
	{
		obj = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action( obj, exit, err = kUnknownErr );
		
		*outObj = obj;
		err = kNoErr;
		goto exit;
	}
	
	// Dictionary
	
	if( ( ( *inType == '\0' ) && ( strcmp( inStr, "{}" ) == 0 ) ) || ( strcmp( inType, "--dict" ) == 0 ) )
	{
		obj = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( obj, exit, err = kUnknownErr );
		
		*outObj = obj;
		err = kNoErr;
		goto exit;
	}
	
	// Assume anything else is just a string.
	
	if( ( *inType == '\0' ) || ( strcmp( inType, "--string" ) == 0 ) )
	{
		obj = CFStringCreateWithCString( NULL, inStr, kCFStringEncodingUTF8 );
		require_action( obj, exit, err = kUnknownErr );
		
		*outObj = obj;
		err = kNoErr;
		goto exit;
	}
	
	err = kFormatErr;
	
exit:
	if( data ) CFRelease( data );
	return( err );
}

//===========================================================================================================================
//	CFCreateWithPlistBytes
//===========================================================================================================================

CFTypeRef	CFCreateWithPlistBytes( const void *inPtr, size_t inLen, uint32_t inFlags, CFTypeID inType, OSStatus *outErr )
{
	CFTypeRef		result = NULL;
	OSStatus		err;
	CFDataRef		data;
	CFTypeRef		obj = NULL;
	Boolean			isMutable;
	
	if( inLen > 0 )
	{
		data = CFDataCreate( NULL, (const uint8_t *) inPtr, (CFIndex) inLen );
		require_action( data, exit, err = kNoMemoryErr );
		
		obj = (CFDictionaryRef) CFPropertyListCreateWithData( NULL, data, inFlags, NULL, NULL );
		CFRelease( data );
		require_action_quiet( obj, exit, err = kFormatErr );
		require_action_quiet( !inType || ( CFGetTypeID( obj ) == inType ), exit, err = kTypeErr );
	}
	else
	{
		isMutable = ( inFlags & kCFPropertyListImmutable ) ? false : true;
		if( inType == CFDictionaryGetTypeID() )
		{
			if( isMutable )	obj = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			else			obj = CFDictionaryCreate( NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require_action( obj, exit, err = kNoMemoryErr );
		}
		else if( inType == CFArrayGetTypeID() )
		{
			if( isMutable )	obj = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			else			obj = CFArrayCreate( NULL, NULL, 0, &kCFTypeArrayCallBacks );
			require_action( obj, exit, err = kNoMemoryErr );
		}
		else
		{
			err = kUnsupportedDataErr;
			goto exit;
		}
	}
	
	result = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( obj );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFDictionaryGetNestedValue
//===========================================================================================================================

const void *	CFDictionaryGetNestedValue( CFDictionaryRef inDict, CFStringRef inKey, ... )
{
	const void *		value;
	va_list				args;
	
	va_start( args, inKey );
	for( ;; )
	{
		value = CFDictionaryGetValue( inDict, inKey );
		if( !value ) break;
		
		inKey = va_arg( args, CFStringRef );
		if( !inKey ) break;
		
		inDict = (CFDictionaryRef) value;
		require_action( CFGetTypeID( inDict ) == CFDictionaryGetTypeID(), exit, value = NULL );
	}
	
exit:
	va_end( args );
	return( value );
}

//===========================================================================================================================
//	CFCopyCString
//===========================================================================================================================

char *	CFCopyCString( CFTypeRef inObj, OSStatus *outErr )
{
	char *			result;
	OSStatus		err;
	CFTypeID		typeID;
	size_t			len;
	char			tempCStr[ 128 ];
	
	result = NULL;
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFStringGetTypeID() )
	{
		err = CFStringCopyUTF8CString( (CFStringRef) inObj, &result );
		require_noerr( err, exit );
	}
	else if( typeID == CFDataGetTypeID() )
	{
		len = (size_t)( ( CFDataGetLength( (CFDataRef) inObj ) * 2 ) + 1 );
		result = (char *) malloc( len );
		require_action( result, exit, err = kNoMemoryErr );
		
		*result = '\0';
		CFGetCString( inObj, result, len );
	}
	else
	{
		*tempCStr = '\0';
		CFGetCString( inObj, tempCStr, sizeof( tempCStr ) );
		
		result = strdup( tempCStr );
		require_action( result, exit, err = kNoMemoryErr );
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFGetCString
//===========================================================================================================================

char *	CFGetCString( CFTypeRef inObj, char *inBuf, size_t inMaxLen )
{
	CFTypeID		typeID;
	
	if( inMaxLen <= 0 ) return( "" );
	if( !inObj ) { *inBuf = '\0'; return( inBuf ); }
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFStringGetTypeID() )
	{
		CFStringGetCString( (CFStringRef) inObj, inBuf, (CFIndex) inMaxLen, kCFStringEncodingUTF8 );
	}
	else if( typeID == CFNumberGetTypeID() )
	{
		double		d;
		int64_t		s64;
		
		if( CFNumberIsFloatType( (CFNumberRef) inObj ) )
		{
			d = 0;
			CFNumberGetValue( (CFNumberRef) inObj, kCFNumberDoubleType, &d );
			snprintf( inBuf, inMaxLen, "%f", d );
		}
		else
		{
			s64 = 0;
			CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &s64 );
			SNPrintF( inBuf, inMaxLen, "%lld", s64 );
		}
	}
	else if( inObj == kCFBooleanTrue )  strlcpy( inBuf, "true", inMaxLen );
	else if( inObj == kCFBooleanFalse ) strlcpy( inBuf, "false", inMaxLen );
	else if( typeID == CFDataGetTypeID() )
	{
		const uint8_t *		src;
		const uint8_t *		end;
		char *				dst;
		char *				lim;
		uint8_t				b;
		
		src = CFDataGetBytePtr( (CFDataRef) inObj );
		end = src + CFDataGetLength( (CFDataRef) inObj );
		dst = inBuf;
		lim = dst + ( inMaxLen - 1 );
		while( ( src < end ) && ( ( lim - dst ) >= 2 ) )
		{
			b = *src++;
			*dst++ = kHexDigitsLowercase[ b >> 4 ];
			*dst++ = kHexDigitsLowercase[ b & 0xF ];
		}
		*dst = '\0';
	}
	else if( typeID == CFDateGetTypeID() )
	{
		int					year, month, day, hour, minute, second, micros;
		const char *		amPMStr;
		
		CFDateGetComponents( (CFDateRef) inObj, &year, &month, &day, &hour, &minute, &second, &micros );
		if(      hour ==  0  )	{ amPMStr = "AM"; hour  = 12; }
		else if( hour  < 12 )	{ amPMStr = "AM"; }
		else if( hour == 12 )	{ amPMStr = "PM"; }
		else					{ amPMStr = "PM"; hour -= 12; }
		snprintf( inBuf, inMaxLen, "%04d-%02d-%02d %02d:%02d:%02d.%06d %s", 
			year, month, day, hour, minute, second, micros, amPMStr );
	}
	else if( typeID == CFDictionaryGetTypeID() )	snprintf( inBuf, inMaxLen, "{}" );
	else if( typeID == CFArrayGetTypeID() )			snprintf( inBuf, inMaxLen, "[]" );
	else											*inBuf = '\0';
	return( inBuf );
}

//===========================================================================================================================
//	CFCopyCFData
//===========================================================================================================================

CFDataRef	CFCopyCFData( CFTypeRef inObj, size_t *outLen, OSStatus *outErr )
{
	CFDataRef		data = NULL;
	OSStatus		err;
	uint8_t *		ptr;
	size_t			len = 0;
	
	ptr = CFCopyData( inObj, &len, &err );
	require_noerr_quiet( err, exit );
	
	data = CFDataCreate( NULL, ptr, (CFIndex) len );
	free( ptr );
	require_action( data, exit, err = kNoMemoryErr );
	
exit:
	if( outLen ) *outLen = len;
	if( outErr ) *outErr = err;
	return( data );
}

//===========================================================================================================================
//	CFCopyData
//===========================================================================================================================

uint8_t *	CFCopyData( CFTypeRef inObj, size_t *outLen, OSStatus *outErr )
{
	uint8_t *			result = NULL;
	OSStatus			err;
	CFTypeID			typeID;
	const uint8_t *		src;
	size_t				len = 0;
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFDataGetTypeID() )
	{
		src = CFDataGetBytePtr( (CFDataRef) inObj );
		len = (size_t) CFDataGetLength( (CFDataRef) inObj );
		result = (uint8_t *) malloc( ( len > 0 ) ? len : 1 ); // Use 1 if 0 since malloc( 0 ) is undefined.
		require_action( result, exit, err = kNoMemoryErr );
		if( len > 0 ) memcpy( result, src, len );
	}
	else if( typeID == CFStringGetTypeID() )
	{
		const char *		utf8Ptr;
		char *				utf8Buf;
		
		err = CFStringGetOrCopyCStringUTF8( (CFStringRef) inObj, &utf8Ptr, &utf8Buf );
		require_noerr( err, exit );
		
		err = HexToDataCopy( utf8Ptr, kSizeCString, 
			kHexToData_IgnoreDelimiters | kHexToData_IgnorePrefixes | kHexToData_IgnoreWhitespace, 
			&result, &len, NULL );
		if( utf8Buf ) free( utf8Buf );
		require_noerr( err, exit );
	}
	else if( typeID == CFNullGetTypeID() )
	{
		result = (uint8_t *) malloc( 1 ); // Use 1 since malloc( 0 ) is undefined.
		require_action( result, exit, err = kNoMemoryErr );
		len = 0;
	}
	else
	{
		err = kUnsupportedErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outLen ) *outLen = len;
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFGetData
//===========================================================================================================================

uint8_t *	CFGetData( CFTypeRef inObj, void *inBuf, size_t inMaxLen, size_t *outLen, OSStatus *outErr )
{
	OSStatus			err;
	CFTypeID			typeID;
	const uint8_t *		src;
	size_t				len;
	
	len = 0;
	require_action_quiet( inObj, exit, err = kNoErr );
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFDataGetTypeID() )
	{
		src = CFDataGetBytePtr( (CFDataRef) inObj );
		len = (size_t) CFDataGetLength( (CFDataRef) inObj );
		if( inBuf )
		{
			if( len > inMaxLen ) len = inMaxLen;
			if( len > 0 ) memcpy( inBuf, src, len );
		}
		else
		{
			inBuf = (void *) src;
		}
	}
	else if( typeID == CFStringGetTypeID() )
	{
		const char *		utf8Ptr;
		char *				utf8Buf;
		
		err = CFStringGetOrCopyCStringUTF8( (CFStringRef) inObj, &utf8Ptr, &utf8Buf );
		require_noerr( err, exit );
		
		HexToData( utf8Ptr, kSizeCString, 
			kHexToData_IgnoreDelimiters | kHexToData_IgnorePrefixes | kHexToData_IgnoreWhitespace, 
			inBuf, inMaxLen, &len, NULL, NULL );
		if( utf8Buf ) free( utf8Buf );
	}
	else if( typeID == CFNullGetTypeID() )
	{
		inBuf = (void *) "";
		len   = 0;
	}
	else
	{
		err = kUnsupportedErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outLen ) *outLen = len;
	if( outErr ) *outErr = err;
	return( (uint8_t *) inBuf );
}

//===========================================================================================================================
//	CFGetDouble
//===========================================================================================================================

double	CFGetDouble( CFTypeRef inObj, OSStatus *outErr )
{
	double			value;
	CFTypeID		typeID;
	OSStatus		err;
	
	value = 0;
	require_action_quiet( inObj, exit, err = kNoErr );
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFNumberGetTypeID() )
	{
		CFNumberGetValue( (CFNumberRef) inObj, kCFNumberDoubleType, &value );
		err = kNoErr;
	}
	else if( typeID == CFStringGetTypeID() )
	{
		char		tempStr[ 128 ];
		Boolean		good;
		
		good = CFStringGetCString( (CFStringRef) inObj, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingASCII );
		require_action_quiet( good, exit, err = kSizeErr );
		
		if(      strcasecmp( tempStr, "true" )  == 0 ) value = 1;
		else if( strcasecmp( tempStr, "false" ) == 0 ) value = 0;
		else if( strcasecmp( tempStr, "yes" )   == 0 ) value = 1;
		else if( strcasecmp( tempStr, "no" )    == 0 ) value = 0;
		else if( strcasecmp( tempStr, "on" )    == 0 ) value = 1;
		else if( strcasecmp( tempStr, "off" )   == 0 ) value = 0;
		else if( sscanf( tempStr, "%lf", &value ) != 1 ) { err = kFormatErr; goto exit; }
		err = kNoErr;
	}
	else if( typeID == CFDateGetTypeID() )
	{
		value = CFDateGetAbsoluteTime( (CFDateRef) inObj );
		err = kNoErr;
	}
	else
	{
		value = (double) CFGetInt64( inObj, &err );
	}
	
exit:
	if( outErr ) *outErr = kNoErr;
	return( value );
}

//===========================================================================================================================
//	CFGetHardwareAddress
//===========================================================================================================================

uint64_t	CFGetHardwareAddress( CFTypeRef inObj, uint8_t *inBuf, size_t inLen, OSStatus *outErr )
{
	uint64_t			scalar;
	OSStatus			err;
	CFTypeID			typeID;
	char				tempStr[ 64 ];
	uint8_t				tempAddr[ 8 ];
	Boolean				good;
	const uint8_t *		ptr;
	
	scalar = 0;
	require_action_quiet( inObj, exit, err = kNoErr );
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFStringGetTypeID() )
	{
		good = CFStringGetCString( (CFStringRef) inObj, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingASCII );
		require_action_quiet( good, exit, err = kSizeErr );
		
		if( inBuf == NULL )
		{
			require_action( inLen <= sizeof( tempAddr ), exit, err = kSizeErr );
			inBuf = tempAddr;
		}
		err = TextToHardwareAddress( tempStr, kSizeCString, inLen, inBuf );
		require_noerr_quiet( err, exit );
		if(      inLen == 6 ) scalar = ReadBig48( inBuf );
		else if( inLen == 8 ) scalar = ReadBig64( inBuf );
	}
	else if( typeID == CFNumberGetTypeID() )
	{
		CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &scalar );
		if( inBuf )
		{
			if(      inLen == 6 ) WriteBig48( inBuf, scalar );
			else if( inLen == 8 ) WriteBig64( inBuf, scalar );
		}
	}
	else if( typeID == CFDataGetTypeID() )
	{
		require_action_quiet( CFDataGetLength( (CFDataRef) inObj ) == (CFIndex) inLen, exit, err = kSizeErr );
		ptr = CFDataGetBytePtr( (CFDataRef) inObj );
		if( inBuf ) memcpy( inBuf, ptr, inLen );
		if(      inLen == 6 ) scalar = ReadBig48( ptr );
		else if( inLen == 8 ) scalar = ReadBig64( ptr );
	}
	else
	{
		err = kTypeErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( scalar );
}

//===========================================================================================================================
//	CFGetInt64
//===========================================================================================================================

int64_t	CFGetInt64( CFTypeRef inObj, OSStatus *outErr )
{
	int64_t			value;
	OSStatus		err;
	CFTypeID		typeID;
	Boolean			good;
	
	value = 0;
	require_action_quiet( inObj, exit, err = kNoErr );
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFNumberGetTypeID() )
	{
		if( CFNumberIsFloatType( (CFNumberRef) inObj ) )
		{
			double		tempDouble;
			
			tempDouble = 0;
			CFNumberGetValue( (CFNumberRef) inObj, kCFNumberDoubleType, &tempDouble );
			if(      tempDouble < INT64_MIN ) { value = INT64_MIN; err = kRangeErr; goto exit; }
			else if( tempDouble > INT64_MAX ) { value = INT64_MAX; err = kRangeErr; goto exit; }
			else								value = (int64_t) tempDouble;
		}
		else
		{
			CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &value );
		}
	}
	else if( ( (CFBooleanRef) inObj ) == kCFBooleanTrue )  value = 1;
	else if( ( (CFBooleanRef) inObj ) == kCFBooleanFalse ) value = 0;
	else if( typeID == CFStringGetTypeID() )
	{
		char		tempStr[ 128 ];
		
		good = CFStringGetCString( (CFStringRef) inObj, tempStr, (CFIndex) sizeof( tempStr ), kCFStringEncodingASCII );
		require_action_quiet( good, exit, err = kSizeErr );
		
		if(      strcasecmp( tempStr, "true" )  == 0 ) value = 1;
		else if( strcasecmp( tempStr, "false" ) == 0 ) value = 0;
		else if( strcasecmp( tempStr, "yes" )   == 0 ) value = 1;
		else if( strcasecmp( tempStr, "no" )    == 0 ) value = 0;
		else if( strcasecmp( tempStr, "on" )    == 0 ) value = 1;
		else if( strcasecmp( tempStr, "off" )   == 0 ) value = 0;
		else if( sscanf( tempStr, "%lli", &value ) != 1 ) { err = kFormatErr; goto exit; }
	}
	else if( typeID == CFDataGetTypeID() )
	{
		const uint8_t *		ptr;
		const uint8_t *		end;
		
		ptr = CFDataGetBytePtr( (CFDataRef) inObj );
		end = ptr + CFDataGetLength( (CFDataRef) inObj );
		require_action_quiet( ( end - ptr ) <= ( (ptrdiff_t) sizeof( int64_t ) ), exit, err = kSizeErr );
		
		for( ; ptr < end; ++ptr )
		{
			value = ( value << 8 ) + *ptr;
		}
	}
	else if( typeID == CFDateGetTypeID() )
	{
		value = (int64_t) CFDateGetAbsoluteTime( (CFDateRef) inObj );
	}
	else if( typeID == CFNullGetTypeID() ) {} // Leave value at 0.
	else
	{
		err = kTypeErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	CFGetUUID
//===========================================================================================================================

OSStatus	CFGetUUID( CFTypeRef inObj, uint8_t outUUID[ 16 ] )
{
	OSStatus		err;
	CFTypeID		typeID;
	char			cstr[ 64 ];
	Boolean			good;
	
	typeID = CFGetTypeID( inObj );
	if( typeID == CFStringGetTypeID() )
	{
		good = CFStringGetCString( (CFStringRef) inObj, cstr, (CFIndex) sizeof( cstr ), kCFStringEncodingASCII );
		require_action_quiet( good, exit, err = kSizeErr );
		
		err = StringToUUID( cstr, kSizeCString, false, outUUID );
		require_noerr_quiet( err, exit );
	}
	else if( typeID == CFDataGetTypeID() )
	{
		require_action_quiet( CFDataGetLength( (CFDataRef) inObj ) == 16, exit, err = kSizeErr );
		memcpy( outUUID, CFDataGetBytePtr( (CFDataRef) inObj ), 16 );
	}
	else
	{
		err = kTypeErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFSetObjectAtPath
//===========================================================================================================================

OSStatus	CFSetObjectAtPath( CFTypeRef inPlist, const char *inPath, CFTypeRef inObj )
{
	OSStatus			err;
	CFTypeRef			parent;
	CFTypeRef			newParent;
	const char *		name;
	char				c;
	size_t				len;
	CFStringRef			key;
	int					lastIndex;
	
	// Parse the format string to find the deepest parent.
	
	lastIndex = -1;
	parent = inPlist;
	for( ;; )
	{
		for( name = inPath; ( ( c = *inPath ) != '\0' ) && ( c != '.' ); ++inPath ) {}
		len = (size_t)( inPath - name );
		if( c == '\0' ) break;
		++inPath;
		
		// Handle [] array specifiers.
		
		if( *name == '[' )
		{
			int		i;
			int		n;
			
			n = SNScanF( name + 1, len - 1, "%i]", &i );
			require_action_quiet( n == 1, exit, err = kFormatErr );
			
			require_action_quiet( CFGetTypeID( parent ) == CFArrayGetTypeID(), exit, err = kTypeErr );
			require_action_quiet( ( i >= 0 ) && ( i < CFArrayGetCount( (CFArrayRef) parent ) ), exit, err = kRangeErr );
			
			parent = CFArrayGetValueAtIndex( (CFArrayRef) parent, i );
			lastIndex = i;
		}
		
		// Handle dictionary key specifiers.
		
		else
		{
			require_action_quiet( CFGetTypeID( parent ) == CFDictionaryGetTypeID(), exit, err = kTypeErr );
			
			key = CFStringCreateWithBytes( NULL, (const uint8_t *) name, (CFIndex) len, kCFStringEncodingUTF8, false );
			require_action( key, exit, err = kUnknownErr );
			
			newParent = CFDictionaryGetValue( (CFDictionaryRef) parent, key );
			if( !newParent )
			{
				newParent = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, 
					&kCFTypeDictionaryValueCallBacks );
				if( newParent )
				{
					CFDictionarySetValue( (CFMutableDictionaryRef) parent, key, newParent );
					CFRelease( newParent );
				}
			}
			CFRelease( key );
			require_action( newParent, exit, err = kNoMemoryErr );
			parent = newParent;
			lastIndex = -1;
		}
	}
	
	// Set the value.
	
	if( CFGetTypeID( parent ) == CFDictionaryGetTypeID() )
	{
		require_action_quiet( len > 0, exit, err = kFormatErr );
		
		key = CFStringCreateWithBytes( NULL, (const uint8_t *) name, (CFIndex) len, kCFStringEncodingUTF8, false );
		require_action( key, exit, err = kUnknownErr );
		
		if( inObj ) CFDictionarySetValue( (CFMutableDictionaryRef) parent, key, inObj );
		else		CFDictionaryRemoveValue( (CFMutableDictionaryRef) parent, key );
		CFRelease( key );
	}
	else if( CFGetTypeID( parent ) == CFArrayGetTypeID() )
	{
		require_action_quiet( len == 0, exit, err = kFormatErr );
		
		if( inObj )
		{
			CFArrayAppendValue( (CFMutableArrayRef) parent, inObj );
		}
		else
		{
			require_action( lastIndex >= 0, exit, err = kFormatErr );
			
			CFArrayRemoveValueAtIndex( (CFMutableArrayRef) parent, lastIndex );
		}
	}
	else
	{
		err = kTypeErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFArrayAppendInt64
//===========================================================================================================================

OSStatus	CFArrayAppendInt64( CFMutableArrayRef inArray, int64_t inValue )
{
	OSStatus		err;
	CFNumberRef		num;
	
	num = CFNumberCreateInt64( inValue );
	require_action( num, exit, err = kNoMemoryErr );
	
	CFArrayAppendValue( inArray, num );
	CFRelease( num );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFArrayAppendCString
//===========================================================================================================================

OSStatus	CFArrayAppendCString( CFMutableArrayRef inArray, const char *inStr, size_t inLen )
{
	OSStatus		err;
	CFStringRef		cfstr;
	
	if( inLen == kSizeCString )
	{
		cfstr = CFStringCreateWithCString( NULL, inStr, kCFStringEncodingUTF8 );
		require_action( cfstr, exit, err = kUnknownErr );
	}
	else
	{
		cfstr = CFStringCreateWithBytes( NULL, (const uint8_t *) inStr, (CFIndex) inLen, kCFStringEncodingUTF8, false );
		require_action( cfstr, exit, err = kUnknownErr );
	}
	
	CFArrayAppendValue( inArray, cfstr );
	CFRelease( cfstr );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFArrayEnsureCreatedAndAppend
//===========================================================================================================================

OSStatus	CFArrayEnsureCreatedAndAppend( CFMutableArrayRef *ioArray, CFTypeRef inValue )
{
	CFMutableArrayRef		array = *ioArray;
	OSStatus				err;
	
	if( !array )
	{
		array = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action( array, exit, err = kNoMemoryErr );
		*ioArray = array;
	}
	CFArrayAppendValue( array, inValue );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFArrayEnsureCreatedAndAppendCString
//===========================================================================================================================

OSStatus	CFArrayEnsureCreatedAndAppendCString( CFMutableArrayRef *ioArray, const char *inStr, size_t inLen )
{
	OSStatus		err;
	CFStringRef		cfstr;
	
	if( inLen == kSizeCString )
	{
		cfstr = CFStringCreateWithCString( NULL, inStr, kCFStringEncodingUTF8 );
		require_action( cfstr, exit, err = kUnknownErr );
	}
	else
	{
		cfstr = CFStringCreateWithBytes( NULL, (const uint8_t *) inStr, (CFIndex) inLen, kCFStringEncodingUTF8, false );
		require_action( cfstr, exit, err = kUnknownErr );
	}
	
	err = CFArrayEnsureCreatedAndAppend( ioArray, cfstr );
	CFRelease( cfstr );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFArrayGetTypedValueAtIndex
//===========================================================================================================================

CFTypeRef	CFArrayGetTypedValueAtIndex( CFArrayRef inArray, CFIndex inIndex, CFTypeID inType, OSStatus *outErr )
{
	CFTypeRef		result = NULL;
	CFTypeRef		value;
	OSStatus		err;
	
	value = CFArrayGetValueAtIndex( inArray, inIndex );
	require_action_quiet( value, exit, err = kNotFoundErr );
	require_action_quiet( CFGetTypeID( value ) == inType, exit, err = kTypeErr );
	result = value;
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFDictionaryCopyKeys
//===========================================================================================================================

CFArrayRef	CFDictionaryCopyKeys( CFDictionaryRef inDict, OSStatus *outErr )
{
	CFArrayRef			result = NULL;
	OSStatus			err;
	CFIndex				n;
	const void **		keys = NULL;
	
	n = CFDictionaryGetCount( inDict );
	if( n > 0 )
	{
		keys = (const void **) malloc( (size_t)( n * sizeof( *keys ) ) );
		require_action( keys, exit, err = kNoMemoryErr );
		CFDictionaryGetKeysAndValues( inDict, keys, NULL );
	}
	
	result = CFArrayCreate( NULL, keys, n, &kCFTypeArrayCallBacks );
	if( keys ) free( (void *) keys );
	require_action( result, exit, err = kNoMemoryErr );
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFDictionaryCreateFromNameTypeValueArgList
//===========================================================================================================================

OSStatus	CFDictionaryCreateFromNameTypeValueArgList( CFDictionaryRef *outDict, int inArgI, int inArgC, const char *inArgV[] )
{
	OSStatus					err;
	CFMutableDictionaryRef		dict;
	const char *				s;
	const char *				name;
	const char *				nameEnd;
	const char *				type;
	const char *				typeEnd;
	const char *				value;
	const char *				valueEnd;
	int							x;
	int							n;
	char						tempName[ 256 ];
	char						tempValue[ 256 ];
	
	dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( dict, exit, err = kNoMemoryErr );
				
	while( inArgI < inArgC )
	{
		s = inArgV[ inArgI++ ];
		require_action( s, exit, err = kParamErr );
		
		// Parse the string as "name:type:value".
	
		name = s;
		nameEnd = strchr( name, ':' );
		require_action_quiet( nameEnd, exit, err = kMalformedErr );
		
		type = nameEnd + 1;
		typeEnd = strchr( type, ':' );
		require_action_quiet( typeEnd, exit, err = kMalformedErr );
		
		value = typeEnd + 1;
		
		// Append a key/value pair based on the type.
		
		if( strncmpx( type, (size_t)( typeEnd - type ), "b" ) == 0 )		// Boolean
		{
			if(      stricmp( value, "true" )	== 0 ) x = 1;
			else if( stricmp( value, "false" )	== 0 ) x = 0;
			else if( stricmp( value, "yes" )	== 0 ) x = 1;
			else if( stricmp( value, "no" )		== 0 ) x = 0;
			else if( stricmp( value, "0" )		== 0 ) x = 0;
			else if( stricmp( value, "1" )		== 0 ) x = 1;
			else { err = kValueErr; goto exit; }
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%b", (int)( nameEnd - name ), name, x );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "i" ) == 0 )	// Integer
		{
			n = sscanf( value, "%i", &x );
			require_action_quiet( n == 1, exit, err = kValueErr );
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%i", (int)( nameEnd - name ), name, x );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "m" ) == 0 )	// MAC address string
		{
			uint8_t		macAddr[ 6 ];
			
			err = TextToMACAddress( value, kSizeCString, macAddr );
			require_noerr_quiet( err, exit );
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%D", (int)( nameEnd - name ), name, macAddr, 6 );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "s" ) == 0 )	// String
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%s", (int)( nameEnd - name ), name, value );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "u" ) == 0 )	// UUID string
		{
			uint8_t		uuid[ 16 ];
			
			err = StringToUUID( value, kSizeCString, false, uuid );
			require_noerr_quiet( err, exit );
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%D", (int)( nameEnd - name ), name, uuid, 16 );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "h" ) == 0 )	// Hex string
		{
			uint8_t		buf[ 256 ];
			size_t		len;
			
			err = HexToData( value, kSizeCString, kHexToData_DefaultFlags, buf, sizeof( buf ), NULL, &len, NULL );
			require_noerr_quiet( err, exit );
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=%D", (int)( nameEnd - name ), name, buf, len );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "{}" ) == 0 )	// Dictionary
		{
			CFMutableDictionaryRef		tempDict;
			
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks={%@}", 
				(int)( nameEnd - name ), name, &tempDict );
			require_noerr( err, exit );
			
			valueEnd = value + strlen( value );
			while( ParseCommaSeparatedNameValuePair( value, valueEnd, 
				tempName,  sizeof( tempName ),  NULL, NULL, 
				tempValue, sizeof( tempValue ), NULL, NULL, 
				&value ) == kNoErr )
			{
				err = CFPropertyListAppendFormatted( NULL, tempDict, "%ks=%s", tempName, tempValue );
				require_noerr( err, exit );
			}
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "[]" ) == 0 )	// Empty Array
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, dict, "%.*ks=[]", (int)( nameEnd - name ), name );
			require_noerr( err, exit );
		}
		else if( strncmpx( type, (size_t)( typeEnd - type ), "s[]" ) == 0 )	// String Array
		{
			CFMutableArrayRef		array;
			
			err = CFPropertyListAppendFormatted( NULL, dict, "%.*ks=[%@]", (int)( nameEnd - name ), name, &array );
			require_noerr( err, exit );
			
			valueEnd = value + strlen( value );
			while( value < valueEnd )
			{
				err = ParseEscapedString( value, valueEnd, ',', tempValue, sizeof( tempValue ), NULL, NULL, &value );
				require_noerr_quiet( err, exit );
				
				err = CFPropertyListAppendFormatted( NULL, array, "%s", tempValue );
				require_noerr( err, exit );
			}
		}
		else
		{
			err = kTypeErr;
			goto exit;
		}
	}
	
	*outDict = dict;
	dict = NULL;
	err = kNoErr;

exit:
	if( dict ) CFRelease( dict );
	return( err );
}

//===========================================================================================================================
//	CFDictionaryCreateWithINIBytes
//===========================================================================================================================

CFMutableDictionaryRef
	CFDictionaryCreateWithINIBytes( 
		const void *	inPtr, 
		size_t			inLen, 
		uint32_t		inFlags, 
		CFStringRef		inSectionNameKey, 
		OSStatus *		outErr )
{
	const char *				src				= (const char *) inPtr;
	const char * const			end				= src + inLen;
	CFMutableDictionaryRef		result			= NULL;
	CFMutableDictionaryRef		rootDict		= NULL;
	CFMutableDictionaryRef		sectionDict		= NULL;
	OSStatus					err;
	uint32_t					flags;
	const char *				namePtr;
	size_t						nameLen;
	const char *				valuePtr;
	size_t						valueLen;
	CFStringRef					cfstr;
	CFMutableArrayRef			sectionArray;
	
	rootDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( rootDict, exit, err = kNoMemoryErr );
	
	while( INIGetNext( src, end, &flags, &namePtr, &nameLen, &valuePtr, &valueLen, &src ) )
	{
		if( flags & kINIFlag_Section )
		{
			// Set up a new section dictionary. If there's a value (e.g. value in [name "value"]), use it as the name).
			
			if( sectionDict ) CFRelease( sectionDict );
			sectionDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require_action( sectionDict, exit, err = kNoMemoryErr );
			if( valuePtr && inSectionNameKey ) CFDictionarySetCString( sectionDict, inSectionNameKey, valuePtr, valueLen );
			
			// Sections dictionaries are grouped into arrays and key'd by the name (e.g. name in [name "value"]).
			
			cfstr = CFStringCreateWithBytes( NULL, (const uint8_t *) namePtr, (CFIndex) nameLen, kCFStringEncodingUTF8, false );
			require_action( cfstr, exit, err = kUnknownErr );
			sectionArray = (CFMutableArrayRef) CFDictionaryGetValue( rootDict, cfstr );
			if( sectionArray )
			{
				CFArrayAppendValue( sectionArray, sectionDict );
			}
			else
			{
				sectionArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
				require_action( sectionArray, exit, err = kNoMemoryErr; CFRelease( cfstr ) );
				CFArrayAppendValue( sectionArray, sectionDict );
				CFDictionarySetValue( rootDict, cfstr, sectionArray );
				CFRelease( sectionArray );
			}
			CFRelease( cfstr );
			continue;
		}
		
		// If there's no section dictionary yet, we're processing global properties so set up a virtual section for them.
		
		if( !sectionDict && !( inFlags & kINIFlag_MergeGlobals ) )
		{
			sectionDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require_action( sectionDict, exit, err = kNoMemoryErr );
			CFDictionarySetValue( rootDict, CFSTR( kINISectionType_Global ), sectionDict );
		}
		
		cfstr = CFStringCreateWithBytes( NULL, (const uint8_t *) namePtr, (CFIndex) nameLen, kCFStringEncodingUTF8, false );
		require_action( cfstr, exit, err = kUnknownErr );
		CFDictionarySetCString( sectionDict ? sectionDict : rootDict, cfstr, valuePtr, valueLen );
		CFRelease( cfstr );
	}
	
	result = rootDict;
	rootDict = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( sectionDict );
	CFReleaseNullSafe( rootDict );
	if( outErr ) *outErr = err;
	return( result );
}

//===========================================================================================================================
//	CFDictionaryMergeDictionary
//===========================================================================================================================

static void	_CFDictionaryMergeDictionaryApplier( const void *inKey, const void *inValue, void *inContext );

OSStatus	CFDictionaryMergeDictionary( CFMutableDictionaryRef inDestinationDict, CFDictionaryRef inSourceDict )
{
	CFDictionaryApplyFunction( inSourceDict, _CFDictionaryMergeDictionaryApplier, inDestinationDict );
	return( kNoErr );
}

static void	_CFDictionaryMergeDictionaryApplier( const void *inKey, const void *inValue, void *inContext )
{
	CFDictionarySetValue( (CFMutableDictionaryRef) inContext, inKey, inValue );
}

//===========================================================================================================================
//	CFDictionaryCopyCString
//===========================================================================================================================

char *	CFDictionaryCopyCString( CFDictionaryRef inDict, const void *inKey, OSStatus *outErr )
{
	OSStatus		err;
	CFTypeRef		tempObj;
	char *			ptr;
	
	tempObj = CFDictionaryGetValue( inDict, inKey );
	if( tempObj )
	{
		ptr = CFCopyCString( tempObj, &err );
	}
	else
	{
		ptr = NULL;
		err = kNotFoundErr;
	}
	if( outErr ) *outErr = err;
	return( ptr );
}

//===========================================================================================================================
//	CFDictionaryGetCString
//===========================================================================================================================

char *	CFDictionaryGetCString( CFDictionaryRef inDict, const void *inKey, char *inBuf, size_t inMaxLen, OSStatus *outErr )
{
	OSStatus		err;
	CFTypeRef		tempObj;
	char *			ptr;
	
	tempObj = CFDictionaryGetValue( inDict, inKey );
	if( tempObj )
	{
		ptr = CFGetCString( tempObj, inBuf, inMaxLen );
		err = kNoErr;
	}
	else
	{
		if( inMaxLen > 0 )
		{
			*inBuf = '\0';
			ptr = inBuf;
		}
		else
		{
			ptr = "";
		}
		err = kNotFoundErr;
	}
	if( outErr ) *outErr = err;
	return( ptr );
}

//===========================================================================================================================
//	CFDictionarySetCString
//===========================================================================================================================

OSStatus	CFDictionarySetCString( CFMutableDictionaryRef inDict, const void *inKey, const void *inStr, size_t inLen )
{
	OSStatus		err;
	CFStringRef		tempCFStr;
	
	if( !inStr ) inStr = "";
	if( inLen == kSizeCString )
	{
		tempCFStr = CFStringCreateWithCString( NULL, (const char *) inStr, kCFStringEncodingUTF8 );
		require_action( tempCFStr, exit, err = kUnknownErr );
	}
	else
	{
		tempCFStr = CFStringCreateWithBytes( NULL, (const uint8_t *) inStr, (CFIndex) inLen, kCFStringEncodingUTF8, false );
		require_action( tempCFStr, exit, err = kUnknownErr );
	}
	
	CFDictionarySetValue( inDict, inKey, tempCFStr );
	CFRelease( tempCFStr );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFDictionaryCopyCFData
//===========================================================================================================================

CFDataRef
	CFDictionaryCopyCFData( 
		CFDictionaryRef	inDict, 
		const void *	inKey, 
		size_t *		outLen, 
		OSStatus *		outErr )
{
	CFDataRef		data;
	CFTypeRef		obj;
	
	obj = CFDictionaryGetValue( inDict, inKey );
	if( obj )
	{
		data = CFCopyCFData( obj, outLen, outErr );
	}
	else
	{
		data = NULL;
		if( outLen ) *outLen = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( data );
}

//===========================================================================================================================
//	CFDictionaryCopyData
//===========================================================================================================================

uint8_t *
	CFDictionaryCopyData( 
		CFDictionaryRef	inDict, 
		const void *	inKey, 
		size_t *		outLen, 
		OSStatus *		outErr )
{
	uint8_t *		ptr;
	CFTypeRef		obj;
	
	obj = CFDictionaryGetValue( inDict, inKey );
	if( obj )
	{
		ptr = CFCopyData( obj, outLen, outErr );
	}
	else
	{
		ptr = NULL;
		if( outLen ) *outLen = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( ptr );
}

//===========================================================================================================================
//	CFDictionaryGetData
//===========================================================================================================================

uint8_t *
	CFDictionaryGetData( 
		CFDictionaryRef	inDict, 
		const void *	inKey, 
		void *			inBuf, 
		size_t			inMaxLen, 
		size_t *		outLen, 
		OSStatus *		outErr )
{
	uint8_t *		ptr;
	CFTypeRef		obj;
	
	obj = CFDictionaryGetValue( inDict, inKey );
	if( obj )
	{
		ptr = CFGetData( obj, inBuf, inMaxLen, outLen, outErr );
	}
	else
	{
		ptr = (uint8_t *) inBuf;
		if( outLen ) *outLen = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( ptr );
}

//===========================================================================================================================
//	CFDictionarySetData
//===========================================================================================================================

OSStatus	CFDictionarySetData( CFMutableDictionaryRef inDict, const void *inKey, const void *inData, size_t inLen )
{
	OSStatus		err;
	CFDataRef		data;
	
	data = CFDataCreate( NULL, (const uint8_t *) inData, (CFIndex) inLen );
	require_action( data, exit, err = kUnknownErr );
	
	CFDictionarySetValue( inDict, inKey, data );
	CFRelease( data );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFDictionaryGetDouble
//===========================================================================================================================

double	CFDictionaryGetDouble( CFDictionaryRef inDict, const void *inKey, OSStatus *outErr )
{
	double			x;
	CFTypeRef		value;
	
	value = CFDictionaryGetValue( inDict, inKey );
	if( value )
	{
		x = CFGetDouble( value, outErr );
	}
	else
	{
		x = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( x );
}

//===========================================================================================================================
//	CFDictionarySetDouble
//===========================================================================================================================

OSStatus	CFDictionarySetDouble( CFMutableDictionaryRef inDict, const void *inKey, double x )
{
	OSStatus		err;
	CFNumberRef		tempNum;
	
	tempNum = CFNumberCreate( NULL, kCFNumberDoubleType, &x );
	require_action( tempNum, exit, err = kNoMemoryErr );
	
	CFDictionarySetValue( inDict, inKey, tempNum );
	CFRelease( tempNum );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFDictionaryGetHardwareAddress
//===========================================================================================================================

uint64_t
	CFDictionaryGetHardwareAddress( 
		CFDictionaryRef inDict, 
		const void *	inKey, 
		uint8_t *		inBuf, 
		size_t			inLen, 
		OSStatus *		outErr )
{
	uint64_t		x;
	CFTypeRef		value;
	
	value = CFDictionaryGetValue( inDict, inKey );
	if( value )
	{
		x = CFGetHardwareAddress( value, inBuf, inLen, outErr );
	}
	else
	{
		x = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( x );
}

//===========================================================================================================================
//	CFDictionaryGetInt64
//===========================================================================================================================

int64_t	CFDictionaryGetInt64( CFDictionaryRef inDict, const void *inKey, OSStatus *outErr )
{
	int64_t			x;
	CFTypeRef		value;
	
	value = CFDictionaryGetValue( inDict, inKey );
	if( value )
	{
		x = CFGetInt64( value, outErr );
	}
	else
	{
		x = 0;
		if( outErr ) *outErr = kNotFoundErr;
	}
	return( x );
}

//===========================================================================================================================
//	CFDictionarySetInt64
//===========================================================================================================================

OSStatus	CFDictionarySetInt64( CFMutableDictionaryRef inDict, const void *inKey, int64_t x )
{
	OSStatus		err;
	CFNumberRef		tempNum;
	
	tempNum = CFNumberCreateInt64( x );
	require_action( tempNum, exit, err = kNoMemoryErr );
	
	CFDictionarySetValue( inDict, inKey, tempNum );
	CFRelease( tempNum );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFDictionarySetNumber
//===========================================================================================================================

OSStatus	CFDictionarySetNumber( CFMutableDictionaryRef inDict, const void *inKey, CFNumberType inType, void *inValue )
{
	OSStatus		err;
	CFNumberRef		tempNum;
	
	tempNum = CFNumberCreate( NULL, inType, inValue );
	require_action( tempNum, exit, err = kUnknownErr );
	
	CFDictionarySetValue( inDict, inKey, tempNum );
	CFRelease( tempNum );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFDictionaryGetTypedValue
//===========================================================================================================================

CFTypeRef	CFDictionaryGetTypedValue( CFDictionaryRef inDict, const void *inKey, CFTypeID inType, OSStatus *outErr )
{
	CFTypeRef		obj = NULL;
	CFTypeRef		tempObj;
	OSStatus		err;
	
	tempObj = CFDictionaryGetValue( inDict, inKey );
	require_action_quiet( tempObj, exit, err = kNotFoundErr );
	require_action_quiet( CFGetTypeID( tempObj ) == inType, exit, err = kTypeErr );
	
	obj = tempObj;
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( obj );
}

//===========================================================================================================================
//	CFDictionaryGetUUID
//===========================================================================================================================

OSStatus	CFDictionaryGetUUID( CFDictionaryRef inDict, const void *inKey, uint8_t outUUID[ 16 ] )
{
	OSStatus		err;
	CFTypeRef		value;
	
	value = CFDictionaryGetValue( inDict, inKey );
	require_action_quiet( value, exit, err = kNotFoundErr );
	
	err = CFGetUUID( value, outUUID );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFStringCreateComponentsSeparatedByString
//===========================================================================================================================

CFArrayRef	CFStringCreateComponentsSeparatedByString( CFStringRef inString, CFStringRef inSeparator )
{
	CFMutableArrayRef		array;
	OSStatus				err;
	const char *			sourceStr;
	char *					sourceStorage;
	const char *			separatorStr;
	char *					separatorStorage;
	size_t					separatorLen;
	const char *			ptr;
	CFStringRef				componentStr;
	
	sourceStorage    = NULL;
	separatorStorage = NULL;
	
	array = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require( array, exit );
	
	err = CFStringGetOrCopyCStringUTF8( inString, &sourceStr, &sourceStorage );
	require_noerr( err, exit );
	
	err = CFStringGetOrCopyCStringUTF8( inSeparator, &separatorStr, &separatorStorage );
	require_noerr( err, exit );
	separatorLen = strlen( separatorStr );
	
	for( ;; )
	{
		ptr = strstr( sourceStr, separatorStr );
		if( ptr )
		{
			componentStr = CFStringCreateWithBytes( NULL, (const uint8_t *) sourceStr, (CFIndex)( ptr - sourceStr ), 
				kCFStringEncodingUTF8, false );
			require( componentStr, exit );
			
			sourceStr = ptr + separatorLen;
		}
		else
		{
			componentStr = CFStringCreateWithCString( NULL, sourceStr, kCFStringEncodingUTF8 );
			require( componentStr, exit );
		}
		
		CFArrayAppendValue( array, componentStr );
		CFRelease( componentStr );
		if( !ptr ) break;
	}
	
exit:
	if( sourceStorage )		free( sourceStorage );
	if( separatorStorage )	free( separatorStorage );
	return( array );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

