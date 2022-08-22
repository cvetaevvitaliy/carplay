/*
	File:    	AirPlayReceiverIPCCommon.h
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
	
	Copyright (C) 2013-2014 Apple Inc. All Rights Reserved.
*/

#ifndef	__AirPlayReceiverIPCCommon_h__
#define	__AirPlayReceiverIPCCommon_h__

#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================================================================
//	Keys
//===========================================================================================================================

#define kAirPlayReceiverIPCKey_ModeChanges		CFSTR( "modeChanges" )	// [Data:AirPlayModeChanges]
#define kAirPlayReceiverIPCKey_ObjectID			CFSTR( "objectID" )		// [Number:uint32_t]
#define kAirPlayReceiverIPCKey_ObjectID2		CFSTR( "objectID2" )	// [Number:uint32_t]

//===========================================================================================================================
//	AirPlayReceiverServer
//===========================================================================================================================

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerCreate			1000
// Request:	<no data>
// Reply:	<w:objectID>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerRelease			1001
// Request:	<w:objectID> 
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerSetDelegate		1002
// Request:	<AirPlayReceiverServerSetDelegateIPCData>
// Reply:	<none>

typedef struct
{
	uint32_t		objectID;
	Boolean			control_f;
	Boolean			copyProperty_f;
	Boolean			setProperty_f;
	Boolean			sessionCreated_f;
	Boolean			sessionFailed_f;
	
}	AirPlayReceiverServerSetDelegateIPCData;

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerControl			1003
// Request:	<binary plist containing objectID, command, qualifier, request params>
// Reply:	<binary plist containing response params>

//---------------------------------------------------------------------------------------------------------------------------
//	AirPlayReceiverServer Delegate
//---------------------------------------------------------------------------------------------------------------------------

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_Control				1100
// Request:	<binary plist containing objectID, command, qualifier, request params>
// Reply:	<binary plist containing response params>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_CopyProperty		1101
// Request:	<binary plist containing objectID, property, qualifier>
// Reply:	<binary plist containing response property, errorCode>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SetProperty			1102
// Request:	<binary plist containing objectID, property, qualifier, value>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionCreated		1103
// Request:	<binary plist containing server objectID, session objectID>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverServerDelegate_SessionFailed		1104
// Request:	<binary plist containing server objectID, reasonCode>
// Reply:	none

//===========================================================================================================================
//	AirPlayReceiverSession
//===========================================================================================================================

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetDelegate		2000
// Request:	<AirPlayReceiverSessionSetDelegateIPCData>
// Reply:	<none>

typedef struct
{
	uint32_t		objectID;
	Boolean			initialize_f;
	Boolean			finalize_f;
	Boolean			control_f;
	Boolean			copyProperty_f;
	Boolean			setProperty_f;
	Boolean			modesChanged_f;
	Boolean			requestUI_f;
	Boolean			duckAudio_f;
	Boolean			unduckAudio_f;

	
}	AirPlayReceiverSessionSetDelegateIPCData;

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionControl					2001
// Request:	<binary plist containing objectID, command, qualifier, request params>
// Reply:	<binary plist containing response params>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionChangeModes				2002
// Request:	<binary plist containing objectID, AirPlayModeChanges, reasonStr>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionForceKeyFrame			2003
// Request:	<binary plist containing objectID>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestSiriAction		2004
// Request:	<binary plist containing objectID, AirPlaySiriAction>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionRequestUI				2005
// Request:	<binary plist containing objectID, URL>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetLimitedUI			2006
// Request: <binary plist containing objectID, Boolean>
// Reply:	<none>
#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionSetNightMode			2007
// Request: <binary plist containing objectID, Boolean>
// Reply:	<none>

//---------------------------------------------------------------------------------------------------------------------------
//	AirPlayReceiverSession Delegate
//---------------------------------------------------------------------------------------------------------------------------

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Finalize				2100
// Request:	<w:objectID>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_Control				2101
// Request:	<binary plist containing objectID, command, qualifier, request params>
// Reply:	<binary plist containing response params>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_CopyProperty			2102
// Request:	<binary plist containing objectID, property, qualifier>
// Reply:	<binary plist containing response property, errorCode>

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_SetProperty			2103
// Request:	<binary plist containing objectID, property, qualifier, value>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_ModesChanged			2105
// Request:	<AirPlayReceiverSessionModesChangedIPCData>
// Reply:	none

typedef struct
{
	uint32_t				objectID;
	AirPlayModeState		state;
	
}	AirPlayReceiverSessionModesChangedIPCData;

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_RequestUI				2106
// Request:	<w:objectID>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_DuckAudio				2107
// Request:	<binary plist containing objectID, duration, volume>
// Reply:	none

#define kAirPlayReceiverIPCOpcode_AirPlayReceiverSessionDelegate_UnduckAudio			2108
// Request:	<binary plist containing objectID, duration>
// Reply:	none

//===========================================================================================================================
//	HIDDevice
//===========================================================================================================================

#define kAirPlayReceiverIPCOpcode_HIDDeviceCreateVirtual		3000
// Request:	<binary plist containing params>
// Reply:	<w:objectID>

#define kAirPlayReceiverIPCOpcode_HIDDeviceRelease				3001
// Request:	<w:objectID> 
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_HIDDeviceCopyProperty			3002
// Request:	<binary plist containing objectID, property, qualifier>
// Reply:	<binary plist containing response property, errorCode>

#define kAirPlayReceiverIPCOpcode_HIDDeviceSetProperty			3003
// Request:	<binary plist containing objectID, property, qualifier, value>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_HIDDevicePostReport			3004
// Request:	<binary plist containing objectID, reportData>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_HIDPostReport					3005
// Request:	<binary plist containing UUID, reportData>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_HIDRegisterDevice				3006
// Request:	<w:objectID> 
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_HIDDeviceStop					3007
// Request:	<w:objectID> 
// Reply:	<none>

//===========================================================================================================================
//	Screen
//===========================================================================================================================

#define kAirPlayReceiverIPCOpcode_ScreenCreate					4000
// Request:	<binary plist containing params>
// Reply:	<w:objectID>

#define kAirPlayReceiverIPCOpcode_ScreenRelease					4001
// Request:	<w:objectID> 
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_ScreenCopyProperty			4002
// Request:	<binary plist containing objectID, property, qualifier>
// Reply:	<binary plist containing response property, errorCode>

#define kAirPlayReceiverIPCOpcode_ScreenSetProperty				4003
// Request:	<binary plist containing objectID, property, qualifier, value>
// Reply:	<none>

#define kAirPlayReceiverIPCOpcode_ScreenRegister				4004
// Request:	<w:objectID> 
// Reply:	<none>

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

OSStatus	AirPlayReceiverIPCServerInitialize( void );

#ifdef __cplusplus
}
#endif

#endif	// __AirPlayReceiverIPCCommon_h__
