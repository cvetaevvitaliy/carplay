/*
 *
 * =====================================================================================
 *
 *       Filename:  carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/14/2015 08:52:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

//#include <AirPlayMain.h>
#include <linux/input.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mh_core.h>
#include <mh_misc.h>
#include "dev_carplay.h"
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <CoreUtils/LogUtils.h>
#include <CoreUtils/StringUtils.h>
#include <AirPlayReceiverServer.h>
#include <AirPlayReceiverSession.h>
#include <AirPlayVersion.h>
#include <AirPlayUtils.h>
#include <CoreUtils/DebugServices.h>
#include <CoreUtils/HIDUtils.h>
#include <CoreUtils/ScreenUtils.h>
#include <CoreUtils/MathUtils.h>
#include <CarPlayControlClient.h>
#include <CoreUtils/CFCompat.h>
#include <CoreUtils/CFUtils.h>
#include <CoreUtils/CommonServices.h>
#include <assert.h>
#include <AirPlayReceiverServerPriv.h>
#include <CoreUtils/BonjourBrowser.h>

#include <linux/rtnetlink.h>

static pthread_t gAirPlayThread = 0;
static AirPlayReceiverServerRef	gAirPlayServer	= NULL;
MHDevCarplay * carplayObject;
static MHDev * iap2;
char address[17];
static uint8_t carplayType	=	1;/* 1、usb 2、wifi */	
static uint8_t wcarplayType	=	0;
static char * wMacAddr = NULL;
static char * cMacAddr = NULL;
static uint8_t connectType = 0;

static CarPlayControlClientRef		gCarPlayControlClient = NULL;
static CFMutableArrayRef			gCarPlayControllers = NULL;
static AirPlayReceiverSessionRef    gAirPlaySession = NULL;

extern GSList * carplayDevlist;
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  UpTicks
 *  Description:
 * =====================================================================================
 */
uint64_t UpTicks( void )
{
	uint64_t            nanos;
	struct timespec     ts;

	ts.tv_sec  = 0;
	ts.tv_nsec = 0;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	nanos = ts.tv_sec;
	nanos *= 1000000000;
	nanos += ts.tv_nsec;
	return( nanos );
}		/*  -----  end of static function UpTicks  ----- */

static CFTypeRef _AirPlayHandleServerCopyProperty(
		AirPlayReceiverServerRef    inServer,
		CFStringRef                 inProperty,
		CFTypeRef                   inQualifier,
		OSStatus *                  outErr,
		void *                      inContext )
{
	OSStatus err = kNoErr;

	CFTypeRef       value = NULL;
	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));

	if( CFEqual( inProperty, CFSTR( kAirPlayKey_Features )))
	{
		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_FirmwareRevision )))
	{
		value	=	CFSTR( "Media-Hub v2.0" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_PlaybackAllowed )))
	{
		value	=	kCFBooleanTrue;

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_BluetoothIDs )))
	{
		char * ids =  mh_misc_get_bluetoothids();

		if( ids != NULL ){
			CFStringRef                  _t_value;
			CFMutableArrayRef			descriptions;
			descriptions = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			require_action( descriptions, exit, err = kNoMemoryErr );

			_t_value = CFStringCreateWithBytes( NULL, (const uint8_t *)ids, (CFIndex) strlen(ids), kCFStringEncodingUTF8, false );

			CFArrayAppendValue( descriptions, _t_value );

			value = descriptions;
			descriptions = NULL;
		}else{
			value	=	CFSTR( "00:00:00:00:00:00" );	
		}

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_HardwareRevision )))
	{
		value	=	CFSTR( "T-Aflus-2wk" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_HIDLanguages )))
	{
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_LimitedUIElements )))
	{
		char str[ PATH_MAX ];
		CFDictionaryGetCString( inServer->config, CFSTR( "limitedUI" ), str, sizeof(str) , NULL );
		g_message("---------- %s ----------\n",str);

		if( *str != '\0' ) 
		{
			CFStringRef                  _t_value;
			CFMutableArrayRef			descriptions;
			descriptions = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );

			require_action( descriptions, exit, err = kNoMemoryErr );

			char * limitedUIName[] = {
				"softKeyboard",
				"softPhoneKeypad",
				"nonMusicLists",
				"musicLists",
				"japanMaps"
			};
			g_message(" %s %s %s %s %s",limitedUIName[0], limitedUIName[1],limitedUIName[2],limitedUIName[3],limitedUIName[4]);

			int _count = 0;
			for( _count = 0; _count < 5; _count++ )
			{
				if( strstr( str, limitedUIName[_count])!= NULL )	
				{
					_t_value = CFStringCreateWithBytes( NULL, limitedUIName[_count], (CFIndex) strlen(limitedUIName[_count]), kCFStringEncodingUTF8, false );
					CFArrayAppendValue( descriptions, _t_value );
				}
			}

			value = descriptions;
			descriptions = NULL;
		}else{
			g_message("No LimitedUIElements\n");
		}
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_LimitedUI )))
	{
		* outErr	=	kNotHandledErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_Manufacturer )))
	{
		value	=	CFSTR( "Neusoft" );

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_NightMode )))
	{
		value	=	kCFBooleanTrue;

		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_OEMIcons )))
	{
		CFTypeRef obj;
		CFMutableDictionaryRef iconDict;
		CFMutableArrayRef iconsArray;

		iconsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action_quiet( iconsArray, exit, err = kNotHandledErr );

		char str[ PATH_MAX ];
		obj = NULL;
		int iconCnt = 0;
		for( iconCnt = 0; iconCnt < 3; iconCnt++ )
		{

			memset( str, 0, sizeof( str ));

			if( iconCnt == 0 ){
				CFDictionaryGetCString( inServer->config, CFSTR( "oemIcon0Path" ), str, sizeof( str ), NULL );
			}else if( iconCnt == 1 ){
				CFDictionaryGetCString( inServer->config, CFSTR( "oemIcon1Path" ), str, sizeof( str ), NULL );
			}else if( iconCnt == 2 ){
				CFDictionaryGetCString( inServer->config, CFSTR( "oemIcon2Path" ), str, sizeof( str ), NULL );
			}
			 printf( "_AirPlay_CopyPropertyFunc kAirPlayProperty_OEMIconPath[%s]\n",str);
			if( *str != '\0' ) obj = CFDataCreateWithFilePath( str, NULL );

			if( obj ) {
				iconDict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
				if( iconDict ) {
					int imagesize;

					memset(str,0,sizeof(str));
					imagesize = 0;

					if(iconCnt == 0){
						CFDictionaryGetCString( inServer->config, CFSTR( "width0Pixels" ), str, sizeof( str ), NULL );
					}else if(iconCnt == 1){
						CFDictionaryGetCString( inServer->config, CFSTR( "width1Pixels" ), str, sizeof( str ), NULL );
					}else if(iconCnt == 2){
						CFDictionaryGetCString( inServer->config, CFSTR( "width2Pixels" ), str, sizeof( str ), NULL );
					}

					imagesize = atoi(str);
					  printf( "_AirPlay_CopyPropertyFunc kAirPlayOEMIconKey_WidthPixels[%s][%d]\n",str,imagesize);

					if(imagesize == 0)imagesize = 104;
					CFDictionarySetInt64( iconDict, CFSTR( kAirPlayKey_WidthPixels ), imagesize );

					memset(str,0,sizeof(str));
					imagesize = 0;

					if(iconCnt == 0){
						CFDictionaryGetCString( inServer->config, CFSTR( "height0Pixels" ), str, sizeof( str ), NULL );
					}else if(iconCnt == 1){
						CFDictionaryGetCString( inServer->config, CFSTR( "height1Pixels" ), str, sizeof( str ), NULL );
					}else if(iconCnt == 2){
						CFDictionaryGetCString( inServer->config, CFSTR( "height2Pixels" ), str, sizeof( str ), NULL );
					}

					imagesize = atoi(str);
					 printf( "_AirPlay_CopyPropertyFunc kAirPlayOEMIconKey_HeightPixels[%s][%d]\n",str,imagesize);
					if(imagesize == 0)imagesize = 104;
					CFDictionarySetInt64( iconDict, CFSTR( kAirPlayKey_HeightPixels ), imagesize );

					CFDictionarySetBoolean( iconDict, CFSTR( kAirPlayOEMIconKey_Prerendered ), true );
					CFDictionarySetValue( iconDict, CFSTR( kAirPlayOEMIconKey_ImageData ), obj );
					CFArrayAppendValue( iconsArray, iconDict );
					CFRelease( iconDict );
					iconDict = NULL;
				}
				CFRelease( obj );
			}
		}
		if( CFArrayGetCount( iconsArray ) > 0) {
			value = iconsArray;
		} else {
			CFRelease( iconsArray );
			err = kNotHandledErr;
			goto exit;
		}
//		value	=	kCFBooleanTrue;
//
//		* outErr	=	kNoErr;
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_RightHandDrive )))
	{
		if( mh_misc_get_righthand() == 1 ) 
		{
			value =  kCFBooleanTrue;
		}
		else
		{
			value =  kCFBooleanFalse;
		}

		* outErr	=	kNoErr;
	}
	else
	{
		* outErr	=	kNotHandledErr;
	}
exit:
	if( outErr ) *outErr = err;
	return( value );
}
//===========================================================================================================================
//  _AirPlayHandleSessionFinalized
//===========================================================================================================================

static void _AirPlayHandleSessionFinalized( AirPlayReceiverSessionRef inSession, void *inContext )
{
	printf( "\033[1;32;40mAirPlay session ended\033[0m\n" );
	carplayObject->inSession	=	NULL;

	HIDRemoveFileConfig();

	if( carplayType	==	2)
	{
		MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
		_devParam->type				=	MH_DEV_WIFI_IAP;
		_devParam->mac_addr			=	g_strdup( address );
		_devParam->connect_status	=	2;	//disconnect

		mh_core_find_dev( _devParam );

		g_free( _devParam->mac_addr );
		g_free( _devParam );
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  wifi_write_iap2
 *  Description:  
 * =====================================================================================
 */
gint wifi_write_iap2( MHDev * dev, guint8 * buf, gint len )
{
	CFDataRef    iAPCmdData;
	iAPCmdData = CFDataCreate( NULL, buf, (CFIndex) len );

	if( carplayObject->inSession !=NULL )
		AirPlayReceiverSessionSendiAPMessage(carplayObject->inSession,iAPCmdData, NULL ,0);

	return len;
}		/* -----  end of function wifi_write_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _type_compare
 *  Description:  
 * =====================================================================================
 */
static gint _type_compare( gconstpointer a, gconstpointer b )
{
	g_message("%s type = %s\n",__func__,MH_DEV(a)->type);
	return g_strcmp0( MH_DEV(a)->type, b );
}		/* -----  end of static function _type_compare  ----- */

//===========================================================================================================================
//	_AirPlayHandleSessionStarted
//===========================================================================================================================

static void _AirPlayHandleSessionStarted( AirPlayReceiverSessionRef inSession, void *inContext )
{
	OSStatus error;
	CFNumberRef value;

	(void) inContext;

	value = (CFNumberRef) AirPlayReceiverSessionCopyProperty( inSession, 0, CFSTR( kAirPlayProperty_TransportType ), NULL, &error );
	if( error == kNoErr && value ) {
		uint32_t transportType;

		CFNumberGetValue( (CFNumberRef) value, kCFNumberSInt32Type, &transportType ); 
		if( NetTransportTypeIsWiFi( transportType ) ) {
			// Start iAP
			iap2 = NULL;

			CFStringRef	str;
			CFRange	range;
			CFIndex	size;

			str = (CFStringRef) AirPlayReceiverSessionCopyProperty( inSession, 0, CFSTR( kAirPlayProperty_DeviceID ), NULL, &error );	

			if( error == kNoErr && str ) {
				uint8_t mac[6];

				range = CFRangeMake( 0, CFStringGetLength( str ) );
				
				CFStringGetBytes( str, range, kCFStringEncodingUTF8, 0, false, mac, 6, &size );

				sprintf( address, "%02x:%02x:%02x:%02x:%02x:%02x",
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				g_message("wifi address = %s \n",address );
			}

			carplayObject->write		=	wifi_write_iap2;

			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_IAP;
			_devParam->mac_addr			=	g_strdup( address );
			_devParam->connect_status	=	1;	

			mh_core_find_dev( _devParam );
			
			g_free( _devParam->mac_addr );
			g_free( _devParam );
		}
	}
}

static CFTypeRef _AirPlayHandleSessionCopyProperty(
						AirPlayReceiverSessionRef   inSession,
						CFStringRef                 inProperty,
						CFTypeRef                   inQualifier,
						OSStatus *                  outErr,
						void *                      inContext )
{
	CFTypeRef       value = NULL;
	OSStatus        err;
	Boolean	_done;

	if( CFEqual( inProperty, CFSTR( kAirPlayProperty_Modes )))
	{
		printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inProperty, kCFStringEncodingUTF8 ));
		value	=	carplayObject->modes;
		require_action_quiet( value, exit, err = kNotHandledErr );
		CFRetain( value );
	}
	else
	{
		assert( 0 );
	}

exit:
	if( outErr ) *outErr = err;
	return( value );
}
static void _AirPlayHandleModesChanged(
		AirPlayReceiverSessionRef   inSession,
		const AirPlayModeState *    inState,
		void *                      inContext )
{
//	printf( "\033[1;32;40mModes changed: screen [ %s ], mainAudio [ %s ], speech [ %s %s ], phone [ %s ], turns [ %s ]\033[0m\n",
//			AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ),
//			AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ),
//			AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ) );

	g_message( "Modes changed: screen [ %s ], mainAudio [ %s ], speech [ %s %s ], phone [ %s ], turns [ %s ], CurrentTime[%lld]\n",
			AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ),
			AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ),
			AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ),
			UpTicks());

	g_signal_emit_by_name( carplayObject, "modes_changed", AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ),
			AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ),
			AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ) );
}

//===========================================================================================================================
//  _AirPlayHandleRequestUI
//===========================================================================================================================

static void _AirPlayHandleRequestUI( AirPlayReceiverSessionRef inSession , CFStringRef inURL, void *inContext )
{
	printf( "\033[1;32;40mRequest accessory UI\033[0m\n" );
	const char *str;
	size_t len;

	(void) inSession;
	(void) inContext;

	if( inURL ) {
		CFLStringGetCStringPtr( inURL, &str, &len);
	} else {
		str = NULL;
	}

	g_signal_emit_by_name( carplayObject, "request_ui", NULL );
}

static OSStatus _AirPlayHandleSessionControl(
						AirPlayReceiverSessionRef   inSession,
						CFStringRef                 inCommand,
						CFTypeRef                   inQualifier,
						CFDictionaryRef             inParams,
						CFDictionaryRef *           outParams,
						void *                      inContext )
{
	OSStatus err;

	const char *devid = 0;
	CFStringRef    deviceID;

	printf( "\033[1;32;40m%s %s\033[0m\n", __func__, CFStringGetCStringPtr( inCommand, kCFStringEncodingUTF8 ));
	
	if( CFEqual( inCommand, CFSTR( kAirPlayCommand_DisableBluetooth )))
	{
		deviceID = CFDictionaryGetCFString( inParams,CFSTR("deviceID"),&err );
		devid = CFStringGetCStringPtr(deviceID,kCFStringEncodingUTF8);
		printf(  "disableBluetooth devid [%s]\n",devid );

		g_signal_emit_by_name( carplayObject, "disable_bluetooth", devid, NULL );

		err = kNoErr;
	}
	else if( CFEqual( inCommand, CFSTR( kAirPlayCommand_iAPSendMessage ) ) ) 
	{
		uint8_t *AirPlayiAPData;
		size_t AirPlayiAPDataLen;

		AirPlayiAPData = CFDictionaryGetData( inParams, CFSTR( kAirPlayKey_Data ), NULL, 0, &AirPlayiAPDataLen, &err );

		if( !err )
		{
			if( iap2 == NULL )	iap2	=	MH_DEV( mh_core_find_dev_custom( "iap2_wifi", _type_compare ));
			mh_dev_write_bt_data(iap2, AirPlayiAPData, (int32_t)AirPlayiAPDataLen );
		}
	}
	else
	{
		assert( 0 );
	}

	return( err );
}

void _AirPlayHandleDuckAudio(
		AirPlayReceiverSessionRef     inSession,
		double                        inDurationSecs,
		double                        inVolume,
		void *                        inContext )
{
	(void)inSession;
	(void)inDurationSecs;
	(void)inVolume;
	(void)inContext;

	printf( "\033[1;32;40m%s [%f] [%f]\033[0m\n", __func__, inDurationSecs, inVolume);

	g_signal_emit_by_name( carplayObject, "duck_audio", inDurationSecs, inVolume, NULL );

}

void _AirPlayHandleUnDuckAudio(
		AirPlayReceiverSessionRef     inSession,
		double                        inDurationSecs,
		void *                        inContext )
{
	(void)inSession;
	(void)inDurationSecs;
	(void)inContext;

	printf( "\033[1;32;40m%s [%f] \033[0m\n", __func__, inDurationSecs);

	g_signal_emit_by_name( carplayObject, "unduck_audio", inDurationSecs, NULL );

}

static void _AirPlayHandleSessionCreated(AirPlayReceiverServerRef inServer,
									    AirPlayReceiverSessionRef inSession,
										void * inContext )
{
	AirPlayReceiverSessionDelegate      delegate;

	(void) inServer;
	(void) inContext;

	printf( "\033[1;32;40mAirPlay session created\033[0m\n" );

	// Register ourself as a delegate to receive session-level events, such as modes changes.
	gAirPlaySession = inSession;
	AirPlayReceiverSessionDelegateInit( &delegate );

//	delegate.initialize_f	=	_AirPlayHandleSessionInitialized;
	delegate.finalize_f     =	_AirPlayHandleSessionFinalized;
	delegate.started_f		= 	_AirPlayHandleSessionStarted;
	delegate.copyProperty_f =	_AirPlayHandleSessionCopyProperty;
	delegate.modesChanged_f =	_AirPlayHandleModesChanged;
	delegate.requestUI_f	= 	_AirPlayHandleRequestUI;
	delegate.control_f      =	_AirPlayHandleSessionControl;
	delegate.duckAudio_f	=	_AirPlayHandleDuckAudio;	
	delegate.unduckAudio_f	=	_AirPlayHandleUnDuckAudio;	

	carplayObject	=	g_object_new( MH_TYPE_DEV_CARPLAY, "io-name", "carplay", NULL );

	carplayObject->inSession	=	inSession;
	carplayObject->inContext	=	inContext;
	carplayObject->inServer		=	inServer;
	MHDev 	  * _dev	=	MH_DEV( carplayObject );
	if( carplayType	==	2)
	{
	//	MHDev 	  * _dev	=	MH_DEV( carplayObject );
		if( _dev->type != NULL )
		{
			g_free( _dev->type );	
		}
		_dev->type = g_strdup( "carplay_wifi" );
		g_message("-----------------------wifi   carplay -------------------\n");
	}
	g_message("\n\n\n\n-------------_dev->type = %s\n\n\n\n", _dev->type);
	mh_core_attach_dev( MH_DEV( carplayObject ));

	AirPlayReceiverSessionSetDelegate( inSession, &delegate );
}
//===========================================================================================================================
//	CarPlayControlClient
//===========================================================================================================================
void CarPlayControlClientEventCallback( CarPlayControlClientRef client, CarPlayControlClientEvent event, void *eventInfo, void *context )
{
    (void) client;
	(void) context;
	CarPlayControllerRef controller = (CarPlayControllerRef)eventInfo;
	OSStatus err;

	if (event == kCarPlayControlClientEvent_AddOrUpdateController) {
		const char *cStr = NULL;
		char *storage = NULL;
		CFStringRef name = NULL;
		CFIndex count;
		uint8_t outAddress[ 6 ];
		CFStringRef DNSName = NULL;
		const char * DNSStr = NULL;
		char * DNSstorage = NULL;
		uint8_t _address[ 17 ];
		uint32_t _type = 0;

		CarPlayControllerCopyName( controller, &name);
		CFStringGetOrCopyCStringUTF8(name, &cStr, &storage, NULL);

		// Add the new client to the array 
		CFArrayAppendValue( gCarPlayControllers, controller );
		count = CFArrayGetCount( gCarPlayControllers );
		g_message("Add __________%s___________name = %s cStr = %s storage = %s count = %d\n",__func__,name,cStr,storage,count);

		CarPlayControllerGetBluetoothMacAddress( controller,outAddress );

		CarPlayControllerCopyDNSName( controller, &DNSName);
		CFStringGetOrCopyCStringUTF8( DNSName, &DNSStr, &DNSstorage, NULL);

		char * _tmp;
		uint32_t _ifindex;
		_tmp = g_strrstr( DNSStr, "%");
		if( _tmp != NULL )	
			_ifindex = atoi( _tmp + 1 );

		printf( "\033[1;32;40m Event callback mac :%02x:%02x:%02x:%02x:%02x:%02x  ifname = %s _p = %s %d\033[0m\n",
				outAddress[0],outAddress[1],outAddress[2],outAddress[3],outAddress[4],outAddress[5],DNSStr,_tmp+1,_ifindex);

		GSList *iterator = NULL;
		for (iterator = carplayDevlist; iterator; iterator = iterator->next) 
		{
			g_message( " find  ifindex = %d  name : %s(%s), \n",((carplayInfo *)iterator->data)->ifindex, ((carplayInfo *)iterator->data)->name, cStr);
			if ((_ifindex == ((carplayInfo *)iterator->data)->ifindex)&&( g_strcmp0(cStr,   ((carplayInfo *)iterator->data)->name   ) == 0))
	//		if ( _ifindex == ((carplayInfo *)iterator->data)->ifindex )
			{
				if( ((carplayInfo *)iterator->data)->mac_addr != NULL )
					g_free(((carplayInfo *)iterator->data)->mac_addr );
				
				sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",	
					outAddress[0], outAddress[1], outAddress[2], outAddress[3], outAddress[4], outAddress[5]);
				((carplayInfo *)iterator->data)->mac_addr = g_strdup( _address );

				_type	=	((carplayInfo *)iterator->data)->transportType;	

				g_message("list name = %s mac_addr = %s ifname = %s ifindex = %d transportType = %d\n",
				((carplayInfo *)iterator->data)->name, 	((carplayInfo *)iterator->data)->mac_addr,
				((carplayInfo *)iterator->data)->ifname, ((carplayInfo *)iterator->data)->ifindex, ((carplayInfo *)iterator->data)->transportType);				
				break;
			}
		}
		g_message("type = %d connectType = %d len = %d(%s)\n",_type,connectType,strlen( wMacAddr ), wMacAddr);
		if(( _type == 1 ) && ( (connectType == 3) || (connectType == 1) ) && (strlen( wMacAddr ) > 17 ))
		{
			int i;
			g_message(" current ready controller number is one\n");
			for( i = 0; i < 5; i++ )
			{
				err = CarPlayControlClientConnect(gCarPlayControlClient, controller);
				if( err != kNoErr )
				{
					sleep( 1 );
					continue;
				} 
				else 
				{
					if( cMacAddr != NULL )	
						g_free( cMacAddr);
					cMacAddr		=	g_strdup( _address );

					if( wMacAddr != NULL )	
						g_free( wMacAddr);
					wMacAddr		=	g_strdup( _address );

					carplayType	=	_type;
					break;
				}
			}
		}
		else
		{
//			if(( strcmp( wMacAddr, _address ) == 0 )
//				|| ( _type == wcarplayType ))
			if( strcmp( wMacAddr, _address ) == 0 )
			{
				int i;

				for( i = 0; i < 5; i++ )
				{
					err = CarPlayControlClientConnect(gCarPlayControlClient, controller);
					if( err != kNoErr )
					{
						sleep( 1 );
						continue;
					}
					else
					{
						if( cMacAddr != NULL )	
							g_free( cMacAddr);
						cMacAddr		=	g_strdup( _address );
						carplayType	=	_type;
					break;
					}
				}
			}
			else
			{
				g_message(" Not a designated connection device \n");
			}
		}
		free( storage );
		free( DNSstorage );
		CFRelease( name );
		CFRelease( DNSName );

	} else if (event == kCarPlayControlClientEvent_RemoveController) {
		CFIndex ndx, count;
		CFStringRef name = NULL;
		const char *cStr = NULL;
		char *storage = NULL;


		CarPlayControllerCopyName( controller, &name);
		CFStringGetOrCopyCStringUTF8(name, &cStr, &storage, NULL);
		
		count = CFArrayGetCount( gCarPlayControllers );
		ndx = CFArrayGetFirstIndexOfValue( gCarPlayControllers, CFRangeMake(0, count), controller );
		
		CFArrayRemoveValueAtIndex( gCarPlayControllers, ndx );
		free( storage );
		CFRelease( name );
	} else {
	}
}

int GetNetStatus()
{
	char    buffer[BUFSIZ];  
	FILE    *read_fp;  
	int        chars_read;  
	int        ret;  

	memset( buffer, 0, BUFSIZ );  
//	read_fp = popen("ifconfig usb0 | grep RUNNING", "r");  
	read_fp = popen("ifconfig usbncm0", "r");  
	if ( read_fp != NULL )  
	{  
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);  
		if (chars_read > 0)  
		{  
			ret = 1;  
		}  
		else  
		{  
			ret = 0;  
		}  
		pclose(read_fp);  
	}  
	else  
	{  
		ret = 0;  
	}  
	return ret;  
}

int NicSleep(int USECS)
{
	for(int i = 0; i < USECS ; i++ )
	{
		usleep(50000);
		if (GetNetStatus() == 0) return -1;
	}
	return 0;
}

int WaitSetInterface()
{
	int flags, myflags, index, plen, scope;
	char addr[8][5];
	char ifname[9];
	FILE *fp = NULL;
//	if (GetNetStatus() == 0) return -1;

	g_message("%s\n",__func__);

	int i = 0;
	while(1)
	{
		if ((fp = fopen("/proc/net/if_inet6", "r")) != NULL && i < 100){
			while (fscanf(fp,
						"%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %8s\n",
						addr[0],addr[1],addr[2],addr[3],
						addr[4],addr[5],addr[6],addr[7],
						&index, &plen, &scope, &flags, ifname) != EOF)
			{
				g_message( "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %8s %d\n",
						addr[0],addr[1],addr[2],addr[3],
						addr[4],addr[5],addr[6],addr[7],
						index, plen, scope, flags, ifname, i );

				if (strcmp(ifname, "usbncm0") == 0)
				{
					if (fp != NULL) {
						int fd_closed = fclose(fp);
						fp=NULL;
						assert(fd_closed == 0);
					}
					if(i == 0)
					{
						return 1;
					}
					else
					{
						if (NicSleep(2) < 0) return -1;
						else return 0;// 100ms
					}
				}
			}
			if (fp != NULL) {
				int fd_closed = fclose(fp);
				fp=NULL;
				assert(fd_closed == 0);
			}
			i++;
			if (NicSleep(4) < 0) return -1;
			else continue;//100ms
		}
		else
		{
			g_message("fopen /proc/net/if_inet6 errno %d i : %d",errno,i);
			if (fp != NULL) {
				int fd_closed = fclose(fp);
				fp=NULL;
				assert(fd_closed == 0);
			}
			return -1;
		}
	}
}

//}
//===========================================================================================================================
//	_AirPlayThread
//===========================================================================================================================
static void *_AirPlayThread( void *inArg )
{
	OSStatus						err;
	AirPlayReceiverServerDelegate		delegate;
	
	(void) inArg;

	// Create the AirPlay server. This advertise via Bonjour and starts listening for connections.
	g_message( "--------------- %s --------------Start\n",__func__);	
#ifdef B511
	system("/etc/rc.d/mdns start"); 
#else
	system("systemctl start mdns");
#endif

	gAirPlayServer	= NULL;
	gCarPlayControlClient = NULL;

	err	=	AirPlayReceiverServerCreate( &gAirPlayServer );
	if (0 != err) {
		g_message("AirPlayReceiverServerCreate failed, errno = %d", err);
	}

	require_noerr( err, exit );

	AirPlayReceiverServerDelegateInit( &delegate );

	delegate.copyProperty_f		= _AirPlayHandleServerCopyProperty;
	delegate.sessionCreated_f = _AirPlayHandleSessionCreated;

	AirPlayReceiverServerSetDelegate( gAirPlayServer, &delegate );
	
	err =	CarPlayControlClientCreateWithServer( &gCarPlayControlClient, gAirPlayServer, CarPlayControlClientEventCallback, NULL );

	if (0 != err) {
		g_message("CarPlayControlClientCreateWithServer failed, errno = %d", err);
	}

	gCarPlayControllers = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	
	// Start the server and run until the app quits.
	err	=	AirPlayReceiverServerStart( gAirPlayServer );

	if (0 != err) {
		g_message("AirPlayReceiverServerStart failed, errno = %d", err);
	}

	require_noerr( err, exit );
#if 1
	int ret = WaitSetInterface();
	g_message("########################WaitSetInterface ret = %d", ret);
	if(ret >= 0)
	{
		if(gCarPlayControlClient != NULL){
			err	=	CarPlayControlClientStart( gCarPlayControlClient );
			if (0 != err) {
				g_message("CarPlay CarPlayControlClientStart failed, errno = %d", err);
			}
		}
		else{
			g_message("CarPlay No support CarPlayControlClient");
		}
	}
#else
	if(gCarPlayControlClient != NULL){
		CarPlayControlClientStart( gCarPlayControlClient );	
	}else{
		g_message("CarPlay No support CarPlayControlClient");
	}
#endif

	CFRunLoopRun();
	if(gCarPlayControlClient != NULL) CarPlayControlClientStop( gCarPlayControlClient );
	if(gAirPlayServer != NULL)	AirPlayReceiverServerStop( gAirPlayServer );
	
exit:
/* lsk start */
	CFReleaseNullSafe( gCarPlayControllers );
	CFReleaseNullSafe( gCarPlayControlClient );
/* lsk end */
	CFReleaseNullSafe( gAirPlayServer );
	gAirPlayServer = NULL;
	gAirPlaySession = NULL;
	gCarPlayControlClient = NULL;
	gCarPlayControllers = NULL;

#ifdef B511
	system("/etc/rc.d/mdns stop"); 
#else
	system("systemctl stop mdns");
#endif
	return NULL;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _unInit
 *  Description:  
 * =====================================================================================
 */
static gboolean _unInit( gpointer user_data )
{
	MHDevParam * _devParam = ( MHDevParam *)user_data;

	MHDevParam * _param 	=	(MHDevParam*)g_new0( MHDevParam, 1);
	_param->type			=	_devParam->type;
	_param->mac_addr		=	g_strdup( _devParam->mac_addr );
	_param->connect_status	=	_devParam->connect_status;		

	g_message(" [%s] type = [%d]  mac_addr = [%s] connect = [%d] wMacAddr = [%s]\n", 
			__func__, _param->type, _param->mac_addr, _param->connect_status, wMacAddr );	

	if( _param->type != MH_DEV_USB_CARPLAY) 
	{
		if( _param->connect_status == 2 )
		{
			g_free( _param->mac_addr );

			if( cMacAddr != NULL )
				_param->mac_addr	=	g_strdup( cMacAddr );
		}

		if(( wMacAddr != NULL ) && strcmp(_param->mac_addr, wMacAddr ) != 0) 
		{
			printf( "\033[1;32;40m  _unInit error  \033[0m\n" );
			goto RETURN;
		}
	}
//	else{
//		if(( wMacAddr != NULL ) && strcmp(_param->mac_addr, wMacAddr ) != 0) 
//		{
//			printf( "\033[1;32;40m  _unInit error  \033[0m\n" );
//			goto RETURN;
//		}
//	}

	if(( gAirPlayServer != NULL ) && ( gAirPlayThread != 0 ))
	{

		AirPlayReceiverServerControl( gAirPlayServer, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SessionDied ), NULL, NULL, NULL );

		if( carplayObject != NULL )
		{
			g_message(" %s   unref carplay\n",__func__);
			mh_core_detach_dev( MH_DEV( carplayObject ) );

			g_object_unref( carplayObject );

			if( cMacAddr != NULL )
			{
				g_free( cMacAddr );
				cMacAddr	=	NULL;
			}
			usleep(20000);
		}else{
			g_message( "carplayObject is NULL\n" );
		}

		g_message("CarPlay iPod_reset_carplay");
//		AirPlayReceiverServerControl( gAirPlayServer, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SessionDied ), NULL, NULL, NULL );
		g_message("CarPlay iPod_reset_carplay end\n");
//		usleep(20000);
	}else{
		g_message("_unInit    gAirPlayServer = %p gAirPlayThread = %d\n",gAirPlayServer,gAirPlayThread);	
	}

	if( gAirPlayThread != 0 ){

		g_message("CarPlay iPod_stop_carplay_server");
		CFRunLoopStop( CFRunLoopGetMain() );
		
		gAirPlayThread = 0;

//		if( carplayObject != NULL )
//		{
//			g_message(" %s   unref carplay\n",__func__);
//			mh_core_detach_dev( MH_DEV( carplayObject ) );
//
//			g_object_unref( carplayObject );
//
//			if( cMacAddr != NULL )
//			{
//				g_free( cMacAddr );
//				cMacAddr	=	NULL;
//			}
//		}else{
//			g_message( "carplayObject is NULL\n" );
//		}
	}
	else{
		g_message( "CarPlay No AirPlayThread" );
	}

	g_free( _param->mac_addr );
	g_free( _param );
	usleep(100000);

RETURN:
	return G_SOURCE_REMOVE;
}		/* -----  end of static function _unInit  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _init
 *  Description:  
 * =====================================================================================
 */
static gboolean create_airplay_thread()
{
	GSList *iterator = NULL;
	for (iterator = carplayDevlist; iterator; iterator = iterator->next) 
	{
		printf( "\033[1;32;40m create_airplay_thread remove device info : name = %s mac_addr = %s ifname = %s\033[0m\n",
				((carplayInfo *)iterator->data)->name,((carplayInfo *)iterator->data)->mac_addr,((carplayInfo *)iterator->data)->ifname);

		if( ((carplayInfo *)iterator->data)->name != NULL )
			g_free(((carplayInfo *)iterator->data)->name );
		if( ((carplayInfo *)iterator->data)->mac_addr != NULL )
			g_free(((carplayInfo *)iterator->data)->mac_addr );
		if( ((carplayInfo *)iterator->data)->ifname != NULL )
			g_free(((carplayInfo *)iterator->data)->ifname );

		carplayDevlist = g_slist_remove( carplayDevlist, iterator->data );
	}

	carplayDevlist = NULL;

	int ret = pthread_create( &gAirPlayThread, NULL, _AirPlayThread, NULL );

	if (ret == 0) {
		g_message("CarPlay AirPlayThread created ok");

		pthread_detach( gAirPlayThread );
	} else {
		g_message("CarPlay AirPlayThread failed to be created, errno = %d", ret);
	}

	return TRUE;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _init
 *  Description:  
 * =====================================================================================
 */
static gboolean _init( gpointer user_data )
{
//	pthread_create( &gAirPlayThread, NULL, _AirPlayThread, NULL );
	MHDevParam * _devParam = ( MHDevParam *)user_data;

	MHDevParam * _param 	=	(MHDevParam*)g_new0( MHDevParam, 1);
	_param->type			=	_devParam->type;
	_param->mac_addr		=	g_strdup( _devParam->mac_addr );
	_param->connect_status	=	_devParam->connect_status;		

	connectType	=	_devParam->connect_status;

	g_message(" [%s] type = [%d]  mac_addr = [%s] connect = [%d]\n", 
			__func__, _param->type, _param->mac_addr, _param->connect_status );	
	if( _param->connect_status	== 3 )
	{
		if( gAirPlayThread == 0){
			g_message("start CarPlay server( _AirPlayThread )");
			
			if( wMacAddr != NULL )
				g_free( wMacAddr );
			wMacAddr	 =	g_strdup( _devParam->mac_addr );

			create_airplay_thread();
		}
		else {
			g_message("CarPlay server already start");
		}
	}else{
		if( wMacAddr != NULL )
			g_free( wMacAddr );
		wMacAddr	 =	g_strdup( _devParam->mac_addr );
		if( _param->type == 4 ) 
		{
			wcarplayType = 	1;
		}else
		{
			wcarplayType = 	2;
		}	
		if(( cMacAddr == NULL )||
			strcmp( wMacAddr, cMacAddr ) != 0 )
		{
			g_message("-----------------------   gAirPlayThread = %d\n",gAirPlayThread);
			if( gAirPlayThread != 0 )	
				_unInit( user_data );

			create_airplay_thread();
		}else{
			g_message( "The mac address of the connected device is the same as the current\n");
		}
	}

	g_free( _param->mac_addr );
	g_free( _param );
RETURN:
	return G_SOURCE_REMOVE;
}		/* -----  end of static function _init  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_instance
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_instance()
{
//	GSource * _source	=	g_idle_source_new();
//
//	g_source_set_callback( _source, _init, NULL, NULL );
//
//	mh_io_dispatch( MH_IO( mh_core_instance() ), _source );
//
//	g_source_unref( _source );

	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_find_dev
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_find_dev( MHDevParam * param )
{
	g_message("carplay %s type = [%d]  mac_addr = [%s] connect = [%d]\n", 
			__func__, param->type, param->mac_addr, param->connect_status );	

	MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
	_devParam->type				=	param->type;
	_devParam->mac_addr			=	g_strdup( param->mac_addr );
	_devParam->connect_status	=	param->connect_status;	

	if(( param->connect_status == 1 )
		|| ( param->connect_status == 3 ))
	{
		GSource * _source	=	g_idle_source_new();

		g_source_set_callback( _source, _init, _devParam, g_free );

		mh_io_dispatch( MH_IO( mh_core_instance() ), _source );

		g_source_unref( _source );
	}
	else
	if(( param->connect_status == 0 )
		|| ( param->connect_status == 2 ))
	{
		GSource * _source	=	g_idle_source_new();

		g_source_set_callback( _source, _unInit, _devParam, g_free );

		mh_io_dispatch( MH_IO( mh_core_instance() ), _source );

		g_source_unref( _source );
	}else{
		g_message( "%s connect num is error!\n",__func__);	
		
		g_free( _devParam->mac_addr );
		g_free( _devParam );
	}

	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */
