/*
 * =====================================================================================
 *
 *       Filename:  collection.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/05/2014 11:06:35 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <mh_col.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif
extern Mediahub2Dbus * dbusClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_create
 *  Description:  
 * =====================================================================================
 */
MHCol * mh_col_create( MHDev * dev )
{
	g_return_val_if_fail( dev != NULL,NULL);

	GError * _error = NULL;

	MHCol * _col	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_create_sync( dbusClient, (guint64) dev, (guint64 *)&_col, NULL, &_error );
#else
	mediahub2_dbus_call_col_create_sync( dbusClient, (guint) dev, (guint *)&_col, NULL, &_error );
#endif
	if( _error != NULL)
	{
		g_warning( "mh_col_create failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	return _col;
}		/* -----  end of function mh_col_create  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_retrieve_data
 *  Description:  
 * =====================================================================================
 */
char ** mh_col_retrieve_data( MHCol * col, MHItemType type, MHMediaType media_type, int32_t * count, bool fuzzy)
{
	g_return_val_if_fail( col != NULL, NULL );

	GError * _error	=	NULL;

	char ** data	=	NULL;

	GVariant * _result;
#ifdef __x86_64__
	mediahub2_dbus_call_col_retrieve_data_sync( dbusClient, (guint64)col, (guint)type, (guint)media_type, 
					 (gint)*count, (gboolean)fuzzy, count, &_result,NULL, &_error);
#else
	mediahub2_dbus_call_col_retrieve_data_sync( dbusClient, (guint)col, (guint)type, (guint)media_type, 
					 (gint)*count, (gboolean)fuzzy, count, &_result,NULL, &_error);
#endif
	if(_error != NULL)
	{
		g_warning( "mh_col_retrieve_data failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	else
	{
		if( *count > 0 )
		{
			int _offset	=	0;
			GVariantIter * _it;
			gchar * _val;

			data	=	g_new0( char *, * count );

			g_variant_get( _result, "a(s)", &_it );

			while( g_variant_iter_loop( _it, "(s)", &_val ))
			{
				data[ _offset ] = g_strdup( _val );
				_offset	++;
			}

			g_variant_unref( _result );
			g_variant_iter_free( _it );

		}
	}
	return data;
}		/* -----  end of function mh_col_retrieve_data  ----- */




/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_create_playlist
 *  Description:  
 * =====================================================================================
 */
 MHPlaylist * mh_col_create_playlist( MHCol * col, MHItemType item_type, MHMediaType media_type, bool fuzzy)
{
	GError * _error	=	NULL;

	MHPlaylist * _playlist	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_create_playlist_sync( dbusClient, (guint64)col, (guint)item_type, 
			(guint) media_type, (gboolean)fuzzy, (guint64 *)&_playlist, NULL,&_error);
#else
	mediahub2_dbus_call_col_create_playlist_sync( dbusClient, (guint)col, (guint)item_type, 
			(guint) media_type, (gboolean)fuzzy, (guint *)&_playlist, NULL,&_error);

#endif
	if(_error != NULL)
	{
		g_warning( "mh_col_create_playlist failed: [ %s ]", _error->message );

		g_error_free( _error );

	}


	return _playlist;

}		/* -----  end of static function mh_col_create_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _col_add_filter
 *  Description:  
 * =====================================================================================
 */
static MHResult _col_add_filter( MHCol * col, MHColFilterType filter_type, va_list varArgs)
{
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "a(uv)" ));
	GVariant * _var;
	MHResult _res;
	GError * _error = NULL;
	while( filter_type != 0)
	{
		switch( filter_type )
		{
			case COL_FILTER_NAME:
			case COL_FILTER_TITLE:
			case COL_FILTER_ARTIST:
			case COL_FILTER_COMPOSER:
			case COL_FILTER_GENRE:
			case COL_FILTER_ALBUM_ARTIST:
				{
			
					char * _param;
					_param	=	va_arg( varArgs, char *);
					_var	=	g_variant_new_string( _param);
				}
				break;

			case COL_FILTER_ALBUM:
				{
					
					MHAlbumInfo * _param;
					_param	=	va_arg( varArgs, MHAlbumInfo *);
					_var	=	g_variant_new("(ssu)", _param->album_title, _param->album_artist, _param->album_compliation );
				}
				break;

			case COL_FILTER_COMPLIATION:
			case COL_FILTER_TRACK:
			case COL_FILTER_YEAR:
			case COL_FILTER_DURATION:
				{
					uint32_t _param;
					_param	=	va_arg( varArgs, uint32_t );
					_var	=	g_variant_new( "u", _param);
				}
				break;
		}
		if( _var != NULL)
		{
			g_variant_builder_add( _builder, "(uv)", filter_type, _var);
		}

		filter_type	=	va_arg(varArgs, uint32_t);
	}

	_var	=	g_variant_builder_end( _builder );

	g_variant_builder_unref( _builder );
#ifdef __x86_64__
	mediahub2_dbus_call_col_add_filter_sync( dbusClient, (guint64) col, _var, &_res, NULL,&_error);
#else
	mediahub2_dbus_call_col_add_filter_sync( dbusClient, (guint) col, _var, &_res, NULL,&_error);
#endif

	if(_error != NULL)
	{
		g_warning( "mh_col_create_playlist failed: [ %s ]", _error->message );

		g_error_free( _error );

	}


	return _res;

}		/* -----  end of static function _col_add_filter  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_add_filter
 *  Description:  
 * =====================================================================================
 */
MHResult mh_col_add_filter( MHCol * col, MHColFilterType type, ...)
{
	va_list _varArgs;

	MHResult _res	=	MH_OK;

	va_start( _varArgs, type);

	_res = _col_add_filter( col, type, _varArgs);

	va_end( _varArgs );

	return _res;

}		/* -----  end of function mh_col_add_filter  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_filter_clear
 *  Description:  
 * =====================================================================================
 */
MHResult mh_col_filter_clear( MHCol * col)
{
	g_return_val_if_fail( col != NULL,MH_INVALID_PARAM);
	MHResult _res;
	GError * _error = NULL;

	MHCol * _col	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_filter_clear_sync( dbusClient, (guint64) col, (guint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_col_filter_clear_sync( dbusClient, (guint) col, (guint *)&_res, NULL, &_error );
#endif

	if( _error != NULL)
	{
		g_warning( "mh_col_create failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	return _res;

}		/* -----  end of function mh_col_filter_clear  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_set_retrieve_key
 *  Description:  
 * =====================================================================================
 */
MHResult mh_col_set_retrieve_key( MHCol * col, MHColFilterType type)
{
	g_return_val_if_fail( col != NULL,MH_INVALID_PARAM);
	MHResult _res;
	GError * _error = NULL;

	MHCol * _col	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_set_retrieve_key_sync(dbusClient, (guint64)col, (guint)type, (guint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_col_set_retrieve_key_sync(dbusClient, (guint)col, (guint)type, (guint *)&_res, NULL, &_error );
#endif
	if( _error != NULL)
	{
		g_warning( "mh_col_create failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	return _res;
}		/* -----  end of function mh_col_set_retrieve_key  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_set_order_type
 *  Description:  
 * =====================================================================================
 */
MHResult mh_col_set_order_type( MHCol * col, MHItemOrderType order)
{
	g_return_val_if_fail( col != NULL,MH_INVALID_PARAM);
	MHResult _res;
	GError * _error = NULL;

	MHCol * _col	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_set_order_type_sync(dbusClient, (guint64)col, (guint)order, (guint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_col_set_order_type_sync(dbusClient, (guint)col, (guint)order, (guint *)&_res, NULL, &_error );
#endif
	if( _error != NULL)
	{
		g_warning( "mh_col_set_order_type failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	return _res;

}		/* -----  end of function mh_col_set_order_type  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_retrieve_album
 *  Description:  
 * =====================================================================================
 */
MHAlbumInfo * mh_col_retrieve_album( MHCol * col, MHItemType item_type, MHMediaType media_type, int * count, bool fuzzy)
{
	g_return_val_if_fail( col != NULL, NULL );
	
	MHAlbumInfo * _res	=	NULL;
	
	GError * _error	=	NULL;
	GVariant * _data;
#ifdef __x86_64__

	mediahub2_dbus_call_col_retrieve_album_sync(dbusClient, (guint64)col, (guint)item_type, (guint)media_type,
			(guint)*count,(gboolean)fuzzy, count, &_data, NULL, &_error);
#else
	mediahub2_dbus_call_col_retrieve_album_sync(dbusClient, (guint)col, (guint)item_type, (guint)media_type,
			(guint)*count,(gboolean)fuzzy, count, &_data, NULL, &_error);
#endif
	if(_error != NULL)
	{
		g_warning( "mh_col_retrieve_album failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	else if( *count > 0)
	{
		int _offset	=	0;
		GVariantIter * _it;
		gchar * _val;
		int i =0;
		gchar * _album_title;
		gchar * _album_artist;
		uint32_t _album_compliation;
		_res	=	(MHAlbumInfo *)g_malloc0(sizeof( MHAlbumInfo )*(*count));
		g_variant_get( _data, "a(ssu)", &_it );
		while( g_variant_iter_loop( _it, "(ssu)", &_album_title, &_album_artist, &_album_compliation ))
		{
			_res[i].album_title	=	g_strdup( _album_title);
			_res[i].album_artist	=	g_strdup( _album_artist);
			_res[i].album_compliation	=	_album_compliation;
			i++;
		}

			g_variant_iter_free( _it );

	}
	else
	{

	}
	return _res;

}		/* -----  end of static function mh_col_retrieve_album  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_col_set_favorite
 *  Description:  
 * =====================================================================================
 */
MHResult mh_col_set_favorite( MHCol * col, MHItemType item_type, bool favorite)
{
	g_return_val_if_fail( col != NULL,MH_INVALID_PARAM);
	MHResult _res;
	GError * _error = NULL;

	MHCol * _col	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_col_set_favorite_sync(dbusClient, (guint64)col, (guint)item_type, (guint)favorite, NULL, &_error );
#else
	mediahub2_dbus_call_col_set_favorite_sync(dbusClient, (guint)col,  (guint)item_type, (guint)favorite, NULL, &_error );
#endif
	if( _error != NULL)
	{
		g_warning( "mediahub2_dbus_call_col_set_favorite_sync failed: [ %s ]", _error->message );

		g_error_free( _error );

	}
	return _res;
}	/* -----  end of function mh_col_set_favorite  ----- */
