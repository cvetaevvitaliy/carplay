/*
	File:    	APSDebugServices.h
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

#ifndef	__APSDebugServices_h__
#define	__APSDebugServices_h__

#include "APSCommonServices.h"

#include <stdarg.h>

#if( TARGET_HAS_STD_C_LIB )
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#if( CFLITE_ENABLED || ( 0 && !0 ) || TARGET_OS_WINDOWS )
	#include CF_HEADER
#endif

#if 0
#pragma mark == Debugging ==
#endif

#if 0
#pragma mark ==    Settings ==
#endif

// General

#if( defined( NDEBUG ) && DEBUG )
	#error NDEBUG defined and DEBUG is also enabled...they need to be in-sync
#endif

// Routine name. Uses ISO __func__ where possible. Otherwise, uses the best thing that is available (if anything).

#if( __ARMCC_VERSION || __GNUC__ )
	#define	__ROUTINE__		__PRETTY_FUNCTION__
#elif( ( _MSC_VER && !defined( _WIN32_WCE ) ) || defined( __ghs__ ) )
	#define	__ROUTINE__		__FUNCTION__
#elif( defined( __MWERKS__ ) )
	#define	__ROUTINE__		__func__
#else
	#define	__ROUTINE__		""
#endif

// Program name.

#if  ( TARGET_OS_BSD || 0 || TARGET_OS_WINDOWS )
	#define	__PROGRAM__		getprogname()
#else
	#define	__PROGRAM__		""
#endif

#if( defined( __MWERKS__ ) || defined( __GNUC__ ) || defined( __ghs__ ) )
	#define	__ROUTINE_FULL__		__PRETTY_FUNCTION__
#elif( _MSC_VER && !defined( _WIN32_WCE ) )
	#define	__ROUTINE_FULL__		__FUNCTION__
#else
	#define	__ROUTINE_FULL__		""
#endif

// Variable argument macro support. Use ANSI C99 __VA_ARGS__ where possible. Otherwise, use the next best thing.

#if( defined( __GNUC__ ) )
	#if( ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ >= 2 ) ) )
		#define	DEBUG_C99_VA_ARGS		1
		#define	DEBUG_GNU_VA_ARGS		0
	#else
		#define	DEBUG_C99_VA_ARGS		0
		#define	DEBUG_GNU_VA_ARGS		1
	#endif
	#define	DEBUG_HAS_VA_ARG_MACROS		1
#elif( TARGET_VISUAL_STUDIO_2005_OR_LATER )
	#define	DEBUG_C99_VA_ARGS			1
	#define	DEBUG_GNU_VA_ARGS			0
	#define	DEBUG_HAS_VA_ARG_MACROS		1
#elif( defined( __MWERKS__ ) )
	#define	DEBUG_C99_VA_ARGS			1
	#define	DEBUG_GNU_VA_ARGS			0
	#define	DEBUG_HAS_VA_ARG_MACROS		1
#else
	#define	DEBUG_C99_VA_ARGS			0
	#define	DEBUG_GNU_VA_ARGS			0
	#define	DEBUG_HAS_VA_ARG_MACROS		0
#endif

// DEBUG_EXPORT_ERROR_STRINGS controls whether DebugGetErrorString is exported for non-debug builds.

#if( !defined( DEBUG_EXPORT_ERROR_STRINGS ) )
	#define	DEBUG_EXPORT_ERROR_STRINGS		0
#endif

#if 0
#pragma mark ==    Levels ==
#endif

// LogLevel

typedef int32_t		LogLevel;

#define	kLogLevelUninitialized		-1			//! PRIVATE: only used internally for tracking LogCategory state.
#define kLogLevelMask				0x000000FF
#define kLogLevelAll				0			//! Intended for setting as a level to enable all logging. Not for passing to ulog.
#define kLogLevelMin				1			//! Intended for passing to ulog as the lowest level. Not for setting as a level.
#define kLogLevelChatty				10
#define kLogLevelVerbose			20			//! Similar to LOG_DEBUG
#define kLogLevelTrace 				30
#define kLogLevelInfo 				40			//! Similar to LOG_INFO
#define kLogLevelNotice				50			//! Similar to LOG_NOTICE
#define kLogLevelWarning			60			//! Similar to LOG_WARNING
#define kLogLevelAssert 			70
#define kLogLevelRequire			80
#define kLogLevelError				90			//! Similar to LOG_ERR
#define kLogLevelCritical			100			//! Similar to LOG_CRIT
#define kLogLevelAlert				110			//! Similar to LOG_ALERT
#define kLogLevelEmergency			120			//! Similar to LOG_EMERG
#define kLogLevelTragic				130
#define kLogLevelMax				0x000000FE	//! Intended for passing to ulog. Not for setting as a level.
#define kLogLevelOff				0x000000FF	//! Only for use when setting a log level. Not for passing to ulog.

#define kLogLevelFlagMask			0xFFFF0000
#define kLogLevelFlagStackTrace		0x00010000	//! Print a stack trace.
#define kLogLevelFlagDebugBreak		0x00020000	//! Break into a debugger (if a debugger is present, ignored otherwise).
#define kLogLevelFlagForceConsole	0x00040000	//! Open /dev/console, print, and close each time to force output.
#define kLogLevelFlagContinuation	0x00080000	//! Use with ulog. Indicates it's part of a previous ulog call.
#define kLogLevelFlagFunction		0x00100000	//! Use with ulog. Prints the name of the function calling ulog.
#define kLogLevelFlagCrashReport	0x00200000	//! Use with ulog. Forces a crash report with stackshot, etc.
#define kLogLevelFlagDontRateLimit	0x00400000	//! Use with ulog. Bypasses normal rate limiting.

#if( ( 0 && !0 ) || TARGET_OS_WINDOWS )
	#define	kDebugDefaultBreakLevel			kLogLevelAssert
#else
	#define	kDebugDefaultBreakLevel			kLogLevelOff // Don't enter the debug by default on embedded platforms.
#endif

#define	kDebugDefaultStackTraceLevel		kLogLevelAssert

#if 0
#pragma mark ==    Output ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_FPRINTF_ENABLED
	@abstract	Enables ANSI C fprintf output.
*/

#if( !defined( DEBUG_FPRINTF_ENABLED ) )
	#if( TARGET_HAS_C_LIB_IO )
		#define	DEBUG_FPRINTF_ENABLED			1
	#else
		#define	DEBUG_FPRINTF_ENABLED			0
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_WINDOWS_EVENT_LOG_ENABLED
	@abstract	Enables Windows Event Log output.
	@discussion
	
	Windows Event Log support requires the advapi32 library, which may not be desirable, so this allows Event Log support
	to be removed entirely by disabling this.
*/

#if( !defined( DEBUG_WINDOWS_EVENT_LOG_ENABLED ) )
	#if( TARGET_OS_WINDOWS && !TARGET_OS_WINDOWS_CE )
		#define	DEBUG_WINDOWS_EVENT_LOG_ENABLED		1
	#else
		#define	DEBUG_WINDOWS_EVENT_LOG_ENABLED		0
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_CF_OBJECTS_ENABLED
	@abstract	Controls Core Foundation objects are supported. Enabling requires Core Foundation framework/DLL.
*/

#if( !defined( DEBUG_CF_OBJECTS_ENABLED ) )
	#if( !0 && ( TARGET_OS_BSD || 0 || TARGET_OS_WINDOWS ) )
		#define	DEBUG_CF_OBJECTS_ENABLED		1
	#else
		#define	DEBUG_CF_OBJECTS_ENABLED		0
	#endif
#endif

#if 0
#pragma mark ==    Macros - General ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_UNUSED
	@abstract	Macro to mark a paramter as unused to avoid unused parameter warnings.
	@discussion
	
	There is no universally supported pragma/attribute for indicating a variable is unused. DEBUG_UNUSED lets us
	indicate a variable is unused in a manner that is supported by most compilers.
*/

#define	DEBUG_UNUSED( X )			(void)(X)

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_USE_ONLY
	@abstract	Macro to mark a variable as used only when debugging is enabled.
	@discussion
	
	Variables are sometimes needed only for debugging. When debugging is turned off, these debug-only variables generate 
	compiler warnings about unused variables. To eliminate these warnings, use these macros to indicate variables that 
	are only used for debugging.
*/

#if( DEBUG )
	#define	DEBUG_USE_ONLY( X )
#else
	#define	DEBUG_USE_ONLY( X )		(void)(X)
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_STATIC
	@abstract	Macro to make variables and functions static when debugging is off, but extern when debugging is on.
	@discussion
	
	Rather than using "static" directly, using this macro allows you to access these variables externally while 
	debugging without being penalized for production builds. It also allows you to get symbols in debug builds.
*/

#if( DEBUG )
	#define	DEBUG_STATIC
#else
	#define	DEBUG_STATIC	static
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_add
	@abstract	Macro to add (or subtract if negative) a value when debugging is on. Does nothing if debugging is off.
*/

#if( DEBUG )
	#define	debug_add( A, B )		(A) += (B)
	#define	debug_sub( A, B )		(A) -= (B)
#else
	#define	debug_add( A, B )		do {} while( 0 )
	#define	debug_sub( A, B )		do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_perform
	@abstract	Macro to perform something in debug-only builds.
*/

#if( DEBUG )
	#define	debug_perform( X )		do { X; } while( 0 )
#else
	#define	debug_perform( X )		do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_track_min/debug_track_max
	@abstract	Tracks the minimum or maximum value seen.
*/

#if( DEBUG )
	#define debug_track_min( VAR, VALUE )	do { if( (VALUE) < (VAR) ) { VAR = (VALUE); } } while( 0 )
	#define debug_track_max( VAR, VALUE )	do { if( (VALUE) > (VAR) ) { VAR = (VALUE); } } while( 0 )
#else
	#define debug_track_min( VAR, VALUE )	do {} while( 0 )
	#define debug_track_max( VAR, VALUE )	do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_increment_wrap
	@abstract	Variant of increment_wrap for debug only.
*/

#if( DEBUG )
	#define debug_increment_wrap( VAR, WRAP )	increment_wrap( (VAR), (WRAP) )
#else
	#define debug_increment_wrap( VAR, WRAP )	do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_increment_saturate
	@abstract	Variant of increment_saturate for debug only.
*/

#if( DEBUG )
	#define debug_increment_saturate( VAR, MAX )	increment_saturate( (VAR), (MAX) )
#else
	#define debug_increment_saturate( VAR, MAX )	do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_add_saturate
	@abstract	Variant of add_saturate for debug only.
*/

#if( DEBUG )
	#define	debug_add_saturate( VAR, VALUE, MAX )	add_saturate( (VAR), (VALUE), (MAX) )
#else
	#define	debug_add_saturate( VAR, VALUE, MAX )	do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_true_conditional
	@abstract	Conditionalizes code using a variable for debug builds, but unconditionally true otherwise.
	@discussion
	
	Example:
	
		// Do something using a runtime conditional for debug builds, but unconditionally true with non-debug builds.
		
		if( debug_true_conditional( gMyVariableToTest ) )
		{
			// Do something
		}
*/

#if( DEBUG )
	#define debug_true_conditional( VAR )		(VAR)
#else
	#define debug_true_conditional( VAR )		1
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_false_conditional
	@abstract	Conditionalizes code using a variable for debug builds, but unconditionally false otherwise.
	@discussion
	
	Example:
	
		// Do something using a runtime conditional for debug builds, but unconditionally false with non-debug builds.
		
		if( debug_false_conditional( gMyVariableToTest ) )
		{
			// Do something
		}
*/

#if( DEBUG )
	#define debug_false_conditional( VAR )		(VAR)
#else
	#define debug_false_conditional( VAR )		0
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_trace_*
	@abstract	Tracing macros to track the time spent between two points.
	@discussion
	
	debug_trace_init:		Initializes a trace. Must be in a place where a variable can be defined.
	debug_trace_init_begin: Initializes a trace and begins a trace. Must be in a place where a variable can be defined.
	debug_trace_begin:		Begins a trace that has already been initialized.
	debug_trace_end:		Ends a trace and records stats.
	
	Example:
		
		#if( DEBUG )
			uint64_t		gMyFunctionMinNanos;
			uint64_t		gMyFunctionMaxNanos;
		#endif
		
		void	MyFunction( void )
		{
			debug_trace_init_begin( uint64_t, nanos, ClockGetNanoseconds() );
			
			int		x;
			int		y;
			
			... do some time consuming stuff.
			
			debug_trace_end( nanos, ClockGetNanoseconds(), gMyFunctionMinNanos, gMyFunctionMaxNanos );
		}
*/

#if( DEBUG )
	#define debug_trace_init( TYPE, NAME )							TYPE	NAME
	#define debug_trace_init_begin( TYPE, NAME, FUNCTION )			TYPE	NAME = FUNCTION
	#define debug_trace_begin( NAME, FUNCTION )						NAME = FUNCTION
	#define debug_trace_end( NAME, FUNCTION, MIN_VAR, MAX_VAR )		\
		do															\
		{															\
			NAME = FUNCTION - NAME;									\
			debug_track_min( MIN_VAR, NAME );						\
			debug_track_max( MAX_VAR, NAME );						\
																	\
		}	while( 0 )
#else
	#define debug_trace_init( TYPE, NAME )
	#define debug_trace_init_begin( TYPE, NAME, FUNCTION )
	#define debug_trace_begin( NAME, FUNCTION )
	#define debug_trace_end( NAME, FUNCTION, MIN_VAR, MAX_VAR )
#endif

#if 0
#pragma mark == Macros - errno ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	translate_errno
	@abstract	Returns 0 if the test is non-zero. If the test fails, returns errno if non-zero and othewise the alternate error.
*/

#if( TARGET_HAS_STD_C_LIB && !0 )
	#define translate_errno( TEST, ERRNO, ALTERNATE_ERROR )		( (TEST) ? 0 : (ERRNO) ? (ERRNO) : (ALTERNATE_ERROR) )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		GlobalErrnoMacros
	@abstract	Macros to help deal with errno.
	@discussion
	
	EXAMPLE (map_global_noerr_errno):
	
		OSStatus		err;
		
		err = gethostname( name, sizeof( name ) );
		err = map_global_noerr_errno( err );
		require_noerr( err, exit );
	
	EXAMPLE (map_global_value_errno):
	
		OSStatus		err;
		int				n;
		
		n = select( maxFD + 1, &readSet, &writeSet, NULL, timeoutPtr );
		err = map_global_value_errno( n >= 0, n );
		require_noerr( err, exit );
*/

#define map_fd_creation_errno( FD )					( IsValidFD( FD ) ? 0 : global_value_errno( FD ) )
#define map_global_noerr_errno( ERR )				( !(ERR) ? 0 : global_value_errno(ERR) )
#define map_global_value_errno( TEST, VALUE )		( (TEST) ? 0 : global_value_errno(VALUE) )

#if( TARGET_HAS_STD_C_LIB && !0 )
	#define map_noerr_errno( ERR )					map_global_noerr_errno( (ERR) )
	
	#define global_value_errno( VALUE )				( errno_compat() ? errno_compat() : kUnknownErr )
#else
	#define global_value_errno( VALUE )				( (VALUE) ? ( (OSStatus)(VALUE) ) : kUnknownErr )
#endif

#define kqueue_errno( N )		( ( (N) > 0 ) ? kNoErr : ( (N) == 0 ) ? kTimeoutErr : global_value_errno( (N) ) )
#define poll_errno( N )			( ( (N) > 0 ) ? kNoErr : ( (N) == 0 ) ? kTimeoutErr : global_value_errno( (N) ) )
#define select_errno( N )		( ( (N) > 0 ) ? kNoErr : ( (N) == 0 ) ? kTimeoutErr : global_value_errno( (N) ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		SocketErrnoMacros
	@abstract	Macros to help deal with errno for sockets.
	@discussion
	
	EXAMPLE (map_socket_creation_errno):
	
		OSStatus		err;
		SocketRef		sock;
		
		sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		err = map_socket_creation_errno( sock );
		require_noerr( err, exit );
	
	EXAMPLE (map_socket_noerr_errno):
		
		OSStatus		err;
		
		err = shutdown( sock, SHUT_WR );
		err = map_socket_noerr_errno( sock, err );
		require_noerr_quiet( err, exit );
	
	EXAMPLE (map_socket_value_errno):
	
		OSStatus		err;
		ssize_t			n;
		
		n = send( sock, "q", 1, 0 );
		err = map_socket_value_errno( sock, n == 1, n );
		require_noerr( err, exit );
*/

#define map_socket_creation_errno( SOCK )				( IsValidSocket( SOCK ) ? 0 : global_value_errno( SOCK ) )
#define map_socket_noerr_errno( SOCK, ERR )				( !(ERR) ? 0 : socket_errno( (SOCK) ) )
#define map_socket_value_errno( SOCK, TEST, VALUE )		( (TEST) ? 0 : socket_value_errno( (SOCK), (VALUE) ) )

#define socket_value_errno( SOCK, VALUE )				socket_errno( SOCK )

	#define socket_errno( SOCK )		( errno_compat() ? errno_compat() : kUnknownErr )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	map_hresult
	@abstract	Returns 0 if SUCCEEDED. If FAILED, returns the HRESULT if non-zero and othewise returns kUnknownErr.
*/

#define map_hresult( HR )		( SUCCEEDED( (HR) ) ? 0 : (HR) ? (int)(HR) : kUnknownErr )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	map_scerror
	@abstract	If TEST is true, returns 0.
				If TEST is false and SystemConfiguration framework's SCError() is non-zero, returns SCError().
				If TEST is false and SystemConfiguration framework's SCError() is zero, returns kUnknownErr.
*/

#define map_scerror( TEST )		( (TEST) ? 0 : SCError() ? SCError() : kUnknownErr )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NEW
	@abstract	Allocates an object or returns NULL if it could not be allocated.
*/

#if( !defined( NEW ) && defined( __cplusplus ) )
	#define NEW		new( std::nothrow )
#endif

#if 0
#pragma mark ==    Macros - Checks ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check
	@abstract	Check that an expression is true (non-zero).
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method.
	
	Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check ) )
	#if( DEBUG )
		#define	check( X )																					\
			do 																								\
			{																								\
				if( unlikely( !(X) ) ) 																		\
				{																							\
					debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __ROUTINE__ );						\
				}																							\
																											\
			}	while( 0 )
	#else
		#define	check( X )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_string
	@abstract	Check that an expression is true (non-zero) with an explanation.
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) and a custom explanation string using the default debugging output method.
	
	Code inside check_string() statements is not compiled into production builds.
*/

#if( !defined( check_string ) )
	#if( DEBUG )
		#define	check_string( X, STR )																		\
			do 																								\
			{																								\
				if( unlikely( !(X) ) ) 																		\
				{																							\
					debug_print_assert( 0, #X, STR, __FILE__, __LINE__, __ROUTINE__ );						\
				}																							\
																											\
			}	while( 0 )
	#else
		#define	check_string( X, STR )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_noerr
	@abstract	Check that an error code is noErr (0).
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method.
	
	Code inside check_noerr() statements is not compiled into production builds.
*/

#if( !defined( check_noerr ) )
	#if( DEBUG )
		#define	check_noerr( ERR )																			\
			do 																								\
			{																								\
				OSStatus		localErr;																	\
																											\
				localErr = (OSStatus)(ERR);																	\
				if( unlikely( localErr != 0 ) ) 															\
				{																							\
					debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );			\
				}																							\
																											\
			}	while( 0 )
	#else
		#define	check_noerr( ERR )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_noerr_string
	@abstract	Check that an error code is noErr (0) with an explanation.
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) and a custom explanation string using the default debugging output method.
	
	Code inside check_noerr_string() statements is not compiled into production builds.
*/

#if( !defined( check_noerr_string ) )
	#if( DEBUG )
		#define	check_noerr_string( ERR, STR )																\
			do 																								\
			{																								\
				OSStatus		localErr;																	\
																											\
				localErr = (OSStatus)(ERR);																	\
				if( unlikely( localErr != 0 ) ) 															\
				{																							\
					debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );				\
				}																							\
																											\
			}	while( 0 )
	#else
		#define	check_noerr_string( ERR, STR )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_ptr_overlap
	@abstract	Checks that two ptrs do not overlap.
*/

#define	check_ptr_overlap( P1, P1_SIZE, P2, P2_SIZE )									\
	do																					\
	{																					\
		check( !( ( (uintptr_t)(P1) >=     (uintptr_t)(P2) ) && 						\
				  ( (uintptr_t)(P1) <  ( ( (uintptr_t)(P2) ) + (P2_SIZE) ) ) ) );		\
		check( !( ( (uintptr_t)(P2) >=     (uintptr_t)(P1) ) && 						\
				  ( (uintptr_t)(P2) <  ( ( (uintptr_t)(P1) ) + (P1_SIZE) ) ) ) );		\
																						\
	}	while( 0 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_ptr_bounds
	@abstract	Checks that a ptr/size doesn't go outside the bounds of its enclosing buffer.
	@discussion
	
	This can used to check that a read or write from a ptr won't access outside the buffer bounds. For example:
	
	uint8_t			buf[ 1024 ];
	uint8_t *		ptr;
	size_t			size;
	
	ptr  = ... somewhere hopefully within buf
	size = ... some size
	
	check_ptr_bounds( buf, sizeof( buf ), ptr, size );
*/

#if( DEBUG )
	#define	check_ptr_bounds( BUF_BASE, BUF_SIZE, PTR, SIZE )												\
		do																									\
		{																									\
			check( (   (uintptr_t)(PTR) )				>= (   (uintptr_t)(BUF_BASE) ) );					\
			check( (   (uintptr_t)(PTR) )				<= ( ( (uintptr_t)(BUF_BASE) ) + (BUF_SIZE) ) );	\
			check( ( ( (uintptr_t)(PTR) ) + (SIZE) )	>= (   (uintptr_t)(BUF_BASE) ) );					\
			check( ( ( (uintptr_t)(PTR) ) + (SIZE) )	<= ( ( (uintptr_t)(BUF_BASE) ) + (BUF_SIZE) ) );	\
																											\
		}	while( 0 )
#else
	#define	check_ptr_bounds( BUF_BASE, BUF_SIZE, PTR, SIZE )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_panic
	@abstract	Debug-only check if an expression is non-zero and panics if it is not.
*/

#if( !defined( check_panic ) )
	#if( DEBUG )
		#define	check_panic( X, STR )															\
			do 																					\
			{																					\
				if( unlikely( !(X) ) ) 															\
				{																				\
					debug_print_assert_panic( 0, #X, STR, __FILE__, __LINE__, __ROUTINE__ );	\
				}																				\
																								\
			}	while( 0 )
	#else
		#define	check_panic( X, STR )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_noerr_panic
	@abstract	Debug-only check if an error is 0 and panics if it is not.
*/

#if( !defined( check_noerr_panic ) )
	#if( DEBUG )
		#define	check_noerr_panic( ERR, STR )																\
			do 																								\
			{																								\
				OSStatus		localErr;																	\
																											\
				localErr = (OSStatus)(ERR);																	\
				if( unlikely( localErr != 0 ) ) 															\
				{																							\
					debug_print_assert_panic( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );		\
				}																							\
																											\
			}	while( 0 )
	#else
		#define	check_noerr_panic( ERR, STR )
	#endif
#endif

#if 0
#pragma mark ==    Macros - Requires ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require
	@abstract	Requires that an expression evaluate to true.
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require ) )
	#define	require( X, LABEL )																				\
		do 																									\
		{																									\
			if( unlikely( !(X) ) ) 																			\
			{																								\
				debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __ROUTINE__ );							\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_string
	@abstract	Requires that an expression evaluate to true with an explanation.
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) and a custom explanation string using the default debugging output method then jumps to a label.
*/

#if( !defined( require_string ) )
	#define	require_string( X, LABEL, STR )																	\
		do 																									\
		{																									\
			if( unlikely( !(X) ) ) 																			\
			{																								\
				debug_print_assert( 0, #X, STR, __FILE__, __LINE__, __ROUTINE__ );							\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_quiet
	@abstract	Requires that an expression evaluate to true.
	@discussion
	
	If expression evalulates to false, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_quiet ) )
	#define	require_quiet( X, LABEL )																		\
		do 																									\
		{																									\
			if( unlikely( !(X) ) )																			\
			{																								\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr
	@abstract	Require that an error code is noErr (0).
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr ) )
	#define	require_noerr( ERR, LABEL )																		\
		do 																									\
		{																									\
			OSStatus		localErr;																		\
																											\
			localErr = (OSStatus)(ERR);																		\
			if( unlikely( localErr != 0 ) ) 																\
			{																								\
				debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );				\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_string
	@abstract	Require that an error code is noErr (0).
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.), and a custom explanation string using the default debugging output method using the 
	default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr_string ) )
	#define	require_noerr_string( ERR, LABEL, STR )															\
		do 																									\
		{																									\
			OSStatus		localErr;																		\
																											\
			localErr = (OSStatus)(ERR);																		\
			if( unlikely( localErr != 0 ) ) 																\
			{																								\
				debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action_string
	@abstract	Require that an error code is noErr (0).
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.), and a custom explanation string using the default debugging output method using the 
	default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action_string ) )
	#define	require_noerr_action_string( ERR, LABEL, ACTION, STR )											\
		do 																									\
		{																									\
			OSStatus		localErr;																		\
																											\
			localErr = (OSStatus)(ERR);																		\
			if( unlikely( localErr != 0 ) ) 																\
			{																								\
				debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );					\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_quiet
	@abstract	Require that an error code is noErr (0).
	@discussion
	
	If the error code is non-0, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_quiet ) )
	#define	require_noerr_quiet( ERR, LABEL )																\
		do 																									\
		{																									\
			if( unlikely( (ERR) != 0 ) ) 																	\
			{																								\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action
	@abstract	Require that an error code is noErr (0) with an action to execute otherwise.
	@discussion
	
	If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action ) )
	#define	require_noerr_action( ERR, LABEL, ACTION )														\
		do 																									\
		{																									\
			OSStatus		localErr;																		\
																											\
			localErr = (OSStatus)(ERR);																		\
			if( unlikely( localErr != 0 ) ) 																\
			{																								\
				debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );				\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action_quiet
	@abstract	Require that an error code is noErr (0) with an action to execute otherwise.
	@discussion
	
	If the error code is non-0, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_action_quiet ) )
	#define	require_noerr_action_quiet( ERR, LABEL, ACTION )												\
		do 																									\
		{																									\
			if( unlikely( (ERR) != 0 ) ) 																	\
			{																								\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action
	@abstract	Requires that an expression evaluate to true with an action to execute otherwise.
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_action ) )
	#define	require_action( X, LABEL, ACTION )																\
		do 																									\
		{																									\
			if( unlikely( !(X) ) ) 																			\
			{																								\
				debug_print_assert( 0, #X, NULL, __FILE__, __LINE__, __ROUTINE__ );							\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action_string
	@abstract	Requires that an expression evaluate to true with an explanation and action to execute otherwise.
	@discussion
	
	If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
	function name, etc.) and a custom explanation string using the default debugging output method then executes an
	action and jumps to a label.
*/

#if( !defined( require_action_string ) )
	#define	require_action_string( X, LABEL, ACTION, STR )													\
		do 																									\
		{																									\
			if( unlikely( !(X) ) ) 																			\
			{																								\
				debug_print_assert( 0, #X, STR, __FILE__, __LINE__, __ROUTINE__ );							\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action_quiet
	@abstract	Requires that an expression evaluate to true with an action to execute otherwise.
	@discussion
	
	If expression evalulates to false, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_action_quiet ) )
	#define	require_action_quiet( X, LABEL, ACTION )														\
		do 																									\
		{																									\
			if( unlikely( !(X) ) ) 																			\
			{																								\
				{ ACTION; }																					\
				goto LABEL;																					\
			}																								\
																											\
		}	while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_string
	@abstract	Prints a debugging C string.
*/

#if( !defined( debug_string ) )
	#if( DEBUG )
		#define	debug_string( STR )																		\
			do 																							\
			{																							\
				debug_print_assert( 0, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );					\
																										\
			}	while( 0 )
	#else
		#define	debug_string( STR )
	#endif
#endif

#if 0
#pragma mark ==    Macros - Expect ==
#endif

//===========================================================================================================================
//	Expect macros
//===========================================================================================================================

// Expect macros allow code to include runtime checking of things that should not happen in shipping code (e.g. internal 
// programmer errors, such as a NULL parameter where it is not allowed). Once the code has been verified to work correctly 
// without asserting, the DEBUG_EXPECT_VERIFIED conditional can be set to eliminate the error checking entirely. It can 
// also be useful to measure the cost of error checking code by profiling with it enabled and with it disabled.

#if( DEBUG_EXPECT_VERIFIED )
	#define	require_expect
	#define	require_string_expect
	#define	require_quiet_expect
	#define	require_noerr_expect
	#define	require_noerr_string_expect
	#define	require_noerr_action_string_expect
	#define	require_noerr_quiet_expect
	#define	require_noerr_action_expect
	#define	require_noerr_action_quiet_expect
	#define	require_action_expect
	#define	require_action_quiet_expect
	#define	require_action_string_expect
#else
	#define	require_expect							require
	#define	require_string_expect					require_string
	#define	require_quiet_expect					require_quiet
	#define	require_noerr_expect					require_noerr
	#define	require_noerr_string_expect				require_noerr_string
	#define	require_noerr_action_string_expect		require_noerr_action_string
	#define	require_noerr_quiet_expect				require_noerr_quiet
	#define	require_noerr_action_expect				require_noerr_action
	#define	require_noerr_action_quiet_expect		require_noerr_action_quiet
	#define	require_action_expect					require_action
	#define	require_action_quiet_expect				require_action_quiet
	#define	require_action_string_expect			require_action_string
#endif

#if 0
#pragma mark ==    Macros - Debug Output ==
#endif

// dlog categories

#if( DEBUG )
	#define	dlog_control( NAME )								LogControl( (NAME) )
	#define dlog_define( NAME, LEVEL, FLAGS, PREFIX, CONFIG )	ulog_define( NAME, LEVEL, FLAGS, PREFIX, CONFIG )
	#define	dlog_extern( NAME )									ulog_extern( NAME )
#else
	#define	dlog_control( NAME )								do {} while( 0 )
	#define dlog_define( NAME, LEVEL, FLAGS, PREFIX, CONFIG )	extern int debug_empty_extern
	#define	dlog_extern( NAME )									extern int debug_empty_extern
#endif

// dlog

#if( !DEBUG_HAS_VA_ARG_MACROS )
	// No VA_ARG macros so we have to do it from a real function.
	
	int	DebugLog_C89( const char *inFunction, LogLevel inLevel, const char *inFormat, ... );
#endif
#if( DEBUG )
	#if( DEBUG_C99_VA_ARGS )
		#define	dlog( LEVEL, ... )		ulog( &log_category_from_name( DebugServicesLogging ), (LEVEL), __VA_ARGS__ )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	dlog( ARGS... )			ulog( &log_category_from_name( DebugServicesLogging ), ## ARGS )
	#else
		#define dlog					DebugLog_C89
	#endif
	
	#define dlogv( LEVEL, FORMAT, ARGS )	\
		ulogv( &log_category_from_name( DebugServicesLogging ), (LEVEL), (FORMAT), (ARGS) )
#else
	#if( DEBUG_C99_VA_ARGS )
		#define	dlog( LEVEL, ... )			do {} while( 0 )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	dlog( ARGS... )				do {} while( 0 )
	#else
		#define	dlog						while( 0 )
	#endif
	
	#define dlogv( LEVEL, FORMAT, ARGS )	do {} while( 0 )
#endif

// dlogc

#if( DEBUG )
	#define dlogc		ulog
#else
	#if( DEBUG_C99_VA_ARGS )
		#define dlogc( CATEGORY_PTR, LEVEL, ... )		do {} while( 0 )
	#elif( DEBUG_GNU_VA_ARGS )
		#define dlogc( CATEGORY_PTR, LEVEL, ARGS... )	do {} while( 0 )
	#else
		#define	dlogc									while( 0 )
	#endif
#endif

// dlogcv

#if( DEBUG )
	#define dlogcv		ulogv
#else
	#define dlogcv( CATEGORY_PTR, LEVEL, FORMAT, ARGS )		do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	dlogassert
	@abstract	Prints a debug-only assert.
*/

#if( DEBUG )
	#if( DEBUG_C99_VA_ARGS )
		#define	dlogassert( ... )	\
			DebugPrintAssert( kDebugAssertFlagsNone, kNoErr, NULL, __FILE__, __LINE__, __ROUTINE__, __VA_ARGS__ )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	dlogassert( ARGS... )	\
			DebugPrintAssert( kDebugAssertFlagsNone, kNoErr, NULL, __FILE__, __LINE__, __ROUTINE__, ## ARGS )
	#else
		#define	dlogassert				while( 0 )
	#endif
#else
	#if( DEBUG_C99_VA_ARGS )
		#define	dlogassert( ... )		do {} while( 0 )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	dlogassert( ARGS... )	do {} while( 0 )
	#else
		#define	dlogassert				while( 0 )
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DebugNSLog
	@abstract	Debug-only macro for the Cocoa NSLog function.
*/

#if( DEBUG )
	#if( DEBUG_C99_VA_ARGS )
		#define	DebugNSLog( ... )			NSLog( __VA_ARGS__ )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	DebugNSLog( ARGS... )		NSLog( ## ARGS )
	#else
		#define	DebugNSLog					NSLog
	#endif
#else
	#if( DEBUG_C99_VA_ARGS )
		#define	DebugNSLog( ... )			do {} while( 0 )
	#elif( DEBUG_GNU_VA_ARGS )
		#define	DebugNSLog( ARGS... )		do {} while( 0 )
	#else
		#define	DebugNSLog					while( 0 )
	#endif
#endif

#if 0
#pragma mark ==    Routines - General ==
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DEBUG_HALT
	@abstract	Halts the program.
*/

#if( TARGET_CPU_PPC || TARGET_CPU_PPC64 )
	#define DEBUG_HALT()		__asm__ __volatile__( "trap" )
#elif( TARGET_CPU_X86 || TARGET_CPU_X86_64 )
	#if( defined( __GNUC__ ) )
		#define DEBUG_HALT()	__asm__ __volatile__( "int3" )
	#elif( _MSC_VER )
		#define DEBUG_HALT()	__asm int 3
	#elif( defined( __MWERKS__ ) )
		#define DEBUG_HALT()	DebugBreak()
	#else
		#warning "unknown x86 compiler...using infinite loop for DEBUG_HALT()"
		#define DEBUG_HALT()	do { for( ;; ) {} } while( 0 )
	#endif
#elif( TARGET_CPU_ARM )
	#define DEBUG_HALT()		__asm__ __volatile__( "bkpt 0xCF" )
#elif( TARGET_CPU_MIPS )
	#define DEBUG_HALT()		__asm__ __volatile__( "break" )
#elif( __clang__ || ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ >= 3 ) ) ) // GCC 3.3 or later has __builtin_trap.
	#define DEBUG_HALT()		__builtin_trap()
#else
	#warning "unknown architecture...using infinite loop for DEBUG_HALT()"
	#define DEBUG_HALT()		do { for( ;; ) {} } while( 0 )
#endif

#if 0
#pragma mark ==    Routines - Compatibility ==
#endif

//===========================================================================================================================
//	Compatibility
//===========================================================================================================================

#if( TARGET_OS_WINDOWS )
	const char *	getprogname( void );
#endif

#if 0
#pragma mark ==    Routines - Output ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugAssertFlags
	@abstract	Flags to control an assert.
*/

typedef uint32_t	DebugAssertFlags;

#define	kDebugAssertFlagsNone		0			//! No flags.
#define	kDebugAssertFlagsPanic		( 1 << 0 )	//! Panic Assert: Print the assert, print a stack trace, then stop.

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugPrintAssert
	@abstract	Prints a message describing the reason the (e.g. an assert failed), an optional error message, 
				an optional source filename, an optional source line number.

	@param		inErrorCode			Error that generated this assert or 0 if not error-code based.
	@param		inAssertString		C string containing assertion text.
	@param		inFilename			C string containing path of file where the error occurred.
	@param		inLineNumber		Line number in source file where the error occurred.
	@param		inFunction			C string containing name of function where assert occurred.
	@param		inMessageFormat		printf-style format string for the message or NULL if there is no message.
	param		VA_ARGS				Variable argument list for the message (if the message is non-NULL).

	@discussion
	
	Example output:	
	
	[ASSERT] "dataPtr != NULL" allocate memory for object failed
	[ASSERT] "MyFile.c", line 123, ("MyFunction")
	
	OR
	
	[ASSERT] -6728 (kNoMemoryErr)
	[ASSERT] "MyFile.c", line 123, ("MyFunction")
*/

void
	DebugPrintAssert( 
		DebugAssertFlags	inFlags, 
		OSStatus			inErrorCode, 
		const char *		inAssertString, 
		const char *		inFilename, 
		long				inLineNumber, 
		const char *		inFunction, 
		const char *		inMessageFormat, 
		... ) PRINTF_STYLE_FUNCTION( 7, 8 ) STATIC_ANALYZER_NORETURN;

#if( DEBUG )
	#define	debug_print_assert( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )			\
		DebugPrintAssert( kDebugAssertFlagsNone, (ERROR_CODE), (ASSERT_STRING), (FILENAME),						\
			(LINE_NUMBER), (FUNCTION), (MESSAGE) )
	
	#define	debug_print_assert_panic( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )		\
		DebugPrintAssert( kDebugAssertFlagsPanic, (ERROR_CODE), (ASSERT_STRING), (FILENAME),					\
			(LINE_NUMBER), (FUNCTION), (MESSAGE) )
#else
	#define	debug_print_assert( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )		do {} while( 0 )
	#define	debug_print_assert_panic( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )	do {} while( 0 )
#endif

#if 0
#pragma mark ==    Routines - Utilities ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugGetErrorString
	@abstract	Gets an error string from an error code.

	@param		inStatus		Error code to get the string for.
	@param		inBuffer		Optional buffer to copy the string to for non-static strings. May be null.
	@param		inBufferSize	Size of optional buffer (including space for the null terminator).
	
	@result		C string containing error string for the error code. Guaranteed to be a valid, static string. If a 
				buffer is supplied, the return value will always be a pointer to the supplied buffer, which will 
				contain the best available description of the error code. If a buffer is not supplied, the return
				value will be the best available description of the error code that can be represented as a static 
				string. This allows code that cannot use a temporary buffer to hold the result to still get a useful 
				error string in most cases, but also allows code that can use a temporary buffer to get the best 
				available description.
*/

#if( DEBUG || DEBUG_EXPORT_ERROR_STRINGS )
	const char *	DebugGetErrorString( OSStatus inErrorCode, char *inBuffer, size_t inBufferSize );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugGetNextError
	@abstract	Gets the next error code in the list of known error codes.

	@param		inIndex		Index of error code. Start at 0 and increment until this function returns an error.
	@param		outErr		Receives error code.
*/

#if( DEBUG || DEBUG_EXPORT_ERROR_STRINGS )
	OSStatus	DebugGetNextError( size_t inIndex, OSStatus *outErr );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugStackTrace
	@abstract	Prints out a stack trace to the output device.
*/

char *		DebugCopyStackTrace( OSStatus *outErr );
OSStatus	DebugStackTrace( LogLevel inLevel );

#if( DEBUG )
	#define	debug_stack_trace( LEVEL )	DebugStackTrace( (LEVEL) )
#else
	#define	debug_stack_trace( LEVEL )	do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugValidPtr
	@abstract	Returns 1 if the pointer/size can be safely accessed. Otherwise, it returns 0.
*/

int	DebugValidPtr( uintptr_t inPtr, size_t inSize, int inRead, int inWrite, int inExecute );

#if( DEBUG )
	#define	debug_valid_ptr_r( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 1, 0, 0 )
	#define	debug_valid_ptr_w( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 0, 1, 0 )
	#define	debug_valid_ptr_e( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 0, 0, 1 )
	#define	debug_valid_ptr_rw( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 1, 1, 0 )
	#define	debug_valid_ptr_re( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 1, 0, 1 )
	#define	debug_valid_ptr_we( PTR, SIZE )		DebugValidPtr( (uintptr_t)(PTR), (SIZE), 0, 1, 1 )
	#define	debug_valid_ptr_rwe( PTR, SIZE )	DebugValidPtr( (uintptr_t)(PTR), (SIZE), 1, 1, 1 )
#else
	#define	debug_valid_ptr_r( PTR, SIZE )		1
	#define	debug_valid_ptr_w( PTR, SIZE )		1
	#define	debug_valid_ptr_e( PTR, SIZE )		1
	#define	debug_valid_ptr_rw( PTR, SIZE )		1
	#define	debug_valid_ptr_re( PTR, SIZE )		1
	#define	debug_valid_ptr_we( PTR, SIZE )		1
	#define	debug_valid_ptr_rwe( PTR, SIZE )	1
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugIsDebuggerPresent
	@abstract	Returns true if a debugger is detected.
*/

int	DebugIsDebuggerPresent( void );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugEnterDebugger
	@abstract	Tries to enter the debugger.
*/

void	DebugEnterDebugger( void );

#if( DEBUG )
	#define debug_enter_debugger()		DebugEnterDebugger()
#else
	#define debug_enter_debugger()		do {} while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	GetProcessNameByPID
	@abstract	Gets a process name from a PID.
*/

#if( TARGET_OS_POSIX )
	char *	GetProcessNameByPID( pid_t inPID, char *inNameBuf, size_t inMaxLen );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPGetReasonPhrase
	@abstract	Returns the reason phrase associated with an HTTP-style status code or an empty string if not found/valid.
*/

const char *	HTTPGetReasonPhrase( int inStatusCode );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	ReportCriticalError
	@abstract	Reports a critical error, optionally triggering a crash log.
*/

void	ReportCriticalError( const char *inReason, uint32_t inExceptionCode, Boolean inCrashLog );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	RollLogFiles
	@abstract	Optionally closes the current log file, shifts existing files, and caps the log files to a max count.
*/

#if( TARGET_HAS_C_LIB_IO )
	OSStatus	RollLogFiles( FILE **ioLogFile, const char *inEndMessage, const char *inBaseName, int inMaxFiles );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	ObjectTrackerRegister
	@abstract	Registers a tracker for an object with option to print a stack trace or call a function when it's released.
	@discussion	Object must be a backed by an Objective-C object, such as CFDictionary, xpc_object_t, Objective-C object, etc.
*/

#define kObjectTrackerFlags_None			0
#define kObjectTrackerFlag_PrintStack		( 1 << 0 ) // Print stack trace on release.

typedef void ( *ObjectTrackerRelease_f )( const void *inObj, void *inContext );

OSStatus
	ObjectTrackerRegister( 
		const void *			inObj, 
		uint32_t				inFlags, 
		ObjectTrackerRelease_f	inCallback, 
		void *					inContext );
void	ObjectTrackerDeregister( CFTypeRef inObj );
		
#if 0
#pragma mark ==    Routines - Unit Tests ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugServicesTest
	@abstract	Unit test.
*/

OSStatus	DebugServicesTest( void );

#ifdef __cplusplus
}
#endif

#if 0
#pragma mark -
#pragma mark == PrintF ==
#endif

//===========================================================================================================================
//	PrintF
//
//	Extra features over the standard C snprintf:
//
//	64-bit support for %d (%lld), %i (%lli), %u (%llu), %o (%llo), %x (%llx), and %b (%llb).
//	Support for hh (char), j (intmax_t), z (size_t), and t (ptrdiff_t) length modifiers. These are part of C99.
//	Support for the thousands separator "'" flag character (e.g. "%'d", 12345 -> "12,345").
//	Support for the "?" flag for optional suppression. Arg=Non-zero int to include, zero int to suppress.
//	%@		- Cocoa/CoreFoundation object. Param is the object. For containers, field width is the indent level.
//			  Note: Non-Mac builds must defined DEBUG_CF_OBJECTS_ENABLED to 1 for this feature to be enabled.
//  %#@		- Same as %@, but writes the object in XML plist format.
//	%.2a	- CEC/HDMI address (2-byte big endian converted to a.b.c.d). Arg=ptr to network address.
//	%.4a	- IPv4 address (big endian). Arg=ptr to network address.
//	%.6a	- MAC address (6 bytes, AA:BB:CC:DD:EE:FF). Arg=ptr to network address.
//	%.8a	- Fibre Channel address (AA:BB:CC:DD:EE:FF:00:11). Arg=ptr to network address.
//	%.16a	- IPv6 address (16 bytes). Arg=ptr to network address.
//	%#.4a	- IPv4 address in host byte order: Arg=uint32_t * (points to host byte order uint32_t).
//	%#.6a	- MAC address from host order uint64_t: Arg=uint64_t * (points to host byte order uint64_t).
//	%#a		- IPv4 or IPv6 mDNSAddr. Arg=ptr to mDNSAddr.
//	%##a	- IPv4 (if AF_INET defined) or IPv6 (if AF_INET6 defined) sockaddr. Arg=ptr to sockaddr.
//	%b		- Binary representation of integer (e.g. 01101011). Modifiers and arg=the same as %d, %x, etc.
//	%##b	- Bit numbers of integer (0xA -> "4 2"). 0 bit at right. Modifiers and arg=mostly the same as %d, %x, etc.
//	%###b	- Bit numbers of integer (0xA -> "4 2"). 0 bit at left. Modifiers and arg=mostly the same as %d, %x, etc.
//			  1 h defaults to 16 bits (%##hb), 2 l defaults to 64 bits (%##llb), otherwise defaults to 32 bits.
//			  Precision overrides the number of bits (e.g. %###.8llb means IBM bit 0 is normal bit 7).
//	%'c		- Same as %c, but wrapped in single quotes (e.g. %'c, 'a' -> "'a'").
//	%C		- Mac-style FourCharCode (e.g. 'APPL'). Arg=32-bit value to print as a Mac-style FourCharCode.
//	%'C		- Same as %C, but wrapped in single quotes (e.g. %'C, 'abcd' -> "'abcd'").
//	%H		- Hex Dump (e.g. "\x6b\xa7" -> "6B A7"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	%#H		- Hex Dump & ASCII (e.g. "\x41\x62" -> "6B A7 |Ab|"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	%##H	- Raw ASCII dump (e.g. "\x41\x62" -> "|Ab|"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	%.1H	- Multi-line hex dump with ASCII. 1st arg=ptr, 2nd arg=size, 3rd arg=max size. Field width is indent count.
//	%.2H	- Hex dump: 1 line <= 16, multi-line > 16. 1st arg=ptr, 2nd arg=size, 3rd arg=max size. Field width is indent count.
//	%.3H	- Lowercase hex byte stream (e.g. "00aa11bb"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
//	%m		- Error Message (e.g. 0 -> "kNoErr"). Arg=error code. Size modifiers are the same as %d, %x, etc.
//	%#m		- Error and Error Message (e.g. 0 -> "0 kNoErr"). Arg=error code. Size modifiers are the same as %d, %x, etc.
//	%##m	- Same as %#m except output is in columns for nicer display of multiple error codes.
//	%N		- Now (current Date/Time string) as YYYY-MM-DD HH:MM:SS AM/PM. No arg.
//	%#s		- Pascal-style length-prefixed string. Arg=ptr to string.
//	%##s	- DNS label-sequence name. Arg=ptr to name.
//	%###s	- Cleansed function name (i.e. isolate just the [<class>::]<function> part). Arg=ptr to C-string function name.
//	%'s		- Same as %s, but wrapped in single quotes (e.g. "%'s, "test" -> "'test'".
//	%''s	- Same as %s, but wrapped in double quotes (e.g. "%''s, "test" -> ""test"".
//	%S		- UTF-16 string, 0x0000 terminated. Host order if no BOM. Precision is UTF-16 count. Precision includes BOM.
//	%#S		- Big Endian UTF-16 string (unless BOM overrides). Otherwise, the same as %S.
//	%##S	- Little Endian UTF-16 string (unless BOM overrides). Otherwise, the same as %S.
//	%T		- Windows TCHAR string. Arg=wchar_t * if UNICODE/_UNICODE defined, otherwise char *. Otherwise same as %s/%S.
//	%U		- Universally Unique Identifier (UUID) in Microsoft little endian format. Arg=ptr to 16-byte UUID.
//			  Little endian: 10 b8 a7 6b ad 9d d1 11 80 b4 00 c0 4f d4 30 c8 -> "6ba7b810-9dad-11d1-80b4-00c04fd430c8".
//	%#U		- Universally Unique Identifier (UUID) in big endian format. Arg=ptr to 16-byte UUID.
//			  Big endian: 6b a7 b8 10 9d ad 11 d1 80 b4 00 c0 4f d4 30 c8 -> "6ba7b810-9dad-11d1-80b4-00c04fd430c8".
//	%v		- NumVersion-style version (e.g. 1.2.3b4). Arg is uint32_t.
//	%V		- Nested PrintF format string and va_list. 1st arg=const char *format, 2nd arg=va_list *args. Note: 2nd is a ptr.
//
//	%{<extension>} extensions:
//
//	%{asbd}		CoreAudio AudioStreamBasicDescription. Arg=AudioStreamBasicDescription *.
//	%{dur}		Time Duration (e.g. 930232 seconds prints "10d 18h 23m 52s"). Arg=Same as %u.
//	%#{dur}		Time Duration (e.g. 930232 seconds prints "10d 18:23:52"). Arg=Same as %u.
//	%{ie}		IEEE 802.11 Information Element (IE).
//	%{pid}		Process name. Arg=pid_t.
//	%#{pid}		Process name with numeric PID. Arg=pid_t.
//	%{sline}	Single line string. \r and \n are replaced with ⏎. Arg=ptr to string.
//	%{text}		Multi-line text. const void *ptr, size_t len. Field width is indent count of each line.
//	%{tpl}		EFI only: Current TPL. const void *ptr, size_t len.
//	%#{tpl}		EFI only: Specified TPL. Arg=EFI_TPL.
//	%{txt}		DNS TXT record name=value pairs. const void *ptr, size_t len. Field width is indent count.
//	%{xpc}		XPC object description. xpc_object_t. Field width is indent count.
//===========================================================================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Enables limited %f floating-point output.

#if( !defined( PRINTF_ENABLE_FLOATING_POINT ) )
	#if( !0 && !TARGET_OS_WINDOWS_KERNEL )
		#define	PRINTF_ENABLE_FLOATING_POINT		1
	#else
		#define	PRINTF_ENABLE_FLOATING_POINT		0
	#endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	SNPrintF/VSNPrintF
	@abstract	PrintF to a fixed-size C string buffer.
*/

int			SNPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 3, 4 );
int			VSNPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, va_list inArgs ) PRINTF_STYLE_FUNCTION( 3, 0 );
OSStatus	SNPrintF_Add( char **ioPtr, char *inEnd, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 3, 4 );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	ASPrintF/VASPrintF
	@abstract	PrintF to a malloc'd C string.
*/

int	AppendPrintF( char **ioStr, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 2, 3 );
int	ASPrintF( char **outStr, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 2, 3 );
int	VASPrintF( char **outStr, const char *inFormat, va_list inArgs ) PRINTF_STYLE_FUNCTION( 2, 0 );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	CPrintF/VCPrintF
	@abstract	PrintF to a callback function.
*/

typedef int	( *PrintFUserCallBack )( const char *inStr, size_t inSize, void *inContext );

int	CPrintF( PrintFUserCallBack inCallBack, void *inContext, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 3, 4 );
int	VCPrintF( PrintFUserCallBack inCallBack, void *inContext, const char *inFormat, va_list inArgs ) PRINTF_STYLE_FUNCTION( 3, 0 );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	FPrintF/VFPrintF
	@abstract	Like fprintf, but supports the PrintFCore extensions and platforms that don't have fprintf.
*/

#if( TARGET_HAS_C_LIB_IO )
	int	FPrintF( FILE *inFile, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 2, 3 );
	int	VFPrintF( FILE *inFile, const char *inFormat, va_list inArgs ) PRINTF_STYLE_FUNCTION( 2, 0 );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	MemPrintF/VMemPrintF
	@abstract	Like SNPrintF, but it doesn't null terminate. Useful for inserting into existing strings.
*/

int	MemPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 3, 4 );
int	VMemPrintF( void *inBuf, size_t inMaxLen, const char *inFormat, va_list inArgs ) PRINTF_STYLE_FUNCTION( 3, 0 );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	PrintFTest
	@abstract	Unit test.
*/

OSStatus	PrintFTest( void );

#if 0
#pragma mark -
#pragma mark == LogUtils ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	group		LogUtils
	@abstract	LogUtils allow logging to be group into categories and control independently.
	@discussion
	
	Syntax:
	
		Empty control string means to delete all actions.
		
		'?' before a control string means to apply it as a default and re-read the prefs afterward.
		
		'+' before a control string means to write the final config string to prefs after applying.
		
		':' to access a variable for a category:
			
			This doesn't use '.' because it's used in name regexes to match any character (e.g. ".*:level=info").
			This doesn't use '->' because '>' is interpreted by shells for redirection to files.
			
			"MyCategory:level" for the "level" variable of MyCategory.
		
		'=' to assign a new value to a variable:
			
			"MyCategory:level=verbose" to set the level to verbose.
		
		';' after the output name for any parameters:
			
			"MyCategory:output=file;path=/var/log/MyCategory.log" to log to "/var/log/MyCategory.log".
		
		',' to separate actions in a control string:
			
			"MyCategory:level=verbose,MyCategory:output=syslog"
			
			Sets the level to verbose and the output type to syslog.
		
		Categories can be specified explicitly or via a simplified regular expression syntax:
		
			C	Matches any literal character C.
			.	Matches any single character.
			^	Matches the beginning of the input string.
			$	Matches the end of the input string
			*	Matches zero or more occurrences of the previous character.
	
	Variables:
	
		"flags" to set flags (separated by ';'):
			
			"none"		-- Clear all flags.
			"time"		-- Print the date/time      in front of all non-continuation log items.
			"pid"		-- Print the process ID     in front of all non-continuation log items.
			"program"	-- Print the program name   in front of all non-continuation log items.
			"category"	-- Print the category name  in front of all non-continuation log items.
			"level"		-- Print the log level name in front of all non-continuation log items.
			"prefix"	-- Print the prefix string  in front of all non-continuation log items.
			"function"	-- Print the function name  in front of all non-continuation log items.
			
			"MyCategory:flags=time;pid;progam" to print time, PID, and program name headers.
		
		"level" -- Set the log level.
			
			"MyCategory:level=verbose+1" to set the level to verbose + 1.
		
		"rate" -- Limit the rate of logging. 1st number: period in milliseconds. 2nd number: max logs per period.
		
			"MyCategory:rate=2;3000" to limit it to no more than 2 logs in a 3 second window.
		
		"output" -- Control where the log output goes (parameters separated with ';'):
		
			"MyCategory:output=syslog" to send log output to syslog.
		
		"output2" -- Same as "output", but for a second output (e.g. write to syslog and write to a file).
		
			"MyCategory:output2=syslog" to send log output to syslog.
	
	Outputs:
	
		"callback"			-- Prints to a callback function you specify.
			"func"				-- Pointer value (%p) to a LogOutputCallBack function to call.
			"arg"				-- Pointer value (%p) for the last context parameter to pass to the callback function.
		"console"			-- Meta output to choose the output that best represents a debug console.
		"file"				-- Prints to a file or one of the standard file-ish things (e.g. stderr, stdout).
			"path"				-- Log file path. May also be "stderr" or "stdout" to go to those special destinations.
			"roll"				-- Control log rolling. Format: roll=<maxSize>#<maxCount>.
			"backup"			-- Control log file backups. Format: backup=<base path to backup files>#<maxCount>.
		"iDebug"			-- Prints using iDebug for IOKit drivers.
		"IOLog"				-- Prints using IOLog for IOKit drivers.
		"kprintf"			-- Prints using kprintf for Mac OS X kernel code.
		"asl"				-- Prints to syslog.
			"facility"			-- Specify the ASL_KEY_FACILITY key (in dot notation). e.g., com.apple.airplay
			"sender"			-- Specify the ASL_KEY_SENDER key (in dot notation).
			"level"				-- Level to override the input log level. Format: level=<fixed level to use>.
		"syslog"			-- Prints to syslog.
			"level"				-- Level to override the input log level. Format: level=<fixed level to use>.
		"WindowsDebugger"	-- Prints to Windows debugger.
		"WindowsEventLog"	-- Prints to Windows Event Log.
		"WindowsKernel"		-- Prints to Windows kernel log window for Windows drivers.
	
	Examples:
	
		// Define a category named "MyCategory" to log notice or higher to the console.
		
		ulog_define( MyCategory, kLogLevelNotice, kLogFlags_Default, "MyCategory", NULL );
		#define my_ulog( LEVEL, ... )		ulog( &log_category_from_name( MyCategory ), (LEVEL), __VA_ARGS__ )
		
		// Log to the "MyCategory" category.
		
		my_ulog( kLogLevelNotice, "Test MyCategory\n" );
		
		// Change the "MyCategory" category from the default console output to a file at "/tmp/MyCategory.log" with rolling.
		
		LogControl( "MyCategory:output=file;path=/tmp/MyCategory.log;roll=32K#4" );
		
		# Change the log level of "MyCategory" to verbose.
		mytool logging MyCategoryMyCategory:level=verbose
*/

// LOGUTILS_ENABLED -- Controls if LogUtils is enabled at all (mainly for platforms that can't support it).

#if( !defined( LOGUTILS_ENABLED ) )
	#define LOGUTILS_ENABLED		1
#endif

#if( LOGUTILS_ENABLED )

// LOGUTILS_CF_ENABLED -- Controls whether CoreFoundation/CFPreferences support is compiled in.

#if( !defined( LOGUTILS_CF_ENABLED ) )
	#if( DEBUG_CF_OBJECTS_ENABLED )
		#define LOGUTILS_CF_ENABLED		1
	#else
		#define LOGUTILS_CF_ENABLED		0
	#endif
#endif

// LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS -- Controls whether CFDistributedNotification support is compiled in.

#if( !defined( LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS ) )
		#define LOGUTILS_CF_DISTRIBUTED_NOTIFICATIONS		0
#endif

// LOGUTILS_CF_PREFERENCES -- Controls whether CFPreferences support is compiled in.

#if( !defined( LOGUTILS_CF_PREFERENCES ) )
	#if( LOGUTILS_CF_ENABLED && ( 0 || ( 0 && !COMMON_SERVICES_NO_CORE_SERVICES ) || TARGET_OS_WINDOWS ) )
		#define LOGUTILS_CF_PREFERENCES		1
	#else
		#define LOGUTILS_CF_PREFERENCES		0
	#endif
#endif

// LOG_STATIC_LEVEL -- Controls what logging is conditionalized out at compile time.

#if( !defined( LOG_STATIC_LEVEL ) )
	#define	LOG_STATIC_LEVEL		0
#endif

// CF Constants

#define kLogUtilsRequestNotification		"logutilsreq"	//! User info dictionary contains control string, etc.
#define kLogUtilsAckNotification			"logutilsack"	//! User info dictionary contains control string, etc.
#define kLogUtilsKey_LogConfig				"logconfig" 	//! [String] LogControl string.

// LogFlags

typedef uint32_t		LogFlags;

#define kLogFlags_None				0			//! [0x00] No flags.
#define kLogFlags_Default			( kLogFlags_PrintTime | kLogFlags_PrintPrefix )

#define kLogFlags_PrintTime			( 1 << 0 )	//! [0x01] Print time of each log.
#define kLogFlags_PrintPID			( 1 << 1 )	//! [0x02] Print the process ID (PID) of each log.
#define kLogFlags_PrintProgram		( 1 << 2 )	//! [0x04] Print the program name of each log.
#define kLogFlags_PrintCategory		( 1 << 3 )	//! [0x08] Print the category name of each log.
#define kLogFlags_PrintLevel		( 1 << 4 )	//! [0x10] Print the log level name of each log.
#define kLogFlags_PrintPrefix		( 1 << 5 )	//! [0x20] Print the category name prefix of each log.
#define kLogFlags_PrintFunction		( 1 << 6 )	//! [0x40] Print the function that call ulog.

// LogOutput

typedef int		LogOutputType;

#define kLogOutputType_None					0
#define kLogOutputType_File					2
#define kLogOutputType_syslog				6
#define kLogOutputType_WindowsDebugger		8
#define kLogOutputType_WindowsEventLog		9
#define kLogOutputType_WindowsKernel		10
#define kLogOutputType_CallBack				11

typedef struct LogOutput			LogOutput;
typedef struct LogPrintFContext		LogPrintFContext;
typedef void ( *LogOutputWriter )( LogPrintFContext *inContext, LogOutput *inOutput, const char *inStr, size_t inLen );
typedef void ( *LogOutputCallBack )( LogPrintFContext *inPFContext, const char *inStr, size_t inLen, void *inContext );

struct LogOutput
{
	LogOutput *			next;
	int32_t				refCount;
	char *				configStr;
	LogOutputWriter		writer;
	LogOutputType		type;
	union
	{
		#if( DEBUG_FPRINTF_ENABLED )
		struct
		{
			char *		logFileName;
			FILE *		logFilePtr;
			int64_t		logFileSize;
			int64_t		logFileMaxSize;
			int			logFileMaxCount;
			char *		logBackupFileName;
			int			logBackupFileMaxCount;
			
		}	file;
		#endif
		
		#if( TARGET_OS_POSIX )
		struct
		{
			LogLevel		fixedLevel;
			
		}	syslog;
		#endif
		
		#if( DEBUG_WINDOWS_EVENT_LOG_ENABLED )
		struct
		{
			HANDLE		source;
		
		}	windowsEventLog;
		#endif
		
		struct
		{
			LogOutputCallBack		func;
			void *					arg;
		
		}	callback;
		
	}	config;
};

// LogCategory

typedef struct LogCategory		LogCategory;
struct LogCategory
{
	LogLevel			level;			// Current level for this category.
	LogLevel			initLevel;		// Default level for this category.
	const char *		initConfig;		// Optional config string to apply. May be NULL.
	LogFlags			flags;			// Flags affecting output, etc.
	const char *		name;			// Name of this category. Underscores separate components (e.g. "com_apple_airport").
	const char *		prefixPtr;		// Prefix to use when logging with the kLogFlags_PrintPrefix flag. May be NULL.
	int					prefixLen;		// Length of the prefix. May be 0.
	LogCategory *		next;			// Next category in the list or NULL if this is the last one.
	LogOutput *			output1;		// Output object for this category. Must never be null after init.
	LogOutput *			output2;		// Secondary output object for this category.
	uint64_t			rateInterval;	// Rate interval in UpTicks (e.g. no more than X counts in Y UpTicks).
	uint64_t			rateEnd;		// UpTicks when a rate limit period should end.
	uint32_t			rateMaxCount;	// Max number of logs in a single interval.
	uint32_t			rateCounter;	// Number of logs in the current period.
};

// Macros

#define	ulog_define( NAME, LEVEL, FLAGS, PREFIX, CONFIG )				\
	LogCategory		gLogCategory_ ## NAME = 							\
		{ kLogLevelUninitialized, (LEVEL), (CONFIG), (FLAGS), # NAME, 	\
		(PREFIX), (int) sizeof_string( (PREFIX) ), NULL, NULL, NULL, 	\
		0, 0, 0, 0 }

#define	ulog_extern( NAME )					extern LogCategory		gLogCategory_ ## NAME
#define log_category_from_name( NAME )		( gLogCategory_ ## NAME )

#define	log_category_enabled( CATEGORY_PTR, LEVEL )						\
	unlikely(															\
		( ( (LEVEL) & kLogLevelMask ) >= LOG_STATIC_LEVEL ) 		&&	\
		( ( (LEVEL) & kLogLevelMask ) >= (CATEGORY_PTR)->level ) 	&&	\
		( ( (CATEGORY_PTR)->level != kLogLevelUninitialized ) || 		\
		  __LogCategory_Initialize( (CATEGORY_PTR), (LEVEL) ) ) )

ulog_extern( DebugServicesLogging );
ulog_extern( LogUtils );

#if( DEBUG_C99_VA_ARGS )
	#define	lu_ulog( LEVEL, ... )			ulog( &log_category_from_name( LogUtils ), ( LEVEL ), __VA_ARGS__ )
#elif( DEBUG_GNU_VA_ARGS )
	#define	lu_ulog( LEVEL, ARGS... )		ulog( &log_category_from_name( LogUtils ), ( LEVEL ), ## ARGS )
#else
	#define	lu_ulog							LogPrintF_C89 // No VA_ARG macros so we have to do it from a real function.
#endif

// Prototypes

OSStatus	LogUtils_EnsureInitialized( void );
void		LogUtils_Finalize( void );

OSStatus	LogControl( const char *inCmd );
#if( LOGUTILS_CF_ENABLED )
	OSStatus	LogControlCF( CFStringRef inCmd );
#endif
#if( LOGUTILS_CF_PREFERENCES )
	void	LogSetAppID( CFStringRef inAppID );
#endif
OSStatus	LogShow( char **outOutput );

Boolean		__LogCategory_Initialize( LogCategory *inCategory, LogLevel inLevel );

#if( !DEBUG_HAS_VA_ARG_MACROS )
	int		LogPrintF_C89( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, ... );
#endif
int			LogPrintF( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, ... );
int			LogPrintFVAList( LogCategory *inCategory, const char *inFunction, LogLevel inLevel, const char *inFormat, va_list inArgs );

// ulog

#if( DEBUG_C99_VA_ARGS )
	#define ulog( CATEGORY_PTR, LEVEL, ... )											\
		do																				\
		{																				\
			if( log_category_enabled( (CATEGORY_PTR), (LEVEL) ) )						\
			{																			\
				LogPrintF( (CATEGORY_PTR), __ROUTINE__, (LEVEL), __VA_ARGS__ );			\
			}																			\
																						\
		}	while( 0 )
#elif( DEBUG_GNU_VA_ARGS )
	#define ulog( CATEGORY_PTR, LEVEL, ARGS... )										\
		do																				\
		{																				\
			if( log_category_enabled( (CATEGORY_PTR), (LEVEL) ) )						\
			{																			\
				LogPrintF( (CATEGORY_PTR), __ROUTINE__, (LEVEL), ## ARGS );				\
			}																			\
																						\
		}	while( 0 )
#else
	#define ulog		LogPrintF_C89 // No VA_ARG macros so we have to do it from a real function.
#endif

#define ulogv( CATEGORY_PTR, LEVEL, FORMAT, ARGS )										\
	do																					\
	{																					\
		if( log_category_enabled( (CATEGORY_PTR), (LEVEL) ) )							\
		{																				\
			LogPrintFVAList( (CATEGORY_PTR ), __ROUTINE__, (LEVEL), (FORMAT), (ARGS) );	\
		}																				\
																						\
	}	while( 0 )

#endif // LOGUTILS_ENABLED

#ifdef __cplusplus
}
#endif

#endif	// __APSDebugServices_h__
