/*
 * =====================================================================================
 *
 *       Filename:  sample.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/17/2014 03:44:32 PM
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
//#include <mh_extern_misc.h>

gpointer input_thread( gpointer data)
{
	char input[256];
	while(1)
	{
		if(scanf("%s", input)!=0)
		{
			printf( "runing\n" );
			switch( input[0] )
			{
				case 'a':
					{
						mh_misc_set_iap_device_mode( MISC_IAP_CARLIFE);
					}
					break;

				case 'b':
					{
						mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY);
					}
					break;
				case 'c':
					{
						mh_misc_set_iap_device_mode( MISC_IAP_CARPLAY_CARLIFE);
					}
					break;
				default:
					break;
			}

		}
	}
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
static GMainLoop *test_loop;
int main ( int argc, char *argv[] )
{


	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	mh_ipc_media_client_init( _conn );

	g_thread_new( "mh_test_input", input_thread, NULL );
	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

