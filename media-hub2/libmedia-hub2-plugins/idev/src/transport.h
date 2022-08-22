/*
 * =====================================================================================
 *
 *       Filename:  transport.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/07/2015 06:35:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__
#include <glib.h>
#include <libudev.h>
#include <dev_iap2.h>

typedef enum _SwitchResult
{
	SWITCH_SUCCESS,
	SWITCH_UNSUPPORTED,
	SWITCH_DEV_INVALID
} SwitchResult;				/* ----------  end of enum SwitchResult  ---------- */

gint host_read_iap2( guint8 * buf, gint len, int hid, MHDev * dev );
gint host_write_iap2( guint8 * buf, gint len, int hid, MHDev * dev );
gint host_read_ea( guint8 * buf, gint len );
gint host_write_ea( guint8 * buf, gint len );
gint dev_read( guint8 * buf, gint len, int _hidFd, MHDev * dev );
gint dev_write( guint8 * buf, gint len, int _hidFd, MHDev * dev );
gint bt_read_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev );
gint bt_write_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev );
gint wifi_read_iap2( guint8 * buf, gint len, int _hidFd, MHDev * dev );
gint wifi_write_iap2_dev( guint8 * buf, gint len, int _hidFd, MHDev * dev );

int ffs_setup();
SwitchResult role_switch( struct udev_list_entry * properties );
int setup_hid( const gchar * device );
gpointer iap2_hid_task( gpointer user_data );
void closehidFd(int hid);
#endif

