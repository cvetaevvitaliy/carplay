/*
 * =====================================================================================
 *
 *       Filename:  filter.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/24/2014 01:34:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif
extern Mediahub2Dbus * dbusClient;


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_filter_create
 *  Description:  
 * =====================================================================================
 */
MHFilter * mh_filter_create( const char * filter)
{
	g_return_val_if_fail( filter != NULL, NULL);
	gboolean _dbusRes;
#ifdef __x86_64__
	guint64 _filter;
	_dbusRes	=	mediahub2_dbus_call_filter_create_sync( dbusClient,(const gchar *)filter, 
				(guint64 *)&_filter, NULL, NULL);
#else
	guint _filter;
	_dbusRes	=	mediahub2_dbus_call_filter_create_sync( dbusClient,(const gchar *)filter, 
				(guint *)&_filter, NULL, NULL);

#endif
	if( _dbusRes)
	{
		return ( MHFilter *) _filter;
	}
	else
	{
		g_message("%s:dbus_error\n",__func__);
		return NULL;
	}

}		/* -----  end of function mh_filter_create  ----- */
