/*
 * =====================================================================================
 *
 *       Filename:  iAP2Hid.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/25/2013 11:51:20 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <iAP2Link.h>
#include <glib.h>
#include <string.h>
#include "iAP2Hid.h"
#include "debug.h"
#include "iAP2ControlSession.h"
#include "dev_iap2.h"

static uint8_t hidDesc[]	=	{
	0x05, 0x0C,	//USAGE_PAGE (Consumer Devices)
	0x09, 0x01,	//USAGE (Consumer Control)
	0xA1, 0x01,	//COLLECTION (Application)
	0x15, 0x00,	//LOGICAL_MINIMUM (0)
	0x25, 0x01,	//LOGICAL_MAXIMUM (1)
	0x09, 0xB0,	//USAGE (Play)
	0x09, 0xB1,	//USAGE (Pause)
	0x09, 0xB5,	//USAGE (Scan Next Track)
	0x09, 0xB6,	//USAGE (Scan Previous Track)
	0x09, 0xB9,	//USAGE (Shuffle)
	0x09, 0xBC,	//USAGE (Repeat)
	0x09, 0xBE,	//USAGE (Reset audiobook playback speed)
	0x09, 0xCA,	//USAGE	(Increase audiobook playback speed)
	0x09, 0xCB,	//USAGE	(Decrease audiobook playback speed)
	0x09, 0xCD,	//USAGE (Play/Pause) 
	0x09, 0xCF, //USAGE (Siri)
	0x09, 0xE2,	//USAGE (Mute)
	0x09, 0xE9,	//USAGE (Volume Up)
	0x09, 0xEA,	//USAGE (Volume Down)
	0x0A, 0x5B, 0x02, //USAGE (iTunes Radio "Play More Like This")
	0x0A, 0x5C, 0x02, //USAGE (iTunes Radio "Never Play This Song")
	0x0A, 0x62, 0x02, //USAGE (iTunes Radio "Add to iTunes Wish List")
	0x75, 0x01,	//REPORT_SIZE (1)
	0x95, 0x11,	//REPORT_COUNT (17)
	0x81, 0x02,	//INPUT (Data,Var,Abs)
	0x75, 0x07,	//REPORT_SIZE (7)
	0x95, 0x01,	//REPORT_COUNT (1)
	0x81, 0x03,	//INPUT (Cnst,Var,Abs)
	0xC0	//END_COLLECTION 
};


static uint8_t hidStartParams[]	=	{
	0x00, 0x06, 0x00, 0x00, 0x01, 0x01,
	0x00, 0x06, 0x00, 0x01, 0x06, 0xAC,
	0x00, 0x06, 0x00, 0x02, 0x00, 0x01
};

uint8_t hidMessage[0x06 + 0x07]	=	{
	0x00, 0x06, 0x00, 0x00, 0x01, 0x01,
	0x00, 0x07, 0x00, 0x01
};
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2GetHidStartParams
 *  Description:  
 * =====================================================================================
 */
void iAP2GetHidStartParams ( uint8_t ** hidParam, int * length )
{
	int _len	=	sizeof(hidDesc) + 4 + sizeof(hidStartParams);
	uint8_t * _param	=	g_malloc0(_len);

	memcpy(_param, hidStartParams, sizeof(hidStartParams));

	_param[sizeof(hidStartParams) + 0]	=	IAP2_HI_BYTE(sizeof(hidDesc) + 4);
	_param[sizeof(hidStartParams) + 1]	=	IAP2_LO_BYTE(sizeof(hidDesc) + 4);
	_param[sizeof(hidStartParams) + 2]	=	0x00;
	_param[sizeof(hidStartParams) + 3]	=	0x04;

	memcpy(_param + sizeof(hidStartParams) + 4, hidDesc, sizeof(hidDesc));

	* hidParam	=	_param;
	* length	=	_len;
}		/* -----  end of function iAP2GetHidStartParams  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2HidExecCommand
 *  Description:  
 * =====================================================================================
 */
void iAP2HidExecCommand (iAP2Link_t * link, iAP2HidCommand cmd)
{
	int _byte, _bit;
	uint8_t _hidDesc[3]	=	{0};

	_byte	=	cmd	/ 8;
	_bit	=	cmd % 8;

	_hidDesc[_byte]	=	(1 << _bit);

	memcpy(hidMessage + 6 + 4, _hidDesc, sizeof(_hidDesc));

	DEBUG_HEX_DISPLAY(hidMessage, sizeof(hidMessage));

	iAP2SendControlMessage(link, 0x6802, hidMessage, sizeof(hidMessage), IAP2_CONTROL_SESSION_ID);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2HidExecCommandDone
 *  Description:  
 * =====================================================================================
 */
void iAP2HidExecCommandDone (iAP2Link_t * link )
{
	uint8_t _hidDesc[3]	=	{0};

	memcpy(hidMessage + 6 + 4, _hidDesc, sizeof(_hidDesc));

	DEBUG_HEX_DISPLAY(hidMessage, sizeof(hidMessage));
	iAP2SendControlMessage(link, 0x6802, hidMessage, sizeof(hidMessage), IAP2_CONTROL_SESSION_ID);
}		/* -----  end of function iAP2HidExecCommandDone  ----- */
