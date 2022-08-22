/*
	File:    	MiscUtils.h
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

#ifndef	__MiscUtils_h__
#define	__MiscUtils_h__

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#if( TARGET_HAS_STD_C_LIB )
	#include <stdarg.h>
	#include <stddef.h>
	#include <stdlib.h>
#endif

#if( 0 || TARGET_OS_POSIX )
	#include CF_HEADER
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#pragma mark == FramesPerSecond ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		FramesPerSecond Statistics
	@abstract	Tracks an exponential moving average frames per second.
*/
typedef struct
{
	double			smoothingFactor;
	double			ticksPerSecF;
	uint64_t		periodTicks;
	uint64_t		lastTicks;
	uint32_t		totalFrameCount;
	uint32_t		lastFrameCount;
	double			lastFPS;
	double			averageFPS;
	
}	FPSData;

void	FPSInit( FPSData *inData, int inPeriods );
void	FPSUpdate( FPSData *inData, uint32_t inFrameCount );

#if 0
#pragma mark == Misc ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	QSortPtrs
	@abstract	Sorts an array of pointers according to a comparison function.
*/
typedef int ( *ComparePtrsFunctionPtr )( const void *inLeft, const void *inRight, void *inContext );

void	QSortPtrs( void *inPtrArray, size_t inPtrCount, ComparePtrsFunctionPtr inCmp, void *inContext );
int		CompareIntPtrs( const void *inLeft, const void *inRight, void *inContext );
int		CompareStringPtrs( const void *inLeft, const void *inRight, void *inContext );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	MemReverse
	@abstract	Copies data from one block to another and reverses the order of bytes in the process.
	@discussion	"inSrc" may be the same as "inDst", but may not point to an arbitrary location inside it.
*/
void	MemReverse( const void *inSrc, size_t inLen, void *inDst );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	Swap16Mem
	@abstract	Endian swaps each 16-bit chunk of memory.
	@discussion	"inSrc" may be the same as "inDst", but may not point to an arbitrary location inside it.
*/
void	Swap16Mem( const void *inSrc, size_t inLen, void *inDst );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	SwapUUID
	@abstract	Endian swaps a UUID to convert it between big and little endian.
	@discussion	"inSrc" may be the same as "inDst", but may not point to an arbitrary location inside it.
*/
void	SwapUUID( const void *inSrc, void *inDst );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	CopySmallFile
	@abstract	Copies a file from one path to another. Only intended for copying small files.
*/
#if( TARGET_HAS_STD_C_LIB )
	OSStatus	CopySmallFile( const char *inSrcPath, const char *inDstPath );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	CopyFileDataByFile
	@abstract	Reads all the data in the file into a malloc'd pointer and null terminates it.
*/
OSStatus	CopyFileDataByFile( FILE *inFile, char **outPtr, size_t *outLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	CopyFileDataByPath
	@abstract	Reads all the data in the file at the path into a malloc'd pointer and null terminates it.
*/
OSStatus	CopyFileDataByPath( const char *inPath, char **outPtr, size_t *outLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	ReadANSIFile / WriteANSIFile
	@abstract	Reads/writes all the specified data (or up to the end of file for reads).
*/
OSStatus	ReadANSIFile( FILE *inFile, void *inBuf, size_t inSize, size_t *outSize );
OSStatus	WriteANSIFile( FILE *inFile, const void *inBuf, size_t inSize );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	CreateTXTRecordWithCString
	@abstract	Creates a malloc'd TXT record from a string. See ParseQuotedEscapedString for escaping/quoting details.
*/
OSStatus	CreateTXTRecordWithCString( const char *inString, uint8_t **outTXTRec, size_t *outTXTLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	TXTRecordGetNextItem
	@abstract	Iterates the items in a TXT record.
*/
Boolean
	TXTRecordGetNextItem( 
		const uint8_t *		inSrc, 
		const uint8_t *		inEnd, 
		const char **		outKeyPtr, 
		size_t *			outKeyLen, 
		const uint8_t **	outValuePtr, 
		size_t *			outValueLen, 
		const uint8_t **	outSrc );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	GetHomePath
	@abstract	Gets a path to the home directory for the current process.
*/
char *	GetHomePath( char *inBuffer, size_t inMaxLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	mkpath
	@abstract	Creates a directory and all intermediate directories if they do not exist.
*/
#if( TARGET_OS_POSIX )
	int	mkpath( const char *path, mode_t mode, mode_t dir_mode );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NormalizePath
	@abstract	Normalizes a path to expand tidle's at the beginning and resolve ".", "..", and sym links.
	
	@param		inSrc		Source path to normalize.
	@param		inLen		Number of bytes in "inSrc" or kSizeCString if it's a NUL-terminated string.
	@param		inDst		Buffer to received normalized string. May be the same as "inSrc" to normalized in-place.
	@param		inMaxLen	Max number of bytes (including a NUL terminator) to write to "inDst".
	@param		inFlags		Flags to control the normalization process.
	
	@result		Ptr to inDst or "" if inMaxLen is 0.
	
	@discussion
	
	If the path is exactly "~" then expand to the current user's home directory.
	If the path is exactly "~user" then expand to "user"'s home directory.
	If the path begins with "~/" then expand the "~/" to the current user's home directory.
	If the path begins with "~user/" then expand the "~user/" to "user"'s home directory.
*/
#define kNormalizePathDontExpandTilde		( 1 << 0 ) // Don't replace "~" or "~user" with user home directory path.
#define kNormalizePathDontResolve			( 1 << 1 ) // Don't resolve ".", "..", or sym links.

char *	NormalizePath( const char *inSrc, size_t inLen, char *inDst, size_t inMaxLen, uint32_t inFlags );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NumberListStringCreateFromUInt8Array
	@abstract	Creates a number list string (e.g. "1,2-3,7-9") from an array of numbers. Caller must free string on success.
*/
OSStatus	NumberListStringCreateFromUInt8Array( const uint8_t *inArray, size_t inCount, char **outStr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	RemovePath
	@abstract	Removes a file or directory. If it's a directory, it recursively removes all items inside.
*/
OSStatus	RemovePath( const char *inPath );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	RunningWindowsVistaOrLater
	@abstract	Returns true if we're running Windows Vista or later.
*/
#if( TARGET_OS_WINDOWS )
	Boolean	RunningWindowsVistaOrLater( void );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	RunProcessAndCaptureOutput
	@abstract	Runs a process specified via a command line and captures everything it writes to stdout.
*/
OSStatus	RunProcessAndCaptureOutput( const char *inCmdLine, char **outResponse );
OSStatus	RunProcessAndCaptureOutputEx( const char *inCmdLine, char **outResponse, size_t *outLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	SCDynamicStoreCopyComputerName
	@abstract	Returns a copy of the computer name string.
	
	@param		inStore			Must be NULL.
	@param		outEncoding		Receives the encoding of the string. May be NULL.
*/
	typedef struct SCDynamicStore *		SCDynamicStoreRef;

#if( ( !0 && 0 ) || COMMON_SERVICES_NO_SYSTEM_CONFIGURATION )
	CFStringRef	SCDynamicStoreCopyComputerName( SCDynamicStoreRef inStore, CFStringEncoding *outEncoding );
#endif

#if( ( !0 && TARGET_OS_POSIX ) || COMMON_SERVICES_NO_SYSTEM_CONFIGURATION )
	CFStringRef	SCDynamicStoreCopyLocalHostName( SCDynamicStoreRef inStore );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSSystemf
	@abstract	Invokes a command line built using a format string.
	
	@param		inLogPrefix		Optional prefix to print before logging the command line. If NULL, no logging is performed.
	@param		inFormat		printf-style format string used to build the command line.
	
	@result		If the command line was executed, the exit status of it is returned. Otherwise, errno is returned.
*/
int	APSSystemf( const char *inLogPrefix, const char *inFormat, ... ) PRINTF_STYLE_FUNCTION( 2, 3 );

#if 0
#pragma mark == Packing/Unpacking ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		Packing/Unpacking
	@abstract	Functions for packing and unpacking data.
*/
OSStatus	PackData( void *inBuffer, size_t inMaxSize, size_t *outSize, const char *inFormat, ... );
OSStatus	PackDataVAList( void *inBuffer, size_t inMaxSize, size_t *outSize, const char *inFormat, va_list inArgs );

OSStatus	UnpackData( const void *inData, size_t inSize, const char *inFormat, ... );
OSStatus	UnpackDataVAList( const void *inData, size_t inSize, const char *inFormat, va_list inArgs );

#if 0
#pragma mark == EDID ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		EDID
	@abstract	Parsing of EDID's.
*/

#define kEDIDKey_CEABlock				CFSTR( "ceaBlock" )
	#define kCEAKey_Revision				CFSTR( "revision" )
#define kEDIDKey_EDIDRevision			CFSTR( "edidRevision" )
#define kEDIDKey_EDIDVersion			CFSTR( "edidVersion" )
#define kEDIDKey_HDMI					CFSTR( "hdmi" )
	#define kHDMIKey_AudioLatencyMs					CFSTR( "audioLatencyMs" )
	#define kHDMIKey_AudioLatencyInterlacedMs		CFSTR( "audioLatencyInterlacedMs" )
	#define kHDMIKey_VideoLatencyMs					CFSTR( "videoLatencyMs" )
	#define kHDMIKey_VideoLatencyInterlacedMs		CFSTR( "videoLatencyInterlacedMs" )
	#define kHDMIKey_SourcePhysicalAddress			CFSTR( "sourcePhysicalAddress" )
#define kEDIDKey_Manufacturer			CFSTR( "manufacturer" )
#define kEDIDKey_MonitorName			CFSTR( "monitorName" )
#define kEDIDKey_MonitorSerialNumber	CFSTR( "monitorSerialNumber" )
#define kEDIDKey_ProductID				CFSTR( "productID" )
#define kEDIDKey_RawBytes				CFSTR( "rawBytes" )
#define kEDIDKey_SerialNumber			CFSTR( "serialNumber" )
#define kEDIDKey_WeekOfManufacture		CFSTR( "weekOfManufacture" )
#define kEDIDKey_YearOfManufacture		CFSTR( "yearOfManufacture" )

CFDictionaryRef	CreateEDIDDictionaryWithBytes( const uint8_t *inData, size_t inSize, OSStatus *outErr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	H264ConvertAVCCtoAnnexBHeader
	@abstract	Parses a H.264 AVCC atom and converts it to an Annex-B header and gets the nal_size for subsequent frames.
	
	@param	inHeaderBuf			Buffer to receive an Annex-B header containing the AVCC data. May be NULL to just find the size.
	@param	inHeaderMaxLen		Max length of inHeaderBuf. May be zero to just find the size.
	@param	outHeaderLen		Ptr to receive the length of the Annex-B header. May be NULL.
	@param	outNALSize			Ptr to receive the number of bytes in the nal_size field before each AVCC-style frame. May be NULL.
	@param	outNext				Ptr to receive pointer to byte following the last byte that was parsed. May be NULL.
*/
OSStatus
	H264ConvertAVCCtoAnnexBHeader( 
		const uint8_t *		inAVCCPtr,
		size_t				inAVCCLen,
		uint8_t *			inHeaderBuf,
		size_t				inHeaderMaxLen,
		size_t *			outHeaderLen,
		size_t *			outNALSize,
		const uint8_t **	outNext );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	H264GetNextNALUnit
	@abstract	Parses a H.264 AVCC-style stream (NAL size-prefixed NAL units) and returns each piece of NAL unit data.
	
	@param	inSrc			Ptr to start of a single NAL unit. This must begin with a NAL size header.
	@param	inEnd			Ptr to end of the NAL unit.
	@param	outDataPtr		Ptr to data to NAL unit data (minus NAL size header).
	@param	outDataLen		Number of bytes of NAL unit data.
	@param	outSrc			Receives ptr to pass as inSrc for the next call.
*/
OSStatus
	H264GetNextNALUnit( 
		const uint8_t *		inSrc, 
		const uint8_t *		inEnd, 
		size_t				inNALSize, 
		const uint8_t * *	outDataPtr, 
		size_t *			outDataLen,
		const uint8_t **	outSrc );

#if 0
#pragma mark == MirroredRingBuffer ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@struct		MirroredRingBuffer
	@abstract	Structure for managing the ring buffer.
*/
typedef struct
{
	uint8_t *			buffer;			// Buffer backing the ring buffer.
	const uint8_t *		end;			// End of the buffer. Useful for check if a pointer is within the ring buffer.
	uint32_t			size;			// Max number of bytes the ring buffer can hold.
	uint32_t			mask;			// Mask for power-of-2 sized buffers to wrap without using a slower mod operator.
	uint32_t			readOffset;		// Current offset for reading. Don't access directly. Use macros.
	uint32_t			writeOffset;	// Current offset for writing. Don't access directly. Use macros.
#if( TARGET_OS_WINDOWS )
	HANDLE				mapFile;		// Handle to file mapping.
#endif
	
}	MirroredRingBuffer;

#define kMirroredRingBufferInit		{ NULL, NULL, 0, 0, 0, 0 }

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		MirroredRingBuffer init/free.
	@abstract	Initializes/frees a ring buffer.
*/
OSStatus	MirroredRingBufferInit( MirroredRingBuffer *inRing, size_t inMinSize, Boolean inPowerOf2 );
void		MirroredRingBufferFree( MirroredRingBuffer *inRing );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		MirroredRingBufferAccessors
	@abstract	Macros for accessing the ring buffer.
*/

// Macros for getting read/write pointers for power-of-2 sized ring buffers.
#define MirroredRingBufferGetReadPtr( RING )			( &(RING)->buffer[ (RING)->readOffset  & (RING)->mask ] )
#define MirroredRingBufferGetWritePtr( RING )			( &(RING)->buffer[ (RING)->writeOffset & (RING)->mask ] )

// Macros for getting read/write pointers for non-power-of-2 sized ring buffers.
#define MirroredRingBufferGetReadPtrSlow( RING )		( &(RING)->buffer[ (RING)->readOffset  % (RING)->size ] )
#define MirroredRingBufferGetWritePtrSlow( RING )		( &(RING)->buffer[ (RING)->writeOffset % (RING)->size ] )

#define MirroredRingBufferReadAdvance( RING, COUNT )	do { (RING)->readOffset  += (COUNT); } while( 0 )
#define MirroredRingBufferWriteAdvance( RING, COUNT )	do { (RING)->writeOffset += (COUNT); } while( 0 )
#define MirroredRingBufferReset( RING )					do { (RING)->readOffset = (RING)->writeOffset; } while( 0 )

#define MirroredRingBufferContainsPtr( RING, PTR )		( ( ( (const uint8_t *)(PTR) ) >= (RING)->buffer ) && \
														  ( ( (const uint8_t *)(PTR) ) <  (RING)->end ) )
#define MirroredRingBufferGetBytesUsed( RING )			( (uint32_t)( (RING)->writeOffset - (RING)->readOffset ) )
#define MirroredRingBufferGetBytesFree( RING )			( (RING)->size - MirroredRingBufferGetBytesUsed( RING ) )

#if 0
#pragma mark == Morse Code ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		MorseCode
	@abstract	Generic library for morse code.
*/

// MorseCodeAction

typedef unsigned char		MorseCodeAction;
#define kMorseCodeAction_Off			0
#define kMorseCodeAction_On				1
#define kMorseCodeAction_Dit			2
#define kMorseCodeAction_Dah			3
#define kMorseCodeAction_MarkDelay		4 // Delay between dots and dashes within a character. 1 unit.
#define kMorseCodeAction_CharDelay		5 // Delay between characters. 3 units.
#define kMorseCodeAction_WordDelay		6 // Delay between words. 7 units.

// MorseCodeFlags

typedef unsigned int		MorseCodeFlags;
#define kMorseCodeFlags_None			0
#define kMorseCodeFlags_RawActions		( 1 << 0 ) //! Don't do on/delay/off...pass dits and dahs directly.

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	MorseCode
	@abstract	Performs morse code using a callback for the underlying primitive (e.g. drive LED, play tone, etc.).
*/
typedef void ( *MorseCodeActionFunc )( MorseCodeAction inAction, void *inArg );

void
	MorseCode( 
		const char *		inMessage, 
		int					inSpeed, 
		MorseCodeFlags		inFlags, 
		MorseCodeActionFunc inActionFunc, 
		void *				inActionArg );

#if 0
#pragma mark == PID Controller ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		Proportional-Integral-Derivative (PID) Controller
	@abstract	Structure and API for a generic PID controller.
*/

typedef struct
{
	double		pGain;		// Proportional Gain.
	double		iState;		// Integrator state.
	double		iMin, iMax;	// Integrator limits.
	double		iGain;		// Integrator gain (always less than 1).
	double		dState;		// Differentiator state.
	double		dpGain;		// Differentiator filter gain = 1 - pole.
	double		dGain;		// Derivative gain.
	
}	PIDContext;

void	PIDInit( PIDContext *inPID, double pGain, double iGain, double dGain, double dPole, double iMin, double iMax );
#define PIDReset( CONTEXT )		do { (CONTEXT)->iState = 0; (CONTEXT)->dState = 0; } while( 0 )
double	PIDUpdate( PIDContext *inPID, double input );

#if 0
#pragma mark == poly1305 ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	poly1305_auth
	@abstract	Generates a 16-byte Poly1305 Message Authentication Code from an N-byte message and a 32-byte key.
	@discussion	See DJB's paper on Poly1305 <http://cr.yp.to/mac.html> for details.
*/
void	poly1305_auth( uint8_t outAuth[ 16 ], const uint8_t *inMsg, size_t inLen, const uint8_t inKey[ 32 ] );

#if 0
#pragma mark == Salsa20-Poly1305 ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	crypto_xsalsa20_poly1305_encrypt
	@abstract	Uses XSalsa20 to encrypt a message and uses Poly1305 to generate an message authentication code for it.
	
	@param		outCiphertext	Buffer of at least "inLen" bytes to receive the encrypted bytes.
								May point to the same buffer as "inPlaintext" for in-place encryption.
	@param		outMAC			16-byte buffer to receive the message authentication code for the encrypted bytes.
	@param		inPlaintext		Ptr to "inLen" bytes to encrypt.
	@param		inLen			Number of bytes to encrypt.
	@param		inNonce			24-byte nonce. This nonce must never be reused with the same key.
	@param		inKey			32-byte key.
*/
void
	crypto_xsalsa20_poly1305_encrypt( 
		uint8_t *		outCiphertext, 
		uint8_t *		outMAC, 
		const uint8_t *	inPlaintext, 
		size_t			inLen, 
		const uint8_t *	inNonce, 
		const uint8_t *	inKey );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	crypto_xsalsa20_poly1305_decrypt
	@abstract	Uses Poly1305 authenticate an encrypted message and uses XSalsa20 to decrypt the message.
	
	@param		outPlaintext	Buffer of at least "inLen" bytes to receive the decrypted bytes.
								May point to the same buffer as "inCiphertext" for in-place decryption.
	@param		inCiphertext	Ptr to "inLen" bytes to decrypt.
	@param		inLen			Number of bytes to decrypt.
	@param		inMAC			16-byte message authentication code.
	@param		inNonce			24-byte nonce.
	@param		inKey			32-byte key.
	
	@result		0 if the message is valid and was decrypted. Any other value a failure and the message cannot be used.
*/
int
	crypto_xsalsa20_poly1305_decrypt( 
		uint8_t *		outPlaintext, 
		const uint8_t *	inCiphertext, 
		size_t			inLen, 
		const uint8_t *	inMAC, 
		const uint8_t *	inNonce, 
		const uint8_t *	inKey );

#if 0
#pragma mark == Security ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		HasCodeSigningRequirementByPath / HasCodeSigningRequirementByPID
	@abstract	Checks if the path or PID is signed and has the specified requirement. 
*/

#if 0
#pragma mark == Debugging ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	MiscUtilsTest
	@abstract	Unit test.
*/
#if( DEBUG )
	OSStatus	MiscUtilsTest( void );
#endif

#ifdef __cplusplus
}
#endif

#endif	// __MiscUtils_h__
