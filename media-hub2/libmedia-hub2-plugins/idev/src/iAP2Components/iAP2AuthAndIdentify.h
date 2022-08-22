/*
 * =====================================================================================
 *
 *       Filename:  iAP2AuthAndIdentify.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 01:08:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_AUTHANDIDENTIFY_H__
#define __IAP2_AUTHANDIDENTIFY_H__

#include <iAP2Link.h>
#include <dev_iap2.h>
#include <glib.h>

typedef struct _IDENTIFY_PARAM_ 
{
	uint16_t id;
	uint8_t * param;
	int length;
} IDENTIFY_PARAM;				/* ----------  end of struct IDENTIFY_PARAM  ---------- */

void iAP2GetIndentifyParam ( MHDevIap2 * _iAP2Object, uint8_t ** idParams, int * paramsLen );
uint8_t iAP2GetNowPlayingUpdateFlag();
uint8_t iAP2GetCallStateUpdateFlag();
uint8_t iAP2GetListUpdatesFlag();
int16_t iAP2AuthReadCertData( uint8_t ** data );
gboolean iAP2AuthWriteChallengeData( uint8_t * data, uint16_t len );
int16_t iAP2AuthReadChallengeResponse( uint8_t ** data );

#endif
