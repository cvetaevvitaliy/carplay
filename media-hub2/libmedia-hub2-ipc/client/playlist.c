/*
 * =====================================================================================
 *
 *       Filename:  playlist.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:11:43 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <sys/mman.h>
#include <sys/stat.h>        /*  For mode constants */
#include <fcntl.h>           /*  For O_* constants */
#include <stdio.h>
#include <stdlib.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif


extern Mediahub2Dbus * dbusClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_foreach
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_foreach( MHPlaylist * self, int32_t start, int32_t count, MHFunc func, void * user_data )
{
	g_message("client %s start:%d, count:%d", __func__, start, count);
	g_return_val_if_fail(( func != NULL ) && ( self != NULL ) && (start >= 0), MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
	gchar * _shm ;
	guint _size;
	int _shmFd;
	int _fd;
	void * _mem;
	GVariant * _resData;
	GVariantIter * _it;
	GVariant * _detail	=	NULL;
	GVariant * _var	=	NULL;
	const char * _streaming_enble = getenv( "MH_STREAMING_ENABLED");

	if( _streaming_enble == NULL)
	{
	#ifdef __x86_64__
		mediahub2_dbus_call_playlist_foreach_shm_sync( dbusClient, ( guint64 )self, start, count, &_shm, &_size,  (gint*)&_res, NULL, &_error );
	#else

		mediahub2_dbus_call_playlist_foreach_shm_sync( dbusClient, ( guint )self, start, count, &_shm, &_size,  (gint *)&_res,NULL, &_error );
	#endif
		if( _error != NULL )
		{
			_res	=	MH_IPC_ERROR;
			g_warning( "mh_playlist_foreach failed: [ %s ]", _error->message );

			g_error_free( _error );
		}
		else
		{
			_fd	=	shm_open( _shm, O_RDONLY, S_IRUSR | S_IWUSR );

			if( _fd > 0 )
			{
				ftruncate( _fd, _size );

				_mem	=	mmap( NULL, _size, PROT_READ, MAP_SHARED, _fd, 0 );

				if( _mem != MAP_FAILED )
				{
					_var	=	g_variant_new_from_data( G_VARIANT_TYPE( "a(susxxuxuv)"), _mem, _size, TRUE, NULL, NULL );

					if( _var != NULL )
					{
						MHItemData * _data	=	g_new0( MHItemData, 1 );

						g_variant_get( _var, "a(susxxuxuv)", &_it );

						while( g_variant_iter_loop( _it, "(susxxuxuv)", &_data->uri, &_data->type, &_data->name, &_data->size,
								&_data->uniqueId, &_data->valid, &_data->tagId, &_data->favorite, &_detail ))
						{
							switch( _data->type )
							{
							case MH_ITEM_MUSIC:
								g_variant_get( _detail, "(sssssiiiii)", 
									&_data->metadata.music.title,
									&_data->metadata.music.album, 
									&_data->metadata.music.artist,
									&_data->metadata.music.genre, 
									&_data->metadata.music.composer,
									&_data->metadata.music.year,
									&_data->metadata.music.track,
								 	&_data->metadata.music.trackCount,
									&_data->metadata.music.mediaType,
									&_data->metadata.music.duration);
								break;
							case MH_ITEM_MOVIE:
								//TODO
								break;
							case MH_ITEM_PICTURE:
								//TODO
								break;
							default:
								break;
							}

							if( func( _data, user_data ) )
							{
								if (_data->type == MH_ITEM_MUSIC)
								{
									g_free( _data->metadata.music.title );
									g_free( _data->metadata.music.album );
									g_free( _data->metadata.music.artist );
									g_free( _data->metadata.music.genre );
									g_free( _data->metadata.music.composer );
								}
								break;
							}
							if (_data->type == MH_ITEM_MUSIC)
							{
								g_free( _data->metadata.music.title );
								g_free( _data->metadata.music.album );
								g_free( _data->metadata.music.artist );
								g_free( _data->metadata.music.genre );
								g_free( _data->metadata.music.composer );
							}
						}

						g_free( _data );

						g_variant_iter_free( _it );
						g_variant_unref( _var );
					}

					munmap( _mem, _size );
				}

				shm_unlink( _shm );
				close( _fd );
			}
			mediahub2_dbus_call_shm_unlink_sync( dbusClient, _shm, NULL, NULL );

			g_free( _shm );
		}
	}
	else
	{
	#ifdef __x86_64__
		mediahub2_dbus_call_playlist_foreach_sync( dbusClient, ( guint64 )self, start, count, &_resData, (gint*)&_res, NULL, &_error);	
	#else
		mediahub2_dbus_call_playlist_foreach_sync( dbusClient, ( guint )self, start, count, &_resData, (gint*)&_res, NULL, &_error);	
	#endif
		if( _error != NULL)
		{
			_res	=	MH_IPC_ERROR;
			g_warning( "mh_playlist_foreach failed: [ %s ]", _error->message );

			g_error_free( _error );
		}
		else
		{
			MHItemData * _data	=	g_new0( MHItemData, 1 );

			g_variant_get( _resData, "a(susxxuxuv)", &_it );
	
			while( g_variant_iter_loop( _it, "(susxxuxuv)", &_data->uri, &_data->type, &_data->name, &_data->size,
					&_data->uniqueId, &_data->valid, &_data->tagId, &_data->favorite, &_detail ))
			{
				switch( _data->type )
				{
					case MH_ITEM_MUSIC:
						g_variant_get( _detail, "(sssssiiiii)", 
								&_data->metadata.music.title,
								&_data->metadata.music.album, 
								&_data->metadata.music.artist,
								&_data->metadata.music.genre, 
								&_data->metadata.music.composer,
								&_data->metadata.music.year,
								&_data->metadata.music.track,
								&_data->metadata.music.trackCount,
								&_data->metadata.music.mediaType,
								&_data->metadata.music.duration);
						break;
					case MH_ITEM_MOVIE:
						//TODO
						break;
					case MH_ITEM_PICTURE:
						//TODO
						break;
					default:
						break;
				}

				if( func( _data, user_data ) )
				{
					break;
				}
			}
		

			g_free( _data );

			g_variant_iter_free( _it );
			g_variant_unref( _resData );
		}

	}
	return _res;
}		/* -----  end of function mh_playlist_foreach  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_sort
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_sort( MHPlaylist * self, MHSortType sort_type, MHItemOrderType order_type)
{
	g_message("client %s sort_type:%d, order_type:%d", __func__, sort_type, order_type);
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_playlist_sort_sync(dbusClient, (guint64)self, (guint)sort_type,
				(guint)order_type, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_playlist_sort_sync(dbusClient, (guint)self, (guint)sort_type,
				(guint)order_type, (gint *)&_res, NULL, NULL);
#endif
	if(_dbusRes)
	{

	}
	else
	{	
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;

	}
	return _res;
}		/* -----  end of function mh_playlist_sort  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_append
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_append( MHPlaylist * playlist, MHItem ** items, uint32_t count) 
{
	g_return_val_if_fail( playlist != NULL && items != NULL, MH_INVALID_PARAM);
	g_message("client %s count:%d", __func__, count);
	MHResult _res	=	MH_OK;

	GVariant * _items;
	int i;
	gboolean _dbusRes;
	GVariantBuilder * _builder;
#ifdef __x86_64__
	_builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));
	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "t", ( guint64 )items[ i ]);
	}
	_items	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	_dbusRes	=	mediahub2_dbus_call_playlist_append_sync(dbusClient, (guint64)playlist, _items,
				(guint)count, (gint *)&_res, NULL, NULL);

#else
	_builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));
	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "u", ( guint )items[ i ]);
	}
	_items	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	_dbusRes	=	mediahub2_dbus_call_playlist_append_sync(dbusClient, (guint)playlist, _items,
				(guint)count, (gint *)&_res, NULL, NULL);

#endif
	if( ! _dbusRes )
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_playlist_append  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_append_playlist
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_append_playlist( MHPlaylist * self, MHPlaylist * _another)
{
	g_message("client %s ", __func__);
	g_return_val_if_fail((( self != NULL ) && (_another != NULL) ), MH_INVALID_PARAM);
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_playlist_append_playlist_sync(dbusClient, (guint64)self, (guint64)_another, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_playlist_append_playlist_sync(dbusClient, (guint)self, (guint)_another, (gint *)&_res, NULL, NULL);
#endif

	if( ! _dbusRes )
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_playlist_append_playlist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_insert
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_insert( MHPlaylist * self, uint32_t index, MHItem ** items, uint32_t count )
{
	g_message("client %s index:%d, count:%d", __func__, index, count);
	g_return_val_if_fail(( items != NULL ) && ( self != NULL ) && (count > 0), MH_INVALID_PARAM);
	MHResult _res	=	MH_OK;
	GVariant * _items;
	int i;
	gboolean _dbusRes;
	GVariantBuilder * _builder;
#ifdef __x86_64__
	_builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));
	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "t", ( guint64 )items[ i ]);
	}
	_items	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );

	_dbusRes	=	mediahub2_dbus_call_playlist_insert_sync(dbusClient, (guint64)self, (guint)index, _items,
				(guint)count, (gint *)&_res, NULL, NULL);

#else
	_builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));
	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "u", ( guint )items[ i ]);
	}
	_items	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	_dbusRes	=	mediahub2_dbus_call_playlist_insert_sync(dbusClient, (guint)self,(guint)index, _items,
				(guint)count,(gint *)&_res, NULL, NULL);
#endif
	if( ! _dbusRes )
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_playlist_insert  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_playlist_remove
 *  Description:  
 * =====================================================================================
 */
MHResult mh_playlist_remove( MHPlaylist * self, uint32_t index, uint32_t count )
{
	g_message("client %s index:%d, count:%d", __func__, index, count);
	g_return_val_if_fail((( self != NULL ) && (count > 0) ), MH_INVALID_PARAM);
	MHResult _res	=	MH_OK;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_playlist_remove_sync(dbusClient, (guint64)self,(guint)index, 
				(guint)count, (gint *)&_res, NULL, NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_playlist_remove_sync(dbusClient, (guint)self,(guint)index, 
				(guint)count,(gint *)&_res, NULL, NULL);
#endif

	if( ! _dbusRes )
	{
		g_message("%s:ipc error\n",__func__);
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_playlist_remove  ----- */
