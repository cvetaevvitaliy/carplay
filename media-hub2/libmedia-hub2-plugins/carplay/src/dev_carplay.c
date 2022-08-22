/*
 * Generated by object-codegen.
 */
#include <glib.h>
#include <stdio.h>
#include "dev_carplay.h"
#include <mh_core.h>
#include <mh_player.h>
#include <mh_carplay.h>
#include <mh_misc.h>
#include <string.h>
#include <gst/gst.h>

#include <AirPlayUtils.h>
#include <CoreUtils/HIDUtils.h>

typedef struct _MHDevCarplayPrivate MHDevCarplayPrivate;

struct _MHDevCarplayPrivate
{
	gint dummy;
};

G_DEFINE_TYPE_WITH_PRIVATE( MHDevCarplay, mh_dev_carplay, MH_TYPE_DEV )

enum {
	/* Ios */

	/* Signals */
	DUCK_AUDIO,
	UNDUCK_AUDIO,
	DISALBE_BLUETOOTH,
	MODES_CHANGED,
	HID_SET_INPUT_MODE,
	REQUEST_UI,
	AUDIO_INFO,

	N_SIGNALS
};

static guint signals[ N_SIGNALS ] = {0};

enum
{
	PROP_0,

	PROP_DEV_CARPLAY_CHANGEMODES,
	PROP_DEV_CARPLAY_KEYFRAME,
	PROP_DEV_CARPLAY_REQUEST_SIRI_ACTION,
	PROP_DEV_CARPLAY_REQUEST_UI,
	PROP_DEV_CARPLAY_NIGHTMODE,
	PROP_DEV_CARPLAY_SETLIMITUI,
	PROP_DEV_CARPLAY_VIDEO_SINK,
	PROP_DEV_CARPLAY_SIGNAL_TOUCH,
	PROP_DEV_CARPLAY_MEDIA_BUTTON,
	PROP_DEV_CARPLAY_PHONE_BUTTON,

	N_PROPERTIES
};

int airPlayTransferType[] = {
	kAirPlayTransferType_NotApplicable,	
	kAirPlayTransferType_Take,
	kAirPlayTransferType_Untake,
	kAirPlayTransferType_Borrow,
	kAirPlayTransferType_Unborrow
};

int airPlayTransferPriority[] = {
	kAirPlayTransferPriority_NotApplicable,	
	kAirPlayTransferPriority_NiceToHave,
	kAirPlayTransferPriority_UserInitiated
};

int airPlayConstraint[] = {
	kAirPlayConstraint_NotApplicable,	
	kAirPlayConstraint_Anytime,
	kAirPlayConstraint_UserInitiated,
	kAirPlayConstraint_Never
};

int airPlayTriState[] = {
	kAirPlayTriState_NotApplicable,	
	kAirPlayTriState_False,
	kAirPlayTriState_True
};

int airPlaySpeechMode[] = {
	kAirPlaySpeechMode_NotApplicable,	
	kAirPlaySpeechMode_None,
	kAirPlaySpeechMode_Speaking,
	kAirPlaySpeechMode_Recognizing
};

extern MHDevCarplay * carplayObject;
static GParamSpec *devCarplayProperties[ N_PROPERTIES ] = { NULL, };

#define DEBUG_HEX_DISPLAY(D, L) \
{ \
	char _tmpChar[100] = {0}; \
	int _cc, i; \
\
	printf( "%lld\n",(long long int) (L) );\
	fprintf(stdout, "*******HEX DISPLAY BEGIN (%03lld bytes)******\n",(long long int) (L));\
\
	for(_cc = 0; _cc < (L); _cc += 16) \
	{ \
		memset(_tmpChar, 0, sizeof(_tmpChar)); \
\
		for(i = 0; i < 16 && i + _cc < (L); i ++) \
		{ \
			_tmpChar[i * 3]     =   ((D)[_cc + i] >> 4)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 1] =   ((D)[_cc + i] & 0x0F)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 2] =   ' '; \
		} \
\
		fprintf(stdout, "\t%s\n",  _tmpChar); \
	} \
	fprintf(stdout, "************HEX DISPLAY END*************\n");\
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _get_property
 *  Description:
 * =====================================================================================
 */
static void _get_property( GObject * object, guint property_id, GValue * value,
		        GParamSpec * spec)
{
	MHDevCarplay * _self  =   MH_DEV_CARPLAY( object );
	void * _videoSink;

	switch( property_id )
	{
	case PROP_DEV_CARPLAY_VIDEO_SINK:
		if( _self->videoPipeline != NULL )
			g_object_get( _self->videoPipeline, "video-sink", &_videoSink, NULL );

		if( _videoSink != NULL )
		{
		#ifdef __x86_64__
			g_value_set_uint( value, ( guint64 )_videoSink );
		#else
			g_value_set_uint( value, ( guint )_videoSink );
		#endif
		}
		break;
	default:
		break;
	}
}       /*  -----  end of static function _get_property  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  post_hid_report
 *  Description:  
 * =====================================================================================
 */
static void post_hid_report( const char * report )
{
	/* Report format: x,xxx,xxx */
	static uint8_t _report[9]	=	{ 0 };

	_report[0]	=	report[0] == '0' ? 0 : 1;
	g_message("post_hid_report _report[0]= %d", _report[0]);

	if( _report[0] == 1 )
	{
		int _x, _y;

		_x	=	atoi( report + 2 );
		_y	=	atoi( report + 7 );
		g_message("x= %d, _y = %d", _x, _y);

		_report[ 1 ]	=	(uint8_t)(   _x        & 0xFF );
		_report[ 2 ]	=	(uint8_t)( ( _x >> 8 ) & 0xFF );
		_report[ 3 ]	=	(uint8_t)(   _y        & 0xFF );
		_report[ 4 ]	=	(uint8_t)( ( _y >> 8 ) & 0xFF );
	}

	HIDPostReport( CFSTR( "e5f7a68d-7b0f-4305-984b-974f677a150b" ),
			_report, sizeof( _report ));
}		/* -----  end of static function post_hid_report  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  post_media_hid_report
 *  Description:  
 * =====================================================================================
 */
static void post_media_hid_report( int report )
{
	static uint8_t _report = 0;	

	_report	=	report == 0 ? 0 : 1;

	if( _report != 0 )
	{
		_report	=	(uint8_t)( ( _report << (report - 1 )) & 0xFF );
	}

	HIDPostReport( CFSTR( "e5f7a68d-7b0f-4305-984b-974f677a1501" ),
			&_report, sizeof( _report ));

}		/* -----  end of static function post_hid_report  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  post_hid_phone_report
 *  Description:  
 * =====================================================================================
 */
static void post_hid_phone_report( int report )
{
	uint8_t _report[3]	=	{ 0 };

	g_message("report = %d\n",report);	

	if( report != 0 )
	{
		if( report > 15 )
		{
			_report[ 2 ]	=	(uint8_t)(( 1 << ( report - 16 )) & 0xFF );
		}
		else 
		if (( report <= 15 )&&( 8 <= report))
		{
			_report[ 1 ]	=	(uint8_t)(( 1 << ( report - 8 )) & 0xFF );
		}
		else
		{
			_report[ 0 ]	=	(uint8_t)(( 1 << ( report - 1 )) & 0xFF );
		}
	}

	HIDPostReport( CFSTR( "e5f7a68d-7b0f-4305-984b-974f677a1502" ),
			&_report, sizeof( _report ));

}		/* -----  end of static function post_hid_phone_report  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _set_property
 *  Description:
 * =====================================================================================
 */
static void _set_property( GObject * object, guint property_id, const GValue * value,
		        GParamSpec * spec)
{
	MHDevCarplay * _self  =   MH_DEV_CARPLAY( object );
	AirPlayModeChanges _changes;
	const gchar * _value;

	switch( property_id )
	{
	case PROP_DEV_CARPLAY_CHANGEMODES:
		_value	=	g_value_get_string( value );
		g_message("value = %s len = %d\n", _value, strlen(_value));
		
		if(( strncmp( _value, "res", 3 ) == 0 )&&( strlen( _value ) == 9 ))
		{
			int _param				=	atoi( _value + 4 );
			int _resourceid			=	_param/10000;
			int _transfertype		=	_param%10000/1000;
			int _transferpriority	=	_param%1000/100;
			int _takeconstraint		=	_param%100/10;
			int _borrowconstraint	=	_param%10;

			g_message(" %d %d %d %d %d %d\n", _param, _resourceid,_transfertype, 
					_transferpriority, _takeconstraint, _borrowconstraint );

			if( _resourceid == MH_CARPLAY_RESOURCE_MAIN_SCREEN )
			{
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_TAKE )
				{
					AirPlayReceiverSessionTakeScreen( _self->inSession, airPlayTransferPriority[ _transferpriority ],
							airPlayConstraint[ _takeconstraint ], airPlayConstraint[ _borrowconstraint ], NULL, NULL, _self->inContext );
				}
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_UNTAKE )
				{
					AirPlayReceiverSessionUntakeScreen( _self->inSession, NULL, NULL, _self->inContext );
				}
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_BORROW )
				{
					AirPlayReceiverSessionBorrowScreen( _self->inSession, airPlayTransferPriority[ _transferpriority ],
							airPlayConstraint[ _borrowconstraint ], NULL, NULL, _self->inContext );
				}
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_UNBORROW )
				{
					AirPlayReceiverSessionUnborrowScreen( _self->inSession, NULL, NULL, _self->inContext );
				}
				else
				{
					g_warning( "Unknown change modes command: %s", _value );
				}
			}
			else
			if( _resourceid == MH_CARPLAY_RESOURCE_MAIN_AUDIO )
			{
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_TAKE )
				{
					AirPlayReceiverSessionTakeMainAudio( _self->inSession, airPlayTransferPriority[ _transferpriority ],
							airPlayConstraint[ _takeconstraint ], airPlayConstraint[ _borrowconstraint ], NULL, NULL, _self->inContext );
				}
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_UNTAKE )
				{
					AirPlayReceiverSessionUntakeMainAudio( _self->inSession, NULL, NULL, _self->inContext );
				}
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_BORROW )
				{
					AirPlayReceiverSessionBorrowMainAudio( _self->inSession, airPlayTransferPriority[ _transferpriority ],
							airPlayConstraint[ _borrowconstraint ], NULL, NULL, _self->inContext );
				}	
				else
				if( _transfertype == MH_CARPLAY_TRANSFERTYPE_UNBORROW )
				{
					AirPlayReceiverSessionUnborrowMainAudio( _self->inSession, NULL, NULL, _self->inContext );
				}
				else
				{
					g_warning( "Unknown change modes command: %s", _value );
				}
			}else{
				g_warning( "Unknown change modes command: %s", _value );
			}
		}
		else
		if(( strncmp( _value, "app", 3 ) == 0 )&&( strlen( _value ) == 7 ))
		{
			int _param			=	atoi( _value + 4 );
			int _speech			=	_param/100;
			int _phoneCall		=	(_param%100)/10;
			int _turnByTurn		=	_param%10;

			int _speechmode[] = {
				kAirPlaySpeechMode_NotApplicable,	
				kAirPlaySpeechMode_None,
				kAirPlaySpeechMode_Speaking,
				kAirPlaySpeechMode_Recognizing
			};

			int _tristate[] = {
				kAirPlayTriState_NotApplicable,
				kAirPlayTriState_False,
				kAirPlayTriState_True
			};
			g_message(" %d %d %d %d\n", _param, _speech,_phoneCall,_turnByTurn );
			AirPlayReceiverSessionChangeAppState( _self->inSession, _speechmode[ _speech ], _tristate[ _phoneCall ], 
					_tristate[ _turnByTurn ], NULL, NULL, _self->inContext );

//			AirPlayReceiverSessionChangeSpeechMode( _self->inSession, _speechmode[ _speech ], NULL, NULL, _self->inContext );	
//			AirPlayReceiverSessionChangePhoneCall( _self->inSession, _tristate[ _phoneCall ], NULL, NULL, _self->inContext );
//			AirPlayReceiverSessionChangeTurnByTurn( _self->inSession, _tristate[ _turnByTurn ], NULL, NULL, _self->inContext );
		}
		else
		{
			g_warning( "Unknown change modes command: %s", _value );
		}
		break;
	case PROP_DEV_CARPLAY_KEYFRAME:
		AirPlayReceiverSessionForceKeyFrame( _self->inSession, NULL, _self->inContext );
		break;
	case PROP_DEV_CARPLAY_REQUEST_SIRI_ACTION:
		AirPlayReceiverSessionRequestSiriAction( _self->inSession, g_value_get_int( value ), NULL, 
				_self->inContext );
		break;
	case PROP_DEV_CARPLAY_REQUEST_UI:
		AirPlayReceiverSessionRequestUI( _self->inSession, 
				CFStringCreateWithCString( NULL, g_value_get_string( value ), kCFStringEncodingUTF8),
				NULL, _self->inContext );
		break;
	case PROP_DEV_CARPLAY_NIGHTMODE:
		AirPlayReceiverSessionSetNightMode( _self->inSession, g_value_get_boolean( value ), NULL,
				_self->inContext );
		break;
	case PROP_DEV_CARPLAY_SETLIMITUI:
		AirPlayReceiverSessionSetLimitedUI( _self->inSession, g_value_get_boolean( value ), NULL,
				_self->inContext );
		break;
	case PROP_DEV_CARPLAY_SIGNAL_TOUCH:
		_value	=	g_value_get_string( value );
			
		post_hid_report( _value );

		break;
	case PROP_DEV_CARPLAY_MEDIA_BUTTON:

		post_media_hid_report( g_value_get_uint( value ) );

		break;
	case PROP_DEV_CARPLAY_PHONE_BUTTON:

		post_hid_phone_report( g_value_get_uint( value ) );

		break;
	default:
		break;
	}
}       /*  -----  end of static function _set_property  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  wifi_write_iap2
 *  Description:  
 * =====================================================================================
 */
static MHResult wifi_write_iap2( MHDev * self, guint8 * buf, gint len )
{
	carplayObject->write( self, buf, len );

	return 0;
}		/* -----  end of static function wifi_write_iap2  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _dispose
 *  Description:
 * =====================================================================================
 */
static void _dispose( GObject * object )
{
	g_message("carplay %s\n",__func__);
	MHDevCarplay * _self	=	MH_DEV_CARPLAY( object );
	MHDev * _dev	=	MH_DEV( object );
	g_free( _dev->type );
//	gst_element_set_state( (GstElement *)_self->videoPipeline, GST_STATE_READY);

//	AirPlayReceiverSessionPlatformFinalize( _self->inSession );
//	AirPlayReceiverSessionTearDown( _self->inSession, NULL, kNoErr, NULL );
//	AirPlayReceiverServerControl( _self->inServer, kCFObjectFlagDirect, CFSTR( kAirPlayCommand_SessionDied ), _self->inSession, NULL, NULL );
	g_message("carplay %s end\n",__func__);
	G_OBJECT_CLASS( mh_dev_carplay_parent_class )->dispose( object );
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _finalize
 *  Description:
 * =====================================================================================
 */
static void _finalize( GObject * object )
{
	g_message("carplay %s\n",__func__);
	MHDevCarplay * _self	=	MH_DEV_CARPLAY( object );
	MHDevCarplayPrivate * _priv	=	mh_dev_carplay_get_instance_private( _self );
	g_message("carplay %s  end\n",__func__);
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_init
 *  Description:
 * =====================================================================================
 */
static void mh_dev_carplay_init( MHDevCarplay * self )
{
	MHDevCarplayPrivate * _priv	=	mh_dev_carplay_get_instance_private( self );
	MHDev * _dev	=	MH_DEV( self );
	AirPlayModeChanges _modes;
	g_message("%s self = %p",__func__,self);
	_dev->type	=	g_strdup("carplay");
	_dev->serial	=	"virtual_carplay_device";
	g_message("%s ======================_dev = %p",__func__,_dev);

	self->pb	=	mh_misc_get_pb_handle();

	AirPlayModeChangesInit( &_modes );
	uint8_t * _param = NULL;
	mh_misc_read_carplay_init_modes( &_param );

	DEBUG_HEX_DISPLAY( _param, _param[0] );

	_modes.screen.type	=	airPlayTransferType[ _param[1] ];
	_modes.screen.priority	=	airPlayTransferPriority[ _param[2] ];
	_modes.screen.takeConstraint	=	airPlayConstraint[ _param[3] ];
	_modes.screen.borrowOrUnborrowConstraint	=	airPlayConstraint[ _param[4] ];
	
	_modes.mainAudio.type	=	airPlayTransferType[ _param[5] ];
	_modes.mainAudio.priority	=	airPlayTransferPriority[ _param[6] ];
	_modes.mainAudio.takeConstraint	=	airPlayConstraint[ _param[7] ];
	_modes.mainAudio.borrowOrUnborrowConstraint	=	airPlayConstraint[ _param[8] ];

	_modes.speech		=	airPlaySpeechMode[_param[9]];
	_modes.phoneCall	=	airPlayTriState[_param[10]];
	_modes.turnByTurn	=	airPlayTriState[_param[11]];

	self->modes	=	AirPlayCreateModesDictionary( &_modes, NULL, NULL );
}       /* -----  end of static function mh_dev_carplay_init  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_class_init
 *  Description:
 * =====================================================================================
 */
static void mh_dev_carplay_class_init( MHDevCarplayClass * klass )
{
	MHDevClass * _parentClass	=	MH_DEV_CLASS( klass );
	GObjectClass * _gobjectClass	=	G_OBJECT_CLASS( klass );

	_gobjectClass->dispose	=	_dispose;
	_gobjectClass->finalize	=	_finalize;
	_gobjectClass->set_property =   _set_property;
	_gobjectClass->get_property =   _get_property;

	_parentClass->wifi_write_iap2	=	wifi_write_iap2;
	/* assemble ios of mh_dev_carplay class */

	/* assemble methods of mh_dev_carplay class */

	/* Properties of mh_dev_carplay class */

	devCarplayProperties[ PROP_DEV_CARPLAY_CHANGEMODES ]	=	
		g_param_spec_string( "change_modes", "MHDevCarplay's property", "Change CarPlay modes request",
				"", G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_KEYFRAME ]	=	
		g_param_spec_boolean( "keyframe", "MHDevCarplay's property", "Force a key frame to be sent for the screen stream",
				FALSE, G_PARAM_READWRITE );

	devCarplayProperties[ PROP_DEV_CARPLAY_REQUEST_SIRI_ACTION ]	=	
		g_param_spec_int( "request_siri_action", "MHDevCarplay's property", "Requests that Siri be invoked with a specified action",
				0, 3, 0, G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_REQUEST_UI ]	=	
		g_param_spec_string( "request_ui", "MHDevCarplay's property", "Ask for the	CarPlay UI to be shown",
				"", G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_NIGHTMODE ]	=	
		g_param_spec_boolean( "night_mode", "MHDevCarplay's property", "Indicate whether it is dark outside",
				FALSE, G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_SETLIMITUI ]	=	
		g_param_spec_boolean( "set_limit_ui", "MHDevCarplay's property", "Indicate whether or not to limit certain UI elements",
				FALSE, G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_VIDEO_SINK ]	=	
		g_param_spec_uint( "video_sink", "MHDevCarplay's property", "Video sink of CarPlay video output",
				0, G_MAXUINT, 0, G_PARAM_READABLE );
	
	devCarplayProperties[ PROP_DEV_CARPLAY_SIGNAL_TOUCH ]	=	
		g_param_spec_string( "signal_touch", "MHDevCarplay's property", "Signal touch data",
				"", G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_MEDIA_BUTTON ]	=	
		g_param_spec_uint( "media_button", "MHDevCarplay's property", "media control button",
				0, 7, 0, G_PARAM_WRITABLE );

	devCarplayProperties[ PROP_DEV_CARPLAY_PHONE_BUTTON ]	=	
		g_param_spec_uint( "phone_key", "MHDevCarplay's property", "phone control key",
				0, 20, 0, G_PARAM_WRITABLE );
	g_object_class_install_properties( _gobjectClass, N_PROPERTIES, devCarplayProperties );

	/* Ios */

	/* Signals */
	signals[ DUCK_AUDIO ]	=	
		g_signal_new( "duck_audio",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				2,
				G_TYPE_DOUBLE, G_TYPE_DOUBLE );       /* durationMS, volume */

	signals[ UNDUCK_AUDIO ]	=	
		g_signal_new( "unduck_audio",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				1,
				G_TYPE_DOUBLE );       /* durationMS */

	signals[ DISALBE_BLUETOOTH ]	=	
		g_signal_new( "disable_bluetooth",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				1,
				G_TYPE_STRING );       /* devicdID */

	signals[ MODES_CHANGED ]	=	
		g_signal_new( "modes_changed",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				6,
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
				G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );

	signals[ HID_SET_INPUT_MODE ]	=	
		g_signal_new( "hid_set_input_mode",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				2,
				G_TYPE_INT, G_TYPE_STRING );    /* hidInputMode, uuid */

	signals[ REQUEST_UI ]	=	
		g_signal_new( "request_ui",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				0 );                /* URL */
	signals[ AUDIO_INFO ]	=	
		g_signal_new( "audio_info",
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST,
				0,
				NULL,
				NULL,
				g_cclosure_marshal_generic,
				G_TYPE_NONE,
				4,
				G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT );

}       /* -----  end of static function mh_dev_carplay_class_init  ----- */

