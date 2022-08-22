/*
 * =====================================================================================
 *
 *       Filename:  server_with_carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/17/2015 08:29:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mh_api.h>
#include <stdbool.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <glib.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  server_event_arrived
 *  Description:  
 * =====================================================================================
 */
static void server_event_arrived(MHIPCServer * server, MHServerEventType type, void * user_data)
{
	if( type	==	SERVER_INIT_FINISH )
	{
		g_message("\n\n\n\nlsk server init finish\n\n\n\n");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/fs/configfs/configfs.ko");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/drivers/usb/gadget/libcomposite.ko");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/drivers/usb/gadget/function/u_ether.ko");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/drivers/usb/gadget/function/usb_f_ncm.ko");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/drivers/usb/gadget/function/usb_f_fs.ko");
		system( "insmod /lib/modules/4.1.22-HS7/kernel/drivers/usb/gadget/legacy/g_ffs.ko");

	}
}		/* -----  end of static function server_event_arrived  ----- */


void * carplay_stub_open_pcm_device()
{
	int _ret;
	snd_pcm_t * _snd	=	NULL;

	_ret	=	snd_pcm_open( &_snd, "hw:2,1", SND_PCM_STREAM_CAPTURE, 0 );

	if( _ret != 0 )
	{
		printf( "%s\n", snd_strerror( _ret ));
	}

	return _snd;
}

bool carplay_stub_set_pcm_params( void * device, uint32_t sample_rate, uint32_t channel_num )
{
	int _ret;

	_ret	=	snd_pcm_set_params( device,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			channel_num,
			sample_rate,
			0,                                  /* soft_resample */
			500000 );                                /* latency */

	if( _ret != 0 )
	{
		printf( "%s\n", snd_strerror( _ret ));
	}

	_ret	=	snd_pcm_prepare( device );

	if( _ret != 0 )
	{
		printf( "%s\n", snd_strerror( _ret ));
	}

	snd_pcm_start( device );

	return _ret == 0;
}
bool carplay_stub_read_pcm_data( void * device, uint8_t * buf, int sample )
{
	int _ret;

	_ret	=	snd_pcm_readi( device, buf, sample );

	if( _ret < 0 )
	{
		printf( "%s\n", snd_strerror( _ret ));

		snd_pcm_recover( device, _ret, 0 );

		snd_pcm_start( device );

		_ret	=	snd_pcm_readi( device, buf, sample );
	}

	return _ret == 0;
}

void carplay_stub_close_pcm_device( void * device )
{
	snd_pcm_close( device );
}

//bool carplay_stub_read_single_touch( bool * press, int * x, int * y )
//{
//	static int _fd	=	0;
//	
//	if( _fd == 0 )
//		_fd	=	open( "/dev/input/event0", O_RDWR );
//
//	while( 1 )
//	{
//		struct input_event _event;
//		static bool _press;
//		ssize_t _ret	=	read( _fd, &_event, sizeof( struct input_event ));
//
//		if( _ret < 0 )
//			perror( "read" );
////		printf( "%d %04X %04X %04X %04X %04X %04X %04X %04X\n", _ret, _event.time.tv_sec & 0xFFFF,
////				_event.time.tv_sec >> 16 , _event.time.tv_usec & 0xFFFF, _event.time.tv_usec >> 16,
////				_event.type, _event.code, _event.value & 0xFFFF, _event.value >> 16 );
//
//		switch( _event.type )
//		{
//		case EV_ABS:
//			switch( _event.code )
//			{
//			case ABS_MT_TOUCH_MAJOR:
//				break;
//			case ABS_MT_POSITION_X:
//				* x	=	( _event.value - 32 );
//				break;
//			case ABS_MT_POSITION_Y:
//				* y	=	( _event.value - 32 );
//				break;
//			case ABS_MT_TRACKING_ID:
//				break;
//			case ABS_MT_PRESSURE:
//				break;
//			default:
//				printf( "Uncached ABS event 0x%02X\n", _event.code );
//				break;
//			}
//			break;
//		case EV_SYN:
//			switch( _event.code )
//			{
//			case SYN_REPORT:
//				goto RETURN;
//
//				break;
//			case SYN_MT_REPORT:
//				break;
//			default:
//				printf( "Uncached SYN event 0x%02X\n", _event.code );
//				break;
//			}
//			break;
//		case EV_KEY:
//			switch( _event.code )
//			{
//			case BTN_TOUCH:
//				* press	=	_event.value;
//				break;
//			default:
//				break;
//			}
//			break;
//		}
//	}
//
//RETURN:
//	return true;
//}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	MHIPCServer * _server	=	mh_ipc_server_create();
//	void * _aaa;
//	uint8_t _buf[ 1600 * 4 ];
//	FILE * _fp	=	fopen( "/tmp/pcm", "w" );

	MHServerEventsListener * _listener	=	(MHServerEventsListener *)g_new0( MHServerEventsListener, 1);

	_listener->callback	=	server_event_arrived;
	_listener->user_data	=	NULL;

	mh_ipc_server_register_events_listener( _server,  _listener);

	mh_ipc_start_media_engine( _server );

//	_aaa	=	carplay_stub_open_pcm_device();
//
//	carplay_stub_set_pcm_params( _aaa, 16000, 2 );
//	while( 1 )
//	{
//
//	carplay_stub_read_pcm_data( _aaa, _buf, 1600 );
//
//	fwrite( _buf, 1600 * 4, 1, _fp );
//	}
	mh_ipc_server_run( _server );

	return 0;
}				/* ----------  end of function main  ---------- */


