import _logging
import time
import math
import importlib
from _functions import *
playlist = None
change_count = 0
_filter	=	None
_first_time	=	0
def item_function(item,data,user_data):
	global change_count,playlist,_first_time
#	print(data.contents.uri.decode('utf8'))
	if data.contents.type == 0:
		time.sleep(2)

		i	=	c_int(-1)
		pi	=	pointer(i)
		_items	=	POINTER(c_void_p)

		_items = mh_folder_get_children(item, _filter,MHFolderPosition.FOLDER_BEGIN, pi, MHItemOrderType.MH_ITEM_ORDER_BY_DEFAULT)

		mh_object_unref( playlist)

		playlist = mh_folder_create_playlist( item, _filter,False)

		_playlist_count = c_int(0)
		_pi_count	=	pointer(_playlist_count)

		mh_object_get_properties(playlist,b"count",_pi_count,c_int( 0 ) )
		print('item_function--->data.contents.name:',data.contents.name)
		if _pi_count.contents.value == 0:
			print('item_function--->count',_pi_count.contents.value)
		else:
			if _first_time == 0:
				_first_time = _first_time+1
				mh_object_set_properties( get_pb(), b"video_sink", b"glimagesink", None)
				time.sleep(5)

			mh_pb_play_by_list(get_pb(), playlist,0)
			index	=	0
			for index in range(0,_pi_count.contents.value):
				mh_pb_next( get_pb())
				index	=	index+1
				time.sleep(2)
			change_count= change_count+1
			print('+++++++++++++++++++++++++++++++++++++++++++++++++++',change_count)

		mh_item_foreach(_items, pi.contents,item_cb(item_function),None)

def run():
	global _filter,playlist
	mh_misc_set_filter(MHMiscFilterType.MH_MISC_FILTER_MUSIC, b"mp3;wma;aac;wav;ogg;mp2",
MHMiscFilterType.MH_MISC_FILTER_MOVIE,b"mp4;3gp;avi;mkv;mpg;mpeg;wmv;vob;flv;swf;mov;dat",
MHMiscFilterType.MH_MISC_FILTER_PICTURE,b"jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif",None)
	_events_listener    =   MHEventsListener( event_cb( event_arrived ), c_void_p( 0 ) )
	_logging.resultA( 'Register Events Listener',
			mh_core_register_events_listener( byref( _events_listener )))

	_devices_listener   =   MHDevicesListener( device_cb( device_attached ), c_void_p( 0 ))
	_logging.resultA( 'Register Devices Listener',
			mh_core_register_devices_listener( byref( _devices_listener )))
	mh_core_start()
	playlist	=	POINTER( c_void_p )
	_logging.output( 'Test is running' )
	_filter	=	POINTER( c_void_p )
	index	=	int(input( '1 means music,2 means movie:\n'))
	print('index',index)
	if index == 1:
		_filter =	get_music_filter()
	elif index == 2:
		_filter =	get_movie_filter()
	else:
		_filter	=	get_picture_filter()

	time.sleep(2)
	while True:
		_base = c_void_p()
		mh_object_get_properties(get_global_dev(),b"base", pointer(_base), c_void_p( 0 ) )

		i	=	c_int(-1)
		pi	=	pointer(i)
		_items	=	POINTER(c_void_p)

		_items = mh_folder_get_children( _base, _filter, MHFolderPosition.FOLDER_BEGIN, pi, MHItemOrderType.MH_ITEM_ORDER_BY_DEFAULT)
		playlist = mh_folder_create_playlist( _base, _filter,False)

		_playlist_count = c_int(0)
		_pi_count	=	pointer(_playlist_count)

		mh_object_get_properties(playlist,b"count",_pi_count,c_int( 0 ) )
		if _pi_count.contents.value == 0:
			print('playlist count=====0\n')
		else:
			mh_pb_play_by_list(get_pb(), playlist,0)
		mh_item_foreach(_items, pi.contents,item_cb(item_function),None)

		time.sleep(2)
