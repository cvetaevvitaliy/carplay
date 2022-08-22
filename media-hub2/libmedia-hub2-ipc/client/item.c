/*
 * =====================================================================================
 *
 *       Filename:  item.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:07:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>        /*  For mode constants */
#include <fcntl.h>           /*  For O_* constants */
#include <stdlib.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif

extern Mediahub2Dbus * dbusClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_item_foreach
 *  Description:  
 * =====================================================================================
 */
MHResult mh_item_foreach( MHItem ** items, int32_t count, MHItemFunc func, void * user_data )
{
	g_return_val_if_fail(( func != NULL ) && ( items != NULL ) && ( count > 0 ), MH_INVALID_PARAM);
	MHResult _res	=	MH_OK;

	GVariant * _items, * _resData;
	gchar * _shm;
	gboolean _result;
	guint _size;
	int _shmFd;
	int i;
#ifdef __x86_64__
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "at" ));

	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "t", ( guint64 )items[ i ]);
	}

#else
	GVariantBuilder * _builder	=	g_variant_builder_new( G_VARIANT_TYPE( "au" ));

	for( i = 0; i < count; i ++ )
	{
		g_variant_builder_add( _builder, "u", ( guint )items[ i ]);
	}
#endif
	_items	=	g_variant_builder_end( _builder );
	g_variant_builder_unref( _builder );
	
	const char * _streaming_enble = getenv( "MH_STREAMING_ENABLED");
	if( _streaming_enble == NULL)
	{
		_result	=	mediahub2_dbus_call_item_foreach_shm_sync( dbusClient, _items, &_shm, &_size, (gint *)&_res, NULL, NULL );

		if( _result )
		{
			int _fd;
			void * _mem;
			GVariantIter * _it;
			MHItem * _item;
			gchar * _uri, * _name;
			MHItemType _type;
			GVariant * _detail, * _var;

			_fd	=	shm_open( _shm, O_RDONLY, S_IRUSR | S_IWUSR );

			if( _fd > 0 )
			{
				ftruncate( _fd, _size );

				_mem	=	mmap( NULL, _size, PROT_READ, MAP_SHARED, _fd, 0 );
				gint64 _mediaId;

				if( _mem != MAP_FAILED )
				{
#ifdef __x86_64__

					_var	=	g_variant_new_from_data( G_VARIANT_TYPE( "a(tsusxutv)"), _mem, _size, TRUE, NULL, NULL );
					if( _var != NULL )
					{
						MHItemData * _data	=	g_new0( MHItemData, 1 );
						g_variant_get( _var, "a(tsusxutv)", &_it );

						while( g_variant_iter_loop( _it, "(tsusxutv)", ( guint * )&_item, &_data->uri, 
									&_data->type, &_data->name, &_data->uniqueId, &_data->favorite, &_mediaId,&_detail ))
						{
							if( _data->type	==	MH_ITEM_MUSIC && _mediaId != 0)
							{
								g_variant_get( _detail, "(sssssuuuuu)", &(_data->metadata.music.title), &(_data->metadata.music.album),
									&(_data->metadata.music.artist), &(_data->metadata.music.genre), &(_data->metadata.music.composer), 
									&(_data->metadata.music.year), &(_data->metadata.music.track),	&(_data->metadata.music.trackCount),
									&(_data->metadata.music.mediaType), &(_data->metadata.music.duration));
							}
							if( func( _item, _data, user_data ))
							{
								break;
							}
						}

#else
					_var	=	g_variant_new_from_data( G_VARIANT_TYPE( "a(ususxutv)"),_mem, _size, TRUE, NULL, NULL );

					if( _var != NULL )
					{
						MHItemData * _data	=	g_new0( MHItemData, 1 );

						g_variant_get( _var, "a(ususxutv)", &_it );

						while( g_variant_iter_loop( _it, "(ususxutv)", ( guint * )&_item, &_data->uri, 
									&_data->type, &_data->name, &_data->uniqueId, &_data->favorite, &_mediaId, &_detail ))
						{
							if( _data->type	==	MH_ITEM_MUSIC && _mediaId != 0)
							{
								g_variant_get( _detail, "(sssssuuuuu)", &(_data->metadata.music.title), &(_data->metadata.music.album),
										&(_data->metadata.music.artist), &(_data->metadata.music.genre), &(_data->metadata.music.composer), 
										&(_data->metadata.music.year), &(_data->metadata.music.track),	&(_data->metadata.music.trackCount),
										&(_data->metadata.music.mediaType), &(_data->metadata.music.duration));
							}

							if( func( _item, _data, user_data ))
							{
								break;
							}
						}
#endif
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
		else
		{
			_res	=	MH_IPC_ERROR;
		}
	}
	else
	{
		_result	=	mediahub2_dbus_call_item_foreach_sync( dbusClient, _items, &_resData, (gint *)&_res,NULL, NULL );

		if( _result )
		{
			GVariantIter * _it;
			MHItem * _item;
			gchar * _uri, * _name;
			MHItemType _type;
			GVariant * _detail ;
			if( _resData != NULL )
			{
				MHItemData * _data	=	g_new0( MHItemData, 1 );

				g_variant_get(_resData, "a(ususxuv)", &_it );

				while( g_variant_iter_loop( _it, "(ususxuv)", ( guint * )&_item, &_data->uri, 
							&_data->type, &_data->name, &_data->uniqueId, &_data->favorite, &_detail ))
				{
					if( func( _item, _data, user_data ))
					{
						break;
					}
				}

				g_free( _data );

				g_variant_iter_free( _it );
				g_variant_unref( _resData );
			}
		}
		else
		{
			_res	=	MH_IPC_ERROR;
		}
	}
	return _res;
}		/* -----  end of function mh_item_foreach  ----- */
