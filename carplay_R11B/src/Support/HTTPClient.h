/*
	File:    	HTTPClient.h
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
	
	Copyright (C) 2011-2014 Apple Inc. All Rights Reserved.
*/

#ifndef	__HTTPClient_h__
#define	__HTTPClient_h__

#include "APSCommonServices.h"
#include "HTTPMessage.h"

#include LIBDISPATCH_HEADER

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientCreate
	@abstract	Creates a new HTTP client.
*/
typedef struct HTTPClientPrivate *		HTTPClientRef;

CFTypeID	HTTPClientGetTypeID( void );
OSStatus	HTTPClientCreate( HTTPClientRef *outClient );
OSStatus	HTTPClientCreateWithSocket( HTTPClientRef *outClient, SocketRef inSock );
#define 	HTTPClientForget( X ) do { if( *(X) ) { HTTPClientInvalidate( *(X) ); CFRelease( *(X) ); *(X) = NULL; } } while( 0 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientInvalidate
	@abstract	Cancels all outstanding operations.
*/
void	HTTPClientInvalidate( HTTPClientRef inClient );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientGetPeerAddress
	@abstract	Gets the address of the connected peer.
	@discussion	Only valid after a connection has been established.
*/
OSStatus	HTTPClientGetPeerAddress( HTTPClientRef inClient, void *inSockAddr, size_t inMaxLen, size_t *outLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSetDestination
	@abstract	Sets the destination hostname, IP address, URL, etc. of the HTTP server to talk to.
	@discussion	Note: this cannot be changed once set.
*/
OSStatus	HTTPClientSetDestination( HTTPClientRef inClient, const char *inDestination, int inDefaultPort );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSetDispatchQueue
	@abstract	Sets the GCD queue to perform all operations on.
	@discussion	Note: this cannot be changed once operations have started.
*/
void	HTTPClientSetDispatchQueue( HTTPClientRef inClient, dispatch_queue_t inQueue );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSetFlags
	@abstract	Enables or disables P2P connections.
*/
typedef uint32_t		HTTPClientFlags;
#define kHTTPClientFlag_None					0
#define kHTTPClientFlag_P2P						( 1 << 0 ) // Enable P2P connections.
#define kHTTPClientFlag_SuppressUnusable		( 1 << 1 ) // Suppress trying to connect on seemingly unusable interfaces.
#define kHTTPClientFlag_Reachability			( 1 << 2 ) // Use the reachability APIs before trying to connect.
#define kHTTPClientFlag_BoundInterface			( 1 << 3 ) // Set bound interface before connect if interface index available.

void	HTTPClientSetFlags( HTTPClientRef inClient, HTTPClientFlags inFlags, HTTPClientFlags inMask );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSetLogging
	@abstract	Sets the log category to use for HTTP message logging.
*/
void	HTTPClientSetLogging( HTTPClientRef inClient, LogCategory *inLogCategory );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSetP2P
	@abstract	Enables or disables P2P connections.
*/
void	HTTPClientSetP2P( HTTPClientRef inClient, Boolean inAllowP2P );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	HTTPClientSendMessage
	@abstract	Sends an HTTP message.
*/
OSStatus	HTTPClientSendMessage( HTTPClientRef inClient, HTTPMessageRef inMsg );
OSStatus	HTTPClientSendMessageSync( HTTPClientRef inClient, HTTPMessageRef inMsg );

#ifdef __cplusplus
}
#endif

#endif // __HTTPClient_h__
