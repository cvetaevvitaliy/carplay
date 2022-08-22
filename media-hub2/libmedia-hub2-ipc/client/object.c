/*
 * =====================================================================================
 *
 *       Filename:  object.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/07/2014 01:22:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <mh_api.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif


extern Mediahub2Dbus * dbusClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_get
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_get( MHObject * object, const char * first_property_name, va_list varArgs )
{
	MHResult _res	=	MH_OK;
	guint _propCount	=	0;
	guint _valCount	=	0;
	void ** _retValues	=	NULL;
	void * _val;
	gchar ** _propLists	=	NULL;
	gboolean _isName	=	FALSE;
	gboolean _ret;
	GVariant * _retVar;


	_propCount	++;
	_propLists	=	g_renew( gchar *, _propLists, _propCount );
	_propLists[ _propCount - 1 ]	=	( gchar * )first_property_name;

	while(( _val = va_arg( varArgs, void * )) != NULL )
	{
		if( _isName )
		{
			_propCount	++;
			_propLists	=	g_renew( gchar *, _propLists, _propCount );
			_propLists[ _propCount - 1 ]	=	_val;
		}
		else
		{
			_valCount	++;
			_retValues	=	g_renew( void *, _retValues, _valCount );
			_retValues[ _valCount - 1 ]	=	_val;
		}

		_isName	=	!_isName;
	}

	_propCount	++;
	_propLists	=	g_renew( gchar *, _propLists, _propCount );
	_propLists[ _propCount - 1 ]	=	NULL;
#ifdef __x86_64__
	g_message("client.c--->call mediahub2_dbus_call_obj_get_properties_sync\n\n");
	_ret	=	mediahub2_dbus_call_obj_get_properties_sync( dbusClient, ( guint64 )object, ( const gchar * const * )_propLists, 
			&_retVar, (gint *)&_res, NULL, NULL );

#else
	_ret	=	mediahub2_dbus_call_obj_get_properties_sync( dbusClient, ( guint )object, ( const gchar * const * )_propLists, 
			&_retVar, (gint *)&_res, NULL, NULL );
#endif
	if( _ret )
	{
		GVariantIter * _it;
		GVariant * _valVar;
		int _offset	=	0;

		g_variant_get( _retVar, "av", &_it );

		while( g_variant_iter_loop( _it, "v", &_valVar ))
		{
			const GVariantType * _type	=	g_variant_get_type( _valVar );
			switch( g_variant_type_peek_string( _type )[0] )
			{
			case G_VARIANT_CLASS_BOOLEAN:
				*(( bool * )_retValues[ _offset ])	=	g_variant_get_boolean( _valVar );
				break;
			case G_VARIANT_CLASS_BYTE:
				*(( uint8_t * )_retValues[ _offset ])	=	g_variant_get_byte( _valVar );
				break;
			case G_VARIANT_CLASS_INT16:
				*(( int16_t * )_retValues[ _offset ])	=	g_variant_get_int16( _valVar );
				break;
			case G_VARIANT_CLASS_UINT16:
				*(( uint16_t * )_retValues[ _offset ])	=	g_variant_get_uint16( _valVar );
				break;
			case G_VARIANT_CLASS_INT32:
				*(( int32_t * )_retValues[ _offset ])	=	g_variant_get_int32( _valVar );
				break;
			case G_VARIANT_CLASS_UINT32:
				*(( uint32_t * )_retValues[ _offset ])	=	g_variant_get_uint32( _valVar );
				break;
			case G_VARIANT_CLASS_INT64:
				*(( int64_t * )_retValues[ _offset ])	=	g_variant_get_int64( _valVar );
				break;
			case G_VARIANT_CLASS_UINT64:
				*(( uint64_t * )_retValues[ _offset ])	=	g_variant_get_uint64( _valVar );
				break;
			case G_VARIANT_CLASS_DOUBLE:
				*(( double * )_retValues[ _offset ])	=	g_variant_get_double( _valVar );
				break;
			case G_VARIANT_CLASS_STRING:
				{
					const gchar * _str	=	g_variant_get_string( _valVar, NULL );
					*(( char ** )_retValues[ _offset ])	=	g_strdup( _str );
				}
				break;
			case G_VARIANT_CLASS_VARIANT:
					*((GVariant **)_retValues[_offset])	=	g_variant_get_variant( _valVar);
				break;
			default:
				g_warning( "%s: Un-catched return value: %s", __func__, ( gchar *)_type );

				break;
			}

			_offset	++;
		}

		g_variant_iter_free( _it );
		g_variant_unref( _retVar );
	}
	else
	{
		_res	=	MH_IPC_ERROR;
		g_message("mediahub2_dbus_call_obj_get_properties_sync is failed");
	}

	g_free( _propLists );
	g_free( _retValues );

	return _res;
}		/* -----  end of function mh_object_get  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_set
 *  Description:  
 * =====================================================================================
 */
MHResult  mh_object_set( void * object, const char * first_property_name, va_list varArgs )
{
	MHResult _res	=	MH_OK;
	
	const char * _property_name	=	first_property_name;
	gboolean _ret;
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(sv)" ));
	GVariant * _setVar;
	while( _property_name	!=	NULL)
	{
#ifdef __x86_64__
		guint64 _type;
		_ret	=	mediahub2_dbus_call_obj_get_property_type_sync( dbusClient, 
			( guint64 )object, ( const gchar * )_property_name, &_type, NULL, NULL );
#else
		guint _type;
		_ret	=	mediahub2_dbus_call_obj_get_property_type_sync( dbusClient, 
			( guint )object, ( const gchar * )_property_name, &_type, NULL, NULL );
#endif
		if( _ret )
		{
			switch( _type )
			{
				case G_TYPE_BOOLEAN:
					{
						gboolean _property_value;
						_property_value	=	va_arg(varArgs, gboolean);
						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_boolean( _property_value ) );
					}
					break;
				case G_TYPE_UCHAR:
					g_message( "type uchar isn't supported now" );
					break;
				case G_TYPE_INT:
					{
						gint _property_value;

						_property_value	=	va_arg(varArgs, gint);

						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_int32( _property_value ) );
					}
					break;
				case G_TYPE_UINT:
					{
						guint _property_value;

						_property_value	=	va_arg(varArgs, guint);

						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_uint32( _property_value ) );
					}
					break;
				case G_TYPE_INT64:
					g_message( "type int64 isn't supported now" );
					break;
				case G_TYPE_UINT64:
					g_message( "type uint64 isn't supported now" );
					break;
				case G_TYPE_DOUBLE:
					{
						gdouble _property_value;

						_property_value	=	va_arg(varArgs, gdouble);

						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_double ( _property_value ) );
					}
					g_message( "type double isn't supported now" );
					break;
				case G_TYPE_STRING:
					{
						gchar * _property_value;

						_property_value	=	va_arg(varArgs, gchar *);

						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_string( _property_value ) );
					}
					break;
				case G_TYPE_POINTER:
					g_message( "type pointer isn't supported now" );
					break;
				case G_TYPE_VARIANT:
					{
						GVariant * _property_value;

						_property_value	=	va_arg( varArgs, GVariant *);

						g_variant_builder_add( _builder, "(sv)", 
								_property_name, g_variant_new_variant ( _property_value));
					}
					break;	
				case G_TYPE_INVALID:
					va_arg( varArgs, char *);
					break;
			}
		}
		_property_name	=	va_arg( varArgs, char *);
	}
	_setVar	=	g_variant_builder_end( _builder );

	g_variant_builder_unref( _builder );
#ifdef __x86_64__
	if( ! mediahub2_dbus_call_obj_set_properties_sync( dbusClient, ( guint64 )object, _setVar, (gint *)&_res, NULL, NULL ))
	{
		_res	=	MH_IPC_ERROR;
	}
#else
	if( ! mediahub2_dbus_call_obj_set_properties_sync( dbusClient, ( guint )object, _setVar, (gint *)&_res, NULL, NULL ))
	{
		_res	=	MH_IPC_ERROR;
	}
#endif
	return _res;
}		/* -----  end of function mh_object_set  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_get_properties
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_get_properties( MHObject * object, const char * first_property_name, ... )
{
	g_message("client->%s->object:%p->first_property_name:%s",__func__, object, first_property_name);
	va_list _varArgs;
	MHResult _res	= 	MH_OK;

	if( object != NULL)
	{

		va_start( _varArgs, first_property_name );

		_res	=	mh_object_get( object, first_property_name, _varArgs );

		va_end( _varArgs );
	}

	return _res;
}		/* -----  end of function mh_object_get_properties  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_set_properties
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_set_properties( MHObject * object, const char * first_property_name, ... )
{
	g_message("client->%s->object:%p->first_property_name:%s",__func__, object, first_property_name);
	MHResult _res	=	MH_OK;
	va_list _varArgs;

	va_start( _varArgs, first_property_name );

	_res	=	mh_object_set( object, first_property_name, _varArgs );

	va_end( _varArgs );

	return _res;
}		/* -----  end of function mh_object_set_properties  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_unref
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_unref( MHObject * object )
{
	g_message("client->%s->object:%p",__func__, object );
	GError * _error	=	NULL;
	MHResult _res	=	MH_OK;
#ifdef __x86_64__
	mediahub2_dbus_call_obj_unref_sync( dbusClient, ( guint64 )object, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_obj_unref_sync( dbusClient, ( guint )object, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__,  _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_object_unref  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_ref
 *  Description:  
 * =====================================================================================
 */
MHObject * mh_object_ref( MHObject * object )
{
	g_message("client->%s->object:%p",__func__, object );
	GError * _error	=	NULL;
	MHObject * _object;
#ifdef __x86_64__
	mediahub2_dbus_call_obj_ref_sync( dbusClient, ( guint64 )object, ( guint64 * )&_object, NULL, &_error );
#else
	mediahub2_dbus_call_obj_ref_sync( dbusClient, ( guint )object, ( guint * )&_object, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__, _error->message );

		g_error_free( _error );

		return NULL;
	}

	return _object;
}		/* -----  end of function mh_object_ref  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_signal_connect
 *  Description:  
 * =====================================================================================
 */
uint64_t mh_object_signal_connect( MHObject * object, const char * signal, void * handler, void * user_data )
{
	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_obj_signal_connect_sync( dbusClient, ( guint64 )object, signal, NULL, &_error );
#else

	mediahub2_dbus_call_obj_signal_connect_sync( dbusClient, ( guint )object, signal, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__, _error->message );

		g_error_free( _error );

		return 0;
	}

	return g_signal_connect( dbusClient, signal, G_CALLBACK( handler ), user_data );
}		/* -----  end of function mh_object_signal_connect  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_signal_disconnect
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_signal_disconnect( uint64_t signal_id )
{
	MHResult _res	=	MH_OK;

	g_signal_handler_disconnect( dbusClient, signal_id );

	return  _res;
}		/* -----  end of function mh_object_signal_disconnect  ----- */

