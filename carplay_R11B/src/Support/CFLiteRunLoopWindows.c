/*
	File:    	CFLiteRunLoopWindows.c
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

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "CFCompat.h"
#include "CFLite.h"
#include "NetUtils.h"

#pragma warning( disable:4201 )	// Disable "nonstandard extension used : nameless struct/union" warning for Microsoft headers.
#include "Mmsystem.h"
#pragma warning( default:4201 )	// Re-enable "nonstandard extension used : nameless struct/union" after Microsoft headers.
#pragma comment( lib, "Winmm.lib" )

#if( CFLITE_ENABLED )

#if 0
#pragma mark == Constants ==
#endif

//===========================================================================================================================
//	Constants
//===========================================================================================================================

#define kCFRunLoopWindowEvent_Socket		( WM_USER + 0x100 )	// Socket event.
#define kCFRunLoopWindowEvent_Wakeup		( WM_USER + 0x101 )	// Wake up the runloop.

CFStringRef		kCFRunLoopDefaultMode = CFSTR( "default" );
CFStringRef		kCFRunLoopCommonModes = CFSTR( "common" );

#if 0
#pragma mark == Types ==
#endif

//===========================================================================================================================
//	Types
//===========================================================================================================================

// CFRunLoop

struct	CFRunLoop
{
	CFRuntimeBase				base;
	Boolean						stop;
	CFRunLoopTimerRef			timerList;
	CFRunLoopSourceRef			sourceList;
	CFRunLoopSourceRef			signaledSourceList;
};

// CFRunLoopSource

struct	CFRunLoopSource
{
	CFRuntimeBase				base;
	CFRunLoopSourceRef			next;
	CFRunLoopSourceRef			nextSignaled;
	CFRunLoopRef				rl;
	CFRunLoopSourceContext		context;
};

// CFRunLoopTimer

struct	CFRunLoopTimer
{
	CFRuntimeBase				base;
	CFRunLoopTimerRef			next;
	CFRunLoopRef				rl;
	uintptr_t					timerID;
	CFAbsoluteTime				fireDate;
	CFTimeInterval				fireInterval;
	CFRunLoopTimerCallBack		callback;
	void *						context;
};

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

#define	CFRunLoopLock()			EnterCriticalSection( &gCFRunLoopLock );
#define	CFRunLoopUnlock()		LeaveCriticalSection( &gCFRunLoopLock );

static OSStatus			__CFRunLoopCreate( CFRunLoopRef *outRL );
static void				__CFRunLoopFree( CFTypeRef inObj );
static LRESULT CALLBACK __CFRunLoopWinProc( HWND inWindow, UINT inMsg, WPARAM inWParam, LPARAM inLParam );
static void				__CFRunLoopRunOne( CFRunLoopRef inRL );

static void	__CFRunLoopSourceFree( CFTypeRef inObj );
static void	__CFRunLoopTimerFree( CFTypeRef inObj );
static void __CFRunLoopTimerFired( CFRunLoopTimerRef inTimer );

static OSStatus	__CFSocketEnsureInitialized( void );
static void		__CFSocketFree( CFTypeRef inObj );
static void		__CFSocketHandler( SOCKET inNativeSock, OSStatus inError, DWORD inEvent );
static void		__CFSocketSourceSchedule( void *inInfo, CFRunLoopRef inRL, CFStringRef inMode );
static void		__CFSocketSourceCancel( void *inInfo, CFRunLoopRef inRL, CFStringRef inMode );
static OSStatus	__CFSocketSetCallBackTypes( CFSocketRef inSock, CFOptionFlags inCallBackTypes );

#if 0
#pragma mark == Globals ==
#pragma mark -
#endif

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static const CFRuntimeClass		kCFRunLoopClass =
{
	0, 					// version
	"CFRunLoop",		// className
	NULL,				// init
	NULL, 				// copy
	__CFRunLoopFree,	// finalize
	NULL,				// equal
	NULL,				// hash
	NULL, 				// copyFormattingDesc
	NULL				// copyDebugDesc
};

static const CFRuntimeClass		kCFRunLoopSourceClass =
{
	0, 						// version
	"CFRunLoopSource",		// className
	NULL,					// init
	NULL, 					// copy
	__CFRunLoopSourceFree,	// finalize
	NULL,					// equal
	NULL,					// hash
	NULL, 					// copyFormattingDesc
	NULL					// copyDebugDesc
};

static const CFRuntimeClass		kCFRunLoopTimerClass =
{
	0, 						// version
	"CFRunLoopTimer",		// className
	NULL,					// init
	NULL, 					// copy
	__CFRunLoopTimerFree,	// finalize
	NULL,					// equal
	NULL,					// hash
	NULL, 					// copyFormattingDesc
	NULL					// copyDebugDesc
};

static const CFRuntimeClass		kCFSocketClass =
{
	0, 					// version
	"CFSocket",			// className
	NULL,				// init
	NULL, 				// copy
	__CFSocketFree,		// finalize
	NULL,				// equal
	NULL,				// hash
	NULL, 				// copyFormattingDesc
	NULL				// copyDebugDesc
};

struct	CFSocket
{
	CFRuntimeBase				base;
	CFRunLoopRef				rl;
	int							busyCount;
	CFSocketNativeHandle		nativeSock;
	CFOptionFlags				flags;
	CFOptionFlags				callbackTypes;
	CFSocketCallBack			callback;
	CFSocketContext				context;
	CFRunLoopSourceRef			source;
};

static CRITICAL_SECTION			gCFRunLoopLock;
static CRITICAL_SECTION *		gCFRunLoopLockPtr			= NULL;
static HWND						gCFRunLoopWindow			= NULL;
static CFRunLoopWindowHookPtr	gCFRunLoopWindowHook		= NULL;
static void *					gCFRunLoopWindowHookContext	= NULL;

static CFTypeID					gCFRunLoopTypeID			= _kCFRuntimeNotATypeID;
static CFTypeID					gCFRunLoopSourceTypeID		= _kCFRuntimeNotATypeID;
static CFTypeID					gCFRunLoopTimerTypeID		= _kCFRuntimeNotATypeID;
static CFRunLoopRef				gCFRunLoopMain				= NULL;

static CFTypeID					gCFSocketTypeID				= _kCFRuntimeNotATypeID;
static CFMutableDictionaryRef	gWinSocketToCFSocketDict	= NULL;

//===========================================================================================================================
//	CFRunLoopEnsureInitialized
//===========================================================================================================================

OSStatus	CFRunLoopEnsureInitialized( void )
{
	OSStatus		err;
	HINSTANCE		instance;
	WNDCLASSEX		wcex;
	HWND			wind;
	
	if( gCFRunLoopMain ) { err = kNoErr; goto exit; }
	
	InitializeCriticalSection( &gCFRunLoopLock );
	gCFRunLoopLockPtr = &gCFRunLoopLock;
	
	// Create an invisible window to allow us to do work on the main thread.
	
	instance = GetModuleHandle( NULL );
	err = map_global_value_errno( instance, instance );
	require_noerr( err, exit );
	
	wcex.cbSize			= sizeof( wcex );
	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC) __CFRunLoopWinProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= instance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT( "CFLiteRunLoop" );
	wcex.hIconSm		= NULL;
	RegisterClassEx( &wcex );
	
	wind = CreateWindow( wcex.lpszClassName, wcex.lpszClassName, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, 
		instance, NULL );
	err = map_global_value_errno( wind, wind );
	require_noerr( err, exit );
	gCFRunLoopWindow = wind;
	
	// Register the core CFRunLoop classes.
	
	gCFRunLoopTypeID = _CFRuntimeRegisterClass( &kCFRunLoopClass );
	require_action( gCFRunLoopTypeID != _kCFRuntimeNotATypeID, exit, err = kUnknownErr );
	
	gCFRunLoopSourceTypeID = _CFRuntimeRegisterClass( &kCFRunLoopSourceClass );
	require_action( gCFRunLoopSourceTypeID != _kCFRuntimeNotATypeID, exit, err = kUnknownErr );
	
	gCFRunLoopTimerTypeID = _CFRuntimeRegisterClass( &kCFRunLoopTimerClass );
	require_action( gCFRunLoopTimerTypeID != _kCFRuntimeNotATypeID, exit, err = kUnknownErr );
	
	err = __CFRunLoopCreate( &gCFRunLoopMain );
	require_noerr( err, exit );
	
exit:
	if( err ) CFRunLoopFinalize();
	return( err );
}

//===========================================================================================================================
//	CFRunLoopFinalize
//===========================================================================================================================

OSStatus	CFRunLoopFinalize( void )
{
	OSStatus		err;
	BOOL			good;
	
	// NOTE: CF doesn't support unregistering CFTypes so we just have to leak them.
	
	ForgetCF( &gCFRunLoopMain );
	ForgetCF( &gWinSocketToCFSocketDict );
	if( gCFRunLoopWindow )
	{
		good = DestroyWindow( gCFRunLoopWindow );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		gCFRunLoopWindow = NULL;
	}
	if( gCFRunLoopLockPtr )
	{
		DeleteCriticalSection( gCFRunLoopLockPtr );
		gCFRunLoopLockPtr = NULL;
	}
	return( kNoErr );
}

//===========================================================================================================================
//	CFRunLoopGetWindow
//===========================================================================================================================

HWND	CFRunLoopGetWindow( void )
{
	CFRunLoopEnsureInitialized();
	check( gCFRunLoopWindow );
	return( gCFRunLoopWindow );
}

//===========================================================================================================================
//	CFRunLoopSetWindowHook
//===========================================================================================================================

void	CFRunLoopSetWindowHook( CFRunLoopWindowHookPtr inHook, void *inContext )
{
	gCFRunLoopWindowHook		= inHook;
	gCFRunLoopWindowHookContext	= inContext;
}

//===========================================================================================================================
//	__CFRunLoopCreate
//===========================================================================================================================

static OSStatus	__CFRunLoopCreate( CFRunLoopRef *outRL )
{
	OSStatus			err;
	CFRunLoopRef		rl;
	size_t				size;
	
	size = sizeof( struct CFRunLoop ) - sizeof( CFRuntimeBase );
	rl = (CFRunLoopRef) _CFRuntimeCreateInstance( kCFAllocatorDefault,  gCFRunLoopTypeID, (uint32_t) size, NULL );
	require_action( rl, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) rl ) + sizeof( CFRuntimeBase ), 0, size );
	
	*outRL = rl;
	rl = NULL;
	err = kNoErr;
	
exit:
	if( rl ) CFRelease( rl );
	return( err );
}

//===========================================================================================================================
//	__CFRunLoopFree
//===========================================================================================================================

static void	__CFRunLoopFree( CFTypeRef inObj )
{
	CFRunLoopRef			rl;
	CFRunLoopTimerRef		timer;
	CFRunLoopTimerRef		nextTimer;
	CFRunLoopSourceRef		source;
	CFRunLoopSourceRef		nextSource;
	
	rl = (CFRunLoopRef) inObj;
	
	for( timer = rl->timerList; timer; timer = nextTimer )
	{
		nextTimer = timer->next;
		CFRunLoopTimerInvalidate( timer );
		CFRelease( timer );
	}
	rl->timerList = NULL;
	
	for( source = rl->sourceList; source; source = nextSource )
	{
		nextSource = source->next;
		CFRunLoopSourceInvalidate( source );
		CFRelease( source );
	}
	rl->sourceList = NULL;
}

//===========================================================================================================================
//	__CFRunLoopWinProc
//===========================================================================================================================

static LRESULT CALLBACK __CFRunLoopWinProc( HWND inWindow, UINT inMsg, WPARAM inWParam, LPARAM inLParam )
{
	LRESULT		result;
	Boolean		handled;
	
	result = 0;
	switch( inMsg )
	{
		case kCFRunLoopWindowEvent_Socket:
			__CFSocketHandler( (SOCKET) inWParam, (OSStatus) WSAGETSELECTERROR( inLParam ), WSAGETSELECTEVENT( inLParam ) );
			__CFRunLoopRunOne( gCFRunLoopMain );
			break;
		
		case kCFRunLoopWindowEvent_Wakeup:
			__CFRunLoopRunOne( gCFRunLoopMain );
			break;
		
		case WM_TIMER:
			__CFRunLoopTimerFired( (CFRunLoopTimerRef) inWParam );
			break;
		
		default:
			handled = false;
			if( gCFRunLoopWindowHook )
			{
				handled = gCFRunLoopWindowHook( inWindow, inMsg, inWParam, inLParam, &result, gCFRunLoopWindowHookContext );
			}
			if( !handled )
			{
				result = DefWindowProc( inWindow, inMsg, inWParam, inLParam );
			}
			break;
	}
	return( result );
}

//===========================================================================================================================
//	CFRunLoopGetTypeID
//===========================================================================================================================

CFTypeID	CFRunLoopGetTypeID( void )
{
	CFRunLoopEnsureInitialized();
	return( gCFRunLoopTypeID );
}

//===========================================================================================================================
//	CFRunLoopGetCurrent
//===========================================================================================================================

CFRunLoopRef	CFRunLoopGetCurrent( void )
{
	CFRunLoopEnsureInitialized();
	return( gCFRunLoopMain );
}

//===========================================================================================================================
//	CFRunLoopGetMain
//===========================================================================================================================

CFRunLoopRef	CFRunLoopGetMain( void )
{
	// CFLite doesn't do per-thread runloops yet so the current is the main.
	
	return( CFRunLoopGetCurrent() );
}

//===========================================================================================================================
//	CFRunLoopRun
//===========================================================================================================================

void	CFRunLoopRun( void )
{
	OSStatus			err;
	CFRunLoopRef		rl;
	
	err = CFRunLoopEnsureInitialized();
	require_noerr( err, exit );
	
	rl = gCFRunLoopMain;
	rl->stop = false;
	while( !rl->stop )
	{
		BOOL		result;
		MSG			msg;
		
		result = GetMessage( &msg, NULL, 0, 0 );
		check( result != 0 ); // Should only be 0 on WM_QUIT and we should never get a WM_QUIT.
		err = map_global_value_errno( result != -1, result );
		require_noerr( err, exit );
		
		TranslateMessage( &msg );
		DispatchMessage( &msg );
    }
    
exit:
	return;
}

//===========================================================================================================================
//	CFRunLoopRunInMode
//===========================================================================================================================

int32_t	CFRunLoopRunInMode( CFStringRef inMode, CFTimeInterval inSeconds, Boolean inReturnAfterSourceHandled )
{
	(void) inMode;
	(void) inSeconds;
	(void) inReturnAfterSourceHandled;
	
	// $$$ TO DO: Implement.
	
	return( kCFRunLoopRunFinished );
}

//===========================================================================================================================
//	__CFRunLoopRunOne
//===========================================================================================================================

static void	__CFRunLoopRunOne( CFRunLoopRef inRL )
{
	check( inRL );
	
	// Process all signaled sources.
	
	while( !inRL->stop )
	{
		CFRunLoopSourceRef		rls;
		
		CFRunLoopLock();
		rls = inRL->signaledSourceList;
		if( rls ) inRL->signaledSourceList = rls->next;
		CFRunLoopUnlock();
		if( !rls ) break;
		
		check( rls->context.perform );
		rls->context.perform( rls->context.info );
	}
}

//===========================================================================================================================
//	CFRunLoopStop
//===========================================================================================================================

void	CFRunLoopStop( CFRunLoopRef inRL )
{
	OSStatus		err;
	BOOL			good;
	
	CFRunLoopLock();
	require_action( gCFRunLoopWindow, exit, err = kInternalErr );
	require_action( inRL, exit, err = kParamErr );
	
	inRL->stop = true;
	
	good = PostMessage( gCFRunLoopWindow, kCFRunLoopWindowEvent_Wakeup, 0, (LPARAM) inRL );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFRunLoopWakeUp
//===========================================================================================================================

void	CFRunLoopWakeUp( CFRunLoopRef inRL )
{
	OSStatus		err;
	BOOL			good;
	
	CFRunLoopLock();
	require_action( gCFRunLoopWindow, exit, err = kInternalErr );
	require_action( inRL, exit, err = kParamErr );
		
	good = PostMessage( gCFRunLoopWindow, kCFRunLoopWindowEvent_Wakeup, 0, (LPARAM) inRL );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFRunLoopAddSource
//===========================================================================================================================

void	CFRunLoopAddSource( CFRunLoopRef inRL, CFRunLoopSourceRef inSource, CFStringRef inMode )
{
	OSStatus		err;
		
	CFRunLoopLock();
	require_action( inRL, exit, err = kParamErr );
	require_action( inSource->rl == NULL, exit, err = kAlreadyInUseErr );
	
	CFRetain( inSource );
	inSource->rl		= inRL;
	inSource->next		= inRL->sourceList;
	inRL->sourceList	= inSource;
	
	if( inSource->context.schedule ) inSource->context.schedule( inSource->context.info, inRL, inMode );
	err = kNoErr;
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFRunLoopRemoveSource
//===========================================================================================================================

void	CFRunLoopRemoveSource( CFRunLoopRef inRL, CFRunLoopSourceRef inSource, CFStringRef inMode )
{
	OSStatus					err;
	Boolean						locked;
	CFRunLoopSourceRef *		p;
	CFRunLoopSourceRef			source;
	
	CFRunLoopLock();
	locked = true;
	
	require_action( inSource, exit, err = kParamErr );
	if( !inRL ) inRL = inSource->rl;
	require_action_quiet( inRL, exit, err = kNotInUseErr );
	
	for( p = &inRL->signaledSourceList; ( ( source = *p ) != NULL ) && ( source != inSource ); p = &source->nextSignaled ) {}
	if( source ) *p = source->next;
	
	for( p = &inRL->sourceList; ( ( source = *p ) != NULL ) && ( source != inSource ); p = &source->next ) {}
	require_action( source, exit, err = kNotFoundErr );
	*p = source->next;
	source->rl = NULL;
	
	if( source->context.cancel ) source->context.cancel( source->context.info, inRL, inMode );
	
	CFRunLoopUnlock();
	locked = false;
	
	CFRelease( source );
	err = kNoErr;
	
exit:
	if( locked ) CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFRunLoopAddTimer
//===========================================================================================================================

void	CFRunLoopAddTimer( CFRunLoopRef inRL, CFRunLoopTimerRef inTimer, CFStringRef inMode )
{
	OSStatus			err;
	CFAbsoluteTime		delayMs;
	
	(void) inMode; // Unused

	CFRunLoopLock();
	require_action( gCFRunLoopWindow, exit, err = kInternalErr );
	require_action( inRL, exit, err = kParamErr );
	require_action( inTimer->rl == NULL, exit, err = kAlreadyInUseErr );
	
	delayMs = ( inTimer->fireDate - CFAbsoluteTimeGetCurrent() ) * 1000;
	if(      delayMs < ( (CFAbsoluteTime) USER_TIMER_MINIMUM ) ) delayMs = USER_TIMER_MINIMUM;
	else if( delayMs > ( (CFAbsoluteTime) USER_TIMER_MAXIMUM ) ) delayMs = USER_TIMER_MAXIMUM;
	
	inTimer->timerID = SetTimer( gCFRunLoopWindow, (UINT_PTR) inTimer, (UINT) delayMs, NULL );
	err = map_global_value_errno( inTimer->timerID, inTier->timerID );
	require_noerr( err, exit );
	
	CFRetain( inTimer );
	inTimer->rl		= inRL;
	inTimer->next	= inRL->timerList;
	inRL->timerList	= inTimer;
	err = kNoErr;
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFRunLoopRemoveTimer
//===========================================================================================================================

void	CFRunLoopRemoveTimer( CFRunLoopRef inRL, CFRunLoopTimerRef inTimer, CFStringRef inMode )
{
	OSStatus				err;
	Boolean					locked;
	CFRunLoopTimerRef *		p;
	CFRunLoopTimerRef		timer;
	BOOL					good;
	
	(void) inMode; // Unused

	CFRunLoopLock();
	locked = true;
	
	require_action( gCFRunLoopWindow, exit, err = kInternalErr );
	require_action( inTimer, exit, err = kParamErr );
	if( !inRL ) inRL = inTimer->rl;
	require_action_quiet( inRL, exit, err = kNotInUseErr );
	
	for( p = &inRL->timerList; ( ( timer = *p ) != NULL ) && ( timer != inTimer ); p = &timer->next ) {}
	require_action( timer, exit, err = kNotFoundErr );
	
	*p = timer->next;
	timer->rl = NULL;
	
	if( timer->timerID != 0 )
	{
		good = KillTimer( gCFRunLoopWindow, timer->timerID );
		err = map_global_value_errno( good, good );
		require_noerr( err, exit );
		timer->timerID = 0;
	}
	
	CFRunLoopUnlock();
	locked = false;
	
	CFRelease( timer );
	
exit:
	if( locked ) CFRunLoopUnlock();
}

#if 0
#pragma mark -
#pragma mark == CFRunLoopSource ==
#endif

//===========================================================================================================================
//	CFRunLoopSourceGetTypeID
//===========================================================================================================================

CFTypeID	CFRunLoopSourceGetTypeID( void )
{
	CFRunLoopEnsureInitialized();
	return( gCFRunLoopSourceTypeID );
}

//===========================================================================================================================
//	CFRunLoopSourceCreate
//===========================================================================================================================

CFRunLoopSourceRef	CFRunLoopSourceCreate( CFAllocatorRef inAllocator, CFIndex inOrder, CFRunLoopSourceContext *inContext )
{
	OSStatus				err;
	CFRunLoopSourceRef		obj;
	size_t					size;
	
	(void) inOrder; // Unused
	
	size = sizeof( struct CFRunLoopSource ) - sizeof( CFRuntimeBase );
	obj = (CFRunLoopSourceRef) _CFRuntimeCreateInstance( inAllocator, CFRunLoopSourceGetTypeID(), (uint32_t) size, NULL );
	require_action( obj, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) obj ) + sizeof( CFRuntimeBase ), 0, size );
	
	obj->context = *inContext;
	err = kNoErr;	
	
exit:
	return( obj );
}

//===========================================================================================================================
//	__CFRunLoopSourceFree
//===========================================================================================================================

static void	__CFRunLoopSourceFree( CFTypeRef inObj )
{
	CFRunLoopSourceInvalidate( (CFRunLoopSourceRef) inObj );
}

//===========================================================================================================================
//	CFRunLoopSourceInvalidate
//===========================================================================================================================

void	CFRunLoopSourceInvalidate( CFRunLoopSourceRef inSource )
{
	OSStatus		err;
	
	require_action( inSource, exit, err = kParamErr );
	
	CFRunLoopRemoveSource( NULL, inSource, kCFRunLoopCommonModes );
	
	// $$$ TO DO: Wait until any currently running callback has completed if we're not inside of that callback.
	
exit:
	return;
}

//===========================================================================================================================
//	CFRunLoopSourceSignal
//===========================================================================================================================

void	CFRunLoopSourceSignal( CFRunLoopSourceRef inSource )
{
	OSStatus					err;
	Boolean						wakeup;
	CFRunLoopRef				rl;
	CFRunLoopSourceRef *		p;
	CFRunLoopSourceRef			source;
	
	wakeup = false;
	rl = NULL;
	
	CFRunLoopLock();
	require_action( inSource, exit, err = kParamErr );
	
	rl = inSource->rl;
	require_action( rl, exit, err = kNotPreparedErr );
	require_action_quiet( inSource->context.perform, exit, err = kNoErr );
	
	for( p = &rl->signaledSourceList; ( ( source = *p ) != NULL ) && ( source != inSource ); p = &source->nextSignaled ) {}
	if( !source )
	{
		inSource->nextSignaled = NULL;
		*p = inSource;
		
		wakeup = true;
	}
	
exit:
	CFRunLoopUnlock();
	if( wakeup ) CFRunLoopWakeUp( rl );
}

#if 0
#pragma mark -
#pragma mark == CFRunLoopTimer ==
#endif

//===========================================================================================================================
//	CFAbsoluteTimeGetCurrent
//===========================================================================================================================

CFAbsoluteTime	CFAbsoluteTimeGetCurrent( void )
{
	static Boolean		sInitialized		= false;
	static Boolean		sHasHighResTimer	= false;
	static double		sTimerToSecondsMultiplier;
	static double		sMillisecondsToSecondsMultiplier;
	
	if( !sInitialized )
	{
		LARGE_INTEGER		freq;
		
		if( QueryPerformanceFrequency( &freq ) )
		{
			if( freq.QuadPart != 0 )
			{
				sTimerToSecondsMultiplier = 1.0 / ( (double) freq.QuadPart );
				sHasHighResTimer = true;
			}
		}
		sMillisecondsToSecondsMultiplier = 1.0 / 1000.0;
		sInitialized = true;
	}
	if( sHasHighResTimer )
	{
		LARGE_INTEGER		t;
		
		if( QueryPerformanceCounter( &t ) )
		{
			return( ( (double) t.QuadPart ) * sTimerToSecondsMultiplier );
		}
	}
	return( ( (double) timeGetTime() ) * sMillisecondsToSecondsMultiplier );
}

//===========================================================================================================================
//	CFRunLoopTimerGetTypeID
//===========================================================================================================================

CFTypeID	CFRunLoopTimerGetTypeID( void )
{
	CFRunLoopEnsureInitialized();
	return( gCFRunLoopTimerTypeID );
}

//===========================================================================================================================
//	CFRunLoopTimerCreate
//===========================================================================================================================

CFRunLoopTimerRef
	CFRunLoopTimerCreate( 
		CFAllocatorRef			inAllocator,
		CFAbsoluteTime			inFireDate,
		CFTimeInterval			inInterval,
		CFOptionFlags			inFlags,
		CFIndex					inOrder,
		CFRunLoopTimerCallBack	inCallBack,
		CFRunLoopTimerContext *	inContext )
{
	OSStatus				err;
	CFRunLoopTimerRef		obj;
	size_t					size;
	
	(void) inFlags; // Unused
	(void) inOrder; // Unused

	obj = NULL;
	require_action( inCallBack, exit, err = kParamErr );
	
	size = sizeof( struct CFRunLoopTimer ) - sizeof( CFRuntimeBase );
	obj = (CFRunLoopTimerRef) _CFRuntimeCreateInstance( inAllocator, CFRunLoopTimerGetTypeID(), (uint32_t) size, NULL );
	require_action( obj, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) obj ) + sizeof( CFRuntimeBase ), 0, size );
	
	obj->fireDate		= inFireDate;
	obj->fireInterval	= inInterval;
	obj->callback		= inCallBack;
	obj->context		= inContext->info;
	err = kNoErr;	
	
exit:
	return( obj );
}

//===========================================================================================================================
//	CFRunLoopTimerCreateRelative
//===========================================================================================================================

OSStatus
	CFRunLoopTimerCreateRelative( 
		CFAllocatorRef			inAllocator, 
		int						inIntervalMs, 
		CFRunLoopTimerCallBack	inCallBack, 
		void *					inContext, 
		CFRunLoopTimerRef *		outTimer )
{
	OSStatus				err;
	CFRunLoopTimerRef		obj;
	size_t					size;
	
	require_action( inIntervalMs > 0, exit, err = kParamErr );
	require_action( inCallBack, exit, err = kParamErr );
	
	size = sizeof( struct CFRunLoopTimer ) - sizeof( CFRuntimeBase );
	obj = (CFRunLoopTimerRef) _CFRuntimeCreateInstance( inAllocator, CFRunLoopTimerGetTypeID(), (uint32_t) size, NULL );
	require_action( obj, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) obj ) + sizeof( CFRuntimeBase ), 0, size );
	
	obj->fireInterval	= ( ( (CFAbsoluteTime) inIntervalMs ) / 1000.0 );
	obj->fireDate		= CFAbsoluteTimeGetCurrent() + obj->fireInterval;
	obj->callback		= inCallBack;
	obj->context		= inContext;
	*outTimer = obj;
	err = kNoErr;	
	
exit:
	return( err );
}

//===========================================================================================================================
//	__CFRunLoopTimerFree
//===========================================================================================================================

static void	__CFRunLoopTimerFree( CFTypeRef inObj )
{
	CFRunLoopTimerInvalidate( (CFRunLoopTimerRef) inObj );
}

//===========================================================================================================================
//	CFRunLoopTimerInvalidate
//===========================================================================================================================

void	CFRunLoopTimerInvalidate( CFRunLoopTimerRef inTimer )
{
	OSStatus		err;
	
	require_action( inTimer, exit, err = kParamErr );
	
	CFRunLoopRemoveTimer( NULL, inTimer, kCFRunLoopCommonModes );
		
	// $$$ TO DO: Wait until any currently running callback has completed if we're not inside of that callback.
	
exit:
	return;
}

//===========================================================================================================================
//	__CFRunLoopTimerFired
//===========================================================================================================================

static void	__CFRunLoopTimerFired( CFRunLoopTimerRef inTimer )
{
	CFRunLoopTimerRef		timer;
	OSStatus				err;

	// $$$ TO DO: Locking? Calling callback with lock held or not?
	
	check( gCFRunLoopMain );
	for( timer = gCFRunLoopMain->timerList; timer && ( timer != inTimer ); timer = timer->next ) {}
	if( timer )
	{
		// If the first fire date is different from the interval then we'll need to stop the timer and restart after.
		// This may also mean the timer is not repeating (fireInterval <= 0), so we won't restart it in that case.
		
		if( timer->fireDate != timer->fireInterval )
		{
			BOOL		good;

			good = KillTimer( gCFRunLoopWindow, timer->timerID );
			err = map_global_value_errno( good, good );
			check_noerr( err );
			timer->timerID = 0;
		}
		
		// Call the callback, but re-search for the timer afterward because the callback may have released it.
		
		timer->callback( timer, timer->context );
		
		for( timer = gCFRunLoopMain->timerList; timer && ( timer != inTimer ); timer = timer->next ) {}
		if( timer )
		{
			// If the timer was stopped and there is a repeat interval then it means the initial fire date
			// and the repeat interval were different so we had to stop and now restart with the new interval.
			// Then we set fireDate = fireInterval so we know not to do this the next time around.
			
			if( ( timer->timerID == 0 ) && ( timer->fireInterval > 0 ) )
			{
				CFAbsoluteTime		intervalMs;
				
				intervalMs = timer->fireInterval * 1000;
				if(      intervalMs < ( (CFAbsoluteTime) USER_TIMER_MINIMUM ) ) intervalMs = USER_TIMER_MINIMUM;
				else if( intervalMs > ( (CFAbsoluteTime) USER_TIMER_MAXIMUM ) ) intervalMs = USER_TIMER_MAXIMUM;
	
				inTimer->timerID = SetTimer( gCFRunLoopWindow, (UINT_PTR) timer, (UINT) intervalMs, NULL );
				err = map_global_value_errno( inTimer->timerID, inTimer->timerID );
				check_noerr( err );
	
				timer->fireDate = timer->fireInterval;
			}
		}
	}
	else
	{
		dlog( kLogLevelNotice, "### timer %p expired, but no longer in list\n", inTimer );
	}
	__CFRunLoopRunOne( gCFRunLoopMain );
}

#if 0
#pragma mark -
#pragma mark == CFSocket ==
#endif

//===========================================================================================================================
//	__CFSocketEnsureInitialized
//===========================================================================================================================

static OSStatus	__CFSocketEnsureInitialized( void )
{
	OSStatus		err;
	
	CFRunLoopEnsureInitialized();
	CFRunLoopLock();
	
	if( gCFSocketTypeID == _kCFRuntimeNotATypeID )
	{
		gCFSocketTypeID = _CFRuntimeRegisterClass( &kCFSocketClass );
		require_action( gCFSocketTypeID != _kCFRuntimeNotATypeID, exit, err = kUnknownErr );
	}
	if( !gWinSocketToCFSocketDict )
	{
		CFDictionaryKeyCallBacks		keyCallBacks;
		CFDictionaryValueCallBacks		valueCallBacks;
		
		keyCallBacks.version			= 0;
		keyCallBacks.retain				= NULL;
		keyCallBacks.release			= NULL;
		keyCallBacks.copyDescription	= NULL;
		keyCallBacks.equal				= NULL;
		keyCallBacks.hash				= NULL;
		
		valueCallBacks.version			= 0;
		valueCallBacks.retain			= NULL;
		valueCallBacks.release			= NULL;
		valueCallBacks.copyDescription	= NULL;
		valueCallBacks.equal			= NULL;
		
		gWinSocketToCFSocketDict = CFDictionaryCreateMutable( NULL, 0, &keyCallBacks, &valueCallBacks );
		require_action( gWinSocketToCFSocketDict, exit, err = kNoMemoryErr );
	}
	err = kNoErr;
	
exit:
	CFRunLoopUnlock();
	return( err );
}

//===========================================================================================================================
//	CFSocketGetTypeID
//===========================================================================================================================

CFTypeID	CFSocketGetTypeID( void )
{
	__CFSocketEnsureInitialized();
	return( gCFSocketTypeID );
}

//===========================================================================================================================
//	CFSocketCreateWithNative
//===========================================================================================================================

CFSocketRef
	CFSocketCreateWithNative( 
		CFAllocatorRef			inAllocator, 
		CFSocketNativeHandle	inSock, 
		CFOptionFlags			inCallBackTypes, 
		CFSocketCallBack		inCallBack, 
		const CFSocketContext *	inContext )
{
	OSStatus		err;
	CFSocketRef		obj;
	size_t			size;
	
	size = sizeof( struct CFSocket ) - sizeof( CFRuntimeBase );
	obj = (CFSocketRef) _CFRuntimeCreateInstance( inAllocator, CFSocketGetTypeID(), (uint32_t) size, NULL );
	require_action( obj, exit, err = kNoMemoryErr );
	memset( ( (uint8_t *) obj ) + sizeof( CFRuntimeBase ), 0, size );
	
	obj->nativeSock		= inSock;
	obj->flags			= kCFSocketAutomaticallyReenableReadCallBack	|
						  kCFSocketAutomaticallyReenableAcceptCallBack	|
						  kCFSocketAutomaticallyReenableDataCallBack	|
						  kCFSocketCloseOnInvalidate;
	obj->callbackTypes	= inCallBackTypes;
	obj->callback		= inCallBack;
	obj->context		= *inContext;
	
	err = kNoErr;
	
exit:
	return( obj );
}

//===========================================================================================================================
//	__CFSocketFree
//===========================================================================================================================

static void	__CFSocketFree( CFTypeRef inObj )
{
	CFSocketInvalidate( (CFSocketRef) inObj );
}

//===========================================================================================================================
//	CFSocketInvalidate
//===========================================================================================================================

void	CFSocketInvalidate( CFSocketRef inSock )
{
	OSStatus				err;
	CFRunLoopSourceRef		source;
	
	require_action( inSock, exit, err = kParamErr );
	
	CFRunLoopLock();
	
	CFDictionaryRemoveValue( gWinSocketToCFSocketDict, (const void *)( (uintptr_t) inSock->nativeSock ) );	
	if( inSock->flags & kCFSocketCloseOnInvalidate )
	{
		ForgetSocket( &inSock->nativeSock );
	}

	source = inSock->source;
	inSock->source = NULL;
	
	CFRunLoopUnlock();
	
	if( source )
	{
		CFRunLoopSourceInvalidate( source );
		CFRelease( source );
	}
	
	// $$$ TO DO: Wait until any currently running callback has completed if we're not inside of that callback.
	
exit:
	return;
}

//===========================================================================================================================
//	CFSocketGetNative
//===========================================================================================================================

CFSocketNativeHandle	CFSocketGetNative( CFSocketRef inSock )
{
	return( inSock->nativeSock );
}

//===========================================================================================================================
//	CFSocketGetSocketFlags
//===========================================================================================================================

CFOptionFlags	CFSocketGetSocketFlags( CFSocketRef inSock )
{
	CFOptionFlags		flags;
	
	CFRunLoopLock();
	flags = inSock->flags;
	CFRunLoopUnlock();
	
	return( flags );
}

//===========================================================================================================================
//	CFSocketSetSocketFlags
//===========================================================================================================================

void	CFSocketSetSocketFlags( CFSocketRef inSock, CFOptionFlags inFlags )
{
	CFRunLoopLock();
	inSock->flags = inFlags;
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFSocketEnableCallBacks
//===========================================================================================================================

void	CFSocketEnableCallBacks( CFSocketRef inSock, CFOptionFlags inCallBackTypes )
{
	OSStatus			err;
	CFOptionFlags		old;
	
	CFRunLoopLock();
	
	old = inSock->callbackTypes;
	inSock->callbackTypes = old | inCallBackTypes;
	
	err = __CFSocketSetCallBackTypes( inSock, inCallBackTypes );
	require_noerr( err, exit );
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	CFSocketDisableCallBacks
//===========================================================================================================================

void	CFSocketDisableCallBacks( CFSocketRef inSock, CFOptionFlags inCallBackTypes )
{
	OSStatus			err;
	CFOptionFlags		old;

	CFRunLoopLock();
	
	old = inSock->callbackTypes;
	inSock->callbackTypes = old & ~inCallBackTypes;
	
	err = __CFSocketSetCallBackTypes( inSock, inCallBackTypes );
	require_noerr( err, exit );
	
	// $$$ TO DO: Wait until any currently running callback has completed if we're not inside of that callback.
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	__CFSocketHandler
//===========================================================================================================================

static void	__CFSocketHandler( SOCKET inNativeSock, OSStatus inError, DWORD inEvent )
{
	OSStatus			err;
	CFSocketRef			sock;
	CFOptionFlags		desiredCallBackTypes;
	CFOptionFlags		pendingCallBackTypes;
	
	if( inError )
	{
		dlog( kLogLevelNotice, "### socket %d failure: event 0x%X, error %#m\n", inNativeSock, inEvent, inError );
	}
	
	// Ignore the event if we don't know about this socket (for stale messages still in the queue).
	
	require_action( gWinSocketToCFSocketDict, exit, err = kNotInitializedErr );
	CFRunLoopLock();
	sock = (CFSocketRef) CFDictionaryGetValue( gWinSocketToCFSocketDict, (const void *)( (uintptr_t) inNativeSock ) );
	CFRunLoopUnlock();
	if( !sock )
	{
		dlog( kLogLevelNotice, "### socket %d msg when not in list...event=0x%X, error=%#m\n", 
			inNativeSock, inEvent, inError );
		goto exit;
	}
	
	// Map Windows event flags to CFSocket flags.
	
	desiredCallBackTypes = sock->callbackTypes;
	pendingCallBackTypes = 0;
	if( inEvent & ( FD_READ | FD_ACCEPT | FD_CONNECT | FD_CLOSE ) )
	{
		pendingCallBackTypes |= kCFSocketReadCallBack;
	}
	if( inEvent & FD_WRITE )
	{
		pendingCallBackTypes |= kCFSocketWriteCallBack;
	}
	
	// Update auto-reenable callbacks.
	
	if( ( pendingCallBackTypes & kCFSocketReadCallBack ) && !( sock->flags & kCFSocketAutomaticallyReenableReadCallBack ) )
	{
		sock->callbackTypes &= ~kCFSocketReadCallBack;
	}
	if( ( pendingCallBackTypes & kCFSocketWriteCallBack ) && !( sock->flags & kCFSocketAutomaticallyReenableWriteCallBack ) )
	{
		sock->callbackTypes &= ~kCFSocketWriteCallBack;
	}
	
	// Call the callback if its interested in receiving this type of callback.
	// $$$ TO DO: this should really mark the socket busy while in the callback so we can wait for it to complete.
	
	if( pendingCallBackTypes & desiredCallBackTypes )
	{
		sock->callback( sock, pendingCallBackTypes, NULL, NULL, sock->context.info );
	}
	else
	{
		dlog( kLogLevelNotice, "### ignored socket events 0x%X (0x%X desired)...event=0x%X, error=%#m\n", 
			pendingCallBackTypes, desiredCallBackTypes, inNativeSock, inError, inEvent );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	CFSocketCreateRunLoopSource
//===========================================================================================================================

CFRunLoopSourceRef	CFSocketCreateRunLoopSource( CFAllocatorRef inAllocator, CFSocketRef inSock, CFIndex inOrder )
{
	OSStatus					err;
	CFRunLoopSourceRef			source;
	CFRunLoopSourceContext		context;
	
	CFRunLoopLock();
	
	source = inSock->source;
	if( !source )
	{
		memset( &context, 0, sizeof( context ) );
		context.info		= inSock;
		context.schedule	= __CFSocketSourceSchedule;
		context.cancel		= __CFSocketSourceCancel;
		
		source = CFRunLoopSourceCreate( inAllocator, inOrder, &context );
		require_action( source, exit, err = kNoMemoryErr );
		
		inSock->source = source;
	}
	CFRetain( source ); // Always retain so there is 1 extra reference for ourself.
	
exit:
	CFRunLoopUnlock();
	return( source );
}

//===========================================================================================================================
//	__CFSocketSourceSchedule
//===========================================================================================================================

static void	__CFSocketSourceSchedule( void *inInfo, CFRunLoopRef inRL, CFStringRef inMode )
{
	OSStatus		err;
	CFSocketRef		sock;
	
	(void) inMode; // Unused

	CFRunLoopLock();
	
	sock = (CFSocketRef) inInfo;
	sock->rl = inRL;
	CFDictionarySetValue( gWinSocketToCFSocketDict, (const void *)( (uintptr_t)( sock->nativeSock ) ), sock );
	
	err = __CFSocketSetCallBackTypes( sock, sock->callbackTypes );
	require_noerr( err, exit );
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	__CFSocketSourceCancel
//===========================================================================================================================

static void	__CFSocketSourceCancel( void *inInfo, CFRunLoopRef inRL, CFStringRef inMode )
{
	CFSocketRef			sock;
	OSStatus			err;
	
	(void) inRL;	// Unused
	(void) inMode;	// Unused

	CFRunLoopLock();
	
	sock = (CFSocketRef) inInfo;
	sock->rl = NULL;
	
	CFDictionaryRemoveValue( gWinSocketToCFSocketDict, (const void *)( (uintptr_t) sock->nativeSock ) );	
	
	err = __CFSocketSetCallBackTypes( sock, sock->callbackTypes );
	require_noerr( err, exit );
	
exit:
	CFRunLoopUnlock();
}

//===========================================================================================================================
//	__CFSocketSetCallBackTypes
//
//	Note: assumes CFRunLoopLock is held.
//===========================================================================================================================

static OSStatus	__CFSocketSetCallBackTypes( CFSocketRef inSock, CFOptionFlags inCallBackTypes )
{
	OSStatus		err;
	long			events;
	
	if( IsValidSocket( inSock->nativeSock ) )
	{
		events = 0;
		if( inSock->rl )
		{
			if( inCallBackTypes & kCFSocketReadCallBack )
			{
				events |= FD_READ | FD_ACCEPT | FD_CONNECT | FD_CLOSE;
			}
			if( inCallBackTypes & kCFSocketWriteCallBack )
			{
				events |= FD_WRITE;
			}
		}
		err = WSAAsyncSelect( inSock->nativeSock, gCFRunLoopWindow, kCFRunLoopWindowEvent_Socket, events );
		err = map_global_value_errno( err != SOCKET_ERROR, err );
		require_noerr( err, exit );
	}
	err = kNoErr;

exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

#endif	// CFLITE_ENABLED
