/*
	File:    	HTTPNetUtils.c
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
	
	Copyright (C) 2008-2014 Apple Inc. All Rights Reserved.
*/

#include <errno.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "HTTPUtils.h"
#include "NetUtils.h"
#include "StringUtils.h"

#include "HTTPNetUtils.h"

//===========================================================================================================================
//	HTTPDownloadFile
//===========================================================================================================================

OSStatus	HTTPDownloadFile( const char *inURL, size_t inMaxSize, char **outData, size_t *outSize )
{
	OSStatus			err;
	SocketRef			sock;
	const char *		p;
	const char *		q;
	char *				host;
	size_t				hostSize;
	HTTPHeader *		header;
	int					n;
	fd_set				set;
	iovec_t				iov;
	iovec_t *			iop;
	int					ion;
	struct timeval		timeout;
	char *				bodyBuffer;
	size_t				bodySize;
	size_t				bodyOffset;
	
	host		= NULL;
	sock		= kInvalidSocketRef;
	header		= NULL;
	bodyBuffer	= NULL;
	
	// Parse the host out of the URL and make a null-terminated string out of it.
	
	require_action( strnicmp( inURL, "http://", sizeof_string( "http://" ) ) == 0, exit, err = kParamErr );
	p = inURL + sizeof_string( "http://" );
	q = strchr( p, '/' );
	if( q ) hostSize = (size_t)( q - p );
	else	hostSize = strlen( p );
	
	host = (char *) malloc( hostSize + 1 );
	require_action( host, exit, err = kNoMemoryErr );
	memcpy( host, p, hostSize );
	host[ hostSize ] = '\0';
	
	// Connect to the server and send the request.
	
	err = TCPConnect( host, "80", 5, &sock );
	require_noerr_quiet( err, exit );
	
	header = (HTTPHeader *) malloc( sizeof( *header ) );
	require_action( header, exit, err = kNoMemoryErr );
	
	HTTPHeader_InitRequest( header, "GET", inURL, "HTTP/1.1" );
	HTTPHeader_SetField( header, "Host", host );
	HTTPHeader_SetField( header, "Connection", "close" );
	err = HTTPHeader_Commit( header );
	require_noerr( err, exit );
	
	dlog( kLogLevelVerbose, "========== HTTP REQUEST header ==========\n" );
	dlog( kLogLevelVerbose, "%.*s", (int) header->len, header->buf );
	
	FD_ZERO( &set );
	
	iov.iov_base = header->buf;
	iov.iov_len  = (unsigned int) header->len;
	iop = &iov;
	ion = 1;
	for( ;; )
	{
		err = SocketWriteData( sock, &iop, &ion );
		if( err == kNoErr )			break;
		if( err == EWOULDBLOCK )	err = kNoErr;
		if( err == EPIPE )			goto exit;
		require_noerr( err, exit );
		
		FD_SET( sock, &set );
		timeout.tv_sec  = 5;
		timeout.tv_usec = 0;
		n = select( (int)( sock + 1 ), NULL, &set, NULL, &timeout );
		require_action_quiet( n != 0, exit, err = kTimeoutErr );
		err = map_global_value_errno( n == 1, n );
		require_noerr( err, exit );
		require_action( FD_ISSET( sock, &set ), exit, err = kUnknownErr );
	}
	
	// Read the response header.
	
	header->len = 0;
	for( ;; )
	{
		err = HTTPReadHeader( sock, header->buf, sizeof( header->buf ), &header->len );
		if( err == kNoErr )			break;
		if( err == EWOULDBLOCK )	err = kNoErr;
		if( err == kConnectionErr )	goto exit;
		require_noerr( err, exit );
		
		FD_SET( sock, &set );
		timeout.tv_sec  = 5;
		timeout.tv_usec = 0;
		n = select( (int)( sock + 1 ), &set, NULL, NULL, &timeout );
		require_action_quiet( n != 0, exit, err = kTimeoutErr );
		err = map_global_value_errno( n == 1, n );
		require_noerr( err, exit );
		require_action( FD_ISSET( sock, &set ), exit, err = kUnknownErr );
	}
	
	dlog( kLogLevelVerbose, "========== HTTP RESPONSE header ==========\n" );
	dlog( kLogLevelVerbose, "%.*s", (int) header->len, header->buf );
	
	err = HTTPHeader_Parse( header );
	require_noerr( err, exit );
	if( header->statusCode == 0 ) header->statusCode = kUnknownErr;
	require_action_quiet( header->statusCode == 200, exit, err = header->statusCode );
	
	n = HTTPScanFHeaderValue( header->buf, header->len, "Content-Length", "%zu", &bodySize );
	if( n <= 0 ) bodySize = 0;
	require_action( ( inMaxSize == 0 ) || ( bodySize < inMaxSize ), exit, err = kSizeErr );
	
	// Read the response body.
	
	bodyBuffer = (char *) malloc( bodySize + 1 );
	require_action( bodyBuffer, exit, err = kNoMemoryErr );
	
	bodyOffset = 0;
	for( ;; )
	{
		err = SocketReadData( sock, bodyBuffer, bodySize, &bodyOffset );
		if( err == kNoErr )			break;
		if( err == EWOULDBLOCK )	err = kNoErr;
		if( err == kConnectionErr )	goto exit;
		require_noerr( err, exit );
		
		FD_SET( sock, &set );
		timeout.tv_sec  = 5;
		timeout.tv_usec = 0;
		n = select( (int)( sock + 1 ), &set, NULL, NULL, &timeout );
		require_action_quiet( n != 0, exit, err = kTimeoutErr );
		err = map_global_value_errno( n == 1, n );
		require_noerr( err, exit );
		require_action( FD_ISSET( sock, &set ), exit, err = kUnknownErr );
	}
	check( bodyOffset == bodySize );
	bodyBuffer[ bodyOffset ] = '\0';
	
	dlog( kLogLevelVerbose, "========== HTTP RESPONSE body ==========\n" );
	dlog( kLogLevelVerbose, "%.*s\n", 8192, bodyBuffer );
	
	// Success!
	
	err = SocketCloseGracefully( sock, 1 );
	check_noerr( err );
	sock = kInvalidSocketRef;
	
	*outData = bodyBuffer;
	bodyBuffer = NULL;
	*outSize = bodySize;
	err = kNoErr;
	
exit:
	ForgetMem( &bodyBuffer );
	ForgetMem( &header );
	ForgetSocket( &sock );
	ForgetMem( &host );
	return( err );
}

//===========================================================================================================================
//	HTTPReadHeader
//===========================================================================================================================

OSStatus	HTTPReadHeader( SocketRef inSock, void *inBuffer, size_t inBufferSize, size_t *ioOffset )
{
	OSStatus		err;
	size_t			offset;
	size_t			len;
	char *			buf;
	char *			dst;
	ssize_t			n;
	
	// Make sure there is room for a null terminator and don't try to read more than the buffer can hold.
	
	require_action( inBufferSize > 0, exit, err = kSizeErr );
	inBufferSize -= 1;
	
	offset = *ioOffset;
	require_action_quiet( offset < inBufferSize, exit, err = kNoSpaceErr );
	
	// Peek at the available bytes so we only read the header and leave the body for other code to read. This 
	// isn't super efficient because it requires two system calls/copies (one to peek and one to consume), but 
	// if we consumed a buffer of data, we'd most likely get data beyond the header and we'd need to track that 
	// leftover data (especially with pipelining). That's a performance tradeoff this code is willing to make.
	
	buf = (char *) inBuffer;
	dst = buf + offset;
	len = inBufferSize - offset;
	do
	{
		n = recv( inSock, dst, (int) len, MSG_PEEK );
		err = map_socket_value_errno( inSock, n >= 0, n );
		
	}	while( err == EINTR );
	if( n > 0 )
	{
		int			found;
		char *		end;
		
		// Check for interleaved binary data (4 byte header that begins with $). See RFC 2326 section 10.12.
		
		end = dst + n;
		if( ( buf[ 0 ] == '$' ) && ( ( end - buf ) >= 4 ) )
		{
			end = buf + 4;
			found = 1;
		}
		else
		{
			char *		ptr;
			
			// Find an empty line (separates the header and body). The HTTP spec defines it as CRLFCRLF, but some
			// use LFLF or weird combos like CRLFLF so this handles CRLFCRLF, LFLF, and CRLFLF (but not CRCR).
			
			*end = '\0';
			if( offset > 2 ) ptr = buf + ( offset - 2 );	// Back up 2 in case we only got CRLFCR of CRLFCRLF last time.
			else			 ptr = buf;						// 2 or fewer bytes total so far so start at the beginning.
			for( ; ; ++ptr )
			{
				char		c;
				
				// Find an LF since all the EOLs we handle ultimately end in LF.
				
				while( ( ( c = *ptr ) != '\0' ) && ( c != '\n' ) ) ++ptr;
				if( c == '\0' )
				{
					found = 0;
					break;
				}
				
				c = ptr[ 1 ];
				if( ( c == '\r' ) && ( ptr[ 2 ] == '\n' ) ) // CRLF followed by LF (CRLFCRLF or LFCRLF)
				{
					end = ptr + 3;
					found = 1;
					break;
				}
				else if( c == '\n' ) // LF followed by LF (LFLF or CRLFLF)
				{
					end = ptr + 2;
					found = 1;
					break;
				}
			}
		}
		
		// Consume the bytes we've searched, even if we didn't find anything, so we're ready for the next time.
		// Note: this always consumes at least 1 byte to handle the case where a partial header was sent and 
		// then we got a disconnect because we can't get the EOF from recv until we consume all the data.
		
		len = (size_t)( end - dst );
		check( len > 0 );
		do
		{
			n = recv( inSock, dst, (int) len, 0 );
			err = map_socket_value_errno( inSock, n >= 0, n );
			
		}	while( err == EINTR );
		check_noerr( err );
		dst[ len ] = '\0';
		
		// Update the offset to indicate the number of bytes read so far and where the next read would go.
		
		*ioOffset = offset + len;
		err = found ? kNoErr : EWOULDBLOCK;
	}
	else if( n == 0 )
	{
		err = kConnectionErr;
	}
	else if( err != EWOULDBLOCK )
	{
		dlog( kLogLevelError, "[%s] %###s: ### recv failed: %#m\n", __PROGRAM__, __ROUTINE__, err );
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	NetSocket_HTTPReadHeader
//===========================================================================================================================

OSStatus	NetSocket_HTTPReadHeader( NetSocketRef inSock, HTTPHeader *inHeader, int32_t inTimeoutSecs )
{
	OSStatus		err;
	size_t			len;
	char *			buf;
	char *			dst;
	char *			lim;
	char *			src;
	char *			end;
	size_t			rem;
	
	buf = inHeader->buf;
	dst = buf;
	lim = buf + sizeof( inHeader->buf );
	src = buf;
	for( ;; )
	{
		len = (size_t)( lim - dst );
		require_action( len > 0, exit, err = kNoSpaceErr );
		
		err = NetSocket_Read( inSock, 1, len, dst, &len, inTimeoutSecs );
		if( err ) goto exit;
		end = dst + len;
		dst = end;
		
		// Check for interleaved binary data (4 byte header that begins with $). See RFC 2326 section 10.12.
		
		if( ( buf[ 0 ] == '$' ) && ( ( end - buf ) >= 4 ) )
		{
			end = buf + 4;
			goto foundHeader;
		}
		
		// Find an empty line (separates the header and body). The HTTP spec defines it as CRLFCRLF, but some
		// use LFLF or weird combos like CRLFLF so this handles CRLFCRLF, LFLF, and CRLFLF (but not CRCR).
		
		for( ;; )
		{
			while( ( src < end ) && ( *src != '\n' ) ) ++src;
			if( src >= end ) break;
			
			rem = (size_t)( end - src );
			if( ( rem >= 3 ) && ( src[ 1 ] == '\r' ) && ( src[ 2 ] == '\n' ) ) // CRLFCRLF or LFCRLF.
			{
				end = src + 3;
				goto foundHeader;
			}
			else if( ( rem >= 2 ) && ( src[ 1 ] == '\n' ) ) // LFLF or CRLFLF.
			{
				end = src + 2;
				goto foundHeader;
			}
			else if( rem <= 1 )
			{
				break;
			}
			++src;
		}
	}
	
foundHeader:
	
	inHeader->len = (size_t)( end - buf );
	err = HTTPHeader_Parse( inHeader );
	require_noerr( err, exit );
	
	inSock->leftoverPtr = end;
	inSock->leftoverEnd = dst;
	
exit:
	return( err );
}

//===========================================================================================================================
//	SocketReadHTTPHeader
//===========================================================================================================================

OSStatus	SocketReadHTTPHeader( SocketRef inSock, HTTPHeader *inHeader )
{
	OSStatus		err;
	char *			buf;
	char *			src;
	char *			dst;
	char *			lim;
	char *			end;
	size_t			len;
	ssize_t			n;
	
	buf = inHeader->buf;
	src = buf;
	dst = buf + inHeader->len;
	lim = buf + sizeof( inHeader->buf );
	for( ;; )
	{
		// If there's data from a previous read, move it to the front to search it first.
		
		len = inHeader->extraDataLen;
		if( len > 0 )
		{
			require_action( len <= (size_t)( lim - dst ), exit, err = kParamErr );
			memmove( dst, inHeader->extraDataPtr, len );
			inHeader->extraDataLen = 0;
		}
		else
		{
			do
			{
				n = read_compat( inSock, dst, (size_t)( lim - dst ) );
				err = map_socket_value_errno( inSock, n >= 0, n );
				
			}	while( err == EINTR );	
			if(      n  > 0 ) len = (size_t) n;
			else if( n == 0 ) { err = kConnectionErr; goto exit; }
			else goto exit;
		}
		dst += len;
		inHeader->len += len;
		
		// Check for interleaved binary data (4 byte header that begins with $). See RFC 2326 section 10.12.
		
		if( ( ( dst - buf ) >= 4 ) && ( buf[ 0 ] == '$' ) )
		{
			end = buf + 4;
			goto foundHeader;
		}
		
		// Find an empty line (separates the header and body). The HTTP spec defines it as CRLFCRLF, but some
		// use LFLF or weird combos like CRLFLF so this handles CRLFCRLF, LFLF, and CRLFLF (but not CRCR).
		
		end = dst;
		for( ;; )
		{
			while( ( src < end ) && ( *src != '\n' ) ) ++src;
			if( src >= end ) break;
			
			len = (size_t)( end - src );
			if( ( len >= 3 ) && ( src[ 1 ] == '\r' ) && ( src[ 2 ] == '\n' ) ) // CRLFCRLF or LFCRLF.
			{
				end = src + 3;
				goto foundHeader;
			}
			else if( ( len >= 2 ) && ( src[ 1 ] == '\n' ) ) // LFLF or CRLFLF.
			{
				end = src + 2;
				goto foundHeader;
			}
			else if( len <= 1 )
			{
				break;
			}
			++src;
		}
	}
	
foundHeader:
	
	inHeader->len = (size_t)( end - buf );
	err = HTTPHeader_Parse( inHeader );
	require_noerr( err, exit );
	
	inHeader->extraDataPtr = end;
	inHeader->extraDataLen = (size_t)( dst - end );
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

