/*
	File:    	DataBufferUtils.c
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

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#if( TARGET_HAS_STD_C_LIB )
	#include <errno.h>
	#include <limits.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#include "DataBufferUtils.h"

#define	kDataBufferFileBufferSize		( 32 * 1024 )

//===========================================================================================================================
//	DataBuffer_Init
//===========================================================================================================================

void	DataBuffer_Init( DataBuffer *inDB, void *inStaticBufferPtr, size_t inStaticBufferLen, size_t inMaxGrowLen )
{
	inDB->staticBufferPtr	= (uint8_t *) inStaticBufferPtr;
	inDB->staticBufferLen	= inStaticBufferLen;
	inDB->maxGrowLen		= inMaxGrowLen;
	inDB->bufferPtr			= (uint8_t *) inStaticBufferPtr;
	inDB->bufferLen			= 0;
	inDB->bufferMaxLen		= inStaticBufferLen;
	inDB->malloced			= 0;
	inDB->firstErr			= kNoErr;
}

//===========================================================================================================================
//	DataBuffer_Free
//===========================================================================================================================

void	DataBuffer_Free( DataBuffer *inDB )
{
	if( inDB->malloced && inDB->bufferPtr )
	{
		free_compat( inDB->bufferPtr );
	}
	inDB->bufferPtr		= inDB->staticBufferPtr;
	inDB->bufferLen		= 0;
	inDB->bufferMaxLen	= inDB->staticBufferLen;
	inDB->malloced		= 0;
	inDB->firstErr		= kNoErr;
}

//===========================================================================================================================
//	DataBuffer_Disown
//===========================================================================================================================

uint8_t *	DataBuffer_Disown( DataBuffer *inDB )
{
	uint8_t *		buf;
	
	buf = DataBuffer_GetPtr( inDB );
	inDB->bufferPtr = NULL;
	inDB->malloced  = 0;
	DataBuffer_Free( inDB );
	return( buf );
}

//===========================================================================================================================
//	DataBuffer_Commit
//===========================================================================================================================

OSStatus	DataBuffer_Commit( DataBuffer *inDB, uint8_t **outPtr, size_t *outLen )
{
	OSStatus		err;
	
	err = inDB->firstErr;
	require_noerr_string( err, exit, "earlier error occurred" );
	
	inDB->firstErr = kAlreadyInUseErr; // Mark in-use to prevent further changes to it until DataBuffer_Free.
	
	if( outPtr ) *outPtr = DataBuffer_GetPtr( inDB );
	if( outLen ) *outLen = DataBuffer_GetLen( inDB );
	
exit:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_Detach
//===========================================================================================================================

OSStatus	DataBuffer_Detach( DataBuffer *inDB, uint8_t **outPtr, size_t *outLen )
{
	OSStatus		err;
	uint8_t *		ptr;
	size_t			len;
	
	len = inDB->bufferLen;
	if( inDB->malloced )
	{
		check( inDB->bufferPtr || ( len == 0 ) );
		ptr = inDB->bufferPtr;
	}
	else
	{
		ptr = (uint8_t *) malloc_compat( ( len > 0 ) ? len : 1 ); // malloc( 0 ) is undefined so use at least 1.
		require_action( ptr, exit, err = kNoMemoryErr );
		
		if( len > 0 ) memcpy( ptr, inDB->bufferPtr, len );
	}
	inDB->bufferPtr		= inDB->staticBufferPtr;
	inDB->bufferLen		= 0;
	inDB->bufferMaxLen	= inDB->staticBufferLen;
	inDB->malloced		= 0;
	inDB->firstErr		= kNoErr;
	
	*outPtr = ptr;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_DetachCString
//===========================================================================================================================

OSStatus	DataBuffer_DetachCString( DataBuffer *inDB, char **outStr )
{
	OSStatus		err;
	uint8_t *		ptr;
	size_t			len;
	
	err = DataBuffer_Append( inDB, "", 1 );
	require_noerr( err, exit );
	
	ptr = NULL;
	err = DataBuffer_Detach( inDB, &ptr, &len );
	require_noerr( err, exit );
	
	*outStr = (char *) ptr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_Replace
//===========================================================================================================================

OSStatus	DataBuffer_Replace( DataBuffer *inDB, size_t inOffset, size_t inOldLen, const void *inNewData, size_t inNewLen )
{
	OSStatus		err;
	size_t			endOffset;
	size_t			oldLen;
	
	err = inDB->firstErr;
	require_noerr( err, exit2 );
	
	if( inNewLen == kSizeCString ) inNewLen = strlen( (const char *) inNewData );
	if( inNewData ) check_ptr_overlap( inDB->bufferPtr, inDB->bufferLen, inNewData, inNewLen );
	
	if( inOffset > inDB->bufferLen )
	{
		endOffset = inOffset + inNewLen;
		require_action( inOldLen == 0, exit, err = kSizeErr );
		require_action( endOffset >= inOffset, exit, err = kSizeErr );
		
		err = DataBuffer_Resize( inDB, endOffset, NULL );
		require_noerr( err, exit2 );
	}
	else
	{
		endOffset = inOffset + inOldLen;
		require_action( endOffset >= inOffset, exit, err = kSizeErr );
		require_action( endOffset <= inDB->bufferLen, exit, err = kRangeErr );
		
		// Shift any data after the data being replaced to make room (growing) or take up slack (shrinking).
		
		oldLen = inDB->bufferLen;
		if( inNewLen > inOldLen )
		{
			err = DataBuffer_Grow( inDB, inNewLen - inOldLen, NULL );
			require_noerr( err, exit2 );
		}
		if( oldLen != endOffset )
		{
			memmove( inDB->bufferPtr + inOffset + inNewLen, inDB->bufferPtr + endOffset, oldLen - endOffset );
		}
		if( inNewLen < inOldLen )
		{
			inDB->bufferLen -= inOldLen - inNewLen;
		}
	}
	
	// Copy in any new data.
	
	if( inNewData )
	{
		memmove( inDB->bufferPtr + inOffset, inNewData, inNewLen );
	}
	goto exit2;
	
exit:
	inDB->firstErr = err;
	
exit2:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_Resize
//===========================================================================================================================

OSStatus	DataBuffer_Resize( DataBuffer *inDB, size_t inNewLen, void *outPtr )
{
	OSStatus		err;
	size_t			oldLen;
	size_t			newMaxLen;
	uint8_t *		newPtr;
		
	err = inDB->firstErr;
	require_noerr( err, exit2 );
	
	// Grow or shink as needed. If growing larger than the max size of the current buffer, reallocate.
	
	oldLen = inDB->bufferLen;
	if( inNewLen > oldLen )
	{
		if( inNewLen > inDB->bufferMaxLen )
		{
			require_action( inNewLen <= inDB->maxGrowLen, exit, err = kOverrunErr );
			
			if(      inNewLen <    256 ) newMaxLen = 256;
			else if( inNewLen <   4096 ) newMaxLen = 4096;
			else if( inNewLen < 131072 ) newMaxLen = AlignUp( inNewLen, 16384 );
			else						 newMaxLen = AlignUp( inNewLen, 131072 );
			
			newPtr = (uint8_t *) malloc_compat( newMaxLen );
			require_action( newPtr, exit, err = kNoMemoryErr );
			
			if( inDB->bufferLen > 0 )
			{
				memcpy( newPtr, inDB->bufferPtr, inDB->bufferLen );
			}
			if( inDB->malloced && inDB->bufferPtr )
			{
				free_compat( inDB->bufferPtr );
			}
			inDB->bufferMaxLen	= newMaxLen;
			inDB->bufferPtr		= newPtr;
			inDB->malloced		= 1;
		}
		inDB->bufferLen = inNewLen;
		if( outPtr ) *( (void **) outPtr ) = inDB->bufferPtr + oldLen;
	}
	else
	{
		// $$$ TO DO: Consider shrinking the buffer if it's shrinking by a large amount.
		
		inDB->bufferLen = inNewLen;
		if( outPtr ) *( (void **) outPtr ) = inDB->bufferPtr;
	}
	goto exit2;
	
exit:
	inDB->firstErr = err;
	
exit2:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_Shrink
//===========================================================================================================================

OSStatus	DataBuffer_Shrink( DataBuffer *inDB, size_t inAmount )
{
	OSStatus		err;
		
	err = inDB->firstErr;
	require_noerr( err, exit2 );
	
	require_action( inAmount <= inDB->bufferLen, exit, err = kSizeErr );
	inDB->bufferLen -= inAmount;
	goto exit2;
	
exit:
	inDB->firstErr = err;
	
exit2:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	DataBuffer_Append
//===========================================================================================================================

OSStatus	DataBuffer_Append( DataBuffer *inDB, const void *inData, size_t inLen )
{
	OSStatus		err;
	uint8_t *		ptr;
	
	if( inLen == kSizeCString ) inLen = strlen( (const char *) inData );
	err = DataBuffer_Grow( inDB, inLen, &ptr );
	require_noerr( err, exit );
	
	memcpy( ptr, inData, inLen );
	
exit:
	return( err );
}

//===========================================================================================================================
//	DataBuffer_AppendF
//===========================================================================================================================

static int	__DataBuffer_PrintFCallBack( const char *inStr, size_t inLen, void *inContext );

OSStatus	DataBuffer_AppendF( DataBuffer *inDB, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = DataBuffer_AppendFVAList( inDB, inFormat, args );
	va_end( args );
	return( err );
}

OSStatus	DataBuffer_AppendFNested( DataBuffer *inDB, const char *inTemplate, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = DataBuffer_AppendF( inDB, inTemplate, inFormat, &args );
	va_end( args );
	return( err );
}

OSStatus	DataBuffer_AppendFVAList( DataBuffer *inDB, const char *inFormat, va_list inArgs )
{
	OSStatus		err;
	int				n;
	
	n = VCPrintF( __DataBuffer_PrintFCallBack, inDB, inFormat, inArgs );
	require_action( n >= 0, exit, err = n );
	err = kNoErr;
	
exit:
	return( err );
}

static int	__DataBuffer_PrintFCallBack( const char *inStr, size_t inLen, void *inContext )
{
	int		result;
	
	result = (int) DataBuffer_Append( (DataBuffer *) inContext, inStr, inLen );
	require_noerr( result, exit );
	result = (int) inLen;
	
exit:
	return( result );
}

//===========================================================================================================================
//	DataBuffer_AppendANSIFile
//===========================================================================================================================

#if( TARGET_HAS_C_LIB_IO )
OSStatus	DataBuffer_AppendANSIFile( DataBuffer *inBuffer, FILE *inFile )
{
	OSStatus		err;
	uint8_t *		buf;
	size_t			n;
	
	buf = (uint8_t *) malloc_compat( kDataBufferFileBufferSize );
	require_action( buf, exit, err = kNoMemoryErr );
	
	for( ;; )
	{
		n = fread( buf, 1, kDataBufferFileBufferSize, inFile );
		if( n == 0 ) break;
		
		err = DataBuffer_Append( inBuffer, buf, n );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	if( buf ) free_compat( buf );
	return( err );
}
#endif

//===========================================================================================================================
//	DataBufferAppendFile
//===========================================================================================================================

#if( TARGET_HAS_C_LIB_IO )
OSStatus	DataBuffer_AppendFile( DataBuffer *inBuffer, const char *inPath )
{
	OSStatus		err;
	FILE *			f;
	
	f = fopen( inPath, "rb" );
	err = map_global_value_errno( f, f );
	require_noerr_quiet( err, exit );
	
	err = DataBuffer_AppendANSIFile( inBuffer, f );
	fclose( f );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	DataBuffer_RunProcessAndAppendOutput
//===========================================================================================================================

#if( TARGET_OS_POSIX )
OSStatus	DataBuffer_RunProcessAndAppendOutput( DataBuffer *inBuffer, const char *inCmdLine )
{
	OSStatus		err;
	FILE *			f;
	
	f = popen( inCmdLine, "r" );
	err = map_global_value_errno( f, f );
	require_noerr_quiet( err, exit );
	
	err = DataBuffer_AppendANSIFile( inBuffer, f );
	if( f ) pclose( f );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

