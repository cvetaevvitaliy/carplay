import _logging
import time
from _functions import *

def run():
	_events_listener    =   MHEventsListener( event_cb( event_arrived ), c_void_p( 0 ) )
	_logging.resultA( 'Register Events Listener',
			mh_core_register_events_listener( byref( _events_listener )))

	_devices_listener   =   MHDevicesListener( device_cb( device_attached ), c_void_p( 0 ))
	_logging.resultA( 'Register Devices Listener',
			mh_core_register_devices_listener( byref( _devices_listener )))

	mh_core_start()

	_logging.output( 'Test is running' )

	while True:
		mh_pb_play( get_pb())
		time.sleep( 2 )
		mh_pb_pause( get_pb() )
		time.sleep( 2 )
