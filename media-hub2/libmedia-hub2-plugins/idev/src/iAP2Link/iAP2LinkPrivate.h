/*
 *	File: iAP2LinkPrivate.h
 *	Package: iAP2Link
 *	Abstract: n/a 
 *
 *	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple
 * 	Inc. ("Apple"), in your capacity as a current, and in good standing,
 *	Licensee in the MFi Licensing Program. Use of this Apple software is
 *	governed by and subject to the terms and conditions of your MFi License,
 *	including, but not limited to, the restrictions specified in the provision
 *	entitled “Public Software”, and is further subject to your agreement to
 *	the following additional terms, and your agreement that the use,
 *	installation, modification or redistribution of this Apple software
 * 	constitutes acceptance of these additional terms. If you do not agree with
 * 	these additional terms, please do not use, install, modify or redistribute
 *	this Apple software.
 *
 *	In consideration of your agreement to abide by the following terms, and
 *	subject to these terms, Apple grants you a personal, non-exclusive
 *	license, under Apple's copyrights in this original Apple software (the
 *	"Apple Software"), to use, reproduce, and modify the Apple Software in
 *	source form, and to use, reproduce, modify, and redistribute the Apple
 *	Software, with or without modifications, in binary form. While you may not
 *	redistribute the Apple Software in source form, should you redistribute
 *	the Apple Software in binary form, in its entirety and without
 *	modifications, you must retain this notice and the following text and
 *	disclaimers in all such redistributions of the Apple Software. Neither the
 *	name, trademarks, service marks, or logos of Apple Inc. may be used to
 *	endorse or promote products derived from the Apple Software without
 *	specific prior written permission from Apple. Except as expressly stated
 *	in this notice, no other rights or licenses, express or implied, are
 *	granted by Apple herein, including but not limited to any patent rights
 *	that may be infringed by your derivative works or by other works in which
 *	the Apple Software may be incorporated.
 *	
 *	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES
 *	NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 *	IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 *	PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 *	ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 *	WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 *	LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 *	OF SUCH DAMAGE.
 *
 *	Copyright (C) 2012 Apple Inc. All Rights Reserved.
 *
 */

#ifndef iAP2Link_iAP2LinkPrivate_h
#define iAP2Link_iAP2LinkPrivate_h


#ifdef __cplusplus
extern "C" {
#endif

/*
****************************************************************
**
**  Routines for state event handling
**
****************************************************************
*/

void iAP2LinkActionNone (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionHandleACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionSendData (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionDetach (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionSendACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionResendMissing (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionResendData (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionHandleData (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionNotifyConnectionFail (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkActionSwitchToiAP1 (struct iAP2FSM_st* fsm, unsigned int* nextEvent);


/*
****************************************************************
**
**  iAP2LinkSetDefaultSYNParam
**
**  Input:
**      None
**
**  Output:
**      param:  SYN packet parameters struct to set to default
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkSetDefaultSYNParam (iAP2PacketSYNData_t* param);


/*
****************************************************************
**
**  iAP2LinkIsValidSynParam
**
**  Input:
**      synParam:       SYN packet parameters
**
**  Output:
**      None
**
**  Return:
**      BOOL    returns TRUE if parameters are valid, else FALSE
**
****************************************************************
*/
BOOL iAP2LinkIsValidSynParam (iAP2PacketSYNData_t* synParam);


/*
****************************************************************
**
**  iAP2LinkValidateSynParam
**
**  Input:
**      synParam:       SYN packet parameters
**
**  Output:
**      synParam:       SYN packet parameters with modified values
**                      if invalid values detected.
**
**  Return:
**      BOOL    returns TRUE if parameters were valid, else FALSE
**
****************************************************************
*/
BOOL iAP2LinkValidateSynParam (iAP2PacketSYNData_t* synParam);


/*
****************************************************************
**
**  iAP2LinkProcessOutQueue
**
**  Input:
**      link:   link structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Process outgoing data... called in response to DataToSend Event
**          to send out packets from the queue(s)
**
****************************************************************
*/
void iAP2LinkProcessOutQueue (iAP2Link_t* link);


/*
****************************************************************
**
**  iAPLinkProcessInOrderPacket
**
**  Input:
**      link:   link structure
**      packet: packet to handle
**
**  Output:
**      None
**
**  Return:
**      BOOL    returns TRUE if processed, else FALSE if invalid packet
**
**  Note: Process a parsed packet and generate appropriate event.
**        This is called after check for in order sequence has been done.
**
****************************************************************
*/
BOOL iAP2LinkProcessInOrderPacket (struct iAP2Link_st* link,
                                   iAP2Packet_t*       packet);


/*
****************************************************************
**
**  iAP2LinkSendPacket
**
**  Input:
**      link:           link structure
**      packet:         packet to send
**      bResend:        this is a resend of a packet
**      tag:            tag for debug logging
**
**  Output:
**      link:       link's various info is updated to reflect packet send
**      packet:     packet's timeStamp is updated to current time
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkSendPacket (iAP2Link_t*    link,
                         iAP2Packet_t*  packet,
                         BOOL           bResend,
                         const char*    tag);


/*
****************************************************************
**
**  iAP2LinkSendPacketWaitSend
**
**  Input:
**      link:           link structure
**      packet:         packet to send
**      bResend:        this is a resend of a packet
**      tag:            tag for debug logging
**
**  Output:
**      link:       link's various info is updated to reflect packet send
**      packet:     packet's timeStamp is updated to current time
**
**  Return:
**      Will wait for packet to be sent out.
**
****************************************************************
*/
void iAP2LinkSendPacketWaitSend (iAP2Link_t*      link,
                                 iAP2Packet_t*    packet,
                                 BOOL             bResend,
                                 const char*      tag);


/*
****************************************************************
**
**  iAP2LinkResetSeqAck
**
**  Input:
**      link:           link structure
**      bOnlySend:      reset only seq# related info
**
**  Output:
**      none
**
**  Return:
**      none
**
****************************************************************
*/
void iAP2LinkResetSeqAck (iAP2Link_t* link, BOOL bOnlySend);


/*
****************************************************************
**
**  iAP2LinkPacketForIndex
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      index:              index to get packet for
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet, NULL if not found.
**
****************************************************************
*/
iAP2Packet_t* iAP2LinkPacketForIndex (uint8_t* listArrayBuffer, uint8_t index);


/*
****************************************************************
**
**  iAP2LinkFindPacket
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      packet:             packet to find in list.
**      func:               Compare function to compare items.
**
**  Output:
**      None
**
**  Return:
**      uint8_t index of found list item, kiAP2ListArrayInvalidIndex if not found.
**
****************************************************************
*/
uint8_t iAP2LinkFindPacket (uint8_t*                    listArrayBuffer,
                            iAP2Packet_t**              packet,
                            piAP2ListArrayCompareFunc   func);


/*
****************************************************************
**
**  iAP2LinkAddPacketAfter
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      prevItemIndex:      index of list item to add new item after.
**      packet:             packet to add.
**
**  Output:
**      listArrayBuffer:    list is updated to add new item
**
**  Return:
**      uint8_t index of added item.
**
**  Note:   If prev != kiAP2ListArrayInvalidIndex, new list node will be added behind prev.
**          If root != kiAP2ListArrayInvalidIndex and prev == kiAP2ListArrayInvalidIndex,
**              new item will be inserted at the beginning of the list.
**          If root == kiAP2ListArrayInvalidIndex and prev == kiAP2ListArrayInvalidIndex,
**              then the item becomes the first and only item in the list.
**
****************************************************************
*/
uint8_t iAP2LinkAddPacketAfter (uint8_t*        listArrayBuffer,
                                uint8_t         prevItemIndex,
                                iAP2Packet_t**  packet);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Link_iAP2LinkPrivate_h */
