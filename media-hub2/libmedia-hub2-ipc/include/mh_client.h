/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/21/2015 01:39:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <glib.h>
#include <mh_api.h>
#include <gio/gunixfdlist.h>
typedef struct  
{
	GDBusConnection * dbus_conn;
	char * connect_name;
	uint16_t connect_port;
	uint32_t connect_scope_id;
	GList * handle_id;
	

}IPCConnect;			
