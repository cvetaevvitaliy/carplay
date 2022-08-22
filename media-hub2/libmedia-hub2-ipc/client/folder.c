/*
 * =====================================================================================
 *
 *       Filename:  folder.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:05:29 PM
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
 *         Name:  mh_folder_get_children
 *  Description:  
 * =====================================================================================
 */
MHItem ** mh_folder_get_children( MHFolder * self, MHFilter * filter, MHFolderPosition pos, int32_t * count, MHItemOrderType order )
{
	g_return_val_if_fail( self != NULL && filter != NULL && count != NULL , NULL);
	if( *count == 0)
	{
		g_message("mh_folder_get_children input error *count == 0");
		return NULL;
	}
	GVariant * _result;
	MHItem ** _items	=	NULL;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_folder_get_children_sync( dbusClient, ( guint64 )self, ( guint64 )filter, pos, * count,
			order, count, &_result, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_folder_get_children_sync( dbusClient, ( guint )self, ( guint )filter, pos, * count,
			order, count, &_result, NULL, NULL );

#endif
	if( _dbusRes)
	{
		if( * count > 0 )
		{
			int _offset	=	0;
			GVariantIter * _it;
		

			_items	=	g_new0( MHItem *, * count );
#ifdef __x86_64__
			guint64 _val;
			g_variant_get( _result, "at", &_it );

			while( g_variant_iter_loop( _it, "t", &_val ))
			{
				*( _items + _offset )	=	( MHItem * )_val;

				_offset	++;
			}

#else
			guint _val;
			g_variant_get( _result, "au", &_it );

			while( g_variant_iter_loop( _it, "u", &_val ))
			{
				*( _items + _offset )	=	( MHItem * )_val;

				_offset	++;
			}
#endif
			g_variant_unref( _result );
			g_variant_iter_free( _it );
		}
		else
		{
			g_message(" client->folder.c:* count:%d", *count);
		}
	}
	else
	{
		* count	=	0;
	}

	return _items;
}		/* -----  end of function mh_folder_get_children  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_folder_create_empty_playlist
 *  Description:  
 * =====================================================================================
 */
MHPlaylist * mh_folder_create_empty_playlist( MHFolder * self )
{
	return NULL;
}		/* -----  end of function mh_folder_create_empty_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_folder_create_playlist
 *  Description:  
 * =====================================================================================
 */
MHPlaylist * mh_folder_create_playlist( MHFolder * self, MHFilter * filter, bool recursive )
{
	g_return_val_if_fail( self != NULL && filter !=	NULL , NULL);
	
	gboolean _dbusRes;
#ifdef __x86_64__
	guint64 _playlist;
	_dbusRes	=	mediahub2_dbus_call_folder_create_playlist_sync( dbusClient, ( guint64 )self, ( guint64 )filter, 
				recursive, (guint64 *)&_playlist, NULL, NULL );
#else
	guint _playlist;
	_dbusRes	=	mediahub2_dbus_call_folder_create_playlist_sync( dbusClient, ( guint )self, ( guint )filter, 
				recursive, (guint *)&_playlist, NULL, NULL );

#endif
	if( _dbusRes)
	{
		return ( MHPlaylist * )_playlist;
	}

	return NULL;
}		/* -----  end of function mh_folder_create_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_folder_seek
 *  Description:  
 * =====================================================================================
 */
MHResult mh_folder_seek( MHFolder * self, MHFilter * filter, int32_t pos, MHItemOrderType order )
{
	g_return_val_if_fail( self  !=	NULL, MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_folder_seek_sync( dbusClient, (guint64)self, (guint64)filter,
			(guint)pos, (guint)order, (gint *)&_res, NULL, NULL );
#else
	_dbusRes	=	mediahub2_dbus_call_folder_seek_sync( dbusClient, (guint)self, (guint)filter,
			(guint)pos, (guint)order, (gint *)&_res, NULL, NULL );

#endif
	if( _dbusRes)
	{
	}
	else
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;

}		/* -----  end of function mh_folder_seek  ----- */
