/*
 * =====================================================================================
 *
 *       Filename:  playback.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/27/2014 04:45:00 PM
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

typedef struct _MHPbDBusInfo 
{
	MHPb * pb;
	MHPbEventsListener listener;
} MHPbDBusInfo;				/* ----------  end of struct MHPbDBusInfo  ---------- */

typedef struct _MHPbStatusDBusInfo 
{
	MHPb * pb;
	MHPbStatusListener listener;
} MHPbStatusDBusInfo;		/* ----------  end of struct MHPbStatusDBusInfo  ---------- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_play_by_list
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_play_by_list( MHPb * self, MHPlaylist * playlist, uint32_t index )
{
	g_return_val_if_fail( self != NULL && playlist != NULL, MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;
	
	GError * _error	=	NULL;

#ifdef __x86_64__
	mediahub2_dbus_call_pb_play_by_list_sync( dbusClient, ( guint64 )self, ( guint64 )playlist, index,
			(gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_play_by_list_sync( dbusClient, ( guint )self, ( guint )playlist, index,
			(gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_play_by_list failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_play_by_list  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_play_radio_by_index
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_play_radio_by_index( MHDev * self, MHPb * pb, uint32_t index )
{
	g_return_if_fail( self != NULL && pb != NULL );
	MHResult _res	=	MH_OK;
	GError * _error	=	NULL;
	
#ifdef __x86_64__
	mediahub2_dbus_call_pb_play_radio_by_index_sync( dbusClient, ( guint64 )self, ( guint64 )pb, index,
			NULL, &_error );
#else
	mediahub2_dbus_call_pb_play_radio_by_index_sync( dbusClient, ( guint )self, ( guint )pb, index,
			NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_play_radio_by_index failed: [ %s ]", _error->message );

		g_error_free( _error );
		
		_res	=	MH_IPC_ERROR;
	}
	
	return _res;
}		/* -----  end of function mh_pb_play_radio_by_index  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_next
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_next( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_next_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_next_sync( dbusClient, ( guint )self,  (gint *)&_res, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_next failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_next  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_previous
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_previous( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_previous_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_previous_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_previous failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_previous  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_stop
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_stop( MHPb * self )
{
	g_message("client->%s->%p", __func__, self);
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_stop_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_stop_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_stop failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_stop  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_close
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_close( MHPb * self )
{
	g_message("client->%s->%p", __func__, self);
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_close_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_close_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_close failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_close  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_play
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_play( MHPb * self )
{
	g_message("client->%s->%p", __func__, self);
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_play_sync( dbusClient, ( guint64 )self,(gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_play_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_play failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_play  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_pause
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_pause( MHPb * self )
{
	g_message("client->%s->%p", __func__, self);
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_pause_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_pause_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_pause failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_play_pause
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_play_pause( MHPb * self )
{
	MHResult _res	=	MH_OK;
	GError * _error	=	NULL;
	
#ifdef __x86_64__
	mediahub2_dbus_call_pb_play_pause_sync( dbusClient, ( guint64 )self, NULL, &_error );
#else
	mediahub2_dbus_call_pb_play_pause_sync( dbusClient, ( guint )self, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_play_pause failed: [ %s ]", _error->message );

		g_error_free( _error );
		
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_play_pause  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_seek
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_seek( MHPb * self, uint32_t second )
{
	g_message("client->%s->%p", __func__, self);
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_seek_sync( dbusClient, ( guint64 )self, second, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_seek_sync( dbusClient, ( guint )self, second, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_seek failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_seek  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_forward
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_forward( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_forward_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_forward_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_forward failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_forward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_forward_done
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_forward_done( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_forward_done_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_forward_done_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_forward_done failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_forward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_backward
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_backward( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_backward_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_backward_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_backward failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_backward  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_backward_done
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_backward_done( MHPb * self )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_backward_done_sync( dbusClient, ( guint64 )self, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_backward_done_sync( dbusClient, ( guint )self, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_backward_done failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_backward_done  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_create
 *  Description:  
 * =====================================================================================
 */
MHPb * mh_pb_create()
{
	GError * _error	=	NULL;
	MHPb * _pb	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_create_sync( dbusClient, ( guint64 * )&_pb, NULL, &_error );
#else
	mediahub2_dbus_call_pb_create_sync( dbusClient, ( guint * )&_pb, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_create failed: [ %s ]", _error->message );

		g_error_free( _error );
	}
	g_message("%s->%p", __func__, _pb);
	return _pb;
}		/* -----  end of function mh_pb_create  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _pb_events
 *  Description:  
 * =====================================================================================
 */
static void _pb_events( Mediahub2Dbus * proxy, MHPb * self, MHPbInfoType type, GVariant * data, gpointer user_data )
{
	MHPbDBusInfo * _info	=	( MHPbDBusInfo * )user_data;
	GVariant * _var	=	g_variant_get_variant( data );
	MHPbInfoData _data;

	if( self == _info->pb )
	{
		switch( type )
		{
		case MH_PB_INFO_PTIME_CHANGE:
			g_variant_get( _var, "(uu)", &_data.time_info.current_time, &_data.time_info.duration );
			break;
		case MH_PB_INFO_TAG:
			g_variant_get( _var, "(ssss)", &_data.tag_info.title, &_data.tag_info.artist, &_data.tag_info.album, &_data.tag_info.genre);
			break;
		case MH_PB_INFO_EOS:
			break;
		case MH_PB_INFO_ERROR:
			break;
		case MH_PB_INFO_TRACK_TOP:
			g_variant_get( _var, "(uss)", &_data.track_info.index, &_data.track_info.uri, &_data.track_info.name );
			break;
		case MH_PB_INFO_PLAYLIST_CHANGE:
			{
#ifdef __x86_64__
			guint64 _tmp;
			g_variant_get( _var, "t", &_tmp );
#else
			guint _tmp;
			g_variant_get( _var, "u", &_tmp );
#endif
			_data.playlist	=	( MHPlaylist * )_tmp;
			}
			break;
		case MH_PB_INFO_ERROR_NOT_EXIST:
			break;

		case MH_PB_IP_INFO_PTIME_CHANGE: 
			g_variant_get( _var, "u", &_data.ptime );
			break;
		case MH_PB_IP_INFO_QUEUE_INDEX:
			g_variant_get( _var, "u", &_data.index );
			break;
		case MH_PB_IP_INFO_MEDIA:
			g_variant_get( _var, "(suusuuuussssx)", &_data.media_info.title, &_data.media_info.rating, &_data.media_info.duration,
				&_data.media_info.album_title, &_data.media_info.track, &_data.media_info.track_count, &_data.media_info.disc,
				&_data.media_info.disc_count, &_data.media_info.artist, &_data.media_info.album_artist, &_data.media_info.genre,
				&_data.media_info.composer, &_data.media_info.tagId );
			break;	
		case MH_PB_IP_INFO_REPEAT_MODE:
			g_variant_get( _var, "u", &_data.repeat_mode );
			break;
		case MH_PB_IP_INFO_SHUFFLE_MODE:
			g_variant_get( _var, "u", &_data.shuffle_mode );
			break;
		case MH_PB_IP_INFO_COVER_PATH:
			g_variant_get( _var, "s", &_data.cover_path );
			break;
		case MH_PB_IP_INFO_APP_NAME:
			g_variant_get( _var, "s", &_data.app_name );
			break;
		case MH_PB_IP_INFO_DEVICE_NAME:
			g_variant_get( _var, "s", &_data.device_name );
			break;
		case MH_PB_IP_INFO_FUNC_UNSUPPORT:
			break;
		case MH_PB_INFO_STATE_ERROR:
			break;
		case MH_PB_IP_INFO_SHUFFLE_LIST:
			{
				GVariantIter * _it;
				GVariant * _v_list;
				guint _seq_data;
				guint _i = 0;

				g_variant_get( _var, "(uv)", &_data.sf_list_info.list_count, &_v_list);
				g_variant_get( _v_list, "a(u)", &_it);

				_data.sf_list_info.shuffle_seq	=	g_new0( uint32_t, _data.sf_list_info.list_count );

				while( g_variant_iter_loop( _it,"(u)",&_seq_data))
				{
					_data.sf_list_info.shuffle_seq[_i]	=	_seq_data;
					_i++;
				}
				g_variant_iter_free ( _it );
			}
			break;
		case MH_PB_IP_INFO_CALL_STATE_UPDATE:
			g_variant_get( _var, "(ssuusssuusu)", &_data.call_state_info.remoteID, &_data.call_state_info.displayName, &_data.call_state_info.status,
				&_data.call_state_info.direction, &_data.call_state_info.callUUID, &_data.call_state_info.addressBookID, &_data.call_state_info.label,
				&_data.call_state_info.service, &_data.call_state_info.isConferenced, &_data.call_state_info.conferenceGroup, 
				&_data.call_state_info.disconnectReason );
			break;	
		case MH_PB_IP_INFO_RECENTS_LIST_UPDATES:
			{
				GVariantIter * _it;
				GVariant 	 * _v_list;
				guint _i = 0;

				g_variant_get( _var, "(uuv)", &_data.recentslist_updates.recentsListAvailable, 
						  					  &_data.recentslist_updates.recentsListCount, &_v_list);

				g_variant_get( _v_list, "a(ussssuutuu)", &_it );

				_data.recentslist_updates.recentList	=	( MHPbRecentListInfo *)g_malloc0( sizeof(MHPbRecentListInfo) 
						                                           *( _data.recentslist_updates.recentsListCount + 1));
				while( g_variant_iter_loop( _it,"(ussssuutuu)",
					&_data.recentslist_updates.recentList[_i].index,
					&_data.recentslist_updates.recentList[_i].remoteID,
					&_data.recentslist_updates.recentList[_i].displayName,
					&_data.recentslist_updates.recentList[_i].label,
					&_data.recentslist_updates.recentList[_i].addressBookID,
					&_data.recentslist_updates.recentList[_i].service, 
					&_data.recentslist_updates.recentList[_i].type,
					&_data.recentslist_updates.recentList[_i].unixTimestamp,
					&_data.recentslist_updates.recentList[_i].duration,
					&_data.recentslist_updates.recentList[_i].occurrences))
				{
					_i++;
				}

				g_variant_iter_free ( _it );
			}
			break;
		case MH_PB_IP_INFO_FAVORITES_LIST_UPDATES:
			{
				GVariantIter * _it;
				GVariant 	 * _v_list;
				guint _i = 0;

				g_variant_get( _var, "(uuv)", &_data.favoriteslist_updates.favoritesListAvailable, 
						  					  &_data.favoriteslist_updates.favoritesListCount, &_v_list);

				g_variant_get( _v_list, "a(ussssu)", &_it);

				_data.favoriteslist_updates.favoritesList	=	(MHPbFavoritesListInfo *)g_malloc0( sizeof(MHPbFavoritesListInfo) 
						                                           *( _data.favoriteslist_updates.favoritesListCount + 1) );
				while( g_variant_iter_loop( _it,"(ussssu)",
					&_data.favoriteslist_updates.favoritesList[_i].index,
					&_data.favoriteslist_updates.favoritesList[_i].remoteID,
					&_data.favoriteslist_updates.favoritesList[_i].displayName,
					&_data.favoriteslist_updates.favoritesList[_i].label,
					&_data.favoriteslist_updates.favoritesList[_i].addressBookID,
					&_data.favoriteslist_updates.favoritesList[_i].service))
				{
					_i++;
				}

				g_variant_iter_free ( _it );
			}
			break;
		case MH_PB_IP_INFO_PLAYBACK_SPEED: 
			g_variant_get( _var, "u", &_data.speed );
			break;
//		case MH_PB_FREQUENCY_ANALYSIS_RESULT:
//			{
//				GVariantIter * _it;
//				GVariant * _v_list;
//				double _seq_data;
//				double _seq_data2;
//				guint _i = 0;
//
//				g_variant_get( _var, "(uv)", &_data.frequency_analysis_result.band, &_v_list);
//				g_variant_get( _v_list, "a(dd)", &_it);
//
//				_data.frequency_analysis_result.bands	=	g_new0( double, _data.frequency_analysis_result.band );
//				_data.frequency_analysis_result.amplitudes	=	g_new0( double, _data.frequency_analysis_result.band );
//
//				while( g_variant_iter_loop( _it,"(dd)",&_seq_data, &_seq_data2))
//				{
//					_data.frequency_analysis_result.bands[_i]	=	_seq_data;
//					_data.frequency_analysis_result.amplitudes[_i]	=	_seq_data2;
//					_i++;
//				}
////				_i=0;
////				while( g_variant_iter_loop( _it,"(d)",&_seq_data))
////				{
////					_data.frequency_analysis_result.amplitudes[_i]	=	_seq_data;
////					_i++;
////				}
//				g_variant_iter_free ( _it );
//			}
//			break;
		default:
			break;
		}

		_info->listener.callback( _info->pb, type, &_data, _info->listener.user_data );

		switch( type )
		{
			case MH_PB_INFO_PTIME_CHANGE:
				break;
			case MH_PB_INFO_TAG:
				g_free( _data.tag_info.title );
				g_free( _data.tag_info.artist );
				g_free( _data.tag_info.album );
				g_free( _data.tag_info.genre );
				break;
			case MH_PB_INFO_EOS:
				break;
			case MH_PB_INFO_ERROR:
				break;
			case MH_PB_INFO_TRACK_TOP:
				g_free( _data.track_info.uri );
				g_free( _data.track_info.name );
				break;
			case MH_PB_IP_INFO_MEDIA:
				g_free( _data.media_info.title );
				g_free( _data.media_info.album_title );
				g_free( _data.media_info.artist );
				g_free( _data.media_info.album_artist );
				g_free( _data.media_info.genre );
				g_free( _data.media_info.composer );
				break;
			case MH_PB_IP_INFO_COVER_PATH:
				g_free( _data.cover_path );
				break;
			case MH_PB_IP_INFO_APP_NAME:
				g_free( _data.app_name );
				break;
			case MH_PB_IP_INFO_DEVICE_NAME:
				g_free( _data.device_name );
				break;
			case MH_PB_IP_INFO_SHUFFLE_LIST:
				g_free( _data.sf_list_info.shuffle_seq );
				break;
			case MH_PB_IP_INFO_CALL_STATE_UPDATE:
				g_free( _data.call_state_info.remoteID );
				g_free( _data.call_state_info.displayName );
				g_free( _data.call_state_info.callUUID );
				g_free( _data.call_state_info.addressBookID );
				g_free( _data.call_state_info.label );
				g_free( _data.call_state_info.conferenceGroup );
				break;
			case MH_PB_IP_INFO_RECENTS_LIST_UPDATES:
				{
					int i;
					for( i=0; i<_data.recentslist_updates.recentsListCount; i++)
					{
						g_free( _data.recentslist_updates.recentList[i].remoteID );
						g_free( _data.recentslist_updates.recentList[i].displayName );
						g_free( _data.recentslist_updates.recentList[i].label );
						g_free( _data.recentslist_updates.recentList[i].addressBookID );
					}

					g_free( _data.recentslist_updates.recentList );
				}
				break;
			case MH_PB_IP_INFO_FAVORITES_LIST_UPDATES:
				{
					int i;
					for( i=0; i<_data.favoriteslist_updates.favoritesListCount; i++)
					{
						g_free( _data.favoriteslist_updates.favoritesList[i].remoteID );
						g_free( _data.favoriteslist_updates.favoritesList[i].displayName );
						g_free( _data.favoriteslist_updates.favoritesList[i].label );
						g_free( _data.favoriteslist_updates.favoritesList[i].addressBookID );
					}

					g_free( _data.favoriteslist_updates.favoritesList );
				}
				break;
//			case MH_PB_FREQUENCY_ANALYSIS_RESULT:
//				g_free( _data.frequency_analysis_result.bands );
//				g_free( _data.frequency_analysis_result.amplitudes );
//				break;
			default:
				break;
		}
	}
}		/* -----  end of static function _pb_events  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: _status_update
 *  Description:  
 * =====================================================================================
 */
static void _status_update( Mediahub2Dbus * proxy, MHPb * self, MHPbStatusType type, gpointer user_data )
{
	MHPbStatusDBusInfo * _info	=	( MHPbStatusDBusInfo * )user_data;

	if( self == _info->pb )
	{
		_info->listener.callback( _info->pb, type, _info->listener.user_data );
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_register_events_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_register_events_listener( MHPb * self, MHPbEventsListener * listener )
{
	g_return_val_if_fail( listener->callback != NULL, MH_INVALID_PARAM );
	
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_register_events_listener_sync( dbusClient, ( guint64 )self,
			(gint *)&_res, NULL, &_error );
#else
		mediahub2_dbus_call_pb_register_events_listener_sync( dbusClient, ( guint )self,
			(gint *)&_res, NULL, &_error );
#endif
	
	if( _error != NULL )
	{
		g_warning( "mh_pb_register_events_listener failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	else
	{
		MHPbDBusInfo * _info	=	g_new0( MHPbDBusInfo, 1 );

		_info->pb	=	self;
		_info->listener		=	* listener;

		g_signal_connect( dbusClient, "pb_events", G_CALLBACK( _pb_events ), _info );
	}
	return _res;
}		/* -----  end of function mh_pb_register_events_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_register_status_listener
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_register_status_listener( MHPb * self, MHPbStatusListener * listener )
{
	g_return_val_if_fail( listener->callback != NULL, MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_register_status_listener_sync( dbusClient, ( guint64 )self,
			(gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_register_status_listener_sync( dbusClient, ( guint )self,
			(gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_register_status_listener failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	else
	{
		MHPbStatusDBusInfo * _info	=	g_new0( MHPbStatusDBusInfo, 1 );

		_info->pb	=	self;
		_info->listener		=	* listener;
	
		g_signal_connect( dbusClient, "status_update", G_CALLBACK( _status_update ), _info );
	}
	return _res;
}		/* -----  end of function mh_pb_register_status_listener  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_playlist_change
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_playlist_change( MHPb * self, MHPlaylist * playlist )
{
	g_return_val_if_fail( playlist != NULL, MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_playlist_change_sync( dbusClient, ( guint64 )self, ( guint64 )playlist, 
			(gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_playlist_change_sync( dbusClient, ( guint )self, ( guint )playlist, 
			(gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "%s failed: [ %s ]", __func__, _error->message );

		g_error_free( _error );
		
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_playlist_change  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_resize
 *  Description:  guint
 * =====================================================================================
 */
MHResult mh_pb_resize( MHPb * self, uint32_t offsetx, uint32_t offsety, uint32_t width, uint32_t height )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_resize_sync( dbusClient, ( guint64 )self, offsetx, offsety, width, height, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_resize_sync( dbusClient, ( guint )self, offsetx, offsety, width, height, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_resize failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_resize  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_pixel_aspect_ratio
 *  Description:  guint
 * =====================================================================================
 */
MHResult mh_pb_pixel_aspect_ratio( MHPb * self, uint32_t pixel_n, uint32_t pixel_d )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_pixel_aspect_ratio_sync( dbusClient, ( guint64 )self, pixel_n, pixel_d, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_pixel_aspect_ratio_sync( dbusClient, ( guint )self, pixel_n, pixel_d, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_pixel_aspect_ratio failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_pixel_aspect_ratio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_audiobook_playback_speed
 *  Description:  guint
 * =====================================================================================
 */
MHResult mh_pb_audiobook_playback_speed( MHPb * self, uint32_t speed )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;
#ifdef __x86_64__
	mediahub2_dbus_call_pb_audiobook_playback_speed_sync( dbusClient, ( guint64 )self, speed, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_audiobook_playback_speed_sync( dbusClient, ( guint )self, speed, (gint *)&_res, NULL, &_error );
#endif
	if( _error != NULL )
	{
		g_warning( "mh_pb_audiobook_playback_speed failed: [ %s ]", _error->message );

		g_error_free( _error );

		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_audiobook_playback_speed  ----- */

/*  
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_get_track_info
 *  Description:  
 * =====================================================================================
 */
MHPbTrackInfo * mh_pb_get_track_info( MHPb * self)
{
	GVariant * _info, * _data, * _v_list;

	GVariantIter * _it;

	uint32_t _total_count;

	uint32_t _i	=	0;

	int32_t	_current_count;

	MHPbTrackInfo * _res = NULL;

	char * _pvar;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_pb_get_track_info_sync( dbusClient, (guint64)self, &_info, NULL,NULL);
#else
	_dbusRes	=	mediahub2_dbus_call_pb_get_track_info_sync( dbusClient, (guint)self, &_info, NULL,NULL);

#endif
	if( _dbusRes )
	{

		g_variant_get( _info, "v", &_data);

		g_variant_get( _data ,"(uiv)",&_total_count, &_current_count, &_v_list);	

		_res	=	(MHPbTrackInfo *) g_malloc0( sizeof(MHPbTrackInfo) );	

		_res->total_count	=	_total_count;
		_res->current_count	=	_current_count;

		g_variant_get( _v_list, "a(s)", &_it);
	
		_res->track_name	=	g_new0( char *, _res->total_count );

		while( g_variant_iter_loop( _it,"(s)",&_pvar))
		{
			_res->track_name[_i]	=	g_strdup( _pvar );
			g_message("%s  track_name	=	[%s]\n",__func__,_res->track_name[_i]);
			_i++;
		}
		g_variant_iter_free ( _it );
	}

	g_variant_unref (_info);

	return _res;
}		/*  -----  end of function mh_pb_get_track_info  ----- */

/*  
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_get_subtitle_info
 *  Description:  
 * =====================================================================================
 */
MHPbSubtitleInfo * mh_pb_get_subtitle_info( MHPb * self)
{
	GVariant * _info, * _data, * _v_list;

	GVariantIter * _it;

	uint32_t _total_count;

	uint32_t _i	=	0;

	int32_t	_current_count;

	MHPbSubtitleInfo * _res = NULL;

	char * _pvar;
	gboolean _dbusRes;
#ifdef __x86_64__
	_dbusRes	=	mediahub2_dbus_call_pb_get_subtitle_info_sync( dbusClient, (guint64)self, &_info, NULL,NULL) ;
#else
	_dbusRes	=	mediahub2_dbus_call_pb_get_subtitle_info_sync( dbusClient, (guint)self, &_info, NULL,NULL) ;

#endif
	if( _dbusRes )
	{
		g_variant_get( _info, "v", &_data);

		g_variant_get( _data ,"(uiv)",&_total_count, &_current_count, &_v_list);	

		_res	=	(MHPbSubtitleInfo *) g_malloc0( sizeof(MHPbSubtitleInfo) );	

		_res->total_count	=	_total_count;
		_res->current_count	=	_current_count;

		g_variant_get( _v_list, "a(s)", &_it);
	
		_res->subtitle_name	=	g_new0( char *, _res->total_count );

		while( g_variant_iter_loop( _it,"(s)",&_pvar))
		{
			_res->subtitle_name[_i]	=	g_strdup( _pvar );
			g_message("%s  subtitle_name	=	[%s]\n",__func__,_res->subtitle_name[_i]);
			_i++;
		}
		g_variant_iter_free ( _it );
	}

	g_variant_unref (_info);

	return _res;
}		/*  -----  end of function mh_pb_get_subtitle_info  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_pb_set_pipeline_status
 *  Description:  
 * =====================================================================================
 */
MHResult mh_pb_set_pipeline_status( MHPb * self, uint32_t status )
{
	MHResult _res	=	MH_OK;

	GError * _error	=	NULL;

#ifdef __x86_64__
	mediahub2_dbus_call_pb_set_pipeline_status_sync( dbusClient, ( guint64 )self, status, (gint *)&_res, NULL, &_error );
#else
	mediahub2_dbus_call_pb_set_pipeline_status_sync( dbusClient, ( guint )self, status, (gint *)&_res, NULL, &_error );
#endif

	if( _error != NULL )
	{
		g_warning( "mh_pb_set_pipeline_status failed: [ %s ]", _error->message );

		g_error_free( _error );
		_res	=	MH_IPC_ERROR;
	}
	return _res;
}		/* -----  end of function mh_pb_set_pipeline_status  ----- */
