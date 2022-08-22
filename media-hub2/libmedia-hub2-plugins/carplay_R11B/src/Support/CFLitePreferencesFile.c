/*
	File:    	CFLitePreferencesFile.c
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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "MiscUtils.h"
#include "ThreadUtils.h"

#include CF_HEADER

//===========================================================================================================================
//	Internals
//===========================================================================================================================

static pthread_mutex_t				gLock  = PTHREAD_MUTEX_INITIALIZER;
static CFMutableDictionaryRef		gPrefs = NULL;

static void						_CFPreferencesCopyKeyListApplier( const void *inKey, const void *inValue, void *inContext );
static CFMutableDictionaryRef	_CopyDictionaryFromFile( CFStringRef inAppID );
static OSStatus					_WritePlistToFile( CFStringRef inAppID, CFPropertyListRef inPlist );

//===========================================================================================================================
//	CFPreferencesCopyKeyList_compat
//===========================================================================================================================

CFArrayRef	CFPreferencesCopyKeyList_compat( CFStringRef inAppID, CFStringRef inUser, CFStringRef inHost )
{
	CFArrayRef					result		= NULL;
	CFDictionaryRef				appDict;
	CFMutableDictionaryRef		appDictCopy = NULL;
	CFMutableArrayRef			keys		= NULL;
	
	(void) inUser;
	(void) inHost;
	
	pthread_mutex_lock( &gLock );
	
	if( !gPrefs )
	{
		gPrefs = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require( gPrefs, exit );
	}
	
	appDict = (CFDictionaryRef) CFDictionaryGetValue( gPrefs, inAppID );
	if( !appDict )
	{
		appDictCopy = _CopyDictionaryFromFile( inAppID );
		if( !appDictCopy )
		{
			appDictCopy = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require( appDictCopy, exit );
		}
		CFDictionarySetValue( gPrefs, inAppID, appDictCopy );
		appDict = appDictCopy;
	}
	
	keys = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require( keys, exit );
	
	CFDictionaryApplyFunction( appDict, _CFPreferencesCopyKeyListApplier, keys );
	result = keys;
	keys = NULL;
	
exit:
	CFReleaseNullSafe( keys );
	CFReleaseNullSafe( appDictCopy );
	pthread_mutex_unlock( &gLock );
	return( result );
}

//===========================================================================================================================
//	_CFPreferencesCopyKeyListApplier
//===========================================================================================================================

static void	_CFPreferencesCopyKeyListApplier( const void *inKey, const void *inValue, void *inContext )
{
	CFMutableArrayRef const		keys = (CFMutableArrayRef) inContext;
	
	(void) inValue;
	
	CFArrayAppendValue( keys, inKey );
}

//===========================================================================================================================
//	CFPreferencesCopyAppValue_compat
//===========================================================================================================================

CFPropertyListRef	CFPreferencesCopyAppValue_compat( CFStringRef inKey, CFStringRef inAppID )
{
	CFPropertyListRef			value = NULL;
	CFDictionaryRef				appDict;
	CFMutableDictionaryRef		appDictCopy = NULL;
	
	pthread_mutex_lock( &gLock );
	
	if( !gPrefs )
	{
		gPrefs = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require( gPrefs, exit );
	}
	
	appDict = (CFDictionaryRef) CFDictionaryGetValue( gPrefs, inAppID );
	if( !appDict )
	{
		appDictCopy = _CopyDictionaryFromFile( inAppID );
		if( !appDictCopy )
		{
			appDictCopy = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require( appDictCopy, exit );
		}
		CFDictionarySetValue( gPrefs, inAppID, appDictCopy );
		appDict = appDictCopy;
	}
	
	value = CFDictionaryGetValue( appDict, inKey );
	CFRetainNullSafe( value );
	
exit:
	CFReleaseNullSafe( appDictCopy );
	pthread_mutex_unlock( &gLock );
	return( value );
}

//===========================================================================================================================
//	CFPreferencesSetAppValue_compat
//===========================================================================================================================

void	CFPreferencesSetAppValue_compat( CFStringRef inKey, CFPropertyListRef inValue, CFStringRef inAppID )
{
	CFMutableDictionaryRef		appDict;
	CFMutableDictionaryRef		appDictCopy = NULL;
	
	pthread_mutex_lock( &gLock );
	
	if( !gPrefs )
	{
		gPrefs = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require( gPrefs, exit );
	}
	
	appDict = (CFMutableDictionaryRef) CFDictionaryGetValue( gPrefs, inAppID );
	if( !appDict )
	{
		appDictCopy = _CopyDictionaryFromFile( inAppID );
		if( !appDictCopy )
		{
			appDictCopy = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			require( appDictCopy, exit );
		}
		CFDictionarySetValue( gPrefs, inAppID, appDictCopy );
		appDict = appDictCopy;
	}
	
	if( inValue )	CFDictionarySetValue( appDict, inKey, inValue );
	else			CFDictionaryRemoveValue( appDict, inKey );
	_WritePlistToFile( inAppID, appDict );
	
exit:
	CFReleaseNullSafe( appDictCopy );
	pthread_mutex_unlock( &gLock );
}

//===========================================================================================================================
//	CFPreferencesAppSynchronize_compat
//===========================================================================================================================

Boolean	CFPreferencesAppSynchronize_compat( CFStringRef inAppID )
{
	// Remove the app dictionary (if it exists) to cause it to be re-read on the next get.
	
	if( gPrefs ) CFDictionaryRemoveValue( gPrefs, inAppID );
	return( true );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_CopyDictionaryFromFile
//===========================================================================================================================

static CFMutableDictionaryRef	_CopyDictionaryFromFile( CFStringRef inAppID )
{
	CFMutableDictionaryRef		dict = NULL;
	OSStatus					err;
	char						homePath[ PATH_MAX ];
	char						path[ PATH_MAX ];
	FILE *						file = NULL;
	CFMutableDataRef			data = NULL;
	uint8_t *					buf  = NULL;
	size_t						len, n;
	
	// Read from "~/Library/Preferences/<app ID>.plist".
	
	*homePath = '\0';
	GetHomePath( homePath, sizeof( homePath ) );
	*path = '\0';
	SNPrintF( path, sizeof( path ), "%s/Library/Preferences/%@.plist", homePath, inAppID );
	
	file = fopen( path, "rb" );
	err = map_global_value_errno( file, file );
	require_noerr_quiet( err, exit );
	
	data = CFDataCreateMutable( NULL, 0 );
	require_action( data, exit, err = kNoMemoryErr );
	
	len = 32 * 1024;
	buf = (uint8_t *) malloc( len );
	require( buf, exit );
	
	for( ;; )
	{
		n = fread( buf, 1, len, file );
		if( n == 0 ) break;
		CFDataAppendBytes( data, buf, (CFIndex) n );
	}
	
	dict = (CFMutableDictionaryRef) CFPropertyListCreateWithData( NULL, data, kCFPropertyListMutableContainers, NULL, NULL );
	if( dict && !CFIsType( dict, CFDictionary ) )
	{
		dlogassert( "Prefs must be a dictionary: %@", dict );
		CFRelease( dict );
		dict = NULL;
	}
	require_quiet( dict, exit );
	
exit:
	if( buf ) free( buf );
	CFReleaseNullSafe( data );
	if( file ) fclose( file );
	return( dict );
}

//===========================================================================================================================
//	_WritePlistToFile
//===========================================================================================================================

static OSStatus	_WritePlistToFile( CFStringRef inAppID, CFPropertyListRef inPlist )
{
	OSStatus			err;
	char				homePath[ PATH_MAX ];
	char				path[ PATH_MAX ];
	CFDataRef			data = NULL;
	int					fd = -1;
	const uint8_t *		ptr;
	const uint8_t *		end;
	ssize_t				n;
	
	// Create the ~/Library/Preferences parent folder if it doesn't already exist.
	
	*homePath = '\0';
	GetHomePath( homePath, sizeof( homePath ) );
	*path = '\0';
	SNPrintF( path, sizeof( path ), "%s/Library/Preferences", homePath );
	
	err = mkpath( path, S_IRWXU, S_IRWXU );
	err = map_global_noerr_errno( err );
	if( err && ( err != EEXIST ) ) dlogassert( "Make parent %s failed: %#m", path, err );
	
	// Write the plist to "~/Library/Preferences/<app ID>.plist".
	
	data = CFPropertyListCreateData( NULL, inPlist, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
	require_action( data, exit, err = kUnknownErr );
	
	*path = '\0';
	SNPrintF( path, sizeof( path ), "%s/Library/Preferences/%@.plist", homePath, inAppID );
	fd = open( path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR );
	err = map_fd_creation_errno( fd );
	require_noerr( err, exit );
	
	ptr = CFDataGetBytePtr( data );
	end = ptr + CFDataGetLength( data );
	for( ; ptr != end; ptr += n )
	{
		n = write( fd, ptr, (size_t)( end - ptr ) );
		err = map_global_value_errno( n > 0, n );
		require_noerr( err, exit );
	}
	
exit:
	if( fd >= 0 ) close( fd );
	CFReleaseNullSafe( data );
	return( err );
}

#if 0
#pragma mark -
#endif

