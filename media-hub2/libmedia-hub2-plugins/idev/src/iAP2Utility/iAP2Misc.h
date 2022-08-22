/*
 *	File: iAP2Misc.h
 *	Package: iAP2Utility
 *	Abstract: n/a
 *
 *	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple
 * 	Inc. ("Apple"), in your capacity as a current, and in good standing,
 *	Licensee in the MFi Licensing Program. Use of this Apple software is
 *	governed by and subject to the terms and conditions of your MFi License,
 *	including, but not limited to, the restrictions specified in the provision
 *	entitled “Public Software”, and is further subject to your agreement to
 *	the following additional terms, and your agreement that the use,
 *	installation, modification or redistribution of this Apple software
 * 	constitutes acceptance of these additional terms. If you do not agree with
 * 	these additional terms, please do not use, install, modify or redistribute
 *	this Apple software.
 *
 *	In consideration of your agreement to abide by the following terms, and
 *	subject to these terms, Apple grants you a personal, non-exclusive
 *	license, under Apple's copyrights in this original Apple software (the
 *	"Apple Software"), to use, reproduce, and modify the Apple Software in
 *	source form, and to use, reproduce, modify, and redistribute the Apple
 *	Software, with or without modifications, in binary form. While you may not
 *	redistribute the Apple Software in source form, should you redistribute
 *	the Apple Software in binary form, in its entirety and without
 *	modifications, you must retain this notice and the following text and
 *	disclaimers in all such redistributions of the Apple Software. Neither the
 *	name, trademarks, service marks, or logos of Apple Inc. may be used to
 *	endorse or promote products derived from the Apple Software without
 *	specific prior written permission from Apple. Except as expressly stated
 *	in this notice, no other rights or licenses, express or implied, are
 *	granted by Apple herein, including but not limited to any patent rights
 *	that may be infringed by your derivative works or by other works in which
 *	the Apple Software may be incorporated.
 *
 *	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES
 *	NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 *	IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 *	PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 *	ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 *	WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 *	LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 *	OF SUCH DAMAGE.
 *
 *	Copyright (C) 2012 Apple Inc. All Rights Reserved.
 *
 */


#ifndef iapd_iAP2Misc_h
#define iapd_iAP2Misc_h

/////////////////////////////////////////////////////////////////
//
//  iAP2MiscRead64
//
//  Input:
//      PTR:   pointer to a memory location to read 64 bits in
//             system format
//
//  Output:
//      None
//
//  Return:
//      uint64_t    byte read in the system (big endian format)
//
/////////////////////////////////////////////////////////////////

#define IAP2_MISC_READ_BIG64( PTR) \
    (uint64_t)( ( ( (uint64_t)( (uint8_t *)(PTR) )[ 0 ] ) << 56 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 1 ] ) << 48 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 2 ] ) << 40 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 3 ] ) << 32 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 4 ] ) << 24 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 5 ] ) << 16 )	| \
                ( ( (uint64_t)( (uint8_t *)(PTR) )[ 6 ] ) <<  8 )	| \
                  ( (uint64_t)( (uint8_t *)(PTR) )[ 7 ] ) )

/////////////////////////////////////////////////////////////////
//
//  iAP2MiscWrite64
//
//  Input:
//      PTR:   byte address to write value X in system format
//        X:   Value to write at byte address PTR
//
//  Output:
//      None
//
//  Return:
//      
//
/////////////////////////////////////////////////////////////////

#define	IAP2_MISC_WRITE_BIG64( PTR, X ) \
    do \
    { \
        ( (uint8_t *)(PTR) )[ 0 ] = (uint8_t)( ( (X) >> 56 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 1 ] = (uint8_t)( ( (X) >> 48 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 2 ] = (uint8_t)( ( (X) >> 40 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 3 ] = (uint8_t)( ( (X) >> 32 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 4 ] = (uint8_t)( ( (X) >> 24 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 5 ] = (uint8_t)( ( (X) >> 16 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 6 ] = (uint8_t)( ( (X) >>  8 ) & 0xFF ); \
        ( (uint8_t *)(PTR) )[ 7 ] = (uint8_t)(   (X)         & 0xFF ); \
        \
    }	while( 0 )


#endif
