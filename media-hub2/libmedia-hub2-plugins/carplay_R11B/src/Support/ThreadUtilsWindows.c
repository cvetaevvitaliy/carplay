/*
	File:    	ThreadUtilsWindows.c
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
	
	Copyright (C) 2007-2014 Apple Inc. All Rights Reserved.
*/

#include "ThreadUtils.h"

#include <errno.h>
#include <time.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "TimeUtils.h"

#if( !TARGET_OS_WINDOWS_CE )
	#include <process.h>
#endif

//===========================================================================================================================
//	Private
//===========================================================================================================================

struct pthread_key_s
{
	pthread_key_t					next;
	pthread_key_destructor_t		destructor;
	DWORD							tlsIndex;
};

static void	_pthread_key_call_destructors( void );

static Boolean					gPThreadsInitialized	= false;
static pthread_mutex_t			gPThreadKeyMutex;
static pthread_mutex_t *		gPThreadKeyMutexPtr		= NULL;
static pthread_key_t			gPThreadKeyList			= NULL;
static size_t					gPThreadKeyCount		= 0;
static DWORD					gPThreadKeySelfTLSIndex	= TLS_OUT_OF_INDEXES;

#if 0
#pragma mark == General ==
#endif

//===========================================================================================================================
//	_pthreads_ensure_initialized
//===========================================================================================================================

int	_pthreads_ensure_initialized( void )
{
	int		err;
	
	require_action_quiet( !gPThreadsInitialized, exit, err = 0 );
	
	err = pthread_mutex_init( &gPThreadKeyMutex, NULL );
	require_noerr( err, exit );
	gPThreadKeyMutexPtr = &gPThreadKeyMutex;
	
	gPThreadKeySelfTLSIndex = TlsAlloc();
	err = map_global_value_errno( gPThreadKeySelfTLSIndex != TLS_OUT_OF_INDEXES, gPThreadKeySelfTLSIndex );
	require_noerr( err, exit );
	
	gPThreadsInitialized = true;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_pthreads_finalize
//===========================================================================================================================

void	_pthreads_finalize( void )
{
	int			err;
	BOOL		good;
	
	if( gPThreadKeySelfTLSIndex != TLS_OUT_OF_INDEXES )
	{
		good = TlsFree( gPThreadKeySelfTLSIndex );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		gPThreadKeySelfTLSIndex = TLS_OUT_OF_INDEXES;
	}
	while( gPThreadKeyList )
	{
		err = pthread_key_delete( gPThreadKeyList );
		check_noerr( err );
	}
	check( gPThreadKeyCount == 0 );
	gPThreadKeyCount = 0;
	
	if( gPThreadKeyMutexPtr )
	{
		err = pthread_mutex_destroy( gPThreadKeyMutexPtr );
		check_noerr( err );
		gPThreadKeyMutexPtr = NULL;
	}
	
	gPThreadsInitialized = false;
}

#if 0
#pragma mark == Mutexes ==
#endif

//===========================================================================================================================
//	pthread_mutex_init
//===========================================================================================================================

int	pthread_mutex_init( pthread_mutex_t *inMutex, const pthread_mutexattr_t *inAttr )
{
	(void) inAttr; // Unused
	
	inMutex->state = 0;
	InitializeCriticalSectionOnce( &inMutex->cs, &inMutex->state );
	return( 0 );
}

//===========================================================================================================================
//	pthread_mutex_destroy
//===========================================================================================================================

int	pthread_mutex_destroy( pthread_mutex_t *inMutex )
{
	DeleteCriticalSectionOnce( &inMutex->cs, &inMutex->state, false );
	return( 0 );
}

//===========================================================================================================================
//	pthread_mutex_trylock
//===========================================================================================================================

int	pthread_mutex_trylock( pthread_mutex_t *inMutex )
{
	InitializeCriticalSectionOnce( &inMutex->cs, &inMutex->state );
	if( TryEnterCriticalSection( &inMutex->cs ) )
	{
		return( 0 );
	}
	return( EBUSY );
}

//===========================================================================================================================
//	pthread_mutex_lock
//===========================================================================================================================

int	pthread_mutex_lock( pthread_mutex_t *inMutex )
{
	InitializeCriticalSectionOnce( &inMutex->cs, &inMutex->state );
	EnterCriticalSection( &inMutex->cs );
	return( 0 );
}

//===========================================================================================================================
//	pthread_mutex_unlock
//===========================================================================================================================

int	pthread_mutex_unlock( pthread_mutex_t *inMutex )
{
	LeaveCriticalSection( &inMutex->cs );
	return( 0 );
}

#if 0
#pragma mark -
#pragma mark == Condition Variables ==
#endif

//===========================================================================================================================
//	pthread_cond_init
//===========================================================================================================================

int pthread_cond_init( pthread_cond_t *inCondition, const pthread_condattr_t *inAttr )
{
	int			err;
	HANDLE		h;
	
	(void) inAttr; // Unused
	
	h = CreateEvent( NULL, FALSE, FALSE, NULL );
	err = h ? 0 : (int) GetLastError();
	require_noerr( err, exit );
	
	*inCondition = h;
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_cond_destroy
//===========================================================================================================================

int	pthread_cond_destroy( pthread_cond_t *inCondition )
{
	int			err;
	BOOL		good;
	
	require_action( inCondition && *inCondition, exit, err = EINVAL );
	
	good = CloseHandle( *inCondition );
	err = good ? 0 : (int) GetLastError();
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_cond_broadcast
//===========================================================================================================================

#if( NOT_IMPLEMENTED_YET )
int	pthread_cond_broadcast( pthread_cond_t *inCondition )
{
	// Windows events do not support broadcasts.
}
#endif

//===========================================================================================================================
//	pthread_cond_signal
//===========================================================================================================================

int	pthread_cond_signal( pthread_cond_t *inCondition )
{
	int			err;
	BOOL		good;
	
	require_action( inCondition && *inCondition, exit, err = EINVAL );
	
	good = SetEvent( *inCondition );
	err = good ? 0 : (int) GetLastError();
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_cond_wait
//===========================================================================================================================

int	pthread_cond_wait( pthread_cond_t *inCondition, pthread_mutex_t *inMutex )
{
	int			err;
	DWORD		result;
	
	require_action( inCondition && *inCondition, exit, err = EINVAL );
	
	err = pthread_mutex_unlock( inMutex );
	check_noerr( err );
	
	result = WaitForSingleObject( *inCondition, INFINITE );
	
	err = pthread_mutex_lock( inMutex );
	check_noerr( err );
	
	err = ( result == WAIT_OBJECT_0 ) ? 0 : EINVAL;
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_cond_timedwait
//===========================================================================================================================

int	pthread_cond_timedwait( pthread_cond_t *inCondition, pthread_mutex_t *inMutex, const struct timespec *inAbsTime )
{
	int					err;
	struct timespec		ts;
		
	clock_gettime( CLOCK_REALTIME, &ts );
	if(   ( ts.tv_sec > inAbsTime->tv_sec ) || 
		( ( ts.tv_sec == inAbsTime->tv_sec ) && ( ts.tv_nsec >= inAbsTime->tv_nsec ) ) )
	{
		err = ETIMEDOUT;
		goto exit;
	}
	ts.tv_sec  = inAbsTime->tv_sec  - ts.tv_sec;
	ts.tv_nsec = inAbsTime->tv_nsec - ts.tv_nsec;
	if( ts.tv_nsec < 0 )
	{
		ts.tv_sec  -= 1;
		ts.tv_nsec += 1000000000;
	}
	
	err = pthread_cond_timedwait_relative_np( inCondition, inMutex, &ts );
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_cond_timedwait_relative_np
//===========================================================================================================================

int	pthread_cond_timedwait_relative_np( pthread_cond_t *inCondition, pthread_mutex_t *inMutex, const struct timespec *inRelTime )
{
	int			err;
	DWORD		result;
	DWORD		waitMs;
	
	require_action( inCondition && *inCondition, exit, err = EINVAL );
	require_action( inRelTime->tv_sec < 1073741, exit, err = kRangeErr ); // Limit to millisecond range of 2^30.
	
	err = pthread_mutex_unlock( inMutex );
	check_noerr( err );
	
	waitMs  = inRelTime->tv_sec * 1000;
	waitMs += inRelTime->tv_nsec / 1000000;
	result = WaitForSingleObject( *inCondition, waitMs );
	
	err = pthread_mutex_lock( inMutex );
	check_noerr( err );
	
	if(      result == WAIT_OBJECT_0 )	err = 0;
	else if( result == WAIT_TIMEOUT )	err = ETIMEDOUT;
	else { dlogassert( "wait error: %u/%#m", result, (OSStatus) GetLastError() ); err = EINVAL; }
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Threads ==
#endif

typedef struct
{
	pthread_func		func;
	void *				arg;
	HANDLE				threadHandle;

}	pthread_args;

static unsigned int	WINAPI _pthread_wrapper( void *inArg );

//===========================================================================================================================
//	pthread_create
//===========================================================================================================================

int	pthread_create( pthread_t *outThread, const pthread_attr_t *inAttr, pthread_func inFunc, void *inArg )
{
	int					err;
	pthread_args *		args;
	HANDLE				h;
	unsigned int		threadID;
	
	(void) inAttr; // Unused

	args = (pthread_args *) calloc( 1, sizeof( *args ) );
	require_action( args, exit, err = ENOMEM );
	
	args->func = inFunc;
	args->arg  = inArg;
	
	h = (HANDLE) _beginthreadex_compat( NULL, 0, _pthread_wrapper, args, CREATE_SUSPENDED, &threadID );
	err = h ? 0 : errno;
	require_noerr( err, exit );
	
	args->threadHandle = h;
	ResumeThread( h );
	
	*outThread = h;
	args = NULL;
	
exit:
	if( args ) free( args );
	return( err );
}

//===========================================================================================================================
//	pthread_join
//===========================================================================================================================

int	pthread_join( pthread_t inThread, void **outExitValue )
{
	int			err;
	DWORD		result;
	BOOL		good;
	
	result = WaitForSingleObject( inThread, INFINITE );
	err = ( result == WAIT_OBJECT_0 ) ? 0 : EINVAL;
	require_noerr( err, exit );
	
	good = GetExitCodeThread( inThread, &result );
	err = good ? 0 : (int) GetLastError();
	require_noerr( err, exit );
	
	good = CloseHandle( inThread );
	err = good ? 0 : (int) GetLastError();
	require_noerr( err, exit );
	
	if( outExitValue ) *outExitValue = (void *)(uintptr_t) result;
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_detach
//===========================================================================================================================

int	pthread_detach( pthread_t inThread )
{
	BOOL		good;
	int			err;
	
	good = CloseHandle( inThread );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_self
//===========================================================================================================================

pthread_t	pthread_self( void )
{
	if( gPThreadKeySelfTLSIndex != TLS_OUT_OF_INDEXES )
	{
		return( TlsGetValue( gPThreadKeySelfTLSIndex ) );
	}
	return( NULL );
}

//===========================================================================================================================
//	pthread_setname_np
//===========================================================================================================================

int	pthread_setname_np( const char *inName )
{
	#define MS_VC_EXCEPTION		0x406D1388
	
	struct
	{
		DWORD		type;		// Must be 0x1000.
		LPCSTR		name;		// Pointer to name (in user addr space).
		DWORD		threadID;	// Thread ID (-1 means calling thread).
		DWORD		flags;		// Reserved for future use, must be zero.
	
	}	info;
	
	info.type		= 0x1000;
	info.name		= inName;
	info.threadID	= (DWORD) -1;
	info.flags		= 0;
	
	__try { RaiseException( MS_VC_EXCEPTION, 0, sizeof( info )/ sizeof( ULONG_PTR ), (ULONG_PTR *) &info ); }
	__except( EXCEPTION_CONTINUE_EXECUTION ) {}
	return( 0 );
}

//===========================================================================================================================
//	_pthread_wrapper
//===========================================================================================================================

static unsigned int	WINAPI _pthread_wrapper( void *inArg )
{
	pthread_args * const		args = (pthread_args *) inArg;
	void *						result;
	
	TlsSetValue( gPThreadKeySelfTLSIndex, args->threadHandle );
	
	result = args->func( args->arg );
	free( args );
	
	if( gPThreadKeyList )
	{
		_pthread_key_call_destructors();
	}
	
	// Call _endthreadex() explicitly instead of just exiting normally to avoid memory leaks when using static run-time
	// libraries. See <http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/createthread.asp>.
	
	_endthreadex_compat( (unsigned int)(uintptr_t) result );
	return( (unsigned int)(uintptr_t) result );
}

#if 0
#pragma mark -
#pragma mark == Thread-specific data ==
#endif

//===========================================================================================================================
//	pthread_key_create
//===========================================================================================================================

int	pthread_key_create( pthread_key_t *outKey, pthread_key_destructor_t inDestructor )
{
	int					err;
	pthread_key_t		obj;
	pthread_key_t *		next;
	pthread_key_t		curr;
	
	obj = NULL;
	
	err = _pthreads_ensure_initialized();
	require_noerr( err, exit );
	
	obj = (pthread_key_t) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = ENOMEM );
	
	obj->destructor = inDestructor;
	obj->tlsIndex = TlsAlloc();
	err = map_global_value_errno( obj->tlsIndex != TLS_OUT_OF_INDEXES, obj->tlsIndex );
	require_noerr( err, exit );
	
	pthread_mutex_lock( gPThreadKeyMutexPtr );
	for( next = &gPThreadKeyList; ( curr = *next ) != NULL; next = &curr->next ) {}
	*next = obj;
	++gPThreadKeyCount;
	pthread_mutex_unlock( gPThreadKeyMutexPtr );
	
	*outKey = obj;
	obj = NULL;
	
exit:
	if( obj ) pthread_key_delete( obj );
	return( err );
}

//===========================================================================================================================
//	pthread_key_delete
//===========================================================================================================================

int	pthread_key_delete( pthread_key_t inKey )
{
	int					err;
	pthread_key_t *		next;
	pthread_key_t		curr;
	BOOL				good;
	
	require_action( inKey, exit, err = EINVAL );
	
	pthread_mutex_lock( gPThreadKeyMutexPtr );
	for( next = &gPThreadKeyList; ( curr = *next ) != NULL; next = &curr->next )
	{
		if( curr == inKey )
		{
			*next = curr->next;
			--gPThreadKeyCount;
			break;
		}
	}
	pthread_mutex_unlock( gPThreadKeyMutexPtr );
	
	if( inKey->tlsIndex != TLS_OUT_OF_INDEXES )
	{
		good = TlsFree( inKey->tlsIndex );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		inKey->tlsIndex = TLS_OUT_OF_INDEXES;
	}
	free( inKey );	
	err = 0;
	
exit:
	return( err );
}

//===========================================================================================================================
//	pthread_getspecific
//===========================================================================================================================

void *	pthread_getspecific( pthread_key_t inKey )
{
	return( TlsGetValue( inKey->tlsIndex ) );
}

//===========================================================================================================================
//	pthread_key_delete
//===========================================================================================================================

int		pthread_setspecific( pthread_key_t inKey, const void *inValue )
{
	int		err;
	BOOL	good;
	
	good = TlsSetValue( inKey->tlsIndex, (void *) inValue );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_pthread_key_call_destructors
//===========================================================================================================================

static void	_pthread_key_call_destructors( void )
{
	pthread_key_t *					keyArray;
	pthread_key_t					curr;
	pthread_key_t					temp;
	int								i, n;
	int								iter;
	void *							value;
	pthread_key_destructor_t		destructor;
	Boolean							called;
	
	pthread_mutex_lock( gPThreadKeyMutexPtr );
	
	// Make an array of the current keys because the list may change during processing.
		
	keyArray = (pthread_key_t *) malloc( gPThreadKeyCount * sizeof( *keyArray ) );
	require( keyArray, exit );
	
	n = 0;
	for( curr = gPThreadKeyList; curr; curr = curr->next )
	{
		keyArray[ n++ ] = curr;
	}
	
	// Run through each saved key. This has to be careful to search the key list before looking
	// at each key because the destructor may delete any key (including itself).
	
	for( iter = 0; iter < PTHREAD_DESTRUCTOR_ITERATIONS; ++iter )
	{
		called = false;
		for( i = 0; i < n; ++i )
		{
			temp = keyArray[ i ];
			for( curr = gPThreadKeyList; curr && ( curr != temp ); curr = curr->next ) {}
			if( !curr ) continue;
			
			destructor = curr->destructor;
			if( !destructor ) continue;
			
			value = pthread_getspecific( curr );
			if( !value ) continue;
			pthread_setspecific( curr, NULL );
			
			pthread_mutex_unlock( gPThreadKeyMutexPtr );
				destructor( value );
			pthread_mutex_lock( gPThreadKeyMutexPtr );
			called = true;
		}
		if( !called ) break;
	}
	
exit:
	pthread_mutex_unlock( gPThreadKeyMutexPtr );
	if( keyArray ) free( keyArray );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

