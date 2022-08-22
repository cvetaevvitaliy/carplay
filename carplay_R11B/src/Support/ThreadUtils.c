/*
	File:    	ThreadUtils.c
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
	
	Copyright (C) 2010-2014 Apple Inc. All Rights Reserved.
*/

#include "ThreadUtils.h"

//===========================================================================================================================
//	GetMachThreadPriority
//===========================================================================================================================

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	SetThreadName
//===========================================================================================================================

// See <http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx>.
#pragma pack(push,8)
typedef struct
{
	DWORD		dwType;		// Must be 0x1000.
	LPCSTR		szName;		// Pointer to name (in user addr space).
	DWORD		dwThreadID;	// Thread ID (-1=caller thread).
	DWORD		dwFlags;	// Reserved for future use, must be zero.
	
}	THREADNAME_INFO;
#pragma pack(pop)

OSStatus	SetThreadName( const char *inName )
{
	THREADNAME_INFO		info;
	
	info.dwType		= 0x1000;
	info.szName		= (LPCSTR) inName;
	info.dwThreadID	= GetCurrentThreadId();
	info.dwFlags	= 0;
	
	__try
	{
		RaiseException( 0x406D1388 /* MS_VC_EXCEPTION */, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR *) &info );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
	return( kNoErr );
}

//===========================================================================================================================
//	MapPOSIXThreadPriority
//===========================================================================================================================
static int	MapPOSIXThreadPriority( int inPriority )
{
	int newPriority;

	if( inPriority >= 64 )		newPriority = THREAD_PRIORITY_TIME_CRITICAL;
	else if( inPriority >= 52 )	newPriority = THREAD_PRIORITY_HIGHEST;
	else if( inPriority >= 32 ) newPriority = THREAD_PRIORITY_ABOVE_NORMAL;
	else if( inPriority >= 31 )	newPriority = THREAD_PRIORITY_NORMAL;
	else if( inPriority >= 11 ) newPriority = THREAD_PRIORITY_BELOW_NORMAL;
	else if( inPriority >= 1 )	newPriority = THREAD_PRIORITY_LOWEST;
	else						newPriority = THREAD_PRIORITY_IDLE;

	return( newPriority );
}

#endif

//===========================================================================================================================
//	SetCurrentThreadPriority
//===========================================================================================================================

OSStatus	SetCurrentThreadPriority( int inPriority )
{
	OSStatus		err;
	
	if( inPriority == kThreadPriority_TimeConstraint )
	{
			dlogassert( "Platform doesn't support time constraint threads" );
			err = kUnsupportedErr;
			goto exit;
	}
#if( TARGET_OS_POSIX )
	else
	{
		int						policy;
		struct sched_param		sched;
		
		err = pthread_getschedparam( pthread_self(), &policy, &sched );
		require_noerr( err, exit );
		
		sched.sched_priority = inPriority;
		err = pthread_setschedparam( pthread_self(), SCHED_FIFO, &sched );
		require_noerr( err, exit );
	}
#elif( TARGET_OS_WINDOWS )
	else
	{
		BOOL		good;

		good = SetThreadPriority( GetCurrentThread(), MapPOSIXThreadPriority( inPriority ) );
		err = map_global_value_errno( good, good );
		require_noerr( err, exit );
	}
#else
	else
	{
		dlogassert( "Platform doesn't support setting thread priority" );
		err = kUnsupportedErr;
		goto exit;
	}
#endif
	
exit:
	return( err );
}
