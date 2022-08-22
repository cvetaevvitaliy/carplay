/*
 * =====================================================================================
 *
 *       Filename:  mh_carplay.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/26/2015 02:21:42 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_CARPLAY_H__
#define __MH_CARPLAY_H__
#include <mh_api.h>

void mh_ipc_start_carplay_engine( MHIPCServer * server );
void mh_ipc_carplay_client_init( MHIPCConnection * conn );

#endif

