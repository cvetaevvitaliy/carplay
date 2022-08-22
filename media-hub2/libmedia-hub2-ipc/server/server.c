/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/06/2014 03:52:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
//#include <mh_dbus.h>
#include <mh_folder.h>
#include <mh_filter.h>
#include <mh_playlist.h>
#include <mh_pb.h>
#include <mh_dev.h>
#include <mh_col.h>
#include <mh_music.h>
#include <sys/mman.h>
#include <sys/stat.h>        /*  For mode constants */
#include <fcntl.h>           /*  For O_* constants */
#include <stdio.h>
#include <mh_player.h>
#include <arpa/inet.h>
#include "dns_sd.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <mh_core.h>
//#include <debug.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif
#include <gst/gstinfo.h>
typedef struct _MHIPCServer 
{
	GDBusServer * ipc_server;
	GDBusServer * streaming_server;
	MHServerEventsListener * listener;
} MHIPCServer;				/* ----------  end of struct MHIPCServer  ---------- */


typedef void ( * load_engine )( GDBusServer *, GDBusConnection *, gpointer );

GList * engineList;
GList * dbusList;
static void * dnsHandle;
static DNSServiceErrorType (*_DNSServiceRegister)(DNSServiceRef *sdRef,DNSServiceFlags flags,uint32_t interfaceIndex,const char *name,      const char *regtype,
    const char *domain,const char *host,uint16_t port,uint16_t txtLen,const void *txtRecord, DNSServiceRegisterReply  callBack,void *context);

static DNSServiceErrorType (* _DNSServiceProcessResult)(DNSServiceRef sdRef);
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_core_start
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_core_start( Mediahub2Dbus * object, 
		GDBusMethodInvocation * invocation )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_core_start();

	mediahub2_dbus_complete_core_start( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_core_start  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_core_stop
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_core_stop( Mediahub2Dbus * object, 
		GDBusMethodInvocation * invocation )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_core_stop();

	mediahub2_dbus_complete_core_stop( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_core_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_core_find_dev
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_core_find_dev( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,  GVariant *arg_devParam )
{
	MHResult _res	=	MH_OK;
	
	GVariant * _result;

	g_variant_get( arg_devParam, "v", &_result);

	MHDevParam * _param	=	(MHDevParam *)g_malloc0( sizeof( MHDevParam ));

	g_variant_get( _result, "(usu)", &(_param->type), &(_param->mac_addr), &(_param->connect_status));

	_res	=	mh_core_find_dev( _param );

	mediahub2_dbus_complete_core_find_dev( object, invocation, _res );

	g_free( _param->mac_addr );
	g_free( _param );
	g_variant_unref( _result );

	return TRUE;
}		/* -----  end of static function handle_core_find_dev  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_arrived
 *  Description:  
 * =====================================================================================
 */
static void dev_arrived( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
#ifdef  __x86_64__
	mediahub2_dbus_emit_core_devices(( Mediahub2Dbus * )user_data, ( guint64 )dev, ( guint )event );
#else
	mediahub2_dbus_emit_core_devices(( Mediahub2Dbus * )user_data, ( guint )dev, ( guint )event );
#endif
}		/* -----  end of static function dev_arrived  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _detach_arrived
 *  Description:  
 * =====================================================================================
 */
static void _dev_detach_arrived( MHDev * dev, void * user_data)
{
#ifdef  __x86_64__
	mediahub2_dbus_emit_dev_detach(( Mediahub2Dbus * )user_data, (guint64) dev);
#else
	mediahub2_dbus_emit_dev_detach(( Mediahub2Dbus * )user_data, (guint) dev);
#endif
}		/* -----  end of static function _detach_arrived  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_event_arrived
 *  Description:  
 * =====================================================================================
 */
static void _dev_event_arrived(MHDev * self, MHDevScanCbType scan_type, MHItemType item_type, void * data, uint32_t percent, void * user_data)
{
#ifdef  __x86_64__
	mediahub2_dbus_emit_dev_event(( Mediahub2Dbus * )user_data,(guint64) self, (guint)scan_type,
								(guint)item_type, (guint64)data, (guint)percent);
#else
	mediahub2_dbus_emit_dev_event(( Mediahub2Dbus * )user_data,(guint) self, (guint)scan_type,
								(guint)item_type, (guint)data, (guint)percent);
#endif
}		/* -----  end of static function _dev_event_arrived  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _core_events
 *  Description:  
 * =====================================================================================
 */
static void _core_events( MHCore * core, MHCoreEvent event, const char * type, void * user_data )
{
	g_message( "object fired: %p", user_data );

	mediahub2_dbus_emit_core_events(( Mediahub2Dbus * )user_data, ( guint )event, (const gchar *)type );
}		/* -----  end of static function _core_events  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_core_register_events_listener
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_core_register_events_listener( Mediahub2Dbus * object,
		GDBusMethodInvocation * invocation )
{

	MHEventsListener *_eventListener =	g_new0( MHEventsListener, 1);

	_eventListener->callback	=	_core_events;

	_eventListener->user_data	=	object;

	g_message( "object registered: %p", object );

	mh_core_register_events_listener( _eventListener );

	g_free( _eventListener);

	mediahub2_dbus_complete_core_register_events_listener( object, invocation );

	return TRUE;
}		/* -----  end of static function handle_core_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_core_register_devices_listener
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_core_register_devices_listener( Mediahub2Dbus * object, 
		GDBusMethodInvocation * invocation )
{
	Mediahub2Dbus * _dbus;
	GList * _list	=	g_list_first( dbusList);
	while( _list != NULL)
	{
		_dbus	=	(Mediahub2Dbus *)(_list->data);
		if( _dbus == object)
		{
			mediahub2_dbus_complete_core_register_devices_listener( object, invocation );
			return TRUE;
		}
		else
		{
			_list	=	g_list_next( _list );
		}

	}

	MHDevicesListener _devListener  =
	{
		.callback   =   dev_arrived,
		.user_data  =   object
	};

	mh_core_register_devices_listener( &_devListener );

	mediahub2_dbus_complete_core_register_devices_listener( object, invocation );
	dbusList =	g_list_append( dbusList, (gpointer) object);

	return TRUE;
}		/* -----  end of static function handle_core_register_devices_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_get_properties
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_obj_get_properties( Mediahub2Dbus * object, 
		GDBusMethodInvocation * invocation, guint64 arg_obj, const gchar * const * arg_properties )
#else
static gboolean handle_obj_get_properties( Mediahub2Dbus * object, 
		GDBusMethodInvocation * invocation, guint arg_obj, const gchar * const * arg_properties )
#endif
{
	MHResult _res	=	MH_OK;
	g_message("server.c----->handle_obj_get_properties");
	int _offset	=	0;
	GObject * _obj	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;
	GVariantBuilder * _varBuilder	=	g_variant_builder_new( G_VARIANT_TYPE( "av" ));
	GVariant * _retVar;

	if( _obj )
	{
		while( arg_properties[ _offset ] != NULL )
		{
			GValue _val	=	G_VALUE_INIT;
			GParamSpec * _spec;
			GVariant * _var;
			const gchar * _format	=	NULL;
			gpointer _object;
			GVariant * _type_var;

			_spec	=	g_object_class_find_property( G_OBJECT_GET_CLASS( _obj ), arg_properties[ _offset ] );

			if( _spec != NULL )
			{
				g_value_init( &_val, _spec->value_type );

				g_object_get_property( _obj, arg_properties[ _offset ], &_val );

				switch( G_VALUE_TYPE( &_val ))
				{
					case G_TYPE_BOOLEAN:
						_format	=	"b";
						break;
					case G_TYPE_UCHAR:
						_format	=	"y";
						break;
					case G_TYPE_INT:
						_format	=	"i";
						break;
					case G_TYPE_POINTER:
						g_message("G_TYPE_POINTER");
						
						_object	=	g_value_get_pointer( &_val );
						g_value_unset( &_val );
#ifdef __x86_64__
						_format	=	"t";
						g_value_init( &_val, G_TYPE_UINT64);
						g_value_set_uint64( &_val, (guint64)_object);
#else
						_format	=	"u";
						g_value_init( &_val, G_TYPE_UINT );
						g_value_set_uint( &_val, ( guint )_object );
#endif
						break;	
					case G_TYPE_UINT:
						_format	=	"u";
						break;
					case G_TYPE_INT64:
						g_message("G_TYPE_INT64");
						_format	=	"x";
						break;
					case G_TYPE_UINT64:
						g_message("G_TYPE_UINT64");
						_format	=	"t";
						break;
					case G_TYPE_DOUBLE:
						_format	=	"d";
						break;
					case G_TYPE_STRING:
						_format	=	"s";
						break;
					case G_TYPE_VARIANT:
						g_message("server.c:type== G_TYPE_VARIANT");
						_format = "v";
						_type_var	=	g_variant_new( "v",g_value_get_variant( &_val ));
						g_value_unset( &_val);
						g_value_init( &_val, G_TYPE_VARIANT);
						g_value_set_variant(&_val, _type_var);
						break;
					default:
						g_warning( "%s: Un-supported value type", __func__ );
						break;
				}
	
				_var	=	g_dbus_gvalue_to_gvariant( &_val, G_VARIANT_TYPE( _format ));

				g_variant_builder_add( _varBuilder, "v", _var );

				g_value_unset( &_val );
				g_variant_unref( _var );
			}

			_offset	++;
		}
	}
	else
	{
		_res	=	MH_INVALID_PARAM;
	}

	_retVar	=	g_variant_builder_end( _varBuilder );

	g_variant_builder_unref( _varBuilder );

	mediahub2_dbus_complete_obj_get_properties( object, invocation, _retVar, _res );

	return TRUE;
}		/* -----  end of static function handle_obj_get_properties  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_get_properties_type
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_obj_get_properties_type( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj, const gchar * const * arg_properties )
{
	g_message("handle_obj_get_properties_type\n\n");
	GObject * _obj	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;
	int _offset	=	0;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));
	GVariant * _retVar;

	if( _obj )
	{
		while( arg_properties[ _offset ] != NULL )
		{
			GParamSpec * _spec;
			g_message("\n\n\n----------------->arg_properties[%d]:%s\n\n", _offset, arg_properties[_offset]);
			_spec	=	g_object_class_find_property( G_OBJECT_GET_CLASS( _obj ), arg_properties[ _offset ] );

			if( _spec != NULL )
			{
				g_variant_builder_add( _builder, "t", _spec->value_type );
			}

			_offset	++;
		}
	}

	_retVar	=	g_variant_builder_end( _builder );

	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_obj_get_properties_type( object, invocation, _retVar );

	return TRUE;

}
#else
static gboolean handle_obj_get_properties_type( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj, const gchar * const * arg_properties )
{
	GObject * _obj	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;
	int _offset	=	0;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));
	GVariant * _retVar;

	if( _obj )
	{
		while( arg_properties[ _offset ] != NULL )
		{
			GParamSpec * _spec;

			_spec	=	g_object_class_find_property( G_OBJECT_GET_CLASS( _obj ), arg_properties[ _offset ] );

			if( _spec != NULL )
			{
				g_variant_builder_add( _builder, "u", _spec->value_type );
			}

			_offset	++;
		}
	}

	_retVar	=	g_variant_builder_end( _builder );

	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_obj_get_properties_type( object, invocation, _retVar );

	return TRUE;
}		/* -----  end of static function handle_obj_get_properties_type  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_set_properties
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_obj_set_properties( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_obj, GVariant * arg_properties )
#else
static gboolean handle_obj_set_properties( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_obj, GVariant * arg_properties )
#endif
{
	GObject * _object	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;

	MHResult _res	=	MH_OK;

	if( _object )
	{
		GVariantIter * _it;
		gchar * _prop	=	NULL;
		GVariant * _var	=	NULL;
		GValue _value;

		g_variant_get( arg_properties, "a(sv)", &_it );

		while( g_variant_iter_loop( _it, "(sv)", &_prop, &_var ))
		{
			g_dbus_gvariant_to_gvalue( _var, &_value );

			g_object_set_property( _object, _prop, &_value );

			g_value_unset( &_value );
		}
		g_variant_iter_free ( _it );
	}
	else
	{
		_res	=	MH_INVALID_PARAM;
	}
	mediahub2_dbus_complete_obj_set_properties( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_obj_set_properties  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_ref
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__

static gboolean handle_obj_ref( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_obj )

{
	mediahub2_dbus_complete_obj_ref( object, invocation, ( guint64 )mh_object_ref(( MHObject * )arg_obj ));
	return TRUE;
}		/* -----  end of static function handle_obj_ref  ----- */
#else
static gboolean handle_obj_ref( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_obj )

{
	mediahub2_dbus_complete_obj_ref( object, invocation, ( guint )mh_object_ref(( MHObject * )arg_obj ));
	return TRUE;
}		/* -----  end of static function handle_obj_ref  ----- */

#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_unref
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_obj_unref( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_obj )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_object_unref(( MHObject * )arg_obj );

	mediahub2_dbus_complete_obj_unref( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_obj_unref  ----- */

#else
static gboolean handle_obj_unref( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_obj )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_object_unref(( MHObject * )arg_obj );

	mediahub2_dbus_complete_obj_unref( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_obj_unref  ----- */
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_folder_get_children
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_folder_get_children( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
		guint64 arg_obj, guint64 arg_filter, guint arg_position, gint arg_count, guint arg_order )
{
	MHFolder * _folder	=	(MHFolder *) arg_obj ;
	MHFilter * _filter	=	(MHFilter *) arg_filter ;
	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));
	gint _offset	=	0;
	MHItem ** _items;
	if( _folder != NULL )
	{
		_items	=	mh_folder_get_children( _folder, _filter, arg_position, &arg_count, arg_order );
		if( _items != NULL)
		{	
			while( _offset < arg_count )
			{
				g_variant_builder_add( _builder, "t", ( guint64 )_items[ _offset ] );
	
				_offset	++;
			}
			g_free( _items );

		
		}
		else
		{
			*(( gint *)(&arg_count)) = 0;
		}
	}

	_result	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_folder_get_children( object, invocation, arg_count, _result );

	return TRUE;
}		/* -----  end of static function handle_folder_get_children  ----- */

#else
static gboolean handle_folder_get_children( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
		guint arg_obj, guint arg_filter, guint arg_position, gint arg_count, guint arg_order )
{
	MHFolder * _folder	=	(MHFolder *) arg_obj ;
	MHFilter * _filter	=	(MHFilter *) arg_filter ;
	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));
	gint _offset	=	0;
	MHItem ** _items;
	if( _folder != NULL )
	{
		_items	=	mh_folder_get_children( _folder, _filter, arg_position, &arg_count, arg_order );
		if( _items != NULL)
		{	
			while( _offset < arg_count )
			{
				g_variant_builder_add( _builder, "u", ( guint )_items[ _offset ] );
	
				_offset	++;
			}
			g_free( _items );

		
		}
		else
		{
			*(( gint *)(&arg_count)) = 0;
		}
	}

	_result	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_folder_get_children( object, invocation, arg_count, _result );

	return TRUE;
}		/* -----  end of static function handle_folder_get_children  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_folder_create_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_folder_create_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj, guint64 arg_filter, gboolean arg_recursive )
{
	MHFolder * _folder	=	MH_FOLDER( arg_obj );
	MHFilter * _filter	=	MH_FILTER( arg_filter );
	MHPlaylist * _playlist	=	NULL;

	if( _folder != NULL )
	{
		_playlist	=	mh_folder_create_playlist( _folder, _filter, arg_recursive );
	}

	mediahub2_dbus_complete_folder_create_playlist( object, invocation, ( guint64 )_playlist );

	return TRUE;
}		/* -----  end of static function handle_folder_create_playlist  ----- */

#else
static gboolean handle_folder_create_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj, guint arg_filter, gboolean arg_recursive )
{
	MHFolder * _folder	=	MH_FOLDER( arg_obj );
	MHFilter * _filter	=	MH_FILTER( arg_filter );
	MHPlaylist * _playlist	=	NULL;

	if( _folder != NULL )
	{
		_playlist	=	mh_folder_create_playlist( _folder, _filter, arg_recursive );
	}

	mediahub2_dbus_complete_folder_create_playlist( object, invocation, ( guint )_playlist );

	return TRUE;
}		/* -----  end of static function handle_folder_create_playlist  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_foreach
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_foreach( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
 								   guint64 arg_playlist, gint arg_start, gint arg_count)
#else
static gboolean handle_playlist_foreach( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
 								   guint arg_playlist, gint arg_start, gint arg_count)
#endif
{
	MHResult	_res	=	MH_OK;
	MHPlaylist * _playlist	=	MH_PLAYLIST( arg_playlist );
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(susxxuxuv)" ));
	int i;
	GVariant * _data, * _detail;
	guint _size;
	gchar *_shm;
	int _fd;

	if( arg_count < 0 )
		arg_count	=	_playlist->array->len - arg_start ;

	for( i = 0; ( i + arg_start ) < _playlist->array->len && i < arg_count; i ++ )
	{
		MHItemData * _item;

		_item	=	g_array_index( _playlist->array, MHItemData *, i + arg_start );

		switch( _item->type )
		{
		case MH_ITEM_MUSIC:
			_detail	=	g_variant_new( "(sssssiiiii)", _item->metadata.music.title ? _item->metadata.music.title : "",
					_item->metadata.music.album ? _item->metadata.music.album : "",
					_item->metadata.music.artist ? _item->metadata.music.artist : "",
					_item->metadata.music.genre ? _item->metadata.music.genre : "",
					_item->metadata.music.composer ? _item->metadata.music.composer : "",
					_item->metadata.music.year, _item->metadata.music.track, _item->metadata.music.trackCount,
					_item->metadata.music.mediaType, _item->metadata.music.duration );
			break;
		default:
			_detail	=	g_variant_new_string( "" );
			break;
		}

		g_variant_builder_add( _builder, "(susxxuxuv)", _item->uri ? _item->uri : "", _item->type, 
				_item->name ? _item->name : "",	_item->size, _item->uniqueId, _item->valid, _item->tagId,_item->favorite, _detail );
	}

	_data	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_playlist_foreach( object, invocation, _data, _res );

	g_variant_unref( _data );
	return TRUE;
}		/* -----  end of static function handle_playlist_foreach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_foreach
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_foreach_shm( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_playlist, gint arg_start, gint arg_count )
#else
static gboolean handle_playlist_foreach_shm( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_playlist, gint arg_start, gint arg_count )
#endif
{
	MHResult _res	=	MH_OK;

	MHPlaylist * _playlist	=	g_object_ref( MH_PLAYLIST( arg_playlist ) );
	if( _playlist == NULL)
	{
		g_message("-----------------ipcserver.c --------_playlist is NULL!!!!\n");
		_res	=	MH_INVALID_PARAM;
		mediahub2_dbus_complete_playlist_foreach_shm( object, invocation, "", 0, _res );
		return TRUE;
	}
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(susxxuxuv)" ));
	int i;
	GVariant * _data, * _detail;
	gchar * _shm;
	int _fd;
	void * _mem;
	guint _size;
	if( arg_count < 0 )
		arg_count	=	_playlist->array->len - arg_start ;
	
	for( i = 0; ( i + arg_start ) < _playlist->array->len && i < arg_count; i ++ )
	{
		MHItemData * _item;

		_item	=	g_array_index( _playlist->array, MHItemData *, i + arg_start );

		switch( _item->type )
		{
		case MH_ITEM_MUSIC:
			_detail	=	g_variant_new( "(sssssiiiii)", _item->metadata.music.title ? _item->metadata.music.title : "",
					_item->metadata.music.album ? _item->metadata.music.album : "",
					_item->metadata.music.artist ? _item->metadata.music.artist : "",
					_item->metadata.music.genre ? _item->metadata.music.genre : "",
					_item->metadata.music.composer ? _item->metadata.music.composer : "",
					_item->metadata.music.year, _item->metadata.music.track, _item->metadata.music.trackCount,
					_item->metadata.music.mediaType, _item->metadata.music.duration );
			break;
		default:
			_detail	=	g_variant_new_string( "" );
			break;
		}

		g_variant_builder_add( _builder, "(susxxuxuv)", _item->uri ? _item->uri : "", _item->type, 
				_item->name ? _item->name : "",	_item->size, _item->uniqueId, _item->valid, _item->tagId, _item->favorite,_detail );
	}

	_data	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	_size	=	g_variant_get_size( _data );
	if( _size > 0)
	{
		_shm	=	g_strdup_printf( "/%p", _data );

		//	_fd		=	shm_open( _shm, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR );
		_fd	=	shm_open( _shm, O_CREAT | O_RDWR, 00666 );

		if( _fd < 0 )
			perror( __func__ );
		else
		{
			ftruncate( _fd, _size );

			_mem	=	mmap( NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0 );

			if( _mem != MAP_FAILED )
			{
				g_variant_store( _data, _mem );

				mediahub2_dbus_complete_playlist_foreach_shm( object, invocation, _shm, _size, _res );

				munmap( _mem, _size );
			}
			else
			{
				_res	=	MH_MAP_FAILED;
				mediahub2_dbus_complete_playlist_foreach_shm( object, invocation, "", 0, _res );

				shm_unlink( _shm );
			}

			close( _fd );
			g_free( _shm );
		}
	}
	else
	{
		mediahub2_dbus_complete_playlist_foreach_shm( object, invocation, "", 0, _res );
	}
	g_variant_unref( _data );

	g_object_unref( _playlist );

	return TRUE;
}		/* -----  end of static function handle_playlist_foreach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_play_by_list
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_play_by_list( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_obj, guint64 arg_playlist, guint arg_index )
#else
static gboolean handle_pb_play_by_list( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_obj, guint arg_playlist, guint arg_index )
#endif
{
	MHResult _res	=	MH_OK;
	MHPb * _pb	=	MH_PB( arg_obj );
	MHPlaylist * _playlist	=	MH_PLAYLIST( arg_playlist );
	if( _playlist	!=	NULL && MH_IS_PLAYLIST( _playlist) )
	{
		if( arg_index >= _playlist->array->len  )
		{
			_playlist->index = 0;
			_playlist->ptime = 0;
		}
		_res	=	mh_pb_play_by_list( _pb, _playlist, arg_index );
	}else{
		g_message("%s playlist is NULL!\n",__func__);
		_res	=	MH_INVALID_PARAM;
	}

	mediahub2_dbus_complete_pb_play_by_list( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_play_by_list  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_play_radio_by_index
 *  Description:  
 * =====================================================================================
 */
 #ifdef __x86_64__
 static gboolean handle_pb_play_radio_by_index( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
	 guint64 arg_dev, guint64 arg_obj, guint arg_index)
#else
static gboolean handle_pb_play_radio_by_index( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
	guint arg_dev, guint arg_obj, guint arg_index)
#endif
{
	MHPb * _pb	=	MH_PB( arg_obj );
	MHDev * _dev = MH_DEV(arg_dev);
	mh_pb_play_radio_by_index( _dev, _pb, arg_index );
	mediahub2_dbus_complete_pb_play_radio_by_index( object, invocation );
	return TRUE;
}		/* -----  end of static function handle_pb_play_radio_by_index  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_next
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_next( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_next( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_next( _pb );

	mediahub2_dbus_complete_pb_next( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_next  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_previous
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_previous( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_obj )
#else
static gboolean handle_pb_previous( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_previous( _pb );

	mediahub2_dbus_complete_pb_previous( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_previous  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_item_foreach
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_item_foreach( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		GVariant * arg_items )
{
	MHResult _res	=	MH_OK;
	MHItem * _item;
	GVariantIter * _it;
	GVariant * _data, * _detail;
#ifdef __x86_64__
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(tsusxuv)" ));
	g_variant_get( arg_items, "at", &_it );

	while( g_variant_iter_loop( _it, "t", ( guint64 *)&_item ))
	{
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			_detail	=	g_variant_new_string( "" );

			g_variant_builder_add( _builder, "(tsusxuv)", ( guint64 )_item, _item->uri, _item->type, 
					_item->name,_item->uniqueId,_item->favorite, _detail );
		}
	}

#else
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(ususxuv)" ));
	g_variant_get( arg_items, "au", &_it );

	while( g_variant_iter_loop( _it, "u", ( guint *)&_item ))
	{
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			_detail	=	g_variant_new_string( "" );

			g_variant_builder_add( _builder, "(ususxuv)", ( guint )_item, _item->uri, _item->type, 
					_item->name,_item->uniqueId, _item->favorite, _detail );
		}
	}
#endif
	_data	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_item_foreach( object, invocation, _data, _res );

	return TRUE;
}		/* -----  end of static function handle_item_foreach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_item_foreach_fd
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_item_foreach_fd( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		GVariant * arg_items, GVariant * arg_fd )
{
	MHResult _res	=	MH_OK;
	MHItem * _item;
	GVariantIter * _it;
	GVariant * _data, * _detail;
	int _fd;
	int _size;
#ifdef __x86_64__
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(tsusv)" ));

	g_variant_get( arg_items, "at", &_it );

	while( g_variant_iter_loop( _it, "t", ( guint64 *)&_item ))
	{
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			_detail	=	g_variant_new_string( "" );

			g_variant_builder_add( _builder, "(tsusv)", ( guint64 )_item, _item->uri, _item->type, 
					_item->name, _detail );
		}
	}
#else
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(ususv)" ));

	g_variant_get( arg_items, "au", &_it );

	while( g_variant_iter_loop( _it, "u", ( guint *)&_item ))
	{
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			_detail	=	g_variant_new_string( "" );

			g_variant_builder_add( _builder, "(ususv)", ( guint )_item, _item->uri, _item->type, 
					_item->name, _detail );
		}
	}

#endif
	_data	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	g_variant_get( arg_fd, "h", &_fd );

	_size	=	g_variant_get_size( _data );

	g_message( "server: size is %d; fd is %d", _size, _fd );
	g_message( "server: write %zx",  write( _fd, &_size, sizeof( _size )));
	
	write( _fd, g_variant_get_data( _data ), _size );

	close( _fd );

	mediahub2_dbus_complete_item_foreach_fd( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_item_foreach_fd  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_item_foreach_shm
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_item_foreach_shm( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		GVariant * arg_items )
{
	g_message("%s");
	MHResult _res	=	MH_OK;
	MHItem * _item;
	GVariantIter * _it;
	GVariant * _data;
	guint _size;
	gchar * _shm;
	int _fd;
	void * _mem;
	static uint32_t _shm_count = 0;
#ifdef __x86_64__
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(tsusxutv)" ));

	g_variant_get( arg_items, "at", &_it );

	while( g_variant_iter_loop( _it, "t", ( guint64 *)&_item ))
	{
		if( _item	==	NULL)
		{
			break;
		}
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			if( _item->type	==	MH_ITEM_MUSIC && _item->mediaId != 0)
			{
				MHMusic * _music	=	MH_MUSIC( _item);
				g_variant_builder_add( _builder, "(tsusxutv)", (guint64 )_item, _item->uri, _item->type, 
						_item->name, _item->uniqueId, _item->favorite, _item->mediaId, g_variant_new("(sssssuuuuu)",
						_music->title ? _music->title : "", _music->album_title ? _music->album_title : "", _music->artist ? _music->artist : "",
						_music->genre ? _music->genre : "", _music->composer ? _music->composer : "", _music->year,
						_music->track, _music->track_count, _music->mediaType, _music->duration));
						
			}
			else
			{
				g_variant_builder_add( _builder, "(tsusxutv)", ( guint64 )_item, _item->uri, _item->type, 
						_item->name, _item->uniqueId,  _item->favorite, _item->mediaId, g_variant_new_string(" ") );

			}

		}
	}

#else
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(ususxutv)" ));

	g_variant_get( arg_items, "au", &_it );

	while( g_variant_iter_loop( _it, "u", ( guint *)&_item ))
	{
		if( _item	==	NULL)
		{
			break;
		}
		if( MH_IS_ITEM( _item ))
		{
			/* TODO */
			if( _item->type	==	MH_ITEM_MUSIC && _item->uniqueId != 0)
			{
				MHMusic * _music	=	MH_MUSIC( _item);
				g_variant_builder_add( _builder, "(ususxutv)", (guint )_item, _item->uri, _item->type, 
						_item->name, _item->uniqueId, _item->favorite, _item->mediaId, g_variant_new("(sssssuuuuu)",
						_music->title, _music->album_title, _music->artist, _music->genre, _music->composer, _music->year,
						_music->track, _music->track_count, _music->mediaType, _music->duration));

			}
			else
			{

				g_variant_builder_add( _builder, "(ususxutv)", ( guint )_item, _item->uri, _item->type, 
						_item->name, _item->uniqueId, _item->favorite, _item->mediaId,  g_variant_new_string(" ") );

			}
		}
	}
#endif
	_data	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	g_variant_iter_free( _it );

	_size	=	g_variant_get_size( _data );
	_shm	=	g_strdup_printf( "/%p_%d", _data , _shm_count);
	_shm_count++;
//	_shm	=	g_strdup_printf( "/%p", _data );
//	_fd	=	shm_open( _shm, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR );
	_fd	=	shm_open( _shm, O_CREAT | O_RDWR, 00666 );
	if( _fd < 0 )
	{
		perror( "handle_item_foreach_shm" );
		_res	=	MH_SHM_OPEN_FAILED;
		mediahub2_dbus_complete_item_foreach_shm( object, invocation, "", 0, _res );
		return TRUE;
	}

	ftruncate( _fd, _size );

	_mem	=	mmap( NULL, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0 );

	if( _mem != MAP_FAILED )
	{
		g_variant_store( _data, _mem );

		mediahub2_dbus_complete_item_foreach_shm( object, invocation, _shm, _size, _res );

		munmap( _mem, _size );
	}
	else
	{
		_res	=	MH_MAP_FAILED;
		mediahub2_dbus_complete_item_foreach_shm( object, invocation, "", 0, _res );

		shm_unlink( _shm );
	}
	close( _fd );
	g_free( _shm );
	g_variant_unref( _data );

	return TRUE;
}		/* -----  end of static function handle_item_foreach_shm  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_restore_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_restore_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_dev, gint64 arg_playlist_id )
{
	MHDev * _dev	=	MH_DEV( arg_dev );
	MHPlaylist * _playlist	=	NULL;

	_playlist	=	mh_dev_restore_playlist( _dev, arg_playlist_id );

	mediahub2_dbus_complete_dev_restore_playlist( object, invocation, ( guint64 )_playlist );

	return TRUE;
}		/* -----  end of static function handle_dev_restore_playlist  ----- */
#else
static gboolean handle_dev_restore_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_dev, gint64 arg_playlist_id )

{
	MHDev * _dev	=	MH_DEV( arg_dev );
	MHPlaylist * _playlist	=	NULL;

	_playlist	=	mh_dev_restore_playlist( _dev, arg_playlist_id );

	mediahub2_dbus_complete_dev_restore_playlist( object, invocation, ( guint )_playlist );

	return TRUE;
}		/* -----  end of static function handle_dev_restore_playlist  ----- */
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_backward
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_backward( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_backward( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_backward( _pb );

	mediahub2_dbus_complete_pb_backward( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_backward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_backward_done
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_backward_done( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_backward_done( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_backward_done( _pb );

	mediahub2_dbus_complete_pb_backward_done( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_backward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_forward
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_forward( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_forward( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_forward( _pb );

	mediahub2_dbus_complete_pb_forward( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_pb_forward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_forward_done
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_forward_done( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_forward_done( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_forward_done( _pb );

	mediahub2_dbus_complete_pb_forward_done( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_forward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_pause
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_pause( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_pause( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult	_res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_pause( _pb );

	mediahub2_dbus_complete_pb_pause( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_play
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_play( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_play( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_play( _pb );

	mediahub2_dbus_complete_pb_play( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_play_pause
 *  Description:  
 * =====================================================================================
 */
 #ifdef __x86_64__
static gboolean handle_pb_play_pause( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_play_pause( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHPb * _pb	=	MH_PB( arg_obj );

	mh_pb_play_pause( _pb );

	mediahub2_dbus_complete_pb_play_pause( object, invocation );

	return TRUE;
}		/* -----  end of static function handle_pb_play_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_seek
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_seek( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj, guint arg_second )
#else
static gboolean handle_pb_seek( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj, guint arg_second )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_seek( _pb, arg_second );

	mediahub2_dbus_complete_pb_seek( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_seek  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_set_pipeline_status
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_set_pipeline_status( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj, guint arg_status )
#else
static gboolean handle_pb_set_pipeline_status( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj, guint arg_status )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_set_pipeline_status( _pb, arg_status );

	mediahub2_dbus_complete_pb_set_pipeline_status( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_set_pipeline_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_resize
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_resize( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint64 arg_obj, guint arg_offsetx, guint arg_offsety, guint arg_width, guint arg_height )   
#else
static gboolean handle_pb_resize( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint arg_obj, guint arg_offsetx, guint arg_offsety, guint arg_width, guint arg_height )      
#endif
{                                                                                                     
	MHResult _res	=	MH_OK;

	MHPb * _pb  =   MH_PB( arg_obj );                                                                 

	_res	=	mh_pb_resize( _pb, arg_offsetx, arg_offsety, arg_width, arg_height );                             

	mediahub2_dbus_complete_pb_resize( object, invocation, _res );                                          

	return TRUE;                                                                                      
}       /*  -----  end of static function handle_pb_resize  ----- */                                     

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_pixel_aspect_ratio
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_pixel_aspect_ratio( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint64 arg_obj, guint arg_pixel_n, guint arg_pixel_d )   
#else
static gboolean handle_pb_pixel_aspect_ratio( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint arg_obj, guint arg_pixel_n, guint arg_pixel_d )      
#endif
{                                                                                                     
	MHResult _res	=	MH_OK;

	MHPb * _pb  =   MH_PB( arg_obj );                                                                 

	_res	=	mh_pb_pixel_aspect_ratio( _pb, arg_pixel_n, arg_pixel_d );                             

	mediahub2_dbus_complete_pb_pixel_aspect_ratio( object, invocation, _res );                                          

	return TRUE;                                                                                      
}       /*  -----  end of static function handle_pb_pixel_aspect_ratio  ----- */ 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_audiobook_playback_speed
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_audiobook_playback_speed( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint64 arg_obj, guint arg_speed )   
#else
static gboolean handle_pb_audiobook_playback_speed( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,         
		         guint arg_obj, guint arg_speed )      
#endif
{                                                                                                     
	g_message("%s",__func__);
	MHResult _res	=	MH_OK;

	MHPb * _pb  =   MH_PB( arg_obj );                                                                 

	_res	=	mh_pb_audiobook_playback_speed( _pb, arg_speed );                             

	mediahub2_dbus_complete_pb_audiobook_playback_speed( object, invocation, _res );                                          

	return TRUE;                                                                                      
}       /*  -----  end of static function handle_pb_audiobook_playback_speed  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_create
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_pb_create( Mediahub2Dbus * object, GDBusMethodInvocation * invocation )
{
	MHPb * _pb	=	mh_pb_create();
#ifdef __x86_64__
	mediahub2_dbus_complete_pb_create( object, invocation, ( guint64 )_pb );

#else
	mediahub2_dbus_complete_pb_create( object, invocation, ( guint )_pb );
#endif
	return TRUE;
}		/* -----  end of static function handle_pb_create  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_stop
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_stop( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_stop( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_stop( _pb );

	mediahub2_dbus_complete_pb_stop( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_close
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_close( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_obj )
#else
static gboolean handle_pb_close( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_obj )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_pb_close( _pb );

	mediahub2_dbus_complete_pb_close( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_close  ----- */

/*  
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_get_track_info
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_get_track_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_obj)
#else
static gboolean handle_pb_get_track_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_obj)
#endif
{
	MHPbTrackInfo * _info;

	GVariant * _res, * _data_v;

	GVariant * _ret_data;

	GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(s)") );

	_info	=	mh_player_get_track_info( MH_PB( arg_obj )->mh_player );

	uint32_t _i = 0;

	if( _info	!=	NULL)
	{
		while( _i < _info->total_count  )
		{
			g_variant_builder_add( _builder, "(s)", _info->track_name[_i]);

			g_free( _info->track_name[_i] );

			_i++;
		}
		_res	=	g_variant_builder_end( _builder);

		g_variant_builder_unref( _builder );

		_data_v =  g_variant_new("(uiv)",(uint32_t)_info->total_count, (int32_t)_info->current_count, _res);

		_ret_data   =  g_variant_new("v",_data_v);

		g_free( _info );
	}

	mediahub2_dbus_complete_pb_get_track_info( object, invocation, _ret_data);

	return TRUE;
}		/*  -----  end of static function handle_pb_get_track_info  ----- */

/*  
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_get_subtitle_info
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_get_subtitle_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_obj)
#else
static gboolean handle_pb_get_subtitle_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_obj)
#endif
{
	MHPbSubtitleInfo * _info;

	GVariant * _res, * _data_v;

	GVariant * _ret_data;

	GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(s)") );

	_info	=	mh_player_get_subtitle_info( MH_PB( arg_obj )->mh_player );

	uint32_t _i = 0;

	if( _info	!=	NULL)
	{
		while( _i < _info->total_count  )
		{
			g_variant_builder_add( _builder, "(s)", _info->subtitle_name[_i]);

			g_free( _info->subtitle_name[_i] );

			_i++;
		}
		_res	=	g_variant_builder_end( _builder);

		g_variant_builder_unref( _builder );

		_data_v =  g_variant_new("(uiv)",(uint32_t)_info->total_count, (int32_t)_info->current_count, _res);

		_ret_data   =  g_variant_new("v",_data_v);

		g_free( _info );
	}

	mediahub2_dbus_complete_pb_get_subtitle_info( object, invocation, _ret_data);

	return TRUE;
}		/*  -----  end of static function handle_pb_get_subtitle_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _pb_events
 *  Description:  
 * =====================================================================================
 */
static void _pb_events( MHPb * pb, MHPbInfoType type, MHPbInfoData * data, gpointer user_data )
{
	GVariant * _data	=	NULL;

	if(NULL == data)
	{
		g_message("ipcserver.c : _playback_events data is NULL[%d]\n", type);
		_data	=	g_variant_new_string( "" );
		mediahub2_dbus_emit_pb_events(( Mediahub2Dbus * )user_data, ( guint )pb, type, g_variant_new_variant( _data ));
		return;
	}

	switch( type )
	{
	case MH_PB_INFO_PTIME_CHANGE:
		_data	=	g_variant_new( "(uu)", data->time_info.current_time, data->time_info.duration );
		break;
	case MH_PB_INFO_TAG:
		_data	=	g_variant_new( "(ssss)", data->tag_info.title, data->tag_info.artist, 
				data->tag_info.album, data->tag_info.genre);
		break;
	case MH_PB_INFO_EOS:
		_data	=	g_variant_new_string( "" );
		break;
	case MH_PB_INFO_ERROR:
		_data	=	g_variant_new_string( "" );
		break;
	case MH_PB_INFO_TRACK_TOP:
		_data	=	g_variant_new( "(uss)", data->track_info.index, data->track_info.uri,
				data->track_info.name );
		break;
	case MH_PB_INFO_PLAYLIST_CHANGE:
#ifdef __x86_64__
		_data	=	g_variant_new( "t", (guint64)data->playlist );
#else
		_data	=	g_variant_new( "u", (guint)data->playlist );
#endif
		break;
	case MH_PB_INFO_ERROR_NOT_EXIST:
		_data	=	g_variant_new_string( "" );
		break;
	case MH_PB_IP_INFO_PTIME_CHANGE: 
		_data	=	g_variant_new( "u", data->ptime );
		break;
	case MH_PB_IP_INFO_QUEUE_INDEX:
		_data	=	g_variant_new( "u", data->index );
		break;
	case MH_PB_IP_INFO_MEDIA:
		_data	=	g_variant_new( "(suusuuuussssx)", data->media_info.title, data->media_info.rating, data->media_info.duration,
				data->media_info.album_title, data->media_info.track, data->media_info.track_count, data->media_info.disc,
				data->media_info.disc_count, data->media_info.artist, data->media_info.album_artist, data->media_info.genre,
				data->media_info.composer, data->media_info.tagId );
		break;	
	case MH_PB_IP_INFO_REPEAT_MODE:
		_data	=	g_variant_new( "u", data->repeat_mode );
		break;
	case MH_PB_IP_INFO_SHUFFLE_MODE:
		_data	=	g_variant_new( "u", data->shuffle_mode );
		break;
	case MH_PB_IP_INFO_COVER_PATH:
		_data	=	g_variant_new( "s", data->cover_path );
		break;
	case MH_PB_IP_INFO_APP_NAME:
		_data	=	g_variant_new( "s", data->app_name );
		break;
	case MH_PB_IP_INFO_DEVICE_NAME:
		_data	=	g_variant_new( "s", data->device_name );
		break;
	case MH_PB_IP_INFO_FUNC_UNSUPPORT:
		_data	=	g_variant_new_string( "" );
		break;
	case MH_PB_INFO_STATE_ERROR:
		_data	=	g_variant_new_string( "" );
		break;
	case MH_PB_IP_INFO_SHUFFLE_LIST:
		{
			GVariant * _res;
			uint32_t _i = 0;

			GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(u)") );

			while( _i < data->sf_list_info.list_count  )
			{
				g_variant_builder_add( _builder, "(u)", data->sf_list_info.shuffle_seq[_i]);
				_i++;
			}
			_res	=	g_variant_builder_end( _builder);

			g_variant_builder_unref( _builder );

			_data =  g_variant_new("(uv)",(uint32_t)data->sf_list_info.list_count, _res);
		}
		break;
	case MH_PB_IP_INFO_CALL_STATE_UPDATE:
		_data	=	g_variant_new( "(ssuusssuusu)", data->call_state_info.remoteID, data->call_state_info.displayName, data->call_state_info.status,
				data->call_state_info.direction, data->call_state_info.callUUID, data->call_state_info.addressBookID, data->call_state_info.label,
				data->call_state_info.service, data->call_state_info.isConferenced, data->call_state_info.conferenceGroup, 
				data->call_state_info.disconnectReason );
		break;	
	case MH_PB_IP_INFO_RECENTS_LIST_UPDATES:
		{
			GVariant * _res;
			uint32_t _i = 0;
			GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(ussssuutuu)"));

			while( _i < data->recentslist_updates.recentsListCount  )
			{
				g_variant_builder_add( _builder, "(ussssuutuu)", data->recentslist_updates.recentList[_i].index,
																 data->recentslist_updates.recentList[_i].remoteID,	
																 data->recentslist_updates.recentList[_i].displayName,
																 data->recentslist_updates.recentList[_i].label, 
																 data->recentslist_updates.recentList[_i].addressBookID,
																 data->recentslist_updates.recentList[_i].service, 
																 data->recentslist_updates.recentList[_i].type,
																 data->recentslist_updates.recentList[_i].unixTimestamp, 
																 data->recentslist_updates.recentList[_i].duration,
																 data->recentslist_updates.recentList[_i].occurrences);
				_i++;
			}
			_res	=	g_variant_builder_end( _builder);

			g_variant_builder_unref( _builder );

			_data =  g_variant_new("(uuv)",(uint32_t)data->recentslist_updates.recentsListAvailable,
										   (uint32_t)data->recentslist_updates.recentsListCount, _res);
		}
		break;
	case MH_PB_IP_INFO_FAVORITES_LIST_UPDATES:
		{
			GVariant * _res;
			uint32_t _i = 0;
			GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(ussssu)") );

			while( _i < data->favoriteslist_updates.favoritesListCount  )
			{
				g_variant_builder_add( _builder, "(ussssu)", data->favoriteslist_updates.favoritesList[_i].index,
															 data->favoriteslist_updates.favoritesList[_i].remoteID,	
															 data->favoriteslist_updates.favoritesList[_i].displayName,
															 data->favoriteslist_updates.favoritesList[_i].label, 
															 data->favoriteslist_updates.favoritesList[_i].addressBookID,
															 data->favoriteslist_updates.favoritesList[_i].service);
				_i++;
			}
			_res	=	g_variant_builder_end( _builder);

			g_variant_builder_unref( _builder );

			_data =  g_variant_new("(uuv)",(uint32_t)data->favoriteslist_updates.favoritesListAvailable,
										   (uint32_t)data->favoriteslist_updates.favoritesListCount, _res);
		}
		break;
	case MH_PB_IP_INFO_PLAYBACK_SPEED:
		_data	=	g_variant_new( "u", data->speed );
		break;
//	case MH_PB_FREQUENCY_ANALYSIS_RESULT:
//		{
//			GVariant * _res;
//			uint32_t _i = 0;
//
//			GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(dd)") );
//
//			while( _i < data->frequency_analysis_result.band  )
//			{
//				g_variant_builder_add( _builder, "(dd)", (gdouble)data->frequency_analysis_result.bands[_i],
//						(gdouble)data->frequency_analysis_result.amplitudes[_i]);
//				_i++;
//			}
////			_i= 0;
////			while( _i < data->frequency_analysis_result.band  )
////			{
////				g_variant_builder_add( _builder, "(d)", data->frequency_analysis_result.amplitudes[_i]);
////				_i++;
////			}
//			_res	=	g_variant_builder_end( _builder);
//
//			g_variant_builder_unref( _builder );
//
//			_data =  g_variant_new("(uv)",(uint32_t)data->frequency_analysis_result.band, _res);
//		}
//		break;
	default:
		g_warning( "Unknown MHPbInfoType [ %d ]", type );
		break;
	}

	if( _data != NULL ) 
	{
#ifdef __x86_64__
		mediahub2_dbus_emit_pb_events(( Mediahub2Dbus * )user_data, ( guint64 )pb, type, g_variant_new_variant( _data )); 
#else
		mediahub2_dbus_emit_pb_events(( Mediahub2Dbus * )user_data, ( guint )pb, type, g_variant_new_variant( _data )); 
#endif
	}
}		/* -----  end of static function _pb_events  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _status_update
 *  Description:  
 * =====================================================================================
 */
static void _status_update( MHPb * pb, MHPbStatusType type, void * user_data )
{
#ifdef __x86_64__

	mediahub2_dbus_emit_status_update(( Mediahub2Dbus * )user_data, ( guint64 )pb, type); 
#else
	mediahub2_dbus_emit_status_update(( Mediahub2Dbus * )user_data, ( guint )pb, type); 
#endif
}		/* -----  end of static function _status_update  ----- */


/* 
 *===  FUNCTION  ======================================================================
 *         Name:  handle_pb_register_events_listener
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_register_events_listener( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_self )
#else
static gboolean handle_pb_register_events_listener( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_self )
#endif
{
	MHResult _res	=	MH_OK;

	MHPbEventsListener _eventListener	=	
	{
		.callback	=	_pb_events,
		.user_data	=	object
	};
	MHPb * _pb	=	MH_PB( arg_self );

	_res	=	mh_pb_register_events_listener( _pb, &_eventListener );

	mediahub2_dbus_complete_pb_register_events_listener( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_register_status_listener
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_register_status_listener( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint64 arg_self )
#else
static gboolean handle_pb_register_status_listener( Mediahub2Dbus * object, GDBusMethodInvocation * invocation, 
		guint arg_self )
#endif
{
	MHResult _res	=	MH_OK;

	MHPbStatusListener _eventListener	=	
	{
		.callback	=	_status_update,
		.user_data	=	object
	};
	MHPb * _pb	=	MH_PB( arg_self );
	
	_res	=	mh_pb_register_status_listener( _pb, &_eventListener );

	mediahub2_dbus_complete_pb_register_status_listener( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_pb_playlist_change
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_pb_playlist_change( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_pb, guint64 arg_playlist )
#else
static gboolean handle_pb_playlist_change( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_pb, guint arg_playlist )
#endif

{
	MHResult _res	=	MH_OK;
	MHPb * _pb	=	MH_PB( arg_pb );

	MHPlaylist * _playlist	=	MH_PLAYLIST( arg_playlist );

	_res	=	mh_pb_playlist_change( _pb, _playlist );

	mediahub2_dbus_complete_pb_playlist_change( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_pb_playlist_change  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_start_scan
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_start_scan( Mediahub2Dbus *object,
  									   GDBusMethodInvocation *invocation,
								       guint64 arg_obj,
   									   guint arg_type )

#else
static gboolean handle_dev_start_scan( Mediahub2Dbus *object,
  									   GDBusMethodInvocation *invocation,
								       guint arg_obj,
   									   guint arg_type )
#endif
{
	MHResult _res	=	MH_OK;
	MHDev * _dev	=	MH_DEV( arg_obj );
	MHDevScanType _type	=	( MHDevScanType )arg_type;

	_res	=	mh_dev_start_scan( _dev, _type);

	mediahub2_dbus_complete_dev_start_scan( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_dev_start_scan  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_filter_create
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_filter_create(  Mediahub2Dbus *object,
								       GDBusMethodInvocation *invocation,
								       const gchar *arg_filter_str )
{
	MHFilter * _filter;
	_filter	=	mh_filter_create( arg_filter_str );
#ifdef __x86_64__
	mediahub2_dbus_complete_filter_create ( object, invocation,(guint64) _filter);
#else
	mediahub2_dbus_complete_filter_create ( object, invocation,(guint) _filter);
#endif

	return TRUE;
}		/* -----  end of static function handle_filter_create  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_register_detach_listener
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_register_detach_listener( Mediahub2Dbus *object,
  											     GDBusMethodInvocation *invocation,
											     guint64 arg_dev)
#else
static gboolean handle_dev_register_detach_listener( Mediahub2Dbus *object,
  											     GDBusMethodInvocation *invocation,
											     guint arg_dev)
#endif
{
	MHDev * _dev	=	MH_DEV(arg_dev);

	MHDevDetachListener _detach_listener	=	
	{
		.callback	=	_dev_detach_arrived,
		.user_data	=	object
	};
	g_message( "object registered: %p", object );
	mh_dev_register_detach_listener( _dev, &_detach_listener );

	mediahub2_dbus_complete_dev_register_events_listener( object, invocation );

	return TRUE;
}		/* -----  end of static function handle_dev_register_detach_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_register_events_listener
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_register_events_listener( Mediahub2Dbus *object,
  												    GDBusMethodInvocation *invocation,
 												    guint64 arg_dev)
#else
static gboolean handle_dev_register_events_listener( Mediahub2Dbus *object,
  												    GDBusMethodInvocation *invocation,
 												    guint arg_dev)
#endif
{
	MHDev * _dev	=	MH_DEV(arg_dev);

	MHResult	_res	=	MH_OK;
	MHDevEventsListener _event_listener	=	
	{
		.callback	=	_dev_event_arrived,
		.user_data	=	object
	};
	g_message( "object registered: %p", object );
	_res	=	mh_dev_register_events_listener( _dev, &_event_listener );

	mediahub2_dbus_complete_dev_register_events_listener( object, invocation );

	return TRUE;
}		/* -----  end of function handle_dev_register_events_listener  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_shm_unlink
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_shm_unlink( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
		const gchar *arg_name )
{
	shm_unlink( arg_name );

	mediahub2_dbus_complete_shm_unlink( object, invocation );

	return TRUE;
}		/* -----  end of static function handle_shm_unlink  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_folder_seek
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_folder_seek( Mediahub2Dbus *object,
							    GDBusMethodInvocation *invocation,
 						   	    guint64 arg_folder,
							    guint64 arg_filter,
							    guint arg_pos,
							    guint arg_order)

#else
static gboolean handle_folder_seek( Mediahub2Dbus *object,
							    GDBusMethodInvocation *invocation,
 						   	    guint arg_folder,
							    guint arg_filter,
							    guint arg_pos,
							    guint arg_order)
#endif
{
	MHResult	_res	=	MH_OK;
	MHFolder * _folder	=	MH_FOLDER( arg_folder );
	MHFilter * _filter	=	MH_FILTER( arg_filter );

	_res	=	mh_folder_seek( _folder, _filter, arg_pos, arg_order);
	
	mediahub2_dbus_complete_folder_seek( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_folder_seek  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_create
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_create( Mediahub2Dbus * object,
								GDBusMethodInvocation * invocation,
								guint64 arg_dev) 
{
	MHDev * _dev	=	MH_DEV( arg_dev );
	MHCol * _col	=	mh_col_create( _dev );
	mediahub2_dbus_complete_col_create(object, invocation, (guint64)_col);
	return TRUE;

}
#else
static gboolean handle_col_create( Mediahub2Dbus * object,
								GDBusMethodInvocation * invocation,
								guint arg_dev) 
{
	MHDev * _dev	=	MH_DEV( arg_dev );
	MHCol * _col	=	mh_col_create( _dev );
	mediahub2_dbus_complete_col_create(object, invocation, (guint)_col);
	return TRUE;
}		/* -----  end of static function handle_col_create  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_retrieve_data
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_retrieve_data( Mediahub2Dbus * object,GDBusMethodInvocation *invocation,
		  guint64 arg_col, guint arg_type, guint arg_media_type, gint arg_count, gboolean arg_fuzzy)
#else
static gboolean handle_col_retrieve_data( Mediahub2Dbus * object,GDBusMethodInvocation *invocation,
		  guint arg_col, guint arg_type, guint arg_media_type, gint arg_count, gboolean arg_fuzzy)
#endif
{
	MHCol * _col	=	MH_COL( arg_col );

	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(s)"));
	gint _offset	=	0;

	char ** _data	=	mh_col_retrieve_data( _col, arg_type, arg_media_type,&arg_count, arg_fuzzy);
	while( _offset < arg_count )
	{
		g_variant_builder_add( _builder, "(s)", _data[_offset]);
		g_free( _data[_offset] );	
		_offset ++;
	}
	g_free( _data );
	_result	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	mediahub2_dbus_complete_col_retrieve_data(object, invocation, arg_count, _result);

	return TRUE;
}		/* -----  end of static function handle_col_retrieve_data  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_set_filter
 *  Description:  
 * =====================================================================================
 */
//static gboolean handle_col_set_filter( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
//	 guint arg_col, guint arg_type, guint arg_media_type, GVariant *arg_properties,guint arg_param_count)
//{
//	MHCol * _col	=	MH_COL( arg_col );
//	uint32_t _num = 0;
//	if( _col )
//	{
//		GVariantIter * _it;
//		uint32_t _type[ arg_param_count/2];
//		gchar * _value[ arg_param_count/2];
//		uint32_t _var_type;
//		gchar * _var_value;
//		g_variant_get( arg_properties, "a(us)", &_it);
//		while( g_variant_iter_loop( _it, "(us)", &_var_type,&_var_value))
//		{
//			_type[ _num ]= _var_type;
//			_value[ _num ] = g_strdup(_var_value);
//
//			_num++;
//		}
//		switch (arg_param_count/2)
//		{
//			case 1:
//				mh_col_set_filter( _col, ( MHItemType )arg_type, ( MHMediaType )arg_media_type, _type[0], _value[0], NULL);
//				break; 
//			case 2:
//				mh_col_set_filter( _col, ( MHItemType )arg_type, ( MHMediaType )arg_media_type, _type[0], _value[0], _type[1], _value[1], NULL);
//				break;
//			case 3:
//				mh_col_set_filter( _col, ( MHItemType )arg_type, ( MHMediaType )arg_media_type, _type[0], _value[0], _type[1], _value[1] ,_type[2], _value[2],NULL);
//				break;
//			default:
//				break;
//		}
//
//	}
//	mediahub2_dbus_complete_col_set_filter( object, invocation);
//	return TRUE;
//}		/* -----  end of static function handle_col_set_filter  ----- */
//
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_create_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_create_playlist (  Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_col,
    guint arg_type,
    guint arg_media_type,
	gboolean arg_fuzzy
  )
{
	MHCol * _col	=	MH_COL( arg_col );
	MHPlaylist * _playlist;
	uint32_t _num = 0;
	if( _col )
	{
		_playlist	=	mh_col_create_playlist( _col, arg_type, arg_media_type, arg_fuzzy);
	}
	mediahub2_dbus_complete_col_create_playlist(object, invocation, (guint64)_playlist);
	return TRUE;

}		/* -----  end of static function handle_col_create_playlist  ----- */

#else
static gboolean handle_col_create_playlist (  Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_col,
    guint arg_type,
    guint arg_media_type,
	gboolean arg_fuzzy
  )
{
	MHCol * _col	=	MH_COL( arg_col );
	MHPlaylist * _playlist;
	uint32_t _num = 0;
	if( _col )
	{
		_playlist	=	mh_col_create_playlist( _col, arg_type, arg_media_type, arg_fuzzy);
	}
	mediahub2_dbus_complete_col_create_playlist(object, invocation, (guint)_playlist);
	return TRUE;

}		/* -----  end of static function handle_col_create_playlist  ----- */
#endif


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_get_playlist_info
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_get_playlist_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_dev)
#else
static gboolean handle_dev_get_playlist_info(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_dev)
#endif
{
	MHPlaylistInfo * _info;
	GVariant * _res;
	GVariantBuilder	* _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(xs)") );
	_info	=	mh_dev_get_playlist_info( MH_DEV( arg_dev ) );
	int i = 0;
	if( _info	!=	NULL)
	{
		while( _info[i].playlistid != 0)
		{
			g_variant_builder_add( _builder, "(xs)", _info[i].playlistid, _info[i].name);
			g_free( _info[i].name);
			i++;
		}
		g_free( _info);

	}
	_res	=	g_variant_builder_end( _builder);
	g_variant_builder_unref( _builder );

	mediahub2_dbus_complete_dev_get_playlist_info( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_dev_get_playlist_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_set_filter
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_set_filter( Mediahub2Dbus *object, GDBusMethodInvocation *invocation, GVariant *arg_type_filter)
{
	MHResult _res	=	MH_OK;
	GVariantIter * _it;
	int _i	=	0;
	int _size	=	g_variant_n_children( arg_type_filter);
	int _type[ _size ];
	char * _filter[ _size ];
	char * _p;
	int _j	=	0;

	g_variant_get( arg_type_filter, "a(us)", &_it);

	while( g_variant_iter_loop( _it, "(us)", &_type[_i],&_p))
	{
		_filter[_i] = g_strdup( _p );
		_i++;	
	}

	g_variant_iter_free ( _it );

	g_variant_unref( arg_type_filter );

	switch (_size)
	{
		case 1:
			_res	=	mh_misc_set_filter(_type[0], _filter[0], NULL);
			break;
		case 2:
			_res	=	mh_misc_set_filter( _type[0], _filter[0], _type[1], _filter[1], NULL);
			break;
		case 3:
			_res	=	mh_misc_set_filter( _type[0], _filter[0], _type[1], _filter[1], _type[2], _filter[2], NULL);
			break;
		case 4:
			_res	=	mh_misc_set_filter( _type[0], _filter[0], _type[1], _filter[1], _type[2], _filter[2], _type[3], _filter[3], NULL);
			break;
		default:
			g_message("%s:default",__func__);
			break;
	}

	for( _j	; _j < _i; _j++ )
	{
		g_free( _filter[_j]);
	}
	mediahub2_dbus_complete_misc_set_filter( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_misc_set_filter  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_save_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_save_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint64 arg_dev, const gchar *arg_name, guint64 arg_playlist)
#else
static gboolean handle_dev_save_playlist( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		guint arg_dev, const gchar *arg_name, guint arg_playlist)
#endif
{	
	MHResult	_res	=	MH_OK;
	MHDev * _dev	=	MH_DEV( arg_dev );
	MHPlaylist * _playlist	=	MH_PLAYLIST( arg_playlist );

	_res	=	mh_dev_save_playlist( _dev, arg_name, _playlist );
	mediahub2_dbus_complete_dev_save_playlist( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_dev_save_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_update_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_update_playlist( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint64 arg_dev, const gchar *arg_name, guint64 arg_playlist,gint64 arg_playlist_id)
#else
static gboolean handle_dev_update_playlist( Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint arg_dev, const gchar *arg_name, guint arg_playlist,gint64 arg_playlist_id)
#endif
{
	MHResult _res	=	MH_OK;
	_res	=	mh_dev_update_playlist( MH_DEV( arg_dev ), arg_name, MH_PLAYLIST( arg_playlist), arg_playlist_id);
	mediahub2_dbus_complete_dev_update_playlist(object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_dev_update_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_set_bluetoothids
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_set_bluetoothids(Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_uri)
{
	MHResult _res	=	MH_OK;

	_res	=	mh_misc_set_bluetoothids( arg_uri );

	mediahub2_dbus_complete_misc_set_bluetoothids(object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_misc_set_bluetoothids  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_set_righthand
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_set_righthand(Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint status )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_misc_set_righthand( status );

	mediahub2_dbus_complete_misc_set_righthand(object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_misc_set_righthand  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_delete_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_delete_playlist(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
				    guint64 arg_dev, gint64 arg_playlist_id)
#else
static gboolean handle_dev_delete_playlist(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
				    guint arg_dev, gint64 arg_playlist_id)
#endif
{
	MHResult _res	=	MH_OK;

	_res	=	mh_dev_delete_playlist( MH_DEV( arg_dev ), arg_playlist_id);
	mediahub2_dbus_complete_dev_delete_playlist(object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_dev_delete_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_get_radiolist
 *  Description:  
 * =====================================================================================
 */
 #ifdef __x86_64__
 static gboolean handle_dev_get_radiolist(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_dev,gint arg_count)
#else
static gboolean handle_dev_get_radiolist(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_dev,gint arg_count)
#endif
{
	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(s)"));
	gint _offset	=	0;

	char ** _data	=	mh_dev_get_radiolist(  MH_DEV( arg_dev ), &arg_count);
	while( _offset < arg_count )
	{
		g_variant_builder_add( _builder, "(s)", _data[_offset]);
		g_free( _data[_offset] );	
		_offset ++;
	}
	g_free( _data );
	_result	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	mediahub2_dbus_complete_dev_get_radiolist(object, invocation, arg_count, _result);

	return TRUE;
}		/* -----  end of static function handle_dev_get_radiolist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_attach_pb
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_attach_pb(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
				    guint64 arg_dev, guint64 arg_pb)
#else
static gboolean handle_dev_attach_pb(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
				    guint arg_dev, guint arg_pb)
#endif
{
	MHResult _res	=	MH_OK;
	_res	=	mh_dev_attach_pb( MH_DEV( arg_dev ), MH_PB( arg_pb ));
	mediahub2_dbus_complete_dev_attach_pb(object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_dev_attach_pb  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_reset
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_reset( Mediahub2Dbus *object, GDBusMethodInvocation *invocation )
{
	MHResult _res	=	MH_OK;
	_res	=	mh_misc_reset();
	mediahub2_dbus_complete_misc_reset( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_misc_reset  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_sort
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_sort(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint64 arg_playlist, guint arg_sort_type, guint arg_order_type)
#else
static gboolean handle_playlist_sort(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint arg_playlist, guint arg_sort_type, guint arg_order_type)
#endif
{
	MHResult _res	=	MH_OK;
	MHPlaylist * _playlist	=	MH_PLAYLIST( arg_playlist );

	_res	=	mh_playlist_sort( _playlist, arg_sort_type, arg_order_type);

	mediahub2_dbus_complete_playlist_sort( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_playlist_sort  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_scan_abort
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_scan_abort( Mediahub2Dbus *object, 
		GDBusMethodInvocation *invocation, guint64 arg_dev)
#else
static gboolean handle_dev_scan_abort( Mediahub2Dbus *object, 
		GDBusMethodInvocation *invocation, guint arg_dev)
#endif
{
	MHResult _res	=	MH_OK;
	MHDev * _dev	=	MH_DEV( arg_dev );

	_res	=	mh_dev_scan_abort( _dev);

	mediahub2_dbus_complete_dev_scan_abort( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_dev_scan_abort  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_db_restore
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_db_restore (Mediahub2Dbus *object, GDBusMethodInvocation *invocation)
{
	MHResult _res	=	MH_OK;
	_res	=	mh_misc_db_restore();
	mediahub2_dbus_complete_misc_db_restore( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_misc_db_restore  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_db_delete_by_serial_number
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_db_delete_by_serial_number (Mediahub2Dbus *object, GDBusMethodInvocation *invocation, const gchar *arg_serialNum)//del db by deviceId
{
	mh_misc_db_delete_by_serial_number(arg_serialNum);
	mediahub2_dbus_complete_misc_db_delete_by_serial_number( object, invocation);
	return TRUE;
}		/* -----  end of static function handle_misc_db_delete_by_serial_number  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_add_filter
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_add_filter(  Mediahub2Dbus *object,
  									    GDBusMethodInvocation *invocation,
 									    guint64 arg_col,
 										GVariant *arg_filter)
#else
static gboolean handle_col_add_filter(  Mediahub2Dbus *object,
  									    GDBusMethodInvocation *invocation,
 									    guint arg_col,
 										GVariant *arg_filter)
#endif
{
	MHResult _res	=	MH_OK;

	uint32_t _type;

 	GVariant *_var;
	GVariantIter * _it;

	MHCol * _col	=	(MHCol *) arg_col;

	g_variant_get( arg_filter, "a(uv)", & _it);

	while( g_variant_iter_loop( _it, "(uv)", &_type, &_var))
	{
		switch( _type)
		{
			case COL_FILTER_NAME:
			case COL_FILTER_TITLE:
			case COL_FILTER_ARTIST:
			case COL_FILTER_COMPOSER:
			case COL_FILTER_GENRE:
			case COL_FILTER_ALBUM_ARTIST:
				{
					gchar * _value;
					g_variant_get( _var, "s", &_value);
					mh_col_add_filter( MH_COL(arg_col), _type, _value, NULL);
				}
				break;
			case COL_FILTER_ALBUM:
				{
					MHAlbumInfo * _value	=	(MHAlbumInfo *)g_malloc0( sizeof( MHAlbumInfo ));
					g_variant_get( _var, "(ssu)", &(_value->album_title), &(_value->album_artist), &(_value->album_compliation));
					mh_col_add_filter( MH_COL(arg_col), _type, _value, NULL);
					g_free( _value );
				}
				break;
			case COL_FILTER_COMPLIATION:
			case COL_FILTER_TRACK:
			case COL_FILTER_YEAR:
			case COL_FILTER_DURATION:
				{
					uint32_t _value;
					g_variant_get( _var, "u", &_value);
					mh_col_add_filter( MH_COL(arg_col), _type, _value, NULL);
				}
				break;
		}

	}
	g_variant_iter_free ( _it );


	mediahub2_dbus_complete_col_add_filter( object, invocation, _res);

	return TRUE;

}		/* -----  end of static function handle_col_add_filter  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_filter_clear
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_filter_clear(  Mediahub2Dbus *object,
  										  GDBusMethodInvocation *invocation,
 										  guint64 arg_col )
#else
static gboolean handle_col_filter_clear(  Mediahub2Dbus *object,
  										  GDBusMethodInvocation *invocation,
 										  guint arg_col )
#endif
{
	MHResult _res	=	MH_OK;
	_res	=	mh_col_filter_clear( (MHCol *) arg_col);
	mediahub2_dbus_complete_col_filter_clear(object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_col_filter_clear  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_set_retrieve_key
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_set_retrieve_key( Mediahub2Dbus *object,
  										 GDBusMethodInvocation *invocation,
   										 guint64 arg_col,
  										 guint arg_type)

#else
static gboolean handle_col_set_retrieve_key( Mediahub2Dbus *object,
  										 GDBusMethodInvocation *invocation,
   										 guint arg_col,
  										 guint arg_type)
#endif
{
	MHResult _res	=	MH_OK;
	_res	=	mh_col_set_retrieve_key( (MHCol*)arg_col, arg_type );
	mediahub2_dbus_complete_col_set_retrieve_key(object, invocation, _res);
	return TRUE;

}		/* -----  end of static function handle_col_set_retrieve_key  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_set_order_type
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_set_order_type( Mediahub2Dbus *object,
 										   GDBusMethodInvocation *invocation,
   										   guint64 arg_col,
										   guint arg_type)
#else
static gboolean handle_col_set_order_type( Mediahub2Dbus *object,
 										   GDBusMethodInvocation *invocation,
   										   guint arg_col,
										   guint arg_type)
#endif
{
	MHResult _res	=	MH_OK;
	_res	=	mh_col_set_order_type( (MHCol*)arg_col, arg_type );
	mediahub2_dbus_complete_col_set_order_type(object, invocation, _res);
	return TRUE;

}		/* -----  end of static function handle_col_set_order_type  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_request_app_launch
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_request_app_launch( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint64 arg_dev,
		const gchar *arg_app_bundle_id)
#else
static gboolean handle_dev_request_app_launch( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint arg_dev,
		const gchar *arg_app_bundle_id)
#endif
{
	MHDev * _dev	=	MH_DEV( arg_dev );

	mh_dev_request_app_launch( _dev, arg_app_bundle_id );

	mediahub2_dbus_complete_dev_request_app_launch(object, invocation);

	return TRUE;
}		/* -----  end of static function handle_dev_request_app_launch  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_write_location_data
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_write_location_data( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint64 arg_dev,
		const gchar *data)
#else
static gboolean handle_dev_write_location_data( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint arg_dev,
		const gchar *data)
#endif
{
	MHDev * _dev	=	MH_DEV( arg_dev );

	mh_dev_write_location_data( _dev, data );

	mediahub2_dbus_complete_dev_write_location_data(object, invocation);

	return TRUE;
}		/* -----  end of static function handle_dev_write_location_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_send_vehicle_status
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_send_vehicle_status( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint64 arg_dev,
		guint remainingRange,
		gint outsideTempreture,
		gint rangeWarning )
#else
static gboolean handle_dev_send_vehicle_status( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation,
		guint arg_dev,
		guint remainingRange,
		gint outsideTempreture,
		gint rangeWarning )
#endif
{
	MHDev * _dev	=	MH_DEV( arg_dev );

	mh_dev_send_vehicle_status( _dev, remainingRange, outsideTempreture, rangeWarning );

	mediahub2_dbus_complete_dev_send_vehicle_status(object, invocation);

	return TRUE;
}		/* -----  end of static function handle_dev_send_vehicle_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_retrieve_album
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_retrieve_album( Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_col,
    guint arg_type,
    guint arg_media_type,
    gint arg_count,
	gboolean arg_fuzzy)

#else
static gboolean handle_col_retrieve_album( Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_col,
    guint arg_type,
    guint arg_media_type,
    gint arg_count,
	gboolean arg_fuzzy)
#endif
{
	MHCol * _col	=	MH_COL( arg_col );

	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE("a(ssu)"));
	gint _offset	=	0;

	MHAlbumInfo * _data	=	mh_col_retrieve_album( _col, arg_type, arg_media_type,&arg_count, arg_fuzzy);
	int i =0 ;
	for( i = 0; i < arg_count; i++)
	{
		g_variant_builder_add( _builder, "(ssu)", _data[i].album_title, _data[i].album_artist, _data[i].album_compliation);
		g_free(_data[i].album_title);
		g_free( _data[i].album_artist);
	}
	g_free( _data );

	_result	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	mediahub2_dbus_complete_col_retrieve_album(object, invocation, arg_count, _result);

	return TRUE;
}		/* -----  end of static function handle_col_retrieve_album  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_get_item_by_uri
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_get_item_by_uri(Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_dev,
    const gchar *arg_uri)
{
	MHItem * _item;
	_item	=	mh_dev_get_item_by_uri( MH_DEV( arg_dev), arg_uri);

	mediahub2_dbus_complete_dev_get_item_by_uri(object, invocation, (guint64)_item);

	return TRUE;
}		/* -----  end of static function handle_dev_get_item_by_uri  ----- */

#else 
static gboolean handle_dev_get_item_by_uri(Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_dev,
    const gchar *arg_uri)
{
	MHItem * _item;
	_item	=	mh_dev_get_item_by_uri( MH_DEV( arg_dev), arg_uri);

	mediahub2_dbus_complete_dev_get_item_by_uri(object, invocation, (guint)_item);

	return TRUE;
}		/* -----  end of static function handle_dev_get_item_by_uri  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_create_empty_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_create_empty_playlist(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_dev)
{
	MHPlaylist * _playlist;
	
	_playlist	=	mh_dev_create_empty_playlist( MH_DEV( arg_dev ));

	mediahub2_dbus_complete_dev_create_empty_playlist( object, invocation, (guint64)_playlist);
	return TRUE;
}		/* -----  end of static function handle_dev_create_empty_playlist  ----- */


#else
static gboolean handle_dev_create_empty_playlist(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_dev)
{
	MHPlaylist * _playlist;
	
	_playlist	=	mh_dev_create_empty_playlist( MH_DEV( arg_dev ));

	mediahub2_dbus_complete_dev_create_empty_playlist( object, invocation, (guint)_playlist);
	return TRUE;
}		/* -----  end of static function handle_dev_create_empty_playlist  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_append
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_append(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_playlist,
    GVariant *arg_items,
    guint arg_count)
{
	MHResult _res	=	MH_OK;
	GVariantIter * _it;
	MHItem ** _items	=	NULL;
	guint64 _val;
	int _offset	=	0;

	_items	=	g_new0( MHItem *, arg_count);

	g_variant_get( arg_items, "at", &_it);
	while( g_variant_iter_loop( _it, "t", &_val))
	{
		*( _items + _offset)	=	( MHItem *)_val;

		_offset ++;
	}
	g_variant_iter_free( _it );
	_res	=	mh_playlist_append( MH_PLAYLIST( arg_playlist), _items, arg_count);
	mediahub2_dbus_complete_playlist_append( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_playlist_append  ----- */
#else
static gboolean handle_playlist_append(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_playlist,
    GVariant *arg_items,
    guint arg_count)
{
	MHResult 	_res	=	MH_OK;
	GVariantIter * _it;
	MHItem ** _items	=	NULL;
	guint _val;
	int _offset	=	0;

	_items	=	g_new0( MHItem *, arg_count);

	g_variant_get( arg_items, "au", &_it);
	while( g_variant_iter_loop( _it, "u", &_val))
	{
		*( _items + _offset)	=	( MHItem *)_val;

		_offset ++;
	}
	g_variant_iter_free( _it );
	_res	=	mh_playlist_append( MH_PLAYLIST( arg_playlist), _items, arg_count);
	mediahub2_dbus_complete_playlist_append( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_playlist_append  ----- */
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_append_playlist
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_append_playlist(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_playlist,
   guint64 arg_playlist_des)
{
	MHResult _res	=	MH_OK;
	_res	=	mh_playlist_append_playlist( MH_PLAYLIST( arg_playlist), MH_PLAYLIST( arg_playlist_des));
	mediahub2_dbus_complete_playlist_append_playlist( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_playlist_append_playlist  ----- */
#else
static gboolean handle_playlist_append_playlist(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_playlist,
    guint arg_playlist_des)
{
	MHResult 	_res	=	MH_OK;
	_res	=	mh_playlist_append_playlist( MH_PLAYLIST( arg_playlist), MH_PLAYLIST( arg_playlist_des));
	mediahub2_dbus_complete_playlist_append_playlist( object, invocation, _res);
	return TRUE;
}		/* -----  end of static function handle_playlist_append_playlist  ----- */
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_insert
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_insert(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_playlist,
    guint arg_index,
    GVariant *arg_items,
    guint arg_count)
{
	MHResult _res	=	MH_OK;
	GVariantIter * _it;
	MHItem ** _items	=	NULL;
	guint64  _val;
	int _offset	=	0;

	_items	=	g_new0( MHItem *, arg_count);

	g_variant_get( arg_items, "at", &_it);
	while( g_variant_iter_loop( _it, "t", &_val))
	{
		*( _items + _offset)	=	( MHItem *)_val;

		_offset ++;
	}
	g_variant_iter_free( _it );

	_res	=	mh_playlist_insert( MH_PLAYLIST(arg_playlist), arg_index, _items, arg_count);
	mediahub2_dbus_complete_playlist_insert( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_playlist_insert  ----- */

#else
static gboolean handle_playlist_insert(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_playlist,
    guint arg_index,
    GVariant *arg_items,
    guint arg_count)
{
	MHResult _res	=	MH_OK;

	GVariantIter * _it;

	MHItem ** _items	=	NULL;

	guint _val;

	int _offset	=	0;

	_items	=	g_new0( MHItem *, arg_count);

	g_variant_get( arg_items, "au", &_it);

	while( g_variant_iter_loop( _it, "u", &_val))
	{
		*( _items + _offset)	=	( MHItem *)_val;

		_offset ++;
	}
	g_variant_iter_free( _it );

	_res	=	mh_playlist_insert( MH_PLAYLIST(arg_playlist), arg_index, _items, arg_count);

	mediahub2_dbus_complete_playlist_insert( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_playlist_insert  ----- */
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_playlist_remove
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_playlist_remove (
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_playlist,
    guint arg_index,
    guint arg_count)
#else
static gboolean handle_playlist_remove (
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_playlist,
    guint arg_index,
    guint arg_count)
#endif
{
	MHResult _res	=	MH_OK;

	_res	=	mh_playlist_remove( MH_PLAYLIST(arg_playlist), arg_index, arg_count);

	mediahub2_dbus_complete_playlist_remove( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_playlist_remove  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_send_signal_touch
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_send_signal_touch( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		  guint64 arg_dev, guint arg_press_type, guint arg_x, guint arg_y )
#else
static gboolean handle_send_signal_touch( Mediahub2Dbus * object, GDBusMethodInvocation * invocation,
		  guint arg_dev, guint arg_press_type, guint arg_x, guint arg_y )
#endif
{
	MHResult _res	=	MH_OK;

	MHDev	* _dev	=	MH_DEV( arg_dev );

	_res	=	mh_dev_send_signal_touch( _dev, arg_press_type, arg_x, arg_y);

	mediahub2_dbus_complete_send_signal_touch( object, invocation, _res );

	return TRUE;
}		/* -----  end of static function handle_send_signal_touch  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_start
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_start(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_dev)
#else
static gboolean handle_dev_start(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_dev)
#endif
{
	MHResult _res	=	MH_OK;

	MHDev	* _dev	=	MH_DEV(arg_dev);

	_res	=	mh_dev_start( _dev);

	mediahub2_dbus_complete_dev_start( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_dev_start  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  request_ui
 *  Description:  
 * =====================================================================================
 */
static void request_ui( MHDev * dev, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_request_ui( user_data, ( guint64 )dev );
#else
	mediahub2_dbus_emit_request_ui( user_data, ( guint )dev );
#endif
}		/* -----  end of static function request_ui  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  modes_changed
 *  Description:  
 * =====================================================================================
 */
static void modes_changed( MHDev * dev, const gchar * screen, const char * main_audio,
		const gchar * speech_entity, const gchar * speech_mode, const gchar * phone,
		const gchar * turns, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_modes_changed( user_data, ( guint64 )dev, screen, main_audio,
			speech_entity, speech_mode, phone, turns );
#else
	mediahub2_dbus_emit_modes_changed( user_data, ( guint )dev, screen, main_audio,
			speech_entity, speech_mode, phone, turns );
#endif
}		/* -----  end of static function modes_changed  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_detach
 *  Description:  
 * =====================================================================================
 */
static void dev_detach( MHDev * dev, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_dev_detach(( Mediahub2Dbus * )user_data, (guint64) dev);
#else
	mediahub2_dbus_emit_dev_detach(( Mediahub2Dbus * )user_data, (guint) dev);
#endif
}		/* -----  end of static function dev_detach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  disable_bluetooth
 *  Description:  
 * =====================================================================================
 */
static void disable_bluetooth( MHDev * dev, const gchar * device_id, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_disable_bluetooth( ( Mediahub2Dbus * )user_data, ( guint64 )dev, device_id );
#else
	mediahub2_dbus_emit_disable_bluetooth( ( Mediahub2Dbus * )user_data, ( guint )dev, device_id );
#endif
}		/* -----  end of static function disable_bluetooth  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  duck_audio
 *  Description:  
 * =====================================================================================
 */
static void duck_audio( MHDev * dev, double durationms, double volume, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_duck_audio( ( Mediahub2Dbus * )user_data, ( guint64 )dev, durationms, volume );
#else
	mediahub2_dbus_emit_duck_audio( ( Mediahub2Dbus * )user_data, ( guint )dev, durationms, volume );
#endif
}		/* -----  end of static function duck_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  unduck_audio
 *  Description:  
 * =====================================================================================
 */
static void unduck_audio( MHDev * dev, double durationms, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_unduck_audio( ( Mediahub2Dbus * )user_data, ( guint64 )dev, durationms );
#else
	mediahub2_dbus_emit_unduck_audio( ( Mediahub2Dbus * )user_data, ( guint )dev, durationms );
#endif
}		/* -----  end of static function unduck_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ea_session_start
 *  Description:  
 * =====================================================================================
 */
static void ea_session_start( MHDev * dev, guint protocol_id, guint session_id, guint pb, gpointer user_data)
{
	g_message("%s ==========================> server.c",__func__);

#ifdef __x86_64__
	mediahub2_dbus_emit_ea_session_start( ( Mediahub2Dbus * )user_data, ( guint64 )dev, protocol_id, session_id, pb );
#else
	mediahub2_dbus_emit_ea_session_start( ( Mediahub2Dbus * )user_data, ( guint )dev, protocol_id, session_id, pb );
#endif
}		/* -----  end of static function ea_session_start  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ea_session_stop
 *  Description:  
 * =====================================================================================
 */
static void ea_session_stop( MHDev * dev, guint session_id, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_ea_session_stop( ( Mediahub2Dbus * )user_data, ( guint64 )dev, session_id );
#else
	mediahub2_dbus_emit_ea_session_stop( ( Mediahub2Dbus * )user_data, ( guint )dev, session_id );
#endif
}		/* -----  end of static function ea_session_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ea_session_data
 *  Description:  
 * =====================================================================================
 */
static void ea_session_data( MHDev * dev, GVariant * var, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_ea_session_data( ( Mediahub2Dbus * )user_data, ( guint64 )dev, var );
#else
	mediahub2_dbus_emit_ea_session_data( ( Mediahub2Dbus * )user_data, ( guint )dev, var );
#endif
}		/* -----  end of static function ea_session_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ea_session_send_pb
 *  Description:  
 * =====================================================================================
 */
static void ea_session_send_pb( MHDev * dev, guint pb, gpointer user_data )
{
	g_message("%s ==========================> server.c",__func__);
#ifdef __x86_64__
	mediahub2_dbus_emit_ea_session_send_pb( ( Mediahub2Dbus * )user_data, ( guint64 )dev, pb );
#else
	mediahub2_dbus_emit_ea_session_send_pb( ( Mediahub2Dbus * )user_data, ( guint )dev, pb );
#endif
}		/* -----  end of static function ea_session_send_pb  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bt_data
 *  Description:  
 * =====================================================================================
 */
static void bt_data( MHDev * dev, GVariant * var, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_bt_data( ( Mediahub2Dbus * )user_data, ( guint64 )dev, var );
#else
	mediahub2_dbus_emit_bt_data( ( Mediahub2Dbus * )user_data, ( guint )dev, var );
#endif
}		/* -----  end of static function bt_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: start_location_info 
 *  Description:  
 * =====================================================================================
 */
static void start_location_info( MHDev * dev, guint location_id, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_start_location_info( ( Mediahub2Dbus * )user_data, ( guint64 )dev, location_id );
#else
	mediahub2_dbus_emit_start_location_info( ( Mediahub2Dbus * )user_data, ( guint )dev, location_id );
#endif
}		/* -----  end of static function start_location_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: stop_location_info 
 *  Description:  
 * =====================================================================================
 */
static void stop_location_info( MHDev * dev, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_stop_location_info( ( Mediahub2Dbus * )user_data, ( guint64 )dev );
#else
	mediahub2_dbus_emit_stop_location_info( ( Mediahub2Dbus * )user_data, ( guint )dev );
#endif
}		/* -----  end of static function stop_location_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: start_vehicle_status_updates
 *  Description:  
 * =====================================================================================
 */
static void start_vehicle_status_updates( MHDev * dev, guint status_id, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_start_vehicle_status_updates( ( Mediahub2Dbus * )user_data, ( guint64 )dev, status_id );
#else
	mediahub2_dbus_emit_start_vehicle_status_updates( ( Mediahub2Dbus * )user_data, ( guint )dev, status_id );
#endif
}		/* -----  end of static function start_vehicle_status_updates  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: stop_vehicle_status_updates 
 *  Description:  
 * =====================================================================================
 */
static void stop_vehicle_status_updates( MHDev * dev, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_stop_vehicle_status_updates( ( Mediahub2Dbus * )user_data, ( guint64 )dev );
#else
	mediahub2_dbus_emit_stop_vehicle_status_updates( ( Mediahub2Dbus * )user_data, ( guint )dev );
#endif
}		/* -----  end of static function stop_vehicle_status_updates  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: req_accessory_wifi_conf_info 
 *  Description:  
 * =====================================================================================
 */
static void req_accessory_wifi_conf_info( MHDev * dev, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_req_accessory_wifi_conf_info( ( Mediahub2Dbus * )user_data, ( guint64 )dev );
#else
	mediahub2_dbus_emit_req_accessory_wifi_conf_info( ( Mediahub2Dbus * )user_data, ( guint )dev );
#endif
}		/* -----  end of static function req_accessory_wifi_conf_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: wifi_carplay_update
 *  Description:  
 * =====================================================================================
 */
static void wifi_carplay_update( MHDev * dev, guint status_id, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_wifi_carplay_update( ( Mediahub2Dbus * )user_data, ( guint64 )dev, status_id );
#else
	mediahub2_dbus_emit_wifi_carplay_update( ( Mediahub2Dbus * )user_data, ( guint )dev, status_id );
#endif
}		/* -----  end of static function wifi_carplay_update  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  audio_info
 *  Description:  
 * =====================================================================================
 */
static void audio_info( MHDev * dev, guint status, guint type, guint rate, guint channel, gpointer user_data )
{
#ifdef __x86_64__
	mediahub2_dbus_emit_audio_info( user_data, ( guint64 )dev, status, type, rate, channel );
#else
	mediahub2_dbus_emit_audio_info( user_data, ( guint )dev, status, type, rate, channel );
#endif
}		/* -----  end of static function audio_info  ----- */

static struct
{
	gchar * signal;
	void * handler;
} signalTable[]	=	
{
	{ "request_ui", request_ui },
	{ "modes_changed", modes_changed },
	{ "dev_detach", dev_detach },
	{ "disable_bluetooth", disable_bluetooth },
	{ "ea_session_start", ea_session_start },
	{ "ea_session_stop", ea_session_stop },
	{ "ea_session_data", ea_session_data },
	{ "ea_session_send_pb", ea_session_send_pb },
	{ "start_location_info", start_location_info },
	{ "stop_location_info", stop_location_info },
	{ "start_vehicle_status_updates", start_vehicle_status_updates },
	{ "stop_vehicle_status_updates", stop_vehicle_status_updates },
	{ "duck_audio", duck_audio },
	{ "unduck_audio", unduck_audio },
	{ "bt_data", bt_data },
	{ "req_accessory_wifi_conf_info", req_accessory_wifi_conf_info },
	{ "wifi_carplay_update", wifi_carplay_update },
	{ "audio_info", audio_info },
	{ NULL, NULL }
};
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_signal_connect
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_obj_signal_connect(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_obj,
    const gchar *arg_signal)
#else
static gboolean handle_obj_signal_connect(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_obj,
    const gchar *arg_signal)
#endif
{

	MHDev * _dev	=	g_object_ref( MH_DEV( arg_obj ));
	int i;

	if( _dev != NULL )
	{
		for( i = 0; signalTable[i].signal != NULL; i ++ )
		{
			if( g_strcmp0( signalTable[i].signal, arg_signal ) == 0 )
			{
				g_signal_connect( _dev, arg_signal, signalTable[i].handler, object );
			}
		}

		mediahub2_dbus_complete_obj_signal_connect( object, invocation );

		g_object_unref( _dev );
	}

	return TRUE;
}		/* -----  end of static function handle_obj_signal_connect  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_close_connection
 *  Description:  
 * =====================================================================================
 */
static void on_close_connection( GDBusConnection * connection, gboolean remote_peer_vanished,
		GError * error, gpointer user_data )
{
	g_message( "The %s connection [ %p ] was closed", (char *)user_data, connection );
//	g_free((char *)user_data);
}		/* -----  end of static function on_close_connection  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_new_connection
 *  Description:  
 * =====================================================================================
 */
static gboolean on_new_connection (GDBusServer *server, GDBusConnection *connection,
		gpointer user_data)
{
	GList * _list	=	engineList;

	while( _list != NULL )
	{
		load_engine _loadEngine	=	_list->data;

		_loadEngine( server, connection, user_data );

		_list	=	_list->next;
	}
	g_signal_connect( connection, "closed", G_CALLBACK( on_close_connection ), user_data );

	return TRUE;
}		/* -----  end of static function on_new_connection  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  register_reply
 *  Description:  
 * =====================================================================================
 */
static void register_reply( DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode,
		const char * name, const char * regtype, const char * domain, void * context )
{
	printf( "%s\n", __func__ );
	printf("name:%s, regtype:%s, domain:%s\n", name, regtype, domain);
}		/* -----  end of static function register_reply  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_ipc_server_create
 *  Description:  
 * =====================================================================================
 */
MHIPCServer * mh_ipc_server_create()
{
	MHIPCServer * _server	=	g_new0( MHIPCServer, 1);
	gchar * _guid;
	GError * _error	=	NULL;

	_guid	=	g_dbus_generate_guid();

	DNSServiceRef _sdRef;

	const char * _streaming_enble;
	char * _user_data;

	g_message("unix:abstract=mediahub2");
	_server->ipc_server	=	g_dbus_server_new_sync( "unix:abstract=mediahub2",
			G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS,
			_guid, NULL, NULL, &_error );

	_streaming_enble	= getenv( "MH_STREAMING_ENABLED");
	
	if( _streaming_enble != NULL)
	{
		char * _dlerror;
		dnsHandle	=	dlopen("libdns_sd.so", RTLD_LAZY);

		_dlerror	=	dlerror();

		if( _dlerror)
		{
			g_message("load libdns_sd:%s", _dlerror);
		}

		_DNSServiceRegister	=	dlsym( dnsHandle, "DNSServiceRegister");

		_dlerror	=	dlerror();

		if( _dlerror)

		{
			g_message("dlsym DNSServiceRegister:%s", _dlerror);
		}

		_DNSServiceProcessResult	=	dlsym( dnsHandle, "DNSServiceProcessResult");

		_dlerror	=	dlerror();

		if( _dlerror)

		{
			g_message("dlsym DNSServiceProcessResult:%s", _dlerror);
		}

		g_message("%d", _DNSServiceRegister( &_sdRef, 0, 0, NULL,
			"_Media-Hub-v2._tcp", NULL, NULL, htons( 6766 ), 0, NULL, register_reply, NULL ));

		_DNSServiceProcessResult( _sdRef );

		_server->streaming_server	=	g_dbus_server_new_sync( "tcp:host=::,port=6766,family=ipv6",
			G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS,
			_guid, NULL, NULL, &_error );

		dlclose(dnsHandle);
		_user_data	=	g_strdup("streaming");
		g_signal_connect( _server->streaming_server, "new-connection", G_CALLBACK( on_new_connection), _user_data);
	}
	if( _error != NULL )
	{
		g_warning( "Create Media-Hub v2.0 ipc server failed: [ %s ]", _error->message );

		g_error_free( _error );

		return NULL;
	}

	g_free( _guid );

	_user_data	=	g_strdup("unix");

	g_signal_connect( _server->ipc_server, "new-connection", G_CALLBACK( on_new_connection ), _user_data );

	return ( MHIPCServer * )_server;
}		/* -----  end of function mh_ipc_server_create  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_file_get_tag
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_file_get_tag(Mediahub2Dbus *object,
								    GDBusMethodInvocation *invocation,
  									const gchar *arg_file_path)
{
	MHTagInfo * _tag;	
	GVariant * _res;
	char * _path	=	g_strdup( arg_file_path);
	_tag	=	mh_file_get_tag( _path);
	_res	=	g_variant_new("(ssssuuu)", _tag->title ? _tag->title : "",
			_tag->album ? _tag->album :"", _tag->artist ? _tag->artist : "", _tag->genre ? _tag->genre : "",
			_tag->duration, _tag->year, _tag->track);

	mediahub2_dbus_complete_file_get_tag( object, invocation, _res );
	
	g_variant_unref( _res );
	_tag->title != NULL ? g_free( _tag->title): NULL;
	_tag->album	!= NULL ? g_free( _tag->album): NULL;
	_tag->artist != NULL ? g_free( _tag->artist) : NULL;
	_tag->genre != NULL ? g_free( _tag->genre) : NULL;
	g_free( _tag);
	g_free(_path);
	
	return TRUE;
}		/* -----  end of static function handle_file_get_tag  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_write_ea_data
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_write_ea_data(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_obj,
    GVariant *arg_data)
#else
static gboolean handle_dev_write_ea_data(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_obj,
    GVariant *arg_data)
#endif
{
	MHDev * _dev	=	MH_DEV( arg_obj );
	gsize _len	=	0;
	uint64_t _resLen;
	
	if( _dev != NULL )
	{
		const guchar * _buf;
		GVariant * _var	=	g_variant_get_variant( arg_data );

		_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( guchar ));

		g_object_ref( _dev );

		_resLen	=	mh_dev_write_ea_data( _dev, _buf, (uint64_t)_len );

		g_object_unref( _dev );
	}

	mediahub2_dbus_complete_dev_write_ea_data( object, invocation, _resLen );

	return TRUE;
}		/* -----  end of static function handle_dev_write_ea_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_write_bt_data
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_write_bt_data(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_obj,
    GVariant *arg_data)
#else
static gboolean handle_dev_write_bt_data(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_obj,
    GVariant *arg_data)
#endif
{
	MHDev * _dev	=	MH_DEV( arg_obj );
	gsize _len	=	0;
	uint64_t _resLen;
	
	if( _dev != NULL )
	{
		const guchar * _buf;
		GVariant * _var	=	g_variant_get_variant( arg_data );

		_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( guchar ));

		g_object_ref( _dev );

		_resLen	=	mh_dev_write_bt_data( _dev, _buf, (uint64_t)_len );

		g_object_unref( _dev );
	}

	mediahub2_dbus_complete_dev_write_bt_data( object, invocation, _resLen );

	return TRUE;
}		/* -----  end of static function handle_dev_write_bt_data  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  hand_send_wifi_conf_info
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_send_wifi_conf_info(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_obj,
    GVariant *arg_ssid,
	GVariant *arg_pass,
	guint arg_securityType,
	guint arg_channel)
#else
static gboolean handle_dev_send_wifi_conf_info(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_obj,
    GVariant *arg_ssid,
	GVariant *arg_pass,
	guint arg_securityType,
	guint arg_channel)
#endif
{
	MHResult _res	=	MH_OK;
	MHDev * _dev	=	MH_DEV( arg_obj );

	const uint8_t * _ssid;
	const uint8_t * _pass;
	gsize _len	=	0;

	GVariant * _var	=	g_variant_get_variant( arg_ssid );
	_ssid	=	g_variant_get_fixed_array( _var, &_len, sizeof( uint8_t ));

	_var	=	g_variant_get_variant( arg_pass );
	_pass	=	g_variant_get_fixed_array( _var, &_len, sizeof( uint8_t ));

	_res	=	mh_dev_send_wifi_conf_info( _dev, _ssid, _pass, arg_securityType, arg_channel );

	mediahub2_dbus_complete_misc_carplay_init_modes( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function hand_send_wifi_conf_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_obj_get_property_type
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__

static gboolean handle_obj_get_property_type( Mediahub2Dbus *object,
		GDBusMethodInvocation *invocation, guint64 arg_obj, const gchar *arg_property)

{
	GObject * _obj	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;

	if( _obj 	==	NULL)
	{
		mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint64) G_TYPE_INVALID);
	}
	else
	{
		GParamSpec * _spec	=	g_object_class_find_property( G_OBJECT_GET_CLASS( _obj), arg_property);

		if( _spec	==	NULL)
		{
			mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint64 )G_TYPE_INVALID);
		}
		else
		{
			mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint64 )_spec->value_type);
		}
	}

	return TRUE;
}		/* -----  end of static function handle_obj_get_property_type  ----- */
#else
static gboolean handle_obj_get_property_type(Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation, guint arg_obj, const gchar *arg_property)
{
	GObject * _obj	=	G_IS_OBJECT( arg_obj ) ? G_OBJECT( arg_obj ) : NULL;

	if( _obj 	==	NULL)
	{
		mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint) G_TYPE_INVALID);
	}
	else
	{
		GParamSpec * _spec	=	g_object_class_find_property( G_OBJECT_GET_CLASS( _obj), arg_property);

		if( _spec	==	NULL)
		{
			mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint )G_TYPE_INVALID);
		}
		else
		{
			mediahub2_dbus_complete_obj_get_property_type( object, invocation, (guint )_spec->value_type);
		}
	}
	return TRUE;

}
#endif 
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dev_status_cb
 *  Description:  
 * =====================================================================================
 */
static void _dev_status_cb(MHDev * self, MHDevStatus status, void * user_data) 
{
#ifdef __x86_64__
	mediahub2_dbus_emit_dev_status( (Mediahub2Dbus*)user_data, (guint64) self, (guint) status);
#else
	mediahub2_dbus_emit_dev_status( (Mediahub2Dbus*)user_data, (guint) self, (guint) status);
#endif
}		/* -----  end of static function _dev_status_cb  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_register_status_listener
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_register_status_listener(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint64 arg_dev)
#else
static gboolean handle_dev_register_status_listener(
    Mediahub2Dbus *object,
    GDBusMethodInvocation *invocation,
    guint arg_dev)
#endif 
{
	MHResult _res	=	MH_OK;

	MHDev * _dev	=	MH_DEV(arg_dev);

	MHDevStatusListener *_status_listener	= 	g_new0( MHDevStatusListener, 1);
	_status_listener->callback	=	_dev_status_cb;
	_status_listener->user_data	=	object;
	
	_res	=	mh_dev_register_status_listener( _dev, _status_listener );

	g_free( _status_listener);

	mediahub2_dbus_complete_dev_register_status_listener( object, invocation, _res );

	return TRUE;

}		/* -----  end of static function handle_dev_register_status_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_col_set_favorite
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_col_set_favorite(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
		guint64 arg_col, guint arg_type, guint arg_favorite)
#else
static gboolean handle_col_set_favorite(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, 
		guint arg_col, guint arg_type, guint arg_favorite)
#endif
{
	g_message("server.c--->%s", __func__);
	MHCol * _col	=	MH_COL( arg_col);
	mh_col_set_favorite(_col, (MHItemType)arg_type, arg_favorite);
	mediahub2_dbus_complete_col_set_favorite(object, invocation);
	return TRUE;
}		/* -----  end of static function handle_col_set_favorite  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_add_file
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_add_file( Mediahub2Dbus *object,GDBusMethodInvocation *invocation,
    guint64 arg_dev, guint64 arg_dest, guint64 arg_source)
{
	MHItem * _item;
	_item	=	mh_dev_add_file( MH_DEV(arg_dev), MH_FOLDER(arg_dest), MH_ITEM(arg_source));
	mediahub2_dbus_complete_dev_add_file(object, invocation, (guint64)_item);	
	g_message("server.c--->%s", __func__);
	return TRUE;
}
#else
static gboolean handle_dev_add_file(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, 
    guint arg_dev, guint arg_dest, guint arg_source)

{
	MHItem * _item;
	_item	=	mh_dev_add_file( MH_DEV(arg_dev), MH_FOLDER(arg_dest), MH_ITEM(arg_source));
	mediahub2_dbus_complete_dev_add_file( object, invocation, (guint)_item);	
	g_message("server.c--->%s", __func__);
	return TRUE;
}		/* -----  end of static function handle_col_set_favorite  ----- */
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_detete_file
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_detete_file(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint64 arg_dev, guint64 arg_item)
#else
static gboolean handle_dev_detete_file(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
    guint arg_dev, guint arg_item)
#endif
{
	MHDev * _dev	=	MH_DEV(arg_dev);
	MHItem * _item	=	MH_ITEM(arg_item);
	MHResult  _res;
	_res	=	mh_dev_delete_file( _dev, _item);
	mediahub2_dbus_complete_dev_detete_file( object, invocation, (guint)_res);
	return TRUE;
}		/* -----  end of static function handle_dev_detete_file  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_set_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_misc_save_pb( Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_obj )
#else
static gboolean handle_misc_save_pb( Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_obj )
#endif
{
	MHResult _res	=	MH_OK;

	MHPb * _pb	=	MH_PB( arg_obj );

	_res	=	mh_misc_save_pb( _pb );

	mediahub2_dbus_complete_misc_save_pb( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_misc_misc_save_pb  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_set_iap_device_mode
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_set_iap_device_mode( Mediahub2Dbus *object, GDBusMethodInvocation *invocation, MiscIapDeviceMode mode )
{
	MHResult _res	=	MH_OK;

	_res	=	mh_misc_set_iap_device_mode( mode );

	mediahub2_dbus_complete_misc_set_iap_device_mode( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_misc_set_iap_device_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_misc_carplay_init_modes
 *  Description:  
 * =====================================================================================
 */
static gboolean handle_misc_carplay_init_modes( Mediahub2Dbus *object, GDBusMethodInvocation *invocation, GVariant *arg_data )
{
	MHResult _res	=	MH_OK;

	const guchar * _buf;
	gsize _len	=	0;

	GVariant * _var	=	g_variant_get_variant( arg_data );

	_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( guchar ));

	_res	=	mh_misc_carplay_init_modes( _buf );

	mediahub2_dbus_complete_misc_carplay_init_modes( object, invocation, _res);

	return TRUE;
}		/* -----  end of static function handle_misc_carplay_init_modes  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _Module_Status
 *  Description:  
 * =====================================================================================
 */
static void _Module_Status(MHDev *dev, ModuleStatusList * list, void * user_data)
{
	g_message("\n\nserver.c*************************%s\n\n", __func__);
	uint32_t _count	=	list->count;
	g_message("server.c->count:%d", _count);
	int i	=	0;
	GVariant * _ret;
	GVariantBuilder * _builder	=	g_variant_builder_new(G_VARIANT_TYPE("a(uu)"));
	for(i; i < _count; i++)
	{
		g_message("server.c-->list[%d].module:%d, list[%d}.status:%d", i, list->list[i].module,
				i, list->list[i].status);
		g_variant_builder_add( _builder, "(uu)", list->list[i].module, list->list[i].status);
	}
	_ret	=	g_variant_builder_end( _builder );

	#ifdef __x86_64__
	mediahub2_dbus_emit_dev_module_status ((Mediahub2Dbus *)user_data, (guint64)dev, _count, _ret);
	#else
	mediahub2_dbus_emit_dev_module_status ((Mediahub2Dbus *)user_data, (guint)dev, _count, _ret);
	#endif
	g_variant_builder_unref( _builder);

}		/* -----  end of static function _Module_Status  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_register_module_status
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_register_module_status (Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_dev)
#else
static gboolean handle_dev_register_module_status (Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_dev)
#endif
{
	g_message("%s", __func__);
	MHResult _res	=	MH_OK;
	MHDev * _dev	=	(MHDev *)arg_dev;

	ModuleStatusListener * _modulestatus	=	g_new0( ModuleStatusListener, 1);

	_modulestatus->callback		=	_Module_Status;

	_modulestatus->user_data	=	object;

	_res	=	mh_dev_register_module_status( _dev, _modulestatus);

	g_free( _modulestatus);

	mediahub2_dbus_complete_dev_register_module_status( object, invocation, _res );
	return TRUE;


}	/* -----  end of static function handle_dev_register_module_status  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _carlife_bt_hfp_request_cb
 *  Description:  
 * =====================================================================================
 */
static void _carlife_bt_hfp_request_cb(MHDev * dev, uint32_t type, const char * phone_num,
		int32_t dtmfcode, gpointer user_data)
{
#ifdef __x86_64__
	mediahub2_dbus_emit_carlife_bt_hfp_request( user_data, ( guint64 )dev, type, phone_num, dtmfcode );
#else
	mediahub2_dbus_emit_carlife_bt_hfp_request( user_data, ( guint32 )dev, type, phone_num, dtmfcode );

#endif
}		/* -----  end of static function _carlife_bt_hfp_request_cb  ----- */

#ifdef __x86_64__
static gboolean handle_carlife_register_bt_hfp_request(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint64 arg_dev)
#else
static gboolean handle_carlife_register_bt_hfp_request(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, guint arg_dev)
#endif
{

//	MHDev * _dev	=	g_object_ref( MH_DEV( arg_dev));
	MHDev * _dev	=	(MHDev *)arg_dev;
	g_signal_connect( _dev, "carlife_bt_hfp_request", (void *)_carlife_bt_hfp_request_cb, object);

	mediahub2_dbus_complete_carlife_register_bt_hfp_request( object, invocation);

	return TRUE;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  handle_dev_search_name
 *  Description:  
 * =====================================================================================
 */
#ifdef __x86_64__
static gboolean handle_dev_search_name(Mediahub2Dbus *object, GDBusMethodInvocation *invocation,
		guint64 arg_dev, guint arg_type, const gchar *arg_string, gint arg_count)
{
	MHItem ** _items	=	NULL;

	MHDev * _dev	=	(MHDev *)arg_dev;

	char * _string	=	g_strdup( arg_string);

	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));
	gint _offset	=	0;

	_items	=	mh_dev_search_name( _dev, arg_type, _string, &arg_count);
	if( _items != NULL)
	{
		while( _offset < arg_count)
		{
			g_variant_builder_add( _builder, "t", (guint64)_items[_offset]);
			_offset ++;
		}
		g_free(_items);
	}
	else
	{
		*((gint*)(&arg_count)) = 0;
	}

	_result	=	g_variant_builder_end( _builder);



	mediahub2_dbus_complete_dev_search_name( object, invocation, arg_count, _result); 

	g_variant_builder_unref( _builder);

	g_free( _string);

	return TRUE;
}
#else
static gboolean handle_dev_search_name(Mediahub2Dbus *object, GDBusMethodInvocation *invocation, 
		guint arg_dev, guint arg_type, const gchar *arg_string, gint arg_count)
{
	MHItem ** _items	=	NULL;

	MHDev * _dev	=	(MHDev *)arg_dev;

	char * _string	=	g_strdup( arg_string);

	GVariant * _result;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));
	gint _offset	=	0;

	_items	=	mh_dev_search_name( _dev, arg_type, _string, &arg_count);
	if( _items != NULL)
	{
		while( _offset < arg_count)
		{
			g_variant_builder_add( _builder, "u", (guint32)_items[_offset]);
			_offset ++;
		}
		g_free(_items);
	}
	else
	{
		*((gint*)(&arg_count)) = 0;
	}

	_result	=	g_variant_builder_end( _builder);

	g_variant_builder_unref( _builder);

	mediahub2_dbus_complete_dev_search_name( object, invocation, arg_count, _result); 

	return TRUE;
}
#endif
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  load_media_engine
 *  Description:  
 * =====================================================================================
 */
static void load_media_engine(GDBusServer *server, GDBusConnection *connection,
		gpointer user_data)
{
	GError * _error	=	NULL;
	Mediahub2Dbus * _object;
	Mediahub2DbusIface * _iface;	

	_object	=	mediahub2_dbus_skeleton_new();

	_iface	=	MEDIAHUB2_DBUS_GET_IFACE( _object );

	_iface->handle_core_register_devices_listener	=	handle_core_register_devices_listener;
	_iface->handle_core_register_events_listener	=	handle_core_register_events_listener;
	_iface->handle_core_start	=	handle_core_start;
	_iface->handle_core_stop	=	handle_core_stop;
	_iface->handle_obj_get_properties	=	handle_obj_get_properties;
	_iface->handle_obj_get_properties_type	=	handle_obj_get_properties_type;
	_iface->handle_obj_set_properties	=	handle_obj_set_properties;
	_iface->handle_obj_unref	=	handle_obj_unref;
	_iface->handle_obj_ref	=	handle_obj_ref;
	_iface->handle_folder_get_children	=	handle_folder_get_children;
	_iface->handle_folder_create_playlist	=	handle_folder_create_playlist;
	_iface->handle_playlist_foreach	=	handle_playlist_foreach;
	_iface->handle_playlist_foreach_shm	=	handle_playlist_foreach_shm;
	_iface->handle_pb_play_by_list	=	handle_pb_play_by_list;
	_iface->handle_pb_play_radio_by_index	=	handle_pb_play_radio_by_index;
	_iface->handle_pb_next	=	handle_pb_next;
	_iface->handle_pb_previous	=	handle_pb_previous;
	_iface->handle_item_foreach	=	handle_item_foreach;
	_iface->handle_item_foreach_fd	=	handle_item_foreach_fd;
	_iface->handle_item_foreach_shm	=	handle_item_foreach_shm;

	_iface->handle_dev_restore_playlist	=	handle_dev_restore_playlist;
	_iface->handle_pb_backward	=	handle_pb_backward;
	_iface->handle_pb_backward_done	=	handle_pb_backward_done;
	_iface->handle_pb_forward	=	handle_pb_forward;
	_iface->handle_pb_forward_done	=	handle_pb_forward_done;
	_iface->handle_pb_pause	=	handle_pb_pause;
	_iface->handle_pb_play	=	handle_pb_play;
	_iface->handle_pb_play_pause	=	handle_pb_play_pause;
	_iface->handle_pb_seek	=	handle_pb_seek;
	_iface->handle_pb_create	=	handle_pb_create;
	_iface->handle_pb_stop	=	handle_pb_stop;
	_iface->handle_pb_close	=	handle_pb_close;
	_iface->handle_pb_register_events_listener	=	handle_pb_register_events_listener;
	_iface->handle_pb_playlist_change	=	handle_pb_playlist_change;
	_iface->handle_dev_start_scan		=	handle_dev_start_scan;
	_iface->handle_filter_create		=	handle_filter_create;
	_iface->handle_dev_register_detach_listener 	=	handle_dev_register_detach_listener;
	_iface->handle_dev_register_events_listener		=	handle_dev_register_events_listener;
	_iface->handle_shm_unlink	=	handle_shm_unlink;
	_iface->handle_folder_seek	=	handle_folder_seek;
	_iface->handle_pb_register_status_listener	=	handle_pb_register_status_listener;
	_iface->handle_pb_resize	=	handle_pb_resize;
	_iface->handle_pb_pixel_aspect_ratio	=	handle_pb_pixel_aspect_ratio;
	_iface->handle_pb_audiobook_playback_speed	=	handle_pb_audiobook_playback_speed;
	_iface->handle_pb_get_track_info	=	handle_pb_get_track_info;
	_iface->handle_pb_get_subtitle_info	=	handle_pb_get_subtitle_info;

	_iface->handle_col_create	=	handle_col_create;
	_iface->handle_col_retrieve_data	=	handle_col_retrieve_data;
//	_iface->handle_col_set_filter	=	handle_col_set_filter;
	_iface->handle_col_create_playlist	=	handle_col_create_playlist;

	_iface->handle_dev_get_playlist_info	=	handle_dev_get_playlist_info;

	_iface->handle_misc_set_filter	=	handle_misc_set_filter;
	
	_iface->handle_dev_save_playlist	=	handle_dev_save_playlist;
	_iface->handle_dev_update_playlist 	=	handle_dev_update_playlist;
	_iface->handle_dev_delete_playlist	=	handle_dev_delete_playlist;
	_iface->handle_dev_get_radiolist	=	handle_dev_get_radiolist;
	_iface->handle_dev_attach_pb	=	handle_dev_attach_pb;
	_iface->handle_misc_reset	=	handle_misc_reset;
	_iface->handle_playlist_sort	=	handle_playlist_sort;
	_iface->handle_dev_scan_abort	=	handle_dev_scan_abort;
	_iface->handle_misc_db_restore	=	handle_misc_db_restore;
	_iface->handle_misc_db_delete_by_serial_number	=	handle_misc_db_delete_by_serial_number;//del db by deviceId

	_iface->handle_col_add_filter	=	handle_col_add_filter;
	_iface->handle_col_filter_clear =	handle_col_filter_clear;
	_iface->handle_col_set_retrieve_key	=	handle_col_set_retrieve_key;
	_iface->handle_col_set_order_type	=	handle_col_set_order_type;

	_iface->handle_dev_request_app_launch	=	handle_dev_request_app_launch;
	_iface->handle_col_retrieve_album	=	handle_col_retrieve_album;
	_iface->handle_dev_get_item_by_uri	=	handle_dev_get_item_by_uri;
	_iface->handle_dev_create_empty_playlist	=	handle_dev_create_empty_playlist;
	_iface->handle_playlist_append	=	handle_playlist_append;
	_iface->handle_playlist_append_playlist	=	handle_playlist_append_playlist;
	_iface->handle_playlist_insert	=	handle_playlist_insert;	
	_iface->handle_playlist_remove	=	handle_playlist_remove;
	_iface->handle_file_get_tag		=	handle_file_get_tag;

	_iface->handle_obj_signal_connect	=	handle_obj_signal_connect;

	_iface->handle_dev_write_ea_data	=	handle_dev_write_ea_data;

	_iface->handle_obj_get_property_type	=	handle_obj_get_property_type;
	_iface->handle_pb_set_pipeline_status	=	handle_pb_set_pipeline_status;	
	_iface->handle_send_signal_touch	=	handle_send_signal_touch;
	_iface->handle_dev_start	=	handle_dev_start;
	_iface->handle_dev_register_status_listener	=	handle_dev_register_status_listener;

	_iface->handle_col_set_favorite	=	handle_col_set_favorite;
	_iface->handle_dev_add_file	=	handle_dev_add_file;
	_iface->handle_dev_detete_file	=	handle_dev_detete_file;
	_iface->handle_misc_set_bluetoothids	=	handle_misc_set_bluetoothids;
	_iface->handle_dev_write_location_data	=	handle_dev_write_location_data;
	_iface->handle_dev_send_vehicle_status	=	handle_dev_send_vehicle_status;
	_iface->handle_misc_set_iap_device_mode	=	handle_misc_set_iap_device_mode;	
	_iface->handle_misc_save_pb				=	handle_misc_save_pb;		
	_iface->handle_misc_carplay_init_modes	=	handle_misc_carplay_init_modes;
	_iface->handle_dev_register_module_status	=	handle_dev_register_module_status;
	_iface->handle_carlife_register_bt_hfp_request	=	handle_carlife_register_bt_hfp_request;
	_iface->handle_core_find_dev	=	handle_core_find_dev;
	_iface->handle_dev_write_bt_data	=	handle_dev_write_bt_data;	
	_iface->handle_dev_send_wifi_conf_info	=	handle_dev_send_wifi_conf_info;
	_iface->handle_misc_set_righthand	=	handle_misc_set_righthand;

	_iface->handle_dev_search_name	=	handle_dev_search_name;

	g_dbus_interface_skeleton_export( G_DBUS_INTERFACE_SKELETON( _object ),
			connection, "/media", &_error );
	if( _error != NULL )
	{
		g_warning( "Create new media connection failed: [ %s ]", _error->message );

		g_error_free( _error );
	}
	else
	{
		g_message( "A new media connection [ %p ] was created", connection );
	}
}		/* -----  end of static function load_media_engine  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_ipc_start_media_engine
 *  Description:  
 * =====================================================================================
 */
MHResult mh_ipc_start_media_engine( MHIPCServer * server )
{
	MHResult _res	=	MH_OK;
	engineList	=	g_list_append( engineList, ( void * )load_media_engine );

	g_message( "Media-Hub v2.0 Media Engine Started" );
	return _res;
}		/* -----  end of function mh_ipc_start_media_engine  ----- */
void log_function  (GstDebugCategory * category, GstDebugLevel      level,
                                 const gchar      * file,
                                 const gchar      * function,
                                 gint               line,
                                 GObject          * object,
                                 GstDebugMessage  * message,
                                 gpointer           user_data)
{
	 const gchar *dbg_msg;

	  dbg_msg = gst_debug_message_get (message);
	  g_message("[%d]%s", level, dbg_msg);

}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_ipc_server_run
 *  Description:  
 * =====================================================================================
 */
MHResult mh_ipc_server_run( MHIPCServer * server )
{
	//////add gst log////
	
//	gst_debug_add_log_function( log_function, NULL, NULL);

	MHResult _res	=	MH_OK;
	GMainLoop * _loop;

	g_dbus_server_start( server->ipc_server );
	if( NULL !=  server->streaming_server)
	{
		g_dbus_server_start( server->streaming_server );
	}

	_loop	=	g_main_loop_new( NULL, FALSE );
	server->listener->callback( server, SERVER_INIT_FINISH, NULL);
	g_main_loop_run( _loop );

	g_object_unref( server->ipc_server );
	if( NULL != server->streaming_server)
	{
		g_object_unref( server->streaming_server);

	}
	g_main_loop_unref( _loop );
	return _res;
}		/* -----  end of function mh_ipc_server_run  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_ipc_server_register_events_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_ipc_server_register_events_listener( MHIPCServer * server, MHServerEventsListener * listener)
{
	g_return_val_if_fail(( server != NULL ) && ( listener != NULL ), MH_INVALID_PARAM);	
	MHResult _res	=	MH_OK;
	server->listener	=	listener;	
	return _res;
}		/* -----  end of function mh_ipc_server_register_events_listener  ----- */
