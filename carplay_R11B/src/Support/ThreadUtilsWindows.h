/*
	File:    	ThreadUtilsWindows.h
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

#ifndef	__ThreadUtilsWindows_h__
#define	__ThreadUtilsWindows_h__

#include "APSCommonServices.h"
#include "TimeUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================================================================
//	General
//===========================================================================================================================

int		_pthreads_ensure_initialized( void );
void	_pthreads_finalize( void );

//===========================================================================================================================
//	Mutexes
//===========================================================================================================================

#define PTHREAD_MUTEX_INITIALIZER		{ 0 }

typedef struct
{
	LONG					state;
	CRITICAL_SECTION		cs;

}	pthread_mutex_t;

typedef struct pthread_mutexattr *		pthread_mutexattr_t;

int	pthread_mutex_init( pthread_mutex_t *inMutex, const pthread_mutexattr_t *inAttr );
int	pthread_mutex_destroy( pthread_mutex_t *inMutex );
int	pthread_mutex_trylock( pthread_mutex_t *inMutex );
int	pthread_mutex_lock( pthread_mutex_t *inMutex );
int	pthread_mutex_unlock( pthread_mutex_t *inMutex );

//===========================================================================================================================
//	Condition Variables
//===========================================================================================================================

#define TARGET_NO_PTHREAD_BROADCAST		1

typedef HANDLE							pthread_cond_t;
typedef struct pthread_condattr *		pthread_condattr_t;

int pthread_cond_init( pthread_cond_t *inCondition, const pthread_condattr_t *inAttr );
int	pthread_cond_destroy( pthread_cond_t *inCondition );
int	pthread_cond_wait( pthread_cond_t *inCondition, pthread_mutex_t *inMutex );
int	pthread_cond_signal( pthread_cond_t *inCondition );
#if( NOT_IMPLEMENTED_YET )
	int	pthread_cond_broadcast( pthread_cond_t *inCondition );
#endif
int	pthread_cond_wait( pthread_cond_t *inCondition, pthread_mutex_t *inMutex );
int	pthread_cond_timedwait( pthread_cond_t *inCondition, pthread_mutex_t *inMutex, const struct timespec *inAbsTime );
int	pthread_cond_timedwait_relative_np( pthread_cond_t *inCondition, pthread_mutex_t *inMutex, const struct timespec *inRelTime );

//===========================================================================================================================
//	Threads
//===========================================================================================================================

typedef HANDLE						pthread_t;
typedef struct pthread_attr *		pthread_attr_t;

typedef void *	( *pthread_func )( void *inArg );

int			pthread_create( pthread_t *outThread, const pthread_attr_t *inAttr, pthread_func inFunc, void *inArg );
int			pthread_join( pthread_t inThread, void **outExitValue );
int			pthread_detach( pthread_t inThread );
#define		pthread_equal( T1, T2 )		( ( T1 ) == ( T2 ) )
pthread_t	pthread_self( void );
int			pthread_setname_np( const char *inName );

//===========================================================================================================================
//	Thread-specific data
//===========================================================================================================================

#define PTHREAD_DESTRUCTOR_ITERATIONS		4

typedef struct pthread_key_s *		pthread_key_t;

typedef void ( *pthread_key_destructor_t )( void *inArg );

int	pthread_key_create( pthread_key_t *outKey, pthread_key_destructor_t inDestructor );
int	pthread_key_delete( pthread_key_t inKey );

void *	pthread_getspecific( pthread_key_t inKey );
int		pthread_setspecific( pthread_key_t inKey, const void *inValue );

#if 0
#pragma mark == Debugging ==
#endif

//===========================================================================================================================
//	Debugging
//===========================================================================================================================

#if( DEBUG )
	OSStatus	ThreadUtilsWindows_Test( void );
#endif

#ifdef __cplusplus
}
#endif

#endif // __ThreadUtilsWindows_h__
