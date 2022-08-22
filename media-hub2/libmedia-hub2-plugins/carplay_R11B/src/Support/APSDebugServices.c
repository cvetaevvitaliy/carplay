/*
	File:    	APSDebugServices.c
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
	
	Copyright (C) 1997-2014 Apple Inc. All Rights Reserved.
*/

#if 0
#pragma mark == Includes ==
#endif

//===========================================================================================================================
//	Includes
//===========================================================================================================================

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

// PREFIX_HEADER must be included before any other file for some compilers. Map to a safe file if not defined.

#undef   PREFIX_HEADER
#define  PREFIX_HEADER "APSCommonServices.h"
#include PREFIX_HEADER

#include "APSCommonServices.h"	// Must be included early for TARGET_*, etc. definitions.
#include "APSDebugServices.h"	// Must be included early for DEBUG_*, etc. definitions.
#include "CFUtils.h"
#include "MiscUtils.h"
#include "TickUtils.h"

#if( TARGET_HAS_STD_C_LIB )
	#include <ctype.h>
	#include <errno.h>
	#include <limits.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <time.h>
#endif

#if( TARGET_OS_LINUX )
	#include <execinfo.h>
#endif

#if( TARGET_OS_POSIX )
	#include <sys/types.h>
	
	#include <fcntl.h>
	#include <net/if.h>
	#if( TARGET_OS_BSD )
		#include <net/if_dl.h>
	#endif
	#include <pthread.h>
	#include <syslog.h>
	#include <linux/sysctl.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#if( !TARGET_OS_QNX )
		#include <sys/user.h>
	#endif
	#include <unistd.h>
#endif

#if( TARGET_OS_QNX )
	#include <backtrace.h>
	#include <devctl.h>
	#include <sys/procfs.h>
#endif

#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
	#include <direct.h>
	#include <fcntl.h>
	#include <io.h>
	
	#include <Dbghelp.h>
#endif

#if( DEBUG_CF_OBJECTS_ENABLED )
	#include CF_HEADER
#endif

// Disable RealView "warning #111-D: statement is unreachable" for debug code.
// Disable RealView "warning #236-D: controlling expression is constant" for debug code.

#if( __ARMCC_VERSION )
	#pragma diag_suppress 111,236
#endif

#if 0
#pragma mark == Prototypes ==
#endif

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

// Utilities

#if( DEBUG && TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
	static void	DebugWinEnableConsole( void );
#endif

static int	__strnicmpx( const void *inS1, size_t inN, const char *inS2 );
#if( TARGET_OS_WINDOWS )
	static TCHAR *
		DebugWinCharToTCharString( 
			const char *	inCharString, 
			size_t 			inCharCount, 
			TCHAR *			outTCharString, 
			size_t 			inTCharCountMax, 
			size_t *		outTCharCount );
#endif

#if 0
#pragma mark == Globals ==
#endif

//===========================================================================================================================
//	Globals
//===========================================================================================================================

ulog_define( DebugServicesAssert,		kLogLevelAll,					kLogFlags_PrintTime, "", NULL );
ulog_define( DebugServicesBreak,		kDebugDefaultBreakLevel,		kLogFlags_PrintTime, "", NULL );
ulog_define( DebugServicesLogging,		kLogLevelInfo + 1,				kLogFlags_PrintTime, "", NULL );
ulog_define( DebugServicesStackTrace,	kDebugDefaultStackTraceLevel,	kLogFlags_PrintTime, "", NULL );

#if( DEBUG_C99_VA_ARGS )
	#define dbs_ulog( LEVEL, ... )			ulog( &log_category_from_name( DebugServicesLogging ), (LEVEL), __VA_ARGS__ )
#elif( DEBUG_GNU_VA_ARGS )
	#define dbs_ulog( LEVEL, ARGS... )		ulog( &log_category_from_name( DebugServicesLogging ), (LEVEL), ## ARGS )
#else
	#define	dbs_ulog						DebugLog_C89 // No VA_ARG macros so we have to do it from a real function.
#endif

#if 0
#pragma mark -
#pragma mark == Compatibility ==
#endif

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	getprogname
//===========================================================================================================================

const char *	getprogname( void )
{
	static Boolean					sInitialized;
	static CRITICAL_SECTION			sProgramNameLock;
	static LONG						sProgramNameLockState = 0;
	static char						sProgramName[ 64 ];
	
	InitializeCriticalSectionOnce( &sProgramNameLock, &sProgramNameLockState );	
	EnterCriticalSection( &sProgramNameLock );
	if( !sInitialized )
	{
		TCHAR		path[ PATH_MAX + 1 ];
		TCHAR *		name;
		TCHAR *		extension;
		TCHAR		c;
		char *		dst;
		char *		lim;
		
		path[ 0 ] = '?';
		GetModuleFileName( NULL, path, (DWORD) countof( path ) );
		path[ countof( path ) - 1 ] = '\0';
		
		name = _tcsrchr( path, '\\' );
		if( name )	name += 1;
		else		name = path;
		
		extension = _tcsrchr( name, '.' );
		if( extension ) *extension = '\0';

		dst = sProgramName;
		lim = dst + ( countof( sProgramName ) - 1 );
		while( ( ( c = *name++ ) != '\0' ) && ( dst < lim ) )
		{
			if( ( c < 32 ) || ( c > 126 ) ) continue;
			*dst++ = (char) c;
		}
		*dst = '\0';
		
		sInitialized = true;
	}
	LeaveCriticalSection( &sProgramNameLock );
	return( sProgramName );
}
#endif

//===========================================================================================================================
//	__strnicmpx
//
//	Like the ANSI C strncmp routine, but case-insensitive and requires all characters in s1 match all characters in s2.
//===========================================================================================================================

static int	__strnicmpx( const void *inS1, size_t inN, const char *inS2 )
{
	const unsigned char *		s1;
	const unsigned char *		s2;
	int							c1;
	int							c2;

	s1 = (const unsigned char *) inS1;
	s2 = (const unsigned char *) inS2;
	while( inN-- > 0 )
	{
		c1 = tolower( *s1 );
		c2 = tolower( *s2 );
		if( c1 < c2 ) return( -1 );
		if( c1 > c2 ) return(  1 );
		if( c2 == 0 ) return(  0 );
		
		++s1;
		++s2;
	}
	if( *s2 != 0 ) return( -1 );
	return( 0 );
}

#if 0
#pragma mark -
#pragma mark == Debug ==
#endif

//===========================================================================================================================
//	DebugLog_C89
//===========================================================================================================================

#if( !DEBUG_HAS_VA_ARG_MACROS )
int	DebugLog_C89( const char *inFunction, LogLevel inLevel, const char *inFormat, ... )
{
	if( log_category_ptr_enabled( &log_category_from_name( DebugServicesLogging ), inLevel ) )
	{
		int				n;
		va_list			args;
		
		va_start( args, inFormat );
		n = LogPrintFVAList( &log_category_from_name( DebugServicesLogging ), inFunction, inLevel, inFormat, args );
		va_end( args );
		return( n );
	}
	return( 0 );
}
#endif

//===========================================================================================================================
//	DebugPrintAssert
//===========================================================================================================================

void
	DebugPrintAssert( 
		DebugAssertFlags	inFlags, 
		OSStatus			inErrorCode, 
		const char *		inAssertString, 
		const char *		inFilename, 
		long				inLineNumber, 
		const char *		inFunction, 
		const char *		inMessageFormat, 
		... )
{
	va_list				args;
	const char *		ptr;
	char				c;
	char *				stackTrace = NULL;
	
	if( !log_category_enabled( &log_category_from_name( DebugServicesAssert ), kLogLevelAssert ) )
	{
		return;
	}
	
	// Strip off any parent folder path in the filename.
	
	if( inFilename )
	{
		for( ptr = inFilename; ( c = *ptr ) != '\0'; ++ptr )
		{
			if( ( c == '/' ) || ( c == '\\' ) )
			{
				inFilename = ptr + 1;
			}
		}
	}
	
	// Print out the assert.
	
	if( !inFilename ) inFilename = "";
	if( !inFunction ) inFunction = "";
	
#if( TARGET_OS_LINUX )
	if( ( inFlags & kDebugAssertFlagsPanic ) || 
		log_category_enabled( &log_category_from_name( DebugServicesStackTrace ), kLogLevelAssert ) )
	{
		stackTrace = DebugCopyStackTrace( NULL );
	}
#endif
	
	va_start( args, inMessageFormat );
	if( inErrorCode != 0 )
	{
		dbs_ulog( kLogLevelAssert, "### [ASSERT] %s:%ld \"%###s\", %#m %V\n%s", 
			inFilename, inLineNumber, inFunction, 
			inErrorCode, inMessageFormat ? inMessageFormat : "", &args, stackTrace ? stackTrace : "" );
	}
	else
	{
		dbs_ulog( kLogLevelAssert, "### [ASSERT] %s:%ld \"%###s\", \"%s\" %V\n%s", 
			inFilename, inLineNumber, inFunction, inAssertString ? inAssertString : "", 
			inMessageFormat ? inMessageFormat : "", &args, stackTrace ? stackTrace : "" );
	}
	va_end( args );
	
	// If this is a panic assert, try to stop.
	
	if( inFlags & kDebugAssertFlagsPanic )
	{
		if( DebugIsDebuggerPresent() )
		{
			DebugEnterDebugger();
		}
		else
		{
			#if( TARGET_OS_WINDOWS )
				SuspendThread( GetCurrentThread() );
			#else
				for( ;; ) {}
			#endif
		}
	}
	else
	{
		if( log_category_enabled( &log_category_from_name( DebugServicesBreak ), kLogLevelAssert ) &&
			DebugIsDebuggerPresent() )
		{
			DebugEnterDebugger();
		}
	}
	if( stackTrace ) free( stackTrace );
}

#if( DEBUG || DEBUG_EXPORT_ERROR_STRINGS )
//===========================================================================================================================
//	DebugErrors
//===========================================================================================================================

#define	CaseErrorString( X, STR )					{ ( X ), ( STR ) }
#define	CaseErrorStringify( X )						{ ( X ), # X }
#define	CaseErrorStringifyHardCode( VALUE, X )		{ (OSStatus)( VALUE ), # X }
#if( TARGET_RT_64_BIT )
	#define CaseEFIErrorString( X, STR )			{ (OSStatus)( ( X ) | UINT64_C( 0x8000000000000000 ) ), # STR }, \
													{ (OSStatus)( ( X ) | 0x80000000U ), # STR }
	#define CaseEFIError2String( X, STR )			{ (OSStatus)( ( X ) | UINT64_C( 0xC000000000000000 ) ), # STR }, \
													{ (OSStatus)( ( X ) | 0xC0000000U ), # STR }
#else
	#define CaseEFIErrorString( X, STR )			{ (OSStatus)( ( X ) | 0x80000000U ), # STR }
	#define CaseEFIError2String( X, STR )			{ (OSStatus)( ( X ) | 0xC0000000U ), # STR }
#endif
#define	CaseEnd()									{ 0, NULL }

typedef struct
{
	OSStatus			err;
	const char *		str;
	
}	DebugErrorEntry;

static const DebugErrorEntry		kDebugErrors[] = 
{
	// General Errors
	
	CaseErrorString( 0,  "noErr" ),
	CaseErrorString( -1, "catch-all unknown error" ),
	
	// Common Services Errors
	
	CaseErrorStringify( kUnknownErr ),
	CaseErrorStringify( kOptionErr ),
	CaseErrorStringify( kSelectorErr ),
	CaseErrorStringify( kExecutionStateErr ),
	CaseErrorStringify( kPathErr ),
	CaseErrorStringify( kParamErr ),
	CaseErrorStringify( kUserRequiredErr ),
	CaseErrorStringify( kCommandErr ),
	CaseErrorStringify( kIDErr ),
	CaseErrorStringify( kStateErr ),
	CaseErrorStringify( kRangeErr ),
	CaseErrorStringify( kRequestErr ),
	CaseErrorStringify( kResponseErr ),
	CaseErrorStringify( kChecksumErr ),
	CaseErrorStringify( kNotHandledErr ),
	CaseErrorStringify( kVersionErr ),
	CaseErrorStringify( kSignatureErr ),
	CaseErrorStringify( kFormatErr ),
	CaseErrorStringify( kNotInitializedErr ),
	CaseErrorStringify( kAlreadyInitializedErr ),
	CaseErrorStringify( kNotInUseErr ),
	CaseErrorStringify( kAlreadyInUseErr ),
	CaseErrorStringify( kTimeoutErr ),
	CaseErrorStringify( kCanceledErr ),
	CaseErrorStringify( kAlreadyCanceledErr ),
	CaseErrorStringify( kCannotCancelErr ),
	CaseErrorStringify( kDeletedErr ),
	CaseErrorStringify( kNotFoundErr ),
	CaseErrorStringify( kNoMemoryErr ),
	CaseErrorStringify( kNoResourcesErr ),
	CaseErrorStringify( kDuplicateErr ),
	CaseErrorStringify( kImmutableErr ),
	CaseErrorStringify( kUnsupportedDataErr ),
	CaseErrorStringify( kIntegrityErr ),
	CaseErrorStringify( kIncompatibleErr ),
	CaseErrorStringify( kUnsupportedErr ),
	CaseErrorStringify( kUnexpectedErr ),
	CaseErrorStringify( kValueErr ),
	CaseErrorStringify( kNotReadableErr ),
	CaseErrorStringify( kNotWritableErr ),
	CaseErrorStringify( kBadReferenceErr ),
	CaseErrorStringify( kFlagErr ),
	CaseErrorStringify( kMalformedErr ),
	CaseErrorStringify( kSizeErr ),
	CaseErrorStringify( kNameErr ),
	CaseErrorStringify( kNotPreparedErr ),
	CaseErrorStringify( kReadErr ),
	CaseErrorStringify( kWriteErr ),
	CaseErrorStringify( kMismatchErr ),
	CaseErrorStringify( kDateErr ),
	CaseErrorStringify( kUnderrunErr ),
	CaseErrorStringify( kOverrunErr ),
	CaseErrorStringify( kEndingErr ),
	CaseErrorStringify( kConnectionErr ),
	CaseErrorStringify( kAuthenticationErr ),
	CaseErrorStringify( kOpenErr ),
	CaseErrorStringify( kTypeErr ),
	CaseErrorStringify( kSkipErr ),
	CaseErrorStringify( kNoAckErr ),
	CaseErrorStringify( kCollisionErr ),
	CaseErrorStringify( kBackoffErr ),
	CaseErrorStringify( kAddressErr ),
	CaseErrorStringify( kInternalErr ),
	CaseErrorStringify( kNoSpaceErr ),
	CaseErrorStringify( kCountErr ),
	CaseErrorStringify( kEndOfDataErr ),
	CaseErrorStringify( kWouldBlockErr ),
	CaseErrorStringify( kLookErr ),
	CaseErrorStringify( kSecurityRequiredErr ),
	CaseErrorStringify( kOrderErr ),
	CaseErrorStringify( kUpgradeErr ),
	CaseErrorStringify( kAsyncNoErr ),
	CaseErrorStringify( kDeprecatedErr ),
	CaseErrorStringify( kPermissionErr ),
	
	// Bonjour Errors
	
	CaseErrorStringifyHardCode( -65537, kDNSServiceErr_Unknown ),
	CaseErrorStringifyHardCode( -65538, kDNSServiceErr_NoSuchName ),
	CaseErrorStringifyHardCode( -65539, kDNSServiceErr_NoMemory ),
	CaseErrorStringifyHardCode( -65540, kDNSServiceErr_BadParam ),
	CaseErrorStringifyHardCode( -65541, kDNSServiceErr_BadReference ),
	CaseErrorStringifyHardCode( -65542, kDNSServiceErr_BadState ),
	CaseErrorStringifyHardCode( -65543, kDNSServiceErr_BadFlags ),
	CaseErrorStringifyHardCode( -65544, kDNSServiceErr_Unsupported ),
	CaseErrorStringifyHardCode( -65545, kDNSServiceErr_NotInitialized ),
	CaseErrorStringifyHardCode( -65546, mStatus_NoCache ),
	CaseErrorStringifyHardCode( -65547, kDNSServiceErr_AlreadyRegistered ),
	CaseErrorStringifyHardCode( -65548, kDNSServiceErr_NameConflict ),
	CaseErrorStringifyHardCode( -65549, kDNSServiceErr_Invalid ),
	CaseErrorStringifyHardCode( -65550, kDNSServiceErr_Firewall ),
	CaseErrorStringifyHardCode( -65551, kDNSServiceErr_Incompatible ),
	CaseErrorStringifyHardCode( -65552, kDNSServiceErr_BadInterfaceIndex ),
	CaseErrorStringifyHardCode( -65553, kDNSServiceErr_Refused ),
	CaseErrorStringifyHardCode( -65554, kDNSServiceErr_NoSuchRecord ),
	CaseErrorStringifyHardCode( -65555, kDNSServiceErr_NoAuth ),
	CaseErrorStringifyHardCode( -65556, kDNSServiceErr_NoSuchKey ),
	CaseErrorStringifyHardCode( -65557, kDNSServiceErr_NATTraversal ),
	CaseErrorStringifyHardCode( -65558, kDNSServiceErr_DoubleNAT ),
	CaseErrorStringifyHardCode( -65559, kDNSServiceErr_BadTime ),
	CaseErrorStringifyHardCode( -65560, kDNSServiceErr_BadSig ),
	CaseErrorStringifyHardCode( -65561, kDNSServiceErr_BadKey ),
	CaseErrorStringifyHardCode( -65562, kDNSServiceErr_Transient ),
	CaseErrorStringifyHardCode( -65563, kDNSServiceErr_ServiceNotRunning ), 
	CaseErrorStringifyHardCode( -65564, kDNSServiceErr_NATPortMappingUnsupported ), 
	CaseErrorStringifyHardCode( -65565, kDNSServiceErr_NATPortMappingDisabled ), 
	CaseErrorStringifyHardCode( -65566, kDNSServiceErr_NoRouter ), 
	CaseErrorStringifyHardCode( -65567, kDNSServiceErr_PollingMode ), 
	CaseErrorStringifyHardCode( -65568, kDNSServiceErr_Timeout ), 
	
	CaseErrorStringifyHardCode( -65787, mStatus_ConnPending ),
	CaseErrorStringifyHardCode( -65788, mStatus_ConnFailed ),
	CaseErrorStringifyHardCode( -65789, mStatus_ConnEstablished ),
	CaseErrorStringifyHardCode( -65790, mStatus_GrowCache ),
	CaseErrorStringifyHardCode( -65791, mStatus_ConfigChanged ),
	CaseErrorStringifyHardCode( -65792, mStatus_MemFree ),
	
	// errno
	
	#ifdef EACCES
	CaseErrorStringify( EACCES ), 
	CaseErrorStringifyHardCode( -EACCES, EACCES ), 
	#endif
	
	#ifdef EADDRINUSE
	CaseErrorStringify( EADDRINUSE ), 
	CaseErrorStringifyHardCode( -EADDRINUSE, EADDRINUSE ), 
	#endif
	
	#ifdef EADDRNOTAVAIL
	CaseErrorStringify( EADDRNOTAVAIL ), 
	CaseErrorStringifyHardCode( -EADDRNOTAVAIL, EADDRNOTAVAIL ), 
	#endif
	
	#ifdef EADV
	CaseErrorStringify( EADV ), 
	CaseErrorStringifyHardCode( -EADV, EADV ), 
	#endif
	
	#ifdef EAFNOSUPPORT
	CaseErrorStringify( EAFNOSUPPORT ), 
	CaseErrorStringifyHardCode( -EAFNOSUPPORT, EAFNOSUPPORT ), 
	#endif
	
	#ifdef EALREADY
		#if( !defined( EBUSY ) || ( EALREADY != EBUSY ) )
		CaseErrorStringify( EALREADY ), 
		CaseErrorStringifyHardCode( -EALREADY, EALREADY ), 
		#endif
	#endif
	
	#ifdef EAUTH
	CaseErrorStringify( EAUTH ), 
	CaseErrorStringifyHardCode( -EAUTH, EAUTH ), 
	#endif
	
	#ifdef EBADARCH
	CaseErrorStringify( EBADARCH ), 
	CaseErrorStringifyHardCode( -EBADARCH, EBADARCH ), 
	#endif
	
	#ifdef EBADE
	CaseErrorStringify( EBADE ), 
	CaseErrorStringifyHardCode( -EBADE, EBADE ), 
	#endif
	
	#ifdef EBADEXEC
	CaseErrorStringify( EBADEXEC ), 
	CaseErrorStringifyHardCode( -EBADEXEC, EBADEXEC ), 
	#endif
	
	#ifdef EBADF
	CaseErrorStringify( EBADF ), 
	CaseErrorStringifyHardCode( -EBADF, EBADF ), 
	#endif
	
	#ifdef EBADFD
	CaseErrorStringify( EBADFD ), 
	CaseErrorStringifyHardCode( -EBADFD, EBADFD ), 
	#endif
	
	#ifdef EBADFSYS
	CaseErrorStringify( EBADFSYS ), 
	CaseErrorStringifyHardCode( -EBADFSYS, EBADFSYS ), 
	#endif
	
	#ifdef EBADMACHO
	CaseErrorStringify( EBADMACHO ), 
	CaseErrorStringifyHardCode( -EBADMACHO, EBADMACHO ), 
	#endif
	
	#ifdef EBADMSG
	CaseErrorStringify( EBADMSG ), 
	CaseErrorStringifyHardCode( -EBADMSG, EBADMSG ), 
	#endif
	
	#ifdef EBADR
	CaseErrorStringify( EBADR ), 
	CaseErrorStringifyHardCode( -EBADR, EBADR ), 
	#endif
	
	#ifdef EBADRPC
	CaseErrorStringify( EBADRPC ), 
	CaseErrorStringifyHardCode( -EBADRPC, EBADRPC ), 
	#endif
	
	#ifdef EBADRQC
	CaseErrorStringify( EBADRQC ), 
	CaseErrorStringifyHardCode( -EBADRQC, EBADRQC ), 
	#endif
	
	#ifdef EBADSLT
	CaseErrorStringify( EBADSLT ), 
	CaseErrorStringifyHardCode( -EBADSLT, EBADSLT ), 
	#endif
	
	#ifdef EBFONT
	CaseErrorStringify( EBFONT ), 
	CaseErrorStringifyHardCode( -EBFONT, EBFONT ), 
	#endif
	
	#ifdef EBUSY
	CaseErrorStringify( EBUSY ), 
	CaseErrorStringifyHardCode( -EBUSY, EBUSY ), 
	#endif
	
	#ifdef ECANCELED
	CaseErrorStringify( ECANCELED ), 
	CaseErrorStringifyHardCode( -ECANCELED, ECANCELED ), 
	#endif
	
	#ifdef ECHILD
	CaseErrorStringify( ECHILD ), 
	CaseErrorStringifyHardCode( -ECHILD, ECHILD ), 
	#endif
	
	#ifdef ECHRNG
	CaseErrorStringify( ECHRNG ), 
	CaseErrorStringifyHardCode( -ECHRNG, ECHRNG ), 
	#endif
	
	#ifdef ECOMM
	CaseErrorStringify( ECOMM ), 
	CaseErrorStringifyHardCode( -ECOMM, ECOMM ), 
	#endif
	
	#ifdef ECONNABORTED
	CaseErrorStringify( ECONNABORTED ), 
	CaseErrorStringifyHardCode( -ECONNABORTED, ECONNABORTED ), 
	#endif
	
	#ifdef ECONNREFUSED
	CaseErrorStringify( ECONNREFUSED ), 
	CaseErrorStringifyHardCode( -ECONNREFUSED, ECONNREFUSED ), 
	#endif
	
	#ifdef ECONNRESET
	CaseErrorStringify( ECONNRESET ), 
	CaseErrorStringifyHardCode( -ECONNRESET, ECONNRESET ), 
	#endif
	
	#ifdef ECTRLTERM
	CaseErrorStringify( ECTRLTERM ), 
	CaseErrorStringifyHardCode( -ECTRLTERM, ECTRLTERM ), 
	#endif
	
	#ifdef EDEADLK
	CaseErrorStringify( EDEADLK ), 
	CaseErrorStringifyHardCode( -EDEADLK, EDEADLK ), 
	#endif
	
	#ifdef EDEADLOCK
	CaseErrorStringify( EDEADLOCK ), 
	CaseErrorStringifyHardCode( -EDEADLOCK, EDEADLOCK ), 
	#endif
	
	#ifdef EDESTADDRREQ
	CaseErrorStringify( EDESTADDRREQ ), 
	CaseErrorStringifyHardCode( -EDESTADDRREQ, EDESTADDRREQ ), 
	#endif
	
	#ifdef EDEVERR
	CaseErrorStringify( EDEVERR ), 
	CaseErrorStringifyHardCode( -EDEVERR, EDEVERR ), 
	#endif
	
	#ifdef EDOM
	CaseErrorStringify( EDOM ), 
	CaseErrorStringifyHardCode( -EDOM, EDOM ), 
	#endif
	
	#ifdef EDQUOT
	CaseErrorStringify( EDQUOT ), 
	CaseErrorStringifyHardCode( -EDQUOT, EDQUOT ), 
	#endif
	
	#ifdef EENDIAN
	CaseErrorStringify( EENDIAN ), 
	CaseErrorStringifyHardCode( -EENDIAN, EENDIAN ), 
	#endif
	
	#ifdef EEXIST
	CaseErrorStringify( EEXIST ), 
	CaseErrorStringifyHardCode( -EEXIST, EEXIST ), 
	#endif
	
	#ifdef EFAULT
	CaseErrorStringify( EFAULT ), 
	CaseErrorStringifyHardCode( -EFAULT, EFAULT ), 
	#endif
	
	#ifdef EFBIG
	CaseErrorStringify( EFBIG ), 
	CaseErrorStringifyHardCode( -EFBIG, EFBIG ), 
	#endif
	
	#ifdef EFPOS
	CaseErrorStringify( EFPOS ), 
	CaseErrorStringifyHardCode( -EFPOS, EFPOS ), 
	#endif
	
	#ifdef EFTYPE
	CaseErrorStringify( EFTYPE ), 
	CaseErrorStringifyHardCode( -EFTYPE, EFTYPE ), 
	#endif
	
	#ifdef EHAVEOOB
	CaseErrorStringify( EHAVEOOB ), 
	CaseErrorStringifyHardCode( -EHAVEOOB, EHAVEOOB ), 
	#endif
	
	#ifdef EHOSTDOWN
	CaseErrorStringify( EHOSTDOWN ), 
	CaseErrorStringifyHardCode( -EHOSTDOWN, EHOSTDOWN ), 
	#endif
	
	#ifdef EHOSTUNREACH
	CaseErrorStringify( EHOSTUNREACH ), 
	CaseErrorStringifyHardCode( -EHOSTUNREACH, EHOSTUNREACH ), 
	#endif
	
	#ifdef EIDRM
	CaseErrorStringify( EIDRM ), 
	CaseErrorStringifyHardCode( -EIDRM, EIDRM ), 
	#endif
	
	#ifdef EIEIO
	CaseErrorStringify( EIEIO ), 
	CaseErrorStringifyHardCode( -EIEIO, EIEIO ), 
	#endif
	
	#ifdef EILSEQ
	CaseErrorStringify( EILSEQ ), 
	CaseErrorStringifyHardCode( -EILSEQ, EILSEQ ), 
	#endif
	
	#ifdef EINPROGRESS
	CaseErrorStringify( EINPROGRESS ), 
	CaseErrorStringifyHardCode( -EINPROGRESS, EINPROGRESS ), 
	#endif
	
	#ifdef EINTR
	CaseErrorStringify( EINTR ), 
	CaseErrorStringifyHardCode( -EINTR, EINTR ), 
	#endif
	
	#ifdef EINVAL
	CaseErrorStringify( EINVAL ), 
	CaseErrorStringifyHardCode( -EINVAL, EINVAL ), 
	#endif
	
	#ifdef EIO
	CaseErrorStringify( EIO ), 
	CaseErrorStringifyHardCode( -EIO, EIO ), 
	#endif
	
	#ifdef EISCONN
	CaseErrorStringify( EISCONN ), 
	CaseErrorStringifyHardCode( -EISCONN, EISCONN ), 
	#endif
	
	#ifdef EISDIR
	CaseErrorStringify( EISDIR ), 
	CaseErrorStringifyHardCode( -EISDIR, EISDIR ), 
	#endif
	
	#ifdef EL2HLT
	CaseErrorStringify( EL2HLT ), 
	CaseErrorStringifyHardCode( -EL2HLT, EL2HLT ), 
	#endif
	
	#ifdef EL2NSYNC
	CaseErrorStringify( EL2NSYNC ), 
	CaseErrorStringifyHardCode( -EL2NSYNC, EL2NSYNC ), 
	#endif
	
	#ifdef EL3HLT
	CaseErrorStringify( EL3HLT ), 
	CaseErrorStringifyHardCode( -EL3HLT, EL3HLT ), 
	#endif
	
	#ifdef EL3RST
	CaseErrorStringify( EL3RST ), 
	CaseErrorStringifyHardCode( -EL3RST, EL3RST ), 
	#endif
	
	#ifdef ELAST
	CaseErrorStringify( ELAST ), 
	CaseErrorStringifyHardCode( -ELAST, ELAST ), 
	#endif
	
	#ifdef ELIBACC
	CaseErrorStringify( ELIBACC ), 
	CaseErrorStringifyHardCode( -ELIBACC, ELIBACC ), 
	#endif
	
	#ifdef ELIBBAD
	CaseErrorStringify( ELIBBAD ), 
	CaseErrorStringifyHardCode( -ELIBBAD, ELIBBAD ), 
	#endif
	
	#ifdef ELIBEXEC
	CaseErrorStringify( ELIBEXEC ), 
	CaseErrorStringifyHardCode( -ELIBEXEC, ELIBEXEC ), 
	#endif
	
	#ifdef ELIBMAX
	CaseErrorStringify( ELIBMAX ), 
	CaseErrorStringifyHardCode( -ELIBMAX, ELIBMAX ), 
	#endif
	
	#ifdef ELIBSCN
	CaseErrorStringify( ELIBSCN ), 
	CaseErrorStringifyHardCode( -ELIBSCN, ELIBSCN ), 
	#endif
	
	#ifdef ELNRNG
	CaseErrorStringify( ELNRNG ), 
	CaseErrorStringifyHardCode( -ELNRNG, ELNRNG ), 
	#endif
	
	#ifdef ELOOP
	CaseErrorStringify( ELOOP ), 
	CaseErrorStringifyHardCode( -ELOOP, ELOOP ), 
	#endif
	
	#ifdef ELOWER
	CaseErrorStringify( ELOWER ), 
	CaseErrorStringifyHardCode( -ELOWER, ELOWER ), 
	#endif
	
	#ifdef EMFILE
	CaseErrorStringify( EMFILE ), 
	CaseErrorStringifyHardCode( -EMFILE, EMFILE ), 
	#endif
	
	#ifdef EMLINK
	CaseErrorStringify( EMLINK ), 
	CaseErrorStringifyHardCode( -EMLINK, EMLINK ), 
	#endif
	
	#ifdef EMORE
	CaseErrorStringify( EMORE ), 
	CaseErrorStringifyHardCode( -EMORE, EMORE ), 
	#endif
	
	#ifdef EMSGSIZE
	CaseErrorStringify( EMSGSIZE ), 
	CaseErrorStringifyHardCode( -EMSGSIZE, EMSGSIZE ), 
	#endif
	
	#ifdef EMULTIHOP
	CaseErrorStringify( EMULTIHOP ), 
	CaseErrorStringifyHardCode( -EMULTIHOP, EMULTIHOP ), 
	#endif
	
	#ifdef ENAMETOOLONG
	CaseErrorStringify( ENAMETOOLONG ), 
	CaseErrorStringifyHardCode( -ENAMETOOLONG, ENAMETOOLONG ), 
	#endif
	
	#ifdef ENEEDAUTH
	CaseErrorStringify( ENEEDAUTH ), 
	CaseErrorStringifyHardCode( -ENEEDAUTH, ENEEDAUTH ), 
	#endif
	
	#ifdef ENETDOWN
	CaseErrorStringify( ENETDOWN ), 
	CaseErrorStringifyHardCode( -ENETDOWN, ENETDOWN ), 
	#endif
	
	#ifdef ENETRESET
	CaseErrorStringify( ENETRESET ), 
	CaseErrorStringifyHardCode( -ENETRESET, ENETRESET ), 
	#endif
	
	#ifdef ENETUNREACH
	CaseErrorStringify( ENETUNREACH ), 
	CaseErrorStringifyHardCode( -ENETUNREACH, ENETUNREACH ), 
	#endif
	
	#ifdef ENFILE
	CaseErrorStringify( ENFILE ), 
	CaseErrorStringifyHardCode( -ENFILE, ENFILE ), 
	#endif
	
	#ifdef ENOANO
	CaseErrorStringify( ENOANO ), 
	CaseErrorStringifyHardCode( -ENOANO, ENOANO ), 
	#endif
	
	#ifdef ENOATTR
	CaseErrorStringify( ENOATTR ), 
	CaseErrorStringifyHardCode( -ENOATTR, ENOATTR ), 
	#endif
	
	#ifdef ENOBUFS
	CaseErrorStringify( ENOBUFS ), 
	CaseErrorStringifyHardCode( -ENOBUFS, ENOBUFS ), 
	#endif
	
	#ifdef ENOCSI
	CaseErrorStringify( ENOCSI ), 
	CaseErrorStringifyHardCode( -ENOCSI, ENOCSI ), 
	#endif
	
	#ifdef ENODATA
	CaseErrorStringify( ENODATA ), 
	CaseErrorStringifyHardCode( -ENODATA, ENODATA ), 
	#endif
	
	#ifdef ENODEV
	CaseErrorStringify( ENODEV ), 
	CaseErrorStringifyHardCode( -ENODEV, ENODEV ), 
	#endif
	
	#ifdef ENOENT
	CaseErrorStringify( ENOENT ), 
	CaseErrorStringifyHardCode( -ENOENT, ENOENT ), 
	#endif
	
	#ifdef ENOEXEC
	CaseErrorStringify( ENOEXEC ), 
	CaseErrorStringifyHardCode( -ENOEXEC, ENOEXEC ), 
	#endif
	
	#ifdef ENOLCK
	CaseErrorStringify( ENOLCK ), 
	CaseErrorStringifyHardCode( -ENOLCK, ENOLCK ), 
	#endif
	
	#ifdef ENOLIC
	CaseErrorStringify( ENOLIC ), 
	CaseErrorStringifyHardCode( -ENOLIC, ENOLIC ), 
	#endif
	
	#ifdef ENOLINK
	CaseErrorStringify( ENOLINK ), 
	CaseErrorStringifyHardCode( -ENOLINK, ENOLINK ), 
	#endif
	
	#ifdef ENOMEM
	CaseErrorStringify( ENOMEM ), 
	CaseErrorStringifyHardCode( -ENOMEM, ENOMEM ), 
	#endif
	
	#ifdef ENOMSG
	CaseErrorStringify( ENOMSG ), 
	CaseErrorStringifyHardCode( -ENOMSG, ENOMSG ), 
	#endif
	
	#ifdef ENONDP
	CaseErrorStringify( ENONDP ), 
	CaseErrorStringifyHardCode( -ENONDP, ENONDP ), 
	#endif
	
	#ifdef ENONET
	CaseErrorStringify( ENONET ), 
	CaseErrorStringifyHardCode( -ENONET, ENONET ), 
	#endif
	
	#ifdef ENOPKG
	CaseErrorStringify( ENOPKG ), 
	CaseErrorStringifyHardCode( -ENOPKG, ENOPKG ), 
	#endif
	
	#ifdef ENOPOLICY
	CaseErrorStringify( ENOPOLICY ), 
	CaseErrorStringifyHardCode( -ENOPOLICY, ENOPOLICY ), 
	#endif
	
	#ifdef ENOPROTOOPT
	CaseErrorStringify( ENOPROTOOPT ), 
	CaseErrorStringifyHardCode( -ENOPROTOOPT, ENOPROTOOPT ), 
	#endif
	
	#ifdef ENOREMOTE
	CaseErrorStringify( ENOREMOTE ), 
	CaseErrorStringifyHardCode( -ENOREMOTE, ENOREMOTE ), 
	#endif
	
	#ifdef ENOSPC
	CaseErrorStringify( ENOSPC ), 
	CaseErrorStringifyHardCode( -ENOSPC, ENOSPC ), 
	#endif
	
	#ifdef ENOSR
	CaseErrorStringify( ENOSR ), 
	CaseErrorStringifyHardCode( -ENOSR, ENOSR ), 
	#endif
	
	#ifdef ENOSTR
	CaseErrorStringify( ENOSTR ), 
	CaseErrorStringifyHardCode( -ENOSTR, ENOSTR ), 
	#endif
	
	#ifdef ENOSYS
	CaseErrorStringify( ENOSYS ), 
	CaseErrorStringifyHardCode( -ENOSYS, ENOSYS ), 
	#endif
	
	#ifdef ENOTBLK
	CaseErrorStringify( ENOTBLK ), 
	CaseErrorStringifyHardCode( -ENOTBLK, ENOTBLK ), 
	#endif
	
	#ifdef ENOTCONN
	CaseErrorStringify( ENOTCONN ), 
	CaseErrorStringifyHardCode( -ENOTCONN, ENOTCONN ), 
	#endif
	
	#ifdef ENOTDIR
	CaseErrorStringify( ENOTDIR ), 
	CaseErrorStringifyHardCode( -ENOTDIR, ENOTDIR ), 
	#endif
	
	#ifdef ENOTEMPTY
	CaseErrorStringify( ENOTEMPTY ), 
	CaseErrorStringifyHardCode( -ENOTEMPTY, ENOTEMPTY ), 
	#endif
	
	#ifdef ENOTRECOVERABLE
	CaseErrorStringify( ENOTRECOVERABLE ), 
	CaseErrorStringifyHardCode( -ENOTRECOVERABLE, ENOTRECOVERABLE ), 
	#endif
	
	#ifdef ENOTSOCK
	CaseErrorStringify( ENOTSOCK ), 
	CaseErrorStringifyHardCode( -ENOTSOCK, ENOTSOCK ), 
	#endif
	
	#ifdef ENOTSUP
	CaseErrorStringify( ENOTSUP ), 
	CaseErrorStringifyHardCode( -ENOTSUP, ENOTSUP ), 
	#endif
	
	#ifdef ENOTTY
	CaseErrorStringify( ENOTTY ), 
	CaseErrorStringifyHardCode( -ENOTTY, ENOTTY ), 
	#endif
	
	#ifdef ENOTUNIQ
	CaseErrorStringify( ENOTUNIQ ), 
	CaseErrorStringifyHardCode( -ENOTUNIQ, ENOTUNIQ ), 
	#endif
	
	#ifdef ENXIO
	CaseErrorStringify( ENXIO ), 
	CaseErrorStringifyHardCode( -ENXIO, ENXIO ), 
	#endif
	
	#ifdef EOPNOTSUPP
	CaseErrorStringify( EOPNOTSUPP ), 
	CaseErrorStringifyHardCode( -EOPNOTSUPP, EOPNOTSUPP ), 
	#endif
	
	#ifdef EOVERFLOW
	CaseErrorStringify( EOVERFLOW ), 
	CaseErrorStringifyHardCode( -EOVERFLOW, EOVERFLOW ), 
	#endif
	
	#ifdef EOWNERDEAD
	CaseErrorStringify( EOWNERDEAD ), 
	CaseErrorStringifyHardCode( -EOWNERDEAD, EOWNERDEAD ), 
	#endif
	
	#ifdef EPERM
	CaseErrorStringify( EPERM ), 
	CaseErrorStringifyHardCode( -EPERM, EPERM ), 
	#endif
	
	#ifdef EPFNOSUPPORT
	CaseErrorStringify( EPFNOSUPPORT ), 
	CaseErrorStringifyHardCode( -EPFNOSUPPORT, EPFNOSUPPORT ), 
	#endif
	
	#ifdef EPIPE
	CaseErrorStringify( EPIPE ), 
	CaseErrorStringifyHardCode( -EPIPE, EPIPE ), 
	#endif
	
	#ifdef EPROCLIM
	CaseErrorStringify( EPROCLIM ), 
	CaseErrorStringifyHardCode( -EPROCLIM, EPROCLIM ), 
	#endif
	
	#ifdef EPROCUNAVAIL
	CaseErrorStringify( EPROCUNAVAIL ), 
	CaseErrorStringifyHardCode( -EPROCUNAVAIL, EPROCUNAVAIL ), 
	#endif
	
	#ifdef EPROGMISMATCH
	CaseErrorStringify( EPROGMISMATCH ), 
	CaseErrorStringifyHardCode( -EPROGMISMATCH, EPROGMISMATCH ), 
	#endif
	
	#ifdef EPROGUNAVAIL
	CaseErrorStringify( EPROGUNAVAIL ), 
	CaseErrorStringifyHardCode( -EPROGUNAVAIL, EPROGUNAVAIL ), 
	#endif
	
	#ifdef EPROTO
	CaseErrorStringify( EPROTO ), 
	CaseErrorStringifyHardCode( -EPROTO, EPROTO ), 
	#endif
	
	#ifdef EPROTONOSUPPORT
	CaseErrorStringify( EPROTONOSUPPORT ), 
	CaseErrorStringifyHardCode( -EPROTONOSUPPORT, EPROTONOSUPPORT ), 
	#endif
	
	#ifdef EPROTOTYPE
	CaseErrorStringify( EPROTOTYPE ), 
	CaseErrorStringifyHardCode( -EPROTOTYPE, EPROTOTYPE ), 
	#endif
	
	#ifdef EPWROFF
	CaseErrorStringify( EPWROFF ), 
	CaseErrorStringifyHardCode( -EPWROFF, EPWROFF ), 
	#endif
	
	#ifdef EQFULL
	CaseErrorStringify( EQFULL ), 
	CaseErrorStringifyHardCode( -EQFULL, EQFULL ), 
	#endif
	
	#ifdef ERANGE
	CaseErrorStringify( ERANGE ), 
	CaseErrorStringifyHardCode( -ERANGE, ERANGE ), 
	#endif
	
	#ifdef EREMCHG
	CaseErrorStringify( EREMCHG ), 
	CaseErrorStringifyHardCode( -EREMCHG, EREMCHG ), 
	#endif
	
	#ifdef EREMOTE
	CaseErrorStringify( EREMOTE ), 
	CaseErrorStringifyHardCode( -EREMOTE, EREMOTE ), 
	#endif
	
	#ifdef ERESTART
	CaseErrorStringify( ERESTART ), 
	CaseErrorStringifyHardCode( -ERESTART, ERESTART ), 
	#endif
	
	#ifdef EROFS
	CaseErrorStringify( EROFS ), 
	CaseErrorStringifyHardCode( -EROFS, EROFS ), 
	#endif
	
	#ifdef ERPCMISMATCH
	CaseErrorStringify( ERPCMISMATCH ), 
	CaseErrorStringifyHardCode( -ERPCMISMATCH, ERPCMISMATCH ), 
	#endif
	
	#ifdef ESHLIBVERS
	CaseErrorStringify( ESHLIBVERS ), 
	CaseErrorStringifyHardCode( -ESHLIBVERS, ESHLIBVERS ), 
	#endif
	
	#ifdef ESHUTDOWN
	CaseErrorStringify( ESHUTDOWN ), 
	CaseErrorStringifyHardCode( -ESHUTDOWN, ESHUTDOWN ), 
	#endif
	
	#ifdef ESOCKTNOSUPPORT
	CaseErrorStringify( ESOCKTNOSUPPORT ), 
	CaseErrorStringifyHardCode( -ESOCKTNOSUPPORT, ESOCKTNOSUPPORT ), 
	#endif
	
	#ifdef ESPIPE
	CaseErrorStringify( ESPIPE ), 
	CaseErrorStringifyHardCode( -ESPIPE, ESPIPE ), 
	#endif
	
	#ifdef ESRCH
	CaseErrorStringify( ESRCH ), 
	CaseErrorStringifyHardCode( -ESRCH, ESRCH ), 
	#endif
	
	#ifdef ESRMNT
	CaseErrorStringify( ESRMNT ), 
	CaseErrorStringifyHardCode( -ESRMNT, ESRMNT ), 
	#endif
	
	#ifdef ESRVRFAULT
	CaseErrorStringify( ESRVRFAULT ), 
	CaseErrorStringifyHardCode( -ESRVRFAULT, ESRVRFAULT ), 
	#endif
	
	#ifdef ESTALE
	CaseErrorStringify( ESTALE ), 
	CaseErrorStringifyHardCode( -ESTALE, ESTALE ), 
	#endif
	
	#ifdef ESTRPIPE
	CaseErrorStringify( ESTRPIPE ), 
	CaseErrorStringifyHardCode( -ESTRPIPE, ESTRPIPE ), 
	#endif
	
	#ifdef ETIME
	CaseErrorStringify( ETIME ), 
	CaseErrorStringifyHardCode( -ETIME, ETIME ), 
	#endif
	
	#ifdef ETIMEDOUT
	CaseErrorStringify( ETIMEDOUT ), 
	CaseErrorStringifyHardCode( -ETIMEDOUT, ETIMEDOUT ), 
	#endif
	
	#ifdef ETOOMANYREFS
	CaseErrorStringify( ETOOMANYREFS ), 
	CaseErrorStringifyHardCode( -ETOOMANYREFS, ETOOMANYREFS ), 
	#endif
	
	#ifdef ETXTBSY
	CaseErrorStringify( ETXTBSY ), 
	CaseErrorStringifyHardCode( -ETXTBSY, ETXTBSY ), 
	#endif
	
	#ifdef EUNATCH
	CaseErrorStringify( EUNATCH ), 
	CaseErrorStringifyHardCode( -EUNATCH, EUNATCH ), 
	#endif
	
	#ifdef EUSERS
	CaseErrorStringify( EUSERS ), 
	CaseErrorStringifyHardCode( -EUSERS, EUSERS ), 
	#endif
	
	#ifdef EWOULDBLOCK
	CaseErrorStringify( EWOULDBLOCK ), 
	CaseErrorStringifyHardCode( -EWOULDBLOCK, EWOULDBLOCK ), 
	#endif
	
	#ifdef EXDEV
	CaseErrorStringify( EXDEV ), 
	CaseErrorStringifyHardCode( -EXDEV, EXDEV ), 
	#endif
	
	#ifdef EXFULL
	CaseErrorStringify( EXFULL ), 
	CaseErrorStringifyHardCode( -EXFULL, EXFULL ), 
	#endif
	
	CaseEnd()
};

//===========================================================================================================================
//	DebugGetErrorString
//===========================================================================================================================

const char *	DebugGetErrorString( OSStatus inErrorCode, char *inBuffer, size_t inBufferSize )
{
	const DebugErrorEntry *		e;
	const char *				s;
	char *						dst;
	char *						end;
#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
	char						buf[ 256 ];
#endif
	
	// Check for HTTP status codes and the HTTP range for OSStatus.
	
	if( ( inErrorCode >= 100 ) && ( inErrorCode <= 599 ) )
	{
		s = HTTPGetReasonPhrase( inErrorCode );
		if( *s != '\0' ) goto gotIt;
	}
	else if( ( inErrorCode >= 200100 ) && ( inErrorCode <= 200599 ) )
	{
		s = HTTPGetReasonPhrase( inErrorCode - 200000 );
		if( *s != '\0' ) goto gotIt;
	}
	
	// Search our own table of error strings. If not found, fall back to other methods.
	
	for( e = kDebugErrors; ( ( s = e->str ) != NULL ) && ( e->err != inErrorCode ); ++e ) {}
	if( s ) goto gotIt;
	
#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
	// If on Windows, try FormatMessage.
	
	if( inBuffer && ( inBufferSize > 0 ) )
	{
		DWORD		n;
		
		n = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, (DWORD) inErrorCode, 
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), buf, sizeof( buf ), NULL );
		if( n > 0 )
		{
			// Remove any trailing CR's or LF's since some messages have them.
			
			while( ( n > 0 ) && isspace_safe( buf[ n - 1 ] ) ) buf[ --n ] = '\0';
			s = buf;
		}
	}
	
#endif
	
	// If we still haven't found a string, try the ANSI C strerror.
	
	if( !s )
	{
		#if( TARGET_HAS_STD_C_LIB && !TARGET_OS_WINDOWS_CE )
			s = strerror( inErrorCode );
		#endif
		if( !s ) s = "<< NO ERROR STRING >>";
	}
	
	// Copy the string to the output buffer. If no buffer is supplied or it is empty, return an empty string.
	
gotIt:
	if( inBuffer && ( inBufferSize > 0 ) )
	{
		dst = inBuffer;
		end = dst + ( inBufferSize - 1 );
		while( ( ( end - dst ) > 0 ) && ( *s != '\0' ) ) *dst++ = *s++;
		*dst = '\0';
		s = inBuffer;
	}
	return( s );
}

//===========================================================================================================================
//	DebugGetNextError
//===========================================================================================================================

OSStatus	DebugGetNextError( size_t inIndex, OSStatus *outErr )
{
	if( inIndex < ( countof( kDebugErrors ) - 1 ) )
	{
		*outErr = kDebugErrors[ inIndex ].err;
		return( kNoErr );
	}
	return( kRangeErr );
}
#endif // DEBUG || DEBUG_EXPORT_ERROR_STRINGS

//===========================================================================================================================
//	DebugCopyStackTrace
//===========================================================================================================================

#if( 0 || TARGET_OS_LINUX )
char *	DebugCopyStackTrace( OSStatus *outErr )
{
	char *		trace  = NULL;
	void *		stack[ 64 ];
	char **		symbols = NULL;
	int			i, j, n;
	
	n = backtrace( stack, (int) countof( stack ) );
	if( n > 0 ) symbols = backtrace_symbols( stack, n );
	
	// Print frames in reverse order (parent to child), but exclude the last one (this function).
	
	AppendPrintF( &trace, "" ); // Empty string so it's non-null in case there are no frames.
	if( symbols )
	{
		for( i = 0; i < ( n - 1 ); ++i )
		{
			j = ( n - i ) - 1;
			AppendPrintF( &trace, "%2d %*s %p  %s\n", i + 1, i, "", stack[ j ], symbols[ j ] );
		}
	}
	
	if( symbols ) 	free( symbols );
	if( outErr )	*outErr = trace ? kNoErr : kUnknownErr;
	return( trace );
}
#endif

//===========================================================================================================================
//	DebugCopyStackTrace
//===========================================================================================================================

#if( TARGET_OS_QNX )
char *	DebugCopyStackTrace( OSStatus *outErr )
{
	char *			trace = NULL;
	bt_memmap_t		map;
	bt_addr_t		stack[ 64 ];
	int				i, j, n, nn;
	char			symbol[ 128 ];
	
	bt_load_memmap( &bt_acc_self, &map );
	n = bt_get_backtrace( &bt_acc_self, stack, (int) countof( stack ) );
	AppendPrintF( &trace, "" ); // Empty string so it's non-null in case there are no frames.
	for( i = 0; i < n; ++i )
	{
		j = ( n - i ) - 1;
		*symbol = '\0';
		nn = bt_sprnf_addrs( &map, &stack[ j ], 1, "%a (%f)", symbol, sizeof( symbol ), NULL );
		if( nn != 1 ) { symbol[ 0 ] = '?'; symbol[ 1 ] = '\0'; }
		AppendPrintF( &trace, "%2d %*s %p  %s\n", i + 1, i, "", stack[ j ], symbol );
	}
	bt_unload_memmap( &map );
	
	if( outErr ) *outErr = trace ? kNoErr : kUnknownErr;
	return( trace );
}
#endif

#if( !TARGET_OS_WINDOWS )
//===========================================================================================================================
//	DebugStackTrace
//===========================================================================================================================

OSStatus	DebugStackTrace( LogLevel inLevel )
{
	OSStatus		err;
	
	if( !log_category_enabled( &log_category_from_name( DebugServicesStackTrace ), inLevel ) )
	{
		err = kRangeErr;
		goto exit;
	}
	
#if( TARGET_OS_LINUX || TARGET_OS_QNX )
{
	char *		trace;
	
	trace = DebugCopyStackTrace( &err );
	require_noerr_quiet( err, exit );
	
	dbs_ulog( inLevel, "\n%s", trace );
	free( trace );
	err = kNoErr;
}
#else
	dbs_ulog( kLogLevelError, "### stack tracing not supported on this platform\n" );
	err = kUnsupportedErr;
#endif
	
exit:
	return( err );
}
#endif // !TARGET_OS_WINDOWS

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	DebugStackTrace
//===========================================================================================================================

typedef BOOL
	( __stdcall * StackWalk64Func )(
		__in DWORD									inMachineType,
		__in HANDLE									inProcess,
		__in HANDLE									inThread,
		__inout LPSTACKFRAME64						ioStackFrame,
		__inout PVOID								ioCurrentRecord,
		__in_opt PREAD_PROCESS_MEMORY_ROUTINE64		inReadMemoryRoutine,
		__in_opt PFUNCTION_TABLE_ACCESS_ROUTINE64	inFunctionTableAccessRoutine,
		__in_opt PGET_MODULE_BASE_ROUTINE64			inGetModuleBaseRoutine,
		__in_opt PTRANSLATE_ADDRESS_ROUTINE64		inTranslateAddress );

typedef DWORD64
	( __stdcall * SymGetModuleBase64Func )(
		__in HANDLE		inProcess,
		__in DWORD64	inAddr );

typedef BOOL 
	( __stdcall * SymFromAddrFunc )(
		HANDLE			inProcess,
		DWORD64			inAddress,
		PDWORD64		inDisplacement,
		PSYMBOL_INFO	inSymbol );

typedef PVOID
	( __stdcall * SymFunctionTableAccess64Func )(
		__in HANDLE		inProcess,
		__in DWORD64	inAddrBase );

typedef BOOL
	( __stdcall * SymInitializeFunc )(
		__in HANDLE		inProcess,
		__in_opt PCSTR	oinUserSearchPath,
		__in BOOL		inInvadeProcess );

typedef DWORD	( __stdcall * SymSetOptionsFunc )( __in DWORD inSymOptions );

typedef struct
{
	DWORD64		addr;
	char		name[ 128 ];

}	StackLevel;

static Boolean							gDbgHelpInitialized;
static CRITICAL_SECTION					gDbgHelpLock;
static LONG								gDbgHelpLockState				= 0;
static HMODULE							gDbgHelpDLL						= NULL;
static StackWalk64Func					gStachWalkFunc					= NULL;
static SymGetModuleBase64Func			gSymGetModuleBase64Func			= NULL;
static SymFromAddrFunc					gSymFromAddrFunc				= NULL;
static SymFunctionTableAccess64Func		gSymFunctionTableAccess64Func	= NULL;
static SymInitializeFunc				gSymInitializeFunc				= NULL;
static SymSetOptionsFunc				gSymSetOptionsFunc				= NULL;

OSStatus	DebugStackTrace( LogLevel inLevel )
{
	OSStatus			err;
	StackLevel			stack[ 64 ];
	StackLevel *		levelPtr;
	CONTEXT				currentContext;
	HANDLE				currentProcess;
	HANDLE				currentThread;
	DWORD				machineType;
	STACKFRAME64		stackFrame;
	size_t				i, n, j;
	BOOL				good;
	size_t				len;
	
	// Skip if specified level is below the current level.
	
	if( !log_category_enabled( &log_category_from_name( DebugServicesStackTrace ), inLevel ) )
	{
		err = kRangeErr;
		goto exit2;
	}
	
	currentProcess = GetCurrentProcess();
	currentThread = GetCurrentThread();
	RtlCaptureContext( &currentContext );
	
	InitializeCriticalSectionOnce( &gDbgHelpLock, &gDbgHelpLockState );
	EnterCriticalSection( &gDbgHelpLock );

	// Load symbols dynamically since they are in Dbghelp.dll, which may not always be available.
	
	if( !gDbgHelpInitialized )
	{
		gDbgHelpInitialized = true; // Mark initialized early for easier cleanup. Lock prevents other threads.
		
		gDbgHelpDLL = LoadLibrary( TEXT( "Dbghelp" ) );
		if( gDbgHelpDLL )
		{
			gStachWalkFunc = (StackWalk64Func)(uintptr_t) GetProcAddress( gDbgHelpDLL, "StackWalk64" );
			require_action_quiet( gStachWalkFunc, exit, err = kUnsupportedErr );
			
			gSymGetModuleBase64Func = (SymGetModuleBase64Func)(uintptr_t) GetProcAddress( gDbgHelpDLL, 
				"SymGetModuleBase64" );
			require_action_quiet( gSymGetModuleBase64Func, exit, err = kUnsupportedErr );
			
			gSymFromAddrFunc = (SymFromAddrFunc)(uintptr_t) GetProcAddress( gDbgHelpDLL, "SymFromAddr" );
			require_action_quiet( gSymFromAddrFunc, exit, err = kUnsupportedErr );
			
			gSymFunctionTableAccess64Func = (SymFunctionTableAccess64Func)(uintptr_t) GetProcAddress( gDbgHelpDLL, 
				"SymFunctionTableAccess64" );
			require_action_quiet( gSymFunctionTableAccess64Func, exit, err = kUnsupportedErr );

			gSymInitializeFunc = (SymInitializeFunc)(uintptr_t) GetProcAddress( gDbgHelpDLL, "SymInitialize" );
			require_action_quiet( gSymInitializeFunc, exit, err = kUnsupportedErr );
			
			gSymSetOptionsFunc = (SymSetOptionsFunc)(uintptr_t) GetProcAddress( gDbgHelpDLL, "SymSetOptions" );
			require_action_quiet( gSymSetOptionsFunc, exit, err = kUnsupportedErr );
		}
		
		gSymSetOptionsFunc( SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME );
		good = gSymInitializeFunc( currentProcess, NULL, TRUE );
		err = map_global_value_errno( good, good );
		require_noerr_quiet( err, exit );
	}
	
	// Setup the info needed by StackWalk64.
	
	memset( &stackFrame, 0, sizeof( stackFrame ) );
#ifdef _M_IX86
	machineType						= IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset		= currentContext.Eip;
	stackFrame.AddrPC.Mode			= AddrModeFlat;
	stackFrame.AddrFrame.Offset		= currentContext.Ebp;
	stackFrame.AddrFrame.Mode		= AddrModeFlat;
	stackFrame.AddrStack.Offset		= currentContext.Esp;
	stackFrame.AddrStack.Mode		= AddrModeFlat;
#elif _M_X64
	machineType						= IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset		= currentContext.Rip;
	stackFrame.AddrPC.Mode			= AddrModeFlat;
	stackFrame.AddrFrame.Offset		= currentContext.Rsp;
	stackFrame.AddrFrame.Mode		= AddrModeFlat;
	stackFrame.AddrStack.Offset		= currentContext.Rsp;
	stackFrame.AddrStack.Mode		= AddrModeFlat;
#elif _M_IA64
	machineType						= IMAGE_FILE_MACHINE_IA64;
	stackFrame.AddrPC.Offset		= currentContext.StIIP;
	stackFrame.AddrPC.Mode			= AddrModeFlat;
	stackFrame.AddrFrame.Offset		= currentContext.IntSp;
	stackFrame.AddrFrame.Mode		= AddrModeFlat;
	stackFrame.AddrBStore.Offset	= currentContext.RsBSP;
	stackFrame.AddrBStore.Mode		= AddrModeFlat;
	stackFrame.AddrStack.Offset		= currentContext.IntSp;
	stackFrame.AddrStack.Mode		= AddrModeFlat;
#else
	#error "unsupported platform"
#endif
	
	// Walk up the stack and save off the PC in each frame.
	
	for( n = 0; n < countof( stack ); ++n )
	{
		good = gStachWalkFunc( machineType, currentProcess, currentThread, &stackFrame, &currentContext, 
			NULL, gSymFunctionTableAccess64Func, gSymGetModuleBase64Func, NULL );
		if( !good ) break;
		
		if( stackFrame.AddrPC.Offset != 0 )
		{
			stack[ n ].addr = stackFrame.AddrPC.Offset;
		}
		else
		{
			break;
		}
	}
	
	// Get the function name at each level.
	
	for( i = 0; i < n; ++i )
	{
		SYMBOL_INFO_PACKAGE		symbolInfo;
		DWORD64					displacement;
		
		j = ( n - i ) - 1;
		levelPtr = &stack[ j ];
		memset( &symbolInfo.si, 0, sizeof( symbolInfo.si ) );
		symbolInfo.si.SizeOfStruct = sizeof( symbolInfo.si );
		symbolInfo.si.MaxNameLen = (ULONG) sizeof( symbolInfo.name );
		good = gSymFromAddrFunc( currentProcess, levelPtr->addr, &displacement, &symbolInfo.si );
		if( good )
		{
			len = strlen( symbolInfo.si.Name );
			len = Min( len, sizeof( levelPtr->name ) - 1 );
			memcpy( levelPtr->name, symbolInfo.si.Name, len );
			levelPtr->name[ len ] = '\0';
		}
		else
		{
			levelPtr->name[ 0 ] = '\0';
		}
	}
	
	LeaveCriticalSection( &gDbgHelpLock );

	// Skip unimportant functions.
	
	for( ; n > 0; --n )
	{
		levelPtr = &stack[ n - 1 ];
		if( strcmp( levelPtr->name, "RtlInitializeExceptionChain" )	== 0 ) continue;
		if( strcmp( levelPtr->name, "BaseThreadInitThunk" )			== 0 ) continue;
		if( strcmp( levelPtr->name, "mainCRTStartup" )				== 0 ) continue;
		if( strcmp( levelPtr->name, "__tmainCRTStartup" )			== 0 ) continue;
		if( strcmp( levelPtr->name, "wWinMainCRTStartup" )			== 0 ) continue;
		break;
	}
	
	// Print each function from top (first called) to bottom (deepest).
	
	for( i = 0; i < n; ++i )
	{
		j = ( n - i ) - 1;
		levelPtr = &stack[ j ];
		if( *levelPtr->name != '\0' )
		{
			dbs_ulog( inLevel, "%2d %*s %s\n", j, (int) i, "", levelPtr->name );
		}
		else
		{
			dbs_ulog( inLevel, "%2d %*s %p ???\n", j, (int) i, "", (uintptr_t) levelPtr->addr );
		}
	}
	err = kNoErr;
	goto exit2;
	
exit:
	if( err && gDbgHelpDLL )
	{
		FreeLibrary( gDbgHelpDLL );
		gDbgHelpDLL = NULL;
	}
	LeaveCriticalSection( &gDbgHelpLock );
	
exit2:
	return( err );
}
#endif // TARGET_OS_WINDOWS

//===========================================================================================================================
//	DebugValidPtr
//===========================================================================================================================

int	DebugValidPtr( uintptr_t inPtr, size_t inSize, int inRead, int inWrite, int inExecute )
{
#if  ( TARGET_OS_WINDOWS )

	if( ( inRead	&& IsBadReadPtr(  (const void *) inPtr, inSize ) ) ||
		( inWrite	&& IsBadWritePtr( (      void *) inPtr, inSize ) ) ||
		( inExecute	&& IsBadCodePtr(  (FARPROC)      inPtr ) ) )
	{
		return( 0 );
	}
	return( 1 );

#else
	(void) inPtr;		// Unused
	(void) inSize;		// Unused
	(void) inRead;		// Unused
	(void) inWrite;		// Unused
	(void) inExecute;	// Unused
	
	return( 1 );
#endif
}

#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
//===========================================================================================================================
//	DebugWinEnableConsole
//===========================================================================================================================

#pragma warning( disable:4311 )

static void	DebugWinEnableConsole( void )
{
	static int		sConsoleEnabled = false;
	BOOL			result;
	int				fileHandle;
	FILE *			file;
	int				err;
	
	if( sConsoleEnabled ) goto exit;
	
	// Create console window.
	
	result = AllocConsole();
	require_quiet( result, exit );

	// Redirect stdin to the console stdin.
	
	fileHandle = _open_osfhandle( (intptr_t) GetStdHandle( STD_INPUT_HANDLE ), _O_TEXT );
	
	#if( defined( __MWERKS__ ) )
		file = __handle_reopen( (unsigned long) fileHandle, "r", stdin );
		require_quiet( file, exit );
	#else
		file = _fdopen( fileHandle, "r" );
		require_quiet( file, exit );
	
		*stdin = *file;
	#endif
	
	err = setvbuf( stdin, NULL, _IONBF, 0 );
	require_noerr_quiet( err, exit );
	
	// Redirect stdout to the console stdout.
	
	fileHandle = _open_osfhandle( (intptr_t) GetStdHandle( STD_OUTPUT_HANDLE ), _O_TEXT );
	
	#if( defined( __MWERKS__ ) )
		file = __handle_reopen( (unsigned long) fileHandle, "w", stdout );
		require_quiet( file, exit );
	#else
		file = _fdopen( fileHandle, "w" );
		require_quiet( file, exit );
		
		*stdout = *file;
	#endif
	
	err = setvbuf( stdout, NULL, _IONBF, 0 );
	require_noerr_quiet( err, exit );
	
	// Redirect stderr to the console stdout.
	
	fileHandle = _open_osfhandle( (intptr_t) GetStdHandle( STD_OUTPUT_HANDLE ), _O_TEXT );
	
	#if( defined( __MWERKS__ ) )
		file = __handle_reopen( (unsigned long) fileHandle, "w", stderr );
		require_quiet( file, exit );
	#else
		file = _fdopen( fileHandle, "w" );
		require_quiet( file, exit );
	
		*stderr = *file;
	#endif
	
	err = setvbuf( stderr, NULL, _IONBF, 0 );
	require_noerr_quiet( err, exit );
	
	sConsoleEnabled = true;
	
exit:
	return;
}

#pragma warning( default:4311 )

#endif // TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	DebugWinCharToTCharString
//===========================================================================================================================

static TCHAR *
	DebugWinCharToTCharString( 
		const char *	inCharString, 
		size_t 			inCharCount, 
		TCHAR *			outTCharString, 
		size_t 			inTCharCountMax, 
		size_t *		outTCharCount )
{
	const char *		src;
	TCHAR *				dst;
	TCHAR *				end;
	
	if( inCharCount == kSizeCString ) inCharCount = strlen( inCharString );
	src = inCharString;
	dst = outTCharString;
	if( inTCharCountMax > 0 )
	{
		inTCharCountMax -= 1;
		if( inTCharCountMax > inCharCount ) inTCharCountMax = inCharCount;
		
		end = dst + inTCharCountMax;
		while( dst < end ) *dst++ = (TCHAR) *src++;
		*dst = 0;
	}
	if( outTCharCount ) *outTCharCount = (size_t)( dst - outTCharString );
	return( outTCharString );
}
#endif

//===========================================================================================================================
//	DebugIsDebuggerPresent
//===========================================================================================================================

int	DebugIsDebuggerPresent( void )
{
#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
	return( IsDebuggerPresent() );
#else
	return( 0 );
#endif
}

//===========================================================================================================================
//	DebugEnterDebugger
//===========================================================================================================================

void	DebugEnterDebugger( void )
{
#if( TARGET_OS_WINDOWS )
	__debugbreak();
#else
	// $$$ TO DO: Think of something better to do in the unknown target case.
#endif
}

#if 0
#pragma mark -
#endif

#if( TARGET_OS_BSD )
//===========================================================================================================================
//	GetProcessNameByPID
//===========================================================================================================================

char *	GetProcessNameByPID( pid_t inPID, char *inNameBuf, size_t inMaxLen )
{
	OSStatus				err;
	int						mib[ 4 ];
	struct kinfo_proc		info;
	size_t					len;
	
	if( inMaxLen < 1 ) return( "" );
	
	mib[ 0 ] = CTL_KERN;
	mib[ 1 ] = KERN_PROC;
	mib[ 2 ] = KERN_PROC_PID;
	mib[ 3 ] = (int) inPID;
	
	memset( &info, 0, sizeof( info ) );
	len = sizeof( info );
	err = sysctl( mib, 4, &info, &len, NULL, 0 );
	err = map_global_noerr_errno( err );
	if( !err )	strlcpy( inNameBuf, info.kp_proc.p_comm, inMaxLen );
	else		*inNameBuf = '\0';
	return( inNameBuf );
}
#endif

#if( TARGET_OS_LINUX )
//===========================================================================================================================
//	GetProcessNameByPID
//===========================================================================================================================

char *	GetProcessNameByPID( pid_t inPID, char *inNameBuf, size_t inMaxLen )
{
	char		path[ PATH_MAX ];
	FILE *		file;
	char *		ptr;
	size_t		len;
	
	if( inMaxLen < 1 ) return( "" );
	
	snprintf( path, sizeof( path ), "/proc/%lld/cmdline", (long long) inPID );
	file = fopen( path, "r" );
	*path = '\0';
	if( file )
	{
		ptr = fgets( path, sizeof( path ), file );
		if( !ptr ) *path = '\0';
		fclose( file );
	}
	ptr = strrchr( path, '/' );
	ptr = ptr ? ( ptr + 1 ) : path;
	len = strlen( ptr );
	if( len >= inMaxLen ) len = inMaxLen - 1;
	memcpy( inNameBuf, ptr, len );
	inNameBuf[ len ] = '\0';
	return( inNameBuf );
}
#endif

#if( TARGET_OS_QNX )
//===========================================================================================================================
//	GetProcessNameByPID
//===========================================================================================================================

char *	GetProcessNameByPID( pid_t inPID, char *inNameBuf, size_t inMaxLen )
{
	OSStatus		err;
	char			path[ PATH_MAX + 1 ];
	int				fd;
	char *			ptr;
	size_t			len;
	struct
	{
		procfs_debuginfo	info;
		char				buf[ PATH_MAX ];
		
	}	name;
	
	if( inMaxLen < 1 ) return( "" );
	
	snprintf( path, sizeof( path ), "/proc/%lld/as", (long long) inPID );
	fd = open( path, O_RDONLY | O_NONBLOCK );
	err = map_fd_creation_errno( fd );
	check_noerr( err );
	if( !err )
	{
		err = devctl( fd, DCMD_PROC_MAPDEBUG_BASE, &name, sizeof( name ), NULL );
		check_noerr( err );
		close( fd );
	}
	if( !err )
	{
		ptr = strrchr( name.info.path, '/' );
		ptr = ptr ? ( ptr + 1 ) : path;
		len = strlen( ptr );
	}
	else
	{
		ptr = "?";
		len = 1;
	}
	if( len >= inMaxLen ) len = inMaxLen - 1;
	memcpy( inNameBuf, ptr, len );
	inNameBuf[ len ] = '\0';
	return( inNameBuf );
}
#endif

//===========================================================================================================================
//	HTTPGetReasonPhrase
//===========================================================================================================================

#define CASE_REASON_PHRASE( NUM, STR )		case NUM: reasonPhrase = STR; break

const char *	HTTPGetReasonPhrase( int inStatusCode )
{
	const char *		reasonPhrase;
	
	switch( inStatusCode )
	{
		// Information 1xx
		
		CASE_REASON_PHRASE( 100, "Continue" );
		CASE_REASON_PHRASE( 101, "Switching Protocols" );
		CASE_REASON_PHRASE( 102, "Processing" );
		CASE_REASON_PHRASE( 103, "Checkpoint" );
		
		// Successfull 2xx
		
		CASE_REASON_PHRASE( 200, "OK" );
		CASE_REASON_PHRASE( 201, "Created" );
		CASE_REASON_PHRASE( 202, "Accepted" );
		CASE_REASON_PHRASE( 203, "Non-Authoritative Information" );
		CASE_REASON_PHRASE( 204, "No Content" );
		CASE_REASON_PHRASE( 205, "Reset Content" );
		CASE_REASON_PHRASE( 206, "Partial Content" );
		CASE_REASON_PHRASE( 207, "Multi-Status" );
		CASE_REASON_PHRASE( 208, "Already Reported" );
		CASE_REASON_PHRASE( 210, "Content Different" );
		CASE_REASON_PHRASE( 226, "IM Used" );
		CASE_REASON_PHRASE( 250, "Low on Storage Space" );
		
		// Redirection 3xx
		
		CASE_REASON_PHRASE( 300, "Multiple Choices" );
		CASE_REASON_PHRASE( 301, "Moved Permanently" );
		CASE_REASON_PHRASE( 302, "Found" );
		CASE_REASON_PHRASE( 303, "See Other" );
		CASE_REASON_PHRASE( 304, "Not Modified" );
		CASE_REASON_PHRASE( 305, "Use Proxy" );
		CASE_REASON_PHRASE( 306, "Switch Proxy" ); // No longer used.
		CASE_REASON_PHRASE( 307, "Temporary Redirect" );
		CASE_REASON_PHRASE( 308, "Resume Incomplete" );
		CASE_REASON_PHRASE( 330, "Moved Location" );
		CASE_REASON_PHRASE( 350, "Going Away" );
		CASE_REASON_PHRASE( 351, "Load Balancing" );
		
		// Client Error 4xx
		
		CASE_REASON_PHRASE( 400, "Bad Request" );
		CASE_REASON_PHRASE( 401, "Unauthorized" );
		CASE_REASON_PHRASE( 402, "Payment Required" );
		CASE_REASON_PHRASE( 403, "Forbidden" );
		CASE_REASON_PHRASE( 404, "Not Found" );
		CASE_REASON_PHRASE( 405, "Method Not Allowed" );
		CASE_REASON_PHRASE( 406, "Not Acceptable" );
		CASE_REASON_PHRASE( 407, "Proxy Authentication Required" );
		CASE_REASON_PHRASE( 408, "Request Timeout" );
		CASE_REASON_PHRASE( 409, "Conflict" );
		CASE_REASON_PHRASE( 410, "Gone" );
		CASE_REASON_PHRASE( 411, "Length Required" );
		CASE_REASON_PHRASE( 412, "Precondition Failed" );
		CASE_REASON_PHRASE( 413, "Request Entity Too Large" );
		CASE_REASON_PHRASE( 414, "Request-URI Too Long" );
		CASE_REASON_PHRASE( 415, "Unsupported Media Type" );
		CASE_REASON_PHRASE( 416, "Requested Range Not Satisfiable" );
		CASE_REASON_PHRASE( 417, "Expectation Failed" );
		CASE_REASON_PHRASE( 418, "I'm a teapot" );
		CASE_REASON_PHRASE( 420, "Enhance Your Calm" );
		CASE_REASON_PHRASE( 422, "Unprocessable Entity" );
		CASE_REASON_PHRASE( 423, "Expectation Failed" );
		CASE_REASON_PHRASE( 424, "Failed Dependency" );
		CASE_REASON_PHRASE( 425, "Unordered Collection" );
		CASE_REASON_PHRASE( 426, "Upgrade Required" );
		CASE_REASON_PHRASE( 428, "Precondition Required" );
		CASE_REASON_PHRASE( 429, "Too Many Requests" );
		CASE_REASON_PHRASE( 431, "Request Header Fields Too Large" );
		CASE_REASON_PHRASE( 444, "No Response" );
		CASE_REASON_PHRASE( 449, "Retry With" );
		CASE_REASON_PHRASE( 450, "Blocked by Parental Controls" );
		CASE_REASON_PHRASE( 451, "Parameter Not Understood" );
		CASE_REASON_PHRASE( 452, "Conference Not Found" );
		CASE_REASON_PHRASE( 453, "Not Enough Bandwidth" );
		CASE_REASON_PHRASE( 454, "Session Not Found" );
		CASE_REASON_PHRASE( 455, "Method Not Valid In This State" );
		CASE_REASON_PHRASE( 456, "Header Field Not Valid" );
		CASE_REASON_PHRASE( 457, "Invalid Range" );
		CASE_REASON_PHRASE( 458, "Parameter Is Read-Only" );
		CASE_REASON_PHRASE( 459, "Aggregate Operation Not Allowed" );
		CASE_REASON_PHRASE( 460, "Only Aggregate Operation Allowed" );
		CASE_REASON_PHRASE( 461, "Unsupported Transport" );
		CASE_REASON_PHRASE( 462, "Destination Unreachable" );
		CASE_REASON_PHRASE( 463, "Destination Prohibited" );
		CASE_REASON_PHRASE( 464, "Data Transport Not Ready Yet" );
		CASE_REASON_PHRASE( 465, "Notification Reason Unknown" );
		CASE_REASON_PHRASE( 466, "Key Management Error" );
		CASE_REASON_PHRASE( 470, "Connection Authorization Required" );
		CASE_REASON_PHRASE( 471, "Connection Credentials not accepted" );
		CASE_REASON_PHRASE( 472, "Failure to establish secure connection" );
		CASE_REASON_PHRASE( 475, "Invalid collblob" );
		CASE_REASON_PHRASE( 499, "Client Closed Request" );
		
		// Server Error 5xx
		
		CASE_REASON_PHRASE( 500, "Internal Server Error" );
		CASE_REASON_PHRASE( 501, "Not Implemented" );
		CASE_REASON_PHRASE( 502, "Bad Gateway" );
		CASE_REASON_PHRASE( 503, "Service Unavailable" );
		CASE_REASON_PHRASE( 504, "Gateway Time-out" );
		CASE_REASON_PHRASE( 505, "Version Not Supported" );
		CASE_REASON_PHRASE( 506, "Variant Also Negotiates" );
		CASE_REASON_PHRASE( 507, "Insufficient Storage" );
		CASE_REASON_PHRASE( 508, "Loop Detected" );
		CASE_REASON_PHRASE( 509, "Bandwidth Limit Exceeded" );
		CASE_REASON_PHRASE( 510, "Not Extended" );
		CASE_REASON_PHRASE( 511, "Network Authentication Required" );
		CASE_REASON_PHRASE( 551, "Option Not Supported" );
		CASE_REASON_PHRASE( 598, "Network Read Timeout" );
		CASE_REASON_PHRASE( 599, "Network Connect Timeout" );
		
		default:
			reasonPhrase = "";
			break;
	}
	return( reasonPhrase );
}

//===========================================================================================================================
//	ReportCriticalError
//===========================================================================================================================

void	ReportCriticalError( const char *inReason, uint32_t inExceptionCode, Boolean inCrashLog )
{
	(void) inReason;
	(void) inExceptionCode;
	(void) inCrashLog;
	
	// TO DO: Figure out what to do on other platforms.
}

//===========================================================================================================================
//	RollLogFiles
//===========================================================================================================================

#if( TARGET_HAS_C_LIB_IO )
OSStatus	RollLogFiles( FILE **ioLogFile, const char *inEndMessage, const char *inBaseName, int inMaxFiles )
{
	OSStatus		err;
	char			oldPath[ PATH_MAX + 1 ];
	char			newPath[ PATH_MAX + 1 ];
	int				i;
	
	// Append a message to the current log file so viewers know it has rolled then close it.
	
	if( ioLogFile && *ioLogFile )
	{
		if( inEndMessage ) fprintf( *ioLogFile, "%s", inEndMessage );
		fclose( *ioLogFile );
		*ioLogFile = NULL;
	}
	
	// Delete the oldest log file.
	
	snprintf( oldPath, sizeof( oldPath ), "%s.%d", inBaseName, inMaxFiles - 1 );
	remove( oldPath );
	
	// Shift all the log files down by 1.
	
	for( i = inMaxFiles - 2; i > 0; --i )
	{
		snprintf( oldPath, sizeof( oldPath ), "%s.%d", inBaseName, i );
		snprintf( newPath, sizeof( newPath ), "%s.%d", inBaseName, i + 1 );
		rename( oldPath, newPath );
	}
	if( inMaxFiles > 1 )
	{
		snprintf( newPath, sizeof( newPath ), "%s.%d", inBaseName, 1 );
		rename( inBaseName, newPath );
	}
	
	// Open a new, empty log file to continue logging.
	
	if( ioLogFile )
	{
		*ioLogFile = fopen( inBaseName, "w" );
		err = map_global_value_errno( *ioLogFile, *ioLogFile );
		require_noerr_quiet( err, exit );
	}
	err = kNoErr;
	
exit:
	return( err );
}
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

#if 0
#pragma mark -
#pragma mark == PrintF ==
#endif

//===========================================================================================================================
//	Structures
//===========================================================================================================================

typedef struct
{
	unsigned		leftJustify:1;
	unsigned		forceSign:1;
	unsigned		zeroPad:1;
	unsigned		havePrecision:1;
	unsigned		suppress:1;
	char			hSize;
	char			lSize;
	char			altForm;
	char			sign; // +, -, or space
	unsigned int	fieldWidth;
	size_t			precision;
	char			group;
	char			prefix;
	char			suffix;
	
}	PrintFFormat;

static const PrintFFormat			kPrintFFormatDefault = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct	PrintFContext		PrintFContext;

typedef int	( *PrintFCallBack )( const char *inStr, size_t inSize, PrintFContext *inContext );

struct	PrintFContext
{
	PrintFCallBack			callback;
	char *					str;
	size_t					usedSize;
	size_t					reservedSize;
	
	PrintFUserCallBack		userCallBack;
	void *					userContext;
};

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static int		PrintFCore( PrintFContext *inContext, const char *inFormat, ... );
static int		PrintFCoreVAList( PrintFContext *inContext, const char *inFormat, va_list inArgs );

#if( TYPE_LONGLONG_NATIVE )
	#define	DIV64x32( VALUE, DIVISOR, REMAIN )						\
		do															\
		{															\
			if( VALUE <= UINT64_C( 0xFFFFFFFF ) )					\
			{														\
				REMAIN = ( (uint32_t)( VALUE ) ) % ( DIVISOR );		\
				VALUE  = ( (uint32_t)( VALUE ) ) / ( DIVISOR );		\
			}														\
			else													\
			{														\
				REMAIN = (uint32_t)( VALUE % ( DIVISOR ) );			\
				VALUE /= ( DIVISOR );								\
			}														\
																	\
		}	while( 0 )
#else
	#define	DIV64x32( VALUE, DIVISOR, REMAIN )	do { PrintFDiv64x32( &VALUE, ( DIVISOR ), &REMAIN ); } while( 0 )
	
	static void	PrintFDiv64x32( uint64_t *ioValue, uint32_t inDivisor, uint32_t *outRemainder );
#endif

static int		PrintFWriteAddr( const uint8_t *inAddr, PrintFFormat *inFormat, char *outStr );
static int		PrintFWriteAudioStreamBasicDescription( PrintFContext *inContext, const AudioStreamBasicDescription *inASBD );
static int		PrintFWriteBits( uint64_t inX, PrintFFormat *inFormat, char *outStr );
#if( DEBUG_CF_OBJECTS_ENABLED )
	static int	PrintFWriteCFObject( PrintFContext *inContext, PrintFFormat *inFormat, CFTypeRef inObj, char *inBuffer );
#endif
#if( DEBUG_CF_OBJECTS_ENABLED && ( !CFLITE_ENABLED || CFL_XML ) )
	static int	PrintFWriteCFXMLObject( PrintFContext *inContext, PrintFFormat *inFormat, CFTypeRef inObj );
#endif
static int
	PrintFWriteHex( 
		PrintFContext *	inContext, 
		PrintFFormat *	inFormat, 
		int				inIndent, 
		const void *	inData, 
		size_t			inSize, 
		size_t			inMaxSize );
static int		PrintFWriteHexOneLine( PrintFContext *inContext, PrintFFormat *inFormat, const uint8_t *inData, size_t inSize );
static int		PrintFWriteHexByteStream( PrintFContext *inContext, const uint8_t *inData, size_t inSize );
static int		PrintFWriteIE( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen );
static int		PrintFWriteIE_WPS( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen );
static int		PrintFWriteMultiLineText( PrintFContext *inContext, PrintFFormat *inFormat, const char *inStr, size_t inLen );
static int		PrintFWriteNumVersion( uint32_t inVersion, char *outStr );
static int		PrintFWriteSingleLineText( PrintFContext *inContext, const char *inStr, size_t inLen );
static int		PrintFWriteString( const char *inStr, PrintFFormat *inFormat, char *inBuf, const char **outStrPtr );
static int		PrintFWriteText( PrintFContext *inContext, PrintFFormat *inFormat, const char *inText, size_t inSize );
static int		PrintFWriteTimeDuration( uint64_t inSeconds, int inAltMode, char *inBuf );
static int		PrintFWriteTXTRecord( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen );
static int		PrintFWriteUnicodeString( const uint8_t *inStr, PrintFFormat *inFormat, char *inBuf );

#define	print_indent( CONTEXT, N )	PrintFCore( ( CONTEXT ), "%*s", (int)( ( N ) * 4 ), "" )

static int		PrintFCallBackFixedString( const char *inStr, size_t inSize, PrintFContext *inContext );
static int		PrintFCallBackAllocatedString( const char *inStr, size_t inSize, PrintFContext *inContext );
static int		PrintFCallBackUserCallBack( const char *inStr, size_t inSize, PrintFContext *inContext );
static char *	PrintF_IPv6AddressToCString( const uint8_t inAddr[ 16 ], uint32_t inScope, int inPort, char *inBuffer );
static char *	PrintF_IPv4AddressToCString( uint32_t inIP, int inPort, char *inBuffer );

//===========================================================================================================================
//	SNPrintF
//===========================================================================================================================

int	SNPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = VSNPrintF( inBuf, inMaxLen, inFormat, args );
	va_end( args );
	return( n );
}

//===========================================================================================================================
//	VSNPrintF
//===========================================================================================================================

int	VSNPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, va_list inArgs )
{
	int					n;
	PrintFContext		context;
	
	context.callback		= PrintFCallBackFixedString;
	context.str		 		= (char *) inBuf;
	context.usedSize		= 0;
	context.reservedSize	= ( inMaxLen > 0 ) ? inMaxLen - 1 : 0;
	
	n = PrintFCoreVAList( &context, inFormat, inArgs );
	if( inMaxLen > 0 ) *( context.str + context.usedSize ) = '\0';
	return( n );
}

//===========================================================================================================================
//	SNPrintF_Add
//===========================================================================================================================

OSStatus	SNPrintF_Add( char **ioPtr, char *inEnd, const char *inFormat, ... )
{
	char * const		ptr = *ioPtr;
	size_t				len;
	int					n;
	va_list				args;
	
	len = (size_t)( inEnd - ptr );
	require_action( len > 0, exit, n = kNoSpaceErr );
	
	va_start( args, inFormat );
	n = VSNPrintF( ptr, len, inFormat, args );
	va_end( args );
	require( n >= 0, exit );
	if( n >= ( (int) len ) )
	{
		dlogassert( "Add '%s' format failed due to lack of space (%d vs %zu)", inFormat, n, len );
		*ioPtr = inEnd;
		n = kOverrunErr;
		goto exit;
	}
	*ioPtr = ptr + n;
	n = kNoErr;
	
exit:
	return( n );
}

//===========================================================================================================================
//	AppendPrintF
//===========================================================================================================================

int	AppendPrintF( char **ioStr, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	char *		tempStr;
	
	va_start( args, inFormat );
	n = ASPrintF( &tempStr, "%s%V", *ioStr ? *ioStr : "", inFormat, &args );
	va_end( args );
	require_quiet( n >= 0, exit );
	
	if( *ioStr ) free( *ioStr );
	*ioStr = tempStr;
	
exit:
	return( n );
}

//===========================================================================================================================
//	ASPrintF
//===========================================================================================================================

int	ASPrintF( char **outStr, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = VASPrintF( outStr, inFormat, args );
	va_end( args );
	return( n );
}

//===========================================================================================================================
//	VASPrintF
//===========================================================================================================================

int	VASPrintF( char **outStr, const char *inFormat, va_list inArgs )
{
	int					n;
	PrintFContext		context;
	int					tmp;
	
	context.callback		= PrintFCallBackAllocatedString;
	context.str		 		= NULL;
	context.usedSize		= 0;
	context.reservedSize	= 0;
	
	n = PrintFCoreVAList( &context, inFormat, inArgs );
	if( n >= 0 )
	{
		tmp = context.callback( "", 1, &context );
		if( tmp < 0 ) n = tmp;
	}
	if( n >= 0 ) *outStr = context.str;
	else if( context.str ) free( context.str );
	return( n );
}

//===========================================================================================================================
//	CPrintF
//===========================================================================================================================

int	CPrintF( PrintFUserCallBack inCallBack, void *inContext, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = VCPrintF( inCallBack, inContext, inFormat, args );
	va_end( args );
	return( n );
}

//===========================================================================================================================
//	VCPrintF
//===========================================================================================================================

int	VCPrintF( PrintFUserCallBack inCallBack, void *inContext, const char *inFormat, va_list inArgs )
{
	int					n;
	PrintFContext		context;
	int					tmp;
	
	context.callback		= PrintFCallBackUserCallBack;
	context.str		 		= NULL;
	context.usedSize		= 0;
	context.reservedSize	= 0;
	context.userCallBack	= inCallBack;
	context.userContext		= inContext;
	
	n = PrintFCoreVAList( &context, inFormat, inArgs );
	if( n >= 0 )
	{
		tmp = context.callback( "", 0, &context );
		if( tmp < 0 ) n = tmp;
	}
	return( n );
}

#if( TARGET_HAS_C_LIB_IO )

//===========================================================================================================================
//	FPrintF
//===========================================================================================================================

int	FPrintF( FILE *inFile, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = VFPrintF( inFile, inFormat, args );
	va_end( args );
	
	return( n );
}

//===========================================================================================================================
//	VFPrintF
//===========================================================================================================================

static int	FPrintFCallBack( const char *inStr, size_t inSize, void *inContext );

int	VFPrintF( FILE *inFile, const char *inFormat, va_list inArgs )
{
	return( VCPrintF( FPrintFCallBack, inFile, inFormat, inArgs ) );
}

//===========================================================================================================================
//	FPrintFCallBack
//===========================================================================================================================

static int	FPrintFCallBack( const char *inStr, size_t inSize, void *inContext )
{
	FILE * const		file = (FILE *) inContext;
	
	if( file ) fwrite( inStr, 1, inSize, file );
	return( (int) inSize );
}

#endif // TARGET_HAS_C_LIB_IO

//===========================================================================================================================
//	MemPrintF
//===========================================================================================================================

int	MemPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = VMemPrintF( inBuf, inMaxLen, inFormat, args );
	va_end( args );
	return( n );
}

//===========================================================================================================================
//	VMemPrintF
//===========================================================================================================================

int	VMemPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, va_list inArgs )
{
	int					n;
	PrintFContext		context;
	
	context.callback		= PrintFCallBackFixedString;
	context.str		 		= (char *) inBuf;
	context.usedSize		= 0;
	context.reservedSize	= inMaxLen;
	
	n = PrintFCoreVAList( &context, inFormat, inArgs );
	return( n );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	PrintFCore
//===========================================================================================================================

static int	PrintFCore( PrintFContext *inContext, const char *inFormat, ... )
{
	int			n;
	va_list		args;
	
	va_start( args, inFormat );
	n = PrintFCoreVAList( inContext, inFormat, args );
	va_end( args );
	return( n );
}

//===========================================================================================================================
//	PrintFCoreVAList
//===========================================================================================================================

#define	kPrintFBufSize				300 // Enough space for a 256-byte domain name and some error text.

#define PrintFIsPrintable( C )		( ( ( C ) >= 0x20 ) && ( ( C ) < 0x7F ) )
#define PrintFMakePrintable( C )	( (char)( PrintFIsPrintable( ( C ) ) ? ( C ) : '^' ) )

static int	PrintFCoreVAList( PrintFContext *inContext, const char *inFormat, va_list inArgs )
{
	int								nTotal;
	const char *					fmt;
	const char *					src;
	int								err;
	char							buf[ kPrintFBufSize ];
	char *							p;
	int								c;
	PrintFFormat					F;
	int								i;
	const char *					digits;
	const char *					s;
	const uint8_t *					a;
	int								n;
	size_t							size;
	size_t							sizeMax;
	uint64_t						x;
	
	nTotal = 0;
	fmt = inFormat;
	for( c = *fmt; ; c = *++fmt )
	{
		// Non-conversion characters are copied directly to the output.
		
		src = fmt;
		while( ( c != '\0' ) && ( c != '%' ) ) c = *++fmt;
		if( fmt != src )
		{
			i = (int)( fmt - src );
			err = inContext->callback( src, (size_t) i, inContext );
			if( err < 0 ) goto error;
			nTotal += i;
		}
		if( c == '\0' ) break;
		
		F = kPrintFFormatDefault;
		
		// Flags
		
		for( ;; )
		{
			c = *++fmt;
			if(      c == '-' )  F.leftJustify	= 1;
			else if( c == '+' )  F.forceSign	= 1;
			else if( c == ' ' )  F.sign			= ' ';
			else if( c == '#' )  F.altForm 	   += 1;
			else if( c == '0' )  F.zeroPad		= 1;
			else if( c == '\'' ) F.group	   += 1;
			else if( c == '?' )  F.suppress		= !va_arg( inArgs, int );
			else break;
		}
		
		// Field Width
		
		if( c == '*' )
		{
			i = va_arg( inArgs, int );
			if( i < 0 )
			{
				i = -i;
				F.leftJustify = 1;
			}
			F.fieldWidth = (unsigned int) i;
			c = *++fmt;
		}
		else
		{
			for( ; ( c >= '0' ) && ( c <= '9' ); c = *++fmt )
			{
				F.fieldWidth = ( 10 * F.fieldWidth ) + ( c - '0' );
			}
		}
		
		// Precision
		
		if( c == '.' )
		{
			c = *++fmt;
			if( c == '*' )
			{
				F.precision = va_arg( inArgs, unsigned int );
				c = *++fmt;
			}
			else
			{
				for( ; ( c >= '0' ) && ( c <= '9' ); c = *++fmt )
				{
					F.precision = ( 10 * F.precision ) + ( c - '0' );
				}
			}
			F.havePrecision = 1;
		}
		if( F.leftJustify ) F.zeroPad = 0;
		
		// Length modifiers
		
		for( ;; )
		{
			if(      c == 'h' ) { ++F.hSize; c = *++fmt; }
			else if( c == 'l' ) { ++F.lSize; c = *++fmt; }
			else if( c == 'j' )
			{
				if( F.hSize || F.lSize ) { err = -1; goto error; }
				if(      sizeof( intmax_t ) == sizeof( long ) )		F.lSize = 1;
				else if( sizeof( intmax_t ) == sizeof( int64_t ) )	F.lSize = 2;
				else												F.lSize = 0;
				c = *++fmt;
				break;
			}
			else if( c == 'z' )
			{
				if( F.hSize || F.lSize ) { err = -1; goto error; };
				if(      sizeof( size_t ) == sizeof( long ) )		F.lSize = 1;
				else if( sizeof( size_t ) == sizeof( int64_t ) )	F.lSize = 2;
				else												F.lSize = 0;
				c = *++fmt;
				break;
			}
			else if( c == 't' )
			{
				if( F.hSize || F.lSize ) { err = -1; goto error; };
				if(      sizeof( ptrdiff_t ) == sizeof( long ) )	F.lSize = 1;
				else if( sizeof( ptrdiff_t ) == sizeof( int64_t ) )	F.lSize = 2;
				else												F.lSize = 0;
				c = *++fmt;
				break;
			}
			else break;
		}
		if( F.hSize > 2 )		 { err = -1; goto error; };
		if( F.lSize > 2 )		 { err = -1; goto error; };
		if( F.hSize && F.lSize ) { err = -1; goto error; };
		
		// Conversions
		
		digits = kHexDigitsUppercase;
		switch( c )
		{
			unsigned int					base;
			uint32_t						remain;
			
			// %d, %i, %u, %o, %b, %x, %X, %p: Number
			
			case 'd':
			case 'i': base = 10; goto canBeSigned;
			case 'u': base = 10; goto notSigned;
			case 'o': base =  8; goto notSigned;
			case 'b': base =  2; goto notSigned;
			case 'x': digits = kHexDigitsLowercase;
			case 'X': base = 16; goto notSigned;
			case 'p':
				x = (uintptr_t) va_arg( inArgs, void * );
				F.precision		= sizeof( void * ) * 2;
				F.havePrecision = 1;
				F.altForm		= 1;
				F.sign			= 0;
				base			= 16;
				c				= 'x';
				goto number;
			
			canBeSigned:
				if(      F.lSize == 1 )	x = (uint64_t) va_arg( inArgs, long );
				else if( F.lSize == 2 )	x = (uint64_t) va_arg( inArgs, int64_t );
				else					x = (uint64_t) va_arg( inArgs, int );
				if(      F.hSize == 1 )	x = (uint64_t)(short)( x & 0xFFFF );
				else if( F.hSize == 2 )	x = (uint64_t)(signed char)( x & 0xFF );
				if( (int64_t) x < 0 ) { x = (uint64_t)( -(int64_t) x ); F.sign = '-'; }
				else if( F.forceSign ) F.sign = '+';
				goto number;
			
			notSigned:
				if(      F.lSize == 1 )	x = va_arg( inArgs, unsigned long );
				else if( F.lSize == 2 )	x = va_arg( inArgs, uint64_t );
				else					x = va_arg( inArgs, unsigned int );
				if(      F.hSize == 1 )	x = (unsigned short)( x & 0xFFFF );
				else if( F.hSize == 2 )	x = (unsigned char)( x & 0xFF );
				F.sign = 0;
				goto number;
			
			number:
				if( F.suppress ) continue;
				if( ( base == 2 ) && ( F.altForm > 1 ) )
				{
					i = PrintFWriteBits( x, &F, buf );
					s = buf;
				}
				else
				{
					if( !F.havePrecision )
					{
						if( F.zeroPad )
						{
							unsigned int		extra;
							
							extra = 0;
							if( F.altForm )
							{
								if(      base ==  8 ) extra += 1; // Make room for the leading "0".
								else if( base != 10 ) extra += 2; // Make room for the leading "0x", "0b", etc.
							}
							if( F.sign ) extra += 1; // Make room for the leading "+" or "-".
							F.precision = ( F.fieldWidth > extra ) ? ( F.fieldWidth - extra ) : 0;
						}
						if( F.precision < 1 ) F.precision = 1;
					}
					if( F.precision > ( sizeof( buf ) - 1 ) ) F.precision = sizeof( buf ) - 1;
					
					p = buf + sizeof( buf );
					i = 0;
					if( F.group )
					{
						n = 0;
						for( ;; )
						{
							DIV64x32( x, base, remain );
							*--p = digits[ remain ]; ++i; ++n;
							if( !x ) break;
							if( F.group && ( ( n % 3 ) == 0 ) ) { *--p = ','; ++i; }
						}
					}
					else
					{
						while( x )
						{
							DIV64x32( x, base, remain );
							*--p = digits[ remain ]; ++i;
						}
					}
					for( ; i < (int) F.precision; ++i )	*--p = '0';
					if( F.altForm )
					{
						if(      base ==  8 ) {                  *--p = '0'; i += 1; }
						else if( base != 10 ) { *--p = (char) c; *--p = '0'; i += 2; }
					}
					if( F.sign ) { *--p = F.sign; ++i; }
					s = p;
				}
				break;
		
		#if( PRINTF_ENABLE_FLOATING_POINT )
			case 'f':	// %f: Floating point
			{
				char		fpFormat[ 8 ];
				double		dx;
				
				i = 0;
				fpFormat[ i++ ]						= '%';
				if( F.forceSign ) fpFormat[ i++ ]	= '+';
				if( F.altForm )   fpFormat[ i++ ]	= '#';
				fpFormat[ i++ ]						= '*';
				if( F.havePrecision )
				{
					fpFormat[ i++ ]					= '.';
					fpFormat[ i++ ]					= '*';
				}
				fpFormat[ i++ ]						= 'f';
				fpFormat[ i ]						= '\0';
				
				i = (int) F.fieldWidth;
				if( F.leftJustify ) i = -i;
				dx = va_arg( inArgs, double );
				if( F.suppress ) continue;
				if( F.havePrecision ) i = snprintf( buf, sizeof( buf ), fpFormat, i, (int) F.precision, dx );
				else				  i = snprintf( buf, sizeof( buf ), fpFormat, i, dx );
				if( i < 0 ) { err = i; goto error; }
				s = buf;
				break;
			}
		#endif
		
		#if( TARGET_OS_WINDOWS && !defined( UNICODE ) && !defined( _UNICODE ) )
			case 'T':	// %T: TCHAR string (Windows only)
		#endif
			case 's':	// %s: String
				src = va_arg( inArgs, const char * );
				if( F.suppress ) continue;
				if( !src && ( !F.havePrecision || ( F.precision != 0 ) ) ) { s = "<<NULL>>"; i = 8; break; }
				if( F.group && F.havePrecision )
				{
					if( F.precision >= 2 ) F.precision -= 2;
					else				 { F.precision  = 0; F.group = '\0'; }
				}
				i = PrintFWriteString( src, &F, buf, &s );
				if(      F.group == 1 ) { F.prefix = '\''; F.suffix = '\''; }
				else if( F.group == 2 ) { F.prefix = '"';  F.suffix = '"'; }
				break;
		
		#if( TARGET_OS_WINDOWS && ( defined( UNICODE ) || defined( _UNICODE ) ) )
			case 'T':	// %T: TCHAR string (Windows only)
		#endif
			case 'S':	// %S: Unicode String
				a = va_arg( inArgs, uint8_t * );
				if( F.suppress ) continue;
				if( !a && ( !F.havePrecision || ( F.precision != 0 ) ) ) { s = "<<NULL>>"; i = 8; break; }
				if( F.group && F.havePrecision )
				{
					if( F.precision >= 2 ) F.precision -= 2;
					else				 { F.precision  = 0; F.group = '\0'; }
				}
				i = PrintFWriteUnicodeString( a, &F, buf );
				s = buf;
				if(      F.group == 1 ) { F.prefix = '\''; F.suffix = '\''; }
				else if( F.group == 2 ) { F.prefix = '"';  F.suffix = '"'; }
				break;
		
			case '@':	// %@: Cocoa/CoreFoundation Object
				a = va_arg( inArgs, uint8_t * );
				if( F.suppress ) continue;
				
				#if( DEBUG_CF_OBJECTS_ENABLED )
				{
					CFTypeRef		cfObj;
					
					cfObj = (CFTypeRef) a;
					if( !cfObj ) cfObj = CFSTR( "<<NULL>>" );
					
					if( F.group && F.havePrecision )
					{
						if( F.precision >= 2 ) F.precision -= 2;
						else				 { F.precision  = 0; F.group = '\0'; }
					}
					if(      F.group == 1 ) { F.prefix = '\''; F.suffix = '\''; }
					else if( F.group == 2 ) { F.prefix = '"';  F.suffix = '"'; }
					
					#if( !CFLITE_ENABLED || CFL_XML )
						if( F.altForm ) err = PrintFWriteCFXMLObject( inContext, &F, cfObj );
						else
					#endif
					err = PrintFWriteCFObject( inContext, &F, cfObj, buf );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				#else
					i = SNPrintF( buf, sizeof( buf ), "<<%%@=%p WITH CF OBJECTS DISABLED>>", a );
					s = buf;
				#endif
				break;
		
			case 'm':	// %m: Error Message
			{
				OSStatus		errCode;
				
				errCode = va_arg( inArgs, OSStatus );
				if( F.suppress ) continue;
				if( F.altForm )
				{
					if( PrintFIsPrintable( ( errCode >> 24 ) & 0xFF ) && 
						PrintFIsPrintable( ( errCode >> 16 ) & 0xFF ) &&
						PrintFIsPrintable( ( errCode >>  8 ) & 0xFF ) &&
						PrintFIsPrintable(   errCode         & 0xFF ) )
					{
						if( F.altForm == 2 )
						{
							i = SNPrintF( buf, sizeof( buf ), "%-11d    0x%08X    '%C'    ", 
								(int) errCode, (unsigned int) errCode, (uint32_t) errCode );
						}
						else
						{
							i = SNPrintF( buf, sizeof( buf ), "%d/0x%X/'%C' ", 
								(int) errCode, (unsigned int) errCode, (uint32_t) errCode );
						}
					}
					else
					{
						if( F.altForm == 2 )
						{
							i = SNPrintF( buf, sizeof( buf ), "%-11d    0x%08X    '^^^^'    ", 
								(int) errCode, (unsigned int) errCode, (uint32_t) errCode );
						}
						else
						{
							i = SNPrintF( buf, sizeof( buf ), "%d/0x%X ", 
								(int) errCode, (unsigned int) errCode, (uint32_t) errCode );
						}
					}
				}
				else
				{
					#if( DEBUG || DEBUG_EXPORT_ERROR_STRINGS )
						i = 0;
					#else
						i = SNPrintF( buf, sizeof( buf ), "%d/0x%X ", (int) errCode, (unsigned int) errCode );
					#endif
				}
				#if( DEBUG || DEBUG_EXPORT_ERROR_STRINGS )
					DebugGetErrorString( errCode, &buf[ i ], sizeof( buf ) - i );
				#endif
				s = buf;
				for( i = 0; s[ i ]; ++i ) {}
				break;
			}
			
			case 'H':	// %H: Hex Dump
				a		= va_arg( inArgs, uint8_t * );
				size	= (size_t) va_arg( inArgs, int );
				sizeMax	= (size_t) va_arg( inArgs, int );
				if( F.suppress ) continue;
				if( a || ( size == 0 ) )
				{
					if( size == kSizeCString ) size = strlen( (const char *) a );
					if(      F.precision == 0 ) err = PrintFWriteHexOneLine( inContext, &F, a, Min( size, sizeMax ) );
					else if( F.precision == 1 ) err = PrintFWriteHex( inContext, &F, F.fieldWidth, a, size, sizeMax );
					else if( F.precision == 2 )
					{
						if(       size <= 0 )	err = PrintFCore( inContext, "(0 bytes)\n" );
						else if ( size <= 16 )	err = PrintFWriteHex( inContext, &F, 0, a, size, sizeMax );
						else
						{
							err = PrintFCore( inContext, "\n" );
							if( err < 0 ) goto error;
							
							err = PrintFWriteHex( inContext, &F, F.fieldWidth, a, size, sizeMax );
						}
					}
					else if( F.precision == 3 ) err = PrintFWriteHexByteStream( inContext, a, Min( size, sizeMax ) );
					else						err = PrintFCore( inContext, "<< BAD %%H PRECISION >>" );
					if( err < 0 ) goto error;
					nTotal += err;
				}
				else
				{
					err = PrintFCore( inContext, "<<NULL %zu/%zu>>", size, sizeMax );
					if( err < 0 ) goto error;
					nTotal += err;
				}
				continue;
			
			case 'c':	// %c: Character
				c = va_arg( inArgs, int );
				if( F.suppress ) continue;
				if( F.group )
				{
					buf[ 0 ] = '\'';
					buf[ 1 ] = PrintFMakePrintable( c );
					buf[ 2 ] = '\'';
					i = 3;
				}
				else
				{
					buf[ 0 ] = (char) c;
					i = 1;
				}
				s = buf;
				break;
			
			case 'C':	// %C: FourCharCode
				x = va_arg( inArgs, uint32_t );
				if( F.suppress ) continue;
				i = 0;
				if( F.group ) buf[ i++ ] = '\'';
				buf[ i ] = (char)( ( x >> 24 ) & 0xFF ); buf[ i ] = PrintFMakePrintable( buf[ i ] ); ++i;
				buf[ i ] = (char)( ( x >> 16 ) & 0xFF ); buf[ i ] = PrintFMakePrintable( buf[ i ] ); ++i;
				buf[ i ] = (char)( ( x >>  8 ) & 0xFF ); buf[ i ] = PrintFMakePrintable( buf[ i ] ); ++i;
				buf[ i ] = (char)(   x         & 0xFF ); buf[ i ] = PrintFMakePrintable( buf[ i ] ); ++i;
				if( F.group ) buf[ i++ ] = '\'';
				s = buf;
				break;
			
			case 'a':	// %a: Address
				a = va_arg( inArgs, const uint8_t * );
				if( F.suppress ) continue;
				if( !a ) { s = "<<NULL>>"; i = 8; break; }
				i = PrintFWriteAddr( a, &F, buf );
				s = buf;
				break;
			
			case 'N':	// %N Now (date/time string).
				if( F.suppress ) continue;
				#if( TARGET_OS_POSIX && !0 )
				{
					struct timeval		now;
					time_t				nowTT;
					struct tm *			nowTM;
					char				dateTimeStr[ 24 ];
					char				amPMStr[ 8 ];
					
					gettimeofday( &now, NULL );
					nowTT = now.tv_sec;
					nowTM = localtime( &nowTT );
					strftime( dateTimeStr, sizeof( dateTimeStr ), "%Y-%m-%d %I:%M:%S", nowTM );
					strftime( amPMStr, sizeof( amPMStr ), "%p", nowTM );
					i = SNPrintF( buf, sizeof( buf ), "%s.%06u %s", dateTimeStr, now.tv_usec, amPMStr );
					s = buf;
				}
				#elif( TARGET_HAS_STD_C_LIB && !0 )
				{
					time_t			now;
					struct tm *		nowTM;
					
					buf[ 0 ] = '\0';
					now = time( NULL );
					nowTM = localtime( &now );
					i = (int) strftime( buf, sizeof( buf ), "%Y-%m-%d %I:%M:%S %p", nowTM );
					s = buf;
				}
				#else
					s = "<<NO TIME>>";
					i = 11;
				#endif
				break;
			
			case 'U':	// %U: UUID
				a = va_arg( inArgs, const uint8_t * );
				if( F.suppress ) continue;
				if( !a ) { s = "<<NULL>>"; i = 8; break; }
				
				// Note: Windows and EFI treat some sections as 32-bit and 16-bit little endian values and those are the
				// most common UUID's so default to that, but allow %#U to print big-endian UUIDs.
				
				if( F.altForm == 0 )
				{
					i = SNPrintF( buf, sizeof( buf ), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
						a[  3 ], a[  2 ], a[  1 ], a[  0 ], a[  5 ], a[  4 ], a[  7 ], a[  6 ], 
						a[  8 ], a[  9 ], a[ 10 ], a[ 11 ], a[ 12 ], a[ 13 ], a[ 14 ], a[ 15 ] );
				}
				else
				{
					i = SNPrintF( buf, sizeof( buf ), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
						a[  0 ], a[  1 ], a[  2 ], a[  3 ], a[  4 ], a[  5 ], a[  6 ], a[  7 ], 
						a[  8 ], a[  9 ], a[ 10 ], a[ 11 ], a[ 12 ], a[ 13 ], a[ 14 ], a[ 15 ] );
				}
				s = buf;
				break;
			
			case 'v' :	// %v: NumVersion
				x = va_arg( inArgs, unsigned int );
				if( F.suppress ) continue;
				i = PrintFWriteNumVersion( (uint32_t) x, buf );
				s = buf;
				break;
			
			case 'V':	// %V: Nested PrintF format string and va_list.
			{
				const char *		nestedFormat;
				va_list *			nestedArgs;
				
				nestedFormat = va_arg( inArgs, const char * );
				nestedArgs   = va_arg( inArgs, va_list * );
				if( F.suppress ) continue;
				if( !nestedFormat || !nestedArgs ) { s = "<<NULL>>"; i = 8; break; }
				
				err = PrintFCoreVAList( inContext, nestedFormat, *nestedArgs );
				if( err < 0 ) goto error;
				
				nTotal += err;
				continue;
			}
			
			case 'n' :	// %n: Receive the number of characters written so far.
				p = va_arg( inArgs, char * );
				if(      F.hSize == 1 ) *( (short   *) p ) = (short) nTotal;
				else if( F.hSize == 2 ) *( (char    *) p ) = (char)  nTotal;
				else if( F.lSize == 1 )	*( (long    *) p ) = (long)  nTotal;
				else if( F.lSize == 2 )	*( (int64_t *) p ) = (long)  nTotal;
				else					*( (int     *) p ) = nTotal;
				continue;
			
			case '%':	// %%: Literal %
				if( F.suppress ) continue;
				buf[ 0 ] = '%';
				i = 1;
				s = buf;
				break;
			
			case '{':	// %{<extension>}
			{
				const char *		extensionPtr;
				size_t				extensionLen;
				
				extensionPtr = ++fmt;
				while( ( c != '\0' ) && ( c != '}' ) ) c = *++fmt;
				extensionLen = (size_t)( fmt - extensionPtr );
				
				// %{asbd}: AudioStreamBasicDescription
				
				if( __strnicmpx( extensionPtr, extensionLen, "asbd" ) == 0 )
				{
					const AudioStreamBasicDescription * const		absd = va_arg( inArgs, const AudioStreamBasicDescription * );
					
					if( F.suppress ) continue;
					err = PrintFWriteAudioStreamBasicDescription( inContext, absd );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// %{dur}:  Time Duration (e.g. 930232 seconds prints "10d 18h 23m 52s").
				// %#{dur}: Time Duration (e.g. 930232 seconds prints "10d 18:23:52").
				
				if( __strnicmpx( extensionPtr, extensionLen, "dur" ) == 0 )
				{
					if(      F.lSize == 1 )	x = va_arg( inArgs, unsigned long );
					else if( F.lSize == 2 )	x = va_arg( inArgs, uint64_t );
					else					x = va_arg( inArgs, unsigned int );
					if(      F.hSize == 1 )	x = (unsigned short)( x & 0xFFFF );
					else if( F.hSize == 2 )	x = (unsigned char)( x & 0xFF );
					if( F.suppress ) continue;
					i = PrintFWriteTimeDuration( x, F.altForm, buf );
					s = buf;
					break;
				}
				
				// %{ie}: IEEE 802.11 Information Element (IE).
				
				if( __strnicmpx( extensionPtr, extensionLen, "ie" ) == 0 )
				{
					a    = va_arg( inArgs, uint8_t * );
					size = va_arg( inArgs, size_t );
					if( F.suppress ) continue;
					err = PrintFWriteIE( inContext, &F, a, size );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// %{pid}: Process name (with optional numeric PID).
				
				if( __strnicmpx( extensionPtr, extensionLen, "pid" ) == 0 )
				{
					#if( TARGET_OS_POSIX )
						pid_t		pid;
						
						if(  sizeof( pid_t ) > sizeof( int ) )	pid = (pid_t) va_arg( inArgs, int64_t );
						else									pid = (pid_t) va_arg( inArgs, int );
						if( F.suppress ) continue;
						*buf = '\0';
						GetProcessNameByPID( pid, buf, sizeof( buf ) );
						if( F.altForm ) err = PrintFCore( inContext, "%s:%lld", buf, (int64_t) pid );
						else			err = PrintFCore( inContext, "%s", buf );
					#else
						err = PrintFCore( inContext, "<< ERROR: %%{pid} not supported on this platform >>" );
					#endif
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// %{sline}: Single line string. \r and \n are replaced with ⏎. Arg=ptr to string.
				
				if( __strnicmpx( extensionPtr, extensionLen, "sline" ) == 0 )
				{
					s    = va_arg( inArgs, const char * );
					size = va_arg( inArgs, size_t );
					if( F.suppress ) continue;
					if( size == kSizeCString ) size = strlen( s );
					
					err = PrintFWriteSingleLineText( inContext, s, size );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// %{text}: Multi-line text (with optional indenting).
				
				if( __strnicmpx( extensionPtr, extensionLen, "text" ) == 0 )
				{
					s    = va_arg( inArgs, const char * );
					size = va_arg( inArgs, size_t );
					if( F.suppress ) continue;
					if( size == kSizeCString ) size = strlen( s );
					
					err = PrintFWriteMultiLineText( inContext, &F, s, size );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// %{txt}: DNS TXT record name=value pairs.
				
				if( __strnicmpx( extensionPtr, extensionLen, "txt" ) == 0 )
				{
					a    = va_arg( inArgs, uint8_t * );
					size = va_arg( inArgs, size_t );
					if( F.suppress ) continue;
					err = PrintFWriteTXTRecord( inContext, &F, a, size );
					if( err < 0 ) goto error;
					nTotal += err;
					continue;
				}
				
				// Unknown extension.
				
				i = SNPrintF( buf, sizeof( buf ), "<<UNKNOWN PRINTF EXTENSION '%.*s'>>", (int) extensionLen, extensionPtr );
				s = buf;
				break;
			}
			
			default:
				i = SNPrintF( buf, sizeof( buf ), "<<UNKNOWN FORMAT CONVERSION CODE %%%c>>", c );
				s = buf;
				break;
		}
		
		// Print the text with the correct padding, etc.
		
		err = PrintFWriteText( inContext, &F, s, (size_t) i );
		if( err < 0 ) goto error;
		nTotal += err;
	}
	return( nTotal );

error:
	return( err );
}

//===========================================================================================================================
//	PrintFDiv64x32
//
//	64-bit divide by a 32-bit value. Needed for environments without native 64-bit divides (such as GCC 2.7 or earlier).
//===========================================================================================================================

#if( !TYPE_LONGLONG_NATIVE )
static void	PrintFDiv64x32( uint64_t *ioValue, uint32_t inDivisor, uint32_t *outRemainder )
{
	// The following need to be non-const variables to workaround a GCC 2.7.2 compiler warning bug with 64-bit values.
	
	static uint64_t		k64BitZero	= 0;
	static uint64_t		k64BitOne	= 1;
	
	uint64_t			remain;
	
	// Optimize for the 32-bit case, if possible.
	
	remain = *ioValue;
	if( remain <= UINT64_C( 0xFFFFFFFF ) )
	{
		*ioValue = ( (uint32_t) remain ) / inDivisor;
		remain	 = ( (uint32_t) remain ) % inDivisor;
	}
	else
	{
		uint64_t		result;
		uint64_t		b;
		uint64_t		d;
		uint64_t		tmp;
		uint32_t		high;
		
		result	= k64BitZero;
		b		= inDivisor;
		d		= k64BitOne;
		
		tmp		= remain;			// 1) These 4 lines are equivalent to:
		tmp	  >>= 31;				// 2) high = remain >> 32;
		tmp	  >>= 1;				// 3) but have to be split up because GCC 2.7.2 cannot handle shifts > 31.
		high	= (uint32_t) tmp;	// 4)
		
		if( high >= inDivisor )
		{
			high	 /= inDivisor;
			
			result	  = high;		// 1) These 3 lines are equivalent to:
			result	<<= 31;			// 2) result = ( (uint64_t) high ) << 32;
			result	<<=  1;			// 3) but have to be split up because GCC 2.7.2 cannot handle shifts > 31.
			
			tmp		  = high;		// 1) These 5 lines are equivalent to:
			tmp		 *= inDivisor;	// 2) remain -= ( (uint64_t)( high * inDivisor ) ) << 32;
			tmp		<<= 31;			// 3) but have to be split up because GCC 2.7.2 cannot handle shifts > 31.
			tmp		<<=  1;			// 4)
			remain	 -= tmp;		// 5)
		}
		while( ( ( (int64_t) b ) > 0 ) && ( b < remain ) )
		{
			b = b + b;
			d = d + d;
		}
		do
		{
			if( remain >= b )
			{
				remain	-= b;
				result	+= d;
			}
			b >>= 1;
			d >>= 1;
			
		}	while( d );
		
		*ioValue = result;
	}
	*outRemainder = (uint32_t) remain;
}
#endif	// !TYPE_LONGLONG_NATIVE

//===========================================================================================================================
//	PrintFWriteAddr
//===========================================================================================================================

typedef struct
{
	int32_t		type;
	union
	{
		uint8_t		v4[ 4 ];
		uint8_t		v6[ 16 ];
		
	}	ip;
	
}	mDNSAddrCompat;

static int	PrintFWriteAddr( const uint8_t *inAddr, PrintFFormat *inFormat, char *outStr )
{
	int					n;
	const uint8_t *		a;
	PrintFFormat *		F;
	
	a = inAddr;
	F = inFormat;
	if( ( F->altForm == 1 ) && ( F->precision == 4 ) ) // %#.4a - IPv4 address in host byte order
	{
		#if( TARGET_RT_BIG_ENDIAN )
			n = SNPrintF( outStr, kPrintFBufSize, "%u.%u.%u.%u", a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
		#else
			n = SNPrintF( outStr, kPrintFBufSize, "%u.%u.%u.%u", a[ 3 ], a[ 2 ], a[ 1 ], a[ 0 ] );
		#endif
	}
	else if( ( F->altForm == 1 ) && ( F->precision == 6 ) ) // %#.6a - MAC address from host order uint64_t *.
	{
		#if( TARGET_RT_BIG_ENDIAN )
			n = SNPrintF( outStr, kPrintFBufSize, "%02X:%02X:%02X:%02X:%02X:%02X", 
				a[ 2 ], a[ 3 ], a[ 4 ], a[ 5 ], a[ 6 ], a[ 7 ] );
		#else
			n = SNPrintF( outStr, kPrintFBufSize, "%02X:%02X:%02X:%02X:%02X:%02X", 
				a[ 5 ], a[ 4 ], a[ 3 ], a[ 2 ], a[ 1 ], a[ 0 ] );
		#endif
	}
	else if( F->altForm == 1 )	// %#a: mDNSAddr
	{
		mDNSAddrCompat *		ip;
		
		ip = (mDNSAddrCompat *) inAddr;
		if( ip->type == 4 )
		{
			a = ip->ip.v4;
			n = SNPrintF( outStr, kPrintFBufSize, "%u.%u.%u.%u", a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
		}
		else if( ip->type == 6 )
		{
			PrintF_IPv6AddressToCString( ip->ip.v6, 0, 0, outStr );
			n = (int) strlen( outStr );
		}
		else
		{
			n = SNPrintF( outStr, kPrintFBufSize, "%s", "<< ERROR: %#a used with unsupported type: %d >>", ip->type );
		}
	}
	else if( F->altForm == 2 )	// %##a: sockaddr
	{
		#if( defined( AF_INET ) )
			int		family;
			
			family = ( (const struct sockaddr *) inAddr )->sa_family;
			if( family == AF_INET )
			{
				const struct sockaddr_in * const		sa4 = (const struct sockaddr_in *) a;
				
				PrintF_IPv4AddressToCString( ntoh32( sa4->sin_addr.s_addr ), ntoh16( sa4->sin_port ), outStr );
				n = (int) strlen( outStr );
			}
			#if( defined( AF_INET6 ) )
				else if( family == AF_INET6 )
				{
					const struct sockaddr_in6 * const		sa6 = (const struct sockaddr_in6 *) a;
					
					PrintF_IPv6AddressToCString( sa6->sin6_addr.s6_addr, sa6->sin6_scope_id, ntoh16( sa6->sin6_port ), outStr );
					n = (int) strlen( outStr );
				}
			#endif
		#if( defined( AF_LINK ) && defined( LLADDR ) )
			else if( family == AF_LINK )
			{
				const struct sockaddr_dl * const		sdl = (const struct sockaddr_dl *) a;
				
				a = (const uint8_t *) LLADDR( sdl );
				if( sdl->sdl_alen == 6 )
				{
					n = SNPrintF( outStr, kPrintFBufSize, "%02X:%02X:%02X:%02X:%02X:%02X", 
						a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ], a[ 4 ], a[ 5 ] );
				}
				else
				{
					n = SNPrintF( outStr, kPrintFBufSize, "<< AF_LINK %H >>", a, sdl->sdl_alen, sdl->sdl_alen );
				}
			}
		#endif
			else if( family == AF_UNSPEC )
			{
				n = SNPrintF( outStr, kPrintFBufSize, "<< AF_UNSPEC >>" );
			}
			else
			{
				n = SNPrintF( outStr, kPrintFBufSize, "<< ERROR: %%##a used with unknown family: %d >>", family );
			}
		#else
			n = SNPrintF( outStr, kPrintFBufSize, "%s", "<< ERROR: %##a used without socket support >>" );
		#endif
	}
	else
	{
		switch( F->precision )
		{
			case 2:
				n = SNPrintF( outStr, kPrintFBufSize, "%u.%u.%u.%u", a[ 0 ] >> 4, a[ 0 ] & 0xF, a[ 1 ] >> 4, a[ 1 ] & 0xF );
				break;
			
			case 4:
				n = SNPrintF( outStr, kPrintFBufSize, "%u.%u.%u.%u", a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
				break;
			
			case 6:
				n = SNPrintF( outStr, kPrintFBufSize, "%02X:%02X:%02X:%02X:%02X:%02X", 
					a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ], a[ 4 ], a[ 5 ] );
				break;
			
			case 8:
				n = SNPrintF( outStr, kPrintFBufSize, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", 
					a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ], a[ 4 ], a[ 5 ], a[ 6 ], a[ 7 ] );
				break;
			
			case 16:
				PrintF_IPv6AddressToCString( a, 0, 0, outStr );
				n = (int) strlen( outStr );
				break;
			
			default:
				n = SNPrintF( outStr, kPrintFBufSize, "%s", 
					"<< ERROR: Must specify address size (i.e. %.4a=IPv4, %.6a=Enet, %.8a=Fibre, %.16a=IPv6) >>" );
				break;
		}
	}
	return( n );
}

//===========================================================================================================================
//	PrintFWriteAudioStreamBasicDescription
//===========================================================================================================================

static int	PrintFWriteAudioStreamBasicDescription( PrintFContext *inContext, const AudioStreamBasicDescription *inASBD )
{
	int					total = 0;
	int					n;
	char				buf[ 32 ];
	const char *		str;
	uint32_t			u32;
	
	if(      inASBD->mFormatID == kAudioFormatMPEG4AAC_ELD )	str = "ELD,";
	else if( inASBD->mFormatID == kAudioFormatMPEG4AAC )		str = "AAC,";
	else if( inASBD->mFormatID == kAudioFormatAppleLossless )	str = "ALAC,";
	else if( inASBD->mFormatID == kAudioFormatLinearPCM )		str = "PCM,";
	else { SNPrintF( buf, sizeof( buf ), "%C,", inASBD->mFormatID ); str = buf; }
	
	n = PrintFCore( inContext, "%-5s %5u Hz", str, (uint32_t) inASBD->mSampleRate );
	require_action_quiet( n >= 0, exit, total = n );
	total += n;
	
	if( inASBD->mBitsPerChannel > 0 )
	{
		n = PrintFCore( inContext, ", %2u-bit", inASBD->mBitsPerChannel );
		require_action_quiet( n >= 0, exit, total = n );
		total += n;
	}
	else if( inASBD->mFormatID == kAudioFormatAppleLossless )
	{
		if(      inASBD->mFormatFlags == kAppleLosslessFormatFlag_16BitSourceData ) str = "16-bit";
		else if( inASBD->mFormatFlags == kAppleLosslessFormatFlag_20BitSourceData ) str = "20-bit";
		else if( inASBD->mFormatFlags == kAppleLosslessFormatFlag_24BitSourceData ) str = "24-bit";
		else if( inASBD->mFormatFlags == kAppleLosslessFormatFlag_32BitSourceData ) str = "32-bit";
		else																		str = "\?\?-bit";
		n = PrintFCore( inContext, ", %s", str );
		require_action_quiet( n >= 0, exit, total = n );
		total += n;
	}
	
	if(      inASBD->mChannelsPerFrame == 1 ) str = "Mono";
	else if( inASBD->mChannelsPerFrame == 2 ) str = "Stereo";
	else { SNPrintF( buf, sizeof( buf ), "%u ch", inASBD->mChannelsPerFrame ); str = buf; }
	n = PrintFCore( inContext, ", %s", str );
	require_action_quiet( n >= 0, exit, total = n );
	total += n;
	
	if( inASBD->mFormatFlags & kAudioFormatFlagIsNonInterleaved )
	{
		n = PrintFCore( inContext, ", Non-interleaved" );
		require_action_quiet( n >= 0, exit, total = n );
		total += n;
	}
	
	if( inASBD->mFormatID == kAudioFormatLinearPCM )
	{
		#if( TARGET_RT_LITTLE_ENDIAN )
		if( inASBD->mFormatFlags & kLinearPCMFormatFlagIsBigEndian )
		#else
		if( !( inASBD->mFormatFlags & kLinearPCMFormatFlagIsBigEndian ) )
		#endif
		{
			n = PrintFCore( inContext, ", Swapped" );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
		}
		
		if( inASBD->mFormatFlags & kAudioFormatFlagIsFloat )
		{
			n = PrintFCore( inContext, ", Float" );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
		}
		else if( ( inASBD->mBitsPerChannel > 0 ) && ( inASBD->mFormatFlags & kLinearPCMFormatFlagIsSignedInteger ) && 
				 ( inASBD->mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask ) )
		{
			u32 = ( inASBD->mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask ) >> kLinearPCMFormatFlagsSampleFractionShift;
		
			n = PrintFCore( inContext, ", %u.%u", inASBD->mBitsPerChannel - u32, u32 );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
		}
	}
	
	if( inASBD->mFramesPerPacket > 1 )
	{
		n = PrintFCore( inContext, ", %u samples/packet", inASBD->mFramesPerPacket );
		require_action_quiet( n >= 0, exit, total = n );
		total += n;
	}
	
exit:
	return( total );	
}

//===========================================================================================================================
//	PrintFWriteBits
//===========================================================================================================================

static int	PrintFWriteBits( uint64_t inX, PrintFFormat *inFormat, char *outStr )
{
#if( TYPE_LONGLONG_NATIVE )
	static const uint64_t			kBit0 = 1;
	uint64_t						x = inX;
#else
	static const unsigned long		kBit0 = 1;
	unsigned long					x = (unsigned long) inX;
#endif
	int								maxBit;
	int								bit;
	char *							dst;
	char *							lim;
	
	dst = outStr;
	lim = dst + kPrintFBufSize;
	if( !inFormat->havePrecision )
	{
		if(      inFormat->hSize == 1 )	inFormat->precision = 8 * sizeof( short );
		else if( inFormat->hSize == 2 )	inFormat->precision = 8 * sizeof( char );
		else if( inFormat->lSize == 1 )	inFormat->precision = 8 * sizeof( long );
		else if( inFormat->lSize == 2 )	inFormat->precision = 8 * sizeof( int64_t );
		else							inFormat->precision = 8 * sizeof( int );
	}
	if( inFormat->precision > ( sizeof( kBit0 ) * 8 ) )
	{
		SNPrintF_Add( &dst, lim, "ERROR: << precision must be 0-%d >>", ( sizeof( kBit0 ) * 8 ) );
	}
	else
	{
		if( inFormat->precision < 1 ) inFormat->precision = 1;
		maxBit = (int)( inFormat->precision - 1 );
		if( inFormat->altForm == 2 )
		{
			for( bit = maxBit; bit >= 0; --bit )
			{
				if( x & ( kBit0 << bit ) )
				{
					SNPrintF_Add( &dst, lim, "%s%d", ( dst != outStr ) ? " " : "", bit );
				}
			}
		}
		else
		{
			for( bit = 0; bit <= maxBit; ++bit )
			{
				if( x & ( kBit0 << ( maxBit - bit ) ) )
				{
					SNPrintF_Add( &dst, lim, "%s%d", ( dst != outStr ) ? " " : "", bit );
				}
			}
		}
	}
	return( (int)( dst - outStr ) );
}

#if( DEBUG_CF_OBJECTS_ENABLED )

//===========================================================================================================================
//	PrintFWriteCFObject
//===========================================================================================================================

typedef struct
{
	PrintFContext *		context;
	PrintFFormat *		format;
	int					indent;
	int					total; // Note: temporary total for a recursive operation.
	OSStatus			error;
	
}	PrintFWriteCFObjectContext;

static int
	PrintFWriteCFObjectLevel( 
		PrintFWriteCFObjectContext *inContext, 
		CFTypeRef					inObj, 
		Boolean						inIsValue, 
		Boolean						inPrintingArray );
static void	PrintFWriteCFObjectApplier( const void *inKey, const void *inValue, void *inContext );

static int	PrintFWriteCFObject( PrintFContext *inContext, PrintFFormat *inFormat, CFTypeRef inObj, char *inBuffer )
{
	int					total;
	CFTypeID			typeID;
	const char *		s;
	int					n;
	
	typeID = CFGetTypeID( inObj );
	
	// CFBoolean
	
	if( typeID == CFBooleanGetTypeID() )
	{
		if(      ( (CFBooleanRef) inObj ) == kCFBooleanTrue )	{ s = "true";   n = 4; }
		else													{ s = "false";  n = 5; }
		total = PrintFWriteText( inContext, inFormat, s, (size_t) n );
	}
	
	// CFNumber
	
	else if( typeID == CFNumberGetTypeID() )
	{
		#if( !CFLITE_ENABLED || CFL_FLOATING_POINT_NUMBERS )
			if( CFNumberIsFloatType( (CFNumberRef) inObj ) )
			{
				double		dval = 0;
				
				CFNumberGetValue( (CFNumberRef) inObj, kCFNumberDoubleType, &dval );
				n = SNPrintF( inBuffer, kPrintFBufSize, "%f", dval );
			}
			else
		#endif
			{
				int64_t		s64 = 0;
				
				CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &s64 );
				n = SNPrintF( inBuffer, kPrintFBufSize, "%lld", s64 );
			}
		total = PrintFWriteText( inContext, inFormat, inBuffer, (size_t) n );
	}
	
	// CFString
	
	else if( typeID == CFStringGetTypeID() )
	{
		CFStringRef const		cfStr = (CFStringRef) inObj;
		CFIndex					cfLen;
		size_t					size;
		
		cfLen = CFStringGetLength( cfStr );
		size = (size_t) CFStringGetMaximumSizeForEncoding( cfLen, kCFStringEncodingUTF8 );
		if( size > 0 )
		{
			char *			cStr;
			CFRange			range;
			CFIndex			i;
			
			cStr = (char *) malloc( size );
			require_action_quiet( cStr, exit, total = kNoMemoryErr );
			
			i = 0;
			range = CFRangeMake( 0, cfLen );
			CFStringGetBytes( cfStr, range, kCFStringEncodingUTF8, '^', false, (UInt8 *) cStr, (CFIndex) size, &i );
			
			// Restrict the string length to the precision, but don't truncate in the middle of a UTF-8 character.
			
			if( inFormat->havePrecision && ( i > (CFIndex) inFormat->precision ) )
			{
				for( i = (int) inFormat->precision; ( i > 0 ) && ( ( cStr[ i ] & 0xC0 ) == 0x80 ); --i ) {}
			}
			
			total = PrintFWriteText( inContext, inFormat, cStr, (size_t) i );
			free( cStr );
		}
		else
		{
			// Note: this is needed because there may be field widths, etc. to fill.
			
			total = PrintFWriteText( inContext, inFormat, "", 0 );
		}
	}
	
	// CFNull
	
	else if( typeID == CFNullGetTypeID() )
	{
		total = PrintFWriteText( inContext, inFormat, "Null", 4 );
	}
	
	// Other
	
	else
	{
		PrintFWriteCFObjectContext		cfContext;
		
		cfContext.context	= inContext;
		cfContext.format	= inFormat;
		cfContext.indent	= inFormat->fieldWidth;
		cfContext.error		= kNoErr;
		
		total = PrintFWriteCFObjectLevel( &cfContext, inObj, true, false );
		require_quiet( total >= 0, exit );
		
		if( ( typeID == CFArrayGetTypeID() ) || ( typeID == CFDictionaryGetTypeID() ) )
		{
			n = inContext->callback( "\n", 1, inContext );
			require_action_quiet( n > 0, exit, total = n );
			total += n;
		}
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteCFObjectLevel
//===========================================================================================================================

static int
	PrintFWriteCFObjectLevel( 
		PrintFWriteCFObjectContext *inContext, 
		CFTypeRef					inObj, 
		Boolean						inIsValue, 
		Boolean						inPrintingArray )
{
	int				total;
	OSStatus		err;
	CFTypeID		typeID;
	CFIndex			i, n;
	CFTypeRef		obj;
	size_t			size;
	
	total = 0;
	typeID = CFGetTypeID( inObj );
	
	// Array
	
	if( typeID == CFArrayGetTypeID() )
	{
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		n = CFArrayGetCount( (CFArrayRef) inObj );
		if( n > 0 )
		{
			err = inContext->context->callback( "[\n", 2, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			for( i = 0; i < n; ++i )
			{
				obj = CFArrayGetValueAtIndex( (CFArrayRef) inObj, i );
				
				++inContext->indent;
				err = PrintFWriteCFObjectLevel( inContext, obj, inIsValue, true );
				--inContext->indent;
				require_action_quiet( err >= 0, exit, total = err );
				total += err;
				
				err = inContext->context->callback( "\n", 1, inContext->context );
				require_action_quiet( err >= 0, exit, total = err );
				total += err;
			}
			
			err = print_indent( inContext->context, inContext->indent );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = inContext->context->callback( "]", 1, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else
		{
			err = inContext->context->callback( "[]", 2, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
	// Boolean
	
	else if( typeID == CFBooleanGetTypeID() )
	{
		const char *		boolStr;
		
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		if( ( (CFBooleanRef) inObj ) == kCFBooleanTrue )	{ boolStr = "true";  size = 4; }
		else												{ boolStr = "false"; size = 5; }
		
		err = inContext->context->callback( boolStr, size, inContext->context );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
	// Data
	
	else if( typeID == CFDataGetTypeID() )
	{
		int		oldIndent;
		
		oldIndent = inContext->indent;
		size = (size_t) CFDataGetLength( (CFDataRef) inObj );
		if( ( size <= 16 ) && !inPrintingArray )
		{
			inContext->indent = 0;
		}
		else
		{
			err = inContext->context->callback( "\n", 1, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			inContext->indent = oldIndent + 1;
		}
		
		err = PrintFWriteHex( inContext->context, inContext->format, inContext->indent, 
			CFDataGetBytePtr( (CFDataRef) inObj ), size, 
			inContext->format->havePrecision ? inContext->format->precision : size );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		inContext->indent = oldIndent;
	}
	
	// Date
	
	else if( typeID == CFDateGetTypeID() )
	{
		int		year, month, day, hour, minute, second, micros;
		
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		CFDateGetComponents( (CFDateRef) inObj, &year, &month, &day, &hour, &minute, &second, &micros );
		err = PrintFCore( inContext->context, "%04d-%02d-%02d %02d:%02d:%02d.%06d", 
			year, month, day, hour, minute, second, micros );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
	// Dictionary
	
	else if( typeID == CFDictionaryGetTypeID() )
	{
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		if( CFDictionaryGetCount( (CFDictionaryRef) inObj ) > 0 )
		{
			err = inContext->context->callback( "{\n", 2, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			++inContext->indent;
			
			inContext->total = total;
			CFDictionaryApplyFunction( (CFDictionaryRef) inObj, PrintFWriteCFObjectApplier, inContext );
			require_action_quiet( inContext->error >= 0, exit, total = inContext->error );
			total = inContext->total;
			
			--inContext->indent;
			
			err = print_indent( inContext->context, inContext->indent );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = inContext->context->callback( "}", 1, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else
		{
			err = inContext->context->callback( "{}", 2, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
	// Number
	
	else if( typeID == CFNumberGetTypeID() )
	{
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		#if( !CFLITE_ENABLED || CFL_FLOATING_POINT_NUMBERS )
			if( CFNumberIsFloatType( (CFNumberRef) inObj ) )
			{
				double		dval = 0;
				
				CFNumberGetValue( (CFNumberRef) inObj, kCFNumberDoubleType, &dval );
				err = PrintFCore( inContext->context, "%f", dval );
			}
			else
		#endif
			{
				int64_t		s64 = 0;
				
				CFNumberGetValue( (CFNumberRef) inObj, kCFNumberSInt64Type, &s64 );
				err = PrintFCore( inContext->context, "%lld", s64 );
			}
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
	// String
	
	else if( typeID == CFStringGetTypeID() )
	{
		CFStringRef const		cfStr = (CFStringRef) inObj;
		
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		if( inIsValue )
		{
			err = inContext->context->callback( "\"", 1, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		
		n = CFStringGetLength( cfStr );
		size = (size_t) CFStringGetMaximumSizeForEncoding( n, kCFStringEncodingUTF8 );
		if( size > 0 )
		{
			char *			cStr;
			CFRange			range;
			CFIndex			converted;
			
			cStr = (char *) malloc( size );
			require_action_quiet( cStr, exit, total = kNoMemoryErr );
			
			converted = 0;
			range = CFRangeMake( 0, n );
			CFStringGetBytes( cfStr, range, kCFStringEncodingUTF8, '^', false, (UInt8 *) cStr, (CFIndex) size, &converted );
			
			err = inContext->context->callback( cStr, (size_t) converted, inContext->context );
			free( cStr );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		
		if( inIsValue )
		{
			err = inContext->context->callback( "\"", 1, inContext->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
	// CFNull
	
	else if( typeID == CFNullGetTypeID() )
	{
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		err = inContext->context->callback( "Null", 4, inContext->context );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
	// Unknown
	
	else
	{
		err = print_indent( inContext->context, inContext->indent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
			
		err = PrintFCore( inContext->context, "<<UNKNOWN CF OBJECT TYPE: %d>>", (int) typeID );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteCFObjectApplier
//===========================================================================================================================

static void	PrintFWriteCFObjectApplier( const void *inKey, const void *inValue, void *inContext )
{
	int										total;
	PrintFWriteCFObjectContext * const		context = (PrintFWriteCFObjectContext *) inContext;
	CFTypeRef const							value	= (CFTypeRef) inValue;
	OSStatus								err;
	CFTypeID								typeID;
	 
	if( context->error ) return;
	
	// Print the key.
	
	err = PrintFWriteCFObjectLevel( context, (CFTypeRef) inKey, false, false );
	require_action_quiet( err >= 0, exit, total = err );
	total = err;
	
	err = context->context->callback( " : ", 3, context->context );
	require_action_quiet( err >= 0, exit, total = err );
	total += err;
	
	// Print the value based on its type.
	
	typeID = CFGetTypeID( value );
	if( typeID == CFArrayGetTypeID() )
	{
		if( CFArrayGetCount( (CFArrayRef) inValue ) > 0 )
		{
			err = context->context->callback( "\n", 1, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = PrintFWriteCFObjectLevel( context, value, true, true );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = context->context->callback( "\n", 1, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else
		{
			err = context->context->callback( "[]\n", 3, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	else if( typeID == CFDictionaryGetTypeID() )
	{
		if( CFDictionaryGetCount( (CFDictionaryRef) inValue ) > 0 )
		{
			err = context->context->callback( "\n", 1, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = PrintFWriteCFObjectLevel( context, value, true, false );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			err = context->context->callback( "\n", 1, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else
		{
			err = context->context->callback( "{}\n", 3, context->context );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	else if( ( typeID == CFDataGetTypeID() ) && ( context->format->altForm != 2 ) )
	{
		err = PrintFWriteCFObjectLevel( context, value, true, false );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	else
	{
		int		oldIndent;
		
		oldIndent = context->indent;
		context->indent = 0;
		
		err = PrintFWriteCFObjectLevel( context, value, true, false );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		context->indent = oldIndent;
		
		err = context->context->callback( "\n", 1, context->context );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
exit:
	context->total += total;
	if( err < 0 ) context->error = err;
}

//===========================================================================================================================
//	PrintFWriteCFXMLObject
//===========================================================================================================================

#if( !CFLITE_ENABLED || CFL_XML )
static int	PrintFWriteCFXMLObject( PrintFContext *inContext, PrintFFormat *inFormat, CFTypeRef inObj )
{
	int				total, err;
	CFDataRef		xmlData;
	const char *	xmlPtr;
	size_t			xmlLen;
	
	total = 0;
	xmlData = CFPropertyListCreateData( NULL, inObj, kCFPropertyListXMLFormat_v1_0, 0, NULL );
	if( !xmlData )
	{
		err = PrintFCore( inContext, "<<PLIST NOT XML-ABLE>>" );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		goto exit;
	}
	xmlPtr = (const char *) CFDataGetBytePtr( xmlData );
	xmlLen = (size_t) CFDataGetLength( xmlData );
	
	err = PrintFWriteMultiLineText( inContext, inFormat, xmlPtr, xmlLen );
	CFRelease( xmlData );
	require_action_quiet( err >= 0, exit, total = err );
	total += err;
	
exit:
	return( total );
}
#endif // !CFLITE_ENABLED || CFL_XML
#endif // DEBUG_CF_OBJECTS_ENABLED

//===========================================================================================================================
//	PrintFWriteHex
//===========================================================================================================================

static int
	PrintFWriteHex( 
		PrintFContext *	inContext, 
		PrintFFormat *	inFormat, 
		int				inIndent, 
		const void *	inData, 
		size_t			inSize, 
		size_t			inMaxSize )
{
	int					total;
	int					err;
	const uint8_t *		start;
	const uint8_t *		ptr;
	size_t				size;
	uint8_t				hex1[ 64 ];
	uint8_t				hex2[ 64 ];
	uint8_t *			currHexPtr;
	uint8_t *			prevHexPtr;
	uint8_t *			tempHexPtr;
	int					dupCount;
	size_t				dupSize;
	
	total		= 0;
	currHexPtr	= hex1;
	prevHexPtr	= hex2;
	dupCount	= 0;
	dupSize		= 0;
	start		= (const uint8_t *) inData;
	ptr			= start;
	size		= ( inSize > inMaxSize ) ? inMaxSize : inSize;
	
	for( ;; )
	{
		size_t			chunkSize;
		uint8_t			ascii[ 64 ];
		uint8_t *		s;
		uint8_t			c;
		size_t			i;
		
		// Build a hex string (space every 4 bytes) and pad with space to fill the full 16-byte range.
		
		chunkSize = Min( size, 16 );
		s = currHexPtr;
		for( i = 0; i < 16; ++i )
		{
			if( ( i > 0 ) && ( ( i % 4 ) == 0 ) ) *s++ = ' ';
			if( i < chunkSize )
			{
				*s++ = (uint8_t) kHexDigitsLowercase[ ptr[ i ] >> 4   ];
				*s++ = (uint8_t) kHexDigitsLowercase[ ptr[ i ] &  0xF ];
			}
			else
			{
				*s++ = ' ';
				*s++ = ' ';
			}
		}
		*s++ = '\0';
		check( ( (size_t)( s - currHexPtr ) ) < sizeof( hex1 ) );
		
		// Build a string with the ASCII version of the data (replaces non-printable characters with '^').
		// Pads the string with spaces to fill the full 16 byte range (so it lines up).
		
		s = ascii;
		for( i = 0; i < 16; ++i )
		{
			if( i < chunkSize )
			{
				c = ptr[ i ];
				if( !PrintFIsPrintable( c ) )
				{
					c = '^';
				}
			}
			else
			{
				c = ' ';
			}
			*s++ = c;
		}
		*s++ = '\0';
		check( ( (size_t)( s - ascii ) ) < sizeof( ascii ) );
		
		// Print the data.
		
		if( inSize <= 16 )
		{
			err = print_indent( inContext, inIndent );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		
			err = PrintFCore( inContext, "%s |%s| (%zu bytes)\n", currHexPtr, ascii, inSize );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else if( ptr == start )
		{
			err = print_indent( inContext, inIndent );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		
			err = PrintFCore( inContext, "+%04X: %s |%s| (%zu bytes)\n", (int)( ptr - start ), currHexPtr, ascii, inSize );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		else if( ( inFormat->group > 0 ) && ( memcmp( currHexPtr, prevHexPtr, 32 ) == 0 ) )
		{
			dupCount += 1;
			dupSize  += chunkSize;
		}
		else
		{
			if( dupCount > 0 )
			{
				err = print_indent( inContext, inIndent );
				require_action_quiet( err >= 0, exit, total = err );
				total += err;
				
				err = PrintFCore( inContext, "* (%zu more identical bytes, %zu total)\n", dupSize, dupSize + 16 );
				require_action_quiet( err >= 0, exit, total = err );
				total += err;
				
				dupCount = 0;
				dupSize  = 0;
			}
			
			err = print_indent( inContext, inIndent );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
				
			err = PrintFCore( inContext, "+%04X: %s |%s|\n", (int)( ptr - start ), currHexPtr, ascii );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		
		tempHexPtr = prevHexPtr;
		prevHexPtr = currHexPtr;
		currHexPtr = tempHexPtr;
		
		ptr  += chunkSize;
		size -= chunkSize;
		if( size <= 0 ) break;
	}
	
	if( dupCount > 0 )
	{
		err = print_indent( inContext, inIndent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		err = PrintFCore( inContext, "* (%zu more identical bytes, %zu total)\n", dupSize, dupSize + 16 );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	if( inSize > inMaxSize )
	{
		err = print_indent( inContext, inIndent );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		err = PrintFCore( inContext, "... %zu more bytes ...\n", inSize - inMaxSize );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteHexOneLine
//===========================================================================================================================

static int	PrintFWriteHexOneLine( PrintFContext *inContext, PrintFFormat *inFormat, const uint8_t *inData, size_t inSize )
{
	int			total;
	int			err;
	size_t		i;
	size_t		j;
	uint8_t		b;
	char		hex[ 3 ];
	char		c;
	
	total = 0;
	require_quiet( inSize > 0, exit );
	
	// Print each byte as hex.
	
	if( inFormat->altForm != 2 )
	{
		for( i = 0; i < inSize; ++i )
		{
			j = 0;
			if( i != 0 ) hex[ j++ ] = ' ';
			b = inData[ i ];
			hex[ j++ ] = kHexDigitsLowercase[ ( b >> 4 ) & 0x0F ];
			hex[ j++ ] = kHexDigitsLowercase[   b        & 0x0F ];
			err = inContext->callback( hex, j, inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
	// Print each byte as ASCII if requested.
	
	if( inFormat->altForm > 0 )
	{
		if( total > 0 ) err = inContext->callback( " |", 2, inContext );
		else			err = inContext->callback(  "|", 1, inContext );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
		
		for( i = 0; i < inSize; ++i )
		{
			c = (char) inData[ i ];
			if( ( c < 0x20 ) || ( c >= 0x7F ) ) c = '^';
			
			err = inContext->callback( &c, 1, inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		
		err = inContext->callback( "|", 1, inContext );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteHexByteStream
//===========================================================================================================================

static int	PrintFWriteHexByteStream( PrintFContext *inContext, const uint8_t *inData, size_t inSize )
{
	int					total;
	int					err;
	const uint8_t *		src;
	const uint8_t *		end;
	char				buf[ 64 ];
	char *				dst;
	char *				lim;
	
	total = 0;
	src = inData;
	end = src + inSize;
	dst = buf;
	lim = dst + sizeof( buf );
	
	while( src < end )
	{
		uint8_t		b;
		
		if( dst == lim )
		{
			err = inContext->callback( buf, (size_t)( dst - buf ), inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
			
			dst = buf;
		}
		
		b = *src++;
		*dst++ = kHexDigitsLowercase[ ( b >> 4 ) & 0x0F ];
		*dst++ = kHexDigitsLowercase[   b        & 0x0F ];
	}
	if( dst != buf )
	{
		err = inContext->callback( buf, (size_t)( dst - buf ), inContext );
		require_action_quiet( err >= 0, exit, total = err );
		total += err;
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteIE
//===========================================================================================================================

#define PTR_LEN_LEN( SRC, END )		( SRC ), (size_t)( ( END ) - ( SRC ) ), (size_t)( ( END ) - ( SRC ) )

static int	PrintFWriteIE( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen )
{
	const unsigned int		kIndent		= inFormat->fieldWidth * 4;
	const unsigned int		kLabelWidth	= 27;
	int						total;
	int						n;
	const uint8_t *			src;
	const uint8_t *			end;
	const uint8_t *			ptr;
	const uint8_t *			next;
	size_t					len;
	uint8_t					eid;
	uint32_t				vid;
	
	total = 0;
	
	src = (const uint8_t *) inPtr;
	end = src + inLen;
	for( ; src < end; src = next )
	{
		len = (size_t)( end - src );
		if( len < 2 )
		{
			n = PrintFCore( inContext, "### bad IE header:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
			goto exit;
		}
		
		eid  = src[ 0 ];
		len  = src[ 1 ];
		ptr  = src + 2;
		next = ptr + len;
		if( ( next < src ) || ( next > end ) )
		{
			n = PrintFCore( inContext, "### bad IE length:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
			goto exit;
		}
		
		switch( eid )
		{
			case 0: // SSID
				n = PrintFCore( inContext, "%*s%3d %-*s '%.*s'\n", kIndent, "", eid, kLabelWidth, "SSID", (int) len, ptr );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
			
			case 1: // Supported Rates
				n = PrintFCore( inContext, "%*s%3d %-*s %2.2H", kIndent, "", eid, kLabelWidth, "Supported Rates", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
			
			case 3: // DS Parameter Set
				n = PrintFCore( inContext, "%*s%3d %-*s %2.2H", kIndent, "", eid, kLabelWidth, "DS Parameter Set", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
			
			case 50: // Extended Supported Rates
				n = PrintFCore( inContext, "%*s%3d %-*s %2.2H", kIndent, "", eid, kLabelWidth, "Extended Supported Rates", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
			
			case 221: // Vendor Specific
				if( ( next - ptr ) < 4 )
				{
					n = PrintFCore( inContext, "### bad vendor IE:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
					require_action_quiet( n >= 0, exit, total = n );
					total += n;
					goto exit;
				}
				
				vid = (uint32_t)( ( ptr[ 0 ] << 24 ) | ( ptr[ 1 ] << 16 ) | ( ptr[ 2 ] << 8 ) | ptr[ 3 ] );
				ptr += 4;
				len -= 4;
				
				switch( vid )
				{
					case 0x00039301: // Apple General IE
					{
						uint8_t			productID;
						uint16_t		flags;
						
						if( ( next - ptr ) < 3 )
						{
							n = PrintFCore( inContext, "### bad Apple General IE:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
							require_action_quiet( n >= 0, exit, total = n );
							total += n;
							goto exit;
						}
						
						productID	= ptr[ 0 ];
						flags		= (uint16_t)( ( ptr[ 1 ] << 8 ) | ptr[ 2 ] );
						n = PrintFCore( inContext, "%*s%3d %-*s product ID %d flags 0x%x\n", kIndent, "", eid, kLabelWidth, "Apple General IE", 
							productID, flags );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						break;
					}
					
					case 0x0017F201: // DWDS
						n = PrintFCore( inContext, "%*s%3d %-*s %2.2H", kIndent, "", eid, kLabelWidth, "DWDS", 
							ptr, len, len );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						break;
					
					case 0x0050F204: // WPS
						n = PrintFCore( inContext, "%*s%3d %-*s\n", kIndent, "", eid, kLabelWidth, "WPS" );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						
						n = PrintFWriteIE_WPS( inContext, inFormat, ptr, len );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						break;
					
					default:
						n = PrintFCore( inContext, "%*s%3d %-*s %2.2H", kIndent, "", eid, 
							kLabelWidth, "<<UNKNOWN VENDOR>", ptr - 4, len + 4, len + 4 );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						break;
				}
				break;
			
			default: // Unknown
				n = PrintFCore( inContext, "%*s%3d %-*s %2.2H\n", kIndent, "", eid, kLabelWidth, "<<UNKNOWN>>", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
		}
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteIE_WPS
//===========================================================================================================================

static int	PrintFWriteIE_WPS( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen )
{
	const unsigned int		kIndent = ( inFormat->fieldWidth + 2 ) * 4;
	const unsigned int		kLabelWidth	= 23;
	int						total;
	int						n;
	const uint8_t *			src;
	const uint8_t *			end;
	const uint8_t *			ptr;
	const uint8_t *			next;
	size_t					len;
	uint16_t				type;
	
	total = 0;
	
	src = (const uint8_t *) inPtr;
	end = src + inLen;
	for( ; src < end; src = next )
	{
		len = (size_t)( end - src );
		if( len < 4 )
		{
			n = PrintFCore( inContext, "### WPS TLV too small for header:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
			goto exit;
		}
		
		type = (uint16_t)( ( src[ 0 ] << 8 ) | src[ 1 ] );
		len  = (size_t)(   ( src[ 2 ] << 8 ) | src[ 3 ] );
		ptr  = src + 4;
		next = ptr + len;
		if( ( next < src ) || ( next > end ) )
		{
			n = PrintFCore( inContext, "### WPS TLV bad length:\n%2.1H\n", PTR_LEN_LEN( src, end ) );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
			goto exit;
		}
		
		switch( type )
		{
			case 0x1008: // ConfigMethods
				n = PrintFCore( inContext, "%*s%-*s %3.2H", kIndent, "", kLabelWidth, "ConfigMethods", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
			
			default:
				n = PrintFCore( inContext, "%*s%-*s %3.2H", kIndent, "", kLabelWidth, "<<UNKNOWN>>", 
					ptr, len, len );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				break;
		}
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteMultiLineText
//===========================================================================================================================

static int	PrintFWriteMultiLineText( PrintFContext *inContext, PrintFFormat *inFormat, const char *inStr, size_t inLen )
{
	int					total, err;
	const char *		line;
	const char *		end;
	const char *		eol;
	const char *		next;
	unsigned int		i, n;
	
	total = 0;
	for( line = inStr, end = line + inLen; line < end; line = next )
	{
		for( eol = line; ( eol < end ) && ( *eol != '\r' ) && ( *eol != '\n' ); ++eol ) {}
		if( eol < end )
		{
			if( ( eol[ 0 ] == '\r' ) && ( ( ( eol + 1 ) < end ) && ( eol[ 1 ] == '\n' ) ) )
			{
				next = eol + 2;
			}
			else
			{
				next = eol + 1;
			}
		}
		else
		{
			next = eol;
		}
		if( ( line < eol ) && ( *line != '\r' ) && ( *line != '\n' ) )
		{
			n = inFormat->fieldWidth;
			for( i = 0; i < n; ++i )
			{
				err = inContext->callback( "    ", 4, inContext );
				require_action_quiet( err >= 0, exit, total = err );
				total += err;
			}
		}
		if( line < eol )
		{
			err = inContext->callback( line, (size_t)( eol - line ), inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		if( eol < end )
		{
			err = inContext->callback( "\n", 1, inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteNumVersion
//===========================================================================================================================

static int	PrintFWriteNumVersion( uint32_t inVersion, char *outStr )
{
	char *		dst;
	char *		lim;
	uint8_t		majorRev;
	uint8_t		minor;
	uint8_t		bugFix;
	uint8_t		stage;
	uint8_t		revision;
		
	majorRev 	= (uint8_t)( ( inVersion >> 24 ) & 0xFF );
	minor		= (uint8_t)( ( inVersion >> 20 ) & 0x0F );
	bugFix		= (uint8_t)( ( inVersion >> 16 ) & 0x0F );
	stage 		= (uint8_t)( ( inVersion >>  8 ) & 0xFF );
	revision 	= (uint8_t)(   inVersion         & 0xFF );
	
	// Convert the major, minor, and bugfix numbers. Bugfix only added if it is non-zero (i.e. print 6.2 and not 6.2.0).
	
	dst  = outStr;
	lim  = dst + kPrintFBufSize;
	SNPrintF_Add( &dst, lim, "%u", majorRev );
	SNPrintF_Add( &dst, lim, ".%u", minor );
	if( bugFix != 0 ) SNPrintF_Add( &dst, lim, ".%u", bugFix );
	
	// Convert the version stage and non-release revision number.
	
	switch( stage )
	{
		case kVersionStageDevelopment:	SNPrintF_Add( &dst, lim, "d%u", revision ); break;
		case kVersionStageAlpha:		SNPrintF_Add( &dst, lim, "a%u", revision ); break;
		case kVersionStageBeta:			SNPrintF_Add( &dst, lim, "b%u", revision ); break;
		case kVersionStageFinal:
			
			// A non-release revision of zero is a special case indicating the software is GM (at the golden master 
			// stage) and therefore, the non-release revision should not be added to the string.
			
			if( revision != 0 ) SNPrintF_Add( &dst, lim, "f%u", revision );
			break;
		
		default:
			SNPrintF_Add( &dst, lim, "<< ERROR: invalid NumVersion stage: 0x%02X >>", revision );
			break;
	}
	return( (int)( dst - outStr ) );
}

//===========================================================================================================================
//	PrintFWriteSingleLineText
//===========================================================================================================================

static int	PrintFWriteSingleLineText( PrintFContext *inContext, const char *inStr, size_t inLen )
{
	int					total, err;
	const char *		src;
	const char *		end;
	const char *		ptr;
	
	total = 0;
	src = inStr;
	end = inStr + inLen;
	while( src < end )
	{
		for( ptr = src; ( src < end ) && ( *src != '\r' ) && ( *src != '\n' ); ++src ) {}
		if( ptr < src )
		{
			err = inContext->callback( ptr, (size_t)( src - ptr ), inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
		
		for( ptr = src; ( src < end ) && ( ( *src == '\r' ) || ( *src == '\n' ) ); ++src ) {}
		if( ( ptr < src ) && ( src < end ) )
		{
			err = inContext->callback( " ⏎ ", sizeof_string( " ⏎ " ), inContext );
			require_action_quiet( err >= 0, exit, total = err );
			total += err;
		}
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteString
//===========================================================================================================================

static int	PrintFWriteString( const char *inStr, PrintFFormat *inFormat, char *inBuf, const char **outStrPtr )
{
	int					i;
	const char *		s;
	PrintFFormat *		F;
	int					c;
	
	s = inStr;
	F = inFormat;
	if( F->altForm == 0 )		// %s: C-string
	{
		i = 0;
		if( F->havePrecision )
		{
			int		j;
			
			while( ( i < (int) F->precision ) && ( s[ i ] != '\0' ) ) ++i;
			
			// Make sure we don't truncate in the middle of a UTF-8 character.
			// If the last character is part of a multi-byte UTF-8 character, back up to the start of it.
			// If the actual count of UTF-8 characters matches the encoded UTF-8 count, add it back.
			
			c = 0;
			j = 0;
			while( ( i > 0 ) && ( ( c = s[ i - 1 ] ) & 0x80 ) ) { ++j; --i; if( ( c & 0xC0 ) != 0x80 ) break; }
			if( ( j > 1 ) && ( j <= 6 ) )
			{
				int		test;
				int		mask;
				
				test = ( 0xFF << ( 8 - j ) ) & 0xFF;
				mask = test | ( 1 << ( ( 8 - j ) - 1 ) );
				if( ( c & mask ) == test ) i += j;
			}
		}
		else
		{
			while( s[ i ] != '\0' ) ++i;
		}
	}
	else if( F->altForm == 1 )	// %#s: Pascal-string
	{
		i = *s++;
	}
	else if( F->altForm == 2 )	// %##s: DNS label-sequence name
	{
		const uint8_t *		a;
		char *				dst;
		char *				lim;
		
		a = (const uint8_t *) s;
		dst = inBuf;
		lim = dst + kPrintFBufSize;
		if( *a == 0 ) *dst++ = '.';	// Special case for root DNS name.
		while( *a )
		{
			if( *a > 63 )
			{
				SNPrintF_Add( &dst, lim, "<<INVALID DNS LABEL LENGTH %u>>", *a );
				break;
			}
			if( ( dst + *a ) >= &inBuf[ 254 ] )
			{
				SNPrintF_Add( &dst, lim, "<<DNS NAME TOO LONG>>" );
				break;
			}
			SNPrintF_Add( &dst, lim, "%#s.", a );
			a += ( 1 + *a );
		}
		i = (int)( dst - inBuf );
		s = inBuf;
	}
	else if( F->altForm == 3 )	// %###s: Cleansed function name (i.e. isolate just the [<class>::]<function> part).
	{
		const char *		functionStart;
		
		// This needs to handle function names with the following forms:
		//
		// main
		// main(int, const char **)
		// int main(int, const char **)
		// MyClass::operator()
		// MyClass::operator'()'
		// const char * MyClass::MyFunction(const char *) const
		// void *MyClass::MyFunction(const char *) const
		// +[MyClass MyMethod]
		// -[MyClass MyMethod:andParam2:]
		
		functionStart = inStr;
		if( ( *functionStart == '+' ) || ( *functionStart == '-' ) ) // Objective-C class or instance method.
		{
			s = functionStart + strlen( functionStart );
		}
		else
		{
			for( s = inStr; ( ( c = *s ) != '\0' ) && ( c != ':' ); ++s )
			{
				if( c == ' ' ) functionStart = s + 1;
			}
			if( c == ':' ) c = *( ++s );
			if( c == ':' ) ++s;
			else
			{
				// Non-C++ function so re-do the search for a C function name.
				
				functionStart = inStr;
				for( s = inStr; ( ( c = *s ) != '\0' ) && ( c != '(' ); ++s )
				{
					if( c == ' ' ) functionStart = s + 1;
				}
			}
			for( ; ( ( c = *s ) != '\0' ) && ( c != ' ' ) && ( c != '(' ); ++s ) {}
			if( (      s[ 0 ] == '(' ) && ( s[ 1 ] == ')' ) && ( s[ 2 ] == '('   ) ) s += 2;
			else if( ( s[ 0 ] == '(' ) && ( s[ 1 ] == ')' ) && ( s[ 2 ] == '\''  ) ) s += 3;
			if( ( functionStart < s ) && ( *functionStart == '*' ) ) ++functionStart;
		}
		i = (int)( s - functionStart );
		s = functionStart;
	}
	else
	{
		i = SNPrintF( inBuf, kPrintFBufSize, "<< ERROR: %%s with too many #'s (%d) >>", F->altForm );
		s = inBuf;
	}
	
	// Make sure we don't truncate in the middle of a UTF-8 character.
	
	if( F->havePrecision && ( i > (int) F->precision ) )
	{
		for( i = (int) F->precision; ( i > 0 ) && ( ( s[ i ] & 0xC0 ) == 0x80 ); --i ) {}
	}
	*outStrPtr = s;
	return( i );
}

//===========================================================================================================================
//	PrintFWriteText
//===========================================================================================================================

static int	PrintFWriteText( PrintFContext *inContext, PrintFFormat *inFormat, const char *inText, size_t inSize )
{
	int		err;
	int		nTotal;
	int		n;
	
	nTotal = 0;
	n = (int) inSize;
	if( inFormat->prefix != '\0' ) n += 1;
	if( inFormat->suffix != '\0' ) n += 1;
	
	// Pad on the left.
	
	if( !inFormat->leftJustify && ( n < (int) inFormat->fieldWidth ) )
	{
		do
		{
			err = inContext->callback( " ", 1, inContext );
			if( err < 0 ) goto error;
			nTotal += 1;
			
		}	while( n < (int) --inFormat->fieldWidth );
	}
	
	// Write the prefix (if any).
	
	if( inFormat->prefix != '\0' )
	{
		err = inContext->callback( &inFormat->prefix, 1, inContext );
		if( err < 0 ) goto error;
		nTotal += 1;
	}
	
	// Write the actual text.
	
	err = inContext->callback( inText, inSize, inContext );
	if( err < 0 ) goto error;
	nTotal += (int) inSize;
	
	// Write the suffix (if any).
	
	if( inFormat->suffix != '\0' )
	{
		err = inContext->callback( &inFormat->suffix, 1, inContext );
		if( err < 0 ) goto error;
		nTotal += 1;
	}
	
	// Pad on the right.
	
	for( ; n < (int) inFormat->fieldWidth; ++n )
	{
		err = inContext->callback( " ", 1, inContext );
		if( err < 0 ) goto error;
		nTotal += 1;
	}
	
	return( nTotal );

error:
	return( err );
}

//===========================================================================================================================
//	PrintFWriteTimeDuration
//
//	Converts seconds into a days, hours, minutes, and seconds string. For example: 930232 -> "10d 18h 23m 52s".
//===========================================================================================================================

static int	PrintFWriteTimeDuration( uint64_t inSeconds, int inAltForm, char *inBuf )
{
	unsigned int		years;
	unsigned int		remain;
	unsigned int		days;
	unsigned int		hours;
	unsigned int		minutes;
	unsigned int		seconds;
	unsigned int		x;
	char *				dst;
	
	years	= (unsigned int)( inSeconds / kSecondsPerYear );
	remain	= (unsigned int)( inSeconds % kSecondsPerYear );
	days    = remain / kSecondsPerDay;
	remain	= remain % kSecondsPerDay;
	hours	= remain / kSecondsPerHour;
	remain	= remain % kSecondsPerHour;
	minutes	= remain / kSecondsPerMinute;
	seconds	= remain % kSecondsPerMinute;
	
	dst = inBuf;
	if( years != 0 )
	{
		append_decimal_string( years, dst );
		*dst++ = 'y';
	}
	if( days != 0 )
	{
		if( dst != inBuf ) *dst++ = ' ';
		append_decimal_string( days, dst );
		*dst++ = 'd';
	}
	x = hours;
	if( x != 0 )
	{
		if( dst != inBuf ) *dst++ = ' ';
		if( inAltForm && ( x < 10 ) ) *dst++ = '0';
		append_decimal_string( x, dst );
		*dst++ = inAltForm ? ':' : 'h';
	}
	x = minutes;
	if( ( x != 0 ) || inAltForm )
	{
		if( !inAltForm && ( dst != inBuf ) )				*dst++ = ' ';
		if( inAltForm  && ( x < 10 ) && ( hours != 0 ) )	*dst++ = '0';
		append_decimal_string( x, dst );
		*dst++ = inAltForm ? ':' : 'm';
	}
	x = seconds;
	if( ( x != 0 ) || ( dst == inBuf ) || inAltForm )
	{
		if( !inAltForm && ( dst != inBuf ) ) *dst++ = ' ';
		if( inAltForm  && ( x < 10 ) )		 *dst++ = '0';
		append_decimal_string( x, dst );
		if( !inAltForm ) *dst++ = 's';
	}
	*dst = '\0';
	return( (int)( dst - inBuf ) );
}

//===========================================================================================================================
//	PrintFWriteTXTRecord
//===========================================================================================================================

static int	PrintFWriteTXTRecord( PrintFContext *inContext, PrintFFormat *inFormat, const void *inPtr, size_t inLen )
{
	const unsigned int		kIndent = inFormat->fieldWidth * 4;
	int						total;
	int						n;
	const uint8_t *			buf;
	const uint8_t *			src;
	const uint8_t *			end;
	size_t					len;
	uint8_t					c;
	
	total = 0;
	buf = (const uint8_t *) inPtr;
	src = buf;
	end = src + inLen;
	
	// Handle AirPort TXT records that are one big entry with comma-separated name=value pairs.
	
	if( ( inLen >= 6 ) && ( memcmp( &src[ 1 ], "waMA=", 5 ) == 0 ) )
	{
		uint8_t			tempBuf[ 256 ];
		uint8_t *		tempPtr;
		
		len = *src++;
		if( ( src + len ) != end )
		{
			n = PrintFCore( inContext, "%*s### bad TXT record length byte (%zu, %zu expected)\n", 
				kIndent, "", len, (size_t)( end - src ) );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
			goto exit;
		}
		while( src < end )
		{
			tempPtr = tempBuf;
			while( src < end )
			{
				c = *src++;
				if( c == ',' ) break;
				if( c == '\\' )
				{
					if( src >= end )
					{
						n = PrintFCore( inContext, "%*s### bad TXT escape: %.*s\n", kIndent, "", 
							(int)( inLen - 1 ), buf + 1 );
						require_action_quiet( n >= 0, exit, total = n );
						total += n;
						goto exit;
					}
					c = *src++;
				}
				*tempPtr++ = c;
			}
			
			n = PrintFCore( inContext, "%*s%.*s\n", kIndent, "", (int)( tempPtr - tempBuf ), tempBuf );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
		}
	}
	else
	{
		for( ; src < end; src += len )
		{
			len = *src++;
			if( ( src + len ) > end )
			{
				n = PrintFCore( inContext, "%*s### TXT record length byte too big (%zu, %zu max)\n", 
					kIndent, "", len, (size_t)( end - src ) );
				require_action_quiet( n >= 0, exit, total = n );
				total += n;
				goto exit;
			}
			n = PrintFCore( inContext, "%*s%.*s\n", kIndent, "", (int) len, src );
			require_action_quiet( n >= 0, exit, total = n );
			total += n;
		}
	}
	if( ( inLen == 0 ) || ( buf[ 0 ] == 0 ) )
	{
		n = PrintFCore( inContext, "\n" );
		require_action_quiet( n >= 0, exit, total = n );
		total += n;
	}
	
exit:
	return( total );
}

//===========================================================================================================================
//	PrintFWriteUnicodeString
//===========================================================================================================================

static int	PrintFWriteUnicodeString( const uint8_t *inStr, PrintFFormat *inFormat, char *inBuf )
{
	int						i;
	const uint8_t *			a;
	const uint16_t *		u;
	PrintFFormat *			F;
	char *					p;
	char *					q;
	int						endianIndex;
	
	a = inStr;
	F = inFormat;
	if( !F->havePrecision || ( F->precision > 0 ) )
	{
		if(      ( a[ 0 ] == 0xFE ) && ( a[ 1 ] == 0xFF ) ) { F->altForm = 1; a += 2; --F->precision; } // Big Endian
		else if( ( a[ 0 ] == 0xFF ) && ( a[ 1 ] == 0xFE ) ) { F->altForm = 2; a += 2; --F->precision; } // Little Endian
	}
	u = (const uint16_t *) a;
	p = inBuf;
	q = p + kPrintFBufSize;
	switch( F->altForm )
	{
		case 0:	// Host Endian
			for( i = 0; ( !F->havePrecision || ( i < (int) F->precision ) ) && u[ i ] && ( ( q - p ) > 0 ); ++i )
			{
				*p++ = PrintFMakePrintable( u[ i ] );
			}
			break;
		
		case 1:	// Big Endian
		case 2:	// Little Endian
			endianIndex = 1 - ( F->altForm - 1 );
			for( i = 0; ( !F->havePrecision || ( i < (int) F->precision ) ) && u[ i ] && ( ( q - p ) > 0 ); ++i )
			{
				*p++ = PrintFMakePrintable( a[ endianIndex ] );
				a += 2;
			}
			break;
		
		default:
			i = SNPrintF( inBuf, kPrintFBufSize, "<< ERROR: %%S with too many #'s (%d) >>", F->altForm );
			break;
	}
	return( i );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	PrintFCallBackFixedString
//===========================================================================================================================

static int	PrintFCallBackFixedString( const char *inStr, size_t inSize, PrintFContext *inContext )
{
	size_t		n;
	
	// If the string is too long, truncate it, but don't truncate in the middle of a UTF-8 character.
	
	n = inContext->reservedSize - inContext->usedSize;
	if( inSize > n )
	{
		while( ( n > 0 ) && ( ( inStr[ n ] & 0xC0 ) == 0x80 ) ) --n;
		inSize = n;
	}
	
	// Copy the string (excluding any null terminator).
	
	if( inSize > 0 ) memcpy( inContext->str + inContext->usedSize, inStr, inSize );
	inContext->usedSize += inSize;
	return( (int) inSize );
}

//===========================================================================================================================
//	PrintFCallBackAllocatedString
//===========================================================================================================================

static int	PrintFCallBackAllocatedString( const char *inStr, size_t inSize, PrintFContext *inContext )
{
	int			result;
	size_t		n;
	
	// If there's not enough room in the buffer, resize it. Amortize allocations by rounding the size up.
	
	n = inContext->usedSize + inSize;
	if( n > inContext->reservedSize )
	{
		char *		tmp;
		
		if( n < 256 ) n = 256;
		else		  n = ( n + 1023 ) & ~1023U;
		
		#if( TARGET_NO_REALLOC )
			tmp = (char *) malloc( n );
			require_action( tmp, exit, result = kNoMemoryErr );
			memcpy( tmp, inContext->str, inContext->usedSize );
			
			free( inContext->str );
			inContext->str = tmp;
			inContext->reservedSize = n;
		#else
			tmp = (char *) realloc( inContext->str, n );
			require_action( tmp, exit, result = kNoMemoryErr );
			inContext->str = tmp;
			inContext->reservedSize = n;
		#endif
	}
	
	// Copy the string (excluding any null terminator).
	
	memcpy( inContext->str + inContext->usedSize, inStr, inSize );
	inContext->usedSize += inSize;
	result = (int) inSize;
	
exit:
	return( result );
}

//===========================================================================================================================
//	PrintFCallBackUserCallBack
//===========================================================================================================================

static int	PrintFCallBackUserCallBack( const char *inStr, size_t inSize, PrintFContext *inContext )
{
	return( inContext->userCallBack( inStr, inSize, inContext->userContext ) );
}

//===========================================================================================================================
//	PrintF_IPv6AddressToCString
//===========================================================================================================================

static char *	PrintF_IPv6AddressToCString( const uint8_t inAddr[ 16 ], uint32_t inScope, int inPort, char *inBuffer )
{
	int						v4Mapped;
	char *					dst;
	int						i;
	int						j;
	int						k;
	int						x;
	int						skip;
	uint8_t					u8;
	uint8_t					u4;
	char					segs[ 8 ][ 5 ];
	char *					seg;
	int						runs[ 8 ];
	char					c;
	
	dst = inBuffer;
	
	// Address must be []-wrapped if there is a port number.
	
	if( inPort > 0 )
	{
		*dst++ = '[';
	}
	
	// Treat IPv4-mapped/compatible addresses specially.
	
	v4Mapped = IsIPv4MappedIPv6Address( inAddr );
	if( v4Mapped || IsIPv4CompatibleIPv6Address( inAddr ) )
	{
		*dst++ = ':';
		*dst++ = ':';
		if( v4Mapped )
		{
			*dst++ = 'f';
			*dst++ = 'f';
			*dst++ = 'f';
			*dst++ = 'f';
			*dst++ = ':';
		}
		for( i = 12; i < 16; ++i )
		{
			x = inAddr[ i ];
			append_decimal_string( x, dst );
			if( i != 15 ) *dst++ = '.';
		}
		goto trailer;
	}
	
	// Build an array of 16-bit string segments from the 16-byte IPv6 address.
	
	k = 0;
	for( i = 0; i < 16; )
	{
		seg  = &segs[ k ][ 0 ];
		skip = 1;
		j    = 0;
		
		u8 = inAddr[ i++ ];
		u4 = (uint8_t)( u8 >> 4 );
		if( u4 != 0 )
		{
			skip = 0;
			seg[ j++ ] = kHexDigitsLowercase[ u4 ];
		}
		u4 = (uint8_t)( u8 & 0x0F );
		if( ( skip == 0 ) || ( ( skip == 1 ) && ( u4 != 0 ) ) )
		{
			skip = 0;
			seg[ j++ ] = kHexDigitsLowercase[ u4 ];
		}
		
		u8 = inAddr[ i++ ];
		u4 = (uint8_t)( u8 >> 4 );
		if( ( skip == 0 ) || ( ( skip == 1 ) && ( u4 != 0 ) ) )
		{
			seg[ j++ ] = kHexDigitsLowercase[ u4 ];
		}
		
		u4 = (uint8_t)( u8 & 0x0F );
		seg[ j++ ] = kHexDigitsLowercase[ u4 ];
		seg[ j ] = '\0';
		
		++k;
	}
	
	// Find runs of zeros to collapse into :: notation.
	
	j = 0;
	for( i = 7; i >= 0; --i )
	{
		x = i * 2;
		if( ( inAddr[ x ] == 0 ) && ( inAddr[ x + 1  ] == 0 ) ) ++j;
		else													j = 0;
		runs[ i ] = j;
	}
	
	// Find the longest run of zeros.
	
	k = -1;
	j = 0;
	for( i = 0; i < 8; ++i )
	{
		x = runs[ i ];
		if( x > j )
		{
			k = i;
			j = x;
		}
	}
	
	// Build the string.
	
	for( i = 0; i < 8; ++i )
	{
		if( i == k )
		{
			if( i == 0 ) *dst++ = ':';
			*dst++ = ':';
			i += ( runs[ i ] - 1 );
			continue;
		}
		seg = &segs[ i ][ 0 ];
		for( j = 0; ( c = seg[ j ] ) != '\0'; ++j ) *dst++ = c;
		if( i != 7 ) *dst++ = ':';
	}
		
trailer:
	if( inScope > 0 )	// Scope
	{
		*dst++ = '%';
		#if( TARGET_OS_POSIX )
		{
			char		buf[ 64 ];
			char *		p;
			
			check_compile_time_code( countof( buf ) >= IFNAMSIZ );
			
			p = if_indextoname( (unsigned int) inScope, buf );
			if( p ) while( ( c = *p++ ) != '\0' ) *dst++ = c;
			else	append_decimal_string( inScope, dst );
		}
		#else
			append_decimal_string( inScope, dst );
		#endif
	}
	if( inPort > 0 )	// Port
	{
		*dst++ = ']';
		*dst++ = ':';
		append_decimal_string( inPort, dst );
	}
	*dst = '\0';
	return( inBuffer );
}

//===========================================================================================================================
//	PrintF_IPv4AddressToCString
//===========================================================================================================================

static char *	PrintF_IPv4AddressToCString( uint32_t inIP, int inPort, char *inBuffer )
{
	char *		dst;
	int			i;
	int			x;
	
	dst = inBuffer;
	for( i = 1; i <= 4; ++i )
	{
		x = ( inIP >> ( 32 - ( 8 * i ) ) ) & 0xFF;
		append_decimal_string( x, dst );
		if( i < 4 ) *dst++ = '.';
	}
	if( inPort > 0 )
	{
		*dst++ = ':';
		append_decimal_string( inPort, dst );
	}
	*dst = '\0';
	return( inBuffer );
}

#if 0
#pragma mark -
#endif

#if( !defined( PRINTF_UTILS_PRINT_TEST ) )
	#if( DEBUG_FPRINTF_ENABLED )
		#define	PRINTF_UTILS_PRINT_TEST		1
	#endif
#endif

#if 0
#pragma mark -
#pragma mark == LogUtils ==
#endif

//===========================================================================================================================
//	LogUtils
//===========================================================================================================================

#if( LOGUTILS_ENABLED )

// LogAction

typedef struct LogAction		LogAction;
struct LogAction
{
	LogAction *		next;
	char *			name;
	char *			variable;
	char *			value;
};

// Prototypes

#if( TARGET_OS_POSIX )
	#define LogUtils_Lock()			pthread_mutex_lock( &gLogUtilsLock )
	#define LogUtils_Unlock()		pthread_mutex_unlock( &gLogUtilsLock )
#elif( TARGET_OS_WINDOWS )
	#define LogUtils_Lock()			EnterCriticalSection( &gLogUtilsLock )
	#define LogUtils_Unlock()		LeaveCriticalSection( &gLogUtilsLock )
#endif

static void	__LogUtils_FreeAction( LogAction *inAction );

#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
	static void	__LogUtils_EnsureCFNotificationsInitialized( void );
	static void
		__LogUtils_HandleCFNotification( 
			CFNotificationCenterRef	inCenter, 
			void *					inObserver, 
			CFStringRef				inName, 
			const void *			inObject, 
			CFDictionaryRef			inUserInfo );
#endif
#if( LOGUTILS_CF_PREFERENCES )
	static void		__LogUtils_ReadCFPreferences( void );
	static OSStatus	__LogUtils_WriteCFPreferences( void );
#endif

static OSStatus		__LogControlLocked( const char *inCmd );
#if( LOGUTILS_CF_ENABLED && LOGUTILS_CF_PREFERENCES )
	static OSStatus	__LogControlLockedCF( CFStringRef inStr );
#endif

static OSStatus		__LogCategory_ApplyActions( LogCategory *inCategory );
static OSStatus		__LogCategory_ApplyAction_Output( LogCategory *inCategory, int inOutputID, LogAction *inAction );
static int			__LogCategory_match( const char *regexp, const char *text );
static int			__LogCategory_matchhere( const char *regexp, const char *text );
static int			__LogCategory_matchstar( int c, const char *regexp, const char *text );
static char *		__LogCategory_LevelToString( LogLevel inLevel, char *inBuf, size_t inLen );
static LogLevel		__LogCategory_StringToLevel( const char *inStr );

static OSStatus		__LogOutputCreate( const char *inConfigStr, LogOutput **outOutput );
static void			__LogOutputDelete( LogOutput *inOutput );
static void			__LogOutputDeleteUnused( void );

#if( DEBUG_FPRINTF_ENABLED )
	static OSStatus	__LogOutputFile_Setup( LogOutput *inOutput, const char *inParams );
	static void		__LogOutputFile_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
	static OSStatus	__LogOutputFile_BackupLogFiles( LogOutput *inOutput );
	static OSStatus	__LogOutputFile_CopyLogFile( const char *inSrcPath, const char *inDstPath );
#endif

#if( TARGET_OS_POSIX )
	static OSStatus	__LogOutputSysLog_Setup( LogOutput *inOutput, const char *inParams );
	static void		__LogOutputSysLog_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
#endif

#if( TARGET_OS_WINDOWS )
	static void		__LogOutputWindowsDebugger_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
#endif

#if( DEBUG_WINDOWS_EVENT_LOG_ENABLED )
	static OSStatus	__LogOutputWindowsEventLog_Setup( LogOutput *inOutput, const char *inParams );
	static void		__LogOutputWindowsEventLog_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
#endif

#if( TARGET_OS_WINDOWS_KERNEL )
	static void		__LogOutputWindowsKernel_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
#endif

static OSStatus		__LogOutputCallBack_Setup( LogOutput *inOutput, const char *inParams );
static void			__LogOutputCallBack_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );

#if( TARGET_OS_POSIX )
	static Boolean	__LogOutput_IsStdErrMappedToDevNull( void );
#else
	#define __LogOutput_IsStdErrMappedToDevNull()		0
#endif

// Globals

#if( TARGET_OS_POSIX )
	static pthread_mutex_t		gLogUtilsLock = PTHREAD_MUTEX_INITIALIZER;
#elif( TARGET_OS_WINDOWS )
	static CRITICAL_SECTION		gLogUtilsLock;
	static LONG					gLogUtilsLockState = 0;
#endif

Boolean							gLogUtilsInitializing = false;
#if( LOGUTILS_CF_ENABLED )
	static Boolean				gLogCFInitialized	= false;
#endif
#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
	static Boolean				gLogCFNotificationInitialized	= false;
	static CFStringRef			gLogCFNotificationObserver		= CFSTR( "LogUtilsCFObserver" );
	static CFStringRef			gLogCFNotificationPoster		= CFSTR( "LogUtilsCFPoster" );
#endif
#if( LOGUTILS_CF_PREFERENCES )
	static CFStringRef			gLogCFPrefsAppID = NULL;
#endif

static LogCategory *			gLogCategoryList	= NULL;
static LogAction *				gLogActionList		= NULL;
static LogOutput *				gLogOutputList		= NULL;

ulog_define( LogUtils, kLogLevelAll, kLogFlags_PrintTime, "LogUtils", NULL );

//===========================================================================================================================
//	LogUtils_EnsureInitialized
//===========================================================================================================================

OSStatus	LogUtils_EnsureInitialized( void )
{
#if( TARGET_OS_WINDOWS )
	InitializeCriticalSectionOnce( &gLogUtilsLock, &gLogUtilsLockState );
#endif

#if( LOGUTILS_CF_ENABLED )
	if( gLogCFInitialized || gLogUtilsInitializing ) return( kNoErr ); // Avoid recursion.
	LogUtils_Lock();
	gLogUtilsInitializing = true;
	if( !gLogCFInitialized )
	{
		gLogCFInitialized = true; // Mark initialized first to handle recursive invocation.
		
		#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
			__LogUtils_EnsureCFNotificationsInitialized();
		#endif
		
		#if( LOGUTILS_CF_PREFERENCES )
			__LogUtils_ReadCFPreferences();
		#endif
	}
	gLogUtilsInitializing = false;
	LogUtils_Unlock();
#endif
	
	return( kNoErr );
}

//===========================================================================================================================
//	LogUtils_Finalize
//===========================================================================================================================

void	LogUtils_Finalize( void )
{
	LogCategory *		category;
	LogAction *			action;
	LogOutput *			output;
	
#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
	CFNotificationCenterRef		dnc;
	
	dnc = CFNotificationCenterGetDistributedCenter();
	if( dnc ) CFNotificationCenterRemoveEveryObserver( dnc, gLogCFNotificationObserver );
#endif
	
	for( category = gLogCategoryList; category; category = category->next )
	{
		category->level   = kLogLevelUninitialized;
		category->output1 = NULL;
		category->output2 = NULL;
	}
	while( ( action = gLogActionList ) != NULL )
	{
		gLogActionList = action->next;
		__LogUtils_FreeAction( action );
	}
	while( ( output = gLogOutputList ) != NULL )
	{
		gLogOutputList = output->next;
		__LogOutputDelete( output );
	}
	
#if( LOGUTILS_CF_PREFERENCES )
	ForgetCF( &gLogCFPrefsAppID );
#endif
	
#if( TARGET_OS_WINDOWS )
	DeleteCriticalSectionOnce( &gLogUtilsLock, &gLogUtilsLockState, true );
#endif
}

//===========================================================================================================================
//	__LogUtils_FreeAction
//===========================================================================================================================

static void	__LogUtils_FreeAction( LogAction *inAction )
{
	ForgetMem( &inAction->name );
	ForgetMem( &inAction->variable );
	ForgetMem( &inAction->value );
	free( inAction );
}

//===========================================================================================================================
//	__LogUtils_EnsureCFNotificationsInitialized
//
//	Note: LogUtils lock must be held.
//===========================================================================================================================

#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
static void	__LogUtils_EnsureCFNotificationsInitialized( void )
{
	CFNotificationCenterRef		dnc;
	
	if( gLogCFNotificationInitialized ) return;
	
	dnc = CFNotificationCenterGetDistributedCenter();
	if( !dnc ) return;
	
	CFNotificationCenterAddObserver( dnc, gLogCFNotificationObserver, __LogUtils_HandleCFNotification, 
		CFSTR( kLogUtilsRequestNotification ), NULL, CFNotificationSuspensionBehaviorDeliverImmediately );
	gLogCFNotificationInitialized = true;
}
#endif

//===========================================================================================================================
//	__LogUtils_HandleCFNotification
//===========================================================================================================================

#if( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS )
static void
	__LogUtils_HandleCFNotification( 
		CFNotificationCenterRef	inCenter, 
		void *					inObserver, 
		CFStringRef				inName, 
		const void *			inObject, 
		CFDictionaryRef			inUserInfo )
{
	CFStringRef					controlStr;
	CFMutableDictionaryRef		userInfo;
	OSStatus					err;
	char *						showCStr;
	CFStringRef					showCFStr;
	
	(void) inObserver;
	(void) inName;
	(void) inObject;
	
	userInfo = NULL;
	
	// Change the config if a control string is specified.
	
	if( inUserInfo )
	{
		controlStr = (CFStringRef) CFDictionaryGetValue( inUserInfo, CFSTR( kLogUtilsKey_LogConfig ) );
		if( controlStr )
		{
			require( CFIsType( controlStr, CFString ), exit );
			LogControlCF( controlStr );
		}
	}
	
	// Post a notification with the latest config.
	
	userInfo = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require( userInfo, exit );
	
	showCStr = NULL;
	err = LogShow( &showCStr );
	require_noerr( err, exit );
	showCFStr = CFStringCreateWithCString( NULL, showCStr, kCFStringEncodingUTF8 );
	free( showCStr );
	require( showCFStr, exit );
	CFDictionarySetValue( userInfo, CFSTR( kLogUtilsKey_LogConfig ), showCFStr );
	CFRelease( showCFStr );
	
	CFNotificationCenterPostNotificationWithOptions( inCenter, CFSTR( kLogUtilsAckNotification ), 
		gLogCFNotificationPoster, userInfo, kCFNotificationDeliverImmediately | kCFNotificationPostToAllSessions );
	
exit:
	if( userInfo ) CFRelease( userInfo );
}
#endif

//===========================================================================================================================
//	__LogUtils_ReadCFPreferences
//
//	Note: LogUtils lock must be held.
//===========================================================================================================================

#if( LOGUTILS_CF_PREFERENCES )
static void	__LogUtils_ReadCFPreferences( void )
{
	CFStringRef		appID;
	CFStringRef		cfStr;
	
	appID = gLogCFPrefsAppID ? gLogCFPrefsAppID : kCFPreferencesCurrentApplication;
	CFPreferencesAppSynchronize( appID );
	cfStr = (CFStringRef) CFPreferencesCopyAppValue( CFSTR( kLogUtilsKey_LogConfig ), appID );
	if( cfStr )
	{
		if( CFGetTypeID( cfStr ) == CFStringGetTypeID() )
		{
			__LogControlLockedCF( cfStr );
		}
		CFRelease( cfStr );
	}
}
#endif

//===========================================================================================================================
//	__LogUtils_WriteCFPreferences
//
//	Note: LogUtils lock must be held.
//===========================================================================================================================

#if( LOGUTILS_CF_PREFERENCES )
static OSStatus	__LogUtils_WriteCFPreferences( void )
{
	OSStatus		err;
	char *			configStr;
	LogAction *		action;
	CFStringRef		configCFStr;
	CFStringRef		appID;
	
	configStr = NULL;
	for( action = gLogActionList; action; action = action->next )
	{
		AppendPrintF( &configStr, "%s%s:%s=%s", ( action == gLogActionList ) ? "" : ",", 
			action->name, action->variable, action->value );
	}
	require_action_quiet( configStr, exit, err = kNoErr );
	
	configCFStr = CFStringCreateWithCString( NULL, configStr, kCFStringEncodingUTF8 );
	require_action_quiet( configCFStr, exit, err = kNoMemoryErr );
	
	appID = gLogCFPrefsAppID ? gLogCFPrefsAppID : kCFPreferencesCurrentApplication;
	CFPreferencesSetAppValue( CFSTR( kLogUtilsKey_LogConfig ), configCFStr, appID );
	CFPreferencesAppSynchronize( appID );
	CFRelease( configCFStr );
	err = kNoErr;
	
exit:
	if( configStr ) free( configStr );
	return( err );
}
#endif

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	LogControl
//===========================================================================================================================

OSStatus	LogControl( const char *inCmd )
{
	OSStatus		err;
	
	LogUtils_EnsureInitialized();
	LogUtils_Lock();
	err = __LogControlLocked( inCmd );
	LogUtils_Unlock();
	return( err );
}

//===========================================================================================================================
//	__LogControlLocked
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static OSStatus	__LogControlLocked( const char *inCmd )
{
	OSStatus			err;
	int					isDefault;
	int					persist;
	char				c;
	const char *		namePtr;
	size_t				nameLen;
	const char *		variablePtr;
	size_t				variableLen;
	const char *		valuePtr;
	size_t				valueLen;
	char *				valueStr;
	LogAction **		actionNext;
	LogAction *			action;
	LogAction *			actionNew;
	
	actionNew = NULL;
	
	// If the command is NULL or empty, it means to delete all actions.
	
	if( ( inCmd == NULL ) || ( *inCmd == '\0' ) )
	{
		while( ( action = gLogActionList ) != NULL )
		{
			gLogActionList = action->next;
			__LogUtils_FreeAction( action );
		}
		err = kNoErr;
		goto exit;
	}
	
	isDefault = ( *inCmd == '?' );
	if( isDefault ) ++inCmd;
	
	persist = ( *inCmd == '+' );
	if( persist ) ++inCmd;
	
	// Parse actions from the control string. This appends each unique action, replacing duplicate actions.
	
	while( *inCmd != '\0' )
	{
		// Parse a name:variable=value segment.
		
		namePtr = inCmd;
		while( ( ( c = *inCmd ) != '\0' ) && ( c != ':' ) ) ++inCmd;
		require_action_quiet( c != '\0', exit, err = kMalformedErr );
		nameLen = (size_t)( inCmd - namePtr );
		++inCmd;
		
		variablePtr = inCmd;
		while( ( ( c = *inCmd ) != '\0' ) && ( c != '=' ) ) ++inCmd;
		require_action_quiet( c != '\0', exit, err = kMalformedErr );
		variableLen = (size_t)( inCmd - variablePtr );
		++inCmd;
		
		valuePtr = inCmd;
		while( ( ( c = *inCmd ) != '\0' ) && ( c != ',' ) ) ++inCmd;
		valueLen = (size_t)( inCmd - valuePtr );
		if( c == ',' ) ++inCmd;
		
		// Search for an action with the same name/variable. If found, replace. If not found, add.
		
		for( actionNext = &gLogActionList; ( action = *actionNext ) != NULL; actionNext = &action->next )
		{
			if( ( __strnicmpx( namePtr,		nameLen,		action->name )		== 0 ) && 
				( __strnicmpx( variablePtr,	variableLen,	action->variable )	== 0 ) )
			{
				break;
			}
		}
		if( action )
		{
			// Action with same name/variable found, just replace the value.
			
			valueStr = (char *) malloc( valueLen + 1 );
			require_action_quiet( valueStr, exit, err = kNoMemoryErr );
			memcpy( valueStr, valuePtr, valueLen );
			valueStr[ valueLen ] = '\0';
			
			free( action->value );
			action->value = valueStr;
		}
		else
		{
			// Action not found, create a new action.
			
			actionNew = (LogAction *) calloc( 1, sizeof( *actionNew ) );
			require_action_quiet( actionNew, exit, err = kNoMemoryErr );
			
			actionNew->name = (char *) malloc( nameLen + 1 );
			require_action_quiet( actionNew->name, exit, err = kNoMemoryErr );
			memcpy( actionNew->name, namePtr, nameLen );
			actionNew->name[ nameLen ] = '\0';
			
			actionNew->variable = (char *) malloc( variableLen + 1 );
			require_action_quiet( actionNew->variable, exit, err = kNoMemoryErr );
			memcpy( actionNew->variable, variablePtr, variableLen );
			actionNew->variable[ variableLen ] = '\0';
			
			actionNew->value = (char *) malloc( valueLen + 1 );
			require_action_quiet( actionNew->value, exit, err = kNoMemoryErr );
			memcpy( actionNew->value, valuePtr, valueLen );
			actionNew->value[ valueLen ] = '\0';
			
			*actionNext = actionNew;
			actionNew = NULL;
		}
	}
	
	// Re-apply all actions to account for the new action(s).
	
	err = __LogCategory_ApplyActions( NULL );
	require_noerr_quiet( err, exit );
	
#if( LOGUTILS_CF_PREFERENCES )
	if( isDefault )		__LogUtils_ReadCFPreferences();
	else if( persist )	__LogUtils_WriteCFPreferences();
#endif
	
exit:
	if( actionNew ) __LogUtils_FreeAction( actionNew );
	return( err );
}

//===========================================================================================================================
//	LogControlCF
//===========================================================================================================================

#if( LOGUTILS_CF_ENABLED )
OSStatus	LogControlCF( CFStringRef inCmd )
{
	OSStatus		err;
	CFRange			range;
	CFIndex			len;
	char *			cmd;
	
	range = CFRangeMake( 0, CFStringGetLength( inCmd ) );
	len = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
	cmd = (char *) malloc( (size_t)( len + 1 ) );
	require_action( cmd, exit, err = kNoMemoryErr );
	
	if( len > 0 )
	{
		range.location = CFStringGetBytes( inCmd, range, kCFStringEncodingUTF8, 0, false, (uint8_t *) cmd, len, &len );
		require_action( range.location == range.length, exit, err = kUnknownErr );
		require_action( len > 0, exit, err = kNoErr );
	}
	cmd[ len ] = '\0';
	
	err = LogControl( cmd );
	require_noerr( err, exit );
	
exit:
	if( cmd ) free( cmd );
	return( err );
}
#endif

//===========================================================================================================================
//	__LogControlLockedCF
//===========================================================================================================================

#if( LOGUTILS_CF_ENABLED && LOGUTILS_CF_PREFERENCES )
static OSStatus	__LogControlLockedCF( CFStringRef inStr )
{
	OSStatus		err;
	CFRange			range;
	CFIndex			len;
	char *			configStr;
	
	configStr = NULL;
	
	range = CFRangeMake( 0, CFStringGetLength( inStr ) );
	len = CFStringGetMaximumSizeForEncoding( range.length, kCFStringEncodingUTF8 );
	
	configStr = (char *) malloc( (size_t)( len + 1 ) );
	require_action_quiet( configStr, exit, err = kNoMemoryErr );
	
	range.location = CFStringGetBytes( inStr, range, kCFStringEncodingUTF8, 0, false, (uint8_t *) configStr, len, &len );
	require_action_quiet( range.location == range.length, exit, err = kUnknownErr );
	require_action_quiet( len > 0, exit, err = kNoErr );
	configStr[ len ] = '\0';
	
	err = __LogControlLocked( configStr );
	
exit:
	if( configStr ) free( configStr );
	return( err );
}
#endif

//===========================================================================================================================
//	LogSetAppID
//===========================================================================================================================

#if( LOGUTILS_CF_PREFERENCES )
void	LogSetAppID( CFStringRef inAppID )
{
	LogUtils_EnsureInitialized();
	LogUtils_Lock();
	
	if( inAppID )			CFRetain( inAppID );
	if( gLogCFPrefsAppID )	CFRelease( gLogCFPrefsAppID );
	gLogCFPrefsAppID = inAppID;
	
	LogUtils_Unlock();
}
#endif

//===========================================================================================================================
//	LogShow
//===========================================================================================================================

OSStatus	LogShow( char **outOutput )
{
	OSStatus			err;
	char *				outputStr;
	LogCategory *		category;
	LogAction *			action;
	int					n;
	char				buf[ 64 ];
	
	LogUtils_Lock();
	
	outputStr = NULL;
	n = ASPrintF( &outputStr, "=== LogUtils (%s, PID %llu) ===\n", getprogname(), (unsigned long long) getpid() );
	err = ( n > 0 ) ? kNoErr : kNoMemoryErr;
	
	// Categories
	
	if( err == kNoErr )
	{
		size_t		widestName, widestLevel, width;
		
		widestName  = 0;
		widestLevel = 0;
		for( category = gLogCategoryList; category; category = category->next )
		{
			width = strlen( category->name );
			if( width > widestName ) widestName = width;
			
			width = strlen( __LogCategory_LevelToString( category->level, buf, sizeof( buf ) ) );
			if( width > widestLevel ) widestLevel = width;
		}
		for( category = gLogCategoryList; category; category = category->next )
		{
			n = AppendPrintF( &outputStr, "  %-*s  L=%-*s  R=%u/%-5llu  O1=%s  O2=%s\n%s", 
				(int) widestName, category->name, 
				(int) widestLevel, __LogCategory_LevelToString( category->level, buf, sizeof( buf ) ), 
				category->rateMaxCount, UpTicksToMilliseconds( category->rateInterval ), 
				category->output1 ? category->output1->configStr : "", 
				category->output2 ? category->output2->configStr : "", 
				category->next ? "" : "\n" );
			if( n <= 0 ) { err = kNoMemoryErr; break; }
		}
	}
	
	// Actions
	
	if( err == kNoErr )
	{
		for( action = gLogActionList; action; action = action->next )
		{
			n = AppendPrintF( &outputStr, "  Action: %s:%s=%s\n", action->name, action->variable, action->value );
			if( n <= 0 ) { err = kNoMemoryErr; break; }
		}
	}
	
	LogUtils_Unlock();
	
	// Return or print the final string.
	
	if( outputStr )
	{
		if( outOutput ) *outOutput = outputStr;
		else
		{
			lu_ulog( kLogLevelMax, "%s", outputStr );
			free( outputStr );
		}
	}
	else
	{
		if( outOutput == NULL )
		{
			lu_ulog( kLogLevelError, "### ERROR: %#m\n", err );
		}
	}
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	__LogCategory_Initialize
//===========================================================================================================================

Boolean	__LogCategory_Initialize( LogCategory *inCategory, LogLevel inLevel )
{
	LogLevel		level;
	
	if( gLogUtilsInitializing ) return( false );
	LogUtils_EnsureInitialized();
	LogUtils_Lock();
	
	if( inCategory->level == kLogLevelUninitialized )
	{
		LogCategory **		next;
		LogCategory *		curr;
		
		inCategory->level = inCategory->initLevel;
		
		for( next = &gLogCategoryList; ( curr = *next ) != NULL; next = &curr->next )
		{
			if( __strnicmpx( curr->name, SIZE_MAX, inCategory->name ) > 0 )
			{
				break;
			}
		}
		inCategory->next = *next;
		*next = inCategory;
		
		if( inCategory->initConfig )
		{
			__LogControlLocked( inCategory->initConfig );
		}
		__LogCategory_ApplyActions( inCategory );
		if( !inCategory->output1 )
		{
			__LogOutputCreate( "console", &inCategory->output1 );
			if( inCategory->output1 ) ++inCategory->output1->refCount;
		}
	}
	level = inCategory->level;
	
	LogUtils_Unlock();
	return( (Boolean)( ( inLevel & kLogLevelMask ) >= level ) );
}

//===========================================================================================================================
//	__LogCategory_ApplyActions
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static OSStatus	__LogCategory_ApplyActions( LogCategory *inCategory )
{
	OSStatus			err;
	LogAction *			action;
	LogCategory *		category;
	LogLevel			level;
	const char *		valueSrc;
	const char *		valueEnd;
	const char *		valueTok;
	size_t				valueLen;
	LogFlags			flags;
	int					outputID;
	
	// Apply level actions.
	
	for( action = gLogActionList; action; action = action->next )
	{
		if( __strnicmpx( action->variable, SIZE_MAX, "level" ) != 0 )
			continue;
		
		level = __LogCategory_StringToLevel( action->value );
		if( level == kLogLevelUninitialized ) continue;
		
		for( category = gLogCategoryList; category; category = category->next )
		{
			if( inCategory && ( inCategory != category ) ) 				continue;
			if( !__LogCategory_match( action->name, category->name ) )	continue;
			
			category->level = level;
		}
	}
	
	// Apply flag actions.
	
	for( action = gLogActionList; action; action = action->next )
	{
		if( __strnicmpx( action->variable, SIZE_MAX, "flags" ) != 0 )
			continue;
		
		flags = 0;
		valueSrc = action->value;
		valueEnd = valueSrc + strlen( valueSrc );
		while( valueSrc < valueEnd )
		{
			for( valueTok = valueSrc; ( valueSrc < valueEnd ) && ( *valueSrc != ';' ); ++valueSrc ) {}
			valueLen = (size_t)( valueSrc - valueTok );
			if( valueSrc < valueEnd ) ++valueSrc;
			
			if(      __strnicmpx( valueTok, valueLen, "none" )		== 0 ) flags  = kLogFlags_None;
			else if( __strnicmpx( valueTok, valueLen, "time" )		== 0 ) flags |= kLogFlags_PrintTime;
			else if( __strnicmpx( valueTok, valueLen, "pid" )		== 0 ) flags |= kLogFlags_PrintPID;
			else if( __strnicmpx( valueTok, valueLen, "program" )	== 0 ) flags |= kLogFlags_PrintProgram;
			else if( __strnicmpx( valueTok, valueLen, "category" )	== 0 ) flags |= kLogFlags_PrintCategory;
			else if( __strnicmpx( valueTok, valueLen, "level" )		== 0 ) flags |= kLogFlags_PrintLevel;
			else if( __strnicmpx( valueTok, valueLen, "prefix" )	== 0 ) flags |= kLogFlags_PrintPrefix;
			else if( __strnicmpx( valueTok, valueLen, "function" )	== 0 ) flags |= kLogFlags_PrintFunction;
			else continue;
		}
		
		for( category = gLogCategoryList; category; category = category->next )
		{
			if( inCategory && ( inCategory != category ) ) 				continue;
			if( !__LogCategory_match( action->name, category->name ) )	continue;
			
			if( flags & kLogFlags_PrintPrefix )
			{
				valueEnd = strchr( category->name, '_' );
				if( !valueEnd ) valueEnd = category->name + strlen( category->name );
				category->prefixPtr = category->name;
				category->prefixLen = (int)( valueEnd - category->prefixPtr );
			}
			category->flags = flags;
		}
	}
	
	// Apply rate limiter actions.
	
	for( action = gLogActionList; action; action = action->next )
	{
		uint64_t		intervalTicks;
		uint32_t		maxCount;
		
		if( __strnicmpx( action->variable, SIZE_MAX, "rate" ) != 0 )
			continue;
		
		valueSrc = action->value;
		valueEnd = valueSrc + strlen( valueSrc );
		
		maxCount = 0;
		for( ; ( valueSrc < valueEnd ) && isdigit_safe( *valueSrc ); ++valueSrc )
			maxCount = ( maxCount * 10 ) + ( *valueSrc - '0' );
		if( valueSrc < valueEnd ) ++valueSrc;
		
		intervalTicks = 0;
		for( ; ( valueSrc < valueEnd ) && isdigit_safe( *valueSrc ); ++valueSrc )
			intervalTicks = ( intervalTicks * 10 ) + ( *valueSrc - '0' );
		intervalTicks = ( UpTicksPerSecond() * intervalTicks ) / 1000;
		
		for( category = gLogCategoryList; category; category = category->next )
		{
			if( inCategory && ( inCategory != category ) ) 				continue;
			if( !__LogCategory_match( action->name, category->name ) )	continue;
			
			category->rateInterval = intervalTicks;
			category->rateMaxCount = maxCount;
		}
	}
	
	// Apply output actions.
	
	for( action = gLogActionList; action; action = action->next )
	{
		if(      __strnicmpx( action->variable, SIZE_MAX, "output" )  == 0 ) outputID = 1;
		else if( __strnicmpx( action->variable, SIZE_MAX, "output2" ) == 0 ) outputID = 2;
		else continue;
		
		for( category = gLogCategoryList; category; category = category->next )
		{
			if( inCategory && ( inCategory != category ) ) 				continue;
			if( !__LogCategory_match( action->name, category->name ) )	continue;
			
			err = __LogCategory_ApplyAction_Output( category, outputID, action );
			require_noerr_quiet( err, exit );
		}
	}
	__LogOutputDeleteUnused();
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__LogCategory_ApplyAction_Output
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static OSStatus	__LogCategory_ApplyAction_Output( LogCategory *inCategory, int inOutputID, LogAction *inAction )
{
	OSStatus			err;
	LogOutput *			newOutput;
	LogOutput *			oldOutput;
	LogOutput **		outputAddr;
	
	if( inAction->value[ 0 ] != '\0' )
	{
		err = __LogOutputCreate( inAction->value, &newOutput );
		require_noerr_quiet( err, exit );
	}
	else
	{
		newOutput = NULL;
		err = kNoErr;
	}
	
	if(      inOutputID == 1 ) outputAddr = &inCategory->output1;
	else if( inOutputID == 2 ) outputAddr = &inCategory->output2;
	else { err = kParamErr; goto exit; }
	
	oldOutput = *outputAddr;
	if( oldOutput != newOutput )
	{
		if( oldOutput ) --oldOutput->refCount;
		if( newOutput ) ++newOutput->refCount;
		*outputAddr = newOutput;
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	Regex code from Rob Pike and Brian Kernighan (I just renamed it slightly and made some things const)
//
//	C	Matches any literal character C.
//	.	Matches any single character.
//	^	Matches the beginning of the input string.
//	$	Matches the end of the input string
//	*	Matches zero or more occurrences of the previous character.
//===========================================================================================================================

// match: search for regexp anywhere in text
static int __LogCategory_match(const char *regexp, const char *text)
{
	if (regexp[0] == '^')
		return __LogCategory_matchhere(regexp+1, text);
	do {    // must look even if string is empty
		if (__LogCategory_matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}

// matchhere: search for regexp at beginning of text
static int __LogCategory_matchhere(const char *regexp, const char *text)
{
   if (regexp[0] == '\0')
	   return 1;
   if (regexp[1] == '*')
	   return __LogCategory_matchstar(regexp[0], regexp+2, text);

   if (regexp[0] == '$' && regexp[1] == '\0')
	   return *text == '\0';
   if (*text!='\0' && (regexp[0]=='.' || regexp[0]==*text))
	   return __LogCategory_matchhere(regexp+1, text+1);
   return 0;
}

// matchstar: search for c*regexp at beginning of text
static int __LogCategory_matchstar(int c, const char *regexp, const char *text)
{
   do {   // a * matches zero or more instances
	   if (__LogCategory_matchhere(regexp, text))
		   return 1;
   } while (*text != '\0' && (*text++ == c || c == '.'));
   return 0;
}

//===========================================================================================================================
//	__LogCategory_LevelToString
//===========================================================================================================================

static const struct
{
	LogLevel		level;
	const char *	name;
	
}	kLogLevelToStringTable[] = 
{
	{ kLogLevelAll,			"all" }, 
	{ kLogLevelMin,			"min" }, 
	{ kLogLevelChatty,		"chatty" }, 
	{ kLogLevelVerbose,		"verbose" }, 
	{ kLogLevelTrace,		"trace" }, 
	{ kLogLevelInfo,		"info" }, 
	{ kLogLevelNotice,		"notice" }, 
	{ kLogLevelWarning,		"warning" }, 
	{ kLogLevelAssert,		"assert" }, 
	{ kLogLevelRequire,		"require" }, 
	{ kLogLevelError,		"error" }, 
	{ kLogLevelCritical,	"critical" }, 
	{ kLogLevelAlert,		"alert" }, 
	{ kLogLevelEmergency,	"emergency" }, 
	{ kLogLevelTragic,		"tragic" }, 
	{ kLogLevelMax,			"max" }, 
	{ kLogLevelOff,			"off" }, 
	{ kLogLevelMax + 1, 	NULL }
};

static char *	__LogCategory_LevelToString( LogLevel inLevel, char *inBuf, size_t inLen )
{
	int					i;
	int					diff;
	int					smallestDiff;
	int					closestIndex;
	const char *		name;
	
	inLevel &= kLogLevelMask;
	
	smallestDiff = INT_MAX;
	closestIndex = 0;
	for( i = 0; kLogLevelToStringTable[ i ].name; ++i )
	{
		diff = inLevel - kLogLevelToStringTable[ i ].level;
		if( diff < 0 ) diff = -diff;
		if( diff < smallestDiff )
		{
			smallestDiff = diff;
			closestIndex = i;
		}
	}
	
	name = kLogLevelToStringTable[ closestIndex ].name;
	diff = inLevel - kLogLevelToStringTable[ closestIndex ].level;
	if(      diff > 0 ) SNPrintF( inBuf, inLen, "%s+%u", name, diff );
	else if( diff < 0 )	SNPrintF( inBuf, inLen, "%s-%u", name, -diff );
	else				SNPrintF( inBuf, inLen, "%s",    name );
	return( inBuf );
}

//===========================================================================================================================
//	__LogCategory_StringToLevel
//===========================================================================================================================

// Workaround until <radar:11684218> is fixed in the clang analyzer.

#ifdef __clang_analyzer__
	#undef  isalpha_safe
	#define isalpha_safe( X )	( ( ( (X) >= 'a' ) && ( (X) <= 'z' ) ) || ( ( (X) >= 'A' ) && ( (X) <= 'Z' ) ) )

	#undef  isdigit_safe
	#define isdigit_safe( X )	( ( (X) >= '0' ) && ( (X) <= '9' ) )
#endif

static LogLevel	__LogCategory_StringToLevel( const char *inStr )
{
	LogLevel			level;
	const char *		ptr;
	char				c;
	int					i;
	size_t				len;
	char				adjust;
	int					x;
	
	for( ptr = inStr; isalpha_safe( c = *ptr ); ++ptr ) {}
	if( ptr != inStr )
	{
		len = (size_t)( ptr - inStr );
		level = kLogLevelUninitialized;
		for( i = 0; kLogLevelToStringTable[ i ].name; ++i )
		{
			if( strncmp( inStr, kLogLevelToStringTable[ i ].name, len ) == 0 )
			{
				level = kLogLevelToStringTable[ i ].level;
				break;
			}
		}
		if( level == kLogLevelUninitialized ) goto exit;
		
		if( c == '\0' ) goto exit;
		adjust = c;
		
		x = 0;
		for( ++ptr; isdigit_safe( c = *ptr ); ++ptr ) x = ( x * 10 ) + ( c - '0' );
		if( c != '\0' ) { level = kLogLevelUninitialized; goto exit; }
		
		if(      adjust == '+' ) level += x;
		else if( adjust == '-' ) level -= x;
		else { level = kLogLevelUninitialized; goto exit; }
	}
	else
	{
		level = 0;
		for( ; isdigit_safe( c = *ptr ); ++ptr ) level = ( level * 10 ) + ( c - '0' );
		if( c != '\0' ) { level = kLogLevelUninitialized; goto exit; }
	}
	
exit:
	return( level );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	LogPrintF
//===========================================================================================================================

struct LogPrintFContext
{
	LogCategory *		category;
	LogLevel			level;
	char				buf[ 256 ];
	size_t				len;
	Boolean				flushOnEnd;
};

static int	__LogPrintFCallBack( const char *inStr, size_t inLen, void *inContext );
static void	__LogPrintFWrite( LogPrintFContext *inContext, const char *inStr, size_t inLen );

#if( !DEBUG_HAS_VA_ARG_MACROS )
int	LogPrintF_C89( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, ... )
{
	if( log_category_ptr_enabled( inCategory, inLevel ) )
	{
		int				n;
		va_list			args;
		
		va_start( args, inFormat );
		n = LogPrintFVAList( inCategory, inFunction, inLevel, inFormat, args );
		va_end( args );
		return( n );
	}
	return( 0 );
}
#endif

int	LogPrintF( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, ... )
{
	int				n;
	va_list			args;
	
	va_start( args, inFormat );
	n = LogPrintFVAList( inCategory, inFunction, inLevel, inFormat, args );
	va_end( args );
	return( n );
}

int	LogPrintFVAList( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, va_list inArgs )
{
	LogPrintFContext		context;
	int						total, n, last;
	va_list					args;
	char *					reason = NULL;
	
	if( inLevel & kLogLevelFlagCrashReport )
	{
		// Note: this is done here to work around what looks like a clang static analyzer issue.
		// <rdar://problem/12292635> False positive for uninitialized value?
		
		va_copy( args, inArgs );
		VASPrintF( &reason, inFormat, args );
		va_end( args );
	}
	
	LogUtils_EnsureInitialized();
	LogUtils_Lock();
	
	context.category	= inCategory;
	context.level		= inLevel;
	context.buf[ 0 ]	= '\0';
	context.len			= 0;
	context.flushOnEnd	= false;
	
	// Print the header.
	
	total = 0;
	if( !( inLevel & kLogLevelFlagContinuation ) )
	{
		LogFlags		flags;
		
		flags = inCategory->flags;
		if( inLevel & kLogLevelFlagFunction ) flags |= kLogFlags_PrintFunction;
		
		// Skip if we're logging too frequently.
		
		if( ( inCategory->rateMaxCount > 0 ) && !( inLevel & kLogLevelFlagDontRateLimit ) )
		{
			if( inCategory->rateEnd == 0 )
			{
				inCategory->rateEnd = UpTicks() + inCategory->rateInterval;
			}
			if( UpTicks() >= inCategory->rateEnd )
			{
				inCategory->rateEnd		= 0;
				inCategory->rateCounter = 0;
			}
			if( inCategory->rateCounter >= inCategory->rateMaxCount )
			{
				LogUtils_Unlock();
				goto exit;
			}
			++inCategory->rateCounter;
		}
		
		// Time
		
		if( flags & kLogFlags_PrintTime )
		{
			n = CPrintF( __LogPrintFCallBack, &context, "%N " );
			if( n > 0 ) total += n;
		}
		
		if( flags & ( kLogFlags_PrintProgram | kLogFlags_PrintPID | kLogFlags_PrintCategory | 
			kLogFlags_PrintLevel | kLogFlags_PrintPrefix | kLogFlags_PrintFunction ) )
		{
			n = CPrintF( __LogPrintFCallBack, &context, "[" );
			if( n > 0 ) total += n;
			last = total;
			
			// Program
			
			if( flags & kLogFlags_PrintProgram )
			{
				n = CPrintF( __LogPrintFCallBack, &context, "%s", __PROGRAM__ );
				if( n > 0 ) total += n;
			}
			
			// PID
			
			#if( TARGET_OS_POSIX || TARGET_OS_WINDOWS )
			if( flags & kLogFlags_PrintPID )
			{
				n = CPrintF( __LogPrintFCallBack, &context, "%s%llu", ( last == total ) ? "" : ":", (uint64_t) getpid() );
				if( n > 0 ) total += n;
			}
			#endif
			
			// Category
			
			if( flags & kLogFlags_PrintCategory )
			{
				n = CPrintF( __LogPrintFCallBack, &context, "%s%s", ( last == total ) ? "" : ",", inCategory->name );
				if( n > 0 ) total += n;
			}
			
			// Prefix
			
			else if( flags & kLogFlags_PrintPrefix )
			{
				n = CPrintF( __LogPrintFCallBack, &context, "%s%.*s", ( last == total ) ? "" : ",", 
					inCategory->prefixLen, inCategory->prefixPtr );
				if( n > 0 ) total += n;
			}
			
			// Function
			
			if( flags & kLogFlags_PrintFunction )
			{
				n = CPrintF( __LogPrintFCallBack, &context, "%s%s", ( last == total ) ? "" : ",", inFunction );
				if( n > 0 ) total += n;
			}
			
			// Level
			
			if( flags & kLogFlags_PrintLevel )
			{
				char		levelStr[ 64 ];
				
				n = CPrintF( __LogPrintFCallBack, &context, "%s%s", ( last == total ) ? "" : "@", 
					__LogCategory_LevelToString( inLevel, levelStr, sizeof( levelStr ) ) );
				if( n > 0 ) total += n;
			}
			
			n = CPrintF( __LogPrintFCallBack, &context, "] " );
			if( n > 0 ) total += n;
		}
	}
	
	// Print the body.
	
	n = VCPrintF( __LogPrintFCallBack, &context, inFormat, inArgs );
	if( n > 0 ) total += n;
	
	context.flushOnEnd = true;
	n = __LogPrintFCallBack( "", 0, &context );
	if( n > 0 ) total += n;
	
	LogUtils_Unlock();
	
	// Print out a stack trace if requested.
	
	if( inLevel & kLogLevelFlagStackTrace )
	{
		DebugStackTrace( kLogLevelMax );
	}
	
	// Break into the debugger if requested.
	
	if( ( inLevel & kLogLevelFlagDebugBreak ) && DebugIsDebuggerPresent() )
	{
		DebugEnterDebugger();
	}
	
	// Force a crash report if requested.
	
	if( inLevel & kLogLevelFlagCrashReport )
	{
		if( reason )
		{
			char *		end;
			
			for( end = reason + strlen( reason ); ( end > reason ) && ( end[ -1 ] == '\n' ); --end ) {}
			*end = '\0';
			ReportCriticalError( reason, 0, true );
		}
	}
	
exit:
	if( reason ) free( reason );
	return( total );
}

static int	__LogPrintFCallBack( const char *inStr, size_t inLen, void *inContext )
{
	LogPrintFContext * const		context = (LogPrintFContext *) inContext;
	
	// Flush buffered data if we got an explicit flush (inLen == 0) or we'd go over our max size.
	
	if( ( ( inLen == 0 ) && context->flushOnEnd ) || ( ( context->len + inLen ) > sizeof( context->buf ) ) )
	{
		if( context->len > 0 )
		{
			__LogPrintFWrite( context, context->buf, context->len );
			context->len = 0;
		}
	}
	
	// Flush immediately if the new data is too big for our buffer. Otherwise, buffer it.
	
	if( inLen > sizeof( context->buf ) )
	{
		__LogPrintFWrite( context, inStr, inLen );
	}
	else if( inLen > 0 )
	{
		memcpy( &context->buf[ context->len ], inStr, inLen );
		context->len += inLen;
	}
	return( (int) inLen );
}

static void	__LogPrintFWrite( LogPrintFContext *inContext, const char *inStr, size_t inLen )
{
	inContext->category->output1->writer( inContext, inContext->category->output1, inStr, inLen );
	if( inContext->category->output2 )
	{
		inContext->category->output2->writer( inContext, inContext->category->output2, inStr, inLen );
	}
	
#if( DEBUG_FPRINTF_ENABLED )
	if( inContext->level & kLogLevelFlagForceConsole )
	{
		#if( TARGET_OS_POSIX )
			int			fd;
			ssize_t		n;
			
			fd = open( "/dev/console", O_WRONLY, 0 );
			if( fd >= 0 )
			{
				n = write( fd, inStr, inLen );
				(void) n;
				close( fd );
				usleep( 200 );
			}
		#else		
			FILE *		f;
			
			f = fopen( "/dev/console", "w" );
			if( f )
			{
				fwrite( inStr, 1, inLen, f );
				fflush( f );
				fclose( f );
			}
		#endif
	}
#endif
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	__LogOutputCreate
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static OSStatus	__LogOutputCreate( const char *inConfigStr, LogOutput **outOutput )
{
	OSStatus			err;
	LogOutput *			output;
	const char *		typeSrc;
	size_t				typeLen;
	char				c;
	
	// If there's an existing output with the same config string, use that one instead.
	
	for( output = gLogOutputList; output; output = output->next )
	{
		if( __strnicmpx( output->configStr, SIZE_MAX, inConfigStr ) == 0 )
		{
			*outOutput = output;
			output = NULL;
			err = kNoErr;
			goto exit;
		}
	}
	
	output = (LogOutput *) calloc( 1, sizeof( *output ) );
	require_action_quiet( output, exit, err = kNoMemoryErr );
	
	output->refCount = 0; // Only referenced when associated with a category.
	output->configStr = strdup( inConfigStr );
	require_action_quiet( output->configStr, exit, err = kNoMemoryErr );
	
	// Parse the output type and set it up based on the type.
	
	typeSrc = inConfigStr;
	while( ( ( c = *inConfigStr ) != '\0' ) && ( c != ';' ) ) ++inConfigStr;
	typeLen = (size_t)( inConfigStr - typeSrc );
	require_action_quiet( typeLen > 0, exit, err = kTypeErr );
	if( c != '\0' ) ++inConfigStr;

	if( 0 ) { }
	else if( __strnicmpx( typeSrc, typeLen, "console" ) == 0 )
	{
		#if( TARGET_OS_POSIX )
			if( __LogOutput_IsStdErrMappedToDevNull() )	err = __LogOutputSysLog_Setup( output, inConfigStr );
			else										err = __LogOutputFile_Setup( output, inConfigStr );
			require_noerr_quiet( err, exit );
		#elif( DEBUG_FPRINTF_ENABLED )
			err = __LogOutputFile_Setup( output, inConfigStr );
			require_noerr_quiet( err, exit );
		#elif( TARGET_OS_WINDOWS_CE )
			output->writer	= __LogOutputWindowsDebugger_Writer;
			output->type	= kLogOutputType_WindowsDebugger;
		#elif( TARGET_OS_WINDOWS_KERNEL )
			output->writer	= __LogOutputWindowsKernel_Writer;
			output->type	= kLogOutputType_WindowsKernel;
		#endif
	}
#if( DEBUG_FPRINTF_ENABLED )
	else if( __strnicmpx( typeSrc, typeLen, "file" ) == 0 )
	{
		err = __LogOutputFile_Setup( output, inConfigStr );
		require_noerr_quiet( err, exit );
	}
#endif
#if( TARGET_OS_POSIX )
	else if( __strnicmpx( typeSrc, typeLen, "syslog" ) == 0 )
	{
		err = __LogOutputSysLog_Setup( output, inConfigStr );
		require_noerr_quiet( err, exit );
	}
#endif
#if( TARGET_OS_WINDOWS )
	else if( __strnicmpx( typeSrc, typeLen, "WindowsDebugger" ) == 0 )
	{
		output->writer	= __LogOutputWindowsDebugger_Writer;
		output->type	= kLogOutputType_WindowsDebugger;
	}
#endif
#if( DEBUG_WINDOWS_EVENT_LOG_ENABLED )
	else if( __strnicmpx( typeSrc, typeLen, "WindowsEventLog" ) == 0 )
	{
		err = __LogOutputWindowsEventLog_Setup( output, inConfigStr );
		require_noerr_quiet( err, exit );
	}
#endif
#if( TARGET_OS_WINDOWS_KERNEL )
	else if( __strnicmpx( typeSrc, typeLen, "WindowsKernel" ) == 0 )
	{
		output->writer	= __LogOutputWindowsKernel_Writer;
		output->type	= kLogOutputType_WindowsKernel;
	}
#endif
	else if( __strnicmpx( typeSrc, typeLen, "callback" ) == 0 )
	{
		err = __LogOutputCallBack_Setup( output, inConfigStr );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = kParamErr;
		goto exit;
	}
	
	output->next	= gLogOutputList;
	gLogOutputList	= output;
	*outOutput		= output;
	output			= NULL;
	err				= kNoErr;
	
exit:
	if( output ) __LogOutputDelete( output );
	return( err );
}

//===========================================================================================================================
//	__LogOutputDelete
//===========================================================================================================================

static void	__LogOutputDelete( LogOutput *inOutput )
{
#if( DEBUG_FPRINTF_ENABLED )
	if( inOutput->type == kLogOutputType_File )
	{
		ForgetMem( &inOutput->config.file.logFileName );
		if( inOutput->config.file.logFilePtr )
		{
			if( ( inOutput->config.file.logFilePtr != stderr ) && 
				( inOutput->config.file.logFilePtr != stdout ) )
			{
				fclose( inOutput->config.file.logFilePtr );
			}
			inOutput->config.file.logFilePtr = NULL;
		}
		ForgetMem( &inOutput->config.file.logBackupFileName );
	}
#endif
	ForgetMem( &inOutput->configStr );
	free( inOutput );
}

//===========================================================================================================================
//	__LogOutputDeleteUnused
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static void	__LogOutputDeleteUnused( void )
{
	LogOutput **	next;
	LogOutput *		curr;
	
	next = &gLogOutputList;
	while( ( curr = *next ) != NULL )
	{
		if( curr->refCount == 0 )
		{
			*next = curr->next;
			__LogOutputDelete( curr );
			continue;
		}
		next = &curr->next;
	}
}

#if( DEBUG_FPRINTF_ENABLED )
//===========================================================================================================================
//	__LogOutputFile_Setup
//===========================================================================================================================

static OSStatus	__LogOutputFile_Setup( LogOutput *inOutput, const char *inParams )
{
	OSStatus			err;
	const char *		namePtr;
	size_t				nameLen;
	const char *		valuePtr;
	const char *		valueEnd;
	size_t				valueLen;
	const char *		ptr;
	char				c;
	char *				str;
	size_t				len;
	int64_t				x;
	
	inOutput->writer = __LogOutputFile_Writer;
	inOutput->type   = kLogOutputType_File;
	
	if( ( *inParams == '\0' ) || ( __strnicmpx( inParams, SIZE_MAX, "stderr" ) == 0 ) )
	{
		#if( DEBUG && TARGET_OS_WINDOWS )
			DebugWinEnableConsole();
		#endif
		
		inOutput->config.file.logFilePtr = stderr;
	}
	else if( __strnicmpx( inParams, SIZE_MAX, "stdout" ) == 0 )
	{
		#if( DEBUG && TARGET_OS_WINDOWS )
			DebugWinEnableConsole();
		#endif
		
		inOutput->config.file.logFilePtr = stdout;
	}
	else
	{
		while( *inParams != '\0' )
		{
			namePtr = inParams;
			while( ( ( c = *inParams ) != '\0' ) && ( c != '=' ) ) ++inParams;
			require_action_quiet( c != '\0', exit, err = kMalformedErr );
			nameLen = (size_t)( inParams - namePtr );
			++inParams;
			
			valuePtr = inParams;
			while( ( ( c = *inParams ) != '\0' ) && ( c != ';' ) ) ++inParams;
			valueEnd = inParams;
			valueLen = (size_t)( inParams - valuePtr );
			if( c != '\0' ) ++inParams;
			
			if( __strnicmpx( namePtr, nameLen, "path" ) == 0 )
			{
				// Format: path=<path to log file>.
				
				require_action_quiet( valueLen > 0, exit, err = kPathErr );
				str = (char *) malloc( valueLen + 1 );
				require_action_quiet( str, exit, err = kNoMemoryErr );
				memcpy( str, valuePtr, valueLen );
				str[ valueLen ] = '\0';
				ForgetMem( &inOutput->config.file.logFileName );
				inOutput->config.file.logFileName = str;
				
				#if( TARGET_OS_POSIX )
				{
					const char *		dirEnd;
					size_t				dirLen;
					char				dirPath[ PATH_MAX + 1 ];
					
					dirEnd = strrchr( inOutput->config.file.logFileName, '/' );
					if( dirEnd )
					{
						dirLen = (size_t)( dirEnd - inOutput->config.file.logFileName );
						require_action_quiet( dirLen < sizeof( dirPath ), exit, err = kPathErr );
						memcpy( dirPath, inOutput->config.file.logFileName, dirLen );
						dirPath[ dirLen ] = '\0';
						
						mkpath( dirPath, S_IRWXU | S_IRWXG, S_IRWXU | S_IRWXG );
					}
				}
				#endif
				inOutput->config.file.logFilePtr = fopen( inOutput->config.file.logFileName, "a" );
				require_action_quiet( inOutput->config.file.logFilePtr, exit, err = kOpenErr );
				
				fseeko( inOutput->config.file.logFilePtr, 0, SEEK_END );
				inOutput->config.file.logFileSize = ftello( inOutput->config.file.logFilePtr );
			}
			else if( __strnicmpx( namePtr, nameLen, "roll" ) == 0 )
			{
				// Format: roll=<maxSize>#<maxCount>.
				
				x = 0;
				for( ; ( valuePtr < valueEnd ) && isdigit_safe( *valuePtr ); ++valuePtr )
					x = ( x * 10 ) + ( *valuePtr - '0' );
				if( valuePtr < valueEnd )
				{
					if(      *valuePtr == 'B' ) ++valuePtr;
					else if( *valuePtr == 'K' ) { x *=          1024;   ++valuePtr; }
					else if( *valuePtr == 'M' ) { x *= ( 1024 * 1024 ); ++valuePtr; }
				}
				require_action_quiet( ( valuePtr == valueEnd ) || ( *valuePtr == '#' ), exit, err = kParamErr );
				inOutput->config.file.logFileMaxSize = x;
				if( valuePtr < valueEnd ) ++valuePtr;
				
				x = 0;
				for( ; ( valuePtr < valueEnd ) && isdigit_safe( *valuePtr ); ++valuePtr )
					x = ( x * 10 ) + ( *valuePtr - '0' );
				require_action_quiet( valuePtr == valueEnd, exit, err = kParamErr );
				inOutput->config.file.logFileMaxCount = (int) x;
			}
			else if( __strnicmpx( namePtr, nameLen, "backup" ) == 0 )
			{
				// Format: backup=<base path to backup files>#<maxCount>.
				
				ptr = valuePtr;
				for( ; ( valuePtr < valueEnd ) && ( *valuePtr != '#' ); ++valuePtr ) {}
				require_action_quiet( ( valuePtr == valueEnd ) || ( *valuePtr == '#' ), exit, err = kParamErr );
				
				str = NULL;
				len = (size_t)( valuePtr - ptr );
				if( len > 0 )
				{
					str = (char *) malloc( len + 1 );
					require_action_quiet( str, exit, err = kNoMemoryErr );
					memcpy( str, ptr, len );
					str[ len ] = '\0';
				}
				ForgetMem( &inOutput->config.file.logBackupFileName );
				inOutput->config.file.logBackupFileName = str;
				if( valuePtr < valueEnd ) ++valuePtr;
				
				x = 0;
				for( ; ( valuePtr < valueEnd ) && isdigit_safe( *valuePtr ); ++valuePtr )
					x = ( x * 10 ) + ( *valuePtr - '0' );
				require_action_quiet( valuePtr == valueEnd, exit, err = kParamErr );
				inOutput->config.file.logBackupFileMaxCount = (int) x;
			}
			else
			{
				err = kUnsupportedErr;
				goto exit;
			}
		}
		require_action_quiet( inOutput->config.file.logFilePtr, exit, err = kParamErr );
	}
	
	// Force output to use non-buffered I/O.
	
	setvbuf( inOutput->config.file.logFilePtr, NULL, _IONBF, 0 );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__LogOutputFile_Writer
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static void	__LogOutputFile_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	(void) inContext;
	
	// Roll the log files if the current file goes above the max size.
	
	if( ( inOutput->config.file.logFilePtr != stderr ) && ( inOutput->config.file.logFilePtr != stdout ) )
	{
		inOutput->config.file.logFileSize += inLen;
		if( inOutput->config.file.logFileMaxSize > 0 )
		{
			if( inOutput->config.file.logFileSize > inOutput->config.file.logFileMaxSize )
			{
				__LogOutputFile_BackupLogFiles( inOutput );
				RollLogFiles( &inOutput->config.file.logFilePtr, 
					"\nLOG ENDED, CONTINUES IN NEXT LOG FILE\n", 
					inOutput->config.file.logFileName, inOutput->config.file.logFileMaxCount );
				inOutput->config.file.logFileSize = inLen;
			}
		}
	}
	if( inOutput->config.file.logFilePtr )
	{
		fwrite( inStr, 1, inLen, inOutput->config.file.logFilePtr );
		fflush( inOutput->config.file.logFilePtr );
	}
}

//===========================================================================================================================
//	__LogOutputFile_BackupLogFiles
//
//	Note: assumes the lock is held.
//===========================================================================================================================

static OSStatus	__LogOutputFile_BackupLogFiles( LogOutput *inOutput )
{
	OSStatus		err;
	char			oldPath[ PATH_MAX + 1 ];
	char 			newPath[ PATH_MAX + 1 ];
	int				i;
	
	require_action_quiet( inOutput->config.file.logBackupFileName, exit, err = kNoErr );
	require_action_quiet( inOutput->config.file.logBackupFileMaxCount > 0, exit, err = kNoErr );
	
	// Delete the oldest file.
	
	SNPrintF( oldPath, sizeof( oldPath ), "%s.%d", inOutput->config.file.logBackupFileName, 
		inOutput->config.file.logBackupFileMaxCount - 1 );
	remove( oldPath );
	
	// Shift all the files down by 1.
	
	for( i = inOutput->config.file.logBackupFileMaxCount - 2; i > 0; --i )
	{
		SNPrintF( oldPath, sizeof( oldPath ), "%s.%d", inOutput->config.file.logBackupFileName, i );
		SNPrintF( newPath, sizeof( newPath ), "%s.%d", inOutput->config.file.logBackupFileName, i + 1 );
		rename( oldPath, newPath );
	}
	SNPrintF( newPath, sizeof( newPath ), "%s.1", inOutput->config.file.logBackupFileName );
	rename( inOutput->config.file.logBackupFileName, newPath );
	
	// Copy the latest file.
	
	SNPrintF( newPath, sizeof( newPath ), "%s", inOutput->config.file.logBackupFileName );
	__LogOutputFile_CopyLogFile( inOutput->config.file.logFileName, newPath );
	err = kNoErr;

exit:
	return( err );
}

//===========================================================================================================================
//	__LogOutputFile_CopyLogFile
//===========================================================================================================================

static OSStatus	__LogOutputFile_CopyLogFile( const char *inSrcPath, const char *inDstPath )
{
	OSStatus		err;
	char *			buffer;
	size_t			bufLen;
	FILE *			srcFile;
	FILE *			dstFile;
	size_t			nRead;
	size_t			nWrote;
	
	srcFile = NULL;
	dstFile = NULL;
	
	bufLen = 4 * 1024;
	buffer = (char *) malloc( bufLen );
	require_action_quiet( buffer, exit, err = kNoMemoryErr );
	
	srcFile = fopen( inSrcPath, "r" );
	err = map_global_value_errno( srcFile, srcFile );
	require_noerr_quiet( err, exit );
	
	dstFile = fopen( inDstPath, "w" );
	err = map_global_value_errno( dstFile, dstFile );
	require_noerr_quiet( err, exit );
	
	for( ;; )
	{
		nRead = fread( buffer, 1, bufLen, srcFile );
		if( nRead == 0 ) break;
		
		nWrote = fwrite( buffer, 1, nRead, dstFile );
		err = map_global_value_errno( nWrote == nRead, nWrote );
		require_noerr_quiet( err, exit );
	}
	
exit:
	if( srcFile )	fclose( srcFile );
	if( dstFile )	fclose( dstFile );
	if( buffer )	free( buffer );
	return( err );
}

#endif // DEBUG_FPRINTF_ENABLED

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	__LogOutputSysLog_Setup
//===========================================================================================================================

static OSStatus	__LogOutputSysLog_Setup( LogOutput *inOutput, const char *inParams )
{
	OSStatus			err;
	const char *		namePtr;
	size_t				nameLen;
	const char *		valuePtr;
	size_t				valueLen;
	char				c;
	char				buf[ 32 ];
	
	inOutput->config.syslog.fixedLevel = kLogLevelUninitialized;
	
	while( *inParams != '\0' )
	{
		namePtr = inParams;
		while( ( ( c = *inParams ) != '\0' ) && ( c != '=' ) ) ++inParams;
		require_action_quiet( c != '\0', exit, err = kMalformedErr );
		nameLen = (size_t)( inParams - namePtr );
		++inParams;
		
		valuePtr = inParams;
		while( ( ( c = *inParams ) != '\0' ) && ( c != ';' ) ) ++inParams;
		valueLen = (size_t)( inParams - valuePtr );
		if( c != '\0' ) ++inParams;
		
		if( __strnicmpx( namePtr, nameLen, "level" ) == 0 )
		{
			// Format: level=<fixed level to use>.
			
			valueLen = Min( valueLen, sizeof( buf ) - 1 );
			memcpy( buf, valuePtr, valueLen );
			buf[ valueLen ] = '\0';
			
			inOutput->config.syslog.fixedLevel = __LogCategory_StringToLevel( buf );
		}
		else
		{
			err = kUnsupportedErr;
			goto exit;
		}
	}
	
	inOutput->writer = __LogOutputSysLog_Writer;
	inOutput->type   = kLogOutputType_syslog;
	err = kNoErr;
	
exit:
	return( err );
}
#endif // TARGET_OS_POSIX

//===========================================================================================================================
//	__LogOutputSysLog_Writer
//===========================================================================================================================

#if( TARGET_OS_POSIX )
static void	__LogOutputSysLog_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	LogLevel		level;
	int				priority;
	
	level = inOutput->config.syslog.fixedLevel;
	priority = ( level == kLogLevelUninitialized ) ? LOG_NOTICE : ( inContext->level & kLogLevelMask );
	if( ( inLen > 0 ) && ( inStr[ inLen - 1 ] == '\n' ) ) --inLen; // Strip trailing newlines.
	syslog( priority, "%.*s", (int) inLen, inStr );
}
#endif

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	__LogOutputWindowsDebugger_Writer
//===========================================================================================================================

static void	__LogOutputWindowsDebugger_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	TCHAR				buf[ 512 ];
	const char *		src;
	const char *		end;
	TCHAR *				dst;
	char				c;
	
	(void) inContext;
	(void) inOutput;
	
	// Copy locally and null terminate the string. This also converts from char to TCHAR in case we are 
	// building with UNICODE enabled since the input is always char. Also convert \r to \n in the process.

	src = inStr;
	if( inLen >= countof( buf ) )
	{
		inLen = countof( buf ) - 1;
	}
	end = src + inLen;
	dst = buf;
	while( src < end )
	{
		c = *src++;
		if( c == '\r' ) c = '\n';
		*dst++ = (TCHAR) c;
	}
	*dst = 0;
	
	// Print out the string to the debugger.
	
	OutputDebugString( buf );
}
#endif

#if( DEBUG_WINDOWS_EVENT_LOG_ENABLED )
//===========================================================================================================================
//	__LogOutputWindowsEventLog_Setup
//===========================================================================================================================

static OSStatus	__LogOutputWindowsEventLog_Setup( LogOutput *inOutput, const char *inParams )
{
	OSStatus			err;
	HKEY				key;
	const char *		programName;
	TCHAR				name[ 128 ];
	const char *		src;
	TCHAR				path[ MAX_PATH ];
	size_t				size;
	DWORD				typesSupported;
	DWORD 				n;
	BOOL				good;
	
	(void) inParams; // Unused

	key = NULL;
	
	// Build the path string using the fixed registry path and app name.
	
	src = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
	programName = getprogname();
	DebugWinCharToTCharString( programName, kSizeCString, name, sizeof( name ), NULL );
	DebugWinCharToTCharString( src, kSizeCString, path, countof( path ), &size );
	DebugWinCharToTCharString( programName, kSizeCString, path + size, countof( path ) - size, NULL );
	
	// Add/Open the source name as a sub-key under the Application key in the EventLog registry key.
	
	err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL );
	require_noerr_quiet( err, exit );
	
	// Set the path in the EventMessageFile subkey. Add 1 to the TCHAR count to include the null terminator.
	
	n = GetModuleFileName( NULL, path, countof( path ) );
	err = map_global_value_errno( n > 0, n );
	require_noerr_quiet( err, exit );
	n += 1;
	n *= sizeof( TCHAR );
	
	err = RegSetValueEx( key, TEXT( "EventMessageFile" ), 0, REG_EXPAND_SZ, (const LPBYTE) path, n );
	require_noerr_quiet( err, exit );
	
	// Set the supported event types in the TypesSupported subkey.
	
	typesSupported = EVENTLOG_SUCCESS | EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE |
					 EVENTLOG_AUDIT_SUCCESS | EVENTLOG_AUDIT_FAILURE;
	err = RegSetValueEx( key, TEXT( "TypesSupported" ), 0, REG_DWORD, (const LPBYTE) &typesSupported, sizeof( DWORD ) );
	require_noerr_quiet( err, exit );
	
	// Set up the event source.
	
	if( inOutput->config.windowsEventLog.source )
	{
		good = DeregisterEventSource( inOutput->config.windowsEventLog.source );
		err = map_global_value_errno( good, good );
		check_noerr( err );
	}
	inOutput->config.windowsEventLog.source = RegisterEventSource( NULL, name );
	err = map_global_value_errno( inOutput->config.windowsEventLog.source, inOutput->config.windowsEventLog.source );
	require_noerr_quiet( err, exit );
	
	inOutput->writer	= __LogOutputWindowsEventLog_Writer;
	inOutput->type		= kLogOutputType_WindowsEventLog;
	
exit:
	if( key ) RegCloseKey( key );
	return( err );
}

//===========================================================================================================================
//	__LogOutputWindowsEventLog_Writer
//===========================================================================================================================

static void	__LogOutputWindowsEventLog_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	WORD				type;
	TCHAR				buf[ 512 ];
	const char *		src;
	const char *		end;
	TCHAR *				dst;
	char				c;
	const TCHAR *		array[ 1 ];
	
	// Map the debug level to a Windows EventLog type.
	
	if(      inContext->level <= kLogLevelNotice )	type = EVENTLOG_INFORMATION_TYPE;
	else if( inContext->level <= kLogLevelWarning )	type = EVENTLOG_WARNING_TYPE;
	else											type = EVENTLOG_ERROR_TYPE;
	
	// Copy locally and null terminate the string. This also converts from char to TCHAR in case we are 
	// building with UNICODE enabled since the input is always char. Also convert \r to \n in the process.
	
	src = inStr;
	if( inLen >= countof( buf ) ) inLen = countof( buf ) - 1;
	end = src + inLen;
	dst = buf;
	while( src < end )
	{
		c = *src++;
		if( c == '\r' ) c = '\n';
		*dst++ = (TCHAR) c;
	}
	*dst = 0;
	
	// Add the the string to the event log.
	
	if( inOutput->config.windowsEventLog.source )
	{
		array[ 0 ] = buf;
		ReportEvent( inOutput->config.windowsEventLog.source, type, 0, 0x20000001L, NULL, 1, 0, array, NULL );
	}
}
#endif // DEBUG_WINDOWS_EVENT_LOG_ENABLED

#if( TARGET_OS_WINDOWS_KERNEL )
//===========================================================================================================================
//	__LogOutputWindowsKernel_Writer
//===========================================================================================================================

static void	__LogOutputWindowsKernel_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	(void) inContext;
	(void) inOutput;
	
	DbgPrint( "%.*s", (int) inLen, inStr );
}
#endif

//===========================================================================================================================
//	__LogOutputCallBack_Setup
//===========================================================================================================================

static OSStatus	__LogOutputCallBack_Setup( LogOutput *inOutput, const char *inParams )
{
	OSStatus			err;
	const char *		namePtr;
	size_t				nameLen;
	const char *		valuePtr;
	size_t				valueLen;
	char				c;
	char				tempStr[ 64 ];
	int					n;
	void *				tempPtr;
	
	inOutput->config.callback.func = NULL;
	inOutput->config.callback.arg  = NULL;
	while( *inParams != '\0' )
	{
		namePtr = inParams;
		while( ( ( c = *inParams ) != '\0' ) && ( c != '=' ) ) ++inParams;
		require_action_quiet( c != '\0', exit, err = kMalformedErr );
		nameLen = (size_t)( inParams - namePtr );
		++inParams;
		
		valuePtr = inParams;
		while( ( ( c = *inParams ) != '\0' ) && ( c != ';' ) ) ++inParams;
		valueLen = (size_t)( inParams - valuePtr );
		if( c != '\0' ) ++inParams;
		
		if( __strnicmpx( namePtr, nameLen, "func" ) == 0 )
		{
			// Format: func=<function pointer>.
			
			require_action_quiet( valueLen < sizeof( tempStr ), exit, err = kSizeErr );
			memcpy( tempStr, valuePtr, valueLen );
			tempStr[ valueLen ] = '\0';
			
			n = sscanf( tempStr, "%p", &tempPtr );
			require_action_quiet( n == 1, exit, err = kMalformedErr );
			inOutput->config.callback.func = (LogOutputCallBack)(uintptr_t) tempPtr;
		}
		else if( __strnicmpx( namePtr, nameLen, "arg" ) == 0 )
		{
			// Format: arg=<pointer arg>.
			
			require_action_quiet( valueLen < sizeof( tempStr ), exit, err = kSizeErr );
			memcpy( tempStr, valuePtr, valueLen );
			tempStr[ valueLen ] = '\0';
			
			n = sscanf( tempStr, "%p", &inOutput->config.callback.arg );
			require_action_quiet( n == 1, exit, err = kMalformedErr );
		}
		else
		{
			err = kUnsupportedErr;
			goto exit;
		}
	}
	
	inOutput->writer = __LogOutputCallBack_Writer;
	inOutput->type   = kLogOutputType_CallBack;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	__LogOutputCallBack_Writer
//===========================================================================================================================

static void	__LogOutputCallBack_Writer( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen )
{
	if( inOutput->config.callback.func ) inOutput->config.callback.func( inContext, inStr, inLen, inOutput->config.callback.arg );
}

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	__LogOutput_IsStdErrMappedToDevNull
//===========================================================================================================================

static Boolean	__LogOutput_IsStdErrMappedToDevNull( void )
{
	Boolean			mapped;
	int				err;
	int				fd;
	struct stat		sb, sb2;
	
	mapped = false;
	
	fd = fileno( stderr );
	require_quiet( fd >= 0, exit );
	
	err = fstat( fd, &sb );
	require_noerr_quiet( err, exit );
	
	fd = open( "/dev/null", O_RDONLY );
	require_quiet( fd >= 0, exit );
	
	err = fstat( fd, &sb2 );
	close( fd );
	require_noerr_quiet( err, exit );
	
	if( ( sb.st_dev == sb2.st_dev ) && ( sb.st_ino == sb2.st_ino ) )
	{
		mapped = true;
	}
	
exit:
	return( mapped );
}
#endif

#endif // LOGUTILS_ENABLED
