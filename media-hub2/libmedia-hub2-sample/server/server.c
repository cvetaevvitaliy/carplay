/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/07/2014 09:11:37 AM
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
#include <mh_api.h>
#include <glib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <alsa/asoundlib.h>
#ifndef HS7 // for nagivi
#include <pulse/simple.h>
#include <pulse/error.h>
#else //for hs7
#include <ah/simple.h>
#endif
//#include <pulse/error.h>

#include <mh_carlife.h>

#include <mh_contents.h>
#include <pthread.h>
//#include <mh_streaming.h>
#ifndef HS7 // for nagivi
pa_sample_spec ss;
#else //for hs7
ah_sample_spec_t ss;
#endif
//static FILE *fp = NULL;

//add for fastboot start
extern sem_t db_open_sem;
extern sem_t adb_wait_sem;
//add for fastboot end

static void usr1_sig_handler(int sig)
{
	g_message("usr1_sig_handler IN!!!!!");
	sem_post(&db_open_sem);
	sem_post(&adb_wait_sem);
	g_message("usr1_sig_handler OUT!!!!!");
}

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
		g_message("\n\n\n\nserver init finish\n\n\n\n");
	}
}		/* -----  end of static function server_event_arrived  ----- */
#ifndef HS7 // for nagivi
bool carplay_stub_set_pcm_params( void * device, uint32_t sample_rate, uint32_t channel_num )
{
	int _ret;

	int type;
	type = * (int *)device;
	g_message("\n\n\n\n  type = %d   \n\n\n\n",type);
	switch( type )
	{
		case 1:
			g_message("This is siri\n");
			break;
		case 2:
			g_message("This is Tele\n");
			break;
		default:
			g_message("Type is error \n");	
			break;
	}

	ss.format 	= PA_SAMPLE_S16LE;
	ss.channels = channel_num;
	ss.rate 	= sample_rate;

	return _ret == 0;
}

void * carplay_stub_open_pcm_device()
{
	int _ret;
	int error;
	g_message("%s",__func__);
	pa_simple * _snd = NULL;

	if (!( _snd = pa_simple_new(NULL, "siri", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
	}

	return _snd;
}

void carplay_stub_close_pcm_device( void * device )
{
	g_message("%s",__func__);
//	fclose(fp);
//	fp = NULL;
	pa_simple_free( device );
}

bool carplay_stub_read_pcm_data( void * device, uint8_t * buf, int sample )
{
	int _ret;
	int error;

	_ret	=	pa_simple_read( device, buf, sample * 2, &error);

//	if (fp == NULL)
//	{
//		fp = fopen("/tmp/abc","w+");
//	}
//
//	fwrite(buf,sample * 2,1,fp);

	if( _ret < 0 )
	{
		fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
	}

	return _ret;
}

void * mh_dev_carlife_open_pcm_device( MHCarlifeSampleFormat format, uint32_t sample_rate, uint32_t channel_num)
{
	pa_sample_spec _spec;

	int _error;

	pa_simple * _device	=	NULL;
	
	if( format == CARLIFE_SAMPLE_S16LE)
	{
		_spec.format	=	PA_SAMPLE_S16LE;
		_spec.rate	=	sample_rate;
		_spec.channels	=	channel_num;
	}
	_device	=	pa_simple_new( NULL, "VR", PA_STREAM_RECORD, NULL, "record", &_spec, NULL, NULL, &_error);
	if( _device == NULL)
	{
		g_message("pa_simple_new failed:%s", pa_strerror( _error));
	}
	return (void *)_device;
}

int mh_dev_carlife_read_pcm_data( void * device,  char * buf, uint32_t len )
{
	int _ret;
	int _error;

	_ret	=	pa_simple_read( device, buf, len , &_error);

	if( _ret < 0)
	{
		g_message("pa_simple_read failed:%s", pa_strerror(_error));
	}
	return _ret;
	
	

}

void mh_dev_carlife_close_pcm_device( void * device)
{
	pa_simple_free( (pa_simple *)device);

}
#else //for hs7
bool carplay_stub_set_pcm_params( void * device, uint32_t sample_rate, uint32_t channel_num )
{
	int _ret;

	int type;
	type = * (int *)device;
	g_message("\n\n\n\n  type = %d   \n\n\n\n",type);
	switch( type )
	{
		case 1:
			g_message("This is siri\n");
			break;
		case 2:
			g_message("This is Tele\n");
			break;
		default:
			g_message("Type is error \n");	
			break;
	}

	ss.format 	= AH_SAMPLE_S16LE;
	ss.channels = channel_num;
	ss.rate 	= sample_rate;

	return _ret == 0;
}

void * carplay_stub_open_pcm_device()
{
	int _ret;
	int error;
	g_message("%s",__func__);
	ah_simple_t * _snd = NULL;

	if (!( _snd = ah_simple_new("siri", AH_STREAM_CAPTURE,  &ss))) {
		g_message("%s ah_simple_new() failed", __func__);
	}

	return _snd;
}

void carplay_stub_close_pcm_device( void * device )
{
	g_message("%s",__func__);
//	fclose(fp);
//	fp = NULL;
    ah_simple_free( (ah_simple_t*)device );
}

bool carplay_stub_read_pcm_data( void * device, uint8_t * buf, int sample )
{
	int _ret;

	_ret	=	ah_simple_read( (ah_simple_t *)device, buf, sample * 2);

//	if (fp == NULL)
//	{
//		fp = fopen("/tmp/abc","w+");
//	}
//
//	fwrite(buf,sample * 2,1,fp);

	if( _ret < 0 )
	{
		g_message("%s ah_simple_read() failed",__func__);
	}

	return _ret;
}

void * mh_dev_carlife_open_pcm_device( MHCarlifeSampleFormat format, uint32_t sample_rate, uint32_t channel_num)
{
	ah_sample_spec_t _spec;

	int _error;

	ah_simple_t * _device	=	NULL;
	
	if( format == CARLIFE_SAMPLE_S16LE)
	{
		_spec.format	=	AH_SAMPLE_S16LE;
		_spec.rate	=	sample_rate;
		_spec.channels	=	channel_num;
	}
	_device	=	ah_simple_new(  "VR", AH_STREAM_CAPTURE, &_spec);
	if( _device == NULL)
	{
		g_message("%s ah_simple_new failed", __func__);
	}
	return (void *)_device;
}

int mh_dev_carlife_read_pcm_data( void * device,  char * buf, uint32_t len )
{
	int _ret;
	int _error;

	_ret	=	ah_simple_read( (ah_simple_t*)device, buf, len );

	if( _ret < 0)
	{
		g_message("%s ah_simple_read() failed",__func__);
	}
	return _ret;
	
	

}

void mh_dev_carlife_close_pcm_device( void * device)
{
    ah_simple_free( (ah_simple_t *)device);

}
#endif
static int get_cpu_stepping( void )
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int stepping = 0;
	char *t;

	fp = fopen( "/proc/cpuinfo", "r" );
	if( fp == NULL ){
		printf( "failed to read /proc/cpufinfo\n" );
		goto e_open;
	}

	while( ( read = getline( &line, &len, fp ) ) != -1 ){

		if( !strncmp( line, "stepping", 8 ) ){

			t = line + 8;
			while( *t ){
				if( ( *t >= '0' ) && ( *t <= '9' ) ){
					sscanf( t, "%d", &stepping );
				}
				t++;
			}
			break;
		}
	}
	free( line );

	fclose( fp );

e_open:
	printf( "cpu stepping %d\n", stepping );
	return stepping;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
#ifndef HS7 // for nagivi
//	setenv("MH_DEV_CARLIFE_BLUETOOTH_AUTO_PAIR", "1",1);
//	setenv("MH_DEV_CARLIFE_BLUETOOTH_INTERNAL_UI", "1",1);
//	setenv("MH_DEV_CARLIFE_VOICE_WAKEUP", "0",1);
//	setenv("MH_DEV_CARLIFE_FOCUS_UI", "1", 1);
//	setenv("MH_APPLE_AUTH_COPROCESSOR_ADDRESS","/dev/i2c-1",1);	
//	setenv("MH_I2C_DEV_ADDR","17",1);

//	setenv("MH_CARPLAY_VIDEO_DEC_NAME","omxh264dec",1);	
//	setenv("MH_CARPLAY_VIDEO_PARSE_NAME","h264parse",1);	

//	int i =	get_cpu_stepping(); 
//	g_message("\n\n\n   cpu  = %d   \n\n\n\n",i);
//	if( i <= 8 )
//	{
//		setenv("MH_I2C_DEV_ADDR","16",1);	
//	}else{
//		setenv("MH_I2C_DEV_ADDR","17",1);	
//	}

	setenv("MH_CARPLAY_AUDIO_SINK_NAME","pulsesink",1);	
	setenv("MH_KERNEL_DRIVE_PATH","/lib/modules/3.18.24-tcc/kernel/drivers/usb/gadget/",1);
	setenv("MH_CARPLAY_CONFIG_FILE_PATH","/etc/carplay.ini",1);
	
//	setenv( "MH_EA_PROTOCOL_NAME", "com.ford.sync.prot0", 1);
//	setenv("MH_IP_MODE_NAME","device",1);
//	setenv("MH_ORDER_ARTICLE_STATE","1",1);
	

//	system( "echo peripheral > /sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state");
//	usleep( 100000 );
//	system( "modprobe dwc3-pci");
//	system( "modprobe dwc3");
//	usleep( 100000 );
//	system( "modprobe libcomposite");
//	system( "modprobe udc_core");
//	system( "modprobe configfs");
//	system( "modprobe u_ether");
//	system( "modprobe usb_f_ncm");
//	system( "modprobe usb_f_fs");
//	system( "modprobe g_ffs");
//	system( "modprobe portmux_intel_drcfg");
//
//	system( "echo host > /sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state");
//
//	system( "modprobe xhci_hcd");
//	system( "modprobe xhci_plat_hcd");
//	system( "modprobe xhci_pci");

//	system( "insmod /home/root/ko/libcomposite.ko");
//	system( "insmod /home/root/ko/u_ether.ko");
//	system( "insmod /home/root/ko/usb_f_ncm.ko");
//	system( "insmod /home/root/ko/usb_f_fs.ko");
//	system( "insmod /home/root/ko/g_ffs.ko int_ncm_or_uac2=0");
//	setenv("MH_PB_AUDIOSINK_SIRI_DEVICENAME","pb_sink_mainaudio",1);	
//	setenv("MH_PB_AUDIOSINK_TELE_DEVICENAME","pb_sink_telephone",1);	
//	setenv("MH_PB_AUDIOSINK_DEVICENAME","tcsink",1);
//	setenv("MH_PB_AUDIOSINK_ALT_DEVICENAME","pb_sink_v_guide",1);

//	setenv("MH_IAP_NOW_PLAYING_UPDATE","0",1);

	setenv("MH_DEV_CARLIFE_BLUETOOTH_AUTO_PAIR", "1",1);
	setenv("MH_DEV_CARLIFE_BLUETOOTH_INTERNAL_UI", "0",1);
	setenv("MH_DEV_CARLIFE_VOICE_WAKEUP", "0",1);
	setenv("MH_DEV_CARLIFE_FOCUS_UI", "0", 1);


#else //for hs7
	setenv("MH_DEV_MEMORY_DATABASE", "1", 1);
	setenv("MH_DEV_CARLIFE_BLUETOOTH_AUTO_PAIR", "1",1);
	setenv("MH_DEV_CARLIFE_BLUETOOTH_INTERNAL_UI", "1",1);
	setenv("MH_DEV_CARLIFE_VOICE_WAKEUP", "0",1);
	setenv("MH_DEV_CARLIFE_FOCUS_UI", "1", 1);
	setenv("MH_APPLE_AUTH_COPROCESSOR_ADDRESS","/dev/i2c-4",1);	

	int i =	get_cpu_stepping(); 
	g_message("\n\n\n   cpu  = %d   \n\n\n\n",i);
	if( i <= 8 )
	{
		setenv("MH_I2C_DEV_ADDR","16",1);	
	}else{
		setenv("MH_I2C_DEV_ADDR","17",1);	
	}
	setenv("MH_CARPLAY_AUDIO_SINK_NAME","ahsink",1);	

//	setenv("LD_LIBRARY_PATH","/usr/lib/media-libva",1);	
//	setenv("LIBVA_DRIVERS_PATH","/usr/lib",1);
//	setenv("LIBVA_DRIVER_NAME","iHD",1);
	setenv("MH_KERNEL_DRIVE_PATH","/lib/modules/4.1.27-HS7/kernel/drivers/usb/gadget/",1);

	setenv( "MH_EA_PROTOCOL_NAME", "com.baidu.CarLifeVehicleProtocol", 1);
	system( "echo peripheral > /sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state");
	usleep( 100000 );
	system( "modprobe dwc3-pci");
	system( "modprobe dwc3");
	usleep( 100000 );
	system( "modprobe libcomposite");
	system( "modprobe udc_core");
	system( "modprobe configfs");
	system( "modprobe u_ether");
	system( "modprobe usb_f_ncm");
	system( "modprobe usb_f_fs");
	system( "modprobe g_ffs");
	system( "modprobe portmux_intel_drcfg");

	system( "echo host > /sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state");

	system( "modprobe xhci_hcd");
	system( "modprobe xhci_plat_hcd");
	system( "modprobe xhci_pci");

#endif
	g_message("signal SIGUSR1 before sample-server!!!!!");
	sem_init(&db_open_sem, 0, 0);
	sem_init(&adb_wait_sem, 0, 0);

	int sig_ret = signal(SIGUSR1, usr1_sig_handler);
	if(sig_ret == SIG_ERR)
	{
		g_message("signal SIGUSR1 error:[%s]\n", strerror(errno));
		sem_post(&db_open_sem);
		sem_post(&adb_wait_sem);
	}
//	pthread_t db_open_t;
//	pthread_create(&db_open_t, NULL, db_open_thread, NULL);
	MHServerEventsListener * _listener	=	(MHServerEventsListener *)g_new0( MHServerEventsListener, 1);

	_listener->callback	=	server_event_arrived;
	_listener->user_data	=	NULL;

	MHIPCServer * _server	=	mh_ipc_server_create();

	mh_ipc_server_register_events_listener( _server,  _listener);

	mh_ipc_start_media_engine( _server );

//	mh_ipc_start_streaming_engine( _server );

	mh_ipc_server_run( _server );
	sem_destroy(&db_open_sem);
	sem_destroy(&adb_wait_sem);

	return 0;
}				/* ----------  end of function main  ---------- */

