import _logging
from ctypes import *
from _mediahub2 import *

music_filter    =   None
movie_filter	=	None
picture_filter	=	None

pb  =   None
global_dev	=	None
def event_arrived( core, event, user_data ):
	global music_filter, movie_filter, picture_filter, pb
	_logging.core( 'Core Event', event )

	if( event == 0 ):
		music_filter    =   mh_filter_create( b'mp3;wma;aac;wav;ogg;mp2' )
		print('--------------------->',music_filter)
		movie_filter	=	mh_filter_create( b'mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat;rm;rmvb;mpg' )
		picture_filter	=	mh_filter_create( b'jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif' )
		pb  =   mh_pb_create()

def device_detached( dev, user_data ):
    _logging.detach( 'Device Event' )
    mh_object_unref( dev )

def each_item_data( itemData, user_data ):
    _logging.output( '[' + str( cast( user_data, POINTER( py_object )).contents.value ) + ']'
            + itemData.contents.name.decode( 'utf8' ))
    cast( user_data, POINTER( py_object )).contents.value   +=  1
    return False

def dev_event( dev, scanType, itemType, folder, user_data ):
	global music_filter, pb
	if( scanType == MHDevScanCbType.MH_DEV_FIRST_FILE ):
		if( itemType == MHItemType.MH_ITEM_MUSIC ):
			_logging.output( 'First available music item was found, creating a playlist and beginning play' )
	#		playlist =   mh_folder_create_playlist( folder, music_filter, False )

	#		mh_pb_play_by_list( pb, playlist, 0 )
	#		_offset  =    0

	#		_logging.output( 'Playlist Contents:' )
		#	mh_playlist_foreach( playlist, 0, -1,  playlist_cb( each_item_data ), byref( py_object( _offset )))
	#		_logging.output( 'End' )

def device_attached( core, dev, event, user_data ):
	global global_dev
	_detach_listener    =   MHDevDetachListener( detach_cb( device_detached ), c_void_p( 0 ))
	_logging.attach( 'Device Event', event )
	_logging.resultA( 'Register Dev Listener',
			mh_dev_register_detach_listener( dev, byref( _detach_listener )))

	mh_object_ref( dev )

	_serial =   c_char_p()
	_fsType =   c_char_p()
	_entry =   c_char_p()

	mh_object_get_properties( dev, b"serial", pointer( _serial ),
			b'fs_type', pointer( _fsType ), b'entry', pointer( _entry ), c_void_p( 0 ))
	_logging.output( 'serial:\t' + _serial.value.decode() )
	_logging.output( 'fs type:\t' + _fsType.value.decode() )
	_logging.output( 'entry:\t' + _entry.value.decode() )

	_dev_events_listener    =   MHDevEventsListener( dev_events_cb( dev_event ), c_void_p( 0 ))
	_logging.resultA( 'Register Dev Events Listener',
			mh_dev_register_events_listener( dev, byref( _dev_events_listener )))

	_logging.output( 'Starting Scan ...' )
	mh_dev_start_scan( dev, MHDevScanType.SCAN_FOLDER )
	global_dev = dev

def get_pb():
	return pb

def get_global_dev():
	print( global_dev )
	return global_dev

def get_music_filter():
	return music_filter

def get_movie_filter():
	return movie_filter

def get_picture_filter():
	return picture_filter



