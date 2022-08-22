/*
 * =====================================================================================
 *
 *       Filename:  transport.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/07/2015 06:55:36 PM
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
#include <glib.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/functionfs.h>
#include <linux/usb/ch9.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "transport.h"
#include "dev_iap2.h"
#include <mh_core.h>
#include <mh_misc.h>
#include <debug.h>
#include <linux/hidraw.h>
#include <errno.h>
#include<sys/socket.h>
#include<sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<libkmod.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define EAP_BUFFER_SOCKET_PORT (8898)

#define DEBUG_SWITCH 1
#if DEBUG_SWITCH
#define g_message_dbg(format,...) g_message("transport.c:%d----->%s------->"format, __LINE__, __func__, ##__VA_ARGS__)
#else
#define g_message_dbg(format,...)
#endif

#define DEBUG_SWITCH_HEX 1
#if DEBUG_SWITCH_HEX
#define EAP_DEBUG_HEX_DISPLAY(D, L)
#else
#define EAP_DEBUG_HEX_DISPLAY(D, L)
#endif

int eap_buffer_socket_server = -1;
int eap_buffer_socket_client = -1;

static int ep0Fd, ep1Fd, ep2Fd , ep3Fd, ep4Fd;
pthread_t  ep0threadId = 0;
pthread_t  ep2threadId = 0;

static int ea_ep0Fd, ea_ep1Fd, ea_ep2Fd;
static bool ea_enable	=	FALSE;

static gchar * devSerial = NULL;
static gchar * devPath = NULL;
static uint32_t intel_otg_flag = 0;
gboolean eaNativeFlag	=	TRUE;
extern MHDev * carplay;
MHDevIap2 * host_iap2;
static uint32_t nagivi_otg_flag = 0; 
static int device_mode = MISC_IAP_CARPLAY;
//static int device_mode = MISC_IAP_CARPLAY_CARLIFE;
bool ep0State , ep2State;
bool eaEp2State;
GSource * _timeoutSource = NULL;
//static int frist_flag = 0;

#define IAP_INTERFACE	"iAP Interface"
#define EA_INTERFACE	"com.baidu.CarLifeVehicleProtocol"

#define MAKE_LCB(f, l) \
	(((l ? 0 : 1) << 1) | (f ? 0 : 1))
struct _report_table 
{
	unsigned char reportId;
	int reportSize;
	int reportCount;
	int direction;                              /* 0: input; 1: output */
} * reportTable;

static int tableCount;

static const struct {
	struct usb_functionfs_strings_head header;
	struct {
		__le16 code;
		const char str1[sizeof IAP_INTERFACE];
//		const char str2[sizeof EA_INTERFACE];
	} __attribute__((packed)) lang0;
} __attribute__((packed)) iAP2Strings = {
	.header = {
		.magic =	 FUNCTIONFS_STRINGS_MAGIC,
		.length =	sizeof iAP2Strings,
		.str_count =   1 ,
		.lang_count =  1 ,
	},
	.lang0 = {
		0x0409, /* en-us */
		IAP_INTERFACE,
//		EA_INTERFACE
	},
};
struct iAP2Interface
{
		struct usb_interface_descriptor intf;
		struct usb_endpoint_descriptor_no_audio sink;
		struct usb_endpoint_descriptor_no_audio source;
} __attribute__((packed));

struct eaInterface
{
		struct usb_interface_descriptor intf0;
		struct usb_interface_descriptor intf1;
		struct usb_endpoint_descriptor_no_audio sink;
		struct usb_endpoint_descriptor_no_audio source;
} __attribute__((packed));

static const struct
{
	struct usb_functionfs_descs_head_v2 header;
	__le32 fs_count;
	__le32 hs_count;
	/* iAP2 Host Mode Full Speed Communication Channel */
	struct iAP2Interface iAP2Fs;
	/* EA Host Mode Full Speed Communication Channel */
//	struct eaInterface eaFs;

	/* iAP2 Host Mode High Speed Communication Channel */
	struct iAP2Interface iAP2Hs;
	/* EA Host Mode High Speed Communication Channel */
//	struct eaInterface eaHs;
} __attribute__((packed)) iAP2Desc	=	
{
	.header	=
	{
		.magic	=	FUNCTIONFS_DESCRIPTORS_MAGIC_V2,
		.length	=	sizeof( iAP2Desc ),
		.flags =  1 | 2,
	},

	.fs_count	=	3,
	.iAP2Fs	=	{
		.intf	=	{
			.bLength			=	sizeof iAP2Desc.iAP2Fs.intf,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	0,
			.bAlternateSetting	=	0,
			.bNumEndpoints		=	2,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,       /* MFi Accessory */
			.bInterfaceProtocol	=	0x00,
			.iInterface			=	1
		},

		.sink	=	{
			.bLength			=	sizeof iAP2Desc.iAP2Fs.sink,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	1 | USB_DIR_IN,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
		},

		.source	=	{
			.bLength			=	sizeof iAP2Desc.iAP2Fs.source,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	2 | USB_DIR_OUT,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
		}
	},

	.hs_count	=	3,
	.iAP2Hs	=	{
		.intf	=	{
			.bLength			=	sizeof iAP2Desc.iAP2Hs.intf,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	0,
			.bAlternateSetting	=	0,
			.bNumEndpoints		=	2,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,               /* MFi Accessory */
			.bInterfaceProtocol	=	0x00,
			.iInterface			=	1,
		},

		.sink = {
			.bLength			=	sizeof iAP2Desc.iAP2Hs.sink,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	1 | USB_DIR_IN,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		=	512
		},

		.source = {
			.bLength			=	sizeof iAP2Desc.iAP2Hs.source,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	2 | USB_DIR_OUT,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		=	512,
			.bInterval			=	1, /* NAK every 1 uframe */
		},
	},
};
static const struct {
	struct usb_functionfs_strings_head header;
	struct {
		__le16 code;
		const char str1[sizeof EA_INTERFACE];
	} __attribute__((packed)) lang0;
} __attribute__((packed)) eaStrings = {
	.header = {
		.magic =	 FUNCTIONFS_STRINGS_MAGIC,
		.length =	sizeof eaStrings,
		.str_count =   1 ,
		.lang_count =  1 ,
	},
	.lang0 = {
		0x0409, /* en-us */
		EA_INTERFACE
	},
};

static const struct
{
	struct usb_functionfs_descs_head_v2 header;
	__le32 fs_count;
	__le32 hs_count;
		/* EA Host Mode Full Speed Communication Channel */
	struct eaInterface eaFs;

	/* EA Host Mode High Speed Communication Channel */
	struct eaInterface eaHs;
} __attribute__((packed)) eaDesc	=	
{
	.header	=
	{
		.magic	=	FUNCTIONFS_DESCRIPTORS_MAGIC_V2,
		.length	=	sizeof( eaDesc ),
		.flags =  1 | 2,
	},

	.fs_count	=	4,
	.eaFs	=	{
		.intf0	=	{
			.bLength			=	sizeof eaDesc.eaFs.intf0,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	2,
			.bAlternateSetting	=	0,
			.bNumEndpoints		=	0,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,       /* MFi Accessory */
			.bInterfaceProtocol	=	0x01,
			.iInterface			=	1
		},
		.intf1	=	{
			.bLength			=	sizeof eaDesc.eaFs.intf1,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	2,
			.bAlternateSetting	=	1,
			.bNumEndpoints		=	2,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,
			.bInterfaceProtocol	=	0x01,
			.iInterface			=	1
		},

		.sink	=	{
			.bLength			=	sizeof eaDesc.eaFs.sink,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	3 | USB_DIR_IN,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
		},

		.source	=	{
			.bLength			=	sizeof eaDesc.eaFs.source,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	4 | USB_DIR_OUT,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
		}
	},

	.hs_count	=	4,

	.eaHs	=	{
		.intf0	=	{
			.bLength			=	sizeof eaDesc.eaHs.intf0,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	2,
			.bAlternateSetting	=	0,
			.bNumEndpoints		=	0,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,
			.bInterfaceProtocol	=	0x01,
			.iInterface			=   1	
		},

		.intf1	=	{
			.bLength			=	sizeof eaDesc.eaHs.intf1,
			.bDescriptorType	=	USB_DT_INTERFACE,
			.bInterfaceNumber	=	2,
			.bAlternateSetting	=	1,
			.bNumEndpoints		=	2,
			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
			.bInterfaceSubClass	=	0xF0,
			.bInterfaceProtocol	=	0x01,
			.iInterface			=	1
		},

		.sink	=	{
			.bLength			=	sizeof eaDesc.eaHs.sink,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	3 | USB_DIR_IN,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		=	512,
		},

		.source	=	{
			.bLength			=	sizeof eaDesc.eaHs.source,
			.bDescriptorType	=	USB_DT_ENDPOINT,
			.bEndpointAddress	=	4 | USB_DIR_OUT,
			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
			.wMaxPacketSize		=	512,
		}
	}

};

//#define MAX_EA_BUF_NUM (1048576) //1MB
//static guint8 ea_ep2Task_buf[MAX_EA_BUF_NUM / 10] = {0};

//static const struct
//{
//	struct usb_functionfs_descs_head header;
//
//	/* iAP2 Host Mode Full Speed Communication Channel */
//	struct iAP2Interface iAP2Fs;
//	/* EA Host Mode Full Speed Communication Channel */
////	struct eaInterface eaFs;
//
//	/* iAP2 Host Mode High Speed Communication Channel */
//	struct iAP2Interface iAP2Hs;
//	/* EA Host Mode High Speed Communication Channel */
////	struct eaInterface eaHs;
//} __attribute__((packed)) iAP2Desc	=	
//{
//	.header	=
//	{
//		.magic	=	FUNCTIONFS_DESCRIPTORS_MAGIC,
//		.length	=	sizeof( iAP2Desc ),
//		.fs_count	=	3,
//		.hs_count	=	3,
//	},
//
//	.iAP2Fs	=	{
//		.intf	=	{
//			.bLength			=	USB_DT_INTERFACE_SIZE,
//			.bDescriptorType	=	USB_DT_INTERFACE,
//			.bInterfaceNumber	=	0,
//			.bAlternateSetting	=	0,
//			.bNumEndpoints		=	2,
//			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
//			.bInterfaceSubClass	=	0xF0,       /* MFi Accessory */
//			.bInterfaceProtocol	=	0x00,
//			.iInterface			=	1
//		},
//
//		.sink	=	{
//			.bLength			=	USB_DT_ENDPOINT_SIZE,
//			.bDescriptorType	=	USB_DT_ENDPOINT,
//			.bEndpointAddress	=	1 | USB_DIR_IN,
//			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
//		},
//
//		.source	=	{
//			.bLength			=	USB_DT_ENDPOINT_SIZE,
//			.bDescriptorType	=	USB_DT_ENDPOINT,
//			.bEndpointAddress	=	2 | USB_DIR_OUT,
//			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
//		}
//	},
//
////	.eaFs	=	{
////		.intf0	=	{
////			.bLength	=	USB_DT_INTERFACE_SIZE,
////			.bDescriptorType	=	USB_DT_INTERFACE,
////			.bInterfaceNumber	=	3,
////			.bAlternateSetting	=	0,
////			.bNumEndpoints		=	0,
////			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
////			.bInterfaceSubClass	=	0xF0,
////			.bInterfaceProtocol	=	0x01,
////			.iInterface			=	2
////		},
////
////		.intf1	=	{
////			.bLength			=	USB_DT_INTERFACE_SIZE,
////			.bDescriptorType	=	USB_DT_INTERFACE,
////			.bInterfaceNumber	=	3,
////			.bAlternateSetting	=	1,
////			.bNumEndpoints		=	2,
////			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
////			.bInterfaceSubClass	=	0xF0,
////			.bInterfaceProtocol	=	0x01,
////			.iInterface			=	2
////		},
////
////		.sink	=	{
////			.bLength			=	USB_DT_ENDPOINT_SIZE,
////			.bDescriptorType	=	USB_DT_ENDPOINT,
////			.bEndpointAddress	=	3 | USB_DIR_IN,
////			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
////		},
////
////		.source	=	{
////			.bLength			=	USB_DT_ENDPOINT_SIZE,
////			.bDescriptorType	=	USB_DT_ENDPOINT,
////			.bEndpointAddress	=	4 | USB_DIR_OUT,
////			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
////		}
////	},
//
//	.iAP2Hs	=	{
//		.intf	=	{
//			.bLength			=	USB_DT_INTERFACE_SIZE,
//			.bDescriptorType	=	USB_DT_INTERFACE,
//			.bInterfaceNumber	=	0,
//			.bAlternateSetting	=	0,
//			.bNumEndpoints		=	2,
//			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
//			.bInterfaceSubClass	=	0xF0,               /* MFi Accessory */
//			.bInterfaceProtocol	=	0x00,
//			.iInterface			=	1,
//		},
//
//		.sink = {
//			.bLength			=	USB_DT_ENDPOINT_SIZE,
//			.bDescriptorType	=	USB_DT_ENDPOINT,
//			.bEndpointAddress	=	1 | USB_DIR_IN,
//			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
//			.wMaxPacketSize		=	512
//		},
//
//		.source = {
//			.bLength			=	USB_DT_ENDPOINT_SIZE,
//			.bDescriptorType	=	USB_DT_ENDPOINT,
//			.bEndpointAddress	=	2 | USB_DIR_OUT,
//			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
//			.wMaxPacketSize		=	512,
//			.bInterval			=	1, /* NAK every 1 uframe */
//		},
//	},
//
////	.eaHs	=	{
////		.intf0	=	{
////			.bLength	=	USB_DT_INTERFACE_SIZE,
////			.bDescriptorType	=	USB_DT_INTERFACE,
////			.bInterfaceNumber	=	3,
////			.bAlternateSetting	=	0,
////			.bNumEndpoints		=	0,
////			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
////			.bInterfaceSubClass	=	0xF0,
////			.bInterfaceProtocol	=	0x01,
////			.iInterface			=	2
////		},
////
////		.intf1	=	{
////			.bLength			=	USB_DT_INTERFACE_SIZE,
////			.bDescriptorType	=	USB_DT_INTERFACE,
////			.bInterfaceNumber	=	3,
////			.bAlternateSetting	=	1,
////			.bNumEndpoints		=	2,
////			.bInterfaceClass	=	USB_CLASS_VENDOR_SPEC,
////			.bInterfaceSubClass	=	0xF0,
////			.bInterfaceProtocol	=	0x01,
////			.iInterface			=	2
////		},
////
////		.sink	=	{
////			.bLength			=	USB_DT_ENDPOINT_SIZE,
////			.bDescriptorType	=	USB_DT_ENDPOINT,
////			.bEndpointAddress	=	3 | USB_DIR_IN,
////			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
////			.wMaxPacketSize		=	512,
////		},
////
////		.source	=	{
////			.bLength			=	USB_DT_ENDPOINT_SIZE,
////			.bDescriptorType	=	USB_DT_ENDPOINT,
////			.bEndpointAddress	=	4 | USB_DIR_OUT,
////			.bmAttributes		=	USB_ENDPOINT_XFER_BULK,
////			.wMaxPacketSize		=	512,
////		}
////	}
//};

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _serial_compare
 *  Description:  
 * =====================================================================================
 */
static gint _type_compare( gconstpointer a, gconstpointer b )
{
	return g_strcmp0( MH_DEV(a)->type, b );
}		/* -----  end of static function _serial_compare  ----- */
static void umount_dev( const char * path)
{
	int _retry	=	0;
	if( umount2(path, MNT_DETACH) != 0)
	{
		g_message("umount failed sleep 1s: trying again");
		sleep(1);
		if( umount2(path, MNT_DETACH) != 0)
		{
			g_message("umount failed again");
		}
	}
}

gpointer ea_buf_task(gpointer user_data)
{
	g_message_dbg("IN");
	struct sockaddr_in client_address;
	socklen_t address_len = sizeof(struct sockaddr_in);
	eap_buffer_socket_client = accept(eap_buffer_socket_server, (struct sockaddr *)&client_address, &address_len);
	if (eap_buffer_socket_client == -1) {
		g_message_dbg("eap_buffer_socket_server accept error[%s]", strerror(errno));
		return 0;
	}
	g_message_dbg("get ea_buf_task eap_buffer_socket_client[%d]", eap_buffer_socket_client);
	guint8* _buf = g_new0(guint8, 600 * 1024);
	guint8 _buf_head[8];
	memset(_buf_head, 0, sizeof(_buf_head));
	guint eap_msg_len = 0;
	int byte_num_head, byte_num_data, byte_seg = 0;
	GVariant * _var;
	while(eaEp2State)
	{
		byte_num_head = recv(eap_buffer_socket_client,_buf_head,8,0);
		if(byte_num_head < 0)
		{
			g_message_dbg("get eap_buffer_socket_client head data error[%s]", strerror(errno));
			break;
		}
		if(0 == byte_num_head)
		{
			g_message_dbg("get eap_buffer_socket_client head data len is 0");
			continue;
		}
		eap_msg_len = (_buf_head[4] << 24) | (_buf_head[5] << 16) | (_buf_head[6] << 8) | (_buf_head[7]);
//		g_message_dbg("get eap_msg_len is : %d", eap_msg_len);
//		g_message_dbg("get eap head is : %08X", eap_msg_len);
		memcpy(_buf, _buf_head, 8);
		if(eap_msg_len != 0 && eap_msg_len <= 600*1024-8)
		{
			byte_num_data = recv(eap_buffer_socket_client,_buf + 8, eap_msg_len,0);
			if(byte_num_data < 0)
			{
				g_message_dbg("get eap_buffer_socket_client msg error[%s]", strerror(errno));
				break;
			}
			if(0 == byte_num_data)
			{
				g_message_dbg("get eap_buffer_socket_client msg len is 0");
				continue;
			}
//			g_message_dbg("get eap msg data success!!!!![%d]", byte_num_data);
//			EAP_DEBUG_HEX_DISPLAY(_buf, 32)
			byte_seg = byte_num_data;
			while(byte_seg < eap_msg_len)
			{
				g_message_dbg("start get eap_buffer_socket_client byte_seg[%d][%d]", byte_seg,eap_msg_len);
				byte_seg += recv(eap_buffer_socket_client,_buf + 8 + byte_seg, eap_msg_len - byte_seg,0);
				if(byte_seg == eap_msg_len)
				{
//					g_message_dbg("end get eap_buffer_socket_client byte_seg[%d][%d]", byte_seg,eap_msg_len);
					break;
				}
				else if(byte_seg > eap_msg_len)
				{
					g_message_dbg("get eap_buffer_socket_client byte_seg error[%d][%d]", byte_seg,eap_msg_len);
					break;
				}
				else
				{
//					g_message_dbg("cotinue get eap_buffer_socket_client byte_seg[%d][%d]", byte_seg,eap_msg_len);
					continue;
				}
			}
			_var	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, _buf, eap_msg_len + 8, sizeof( guchar ));
			_var	=	g_variant_new_variant( _var );
			g_signal_emit_by_name(host_iap2 , "ea_native_data", _var );
		}

	}
	g_free(_buf);
	if(eap_buffer_socket_server != -1)
	{
		g_message_dbg("close eap_buffer_socket_server");
		close(eap_buffer_socket_server);
		shutdown(eap_buffer_socket_server, SHUT_RDWR);
		eap_buffer_socket_server = -1;
	}
	if(eap_buffer_socket_client != -1)
	{
		g_message_dbg("close eap_buffer_socket_client");
		close(eap_buffer_socket_client);
		shutdown(eap_buffer_socket_client, SHUT_RDWR);
		eap_buffer_socket_client = -1;
	}
	g_message_dbg("OUT");
	return 0;

}

gpointer ea_ep0Task( gpointer user_data )
{
	static const char * const _eventNames[]	=	{
		"BIND",
		"UNBIND",
		"ENABLE",
		"DISABLE",
		"SETUP",
		"SUSPEND",
		"RESUME"
	};
	ssize_t _size;
	struct usb_functionfs_event _event;
	gboolean _enabled	=	FALSE;
	char * _path	=	NULL;
	
	while( ep0State )
	{
		_size	=	read( ea_ep0Fd, &_event, sizeof( _event ) );

		if( _size > 0 )
		{
			g_message( "ea Transport Event: %s", _eventNames[ _event.type ]);

			switch( _event.type )
			{
			case FUNCTIONFS_BIND:
				break;
			case FUNCTIONFS_UNBIND:
				{
					ea_enable	=	FALSE;
				

					close( ea_ep0Fd);
					shutdown( ea_ep1Fd, SHUT_WR);
					close( ea_ep1Fd);
					shutdown(ea_ep2Fd, SHUT_RD);
					close( ea_ep2Fd);
					eaNativeFlag = TRUE;
					eaEp2State = FALSE;
					_path	=	g_strdup("/dev/ea");
					umount_dev( (const char *)_path);
					g_free( _path);
				}


				break;
			case FUNCTIONFS_ENABLE:
				{
					ea_enable	=	TRUE;
					if( eaNativeFlag == TRUE)
					{
						eaNativeFlag	=	FALSE;
//						if( host_iap2 != NULL)
//						{
//							g_signal_emit_by_name( host_iap2, "ea_native_stop");
//						}
					}
					else
					{
						eaNativeFlag	=	TRUE;
						if( host_iap2 != NULL)
						{
							g_signal_emit_by_name( host_iap2, "ea_native_start");
						}
					}
				}

				break;
			case FUNCTIONFS_DISABLE:
				ea_enable	=	FALSE;
				close( ea_ep0Fd);
				shutdown( ea_ep1Fd, SHUT_WR);
				close( ea_ep1Fd);
				shutdown(ea_ep2Fd, SHUT_RD);
				close( ea_ep2Fd);
				eaNativeFlag = TRUE;
				eaEp2State = FALSE;

				_path	=	g_strdup("/dev/ea");
				umount_dev( (const char *)_path);
				g_free( _path);

				break;
			case FUNCTIONFS_SETUP:
				break;
			case FUNCTIONFS_SUSPEND:
				break;
			case FUNCTIONFS_RESUME:
				break;
			default:
				break;
			}
		}
		else if( ea_enable == FALSE)
		{
			break;
		}
		else
		{
			if( errno != EINTR ) perror( "ea_ep0Task" );
		}
	}
	g_message("_____________ea----->ep0Task exit_____________________\n\n");
	return 0;
}		/* -----  end of static function ep0Task  ----- */
gpointer ea_ep2Task( gpointer user_data )
{
	g_message_dbg("IN");
//	MHDevIap2 * _iap2	=	MH_DEV_IAP2( user_data );
	ssize_t _size;
	int send_result;
	BOOL _detect;
	guint8 _buf[32 * 1024];
    int client_socket = socket ( AF_INET, SOCK_STREAM, 0 );
    if(-1 == client_socket)
    {
    	g_message_dbg("create client_socket error[%s]", strerror(errno));
    	return 0;
    }
    int on = 1;
    if ( setsockopt ( client_socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    {
    	g_message_dbg("setsockopt client_socket error[%s]", strerror(errno));
    	return 0;
    }
    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(EAP_BUFFER_SOCKET_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int status = connect ( client_socket, ( struct sockaddr * ) &server_addr, sizeof ( server_addr ) );

    if ( -1 == status){
    	g_message_dbg("connect client_socket error[%s]", strerror(errno));
    	return 0;
    }
	GVariant * _var;
	while( eaEp2State )
	{
		_size	=	read( ea_ep2Fd, _buf, sizeof(_buf));
		if(_size > 0)
		{
//			g_message_dbg("read eap raw data success!!!!![%d]", _size);
//			g_message_dbg("read eap raw data is : %02X%02X%02X%02X", _buf[0], _buf[1], _buf[2], _buf[3]);
			send_result = send(client_socket, _buf, _size, MSG_NOSIGNAL);
			if(-1 == send_result)
			{
				g_message_dbg("send eap raw data to buf socket error[%d]", strerror(errno));
			}
		}
		else if(0 == _size)
		{
			g_message_dbg("read eap raw data len is 0");
			continue;
		}
		else
		{
			g_message_dbg("read eap raw data error[%s][%d]", strerror(errno), ea_enable);
			if( ea_enable == FALSE)
			{
				break;
			}
			continue;
		}
	}
	if(client_socket != -1)
	{
		g_message_dbg("close client_socket");
		close(client_socket);
		shutdown(client_socket, SHUT_RDWR);
		client_socket = -1;
	}
	g_message_dbg("OUT");

	return NULL;
}		/* -----  end of function ep2Task  ----- */

gint ea_native_write( MHDev * dev, const uint8_t * buf, gint len)
{
	g_message("%s\n", __func__);
	int _size	=	write( ea_ep1Fd, buf, len );

	if( _size < 0 )
		perror( __func__ );

	return _size;
}		/* -----  end of function host_write_iap2  ----- */

gboolean echo_write( char * path, char * context)
{
	int _fd	=	0;
	gboolean _res	=	TRUE;
	g_message("%s-->%s--->%s", __func__, path, context);	
	_fd	=	open(path, O_RDWR);
	if( _fd	==	-1)
	{
		g_message("path:%s", path);
		perror("open path error");
		return FALSE;
	}
	if(-1 == write( _fd, context, strlen( context)))
	{
		perror("write error");

		_res	=	FALSE;

	}
	close( _fd);
	return FALSE;


}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ep0Task
 *  Description:  
 * =====================================================================================
 */
gpointer ep0Task( gpointer user_data )
{
	static const char * const _eventNames[]	=	{
		"BIND",
		"UNBIND",
		"ENABLE",
		"DISABLE",
		"SETUP",
		"SUSPEND",
		"RESUME"
	};
	ssize_t _size;
	struct usb_functionfs_event _event;
	gboolean _enabled	=	FALSE;
	MHDevIap2 * _ret;
	char * _path;
	char * _context;
	
//	while( TRUE )
	while( ep0State )
	{
		_size	=	read( ep0Fd, &_event, sizeof( _event ) );

		if( _size > 0 )
		{
			g_message( "Transport Event: %s", _eventNames[ _event.type ]);

			switch( _event.type )
			{
			case FUNCTIONFS_BIND:
				if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY || mh_misc_get_local_iap_device_mode()==MISC_IAP_CARPLAY_CARLIFE )
				{
					system( "ifconfig usbncm0 up" );
					system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/accept_dad" );
					system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/router_solicitation_delay");
					system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/router_solicitation_interval");
				}
				break;
			case FUNCTIONFS_UNBIND:
				if( _enabled )
				{
					_enabled	=	FALSE;
					ep0State = FALSE;
					if( _ret != NULL )
					{
						MHDev * _carplay;
						g_message("%s--->mh_core_detach_dev:%p", __func__, _ret);
						mh_core_detach_dev( MH_DEV( _ret ));
						g_object_unref( _ret );

						/* See if we have carpley device registered and release it */
						_carplay	=	MH_DEV( mh_core_find_dev_custom( "carplay", _type_compare ));

						if( _carplay != NULL )
						{
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_USB_CARPLAY;
							_devParam->mac_addr			=	g_strdup( devSerial );
							_devParam->connect_status	=	2;	

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );

							//							mh_core_detach_dev( _carplay );

							//							g_object_unref( _carplay );
						}else{
							g_message("Don't find usb carplay device00000000\n");
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_USB_CARPLAY;
							_devParam->mac_addr			=	g_strdup( devSerial );
							_devParam->connect_status	=	2;

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );
							g_message("Don't find usb carplay device11111111\n");
						}

						_ret	=	NULL;
					}
					if(close( ep0Fd ) != 0)
					{
						g_message("ep0Task UNBIND_____________close ep0Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(shutdown(ep1Fd, SHUT_WR) != 0)
					{
						g_message("ep0Task UNBIND_____________shutdown ep1Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(close( ep1Fd ) != 0)
					{
						g_message("ep0Task UNBIND_____________close ep1Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(shutdown(ep2Fd, SHUT_WR) != 0)
					{
						g_message("ep0Task UNBIND_____________shutdown ep2Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(close( ep2Fd ) != 0)
					{
						g_message("ep0Task UNBIND_____________close ep2Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					g_message("ep0Task UNBIND_____________umount iap2_____________________\n\n");

					_path	=	g_strdup("/dev/iap2");
					umount_dev((const char *)_path);
					g_free( _path);

					mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );

				}

				break;
			case FUNCTIONFS_ENABLE:
				if( !_enabled )
				{
					if( _timeoutSource != NULL )
					{
						g_message("____________get enable delete _timeoutSource _____________________\n\n");
						g_source_destroy( _timeoutSource );

						g_source_unref( _timeoutSource );

						_timeoutSource = NULL;
					}
					/* if role switch success, it must be iAP2 device */

					_ret	=	g_object_new( MH_TYPE_DEV_IAP2, "io-name", "iap2", NULL );
					host_iap2	=	_ret;
					/* use host mode apis */
					_ret->read	=	host_read_iap2;
					_ret->write	=	host_write_iap2;
					_ret->hostMode	=	TRUE;
					_ret->eaNative_write	=	ea_native_write;	

					MH_DEV( _ret )->serial	=	g_strdup(devSerial);
					if (devPath != NULL)
						MH_DEV( _ret )->devPath	=	g_strdup(devPath);

//					devSerial	=	NULL;
					mh_core_attach_dev( MH_DEV( _ret ));

					iAP2LinkRunLoopAttached( _ret->plinkRunLoop );

					_enabled	=	TRUE;
				}

				break;
			case FUNCTIONFS_DISABLE:
				if( _enabled )
				{
					_enabled	=	FALSE;
					ep0State = FALSE;
					if( _ret != NULL )
					{
						MHDev * _carplay;
						g_message("%s--->mh_core_detach_dev:%p", __func__, _ret);
						mh_core_detach_dev( MH_DEV( _ret ));
						g_object_unref( _ret );

						/* See if we have carpley device registered and release it */
						_carplay	=	MH_DEV( mh_core_find_dev_custom( "carplay", _type_compare ));

						if( _carplay != NULL )
						{
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_USB_CARPLAY;
							_devParam->mac_addr			=	g_strdup( devSerial );
							_devParam->connect_status	=	2;	

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );

//							mh_core_detach_dev( _carplay );

//							g_object_unref( _carplay );
						}else{
							g_message("DISABLE Don't find usb carplay device00000000\n");
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_USB_CARPLAY;
							_devParam->mac_addr			=	g_strdup( devSerial );
							_devParam->connect_status	=	2;

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );
							g_message("DISABLE Don't find usb carplay device11111111\n");
						}

						_ret	=	NULL;
					}
					if(close( ep0Fd ) != 0)
					{
						g_message("ep0Task DISABLE_____________close ep0Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(shutdown(ep1Fd, SHUT_WR) != 0)
					{
						g_message("ep0Task DISABLE_____________shutdown ep1Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(close( ep1Fd ) != 0)
					{
						g_message("ep0Task DISABLE_____________close ep1Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(shutdown(ep2Fd, SHUT_WR) != 0)
					{
						g_message("ep0Task DISABLE_____________shutdown ep2Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					if(close( ep2Fd ) != 0)
					{
						g_message("ep0Task DISABLE_____________close ep2Fd failed[%s]_____________________\n\n", strerror(errno));
					}
					g_message("ep0Task DISABLE_____________umount iap2_____________________\n\n");
					_path	=	g_strdup("/dev/iap2");
					umount_dev((const char *)_path);
					g_free( _path);

					mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );

				}

				break;
			case FUNCTIONFS_SETUP:
				break;
			case FUNCTIONFS_SUSPEND:
				if( intel_otg_flag == 1)
				{
					echo_write( "/sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state", "host");
					intel_otg_flag	=	0;
				}
				if (nagivi_otg_flag == 1)
				{
					g_message("%s otg host",__func__);
//					system( "echo host > /sys/devices/dwc_otg/drdmode");
					system( "echo on > /sys/devices/dwc_otg/vbus");
					system( "echo host > /sys/devices/dwc_otg/drdmode");
					nagivi_otg_flag = 0;
				}
				break;
			case FUNCTIONFS_RESUME:
				break;
			default:
				break;
			}
		}
		else
		{
			if( errno != EINTR ) perror( "ep0Task" );
		}
	}
//	close( ep0Fd );
	g_message("_____________ep0Task exit_____________________\n\n");
	if( _timeoutSource != NULL )
	{
		g_message("_____________ep0Task exit delete _timeoutSource _____________________\n\n");
		g_source_destroy( _timeoutSource );

		g_source_unref( _timeoutSource );

		_timeoutSource = NULL;
	}

//	if ((FALSE == ep2State)&&(FALSE == ep0State))
//	{
//		close( ep0Fd );
//		close( ep1Fd );
//		close( ep2Fd );
//		g_message("ep0_____________umount iap2_____________________\n\n");
//		_path	=	g_strdup("/dev/iap2");
//		umount_dev((const char *)_path);
//		g_free( _path);
//
//		mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );
//	}
	return 0;
}		/* -----  end of static function ep0Task  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ep2Task
 *  Description:  
 * =====================================================================================
 */
gpointer ep2Task( gpointer user_data )
{
//	MHDevIap2 * _iap2	=	MH_DEV_IAP2( user_data );
	ssize_t _size;
	BOOL _detect;
	guint8 _buf[ 8 * 1024 ];
	static iAP2Packet_t * iPacket	=	NULL;
	uint32_t _remlen = 0;
	uint32_t _cumlen = 0;
	
//	while( TRUE )
	while( ep2State )
	{
		_size	=	read( ep2Fd, _buf, sizeof( _buf ) );

//		g_message("\n\n\n\n%s--->read:%d\n\n\n\n", __func__, _size);
		if( _size > 0 )
		{
			_cumlen = 0;
			if( iPacket == NULL )
				iPacket	=	iAP2PacketCreateEmptyRecvPacket( host_iap2->plinkRunLoop->link );

			_remlen = iAP2PacketParseBuffer( _buf, _size, iPacket, 0, &_detect, NULL, NULL );
			if( _detect )
				g_message( "iAP 1.0/2.0 has been detected" );
			if( iAP2PacketIsComplete( iPacket ))
			{
				iAP2LinkRunLoopHandleReadyPacket( host_iap2->plinkRunLoop, iPacket );

				iPacket	=	NULL;
			}
			if ((_remlen < _size)&&(_remlen != 0))
			{
				iPacket	=	iAP2PacketCreateEmptyRecvPacket( host_iap2->plinkRunLoop->link );
				_cumlen = iAP2PacketParseBuffer( _buf+_remlen, _size-_remlen, iPacket, 0, &_detect, NULL, NULL );
				g_message("%s ==== iAP2PacketParseBuffer return ====>_cumlen = %d, _size-_remlen = %d",__func__,_cumlen,_size-_remlen);
				if( _detect )
					g_message( "iAP 1.0/2.0 has been detected" );
				if( iAP2PacketIsComplete( iPacket ))
				{
					iAP2LinkRunLoopHandleReadyPacket( host_iap2->plinkRunLoop, iPacket );

					iPacket	=	NULL;
				}
			}
//			while( TRUE )
//			{
//				_remlen = iAP2PacketParseBuffer( _buf+_cumlen, _size-_cumlen, iPacket, 0, &_detect, NULL, NULL );
//
//				if( _detect )
//					g_message( "iAP 1.0/2.0 has been detected" );
////				g_message("%s ===================>iPacket->state = %d",__func__,iPacket->state);
//				g_message("%s ===================>iAP2PacketIsComplete( iPacket ) = %d",__func__,iAP2PacketIsComplete( iPacket ));
//				if( iAP2PacketIsComplete( iPacket ))
//				{
//					iAP2LinkRunLoopHandleReadyPacket( linkRunLoop, iPacket );
//
//					iPacket	=	NULL;
//				}
//				if ( ( _remlen < ( _size-_cumlen ) ) && ( _remlen != 0 ) )
//				{
//					_cumlen += _remlen;
//					iPacket	=	iAP2PacketCreateEmptyRecvPacket( linkRunLoop->link );
//				}
//				else
//				{
//					break;
//				}
//			}
		}
		else
		{
			if( iPacket != NULL )
				iAP2PacketDelete( iPacket );

			iPacket	=	NULL;


			if (108 == errno || errno == 9)
			{
				g_message("%s[%d]errno = %d",__func__,__LINE__,errno);
				g_message("%s ep2  not ok",__func__);
				ep2State = FALSE;
				break;
			}
			if( errno != EINTR )
			{
				perror( "ep2Task" );
				g_message("%s ep2 is [%d],  _size is [%d], _buf is %p, _buf size is %d",__func__, ep2Fd, _size, _buf, sizeof(_buf));

//				break;
			}
		}
	}
	g_message("_____________ep2Task exit_____________________\n\n");
//	close( ep1Fd );
//	close( ep2Fd );
//	if ((FALSE == ep2State)&&(FALSE == ep0State))
//	{
//		close( ep0Fd );
//		close( ep1Fd );
//		close( ep2Fd );
//		g_message("ep2_____________umount iap2_____________________\n\n");
//		system( "umount /dev/iap2");
//		mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );
//	}
	return 0;
}		/* -----  end of function ep2Task  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ep4Task
 *  Description:  
 * =====================================================================================
 */
gpointer ep4Task( gpointer user_data )
{
	return 0;
}		/* -----  end of function ep4Task  ----- */
int ffs_ea()
{
	g_message("\n\n\n%s\n\n\n", __func__);
	int _fd;
	if(mkdir("/dev/ea", S_IRUSR|S_IXUSR) < 0)
	{
//		perror("mkdir /dev/ea error");
		g_message("transport.c : ffs_ea-----mkdir /dev/ea error!!!!![%s]", strerror(errno));
	}
	if(mount("ea", "/dev/ea", "functionfs", MS_MGC_VAL, NULL) < 0)
	{
		g_message("transport.c : ffs_ea-----mount /dev/ea error!!!!![%s]", strerror(errno));
//		perror("mount ea /dev/ea error");
	}

	_fd	=	open( "/dev/ea/ep0", O_RDWR, 0 );
	int desc_ret, str_ret, index;
	if( _fd > 0 )
	{
		if( (desc_ret = write( _fd, &eaDesc, sizeof( eaDesc ))) == sizeof( eaDesc ))
		{
			g_message( "ea Write descs success[%d]", desc_ret);
			printf("--------------start output eaDesc--------------\n");
			for(index = 0; index < sizeof( eaDesc ); index++)
			{
				printf("%02X", *(((unsigned char*)(&eaDesc)) + index));
			}
			printf("\n--------------end output eaDesc--------------\n");
		}
		else
		{
			close( _fd );
			perror( " ea Write descs to ep3" );

			return -1;
		}

		if( (str_ret = write( _fd, &eaStrings, sizeof( eaStrings ))) == sizeof( eaStrings ))
		{
			g_message( "ea Write strings success[%d]", str_ret);
			printf("--------------start output strings--------------\n");
			for(index = 0; index < sizeof( eaStrings ); index++)
			{
				printf("%02X", *(((unsigned char*)(&eaStrings)) + index));
			}
			printf("\n--------------end output strings--------------\n");
		}
		else
		{
			close( _fd );
			perror( "ea Write strings to ep4" );

			return -1;
		}

		ea_ep0Fd	=	_fd;
		ea_enable	=	TRUE;
		ea_ep1Fd	=	open( "/dev/ea/ep1", O_RDWR | O_NONBLOCK );
		ea_ep2Fd	=	open( "/dev/ea/ep2", O_RDWR | O_NONBLOCK);
		eaEp2State	=	TRUE;
		g_message( "ffs_ea ea [%d][%d][%d]", ea_ep0Fd, ea_ep1Fd, ea_ep2Fd);

		//add for eap implement start
		g_message_dbg("CREATE_EAP_BUFFER_SOCKET_SERVER start");
		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(struct sockaddr_in));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(EAP_BUFFER_SOCKET_PORT);
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		eap_buffer_socket_server = socket(AF_INET, SOCK_STREAM, 0);
		if (eap_buffer_socket_server == -1) {
			g_message_dbg("eap_buffer_socket_server socket error[%s]", strerror(errno));
			return -1;
		}
		int on = 1;
		if ( setsockopt ( eap_buffer_socket_server, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
		{
			g_message_dbg("set sockoprt error[%s]", strerror(errno));
			return -1;
		}
		int bind_result = bind(eap_buffer_socket_server, (struct sockaddr *)&server_addr, sizeof(server_addr));
		if (bind_result == -1) {
			g_message_dbg("eap_buffer_socket_server bind error[%s]", strerror(errno));
			return -1;
		}
		int listen_result = listen(eap_buffer_socket_server, 1);
		if (listen_result == -1) {
			g_message_dbg("eap_buffer_socket_server listen error[%s]", strerror(errno));
			return -1;
		}
		g_message_dbg("CREATE_EAP_BUFFER_SOCKET_SERVER end");
		g_thread_new( "ea_buf_task", ea_buf_task, NULL );
	//add for eap implement end

		g_thread_new( "ea_ep0Task", ea_ep0Task, NULL );
		g_thread_new( "ea_ep2Task", ea_ep2Task, NULL );
	}
	else
	{
		perror( "Open /dev/ea/ep0" );
	}
	g_message("============================================ end");
	return _fd;

}
static gboolean do_rmmod( char * name)
{
	struct kmod_ctx *_ctx;
	const char * _config = NULL;
	struct kmod_module *_mod;
	int _err	=	0;
	gboolean _res	=	TRUE;

	g_message("%s--->%s", __func__, name);
	_ctx = kmod_new(NULL, &_config);
	if ( ! _ctx )
	{
		g_message("%s--->kmod_new() failed!", __func__);
		return FALSE;
	}

	_err	=	kmod_module_new_from_name( _ctx, name, &_mod);
	if( _err < 0)
	{
		g_message("%s-->kmod_module_new_from_name Failed:%s", __func__, strerror(-_err));
		kmod_unref( _ctx);
		return FALSE;
	}
	
	_err	=	kmod_module_remove_module( _mod, 0);
	if( _err < 0)
	{
		g_message("%s-->kmod_module_remove_module Failed:%s", __func__, strerror(-_err));
		_res	=	FALSE;
	}
	kmod_module_unref( _mod);
	kmod_unref( _ctx);

	return _res;

	
}
static gboolean do_insmod( const char * name, char * option)
{
	struct kmod_ctx *_ctx;
	const char * _config = NULL;
	struct kmod_module *_mod;
	int _err	=	0;
	gboolean _res	=	TRUE;
	g_message("%s-->name:%s--->option:%s", __func__, name , option);
	_ctx = kmod_new(NULL, &_config);
	if ( ! _ctx )
	{
		g_message("%s--->kmod_new() failed!", __func__);
		return FALSE;
	}
	_err	=	kmod_module_new_from_path( _ctx, name, &_mod);
	if( _err < 0)
	{
		g_message("%s-->kmod_module_new_from_path Failed:%s", __func__, strerror(-_err));
		kmod_unref( _ctx);
		return FALSE;
	}
	_err	=	kmod_module_insert_module( _mod, 0, option);
	if( _err < 0)
	{
		g_message("%s-->kmod_module_insert_module Failed:%s", __func__, strerror(-_err));
		_res	=	FALSE;
	}
	kmod_module_unref( _mod);
	kmod_unref( _ctx);
	return _res;

}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ffs_setup
 *  Description:  
 * =====================================================================================
 */
int ffs_setup()
{
	int _fd;
	const char * _kernel_path;
	const char * _kernel_path_ncm;
	const char * _kernel_path_uac2;
	const char * _kernel_path_ffs;
	const char * _kernel_path_usb_ffs;

//	uint8_t * _cert, * _response;
//	int16_t _certLen, _resLen;
//	_certLen	=	iAP2AuthReadCertData( &_cert );
//
	g_message("iAP2 %s",__func__);

	if( mh_misc_get_local_iap_device_mode() != device_mode )
	{
		device_mode = mh_misc_get_local_iap_device_mode();
		_kernel_path = getenv( "MH_KERNEL_DRIVE_PATH" );
		_kernel_path = _kernel_path ? _kernel_path : "/lib/modules/3.18.24-tcc/kernel/drivers/usb/gadget/";
		g_message("%s _kernel_path = %s",__func__,_kernel_path);

//		char * _path;
//		_path	=	g_strdup("/dev/iap2");
//		umount_dev((const char *)_path);
//		g_free( _path);

		if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY ) 
		{
			_kernel_path_ncm = g_strdup_printf("%s%s", _kernel_path, "function/usb_f_ncm.ko" );
			g_message("%s _kernel_path_ncm = %s",__func__,_kernel_path_ncm);
			_kernel_path_ffs = g_strdup_printf("%s%s", _kernel_path, "legacy/g_ffs.ko" );
			g_message("%s _kernel_path_ffs = %s",__func__,_kernel_path_ffs);
			do_rmmod( "g_ffs");
			do_rmmod( "usb_f_uac2");

			do_insmod(_kernel_path_ncm, NULL);
			do_insmod( _kernel_path_ffs, "int_ncm_or_uac2=0");
//			iAP2Desc.header.flags = 1 | 2;
//			system("modprobe usb_f_ncm");
//			system("modprobe g_ffs int_ncm_or_uac2=0");
//			system( _kernel_path_ncm);
//			system( _kernel_path_ffs);
//			system( "insmod /home/root/ko/usb_f_ncm.ko");
//			system( "insmod /home/root/ko/g_ffs.ko int_ncm_or_uac2=0");
		}
		else if( mh_misc_get_local_iap_device_mode() == MISC_IAP )
		{
			_kernel_path_uac2 = g_strdup_printf("%s%s", _kernel_path, "function/usb_f_uac2.ko" );
			g_message("%s _kernel_path_uac2 = %s",__func__,_kernel_path_uac2);
			_kernel_path_ffs = g_strdup_printf("%s%s", _kernel_path, "legacy/g_ffs.ko" );
			g_message("%s _kernel_path_ffs = %s",__func__,_kernel_path_ffs);
			do_rmmod( "g_ffs");
			do_rmmod( "usb_f_ncm");
			do_insmod( _kernel_path_uac2,NULL);
			do_insmod( _kernel_path_ffs, "int_ncm_or_uac2=1");
//			system( "insmod /home/root/ko/usb_f_uac2.ko");
//			system( "insmod /home/root/ko/g_ffs.ko int_ncm_or_uac2=1");
		}
		else if( mh_misc_get_local_iap_device_mode()	==	MISC_IAP_CARLIFE)
		{
			g_message("\n\n\n%s-->load carlie drivers\n\n", __func__);
			do_rmmod("g_ffs");
			do_rmmod("usb_f_fs");
			do_rmmod( "usb_f_ncm");
			_kernel_path_ffs = g_strdup_printf("%s%s", _kernel_path, "legacy/g_ffs.ko" );
			_kernel_path_usb_ffs = g_strdup_printf("%s%s", _kernel_path, "function/usb_f_fs.ko" );
			do_insmod( _kernel_path_usb_ffs, NULL);
			do_insmod( _kernel_path_ffs, "int_ncm_or_uac2=2 functions=iap2,ea");
//			iAP2Desc.header.flags = 1 | 2 | 16;
//			system("modprobe g_ffs functions=iap2,ea");
		}
		else if( mh_misc_get_local_iap_device_mode()	==	MISC_IAP_CARPLAY_CARLIFE)
		{
			_kernel_path_ncm = g_strdup_printf("%s%s", _kernel_path, "function/usb_f_ncm.ko" );
			g_message("%s _kernel_path_ncm = %s",__func__,_kernel_path_ncm);
			_kernel_path_ffs = g_strdup_printf("%s%s", _kernel_path, "legacy/g_ffs.ko int_ncm_or_uac2=0 functions=iap2,ea" );
			g_message("%s _kernel_path_ffs = %s",__func__,_kernel_path_ffs);
			do_rmmod( "g_ffs");
			do_rmmod( "usb_f_uac2");

			do_insmod( _kernel_path_ncm, NULL);
			do_insmod( _kernel_path_ffs, "int_ncm_or_uac2=0 functions=iap2,ea");

		}
	}
	else
	{
		g_message("mh_misc_get_local_iap_device_mode() == device_mode ");
	}

	if(mkdir("/dev/iap2", S_IRUSR|S_IXUSR) < 0)
	{
		g_message("transport.c : ffs_setup-----mkdir /dev/iap2 error!!!!![%s]", strerror(errno));
//		perror("mkdir /dev/iap2 error");
	}
	if(mount("iap2", "/dev/iap2", "functionfs", MS_MGC_VAL, NULL) < 0)
	{
		g_message("transport.c : ffs_setup-----mount /dev/iap2 error!!!!![%s]", strerror(errno));
//		perror("mount iap2 /dev/iap2 error");
	}

//	if( frist_flag == 0 )
//	{
//		if(-1 == mkdir("/dev/iap2", S_IRUSR|S_IXUSR))
//		{
//			perror("mkdir /dev/iap2 error");
//		}
//		if( -1 == mount("iap2", "/dev/iap2", "functionfs", MS_MGC_VAL, NULL))
//		{
//			perror("mount iap2 /dev/iap2 error");
//		}
//	}

	_fd	=	open( "/dev/iap2/ep0", O_RDWR, 0 );
	int desc_ret, str_ret;
	if( _fd > 0 )
	{
		if( (desc_ret = write( _fd, &iAP2Desc, sizeof( iAP2Desc ))) == sizeof( iAP2Desc ))
		{
			g_message( "Write descs success[%d]", desc_ret);
		}
		else
		{
			close( _fd );
			perror( "Write descs to ep0" );

			return -1;
		}

		if( (str_ret = write( _fd, &iAP2Strings, sizeof( iAP2Strings ))) == sizeof( iAP2Strings ))
		{
			g_message( "Write strings success[%d]", str_ret);
		}
		else
		{
			close( _fd );
			perror( "Write strings to ep0" );

			return -1;
		}

		ep0Fd	=	_fd;
		if(mh_misc_get_local_iap_device_mode()	==	MISC_IAP_CARLIFE)
		{
			g_message( "transport.c : open ep1Fd & ep2Fd as carlife");
			ep1Fd	=	open( "/dev/iap2/ep1", O_RDWR | O_NONBLOCK );
			ep2Fd	=	open( "/dev/iap2/ep2", O_RDWR);
//			if ((fcntl(ep2Fd, F_GETFL, 0)) < 0)
//			{
//				/* Handle error */
//				g_message( "transport.c : open ep1Fd & ep2Fd as carplay");
//			}

		}
		else
		{
			g_message( "transport.c : open ep1Fd & ep2Fd as carplay");
			ep1Fd	=	open( "/dev/iap2/ep1", O_RDWR | O_NONBLOCK );
			ep2Fd	=	open( "/dev/iap2/ep2", O_RDWR );

		}
//		ep1Fd	=	open( "/dev/iap2/ep81", O_RDWR | O_NONBLOCK );
//		ep2Fd	=	open( "/dev/iap2/ep02", O_RDWR );
		g_message( "ffs_setup iap2 [%d][%d][%d]", ep0Fd, ep1Fd, ep2Fd);

		ep0State = TRUE;
		ep2State = TRUE;
		//g_thread_new( "ep0Task", ep0Task, NULL );
		//g_thread_new( "ep2Task", ep2Task, NULL );

		if (pthread_create(&ep0threadId,NULL,(void  *) ep0Task,NULL) !=0)
		{
			g_message("------------------>Create pthread error!\n");
		}
		if (pthread_create(&ep2threadId,NULL,(void  *) ep2Task,NULL) !=0)
		{
			g_message("------------------>Create pthread2 error!\n");
		}
	}
	else
	{
		perror( "Open /dev/iap2/ep0" );
	}

	g_message("============================================ start");
	if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY || mh_misc_get_local_iap_device_mode()==MISC_IAP_CARPLAY_CARLIFE )
	{
		system( "ifconfig usbncm0 up");
		g_message("ifconfig usbncm0 up");

//		system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/accept_dad" );
//		usleep(1000);
//		system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/router_solicitation_delay");                  
//		usleep(1000);
//		system( "echo 0 > /proc/sys/net/ipv6/conf/usbncm0/router_solicitation_interval");  
//		usleep(1000);
//				system( "echo 1 > /proc/sys/net/ipv6/conf/usbncm0/optimistic_dad");
//				system( "echo 1 > /proc/sys/net/ipv6/conf/usbncm0/use_optimistic");
	}
	else
	{
		system( "ifconfig usbncm0 down");
		g_message("ifconfig usbncm0 down");
	}
	
//	uint8_t * _cert, * _response;
//	int16_t _certLen, _resLen;
//	_certLen	=	iAP2AuthReadCertData( &_cert );
//

	g_message("============================================ end");
	return _fd;
}		/* -----  end of function ffs_setup  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parse_hid_desc
 *  Description:  
 * =====================================================================================
 */
static void parse_hid_desc( guint8 * report, int length )
{
	int _offset	=	0, i;
	int _dataLen[]	=	{0, 1, 2, 4};
	int _reportSize	=	0;

	reportTable	=	(struct _report_table *)realloc(NULL, sizeof(struct _report_table));
	tableCount	=	0;

	memset(reportTable, 0, sizeof(struct _report_table));

	while(_offset < length)
	{
		int _itemSize;
		int _itemType;
		int _itemTag;
		int _tmpInt;

		if(report[_offset] != 0xFE)             /* short item tag */
		{
			_itemSize	=	_dataLen[(report[_offset] & 0x03)];
			_itemType	=	(report[_offset] & 0x0C) >> 2;
			_itemTag	=	(report[_offset] & 0xF0) >> 4;

			//printf("Item(");

			switch(_itemType)
			{
				case 0:
					//printf("Main  ):"); 
					switch(_itemTag)
					{
					case 0b1000:
						//printf(" Input, data= [ ");

						reportTable[tableCount - 1].direction	=	0;

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");

						break;
					case 0b1001:
						//printf(" Output, data= [ ");

						reportTable[tableCount - 1].direction	=	1;

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");


						break;
					case 0b1011:
						//printf(" Feature, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");


						break;
					case 0b1010:
						//printf(" Collection, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");


						break;
					case 0b1100:
						//printf(" End Collection, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");


						break;
					}
					break;
				case 1:
					//printf("Gloabl):");
					switch(_itemTag)
					{
					case 0b0000:
						//printf(" Usage Page, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");
						break;
					case 0b0001:
						//printf(" Logical Minimum, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");
						break;
					case 0b0010:
						//printf(" Logical Maximum, data= [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");
						break;
					case 0b0111:
						//printf(" Report Size, data= [ ");
						
						_tmpInt	=	0;
						
						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);

							_tmpInt	+=	report[_offset + 1 + i] << (i * 8);
						}

						_reportSize	=	_tmpInt;

						//printf("]");
						break;
					case 0b1000:
						//printf(" Report ID, data = [ ");
						if(tableCount != 0)
						{
							tableCount	++;

							reportTable	=	(struct _report_table *)realloc(reportTable, tableCount * sizeof(struct _report_table));
						}
						else
						{
							tableCount	++;
						}

						reportTable[tableCount - 1].reportId	=	report[_offset + 1];
						reportTable[tableCount - 1].reportSize	=	_reportSize;

						//printf("0x%02x ", report[_offset + 1]);

						//printf("]");

						break;
					case 0b1001:
						//printf(" Report Count, data = [ ");

						_tmpInt	=	0;
						
						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);

							_tmpInt	+=	report[_offset + 1 + i] << (i * 8);
						}

						if(tableCount == 0)
						{
							reportTable[0].reportCount	=	_tmpInt + 1;
						}
						else
						{
							reportTable[tableCount - 1].reportCount	=	_tmpInt + 1;
						}

						//printf("]");

						break;
					}
					break;
				case 2:
					//printf("Local ):");
					switch(_itemTag)
					{
					case 0b0000:
						//printf(" Usage, data = [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");

						break;
					case 0b0001:
						//printf(" Usage Minimum, data = [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");

						break;
					case 0b0010:
						//printf(" Usage Maximum, data = [ ");

						for(i = 0; i < _itemSize; i ++)
						{
							//printf("0x%02x ", report[_offset + 1 + i]);
						}

						//printf("]");

						break;
					default:
						break;
					}
					break;
				default:
					//printf("Reserverd)");
					break;
			}

			//printf("\n");

			_offset		+=	_itemSize + 1;
		}
		else                                    /* long item tag */
		{
			_itemSize	=	report[_offset + 1];/* No long item is supported now. */
			_offset	+=	_itemSize + 3;
		}
	}

}		/* -----  end of static function parse_hid_desc  ----- */

#define HID_DEV_NAME	"/dev/hidraw0"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  setup_hid
 *  Description:  
 * =====================================================================================
 */
int setup_hid( const gchar * device )
{
	int _result	=	0;
	int hidFdtmp;
	int _count	=	0;
	int _fd	=	open( device, O_RDWR, 0 );
	struct hidraw_devinfo _hidInfo;
	struct hidraw_report_descriptor _desc;
	unsigned char _config;
	struct usbdevfs_ctrltransfer _ctrl	=	
	{
		.bRequestType	=	0x80,
		.bRequest		=	0x08,
		.wValue			=	0x00,
		.wIndex			=	0x00,
		.wLength		=	0x01,
		.timeout		=	1000,
		.data			=	&_config
	};

	if( _fd > 0 )
	{
		if( ioctl( _fd, USBDEVFS_CONTROL, &_ctrl ) < 0 )
		{
			perror( "get configuration" );
		}
		else
		{
			if( _config != 2 )
			{
				unsigned int _value	=	2;

				g_message( "Device's Configuration is #%d, changing it to #2", _config );

				if( ioctl( _fd, USBDEVFS_SETCONFIGURATION, &_value ) == 0 )
				{
					_result	=	0;
				}
				else
				{
					perror( " set configuration" );
					close( _fd );
					return 0;
				}
			}
			else
			{
				g_message( "Device's Configuration is #2" );
			}
		}

		close( _fd );
	}
	else
	{
		return 0;
	}

	do
	{
			hidFdtmp	=	open( HID_DEV_NAME, O_RDWR, 0 );
			g_message("HID_DEV_NAME = %s ", HID_DEV_NAME);

		if(( hidFdtmp > 0 ) || ( _count > 10 )) break;

		_count ++;
		
		g_message( "try to open hidraw0 Failed!!!" );	

		usleep( 300000 );

	} while( TRUE );

	if( _count > 10 )
	{
		return 0;
	}

	/* TODO: rewrite following code to get ipod hid device name dynamicly */
	if( ioctl( hidFdtmp, HIDIOCGRAWINFO, &_hidInfo ) == 0 )
	{
		g_message( "%04X %04X", _hidInfo.vendor, _hidInfo.product );
	}

	if( ioctl( hidFdtmp, HIDIOCGRDESCSIZE, &_desc.size ) == 0 )
	{
		g_message( "desc size: %d", _desc.size );

		if( ioctl( hidFdtmp, HIDIOCGRDESC, &_desc ) == 0 )
		{
			parse_hid_desc( _desc.value, _desc.size );

			_result	=	hidFdtmp;
		}
		else
		{
			perror( "setup_hid get desc" );

			close( hidFdtmp );
		}
	}
	else
	{
		perror( "setup_hid get desc size" );

		close( hidFdtmp );
	}

	return hidFdtmp;
}		/* -----  end of function setup_hid  ----- */

 /*
 * ===  FUNCTION  ======================================================================
 *         Name:  chg_timeout
 *  Description:
 * =====================================================================================
 */
static gboolean chg_timeout(gpointer user_data)
{
	g_message("chg_timeout in");

	char * _path;
	ep0State = FALSE;
	ep2State = FALSE;
	if (ep0threadId != 0)
	{
		pthread_cancel(ep0threadId);
		ep0threadId = 0;
	}
	if (ep2threadId != 0)
	{
		pthread_cancel(ep2threadId);
		ep2threadId = 0;
	}
	
	shutdown(ep0Fd, SHUT_RD);
	close( ep0Fd );
	shutdown(ep1Fd, SHUT_WR);
	close( ep1Fd );
	g_message("chg_timeout_____________close ep2Fd_____________________\n\n");
	shutdown(ep2Fd, SHUT_RD);
	close( ep2Fd );
	g_message("chg_timeout_____________umount iap2_____________________\n\n");
	
	usleep(100*1000);
	_path	=	g_strdup("/dev/iap2");
	umount_dev((const char *)_path);
	g_free( _path);
		
	mh_misc_set_local_iap_device_mode( mh_misc_get_iap_device_mode() );
	
	if (nagivi_otg_flag == 1)
	{
		g_message("%s otg host",__func__);
		system( "echo on > /sys/devices/dwc_otg/vbus");
		system( "echo host > /sys/devices/dwc_otg/drdmode");
		nagivi_otg_flag = 0;
	}else if ( intel_otg_flag == 1 )
	{
		echo_write( "/sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state", "host");
	}
						
	return FALSE;
}      /*  -----  end of static function chg_timeout  ----- */ 

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  role_switch
 *  Description:  
 * =====================================================================================
 */
SwitchResult role_switch( struct udev_list_entry * properties )
{
	const char * _mode_name	=	getenv( "MH_IP_MODE_NAME" );
	_mode_name	=	_mode_name ? _mode_name : "host";

	g_message("%s _mode_name = %s",__func__,_mode_name);
	if( strcmp( _mode_name, "device" ) == 0 )
	{
		return 	SWITCH_UNSUPPORTED;
	}

	int _fd;
//	struct usbdevfs_urb _urb	=	{ 0 };
	struct usbdevfs_ctrltransfer _ctrl	=	{ 0 };
	struct udev_list_entry * _model;
	const gchar * _devName, * _devPath;
	SwitchResult _result	=	SWITCH_UNSUPPORTED;

	_model	=	udev_list_entry_get_by_name( properties, "DEVPATH" );
	_devPath	=	udev_list_entry_get_value( _model );
	g_message("%s _devPath = %s",__func__,_devPath);

	/* if device is inserted into Freescale OTG port */
	if( g_strrstr( _devPath, "ci_hdrc.0" ) != NULL )
	{
		g_message( "Freescale chipidea otg controller is found" );
	}
	else if ( g_strrstr( _devPath, "pci" ) != NULL )
	{
		g_message( "intel chipidea otg controller is found" );
		if( g_strrstr( _devPath, "1-1" ) == NULL )
		{
			g_message( "SWITCH_DEV_INVALID intel chipidea otg controller is found 1-1" );
			return SWITCH_DEV_INVALID; 
		}
	}else if ( g_strrstr( _devPath, "dwc_otg" ) != NULL )
	{
		g_message( "B511 otg controller is found" );
	}else{
		return SWITCH_UNSUPPORTED;
	}
	
	if(devPath != NULL)
	{
		g_free( devPath );
		devPath = NULL;
	}
	devPath	=	g_strdup( _devPath );

	_model	=	udev_list_entry_get_by_name( properties, "DEVNAME" );
	_devName	=	udev_list_entry_get_value( _model );
	
	_fd	=	open( _devName, O_RDWR, 0 );

	if( _fd > 0 )
	{
		_ctrl.bRequestType	=	0x40;
		_ctrl.bRequest		=	0x51;

		if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY || mh_misc_get_iap_device_mode()== MISC_IAP_CARPLAY_CARLIFE )
		{
			_ctrl.wValue		=	0x01;
		}else{
			_ctrl.wValue		=	0x00;
		}
	
		_ctrl.wIndex		=	0x00;
		_ctrl.wLength		=	0x00;
		_ctrl.timeout		=	1000;
		_ctrl.data			=	NULL;

//		_urb.usercontext	=	NULL;
//		_urb.type			=	USBDEVFS_URB_TYPE_CONTROL;
//		_urb.endpoint		=	0;
//		_urb.buffer			=	&_ctrl;
//		_urb.buffer_length	=	sizeof( struct usbdevfs_ctrltransfer );

//		if( ioctl( _fd, USBDEVFS_SUBMITURB, &_urb ) == 0 )
		if( ioctl( _fd, USBDEVFS_CONTROL, &_ctrl ) == 0 )
		{
			const gchar * _value;
		
//			if( frist_flag == 0 ) 
//				frist_flag = 1;
//			else 
//				ffs_setup();
			ffs_setup();
			if( mh_misc_get_iap_device_mode() == MISC_IAP_CARLIFE || mh_misc_get_local_iap_device_mode()== MISC_IAP_CARPLAY_CARLIFE)
			{
				g_message("%s[%d] call ffs_ea", __func__, __LINE__);
				ffs_ea();
			}
			_result	=	SWITCH_SUCCESS;

			_model	=	udev_list_entry_get_by_name( properties, "ID_SERIAL_SHORT" );

			if( _model != NULL )
			{
				_value	=	udev_list_entry_get_value( _model );
				/* TODO: Will cause memory leak in some cases, should be fixed in future. */
				if(devSerial != NULL)
				{
					g_free( devSerial );
				}
				devSerial	=	g_strdup( _value );
			}

			if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY || mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY_CARLIFE)
			{
				MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
				_devParam->type				=	MH_DEV_USB_CARPLAY;
				_devParam->mac_addr			=	g_strdup( devSerial );
				_devParam->connect_status	=	3;	

				mh_core_find_dev( _devParam );

				g_free( _devParam->mac_addr );
				g_free( _devParam );
			}

			if( g_strrstr( _devPath, "ci_hdrc.0" ) != NULL )
			{
				/* For freescale iMX6 platform, we need trigger hnp manually */
				echo_write("/sys/bus/platform/devices/ci_hdrc.0/inputs/a_bus_req", "0");

			}else if( g_strrstr( _devPath, "pci" ) != NULL )
			{
				echo_write("/sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state", "peripheral");
				intel_otg_flag	=	1;

				_timeoutSource	=	g_timeout_source_new(5000);
				g_source_set_callback( _timeoutSource, chg_timeout, NULL, NULL );
				mh_io_dispatch( MH_IO( mh_core_instance() ), _timeoutSource );
			}
			else if ( g_strrstr( _devPath, "dwc_otg" ) != NULL )
			{
				g_message("%s otg device",__func__);
	#if 1
				system( "echo device > /sys/devices/dwc_otg/drdmode");
				//system( "/usr/bin/iap2");
	#else
				system( "echo off > /sys/devices/dwc_otg/vbus");
				usleep(300*1000);
				system( "echo device > /sys/devices/dwc_otg/drdmode");
				usleep(300*1000);
				system( "echo on > /sys/devices/dwc_otg/vbus");
	#endif
				//				system( "echo off > /sys/devices/dwc_otg/vbus");
				nagivi_otg_flag	=	1;

				_timeoutSource	=	g_timeout_source_new(5000);
				g_source_set_callback( _timeoutSource, chg_timeout, NULL, NULL );
				mh_io_dispatch( MH_IO( mh_core_instance() ), _timeoutSource );
			
//				system( "ifconfig usbncm0 up");
			}
		}
		else
		{
			g_message( "role switch" );
			if( mh_misc_get_local_iap_device_mode() == MISC_IAP_CARPLAY || mh_misc_get_local_iap_device_mode()== MISC_IAP_CARPLAY_CARLIFE )
			{
				mh_misc_set_local_iap_device_mode( MISC_IAP );
				_ctrl.bRequestType	=	0x40;
				_ctrl.bRequest		=	0x51;
				_ctrl.wValue		=	0x00;
				_ctrl.wIndex		=	0x00;
				_ctrl.wLength		=	0x00;
				_ctrl.timeout		=	1000;
				_ctrl.data			=	NULL;
				if( ioctl( _fd, USBDEVFS_CONTROL, &_ctrl ) == 0 )
				{
					const gchar * _value;
					ffs_setup();
					if( mh_misc_get_iap_device_mode() == MISC_IAP_CARLIFE || mh_misc_get_iap_device_mode() == MISC_IAP_CARPLAY_CARLIFE)
					{
						g_message("%s[%d] call ffs_ea", __func__, __LINE__);
						ffs_ea();
					}

					_result	=	SWITCH_SUCCESS;

					_model	=	udev_list_entry_get_by_name( properties, "ID_SERIAL_SHORT" );

					if( _model != NULL )
					{
						_value	=	udev_list_entry_get_value( _model );
						/* TODO: Will cause memory leak in some cases, should be fixed in future. */
						if(devSerial != NULL)
						{
							g_free( devSerial );
						}
						devSerial	=	g_strdup( _value );
					}

					if( g_strrstr( _devPath, "ci_hdrc.0" ) != NULL )
					{
						/* For freescale iMX6 platform, we need trigger hnp manually */
						echo_write("/sys/bus/platform/devices/ci_hdrc.0/inputs/a_bus_req", "0");
						
						g_message("%s otg host MISC_IAP ci_hdrc ",__func__);
					}else if( g_strrstr( _devPath, "pci" ) != NULL )
					{
						echo_write("/sys/bus/platform/devices/intel-mux-drcfg/portmux.0/state", "peripheral");
						intel_otg_flag	=	1;
						
						_timeoutSource	=	g_timeout_source_new(5000);
						g_source_set_callback( _timeoutSource, chg_timeout, NULL, NULL );
						mh_io_dispatch( MH_IO( mh_core_instance() ), _timeoutSource );

						g_message("%s otg host MISC_IAP pci ",__func__);
					}
					else if ( g_strrstr( _devPath, "dwc_otg" ) != NULL )
					{
						g_message("%s otg device",__func__);
						system( "echo device > /sys/devices/dwc_otg/drdmode");
						nagivi_otg_flag	=	1;
						
						g_message("%s otg host MISC_IAP dwc_otg ",__func__);
					}
				}
			}
			else{
				perror( "role switch 2" );
			}
		}

		close( _fd );
	}
	else
	{
		_result	=	SWITCH_DEV_INVALID;
	}

	return _result;
}		/* -----  end of static function role_switch  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  host_read_iap2
 *  Description:  
 * =====================================================================================
 */
gint host_read_iap2( guint8 * buf, gint len, int hid, MHDev * dev )
{
	return read( ep2Fd, buf, len );
}		/* -----  end of function host_read_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  host_write_iap2
 *  Description:  
 * =====================================================================================
 */
gint host_write_iap2( guint8 * buf, gint len,  int hid, MHDev * dev )
{
	int _size	=	write( ep1Fd, buf, len );

	if( _size < 0 )
		perror( __func__ );

	return _size;
}		/* -----  end of function host_write_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_read
 *  Description:  
 * =====================================================================================
 */
gint dev_read( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
	gint _size;

	_size	=	read( _hidFd, buf, len );
	if( _size < 0 )
	{
		perror( __func__ );

		close( _hidFd );
	}

	return _size;
}		/* -----  end of function dev_read  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_write
 *  Description:  
 * =====================================================================================
 */
gint dev_write( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
	int i, _size, _reportLen	=	0;
	guint8 * _report;
	int _reportId;

	for(i = 0; i < tableCount; i ++)
	{
		if(reportTable[i].direction != 1) continue;

		if( _reportLen < (reportTable[i].reportCount * reportTable[i].reportSize) / 8 )
		{
			_reportLen	=	(reportTable[i].reportCount * reportTable[i].reportSize) / 8;
			_reportId	=	reportTable[i].reportId;
		}

		if(( len + 2 ) < (reportTable[i].reportCount * reportTable[i].reportSize) / 8)
		{
			break;
		}
	}

	_report	=	g_new0( guint8, _reportLen );

	if( _reportLen >= len + 2 )
	{
		_report[0]	=	_reportId;
		_report[1]	=	MAKE_LCB( TRUE, TRUE );

		memcpy( _report + 2, buf, len );

		if(( _size = write( _hidFd, _report, len + 2 )) != len + 2 )
		{
			perror( "dev_write" );
		}
	}
	else
	{
		int _remain	=	len;
		int _offset	=	0;
		uint8_t _lcb;

		_lcb	=	MAKE_LCB( TRUE, FALSE );

		do
		{
			memset( _report, 0, _reportLen );

			_report[0]	=	_reportId;
			_report[1]	=	_lcb;

			if(_remain <= _reportLen - 2)
			{
				_lcb	=	MAKE_LCB(FALSE, TRUE);
				_report[1]	=	_lcb;
				memcpy(_report+ 2, buf + _offset, _remain);
			}
			else
			{
				_lcb	=	MAKE_LCB(FALSE, FALSE);
				memcpy(_report+ 2, buf + _offset, _reportLen - 2);
			}

			if(( _size = write( _hidFd, _report, _reportLen )) != _reportLen )
			{
				perror( "dev_write" );
			}

			_offset	+=	_reportLen - 2;
			_remain	-=	_reportLen - 2;
		} while( _remain > 0 );
	}

	free( _report );

	return _size;
}		/* -----  end of function dev_write  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bt_write_iap2
 *  Description:  
 * =====================================================================================
 */
gint bt_write_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
//	g_message("bt_write_iap2  ************************** start\n");
//	DEBUG_HEX_DISPLAY( buf, len );
//	g_message("bt_write_iap2  ************************** end\n");

	GVariant * _var;

	_var	=	g_variant_new_fixed_array( G_VARIANT_TYPE_BYTE, buf, len, sizeof( guchar ));
	_var	=	g_variant_new_variant( _var );
	g_signal_emit_by_name( MH_DEV_IAP2( dev ), "bt_data", _var );

	return len;
}		/* -----  end of function bt_write_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  bt_read_iap2
 *  Description:  
 * =====================================================================================
 */
gint bt_read_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
	int _size	=	0;

	return _size;
}		/* -----  end of function bt_read_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  wifi_write_iap2
 *  Description:  
 * =====================================================================================
 */
gint wifi_write_iap2_dev( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
	mh_dev_wifi_write_iap2( carplay, buf, len ); 	
	return len;
}		/* -----  end of function wifi_write_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  wifi_read_iap2
 *  Description:  
 * =====================================================================================
 */
gint wifi_read_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev )
{
	int _size	=	0;

	return _size;
}		/* -----  end of function wifi_read_iap2  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iap2_hid_task
 *  Description:  
 * =====================================================================================
 */
gpointer iap2_hid_task( gpointer user_data )
{
	MHDevIap2 * _iap2	=	MH_DEV_IAP2( user_data );
	guint8 _buf[ 768 ];
	ssize_t _size;
	BOOL _detect;
	static iAP2Packet_t * iPacket	=	NULL;

	while( TRUE )
	{
		int _size	=	read( _iap2->_iap2hidFd, _buf, sizeof( _buf ));

		if( _size < 0 )
		{
			perror( "iap2_hid_task read" );

			if( iPacket != NULL )
				iAP2PacketDelete( iPacket );

			iPacket	=	NULL;

			close( _iap2->_iap2hidFd );

			break;
		}
		else
		{
			if( iPacket == NULL )
				iPacket	=	iAP2PacketCreateEmptyRecvPacket( _iap2->plinkRunLoop->link );

			iAP2PacketParseBuffer( _buf + 2, _size - 2, iPacket, 0, &_detect, NULL, NULL );

			if( _detect )
				g_message( "iAP 1.0/2.0 has been detected" );

			if( iAP2PacketIsComplete( iPacket ))
			{
				iAP2LinkRunLoopHandleReadyPacket( _iap2->plinkRunLoop, iPacket );

				iPacket	=	NULL;
			}
		}
	}

	return NULL;
}		/* -----  end of function iap2_hid_task  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  closehidFd
 *  Description:  
 * =====================================================================================
 */
void closehidFd(int hid)
{
	close( hid );
}		/* -----  end of function closehidFd  ----- */

