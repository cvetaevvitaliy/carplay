/*
 * =====================================================================================
 *
 *       Filename:  iAP2Hid.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/25/2013 12:02:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_HID_H__
#define __IAP2_HID_H__

typedef enum _iAP2HidCommand_
{
	HID_PLAY,						/* Play */
	HID_PAUSE,						/* Pause */
	HID_SCAN_NEXT_TRACK,			/* Transport Right */
	HID_SCAN_PREVIOUS_TRACK,		/* Transport Left */
	HID_RANDOM_PLAY,				/* Shuffle */
	HID_REPEAT,						/* Repeat */
	HID_TRACKING_NORMAL,			/* Reset audiobook playback speed to default */
	HID_TRACKING_INCREMENT,			/* Increase audiobook playback speed */
	HID_TRACKING_DECREMENT,			/* Decrease audiobook playback speed */
	HID_PLAY_PAUSE,					/* Play/Pause */
	HID_VOICE_COMMAND,				/* Siri */
	HID_MUTE,						/* Mute */
	HID_VOLUME_INCREMENT,			/* Louder */
	HID_VOLUME_DECREMENT,			/* Softer */
	HID_PROMOTE,					/* iTunes Radio "Play More Like This" */
	HID_DEMOTE,						/* iTunes Radio "Never Play This Song" */
	HID_ADD_TO_CART					/* iTunes Radio "Add to iTunes Wish List" */
} iAP2HidCommand;					/* ----------  end of enum iAP2HidCommand  ---------- */

void iAP2GetHidStartParams ( uint8_t ** hidParam, int * length );
void iAP2HidExecCommand (iAP2Link_t * link, iAP2HidCommand cmd);
void iAP2HidExecCommandDone (iAP2Link_t * link);

#endif
