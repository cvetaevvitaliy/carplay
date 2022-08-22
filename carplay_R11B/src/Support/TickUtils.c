/*
	File:    	TickUtils.c
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
	
	Copyright (C) 2001-2014 Apple Inc. All Rights Reserved.
*/

#include "TickUtils.h"

#include "APSCommonServices.h" // Include early for TARGET_* conditionals.
#include "APSDebugServices.h"  // Include early for DEBUG* conditionals.

#if( TARGET_OS_POSIX )
	#include <errno.h>
	
	#include <sys/time.h>
#endif

#if( TARGET_OS_WINDOWS )
	#pragma warning( disable:4201 )	// Disable "nonstandard extension used : nameless struct/union" warning for Microsoft headers.
	#include "Mmsystem.h"
	#pragma warning( default:4201 )	// Re-enable "nonstandard extension used : nameless struct/union" after Microsoft headers.
	#pragma comment( lib, "Winmm.lib" )
#endif

#if( TARGET_OS_FREEBSD || TARGET_OS_LINUX || TARGET_OS_QNX )

#if 0
#pragma mark -
#pragma mark == FreeBSD/Linux/QNX ==
#endif

//===========================================================================================================================
//	UpTicks
//===========================================================================================================================

uint64_t	UpTicks( void )
{
	uint64_t			nanos;
	struct timespec		ts;
	
	ts.tv_sec  = 0;
	ts.tv_nsec = 0;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	nanos = ts.tv_sec;
	nanos *= 1000000000;
	nanos += ts.tv_nsec;
	return( nanos );
}
#endif

#if( TARGET_OS_WINDOWS )

#if 0
#pragma mark -
#pragma mark == Windows ==
#endif

//===========================================================================================================================
//	UpTicks
//===========================================================================================================================

uint64_t	UpTicks( void )
{
	static Boolean		sInitialized		= false;
	static Boolean		sHasHighResTimer	= false;
	LARGE_INTEGER		t;
	
	if( !sInitialized )
	{
		if( QueryPerformanceCounter( &t ) )
		{
			sHasHighResTimer = true;
		}
		sInitialized = true;
	}
	if( sHasHighResTimer )
	{
		LARGE_INTEGER		t;
		
		if( QueryPerformanceCounter( &t ) )
		{
			return( (uint64_t) t.QuadPart );
		}
		else
		{
			dlogassert( "QueryPerformanceCounter() failed when it worked earlier: %#m", GetLastError() );
		}
	}
	return( (uint64_t) timeGetTime() );
}

//===========================================================================================================================
//	UpTicksPerSecond
//===========================================================================================================================

uint64_t	UpTicksPerSecond( void )
{
	static Boolean		sInitialized	= false;
	static uint64_t		sTicksPerSecond	= 1000; // Default to milliseconds (what timeGetTime returns) if init fails.
	
	if( !sInitialized )
	{
		LARGE_INTEGER		freq;
		
		if( QueryPerformanceFrequency( &freq ) && ( freq.QuadPart != 0 ) )
		{
			sTicksPerSecond  = (uint64_t) freq.QuadPart;
		}
		else
		{
			dlog( kLogLevelNotice, "no high-res timer: %#m\n", GetLastError() );
		}
		sInitialized = true;
	}
	return( sTicksPerSecond );
}

#endif // TARGET_OS_WINDOWS

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	SleepForUpTicks
//===========================================================================================================================

#if( TARGET_OS_POSIX )
void	SleepForUpTicks( uint64_t inTicks )
{
	uint64_t			ticksPerSec, ticks, deadline;
	struct timespec		ts;
	OSStatus			err;
	
	DEBUG_USE_ONLY( err );
	
	ticksPerSec = UpTicksPerSecond();
	ticks = UpTicks();
	deadline = ticks + inTicks;
	for( ; ticks < deadline; ticks = UpTicks() )
	{
		ticks		= deadline - ticks;
		ts.tv_sec   = (int32_t)(     ticks / ticksPerSec );
		ts.tv_nsec  = (int32_t)( ( ( ticks % ticksPerSec ) * kNanosecondsPerSecond ) / ticksPerSec );
		err = nanosleep( &ts, NULL );
		err = map_global_noerr_errno( err );
		if( err == EINTR ) continue;
		check_noerr( err );
	}
}
#endif

#if( TARGET_OS_WINDOWS )
void	SleepForUpTicks( uint64_t inTicks )
{
	Sleep( (DWORD)( ( inTicks * kMillisecondsPerSecond ) / UpTicksPerSecond() ) );
}
#endif

//===========================================================================================================================
//	SleepUntilUpTicks
//===========================================================================================================================

#if( TickUtils_HAS_SLEEP_PRIMITIVE )
void	SleepUntilUpTicks( uint64_t inTicks )
{
	uint64_t		ticks;
	
	if( inTicks > 0 )
	{
		ticks = UpTicks();
		if( ticks < inTicks )
		{
			SleepForUpTicks( inTicks - ticks );
		}
	}
}
#endif

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	UpTicksTo*
//===========================================================================================================================

uint64_t	UpTicksToSeconds( uint64_t inTicks )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = 1.0 / ( (double) UpTicksPerSecond() );
	return( (uint64_t)( ( (double) inTicks ) * sMultiplier ) );
}

uint64_t	UpTicksToMilliseconds( uint64_t inTicks )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) kMillisecondsPerSecond ) / ( (double) UpTicksPerSecond() );
	return( (uint64_t)( ( (double) inTicks ) * sMultiplier ) );
}

uint64_t	UpTicksToMicroseconds( uint64_t inTicks )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) kMicrosecondsPerSecond ) / ( (double) UpTicksPerSecond() );
	return( (uint64_t)( ( (double) inTicks ) * sMultiplier ) );
}

uint64_t	UpTicksToNanoseconds( uint64_t inTicks )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) kNanosecondsPerSecond ) / ( (double) UpTicksPerSecond() );
	return( (uint64_t)( ( (double) inTicks ) * sMultiplier ) );
}

uint64_t	UpTicksToNTP( uint64_t inTicks )
{
	uint64_t		ticksPerSec;
	
	ticksPerSec = UpTicksPerSecond();
	return( ( ( inTicks / ticksPerSec ) << 32 ) | ( ( ( inTicks % ticksPerSec ) << 32 ) / ticksPerSec ) );
}

//===========================================================================================================================
//	UpTicksToTimeValTimeout
//===========================================================================================================================

struct timeval *	UpTicksToTimeValTimeout( uint64_t inDeadlineTicks, struct timeval *inTimeVal )
{
	uint64_t		ticks;
	uint64_t		mics;
	
	if( inDeadlineTicks != kUpTicksForever )
	{
		ticks = UpTicks();
		if( inDeadlineTicks > ticks )
		{
			mics = UpTicksToMicroseconds( inDeadlineTicks - ticks );
			inTimeVal->tv_sec  = (int)( mics / 1000000 );
			inTimeVal->tv_usec = (int)( mics % 1000000 );
		}
		else
		{
			inTimeVal->tv_sec  = 0;
			inTimeVal->tv_usec = 0;
		}
		return( inTimeVal );
	}
	return( NULL );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	*ToUpTicks
//===========================================================================================================================

uint64_t	SecondsToUpTicks( uint64_t x )
{
	static uint64_t		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = UpTicksPerSecond();
	return( x * sMultiplier );
}

uint64_t	MillisecondsToUpTicks( uint64_t x )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) UpTicksPerSecond() ) / ( (double) kMillisecondsPerSecond );
	return( (uint64_t)( ( (double) x ) * sMultiplier ) );
}

uint64_t	MicrosecondsToUpTicks( uint64_t x )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) UpTicksPerSecond() ) / ( (double) kMicrosecondsPerSecond );
	return( (uint64_t)( ( (double) x ) * sMultiplier ) );
}

uint64_t	NanosecondsToUpTicks( uint64_t x )
{
	static double		sMultiplier = 0;
	
	if( sMultiplier == 0 ) sMultiplier = ( (double) UpTicksPerSecond() ) / ( (double) kNanosecondsPerSecond );
	return( (uint64_t)( ( (double) x ) * sMultiplier ) );
}

uint64_t	NTPtoUpTicks( uint64_t inNTP )
{
	uint64_t		ticksPerSec, ticks;
	
	ticksPerSec = UpTicksPerSecond();
	ticks =      ( inNTP >> 32 )					* ticksPerSec;
	ticks += ( ( ( inNTP & UINT32_C( 0xFFFFFFFF ) ) * ticksPerSec ) >> 32 );
	return( ticks );
}
