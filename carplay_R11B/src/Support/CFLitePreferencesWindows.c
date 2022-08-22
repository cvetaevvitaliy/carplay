/*
	File:    	CFLitePreferencesWindows.c
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
*/

#include "CFLitePreferencesWindows.h"

#include <stdlib.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "CFUtils.h"
#include "StringUtils.h"
#include "utfconv.h"

//===========================================================================================================================
//	Constants
//===========================================================================================================================

const CFStringRef		kCFPreferencesCurrentApplication = CFSTR( "CurrentApplication" );

#define	kCFPreferencesRegistryKeyPath		"Software\\Apple Inc.\\Preferences\\"

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static OSStatus	CFPreferencesSetupAppIDRegistryKey( CFStringRef inAppID, Boolean inAllowCreate, HKEY *outKey );
static OSStatus	CFStringCreateWithCStringUTF16( const void *inUTF16, CFStringRef *outStr );
static OSStatus	CFStringGetOrCopyCStringUTF16( CFStringRef inString, const uint16_t **outUTF8, uint16_t **outStorage );

//===========================================================================================================================
//	CFPreferencesCopyAppValue_compat
//===========================================================================================================================

CFPropertyListRef	CFPreferencesCopyAppValue_compat( CFStringRef inKey, CFStringRef inAppID )
{
	CFPropertyListRef		obj;
	OSStatus				err;
	HKEY					regKey;
	const WCHAR *			name;
	WCHAR *					nameStorage;
	DWORD					type;
	DWORD					size;
	void *					data;
	
	obj			= NULL;
	regKey		= NULL;
	nameStorage	= NULL;
	data		= NULL;
	
	err = CFPreferencesSetupAppIDRegistryKey( inAppID, false, &regKey );
	require_noerr_quiet( err, exit );
		
	err = CFStringGetOrCopyCStringUTF16( inKey, &name, &nameStorage );
	require_noerr( err, exit );
	
	size = 0;
	err = RegQueryValueExW( regKey, name, NULL, &type, NULL, &size );
	check( ( err == ERROR_SUCCESS ) || ( err == ERROR_FILE_NOT_FOUND ) );
	require_noerr_quiet( err, exit );
	require_action( size > 0, exit, err = kSizeErr );
	
	if( ( type == REG_BINARY ) && ( size == 1 ) ) // Boolean
	{
		uint8_t		b;
		
		err = RegQueryValueExW( regKey, name, NULL, &type, &b, &size );
		require_noerr( err, exit );
		
		obj = b ? kCFBooleanTrue : kCFBooleanFalse;
		CFRetain( obj );
	}
	else if( type == REG_DWORD ) // Number
	{
		int32_t		v;
		
		err = RegQueryValueExW( regKey, name, NULL, &type, (LPBYTE) &v, &size );
		require_noerr( err, exit );
		
		obj = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &v );
		require_action( obj, exit, err = kNoMemoryErr );
	}
	else
	{
		data = malloc( size );
		require_action( data, exit, err = kNoMemoryErr );
		
		err = RegQueryValueExW( regKey, name, NULL, &type, (LPBYTE) data, &size );
		require_noerr( err, exit );
		
		if( type == REG_SZ ) // String
		{
			CFStringRef		str;
			
			err = CFStringCreateWithCStringUTF16( data, &str );
			require_noerr( err, exit );
			
			obj = str;
		}
		else
		{
			CFDataRef		dataObj;

			dataObj = CFDataCreate( NULL, data, (CFIndex) size );
			require_action( dataObj, exit, err = kNoMemoryErr );

			obj = CFPropertyListCreateWithData( kCFAllocatorDefault, data, size, NULL, NULL );
			if( obj )	CFRelease( dataObj );
			else		obj = dataObj; // If it's not a plist, return the raw data.
		}
	}
	
exit:
	if( data )			free( data );
	if( nameStorage )	free( nameStorage );
	if( regKey )		RegCloseKey( regKey );
	return( obj );
}

//===========================================================================================================================
//	CFPreferencesSetAppValue_compat
//===========================================================================================================================

void	CFPreferencesSetAppValue_compat( CFStringRef inKey, CFPropertyListRef inValue, CFStringRef inAppID )
{
	OSStatus			err;
	HKEY				regKey;
	const WCHAR *		name;
	WCHAR *				nameStorage;
	CFTypeID			typeID;
	Boolean				good;
	
	regKey		= NULL;
	nameStorage	= NULL;
	
	err = CFPreferencesSetupAppIDRegistryKey( inAppID, true, &regKey );
	require_noerr( err, exit );
	
	err = CFStringGetOrCopyCStringUTF16( inKey, &name, &nameStorage );
	require_noerr( err, exit );
	
	typeID = CFGetTypeID( inValue );
	if( typeID == CFBooleanGetTypeID() )
	{
		uint8_t		b;
		
		b = (uint8_t) CFBooleanGetValue( (CFBooleanRef) inValue );
		
		err = RegSetValueExW( regKey, name, 0, REG_BINARY, &b, 1 );
		require_noerr( err, exit );
	}
	else if( typeID == CFNumberGetTypeID() )
	{
		int32_t		v;
		
		good = CFNumberGetValue( (CFNumberRef) inValue, kCFNumberSInt32Type, &v );
		require_action( good, exit, err = kTypeErr );
		
		err = RegSetValueExW( regKey, name, 0, REG_DWORD, (LPBYTE) &v, sizeof( v ) );
		require_noerr( err, exit );
	}
	else if( typeID == CFStringGetTypeID() )
	{
		const WCHAR *		utf16;
		WCHAR *				utf16Storage;
	
		err = CFStringGetOrCopyCStringUTF16( (CFStringRef) inValue, &utf16, &utf16Storage );
		require_noerr( err, exit );
		
		err = RegSetValueExW( regKey, name, 0, REG_SZ, (LPBYTE) utf16, (DWORD)( ( UTF16strlen( utf16 ) + 1 ) * 2 ) );
		if( utf16Storage ) free( utf16Storage );
		require_noerr( err, exit );
	}
	else
	{
		CFDataRef		data;
		
		data = CFPropertyListCreateData( NULL, inValue, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
		require_action( data, exit, err = kUnknownErr );
		
		err = RegSetValueExW( regKey, name, 0, REG_BINARY, CFDataGetBytePtr( data ), (DWORD) CFDataGetLength( data ) );
		CFRelease( data );
		require_noerr( err, exit );
	}
	
exit:
	if( nameStorage )	free( nameStorage );
	if( regKey )		RegCloseKey( regKey );
}

//===========================================================================================================================
//	CFPreferencesAppSynchronize_compat
//===========================================================================================================================

Boolean	CFPreferencesAppSynchronize_compat( CFStringRef inAppID )
{
	(void) inAppID; // Unused
	
	// This implementation doesn't cache anything so it's always synchronized.
	
	return( true );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CFPreferencesSetupAppIDRegistryKey
//===========================================================================================================================

static OSStatus	CFPreferencesSetupAppIDRegistryKey( CFStringRef inAppID, Boolean inAllowCreate, HKEY *outKey )
{
	OSStatus				err;
	const char *			appIDUTF8;
	char *					appIDUTF8Storage;
	int						totalSize;
	char *					registryPathUTF8;
	WCHAR *					registryPathUTF16;
	HKEY					registryKey;
	
	appIDUTF8Storage	= NULL;
	registryPathUTF8	= NULL;
	registryKey			= NULL;
	
	err = CFStringGetOrCopyCStringUTF8( inAppID, &appIDUTF8, &appIDUTF8Storage );
	require_noerr( err, exit );
	
	totalSize = ASPrintF( &registryPathUTF8, "%s%s", kCFPreferencesRegistryKeyPath, appIDUTF8 );
	require_action( totalSize > 0, exit, err = kNoMemoryErr );
	
	err = utf8_decodestr_copy( registryPathUTF8, (size_t) totalSize, &registryPathUTF16, NULL, '/', 0 );
	require_noerr( err, exit );
	
	if( inAllowCreate )
	{
		err = RegCreateKeyExW( HKEY_CURRENT_USER, registryPathUTF16, 0, L"", 0, KEY_ALL_ACCESS, NULL, &registryKey, NULL );
		utffree( registryPathUTF16 );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = RegOpenKeyExW( HKEY_CURRENT_USER, registryPathUTF16, 0, KEY_READ, &registryKey );
		utffree( registryPathUTF16 );
		require_noerr_quiet( err, exit );
	}
	
	*outKey = registryKey;
	registryKey = NULL;
	
exit:
	if( registryKey )		RegCloseKey( registryKey );
	if( appIDUTF8Storage )	free( appIDUTF8Storage );
	if( registryPathUTF8 )	free( registryPathUTF8 );
	return( err );
}

//===========================================================================================================================
//	CFStringCreateWithCStringUTF16
//===========================================================================================================================

static OSStatus	CFStringCreateWithCStringUTF16( const void *inUTF16, CFStringRef *outStr )
{
	OSStatus				err;
	const uint16_t *		utf16Ptr;
	size_t					utf16Len;
	char *					utf8;
	CFStringRef				str;
	
	utf16Ptr = (const uint16_t *) inUTF16;
	utf16Len = UTF16strlen( utf16Ptr ) * 2;
	err = utf8_encodestr_copy( utf16Ptr, utf16Len, &utf8, NULL, '/', 0 );
	require_noerr( err, exit );
	
	str = CFStringCreateWithCString( kCFAllocatorDefault, utf8, kCFStringEncodingUTF8 );
	free( utf8 );
	require_action( str, exit, err = kNoMemoryErr );
	
	*outStr = str;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CFStringGetOrCopyCStringUTF16
//===========================================================================================================================

static OSStatus	CFStringGetOrCopyCStringUTF16( CFStringRef inString, const uint16_t **outUTF16, uint16_t **outUTF16Storage )
{
	OSStatus			err;
	const char *		utf8;
	char *				utf8Storage;
	uint16_t *			utf16;
	
	err = CFStringGetOrCopyCStringUTF8( inString, &utf8, &utf8Storage );
	require_noerr( err, exit );
	
	err = utf8_decodestr_copy( utf8, strlen( utf8 ), &utf16, NULL, '/', 0 );
	if( utf8Storage ) free( utf8Storage );
	require_noerr( err, exit );
	
	*outUTF16			= utf16;
	*outUTF16Storage	= utf16;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

