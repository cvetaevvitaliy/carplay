/*
	File:    	AESUtils.c
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

#include "AESUtils.h"

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "MiscUtils.h"
#include "TickUtils.h"

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
	#include <CommonCrypto/CommonCryptorSPI.h>
#endif

#if( !AES_UTILS_USE_COMMON_CRYPTO && !AES_UTILS_USE_GLADMAN_AES && TARGET_NO_OPENSSL )
static
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key,
		     unsigned char *ivec, const int enc);
#endif

#if( AES_UTILS_USE_WINDOWS_API )
typedef struct
{
	BLOBHEADER		hdr;
	DWORD			len;
	uint8_t			key[ 16 ];
	
}	AESKeyBlob;

static OSStatus	_CreateWindowsCryptoAPIContext( HCRYPTPROV *outProvider, HCRYPTKEY *outKey, const uint8_t *inKey, DWORD inMode );
#endif

//===========================================================================================================================
//	AES_CTR_Init
//===========================================================================================================================

OSStatus
	AES_CTR_Init( 
		AES_CTR_Context *	inContext, 
		const uint8_t		inKey[ kAES_CTR_Size ], 
		const uint8_t		inNonce[ kAES_CTR_Size ] )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	OSStatus		err;
	
	inContext->cryptor = NULL;
	err = CCCryptorCreate( kCCEncrypt, kCCAlgorithmAES128, kCCOptionECBMode, inKey, kAES_CTR_Size, NULL, 
		&inContext->cryptor );
	check_noerr( err );
	if( err ) return( err );
#elif( AES_UTILS_USE_GLADMAN_AES )
	aes_init();
	aes_encrypt_key128( inKey, &inContext->ctx );
#elif( AES_UTILS_USE_USSL )
	aes_setkey_enc( &inContext->ctx, (unsigned char *) inKey, kAES_CTR_Size * 8 );
#elif( AES_UTILS_USE_WINDOWS_API )
	OSStatus		err;
	
	err = _CreateWindowsCryptoAPIContext( &inContext->provider, &inContext->key, inKey, CRYPT_MODE_ECB );
	check_noerr( err );
	if( err ) return( err );
#else
	AES_set_encrypt_key( inKey, kAES_CTR_Size * 8, &inContext->key );
#endif
	memcpy( inContext->ctr, inNonce, kAES_CTR_Size );
	inContext->used = 0;
	inContext->legacy = false;
	return( kNoErr );
}

//===========================================================================================================================
//	AES_CTR_Increment
//===========================================================================================================================

STATIC_INLINE void	AES_CTR_Increment( uint8_t *inCounter )
{
	int		i;
	
	// Note: counter is always big endian so this adds from right to left.
	
	for( i = kAES_CTR_Size - 1; i >= 0; --i )
	{
		if( ++( inCounter[ i ] ) != 0 )
		{
			break;
		}
	}
}

//===========================================================================================================================
//	AES_CTR_Update
//===========================================================================================================================

OSStatus	AES_CTR_Update( AES_CTR_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus			err;
	const uint8_t *		src;
	uint8_t *			dst;
	uint8_t *			buf;
	size_t				used;
	size_t				i;
	
	// inSrc and inDst may be the same, but otherwise, the buffers must not overlap.
	
#if( DEBUG )
	if( inSrc != inDst ) check_ptr_overlap( inSrc, inLen, inDst, inLen );
#endif
	
	src = (const uint8_t *) inSrc;
	dst = (uint8_t *) inDst;
	
	// If there's any buffered key material from a previous block then use that first.
	
	buf  = inContext->buf;
	used = inContext->used;
	while( ( inLen > 0 ) && ( used != 0 ) ) 
	{
		*dst++ = *src++ ^ buf[ used++ ];
		used %= kAES_CTR_Size;
		inLen -= 1;
	}
	inContext->used = used;
	
	// Process whole blocks.
	
	while( inLen >= kAES_CTR_Size )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorUpdate( inContext->cryptor, inContext->ctr, kAES_CTR_Size, buf, kAES_CTR_Size, &i );
			require_noerr( err, exit );
			require_action( i == kAES_CTR_Size, exit, err = kSizeErr );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			aes_ecb_encrypt( inContext->ctr, buf, kAES_CTR_Size, &inContext->ctx );
		#elif( AES_UTILS_USE_USSL )
			aes_crypt_ecb( &inContext->ctx, AES_ENCRYPT, inContext->ctr, buf );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		len;
			
			len = kAES_CTR_Size;
			memcpy( buf, inContext->ctr, kAES_CTR_Size );
			good = CryptEncrypt( inContext->key, 0, false, 0, buf, &len, len );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			AES_encrypt( inContext->ctr, buf, &inContext->key );
		#endif
		AES_CTR_Increment( inContext->ctr );
		
		for( i = 0; i < kAES_CTR_Size; ++i )
		{
			dst[ i ] = src[ i ] ^ buf[ i ];
		}
		src   += kAES_CTR_Size;
		dst   += kAES_CTR_Size;
		inLen -= kAES_CTR_Size;
	}
	
	// Process any trailing sub-block bytes. Extra key material is buffered for next time.
	
	if( inLen > 0 )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorUpdate( inContext->cryptor, inContext->ctr, kAES_CTR_Size, buf, kAES_CTR_Size, &i );
			require_noerr( err, exit );
			require_action( i == kAES_CTR_Size, exit, err = kSizeErr );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			aes_ecb_encrypt( inContext->ctr, buf, kAES_CTR_Size, &inContext->ctx );
		#elif( AES_UTILS_USE_USSL )
			aes_crypt_ecb( &inContext->ctx, AES_ENCRYPT, inContext->ctr, buf );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		len;
			
			len = kAES_CTR_Size;
			memcpy( buf, inContext->ctr, kAES_CTR_Size );
			good = CryptEncrypt( inContext->key, 0, false, 0, buf, &len, len );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			AES_encrypt( inContext->ctr, buf, &inContext->key );
		#endif
		AES_CTR_Increment( inContext->ctr );
		
		for( i = 0; i < inLen; ++i )
		{
			*dst++ = *src++ ^ buf[ used++ ];
		}
		
		// For legacy mode, always leave the used amount as 0 so we always increment the counter each time.
		
		if( !inContext->legacy )
		{
			inContext->used = used;
		}
	}
	err = kNoErr;
	
#if( AES_UTILS_USE_COMMON_CRYPTO || AES_UTILS_USE_WINDOWS_API )
exit:
#endif
	return( err );
}

//===========================================================================================================================
//	AES_CTR_Final
//===========================================================================================================================

void	AES_CTR_Final( AES_CTR_Context *inContext )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	if( inContext->cryptor ) CCCryptorRelease( inContext->cryptor );
#elif( AES_UTILS_USE_WINDOWS_API )
	if( inContext->key )		CryptDestroyKey( inContext->key );
	if( inContext->provider )	CryptReleaseContext( inContext->provider, 0 );
#endif
	MemZeroSecure( inContext, sizeof( *inContext ) );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AES_CBCFrame_Init
//===========================================================================================================================

OSStatus
	AES_CBCFrame_Init( 
		AES_CBCFrame_Context *	inContext, 
		const uint8_t			inKey[ kAES_CBCFrame_Size ], 
		const uint8_t			inIV[ kAES_CBCFrame_Size ], 
		Boolean					inEncrypt )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	OSStatus		err;
	
	inContext->cryptor = NULL;
	err = CCCryptorCreate( inEncrypt ? kCCEncrypt : kCCDecrypt, kCCAlgorithmAES128, 0, inKey, kAES_CTR_Size, 
		NULL, &inContext->cryptor );
	check_noerr( err );
	if( err ) return( err );
#elif( AES_UTILS_USE_GLADMAN_AES )
	aes_init();
	if( inEncrypt ) aes_encrypt_key128( inKey, &inContext->ctx.encrypt );
	else			aes_decrypt_key128( inKey, &inContext->ctx.decrypt );
	inContext->encrypt = inEncrypt;
#elif( AES_UTILS_USE_USSL )
	if( inEncrypt ) aes_setkey_enc( &inContext->ctx, (unsigned char *) inKey, kAES_CBCFrame_Size * 8 );
	else			aes_setkey_dec( &inContext->ctx, (unsigned char *) inKey, kAES_CBCFrame_Size * 8 );
	inContext->encrypt = inEncrypt;
#elif( AES_UTILS_USE_WINDOWS_API )
	OSStatus		err;
	
	err = _CreateWindowsCryptoAPIContext( &inContext->provider, &inContext->key, inKey, CRYPT_MODE_CBC );
	check_noerr( err );
	if( err ) return( err );
	
	inContext->encrypt = inEncrypt;
#else
	if( inEncrypt ) AES_set_encrypt_key( inKey, kAES_CBCFrame_Size * 8, &inContext->key );
	else			AES_set_decrypt_key( inKey, kAES_CBCFrame_Size * 8, &inContext->key );
	inContext->mode = inEncrypt ? AES_ENCRYPT : AES_DECRYPT;
#endif
	memcpy( inContext->iv, inIV, kAES_CBCFrame_Size );
	return( kNoErr );
}

//===========================================================================================================================
//	AES_CBCFrame_Update
//===========================================================================================================================

OSStatus	AES_CBCFrame_Update( AES_CBCFrame_Context *inContext, const void *inSrc, size_t inSrcLen, void *inDst )
{
	OSStatus			err;
	const uint8_t *		src;
	const uint8_t *		end;
	uint8_t *			dst;
	size_t				len;
	
	src = (const uint8_t *) inSrc;
	end = src + inSrcLen;
	dst = (uint8_t *) inDst;
	
	// Process whole blocks.
	
	len = inSrcLen & ~( (size_t)( kAES_CBCFrame_Size - 1 ) );
	if( len > 0 )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorReset(  inContext->cryptor, inContext->iv );
			require_noerr( err, exit );
			
			err = CCCryptorUpdate( inContext->cryptor, src, len, dst, len, &len );
			require_noerr( err, exit );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			uint8_t		iv[ kAES_CBCFrame_Size ];
			
			memcpy( iv, inContext->iv, kAES_CBCFrame_Size ); // Use local copy so original IV is not changed.
			if( inContext->encrypt )	aes_cbc_encrypt( src, dst, (int) len, iv, &inContext->ctx.encrypt );
			else						aes_cbc_decrypt( src, dst, (int) len, iv, &inContext->ctx.decrypt );
		#elif( AES_UTILS_USE_USSL )
			uint8_t		iv[ kAES_CBCFrame_Size ];
			
			memcpy( iv, inContext->iv, kAES_CBCFrame_Size ); // Use local copy so original IV is not changed.
			if( inContext->encrypt )	aes_crypt_cbc( &inContext->ctx, AES_ENCRYPT, len, iv, (unsigned char *) src, dst );
			else						aes_crypt_cbc( &inContext->ctx, AES_DECRYPT, len, iv, (unsigned char *) src, dst );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		dwLen;
			
			good = CryptSetKeyParam( inContext->key, KP_IV, inContext->iv, 0 );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
			
			if( src != dst ) memcpy( dst, src, len );
			dwLen = (DWORD) len;
			if( inContext->encrypt )	good = CryptEncrypt( inContext->key, 0, false, 0, dst, &dwLen, dwLen );
			else						good = CryptDecrypt( inContext->key, 0, false, 0, dst, &dwLen );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			uint8_t		iv[ kAES_CBCFrame_Size ];
			
			memcpy( iv, inContext->iv, kAES_CBCFrame_Size ); // Use local copy so original IV is not changed.
			AES_cbc_encrypt( src, dst, (unsigned long) len, &inContext->key, iv, inContext->mode );
		#endif
		src += len;
		dst += len;
	}
	
	// The remaining bytes are just copied unencrypted.
	
	while( src != end ) *dst++ = *src++;
	err = kNoErr;
	
#if( AES_UTILS_USE_COMMON_CRYPTO || AES_UTILS_USE_WINDOWS_API )
exit:
#endif
	return( err );
}

//===========================================================================================================================
//	AES_CBCFrame_Update2
//===========================================================================================================================

OSStatus
	AES_CBCFrame_Update2( 
		AES_CBCFrame_Context *	inContext, 
		const void *			inSrc1, 
		size_t					inLen1, 
		const void *			inSrc2, 
		size_t					inLen2, 
		void *					inDst )
{
	const uint8_t *		src1 = (const uint8_t *) inSrc1;
	const uint8_t *		end1 = src1 + inLen1;
	const uint8_t *		src2 = (const uint8_t *) inSrc2;
	const uint8_t *		end2 = src2 + inLen2;
	uint8_t *			dst  = (uint8_t *) inDst;
	OSStatus			err;
	size_t				len;
	size_t				i;
#if( !AES_UTILS_USE_COMMON_CRYPTO && !AES_UTILS_USE_WINDOWS_API )
	uint8_t				iv[ kAES_CBCFrame_Size ];
#endif
	
#if( AES_UTILS_USE_COMMON_CRYPTO )
	if( ( inLen1 + inLen2 ) >= kAES_CBCFrame_Size )
	{
		err = CCCryptorReset(  inContext->cryptor, inContext->iv );
		require_noerr( err, exit );
	}
#elif( AES_UTILS_USE_WINDOWS_API )
	if( ( inLen1 + inLen2 ) >= kAES_CBCFrame_Size )
	{
		BOOL		good;
		
		good = CryptSetKeyParam( inContext->key, KP_IV, inContext->iv, 0 );
		err = map_global_value_errno( good, good );
		require_noerr( err, exit );
	}
#else
	memcpy( iv, inContext->iv, kAES_CBCFrame_Size ); // Use local copy so original IV is not changed.
#endif
	
	// Process all whole blocks from buffer 1.
	
	len = inLen1 & ~( (size_t)( kAES_CBCFrame_Size - 1 ) );
	if( len > 0 )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorUpdate( inContext->cryptor, src1, len, dst, len, &len );
			require_noerr( err, exit );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			if( inContext->encrypt )	aes_cbc_encrypt( src1, dst, (int) len, iv, &inContext->ctx.encrypt );
			else						aes_cbc_decrypt( src1, dst, (int) len, iv, &inContext->ctx.decrypt );
		#elif( AES_UTILS_USE_USSL )
			if( inContext->encrypt )	aes_crypt_cbc( &inContext->ctx, AES_ENCRYPT, len, iv, (unsigned char *) src1, dst );
			else						aes_crypt_cbc( &inContext->ctx, AES_DECRYPT, len, iv, (unsigned char *) src1, dst );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		dwLen;
			
			if( src1 != dst ) memcpy( dst, src1, len );
			dwLen = (DWORD) len;
			if( inContext->encrypt )	good = CryptEncrypt( inContext->key, 0, false, 0, dst, &dwLen, dwLen );
			else						good = CryptDecrypt( inContext->key, 0, false, 0, dst, &dwLen );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			AES_cbc_encrypt( src1, dst, (unsigned long) len, &inContext->key, iv, inContext->mode );
		#endif
		src1 += len;
		dst  += len;
	}
	
	// If there are any partial block bytes in buffer 1 and enough bytes in buffer 2 to fill a 
	// block then combine them into a temporary buffer and encrypt it.
	
	if( ( src1 != end1 ) && ( ( ( end1 - src1 ) + ( end2 - src2 ) ) >= kAES_CBCFrame_Size ) )
	{
		uint8_t		buf[ kAES_CBCFrame_Size ];
		
		for( i = 0; src1 != end1; ++i )
		{
			buf[ i ] = *src1++;
		}
		for( ; ( i < kAES_CBCFrame_Size ) && ( src2 != end2 ); ++i )
		{
			buf[ i ] = *src2++;
		}
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorUpdate( inContext->cryptor, buf, i, dst, i, &i );
			require_noerr( err, exit );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			if( inContext->encrypt )	aes_cbc_encrypt( buf, dst, (int) i, iv, &inContext->ctx.encrypt );
			else						aes_cbc_decrypt( buf, dst, (int) i, iv, &inContext->ctx.decrypt );
		#elif( AES_UTILS_USE_USSL )
			if( inContext->encrypt )	aes_crypt_cbc( &inContext->ctx, AES_ENCRYPT, i, iv, buf, dst );
			else						aes_crypt_cbc( &inContext->ctx, AES_DECRYPT, i, iv, buf, dst );
		#elif( AES_UTILS_USE_WINDOWS_API )
		{
			BOOL		good;
			DWORD		dwLen;
			
			memcpy( dst, buf, i );
			dwLen = (DWORD) i;
			if( inContext->encrypt )	good = CryptEncrypt( inContext->key, 0, false, 0, dst, &dwLen, dwLen );
			else						good = CryptDecrypt( inContext->key, 0, false, 0, dst, &dwLen );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		}
		#else
			AES_cbc_encrypt( buf, dst, (unsigned long) i, &inContext->key, iv, inContext->mode );
		#endif
		dst += i;
	}
	
	// Process any remaining whole blocks in buffer 2.
	
	len = ( (size_t)( end2 - src2 ) ) & ~( (size_t)( kAES_CBCFrame_Size - 1 ) );
	if( len > 0 )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			err = CCCryptorUpdate( inContext->cryptor, src2, len, dst, len, &len );
			require_noerr( err, exit );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			if( inContext->encrypt )	aes_cbc_encrypt( src2, dst, (int) len, iv, &inContext->ctx.encrypt );
			else						aes_cbc_decrypt( src2, dst, (int) len, iv, &inContext->ctx.decrypt );
		#elif( AES_UTILS_USE_USSL )
			if( inContext->encrypt )	aes_crypt_cbc( &inContext->ctx, AES_ENCRYPT, len, iv, (unsigned char *) src2, dst );
			else						aes_crypt_cbc( &inContext->ctx, AES_DECRYPT, len, iv, (unsigned char *) src2, dst );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		dwLen;
			
			if( src2 != dst ) memcpy( dst, src2, len );
			dwLen = len;
			if( inContext->encrypt )	good = CryptEncrypt( inContext->key, 0, false, 0, dst, &dwLen, dwLen );
			else						good = CryptDecrypt( inContext->key, 0, false, 0, dst, &dwLen );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			AES_cbc_encrypt( src2, dst, (unsigned long) len, &inContext->key, iv, inContext->mode );
		#endif
		src2 += len;
		dst  += len;
	}
	
	// Any remaining bytes are just copied unencrypted.
	
	while( src1 != end1 ) *dst++ = *src1++;
	while( src2 != end2 ) *dst++ = *src2++;
	err = kNoErr;
	
#if( AES_UTILS_USE_COMMON_CRYPTO || AES_UTILS_USE_WINDOWS_API )
exit:
#endif
	return( err );
}

//===========================================================================================================================
//	AES_CBCFrame_Final
//===========================================================================================================================

void	AES_CBCFrame_Final( AES_CBCFrame_Context *inContext )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	if( inContext->cryptor ) CCCryptorRelease( inContext->cryptor );
#elif( AES_UTILS_USE_WINDOWS_API )
	if( inContext->key )		CryptDestroyKey( inContext->key );
	if( inContext->provider )	CryptReleaseContext( inContext->provider, 0 );
#endif
	MemZeroSecure( inContext, sizeof( *inContext ) );
}

//===========================================================================================================================
//	AES_cbc_encrypt
//===========================================================================================================================

#if( !AES_UTILS_USE_COMMON_CRYPTO && !AES_UTILS_USE_GLADMAN_AES && TARGET_NO_OPENSSL )
// From OpenSSL
static
void AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
		     const unsigned long length, const AES_KEY *key,
		     unsigned char *ivec, const int enc) {

	unsigned long n;
	unsigned long len = length;
	unsigned char tmp[AES_BLOCK_SIZE];
	const unsigned char *iv = ivec;

	if (AES_ENCRYPT == enc) {
		while (len >= AES_BLOCK_SIZE) {
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] = in[n] ^ iv[n];
			AES_encrypt(out, out, key);
			iv = out;
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			for(n=0; n < len; ++n)
				out[n] = in[n] ^ iv[n];
			for(n=len; n < AES_BLOCK_SIZE; ++n)
				out[n] = iv[n];
			AES_encrypt(out, out, key);
			iv = out;
		}
		memcpy(ivec,iv,AES_BLOCK_SIZE);
	} else if (in != out) {
		while (len >= AES_BLOCK_SIZE) {
			AES_decrypt(in, out, key);
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] ^= iv[n];
			iv = in;
			len -= AES_BLOCK_SIZE;
			in  += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			AES_decrypt(in,tmp,key);
			for(n=0; n < len; ++n)
				out[n] = tmp[n] ^ iv[n];
			iv = in;
		}
		memcpy(ivec,iv,AES_BLOCK_SIZE);
	} else {
		while (len >= AES_BLOCK_SIZE) {
			memcpy(tmp, in, AES_BLOCK_SIZE);
			AES_decrypt(in, out, key);
			for(n=0; n < AES_BLOCK_SIZE; ++n)
				out[n] ^= ivec[n];
			memcpy(ivec, tmp, AES_BLOCK_SIZE);
			len -= AES_BLOCK_SIZE;
			in += AES_BLOCK_SIZE;
			out += AES_BLOCK_SIZE;
		}
		if (len) {
			memcpy(tmp, in, AES_BLOCK_SIZE);
			AES_decrypt(tmp, out, key);
			for(n=0; n < len; ++n)
				out[n] ^= ivec[n];
			for(n=len; n < AES_BLOCK_SIZE; ++n)
				out[n] = tmp[n];
			memcpy(ivec, tmp, AES_BLOCK_SIZE);
		}
	}
}
#endif

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	AES_ECB_Init
//===========================================================================================================================

OSStatus	AES_ECB_Init( AES_ECB_Context *inContext, uint32_t inMode, const uint8_t inKey[ kAES_ECB_Size ] )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	OSStatus		err;
	
	inContext->cryptor = NULL;
	err = CCCryptorCreate( inMode, kCCAlgorithmAES128, kCCOptionECBMode, inKey, kAES_ECB_Size, NULL, &inContext->cryptor );
	check_noerr( err );
	if( err ) return( err );
#elif( AES_UTILS_USE_GLADMAN_AES )
	aes_init();
	if( inMode == kAES_ECB_Mode_Encrypt )	aes_encrypt_key128( inKey, &inContext->ctx.encrypt );
	else									aes_decrypt_key128( inKey, &inContext->ctx.decrypt );
	inContext->encrypt = inMode;
#elif( AES_UTILS_USE_USSL )
	if( inMode == kAES_ECB_Mode_Encrypt ) 	aes_setkey_enc( &inContext->ctx, (unsigned char *) inKey, kAES_ECB_Size * 8 );
	else									aes_setkey_dec( &inContext->ctx, (unsigned char *) inKey, kAES_ECB_Size * 8 );
	inContext->mode = inMode;
#elif( AES_UTILS_USE_WINDOWS_API )
	OSStatus		err;
	
	err = _CreateWindowsCryptoAPIContext( &inContext->provider, &inContext->key, inKey, CRYPT_MODE_ECB );
	check_noerr( err );
	if( err ) return( err );
	inContext->encrypt = ( inMode == kAES_ECB_Mode_Encrypt );
#else
	AES_set_encrypt_key( inKey, kAES_ECB_Size * 8, &inContext->key );
	inContext->cryptFunc = ( inMode == kAES_ECB_Mode_Encrypt ) ? AES_encrypt : AES_decrypt;
#endif
	return( kNoErr );
}

//===========================================================================================================================
//	AES_ECB_Update
//===========================================================================================================================

OSStatus	AES_ECB_Update( AES_ECB_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus			err;
	const uint8_t *		src;
	uint8_t *			dst;
	size_t				n;
	
	// inSrc and inDst may be the same, but otherwise, the buffers must not overlap.
	
#if( DEBUG )
	if( inSrc != inDst ) check_ptr_overlap( inSrc, inLen, inDst, inLen );
	if( ( inLen % kAES_ECB_Size ) != 0 ) dlogassert( "ECB doesn't support non-block-sized operations (%zu bytes)", inLen );
#endif
	
	src = (const uint8_t *) inSrc;
	dst = (uint8_t *) inDst;
	for( n = inLen / kAES_ECB_Size; n > 0; --n )
	{
		#if( AES_UTILS_USE_COMMON_CRYPTO )
			size_t		len;
			
			err = CCCryptorUpdate( inContext->cryptor, src, kAES_ECB_Size, dst, kAES_ECB_Size, &len );
			require_noerr( err, exit );
			check( len == kAES_ECB_Size );
		#elif( AES_UTILS_USE_GLADMAN_AES )
			if( inContext->encrypt )	aes_ecb_encrypt( src, dst, kAES_ECB_Size, &inContext->ctx.encrypt );
			else						aes_ecb_decrypt( src, dst, kAES_ECB_Size, &inContext->ctx.decrypt );
		#elif( AES_UTILS_USE_USSL )
			aes_crypt_ecb( &inContext->ctx, inContext->mode, (unsigned char *) src, dst );
		#elif( AES_UTILS_USE_WINDOWS_API )
			BOOL		good;
			DWORD		dwLen;
			
			if( src != dst ) memcpy( dst, src, kAES_ECB_Size );
			dwLen = kAES_ECB_Size;
			if( inContext->encrypt )	good = CryptEncrypt( inContext->key, 0, false, 0, dst, &dwLen, dwLen );
			else						good = CryptDecrypt( inContext->key, 0, false, 0, dst, &dwLen );
			err = map_global_value_errno( good, good );
			require_noerr( err, exit );
		#else
			inContext->cryptFunc( src, dst, &inContext->key );
		#endif
		src += kAES_ECB_Size;
		dst += kAES_ECB_Size;
	}
	err = kNoErr;
	
#if( AES_UTILS_USE_COMMON_CRYPTO || AES_UTILS_USE_WINDOWS_API )
exit:
#endif
	return( err );
}

//===========================================================================================================================
//	AES_ECB_Final
//===========================================================================================================================

void	AES_ECB_Final( AES_ECB_Context *inContext )
{
#if( AES_UTILS_USE_COMMON_CRYPTO )
	if( inContext->cryptor ) CCCryptorRelease( inContext->cryptor );
#elif( AES_UTILS_USE_WINDOWS_API )
	if( inContext->key )		CryptDestroyKey( inContext->key );
	if( inContext->provider )	CryptReleaseContext( inContext->provider, 0 );
#endif
	MemZeroSecure( inContext, sizeof( *inContext ) );
}

#if 0
#pragma mark -
#endif

#if( AES_UTILS_HAS_GCM )

//===========================================================================================================================
//	AES_GCM_Init
//===========================================================================================================================

OSStatus
	AES_GCM_Init( 
		AES_GCM_Context *	inContext, 
		const uint8_t		inKey[ kAES_CGM_Size ], 
		const uint8_t		inNonce[ kAES_CGM_Size ] )
{
	OSStatus		err;
	
#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
	err = CCCryptorCreateWithMode( kCCEncrypt, kCCModeGCM, kCCAlgorithmAES128, ccNoPadding, NULL, 
		inKey, kAES_CGM_Size, NULL, 0, 0, 0, &inContext->cryptor );
	require_noerr( err, exit );
#elif( AES_UTILS_HAS_GLADMAN_GCM )
	err = gcm_init_and_key( inKey, kAES_CGM_Size, &inContext->ctx );
	require_noerr( err, exit );
#else
	#error "GCM enabled, but no implementation?"
#endif
	
	if( inNonce ) memcpy( inContext->nonce, inNonce, kAES_CGM_Size );
	
exit:
	return( err );
}

//===========================================================================================================================
//	AES_GCM_Final
//===========================================================================================================================

void	AES_GCM_Final( AES_GCM_Context *inContext )
{
#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
	if( inContext->cryptor ) CCCryptorRelease( inContext->cryptor );
#elif( AES_UTILS_HAS_GLADMAN_GCM )
	gcm_end( &inContext->ctx );
#else
	#error "GCM enabled, but no implementation?"
#endif
	MemZeroSecure( inContext, sizeof( *inContext ) );
}

//===========================================================================================================================
//	AES_GCM_InitMessage
//===========================================================================================================================

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
OSStatus	AES_GCM_InitMessage( AES_GCM_Context *inContext, const uint8_t inNonce[ kAES_CGM_Size ] )
{
	CCCryptorRef const		cryptor = inContext->cryptor;
	OSStatus				err;
	
	err = CCCryptorGCMReset( cryptor );
	require_noerr( err, exit );
	
	if( inNonce == kAES_CGM_Nonce_Auto )
	{
		AES_CTR_Increment( inContext->nonce );
		inNonce = inContext->nonce;
	}
	err = CCCryptorGCMAddIV( cryptor, inNonce, kAES_CGM_Size );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#elif( AES_UTILS_HAS_GLADMAN_GCM )
OSStatus	AES_GCM_InitMessage( AES_GCM_Context *inContext, const uint8_t inNonce[ kAES_CGM_Size ] )
{
	OSStatus		err;
	
	if( inNonce == kAES_CGM_Nonce_Auto )
	{
		AES_CTR_Increment( inContext->nonce );
		inNonce = inContext->nonce;
	}
	err = gcm_init_message( inNonce, kAES_CGM_Size, &inContext->ctx );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	AES_GCM_FinalizeMessage
//===========================================================================================================================

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
OSStatus	AES_GCM_FinalizeMessage( AES_GCM_Context *inContext, uint8_t outAuthTag[ kAES_CGM_Size ] )
{
	OSStatus		err;
	size_t			len;
	
	len = kAES_CGM_Size;
	err = CCCryptorGCMFinal( inContext->cryptor, outAuthTag, &len );
	require_noerr( err, exit );
	require_action( len == kAES_CGM_Size, exit, err = kSizeErr );
	
exit:
	return( err );
}
#elif( AES_UTILS_HAS_GLADMAN_GCM )
OSStatus	AES_GCM_FinalizeMessage( AES_GCM_Context *inContext, uint8_t outAuthTag[ kAES_CGM_Size ] )
{
	OSStatus		err;
	
	err = gcm_compute_tag( outAuthTag, kAES_CGM_Size, &inContext->ctx );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	AES_GCM_VerifyMessage
//===========================================================================================================================

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
OSStatus	AES_GCM_VerifyMessage( AES_GCM_Context *inContext, const uint8_t inAuthTag[ kAES_CGM_Size ] )
{
	OSStatus		err;
	size_t			len;
	uint8_t			authTag[ kAES_CGM_Size ];
	
	len = kAES_CGM_Size;
	err = CCCryptorGCMFinal( inContext->cryptor, authTag, &len );
	require_noerr( err, exit );
	require_action( len == kAES_CGM_Size, exit, err = kSizeErr );
	require_action_quiet( memcmp_constant_time( authTag, inAuthTag, len ) == 0, exit, err = kAuthenticationErr );
	
exit:
	return( err );
}
#elif( AES_UTILS_HAS_GLADMAN_GCM )
OSStatus	AES_GCM_VerifyMessage( AES_GCM_Context *inContext, const uint8_t inAuthTag[ kAES_CGM_Size ] )
{
	OSStatus		err;
	uint8_t			authTag[ kAES_CGM_Size ];
	
	err = gcm_compute_tag( authTag, kAES_CGM_Size, &inContext->ctx );
	require_noerr( err, exit );
	require_action_quiet( memcmp_constant_time( authTag, inAuthTag, kAES_CGM_Size ) == 0, exit, err = kAuthenticationErr );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	AES_GCM_AddAAD
//===========================================================================================================================

OSStatus	AES_GCM_AddAAD( AES_GCM_Context *inContext, const void *inPtr, size_t inLen )
{
	OSStatus		err;
	
#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
	err = CCCryptorGCMAddAAD( inContext->cryptor, inPtr, inLen );
	require_noerr( err, exit );
#elif( AES_UTILS_HAS_GLADMAN_GCM )
	err = gcm_auth_header( inPtr, inLen, &inContext->ctx );
	require_noerr( err, exit );
#else
	#error "GCM enabled, but no implementation?"
#endif
	
exit:
	return( err );
}

//===========================================================================================================================
//	AES_GCM_Encrypt
//===========================================================================================================================

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
OSStatus	AES_GCM_Encrypt( AES_GCM_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus		err;
	
	err = CCCryptorGCMEncrypt( inContext->cryptor, inSrc, inLen, inDst );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#elif( AES_UTILS_HAS_GLADMAN_GCM )
OSStatus	AES_GCM_Encrypt( AES_GCM_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus		err;
	
	err = gcm_encrypt( inDst, inSrc, inLen, &inContext->ctx );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	AES_GCM_Decrypt
//===========================================================================================================================

#if( AES_UTILS_HAS_COMMON_CRYPTO_GCM )
OSStatus	AES_GCM_Decrypt( AES_GCM_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus		err;
	
	err = CCCryptorGCMDecrypt( inContext->cryptor, inSrc, inLen, inDst );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#elif( AES_UTILS_HAS_GLADMAN_GCM )
OSStatus	AES_GCM_Decrypt( AES_GCM_Context *inContext, const void *inSrc, size_t inLen, void *inDst )
{
	OSStatus		err;
	
	err = gcm_decrypt( inDst, inSrc, inLen, &inContext->ctx );
	require_noerr( err, exit );
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	AES_GCM_Test
//===========================================================================================================================

#if( DEBUG )

typedef struct
{
	const char *	name;
	uint8_t			key[ kAES_CGM_Size ];
	uint8_t			pt[ 128 ];
	size_t			ptLen;
	uint8_t			aad[ 128 ];
	uint8_t			aadLen;
	uint8_t			iv[ kAES_CGM_Size ];
	uint8_t			ct[ 128 ];
	uint8_t			tag[ kAES_CGM_Size ];
	
}	AES_GCM_TestVector;

static const AES_GCM_TestVector		kAES_GCM_TestVectors[] =
{
	{
		/* name */	"Test case #46 from libtom", 
		/* key */	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		/* PT */	
		{
			0xa2, 0xaa, 0xb3, 0xad, 0x8b, 0x17, 0xac, 0xdd, 
			0xa2, 0x88, 0x42, 0x6c, 0xd7, 0xc4, 0x29, 0xb7, 
			0xca, 0x86, 0xb7, 0xac, 0xa0, 0x58, 0x09, 0xc7, 
			0x0c, 0xe8, 0x2d, 0xb2, 0x57, 0x11, 0xcb, 0x53,
			0x02, 0xeb, 0x27, 0x43, 0xb0, 0x36, 0xf3, 0xd7, 
			0x50, 0xd6, 0xcf, 0x0d, 0xc0, 0xac, 0xb9, 0x29, 
			0x50, 0xd5, 0x46, 0xdb, 0x30, 0x8f, 0x93, 0xb4, 
			0xff, 0x24, 0x4a, 0xfa, 0x9d, 0xc7, 0x2b, 0xcd,
			0x75, 0x8d, 0x2c
		}, 67, 
		/* aad */	{ 0x68, 0x8e, 0x1a, 0xa9, 0x84, 0xde, 0x92, 0x6d, 0xc7, 0xb4, 0xc4, 0x7f, 0x44 }, 13, 
		/* iv */	{ 0xb7, 0x21, 0x38, 0xb5, 0xa0, 0x5f, 0xf5, 0x07, 0x0e, 0x8c, 0xd9, 0x41, 0x83, 0xf7, 0x61, 0xd8 },
		/* CT */	
		{
			0xcb, 0xc8, 0xd2, 0xf1, 0x54, 0x81, 0xa4, 0xcc, 
			0x7d, 0xd1, 0xe1, 0x9a, 0xaa, 0x83, 0xde, 0x56, 
			0x78, 0x48, 0x3e, 0xc3, 0x59, 0xae, 0x7d, 0xec, 
			0x2a, 0xb8, 0xd5, 0x34, 0xe0, 0x90, 0x6f, 0x4b,
			0x46, 0x63, 0xfa, 0xff, 0x58, 0xa8, 0xb2, 0xd7, 
			0x33, 0xb8, 0x45, 0xee, 0xf7, 0xc9, 0xb3, 0x31, 
			0xe9, 0xe1, 0x0e, 0xb2, 0x61, 0x2c, 0x99, 0x5f, 
			0xeb, 0x1a, 0xc1, 0x5a, 0x62, 0x86, 0xcc, 0xe8,
			0xb2, 0x97, 0xa8
		}, 
		/* Tag */	{ 0x8d, 0x2d, 0x2a, 0x93, 0x72, 0x62, 0x6f, 0x6b, 0xee, 0x85, 0x80, 0x27, 0x6a, 0x63, 0x66, 0xbf }
	}
};

OSStatus	AES_GCM_Test( void );
OSStatus	AES_GCM_Test( void )
{
	OSStatus			err;
	AES_GCM_Context		gcm, gcm2;
	size_t				i;
	uint8_t				buf[ 1024 ];
	uint8_t				buf2[ 1024 ];
	uint8_t				tag[ kAES_CGM_Size ];
	uint8_t				pkt[ 1500 ];
	uint64_t			ticks, encryptTicks, decryptTicks;
	
	for( i = 0; i < countof( kAES_GCM_TestVectors ); ++i )
	{
		const AES_GCM_TestVector *		test = &kAES_GCM_TestVectors[ i ];
		
		dlog( kLogLevelMax, "AES-GCM Test: %s\n", test->name );
		
		// Encrypt
		
		err = AES_GCM_Init( &gcm, test->key, kAES_CGM_Nonce_None );
		require_noerr( err, exit );
		
		err = AES_GCM_InitMessage( &gcm, test->iv );
		require_noerr( err, exit );
		
		err = AES_GCM_AddAAD( &gcm, test->aad, test->aadLen );
		require_noerr( err, exit );
		
		err = AES_GCM_Encrypt( &gcm, test->pt, test->ptLen, buf );
		require_noerr( err, exit );
		require_action( memcmp( buf, test->ct, test->ptLen ) == 0, exit, err = -1;
			dlog( kLogLevelError, "Bad:\n%1.1H\nGood:\n%1.1H\n\n", 
			buf,      (int) test->ptLen, (int) test->ptLen, 
			test->ct, (int) test->ptLen, (int) test->ptLen ) );
		
		err = AES_GCM_FinalizeMessage( &gcm, tag );
		require_noerr( err, exit );
		require_action( memcmp( tag, test->tag, sizeof( tag ) ) == 0, exit, err = -1 );
		
		AES_GCM_Final( &gcm );
		
		// Decrypt good
		
		err = AES_GCM_Init( &gcm, test->key, kAES_CGM_Nonce_None );
		require_noerr( err, exit );
		
		err = AES_GCM_InitMessage( &gcm, test->iv );
		require_noerr( err, exit );
		
		err = AES_GCM_AddAAD( &gcm, test->aad, test->aadLen );
		require_noerr( err, exit );
		
		err = AES_GCM_Decrypt( &gcm, buf, test->ptLen, buf2 );
		require_noerr( err, exit );
		require_action( memcmp( buf2, test->pt, test->ptLen ) == 0, exit, err = -1 );
		
		err = AES_GCM_VerifyMessage( &gcm, test->tag );
		require_noerr( err, exit );
		
		AES_GCM_Final( &gcm );
		
		// Decrypt bad
		
		err = AES_GCM_Init( &gcm, test->key, kAES_CGM_Nonce_None );
		require_noerr( err, exit );
		
		err = AES_GCM_InitMessage( &gcm, test->iv );
		require_noerr( err, exit );
		
		err = AES_GCM_AddAAD( &gcm, test->aad, test->aadLen );
		require_noerr( err, exit );
		
		buf[ test->ptLen / 2 ] ^= 0x01;
		err = AES_GCM_Decrypt( &gcm, buf, test->ptLen, buf2 );
		require_noerr( err, exit );
		require_action( memcmp( buf2, test->pt, test->ptLen ) != 0, exit, err = -1 );
		
		err = AES_GCM_VerifyMessage( &gcm, test->tag );
		require_action( err != kNoErr, exit, err = -1 );
		
		AES_GCM_Final( &gcm );
	}
	
	// Performance test.
	
	err = AES_GCM_Init( &gcm, 
		(const uint8_t *) "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF",
		(const uint8_t *) "\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF\x00" );
	require_noerr( err, exit );
	
	err = AES_GCM_Init( &gcm2, 
		(const uint8_t *) "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF",
		(const uint8_t *) "\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF\x00" );
	require_noerr( err, exit );
	
	encryptTicks = 0;
	decryptTicks = 0;
	memset( pkt, 'z', sizeof( pkt ) );
	for( i = 0; i < 10000; ++i )
	{
		// Encrypt
		
		memset( tag, 0, sizeof( tag ) );
		
		ticks = UpTicks();
		
		err = AES_GCM_InitMessage( &gcm, kAES_CGM_Nonce_Auto );
		require_noerr( err, exit );
		
		err = AES_GCM_AddAAD( &gcm, pkt, 12 );
		require_noerr( err, exit );
		
		err = AES_GCM_Encrypt( &gcm, &pkt[ 12 ], sizeof( pkt ) - 12, &pkt[ 12 ] );
		require_noerr( err, exit );
		
		err = AES_GCM_FinalizeMessage( &gcm, tag );
		require_noerr( err, exit );
		
		encryptTicks += ( UpTicks() - ticks );
		
		// Decrypt
		
		ticks = UpTicks();
		
		err = AES_GCM_InitMessage( &gcm2, kAES_CGM_Nonce_Auto );
		require_noerr( err, exit );
		
		err = AES_GCM_AddAAD( &gcm2, pkt, 12 );
		require_noerr( err, exit );
		
		err = AES_GCM_Decrypt( &gcm2, &pkt[ 12 ], sizeof( pkt ) - 12, &pkt[ 12 ] );
		require_noerr( err, exit );
		
		err = AES_GCM_VerifyMessage( &gcm2, tag );
		require_noerr( err, exit );
		
		decryptTicks += ( UpTicks() - ticks );
	}
	dlog( kLogLevelMax, "Encrypt %llu µSec, Decrypt: %llu µSec\n", 
		UpTicksToMicroseconds( encryptTicks / i ), 
		UpTicksToMicroseconds( decryptTicks / i ) );
	
	AES_GCM_Final( &gcm );
	AES_GCM_Final( &gcm2 );
	
exit:
	dlog( kLogLevelMax, "%###s: %s\n", __ROUTINE__, !err ? "PASSED" : "FAILED" );
	return( err );
}
#endif // DEBUG
#endif // AES_UTILS_HAS_GCM

#if 0
#pragma mark -
#endif

#if( AES_UTILS_USE_WINDOWS_API )
//===========================================================================================================================
//	_CreateWindowsCryptoAPIContext
//===========================================================================================================================

static OSStatus	_CreateWindowsCryptoAPIContext( HCRYPTPROV *outProvider, HCRYPTKEY *outKey, const uint8_t *inKey, DWORD inMode )
{
	OSStatus		err;
	HCRYPTPROV		provider = 0;
	HCRYPTKEY		key = 0;
	BOOL			good;
	AESKeyBlob		blob;
	
	good = CryptAcquireContext( &provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
	memset( &blob, 0, sizeof( blob ) );
	blob.hdr.bType		= PLAINTEXTKEYBLOB;
	blob.hdr.bVersion	= CUR_BLOB_VERSION;
	blob.hdr.reserved	= 0;
	blob.hdr.aiKeyAlg	= CALG_AES_128;
	blob.len			= kAES_CTR_Size;
	memcpy( blob.key, inKey, kAES_CTR_Size );
	
	good = CryptImportKey( provider, (const BYTE *) &blob, (DWORD) sizeof( blob ), 0, 0, &key );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
	good = CryptSetKeyParam( key, KP_MODE, (BYTE *) &inMode, 0 );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
	*outProvider = provider;
	provider = 0;

	*outKey = key;
	key = 0;

exit:
	if( key )		CryptDestroyKey( key );
	if( provider )	CryptReleaseContext( provider, 0 );
	return( err );
}
#endif

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

