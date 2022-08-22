/*
	File:    	HIDUtils.c
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
	
	Copyright (C) 2012-2014 Apple Inc. All Rights Reserved.
*/

#include "HIDUtils.h"

#include "APSCommonServices.h"
#include "MathUtils.h"
#include "ThreadUtils.h"

	#define kHIDPage_Telephony					0x0B
	#define kHIDUsage_Tfon_HookSwitch			0x20
	#define kHIDUsage_Tfon_Flash				0x21
	#define kHIDUsage_Tfon_Drop					0x26
	#define kHIDUsage_Tfon_PhoneKey0			0xB0
	#define kHIDUsage_Tfon_PhoneKey1			0xB1
	#define kHIDUsage_Tfon_PhoneKey2			0xB2
	#define kHIDUsage_Tfon_PhoneKey3			0xB3
	#define kHIDUsage_Tfon_PhoneKey4			0xB4
	#define kHIDUsage_Tfon_PhoneKey5			0xB5
	#define kHIDUsage_Tfon_PhoneKey6			0xB6
	#define kHIDUsage_Tfon_PhoneKey7			0xB7
	#define kHIDUsage_Tfon_PhoneKey8			0xB8
	#define kHIDUsage_Tfon_PhoneKey9			0xB9
	#define kHIDUsage_Tfon_PhoneKeyStar			0xBA
	#define kHIDUsage_Tfon_PhoneKeyPound		0xBB

	#define kHIDPage_Consumer					0x0C
	#define kHIDUsage_Csmr_ACHome				0x223
	#define kHIDUsage_Csmr_Record				0xB2
	#define kHIDUsage_Csmr_Play					0xB0
	#define kHIDUsage_Csmr_Pause				0xB1
	#define kHIDUsage_Csmr_PlayOrPause			0xCD
	#define kHIDUsage_Csmr_ScanNextTrack		0xB5
	#define kHIDUsage_Csmr_ScanPreviousTrack	0xB6

#if 0
#pragma mark == Overrides ==
#endif

//===========================================================================================================================
//	HIDButtonsCreateDescriptor
//===========================================================================================================================

OSStatus	HIDButtonsCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
	static const uint8_t		kDescriptorTemplate[] = 
	{
		0x05, 0x0C,			// Usage Page (Consumer)
		0x09, 0x01,			// Usage 1 (0x1)
		0xA1, 0x01,			// Collection (Application)
			0x15, 0x00,			// Logical Minimum......... (0)
			0x25, 0x06,			// Logical Maximum......... (6)
			0x05, 0x0C,			// Usage Page (Consumer)
			0x0A, 0x00, 0x00,	// Usage 0 (0x0) 		// Unassigned
			0x0A, 0x23, 0x02,	// Usage 547 (0x223) 	// AC Home
			0x0A, 0xB0, 0x00,	// Usage 176 (0xb0) 	// Play
			0x0A, 0xB1, 0x00,	// Usage 177 (0xb1) 	// Pause
			0x0A, 0xCD, 0x00,	// Usage 205 (0xcd) 	// Play / Pause
			0x0A, 0xB5, 0x00,	// Usage 181 (0xb5) 	// Scan Next Track
			0x0A, 0xB6, 0x00,	// Usage 182 (0xb6) 	// Scan Previous Track
			0x75, 0x08,			// Report Size............. (8)
			0x95, 0x01,			// Report Count............ (1)
			0x81, 0x00,			// Input...................(Data, Array, Absolute)
		0xC0				// End Collection
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDDPadCreateDescriptor
//===========================================================================================================================

OSStatus	HIDDPadCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
	static const uint8_t		kDescriptorTemplate[] =
	{
		0x05, 0x0c,
		0x09, 0x01,
		0xA1, 0x01,
			// mask 0
			0x15, 0x00, // logical min
			0x25, 0x01, // logical max
			0x75, 0x01, // report size
			
			0x0a, 0x23, 0x02,// ac home
			0x0a, 0x24, 0x02,// ac back
			0x95, 0x02, // report count 2
			0x81, 0x02,
			
			0x95, 0x06, // report count 6
			0x81, 0x01, // constant
			
			// mask 1
			0x19, 0x40, // usage min menu
			0x29, 0x46, // usage max escape
			0x95, 0x07, // report count 7
			0x81, 0x02,
			
			0x95, 0x01, // report count 5
			0x81, 0x01, // constant
		0xC0
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDDPadFillReport
//===========================================================================================================================

void
	HIDDPadFillReport(
		uint8_t	inReport[ 2 ],
		Boolean	inUp, 
		Boolean	inDown, 
		Boolean	inLeft, 
		Boolean	inRight, 
		Boolean	inSelect, 
		Boolean	inHome, 
		Boolean	inMenu, 
		Boolean	inBack )
{
	inReport[ 0 ] = (uint8_t)( ( inHome << 0 ) | ( inBack   << 1 ) );
	inReport[ 1 ] = (uint8_t)( ( inMenu << 0 ) | ( inSelect << 1 ) | ( inUp    << 2 ) |
							   ( inDown << 3 ) | ( inLeft   << 4 ) | ( inRight << 5 ) | ( inBack << 6 ) );
}

//===========================================================================================================================
//	HIDKnobCreateDescriptor
//===========================================================================================================================

OSStatus	HIDKnobCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
	static const uint8_t		kDescriptorTemplate[] = 
	{
		0x05, 0x01, 
		0x09, 0x08,
		0xA1, 0x01, 
			// Button
			0x15, 0x00,
			0x25, 0x01,
			0x75, 0x01,
			0x05, 0x09,
			0x09, 0x01,
			0x95, 0x01,
			0x81, 0x02,
			
			// consumer
			0x05, 0x0c,
			0x0a, 0x23, 0x02,// ac home
			0x0a, 0x24, 0x02,// ac back
			0x95, 0x02,
			0x81, 0x02,
			
			// Constant
			0x95, 0x05,
			0x81, 0x01,
			
			// Pointer
			0x05, 0x01,
			0x09, 0x01,
			0xA1, 0x00,
				0x09, 0x30, 
				0x09, 0x31, 
				
				// log min/max
				0x15, 0x81,
				0x25, 0x7f,
				
				// report size
				0x75, 0x08,
				// report count
				0x95, 0x02,
				// variable, absolute
				0x81, 0x02,
			0xC0,
			
			// Wheel Padding
			0x09, 0x38,
			// log min/max
			0x15, 0x81,
			0x25, 0x7f,
			// report size
			0x75, 0x08,
			// report count
			0x95, 0x01,
			// variable, relative
			0x81, 0x06,
		0xC0
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDKnobFillReport
//===========================================================================================================================

void
	HIDKnobFillReport( 
		uint8_t	inReport[ 4 ], 
		Boolean	inButton, 
		Boolean	inHome, 
		Boolean	inBack,
		int8_t	inX, 
		int8_t	inY, 
		int8_t	inWheel )
{
	inReport[ 0 ] = (uint8_t)( inButton | ( inHome << 1 ) | ( inBack << 2 ) );
	inReport[ 1 ] = (uint8_t) inX;
	inReport[ 2 ] = (uint8_t) inY;
	inReport[ 3 ] = (uint8_t) inWheel;
}

//===========================================================================================================================
//	HIDPointingCreateDescriptor
//===========================================================================================================================

OSStatus	HIDPointingCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
	static const uint8_t		kDescriptorTemplate[] = 
	{
		0x05, 0x0D, 				// usage page (digitizers)
		0x09, 0x04, 				// usage (touch screen)
		0xA1, 0x01, 				// collection (application)
			
			// Buttons
			0x05, 0x09, 			// usage page (button)
			0x19, 0x01, 			// usage min (1)
			0x29, 0x08, 			// usage max (8)
			0x15, 0x00, 			// logical min (0)
			0x25, 0x01, 			// logical max (1)
			0x95, 0x08, 			// report count (8)
			0x75, 0x01, 			// report size (1)
			0x81, 0x02, 			// input (2)
			
			// Constant
			0x95, 0x00, 			// report count (0)
			0x75, 0x01, 			// report size (1)
			0x81, 0x01, 			// input (1)
			
			// Pointer
			0x05, 0x01, 			// usage page (generic desktop)
			0x09, 0x01, 			// usage (pointer)
			0xA1, 0x00, 			// collection (physical) 
				0x09, 0x30, 		// usage (X)
				0x09, 0x31, 		// usage (Y)
				0x16, 0x00, 0x00, 	// logical min (0) 
				0x26, 0xff, 0x7f, 	// logical max (0x7fff)
				0x36, 0x00, 0x00, 	// physical max (0) 
				0x46, 0x00, 0x00, 	// physical min (0)
				0x55, 0x00, 		// unit exponent (0)
				0x65, 0x00, 		// unit (0)
				0x75, 0x10, 		// report size (16) 
				0x95, 0x02, 		// report count (2)
				0x81, 0x02, 		// input (0x2)
			0xC0, 					// end collection
		0xC0 						// end collection
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDPointingFillReport
//===========================================================================================================================

void	HIDPointingFillReport( uint8_t inReport[ 5 ], uint8_t inButtons, double inX, double inY )
{
	uint16_t		x, y;
	
	x = (uint16_t) TranslateValue( inX, 0.0, 960, 0, 960 );
	y = (uint16_t) TranslateValue( inY, 0.0, 540.0, 0, 540 );
	inReport[ 0 ] = inButtons;
	inReport[ 1 ] = (uint8_t)(   x        & 0xFF );
	inReport[ 2 ] = (uint8_t)( ( x >> 8 ) & 0xFF );
	inReport[ 3 ] = (uint8_t)(   y        & 0xFF );
	inReport[ 4 ] = (uint8_t)( ( y >> 8 ) & 0xFF );
}

//===========================================================================================================================
//	HIDSimpleFillReport
//===========================================================================================================================

OSStatus	HIDSimpleFillReport( uint8_t inReport[ 1 ], uint32_t inUsagePage, uint32_t inUsageCode, Boolean inDown )
{
	uint8_t		selector;
	
	switch( inUsagePage )
	{
		case kHIDPage_Telephony:	
			switch( inUsageCode )
			{
				case kHIDUsage_Tfon_HookSwitch:			selector =  1; break;
				case kHIDUsage_Tfon_Flash:				selector =  2; break;
				case kHIDUsage_Tfon_Drop:				selector =  3; break;
				case kHIDUsage_Tfon_PhoneKey0:			selector =  4; break;
				case kHIDUsage_Tfon_PhoneKey1:			selector =  5; break;
				case kHIDUsage_Tfon_PhoneKey2:			selector =  6; break;
				case kHIDUsage_Tfon_PhoneKey3:			selector =  7; break;
				case kHIDUsage_Tfon_PhoneKey4:			selector =  8; break;
				case kHIDUsage_Tfon_PhoneKey5:			selector =  9; break;
				case kHIDUsage_Tfon_PhoneKey6:			selector = 10; break;
				case kHIDUsage_Tfon_PhoneKey7:			selector = 11; break;
				case kHIDUsage_Tfon_PhoneKey8:			selector = 12; break;
				case kHIDUsage_Tfon_PhoneKey9:			selector = 13; break;
				case kHIDUsage_Tfon_PhoneKeyStar:		selector = 14; break;
				case kHIDUsage_Tfon_PhoneKeyPound:		selector = 15; break;
				default:								return( kParamErr );
			}
			break;
		
		case kHIDPage_Consumer:	
			switch( inUsageCode )
			{
				case kHIDUsage_Csmr_ACHome:				selector = 1; break;
				case kHIDUsage_Csmr_Play:				selector = 2; break;
				case kHIDUsage_Csmr_Pause:				selector = 3; break;
				case kHIDUsage_Csmr_PlayOrPause:		selector = 4; break;
				case kHIDUsage_Csmr_ScanNextTrack:		selector = 5; break;
				case kHIDUsage_Csmr_ScanPreviousTrack:	selector = 6; break;
				default:								return( kParamErr );
			}
			break;
		
		default:
			return( kParamErr );
	}
	
	inReport[ 0 ] = inDown ? selector : 0;
	return( kNoErr );
}

//===========================================================================================================================
//	HIDTelephonyCreateDescriptor
//===========================================================================================================================

OSStatus	HIDTelephonyCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
	static const uint8_t		kDescriptorTemplate[] = 
	{
		0x05, 0x0B, // usage page
		0x09, 0x01, // usage
		0xA1, 0x01, // collection
			// Telephony
			0x15, 0x00, // logical min
			0x25, 0x0F, // logical max
			0x05, 0x0B, // usage page
			0x09, 0x00, // Unassigned
			0x09, 0x20, // hook
			0x09, 0x21, // flash
			0x09, 0x26, // drop
			0x09, 0xB0, // PhoneKey 0
			0x09, 0xB1, // PhoneKey 1
			0x09, 0xB2, // PhoneKey 2
			0x09, 0xB3, // PhoneKey 3
			0x09, 0xB4, // PhoneKey 4
			0x09, 0xB5, // PhoneKey 5
			0x09, 0xB6, // PhoneKey 6
			0x09, 0xB7, // PhoneKey 7
			0x09, 0xB8, // PhoneKey 8
			0x09, 0xB9, // PhoneKey 9
			0x09, 0xBA, // PhoneKey Star
			0x09, 0xBB, // PhoneKey Pound
			0x75, 0x08, // report size
			0x95, 0x01, // report count
			0x81, 0x00, // array
		0xC0 // end collection
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDTrackpadCreateDescriptor
//===========================================================================================================================

OSStatus HIDTrackpadCreateDescriptor( uint8_t **outDescriptor, size_t *outLen )
{
    static const uint8_t	kDescriptorTemplate[] = {
		0x05, 0x0D,			// Usage Page (Digitizer)
		0x09, 0x05,			// Usage (Touch Pad)
		0xA1, 0x01,			// Collection (Application)
			0x05, 0x0D,			// Usage Page (Digitizer)
			0x09, 0x22,			// Usage (Finger)
			0xA1, 0x02,			// Collection (Logical)
				0x05, 0x0D,			// Usage Page (Digitizer)
				0x09, 0x33,			// Usage (Touch)
				0x15, 0x00,			// Logical Minimum......... (0)
				0x25, 0x01,			// Logical Maximum......... (1)
				0x75, 0x01,			// Report Size............. (1)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
				0x75, 0x07,			// Report Size............. (7)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x01,			// Input...................(Constant)
				0x05, 0x01,			// Usage Page (Generic Desktop)
				0x09, 0x30,			// Usage (X)
				0x15, 0x00,			// Logical Minimum......... (0)
				0x26, 0x00, 0x04,	// Logical Maximum......... (1024)
				0x75, 0x10,			// Report Size............. (16)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
				0x09, 0x31,			// Usage (Y)
				0x15, 0x00,			// Logical Minimum......... (0)
				0x26, 0x00, 0x04,	// Logical Maximum......... (1024)
				0x75, 0x10,			// Report Size............. (16)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
			0xC0,				// End Collection
			0x05, 0x0D,			// Usage Page (Digitizer)
			0x09, 0x24,			// Usage (Gesture Character)
			0xA1, 0x02,			// Collection (Logical)
				0x05, 0x0D,			// Usage Page (Digitizer)
				0x09, 0x63,			// Usage (Gesture Character Data)
				0x75, 0x20,			// Report Size............. (32)
				0x95, 0x01,			// Report Count............ (1)
				0x82, 0x02, 0x01,	// Input...................(Data, Variable, Absolute, Buffered bytes)
				0x09, 0x65,			// Usage (Gesture Character Encoding UTF8)
				0x09, 0x62,			// Usage (Gesture Character Data Length)
				0x75, 0x08,			// Report Size............. (8)
				0x95, 0x02,			// Report Count............ (2)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
				0x09, 0x61,			// Usage (Gesture Character Quality)
				0x15, 0x00,			// Logical Minimum......... (0)
				0x25, 0x64,			// Logical Maximum......... (100)
				0x75, 0x08,			// Report Size............. (8)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
			0xC0,				// End Collection
			0x05, 0x0D,			// Usage Page (Digitizer)
			0x09, 0x24,			// Usage (Gesture Character)
			0xA1, 0x02,			// Collection (Logical)
				0x05, 0x0D,			// Usage Page (Digitizer)
				0x09, 0x63,			// Usage (Gesture Character Data)
				0x75, 0x20,			// Report Size............. (32)
				0x95, 0x01,			// Report Count............ (1)
				0x82, 0x02, 0x01,	// Input...................(Data, Variable, Absolute, Buffered bytes)
				0x09, 0x65,			// Usage (Gesture Character Encoding UTF8)
				0x09, 0x62,			// Usage (Gesture Character Data Length)
				0x75, 0x08,			// Report Size............. (8)
				0x95, 0x02,			// Report Count............ (2)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
				0x09, 0x61,			// Usage (Gesture Character Quality)
				0x15, 0x00,			// Logical Minimum......... (0)
				0x25, 0x64,			// Logical Maximum......... (100)
				0x75, 0x08,			// Report Size............. (8)
				0x95, 0x01,			// Report Count............ (1)
				0x81, 0x02,			// Input...................(Data, Variable, Absolute)
			0xC0,				// End Collection
		0xC0				// End Collection
	};
	
	OSStatus		err;
	size_t			len;
	uint8_t *		desc;
	   
	len = sizeof( kDescriptorTemplate );
	desc = (uint8_t *) malloc( len );
	require_action( desc, exit, err = kNoMemoryErr );
	memcpy( desc, kDescriptorTemplate, len );
	
	*outDescriptor = desc;
	*outLen = len;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	HIDTrackpadFillReport
//===========================================================================================================================

void	HIDTrackpadFillReport(
			uint8_t inReport[ 19 ],
			uint8_t inCharacter1Length,
			uint8_t inCharacter1Data[ 4 ],
			uint8_t inCharacter1Quality,
			uint8_t inCharacter2Length,
			uint8_t inCharacter2Data[ 4 ],
			uint8_t inCharacter2Quality,
			uint8_t inTransducerState,
			uint16_t inTransducerX,
			uint16_t inTransducerY)
{
	inReport[0] = inTransducerState;
	WriteLittle16( &inReport[ 1 ], inTransducerX );
	WriteLittle16( &inReport[ 3 ], inTransducerY );
	
	// First candidate
	memcpy( &inReport[ 5 ], inCharacter1Data, inCharacter1Length );
	memset( &inReport[ 5 + inCharacter1Length ], 0, 4 - inCharacter1Length );
	inReport[  9 ]	= 1; // utf8 encoding
	inReport[ 10 ]	= inCharacter1Length;
	inReport[ 11 ]	= inCharacter1Quality;
	
	// Second candidate
	memcpy( &inReport[ 12 ], inCharacter2Data, inCharacter2Length );
	memset( &inReport[ 12 + inCharacter2Length ], 0, 4 - inCharacter2Length );
	inReport[ 16 ] = 1; // utf8 encoding
	inReport[ 17 ] = inCharacter2Length;
	inReport[ 18 ] = inCharacter2Quality;
}

#if 0
#pragma mark -
#endif

static pthread_mutex_t				gHIDOverrideLock	= PTHREAD_MUTEX_INITIALIZER;
static CFMutableDictionaryRef		gHIDOverrides		= NULL;


//===========================================================================================================================
//	HIDRegisterOverrideDescriptor
//===========================================================================================================================

OSStatus	HIDRegisterOverrideDescriptor( const HIDInfo *inInfo, const uint8_t *inPtr, size_t inLen )
{
	OSStatus	err;
	CFDataRef	key = NULL;
	CFDataRef	value = NULL;
	
	pthread_mutex_lock( &gHIDOverrideLock );
	
	if( !gHIDOverrides )
	{
		gHIDOverrides = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( gHIDOverrides, exit, err = kNoMemoryErr );
	}
	
	key = CFDataCreate( NULL, (const UInt8 *) inInfo, sizeof( HIDInfo ) );
	value = CFDataCreate( NULL, (const UInt8 *) inPtr, inLen );
	require_action( key && value, exit, err = kNoMemoryErr );
	
	CFDictionarySetValue( gHIDOverrides, key, value );
	
	err = kNoErr;
	
exit:
	pthread_mutex_unlock( &gHIDOverrideLock );
	CFReleaseNullSafe( key );
	CFReleaseNullSafe( value );
	
	return err;
}

//===========================================================================================================================
//	HIDDeregisterOverrideDescriptor
//===========================================================================================================================

OSStatus	HIDDeregisterOverrideDescriptor( const HIDInfo *inInfo )
{
	OSStatus	err;
	CFDataRef	key = NULL;
	
	pthread_mutex_lock( &gHIDOverrideLock );
	
	if( gHIDOverrides )
	{
		key = CFDataCreate( NULL, (const UInt8 *) inInfo, sizeof( HIDInfo ) );
		require_action( key, exit, err = kNoMemoryErr );
		
		CFDictionaryRemoveValue( gHIDOverrides, key );
	}

	err = kNoErr;
	
exit:
	pthread_mutex_unlock( &gHIDOverrideLock );
	CFReleaseNullSafe( key );
	
	return err;
}

//===========================================================================================================================
//	HIDGetOverrideDescriptor
//===========================================================================================================================

OSStatus	HIDGetOverrideDescriptor( const HIDInfo *inInfo, const uint8_t **outPtr, uint8_t **outStorage, size_t *outLen )
{
	OSStatus			err;
	const uint8_t *		ptr;
	uint8_t *			buf;
	size_t				len;
	
	pthread_mutex_lock( &gHIDOverrideLock );

	if( gHIDOverrides )
	{
		CFDataRef	key;
		CFDataRef	value;
		
		key = CFDataCreate( NULL, (const UInt8 *) inInfo, sizeof( HIDInfo ) );
		require_action( key, exit, err = kNoMemoryErr );
		
		value = (CFDataRef) CFDictionaryGetValue( gHIDOverrides, key );
		ForgetCF( &key );

		if( value )
		{
			len = CFDataGetLength( value );
			ptr = buf = malloc( *outLen );
			memcpy( buf, CFDataGetBytePtr( value ), len );
		}
		else
		{
			ptr = buf = NULL;
			len = 0;
		}
	}
	else
	{
		ptr = buf = NULL;
		len = 0;
	}

	require_action( ptr, exit, err = kNotFoundErr );

	*outPtr		= ptr;
	*outStorage	= buf;
	*outLen		= len;
	err			= kNoErr;
	
exit:
	pthread_mutex_unlock( &gHIDOverrideLock );

	return( err );
}
