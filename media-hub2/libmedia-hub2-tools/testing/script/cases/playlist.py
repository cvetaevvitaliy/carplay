import _logging
import time
from _functions import *

def _foreach( itemData, user_data ):
    print( type( itemData.contents.metadata.music) )

    return False

def local_device( dev, scanType, itemType, folder, user_data ):
    print( 'scan complete' )
    if( scanType == MHDevScanCbType.MH_DEV_FINISH ):
        _playlists  =	mh_dev_get_playlist_info( dev )

    i   =   0

    while( _playlists[i] != c_void_p( 0 )):
        print( _playlists[i].name.decode( 'utf8') )

        playlist    =   mh_dev_restore_playlist( dev, _playlists[i].playlistid )
        mh_playlist_foreach( playlist, 0, -1, playlist_cb( _foreach ), c_void_p( 0 ))

        i   +=  1

def local_attached( core, dev, event, user_data ):
    _dev_events_listener    =   MHDevEventsListener( dev_events_cb( local_device ), c_void_p( 0 ))
    _logging.resultA( 'Register Dev Events Listener',
            mh_dev_register_events_listener( dev, byref( _dev_events_listener )))

    mh_dev_start_scan( dev, MHDevScanType.SCAN_FOLDER )

def run():
    _devices_listener   =   MHDevicesListener( device_cb( local_attached ), c_void_p( 0 ))

    _logging.resultA( 'Register Devices Listener',
            mh_core_register_devices_listener( byref( _devices_listener )))

    mh_core_start()

    _logging.output( 'Test is running' )

    input( '' )
