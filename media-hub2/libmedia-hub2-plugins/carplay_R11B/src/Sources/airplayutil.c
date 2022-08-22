/*
	File:    	airplayutil.c
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

#include "APSCommonServices.h" // Include early to work around problematic system headers on some systems. 

#include <stdio.h>

#include <dns_sd.h>
#include <errno.h>

#include "AirPlayCommon.h"
#include "AirPlaySettings.h"
#include "AirPlayVersion.h"
#include "APSDebugServices.h"
#include "AsyncConnection.h"
#include "AudioUtils.h"
#include "CFUtils.h"
#include "CommandLineUtils.h"
#include "DebugIPCUtils.h"
#include "HTTPClient.h"
#include "HTTPMessage.h"
#include "HTTPNetUtils.h"
#include "HTTPUtils.h"
#include "MathUtils.h"
#include "MiscUtils.h"
#include "NetPerf.h"
#include "NetUtils.h"
#include "NTPUtils.h"
#include "RandomNumberUtils.h"
#include "StringUtils.h"
#include "SystemUtils.h"
#include "TickUtils.h"

#if( TARGET_OS_POSIX )
	#include <fcntl.h>
	#include <pwd.h>
	#include <sys/stat.h>
	#include <sys/sysctl.h>
	#include <sys/uio.h>
	#include <sys/un.h>
#endif

#if( TARGET_OS_WINDOWS )
	#include <io.h>
#endif

	#include "HIDUtils.h"

#if( 1 && ( TARGET_OS_LINUX || TARGET_OS_QNX ) )
	#include "ScreenUtils.h"
#endif

	#include "APSMFiSAP.h"

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static void		cmd_control( void );
	static void	cmd_hid( void );
static void		cmd_http( void );
static void		cmd_kill_all( void );
static void		cmd_logging( void );
	static void	cmd_mfi( void );
	static void	cmd_mirror_file( void );
static void		cmd_moved_to_cuutil ( void );
static void		cmd_netperf( void );
static void		cmd_ntp( void );

static void		cmd_prefs( void );
static void		cmd_show( void );
static void		cmd_test( void );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

static int				gVerbose		= 0;
static int				gWait			= 0;

#if 0
#pragma mark == Command Options ==
#endif

//===========================================================================================================================
//	Command Line
//===========================================================================================================================

// HTTP

static const char *		gHTTPAddress	= NULL;
static int				gHTTPDecode		= false;
static const char *		gHTTPMethod		= "GET";
static int				gHTTPRepeatMs	= -1;
static const char *		gHTTPURL		= NULL;

static CLIOption		kHTTPOptions[] = 
{
	CLI_OPTION_STRING(  'a', "address",	&gHTTPAddress,	"address",	"Destination DNS name or IP address of accessory.", NULL ), 
	CLI_OPTION_BOOLEAN( 'd', "decode",	&gHTTPDecode,				"Attempt to decode based on content type.", NULL ), 
	CLI_OPTION_STRING(  'm', "method",	&gHTTPMethod,	"method",	"HTTP method (e.g. GET, POST, etc.). Defaults to GET.", NULL ), 
	CLI_OPTION_INTEGER( 'r', "repeat",	&gHTTPRepeatMs,	"ms",		"Delay between repeats. If not specified, it doesn't repeat.", NULL ), 
	CLI_OPTION_STRING(  'u', "URL",		&gHTTPURL, 		"URL",		"URL to get. Maybe be relative if an address is specified", NULL ), 
	CLI_OPTION_END()
};

// MirrorFile

static const char *		gMirrorFileSourcePath	= NULL;
static const char *		gMirrorFileDestination	= NULL;
static const char *		gMirrorFileOutputPath	= NULL;
static const char *		gMirrorFileAVCCPath		= NULL;

static CLIOption		kMirrorFileOptions[] = 
{
	CLI_OPTION_STRING_EX( 's', "source",		&gMirrorFileSourcePath,		"file path",	"Path of an H.264 file to mirror (or - for stdin).",	kCLIOptionFlags_Required, NULL ), 
	CLI_OPTION_STRING(    'd', "destination",	&gMirrorFileDestination,	"IP address",	"IP address to mirror to.", NULL ), 
	CLI_OPTION_STRING(    'a', "avcc",			&gMirrorFileAVCCPath,		"file path",	"Path to an H.264 AVCC file", NULL ), 
	CLI_OPTION_STRING(    'o', "output",		&gMirrorFileOutputPath,		"file path",	"Path to write an Annex-B format file", NULL ), 
	CLI_OPTION_END()
};

// NetPerf

static const char *		gNetPerfAddress = NULL;

static CLIOption		kNetPerfOptions[] = 
{
	CLI_OPTION_STRING( 'a', "address", &gNetPerfAddress, "address", "Destination DNS name, IP address, or URL to test.", NULL ), 
	CLI_OPTION_END()
};

// NTP

static const char *		gNTPAddress			= NULL;
static int				gNTPPort			= 0;
static int				gNTPUseRTCP			= false;
static int				gNTPTimeoutSecs		= -1;

static CLIOption		kNTPOptions[] = 
{
	CLI_OPTION_STRING(  'a', "address",	&gNTPAddress,		"address",		"IP address to sending NTP client requests to.", NULL ),
	CLI_OPTION_INTEGER( 'p', "port",	&gNTPPort,			NULL,			"Port number to use.", NULL ),
	CLI_OPTION_BOOLEAN( 'r', "rtcp",	&gNTPUseRTCP,						"For client mode: encasulate NTP requests in RTCP for testing.", NULL ),
	CLI_OPTION_INTEGER( 't', "timeout",	&gNTPTimeoutSecs,	"seconds",		"Seconds to run before stopping. Infinite if not specified.", NULL ),
	CLI_OPTION_END()
};

// Prefs

static const char *		gPrefsFilePath = NULL;

static CLIOption		kPrefsOptions[] = 
{
	CLI_OPTION_STRING( 'f', "file", &gPrefsFilePath, "path", "Custom file to read or modify.", NULL ), 
	CLI_OPTION_END()
};

// Show

static const char *		gShowCommand = NULL;

static CLIOption		kShowOptions[] = 
{
	CLI_OPTION_STRING( 'c', "command", &gShowCommand, "command", "Custom show command to use.", NULL ), 
	CLI_OPTION_END()
};

// Test

static const char *		gTestAddress			= NULL;
static int				gTestMinSize			= 1000;
static int				gTestMaxSize			= 40000;
static int				gTestFPS				= 60;
static int				gTestMinPause			= 100;
static int				gTestMaxPause			= 3000;
static int				gTestMinPauseDelay		= 1000;
static int				gTestMaxPauseDelay		= 5000;
static int				gTestMinMeasureBurst	= 6;
static int				gTestMaxMeasureBurst	= 30;
static int				gTestReportPeriodMs		= -1;
static int				gTestTimeoutSecs		= -1;

static CLIOption		kTestOptions[] = 
{
	CLI_OPTION_STRING_EX(	'a', "address",				&gTestAddress,			"IP/DNS name",	"Network address of device to test.", kCLIOptionFlags_Required, NULL ), 
	CLI_OPTION_INTEGER(		'f', "fps",					&gTestFPS,				NULL,			"Average frames per second.", NULL ), 
	CLI_OPTION_INTEGER(		's', "min-size",			&gTestMinSize,			"bytes",		"Min frame size to send.", NULL ), 
	CLI_OPTION_INTEGER(		'S', "max-size",			&gTestMaxSize,			"bytes",		"Max frame size to send.", NULL ), 
	CLI_OPTION_INTEGER(		'p', "min-pause",			&gTestMinPause,			"ms",			"Min time to pause.", NULL ), 
	CLI_OPTION_INTEGER(		'P', "max-pause",			&gTestMaxPause,			"ms",			"Max time to pause.", NULL ), 
	CLI_OPTION_INTEGER(		'd', "min-pause-delay",		&gTestMinPauseDelay,	"ms",			"Min delay between pauses.", NULL ), 
	CLI_OPTION_INTEGER(		'D', "max-pause-delay",		&gTestMaxPauseDelay,	"ms",			"Max delay between pauses.", NULL ), 
	CLI_OPTION_INTEGER(		'b', "min-measure-burst",	&gTestMinMeasureBurst,	"packets",		"Min packets before measuring a burst.", NULL ), 
	CLI_OPTION_INTEGER(		'B', "max-measure-burst",	&gTestMaxMeasureBurst,	"packets",		"Max packets to measure in a burst.", NULL ), 
	CLI_OPTION_INTEGER(		'r', "report-period",		&gTestReportPeriodMs,	"ms",			"How often to report progress.", NULL ), 
	CLI_OPTION_INTEGER(		't', "timeout",				&gTestTimeoutSecs,		"seconds",		"How many seconds to run the test.", NULL ), 
	CLI_OPTION_END()
};

#if 0
#pragma mark == Command Table ==
#endif

//===========================================================================================================================
//	Command Table
//===========================================================================================================================

#define kMovedToCUutil	"Not available in airplayutil anymore. Please use cuutil."

static CLIOption		kGlobalOptions[] = 
{
	CLI_OPTION_VERSION( kAirPlayMarketingVersionStr, kAirPlaySourceVersionStr ), 
	CLI_OPTION_HELP(), 
	CLI_OPTION_BOOLEAN( 'v', "verbose",		&gVerbose,	"Increase logging output of commands.", NULL ), 
	CLI_OPTION_INTEGER( 0,	"wait",			&gWait,		"seconds", "Seconds to wait before exiting.", NULL ), 
	
	CLI_COMMAND( "control",					cmd_control,				NULL,					"Control AirPlay internal state.", NULL ),
	CLI_COMMAND_EX( "error",				cmd_moved_to_cuutil,		NULL, kCLIOptionFlags_NotCommon, kMovedToCUutil, NULL ),
	CLI_COMMAND_EX( "hid",					cmd_hid,					NULL, kCLIOptionFlags_NotCommon, "Show HID devices.", NULL ),
	CLI_COMMAND_HELP(), 
	CLI_COMMAND( "http",					cmd_http,					kHTTPOptions,			"Download via HTTP.", NULL ),
	CLI_COMMAND( "ka",						cmd_kill_all,				NULL,					"Does killall of all AirPlay-related processes.", NULL ), 
	CLI_COMMAND( "logging",					cmd_logging,				NULL,					"Show or change the logging configuration.", NULL ), 
	CLI_COMMAND( "mfi",						cmd_mfi,					NULL,					"Tests the MFi auth IC.", NULL ), 
	CLI_COMMAND_EX( "mirror-file",			cmd_mirror_file,			kMirrorFileOptions, kCLIOptionFlags_NotCommon, "Mirrors a file to a destination.", NULL ), 
	CLI_COMMAND( "netperf",					cmd_netperf,				kNetPerfOptions,		"Network performance measurements.", NULL ),
	CLI_COMMAND( "ntp",						cmd_ntp,					kNTPOptions,			"Runs an NTP client or server for testing.", NULL ), 
	CLI_COMMAND( "prefs",					cmd_prefs,					kPrefsOptions,			"Reads/writes/deletes AirPlay prefs.", NULL ), 
	CLI_COMMAND( "show",					cmd_show,					kShowOptions,			"Shows state.", NULL ), 
	CLI_COMMAND_EX( "test",					cmd_test,					kTestOptions, kCLIOptionFlags_NotCommon, "Tests network performance.", NULL ), 
	CLI_COMMAND_VERSION( kAirPlayMarketingVersionStr, kAirPlaySourceVersionStr ), 
	
	CLI_OPTION_END()
};

//===========================================================================================================================
//	main
//===========================================================================================================================

int main( int argc, const char **argv )
{
	{
		gProgramLongName = "AirPlay Command Line Utility";
		CLIInit( argc, argv );
		CLIParse( kGlobalOptions, kCLIFlags_None );
		
	}
	if( gWait < 0 ) for( ;; ) sleep( 30 );
	if( gWait > 0 ) sleep( gWait );
	return( gExitCode );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	cmd_control
//===========================================================================================================================

static void	cmd_control( void )
{
	const char *		arg;
	
	if( gArgI >= gArgC ) ErrQuit( 1, "error: no control command specified\n" );
	arg = gArgV[ gArgI++ ];
	
{
	OSStatus		err;
	
	err = DebugIPC_PerformF( NULL, NULL,
		"{"
			"%kO=%O"
			"%kO=%s"
		"}", 
		kDebugIPCKey_Command, kDebugIPCOpCode_Control, 
		kDebugIPCKey_Value,   arg );
	if( err ) ErrQuit( EINVAL, "error: %#m\n", err );
}
}

//===========================================================================================================================
//	cmd_hid
//===========================================================================================================================

static void	_cmd_hid_show( void );
static void	_cmd_hid_browser_handler( HIDBrowserEventType inType, CFTypeRef inParam, void *inContext );
static void
	_cmd_hid_device_handler( 
		HIDDeviceRef		inDevice, 
		HIDDeviceEventType	inType, 
		OSStatus			inStatus, 
		const uint8_t *		inPtr, 
		size_t				inLen, 
		void *				inContext );

static void	cmd_hid( void )
{
	const char *		arg;
	OSStatus			err;
	HIDBrowserRef		browser = NULL;
		
	arg = ( gArgI < gArgC ) ? gArgV[ gArgI++ ] : "";
	if( stricmp( arg, "browse" ) == 0 )
	{
		err = HIDBrowserCreate( &browser );
		require_noerr( err, exit );
		
		HIDBrowserSetEventHandler( browser, _cmd_hid_browser_handler, NULL );
		err = HIDBrowserStart( browser );
		require_noerr( err, exit );
		
		FPrintF( stderr, "HID browser running\n" );
		CFRunLoopRun();
		FPrintF( stderr, "HID browser stopped\n" );
	}
	else
	{
		_cmd_hid_show();
		err = kNoErr;
	}
	
exit:
	if( browser ) HIDBrowserStopDevices( browser );
	HIDBrowserForget( &browser );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_cmd_hid_show
//===========================================================================================================================

static void	_cmd_hid_show( void )
{
	OSStatus			err;
	CFArrayRef			devices;
	CFIndex				i, n;
	HIDDeviceRef		device;
	int					width;
	int					count;
	CFStringRef			key;
	CFTypeRef			prop;
	
	devices = HIDCopyDevices( &err );
	require_noerr( err, exit );
	
	width = 13;
	count = 0;
	n = CFArrayGetCount( devices );
	for( i = 0; i < n; ++i )
	{
		device = (HIDDeviceRef) CFArrayGetValueAtIndex( devices, i );
		
		key = kHIDDeviceProperty_DisplayUUID;
		prop = HIDDeviceCopyProperty( device, key, NULL, NULL );
		FPrintF( stdout, "%-*@ = %@\n", width, key, prop );
		
		key = kHIDDeviceProperty_Name;
		prop = HIDDeviceCopyProperty( device, key, NULL, NULL );
		FPrintF( stdout, "%-*@ = %@\n", width, key, prop );
		
		key = kHIDDeviceProperty_ReportDescriptor;
		prop = HIDDeviceCopyProperty( device, key, NULL, NULL );
		FPrintF( stdout, "%-*@ = %@\n", width, key, prop );
		
		key = kHIDDeviceProperty_SampleRate;
		prop = HIDDeviceCopyProperty( device, key, NULL, NULL );
		FPrintF( stdout, "%-*@ = %@\n", width, key, prop );
		
		key = kHIDDeviceProperty_UUID;
		prop = HIDDeviceCopyProperty( device, key, NULL, NULL );
		FPrintF( stdout, "%-*@ = %@\n", width, key, prop );
	}
	if( count == 0 ) FPrintF( stdout, "No HID devices found\n" );
	
exit:
	CFReleaseNullSafe( devices );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_cmd_hid_browser_handler
//===========================================================================================================================

static void	_cmd_hid_browser_handler( HIDBrowserEventType inType, CFTypeRef inParam, void *inContext )
{
	HIDDeviceRef		device;
	OSStatus			err;
	CFTypeRef			value;
	
	(void) inContext;
	
	FPrintF( stderr, "HID browser event: %d (%s)\n", inType, HIDBrowserEventToString( inType ) );
	
	if( inType == kHIDBrowserEventAttached )
	{
		device = (HIDDeviceRef) inParam;
		
		value = HIDDeviceCopyProperty( device, kHIDDeviceProperty_UUID, NULL, &err );
		FPrintF( stderr, "    UUID: '%@'\n", value );
		CFReleaseNullSafe( value );
		
		value = HIDDeviceCopyProperty( device, kHIDDeviceProperty_Name, NULL, &err );
		FPrintF( stderr, "    Name: '%@'\n", value );
		CFReleaseNullSafe( value );
		
		value = HIDDeviceCopyProperty( device, kHIDDeviceProperty_ReportDescriptor, NULL, &err );
		FPrintF( stderr, "    Report Descriptor:\n%1@\n", value );
		CFReleaseNullSafe( value );
		
		HIDDeviceSetEventHandler( device, _cmd_hid_device_handler, device );
		err = HIDDeviceStart( device );
		if( err ) FPrintF( stderr, "### HID device start failed: %#m\n", err );
	}
	else if( inType == kHIDBrowserEventDetached )
	{
		device = (HIDDeviceRef) inParam;
		
		value = HIDDeviceCopyProperty( device, kHIDDeviceProperty_UUID, NULL, &err );
		FPrintF( stderr, "    UUID: '%@'\n", value );
		CFReleaseNullSafe( value );
		
		value = HIDDeviceCopyProperty( device, kHIDDeviceProperty_Name, NULL, &err );
		FPrintF( stderr, "    Name: '%@'\n", value );
		CFReleaseNullSafe( value );
	}
}

//===========================================================================================================================
//	_cmd_hid_device_handler
//===========================================================================================================================

static void
	_cmd_hid_device_handler( 
		HIDDeviceRef		inDevice, 
		HIDDeviceEventType	inType, 
		OSStatus			inStatus, 
		const uint8_t *		inPtr, 
		size_t				inLen, 
		void *				inContext )
{
	(void) inContext;
	
	if( inType == kHIDDeviceEventReport )
	{
		if( !inStatus )
		{
			FPrintF( stdout, "HID device report: %.3H\n", inPtr, (int) inLen, (int) inLen );
		}
		else
		{
			if( inStatus != kEndingErr ) FPrintF( stderr, "### HID device report error: %#m\n", inStatus );
			HIDDeviceStop( inDevice );
		}
	}
	else if( inType == kHIDDeviceEventStopped )
	{
		FPrintF( stderr, "HID device stopped\n" );
	}
	else
	{
		FPrintF( stderr, "HID device event: %d (%s)\n", inType, HIDDeviceEventToString( inType ) );
	}
}

//===========================================================================================================================
//	cmd_http
//===========================================================================================================================

static void	cmd_http( void )
{
	OSStatus			err;
	HTTPClientRef		client = NULL;
	dispatch_queue_t	queue;
	HTTPMessageRef		msg = NULL;
	size_t				n;
	const char *		valuePtr;
	size_t				valueLen;
	Boolean				decoded;
	CFTypeRef			obj;
	uint32_t			count;
	CFAbsoluteTime		t;
	
	if( !gHTTPAddress )
	{
		CLIHelpCommand( "http" );
		err = kNoErr;
		goto exit;
	}
	
	for( count = 1; ; ++count )
	{
		err = HTTPClientCreate( &client );
		require_noerr( err, exit );
		
		queue = dispatch_queue_create( "HTTP", 0 );
		require_action( queue, exit, err = kUnknownErr );
		HTTPClientSetDispatchQueue( client, queue );
		dispatch_release( queue );
	
		err = HTTPClientSetDestination( client, gHTTPAddress, kAirPlayFixedPort_RTSPControl );
		require_noerr( err, exit );
	
		err = HTTPMessageCreate( &msg );
		require_noerr( err, exit );
	
		HTTPHeader_InitRequest( &msg->header, gHTTPMethod, gHTTPURL, "HTTP/1.1" );
		HTTPHeader_SetField( &msg->header, kHTTPHeader_CSeq, "1" ); // Required by some servers.
		HTTPHeader_SetField( &msg->header, kHTTPHeader_UserAgent, kAirPlayUserAgentStr );
		if( stricmp( gHTTPMethod, "GET" ) != 0 ) HTTPHeader_SetField( &msg->header, kHTTPHeader_ContentLength, "0" );
		t = CFAbsoluteTimeGetCurrent();
		err = HTTPClientSendMessageSync( client, msg );
		require_noerr( err, exit );
		t = CFAbsoluteTimeGetCurrent() - t;
		
		decoded = false;
		if( gHTTPDecode )
		{
			valuePtr = NULL;
			valueLen = 0;
			HTTPGetHeaderField( msg->header.buf, msg->header.len, kHTTPHeader_ContentType, NULL, NULL, &valuePtr, &valueLen, NULL );
			if( MIMETypeIsPlist( valuePtr, valueLen ) )
			{
				obj = CFCreateWithPlistBytes( msg->bodyPtr, msg->bodyLen, 0, 0, NULL );
				if( obj )
				{
					FPrintF( stdout, "%@", obj );
					CFRelease( obj );
					decoded = true;
				}
			}
		}
		if( !decoded )
		{
			n = fwrite( msg->bodyPtr, 1, msg->bodyLen, stdout );
			err = map_global_value_errno( n == msg->bodyLen, n );
			require_noerr( err, exit );
		}
		ForgetCF( &msg );
		HTTPClientForget( &client );
		
		if( gVerbose && ( gHTTPRepeatMs >= 0 ) ) fprintf( stderr, "Cycle %u, %f ms\n", count, 1000 * t );
		
		if(      gHTTPRepeatMs > 0 ) usleep( gHTTPRepeatMs * 1000 );
		else if( gHTTPRepeatMs < 0 ) break;
	}
	
exit:
	CFReleaseNullSafe( msg );
	HTTPClientForget( &client );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	cmd_kill_all
//===========================================================================================================================

static void	cmd_kill_all( void )
{
	const char *		cmd;
	
#if  ( TARGET_OS_LINUX )
	cmd = "sudo /etc/init.d/airplay restart";
#else
	ErrQuit( 1, "error: not supported on this platform\n" );
	cmd = "";
#endif
	APSSystemf( "", cmd );
}

//===========================================================================================================================
//	cmd_logging
//===========================================================================================================================

static void	cmd_logging( void )
{
	OSStatus		err;
	
	err = DebugIPC_LogControl( ( gArgI < gArgC ) ? gArgV[ gArgI++ ] : NULL );
	if( err ) ErrQuit( EINVAL, "error: %#m\n", err );
}

//===========================================================================================================================
//	cmd_mfi
//===========================================================================================================================

static void	cmd_mfi( void )
{
	OSStatus		err;
	uint64_t		ticks;
	uint8_t *		ptr;
	size_t			len;
	uint8_t			digest[ 20 ];
	
	ticks = UpTicks();
	err = APSMFiPlatform_Initialize();
	ticks = UpTicks() - ticks;
	require_noerr( err, exit );
	FPrintF( stderr, "Init time: %llu ms\n", UpTicksToMilliseconds( ticks ) );
	
	ticks = UpTicks();
	err = APSMFiPlatform_CopyCertificate( &ptr, &len );
	ticks = UpTicks() - ticks;
	require_noerr( err, exit );
	FPrintF( stderr, "Certificate (%llu ms):\n%.2H\n", UpTicksToMilliseconds( ticks ), ptr, (int) len, (int) len );
	free( ptr );
	
	memset( digest, 0, sizeof( digest ) );
	ticks = UpTicks();
	err = APSMFiPlatform_CreateSignature( digest, sizeof( digest ), &ptr, &len );
	ticks = UpTicks() - ticks;
	require_noerr( err, exit );
	FPrintF( stderr, "Signature (%llu ms):\n%.2H\n", UpTicksToMilliseconds( ticks ), ptr, (int) len, (int) len );
	free( ptr );
	
exit:
	APSMFiPlatform_Finalize();
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	cmd_mirror_file
//===========================================================================================================================

static void	cmd_mirror_file( void )
{
	OSStatus			err;
	FILE *				sourceFile		= NULL;
	FILE *				sourceFilePtr;
	FILE *				outputFile		= NULL;
	FILE *				outputFilePtr	= NULL;
#if( TARGET_OS_LINUX || TARGET_OS_QNX )
	ScreenStreamRef		screenStream	= NULL;
#endif
	CFDataRef			avccData		= NULL;
	const uint8_t *		avccPtr;
	size_t				avccLen;
	uint8_t *			buffer			= NULL;
	uint8_t *			annexBHeaderPtr	= NULL;
	size_t				annexBHeaderLen	= 0;
	size_t				nalSize			= 0;
	size_t				maxLen, len, n, frameLen;
	const uint8_t *		src;
	const uint8_t *		end;
	const uint8_t *		nalPtr;
	size_t				nalLen;
	
	if( gMirrorFileDestination && ( strcmp( gMirrorFileDestination, "-" ) != 0 ) )
	{
			ErrQuit( 1, "error: remote destinations not supported on this platform\n" );
		err = kNoErr;
		goto exit;
	}
	
	if( strcmp( gMirrorFileSourcePath, "-" ) == 0 )
	{
		sourceFilePtr = stdin;
	}
	else
	{
		sourceFile = fopen( gMirrorFileSourcePath, "rb" );
		err = map_global_value_errno( sourceFile, sourceFile );
		require_noerr_quiet( err, exit );
		sourceFilePtr = sourceFile;
	}
	
	if( gMirrorFileAVCCPath )
	{
		avccData = CFDataCreateWithFilePath( gMirrorFileAVCCPath, &err );
		require_noerr_quiet( err, exit );
		
		avccPtr = CFDataGetBytePtr( avccData );
		avccLen = (size_t) CFDataGetLength( avccData );
		err = H264ConvertAVCCtoAnnexBHeader( avccPtr, avccLen, NULL, 0, &annexBHeaderLen, NULL, NULL );
		require_noerr( err, exit );
	
		annexBHeaderPtr = (uint8_t *) malloc( annexBHeaderLen );
		require_action( annexBHeaderPtr, exit, err = kNoMemoryErr );
		err = H264ConvertAVCCtoAnnexBHeader( avccPtr, avccLen, annexBHeaderPtr, annexBHeaderLen, &annexBHeaderLen, 
			&nalSize, NULL );
		require_noerr( err, exit );
	}
	
	if( gMirrorFileDestination && ( strcmp( gMirrorFileDestination, "-" ) == 0 ) )
	{
		#if( TARGET_OS_LINUX || TARGET_OS_QNX )
			err = ScreenStreamCreate( NULL, &screenStream );
			require_noerr( err, exit );
		
			err = ScreenStreamStart( screenStream );
			require_noerr( err, exit );
		
			if( avccData )
			{
				err = ScreenStreamSetProperty( screenStream, kScreenStreamProperty_AVCC, NULL, avccData );
				require_noerr( err, exit );
			}
			else
			{
				err = ScreenStreamSetProperty( screenStream, kScreenStreamProperty_SourceFormat, NULL, 
					kScreenStreamFormat_H264AnnexB );
				require_noerr( err, exit );
			}
		#else
			ErrQuit( 1, "error: local destinations not supported on this platform\n" );
			err = kNoErr;
			goto exit;
		#endif
	}
	if( gMirrorFileOutputPath )
	{
		if( strcmp( gMirrorFileOutputPath, "-" ) == 0 )
		{
			outputFilePtr = stdout;
		}
		else
		{	
			outputFile = fopen( gMirrorFileOutputPath, "wb" );
			err = map_global_value_errno( outputFile, outputFile );
			require_noerr_quiet( err, exit );
			outputFilePtr = outputFile;
			
			if( annexBHeaderLen > 0 )
			{
				n = fwrite( annexBHeaderPtr, 1, annexBHeaderLen, outputFilePtr );
				require_action( n == annexBHeaderLen, exit, err = kWriteErr );
			}
		}
	}
	
	maxLen = 64 * 1024;
	buffer = (uint8_t *) malloc( maxLen );
	require_action( buffer, exit, err = kNoMemoryErr );
	
	for( ;; )
	{
		if( avccData )
		{
			n = fread( buffer, 1, nalSize, sourceFilePtr );
			if( n == 0 ) break;
			require_action( n == nalSize, exit, err = kReadErr );
			if(      n == 1 ) len = Read8( buffer );
			else if( n == 2 ) len = ReadBig16( buffer );
			else if( n == 4 ) len = ReadBig32( buffer );
			else { dlogassert( "Bad NAL size: %zu", nalSize ); err = kSizeErr; goto exit; }
			
			frameLen = nalSize + len;
			if( frameLen > maxLen )
			{
				uint8_t *		ptr;
			
				maxLen = frameLen;
				ptr = (uint8_t *) realloc( buffer, maxLen );
				require_action( ptr, exit, err = kNoMemoryErr );
				buffer = ptr;
			}
			
			n = fread( &buffer[ nalSize ], 1, len, sourceFilePtr );
			require_action( n == len, exit, err = kReadErr );
		}
		else
		{
			n = fread( buffer, 1, maxLen, sourceFilePtr );
			if( n == 0 ) break;
			frameLen = n;
		}
				
		#if( TARGET_OS_LINUX || TARGET_OS_QNX )
		if( screenStream )
		{
			err = ScreenStreamProcessData( screenStream, buffer, frameLen, UpTicks(), NULL, NULL, NULL );
			require_noerr( err, exit );
		}
		#endif
		
		if( outputFilePtr && avccData )
		{
			src = buffer;
			end = src + frameLen;
			while( H264GetNextNALUnit( src, end, nalSize, &nalPtr, &nalLen, &src ) == kNoErr )
			{
				n = fwrite( "\x00\x00\x00\x01", 1, 4, outputFilePtr );
				require_action( n == 4, exit, err = kWriteErr );
			
				n = fwrite( nalPtr, 1, nalLen, outputFilePtr );
				require_action( n == nalLen, exit, err = kWriteErr );
			}
		}
	}
	err = kNoErr;
	
exit:
	ForgetMem( &buffer );
	ForgetMem( &annexBHeaderPtr );
	ForgetCF( &avccData );
#if( TARGET_OS_LINUX || TARGET_OS_QNX )
	ScreenStreamForget( &screenStream );
#endif
	ForgetANSIFile( &outputFile );
	ForgetANSIFile( &sourceFile );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	cmd_moved_to_cuutil
//===========================================================================================================================

static void	cmd_moved_to_cuutil( void )
{
	fprintf( stdout, "%s\n", kMovedToCUutil );
}

//===========================================================================================================================
//	cmd_netperf
//===========================================================================================================================

static void	_cmd_netperf_event_handler( uint32_t inType, CFDictionaryRef inDetails, void *inContext );

static void	cmd_netperf( void )
{
	OSStatus					err;
	NetPerfRef					netPerf = NULL;
	CFMutableDictionaryRef		config  = NULL;
	
	if( !gNetPerfAddress )
	{
		CLIHelpCommand( "netperf" );
		err = kNoErr;
		goto exit;
	}
	
	// Build the config.
	
	config = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( config, exit, err = kNoMemoryErr );
	
	CFDictionarySetCString( config, kNetPerfKey_Destination, gNetPerfAddress, kSizeCString );
	
	// Start the client.
	
	err = NetPerfCreate( &netPerf );
	require_noerr( err, exit );
	
	NetPerfSetEventHandler( netPerf, _cmd_netperf_event_handler, NULL, NULL );
	
	err = NetPerfControlSync( netPerf, kNetPerfCommand_StartClientSession, config, NULL );
	require_noerr( err, exit );
	
	CFRunLoopRun();
	
exit:
	if( netPerf )
	{
		NetPerfControlSync( netPerf, kNetPerfCommand_StopClientSession, NULL, NULL );
		CFRelease( netPerf );
	}
	if( config )	CFRelease( config );
	if( err )		ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_cmd_netperf_event_handler
//===========================================================================================================================

static void	_cmd_netperf_event_handler( uint32_t inType, CFDictionaryRef inDetails, void *inContext )
{
	(void) inDetails;
	(void) inContext;
	
	switch( inType )
	{
		default:
			break;
	}
}

//===========================================================================================================================
//	cmd_ntp
//===========================================================================================================================

static void	cmd_ntp( void )
{
	OSStatus		err;
	NTPClockRef		ntpClock = NULL;
	sockaddr_ip		sip;
	
	err = NTPClockCreate( &ntpClock );
	require_noerr( err, exit );
	
	NTPClockSetP2P( ntpClock, true );
	NTPClockSetRTCP( ntpClock, gNTPUseRTCP ? true : false );
	
	if( gNTPAddress )
	{
		err = StringToSockAddr( gNTPAddress, &sip, sizeof( sip ), NULL );
		require_noerr_action_quiet( err, exit, ErrQuit( 1, "error: bad address '%s'\n", gNTPAddress ); err = kNoErr );
		NTPClockSetPeer( ntpClock, &sip, gNTPPort );
		
		err = NTPClockStartClient( ntpClock );
		require_noerr_quiet( err, exit );
	}
	else
	{
		NTPClockSetListenPort( ntpClock, ( gNTPPort > 0 ) ? gNTPPort : 7010 );
		
		err = NTPClockStartServer( ntpClock );
		require_noerr( err, exit );
		
		FPrintF( stderr, "%N: Running NTP server on port %d", NTPClockGetListenPort( ntpClock ) );
		if( gNTPTimeoutSecs > 0 )	FPrintF( stderr, " for %d seconds\n", gNTPTimeoutSecs );
		else						FPrintF( stderr, "\n" );
	}
	
	{
		CFRunLoopRun();
	}
	
exit:
	NTPClockForget( &ntpClock );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

#if 0
#pragma mark -
#endif

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	cmd_prefs
//===========================================================================================================================

static void	cmd_prefs( void )
{
	CFMutableDictionaryRef		prefs = NULL;			
	const char *				cmd;
	const char *				arg;
	CFArrayRef					keys;
	CFIndex						i, n;
	CFStringRef					key = NULL, key2;
	CFTypeRef					value;
	OSStatus					err;
	CFMutableArrayRef			sortedKeys;
	int							width, widest;
	int							color;
	
	cmd = ( gArgI < gArgC ) ? gArgV[ gArgI++ ] : "";
	
	// Read
	
	if( ( *cmd == '\0' ) || strcmp( cmd, "read" ) == 0 )
	{
		if( gPrefsFilePath )
		{
			prefs = (CFMutableDictionaryRef) CFPropertyListCreateFromFilePath( gPrefsFilePath, 
				kCFPropertyListMutableContainers, NULL );
			if( prefs && !CFIsType( prefs, CFDictionary ) )
			{
				ErrQuit( 1, "error: Prefs file is not a dictionary\n" );
				err = kNoErr;
				goto exit;
			}
		}
		
		color = isatty( fileno( stdout ) );
		if( gArgI >= gArgC ) // No key specified so print all keys.
		{
			if( prefs )	keys = CFDictionaryCopyKeys( prefs, &err );
			else		keys = AirPlaySettings_CopyKeys( &err );
			require_noerr( err, exit );
			
			sortedKeys = CFArrayCreateMutableCopy( NULL, 0, keys );
			CFRelease( keys );
			require_action( sortedKeys, exit, err = kNoMemoryErr );
			CFArraySortValues( sortedKeys, CFRangeMake( 0, CFArrayGetCount( sortedKeys ) ), CFSortLocalizedStandardCompare, NULL );
			
			widest = 0;
			n = CFArrayGetCount( sortedKeys );
			for( i = 0; i < n; ++i )
			{
				key2 = (CFStringRef) CFArrayGetCFStringAtIndex( sortedKeys, i, NULL );
				width = (int) CFStringGetLength( key2 );
				if( width > widest ) widest = width;
			}
			
			n = CFArrayGetCount( sortedKeys );
			for( i = 0; i < n; ++i )
			{
				key2 = (CFStringRef) CFArrayGetCFStringAtIndex( sortedKeys, i, NULL );
				if( !key2 ) continue;
				
				if( prefs )	value = CFDictionaryGetValue( prefs, key2 );
				else		value = AirPlaySettings_CopyValue( NULL, key2, NULL );
				if( !value ) continue;
				
				FPrintF( stdout, "%-*@ : %s%@%s\n", widest, key2, color ? kANSIMagenta : "", value, color ? kANSINormal : "" );
				if( !prefs ) CFRelease( value );
			}
			if( n == 0 ) fprintf( stderr, "### No prefs found\n" );
			CFRelease( sortedKeys );
		}
		while( gArgI < gArgC )
		{
			arg = gArgV[ gArgI++ ];
			key = CFStringCreateWithCString( NULL, arg, kCFStringEncodingUTF8 );
			require_action( key, exit, err = kNoMemoryErr );
			
			if( prefs )	value = CFDictionaryGetValue( prefs, key );
			else		value = AirPlaySettings_CopyValue( NULL, key, NULL );
			CFRelease( key );
			key = NULL;
			if( !value ) { FPrintF( stderr, "error: Key '%s' does not exist.\n", arg ); continue; }
			
			FPrintF( stdout, "%@\n", value );
			if( !prefs ) CFRelease( value );
		}
	}
	
	// Write
	
	else if( strcmp( cmd, "write" ) == 0 )
	{
		if( gArgI >= gArgC ) { ErrQuit( 1, "error: No key specified\n" ); err = kNoErr; goto exit; }
		arg = gArgV[ gArgI++ ];
		key = CFStringCreateWithCString( NULL, arg, kCFStringEncodingUTF8 );
		require_action( key, exit, err = kNoMemoryErr );
		
		if( gArgI >= gArgC ) { ErrQuit( 1, "error: No value specified\n" ); err = kNoErr; goto exit; }
		arg = gArgV[ gArgI++ ];
		
		if( gPrefsFilePath )
		{
			prefs = (CFMutableDictionaryRef) CFPropertyListCreateFromFilePath( gPrefsFilePath, 
				kCFPropertyListMutableContainers, NULL );
			if( prefs && !CFIsType( prefs, CFDictionary ) )
			{
				ErrQuit( 1, "error: Prefs file is not a dictionary.\n" );
				err = kNoErr;
				goto exit;
			}
			if( !prefs )
			{
				prefs = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
				require_action( prefs, exit, err = kNoMemoryErr );
			}
			
			err = CFDictionarySetCString( prefs, key, arg, kSizeCString );
			require_noerr( err, exit );
			
			err = CFPropertyListWriteToFilePath( prefs, "binary1", gPrefsFilePath );
			require_noerr( err, exit );
		}
		else
		{
			{
			
				err = AirPlaySettings_SetCString( key, arg, kSizeCString );
				require_noerr( err, exit );
			
				AirPlaySettings_Synchronize();
			}
		}
	}
	
	// Delete
	
	else if( strcmp( cmd, "delete" ) == 0 )
	{
		if( gPrefsFilePath )
		{
			prefs = (CFMutableDictionaryRef) CFPropertyListCreateFromFilePath( gPrefsFilePath, 
				kCFPropertyListMutableContainers, NULL );
			if( prefs && !CFIsType( prefs, CFDictionary ) )
			{
				ErrQuit( 1, "error: Prefs file is not a dictionary\n" );
				err = kNoErr;
				goto exit;
			}
		}
		
		if( gArgI >= gArgC ) // No key specified so delete all keys.
		{
			if( prefs )	keys = CFDictionaryCopyKeys( prefs, &err );
			else		keys = AirPlaySettings_CopyKeys( &err );
			require_noerr( err, exit );
			
			n = CFArrayGetCount( keys );
			for( i = 0; i < n; ++i )
			{
				key2 = (CFStringRef) CFArrayGetCFStringAtIndex( keys, i, NULL );
				if( !key2 ) continue;
				
				if( prefs )
				{
					CFDictionaryRemoveValue( prefs, key2 );
				}
				else
				{
					err = AirPlaySettings_RemoveValue( key2 );
					if( err ) FPrintF( stderr, "### Remove pref '%@' failed: %#m\n", key2, err );
				}
			}
			CFRelease( keys );
			err = kNoErr;
		}
		while( gArgI < gArgC )
		{
			arg = gArgV[ gArgI++ ];
			key = CFStringCreateWithCString( NULL, arg, kCFStringEncodingUTF8 );
			require_action( key, exit, err = kNoMemoryErr );
			
			if( prefs )
			{
				CFDictionaryRemoveValue( prefs, key );
			}
			else
			{
				err = AirPlaySettings_RemoveValue( key );
				if( err ) FPrintF( stderr, "### Remove pref '%s' failed: %#m\n", arg, err );
			}
			CFRelease( key );
			key = NULL;
		}
		
		if( gPrefsFilePath )
		{
			if( prefs )
			{
				err = CFPropertyListWriteToFilePath( prefs, "binary1", gPrefsFilePath );
				require_noerr( err, exit );
			}
		}
		else
		{
			AirPlaySettings_Synchronize();
		}
	}
	else
	{
		ErrQuit( 1, "error: Bad command '%s'. Must be 'read', 'write', or 'delete'.\n", cmd );
	}
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( key );
	CFReleaseNullSafe( prefs );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	cmd_show
//===========================================================================================================================

static void	cmd_show( void )
{
	const char *		command;
	
	command = gShowCommand ? gShowCommand : "show";
	
{
	OSStatus		err;
	
	err = DebugIPC_PerformF( NULL, NULL,
		"{"
			"%kO=%s"
			"%kO=%s"
			"%kO=%i"
		"}", 
		kDebugIPCKey_Command,	command, 
		kDebugIPCKey_Value,		( gArgI < gArgC ) ? gArgV[ gArgI++ ] : NULL, 
		CFSTR( "verbose" ),		gVerbose );
	if( err ) ErrQuit( EINVAL, "error: %#m\n", err );
}
}

//===========================================================================================================================
//	cmd_test
//===========================================================================================================================

static void	cmd_test( void )
{
	OSStatus						err;
	uint8_t *						buf;
	SocketRef						sock;
	NetSocketRef					netSock;
	char							url[ 256 ];
	uint64_t						fpsDelayTicks, lastFrameTicks;
	uint64_t						nowTicks, intervalTicks, lastProgressTicks, endTicks;
	size_t							size;
	HTTPHeader						header;
	iovec_t							iov[ 2 ];
	int								frameNum;
	uint64_t						transactionTicks;
	size_t							transactionBytes;
	double							mbps, tcpMbps;
	EWMA_FP_Data					mbpsAvg;
#if( defined( TCP_MEASURE_SND_BW ) )
	int								option;
	struct tcp_measure_bw_burst		burstInfo;
	struct tcp_info					tcpInfo;
	socklen_t						len;
#endif
	
	buf = NULL;
	if( gTestMinSize > gTestMaxSize )				ErrQuit( 1, "error: min-size must be >= max-size\n" );
	if( gTestMaxSize <= 0 )							ErrQuit( 1, "error: max-size must be >= 0\n" );
	if( gTestFPS <= 0 )								ErrQuit( 1, "error: fps must be >= 0\n" );
	if( gTestMinPause > gTestMaxPause )				ErrQuit( 1, "error: min-pause must be >= max-pause\n" );
	if( gTestMinPauseDelay > gTestMaxPauseDelay )	ErrQuit( 1, "error: min-pause-delay must be >= max-pause-delay\n" );
	
	fprintf( stderr, "Network test to %s, %d-%d bytes, %d fps, %d-%d ms pauses, %d-%d ms pause delay\n", 
		gTestAddress, gTestMinSize, gTestMaxSize, gTestFPS, gTestMinPause, gTestMaxPause, 
		gTestMinPauseDelay, gTestMaxPauseDelay );
	
	fpsDelayTicks = UpTicksPerSecond() / gTestFPS;
	
	buf = (uint8_t *) calloc( 1, (size_t) gTestMaxSize );
	require_action( buf, exit, err = kNoMemoryErr; ErrQuit( 1, "error: no memory for %d max-size\n", gTestMaxSize ) );
	
	err = AsyncConnection_ConnectSync( gTestAddress, kAirPlayPort_MediaControl, kAsyncConnectionFlag_P2P, 0, 
		kSocketBufferSize_DontSet, kSocketBufferSize_DontSet, NULL, NULL, &sock );
	require_noerr_action_quiet( err, exit, ErrQuit( 1, "error: connected to %s failed: %#m\n", gTestAddress, err ) );
	
	err = NetSocket_CreateWithNative( &netSock, sock );
	require_noerr( err, exit );
	
	// Setup measurements.
	
#if( defined( TCP_MEASURE_SND_BW ) )
	option = 1;
	err = setsockopt( netSock->nativeSock, IPPROTO_TCP, TCP_MEASURE_SND_BW, &option, (socklen_t) sizeof( option ) );
	if( err ) FPrintF( stderr, "### TCP_MEASURE_SND_BW failed: %#m\n", err );
	
	burstInfo.min_burst_size = (uint32_t) gTestMinMeasureBurst;
	burstInfo.max_burst_size = (uint32_t) gTestMaxMeasureBurst;
	err = setsockopt( netSock->nativeSock, IPPROTO_TCP, TCP_MEASURE_BW_BURST, &burstInfo, (socklen_t) sizeof( burstInfo ) );
	err = map_socket_noerr_errno( nativeSock, err );
	if( err ) FPrintF( stderr, "### TCP_MEASURE_BW_BURST failed: %#m\n", err );
	
	memset( &tcpInfo, 0, sizeof( tcpInfo ) );
#endif
	
	// Init stats.
	
	EWMA_FP_Init( &mbpsAvg, 0.1, kEWMAFlags_StartWithFirstValue );
	
	nowTicks = UpTicks();
	intervalTicks = ( gTestReportPeriodMs > 0 ) ? MillisecondsToUpTicks( gTestReportPeriodMs ) : 0;
	endTicks = ( gTestTimeoutSecs >= 0 ) ? ( nowTicks + SecondsToUpTicks( gTestTimeoutSecs ) ) : kUpTicksForever;
	
	lastFrameTicks = nowTicks;
	lastProgressTicks = nowTicks;
	
	// Main loop.
	
	for( frameNum = 1; ; ++frameNum )
	{
		size = RandomRange( gTestMinSize, gTestMaxSize );
		snprintf( url, sizeof( url ), "http://%s/test/%d", gTestAddress, (int) size );
		HTTPHeader_InitRequest( &header, "PUT", url, "HTTP/1.1" );
		HTTPHeader_SetField( &header, "Content-Length", "%zu", size );
		err = HTTPHeader_Commit( &header );
		require_noerr( err, exit );
		
		iov[ 0 ].iov_base = header.buf;
		iov[ 0 ].iov_len  = header.len;
		iov[ 1 ].iov_base = (char *) buf;
		iov[ 1 ].iov_len  = size;
		transactionBytes = header.len + size;
		transactionTicks = UpTicks();
		err = NetSocket_WriteV( netSock, iov, 2, -1 );
		require_noerr_action( err, exit, ErrQuit( 1, "error: write of %zu bytes failed: %#m\n", size, err ) );
		
		err = NetSocket_HTTPReadHeader( netSock, &header, -1 );
		require_noerr_action( err, exit, ErrQuit( 1, "error: read failed: %#m\n", err ) );
		require_action( IsHTTPStatusCode_Success( header.statusCode ), exit, 
			ErrQuit( 1, "error: request failed: %d\n", header.statusCode ) );
		
		// Measure bandwidth.
		
		nowTicks = UpTicks();
		transactionTicks = nowTicks - transactionTicks;
		mbps = ( 8 * ( (double) transactionBytes ) / ( ( (double) transactionTicks ) / ( (double) UpTicksPerSecond() ) ) / 1000000 );
		EWMA_FP_Update( &mbpsAvg, mbps );
		
		#if( defined( TCP_MEASURE_SND_BW ) )
			len = (socklen_t) sizeof( tcpInfo );
			getsockopt( netSock->nativeSock, IPPROTO_TCP, TCP_INFO, &tcpInfo, &len );
			tcpMbps = ( (double) tcpInfo.tcpi_snd_bw ) / 1000000.0;
		#else
			tcpMbps = 0;
		#endif
		
		if( intervalTicks == 0 )
		{
			fprintf( stderr, "%4d: Bytes: %6zu  Mbps: %.3f  Avg Mbps: %.3f  TCP Mbps: %0.3f\n", 
				frameNum, size, mbps, EWMA_FP_Get( &mbpsAvg ), tcpMbps );
		}
		else if( ( nowTicks - lastProgressTicks ) >= intervalTicks )
		{
			lastProgressTicks = nowTicks;
			fprintf( stderr, "Avg Mbps: %.3f  TCP Mbps: %.3f\n", EWMA_FP_Get( &mbpsAvg ), tcpMbps );
		}
		if( ( gTestTimeoutSecs >= 0 ) && ( nowTicks >= endTicks ) )
		{
			break;
		}
		
		lastFrameTicks += fpsDelayTicks;
		SleepUntilUpTicks( lastFrameTicks );
	}
	
exit:
	if( buf ) free( buf );
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

#if 0
#pragma mark -
#endif

