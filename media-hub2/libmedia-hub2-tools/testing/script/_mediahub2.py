from ctypes import *
from enum import Enum

class MHMiscFilterType:
	MH_MISC_FILTER_MUSIC	=	0
	MH_MISC_FILTER_MOVIE	=	1
	MH_MISC_FILTER_PICTURE	=	2
	MH_MISC_FILTER_PLAYLIST =	3


class MHDevScanType:
	SCAN_FOLDER	=	0
	SCAN_TAG	=	1

class MHDevScanCbType:
	MH_DEV_FIRST_FILE	=	0
	MH_DEV_UPDATE_FILE	=	1
	MH_DEV_FINISH	=	2

class MHItemType:
	MH_ITEM_FOLDER	=	0
	MH_ITEM_MUSIC	=	1
	MH_ITEM_MOVIE	=	2

class MHItemOrderType:
	MH_ITEM_ORDER_BY_ALPHABET	=	0
	MH_ITEM_ORDER_BY_DEFAULT	=	1

class MHFolderPosition:
	FOLDER_BEGIN	=	0
	FOLDER_CUR	=	1
	FOLDER_NUM	=	2

event_cb    =   CFUNCTYPE( None, c_void_p, c_uint, c_void_p )

class MHEventsListener( Structure ):
    _fields_    =   [
        ( 'callback', event_cb ),
        ( 'user_data', c_void_p )
    ]

device_cb    =   CFUNCTYPE( None, c_void_p, c_void_p, c_uint, c_void_p )

class MHDevicesListener( Structure ):
    _fields_    =   [
        ( 'callback', device_cb ),
        ( 'user_data', c_void_p )
    ]

detach_cb    =   CFUNCTYPE( None, c_void_p, c_void_p )

class MHDevDetachListener( Structure ):
    _fields_    =   [
        ( 'callback', detach_cb ),
        ( 'user_data', c_void_p )
    ]

dev_events_cb	=	CFUNCTYPE( None, c_void_p, c_uint, c_uint, c_void_p, c_void_p )

class MHDevEventsListener( Structure ):
	_fields_	=	[
		( 'callback', dev_events_cb ),
		( 'user_data', c_void_p )
	]
class MHPbInfoType:
	MH_PB_INFO_PTIME_CHANGE	=	0
	MH_PB_INFO_DURATION	=	1
	MH_PB_INFO_TAG	=	2
	MH_PB_INFO_EOS	=	3
	MH_PB_INFO_ERROR	=	4
	MH_PB_INFO_TRACK_TOP	=	5

class MHPbInfoData( Union ):
	class tag_info( Structure ):
		_fields_	=	[
			( 'title', c_char_p ),
			( 'artist', c_char_p ),
			( 'album', c_char_p )
		]
	class track_info( Structure ):
		_fields_	=	[
			( 'index', c_uint ),
			( 'uri', c_char_p ),
			( 'name', c_char_p )
		]
	_fields_	=	[
		( 'current_time', c_uint ),
		( 'duration', c_uint ),
		( 'tag_info', tag_info ),
		( 'track_info', track_info)
	]
pb_events_cb	=	CFUNCTYPE( None, c_void_p, c_uint, POINTER( MHPbInfoData ), c_void_p )

class MHPbEventsListener( Structure ):
	_fields_	=	[
		( 'callback', pb_events_cb ),
		( 'user_data', c_void_p )
	]

pb_status_cb	=	CFUNCTYPE( None, c_void_p, c_uint, c_void_p )

class MHPbStatusListener( Structure ):
	_fields_	=	[
		( 'callback', pb_status_cb ),
		( 'user_data', c_void_p )
	]

class MHItemData( Structure ):
	class metadata( Union ):
		class music( Structure ):
			_fields_	=	[
				( 'title', c_char_p ),
				( 'album', c_char_p ),
				( 'artist', c_char_p ),
				( 'genre', c_char_p ),
				( 'composer', c_char_p ),
				( 'year', c_int ),
				( 'track', c_int ),
				( 'trackCount', c_int )
			]

		_fields_	=	[
			( 'music', music )
		]

	_fields_	=	[
		( 'uri', c_char_p ),
		( 'type', c_uint ),
		( 'name', c_char_p ),
		( 'uniqueId', c_uint ),
		( 'valid', c_uint ),
		( 'tagId', c_longlong ),
		( 'metadata', metadata )
	]

class MHPlaylistInfo( Structure ):
	_fields_	=	[
		( 'playlistid', c_longlong ),
		( 'name', c_char_p )
	]

playlist_cb	=	CFUNCTYPE( c_bool, POINTER( MHItemData ), c_void_p )
mediahub2   =   CDLL( 'libmedia-hub2.so' )

mh_built_date	=	mediahub2.mh_built_date
mh_built_date.restype	=	c_char_p

mh_built_time	=	mediahub2.mh_built_time
mh_built_time.restype	=	c_char_p

mh_core_register_events_listener	=	mediahub2.mh_core_register_events_listener
mh_core_register_events_listener.restype    =   c_int
mh_core_register_events_listener.argtypes   =   [ POINTER( MHEventsListener ) ]

mh_core_register_devices_listener	=	mediahub2.mh_core_register_devices_listener
mh_core_register_devices_listener.restype    =   c_int

mh_dev_register_detach_listener	=	mediahub2.mh_dev_register_detach_listener
mh_dev_register_detach_listener.restype    =   c_int

mh_core_start	=	mediahub2.mh_core_start
mh_core_start.restype    =   None

mh_core_stop	=	mediahub2.mh_core_stop
mh_core_stop.restype    =   None

compare_data_func    =   CFUNCTYPE( c_int, POINTER( MHItemData ), POINTER( MHItemData ), c_void_p )
mh_playlist_sort	=	mediahub2.mh_playlist_sort
mh_playlist_sort.restype    =   None
mh_playlist_sort.argtypes	=	[ c_void_p, compare_data_func, c_void_p ]

mh_playlist_foreach	=	mediahub2.mh_playlist_foreach
mh_playlist_foreach.restype	=	None
mh_playlist_foreach.argtypes	=	[ c_void_p, c_int, c_int, playlist_cb, c_void_p ]

mh_playlist_append	=	mediahub2.mh_playlist_append
mh_playlist_append.restype	=	None
mh_playlist_append.argtypes	=	[ c_void_p, POINTER(c_void_p), c_uint ]

mh_playlist_insert	=	mediahub2.mh_playlist_insert
mh_playlist_insert.restype	=	None
mh_playlist_insert.argtypes	=	[ c_void_p, c_uint, POINTER(c_void_p), c_uint]

mh_playlist_remove	=	mediahub2.mh_playlist_remove
mh_playlist_remove.restype	=	None
mh_playlist_remove.argtypes	=	[ c_void_p, c_uint, c_uint ]




mh_object_unref	=	mediahub2.mh_object_unref
mh_object_unref.argtypes	=	[ c_void_p ]

mh_object_ref	=	mediahub2.mh_object_ref
mh_object_ref.argtypes	=	[ c_void_p ]
mh_object_ref.restype	=	c_void_p

mh_object_get_properties	=	mediahub2.mh_object_get_properties
mh_object_get_properties.restype	=	None

mh_object_set_properties	=	mediahub2.mh_object_set_properties
mh_object_set_properties.restype	=	None

mh_dev_register_events_listener	=	mediahub2.mh_dev_register_events_listener
mh_dev_register_events_listener.restype	=	c_uint
mh_dev_register_events_listener.argtypes	=	[ c_void_p, POINTER( MHDevEventsListener )]

mh_dev_start_scan	=	mediahub2.mh_dev_start_scan
mh_dev_start_scan.restype	=	None
mh_dev_start_scan.argtypes	=	[ c_void_p, c_uint ]

mh_dev_create_collection	=	mediahub2.mh_dev_create_collection
mh_dev_create_collection.restype	=	c_void_p
mh_dev_create_collection.argtypes	=	[ c_void_p ]

#mh_dev_get_playlist_names	=	mediahub2.mh_dev_get_playlist_names
#mh_dev_get_playlist_names.restype	=	POINTER( c_char_p )
#mh_dev_get_playlist_names.argtypes	=	[ c_void_p ]

mh_dev_save_playlist	=	mediahub2.mh_dev_save_playlist
mh_dev_save_playlist.restype	=	c_uint
mh_dev_save_playlist.argtypes	=	[ c_void_p, c_void_p, c_char_p ]

mh_dev_restore_playlist	=	mediahub2.mh_dev_restore_playlist
mh_dev_restore_playlist.restype	=	c_void_p
mh_dev_restore_playlist.argtypes	=	[ c_void_p, c_longlong ]

mh_folder_seek	=	mediahub2.mh_folder_seek
mh_folder_seek.restype	=	None
mh_folder_seek.argtypes	=	[ c_void_p, c_void_p, c_int, c_uint ]

mh_folder_create_empty_playlist	=	mediahub2.mh_folder_create_empty_playlist
mh_folder_create_empty_playlist.restype	=	c_void_p
mh_folder_create_empty_playlist.argtypes	=	[ c_void_p ]

mh_folder_create_playlist	=	mediahub2.mh_folder_create_playlist
mh_folder_create_playlist.restype	=	c_void_p
mh_folder_create_playlist.argtypes	=	[ c_void_p, c_void_p, c_bool ]

mh_folder_get_children	=	mediahub2.mh_folder_get_children
mh_folder_get_children.restype	=	POINTER(c_void_p)
mh_folder_get_children.argtypes	=	[ c_void_p, c_void_p, c_uint, POINTER(c_int), c_uint]

mh_filter_create	=	mediahub2.mh_filter_create
mh_filter_create.restype	=	c_void_p
mh_filter_create.argtypes	=	[ c_char_p ]

mh_filter_match		=	mediahub2.mh_filter_match
mh_filter_match.restype =   c_bool
mh_filter_match.argtypes=	[ c_void_p, c_char_p ]

#mh_contents_query	=	mediahub2.mh_contents_query
#mh_contents_query.restype	=	c_void_p
#mh_contents_query.argtypes	=	[ c_void_p, c_char_p ]



#mh_pb_play_items	=	mediahub2.mh_pb_play_items
#mh_pb_play_items.restype	=	None
#mh_pb_play_items.argtypes	=	[ POINTER( c_ulong ), c_uint ] 

mh_pb_create	=	mediahub2.mh_pb_create
mh_pb_create.restype	=	c_void_p

mh_pb_play_by_list	=	mediahub2.mh_pb_play_by_list
mh_pb_play_by_list.restype	=	None
mh_pb_play_by_list.argtypes	=	[ c_void_p, c_void_p, c_uint ]

mh_pb_next	=	mediahub2.mh_pb_next
mh_pb_next.restype	=	None
mh_pb_next.argtypes	=	[ c_void_p ]

mh_pb_stop	=	mediahub2.mh_pb_stop
mh_pb_stop.restype	=	None
mh_pb_stop.argtypes	=	[ c_void_p ]

mh_pb_play	=	mediahub2.mh_pb_play
mh_pb_play.restype	=	None
mh_pb_play.argtypes	=	[ c_void_p ]

mh_pb_pause	=	mediahub2.mh_pb_pause
mh_pb_pause.restype	=	None
mh_pb_pause.argtypes	=	[ c_void_p ]

mh_pb_play_pause	=	mediahub2.mh_pb_play_pause
mh_pb_play_pause.restype	=	None

mh_pb_resume	=	mediahub2.mh_pb_resume
mh_pb_resume.restype	=	None

mh_pb_set_position	=	mediahub2.mh_pb_set_position
mh_pb_set_position.restype	=	None
mh_pb_set_position.argtypes	=	[ c_uint ]

mh_pb_forward	=	mediahub2.mh_pb_forward
mh_pb_forward.restype	=	None
mh_pb_forward.argtypes	=	[ c_void_p ]

mh_pb_forward_done	=	mediahub2.mh_pb_forward_done
mh_pb_forward_done.restype	=	None
mh_pb_forward_done.artist	=	[ c_void_p ]

mh_pb_backward	=	mediahub2.mh_pb_backward
mh_pb_backward.restype	=	None
mh_pb_backward.argtypes	=	[ c_void_p ]

mh_pb_backward_done	=	mediahub2.mh_pb_backward_done
mh_pb_backward_done.restype	=	None
mh_pb_backward_done.argtypes	=	[ c_void_p ]

mh_pb_set_speed	=	mediahub2.mh_pb_set_speed
mh_pb_set_speed.restype	=	None
mh_pb_set_speed.argtypes	=	[ c_uint ]

mh_pb_set_shuffle	=	mediahub2.mh_pb_set_shuffle
mh_pb_set_shuffle.restype	=	None

mh_pb_previous	=	mediahub2.mh_pb_previous
mh_pb_previous.restype	=	None
mh_pb_previous.argtypes	=	[ c_void_p ]

mh_pb_seek	=	mediahub2.mh_pb_seek
mh_pb_seek.restype	=	None
mh_pb_seek.argtypes	=	[ c_void_p, c_uint ]

mh_pb_playlist_change	=	mediahub2.mh_pb_playlist_change
mh_pb_playlist_change.restype	=	None
mh_pb_playlist_change.argtypes	=	[ c_void_p, c_void_p ]

mh_pb_resize	=	mediahub2.mh_pb_resize
mh_pb_resize.restype	=	None
mh_pb_resize.argtypes	=	[ c_void_p, c_uint, c_uint, c_uint, c_uint ]

mh_pb_register_events_listener	=	mediahub2.mh_pb_register_events_listener
mh_pb_register_events_listener.restype	=	None
mh_pb_register_events_listener.argtypes	=	[ c_void_p, POINTER( MHPbEventsListener )]

mh_pb_register_status_listener	=	mediahub2.mh_pb_register_status_listener
mh_pb_register_status_listener.restype	=	None
mh_pb_register_status_listener.argtypes	=	[ c_void_p, POINTER( MHPbStatusListener )]

item_cb		=	CFUNCTYPE( c_bool, c_void_p, POINTER(MHItemData), c_void_p)
mh_item_foreach	=	mediahub2.mh_item_foreach
mh_item_foreach.restype		=	None
mh_item_foreach.argtypes	=	[POINTER(c_void_p), c_int, item_cb, c_void_p]

mh_col_create	=	mediahub2.mh_col_create
mh_col_create.restype	=	c_void_p
mh_col_create.argtypes	=	[ c_void_p ]

col_cb	=	CFUNCTYPE( c_bool, c_void_p, c_void_p )
mh_col_foreach	=	mediahub2.mh_col_foreach
mh_col_foreach.restype	=	c_uint
mh_col_foreach.argtypes	=	[ c_void_p, col_cb, c_void_p ]

mh_col_set_filter	=	mediahub2.mh_col_set_filter
mh_col_set_filter.restype	=	c_uint

mh_col_retrieve_data	=	mediahub2.mh_col_retrieve_data
mh_col_retrieve_data.restype	=	POINTER( c_char_p )
mh_col_retrieve_data.argtypes	=	[ c_void_p, c_uint, c_uint, POINTER( c_int )]

mh_col_create_playlist	=	mediahub2.mh_col_create_playlist
mh_col_create_playlist.restype	=	c_void_p
mh_col_create_playlist.argtypes	=	[ c_void_p ]

#mh_ipc_client_init	=	mediahub2.mh_ipc_client_init
#mh_ipc_client_init.restype	=	None

#mh_ipc_server_run	=	mediahub2.mh_ipc_server_run
#mh_ipc_server_run.restype	=	None

mh_misc_set_filter	=	mediahub2.mh_misc_set_filter
mh_misc_set_filter.restype		=	None

mh_dev_get_playlist_info	=	mediahub2.mh_dev_get_playlist_info
mh_dev_get_playlist_info.restype	=	POINTER( MHPlaylistInfo )
mh_dev_get_playlist_info.argtypes	=	[ c_void_p ]
