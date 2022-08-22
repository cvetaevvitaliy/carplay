/*
 * =====================================================================================
 *
 *       Filename:  iAP2ControlSession.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 11:41:59 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_CONTROLSESSION_H__
#define __IAP2_CONTROLSESSION_H__

#include "iAP2Defines.h"

void iAP2ParseControlSession (iAP2Link_t * link, uint8_t * data, uint32_t dataLen, uint8_t session );
void iAP2SendControlMessageWithParam (iAP2Link_t * link, uint16_t messageId, uint16_t paramId,uint8_t * data, uint32_t dataLen, uint8_t session);
int8_t iAP2SendControlMessage ( iAP2Link_t * link, uint16_t messageId, uint8_t * data, uint32_t dataLen, uint8_t session );
void iAP2StartMLUpdate( iAP2Link_t * link );
void iAP2StopMLUpdate( iAP2Link_t * link );
void iAP2StopMLMessageUpdate( iAP2Link_t * link, uint8_t * data, uint32_t dataLen);

#endif
