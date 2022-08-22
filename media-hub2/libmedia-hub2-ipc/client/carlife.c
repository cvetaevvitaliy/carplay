/*
 * =====================================================================================
 *
 *       Filename:  carlife.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/07/2016 01:18:43 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <mh_api.h>
#include <stdio.h>
#include <string.h>
#include <mh_carlife.h>
#include <glib.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif
extern Mediahub2Dbus * dbusClient;

typedef struct _MHCarlifeBtHtpDBusInfo 
{
	MHDev * dev;
	MHCarlifeBtHfpRequestListener listener;
	uint64_t event_id;
	uint64_t detach_id;
} MHCarlifeBtHtpDBusInfo;				/* ----------  end of struct MHCarlifeBtHtpDBusInfo  ---------- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_send_hardkey
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_send_hardkey(MHDev *dev,  MHCarlifeHardkey key)
{
	g_return_val_if_fail( dev != NULL, MH_INVALID_PARAM);
	if( key > MH_CARLIFE_HARDKEY_NUMBER_PLUS || key <  MH_CARLIFE_HARDKEY_HOME)
	{
		return MH_INVALID_PARAM;
	}
	MHResult _res	=	MH_OK;

	_res	=	mh_object_set_properties(( MHObject *)dev, "send_hardkey", key, NULL);

	return _res;
}		/* -----  end of function mh_dev_carlife_send_hardkey  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_video_start
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_video_start( MHDev * dev)
{
	g_return_val_if_fail( dev != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	_res	=	mh_object_set_properties( (MHObject*)dev, "video_status", 2, NULL);

	return _res;
}		/* -----  end of function mh_dev_carlife_video_start  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_video_pause
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_video_pause( MHDev *dev)
{
	g_return_val_if_fail( dev != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	_res	=	mh_object_set_properties( (MHObject*)dev, "video_status", 0, NULL);

	return _res;
}		/* -----  end of function mh_dev_carlife_video_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_video_reset
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_video_reset( MHDev * dev)
{
	g_return_val_if_fail( dev != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	_res	=	mh_object_set_properties( (MHObject*)dev, "video_status", 1, NULL);

	return _res;
}		/* -----  end of function mh_dev_carlife_video_reset  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_velocity
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_car_velocity( MHDev * dev, MHCarlifeCarVelocity *velocity)
{
	g_return_val_if_fail( dev != NULL && velocity != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(ut)", velocity->speed, velocity->timeStamp);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_velocity",_var , NULL);

	return _res;

}		/* -----  end of function mh_dev_carlife_car_velocity  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_GPS
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_car_GPS( MHDev * dev, MHCarlifeCarGPS * gps)
{
	g_return_val_if_fail( dev != NULL && gps !=  NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;
	
	GVariantBuilder * _builder;

	GVariant  *_var;

	_builder	=	g_variant_builder_new( G_VARIANT_TYPE("au"));

	g_variant_builder_add( _builder, "u", gps->antennaState);
	g_variant_builder_add( _builder, "u", gps->signalQuality);
	g_variant_builder_add( _builder, "u", gps->latitude);
	g_variant_builder_add( _builder, "u", gps->longitude);
	g_variant_builder_add( _builder, "u", gps->height);

	g_variant_builder_add( _builder, "u", gps->speed);
	g_variant_builder_add( _builder, "u", gps->heading);
	g_variant_builder_add( _builder, "u", gps->year);
	g_variant_builder_add( _builder, "u", gps->month);
	g_variant_builder_add( _builder, "u", gps->day);

	g_variant_builder_add( _builder, "u", gps->hour);
	g_variant_builder_add( _builder, "u", gps->min);
	g_variant_builder_add( _builder, "u", gps->sec);
	g_variant_builder_add( _builder, "u", gps->fix);
	g_variant_builder_add( _builder, "u", gps->hdop);

	g_variant_builder_add( _builder, "u", gps->pdop);
	g_variant_builder_add( _builder, "u", gps->vdop);
	g_variant_builder_add( _builder, "u", gps->satsUsed);
	g_variant_builder_add( _builder, "u", gps->satsVisible);
	g_variant_builder_add( _builder, "u", gps->horPosError);

	g_variant_builder_add( _builder, "u", gps->vertPosError);
	g_variant_builder_add( _builder, "u", gps->northSpeed);
	g_variant_builder_add( _builder, "u", gps->eastSpeed);
	g_variant_builder_add( _builder, "u", gps->vertSpeed);

	_var	=	g_variant_builder_end( _builder);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_gps",_var , NULL);

//	g_variant_unref( _var);

	return _res;
}		/* -----  end of function mh_dev_carlife_car_GPS  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_gyroscope
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_car_gyroscope( MHDev * dev, MHCarlifeCarGyroscope * gyroscope)
{
	g_return_val_if_fail( dev != NULL && gyroscope !=  NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(idddt)", gyroscope->gyroType, gyroscope->gyroX,
			gyroscope->gyroY, gyroscope->gyroZ, gyroscope->timeStamp);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_gyroscope", _var , NULL);

//	g_variant_unref( _var );

	return _res;
}		/* -----  end of function mh_dev_carlife_car_gyroscope  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_acceleration
 *  Description:  
 * =====================================================================================
 */

MHResult mh_dev_carlife_car_acceleration( MHDev *dev, MHCarlifeCarAcceleration * acceleration)
{
	g_return_val_if_fail( dev != NULL && acceleration !=  NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(dddt)", acceleration->accX, acceleration->accY,
			acceleration->accZ, acceleration->timeStamp);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_acceleration",_var , NULL);

//	g_variant_unref( _var);

	return _res;
}		/* -----  end of function mh_dev_carlife_car_acceleration  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_oil
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_car_oil( MHDev * dev, MHCarlifeCarOil * oil)
{
	g_return_val_if_fail( dev != NULL && oil != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(iib)", oil->level, oil->range,
			oil->lowFullWarning);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_oil",_var , NULL);

	return _res;

}		/* -----  end of function mh_dev_carlife_car_oil  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_launch_mode
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_launch_mode( MHDev * dev, MHCarlifeLaunchMode mode)
{
	g_return_val_if_fail( dev!= NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	_res	=	mh_object_set_properties((MHObject *)dev, "launch_mode", mode, NULL);
	
	return 0;
}		/* -----  end of function mh_dev_carlife_launch_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carlife_car_info
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_carlife_car_info( MHDev * dev, MHCarlifeCarInfo * info)
{
	g_return_val_if_fail( dev != NULL && info != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var, *_var1;

	GVariantBuilder * _builder, * _builder1;

	_builder	=	g_variant_builder_new( G_VARIANT_TYPE("as"));

	g_variant_builder_add( _builder, "s", info->os ? info->os : "");
	g_variant_builder_add( _builder, "s", info->board ? info->board : "");
	g_variant_builder_add( _builder, "s", info->bootloader ? info->bootloader : "");
	g_variant_builder_add( _builder, "s", info->brand ? info->brand : "");
	g_variant_builder_add( _builder, "s", info->cpu_abi ? info->cpu_abi : "");
	g_variant_builder_add( _builder, "s", info->cpu_abi2 ? info->cpu_abi2 : "");
	g_variant_builder_add( _builder, "s", info->device ? info->device : "");
	g_variant_builder_add( _builder, "s", info->display ? info->display : "");
	g_variant_builder_add( _builder, "s", info->fingerprint ? info->fingerprint : "");
	g_variant_builder_add( _builder, "s", info->hardware ? info->hardware : "");
	g_variant_builder_add( _builder, "s", info->host ? info->host : "");
	g_variant_builder_add( _builder, "s", info->cid ? info->cid : "");
	g_variant_builder_add( _builder, "s", info->manufacturer ? info->manufacturer : "");
	g_variant_builder_add( _builder, "s", info->model ? info->model : "");
	g_variant_builder_add( _builder, "s", info->product ? info->product : "");
	g_variant_builder_add( _builder, "s", info->serial ? info->serial : "");
	g_variant_builder_add( _builder, "s", info->codename ? info->codename : "");
	g_variant_builder_add( _builder, "s", info->incremental ? info->incremental : "");
	g_variant_builder_add( _builder, "s", info->release ? info->release : "");
	g_variant_builder_add( _builder, "s", info->sdk ? info->sdk : "");
	g_variant_builder_add( _builder, "s", info->token ? info->token : "");
	g_variant_builder_add( _builder, "s", info->btaddress ? info->btaddress : "");


	_var1	=	g_variant_builder_end( _builder);

	_var	=	g_variant_new("(uv)", info->sdk_int, _var1);

	_res	=	mh_object_set_properties( (MHObject*)dev, "car_info", _var, NULL);

	return _res;
}		/* -----  end of function mh_dev_carlife_car_info  ----- */
MHResult mh_dev_carlife_bt_start_identify_req( MHDev *dev, MHCarlifeBtStartIdentifyReq * bt_start )
{
	MHResult _res	=	MH_OK;

	GVariant * _var	=	NULL;

	_var	=	g_variant_new("s", bt_start->address);
	
	_res	=	mh_object_set_properties( (MHObject*)dev, "bt_start_identify",_var , NULL);

	return _res;


}

MHResult mh_dev_carlife_send_bt_pair_info( MHDev * dev, MHCarlifeBtPairInfo * info)
{
	g_message("client-->carlife.c:%s\n\n", __func__);
	MHResult _res	=	MH_OK;

	GVariant * _var	=	NULL;
	
	_var	=	g_variant_new("(ssssssu)", info->address, info->passKey, info->hash, 
			info->randomizer, info->uuid, info->name, info->status);

	_res	=	mh_object_set_properties((MHObject *)dev, "hu_bt_pair_info", _var, NULL);

	return _res;
}
  
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _carlife_bt_hfp_request_cb
 *  Description:  
 * =====================================================================================
 */
static void _carlife_bt_hfp_request_cb(Mediahub2Dbus * proxy, MHDev * dev, uint32_t type, 
		const char * phone_num, int32_t dtmfcode, gpointer user_data)
{
	MHCarlifeBtHtpDBusInfo * _info	=	(MHCarlifeBtHtpDBusInfo *)user_data;
	if( dev == _info->dev)
	{
		MHCarlifeBtHfpRequest _request;
		_request.type	=	type;
		_request.phoneNum	=	g_strdup( phone_num);
		_request.dtmfCode	=	dtmfcode;
		_info->listener.callback( _info->dev, &_request, _info->listener.user_data);
		g_free( _request.phoneNum);
	}
}		/* -----  end of static function _carlife_bt_hfp_request_cb  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _carlife_bt_hfp_detach
 *  Description:  
 * =====================================================================================
 */
static void _carlife_bt_hfp_detach(Mediahub2Dbus * proxy, MHDev * dev, gpointer * user_data)
{
	MHCarlifeBtHtpDBusInfo * _info	=	(MHCarlifeBtHtpDBusInfo *)user_data;
	g_signal_handler_disconnect( dbusClient, _info->detach_id);
	g_signal_handler_disconnect( dbusClient, _info->event_id );
}		/* -----  end of static function _carlife_bt_hfp_detach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_register_bt_hfp_request
 *  Description:  
 * =====================================================================================
 */
MHResult mh_dev_register_bt_hfp_request( MHDev * self, MHCarlifeBtHfpRequestListener * listener)
{
	g_return_val_if_fail( self != NULL && listener != NULL && listener->callback != NULL, MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;

#ifdef __x86_64__
	mediahub2_dbus_call_carlife_register_bt_hfp_request_sync( dbusClient, ( guint64 )self,
			NULL, &_error );
#else
	mediahub2_dbus_call_carlife_register_bt_hfp_request_sync( dbusClient, ( guint )self,
			NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_dev_register_bt_hfp_request falied: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_INVALID_PARAM;
	}
	else
	{
		MHCarlifeBtHtpDBusInfo * _info	=	g_new0( MHCarlifeBtHtpDBusInfo, 1 );

		_info->dev	=	self;
		_info->listener		=	* listener;
		
		_info->event_id		=	g_signal_connect( dbusClient, "carlife_bt_hfp_request", G_CALLBACK( _carlife_bt_hfp_request_cb ), _info );
		_info->detach_id	=	g_signal_connect( dbusClient, "dev_detach", G_CALLBACK( _carlife_bt_hfp_detach ), _info );

	}

	return _res;

}		/* -----  end of function mh_dev_register_bt_hfp_request  ----- */

MHResult mh_dev_carlife_send_bt_hfp_indication( MHDev * dev, MHCarlifeBtHfpIndication * indication)
{
	g_return_val_if_fail( dev != NULL && indication !=  NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(usss)", indication->type, indication->phoneNum ? indication->phoneNum : "", 
			indication->name ? indication->name : "", indication->address ? indication->address : "");

	_res	=	mh_object_set_properties((MHObject *)dev, "bt_hfp_indication", _var, NULL);

	return _res;

}

MHResult mh_dev_carlife_send_bt_hfp_connection( MHDev * dev, MHCarlifeBtHfpConnection * connection) 
{
	g_return_val_if_fail( dev != NULL && connection != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(uss)", connection->type, connection->address ? connection->address : "",
			connection->name ? connection->name : "");

	_res	=	mh_object_set_properties((MHObject *)dev, "bt_hfp_connection", _var, NULL);

	return _res;
}
MHResult mh_dev_carlife_send_bt_hfp_response( MHDev * dev, MHCarlifeBtHfpResponse * response)
{
	g_return_val_if_fail( dev != NULL && response != NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GVariant * _var;

	_var	=	g_variant_new("(uuu)", response->status, response->cmd, response->dtmfCode);

	_res	=	mh_object_set_properties((MHObject *)dev, "bt_hfp_response", _var, NULL);

	return _res;

}
