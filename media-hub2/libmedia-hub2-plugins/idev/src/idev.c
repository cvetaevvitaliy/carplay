/*
 * =====================================================================================
 *
 *       Filename:  idev.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/07/2015 04:01:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <glib.h>
#include <stdlib.h>
#include <mh_core.h>
#include <mh_misc.h>
#include "dev_iap1.h"
#include "dev_iap2.h"
#include "transport.h"
#include <sys/poll.h>
#include <string.h>

static struct udev * udevCtx;
static GPollFD udevFd;
static struct udev_monitor * udevMonitor;

MHDev * carplay;
#if 1 //double iap2
extern GSList * _iAP2GlobalInfolist;
#endif

MHDevIap1 * _iap1	=	NULL;
MHDevIap2 * tempIap2 = NULL;

gboolean roleSwitching;

iAP2PacketSYNData_t synParam = {
	.version				=	1,
	.maxOutstandingPackets	=	kiAP2LinkSynValMaxOutstandingMax,
	.maxPacketSize			=	kiAP2LinkSynValMaxPacketSizeMax,
	.retransmitTimeout		=	2000,
	.cumAckTimeout			=	1000,
	.maxRetransmissions		=	30,
	.maxCumAck				=	3,
	.numSessionInfo			=	3,
	.sessionInfo			=	{
		{
		.id			=	IAP2_CONTROL_SESSION_ID,
		.type		=	kIAP2PacketServiceTypeControl,
		.version	=	2
		},
		{
		.id			=	IAP2_FILE_SESSION_ID,
		.type		=	kIAP2PacketServiceTypeBuffer,
		.version	=	1
		},
		{
		.id			=	IAP2_EA_SESSION_ID,
		.type		=	kIAP2PacketServiceTypeEA,
		.version	=	1
		}
	}
};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _is_idev
 *  Description:  
 * =====================================================================================
 */
static gboolean _is_idev( struct udev_list_entry * properties )
{
	struct udev_list_entry * _entry, * _model;

	_model	=	udev_list_entry_get_by_name( properties, "ID_VENDOR_ID" );

	if( _model != NULL && g_ascii_strcasecmp( udev_list_entry_get_value( _model ), "05ac" ) == 0 )
	{
		_model	=	udev_list_entry_get_by_name( properties, "ID_MODEL_ID" );

		if( g_ascii_strncasecmp( udev_list_entry_get_value( _model ), "12", 2 ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}		/* -----  end of static function _is_idev  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  is_iap2
 *  Description:  
 * =====================================================================================
 */
static gboolean is_iap2(int hid)
{
	guint8 _detect[]	=	{0xFF, 0x55, 0x02, 0x00, 0xEE, 0x10};
	guint8 _result[16]	=	{0};

	dev_write( _detect, sizeof( _detect ), hid, NULL );

	dev_read( _result, 16, hid, NULL );

	if( memcmp( _detect, _result + 2, sizeof( _detect )) == 0 )
		return TRUE;
	else
		return FALSE;
}		/* -----  end of static function is_iap2  ----- */

static gboolean check_is_iap2( struct udev_list_entry * properties)
{
	struct udev_list_entry * _model;
	const gchar * _value;
	gboolean res = FALSE;
	int hid = 0;
	_model	=	udev_list_entry_get_by_name( properties, "DEVNAME" );
	_value	=	udev_list_entry_get_value( _model );

	hid = setup_hid( _value ) ;
	if( 0 == hid)
		return FALSE;

	/* Detect iAP1 or iAP2 */
	if( is_iap2(hid) )
		res = TRUE;
	else
		res = FALSE;

	g_message("check_is_iap2 res = %d ", res);

	closehidFd(hid);
	return res;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _mount_idev
 *  Description:  
 * =====================================================================================
 */
static MHDev * _mount_idev( struct udev_list_entry * properties )
{
	MHDevIap2 * _ret2	=	NULL;
	MHDev * _ret	=	NULL;
	struct udev_list_entry * _model;
	const gchar * _value;
	int hid = 0;
	_model	=	udev_list_entry_get_by_name( properties, "DEVNAME" );
	_value	=	udev_list_entry_get_value( _model );

	hid = setup_hid( _value ) ;
	if( 0 == hid)
		return NULL;

	g_message("_mount_idev HID = %d ", hid);
	/* Detect iAP1 or iAP2 */
	if( is_iap2(hid) )
	{
		_ret2	=	g_object_new( MH_TYPE_DEV_IAP2, "io-name", "iap2", NULL );
		_ret2->_iap2hidFd = hid;
		/* use device mode apis */
		_ret2->read	=	dev_read;
		_ret2->write	=	dev_write;
		_ret2->hostMode	=	FALSE;

		/* GThread will hold itself reference */
		_ret2->hidTask	=	g_thread_new( "iap2_hid_task", iap2_hid_task, _ret2 );

		/* GThread will hold itself reference */
		g_thread_unref( _ret2->hidTask );

		iAP2LinkRunLoopAttached( _ret2->plinkRunLoop );

		_ret	=	MH_DEV( _ret2 );
	}else{
		g_message(" iAP2 don't support device!");
		closehidFd(hid);
		_iap1	=	g_object_new( MH_TYPE_DEV_IAP1, "io-name", "iap1", NULL );
	}
//	else
//	{
//		_ret1	=	g_object_new( MH_TYPE_DEV_IAP1, "io-name", "iap1", NULL );
//
//		_ret	=	MH_DEV( _ret1 );
//	}

	if( _ret != NULL )
	{
		_model	=	udev_list_entry_get_by_name( properties, "ID_SERIAL_SHORT" );

		if( _model != NULL )
		{
			_value	=	udev_list_entry_get_value( _model );

			_ret->serial	=	g_strdup( _value );
		}
		
		_model	=	udev_list_entry_get_by_name( properties, "DEVPATH" );
		
		if( _model != NULL )
		{
			_value	=	udev_list_entry_get_value( _model );

			_ret->devPath	=	g_strdup( _value );
		}

		mh_core_attach_dev( MH_DEV( _ret ));
	}

	return _ret;
}		/* -----  end of static function _mount_idev  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _scan_idev
 *  Description:  
 * =====================================================================================
 */
static gboolean _scan_idev()
{
	struct udev_enumerate * _enum;
	struct udev_list_entry * _idev, * _entry, * _properties;
	struct udev_device * _udevDev;

	_enum	=	udev_enumerate_new( udevCtx );

	if( _enum == NULL )
	{
		g_warning( "udev_enumerate_new failed" );

		return FALSE;
	}

	udev_enumerate_add_match_subsystem( _enum, "usb" );

	udev_enumerate_scan_devices( _enum );

	_idev	=	udev_enumerate_get_list_entry( _enum );

	udev_list_entry_foreach( _entry, _idev )
	{
		const gchar * _path	=	udev_list_entry_get_name( _entry );
		struct udev_list_entry * _entry1;

		_udevDev	=	udev_device_new_from_syspath( udevCtx, _path );

		_properties	=	udev_device_get_properties_list_entry( _udevDev );

#if 1  //double iap2
		if(_is_idev( _properties ))
#else
		if((iAP2Object == NULL ) && _is_idev( _properties ))
#endif
		{
			MHDev * _idev	=	NULL;

//			if ( iAP2Object != NULL )
//				return TRUE;
			if (TRUE == check_is_iap2(_properties))
			{
				switch( role_switch( _properties ))
				{
				case SWITCH_SUCCESS:
					roleSwitching	=	TRUE;
					break;
				case SWITCH_UNSUPPORTED:
					if(( _idev = _mount_idev( _properties )) == NULL )
					{
						g_warning( "mount idev failed" );
					}
					break;
				case SWITCH_DEV_INVALID:
					break;
				default:
					break;
				}
			}
			else
			{
				if(( _idev = _mount_idev( _properties )) == NULL )
				{
					g_warning( "mount idev failed" );
				}
			}
			/* we will inform app the device was attached after authenticated */
//			mh_core_attach_dev( MH_DEV( _idev ));
		}

		udev_device_unref( _udevDev );
	}

	udev_enumerate_unref( _enum );

	return TRUE;
}		/* -----  end of static function _scan_idev  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _serial_compare
 *  Description:  
 * =====================================================================================
 */
static gint _serial_compare( gconstpointer a, gconstpointer b )
{
	return g_strcmp0( MH_DEV(a)->serial, b );
}		/* -----  end of static function _serial_compare  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _parse_events
 *  Description:  
 * =====================================================================================
 */
static gboolean _parse_events( gpointer user_data )
{
	MHCore * _core	=	MH_CORE( user_data );
	struct udev_device * _udevDev;
	struct udev_list_entry * _properties;

	_udevDev	=	udev_monitor_receive_device( udevMonitor );

	if( _udevDev )
	{
		const gchar * _udevAction;

		_udevAction	=	udev_device_get_action( _udevDev );

		_properties	=	udev_device_get_properties_list_entry( _udevDev );

		if( _is_idev( _properties ))
		{
#if 1  //double iap2
			if(( g_strcmp0( _udevAction, "add" ) == 0 ))
#else
			if(( g_strcmp0( _udevAction, "add" ) == 0 ) && ( iAP2Object == NULL ))
#endif
			{
				MHDev * _idev;
				if (TRUE == check_is_iap2(_properties))
				{
					switch( role_switch( _properties ))
					{
					case SWITCH_SUCCESS:
						roleSwitching	=	TRUE;
						break;
					case SWITCH_UNSUPPORTED:
						if(( _idev = _mount_idev( _properties )) == NULL )
						{
							g_warning( "mount idev failed" );
						}
						break;
					case SWITCH_DEV_INVALID:
						break;
					default:
						break;
					}
				}
				else
				{
					if(( _idev = _mount_idev( _properties )) == NULL )
					{
						g_warning( "mount idev failed" );
					}
				}
				/* we will inform app the device was attached after authenticated */
//				mh_core_attach_dev( MH_DEV( _idev ));
			}
			else
			if( g_strcmp0( _udevAction, "remove" ) == 0 )
			{
				const gchar * _serial;
				struct udev_list_entry * _model;
				MHDev * _dev;
				GSList *iterator = NULL;
				MHDevIap2 * _iAP2Object	=	NULL;

				if( !roleSwitching )
				{
//					if( iAP2Object != NULL )
					{
						_model	=	udev_list_entry_get_by_name( _properties, "ID_SERIAL_SHORT" );

						if( _model != NULL )
						{
							_serial	=	udev_list_entry_get_value( _model );
						}

						for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
						{
							_iAP2Object= ((iAP2GlobalInfo *)iterator->data)->piAP2Object;
							if (g_strcmp0( _serial, MH_DEV( _iAP2Object )->serial	 ) == 0)
							{
								g_message("_parse_events find _iAP2Object to mh_core_detach_dev iAP2 _iAP2Object = %p",_iAP2Object);
								mh_core_detach_dev( MH_DEV( _iAP2Object ));	
								g_object_unref( _iAP2Object );
								break;
							}
						}
					}
				}
				
				if(_iap1 != NULL)
				{
					g_message("remove iap1");
					mh_core_detach_dev( MH_DEV( _iap1 ));
					g_object_unref( _iap1 );
					mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );
					_iap1 = NULL;
				}
			}
			else
			{
				g_warning( "Unknown udev action( %s ) received", _udevAction );
			}
		}

		udev_device_unref( _udevDev );
	}

	return G_SOURCE_CONTINUE;
}		/* -----  end of static function _parse_events  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _prepare
 *  Description:  
 * =====================================================================================
 */
static gboolean _prepare( GSource * source, gint * timeout )
{
	* timeout	=	-1;
	
	return FALSE;
}		/* -----  end of static function _prepare  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _check
 *  Description:  
 * =====================================================================================
 */
static gboolean _check( GSource * source )
{
	gboolean _ret	=	FALSE;

	if( udevFd.revents != 0 )
	{
		_ret	=	TRUE;
	}

	return _ret;
}		/* -----  end of static function _check  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dispatch
 *  Description:  
 * =====================================================================================
 */
static gboolean _dispatch( GSource * source, GSourceFunc callback, gpointer user_data )
{
	return callback( user_data );
}		/* -----  end of static function _dispatch  ----- */

static GSourceFuncs _funcs	=	
{
	.prepare	=	_prepare,
	.check		=	_check,
	.dispatch	=	_dispatch
};
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _register_monitor
 *  Description:  
 * =====================================================================================
 */
static gboolean _register_monitor()
{
	GSource * _source	=	NULL;
	udevMonitor	=	udev_monitor_new_from_netlink( udevCtx, "udev" );

	if( udevMonitor == NULL )
	{
		g_warning( "udev_monitor_new_from_netlink failed" );

		goto NEED_FREE_CTX;
	}

	if( udev_monitor_filter_add_match_subsystem_devtype( udevMonitor, "usb", 0 ) < 0 )
	{
		g_warning( "udev_monitor_filter_add_match_subsystem_devtype failed" );

		goto NEED_FREE_MONITOR;
	}

	if ( udev_monitor_enable_receiving( udevMonitor ) < 0 )
	{
		g_warning( "udev_monitor_enable_receiving failed" );

		goto NEED_FREE_MONITOR;
	}

	udevFd.fd	=	udev_monitor_get_fd( udevMonitor );

	if( udevFd.fd > 0 )
	{
		udevFd.events	=	POLLIN;

		_source	=	g_source_new( &_funcs, sizeof( GSource ));

		g_source_add_poll( _source, &udevFd );

		g_source_set_callback( _source, _parse_events, mh_core_instance(), NULL );

		mh_io_dispatch( MH_IO( mh_core_instance() ), _source );

		g_source_unref( _source );
	}
	else
	{
		g_warning( "udev_monitor_get_fd faild" );

		goto NEED_FREE_MONITOR;
	}

	return TRUE;

NEED_FREE_MONITOR:
	udev_monitor_unref( udevMonitor );
NEED_FREE_CTX:
	udev_unref( udevCtx );

	return FALSE;
}		/* -----  end of static function _register_monitor  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _init
 *  Description:  
 * =====================================================================================
 */
static gboolean _init( gpointer user_data )
{
	g_message("iAP2 %s",__func__);
//	ffs_setup();
	const char * _eaProName	=	getenv("MH_EA_PROTOCOL_NAME");
	_eaProName	=	_eaProName ? _eaProName : "com.neusoft.ea";

	if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY )
	{
		const char * _nowPlaying	=	getenv("MH_IAP_NOW_PLAYING_UPDATE");
		g_message("iAP2 %s _eaProName = %s",_nowPlaying, _eaProName);
		if( _nowPlaying == NULL )
		{
			g_message("-----idev.c : %d-----numSessionInfo	= 1",__LINE__);
			synParam.numSessionInfo	= 1;
		}else{
			if (NULL == _eaProName)
			{
				g_message("-----idev.c : %d-----numSessionInfo	= 2",__LINE__);
				g_message("iAP2 %s",__func__);
				synParam.numSessionInfo = 2;
			}
			else
			{
				g_message("-----idev.c : %d-----numSessionInfo	= 3",__LINE__);
				g_message("iAP2 %s",__func__);
				synParam.numSessionInfo = 3;

			}
		}
	}

	udevCtx	=	udev_new();

	if( udevCtx == NULL)
	{
		g_warning( "udev_new failed" );

		goto RETURN; 
	}
	/* Scanning the connected apple device */
	if( !_scan_idev())
	{
		g_warning( "_scan_idev failed" );

		goto RETURN; 
	}

	/* Registering udev event monitor */
	if( !_register_monitor())
	{
		g_warning( "_register_monitor failed" );

		goto RETURN; 
	}

RETURN:
	g_signal_emit_by_name(mh_core_instance(), "core_events", MH_CORE_PLUGIN_LOAD_SUCCESS, "iap2");
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
	g_message("iAP2 %s",__func__);
	_init(NULL);
	
	return TRUE;
//	GSource * _source	=	g_idle_source_new();
//
//	g_source_set_callback( _source, _init, NULL, NULL );
//
//	mh_io_dispatch( MH_IO( mh_core_instance() ), _source );
//
//	g_source_unref( _source );
//
//	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _type_compare
 *  Description:  
 * =====================================================================================
 */
static gint _type_compare( gconstpointer a, gconstpointer b )
{
	return g_strcmp0( MH_DEV(a)->type, b );
}		/* -----  end of static function _type_compare  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_find_dev
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_find_dev( MHDevParam * param )
{
	g_message("iap %s type = [%d]  mac_addr = [%s] connect = [%d]\n", 
			__func__, param->type, param->mac_addr, param->connect_status );	
	
	int temp = 0;
	if( param->type == MH_DEV_BT_IAP )
	{
		if( param->connect_status == 1 )
		{
#if 0 //tongxsh		
			if( mh_misc_get_iap_device_mode() == MISC_IAP_CARPLAY )
			{
				MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
				_devParam->type				=	MH_DEV_WIFI_CARPLAY;
				_devParam->mac_addr			=	g_strdup( param->mac_addr );
				_devParam->connect_status	=	3;	

				mh_core_find_dev( _devParam );

				g_free( _devParam->mac_addr );
				g_free( _devParam );
			}	
#endif
			MHDevIap2 * _ret2	=	NULL;
			if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY )
			{
				temp = synParam.numSessionInfo;
				g_message("-----idev.c : %d-----numSessionInfo	= 1",__LINE__);
				synParam.numSessionInfo = 1;
			}

			_ret2	=	g_object_new( MH_TYPE_DEV_IAP2, "io-name", "iap2", NULL );

			/* use device mode apis */
			_ret2->read				=	bt_read_iap2;
			_ret2->write			=	bt_write_iap2;
			_ret2->hostMode			=	FALSE;
			_ret2->transportType	=	MH_DEV_BT_IAP;
			_ret2->macAddress		=	g_strdup( param->mac_addr );
			MH_DEV( _ret2 )->serial =	g_strdup( param->mac_addr );	
			
			MHDev 	  * _dev	=	MH_DEV( _ret2 );

			if( _dev->type != NULL )
			{
				g_free( _dev->type );	
				_dev->type = g_strdup( "iap2_bt" );
			}

			iAP2LinkRunLoopAttached( _ret2->plinkRunLoop );

			mh_core_attach_dev( MH_DEV( _dev ));
			
			if (temp != 0)
			{
				g_message("-----idev.c : %d-----numSessionInfo	= %d",__LINE__, temp);
				synParam.numSessionInfo = temp;
			}
			g_message("%s MH_DEV_BT_IAP %p\n",__func__,_ret2);
		}
		else
		{
			GSList *iterator = NULL;
			MHDevIap2 * _iAP2Object	=	NULL;	

			for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
			{
				_iAP2Object= ((iAP2GlobalInfo *)iterator->data)->piAP2Object;
				if (g_strcmp0( param->mac_addr,  _iAP2Object->macAddress	 ) == 0 )
				{
				
					MHDev	  * _dev1	=	MH_DEV( _iAP2Object );
					g_message("_parse_events find _iAP2Object to mh_core_detach_dev BT _iAP2Object = %p _dev->type = %s, tempIap2 = %p ",
						_iAP2Object, _dev1->type, tempIap2);
					if (tempIap2 != NULL) //tongxsh
						g_signal_emit_by_name( MH_DEV( tempIap2), "dev_status", IAP_AUTH_SUCCESS);
					tempIap2 = NULL;
					mh_core_detach_dev( MH_DEV( _iAP2Object ));	
					g_object_unref( _iAP2Object );
					break;
				}
			}
		}
	}
	else
	if( param->type == MH_DEV_WIFI_IAP )
	{
		if( param->connect_status == 1 )
		{
			carplay	=	MH_DEV( mh_core_find_dev_custom( "carplay_wifi", _type_compare ));

			MHDevIap2 * _ret2	=	NULL;
			if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY )
			{
				g_message("-----idev.c : %d-----numSessionInfo	= 2",__LINE__);
				temp = synParam.numSessionInfo;
				synParam.numSessionInfo = 2;
			}

			_ret2	=	g_object_new( MH_TYPE_DEV_IAP2, "io-name", "iap2", NULL );

			/* use device mode apis */
			_ret2->read				=	wifi_read_iap2;
			_ret2->write			=	wifi_write_iap2_dev;
			_ret2->hostMode			=	FALSE;
			_ret2->transportType	=	MH_DEV_WIFI_IAP;
			_ret2->macAddress		=	g_strdup( param->mac_addr );
			MH_DEV( _ret2 )->serial =	g_strdup( param->mac_addr );	

			MHDev 	  * _dev	=	MH_DEV( _ret2 );

			if( _dev->type != NULL )
			{
				g_free( _dev->type );	
			}
			_dev->type = g_strdup( "iap2_wifi" );

			iAP2LinkRunLoopAttached( _ret2->plinkRunLoop );

			mh_core_attach_dev( MH_DEV( _dev ));
			tempIap2 = _ret2;
			if (temp != 0)
			{
				g_message("-----idev.c : %d-----numSessionInfo	= %d",__LINE__, temp);
				synParam.numSessionInfo = temp;
			}

			g_message("%s MH_DEV_WIFI_IAP %p\n",__func__,_ret2);
		}else{
			GSList *iterator = NULL;
			MHDevIap2 * _iAP2Object	=	NULL;	

			for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
			{
				_iAP2Object= ((iAP2GlobalInfo *)iterator->data)->piAP2Object;
				g_message("param->mac_addr = %s macAddress	=	%s\n",param->mac_addr, _iAP2Object->macAddress);
				if (g_strcmp0( param->mac_addr,  _iAP2Object->macAddress	 ) == 0)
				{
					g_message("_parse_events find _iAP2Object to mh_core_detach_dev WIFI _iAP2Object = %p",_iAP2Object);
					mh_core_detach_dev( MH_DEV( _iAP2Object ));	
					g_object_unref( _iAP2Object );
					break;
				}
			}
		
			if( carplay != NULL )
			{
				MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
				_devParam->type				=	MH_DEV_WIFI_CARPLAY;
				_devParam->mac_addr			=	g_strdup( param->mac_addr );
				_devParam->connect_status	=	2;	

				mh_core_find_dev( _devParam );

				g_free( _devParam->mac_addr );
				g_free( _devParam );
			}else{
				g_message("Don't wifi carplay device\n");
			}
		}
	}else{
		g_message(" idev error\n");
	}

	return TRUE;
}		/* -----  end of function mh_plugin_find_dev  ----- */
