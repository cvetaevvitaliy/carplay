/*
 * =====================================================================================
 *
 *       Filename:  misc.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/27/2016 08:49:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include<glib.h>
#include<mh_api.h>
#include<gio/gio.h>
//#include <gio/gunixsocketaddress.h>
#include<string.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>
#include <stdio.h>
#include<stdlib.h>
#include <netinet/in.h>
MHResult mh_misc_create_dev(char * type, char * serial, char * path)
{	
	int _fd;

	struct sockaddr_un _addr;  

	int _dataLen;
	char * _data;

	MHResult _res	=	MH_OK;
	if( (_fd	= socket(AF_UNIX, SOCK_STREAM, 0)) < 0) 
	{
		g_message("%s->socket error", __func__);
		_res	=	MH_IPC_ERROR;

	}
	else
	{
		_addr.sun_family = AF_UNIX;  

		strcpy(_addr.sun_path, "/tmp/device_socket");  

		if( connect(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		{
			g_message("%s->connect failed", __func__);

			_res	=	MH_IPC_ERROR;

		}
		else
		{
			_data	=	g_strdup_printf("%s;%s;%s", type, serial, path);

			_dataLen	=	send( _fd, _data, strlen( _data), 0);

			if( _dataLen < 0)
			{
				g_message("%s->send error", __func__);
			}
			else
			{
				g_message("%s->send %d", __func__, strlen( _data));
				g_message("%s->send return %d", __func__, _dataLen);
			}
		}
		close( _fd);

	}
	return _res;

	
}
