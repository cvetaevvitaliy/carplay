/*
 * =====================================================================================
 *
 *       Filename:  carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/14/2015 08:52:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <AirPlayMain.h>
#include <linux/input.h>
#include <HIDUtils.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mh_core.h>
#include <mh_carplay.h>
//e5f7a68d-7b0f-4305-984b-974f677a150b
//gpointer ts_task( gpointer user_data )
//{
//	bool _press;
//	int _x, _y;
//	static uint8_t _lastReport[5] = { 0 };
//	uint8_t _report[5];
//
//	while( 1 )
//	{
//		if( carplay_stub_read_single_touch( &_press, &_x, &_y ))
//		{
//			_report[ 0 ]	=	_press;
//			_report[ 1 ]	=	(uint8_t)(   _x        & 0xFF );
//			_report[ 2 ]	=	(uint8_t)( ( _x >> 8 ) & 0xFF );
//			_report[ 3 ]	=	(uint8_t)(   _y        & 0xFF );
//			_report[ 4 ]	=	(uint8_t)( ( _y >> 8 ) & 0xFF );
//
//			if( memcmp( _lastReport, _report, sizeof( _report ) ) != 0 )
//			{
//				HIDPostReport( CFSTR( "e5f7a68d-7b0f-4305-984b-974f677a150b" ),
//						_report, sizeof( _report ));
//
//				memcpy( _lastReport, _report, sizeof( _report ));
//			}
//		}
//		else
//		{
//			g_warning( "carplay_stub_read_single_touch return false" );
//		}
//	}
//
//	return NULL;
//}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _init
 *  Description:  
 * =====================================================================================
 */
static gboolean _init( gpointer user_data )
{
	AirPlayStartMain();

//	g_thread_new( "touch", ts_task, NULL );

RETURN:
	return G_SOURCE_REMOVE;
}		/* -----  end of static function _init  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_instance
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_instance()
{
	GSource * _source	=	g_idle_source_new();

	g_source_set_callback( _source, _init, NULL, NULL );

	mh_io_dispatch( MH_IO( mh_core_instance() ), _source );

	g_source_unref( _source );

	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */
