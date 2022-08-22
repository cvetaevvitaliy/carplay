/*
 * =====================================================================================
 *
 *       Filename:  object.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/23/2014 09:36:12 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_api.h>
#include <glib-object.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_set
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_get_properties( MHObject * object, const char * first_property_name, ... )
{

	g_return_val_if_fail( G_IS_OBJECT( object ), MH_INVALID_PARAM );

	MHResult _res	=	MH_OK;

	va_list _varArgs;

	va_start( _varArgs, first_property_name );

	g_object_get_valist( G_OBJECT( object ), first_property_name, _varArgs );

	va_end( _varArgs );

	return _res;
}		/* -----  end of function mh_object_set  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_set
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_set_properties( MHObject * object, const char * first_property_name, ... )
{

	g_return_val_if_fail( G_IS_OBJECT( object ), MH_INVALID_PARAM );
	
	MHResult _res	=	MH_OK;

	va_list _varArgs;

	va_start( _varArgs, first_property_name );

	g_object_set_valist( G_OBJECT( object ), first_property_name, _varArgs );

	va_end( _varArgs );

	return _res;
}		/* -----  end of function mh_object_set  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_unref
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_unref( MHObject * object )
{
	g_message("%s ======================object = %p",__func__,object);
	g_return_val_if_fail( G_IS_OBJECT( object ), MH_INVALID_PARAM);

	MHResult _res	=	MH_OK;

	g_object_unref( object );

	return _res;
}		/* -----  end of function mh_object_unref  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_ref
 *  Description:  
 * =====================================================================================
 */
MHObject * mh_object_ref( MHObject * object )
{
	g_message("%s ======================object = %p",__func__,object);
	g_return_val_if_fail( G_IS_OBJECT( object ), NULL );

	return ( MHObject * )g_object_ref( object );
}		/* -----  end of function mh_object_ref  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_signal_connect
 *  Description:  
 * =====================================================================================
 */
uint64_t mh_object_signal_connect( MHObject * object, const char * signal, void * handler, void * user_data )
{
	g_warning( "Function %s should only be used in IPC library", __func__ );
//	return g_signal_connect( object, signal, handler, user_data );
	return 0;
}		/* -----  end of function mh_object_signal_connect  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_object_signal_disconnect
 *  Description:  
 * =====================================================================================
 */
MHResult mh_object_signal_disconnect( uint64_t signal_id )
{
	g_warning( "Function %s should only be used in IPC library", __func__ );
	MHResult _res	=	MH_OK;
	return _res;
//	g_signal_handler_disconnect( object, signal_id );
}		/* -----  end of function mh_object_signal_disconnect  ----- */
