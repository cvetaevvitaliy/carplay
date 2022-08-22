/*
 * =====================================================================================
 *
 *       Filename:  mh_misc.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/03/2014 11:43:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_MISC_H__
#define __MH_MISC_H__

#include <mh_api.h>
#include <glib.h>
#include <mh_pb.h>

typedef struct _MHFilterType 
{
	char * filter;
	MHItemType type;
} MHFilterType;				/* ----------  end of struct MHFilterType  ---------- */

typedef struct _MHFilterTypes 
{
	guint size;
	MHFilterType * data;
} MHFilterData;				/* ----------  end of struct MHFilterTypes  ---------- */

MHItemType mh_misc_get_file_type( gchar * name );
MHItemType mh_misc_get_ext_type(gchar  * ext);
MHFilterData * mh_misc_get_filter();
MiscIapDeviceMode mh_misc_get_iap_device_mode();
MiscIapDeviceMode mh_misc_get_local_iap_device_mode();
MHResult mh_misc_set_local_iap_device_mode( MiscIapDeviceMode mode );
char *  mh_misc_get_bluetoothids();
MHPb *  mh_misc_get_pb_handle();
void mh_misc_read_carplay_init_modes( uint8_t ** data );
uint32_t mh_misc_get_righthand();
#endif
