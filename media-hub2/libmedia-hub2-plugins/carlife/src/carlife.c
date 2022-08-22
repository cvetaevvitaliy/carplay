/*
 * =====================================================================================
 *
 *       Filename:  carlife.c
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

#include <linux/input.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mh_core.h>
#include <glib.h>
#include <CCarLifeLibWrapper_c.h>
#include <stdlib.h>
#include <stdio.h>
#include <dev_carlife.h> 
#include <string.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <mh_dev.h>
#include <mh_pb.h>
#include <sys/poll.h>
#include <libudev.h>
#include <blkid/blkid.h>
#include <debug.h>
#include <mh_misc.h>
#include <CTransPackageProcess.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <semaphore.h>
#include <libusb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
static struct udev * udevCtx;
static GPollFD udevFd;
static struct udev_monitor * udevMonitor;

#define ADB_WAIT_FOR_DEVICE  "adb -d wait-for-device"
#define ADB_GET_DEVICE_SERIAL	"adb -d devices"
//#define ADB_GET_SERIAL "adb get-serialno"
//#define ADB_GET_ANDROID_VER "adb -d shell getprop ro.build.version.release"
//#define ADB_LIST_CARLIFE_PACKAGE "adb shell pm list packages | grep carlife"

#define MAX_CMD_RET_LENGTH 1024*20

#define DUP_CARLIFE_DATA 1

#ifdef DUP_CARLIFE_DATA
FILE * videoFp	=	NULL;
FILE * audioFp	=	NULL;

static int audioname =	10000;
#endif

S_VIDEO_ENCODER_INIT_t initVideoParam	=	{800, 480,  30};
//S_STATISTICS_INFO_t statisticsInfo	=	{"123445","1.1", 1, "20282103", 0,1,0,""};
S_STATISTICS_INFO_t statisticsInfo	=	{"123445","1.1", 1, "20362102", 0,1,0,""};

//GstElement * src;
MHDevCarlife * _carlife	=	NULL;

//add for fastboot start
extern gboolean usr1_sig_once_flag = TRUE;
extern sem_t adb_wait_sem;
//add for fastboot end

//add for aoa carlife start
#define DEBUG_SWITCH 1
#if DEBUG_SWITCH
#define g_message_dbg(format,...) g_message("carlife.c:%d----->%s------->"format, __LINE__, __func__, ##__VA_ARGS__)
#else
#define g_message_dbg(format,...)
#endif

typedef enum
{
	AOA_CARLIFE_CONNECT = 0,
	AOA_CARLIFE_DISCONNECT
}AOA_CARLIFE_STATUS;

AOA_CARLIFE_STATUS aoa_carlife_status = AOA_CARLIFE_DISCONNECT;

typedef enum
{
	AOA_CHANNEL_ID_CMD = 1,
	AOA_CHANNEL_ID_VIDEO = 2,
	AOA_CHANNEL_ID_AUDIO = 3,
	AOA_CHANNEL_ID_AUDIO_TTS = 4,
	AOA_CHANNEL_ID_AUDIO_VR = 5,
	AOA_CHANNEL_ID_TOUCH = 6
}AOA_CHANNEL_ID;

#define USB_ACCESSORY_VENDOR_ID             0x18D1
//AOA1.0 Specific
#define USB_ACCESSORY_PRODUCT_ID            0x2D00
#define USB_ACCESSORY_ADB_PRODUCT_ID        0x2D01

//AOA2.0 Specific
#define USB_AUDIO_PRODUCT_ID                0x2D02
#define USB_AUDIO_ADB_PRODUCT_ID            0x2D03
#define USB_ACCESSORY_AUDIO_PRODUCT_ID      0x2D04
#define USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID  0x2D05
#define ACCESSORY_PID 						0x2D01
#define ACCESSORY_PID_ALT 					0x2D00

#define ACCESSORY_STRING_MANUFACTURER       0
#define ACCESSORY_STRING_MODEL              1
#define ACCESSORY_STRING_DESCRIPTION        2
#define ACCESSORY_STRING_VERSION            3
#define ACCESSORY_STRING_URI                4
#define ACCESSORY_STRING_SERIAL             5

#define ACCESSORY_GET_PROTOCOL              51
#define ACCESSORY_SEND_STRING               52
#define ACCESSORY_START                     53

#define CMD_SOCKET_PORT_HU	                7201
#define VIDEO_SOCKET_PORT_HU	            8200
#define MEDIA_SOCKET_PORT_HU                9200
#define TTS_SOCKET_PORT_HU	                9201
#define VR_SOCKET_PORT_HU	                9202
#define TOUCH_SOCKET_PORT_HU		        9300

#define CMD_HEAD_LEN                        8

#define AOA_HEAD_LEN                        8
#define CMD_DATA_SIZE                       40 * 1024
#define VIDEO_HEAD_LEN                      12
#define VIDEO_DATA_SIZE                     600*1024//normally VIDEO_DATA_SIZE<300X1024

#define MEDIA_HEAD_LEN                      12
#define MEDIA_DATA_SIZE                     100*1024

#define TTS_HEAD_LEN                        12
#define TTS_DATA_SIZE                       50*1024//normally TTS_DATA_SIZE<10x1024

#define VR_HEAD_LEN                         12
#define VR_DATA_SIZE                        50*1024//normally VR_DATA_SIZE<10x1024

#define TOUCH_HEAD_LEN                       8
#define TOUCH_DATA_SIZE                      10*1024

#define LOCAL_IP_ADDR "127.0.0.1"

#define CREATE_SOCKET_SERVER(PORT, server, client)\
	g_message_dbg("CREATE_SOCKET_SERVER start");\
    struct sockaddr_in server_addr;\
    memset(&server_addr, 0, sizeof(struct sockaddr_in));\
    server_addr.sin_family = AF_INET;\
    server_addr.sin_port = htons(PORT);\
    server_addr.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDR);\
    server = socket(AF_INET, SOCK_STREAM, 0);\
    if (server == -1) {\
    	g_message_dbg("socket error[%s]", strerror(errno));\
        return NULL;\
    }\
    int on = 1;\
    if ( setsockopt ( server, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )\
	{\
    	g_message_dbg("set sockoprt error[%s]", strerror(errno));\
    	return NULL;\
	}\
    int bind_result = bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr));\
    if (bind_result == -1) {\
    	g_message_dbg("bind error[%s]", strerror(errno));\
        return NULL;\
    }\
	int listen_result = listen(server, 1);\
    if (listen_result == -1) {\
    	g_message_dbg("listen error[%s]", strerror(errno));\
        return NULL;\
    }\
    struct sockaddr_in client_address;\
    socklen_t address_len = sizeof(struct sockaddr_in);\
	client = accept(server, (struct sockaddr *)&client_address, &address_len);\
    if (client == -1) {\
    	g_message_dbg("accept error[%s]", strerror(errno));\
        return NULL;\
    }\
	g_message_dbg("CREATE_SOCKET_SERVER end");

#define READ_CLIENT_DATA(client, headlen, datalen)\
	guchar* recv_msg = g_new0(guchar, CMD_HEAD_LEN + CMD_DATA_SIZE);\
	while(1)\
	{\
		int byte_num_head = recv(client,recv_msg,headlen,0);\
		if(byte_num_head < 0)\
		{\
			g_message_dbg("get "#client" head data from HU to MD error[%s]", strerror(errno));\
			break;\
		}\
		if(0 == byte_num_head)\
		{\
			g_message_dbg("get "#client" head data from HU len is 0");\
			continue;\
		}\
/*		g_message_dbg("get "#client" head data from HU to MD[%d]:", byte_num_head);*/\
		AOA_DEBUG_HEX_DISPLAY(recv_msg, headlen)\
		uint32_t msg_len = 0;\
		int byte_num_data = 0;\
		if(8 == byte_num_head)\
		{\
			msg_len = 0x00000000 | recv_msg[0] << 8 | recv_msg[1];\
/*			g_message_dbg("get head data PB length from HU to MD[%d]:", msg_len);*/\
		}\
		else if(12 == byte_num_head)\
		{\
			msg_len = recv_msg[0] << 24 | recv_msg[1] << 16 | recv_msg[2] << 8 | recv_msg[3];\
/*			g_message_dbg("get head data PB length from HU to MD[%d]:", msg_len);*/\
		}\
		else\
		{\
			g_message_dbg("get head data PB length from HU to MD wrong!");\
		}\
		if(msg_len != 0)\
		{\
			byte_num_data = recv(client,recv_msg + headlen,msg_len,0);\
			if(byte_num_data < 0)\
			{\
				g_message_dbg("get msg data from HU to MD error[%s]", strerror(errno));\
				break;\
			}\
/*			g_message_dbg("get msg data from HU to MD[%d]:", byte_num_data);*/\
			AOA_DEBUG_HEX_DISPLAY(recv_msg + headlen, 32)\
		}

#define CLOSE_SOCKET_SERVER_AND_CLIENT(server, client)\
	if(server != -1)\
	{\
		close(server);\
		server = -1;\
	}\
	if(client != -1)\
	{\
		close(client);\
		client = -1;\
	}
#define DEBUG_SWITCH_HEX 0
#if DEBUG_SWITCH_HEX
#define AOA_DEBUG_HEX_DISPLAY(D, L) \
{ \
	char _tmpChar[100] = {0}; \
	int _cc, i; \
\
	printf( "%lld\n",(long long int) (L) );\
	fprintf(stdout, "*******HEX DISPLAY BEGIN (%03lld bytes)******\n",(long long int) (L));\
\
	for(_cc = 0; _cc < (L); _cc += 16) \
	{ \
		memset(_tmpChar, 0, sizeof(_tmpChar)); \
\
		for(i = 0; i < 16 && i + _cc < (L); i ++) \
		{ \
			_tmpChar[i * 3]     =   ((D)[_cc + i] >> 4)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 1] =   ((D)[_cc + i] & 0x0F)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 2] =   ' '; \
		} \
\
		fprintf(stdout, "\t%s\n",  _tmpChar); \
	} \
	fprintf(stdout, "************HEX DISPLAY END*************\n");\
}
#else
#define AOA_DEBUG_HEX_DISPLAY(D, L)
#endif

#define AOA_DEBUG_HEX_DISPLAY_FOR_READ(D, L) \
{ \
	char _tmpChar[100] = {0}; \
	int _cc, i; \
\
	printf( "%lld\n",(long long int) (L) );\
	fprintf(stdout, "*******HEX DISPLAY BEGIN (%03lld bytes)******\n",(long long int) (L));\
\
	for(_cc = 0; _cc < (L); _cc += 16) \
	{ \
		memset(_tmpChar, 0, sizeof(_tmpChar)); \
\
		for(i = 0; i < 16 && i + _cc < (L); i ++) \
		{ \
			_tmpChar[i * 3]     =   ((D)[_cc + i] >> 4)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 1] =   ((D)[_cc + i] & 0x0F)["0123456789ABCDEF"]; \
			_tmpChar[i * 3 + 2] =   ' '; \
		} \
\
		fprintf(stdout, "\t%s\n",  _tmpChar); \
	} \
	fprintf(stdout, "************HEX DISPLAY END*************\n");\
}

libusb_context *usb_ctx = NULL;                  //need libusb_exit when disconnect
struct libusb_device_handle* usb_handle = NULL;  //need libusb_close() when disconnect
int aoa_protocol_version = -1;
static guchar aoa_in_ep = 0xFF;
static guchar aoa_out_ep = 0xFF;
static const unsigned char* aoa_manufacturer = "Baidu";
static const unsigned char* aoa_model = "CarLife";
static const unsigned char* aoa_description = "Baidu CarLife";
static const unsigned char* aoa_version = "1.0.0";
static const unsigned char* aoa_uri = "http://carlife.baidu.com/";
static const unsigned char* aoa_serial = "0720SerialNo.";

static int connect_aoa_device();
static int search_aoa_device();
static int get_aoa_protocol();
static int find_aoa_end_point(libusb_device *dev);
static void start_aoa_socekt_server_threads();
static void start_aoa_read_thread();
static void close_all_aoa_socket();

pthread_t aoa_cmd_thread_t;
pthread_t aoa_media_thread_t;
pthread_t aoa_video_thread_t;
pthread_t aoa_tts_thread_t;
pthread_t aoa_vr_thread_t;
pthread_t aoa_touch_thread_t;
pthread_t aoa_read_thread_t;

pthread_mutex_t aoa_write_mutex;

void* aoa_cmd_thread(void* param);
void* aoa_media_thread(void* param);
void* aoa_video_thread(void* param);
void* aoa_tts_thread(void* param);
void* aoa_vr_thread(void* param);
void* aoa_touch_thread(void* param);
void* aoa_read_thread(void* param);

int aoa_cmd_server_socket_fd = -1;
int aoa_media_server_socket_fd = -1;
int aoa_video_server_socket_fd = -1;
int aoa_tts_server_socket_fd = -1;
int aoa_vr_server_socket_fd = -1;
int aoa_touch_server_socket_fd = -1;

int aoa_cmd_client_socket_fd = -1;
int aoa_media_client_socket_fd = -1;
int aoa_video_client_socket_fd = -1;
int aoa_tts_client_socket_fd = -1;
int aoa_vr_client_socket_fd = -1;
int aoa_touch_client_socket_fd = -1;

uint16_t dev_id_vendor = 0xFFFF;
uint16_t dev_id_product = 0xFFFF;
uint16_t aoa_id_vendor = 0xFFFF;
uint16_t aoa_id_product = 0xFFFF;

//add for aoa carlife end

static gboolean busCallback (GstBus *bus,GstMessage *message,gpointer data);
static void deep_setupSrc( GObject * object, GstObject * orig, GParamSpec * pspec, gchar ** excluded_props );
static void _enoughData( GstElement * src, gpointer user_data );
void *  adb_wait_for_device( void * user_data );
static uint64_t	UpTicks( void )
{
	uint64_t			nanos;
	struct timespec		ts;
	
	ts.tv_sec  = 0;
	ts.tv_nsec = 0;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	nanos = ts.tv_sec;
	nanos *= 1000000000;
	nanos += ts.tv_nsec;
	return( nanos );
}

void carlife_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data );
void carlife_detach_event(	MHDev * dev, void * user_data);
void carlife_status_cb( MHDev * dev, MHDevStatus status, void * user_data);
MHDevicesListener _carlifeListener	=	
{
	.callback	=	carlife_event,
	.user_data	=	NULL
};
MHDevDetachListener _carlifeDetachListener	=	
{
	.callback	=	carlife_detach_event,
	.user_data	=	NULL
};
MHDevStatusListener _carlifeStatusListener	=	
{
	.callback	=	carlife_status_cb,
	.user_data	=	NULL
};

static void close_all_aoa_socket()
{
	g_message_dbg("IN");
	if(aoa_cmd_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_cmd_server_socket_fd!");
		close(aoa_cmd_server_socket_fd);
		shutdown(aoa_cmd_server_socket_fd, SHUT_RDWR);
		aoa_cmd_server_socket_fd = -1;
	}

	if(aoa_media_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_media_server_socket_fd!");
		close(aoa_media_server_socket_fd);
		shutdown(aoa_media_server_socket_fd, SHUT_RDWR);
		aoa_media_server_socket_fd = -1;
	}

	if(aoa_video_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_video_server_socket_fd!");
		close(aoa_video_server_socket_fd);
		shutdown(aoa_video_server_socket_fd, SHUT_RDWR);
		aoa_video_server_socket_fd = -1;
	}

	if(aoa_tts_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_tts_server_socket_fd!");
		close(aoa_tts_server_socket_fd);
		shutdown(aoa_tts_server_socket_fd, SHUT_RDWR);
		aoa_tts_server_socket_fd = -1;
	}

	if(aoa_vr_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_vr_server_socket_fd!");
		close(aoa_vr_server_socket_fd);
		shutdown(aoa_vr_server_socket_fd, SHUT_RDWR);
		aoa_vr_server_socket_fd = -1;
	}

	if(aoa_touch_server_socket_fd != -1)
	{
		g_message_dbg("close aoa_touch_server_socket_fd!");
		close(aoa_touch_server_socket_fd);
		shutdown(aoa_touch_server_socket_fd, SHUT_RDWR);
		aoa_touch_server_socket_fd = -1;
	}

	if(aoa_cmd_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_cmd_client_socket_fd!");
		close(aoa_cmd_client_socket_fd);
		shutdown(aoa_cmd_client_socket_fd, SHUT_RDWR);
		aoa_cmd_client_socket_fd = -1;
	}

	if(aoa_media_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_media_client_socket_fd!");
		close(aoa_media_client_socket_fd);
		shutdown(aoa_media_client_socket_fd, SHUT_RDWR);
		aoa_media_client_socket_fd = -1;
	}

	if(aoa_video_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_video_client_socket_fd!");
		close(aoa_video_client_socket_fd);
		shutdown(aoa_video_client_socket_fd, SHUT_RDWR);
		aoa_video_client_socket_fd = -1;
	}

	if(aoa_tts_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_tts_client_socket_fd!");
		close(aoa_tts_client_socket_fd);
		shutdown(aoa_tts_client_socket_fd, SHUT_RDWR);
		aoa_tts_client_socket_fd = -1;
	}

	if(aoa_vr_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_vr_client_socket_fd!");
		close(aoa_vr_client_socket_fd);
		shutdown(aoa_vr_client_socket_fd, SHUT_RDWR);
		aoa_vr_client_socket_fd = -1;
	}

	if(aoa_touch_client_socket_fd != -1)
	{
		g_message_dbg("close aoa_touch_client_socket_fd!");
		close(aoa_touch_client_socket_fd);
		shutdown(aoa_touch_client_socket_fd, SHUT_RDWR);
		aoa_touch_client_socket_fd = -1;
	}

	g_message_dbg("OUT");
}

void* aoa_read_thread(void* param)
{
	g_message_dbg("IN");
	unsigned char* msg_head = (unsigned char*)malloc(AOA_HEAD_LEN);
	unsigned char* msg = (unsigned char*)malloc(VIDEO_DATA_SIZE);
//	msg_head = g_new0(guchar, AOA_HEAD_LEN);
//	msg = g_new0(guchar, VIDEO_DATA_SIZE);
	uint32_t channel_id;
	uint32_t msg_len;
	int ret, transferred;
	while(AOA_CARLIFE_CONNECT == aoa_carlife_status)
	{
//		g_message_dbg("start read aoa head data");
		ret = libusb_bulk_transfer(usb_handle, aoa_in_ep, msg_head, AOA_HEAD_LEN, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("read aoa head data error[%d][%s]", ret, strerror(errno));
			continue;
		}
		if(0 == transferred)
		{
			g_message_dbg("read aoa head data zero[%d][%d]", ret, transferred);
			continue;
		}
//		else if(0 == ret)
//		{
//			g_message_dbg("read aoa head data zero[%d][%d]", ret, transferred);
//			continue;
//		}
//		g_message_dbg("carlife msg len in aoa head is[%d] : 0x%02X%02X%02X%02X%02X%02X%02X%02X", transferred, \
				msg_head[0], msg_head[1], msg_head[2], msg_head[3], msg_head[4], msg_head[5], msg_head[6], msg_head[7]);
//		AOA_DEBUG_HEX_DISPLAY_FOR_READ(msg_head, 8)
//		g_message_dbg("read aoa head msg len is %d", transferred);
		channel_id = (msg_head[0] << 24) | (msg_head[1] << 16) | (msg_head[2] << 8) | (msg_head[3]);
//		g_message_dbg("aoa channel id is %08X", channel_id);
		msg_len = (msg_head[4] << 24) | (msg_head[5] << 16) | (msg_head[6] << 8) | (msg_head[7]);
//		g_message_dbg("aoa msg len is %08X", msg_len);
//		g_message_dbg("aoa carlife msg len is %d(before read)", msg_len);
//		g_message_dbg("start read aoa msg data");
		ret = libusb_bulk_transfer(usb_handle, aoa_in_ep, msg, (int)msg_len, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("read carlife msg error[%d][%s]", ret, strerror(errno));
			continue;
		}
		if(0 == transferred)
		{
			g_message_dbg("read carlife msg zero[%d][%d]", ret, transferred);
			continue;
		}
//		g_message_dbg("aoa carlife msg len is %d(after read)", transferred);
		AOA_DEBUG_HEX_DISPLAY(msg, 32)
		switch(channel_id)
		{
			case 1:
				if(ret = send(aoa_cmd_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_cmd_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_cmd_client_socket_fd data success![%d]", ret);
				}
				break;
			case 2:
				if(ret = send(aoa_video_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_video_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_video_client_socket_fd data success![%d]", ret);
				}
				break;
			case 3:
				if(ret = send(aoa_media_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_media_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_media_client_socket_fd data success![%d]", ret);
				}
				break;
			case 4:
				if(ret = send(aoa_tts_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_tts_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_tts_client_socket_fd data success![%d]", ret);
				}
				break;
			case 5:
				if(ret = send(aoa_vr_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_vr_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_vr_client_socket_fd data success![%d]", ret);
				}
				break;
			case 6:
				if(ret = send(aoa_touch_client_socket_fd, msg, transferred, 0) < 0)
				{
					g_message_dbg("send aoa_touch_client_socket_fd data failed![%s]", strerror(errno));
				}
				else
				{
//					g_message_dbg("send aoa_touch_client_socket_fd data success![%d]", ret);
				}
				break;
			default:
				g_message_dbg("bad channel id!!!!![%d]", channel_id);
				break;
		}
	}
	g_free(msg_head);
	g_free(msg);
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_cmd_thread(void* param)
{
	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(CMD_SOCKET_PORT_HU, aoa_cmd_server_socket_fd, aoa_cmd_client_socket_fd)
	int transferred, ret;
	READ_CLIENT_DATA(aoa_cmd_client_socket_fd, CMD_HEAD_LEN, CMD_DATA_SIZE)
		uint32_t aoa_msg_len = byte_num_head + byte_num_data;
		g_message_dbg("aoa_msg_len is : %d", aoa_msg_len);
		uint8_t aoa_head_msg[8] = {0x00, 0x00, 0x00, 0x01, aoa_msg_len >> 24 & 0x000000FF, aoa_msg_len >> 16 & 0x000000FF, aoa_msg_len >> 8 & 0x000000FF, aoa_msg_len & 0x000000FF};
		AOA_DEBUG_HEX_DISPLAY(aoa_head_msg, 8)
		pthread_mutex_lock(&aoa_write_mutex);
		//write head data to aoa out endpoint
		ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, aoa_head_msg, 8, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("write aoa_cmd_client_socket_fd aoa head data from HU to MD failed![%d]", ret);
			pthread_mutex_unlock(&aoa_write_mutex);
			break;
		}
		g_message_dbg("write aoa head data from HU to MD success![%d, %d]", ret, transferred);
		//write msg data to aoa out endpoint
		ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, recv_msg, byte_num_head + byte_num_data, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("write aoa msg data from HU to MD failed![%d]", ret);
			pthread_mutex_unlock(&aoa_write_mutex);
			break;
		}
		g_message_dbg("write aoa msg data from HU to MD success![%d, %d]", ret, transferred);
		pthread_mutex_unlock(&aoa_write_mutex);
	}
	g_free(recv_msg);
	recv_msg = 0;
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_cmd_server_socket_fd, aoa_cmd_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_media_thread(void* param)
{
	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(MEDIA_SOCKET_PORT_HU, aoa_media_server_socket_fd, aoa_media_client_socket_fd)
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_media_server_socket_fd, aoa_media_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_video_thread(void* param)
{
	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(VIDEO_SOCKET_PORT_HU, aoa_video_server_socket_fd, aoa_video_client_socket_fd)
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_video_server_socket_fd, aoa_video_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_tts_thread(void* param)
{

	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(TTS_SOCKET_PORT_HU, aoa_tts_server_socket_fd, aoa_tts_client_socket_fd)
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_tts_server_socket_fd, aoa_tts_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_vr_thread(void* param)
{
	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(VR_SOCKET_PORT_HU, aoa_vr_server_socket_fd, aoa_vr_client_socket_fd)
	int transferred, ret;
	READ_CLIENT_DATA(aoa_vr_client_socket_fd, VR_HEAD_LEN, VR_DATA_SIZE)
		uint32_t aoa_msg_len = byte_num_head + byte_num_data;
//		g_message_dbg("aoa_msg_len is : %d", aoa_msg_len);
		uint8_t aoa_head_msg[8] = {0x00, 0x00, 0x00, 0x05, aoa_msg_len >> 24 & 0x000000FF, aoa_msg_len >> 16 & 0x000000FF, aoa_msg_len >> 8 & 0x000000FF, aoa_msg_len & 0x000000FF};
		AOA_DEBUG_HEX_DISPLAY(aoa_head_msg, 8)
		pthread_mutex_lock(&aoa_write_mutex);
		//write head data to aoa out endpoint
		ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, aoa_head_msg, 8, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("write aoa head data from HU to MD failed![%d]", ret);
			pthread_mutex_unlock(&aoa_write_mutex);
			break;
		}
//		g_message_dbg("write aoa head data from HU to MD success![%d, %d]", ret, transferred);
		//write msg data to aoa out endpoint
		ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, recv_msg, byte_num_head + byte_num_data, &transferred, 1000);
		if(ret < 0)
		{
			g_message_dbg("write aoa msg data from HU to MD failed![%d]", ret);
			pthread_mutex_unlock(&aoa_write_mutex);
			break;
		}
//		g_message_dbg("write aoa msg data from HU to MD success![%d, %d]", ret, transferred);
		pthread_mutex_unlock(&aoa_write_mutex);
	}
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_vr_server_socket_fd, aoa_vr_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

void* aoa_touch_thread(void* param)
{
	g_message_dbg("IN");
	CREATE_SOCKET_SERVER(TOUCH_SOCKET_PORT_HU, aoa_touch_server_socket_fd, aoa_touch_client_socket_fd)
	int transferred, ret;
//	READ_CLIENT_DATA(aoa_touch_client_socket_fd, TOUCH_HEAD_LEN, TOUCH_DATA_SIZE)
	guchar* recv_msg = g_new0(guchar, 8 + 40 * 1024);
	while(1)
	{
		int byte_num_head = recv(aoa_touch_client_socket_fd,recv_msg,8,0);
		if(byte_num_head < 0)
		{
			g_message("carlife.c:%d----->%s------->""get ""aoa_touch_client_socket_fd"" head data from HU to MD error[%s]", 694, __func__, strerror(errno));
			break;
		}
		if(0 == byte_num_head)
		{
			g_message("carlife.c:%d----->%s------->""get ""aoa_touch_client_socket_fd"" head data from HU len is 0", 694, __func__);
			continue;
		}
//		g_message("carlife.c:%d----->%s------->""get ""aoa_touch_client_socket_fd"" head data from HU to MD[%d]:", 694, __func__, byte_num_head);
		uint32_t msg_len = 0;
		int byte_num_data = 0;
		if(8 == byte_num_head)
		{
			msg_len = 0x00000000 | recv_msg[0] << 8 | recv_msg[1];
//			g_message("carlife.c:%d----->%s------->""get head data PB length from HU to MD[%d]:", 694, __func__, msg_len);
		}
		else if(12 == byte_num_head)
		{
			msg_len = recv_msg[0] << 24 | recv_msg[1] << 16 | recv_msg[2] << 8 | recv_msg[3];
//			g_message("carlife.c:%d----->%s------->""get head data PB length from HU to MD[%d]:", 694, __func__, msg_len);
		}
		else
		{
			g_message("carlife.c:%d----->%s------->""get head data PB length from HU to MD wrong!", 694, __func__);
		}
		if(msg_len != 0)
		{
			byte_num_data = recv(aoa_touch_client_socket_fd,recv_msg + 8,msg_len,0);
			if(byte_num_data < 0)
			{
				g_message("carlife.c:%d----->%s------->""get msg data from HU to MD error[%s]", 694, __func__, strerror(errno));
				break;
			}
//			g_message("carlife.c:%d----->%s------->""get msg data from HU to MD[%d]:", 694, __func__, byte_num_data);
		}
	uint32_t aoa_msg_len = byte_num_head + byte_num_data;
//	g_message_dbg("aoa_msg_len is : %d", aoa_msg_len);
	uint8_t aoa_head_msg[8] = {0x00, 0x00, 0x00, 0x01, aoa_msg_len >> 24 & 0x000000FF, aoa_msg_len >> 16 & 0x000000FF, aoa_msg_len >> 8 & 0x000000FF, aoa_msg_len & 0x000000FF};
	AOA_DEBUG_HEX_DISPLAY(aoa_head_msg, 8)
	pthread_mutex_lock(&aoa_write_mutex);
	//write head data to aoa out endpoint
	ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, aoa_head_msg, 8, &transferred, 1000);
	if(ret < 0)
	{
		g_message_dbg("write aoa head data from HU to MD failed![%d]", ret);
		pthread_mutex_unlock(&aoa_write_mutex);
		break;
	}
//	g_message_dbg("write aoa head data from HU to MD success![%d, %d]", ret, transferred);
	//write msg data to aoa out endpoint
	ret = libusb_bulk_transfer(usb_handle, aoa_out_ep, recv_msg, byte_num_head + byte_num_data, &transferred, 1000);
	if(ret < 0)
	{
		g_message_dbg("write aoa msg data from HU to MD failed![%d]", ret);
		pthread_mutex_unlock(&aoa_write_mutex);
		break;
	}
//	g_message_dbg("write aoa msg data from HU to MD success![%d, %d]", ret, transferred);
	pthread_mutex_unlock(&aoa_write_mutex);
}
//	CLOSE_SOCKET_SERVER_AND_CLIENT(aoa_touch_server_socket_fd, aoa_touch_client_socket_fd)
	g_message_dbg("OUT");
	return NULL;
}

static void start_aoa_socekt_server_threads()
{
	g_message_dbg("IN");
	int pthread_res;
	pthread_res=pthread_create(&aoa_cmd_thread_t, NULL, aoa_cmd_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_cmd_thread create failed errno:%d", pthread_res);
		return;
	}

	pthread_res=pthread_create(&aoa_media_thread_t, NULL, aoa_media_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_media_thread create failed errno:%d", pthread_res);
		return;
	}

	pthread_res=pthread_create(&aoa_video_thread_t, NULL, aoa_video_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_video_thread create failed errno:%d", pthread_res);
		return;
	}

	pthread_res=pthread_create(&aoa_tts_thread_t, NULL, aoa_tts_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_tts_thread create failed errno:%d", pthread_res);
		return;
	}

	pthread_res=pthread_create(&aoa_vr_thread_t, NULL, aoa_vr_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_vr_thread create failed errno:%d", pthread_res);
		return;
	}

	pthread_res=pthread_create(&aoa_touch_thread_t, NULL, aoa_touch_thread, NULL);
	if(pthread_res!=0)
	{
		g_message_dbg("aoa_cmd_thread create failed errno:%d", pthread_res);
		return;
	}
	g_message_dbg("OUT");
	return;

}
static void start_aoa_read_thread()
{
	g_message_dbg("IN");
	pthread_create(&aoa_read_thread_t, NULL, aoa_read_thread, NULL);
	g_message_dbg("OUT");
}

static int connect_aoa_device()
{
	g_message_dbg("IN");
	int ret = 0;
    unsigned char ioBuffer[2];
    int protocol;
    int res;
    int tries = 5;
//    uint16_t idVendor, idProduct;

    // Search for AOA support device in the all USB devices
    if ((protocol=search_aoa_device()) < 0) {
    	g_message_dbg("AOA device not found.");
        return -1;
    }
    //already in accessory mode ?
    if( protocol == 0 ) {
    	g_message_dbg("already in accessory mode![%d]", aoa_carlife_status);
    	if(AOA_CARLIFE_CONNECT != aoa_carlife_status)
    	{
    	       usb_handle = libusb_open_device_with_vid_pid(usb_ctx, aoa_id_vendor, aoa_id_product);
    	       libusb_claim_interface(usb_handle, 0);
    	       aoa_carlife_status = AOA_CARLIFE_CONNECT;
    	       start_aoa_socekt_server_threads();
    			_carlife	=	g_object_new(MH_TYPE_DEV_CARLIFE, "io-name", "carlife", NULL);
    			_carlife->exist = 	true;

    			guchar   _serial[32];
    			memset(_serial, 0, 32);
    			struct libusb_device_descriptor desc;
    			libusb_get_device_descriptor(libusb_get_device(usb_handle), &desc);
    			ret = libusb_get_string_descriptor_ascii(usb_handle, desc.iSerialNumber, _serial, sizeof(_serial));
    			if (ret < 0) {
    				g_message_dbg("Get SerialNumber  failed\n");
    			} else {
    				g_message_dbg("\tSerialNumber : %s\n", _serial);
    			}
    			MH_DEV( _carlife)->serial	=	g_strndup( _serial, strlen(_serial) );

    			_carlife->md_os	=	g_strdup("android");
    			mh_core_attach_dev( MH_DEV( _carlife ));
    	       start_aoa_read_thread();
    	}
	       return 0;
    }

    usb_handle = libusb_open_device_with_vid_pid(usb_ctx, dev_id_vendor, dev_id_product);
    libusb_claim_interface(usb_handle, 0);
    usleep(1000);//sometimes hangs on the next transfer :(

    // Send accessory identifications
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 0, (unsigned char*)aoa_manufacturer, strlen(aoa_model) + 1, 0);
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 1, (unsigned char*)aoa_model, strlen(aoa_model) + 1, 0);
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 2, (unsigned char*)aoa_description, strlen(aoa_description) + 1, 0);
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 3, (unsigned char*)aoa_version, strlen(aoa_version) + 1, 0);
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 4, (unsigned char*)aoa_uri, strlen(aoa_uri) + 1, 0);
    libusb_control_transfer(usb_handle, 0x40, ACCESSORY_SEND_STRING, 0, 5, (unsigned char*)aoa_serial, strlen(aoa_serial) + 1, 0);

    // Switch to accessory mode
    ret = libusb_control_transfer(usb_handle,0x40,ACCESSORY_START,0,0,NULL,0,0);
    if(ret < 0){
    	g_message_dbg("Switch to accessory mode failed!");
        libusb_close(usb_handle);
        usb_handle = NULL;
        return -1;
    }

    if(usb_handle != NULL){
        libusb_close(usb_handle);
        usb_handle = NULL;
    }

    // Wait a moment
    usleep(10000);

    g_message_dbg("connect to new PID...");
    //attempt to connect to new PID, if that doesn't work try ACCESSORY_PID_ALT
//    for(;;){
//        tries--;
//        if(search_aoa_device(&idVendor, &idProduct) != 0 ){
//            continue;
//        }
//        if((usb_handle = libusb_open_device_with_vid_pid(usb_ctx, idVendor, idProduct)) == NULL){
//            if(tries < 0){
//                return -1;
//            }
//        }else{
//            break;
//        }
//        sleep(1);
//    }

//    ret = libusb_claim_interface(usb_handle, 0);
    if(ret < 0){
        return -3;
    }

    g_message_dbg("Established AOA connection.\n");
    g_message_dbg("OUT[%d]", ret);
	return ret;
}

static int search_aoa_device()
{
	g_message_dbg("IN");
	int ret = -1;

    int res;
    int i;
    libusb_device **devs;
    libusb_device *dev;
    struct libusb_device_descriptor desc;
    ssize_t devcount;

    res = -1;
    devcount = libusb_get_device_list(usb_ctx, &devs);
    if(devcount < 0){
    	g_message_dbg("Get device error.");
       return -1;
    }

    //enumerate USB deivces
    for(i=0; i<devcount; i++){
        dev = devs[i];
        libusb_get_device_descriptor(dev, &desc);
        g_message_dbg("VID:%04X, PID:%04X Class:%02X\n", desc.idVendor, desc.idProduct, desc.bDeviceClass);
        //Ignore non target device
        if( desc.bDeviceClass != 0 ){
            continue;
        }

        //Already AOA mode ?
        if(desc.idVendor == USB_ACCESSORY_VENDOR_ID &&
            (desc.idProduct >= USB_ACCESSORY_PRODUCT_ID &&
             desc.idProduct <= USB_ACCESSORY_AUDIO_ADB_PRODUCT_ID)
        ){
        	g_message_dbg("already in accessory mode.");
            aoa_id_vendor = desc.idVendor;
            aoa_id_product = desc.idProduct;
            g_message_dbg("aoa_id_vendor:%04X, aoa_id_product:%04X", aoa_id_vendor, aoa_id_product);
        	return 0;
        }

        if(0xFFFF != dev_id_vendor && 0xFFFF != dev_id_product)
        {
        	g_message_dbg("already get aoa device!\n");
        	return -1;
        }

        //Checking the AOA capability.
        if((usb_handle = libusb_open_device_with_vid_pid(usb_ctx, desc.idVendor,  desc.idProduct)) == NULL) {
        	g_message_dbg("Device open error.\n");
        } else {
                libusb_claim_interface(usb_handle, 0);
                ret = get_aoa_protocol();
                libusb_release_interface (usb_handle, 0);
                libusb_close(usb_handle);
                usb_handle = NULL;
                if( ret != -1 ){
                	g_message_dbg("AOA protocol version: %d", ret);
                	aoa_protocol_version = ret;
                    break; //AOA found.
                }
        }
    }

    if(-1 == ret)
    {
    	g_message_dbg("AOA device not found.");
    	return -1;
    }

     //find end point number
    if( 0xFF == aoa_in_ep && 0xFF == aoa_out_ep)
    {
    	if(find_aoa_end_point(dev) < 0 )
    	{
    		g_message_dbg("Endpoint not found.");
    		ret = -1;
    	}
    }

    dev_id_vendor = desc.idVendor;
    dev_id_product = desc.idProduct;
    g_message_dbg("dev_id_vendor:%04X, dev_id_product:%04X", dev_id_vendor, dev_id_product);

    g_message_dbg("OUT[%d]", ret);
	return ret;
}

static int get_aoa_protocol()
{
	g_message_dbg("IN");
    unsigned short protocol;
    unsigned char buf[2];
    int res;

    res = libusb_control_transfer(usb_handle, 0xc0, ACCESSORY_GET_PROTOCOL, 0, 0, buf, 2, 0);
    if(res < 0){
    	g_message_dbg("get aoa protocol error!");
        return -1;
    }
    protocol = buf[1] << 8 | buf[0];
    g_message_dbg("OUT[aoa protocol version is %u]", protocol);
    return((int)protocol);
}

static int find_aoa_end_point(libusb_device *dev)
{
	g_message_dbg("IN");
	int ret = 0;

    struct libusb_config_descriptor *config;
    libusb_get_config_descriptor (dev, 0, &config);

    //initialize end point number
//    aoa_in_ep = aoa_out_ep = 0;

    //Evaluate first interface and endpoint descriptor
    g_message_dbg("bNumInterfaces: %d", config->bNumInterfaces);
    const struct libusb_interface *itf = &config->interface[0];
    struct libusb_interface_descriptor ifd = itf->altsetting[0];
    g_message_dbg("bNumEndpoints: %d", ifd.bNumEndpoints);
    int i;
    for(i = 0; i < ifd.bNumEndpoints; i++){
        struct libusb_endpoint_descriptor epd;
        epd = ifd.endpoint[i];
        if( epd.bmAttributes == 2 ) { //Bulk Transfer ?
            g_message_dbg("bmAttributes is 2, epd.bEndpointAddress : %d", epd.bEndpointAddress);
            if( epd.bEndpointAddress & 0x80){ //IN
                if( aoa_in_ep == 0xFF )
                    aoa_in_ep = epd.bEndpointAddress;
            }else{                            //OUT
                if( aoa_out_ep == 0xFF )
                    aoa_out_ep = epd.bEndpointAddress;
            }
        }
        g_message_dbg(" aoa_in_ep: %02X, aoa_out_ep:%02X", aoa_in_ep, aoa_out_ep);
    }
    if( aoa_in_ep == 0 || aoa_out_ep == 0) {
        ret =  -1;
    }
    g_message_dbg("OUT[%d]", ret);
	return ret;
}

void carlife_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data)

{
	g_message("\n\n\n\n\n\n%s\n\n\n\n\n\n", __func__);
	char * _dev_type	=	NULL;

	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );

	if( _dev_type && strcmp( _dev_type, "iap2" ) == 0 )
	{
		MiscIapDeviceMode _mode	=	mh_misc_get_iap_device_mode();
		if( _mode	==	MISC_IAP_CARLIFE || _mode == MISC_IAP_CARPLAY_CARLIFE)
		{

			g_message("%s->%lld\n", __func__, __LINE__);
			_carlife	=	g_object_new(MH_TYPE_DEV_CARLIFE, "io-name", "carlife", NULL); 

			_carlife->md_os	=	strdup("ios");
			_carlife->iap2Dev	=	(MHDev *)mh_object_ref((MHObject *)dev);



			_carlifeStatusListener.user_data	=	_carlife;
			_carlifeDetachListener.user_data	=	_carlife;

			mh_dev_register_status_listener( dev, &_carlifeStatusListener );
			mh_dev_register_detach_listener( dev, &_carlifeDetachListener );

			mh_core_attach_dev(  MH_DEV( _carlife));
		}

	}
	if( _dev_type != NULL)
	{
		g_free( _dev_type);
	}

}
static gboolean  emit_iap_status( gpointer user_data)
{
	gpointer * _data	=	(gpointer *)user_data;
	MHDevCarlife * _carlife	=	(MHDevCarlife *)_data[0];
	MHDevStatus * _status	=	(MHDevStatus *)_data[1];
	if( *_status == IAP_AUTH_SUCCESS)
	{
		g_message("%s----->CARLIFE_IAP_AUTH_SUCCESS", __func__);
		g_signal_emit_by_name(MH_DEV( _carlife), "dev_status", CARLIFE_IAP_AUTH_SUCCESS);

		_carlife->status	=	CARLIFE_IAP_AUTH_SUCCESS;

	}
	else if(*_status	==	IAP_AUTH_START)
	{

		g_message("%s----->CARLIFE_IAP_AUTH", __func__);
		g_signal_emit_by_name(MH_DEV( _carlife), "dev_status", CARLIFE_IAP_AUTH);
		_carlife->status	=	CARLIFE_IAP_AUTH;
	}

	g_free( _status);
	g_free(_data);
	return FALSE;

}
void carlife_status_cb( MHDev * dev, MHDevStatus status, void * user_data)
{
	MHDevCarlife * _carlife	=	(MHDevCarlife *)user_data;
	if( status	==	IAP_AUTH_SUCCESS || status == IAP_AUTH_START )
	{
		g_message("%s--------------------------------->status-->%d", __func__, status);
		GSource * _source	=	g_idle_source_new();
		
		gpointer * user_data	=	g_malloc0( sizeof(gpointer)*2);
		MHDevStatus * _status	=	g_new0(MHDevStatus, 1);
		*_status	=	status;
		user_data[0]	=	(gpointer)_carlife;
		user_data[1]	=	(gpointer)_status;

		g_source_set_callback( _source, emit_iap_status, user_data, NULL);

		mh_io_dispatch(MH_IO( _carlife), _source);

		g_source_unref( _source);
	}

}
void carlife_detach_event(	MHDev * dev, void * user_data)
{
	g_message_dbg("IN");
	MHDevCarlife * _carlife	=	(MHDevCarlife *) user_data;

	if( dev == _carlife->iap2Dev)
	{
		g_message_dbg("11111");
		mh_object_unref( (MHObject *)dev);
		mh_core_detach_dev(  MH_DEV( _carlife));

		g_object_unref( _carlife);

	}
	g_message_dbg("OUT");

}	

S_AUTHEN_REQUEST_t authenRequest={"CarLife_1.0.0"};



GstClockTime timestamp = 0;
void videoHeartBeat(){
	g_message("videoHeartBeat() is invoked");
	g_message("video heart beat received!");
}
static void _enoughData( GstElement * src, gpointer user_data )
{
	g_message( "video %s", __func__ );
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _needData
 *  Description:
 * =====================================================================================
 */
static void _needData( GstAppSrc * src, guint arg1, gpointer user_data )
{
//	g_message( "\n   %s %d\n", __func__,arg1 );
}		/* -----  end of static function _needData  ----- */
static void deep_setupSrc( GObject * object, GstObject * orig, GParamSpec * pspec, gchar ** excluded_props )
{
	g_message("\n\n\n\n\n\n\ndeep_setupSrc\n\n\n\n\n\n\n");
	g_object_get( orig, pspec->name , & (_carlife->video_param->src), NULL);

//	_carlife->video_param->src	=	element;

	GstCaps * _caps;

	g_object_set( _carlife->video_param->src, "stream-type", GST_APP_STREAM_TYPE_STREAM,  
			"is-live", TRUE, "block", TRUE,
			"format", GST_FORMAT_TIME, NULL );
	g_object_set( _carlife->video_param->src, "do-timestamp", TRUE, NULL );

	g_object_set( _carlife->video_param->src, "min-latency",(gint64)0, NULL );

	g_object_set( _carlife->video_param->src, "max-latency", (gint64)0, NULL );
g_message("***********************%s--->width:%d, height:%d", __func__, _carlife->video_param->width, _carlife->video_param->height);
	_caps 	=	gst_caps_new_simple ("video/x-h264",
			"width", G_TYPE_INT, _carlife->video_param->width,
			"height", G_TYPE_INT, _carlife->video_param->height,
			"framerate", GST_TYPE_FRACTION, _carlife->video_param->frameRate, 1,
			NULL);

	g_object_set( _carlife->video_param->src, "caps", _caps, NULL);
	
	gst_caps_unref( _caps );
	g_signal_connect( _carlife->video_param->src, "need-data", G_CALLBACK( _needData ), NULL );
	g_signal_connect( _carlife->video_param->src, "enough-data", G_CALLBACK( _enoughData ), _carlife->video_param->src );

}		/* -----  end of static function deep_setupSrc  ----- */

static void _enoughAudioData( GstElement * src, gpointer user_data )
{
	g_message( "audio %s", __func__ );
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  _needAudioData
 *  Description:
 * =====================================================================================
 */
static void _needAudioData( GstAppSrc * src, guint arg1, gpointer user_data )
{
//	g_message( "\n\n\n\n\n\n   %s %d\n\n\n\n\n", __func__,arg1 );
}		/* -----  end of static function _needAudioData  ----- */





//void mediaStop(){
//	g_message("mediaStop() is invoked");
//	g_message("media stop status received!");
//}
//void mediaSeek(){
//	g_message("mediaSeek() is invoked");
//	g_message("media seek status received!");
//}


//static FILE * vr_file	=	NULL;

FILE *Fp	=	NULL;



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  check_bdsc
 *  Description:  
 * =====================================================================================
 */
static void check_bdsc()
{
//	return 0;
}		/* -----  end of static function check_bdsc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  adb_work
 *  Description:  
 * =====================================================================================
 */
static gboolean adb_work( gpointer object)
{ 
	MHDevCarlife * _carlife	=	(MHDevCarlife *)object;
	Cmd_Ret * _res;
//	char * _cmd	=	g_strdup_printf("adb -s %s forward tcp:7200 tcp:7240", MH_DEV(_carlife)->serial);
//	//					char * _cmd	=	g_strdup("adb forward tcp:7200 tcp:7240");
//	doCmd( _cmd);
//
//	g_free( _cmd);
//	_cmd	=	g_strdup_printf("adb -s %s forward tcp:8200 tcp:8240", MH_DEV(_carlife)->serial);
//	//					_cmd	=	g_strdup("adb forward tcp:8200 tcp:8240");
//	doCmd( _cmd);
//
//	g_free( _cmd);
//	_cmd	=	g_strdup_printf("adb -s %s forward tcp:9200 tcp:9240", MH_DEV(_carlife)->serial);
//	//					_cmd	=	g_strdup("adb forward tcp:9200 tcp:9240");
//	doCmd( _cmd);
//
//	g_free( _cmd);
//	_cmd	=	g_strdup_printf("adb -s %s forward tcp:9201 tcp:9241", MH_DEV(_carlife)->serial);
//	//					_cmd	=	g_strdup("adb forward tcp:9201 tcp:9241");
//	doCmd( _cmd);
//
//	g_free( _cmd);
//
//	_cmd	=	g_strdup_printf("adb -s %s forward tcp:9202 tcp:9242", MH_DEV(_carlife)->serial);
//	//					_cmd	=	g_strdup("adb forward tcp:9202 tcp:9242");
//	doCmd( _cmd);
//
//	g_free( _cmd);
//	_cmd	=	g_strdup_printf("adb -s %s forward tcp:9300 tcp:9340", MH_DEV(_carlife)->serial);
//	//					_cmd	=	g_strdup("adb forward tcp:9300 tcp:9340");
//	doCmd( _cmd);
//
//	g_free( _cmd);

	bool _systemNotRunning	= true;
	char * _list	=	g_strdup_printf("adb -s %s shell pm list packages", MH_DEV(_carlife)->serial); 
	while( _systemNotRunning)
	{
		_res	=	NULL;
		_res	=	Cmd_With_Result( _list);
		if( _res != NULL)
		{
			if( TRUE	==	g_str_has_prefix( _res->data, "Error"))
			{
				g_message("sleep 5");
				sleep(5);

			}
			else if( NULL != g_strrstr(_res->data, "carlife"))
			{
				_systemNotRunning	=	false;
				_carlife->exist = 	true;

			}
			else
			{
				_systemNotRunning = false;
				_carlife->exist	=	false;
			}
			g_free( _res);
		}
	}


	mh_core_attach_dev( MH_DEV( _carlife ));
	g_free( _list);
	g_message("carlife->%s*******************out", __func__);
	return G_SOURCE_REMOVE;

}		/* -----  end of static function adb_work  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _init
 *  Description:  
 * =====================================================================================
 */
void *  adb_wait_for_device( void * user_data )
{
	g_message("carlife.c->%s", __func__);
//	system(ADB_WAIT_FOR_DEVICE);
//	g_message("system------>out");

//	if(usr1_sig_once_flag)
//	{
//		g_message("carlife.c->%s-----wait adb_wait_sem before", __func__);
//		sem_wait(&adb_wait_sem);
//		usr1_sig_once_flag = FALSE;
//		g_message("carlife.c->%s-----wait adb_wait_sem end", __func__);
//	}

	Cmd_Ret * _res;
	bool _devFlag	=	false;
	int _serialStart = 0, _serialEnd = 0;

	doCmd( ADB_WAIT_FOR_DEVICE);
	while( _devFlag == false)
	{
		if(_res	=	Cmd_With_Result( ADB_GET_DEVICE_SERIAL)) 
		{
		
			g_message("%s--->%s-->%s", __func__, ADB_GET_DEVICE_SERIAL, _res->data);

			int _len	=	strlen( _res->data);
			g_message("%s--->_len=%d", __func__, _len);
			if( _len < 28)
			{
				adb_wait_for_device( _carlife);
				break;
			}
			else
			{
				int i=0;
				for( i; i< _len ;i++)
				{
					g_message("_res->data[i]:%c", _res->data[i]);
					if( _res->data[i] == 10)
					{
						_serialStart	=	i + 1;	
						g_message("_serialStart:%d", _serialStart);

					}
					if( _res->data[i]	==	9 && _res->data[i+1] == 'd')
					{
						_serialEnd = i;
	g_message("_serialEnd:%d", _serialEnd);

						_devFlag	= true;
						break;
					}
				}
				if( _devFlag == false)
				{
					sleep(1);
				}
			}



		}
		else
		{
			g_message("serial:%s", __func__);
			return NULL;
		}
	}
	//		g_message("%s", _res+27);
	//	carLifeLibInit();
	//	_res	=	Cmd_With_Result( ADB_GET_ANDROID_VER);
	//	if( _res 	!=	NULL)
	//	{
	//		_carlife->android_version	=	g_strdup( _res->data );
	//		g_message("android_version--->%s", _carlife->android_version);
	//	}
	_carlife	=	g_object_new(MH_TYPE_DEV_CARLIFE, "io-name", "carlife", NULL); 

	g_message("\n\n\n\n%s--->%s\nstart:%s\n--->%d\n", __func__, _res->data, _res->data + _serialStart, _serialEnd-_serialStart);
	MH_DEV( _carlife)->serial	=	g_strndup( _res->data + _serialStart, _serialEnd - _serialStart );

	g_message("%s-->serial:%s", __func__, MH_DEV( _carlife)->serial);

	_carlife->md_os	=	g_strdup("android");

	GSource * _source	=	g_idle_source_new();
	g_source_set_callback( _source, adb_work, _carlife, NULL);
	g_source_attach( _source, _carlife->pipe_param->context);
	g_source_unref( _source);
	g_free( _res);

	return NULL;

}		/* -----  end of static function _init  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _prepare
 *  Description:  
 * =====================================================================================
 */
static gboolean _prepare( GSource * source, gint * timeout )
{
	* timeout	=	-1;
	
	return FALSE;
}		/* -----  end of static function _prepare  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _check
 *  Description:  
 * =====================================================================================
 */
static gboolean _check( GSource * source )
{
	gboolean _ret	=	FALSE;

	if( udevFd.revents != 0 )
	{
		_ret	=	TRUE;
	}

	return _ret;
}		/* -----  end of static function _check  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _dispatch
 *  Description:  
 * =====================================================================================
 */
static gboolean _dispatch( GSource * source, GSourceFunc callback, gpointer user_data )
{
	return callback( user_data );
}		/* -----  end of static function _dispatch  ----- */

static GSourceFuncs _funcs	=	
{
	.prepare	=	_prepare,
	.check		=	_check,
	.dispatch	=	_dispatch
};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  emit_detach
 *  Description:  
 * =====================================================================================
 */
static gboolean emit_detach( gpointer object)
{
	MHDevCarlife * _carlife	=	(MHDevCarlife *) object;

	g_message("\n\n%s--->mh_core_detach_dev---%s\n\n", __func__, MH_DEV( _carlife)->serial);
	if(_carlife != 	NULL &&  _carlife->detachFlag != true)
	{
		mh_core_detach_dev(MH_DEV( _carlife));
		_carlife->detachFlag	=	true;
		g_object_unref( _carlife);
	}
	return G_SOURCE_REMOVE;
}		/* -----  end of static function emit_detach  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _parse_events
 *  Description:  
 * =====================================================================================
 */
static gboolean _parse_events( gpointer user_data )
{
	g_message("carlife.c----->%s", __func__);
	struct udev_device * _udevDev;
	struct udev_list_entry * _properties,* _model,* _entry1;
	const gchar   * _serial,  *_devNode, *_value;
	_udevDev	=	udev_monitor_receive_device( udevMonitor );
//	if( _udevDev )
//	{
//			_properties	=	udev_device_get_properties_list_entry( _udevDev );
//			udev_list_entry_foreach( _entry1, _properties )
//			{
//				printf( "\t%s, %s\n", udev_list_entry_get_name( _entry1 ), udev_list_entry_get_value( _entry1 ));
//			}
//			printf("\n\n\n\n");
//
//	}

	if( _udevDev )
	{
		const gchar * _udevAction;

		_udevAction	=	udev_device_get_action( _udevDev );

		_properties	=	udev_device_get_properties_list_entry( _udevDev );

	
		if( g_strcmp0( _udevAction, "remove" ) == 0 )
		{
			_model	=	udev_list_entry_get_by_name( _properties, "SUBSYSTEM" );

			_value 	=	udev_list_entry_get_value( _model );

			if( g_strcmp0( _value, "usb" ) != 0 )
			{
				udev_device_unref( _udevDev );
				return G_SOURCE_CONTINUE;
			}
			_model	=	udev_list_entry_get_by_name( _properties, "DEVNAME" );

			_devNode	=	udev_list_entry_get_value( _model );

			if( _model == NULL || _devNode == 	NULL)
			{
				udev_device_unref( _udevDev );
				return G_SOURCE_CONTINUE;
			}
			_model	=	udev_list_entry_get_by_name( _properties, "ID_SERIAL_SHORT");

			if( _model != NULL)
			{
				_serial	=	udev_list_entry_get_value( _model);


				int res;
//				if( _carlife != NULL && !(g_strcmp0( MH_DEV_CARLIFE(_carlife)->md_os, "android")))
				if( _carlife != NULL && MH_DEV(_carlife)->serial != NULL)
				{
					g_message_dbg("(_carlife)->serial is : [%s][%d]", MH_DEV(_carlife)->serial, strlen(MH_DEV(_carlife)->serial));
					g_message_dbg("_serial is : [%s][%d]", _serial, strlen(_serial));
					res	=	g_strcmp0( MH_DEV(_carlife)->serial, _serial);
					if( MH_IS_DEV_CARLIFE( _carlife)  &&  g_strcmp0(MH_DEV(_carlife)->serial, _serial) == 0)
					{
						GSource * _source	=	g_idle_source_new();
						g_source_set_callback( _source, emit_detach, _carlife, NULL);
						g_source_attach( _source, _carlife->pipe_param->context);
						g_source_unref( _source);
						if(g_strcmp0(MH_DEV_CARLIFE(_carlife)->md_os, "android") == 0)
						{
							g_message_dbg("detach android device.....");
							aoa_carlife_status = AOA_CARLIFE_DISCONNECT;
							close_all_aoa_socket();
							dev_id_product = 0xFFFF;
							aoa_id_product = 0xFFFF;
							dev_id_vendor= 0xFFFF;
							aoa_id_vendor= 0xFFFF;
							aoa_in_ep = 0xFF;
							aoa_out_ep = 0xFF;
			                libusb_release_interface (usb_handle, 0);
			                libusb_close(usb_handle);
			                usb_handle = NULL;
						}
//		                libusb_exit(usb_ctx);
//		                usb_ctx = NULL;

//						pthread_t _id;
//						pthread_create( &_id, NULL, adb_wait_for_device, NULL);

					}
				}
				else
				{
					g_message("_carlife == 	NULL");
				}

			}
			else
			{
				g_message("ID_SERIAL_SHORT--->_model == NULL");	
			}
							
			
		}
		else if(g_strcmp0( _udevAction, "add" ) == 0)
		{
			g_message_dbg("add usb");
			if(connect_aoa_device() > 0)
			{
				   g_message_dbg("connect aoa device success!!!!!");
			}
		}
		else
		{
			g_warning( "Unknown udev action( %s ) received", _udevAction );
		}


		udev_device_unref( _udevDev );
	}

	return G_SOURCE_CONTINUE;
}		/* -----  end of static function _parse_events  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  udev_init
 *  Description:  
 * =====================================================================================
 */
static gboolean udev_init()
{
	g_message("carlife.c---->%s", __func__);
	udevCtx	=	udev_new();
	if( udevCtx == NULL)
	{
		g_warning( "udev_new failed" );
		return TRUE;
	}
	GSource * _source	=	NULL;

	udevMonitor	=	udev_monitor_new_from_netlink( udevCtx, "udev" );

	if( udevMonitor == NULL )
	{
		g_warning( "udev_monitor_new_from_netlink failed" );

		goto NEED_FREE_CTX;
	}

	/* NOTE: in first stage, we only support the block storage in mh_storage */
//	if( udev_monitor_filter_add_match_subsystem_devtype( udevMonitor, "block", 0 ) < 0 )
//	{
//		g_warning( "udev_monitor_filter_add_match_subsystem_devtype failed" );
//
//		goto NEED_FREE_MONITOR;
//	}
//	g_message("33333333333333333333333333333333");
//	if( udev_monitor_filter_add_match_subsystem_devtype( udevMonitor, "usb", 0 ) < 0 )
//	{
//		g_warning( "udev_monitor_filter_add_match_subsystem_devtype failed" );
//
//		goto NEED_FREE_MONITOR;
//	}
	if ( udev_monitor_enable_receiving( udevMonitor ) < 0 )
	{
		g_warning( "udev_monitor_enable_receiving failed" );

	}


	udevFd.fd	=	udev_monitor_get_fd( udevMonitor );

	if( udevFd.fd > 0 )
	{
		udevFd.events	=	POLLIN;

		_source	=	g_source_new( &_funcs, sizeof( GSource ));

		g_source_add_poll( _source, &udevFd );

		g_source_set_callback( _source, _parse_events, mh_core_instance(), NULL );

		mh_io_dispatch( MH_IO( mh_core_instance() ), _source );

		g_source_unref( _source );
	}
	else
	{
		g_warning( "udev_monitor_get_fd faild" );

		goto NEED_FREE_MONITOR;
	}

	mh_core_register_devices_listener( &_carlifeListener);

	g_signal_emit_by_name(mh_core_instance(), "core_events", MH_CORE_PLUGIN_LOAD_SUCCESS, "carlife");
	return G_SOURCE_REMOVE;


NEED_FREE_MONITOR:
	udev_monitor_unref( udevMonitor );
NEED_FREE_CTX:
	udev_unref( udevCtx );


	return 0;
}		/* -----  end of static function udev_init  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_instance
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_instance()
{

//  	pthread_t _id;
	pthread_mutex_init(&aoa_write_mutex, NULL);
	libusb_init(&usb_ctx);
//	libusb_set_option(usb_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
	libusb_set_debug(usb_ctx, LIBUSB_LOG_LEVEL_INFO);

	C_carLifeLibInit();
//	C_carLifeLibInit();
//	C_cmdRegisterProtocolVersionMatchStatus(cmdProtocolVersionMatchStatus);
//	C_cmdRegisterMDInfro(cmdMDInfro);
//	C_cmdRegisterMDBTPairInfro(cmdMDBTPairInfro);
//	C_cmdRegisterVideoEncoderInitDone(cmdVideoEncoderInitDone);
//	C_cmdRegisterVideoEncoderFrameRateChangeDone(cmdVideoEncoderFrameRateChangeDone);
//	C_cmdRegisterTelStateChangeIncoming(cmdTelStateChangeIncoming);
//	C_cmdRegisterTelStateChangeOutGoing(cmdTelStateChangeOutGoing);
//	C_cmdRegisterTelStateChangeIdle(cmdTelStateChangeIdle);
//	C_cmdRegisterTelStateChangeInCalling(cmdTelStateChangeInCalling);
//	C_cmdRegisterScreenOn(cmdScreenOn);
//	C_cmdRegisterScreenOff(cmdScreenOff);
//	C_cmdRegisterScreenUserPresent(cmdScreenUserPresent);
//	C_cmdRegisterForeground(cmdForeground);
//	C_cmdRegisterBackground(cmdBackground);
//	C_cmdRegisterGoToDeskTop(cmdGoToDeskTop);
//	C_cmdRegisterMicRecordWakeupStart(cmdMicRecordWakeupStart);
//	//0x00010023
//	C_cmdRegisterMicRecordEnd(cmdMicRecordEnd);
//	//0x00010024
//	C_cmdRegisterMicRecordRecogStart(cmdMicRecordRecogStart);
//	//0x00010026
//	C_cmdRegisterModuleStatus(cmdModuleStatus);
//	//0x00010030
//	C_cmdRegisterNaviNextTurnInfo(cmdNaviNextTurnInfo);
//	//0x00010031
//	C_cmdRegisterCarDataSubscribe(cmdCarDataSubscribe);	
//	//0x00010033
//	C_cmdRegisterCarDataSubscribeStart(cmdCarDataSubscribeStart);
//	//0x00010034
//	C_cmdRegisterCarDataSubscribeStop(cmdCarDataSubscribeStop);
//	//0x00010035
//	C_cmdRegisterMediaInfo(cmdMediaInfo);
//	//0x00010036
//	C_cmdRegisterMediaProgressBar(cmdMediaProgressBar);
//	//0x00010037
//	C_cmdRegisterConnectException(cmdRegisterConnectException);
//	//0x00010038
//	C_cmdRegisterRequestGoToForeground(cmdRegisterRequestGoToForeground);
//	//0x00010039
//	C_cmdRegisterUIActionSound(cmdUIActionSound);
//
//	//0x00010049 
//	C_cmdRegisterMdAuthenResponse(cmdRegisterMdAuthenResponse);
//
//	C_cmdRegisterMdAuthenResult( cmdMdAuthenResult);
//	//0x00010059	
//	C_cmdRegisterMdExit( MdExit_cb );
//
//	C_videoRegisterDataReceive(videoDataReceive);
////	C_videoRegisterHeartBeat(videoHeartBeat);
//
//	C_mediaRegisterInit(mediaInit);
//	C_mediaRegisterNormalData(mediaNormalData);
////	C_mediaRegisterStop(mediaStop);//carlife drop 
//	C_mediaRegisterPause(mediaPause);
//	C_mediaRegisterResume(mediaResume);
////	C_mediaRegisterSeek(mediaSeek);  //carlife drop
//
//	C_ttsRegisterInit(naviInit);
//	C_ttsRegisterNormalData(naviNormalData);
//	C_ttsRegisterStop(naviStop);
//
//	C_vrRegisterInit(vrInit);
//	C_vrRegisterNormalData(vrNormalData);
//	C_vrRegisterStop(vrStop);
//	/////bt////
//	C_cmdRegisterStartBtAutoPairRequest( btAutoPairRequest);	
// 	C_cmdRegisterFeatureConfigRequest( featureConfigRequest);
//	C_cmdRegisterBTIdentifyResultInd( btIdentifyResultInd);
//	C_cmdRegisterBtHfpRequest( BtHfpRequest_cb);

//byzz	pthread_create( &_id, NULL, adb_wait_for_device, NULL);
//	char input[256];
//	while(1)
//	{
//		if(scanf("%s", input)!=0)
//		{
////			C_cmdHUProtoclVersion( &huProtocolVersion );
//			C_cmdVideoEncoderInit(&initVideoParam);
//		}
//	}
	udev_init();
	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_plugin_find_dev
 *  Description:  
 * =====================================================================================
 */
gboolean mh_plugin_find_dev()
{	
	return TRUE;
}		/* -----  end of function mh_plugin_instance  ----- */
