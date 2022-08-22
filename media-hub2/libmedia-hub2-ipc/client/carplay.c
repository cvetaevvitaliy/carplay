/*
 * =====================================================================================
 *
 *       Filename:  carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/21/2015 03:16:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#include <mh_api.h>
#include <stdio.h>
#include <string.h>
#ifdef __x86_64__
	#include<mh_dbus_x64.h>
#else
	#include <mh_dbus.h>
#endif

extern Mediahub2Dbus * dbusClient;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_take_screen
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_take_screen( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "screen:take", NULL );
}		/* -----  end of function mh_dev_carplay_take_screen  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_untake_screen
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_untake_screen( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "screen:untake", NULL );
}		/* -----  end of function mh_dev_carplay_untake_screen  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_borrow_screen
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_borrow_screen( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "screen:borrow", NULL );
}		/* -----  end of function mh_dev_carplay_borrow_screen  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_unborrow_screen
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_unborrow_screen( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "screen:unborrow", NULL );
}		/* -----  end of function mh_dev_carplay_unborrow_screen  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_take_main_audio
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_take_main_audio( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "main_audio:take", NULL );
}		/* -----  end of function mh_dev_carplay_take_main_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_untake_main_audio
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_untake_main_audio( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "main_audio:untake", NULL );
}		/* -----  end of function mh_dev_carplay_untake_main_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_borrow_main_audio
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_borrow_main_audio( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "main_audio:borrow", NULL );
}		/* -----  end of function mh_dev_carplay_borrow_main_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_unborrow_main_audio
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_unborrow_main_audio( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "change_modes", "main_audio:unborrow", NULL );
}		/* -----  end of function mh_dev_carplay_unborrow_main_audio  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_request_ui
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_request_ui( MHDev * dev, const char * ui )
{
	g_message(" %s ui = [%s] \n",__func__, ui );
	mh_object_set_properties( ( MHObject * )dev, "request_ui", ui, NULL );
}		/* -----  end of function mh_dev_carplay_request_ui  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_request_siri_prewarm
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_request_siri_prewarm( MHDev * dev )
{
	g_message(" %s \n",__func__);
	mh_object_set_properties( ( MHObject * )dev, "request_siri_action", 1, NULL );
}		/* -----  end of function mh_dev_carplay_request_siri_prewarm  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_request_siri_button_down
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_request_siri_button_down( MHDev * dev )
{
	g_message(" %s \n",__func__);
	mh_object_set_properties( ( MHObject * )dev, "request_siri_action", 2, NULL );
}		/* -----  end of function mh_dev_carplay_request_siri_button_down  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_request_siri_button_up
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_request_siri_button_up( MHDev * dev )
{
	g_message(" %s \n",__func__);
	mh_object_set_properties( ( MHObject * )dev, "request_siri_action", 3, NULL );
}		/* -----  end of function mh_dev_carplay_request_siri_button_up  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_set_night_mode
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_set_night_mode( MHDev * dev, bool mode )
{
	g_message(" %s mode = [%d]\n",__func__, mode );
	mh_object_set_properties( ( MHObject * )dev, "night_mode", mode, NULL );
}		/* -----  end of function mh_dev_carplay_set_night_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_send_signal_touch
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_send_signal_touch( MHDev * dev, int press, int x, int y )
{
	static int _press	=	0, _x	=	0, _y	=	0;
//	g_message( "%s press = [%d] x = [%d] y = [%d]\n",__func__, press, x, y );

	if( _press < 0 || _press > 1 )
	{
		g_warning( "press: 0/1" );
		
		return;
	}

	if( x > 1280 || x < 0 )
	{
		g_warning( "x: 0 - 1280" );

		return;
	}

	if( y > 720 || y < 0 )
	{
		g_warning( "y: 0 - 720" );

		return;
	}

	if( _press != press || _x != x || _y != y )
	{
		char _buf[20];
		
		sprintf( _buf, "%d,%04d,%04d", press, x, y );
		
		_press	=	press;
		_x		=	x;
		_y		=	y;
		
		mh_object_set_properties( ( MHObject * )dev, "signal_touch", _buf, NULL );
	}
}		/* -----  end of function mh_dev_carplay_send_signal_touch  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_force_key_frame
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_force_key_frame ( MHDev * dev )
{
	mh_object_set_properties( ( MHObject * )dev, "keyframe", FALSE, NULL );
}		/* -----  end of function mh_dev_carplay_force_key_frame  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_change_resource_mode
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_change_resource_mode( MHDev * dev , const uint8_t * mode )
{
	char _buf[10];

	sprintf( _buf, "res:%d%d%d%d%d", mode[0],mode[1],mode[2],mode[3],mode[4]);

	g_message("%s res mode = [%s]\n",__func__, _buf);

	mh_object_set_properties( ( MHObject * )dev, "change_modes", _buf, NULL );
}		/* -----  end of function mh_dev_carplay_change_resource_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_change_app_mode
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_change_app_mode( MHDev * dev , const uint8_t * mode )
{
	char _buf[10];

	sprintf( _buf, "app:%d%d%d", mode[0],mode[1],mode[2]);

	g_message("%s app mode = [%s]\n",__func__, _buf);

	mh_object_set_properties( ( MHObject * )dev, "change_modes", _buf, NULL );

}		/* -----  end of function mh_dev_carplay_change_app_mode  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_send_media_button
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_send_media_button( MHDev * dev, int btn )
{
	g_message("%s media btn = [%d]\n",__func__, btn);

	if( btn < 0 || btn > 7 )
	{
		g_warning( "btn: 0 - 6" );
		
		return;
	}

	mh_object_set_properties( ( MHObject * )dev, "media_button", btn, NULL );
}		/* -----  end of function mh_dev_carplay_send_media_button  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_send_phone_button
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_send_phone_button( MHDev * dev, int key )
{
	g_message("%s phone btn = [%d]\n",__func__, key);

	if( key > 20 || key < 0 )
	{
		g_warning( "key: 0 - 11" );

		return;
	}

	mh_object_set_properties(( MHObject * )dev, "phone_key", key, NULL );
}		/* -----  end of function mh_dev_carplay_send_phone_button  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  mh_dev_carplay_set_limited_ui
 *  Description:  
 * =====================================================================================
 */
void mh_dev_carplay_set_limited_ui( MHDev * dev, bool flag )
{
	g_message("%s limitedUI = [%d]\n",__func__, flag);

	mh_object_set_properties(( MHObject * )dev, "set_limit_ui", flag, NULL );
}		/* -----  end of function mh_dev_carplay_set_limited_ui  ----- */
