/*
	File:    	MiscUtils.c
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

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#include "MiscUtils.h"

#include "APSCommonServices.h"	// Include early for TARGET_*, etc. definitions.
#include "APSDebugServices.h"	// Include early for DEBUG_*, etc. definitions.

#if( TARGET_HAS_STD_C_LIB )
	#include <ctype.h>
	#include <errno.h>
	#include <stdarg.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#if  ( TARGET_OS_BSD )
	#include <sys/stat.h>
#endif

#if( TARGET_OS_POSIX )
	#include <ftw.h>
	#include <pthread.h>
	#include <pwd.h>
	#include <spawn.h>
	#include <sys/mman.h>
	#include <linux/sysctl.h>
	#include <sys/wait.h>
	#include <unistd.h>
#endif

#include "CFUtils.h"
#include "DataBufferUtils.h"
#include "MathUtils.h"
#include "RandomNumberUtils.h"
#include "StringUtils.h"
#include "TickUtils.h"

#if 0
#pragma mark == FramesPerSecond ==
#endif

//===========================================================================================================================
//	FPSInit
//===========================================================================================================================

void	FPSInit( FPSData *inData, int inPeriods )
{
	inData->smoothingFactor = 2.0 / ( inPeriods + 1 );
	inData->ticksPerSecF	= (double) UpTicksPerSecond();
	inData->periodTicks		= UpTicksPerSecond();
	inData->lastTicks		= UpTicks();
	inData->totalFrameCount	= 0;
	inData->lastFrameCount	= 0;
	inData->lastFPS			= 0;
	inData->averageFPS		= 0;
}

//===========================================================================================================================
//	FPSUpdate
//===========================================================================================================================

void	FPSUpdate( FPSData *inData, uint32_t inFrameCount )
{
	uint64_t		nowTicks, deltaTicks;
	uint32_t		deltaFrames;
	double			deltaSecs;
	
	inData->totalFrameCount += inFrameCount;
	
	nowTicks   = UpTicks();
	deltaTicks = nowTicks - inData->lastTicks;
	if( deltaTicks >= inData->periodTicks )
	{
		deltaSecs				= deltaTicks / inData->ticksPerSecF;
		deltaFrames				= inData->totalFrameCount - inData->lastFrameCount;
		inData->lastFrameCount	= inData->totalFrameCount;
		inData->lastTicks		= nowTicks;
		inData->lastFPS			= deltaFrames / deltaSecs;
		inData->averageFPS		= ( ( 1.0 - inData->smoothingFactor ) * inData->averageFPS ) + 
										  ( inData->smoothingFactor   * inData->lastFPS );
	}
}

#if 0
#pragma mark -
#pragma mark == Misc ==
#endif

//===========================================================================================================================
//	qsort-compatibile comparison functions.
//===========================================================================================================================

DEFINE_QSORT_NUMERIC_COMPARATOR( int8_t,	qsort_cmp_int8 )
DEFINE_QSORT_NUMERIC_COMPARATOR( uint8_t,	qsort_cmp_uint8 )
DEFINE_QSORT_NUMERIC_COMPARATOR( int16_t,	qsort_cmp_int16 )
DEFINE_QSORT_NUMERIC_COMPARATOR( uint16_t,	qsort_cmp_uint16 )
DEFINE_QSORT_NUMERIC_COMPARATOR( int32_t,	qsort_cmp_int32 )
DEFINE_QSORT_NUMERIC_COMPARATOR( uint32_t,	qsort_cmp_uint32 )
DEFINE_QSORT_NUMERIC_COMPARATOR( int64_t,	qsort_cmp_int64 )
DEFINE_QSORT_NUMERIC_COMPARATOR( uint64_t,	qsort_cmp_uint64 )
DEFINE_QSORT_NUMERIC_COMPARATOR( float,		qsort_cmp_float )
DEFINE_QSORT_NUMERIC_COMPARATOR( double,	qsort_cmp_double )

//===========================================================================================================================
//	QSortPtrs
//
//	QuickSort code derived from the simple quicksort code from the book "The Practice of Programming".
//===========================================================================================================================

void	QSortPtrs( void *inPtrArray, size_t inPtrCount, ComparePtrsFunctionPtr inCmp, void *inContext )
{
	void ** const		ptrArray = (void **) inPtrArray;
	void *				t;
	size_t				i, last;
	
	if( inPtrCount <= 1 )
		return;
	
	i = Random32() % inPtrCount;
	t = ptrArray[ 0 ];
	ptrArray[ 0 ] = ptrArray[ i ];
	ptrArray[ i ] = t;
	
	last = 0;
	for( i = 1; i < inPtrCount; ++i )
	{
		if( inCmp( ptrArray[ i ], ptrArray[ 0 ], inContext ) < 0 )
		{
			t = ptrArray[ ++last ];
			ptrArray[ last ] = ptrArray[ i ];
			ptrArray[ i ] = t;
		}
	}
	t = ptrArray[ 0 ];
	ptrArray[ 0 ] = ptrArray[ last ];
	ptrArray[ last ] = t;
	
	QSortPtrs( ptrArray, last, inCmp, inContext );
	QSortPtrs( &ptrArray[ last + 1 ], ( inPtrCount - last ) - 1, inCmp, inContext );
}

int	CompareIntPtrs( const void *inLeft, const void *inRight, void *inContext )
{
	int const		a = *( (const int *) inLeft );
	int const		b = *( (const int *) inRight );
	
	(void) inContext;
	
	return( ( a > b ) - ( a < b ) );
}

int	CompareStringPtrs( const void *inLeft, const void *inRight, void *inContext )
{
	(void) inContext; // Unused
	
	return( strcmp( (const char *) inLeft, (const char *) inRight ) );
}

//===========================================================================================================================
//	MemReverse
//===========================================================================================================================

void	MemReverse( const void *inSrc, size_t inLen, void *inDst )
{
	check( ( inSrc == inDst ) || !PtrsOverlap( inSrc, inLen, inDst, inLen ) );
	
	if( inSrc == inDst )
	{
		if( inLen > 1 )
		{
			uint8_t *		left  = (uint8_t *) inDst;
			uint8_t *		right = left + ( inLen - 1 );
			uint8_t			temp;
		
			while( left < right )
			{
				temp		= *left;
				*left++		= *right;
				*right--	= temp;
			}
		}
	}
	else
	{
		const uint8_t *		src = (const uint8_t *) inSrc;
		const uint8_t *		end = src + inLen;
		uint8_t *			dst = (uint8_t *) inDst;
		
		while( src < end )
		{
			*dst++ = *( --end );
		}
	}
}

//===========================================================================================================================
//	Swap16Mem
//===========================================================================================================================

void	Swap16Mem( const void *inSrc, size_t inLen, void *inDst )
{
	const uint16_t *			src = (const uint16_t *) inSrc;
	const uint16_t * const		end = src + ( inLen / 2 );
	uint16_t *					dst = (uint16_t *) inDst;
	
	check( ( inLen % 2 ) == 0 );
	check( IsPtrAligned( src, 2 ) );
	check( IsPtrAligned( dst, 2 ) );
	check( ( src == dst ) || !PtrsOverlap( src, inLen, dst, inLen ) );
	
	while( src != end )
	{
		*dst++ = ReadSwap16( src );
		++src;
	}
}

//===========================================================================================================================
//	SwapUUID
//===========================================================================================================================

void	SwapUUID( const void *inSrc, void *inDst )
{
	uint8_t * const		dst = (uint8_t *) inDst;
	uint8_t				tmp[ 16 ];
	
	check( ( inSrc == inDst ) || !PtrsOverlap( inSrc, 16, inDst, 16 ) );
	
	memcpy( tmp, inSrc, 16 );
	dst[  0 ] = tmp[  3 ];
	dst[  1 ] = tmp[  2 ];
	dst[  2 ] = tmp[  1 ];
	dst[  3 ] = tmp[  0 ];
	dst[  4 ] = tmp[  5 ];
	dst[  5 ] = tmp[  4 ];
	dst[  6 ] = tmp[  7 ];
	dst[  7 ] = tmp[  6 ];
	dst[  8 ] = tmp[  8 ];
	dst[  9 ] = tmp[  9 ];
	dst[ 10 ] = tmp[ 10 ];
	dst[ 11 ] = tmp[ 11 ];
	dst[ 12 ] = tmp[ 12 ];
	dst[ 13 ] = tmp[ 13 ];
	dst[ 14 ] = tmp[ 14 ];
	dst[ 15 ] = tmp[ 15 ];
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CopySmallFile
//===========================================================================================================================

#if( TARGET_HAS_STD_C_LIB )
OSStatus	CopySmallFile( const char *inSrcPath, const char *inDstPath )
{
	OSStatus		err;
	FILE *			srcFile;
	FILE *			dstFile;
	size_t			bufSize;
	uint8_t *		buf;
	size_t			readSize;
	size_t			writtenSize;
	
	dstFile	= NULL;
	buf		= NULL;
	
	srcFile = fopen( inSrcPath, "rb" );
	err = map_global_value_errno( srcFile, srcFile );
	require_noerr( err, exit );
	
	dstFile = fopen( inDstPath, "wb" );
	err = map_global_value_errno( dstFile, dstFile );
	require_noerr( err, exit );
	
	bufSize = 256 * 1024;
	buf = (uint8_t *) malloc( bufSize );
	require_action( buf, exit, err = kNoMemoryErr );
	
	for( ;; )
	{
		readSize = fread( buf, 1, bufSize, srcFile );
		if( readSize == 0 ) break;
		
		writtenSize = fwrite( buf, 1, readSize, dstFile );
		err = map_global_value_errno( writtenSize == readSize, dstFile );
		require_noerr( err, exit );
	}
	
exit:
	if( buf )		free( buf );
	if( dstFile )	fclose( dstFile );
	if( srcFile )	fclose( srcFile );
	return( err );
}
#endif

//===========================================================================================================================
//	CopyFileDataByFile
//===========================================================================================================================

OSStatus	CopyFileDataByFile( FILE *inFile, char **outPtr, size_t *outLen )
{
	OSStatus		err;
	char *			buf = NULL;
	size_t			maxLen;
	size_t			offset;
	char *			tmp;
	size_t			len;
	
	maxLen = 0;
	offset = 0;
	for( ;; )
	{
		if( offset >= maxLen )
		{
			if(      maxLen <  160000 )	maxLen = 160000;
			else if( maxLen < 4000000 )	maxLen *= 2;
			else						add_saturate( maxLen, 4000000, SIZE_MAX );
			require_action( maxLen < SIZE_MAX, exit, err = kSizeErr );
			tmp = (char *) realloc( buf, maxLen + 1 );
			require_action( tmp, exit, err = kNoMemoryErr );
			buf = tmp;
		}
		
		len = fread( &buf[ offset ], 1, maxLen - offset, inFile );
		if( len == 0 ) break;
		offset += len;
	}
	
	// Shrink the buffer to avoid wasting memory and null terminate.
	
	tmp = (char *) realloc( buf, offset + 1 );
	require_action( tmp, exit, err = kNoMemoryErr );
	buf = tmp;
	buf[ offset ] = '\0';
	
	*outPtr = buf;
	if( outLen ) *outLen = offset;
	buf = NULL;
	err = kNoErr;
	
exit:
	if( buf ) free( buf );
	return( err );
}

//===========================================================================================================================
//	CopyFileDataByPath
//===========================================================================================================================

OSStatus	CopyFileDataByPath( const char *inPath, char **outPtr, size_t *outLen )
{
	OSStatus		err;
	FILE *			file;
	
	file = fopen( inPath, "rb" );
	err = map_global_value_errno( file, file );
	require_noerr_quiet( err, exit );
	
	err = CopyFileDataByFile( file, outPtr, outLen );
	fclose( file );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	ReadANSIFile
//===========================================================================================================================

OSStatus	ReadANSIFile( FILE *inFile, void *inBuf, size_t inSize, size_t *outSize )
{
	uint8_t *		ptr;
	size_t			n;
	
	ptr = (uint8_t *) inBuf;
	while( inSize > 0 )
	{
		n = fread( ptr, 1, inSize, inFile );
		if( n == 0 ) break;
		ptr    += n;
		inSize -= n;
	}
	if( outSize ) *outSize = (size_t)( ptr - ( (uint8_t *) inBuf ) );
	return( kNoErr );
}

//===========================================================================================================================
//	WriteANSIFile
//===========================================================================================================================

OSStatus	WriteANSIFile( FILE *inFile, const void *inBuf, size_t inSize )
{
	const uint8_t *		ptr;
	size_t				n;
	
	ptr = (const uint8_t *) inBuf;
	while( inSize > 0 )
	{
		n = fwrite( ptr, 1, inSize, inFile );
		if( n == 0 ) break;
		ptr    += n;
		inSize -= n;
	}
	return( kNoErr );
}

//===========================================================================================================================
//	CreateTXTRecordWithCString
//===========================================================================================================================

OSStatus	CreateTXTRecordWithCString( const char *inString, uint8_t **outTXTRec, size_t *outTXTLen )
{
	OSStatus			err;
	DataBuffer			dataBuf;
	const char *		src;
	const char *		end;
	char				buf[ 256 ];
	size_t				len;
	uint8_t *			ptr;
	
	DataBuffer_Init( &dataBuf, NULL, 0, SIZE_MAX );
	
	src = inString;
	end = src + strlen( src );
	while( ParseQuotedEscapedString( src, end, ' ', buf, sizeof( buf ), &len, NULL, &src ) )
	{
		err = DataBuffer_Grow( &dataBuf, 1 + len, &ptr );
		require_noerr( err, exit );
		
		*ptr++ = (uint8_t) len;
		memcpy( ptr, buf, len );
	}
	
	err = DataBuffer_Detach( &dataBuf, outTXTRec, outTXTLen );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );	
}

//===========================================================================================================================
//	TXTRecordGetNextItem
//===========================================================================================================================

Boolean
	TXTRecordGetNextItem( 
		const uint8_t *		inSrc, 
		const uint8_t *		inEnd, 
		const char **		outKeyPtr, 
		size_t *			outKeyLen, 
		const uint8_t **	outValuePtr, 
		size_t *			outValueLen, 
		const uint8_t **	outSrc )
{
	const uint8_t *		end;
	const uint8_t *		keyPtr;
	const uint8_t *		keyEnd;
	size_t				len;
	
	if( inSrc >= inEnd )
	{
		return( false );
	}
	
	len = *inSrc++;
	end = inSrc + len;
	if( end > inEnd )
	{
		len = (size_t)( inEnd - inSrc );
		dlogassert( "bad TXT record: \n%1.1H", inSrc, len, 256 );
		return( false );
	}
	
	for( keyPtr = inSrc; ( inSrc < end ) && ( *inSrc != '=' ); ++inSrc ) {}
	keyEnd = inSrc;
	if( inSrc < end ) inSrc += 1; // Skip '='.
	
	*outKeyPtr		= (const char *) keyPtr;
	*outKeyLen		= (size_t)( keyEnd - keyPtr );
	*outValueLen	= (size_t)( end - inSrc );
	*outValuePtr	= inSrc;
	*outSrc			= end;
	return( true );
}

//===========================================================================================================================
//	GetHomePath
//===========================================================================================================================

#if( TARGET_OS_POSIX )
char *	GetHomePath( char *inBuffer, size_t inMaxLen )
{
	char *				path = NULL;
	long				len;
	char *				buf;
	struct passwd		pwdStorage;
	struct passwd *		pwdPtr;
	
	if( inMaxLen < 1 ) return( "" );
	*inBuffer = '\0';
	
	len = sysconf( _SC_GETPW_R_SIZE_MAX );
	if( ( len <= 0 ) || ( len > SSIZE_MAX ) ) len = 4096;
	
	buf = (char *) malloc( (size_t) len );
	check( buf );
	if( buf )
	{
		pwdPtr = NULL;
		if( ( getpwuid_r( getuid(), &pwdStorage, buf, (size_t) len, &pwdPtr ) == 0 ) && pwdPtr )
		{
			path = pwdPtr->pw_dir;
		}
	}
	if( !path ) path = getenv( "HOME" );
	if( !path ) path = ( getuid() == 0 ) ? "/root" : ".";
	strlcpy( inBuffer, path, inMaxLen );
	if( buf ) free( buf );
	return( path );
}
#endif

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	mkpath
//
//	Copied from the mkdir tool and tweaked to work with const paths and without console output.
//===========================================================================================================================

int	mkpath( const char *path, mode_t mode, mode_t dir_mode )
{
	char buf[PATH_MAX];
	size_t len;
	struct stat sb;
	char *slash;
	int done, err;

	len = strlen(path);
	if(len > (sizeof(buf) - 1)) len = sizeof(buf) - 1;
	memcpy(buf, path, len);
	buf[len] = '\0';
	slash = buf;

	for (;;) {
		slash += strspn(slash, "/");
		slash += strcspn(slash, "/");
		done = (*slash == '\0');
		*slash = '\0';

		err = mkdir(buf, done ? mode : dir_mode);
		if ((err < 0) && (errno != EEXIST)) {
			/*
			 * Can't create; path exists or no perms.
			 * stat() path to determine what's there now.
			 */
			err = errno;
			if (stat(buf, &sb) < 0) {
				/* Not there; use mkdir()'s error */
				return err ? err : -1;
			}
			if (!S_ISDIR(sb.st_mode)) {
				/* Is there, but isn't a directory */
				return ENOTDIR;
			}
		} else if (done) {
			/*
			 * Created ok, and this is the last element
			 */
			/*
			 * The mkdir() and umask() calls both honor only the
			 * file permission bits, so if you try to set a mode
			 * including the sticky, setuid, setgid bits you lose
			 * them. So chmod().
			 */
			if ((mode & ~(S_IRWXU|S_IRWXG|S_IRWXO)) != 0 &&
				chmod(buf, mode) == -1) {
				err = errno;
				return err ? err : -1;
			}
		}
		if (done) {
			break;
		}
		*slash = '/';
	}
	return 0;
}
#endif // TARGET_OS_POSIX

//===========================================================================================================================
//	NormalizePath
//===========================================================================================================================

#if( TARGET_OS_POSIX )
char *	NormalizePath( const char *inSrc, size_t inLen, char *inDst, size_t inMaxLen, uint32_t inFlags )
{
	const char *		src = inSrc;
	const char *		end = ( inLen == kSizeCString ) ? ( inSrc + strlen( inSrc ) ) : ( inSrc + inLen );
	const char *		ptr;
	char *				dst;
	char *				lim;
	size_t				len;
	char				buf1[ PATH_MAX ];
	char				buf2[ PATH_MAX ];
	const char *		replacePtr;
	const char *		replaceEnd;
	
	// If the path is exactly "~" then expand to the current user's home directory.
	// If the path is exactly "~user" then expand to "user"'s home directory.
	// If the path begins with "~/" then expand the "~/" to the current user's home directory.
	// If the path begins with "~user/" then expand the "~user/" to "user"'s home directory.
	
	dst = buf1;
	lim = dst + ( sizeof( buf1 ) - 1 );
	if( !( inFlags & kNormalizePathDontExpandTilde ) && ( src < end ) && ( *src == '~' ) )
	{
		replacePtr = NULL;
		replaceEnd = NULL;
		for( ptr = inSrc + 1; ( src < end ) && ( *src != '/' ); ++src ) {}
		len = (size_t)( src - ptr );
		if( len == 0 ) // "~" or "~/". Expand to current user's home directory.
		{
			replacePtr = getenv( "HOME" );
			if( replacePtr ) replaceEnd = replacePtr + strlen( replacePtr );
		}
		else // "~user" or "~user/". Expand to "user"'s home directory.
		{
			struct passwd *		pw;
			
			len = Min( len, sizeof( buf2 ) - 1 );
			memcpy( buf2, ptr, len );
			buf2[ len ] = '\0';
			pw = getpwnam( buf2 );
			if( pw && pw->pw_dir )
			{
				replacePtr = pw->pw_dir;
				replaceEnd = replacePtr + strlen( replacePtr );
			}
		}
		if( replacePtr ) while( ( dst < lim ) && ( replacePtr < replaceEnd ) ) *dst++ = *replacePtr++;
		else src = inSrc;
	}
	while( ( dst < lim ) && ( src < end ) ) *dst++ = *src++;
	*dst = '\0';
	
	// Resolve the path to remove ".", "..", and sym links.
	
	if( !( inFlags & kNormalizePathDontResolve ) )
	{
		if( inMaxLen >= PATH_MAX )
		{
			dst = realpath( buf1, inDst );
			if( !dst ) strlcpy( inDst, buf1, inMaxLen );
		}
		else
		{
			dst = realpath( buf1, buf2 );
			strlcpy( inDst, dst ? buf2 : buf1, inMaxLen );
		}
	}
	else
	{
		strlcpy( inDst, buf1, inMaxLen );
	}
	return( ( inMaxLen > 0 ) ? inDst : "" );
}
#endif // TARGET_OS_POSIX

//===========================================================================================================================
//	NumberListStringCreateFromUInt8Array
//===========================================================================================================================

#if( TARGET_HAS_STD_C_LIB )

static int	__NumberListStringCreateFromUInt8ArraySorter( const void *inLeft, const void *inRight );

OSStatus	NumberListStringCreateFromUInt8Array( const uint8_t *inArray, size_t inCount, char **outStr )
{
	OSStatus		err;
	DataBuffer		dataBuf;
	uint8_t *		sortedArray;
	size_t			i;
	uint8_t			x;
	uint8_t			y;
	uint8_t			z;
	char			buf[ 32 ];
	char *			ptr;
	
	sortedArray = NULL;
	DataBuffer_Init( &dataBuf, NULL, 0, SIZE_MAX );
	
	if( inCount > 0 )
	{
		sortedArray = (uint8_t *) malloc( inCount * sizeof( *inArray ) );
		require_action( sortedArray, exit, err = kNoMemoryErr );
		
		memcpy( sortedArray, inArray, inCount * sizeof( *inArray ) );
		qsort( sortedArray, inCount, sizeof( *sortedArray ), __NumberListStringCreateFromUInt8ArraySorter );
		
		i = 0;
		while( i < inCount )
		{
			x = sortedArray[ i++ ];
			y = x;
			while( ( i < inCount ) && ( ( ( z = sortedArray[ i ] ) - y ) <= 1 ) )
			{
				y = z;
				++i;
			}
			
			ptr = buf;
			if( x == y )		ptr  += snprintf( ptr, sizeof( buf ), "%d", x );
			else				ptr  += snprintf( ptr, sizeof( buf ), "%d-%d", x, y );
			if( i < inCount )  *ptr++ = ',';
			
			err = DataBuffer_Append( &dataBuf, buf, (size_t)( ptr - buf ) );
			require_noerr( err, exit );
		}
	}
	
	err = DataBuffer_Append( &dataBuf, "", 1 );
	require_noerr( err, exit );
	
	*outStr = (char *) dataBuf.bufferPtr;
	dataBuf.bufferPtr = NULL;
	
exit:
	if( sortedArray ) free( sortedArray );
	DataBuffer_Free( &dataBuf );
	return( err );
}

static int	__NumberListStringCreateFromUInt8ArraySorter( const void *inLeft, const void *inRight )
{
	return( *( (const uint8_t *)  inLeft ) - *( (const uint8_t *)  inRight ) );
}
#endif // TARGET_HAS_STD_C_LIB

//===========================================================================================================================
//	RemovePath
//===========================================================================================================================

#if( TARGET_OS_POSIX )
static int	_RemovePathCallBack( const char *inPath, const struct stat *inStat, int inFlags, struct FTW *inFTW );

OSStatus	RemovePath( const char *inPath )
{
	OSStatus		err;
	
	err = nftw( inPath, _RemovePathCallBack, 64, FTW_CHDIR | FTW_DEPTH | FTW_MOUNT | FTW_PHYS );
	err = map_global_noerr_errno( err );
	return( err );
}

static int	_RemovePathCallBack( const char *inPath, const struct stat *inStat, int inFlags, struct FTW *inFTW )
{
	int		err;
	
	(void) inStat;
	(void) inFlags;
	(void) inFTW;
	
	err = remove( inPath );
	err = map_global_noerr_errno( err );
	return( err );
}
#endif

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	RunningWindowsVistaOrLater
//===========================================================================================================================

Boolean	RunningWindowsVistaOrLater( void )
{
	OSVERSIONINFOEX		osvi;
	DWORDLONG			conditionMask;

	memset( &osvi, 0, sizeof( osvi ) );
	osvi.dwOSVersionInfoSize	= sizeof( osvi );
	osvi.dwMajorVersion			= 6; // Windows Vista
	osvi.dwMinorVersion			= 0;
	
	conditionMask = 0;
	VER_SET_CONDITION( conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
	VER_SET_CONDITION( conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );
	
	if( VerifyVersionInfo( &osvi, VER_MAJORVERSION | VER_MINORVERSION, conditionMask ) )
	{
		return( true );
	}
	return( false );
}
#endif

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	RunProcessAndCaptureOutput
//===========================================================================================================================

OSStatus	RunProcessAndCaptureOutput( const char *inCmdLine, char **outResponse )
{
	return( RunProcessAndCaptureOutputEx( inCmdLine, outResponse, NULL ) );
}

OSStatus	RunProcessAndCaptureOutputEx( const char *inCmdLine, char **outResponse, size_t *outLen )
{
	OSStatus		err;
	DataBuffer		dataBuf;
	
	DataBuffer_Init( &dataBuf, NULL, 0, SIZE_MAX );
	
	err = DataBuffer_RunProcessAndAppendOutput( &dataBuf, inCmdLine );
	require_noerr_quiet( err, exit );
	
	err = DataBuffer_Append( &dataBuf, "", 1 );
	require_noerr( err, exit );
	
	*outResponse = (char *) DataBuffer_GetPtr( &dataBuf );
	if( outLen ) *outLen =  DataBuffer_GetLen( &dataBuf );
	dataBuf.bufferPtr = NULL;
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}
#endif

#if( ( !0 && 0 ) || COMMON_SERVICES_NO_SYSTEM_CONFIGURATION )
//===========================================================================================================================
//	SCDynamicStoreCopyComputerName
//===========================================================================================================================

CFStringRef	SCDynamicStoreCopyComputerName( SCDynamicStoreRef inStore, CFStringEncoding *outEncoding )
{
	OSStatus		err;
	char *			str;
	CFStringRef		cfStr;
	
	(void) inStore; // Unused
	
	cfStr = NULL;
	
	err = ACPGetProperty( kACPPropertySysName, 0, 0, &str, NULL );
	require_noerr( err, exit );
	
	cfStr = CFStringCreateWithCString( NULL, str, kCFStringEncodingUTF8 );
	free( str );
	require( cfStr, exit );
	
	if( outEncoding ) *outEncoding = kCFStringEncodingUTF8;
	
exit:
	return( cfStr );
}
#endif

#if( ( !0 && 0 ) || COMMON_SERVICES_NO_SYSTEM_CONFIGURATION )
//===========================================================================================================================
//	SCDynamicStoreCopyLocalHostName
//===========================================================================================================================

CFStringRef	SCDynamicStoreCopyLocalHostName( SCDynamicStoreRef inStore )
{
	char			str[ 256 ];
	size_t			len;
	CFStringRef		cfStr;
	OSStatus		err;
	
	(void) inStore; // Unused
	
	cfStr = NULL;
	
	// Try the official DNS name property. If not set fall back to UTF-8 name and cleanse it.
	
	len = 0;
	ACPGetProperty( kACPPropertySysDNSName, kACPPropertyOptionCopyDirect, sizeof( str ) - 1, str, &len );
	str[ len ] = '\0';
	ConvertUTF8StringToRFC1034LabelString( str, str );
	if( *str == '\0' )
	{
		len = 0;
		ACPGetProperty( kACPPropertySysName, kACPPropertyOptionCopyDirect, sizeof( str ) - 1, str, &len );
		str[ len ] = '\0';
		ConvertUTF8StringToRFC1034LabelString( str, str );
	}
	if( *str == '\0' )
	{
		uint8_t		mac[ 6 ];
		
		// No valid RFC-1034 character in the hostname so use a default name based on the AirPort ID.
		
		memset( mac, 0, sizeof( mac ) );
		err = ACPGetProperty( kACPPropertyRadioMACAddress, kACPPropertyOptionCopyDirect, 0, mac, NULL );
		if( err ) ACPGetProperty( kACPPropertyWANMACAddress, kACPPropertyOptionCopyDirect, 0, mac, NULL );
		snprintf( str, sizeof( str ), "Base-Station-%02x%02x%02x", mac[ 3 ], mac[ 4 ], mac[ 5 ] );
	}
	
	cfStr = CFStringCreateWithCString( NULL, str, kCFStringEncodingUTF8 );
	require( cfStr, exit );
	
exit:
	return( cfStr );
}
#elif( !0 && TARGET_OS_POSIX )
CFStringRef	SCDynamicStoreCopyLocalHostName( SCDynamicStoreRef inStore )
{
	char			buf[ 256 ];
	CFStringRef		cfStr;
	
	(void) inStore; // Unused
	
	buf[ 0 ] = '\0';
	gethostname( buf, sizeof( buf ) );
	buf[ sizeof( buf ) - 1 ] = '\0';
	
	cfStr = CFStringCreateWithCString( NULL, buf, kCFStringEncodingUTF8 );
	require( cfStr, exit );
		
exit:
	return( cfStr );
}
#endif

#if( TARGET_OS_POSIX )
//===========================================================================================================================
//	APSSystemf
//===========================================================================================================================

extern char ** environ;

int	APSSystemf( const char *inLogPrefix, const char *inFormat, ... )
{
	int				err;
	va_list			args;
	char			*cmd, *parsedCmd, *currentParsedCmd;
	const char		*currentCmd;
	char			**parsedArgs;
	size_t			currentParsedCmdLen, parsedArgsCount, parsedArgsCapacity, parsedArgLen;
	pid_t			pid;
	
	cmd = NULL;
	parsedCmd = NULL;
	parsedArgs = NULL;
	va_start( args, inFormat );
	VASPrintF( &cmd, inFormat, args );
	va_end( args );
	require_action( cmd, exit, err = kUnknownErr );
	
	parsedArgsCount = 0;
	parsedArgsCapacity = 0;
	currentCmd = cmd;
	currentParsedCmdLen = strlen( cmd );
	currentParsedCmd = parsedCmd = malloc( currentParsedCmdLen + 1 ); // +1 for last NULL-termination
	require_action( parsedCmd, exit, err = kNoMemoryErr );

	while( ParseQuotedEscapedString( currentCmd, NULL, ' ', currentParsedCmd, currentParsedCmdLen, &parsedArgLen, NULL, &currentCmd ) )
	{
		if( parsedArgsCount == parsedArgsCapacity )
		{
			parsedArgsCapacity += 10;
			parsedArgs = realloc( parsedArgs, ( parsedArgsCapacity + 1 )*sizeof( char* ) ); // +1 for NULL-terminating entry
			require_action( parsedArgs, exit, err = kNoMemoryErr );
		}
		
		currentParsedCmd[ parsedArgLen++ ] = '\0';
		parsedArgs[ parsedArgsCount++ ] = currentParsedCmd;
		currentParsedCmd += parsedArgLen;
		currentParsedCmdLen -= parsedArgLen;
	}
	require_action( parsedArgsCount, exit, err = kParamErr );
	parsedArgs[ parsedArgsCount ] = NULL;
	
	if( inLogPrefix ) fprintf( stderr, "%s%s\n", inLogPrefix, cmd );
	err = posix_spawnp( &pid, parsedArgs[ 0 ], NULL, NULL, parsedArgs, environ );
	if ( err == 0 )
	{
		if( waitpid( pid, &err, 0 ) < 0 )
			err = errno;
		else if ( WIFEXITED( err ) )
			err = WEXITSTATUS( err );
		else // WIFSIGNALLED
			err = kUnknownErr;
	}
	
exit:
	free( cmd );
	free( parsedCmd );
	free( parsedArgs );
	return( err );
}
#endif

#if 0
#pragma mark -
#pragma mark == Packing/Unpacking ==
#endif

//===========================================================================================================================
//	PackData
//===========================================================================================================================

OSStatus	PackData( void *inBuffer, size_t inMaxSize, size_t *outSize, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = PackDataVAList( inBuffer, inMaxSize, outSize, inFormat, args );
	va_end( args );
	
	return( err );
}

//===========================================================================================================================
//	PackDataVAList
//===========================================================================================================================

OSStatus	PackDataVAList( void *inBuffer, size_t inMaxSize, size_t *outSize, const char *inFormat, va_list inArgs )
{
	OSStatus			err;
	const char *		fmt;
	char				c;
	const uint8_t *		src;
	const uint8_t *		end;
	uint8_t *			dst;
	uint8_t *			lim;
	uint8_t				u8;
	uint16_t			u16;
	uint32_t			u32;
	size_t				size;
	
	dst = (uint8_t *) inBuffer;
	lim = dst + inMaxSize;
	
	// Loop thru each character in the format string, decode it, and pack the data appropriately.
	
	fmt = inFormat;
	for( c = *fmt; c != '\0'; c = *++fmt )
	{
		int		altForm;
		
		// Ignore non-% characters like spaces since they can aid in format string readability.
		
		if( c != '%' ) continue;
		
		// Flags
		
		altForm = 0;
		for( ;; )
		{
			c = *++fmt;
			if( c == '#' ) altForm += 1;
			else break;
		}
		
		// Format specifiers.
		
		switch( c )
		{
			case 'b':	// %b: Write byte (8-bit); arg=unsigned int
				
				require_action( dst < lim, exit, err = kOverrunErr );
				u8 = (uint8_t) va_arg( inArgs, unsigned int );
				*dst++ = u8;
				break;
			
			case 'H':	// %H: Write big endian half-word (16-bit); arg=unsigned int
				
				require_action( ( lim - dst ) >= 2, exit, err = kOverrunErr );
				u16	= (uint16_t) va_arg( inArgs, unsigned int );
				*dst++ 	= (uint8_t)( ( u16 >>  8 ) & 0xFF );
				*dst++ 	= (uint8_t)(   u16         & 0xFF );
				break;
			
			case 'W':	// %W: Write big endian word (32-bit); arg=unsigned int
				
				require_action( ( lim - dst ) >= 4, exit, err = kOverrunErr );
				u32	= (uint32_t) va_arg( inArgs, unsigned int );
				*dst++ 	= (uint8_t)( ( u32 >> 24 ) & 0xFF );
				*dst++ 	= (uint8_t)( ( u32 >> 16 ) & 0xFF );
				*dst++ 	= (uint8_t)( ( u32 >>  8 ) & 0xFF );
				*dst++ 	= (uint8_t)(   u32         & 0xFF );
				break;
			
			case 's':	// %s/%#s: Write string/length byte-prefixed string; arg=const char *inStr (null-terminated)
						// Note: Null terminator is not written.
				
				src = va_arg( inArgs, const uint8_t * );
				require_action( src, exit, err = kParamErr );
				
				for( end = src; *end; ++end ) {}
				size = (size_t)( end - src );
				
				if( altForm ) // Pascal-style length byte-prefixed string
				{
					require_action( size <= 255, exit, err = kSizeErr );
					require_action( ( 1 + size ) <= ( (size_t)( lim - dst ) ), exit, err = kOverrunErr );
					*dst++ = (uint8_t) size;
				}
				else
				{
					require_action( size <= ( (size_t)( lim - dst ) ), exit, err = kOverrunErr );
				}
				while( src < end )
				{
					*dst++ = *src++;
				}
				break;
			
			case 'n':	// %n: Write N bytes; arg 1=const void *inData, arg 2=unsigned int inSize
				
				src = va_arg( inArgs, const uint8_t * );
				require_action( src, exit, err = kParamErr );
				
				size = (size_t) va_arg( inArgs, unsigned int );
				require_action( size <= ( (size_t)( lim - dst ) ), exit, err = kOverrunErr );
				
				end = src + size;
				while( src < end )
				{
					*dst++ = *src++;
				}
				break;
			
			default:
				dlogassert( "unknown format specifier: %%%c", c );
				err = kUnsupportedErr;
				goto exit;
		}
	}
	
	*outSize = (size_t)( dst - ( (uint8_t *) inBuffer ) );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	UnpackData
//===========================================================================================================================

OSStatus	UnpackData( const void *inData, size_t inSize, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	
	va_start( args, inFormat );
	err = UnpackDataVAList( inData, inSize, inFormat, args );
	va_end( args );
	
	return( err );
}

//===========================================================================================================================
//	UnpackDataVAList
//===========================================================================================================================

OSStatus	UnpackDataVAList( const void *inData, size_t inSize, const char *inFormat, va_list inArgs )
{
	OSStatus				err;
	const char *			fmt;
	char					c;
	const uint8_t *			src;
	const uint8_t *			end;
	const uint8_t *			ptr;
	uint16_t				u16;
	uint32_t				u32;
	size_t					size;
	uint8_t *				bArg;
	uint16_t *				hArg;
	uint32_t *				wArg;
	const uint8_t **		ptrArg;
	size_t *				sizeArg;
		
	src = (const uint8_t *) inData;
	end = src + inSize;
	
	// Loop thru each character in the format string, decode it, and unpack the data appropriately.
	
	fmt = inFormat;
	for( c = *fmt; c != '\0'; c = *++fmt )
	{
		int		altForm;
		
		// Ignore non-% characters like spaces since they can aid in format string readability.
		
		if( c != '%' ) continue;
		
		// Flags
		
		altForm = 0;
		for( ;; )
		{
			c = *++fmt;
			if( c == '#' ) altForm += 1;
			else break;
		}
		
		// Format specifiers.
		
		switch( c )
		{
			case 'b':	// %b: Read byte (8-bit); arg=uint8_t *outByte
				
				require_action( altForm == 0, exit, err = kFlagErr );
				require_action_quiet( ( end - src ) >= 1, exit, err = kUnderrunErr );
				bArg = va_arg( inArgs, uint8_t * );
				if( bArg ) *bArg = *src;
				++src;
				break;
			
			case 'H':	// %H: Read big endian half-word (16-bit); arg=uint16_t *outU16
				
				require_action( ( end - src ) >= 2, exit, err = kUnderrunErr );
				u16  = (uint16_t)( *src++ << 8 );
				u16 |= (uint16_t)( *src++ );
				hArg = va_arg( inArgs, uint16_t * );
				if( hArg ) *hArg = u16;
				break;
			
			case 'W':	// %H: Read big endian word (32-bit); arg=uint32_t *outU32
				
				require_action( ( end - src ) >= 4, exit, err = kUnderrunErr );
				u32  = (uint32_t)( *src++ << 24 );
				u32 |= (uint32_t)( *src++ << 16 );
				u32 |= (uint32_t)( *src++ << 8 );
				u32 |= (uint32_t)( *src++ );
				wArg = va_arg( inArgs, uint32_t * );
				if( wArg ) *wArg = u32;
				break;
			
			case 's':	// %s: Read string; arg 1=const char **outStr, arg 2=size_t *outSize (size excludes null terminator).
				
				if( altForm ) // Pascal-style length byte-prefixed string
				{
					require_action( src < end, exit, err = kUnderrunErr );
					size = *src++;
					require_action( size <= (size_t)( end - src ), exit, err = kUnderrunErr );
					
					ptr = src;
					src += size;
				}
				else
				{
					for( ptr = src; ( src < end ) && ( *src != 0 ); ++src ) {}
					require_action( src < end, exit, err = kUnderrunErr );
					size = (size_t)( src - ptr );
					++src;
				}
				
				ptrArg = va_arg( inArgs, const uint8_t ** );
				if( ptrArg ) *ptrArg = ptr;
				
				sizeArg	= va_arg( inArgs, size_t * );
				if( sizeArg ) *sizeArg = size;
				break;
			
			case 'n':	// %n: Read N bytes; arg 1=size_t inSize, arg 2=const uint8_t **outData
				
				size = (size_t) va_arg( inArgs, unsigned int );
				require_action( size <= (size_t)( end - src ), exit, err = kUnderrunErr );
				
				ptrArg = va_arg( inArgs, const uint8_t ** );
				if( ptrArg ) *ptrArg = src;
				
				src += size;
				break;
			
			default:
				dlogassert( "unknown format specifier: %%%c", c );
				err = kUnsupportedErr;
				goto exit;
		}
	}
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == EDID ==
#endif

//===========================================================================================================================
//	CreateEDIDDictionaryWithBytes
//===========================================================================================================================

static OSStatus	_ParseEDID_CEABlock( const uint8_t *inData, CFMutableDictionaryRef inEDIDDict );
static OSStatus	_ParseEDID_CEABlock_HDMI( const uint8_t *inData, size_t inSize, CFMutableDictionaryRef inCAEDict );

CFDictionaryRef	CreateEDIDDictionaryWithBytes( const uint8_t *inData, size_t inSize, OSStatus *outErr )
{
	CFDictionaryRef				result		= NULL;
	CFMutableDictionaryRef		edidDict	= NULL;
	OSStatus					err;
	const uint8_t *				src;
	const uint8_t *				end;
	uint8_t						u8;
	uint32_t					u32;
	char						strBuf[ 256 ];
	const char *				strPtr;
	const char *				strEnd;
	char						c;
	CFStringRef					key;
	CFStringRef					cfStr;
	int							i;
	char *						dst;
	int							extensionCount;
	
	require_action_quiet( inSize >= 128, exit, err = kSizeErr );
	require_action_quiet( memcmp( inData, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8 ) == 0, exit, err = kFormatErr );
	
	src = inData;
	end = src + 128;
	for( u8 = 0; src < end; ++src ) u8 += *src;
	require_action_quiet( u8 == 0, exit, err = kChecksumErr );
	
	edidDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( edidDict, exit, err = kNoMemoryErr ); 
	
	CFDictionarySetData( edidDict, kEDIDKey_RawBytes, inData, inSize );
	
	// Manufacturer. It's 3 characters that get encoded as 2 bytes.
	
	strBuf[ 0 ] = (char)( ( ( inData[ 8 ] & 0x7C ) >> 2 ) + '@' );
	strBuf[ 1 ] = (char)( ( ( inData[ 8 ] & 0x03 ) << 3 ) + ( ( inData[ 9 ] & 0xE0 ) >> 5 ) + '@' );
	strBuf[ 2 ] = (char)( (   inData[ 9 ] & 0x1F ) + '@' );
	strBuf[ 3 ] = '\0';
	
	cfStr = CFStringCreateWithCString( NULL, strBuf, kCFStringEncodingUTF8 );
	if( !cfStr )
	{
		// Malformed manufacturer so %-escape the raw bytes.
		
		snprintf( strBuf, sizeof( strBuf ), "<<%%%02X%%%02X>>", inData[ 8 ], inData[ 9 ] );
		cfStr = CFStringCreateWithCString( NULL, strBuf, kCFStringEncodingUTF8 );
		require_action( cfStr, exit, err = kNoMemoryErr );
	}
	CFDictionarySetValue( edidDict, kEDIDKey_Manufacturer, cfStr );
	CFRelease( cfStr );
	
	// Product ID
	
	u32 = inData[ 10 ] | ( inData[ 11 ] << 8 );
	CFDictionarySetInt64( edidDict, kEDIDKey_ProductID, u32 );
	
	// Serial number
	
	u32 = inData[ 12 ] | ( inData[ 13 ] << 8 ) | ( inData[ 14 ] << 16 ) | ( inData[ 15 ] << 24 );
	CFDictionarySetInt64( edidDict, kEDIDKey_SerialNumber, u32 );
	
	// Manufacture date (year and week).
	
	CFDictionarySetInt64( edidDict, kEDIDKey_WeekOfManufacture, inData[ 16 ] );
	CFDictionarySetInt64( edidDict, kEDIDKey_YearOfManufacture, 1990 + inData[ 17 ] );
	
	// EDID versions.
	
	CFDictionarySetInt64( edidDict, kEDIDKey_EDIDVersion, inData[ 18 ] );
	CFDictionarySetInt64( edidDict, kEDIDKey_EDIDRevision, inData[ 19 ] );
	
	// $$$ TO DO: Parse bytes 20-53.
	
	// Parse descriptor blocks.
	
	src = &inData[ 54 ]; // Descriptor Block 1 (4 of them total, 18 bytes each).
	for( i = 0; i < 4; ++i )
	{
		// If the first two bytes are 0 then it's a monitor descriptor.
		
		if( ( src[ 0 ] == 0 ) && ( src[ 1 ] == 0 ) )
		{
			key = NULL;
			u8 = src[ 3 ];
			if(      u8 == 0xFC ) key = kEDIDKey_MonitorName;
			else if( u8 == 0xFF ) key = kEDIDKey_MonitorSerialNumber;
			else {} // $$$ TO DO: parse other descriptor block types.
			if( key )
			{
				dst = strBuf;
				strPtr = (const char *) &src[  5 ];
				strEnd = (const char *) &src[ 18 ];
				while( ( strPtr < strEnd ) && ( ( c = *strPtr ) != '\n' ) && ( c != '\0' ) )
				{
					c = *strPtr++;
					if( ( c >= 32 ) && ( c <= 126 ) )
					{
						*dst++ = c;
					}
					else
					{
						dst += snprintf( dst, 3, "%%%02X", (uint8_t) c );
					}
				}
				*dst = '\0';
				CFDictionarySetCString( edidDict, key, strBuf, kSizeCString );
			}
			else
			{
				// $$$ TO DO: parse other descriptor blocks.
			}
		}
		else
		{
			// $$$ TO DO: parse video timing descriptors.
		}
		
		src += 18; // Move to the next descriptor block.
	}
	
	// Parse extension blocks in EDID versions 1.1 or later.
	
	extensionCount = 0;
	u32 = ( inData[ 18 ] << 8 ) | inData[ 19 ]; // Combine version and revision for easier comparisons.
	if( u32 >= 0x0101 )
	{
		extensionCount = inData[ 126 ];
	}
	src = &inData[ 128 ];
	end = inData + inSize;
	for( i = 0; i < extensionCount; ++i )
	{
		if( ( end - src ) < 128 ) break;
		if( src[ 0 ] == 2 ) _ParseEDID_CEABlock( src, edidDict );
		src += 128;
	}
	
	result = edidDict;
	edidDict = NULL;
	err = kNoErr;	
	
exit:
	if( edidDict )	CFRelease( edidDict );
	if( outErr )	*outErr = err;
	return( result );	
}

//===========================================================================================================================
//	_ParseEDID_CEABlock
//===========================================================================================================================

static OSStatus	_ParseEDID_CEABlock( const uint8_t *inData, CFMutableDictionaryRef inEDIDDict )
{
	OSStatus					err;
	CFMutableDictionaryRef		ceaDict;
	uint8_t						u8;
	const uint8_t *				src;
	const uint8_t *				end;
	uint8_t						dtdOffset;
	uint32_t					regID;
	uint8_t						tag, len;
	
	ceaDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( ceaDict, exit, err = kNoMemoryErr ); 
	CFDictionarySetValue( inEDIDDict, kEDIDKey_CEABlock, ceaDict );
	
	// Verify the checksum.
	
	src = inData;
	end = src + 128;
	for( u8 = 0; src < end; ++src ) u8 += *src;
	require_action_quiet( u8 == 0, exit, err = kChecksumErr );
	
	// Revision.
	
	CFDictionarySetInt64( inEDIDDict, kCEAKey_Revision, inData[ 0 ] );
	
	// $$$ TO DO: Parse general video info.
	
	// Parse each data block.
	
	dtdOffset = inData[ 2 ];
	require_action_quiet( dtdOffset <= 128, exit, err = kRangeErr );
	
	src = &inData[ 4 ];
	end = &inData[ dtdOffset ];
	for( ; src < end; src += ( 1 + len ) )
	{
		tag = src[ 0 ] >> 5;
		len = src[ 0 ] & 0x1F;
		require_action_quiet( ( src + 1 + len ) <= end, exit, err = kUnderrunErr );
		
		switch( tag )
		{
			case 1:
				// $$$ TO DO: Parse Audio Data Block.
				break;
			
			case 2:
				// $$$ TO DO: Parse Video Data Block.
				break;
			
			case 3:
				if( len < 3 ) break;
				regID = src[ 1 ] | ( src[ 2 ] << 8 ) | ( src[ 3 ] << 16 );
				switch( regID )
				{
					case 0x000C03:
						_ParseEDID_CEABlock_HDMI( src, len + 1, ceaDict );
						break;
					
					default:
						break;
				}
				break;
			
			case 4:
				// $$$ TO DO: Parse Speaker Allocation Data Block.
				break;
			
			default:
				break;
		}
	}
	
	// $$$ TO DO: Parse Detailed Timing Descriptors (DTDs).
	
	err = kNoErr;
	
exit:
	if( ceaDict ) CFRelease( ceaDict );
	return( err );
}

//===========================================================================================================================
//	ParseEDID_CEABlock_HDMI
//
//	Note: "inData" points to the data block header byte and "inSize" is the size of the data including the header byte 
//	(inSize = len + 1). This makes a easier to follow byte offsets from the HDMI spec, which are header relative.
//===========================================================================================================================

static OSStatus	_ParseEDID_CEABlock_HDMI( const uint8_t *inData, size_t inSize, CFMutableDictionaryRef inCAEDict )
{
	OSStatus					err;
	CFMutableDictionaryRef		hdmiDict;
	const uint8_t *				ptr;
	const uint8_t *				end;
	int16_t						s16;
	uint32_t					u32;
	uint8_t						latencyFlags;
	const uint8_t *				latencyPtr;
	
	hdmiDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( hdmiDict, exit, err = kNoMemoryErr ); 
	CFDictionarySetValue( inCAEDict, kEDIDKey_HDMI, hdmiDict );
	
	ptr = inData;
	end = inData + inSize;
	require_action_quiet( ( end - ptr ) >= 6, exit, err = kSizeErr );
	
	// Source Physical Address.
	
	u32 = ( ptr[ 4 ] << 8 ) | ptr[ 5 ];
	CFDictionarySetInt64( hdmiDict, kHDMIKey_SourcePhysicalAddress, u32 );
	
	if( inSize > 8 )
	{
		// Parse latency. Note: latency value = ( latency-in-milliseconds / 2 ) + 1 (max value of 251 = 500 ms).
		
		latencyFlags = inData[ 8 ];
		if( latencyFlags & 0x80 )
		{
			latencyPtr = &inData[ 9 ];
			require_action_quiet( ( latencyPtr + 1 ) < end, exit, err = kUnderrunErr );
			
			// Video latency.
			
			s16 = *latencyPtr++;
			s16 = ( s16 - 1 ) * 2;
			CFDictionarySetInt64( hdmiDict, kHDMIKey_VideoLatencyMs, s16 );
			
			// Audio latency.
			
			s16 = *latencyPtr++;
			s16 = ( s16 - 1 ) * 2;
			CFDictionarySetInt64( hdmiDict, kHDMIKey_AudioLatencyMs, s16 );
			
			// Parse interlaced latency.
			
			if( latencyFlags & 0x40 )
			{
				require_action_quiet( ( latencyPtr + 1 ) < end, exit, err = kUnderrunErr );
				
				// Video latency.
				
				s16 = *latencyPtr++;
				s16 = ( s16 - 1 ) * 2;
				CFDictionarySetInt64( hdmiDict, kHDMIKey_VideoLatencyInterlacedMs, s16 );
				
				// Audio latency.
				
				s16 = *latencyPtr++;
				s16 = ( s16 - 1 ) * 2;
				CFDictionarySetInt64( hdmiDict, kHDMIKey_AudioLatencyInterlacedMs, s16 );
			}
		}
	}
	err = kNoErr;
	
exit:
	if( hdmiDict ) CFRelease( hdmiDict );
	return( err );
}

#if 0
#pragma mark -
#pragma mark == H.264 ==
#endif

//===========================================================================================================================
//	H264ConvertAVCCtoAnnexBHeader
//===========================================================================================================================

OSStatus
	H264ConvertAVCCtoAnnexBHeader( 
		const uint8_t *		inAVCCPtr,
		size_t				inAVCCLen,
		uint8_t *			inHeaderBuf,
		size_t				inHeaderMaxLen,
		size_t *			outHeaderLen,
		size_t *			outNALSize,
		const uint8_t **	outNext )
{
	const uint8_t *				src = inAVCCPtr;
	const uint8_t * const		end = src + inAVCCLen;
	uint8_t * const				buf = inHeaderBuf;
	uint8_t *					dst = buf;
	const uint8_t * const		lim = dst + inHeaderMaxLen;
	OSStatus					err;
	size_t						nalSize;
	int							i, n;
	uint16_t					len;
	
	// AVCC Format is documented in ISO/IEC STANDARD 14496-15 (AVC file format) section 5.2.4.1.1.
	//
	// [0x00] version = 1.
	// [0x01] AVCProfileIndication		Profile code as defined in ISO/IEC 14496-10.
	// [0x02] Profile Compatibility		Byte between profile_IDC and level_IDC in SPS from ISO/IEC 14496-10.
	// [0x03] AVCLevelIndication		Level code as defined in ISO/IEC 14496-10.
	// [0x04] LengthSizeMinusOne		0b111111xx where xx is 0, 1, or 3 mapping to 1, 2, or 4 bytes for nal_size.
	// [0x05] SPSCount					0b111xxxxx where xxxxx is the number of SPS entries that follow this field.
	// [0x06] SPS entries				Variable-length SPS array. Each entry has the following structure:
	//		uint16_t	spsLen			Number of bytes in the SPS data until the next entry or the end (big endian).
	//		uint8_t		spsData			SPS entry data.
	// [0xnn] uint8_t	PPSCount		Number of Picture Parameter Sets (PPS) that follow this field.
	// [0xnn] PPS entries				Variable-length PPS array. Each entry has the following structure:
	//		uint16_t	ppsLen			Number of bytes in the PPS data until the next entry or the end (big endian).
	//		uint8_t		ppsData			PPS entry data.
	//
	// Annex-B format is documented in the H.264 spec in Annex-B.
	// Each NAL unit is prefixed with 0x00 0x00 0x00 0x01 and the nal_size from the AVCC is removed.
	
	// Write the SPS NAL units.
	
	require_action( ( end - src ) >= 6, exit, err = kSizeErr );
	nalSize	= ( src[ 4 ] & 0x03 ) + 1;
	n		=   src[ 5 ] & 0x1F;
	src		=  &src[ 6 ];
	for( i = 0; i < n; ++i )
	{
		require_action( ( end - src ) >= 2, exit, err = kUnderrunErr );
		len = (uint16_t)( ( src[ 0 ] << 8 ) | src[ 1 ] );
		src += 2;
		
		require_action( ( end - src ) >= len, exit, err = kUnderrunErr );
		if( inHeaderBuf )
		{
			require_action( ( lim - dst ) >= ( 4 + len ), exit, err = kOverrunErr );
			dst[ 0 ] = 0x00;
			dst[ 1 ] = 0x00;
			dst[ 2 ] = 0x00;
			dst[ 3 ] = 0x01;
			memcpy( dst + 4, src, len );
		}
		src += len;
		dst += ( 4 + len );
	}
	
	// Write PPS NAL units.
	
	if( ( end - src ) >= 1 )
	{
		n = *src++;
		for( i = 0; i < n; ++i )
		{
			require_action( ( end - src ) >= 2, exit, err = kUnderrunErr );
			len = (uint16_t)( ( src[ 0 ] << 8 ) | src[ 1 ] );
			src += 2;
			
			require_action( ( end - src ) >= len, exit, err = kUnderrunErr );
			if( inHeaderBuf )
			{
				require_action( ( lim - dst ) >= ( 4 + len ), exit, err = kOverrunErr );
				dst[ 0 ] = 0x00;
				dst[ 1 ] = 0x00;
				dst[ 2 ] = 0x00;
				dst[ 3 ] = 0x01;
				memcpy( dst + 4, src, len );
			}
			src += len;
			dst += ( 4 + len );
		}
	}
	
	if( outHeaderLen )	*outHeaderLen	= (size_t)( dst - buf );
	if( outNALSize )	*outNALSize		= nalSize;
	if( outNext )		*outNext		= src;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	H264GetNextNALUnit
//===========================================================================================================================

OSStatus
	H264GetNextNALUnit( 
		const uint8_t *		inSrc, 
		const uint8_t *		inEnd, 
		size_t				inNALSize, 
		const uint8_t * *	outDataPtr, 
		size_t *			outDataLen,
		const uint8_t **	outSrc )
{
	OSStatus		err;
	size_t			len;
	
	require_action_quiet( inSrc != inEnd, exit, err = kEndingErr );
	require_action_quiet( ( (size_t)( inEnd - inSrc ) ) >= inNALSize, exit, err = kUnderrunErr );
	switch( inNALSize )
	{
		case 1:
			len = *inSrc++;
			break;
		
		case 2:
			len  = ( *inSrc++ << 8 );
			len |=   *inSrc++;
			break;
		
		case 4:
			len  = ( *inSrc++ << 24 );
			len |= ( *inSrc++ << 16 );
			len |= ( *inSrc++ <<  8 );
			len |=   *inSrc++;
			break;
		
		default:
			err = kParamErr;
			goto exit;
	}
	require_action_quiet( ( (size_t)( inEnd - inSrc ) ) >= len, exit, err = kUnderrunErr );
	
	*outDataPtr = inSrc;
	*outDataLen = len;
	*outSrc		= inSrc + len;
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#pragma mark == MirroredRingBuffer ==
#endif

#if( TARGET_OS_POSIX && !0 )
//===========================================================================================================================
//	MirroredRingBufferInit
//===========================================================================================================================

OSStatus	MirroredRingBufferInit( MirroredRingBuffer *inRing, size_t inMinSize, Boolean inPowerOf2 )
{
	char			path[] = "/tmp/MirrorRingBuffer-XXXXXX";
	OSStatus		err;
	long			pageSize;
	int				fd;
	size_t			len;
	uint8_t *		base = (uint8_t *) MAP_FAILED;
	uint8_t *		addr;
	
	// Create a temp file and remove it from the file system, but keep a file descriptor to it.
	
	fd = mkstemp( path );
	err = map_global_value_errno( fd >= 0, fd );
	require_noerr( err, exit );
	
	err = unlink( path );
	err = map_global_noerr_errno( err );
	check_noerr( err );
	
	// Resize the file to the size of the ring buffer.
	
	pageSize = sysconf( _SC_PAGE_SIZE );
	err = map_global_value_errno( pageSize > 0, pageSize );
	require_noerr( err, exit );
	len = RoundUp( inMinSize, (size_t) pageSize );
	if( inPowerOf2 ) len = iceil2( (uint32_t) len );
	err = ftruncate( fd, len );
	err = map_global_noerr_errno( err );
	require_noerr( err, exit );
	
	// Allocate memory for 2x the ring buffer size and map the two halves on top of each other.
	
	base = (uint8_t *) mmap( NULL, len * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 );
	err = map_global_value_errno( base != MAP_FAILED, base );
	require_noerr( err, exit );
	
	addr = (uint8_t *) mmap( base, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0 );
	err = map_global_value_errno( addr == base, addr );
	require_noerr( err, exit );
	
	addr = (uint8_t *) mmap( base + len, len, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0 );
	err = map_global_value_errno( addr == ( base + len ), addr );
	require_noerr( err, exit );
	
	// Success
	
	inRing->buffer		= base;
	inRing->end			= base + len;
	inRing->size		= (uint32_t) len;
	inRing->mask		= (uint32_t)( len - 1 );
	inRing->readOffset	= 0;
	inRing->writeOffset	= 0;
	base = (uint8_t *) MAP_FAILED;
	
exit:
	if( base != MAP_FAILED )	munmap( base, len * 2 );
	if( fd >= 0 )				close( fd );
	return( err );
}
#endif // TARGET_OS_POSIX && !0

#if( TARGET_OS_WINDOWS )
//===========================================================================================================================
//	MirroredRingBufferInit
//===========================================================================================================================

OSStatus	MirroredRingBufferInit( MirroredRingBuffer *inRing, size_t inMinSize, Boolean inPowerOf2 )
{
	OSStatus		err;
	SYSTEM_INFO		info;
	size_t			len;
	HANDLE			mapFile;
	uint8_t *		base = NULL;
	uint8_t *		addr;
	uint8_t *		addr2;
	int				tries;
	
	// Allocate a buffer 2x the size so we can remap the 2nd chunk to the 1st chunk.
	// If we're requiring a power-of-2 sized buffer then make sure the length is increased to the nearest power of 2.
	
	GetSystemInfo( &info );
	len = RoundUp( inMinSize, info.dwAllocationGranularity );
	if( inPowerOf2 ) len = iceil2( (uint32_t) len );
	
	mapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, len * 2, NULL );
	err = map_global_value_errno( mapFile, mapFile );
	require_noerr( err, exit );
	
	addr2 = NULL;
	for( tries = 1; tries < 100; ++tries )
	{
		// Map the whole thing to let the system find a logical address range then unmap it.
		
		base = MapViewOfFile( mapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, len * 2 );
		err = map_global_value_errno( base, base );
		require_noerr( err, exit );
		UnmapViewOfFile( base );
		
		// Now try to map to two logical address ranges to the same physical pages.
		// This may fail if another thread came in and stole that address so we'll have to try on failure.
		
		addr = MapViewOfFileEx( mapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, len, base );
		if( !addr ) continue;
		
		addr2 = MapViewOfFileEx( mapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, len, base + len );
		if( addr2 ) break;
		
		// Failed so unmap and try again.
		
		UnmapViewOfFile( addr );
	}
	require_action( addr2, exit, err = kNoMemoryErr );
	
	inRing->mapFile		= mapFile;
	inRing->buffer		= base;
	inRing->end			= base + len;
	inRing->size		= (uint32_t) len;
	inRing->mask		= (uint32_t)( len - 1 );
	inRing->readOffset	= 0;
	inRing->writeOffset	= 0;
	mapFile	= NULL;
	
exit:
	if( mapFile ) CloseHandle( mapFile );
	return( err );
}
#endif

//===========================================================================================================================
//	MirroredRingBufferFree
//===========================================================================================================================

#if( TARGET_OS_POSIX && !0 )
void	MirroredRingBufferFree( MirroredRingBuffer *inRing )
{
	OSStatus		err;
	
	if( inRing->buffer )
	{
		err = munmap( inRing->buffer, inRing->size * 2 );
		err = map_global_noerr_errno( err );
		check_noerr( err );
		inRing->buffer = NULL;
	}
	memset( inRing, 0, sizeof( *inRing ) );
}
#endif

#if( TARGET_OS_WINDOWS )
void	MirroredRingBufferFree( MirroredRingBuffer *inRing )
{
	OSStatus		err;
	BOOL			good;
	
	if( inRing->buffer )
	{
		good = UnmapViewOfFile( inRing->buffer );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		
		good = UnmapViewOfFile( inRing->buffer + inRing->size );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		
		inRing->buffer = NULL;
	}
	if( inRing->mapFile )
	{
		good = CloseHandle( inRing->mapFile );
		err = map_global_value_errno( good, good );
		check_noerr( err );
		inRing->mapFile = NULL;
	}
	memset( inRing, 0, sizeof( *inRing ) );
}
#endif

#if 0
#pragma mark -
#pragma mark == Morse Code ==
#endif

//===========================================================================================================================
//	MorseCode
//===========================================================================================================================

#define dit		kMorseCodeAction_Dit // 1 unit of time.
#define dah		kMorseCodeAction_Dah // 3 units of time.

typedef unsigned char		MorseSymbol[ 8 ];

static const MorseSymbol	kMorseCodeAlpha[] =
{
	{dit, dah},				// A
	{dah, dit, dit, dit},	// B
	{dah, dit, dah, dit},	// C
	{dah, dit, dit},		// D
	{dit},					// E
	{dit, dit, dah, dit},	// F
	{dah, dah, dit},		// G
	{dit, dit, dit, dit},	// H
	{dit, dit},				// I
	{dit, dah, dah, dah},	// J
	{dah, dit, dah},		// K
	{dit, dah, dit, dit},	// L
	{dah, dah},				// M
	{dah, dit},				// N
	{dah, dah, dah},		// O
	{dit, dah, dah, dit},	// P
	{dah, dah, dit, dah},	// Q
	{dit, dah, dit},		// R
	{dit, dit, dit},		// S
	{dah},					// T
	{dit, dit, dah},		// U
	{dit, dit, dit, dah},	// V
	{dit, dah, dah},		// W
	{dah, dit, dit, dah},	// X
	{dah, dit, dah, dah},	// Y
	{dah, dah, dit, dit}	// Z
};

static const MorseSymbol	kMorseCodeDigit[] =
{
	{dah, dah, dah, dah, dah},	// 0
	{dit, dah, dah, dah, dah}, 	// 1
	{dit, dit, dah, dah, dah}, 	// 2
	{dit, dit, dit, dah, dah}, 	// 3
	{dit, dit, dit, dit, dah}, 	// 4
	{dit, dit, dit, dit, dit}, 	// 5
	{dah, dit, dit, dit, dit}, 	// 6
	{dah, dah, dit, dit, dit}, 	// 7
	{dah, dah, dah, dit, dit}, 	// 8
	{dah, dah, dah, dah, dit}, 	// 9
};

static const MorseSymbol	kMorseCodePunct[] =
{
	{dit, dah, dit, dit, dah, dit}, 	// "
	{dit, dah, dah, dah, dah, dit},		// '
	{dah, dit, dah, dah, dit}, 			// (
	{dah, dit, dah, dah, dit, dah}, 	// )
	{dah, dah, dit, dit, dah, dah},		// ,
	{dah, dit, dit, dit, dit, dah},		// -
	{dit, dah, dit, dah, dit, dah}, 	// .
	{dah, dit, dit, dah, dit},			// /
	{dah, dah, dah, dit, dit, dit},		// :
	{dit, dit, dah, dah, dit, dit}, 	// ?
	{dit, dah, dah, dit, dah, dit},		// @
	{dah, dit, dah, dit, dah, dah},		// !
	{dit, dah, dit, dit, dit},			// &
	{dah, dit, dah, dit, dah, dit},		// ;
	{dah, dit, dit, dit, dah},			// =
	{dit, dah, dit, dah, dah},			// +
	{dit, dit, dah, dah, dit, dah},		// _
	{dit, dit, dit, dah, dit, dit, dah}	// $
};

typedef struct
{
	MorseCodeFlags			flags;
	MorseCodeActionFunc		actionFunc;
	void *					actionArg;
	unsigned int			unitMics;
	
}	MorseCodeContext;

static void	_MorseCodeDoChar( MorseCodeContext *inContext, const unsigned char *inSymbols );
static void	_MorseCodeAction( MorseCodeContext *inContext, MorseCodeAction inAction );

//===========================================================================================================================
//	MorseCode
//===========================================================================================================================

void
	MorseCode( 
		const char *		inMessage, 
		int					inSpeed, 
		MorseCodeFlags		inFlags, 
		MorseCodeActionFunc inActionFunc, 
		void *				inActionArg )
{
	MorseCodeContext		context;
	char					c;
	
	context.flags		= inFlags;
	context.actionFunc	= inActionFunc;
	context.actionArg	= inActionArg;
	
	if( inSpeed == 0 )	inSpeed  = 10; // Default to 10 words per minute.
	if( inSpeed  > 0 )	context.unitMics = (unsigned int)( ( 1200 * 1000 ) /  inSpeed ); // > 0 means words per minute
	else				context.unitMics = (unsigned int)( ( 6000 * 1000 ) / -inSpeed ); // < 0 means characters per minute (negated).
	
	if( !( inFlags & kMorseCodeFlags_RawActions ) )
	{
		_MorseCodeAction( &context, kMorseCodeAction_Off ); // Start with it off.
		_MorseCodeAction( &context, kMorseCodeAction_WordDelay );
	}
	while( ( c = *inMessage++ ) != '\0' )
	{
		if(      isalpha_safe( c ) )	_MorseCodeDoChar( &context, kMorseCodeAlpha[ toupper( c ) - 'A' ] );
		else if( isdigit_safe( c ) )	_MorseCodeDoChar( &context, kMorseCodeDigit[ c - '0' ] );
		else if( c == '"' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 0 ] );
		else if( c == '\'' )			_MorseCodeDoChar( &context, kMorseCodePunct[ 1 ] );
		else if( c == '(' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 2 ] );
		else if( c == ')' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 3 ] );
		else if( c == ',' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 4 ] );
		else if( c == '-' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 5 ] );
		else if( c == '.' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 6 ] );
		else if( c == '/' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 7 ] );
		else if( c == ':' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 8 ] );
		else if( c == '?' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 9 ] );
		else if( c == '@' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 10 ] );
		else if( c == '!' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 11 ] );
		else if( c == '&' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 12 ] );
		else if( c == ';' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 13 ] );
		else if( c == '=' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 14 ] );
		else if( c == '+' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 15 ] );
		else if( c == '_' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 16 ] );
		else if( c == '$' )				_MorseCodeDoChar( &context, kMorseCodePunct[ 17 ] );
		else if( isspace_safe( c ) )	_MorseCodeAction( &context, kMorseCodeAction_WordDelay );
		else dlog( kLogLevelWarning, "### bad morse code letter '%c' (0x%02x)\n", c, c );
		
		if( ( *inMessage != '\0' ) && !isspace_safe( c ) )
		{
			_MorseCodeAction( &context, kMorseCodeAction_CharDelay );
		}
	}
}

//===========================================================================================================================
//	_MorseCodeDoChar
//===========================================================================================================================

static void	_MorseCodeDoChar( MorseCodeContext *inContext, const unsigned char *inSymbols )
{
	unsigned char		symbol;
	
	while( ( symbol = *inSymbols++ ) != 0 )
	{
		_MorseCodeAction( inContext, symbol );
		if( *inSymbols != 0 ) _MorseCodeAction( inContext, kMorseCodeAction_MarkDelay );
	}
}

//===========================================================================================================================
//	_MorseCodeAction
//===========================================================================================================================

static void	_MorseCodeAction( MorseCodeContext *inContext, MorseCodeAction inAction )
{
	if( inContext->flags & kMorseCodeFlags_RawActions )
	{
		inContext->actionFunc( inAction, inContext->actionArg );
	}
	else
	{
		if( inAction == kMorseCodeAction_Dit )
		{
			inContext->actionFunc( kMorseCodeAction_On, inContext->actionArg );
			usleep( 1 * inContext->unitMics );
			inContext->actionFunc( kMorseCodeAction_Off, inContext->actionArg );
		}
		else if( inAction == kMorseCodeAction_Dah )
		{
			inContext->actionFunc( kMorseCodeAction_On, inContext->actionArg );
			usleep( 3 * inContext->unitMics );
			inContext->actionFunc( kMorseCodeAction_Off, inContext->actionArg );
		}
		else if( inAction == kMorseCodeAction_MarkDelay )	usleep( 1 * inContext->unitMics );
		else if( inAction == kMorseCodeAction_CharDelay )	usleep( 3 * inContext->unitMics );
		else if( inAction == kMorseCodeAction_WordDelay )	usleep( 7 * inContext->unitMics );
		else if( inAction == kMorseCodeAction_Off )			inContext->actionFunc( inAction, inContext->actionArg );
		else if( inAction == kMorseCodeAction_On )			inContext->actionFunc( inAction, inContext->actionArg );
		else dlogassert( "BUG: bad action %d", inAction );
	}
}

#if 0
#pragma mark -
#pragma mark == PID Controller ==
#endif

//===========================================================================================================================
//	PIDInit
//===========================================================================================================================

void	PIDInit( PIDContext *inPID, double pGain, double iGain, double dGain, double dPole, double iMin, double iMax )
{
	inPID->iState	= 0.0;
	inPID->iMax		= iMax;
	inPID->iMin		= iMin;
	inPID->iGain	= iGain;
	
	inPID->dState	= 0.0;
	inPID->dpGain	= 1.0 - dPole;
	inPID->dGain	= dGain * ( 1.0 - dPole );
	
	inPID->pGain	= pGain;
}

//===========================================================================================================================
//	PIDUpdate
//===========================================================================================================================

double	PIDUpdate( PIDContext *inPID, double input )
{
	double		output;
	double		dTemp;
	double		pTerm;
	
	pTerm = input * inPID->pGain; // Proportional.
	
	// Update the integrator state and limit it.
	
	inPID->iState = inPID->iState + ( inPID->iGain * input );
	output = inPID->iState + pTerm;
	if( output > inPID->iMax )
	{
		inPID->iState = inPID->iMax - pTerm;
		output = inPID->iMax;
	}
	else if( output < inPID->iMin )
	{
		inPID->iState = inPID->iMin - pTerm;
		output = inPID->iMin;
	}
	
	// Update the differentiator state.
	
	dTemp = input - inPID->dState;
	inPID->dState += inPID->dpGain * dTemp;
	output += dTemp * inPID->dGain;
	return( output );
}

#if 0
#pragma mark -
#pragma mark == poly1305 ==
#endif

//===========================================================================================================================
//	poly1305_auth
//
//	Port of floodyberry's Poly1305 code: <https://github.com/floodyberry/poly1305-donna>.
//	Based on DJB's Poly1305: <http://cr.yp.to/mac.html>.
//===========================================================================================================================

#define U8TO64_LE( PTR )			ReadLittle64( (PTR) )
#define U64TO8_LE( PTR, VALUE )		WriteLittle64( (PTR), (VALUE) )

#define U8TO32_LE( PTR )			ReadLittle32( (PTR) )
#define U32TO8_LE( PTR, VALUE )		WriteLittle32( (PTR), (VALUE) )

#if( TARGET_RT_64_BIT )

#define mul64x64_128(uint128_out, uint64_a, uint64_b) \
	uint128_out = ((uint128_t)(uint64_a) * (uint64_b))

#define shr128_pair(uint64_hi, uint64_lo, int_shift) \
	(uint64_t)((((uint128_t)(uint64_hi) << 64) | (uint64_lo)) >> (int_shift))

#define shr128(uint128_in, int_shift) \
	(uint64_t)((uint128_in) >> (int_shift))

#define add128(uint128_out, uint128_in) {   \
	uint128_out += (uint128_in);            \
}

#define add128_64(uint128_out, uint64_in) { \
	uint128_out += (uint64_in);             \
}

#define lo128(uint128_in) \
	(uint64_t)(uint128_in)

#define hi128(uint128_in) \
	(uint64_t)((uint128_in) >> 64)

void
poly1305_auth(unsigned char out[16], const unsigned char *m, size_t inlen, const unsigned char key[32]) {
	uint64_t t0,t1;
	uint64_t h0,h1,h2;
	uint64_t r0,r1,r2;
	uint64_t g0,g1,g2,c,nc;
	uint64_t s1,s2;
	size_t j;
	uint128_t d[3],m0,m1;
	unsigned char mp[16] = {0};

	/* clamp key */
	t0 = U8TO64_LE(&key[0]);
	t1 = U8TO64_LE(&key[8]);

	/* pre-compute multipliers */
	r0 = t0 & 0xffc0fffffff; t0 >>= 44; t0 |= t1 << 20;
	r1 = t0 & 0xfffffc0ffff; t1 >>= 24;
	r2 = t1 & 0x00ffffffc0f;

	s1 = r1 * (5 << 2);
	s2 = r2 * (5 << 2);

	/* state */
	h0 = 0;
	h1 = 0;
	h2 = 0;

	/* full blocks */
	if (inlen < 16) goto poly1305_donna_atmost15bytes;
	if (inlen < 64) goto poly1305_donna_atmost63bytes;

	/* macros */
	#define multiply_by_r_and_partial_reduce() \
		mul64x64_128(d[0], h0, r0); mul64x64_128(m0, h1, s2);  mul64x64_128(m1, h2, s1); add128(d[0], m0); add128(d[0], m1); \
		mul64x64_128(d[1], h0, r1); mul64x64_128(m0, h1, r0);  mul64x64_128(m1, h2, s2); add128(d[1], m0); add128(d[1], m1); \
		mul64x64_128(d[2], h0, r2); mul64x64_128(m0, h1, r1);  mul64x64_128(m1, h2, r0); add128(d[2], m0); add128(d[2], m1); \
		                    h0 = lo128(d[0]) & 0xfffffffffff; c = shr128(d[0], 44); \
		add128_64(d[1], c); h1 = lo128(d[1]) & 0xfffffffffff; c = shr128(d[1], 44); \
		add128_64(d[2], c); h2 = lo128(d[2]) & 0x3ffffffffff; c = shr128(d[2], 42); \
		h0   += c * 5;

	#define do_block(offset) \
		/* h += ((1 << 128) + m) */             \
		t0 = U8TO64_LE(m+offset);               \
		t1 = U8TO64_LE(m+offset+8);             \
		h0 += t0 & 0xfffffffffff;               \
		t0 = shr128_pair(t1, t0, 44);           \
		h1 += t0 & 0xfffffffffff;               \
		h2 += (t1 >> 24) | ((uint64_t)1 << 40); \
		/* h = (h * r) % ((1 << 130) - 5) */    \
		multiply_by_r_and_partial_reduce()

/* 4 blocks */
poly1305_donna_64bytes:
	do_block(0)
	do_block(16)
	do_block(32)
	do_block(48)

	m += 64;
	inlen -= 64;

	if (inlen >= 64) goto poly1305_donna_64bytes;
	if (inlen < 16) goto poly1305_donna_atmost15bytes;

/* 1 block */
poly1305_donna_atmost63bytes:
	do_block(0)
	m += 16;
	inlen -= 16;

	if (inlen >= 16) goto poly1305_donna_atmost63bytes;

/* partial block */
poly1305_donna_atmost15bytes:
	if (!inlen) goto poly1305_donna_finish;

	for (j = 0; j < inlen; j++) mp[j] = m[j];
	mp[j++] = 1;
	for (; j < 16; j++)	mp[j] = 0;

	t0 = U8TO64_LE(mp+0);
	t1 = U8TO64_LE(mp+8);
	h0 += t0 & 0xfffffffffff;
	t0 = shr128_pair(t1, t0, 44);
	h1 += t0 & 0xfffffffffff;
	h2 += (t1 >> 24);

	multiply_by_r_and_partial_reduce()

/* finish */
poly1305_donna_finish:
	             c = (h0 >> 44); h0 &= 0xfffffffffff;
	h1 += c    ; c = (h1 >> 44); h1 &= 0xfffffffffff;
	h2 += c    ; c = (h2 >> 42); h2 &= 0x3ffffffffff;
	h0 += c * 5; c = (h0 >> 44); h0 &= 0xfffffffffff; 
	h1 += c    ;

	g0 = h0 + 5; c = (g0 >> 44); g0 &= 0xfffffffffff;
	g1 = h1 + c; c = (g1 >> 44); g1 &= 0xfffffffffff;
	g2 = h2 + c - ((uint64_t)1 << 42);

	c = (g2 >> 63) - 1;
	nc = ~c;
	h0 = (h0 & nc) | (g0 & c);
	h1 = (h1 & nc) | (g1 & c);
	h2 = (h2 & nc) | (g2 & c);

	t0 = U8TO64_LE(key+16);
	t1 = U8TO64_LE(key+24);
	h0 += (t0 & 0xfffffffffff)    ; c = (h0 >> 44); h0 &= 0xfffffffffff;
	t0 = shr128_pair(t1, t0, 44);
	h1 += (t0 & 0xfffffffffff) + c; c = (h1 >> 44); h1 &= 0xfffffffffff;
	h2 += (t1 >> 24          ) + c;

	U64TO8_LE(&out[ 0], ((h0      ) | (h1 << 44)));
	U64TO8_LE(&out[ 8], ((h1 >> 20) | (h2 << 24)));

	#undef multiply_by_r_and_partial_reduce
	#undef do_block
}
#else // ! 64-bit
#define mul32x32_64(a,b) ((uint64_t)(a) * (b))

void
poly1305_auth(unsigned char out[16], const unsigned char *m, size_t inlen, const unsigned char key[32]) {
	uint32_t t0,t1,t2,t3;
	uint32_t h0,h1,h2,h3,h4;
	uint32_t r0,r1,r2,r3,r4;
	uint32_t s1,s2,s3,s4;
	uint32_t b, nb;
	size_t j;
	uint64_t t[5];
	uint64_t f0,f1,f2,f3;
	uint32_t g0,g1,g2,g3,g4;
	uint64_t c;
	unsigned char mp[16];

	/* clamp key */
	t0 = U8TO32_LE(key+0);
	t1 = U8TO32_LE(key+4);
	t2 = U8TO32_LE(key+8);
	t3 = U8TO32_LE(key+12);

	/* precompute multipliers */
	r0 = t0 & 0x3ffffff; t0 >>= 26; t0 |= t1 << 6;
	r1 = t0 & 0x3ffff03; t1 >>= 20; t1 |= t2 << 12;
	r2 = t1 & 0x3ffc0ff; t2 >>= 14; t2 |= t3 << 18;
	r3 = t2 & 0x3f03fff; t3 >>= 8;
	r4 = t3 & 0x00fffff;

	s1 = r1 * 5;
	s2 = r2 * 5;
	s3 = r3 * 5;
	s4 = r4 * 5;

	/* init state */
	h0 = 0;
	h1 = 0;
	h2 = 0;
	h3 = 0;
	h4 = 0;

	/* full blocks */
	if (inlen < 16) goto poly1305_donna_atmost15bytes;
poly1305_donna_16bytes:
	m += 16;
	inlen -= 16;

	t0 = U8TO32_LE(m-16);
	t1 = U8TO32_LE(m-12);
	t2 = U8TO32_LE(m-8);
	t3 = U8TO32_LE(m-4);

	h0 += t0 & 0x3ffffff;
	h1 += ((((uint64_t)t1 << 32) | t0) >> 26) & 0x3ffffff;
	h2 += ((((uint64_t)t2 << 32) | t1) >> 20) & 0x3ffffff;
	h3 += ((((uint64_t)t3 << 32) | t2) >> 14) & 0x3ffffff;
	h4 += (t3 >> 8) | (1 << 24);


poly1305_donna_mul:
	t[0]  = mul32x32_64(h0,r0) + mul32x32_64(h1,s4) + mul32x32_64(h2,s3) + mul32x32_64(h3,s2) + mul32x32_64(h4,s1);
	t[1]  = mul32x32_64(h0,r1) + mul32x32_64(h1,r0) + mul32x32_64(h2,s4) + mul32x32_64(h3,s3) + mul32x32_64(h4,s2);
	t[2]  = mul32x32_64(h0,r2) + mul32x32_64(h1,r1) + mul32x32_64(h2,r0) + mul32x32_64(h3,s4) + mul32x32_64(h4,s3);
	t[3]  = mul32x32_64(h0,r3) + mul32x32_64(h1,r2) + mul32x32_64(h2,r1) + mul32x32_64(h3,r0) + mul32x32_64(h4,s4);
	t[4]  = mul32x32_64(h0,r4) + mul32x32_64(h1,r3) + mul32x32_64(h2,r2) + mul32x32_64(h3,r1) + mul32x32_64(h4,r0);

	                h0 = (uint32_t)t[0] & 0x3ffffff; c =           (t[0] >> 26);
	t[1] += c;      h1 = (uint32_t)t[1] & 0x3ffffff; b = (uint32_t)(t[1] >> 26);
	t[2] += b;      h2 = (uint32_t)t[2] & 0x3ffffff; b = (uint32_t)(t[2] >> 26);
	t[3] += b;      h3 = (uint32_t)t[3] & 0x3ffffff; b = (uint32_t)(t[3] >> 26);
	t[4] += b;      h4 = (uint32_t)t[4] & 0x3ffffff; b = (uint32_t)(t[4] >> 26);
	h0 += b * 5;

	if (inlen >= 16) goto poly1305_donna_16bytes;

	/* final bytes */
poly1305_donna_atmost15bytes:
	if (!inlen) goto poly1305_donna_finish;

	for (j = 0; j < inlen; j++) mp[j] = m[j];
	mp[j++] = 1;
	for (; j < 16; j++)	mp[j] = 0;
	inlen = 0;

	t0 = U8TO32_LE(mp+0);
	t1 = U8TO32_LE(mp+4);
	t2 = U8TO32_LE(mp+8);
	t3 = U8TO32_LE(mp+12);

	h0 += t0 & 0x3ffffff;
	h1 += ((((uint64_t)t1 << 32) | t0) >> 26) & 0x3ffffff;
	h2 += ((((uint64_t)t2 << 32) | t1) >> 20) & 0x3ffffff;
	h3 += ((((uint64_t)t3 << 32) | t2) >> 14) & 0x3ffffff;
	h4 += (t3 >> 8);

	goto poly1305_donna_mul;

poly1305_donna_finish:
	             b = h0 >> 26; h0 = h0 & 0x3ffffff;
	h1 +=     b; b = h1 >> 26; h1 = h1 & 0x3ffffff;
	h2 +=     b; b = h2 >> 26; h2 = h2 & 0x3ffffff;
	h3 +=     b; b = h3 >> 26; h3 = h3 & 0x3ffffff;
	h4 +=     b; b = h4 >> 26; h4 = h4 & 0x3ffffff;
	h0 += b * 5; b = h0 >> 26; h0 = h0 & 0x3ffffff;
    h1 +=     b;

	g0 = h0 + 5; b = g0 >> 26; g0 &= 0x3ffffff;
	g1 = h1 + b; b = g1 >> 26; g1 &= 0x3ffffff;
	g2 = h2 + b; b = g2 >> 26; g2 &= 0x3ffffff;
	g3 = h3 + b; b = g3 >> 26; g3 &= 0x3ffffff;
	g4 = h4 + b - (1 << 26);

	b = (g4 >> 31) - 1;
	nb = ~b;
	h0 = (h0 & nb) | (g0 & b);
	h1 = (h1 & nb) | (g1 & b);
	h2 = (h2 & nb) | (g2 & b);
	h3 = (h3 & nb) | (g3 & b);
	h4 = (h4 & nb) | (g4 & b);

	f0 = ((h0      ) | (h1 << 26)) + (uint64_t)U8TO32_LE(&key[16]);
	f1 = ((h1 >>  6) | (h2 << 20)) + (uint64_t)U8TO32_LE(&key[20]);
	f2 = ((h2 >> 12) | (h3 << 14)) + (uint64_t)U8TO32_LE(&key[24]);
	f3 = ((h3 >> 18) | (h4 <<  8)) + (uint64_t)U8TO32_LE(&key[28]);

	U32TO8_LE(&out[ 0], f0); f1 += (f0 >> 32);
	U32TO8_LE(&out[ 4], f1); f2 += (f1 >> 32);
	U32TO8_LE(&out[ 8], f2); f3 += (f2 >> 32);
	U32TO8_LE(&out[12], f3);
}
#endif // Else of TARGET_RT_64_BIT

#if 0
#pragma mark -
#pragma mark == XSalsa20-Poly1305 ==
#endif

//===========================================================================================================================
//	XSalsa20-Poly1305 authenticated encryption
//===========================================================================================================================

// "expand 32-byte k", as 4 little endian 32-bit unsigned integers.
static const uint32_t		kSalsa20Constants[ 4 ] = { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 };

static void crypto_core_hsalsa20(uint8_t *out, const uint8_t *in, const uint8_t *k);
static void crypto_core_salsa20(uint8_t *out, const uint8_t *in, const uint8_t *k);

//===========================================================================================================================
//	crypto_core_hsalsa20
//
//	out = 32-byte output
//	in  = 16-byte input
//	k   = 32-byte key
//===========================================================================================================================

static void crypto_core_hsalsa20(uint8_t *out, const uint8_t *in, const uint8_t *k)
{
  uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  int i;

  x0 = kSalsa20Constants[0];
  x1 = ReadLittle32(k + 0);
  x2 = ReadLittle32(k + 4);
  x3 = ReadLittle32(k + 8);
  x4 = ReadLittle32(k + 12);
  x5 = kSalsa20Constants[1];
  x6 = ReadLittle32(in + 0);
  x7 = ReadLittle32(in + 4);
  x8 = ReadLittle32(in + 8);
  x9 = ReadLittle32(in + 12);
  x10 = kSalsa20Constants[2];
  x11 = ReadLittle32(k + 16);
  x12 = ReadLittle32(k + 20);
  x13 = ReadLittle32(k + 24);
  x14 = ReadLittle32(k + 28);
  x15 = kSalsa20Constants[3];

  for (i = 20;i > 0;i -= 2) {
     x4 ^= ROTL32( x0+x12, 7);
     x8 ^= ROTL32( x4+ x0, 9);
    x12 ^= ROTL32( x8+ x4,13);
     x0 ^= ROTL32(x12+ x8,18);
     x9 ^= ROTL32( x5+ x1, 7);
    x13 ^= ROTL32( x9+ x5, 9);
     x1 ^= ROTL32(x13+ x9,13);
     x5 ^= ROTL32( x1+x13,18);
    x14 ^= ROTL32(x10+ x6, 7);
     x2 ^= ROTL32(x14+x10, 9);
     x6 ^= ROTL32( x2+x14,13);
    x10 ^= ROTL32( x6+ x2,18);
     x3 ^= ROTL32(x15+x11, 7);
     x7 ^= ROTL32( x3+x15, 9);
    x11 ^= ROTL32( x7+ x3,13);
    x15 ^= ROTL32(x11+ x7,18);
     x1 ^= ROTL32( x0+ x3, 7);
     x2 ^= ROTL32( x1+ x0, 9);
     x3 ^= ROTL32( x2+ x1,13);
     x0 ^= ROTL32( x3+ x2,18);
     x6 ^= ROTL32( x5+ x4, 7);
     x7 ^= ROTL32( x6+ x5, 9);
     x4 ^= ROTL32( x7+ x6,13);
     x5 ^= ROTL32( x4+ x7,18);
    x11 ^= ROTL32(x10+ x9, 7);
     x8 ^= ROTL32(x11+x10, 9);
     x9 ^= ROTL32( x8+x11,13);
    x10 ^= ROTL32( x9+ x8,18);
    x12 ^= ROTL32(x15+x14, 7);
    x13 ^= ROTL32(x12+x15, 9);
    x14 ^= ROTL32(x13+x12,13);
    x15 ^= ROTL32(x14+x13,18);
  }

  WriteLittle32(out + 0,x0);
  WriteLittle32(out + 4,x5);
  WriteLittle32(out + 8,x10);
  WriteLittle32(out + 12,x15);
  WriteLittle32(out + 16,x6);
  WriteLittle32(out + 20,x7);
  WriteLittle32(out + 24,x8);
  WriteLittle32(out + 28,x9);
}

//===========================================================================================================================
//	crypto_core_salsa20
//
//	out = 64-byte output
//	in  = 16-byte input
//	k   = 32-byte key
//===========================================================================================================================

static void crypto_core_salsa20(uint8_t *out, const uint8_t *in, const uint8_t *k)
{
  uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  uint32_t j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
  int i;

  j0 = x0 = kSalsa20Constants[0];
  j1 = x1 = ReadLittle32(k + 0);
  j2 = x2 = ReadLittle32(k + 4);
  j3 = x3 = ReadLittle32(k + 8);
  j4 = x4 = ReadLittle32(k + 12);
  j5 = x5 = kSalsa20Constants[1];
  j6 = x6 = ReadLittle32(in + 0);
  j7 = x7 = ReadLittle32(in + 4);
  j8 = x8 = ReadLittle32(in + 8);
  j9 = x9 = ReadLittle32(in + 12);
  j10 = x10 = kSalsa20Constants[2];
  j11 = x11 = ReadLittle32(k + 16);
  j12 = x12 = ReadLittle32(k + 20);
  j13 = x13 = ReadLittle32(k + 24);
  j14 = x14 = ReadLittle32(k + 28);
  j15 = x15 = kSalsa20Constants[3];

  for (i = 20;i > 0;i -= 2) {
     x4 ^= ROTL32( x0+x12, 7);
     x8 ^= ROTL32( x4+ x0, 9);
    x12 ^= ROTL32( x8+ x4,13);
     x0 ^= ROTL32(x12+ x8,18);
     x9 ^= ROTL32( x5+ x1, 7);
    x13 ^= ROTL32( x9+ x5, 9);
     x1 ^= ROTL32(x13+ x9,13);
     x5 ^= ROTL32( x1+x13,18);
    x14 ^= ROTL32(x10+ x6, 7);
     x2 ^= ROTL32(x14+x10, 9);
     x6 ^= ROTL32( x2+x14,13);
    x10 ^= ROTL32( x6+ x2,18);
     x3 ^= ROTL32(x15+x11, 7);
     x7 ^= ROTL32( x3+x15, 9);
    x11 ^= ROTL32( x7+ x3,13);
    x15 ^= ROTL32(x11+ x7,18);
     x1 ^= ROTL32( x0+ x3, 7);
     x2 ^= ROTL32( x1+ x0, 9);
     x3 ^= ROTL32( x2+ x1,13);
     x0 ^= ROTL32( x3+ x2,18);
     x6 ^= ROTL32( x5+ x4, 7);
     x7 ^= ROTL32( x6+ x5, 9);
     x4 ^= ROTL32( x7+ x6,13);
     x5 ^= ROTL32( x4+ x7,18);
    x11 ^= ROTL32(x10+ x9, 7);
     x8 ^= ROTL32(x11+x10, 9);
     x9 ^= ROTL32( x8+x11,13);
    x10 ^= ROTL32( x9+ x8,18);
    x12 ^= ROTL32(x15+x14, 7);
    x13 ^= ROTL32(x12+x15, 9);
    x14 ^= ROTL32(x13+x12,13);
    x15 ^= ROTL32(x14+x13,18);
  }

  x0 += j0;
  x1 += j1;
  x2 += j2;
  x3 += j3;
  x4 += j4;
  x5 += j5;
  x6 += j6;
  x7 += j7;
  x8 += j8;
  x9 += j9;
  x10 += j10;
  x11 += j11;
  x12 += j12;
  x13 += j13;
  x14 += j14;
  x15 += j15;

  WriteLittle32(out + 0,x0);
  WriteLittle32(out + 4,x1);
  WriteLittle32(out + 8,x2);
  WriteLittle32(out + 12,x3);
  WriteLittle32(out + 16,x4);
  WriteLittle32(out + 20,x5);
  WriteLittle32(out + 24,x6);
  WriteLittle32(out + 28,x7);
  WriteLittle32(out + 32,x8);
  WriteLittle32(out + 36,x9);
  WriteLittle32(out + 40,x10);
  WriteLittle32(out + 44,x11);
  WriteLittle32(out + 48,x12);
  WriteLittle32(out + 52,x13);
  WriteLittle32(out + 56,x14);
  WriteLittle32(out + 60,x15);
}

//===========================================================================================================================
//	crypto_xsalsa20_poly1305_encrypt
//	
//	ct	= len-byte ciphertext output
//	mac	= 16-byte message authentication code output
//	m	= len-byte plaintext input
//	len	= number of input bytes
//	n	= 24-byte nonce
//	k	= 32-byte key
//===========================================================================================================================

void crypto_xsalsa20_poly1305_encrypt(uint8_t *ct,uint8_t *mac,const uint8_t *m,size_t len,const uint8_t *n,const uint8_t *k)
{
  uint8_t *c = ct;
  size_t mlen = len;
  uint8_t subkey[32];
  uint8_t in[16];
  uint8_t firstblock[64];
  const uint8_t *p;
  uint8_t block[64];
  size_t i;
  unsigned int u;

  // Expand the first 16 bytes of the nonce to form a new 32-byte key for XSalsa20.
  // Use the remaning 8 bytes of the nonce as the Salsa20 nonce.
  // For details, see "Extending the Salsa20 nonce" <http://cr.yp.to/snuffle/xsalsa-20081128.pdf>.
  crypto_core_hsalsa20(subkey,n,k);
  n += 16;
  for (i = 0;i < 8;++i) in[i] = n[i];
  for (i = 8;i < 16;++i) in[i] = 0;

  // Use 1st 32 bytes of first block for poly1305 key. 
  // Use 2nd 32 bytes of first block to encrypt first 32 bytes of message.
  if (mlen) {
    crypto_core_salsa20(firstblock,in,subkey);
    p = firstblock + 32;
    for (i = 0;(i < 32) && (i < mlen);++i) c[i] = m[i] ^ p[i];
    mlen -= i;
    c += i;
    m += i;
    u = 1;
    for (i = 8;i < 16;++i) {
      u += (unsigned int) in[i];
      in[i] = (uint8_t) u;
      u >>= 8;
    }
  }

  // Encrypt remaining data normally.
  while (mlen >= 64) {
    crypto_core_salsa20(block,in,subkey);
    for (i = 0;i < 64;++i) c[i] = m[i] ^ block[i];

    u = 1;
    for (i = 8;i < 16;++i) {
      u += (unsigned int) in[i];
      in[i] = (uint8_t) u;
      u >>= 8;
    }

    mlen -= 64;
    c += 64;
    m += 64;
  }
  if (mlen) {
    crypto_core_salsa20(block,in,subkey);
    for (i = 0;i < mlen;++i) c[i] = m[i] ^ block[i];
  }

  poly1305_auth(mac,ct,len,firstblock);
}

//===========================================================================================================================
//	crypto_xsalsa20_poly1305_decrypt
//	
//	pt	= mlen-byte platintext output
//	ct	= mlen-byte ciphertext input
//	len	= number of input bytes
//	mac	= 16-byte message authentication code
//	n	= 24-byte nonce
//	k	= 32-byte key
//===========================================================================================================================

int crypto_xsalsa20_poly1305_decrypt(uint8_t *pt,const uint8_t *ct,size_t len,const uint8_t *mac,const uint8_t *n,const uint8_t *k)
{
  uint8_t subkey[32];
  uint8_t firstblock[64];
  uint8_t block[64];
  const uint8_t *p;
  uint8_t in[16];
  size_t i;
  uint8_t mac2[16];
  unsigned int u;

  // Expand the first 16 bytes of the nonce to form a new 32-byte key for XSalsa20.
  // Use the remaning 8 bytes of the nonce as the Salsa20 nonce.
  // For details, see "Extending the Salsa20 nonce" <http://cr.yp.to/snuffle/xsalsa-20081128.pdf>.
  crypto_core_hsalsa20(subkey,n,k);
  p = n + 16;
  for (i = 0;i < 8;++i) in[i] = p[i];
  for (i = 8;i < 16;++i) in[i] = 0;

  // Use 1st 32 bytes of first block for poly1305 key. Verify the message with it.
  crypto_core_salsa20(firstblock,in,subkey);
  poly1305_auth(mac2,ct,len,firstblock);
  if (memcmp_constant_time(mac2,mac,16) != 0) return -1;
  if (!len) return 0;

  // Use 2nd 32 bytes of first block to decrypt first 32 bytes of message.
  p = firstblock + 32;
  for (i = 0;(i < 32) && (i < len);++i) pt[i] = ct[i] ^ p[i];
  len -= i;
  pt += i;
  ct += i;
  u = 1;
  for (i = 8;i < 16;++i) {
    u += (unsigned int) in[i];
    in[i] = (uint8_t) u;
    u >>= 8;
  }

  // Decrypt remaining data normally.
  while (len >= 64) {
    crypto_core_salsa20(block,in,subkey);
    for (i = 0;i < 64;++i) pt[i] = ct[i] ^ block[i];

    u = 1;
    for (i = 8;i < 16;++i) {
      u += (unsigned int) in[i];
      in[i] = (uint8_t) u;
      u >>= 8;
    }

    len -= 64;
    pt += 64;
    ct += 64;
  }
  if (len) {
    crypto_core_salsa20(block,in,subkey);
    for (i = 0;i < len;++i) pt[i] = ct[i] ^ block[i];
  }
  return 0;
}

#if 0
#pragma mark -
#pragma mark == Security ==
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

