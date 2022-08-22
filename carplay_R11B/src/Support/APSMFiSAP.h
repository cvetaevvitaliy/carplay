/*
	File:    	APSMFiSAP.h
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
/*!
	@header			MFi-SAP
	@discussion		APIs and platform interfaces for the Made for iPod (MFi) Security Association Protocol (SAP).
*/

#ifndef	__APSMFiSAP_h__
#define	__APSMFiSAP_h__

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================================================================
//	Configuration
//===========================================================================================================================

// APSMFI_SAP_ENABLE_SERVER: 1=Enable code for the server side of MFi-SAP. 0=Strip out server code.

#if( !defined( APSMFI_SAP_ENABLE_SERVER ) )
	#if( 0 || TARGET_OS_WINDOWS )
		#define APSMFI_SAP_ENABLE_SERVER		0
	#else
		#define APSMFI_SAP_ENABLE_SERVER		1
	#endif
#endif

// MFI_SAP_THROTTLE_SERVER: 1=Enable throttling code for server side of MFi-SAP. 0=Don't throttle requests.

#if( !defined( APSMFI_SAP_THROTTLE_SERVER ) )
	#define APSMFI_SAP_THROTTLE_SERVER			1
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		Creation and deletion.
	@internal
	@abstract	APIs for performing the Made for iPod (MFi) Security Association Protocol (SAP).
*/
#define kAPSMFiSAPVersion1		1 // Curve25519 ECDH key exchange, RSA signing, AES-128 CTR encryption.

typedef struct APSMFiSAP *		APSMFiSAPRef;

OSStatus	APSMFiSAP_Create( APSMFiSAPRef *outRef, uint8_t inVersion );
void		APSMFiSAP_Delete( APSMFiSAPRef inRef );
#define		APSMFiSAP_Forget( X )	do { if( *(X) ) { APSMFiSAP_Delete( *(X) ); *(X) = NULL; } } while( 0 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiSAP_Exchange
	@internal
	@abstract	Perform key exchange.
	@discussion
	
	This is called on both the client and server sides. Each side repeatedly cals APSMFiSAP_Exchange until it returns an
	error to indicate a failure or it returns kNoErr and sets "done" to true.
*/
OSStatus
	APSMFiSAP_Exchange(
		APSMFiSAPRef	inRef,
		const uint8_t *	inInputPtr,
		size_t			inInputLen, 
		uint8_t **		outOutputPtr,
		size_t *		outOutputLen, 
		Boolean *		outDone );
OSStatus	APSMFiSAP_Encrypt( APSMFiSAPRef inRef, const void *inPlaintext,  size_t inLen, void *inCiphertextBuf );
OSStatus	APSMFiSAP_Decrypt( APSMFiSAPRef inRef, const void *inCiphertext, size_t inLen, void *inPlaintextBuf );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiSAP_DeriveAESKey
	@internal
	@abstract	Derive AES key and IV from the shared secret.
*/
OSStatus
	APSMFiSAP_DeriveAESKey(
		APSMFiSAPRef		inRef,
		const void *	inKeySaltPtr, 
		size_t			inKeySaltLen, 
		const void *	inIVSaltPtr, 
		size_t			inIVSaltLen, 
		uint8_t			outKey[ 16 ], 
		uint8_t			outIV[ 16 ] );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_Initialize
	@abstract	Performs any platform-specific initialization needed.
	@discussion	MFi accessory implementors must provide this function.
*/
OSStatus	APSMFiPlatform_Initialize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_Finalize
	@abstract	Performs any platform-specific cleanup needed.
	@discussion	MFi accessory implementors must provide this function.
*/
void	APSMFiPlatform_Finalize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_CreateSignature
	@internal
	@abstract	Create an RSA signature from the specified SHA-1 digest using the MFi authentication coprocessor.
	
	@param		inDigestPtr			Ptr to 20-byte SHA-1 digest.
	@param		inDigestLen			Number of bytes in the digest. Must be 20.
	@param		outSignaturePtr		Receives malloc()'d ptr to RSA signature. Caller must free() on success.
	@param		outSignatureLen		Receives number of bytes in RSA signature.
	
	@discussion	MFi accessory implementors must provide this function.
*/
OSStatus
	APSMFiPlatform_CreateSignature(
		const void *	inDigestPtr, 
		size_t			inDigestLen, 
		uint8_t **		outSignaturePtr,
		size_t *		outSignatureLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_VerifySignature
	@internal
	@abstract	Verifies the MFi auth IC certificate and that the RSA signature is valid for the specified data.
	
	@param		inDataPtr			Data that was signed.
	@param		inDataLen			Number of bytes of data that was signed.
	@param		inSignaturePtr		RSA signature of a SHA-1 hash of the data.
	@param		inSignatureLen		Number of bytes in the signature.
	@param		inCertificatePtr	DER-encoded PKCS#7 message containing the certificate for the signing entity.
	@param		inCertificateLen	Number of bytes in inCertificatePtr.
	
	@discussion	This is only needed on the client side so accessory implementors do not need to provide this function.
*/
OSStatus
	APSMFiPlatform_VerifySignature(
		const void *	inDataPtr, 
		size_t			inDataLen, 
		const void *	inSignaturePtr, 
		size_t			inSignatureLen, 
		const void *	inCertificatePtr, 
		size_t			inCertificateLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_CopyCertificate
	@abstract	Copy the certificate from the MFi authentication coprocessor. 
	
	@param		outCertificatePtr	Receives malloc()'d ptr to a DER-encoded PKCS#7 message containing the certificate.
									Caller must free() on success.
	@param		outCertificateLen	Number of bytes in the DER-encoded certificate.
	
	@discussion	MFi accessory implementors must provide this function.
*/
OSStatus	APSMFiPlatform_CopyCertificate( uint8_t **outCertificatePtr, size_t *outCertificateLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiPlatform_VerifyCertificate
	@internal
	@abstract	Verifies that a certificate is a valid MFi auth IC certificate.
	
	@param		inCertificatePtr	DER-encoded PKCS#7 message containing the certificate for the signing entity.
	@param		inCertificateLen	Number of bytes in inCertificatePtr.
	
	@discussion	This is only needed on the client side so accessory implementors do not need to provide this function.
*/
OSStatus	APSMFiPlatform_VerifyCertificate( const void *inCertificatePtr, size_t inCertificateLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	APSMFiSAP_Test
	@internal
	@abstract	Unit test for the MFi-SAP library.
*/
OSStatus	APSMFiSAP_Test( void );

#ifdef __cplusplus
}
#endif

#endif	// __APSMFiSAP_h__
