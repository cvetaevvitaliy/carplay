/*
 *	File: iAP2Link.c
 *	Package: iAP2Link
 *	Abstract: n/a 
 *
 *	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple
 * 	Inc. ("Apple"), in your capacity as a current, and in good standing,
 *	Licensee in the MFi Licensing Program. Use of this Apple software is
 *	governed by and subject to the terms and conditions of your MFi License,
 *	including, but not limited to, the restrictions specified in the provision
 *	entitled â€œPublic Softwareâ€? and is further subject to your agreement to
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iAP2FSM.h>
#include <iAP2LinkConfig.h>
#include <iAP2Time.h>
#include <iAP2BuffPool.h>
#include <iAP2ListArray.h>
#include <iAP2Log.h>

#include "iAP2Packet.h"
#include "iAP2Link.h"
#include "iAP2LinkPrivate.h"
#if iAP2_LINK_USE_LINKRUNLOOP
#include "iAP2LinkRunLoop.h"
#endif
#include <glib.h>

#if DEBUG
#define iAP2LINK_DEBUG 1
#define iAP2LINK_DEBUG_PACKET 1
#endif

#ifdef iAP2_FOR_ACCESSORY
/* Accessory States */
extern const iAP2FSMState_t iAP2LinkAccessoryStates [kiAP2LinkStateCount];
#endif

#ifdef iAP2_FOR_DEVICE
/* Device States */
extern const iAP2FSMState_t iAP2LinkDeviceStates [kiAP2LinkStateCount];
#endif


#if iAP2LINK_DEBUG
const char* stateNames [kiAP2LinkStateCount] =
{
    "Init",
    "Detached",
    "Detect",
    "Idle",
    "Pending",
    "Connected",
    "Paused",
    "Failed"
};

const char* eventNames [kiAP2LinkEventCount] =
{
    "InitDone",
    "Attach",
    "RecvSYN",
    "RecvSYNACK",
    "RecvSYNACKOLD",
    "RecvSYNACKNEW",
    "RecvRST",
    "WaitACKTimeout",
    "SendACKTimeout",
    "Detach",
    "RecvData",
    "RecvLastData",
    "EAK",
    "DataToSend",
    "MaxResend",
    "RecvACK",
    "RecvACKBadLink",
    "RecvDetect",
    "RecvDetectBad",
    "RecviAP1Packet",
    "WaitDetectTimeout",
    "Suspend"
};
#endif /*#if iAP2LINK_DEBUG*/


const uint8_t   kIap2PacketDetectData[]     = { 0xFF, 0x55, 0x02, 0x00, 0xEE, 0x10 };
uint32_t        kIap2PacketDetectDataLen    = 6;
const uint8_t   kIap2PacketDetectBadData[]  = { 0xFF, 0x55, 0x04, 0x00, 0x02, 0x04, 0xEE, 0x08 };
uint32_t        kIap2PacketDetectBadDataLen = 8;
extern GRecMutex * loop_mutex_iAP2;

/*
****************************************************************
**
**  __AnySessionSendListHasPacket
**
**  return TRUE if any sessSendPckList contains something to send out.
**
****************************************************************
*/
 BOOL __AnySessionSendListHasPacket (iAP2Link_t* link)
{
    if (link)
    {
        int i;
        for (i = 0; i < kIAP2PacketServiceTypeCount; ++i)
        {
            if (iAP2ListArrayGetCount (link->sessSendPckList[i]) > 0)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
****************************************************************
**
**  _DeletePckCB
**
**  Callback functions used for iAP2ListArray()
**
****************************************************************
*/
 void _DeletePckCB (void* item)
{
    if (item != NULL)
    {
        iAP2Packet_t* pck = *((iAP2Packet_t**) item);
        if (pck)
        {
            iAP2PacketDelete (pck);
        }
    }
}


/*
****************************************************************
**
**  _ComparePckTimerIdCB
**
**  Callback functions used for iAP2ListArray()
**
****************************************************************
*/
 intptr_t _ComparePckTimerIdCB (void* a, void* b, uint8_t dataSize)
{
    iAP2Packet_t* aPck = *((iAP2Packet_t**) a);
    iAP2Packet_t* bPck = *((iAP2Packet_t**) b);
    assert (aPck && bPck && dataSize == sizeof(uintptr_t));
    return aPck->timer - bPck->timer;
}


#if iAP2LINK_DEBUG
/*
 ****************************************************************
 **
 **  __printPacketItemWithIndexAndParam
 **
 ****************************************************************
 */
 void __printPacketItemWithIndexAndParam (void* item, uint8_t index, void* param)
{
    iAP2LogType_t* pType = (iAP2LogType_t*) param;
    iAP2Packet_t* pck = *((iAP2Packet_t**) item);
    iAP2LogTypeNL(*pType, "        [%03u] entry(%p): control=%xh seq=%u ack=%u sessID=%u pckLen=%u\n",
                  index, pck, pck->pckData->ctl, pck->pckData->seq, pck->pckData->ack, pck->pckData->sess, pck->packetLen);
}
#endif /* #if iAP2LINK_DEBUG */


/*
****************************************************************
**
**  __printPacketList
**
****************************************************************
*/
 void __printPacketList (const char*  tag,
                               const char*  typeStr,
                               iAP2Link_t*  link,
                               uint8_t*     listBuffer,
                               BOOL         needStartStop,
                               BOOL         bDebug)
{
#if iAP2LINK_DEBUG
    iAP2LogType_t type = (bDebug ? kiAP2LogTypeData : kiAP2LogTypeLog);
    if (needStartStop)
    {
        iAP2LogStart();
    }
    iAP2LogTypeNL(type, "%s%sPACKET LIST - %s (%p): link=%p\n",
                  (tag ? tag : ""), (tag ? ": " : ""), typeStr, listBuffer, link);
    iAP2LogTypeNL(type, "%s%s    count=%u first=%u last=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  iAP2ListArrayGetCount (listBuffer),
                  iAP2ListArrayGetFirstItemIndex (listBuffer), iAP2ListArrayGetLastItemIndex (listBuffer));
    if (bDebug)
    {
        iAP2ListArrayForEachWithIndexAndParam (listBuffer, __printPacketItemWithIndexAndParam, &type);
    }
    if (needStartStop)
    {
        iAP2LogStop();
    }
#endif /* #if iAP2LINK_DEBUG */
}


/*
****************************************************************
**
**  _iAP2LinkComparePacketSeq
**
**  return 0 if packet1 and packet2 have same seq#
**  return < 0 if packet1 has smaller seq# than packet2
**  return > 0 if packet1 has larger seq# than packet2
**
****************************************************************
*/
 intptr_t _iAP2LinkComparePacketSeq (void* a, void* b, uint8_t dataSize)
{
    iAP2Packet_t* pck1 = *((iAP2Packet_t**) a);
    iAP2Packet_t* pck2 = *((iAP2Packet_t**) b);
    iAP2Link_t* link;
    long result = iAP2PacketCalcSeqGap (pck1->pckData->seq, pck2->pckData->seq);
    assert (pck1->link == pck2->link);
    link = (iAP2Link_t*) pck1->link;
    if (0 == result)
    {
        return result;
    }
    else if (result > link->param.peerMaxOutstandingPackets)
    {
        return result - 256;
    }
    return result;
}


/*
** Reset the sendPckList
*/
 void _ResetAckedPackets (iAP2Link_t* link)
{
    iAP2ListArrayCleanup (link->sendPckList, _DeletePckCB);
}


/*
** Clean up the sendPckList based on received ACK#.
** - remove from list all packets that have been ACK'd.
*/
 void _CleanupAckedPackets (iAP2Link_t* link)
{
    BOOL bNeedNotify = FALSE;
    uint8_t item = iAP2ListArrayGetFirstItemIndex (link->sendPckList);
    __printPacketList (NULL, "SEND", link, link->sendPckList, TRUE, TRUE);
    while (item != kiAP2ListArrayInvalidIndex)
    {
        uint8_t nextItem = iAP2ListArrayGetNextItemIndex (link->sendPckList, item);
        iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->sendPckList, item);
/********************************************liushukun***************/
#if 0
        if (pck &&
            iAP2PacketIsSeqACKd (pck->pckData->seq + pck->seqPlus,
                                 link->recvAck,
                                 link->param.peerMaxOutstandingPackets))
#endif
		if (pck &&
            iAP2PacketIsSeqACKd (pck->pckData->seq,
                                 link->recvAck + pck->seqPlus,
                                 link->param.peerMaxOutstandingPackets))
/********************************************liushukun***************/
        {
#if iAP2LINK_DEBUG
            iAP2LogStart();
            iAP2LogDbg("%s:%d _CleanupAckedPackets DeleteACK'd seq=%u seqPlus=%u recvAck=%u window=%u\n",
                       __FILE__, __LINE__,
                       pck->pckData->seq,
                       pck->seqPlus,
                       link->recvAck,
                       link->param.peerMaxOutstandingPackets);
#if iAP2LINK_DEBUG_PACKET
            iAP2PacketDebugPrintPacketNL (pck, __FILE__, __LINE__, "_CleanupAckedPackets", "");
#endif
            iAP2LogStop();
#endif
#if DEBUG
#if iAP2_LINK_ALLOW_STATS
            {
                uint32_t curTimeMs;
                curTimeMs = iAP2TimeGetCurTimeMs();
                link->totACKDelays += (curTimeMs - pck->timeStamp);
                ++(link->numACKDelays);
            }
#endif /*#if iAP2_LINK_ALLOW_STATS*/
#endif
            iAP2ListArrayDeleteItem (link->sendPckList, item, _DeletePckCB);
            __printPacketList (NULL, "SEND after deleteItem", link, link->sendPckList, TRUE, TRUE);
            bNeedNotify = TRUE;
        }
#if iAP2LINK_DEBUG
        else
        if (0)
        {
            iAP2LogDbg("%s:%d _CleanupAckedPackets unACK'd seq=%u seqPlus=%u recvAck=%u window=%u\n",
                       __FILE__, __LINE__,
                       pck->pckData->seq,
                       pck->seqPlus,
                       link->recvAck,
                       link->param.peerMaxOutstandingPackets);
        }
#endif
        item = nextItem;
    }
    if (bNeedNotify)
    {
        link->signalSendBuffCB (link);
    }
}


/*
** Reset the recvPckList
*/
 void _ResetRecvPackets (iAP2Link_t* link)
{
    iAP2ListArrayCleanup (link->sendPckList, _DeletePckCB);
}


/*
** Clean up the recvPckList based on received SEQ#.
** - process and remove from list all packets that have been received in sequence.
*/
 void _CleanupRecvPackets (iAP2Link_t* link)
{
    uint8_t item = iAP2ListArrayGetFirstItemIndex (link->recvPckList);
    __printPacketList (NULL, "RECV", link, link->recvPckList, TRUE, TRUE);
    while (item != kiAP2ListArrayInvalidIndex)
    {
        iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->recvPckList, item);
        uint8_t nextItem = iAP2ListArrayGetNextItemIndex (link->recvPckList, item);
        if (pck)
        {
            uint8_t seqDiff = (link->bValidRecvSeq
                               ? iAP2PacketCalcSeqGap (link->recvSeq, pck->pckData->seq)
                               : kiAP2LinkSynValMaxOutstandingMax + 1);
            if (!link->bValidRecvSeq ||
                (seqDiff <= link->param.maxOutstandingPackets &&
                 iAP2PacketNextSeq(link->recvSeq) == pck->pckData->seq))
            {
                /* In sequence */
#if iAP2LINK_DEBUG
                iAP2LogDbg("%s:%d _CleanupRecvPackets ProcessPacket seq=%u seqPlus=%u recvSeq=%u window=%u\n",
                           __FILE__, __LINE__,
                           pck->pckData->seq,
                           pck->seqPlus,
                           link->recvSeq,
                           link->param.maxOutstandingPackets);
#endif
                iAP2ListArrayDeleteItem (link->recvPckList, item, NULL);
                __printPacketList (NULL, "RECV after deleteItem", link, link->recvPckList, TRUE, TRUE);
                iAP2LinkProcessInOrderPacket (link, pck);
                iAP2PacketDelete (pck);
                nextItem = iAP2ListArrayGetFirstItemIndex (link->recvPckList);
            }
            else if (seqDiff > link->param.maxOutstandingPackets)
            {
                /* Outside window... delete. */
                iAP2ListArrayDeleteItem (link->recvPckList, item, _DeletePckCB);
                __printPacketList (NULL, "RECV after deleteItem", link, link->recvPckList, TRUE, TRUE);
            }
            else
            {
                /* Not in sequence yet, skip. */
            }
        }
        item = nextItem;
    }
}


/*
** Return TRUE if packet has been received already
*/
 BOOL _IsReceivedPacket (iAP2Link_t*   link,
                               iAP2Packet_t* packet)
{
    BOOL bReceived = FALSE;
    uint8_t seqDiff = iAP2PacketCalcSeqGap (link->recvSeq, packet->pckData->seq);
    if (seqDiff <= link->param.maxOutstandingPackets)
    {
        uint8_t item = iAP2LinkFindPacket (link->recvPckList, &packet, _iAP2LinkComparePacketSeq);
        bReceived = (item != kiAP2ListArrayInvalidIndex);
    }
#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d _IsReceivedPacket bReceived=%d pck-seq=%u seqPlus=%u\n",
               __FILE__, __LINE__,
               bReceived,
               packet->pckData->seq,
               packet->seqPlus);
#endif
    return bReceived;
}


 void _iAP2LinkHandleTimerCancel (iAP2Timer_t* timer, uint8_t timerID)
{
    if (timer)
    {
        iAP2Packet_t tmpPck;
        iAP2Packet_t* pTmpPck = &tmpPck;
        uint8_t      item;
        iAP2Link_t*  link = iAP2TimeGetContext (timer);
        tmpPck.timer = timerID;
        item = iAP2LinkFindPacket(link->sendPckList, &pTmpPck, _ComparePckTimerIdCB);
        if (item != kiAP2ListArrayInvalidIndex)
        {
            iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->sendPckList, item);
            iAP2PacketRemoveTimer (pck);
        }
        else
        {
            item = iAP2ListArrayGetFirstItemIndex (link->recvPckList);
            if (item != kiAP2ListArrayInvalidIndex)
            {
                iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->recvPckList, item);
                iAP2PacketRemoveTimer (pck);
            }
        }
    }
}


 void _iAP2LinkHandleTimerExpire (iAP2Timer_t* timer,
                                        uint8_t      timeoutID,
                                        uint8_t      timeoutType,
                                        uint32_t     curTime)
{
    if (timer)
    {
        iAP2Link_t* link = (iAP2Link_t*) iAP2TimeGetContext (timer);
        if (link)
        {
#if iAP2_LINK_USE_LINKRUNLOOP
            if (link->bUseiAP2LinkRunLoop)
            {
                iAP2LinkRunLoopTimeout (link->context,
                                        timeoutID,
                                        timeoutType,
                                        curTime);
            }
            else
#endif
            {
                switch (timeoutType)
                {
                    case kiAP2LinkEventWaitACKTimeout:
                        iAP2LinkHandleWaitACKTimeoutLink (link, curTime);
                        break;
                    case kiAP2LinkEventSendACKTimeout:
                        iAP2LinkHandleSendACKTimeoutLink (link, curTime);
                        break;
                    case kiAP2LinkEventWaitDetectTimeout:
                        iAP2LinkHandleWaitDetectTimeoutLink (link, curTime);
                        break;
                }
            }
        }
    }
}


/*
** Prepare a packet with out of order packet seq#s payload
*/
 iAP2Packet_t* _PrepareEAKDataPacket (iAP2Link_t*  link)
{
    iAP2Packet_t* packet = NULL;
    if (link)
    {
        uint8_t  max = iAP2ListArrayGetCount(link->recvPckList);
        if (max > iAP2LinkGetMaxSendPayloadSize (link))
        {
            max = iAP2LinkGetMaxSendPayloadSize (link);
        }
        packet = iAP2PacketCreateEAKPacket (link,
                                            link->sentSeq,
                                            link->recvSeq,
                                            NULL,
                                            max);
        if (packet)
        {
            uint8_t  count = 0;
            uint8_t* pt = packet->pckData->data;
            uint8_t  item = iAP2ListArrayGetFirstItemIndex (link->recvPckList);
            while (count < max && item != kiAP2ListArrayInvalidIndex)
            {
                iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->recvPckList, item);
                if (pck)
                {
                    *(pt++) = pck->pckData->seq;
                    ++count;
                }
                item = iAP2ListArrayGetNextItemIndex (link->recvPckList, item);
            }
        }
    }
    return packet;
}


/*
****************************************************************
**
**  _iAP2LinkStartSendAckTimerIfNotRunning
**
****************************************************************
*/
 void _iAP2LinkStartSendAckTimerIfNotRunning (iAP2Link_t* link, uint8_t seq)
{
    /* Set SendAckTimer once until expiry */
    if (INVALID_TIMEOUT_ID == link->sendAckTimeoutID)
    {
        link->sendAckTimeoutID = iAP2TimeCallbackAfter(link->mainTimer,
                                                       kiAP2LinkEventSendACKTimeout,
                                                       link->param.cumAckTimeout);
    }
}


/*
****************************************************************
**
**  _iAP2LinkNeedEAK
**
****************************************************************
*/
 BOOL _iAP2LinkNeedEAK (iAP2Link_t* link)
{
    BOOL result = FALSE;
    if (link->recvPckList && iAP2ListArrayGetCount(link->recvPckList) > 0)
    {
        uint8_t item = iAP2ListArrayGetFirstItemIndex (link->recvPckList);
        uint8_t lowSeqDiff = 0xff;
        while (item != kiAP2ListArrayInvalidIndex)
        {
            iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->recvPckList, item);
            if (pck)
            {
                uint8_t seqDiff = (link->bValidRecvSeq
                                   ? iAP2PacketCalcSeqGap (link->recvSeq, pck->pckData->seq)
                                   : kiAP2LinkSynValMaxOutstandingMax + 1);
                if (seqDiff < lowSeqDiff)
                {
                    lowSeqDiff = seqDiff;
                }
            }
            item = iAP2ListArrayGetNextItemIndex (link->recvPckList, item);
        }
        if (lowSeqDiff > 1 && lowSeqDiff <= link->param.maxOutstandingPackets)
        {
            result = TRUE;
        }
    }
    return result;
}


/*
****************************************************************
** **************************************************************
**
**  BEGIN Routines for state event handling
**
** **************************************************************
****************************************************************
*/

/*
****************************************************************
**
**  iAP2LinkActionNone
**
****************************************************************
*/
void iAP2LinkActionNone (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;
}

/*
****************************************************************
**
**  iAP2LinkActionHandleACK
**
****************************************************************
*/
void iAP2LinkActionHandleACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    /* ACK received */

#if iAP2LINK_DEBUG
    {
        iAP2Link_t* link = (iAP2Link_t*) fsm->data;
        
        iAP2LogDbg("%s:%d %s sendSeq=%u recvAck=%u\n", __FILE__, __LINE__,
                   (link->type == kiAP2LinkTypeAccessory
                    ? "Accessory:HandleACK"
                    : "Device:HandleACK"),
                   link->sentSeq,
                   link->recvAck);
    }
#endif
}

/*
****************************************************************
**
**  iAP2LinkActionSendData
**
****************************************************************
*/
void iAP2LinkActionSendData (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    iAP2LinkProcessOutQueue (link);
}


/*
****************************************************************
**
**  iAP2LinkActionDetach
**
****************************************************************
*/
void iAP2LinkActionDetach (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    /* Call connected callback */
    if (link->connectedCB)
    {
        /* Notify about the connection establishment */
        (*link->connectedCB) (link, FALSE);
    }
}


/*
****************************************************************
**
**  iAP2LinkActionSendACK
**
****************************************************************
*/
void iAP2LinkActionSendACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    iAP2TimeCancelTimer(link->mainTimer, link->sendAckTimeoutID);
    link->sendAckTimeoutID = INVALID_TIMEOUT_ID;

    /* Check if we need to send an EAK instead. */
    if ( ! _iAP2LinkNeedEAK (link) )
    {
        /* Send ACK */
        iAP2Packet_t* pck = iAP2PacketCreateACKPacket (link,
                                                       link->sentSeq,
                                                       link->recvSeq,
                                                       NULL,
                                                       0,
                                                       kIAP2PacketReservedSessionID);
#if iAP2LINK_DEBUG
        iAP2LogDbg ("%s:%d %s recvSeq=%u sentAck=%u\n",
                    __FILE__, __LINE__,
                    (link->type == kiAP2LinkTypeAccessory
                     ? "Accessory:SendACK"
                     : "Device:SendACK"),
                    link->recvSeq,
                    link->sentAck);
#endif
#if iAP2_LINK_ALLOW_STATS
        ++(link->numSentACK);
#endif
        iAP2LinkSendPacket (link, pck, FALSE,
                            (link->type == kiAP2LinkTypeAccessory
                             ? "Accessory:SendACK"
                             : "Device:SendACK"));
    }
    else
    {
        /* Send EAK */
        if (iAP2ListArrayGetCount(link->recvPckList))
        {
            iAP2Packet_t* eakPacket = _PrepareEAKDataPacket (link);
            if (eakPacket)
            {
#if iAP2LINK_DEBUG
                iAP2LogDbg ("%s:%d %s recvSeq=%u sentAck=%u\n",
                            __FILE__, __LINE__,
                            (link->type == kiAP2LinkTypeAccessory
                             ? "Accessory:SendACK"
                             : "Device:SendACK"),
                            link->recvSeq,
                            link->sentAck);
#endif
#if iAP2_LINK_ALLOW_STATS
                ++(link->numOutOfOrder);
                ++(link->numSentEAK);
#endif
                iAP2LinkSendPacket(link, eakPacket, FALSE,
                                   (link->type == kiAP2LinkTypeAccessory
                                    ? "Accessory:SendEAK"
                                    : "Device:SendEAK"));
            }
            else
            {
                assert(FALSE);
            }
        }
        else
        {
            assert(FALSE);
        }
    }
}


/*
****************************************************************
**
**  iAP2LinkActionResendMissing
**
****************************************************************
*/
void iAP2LinkActionResendMissing (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t*     link;
    iAP2Packet_t*   eakPacket;
    uint32_t        missingLen;
    uint8_t*        missing;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    /* EACK received */

    link = (iAP2Link_t*) fsm->data;

#if iAP2LINK_DEBUG_PACKET
    iAP2PacketDebugPrintPacket (link->recvPck, __FILE__, __LINE__,
                                (link->type == kiAP2LinkTypeAccessory
                                 ? "Accessory:ResendMissing"
                                 : "Device:ResendMissing"),
                                "");
#endif

    eakPacket  = link->recvPck;
    missingLen = 0;
    missing    = iAP2PacketGetMissingSeqFromEAK (eakPacket,
                                                 link->param.peerMaxOutstandingPackets,
                                                 &missingLen);
    if (missing != NULL && missingLen > 0)
    {
        uint8_t*      pt = missing;
        uint8_t*      ptEnd = pt + missingLen;
        iAP2Packet_t* pTmpPck = iAP2PacketCreateEmptySendPacket (link);
        pTmpPck->link = link;
        while (pt < ptEnd)
        {
            uint8_t item;
            pTmpPck->pckData->seq = *pt;
            item = iAP2LinkFindPacket(link->sendPckList, &pTmpPck, _iAP2LinkComparePacketSeq);
            if (item != kiAP2ListArrayInvalidIndex)
            {
                iAP2Packet_t* pck = iAP2LinkPacketForIndex (link->sendPckList, item);
                assert(pck);
                if (!iAP2PacketIsACKOnly(pck))
                {
#if iAP2LINK_DEBUG
                    iAP2LogDbg("%s:%d %s Found (look for seq=%u) seq=%u recvAck=%u timeStamp=%u reTxCount=%d/%d\n",
                               __FILE__, __LINE__,
                               (link->type == kiAP2LinkTypeAccessory
                                ? "Accessory:ResendMissing"
                                : "Device:ResendMissing"),
                               *pt,
                               pck->pckData->seq, link->recvAck,
                               pck->timeStamp, pck->retransmitCount,
                               link->param.maxRetransmissions);
#endif
                    if (pck->retransmitCount < link->param.maxRetransmissions)
                    {
                        ++(pck->retransmitCount);
#if iAP2_LINK_ALLOW_STATS
                        ++(link->missingReTxCount);
                        ++(link->numResentDATA);
#endif
                        iAP2LinkSendPacket (link, pck, TRUE,
                                            (link->type == kiAP2LinkTypeAccessory
                                             ? "Accessory:ResendMissing"
                                             : "Device:ResendMissing"));
                    }
                    else
                    {
                        iAP2LogError("%s:%d %s Resend too many times!\n",
                                     __FILE__, __LINE__,
                                     (link->type == kiAP2LinkTypeAccessory
                                      ? "Accessory:ResendMissing"
                                      : "Device:ResendMissing"));
                        *nextEvent = kiAP2LinkEventMaxResend;
                    }
                }
                else
                {
                    /* Cleanup old ACK packet */
                    iAP2ListArrayDeleteItem(link->sendPckList, item, _DeletePckCB);
                }
            }
            ++pt;
        }
        iAP2PacketDelete (pTmpPck);
        pTmpPck = NULL;
    }

    if (missing != NULL)
    {
        iAP2BuffPoolReturn (link->buffPool, missing);
    }
}


/*
****************************************************************
**
**  iAP2LinkActionResendData
**
****************************************************************
*/
void iAP2LinkActionResendData (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;
    uint32_t curTime;
    uint32_t expireTime;
    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    /* Retransmit unack'd packets */
    curTime = iAP2TimeGetCurTimeMs();
    expireTime = curTime + link->param.retransmitTimeout;

    if (iAP2ListArrayGetCount(link->sendPckList) > 0)
    {
        uint8_t item = iAP2ListArrayGetFirstItemIndex (link->sendPckList);
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d %s sendSeq=%u recvAck=%u\n", __FILE__, __LINE__,
                   (link->type == kiAP2LinkTypeAccessory
                    ? "Accessory:ResendData"
                    : "Device:ResendData"),
                   link->sentSeq,
                   link->recvAck);
#endif
        while (item != kiAP2ListArrayInvalidIndex)
        {
            uint8_t nextItem = iAP2ListArrayGetNextItemIndex (link->sendPckList, item);
            iAP2Packet_t* packet = iAP2LinkPacketForIndex (link->sendPckList, item);
            if (packet && packet->timeStamp <= expireTime)
            {
                uint8_t seqDiff = (link->bValidRecvSeq
                                   ? iAP2PacketCalcSeqGap (link->recvAck, packet->pckData->seq)
                                   : kiAP2LinkSynValMaxOutstandingMax + 1);
                if (seqDiff > 0 &&
                    seqDiff <= link->param.maxOutstandingPackets &&
                    !iAP2PacketIsACKOnly(packet))
                {
#if iAP2LINK_DEBUG
                    iAP2LogDbg("%s:%d %s seq=%u seqPlus=%u recvAck=%u timeStamp=%u expireTime=%u reTxCount=%d/%d\n",
                               __FILE__, __LINE__,
                               (link->type == kiAP2LinkTypeAccessory
                                ? "Accessory:ResendData"
                                : "Device:ResendData"),
                               packet->pckData->seq, packet->seqPlus, link->recvAck,
                               packet->timeStamp, expireTime,
                               packet->retransmitCount,
                               link->param.maxRetransmissions);
#endif
                    if (packet->retransmitCount < link->param.maxRetransmissions)
                    {
                        ++(packet->retransmitCount);
#if iAP2_LINK_ALLOW_STATS
                        ++(link->noAckReTxCount);
                        ++(link->numResentDATA);
#endif
                        iAP2LinkSendPacket (link, packet, TRUE,
                                            (link->type == kiAP2LinkTypeAccessory
                                             ? "Accessory:ResendData"
                                             : "Device:ResendData"));
                    }
                    else
                    {
                        iAP2LogError("%s:%d %s Resend too many times!\n",
                                     __FILE__, __LINE__,
                                     (link->type == kiAP2LinkTypeAccessory
                                      ? "Accessory:ResendData"
                                      : "Device:ResendData"));
                        *nextEvent = kiAP2LinkEventMaxResend;
                    }
                }
                else
                {
                    /* Cleanup old/ACK packet */
                    iAP2ListArrayDeleteItem (link->sendPckList, item, _DeletePckCB);
                }
            }
            item = nextItem;
        }
    }
}


/*
****************************************************************
**
**  iAP2LinkActionHandleData
**
****************************************************************
*/
void iAP2LinkActionHandleData (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;
    iAP2Packet_t* pck;
    iAP2PacketSessionInfo_t* pSession;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    pck = link->recvPck;
#if iAP2_LINK_ALLOW_STATS
    link->dataBytesRcvd += iAP2PacketGetPayloadLen (pck);
    ++(link->dataPacketsRcvd);
#endif
    pSession = iAP2LinkGetSessionInfo (link, pck->pckData->sess);
    if (pSession)
    {
#if iAP2LINK_DEBUG
        iAP2PacketDebugPrintPacket(pck, __FILE__, __LINE__,
                                   (link->type == kiAP2LinkTypeAccessory
                                    ? "Accessory:HandleData"
                                    : "Device:HandleData"),
                                   "");
#endif
        (*link->recvDataCB) (link,
                             iAP2PacketGetPayload (pck),
                             iAP2PacketGetPayloadLen (pck),
                             pck->pckData->sess);
    }
    else
    {
        iAP2LogError("%s:%d Invalid session(%u)!\n", __FILE__, __LINE__, pck->pckData->sess);
    }

    _iAP2LinkStartSendAckTimerIfNotRunning (link, pck->pckData->seq);
}


/*
****************************************************************
**
**  iAP2LinkActionNotifyConnectionFail
**
****************************************************************
*/
void iAP2LinkActionNotifyConnectionFail (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d %s connectedCB=%p\n",
               __FILE__, __LINE__,
               (link->type == kiAP2LinkTypeAccessory
                ? "Accessory:ConnectionFail"
                : "Device:ConnectionFail"),
               link->connectedCB);
#endif
    if (link->connectedCB)
    {
        /* Notify about the connection establishment */
        (*link->connectedCB) (link, FALSE);
    }
}


/*
****************************************************************
**
**  iAP2LinkActionSwitchToiAP1
**
****************************************************************
*/
void iAP2LinkActionSwitchToiAP1 (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d %s sendDetectCB=%p\n",
               __FILE__, __LINE__,
               (link->type == kiAP2LinkTypeAccessory
                ? "Accessory:SendDetect"
                : "Device:SendDetect"),
               link->sendDetectCB);
#endif

    if (link->sendDetectCB)
    {
        /* Send Detect BAD ACK byte seqeunce */
        (*link->sendDetectCB) (link, TRUE);
    }
    if (link->connectedCB)
    {
        /* Notify about the connection failure */
        (*link->connectedCB) (link, FALSE);
    }
}

/*
****************************************************************
** **************************************************************
**
**  END Routines for state event handling
**
** **************************************************************
****************************************************************
*/



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
void iAP2LinkSetDefaultSYNParam (iAP2PacketSYNData_t* param)
{
    if (param != NULL)
    {
        param->version               = kiAP2LinkSynDefaultVersion;
        param->maxOutstandingPackets = kiAP2LinkSynDefaultMaxOutstanding;
        param->maxPacketSize         = kiAP2LinkSynDefaultMaxPacketSize;
        param->retransmitTimeout     = kiAP2LinkSynDefaultRetransmitTimeout;
        param->cumAckTimeout         = kiAP2LinkSynDefaultCumAckTimeout;
        param->maxRetransmissions    = kiAP2LinkSynDefaultMaxRetransmit;
        param->maxCumAck             = kiAP2LinkSynDefaultMaxCumAck;
        param->numSessionInfo        = kiAP2LinkSynDefaultNumSessionInfo;
        param->peerMaxOutstandingPackets = kiAP2LinkSynDefaultMaxOutstanding;
        param->peerMaxPacketSize         = kiAP2LinkSynDefaultMaxPacketSize;
    }
}


 BOOL _IsSessionDuplicate (iAP2PacketSYNData_t *synData, iAP2PacketSessionInfo_t *session)
{
    uint32_t i;
    for (i = 0; i < synData->numSessionInfo; ++i)
    {
        if (synData->sessionInfo[i].id == session->id || synData->sessionInfo[i].type == session->type)
        {
            return TRUE;
        }
    }
    return FALSE;
}


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
BOOL iAP2LinkIsValidSynParam (iAP2PacketSYNData_t* synParam)
{
    BOOL result = FALSE;
    if (synParam != NULL)
    {
        result = (synParam->maxOutstandingPackets >= kiAP2LinkSynValMaxOutstandingMin &&
                  synParam->maxOutstandingPackets <= kiAP2LinkSynValMaxOutstandingMax &&
                  synParam->maxPacketSize >= kiAP2LinkSynValMaxPacketSizeMin &&
                  synParam->maxPacketSize <= kiAP2LinkSynValMaxPacketSizeMax &&
                  synParam->retransmitTimeout >= kiAP2LinkSynValRetransmitTimeoutMin &&
                  synParam->retransmitTimeout <= kiAP2LinkSynValRetransmitTimeoutMax &&
                  synParam->cumAckTimeout >= kiAP2LinkSynValCumAckTimeoutMin &&
                  synParam->cumAckTimeout <= kiAP2LinkSynValCumAckTimeoutMax &&
                  synParam->maxRetransmissions >= kiAP2LinkSynValMaxRetransmitMin &&
                  synParam->maxRetransmissions <= kiAP2LinkSynValMaxRetransmitMax &&
                  synParam->maxCumAck >= kiAP2LinkSynValMaxCumAckMin &&
                  synParam->maxCumAck <= kiAP2LinkSynValMaxCumAckMax);
        if (result)
        {
            BOOL    bControlSessionFound = FALSE;
            iAP2PacketSYNData_t tempParam;
            uint32_t i;

            for (i = 0; i < synParam->numSessionInfo; ++i)
            {
                /* Check for duplicate sessionIDs */
                tempParam.numSessionInfo = 0;
                if (_IsSessionDuplicate(&tempParam, &synParam->sessionInfo[i]))
                {
                    result = FALSE;
                    break;
                }
                tempParam.sessionInfo[tempParam.numSessionInfo].id      = synParam->sessionInfo[i].id;
                tempParam.sessionInfo[tempParam.numSessionInfo].type    = synParam->sessionInfo[i].type;
                tempParam.sessionInfo[tempParam.numSessionInfo].version = synParam->sessionInfo[i].version;
                ++tempParam.numSessionInfo;

                /* Session ID 0x0 is reserved and shall not be used for any session types */
                if ((synParam->sessionInfo[i].id == kIAP2PacketReservedSessionID) ||
                    (synParam->sessionInfo[i].type >= kIAP2PacketServiceTypeCount))
                {
                    result = FALSE;
                    break;
                }
                else if (synParam->sessionInfo[i].type == kIAP2PacketServiceTypeControl)
                {
                    /* Required control session type has been found */
                    bControlSessionFound = TRUE;
                }
            }

            if (FALSE == bControlSessionFound)
            {
                result = FALSE;     /* ERROR: Control session missing ! */
            }
        }

        if (!result)
        {
            int i;

            iAP2LogStart();
            iAP2LogErrorNL("Invalid SYN Params detected:\n");
            iAP2LogErrorNL("    maxOutstanding=%d maxPacketSize=%d\n",
                           synParam->maxOutstandingPackets, synParam->maxPacketSize);
            iAP2LogErrorNL("    retransmitTimeout=%d cumAckTimeout=%d\n",
                           synParam->retransmitTimeout, synParam->cumAckTimeout);
            iAP2LogErrorNL("    maxRetransmissions=%d maxCumAck=%d\n",
                           synParam->maxRetransmissions, synParam->maxCumAck);
            iAP2LogErrorNL("    numSessionInfo=%u\n",
                           synParam->numSessionInfo);
            for (i = 0; i < synParam->numSessionInfo; ++i)
            {
                iAP2LogErrorNL("    session %u [id=%u type=%u ver=%u]\n",
                               i,
                               synParam->sessionInfo[i].id,
                               synParam->sessionInfo[i].type,
                               synParam->sessionInfo[i].version);
            }
            iAP2LogStop();
        }
    }
    return result;
} /* iAP2LinkIsValidSynParam */


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
BOOL iAP2LinkValidateSynParam (iAP2PacketSYNData_t* synParam)
{
    iAP2PacketSYNData_t tempParam;
    BOOL bControlSessionFound;
    uint32_t i;
    BOOL result = FALSE;
    if (synParam != NULL)
    {
        result = TRUE;

        if (synParam->retransmitTimeout < kiAP2LinkSynValRetransmitTimeoutMin)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: retransmitTimeout=%d->%d\n",
                         synParam->retransmitTimeout, kiAP2LinkSynValRetransmitTimeoutMin);
            synParam->retransmitTimeout = kiAP2LinkSynValRetransmitTimeoutMin;
        }
        if (synParam->retransmitTimeout > kiAP2LinkSynValRetransmitTimeoutMax)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: retransmitTimeout=%d->%d\n",
                         synParam->retransmitTimeout, kiAP2LinkSynValRetransmitTimeoutMax);
            synParam->retransmitTimeout = kiAP2LinkSynValRetransmitTimeoutMax;
        }
        if (synParam->cumAckTimeout < kiAP2LinkSynValCumAckTimeoutMin)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: cumAckTimeout=%d->%d\n",
                         synParam->cumAckTimeout, kiAP2LinkSynValCumAckTimeoutMin);
            synParam->cumAckTimeout = kiAP2LinkSynValCumAckTimeoutMin;
        }
        if (synParam->cumAckTimeout > kiAP2LinkSynValCumAckTimeoutMax)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: cumAckTimeout=%d->%d\n",
                         synParam->cumAckTimeout, kiAP2LinkSynValCumAckTimeoutMax);
            synParam->cumAckTimeout = kiAP2LinkSynValCumAckTimeoutMax;
        }
        if (synParam->maxRetransmissions < kiAP2LinkSynValMaxRetransmitMin)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: maxRetransmissions=%d->%d\n",
                         synParam->maxRetransmissions, kiAP2LinkSynValMaxRetransmitMin);
            synParam->maxRetransmissions = kiAP2LinkSynValMaxRetransmitMin;
        }
        if (synParam->maxRetransmissions > kiAP2LinkSynValMaxRetransmitMax)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: maxRetransmissions=%d->%d\n",
                         synParam->maxRetransmissions, kiAP2LinkSynValMaxRetransmitMax);
            synParam->maxRetransmissions = kiAP2LinkSynValMaxRetransmitMax;
        }
        if (synParam->maxCumAck < kiAP2LinkSynValMaxCumAckMin)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: maxCumAck=%d->%d\n",
                         synParam->maxCumAck, kiAP2LinkSynValMaxCumAckMin);
            synParam->maxCumAck = kiAP2LinkSynValMaxCumAckMin;
        }
        if (synParam->maxCumAck > kiAP2LinkSynValMaxCumAckMax)
        {
            result = FALSE;
            iAP2LogError("Invalid SYN Params detected: maxCumAck=%d->%d\n",
                         synParam->maxCumAck, kiAP2LinkSynValMaxCumAckMax);
            synParam->maxCumAck = kiAP2LinkSynValMaxCumAckMax;
        }

        tempParam.numSessionInfo = 0;
        bControlSessionFound = FALSE;
        for (i = 0; i < synParam->numSessionInfo; ++i)
        {
            if (synParam->sessionInfo[i].type == kIAP2PacketServiceTypeControl) {
                bControlSessionFound = TRUE;
            }
            
            if ((synParam->sessionInfo[i].id != kIAP2PacketReservedSessionID) &&
                (synParam->sessionInfo[i].type < kIAP2PacketServiceTypeCount))
            {
                if (!_IsSessionDuplicate(&tempParam, &synParam->sessionInfo[i])) {
                    tempParam.sessionInfo[tempParam.numSessionInfo].id
                        = synParam->sessionInfo[i].id;
                    tempParam.sessionInfo[tempParam.numSessionInfo].type
                        = synParam->sessionInfo[i].type;
                    tempParam.sessionInfo[tempParam.numSessionInfo].version
                        = synParam->sessionInfo[i].version;
                    ++tempParam.numSessionInfo;
                } else {
                    iAP2LogError("Duplicate SYN Params detected: session %u [id=%u type=%u ver=%u]\n",
                                 i,
                                 synParam->sessionInfo[i].id,
                                 synParam->sessionInfo[i].type,
                                 synParam->sessionInfo[i].version);
                }
            }
            else
            {
                iAP2LogError("Invalid SYN Params detected: session %u [id=%u type=%u ver=%u]\n",
                             i,
                             synParam->sessionInfo[i].id,
                             synParam->sessionInfo[i].type,
                             synParam->sessionInfo[i].version);
                result = FALSE;
            }
        }
        
        /* Add control session if not present and max sessions not exceeded */
        if (!bControlSessionFound && tempParam.numSessionInfo < kIAP2PacketMaxSessions) {
            /* find unique session id for the control session */
            uint8_t sessionId = (tempParam.numSessionInfo > 0
                                 ? tempParam.sessionInfo[tempParam.numSessionInfo -1].id + 1
                                 : 1);
            BOOL uniqueIdFound = FALSE;
            while (!uniqueIdFound) {
                uint8_t j;
                for (j = 0; j < tempParam.numSessionInfo; ++j) {
                    if (tempParam.sessionInfo[j].id == sessionId) {
                        sessionId++;
                        break;
                    }
                }
                if (j == tempParam.numSessionInfo) {
                    uniqueIdFound = TRUE;
                }
            }
            
            tempParam.sessionInfo[tempParam.numSessionInfo].id = sessionId;
            tempParam.sessionInfo[tempParam.numSessionInfo].type = kIAP2PacketServiceTypeControl;
            tempParam.sessionInfo[tempParam.numSessionInfo].version = 1;
            ++tempParam.numSessionInfo;
        }
        
        /* Copy the sessionInfo from tempParam to synParam incase it changed due to duplicates or adding a control session */
        synParam->numSessionInfo = tempParam.numSessionInfo;
        memcpy (&(synParam->sessionInfo),
                &(tempParam.sessionInfo),
                sizeof(tempParam.sessionInfo));
    }
    return result;
} /* iAP2LinkValidateSynParam */


/*
****************************************************************
**
**  iAP2LinkGetBuffSize
**
**  Input:
**      maxPacketSentAtOnce Max number of packets that will be sent at once
**                              regarless of the maxOutstandingPackets value.
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of buff required for proper operation based
**
**
****************************************************************
*/
uint32_t iAP2LinkGetBuffSize (uint8_t maxPacketSentRcvdAtOnce)
{
    int i;
#if iAP2_LINK_ALLOW_MALLOC != 0
    uint32_t result = sizeof(iAP2Link_t) + iAP2_LINK_BUFFER_OFFSET;
#else
    uint32_t result = sizeof(iAP2Link_t);
#endif
    /* Assume max configuration supported for Device */
    /*
    ** TODO: this should be overridden by actual number of packets
    **       accessory is willing to send without ACK
    */
    uint8_t maxTimeouts = (maxPacketSentRcvdAtOnce +
                           2); /* +2, one for DETECT timeout and one for sendACK timeout */
    result += iAP2FSMGetBuffSize();
    result += iAP2TimeGetBuffSize(maxTimeouts);

    /* Assume use malloc */
    /*
     ** TODO: this should be overridden by actual number/size of packets/buffers
     **       that should be supported
     */
    result += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeBuff, 0, 0);
    result += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeSendPacket, 0, 0);
    result += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeRecvPacket, 0, 0);

    /* 3 lists, recv packet list, send packet list, and send buff list. */
    result += iAP2ListArrayGetBuffSize(maxPacketSentRcvdAtOnce, (uint8_t)sizeof(uintptr_t));
    result += iAP2ListArrayGetBuffSize(maxPacketSentRcvdAtOnce, (uint8_t)sizeof(uintptr_t));
    result += iAP2ListArrayGetBuffSize(maxPacketSentRcvdAtOnce, (uint8_t)sizeof(uintptr_t));

    /*
    ** list per session type
    ** This supports all session types but if the implementation does not
    ** need to support all session types, only the supported ones need have
    ** the sessSendPckList created.
    */
    for (i = 0; i < kIAP2PacketServiceTypeCount; ++i)
    {
        result += iAP2ListArrayGetBuffSize (kiAP2ListArrayMaxCount, (uint8_t)sizeof(uintptr_t));
    }

    /* TODO: iAP2List should use preallocated buffers as well. */

    return result;
}


/*
****************************************************************
**
**  iAP2LinkCreate
**  iAP2LinkCreateAccessory (type = kiAP2LinkTypeAccessory)
**  iAP2LinkCreateDevice (type = kiAP2LinkTypeDevice)
**
**  Input:
**      type:           Indicates whether this is for Accessory or Device.
**      context:            Context info to store with the link.
**      synParam:       SYN packet parameters
**      sendPacketCB:       Callback function to call when packet is ready to be sent.
**                              This callback will actually send the packet out.
**      recvDataCB:         Callback function to call when received data is ready.
**      connectedCB:        Callback function to call when link connection is UP/DOWN.
**      sendDetectCB:       Callback function to call to send detect byte sequence.
**      signalSendBuffCB:   Callback function to signal a call back into
**                              iAP2TansportProcessSendbuff() (asynchronously)
**                              to process outgoing data.
**      bValidateSYN        Flag indicating whether to check for valid SYN param.
**                              (for debugging/testing purpose)
**      maxPacketSentAtOnce Max number of packets that will be sent at once
**                              regarless of the maxOutstandingPackets value.
**      linkBuffer          Pre-allocated buffer to use for storing iAP2Link_t
**                              and other related structures.
**
**  Output:
**      Passed in linkBuffer is initiliazed.
**
**  Return:
**      iAP2Link_t*     pointer to new allocated iAP2Link_t structure
**                      or the passed in linkRLBuffer.
**
****************************************************************
*/
iAP2Link_t* iAP2LinkCreateAccessory (iAP2PacketSYNData_t*       synParam,
                                     void*                      context,
                                     iAP2LinkSendPacketCB_t     sendPacketCB,
                                     iAP2LinkDataReadyCB_t      recvDataCB,
                                     iAP2LinkConnectedCB_t      connectedCB,
                                     iAP2LinkSendDetectCB_t     sendDetectCB,
                                     iAP2LinkSignalSendBuffCB_t signalSendBuffCB,
                                     BOOL                       bValidateSYN,
                                     uint8_t                    maxPacketSentAtOnce,
                                     uint8_t*                   linkBuffer)
{
    return iAP2LinkCreate (kiAP2LinkTypeAccessory,
                           context,
                           synParam,
                           sendPacketCB,
                           NULL,
                           recvDataCB,
                           connectedCB,
                           sendDetectCB,
                           signalSendBuffCB,
                           bValidateSYN,
                           maxPacketSentAtOnce,
                           linkBuffer);
}


iAP2Link_t* iAP2LinkCreateDevice (iAP2PacketSYNData_t*       synParam,
                                  void*                      context,
                                  iAP2LinkSendPacketCB_t     sendPacketCB,
                                  iAP2LinkSendPacketWaitCB_t sendPacketWaitCB,
                                  iAP2LinkDataReadyCB_t      recvDataCB,
                                  iAP2LinkConnectedCB_t      connectedCB,
                                  iAP2LinkSendDetectCB_t     sendDetectCB,
                                  iAP2LinkSignalSendBuffCB_t signalSendBuffCB,
                                  BOOL                       bValidateSYN,
                                  uint8_t                    maxPacketSentAtOnce,
                                  uint8_t*                   linkBuffer)
{
    return iAP2LinkCreate (kiAP2LinkTypeDevice,
                           context,
                           synParam,
                           sendPacketCB,
                           sendPacketWaitCB,
                           recvDataCB,
                           connectedCB,
                           sendDetectCB,
                           signalSendBuffCB,
                           bValidateSYN,
                           maxPacketSentAtOnce,
                           linkBuffer);
}


iAP2Link_t* iAP2LinkCreate (iAP2LinkType_t              type,
                            void*                       context,
                            iAP2PacketSYNData_t*        synParam,
                            iAP2LinkSendPacketCB_t      sendPacketCB,
                            iAP2LinkSendPacketWaitCB_t  sendPacketWaitCB,
                            iAP2LinkDataReadyCB_t       recvDataCB,
                            iAP2LinkConnectedCB_t       connectedCB,
                            iAP2LinkSendDetectCB_t      sendDetectCB,
                            iAP2LinkSignalSendBuffCB_t  signalSendBuffCB,
                            BOOL                        bValidateSYN,
                            uint8_t                     maxPacketSentAtOnce,
                            uint8_t*                    linkBuffer)
{
    uint8_t     maxTimeouts = 0;
    int         i;
    iAP2Link_t* link;
    uint8_t*    linkBufferNext;
#if iAP2_LINK_ALLOW_MALLOC != 0
    if (NULL == linkBuffer)
    {
        uint8_t* buff = malloc (iAP2LinkGetBuffSize(maxPacketSentAtOnce));
        link             = (iAP2Link_t*) buff;
        linkBufferNext   = buff;
        link->linkBuffer = buff;
    }
    else
    {
        /*
        ** We add offset in this case so that link != link->linkBuffer, indicating
        ** that iAP2Link did not allocate the buffer for link layer.
        */
        link             = (iAP2Link_t*) (linkBuffer + iAP2_LINK_BUFFER_OFFSET);
        linkBufferNext   = linkBuffer + iAP2_LINK_BUFFER_OFFSET;
        link->linkBuffer = linkBuffer;
    }
#else
    assert (linkBuffer);
    link             = (iAP2Link_t*) (linkBuffer);
    linkBufferNext   = linkBuffer;
    link->linkBuffer = linkBuffer;
#endif
    linkBufferNext += sizeof(iAP2Link_t);
    assert(link);
    if (bValidateSYN && iAP2LinkIsValidSynParam(synParam) == FALSE)
    {
#if iAP2_LINK_ALLOW_MALLOC != 0
        if (NULL == linkBuffer)
        {
            free (link);
        }
#endif
        return NULL;
    }

    switch (type)
    {
        case kiAP2LinkTypeDevice:
#if iAP2_FOR_DEVICE
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d Create Device side link\n", __FILE__, __LINE__);
#endif
            /* Assume max configuration supported for Device */
            maxTimeouts = (maxPacketSentAtOnce +
                           2); /* +2, one for DETECT timeout and one for sendACK timeout */
            link->fsm = iAP2FSMCreate (kiAP2LinkStateCount,
                                       kiAP2LinkStateInit,
                                       kiAP2LinkEventCount,
                                       iAP2LinkDeviceStates,
                                       link,
                                       "DeviceFSM",
#if iAP2LINK_DEBUG
                                       stateNames,
                                       eventNames,
#else
                                       NULL,
                                       NULL,
#endif
                                       linkBufferNext);
            linkBufferNext += iAP2FSMGetBuffSize();
#endif /*#if iAP2_FOR_DEVICE*/
            break;
        case kiAP2LinkTypeAccessory:
#if iAP2_FOR_ACCESSORY
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d Create Accessory side link\n", __FILE__, __LINE__);
#endif
            /* Assume max configuration supported for Accessory for now */
            /* TODO: this should be overridden by actual number of packets
             *       accessory is willing to send without ACK
             */
            maxTimeouts = (maxPacketSentAtOnce +
                           2); /* +2, one for DETECT timeout and one for sendACK timeout */
            link->fsm = iAP2FSMCreate (kiAP2LinkStateCount,
                                       kiAP2LinkStateInit,
                                       kiAP2LinkEventCount,
                                       iAP2LinkAccessoryStates,
                                       link,
                                       "AccessoryFSM",
#if iAP2LINK_DEBUG
                                       stateNames,
                                       eventNames,
#else
                                       NULL,
                                       NULL,
#endif
                                       linkBufferNext);
            linkBufferNext += iAP2FSMGetBuffSize();
#endif /*#if iAP2_FOR_ACCESSORY*/
            break;
        default:
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d Invalid type(%d)!\n", __FILE__, __LINE__, type);
#endif
            abort();
            return NULL;
    }

    link->type             = type;
    link->context          = context;
    link->sendPacketCB     = sendPacketCB;
    link->sendPacketWaitCB = sendPacketWaitCB;
    link->recvDataCB       = recvDataCB;
    link->connectedCB      = connectedCB;
    link->sendDetectCB     = sendDetectCB;
    link->signalSendBuffCB = signalSendBuffCB;
    link->startSeq         = (uint8_t) rand();
    link->sentSeq          = 0;
    link->bValidSentSeq    = FALSE;
    link->recvSeq          = 0;
    link->bValidRecvSeq    = FALSE;
    link->sentAck          = 0;
    link->bValidSentAck    = FALSE;
    link->recvAck          = 0;
    link->bValidRecvAck    = FALSE;
    link->recvPck          = NULL;
    link->mainTimer        = iAP2TimeCreate (link,
                                             _iAP2LinkHandleTimerExpire,
                                             _iAP2LinkHandleTimerCancel,
                                             maxTimeouts,
                                             linkBufferNext);
    linkBufferNext += iAP2TimeGetBuffSize (maxTimeouts);

    link->detectAckTimeoutID = INVALID_TIMEOUT_ID;
    link->sendAckTimeoutID = INVALID_TIMEOUT_ID;
    link->bUseiAP2LinkRunLoop = FALSE;

    iAP2ListArrayInit (linkBufferNext, maxPacketSentAtOnce, (uint8_t)sizeof(uintptr_t));
    link->recvPckList = linkBufferNext;
    linkBufferNext += iAP2ListArrayGetBuffSize (maxPacketSentAtOnce, (uint8_t)sizeof(uintptr_t));

    iAP2ListArrayInit (linkBufferNext, maxPacketSentAtOnce, (uint8_t)sizeof(uintptr_t));
    link->sendPckList = linkBufferNext;
    linkBufferNext += iAP2ListArrayGetBuffSize (maxPacketSentAtOnce, (uint8_t)sizeof(uintptr_t));

    /*
    ** This supports all session types but if the implementation does not
    ** need to support all session types, only the supported ones need have
    ** the sessSendPckList created.
    */
    for (i = 0; i < kIAP2PacketServiceTypeCount; ++i)
    {
        iAP2ListArrayInit (linkBufferNext, kiAP2ListArrayMaxCount, (uint8_t)sizeof(uintptr_t));
        link->sessSendPckList[i] = linkBufferNext;
        linkBufferNext += iAP2ListArrayGetBuffSize (kiAP2ListArrayMaxCount, (uint8_t)sizeof(uintptr_t));
    }

    /* SYN params */
    link->initParam.version                = synParam->version;
    link->initParam.maxOutstandingPackets  = synParam->maxOutstandingPackets;
    link->initParam.maxPacketSize          = synParam->maxPacketSize;
    link->initParam.retransmitTimeout      = synParam->retransmitTimeout;
    link->initParam.cumAckTimeout          = synParam->cumAckTimeout;
    link->initParam.maxRetransmissions     = synParam->maxRetransmissions;
    link->initParam.maxCumAck              = synParam->maxCumAck;
    link->initParam.peerMaxOutstandingPackets  = synParam->maxOutstandingPackets;
    link->initParam.peerMaxPacketSize          = synParam->maxPacketSize;
    link->initParam.numSessionInfo         = synParam->numSessionInfo;
    memcpy (link->initParam.sessionInfo,
            synParam->sessionInfo,
            sizeof(synParam->sessionInfo[0]) * kIAP2PacketMaxSessions);

    memcpy (&(link->negotiatedParam),
            &(link->initParam),
            sizeof(link->initParam));

    iAP2LinkSetDefaultSYNParam (&(link->param));

    /* Must be called after link is initilaized */
    link->recvPckPool = iAP2BuffPoolInit (kiAP2BuffPoolTypeRecvPacket,
                                          (uintptr_t) link,
                                          0, 0, linkBufferNext);
    linkBufferNext += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeRecvPacket, 0, 0);
    link->sendPckPool = iAP2BuffPoolInit (kiAP2BuffPoolTypeSendPacket,
                                          (uintptr_t) link,
                                          0, 0, linkBufferNext);
    linkBufferNext += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeSendPacket, 0, 0);
    link->buffPool = iAP2BuffPoolInit (kiAP2BuffPoolTypeBuff,
                                       (uintptr_t) link,
                                       0, 0, linkBufferNext);
    linkBufferNext += iAP2BuffPoolGetBuffSize (kiAP2BuffPoolTypeBuff, 0, 0);

    link->linkBufferNext = linkBufferNext;

    /* Counters */
    link->numRecvSYN = 0;
    link->numRecvSYNACK = 0;
    link->numRecvCumSYN = 0;
    link->numRecvCumSYNACK = 0;
    link->numSentSYN = 0;
    link->numSentSYNACK = 0;
    link->numSentCumSYN = 0;
    link->numSentCumSYNACK = 0;
    link->numResentSYN = 0;
    link->numResentSYNACK = 0;
    link->numResentCumSYN = 0;
    link->numResentCumSYNACK = 0;
#if iAP2_LINK_ALLOW_STATS
    /* Stats */
    link->bytesSent = 0;
    link->bytesRcvd = 0;
    link->packetsSent = 0;
    link->packetsRcvd = 0;
    link->dataBytesSent = 0;
    link->dataBytesRcvd = 0;
    link->dataPacketsSent = 0;
    link->dataPacketsRcvd = 0;
    link->noAckReTxCount = 0;
    link->missingReTxCount = 0;
    link->invalidPackets = 0;
    link->numOutOfOrder = 0;
    link->numRecvEAK = 0;
    link->numRecvRST = 0;
    link->numRecvACK = 0;
    link->numRecvDATA = 0;
    link->numSentEAK = 0;
    link->numSentRST = 0;
    link->numSentACK = 0;
    link->numSentDATA = 0;
    link->totACKDelays = 0;
    link->numACKDelays = 0;
    link->numResentACK = 0;
    link->numResentDATA = 0;
#endif /*#if iAP2_LINK_ALLOW_STATS*/

    link->bIgnoreSynRetryLimit = FALSE;

    if (NULL == sendPacketCB)
    {
        iAP2LogError("%s:%d NULL sendPacketCB!\n", __FILE__, __LINE__);
    }

    return link;
}


/*
****************************************************************
**
**  iAP2LinkResetSend
**
**  Input:
**      link:  link structure to reset
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Clears the send packet lists
**
****************************************************************
*/
void iAP2LinkResetSend (iAP2Link_t* link)
{
    if (link != NULL)
    {
        uint8_t i;
        if (link->sendPckList != NULL)
        {
            iAP2ListArrayCleanup (link->sendPckList, _DeletePckCB);
        }
        for (i = 0; i < kIAP2PacketServiceTypeCount; ++i)
        {
            if (link->sessSendPckList[i] != NULL)
            {
                iAP2ListArrayCleanup (link->sessSendPckList[i], _DeletePckCB);
            }
        }
        iAP2LinkSetDefaultSYNParam (&(link->param));
        iAP2LinkResetSeqAck (link, TRUE);
    }
    else
    {
        iAP2LogError("%s:%d NULL link!\n", __FILE__, __LINE__);
    }
}


/*
****************************************************************
**
**  iAP2LinkDelete
**
**  Input:
**      link:  link structure to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkDelete (iAP2Link_t* link)
{
    if (link != NULL)
    {
        if (link->fsm != NULL)
        {
            iAP2FSMDelete(link->fsm);
            link->fsm = NULL;
        }
        if (link->recvPck != NULL)
        {
            iAP2PacketDelete(link->recvPck);
            link->recvPck = NULL;
        }
        if (link->recvPckList != NULL)
        {
            iAP2ListArrayCleanup (link->recvPckList, _DeletePckCB);
            link->recvPckList = NULL;
        }

        /* Reset send packet lists */
        iAP2LinkResetSend (link);

        if (link->mainTimer != NULL)
        {
            iAP2TimeDelete (link->mainTimer);
            link->mainTimer = NULL;
        }

        iAP2BuffPoolCleanup (link->recvPckPool);
        link->recvPckPool = NULL;
        iAP2BuffPoolCleanup (link->sendPckPool);
        link->sendPckPool = NULL;
        iAP2BuffPoolCleanup (link->buffPool);
        link->buffPool = NULL;

#if iAP2_LINK_ALLOW_MALLOC != 0
        if ((uint8_t*)link == link->linkBuffer)
        {
            free ((uint8_t*)link);
        }
#endif
    }
    else
    {
        iAP2LogError("%s:%d NULL link!\n", __FILE__, __LINE__);
    }
}


/*
****************************************************************
**
**  iAP2LinkGetMaxPayloadSize
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      uint8_t max payload size.
**
**  Note: OBSOLETE: equivalent to iAP2LinkGetMaxSendPayloadSize
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxPayloadSize (iAP2Link_t* link)
{
    return iAP2LinkGetMaxSendPayloadSize (link);
}


/*
****************************************************************
**
**  iAP2LinkGetMaxSendPayloadSize
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      uint8_t max payload size.
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxSendPayloadSize (iAP2Link_t* link)
{
    if (link)
    {
        return (iAP2LinkGetMaxSendPacketSize (link)
                - kIAP2PacketHeaderLen
                - kIAP2PacketChksumLen);
    }
    return 0;
}


/*
****************************************************************
**
**  iAP2LinkGetMaxRecvPayloadSize
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      uint32_t max receive payload size.
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxRecvPayloadSize (iAP2Link_t* link)
{
    if (link)
    {
        return (iAP2LinkGetMaxRecvPacketSize (link)
                - kIAP2PacketHeaderLen
                - kIAP2PacketChksumLen);
    }
    return 0;
}


/*
 ****************************************************************
 **
 **  iAP2LinkGetMaxSendPacketSize
 **
 **  Input:
 **      link:  link structure
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint8_t max send packet size.
 **
 ****************************************************************
 */
uint32_t iAP2LinkGetMaxSendPacketSize (iAP2Link_t* link)
{
    if (link)
    {
        return link->param.peerMaxPacketSize;
    }
    return 0;
}


/*
 ****************************************************************
 **
 **  iAP2LinkGetMaxRecvPacketSize
 **
 **  Input:
 **      link:  link structure
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint32_t max receive packet size.
 **
 ****************************************************************
 */
uint32_t iAP2LinkGetMaxRecvPacketSize (iAP2Link_t* link)
{
    if (link)
    {
        uint32_t maxPacketSize = link->initParam.maxPacketSize;
        if (maxPacketSize < kiAP2LinkSynDefaultMaxPacketSize)
        {
            /*
             * We need to allocate at least default max packet size so that
             * link negotiation can occur correctly.
             */
            maxPacketSize = kiAP2LinkSynDefaultMaxPacketSize;
        }
        return maxPacketSize;
    }
    return 0;
}


/*
****************************************************************
**
**  iAP2LinkStart
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Start the link FSM.  Should be done after all init is finished.
**
****************************************************************
*/
void iAP2LinkStart (iAP2Link_t* link)
{
    iAP2FSMHandleEvent(link->fsm, kiAP2LinkEventInitDone);
}


/*
****************************************************************
**
**  iAP2LinkProcessSendBuff
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if processed else FALSE
**
**  Note: process the send buffer.
**
****************************************************************
*/
BOOL iAP2LinkProcessSendBuff (iAP2Link_t* link)
{
    BOOL result = FALSE;
    if (link)
    {
        if (__AnySessionSendListHasPacket(link))
        {
            if (iAP2LinkSendWindowAvailable(link))
            {
                iAP2FSMHandleEvent(link->fsm, kiAP2LinkEventDataToSend);
                result = TRUE;
            }
            /* else reached window limit */
#if iAP2LINK_DEBUG
            else
            {
                uint8_t pckSent = (link->bValidRecvAck && link->bValidSentSeq
                                   ? iAP2PacketCalcSeqGap (link->recvAck,
                                                           link->sentSeq)
                                   : 0);
                iAP2LogDbg("%s:%d ProcessSendBuff waiting for ACK to open window recvAck=%d sentSeq=%d (used %u/%u)\n",
                           __FILE__, __LINE__,
                           link->recvAck, link->sentSeq,
                           pckSent, link->param.peerMaxOutstandingPackets);
            }
#endif
        }
    }
    return result;
}


/*
****************************************************************
**
**  iAP2LinkAttached
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Notify link that the link has been established.
**
****************************************************************
*/
void iAP2LinkAttached (iAP2Link_t* link)
{
    if (link != NULL)
    {
        link->startSeq = (uint8_t) rand();

        iAP2LinkSetDefaultSYNParam (&(link->param));

        iAP2FSMHandleEvent(link->fsm, kiAP2LinkEventAttach);
    }
}


/*
****************************************************************
**
**  iAP2LinkDetached
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Notify link that the link has been torn down.
**
****************************************************************
*/
void iAP2LinkDetached (iAP2Link_t* link)
{
    if (link != NULL)
    {
        iAP2FSMHandleEvent(link->fsm, kiAP2LinkEventDetach);

        /* Set to default */
        iAP2LinkSetDefaultSYNParam (&(link->param));

        link->startSeq = (uint8_t) rand();
    }
}


/*
****************************************************************
**
**  iAP2LinkIsDetached
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if link is in detached state, else FALSE
**
****************************************************************
*/
BOOL iAP2LinkIsDetached (iAP2Link_t* link)
{
    if (link && link->fsm &&
        link->fsm->currentState > kiAP2LinkStateDetached)
    {
        return FALSE;
    }
    return TRUE;
}


/*
****************************************************************
**
**  iAP2LinkProcessOutQueue
**
**  Input:
**      link:  link structure
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
void iAP2LinkProcessOutQueue (iAP2Link_t* link)
{
    BOOL bContinue;
    int  count;
    assert(link);

#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d ProcessOutQueue count:ctl=%u file=%u ea=%u\n",
               __FILE__, __LINE__,
               iAP2ListArrayGetCount (link->sessSendPckList[kIAP2PacketServiceTypeControl]),
               iAP2ListArrayGetCount (link->sessSendPckList[kIAP2PacketServiceTypeBuffer]),
               iAP2ListArrayGetCount (link->sessSendPckList[kIAP2PacketServiceTypeEA]));
#endif
    bContinue = TRUE;
    count = 5; /* Just do few iterations per call to prevent outgoing data from overloading link layer processing. */
    while (bContinue && count-- > 0 && iAP2LinkSendWindowAvailable(link))
    {
        int i;

        bContinue = FALSE;

        for (i = 0; i < kIAP2PacketServiceTypeCount && iAP2LinkSendWindowAvailable(link); ++i)
        {
            uint8_t* pckList = link->sessSendPckList[i];
            if (iAP2ListArrayGetCount (pckList) > 0)
            {
                uint8_t item = iAP2ListArrayGetFirstItemIndex (pckList);
                iAP2Packet_t* pck = iAP2LinkPacketForIndex (pckList, item);
                if (pck)
                {
                    iAP2ListArrayDeleteItem(pckList, item, NULL);
#if iAP2_LINK_ALLOW_STATS
                    link->dataBytesSent += iAP2PacketGetPayloadLen (pck);
                    ++(link->dataPacketsSent);
#endif
                    pck->pckData->seq = link->sentSeq + 1;
                    pck->pckData->ack = link->recvSeq;
                    iAP2LinkSendPacket (link, pck, FALSE,
                                        (link->type == kiAP2LinkTypeAccessory
                                         ? "Accessory:SendData"
                                         : "Device:SendData"));
                    if (pck->callbackOnSend != NULL)
                    {
                        iAP2LinkDataSentCB_t callback = (iAP2LinkDataSentCB_t) pck->callbackOnSend;
                        callback (link, pck->cbContext);
                    }
                }
                if (iAP2ListArrayGetCount (pckList) > 0)
                {
                    bContinue = TRUE;
                }
            }
        }
    }
    if (bContinue)
    {
        link->signalSendBuffCB (link);
    }
}


/*
****************************************************************
**
**  iAP2LinkQueueSendData
**
**  Input:
**      link:       link structure
**      payload:    payload data buffer to send
**      payloadLen: size of the payload data buffer
**      session:    sessionID
**      context:    context to use when calling callback on data buffer send
**      callback:   callback to call when data buffer has been sent
**
**  Output:
**      link:       link structure is updated to reflect state after
**                      processing payload data.
**
**  Return:
**      TRUE        if successfullly queued up data for send
**      FALSE       if could not queue up data for send
**
**  Note: Queue data for sending. Must cause a DataToSend event to be generated
**          for the data to actually get sent out.
**
****************************************************************
*/
BOOL iAP2LinkQueueSendData (iAP2Link_t*            link,
                            const uint8_t*         payload,
                            uint32_t               payloadLen,
                            uint8_t                session,
                            void*                  context,
                            iAP2LinkDataSentCB_t   callback)
{
    BOOL result = FALSE;
#if iAP2LINK_DEBUG
    iAP2LogStart();
    iAP2LogPrintData (payload, payloadLen, "QueueSendData",
                      "%s:%d payload=%p payloadLen=%u session=%u context=%p callback=%p\n",
                      __FILE__, __LINE__, payload, payloadLen, session, context, callback);
    iAP2LogStop();
#endif
    if (link != NULL && payload != NULL && payloadLen > 0)
    {
        iAP2PacketSessionInfo_t* pSession = iAP2LinkGetSessionInfo (link, session);
        if (pSession)
        {
            /*
             * Put payload data into packets and keep the packets in a list
             * per service type.
             * Segment data across multiple packets as needed.
             * TODO: if there is a packet in the queue already and it is not
             *       full, fill up the packet with some of the passed in data
             *       to fill the packet to max payload size.
             */
            const uint8_t*  data = payload;
            uint32_t        doneLen = 0;
            uint32_t        dataLen = iAP2LinkGetMaxSendPayloadSize (link);
            uint8_t*        sessSendPckList = link->sessSendPckList[pSession->type];

            if (dataLen > payloadLen)
            {
                dataLen = payloadLen;
            }
            result = TRUE; /* assume success */
			g_rec_mutex_lock( loop_mutex_iAP2 );
            if (sessSendPckList)
            {
                BOOL last = FALSE;
                while (result && doneLen < payloadLen)
                {
                    iAP2Packet_t* packet;
                    if (dataLen > (payloadLen - doneLen))
                    {
                        dataLen = (payloadLen - doneLen);
                    }
                    last = ((payloadLen - doneLen) == dataLen);
                    packet = iAP2PacketCreateACKPacket (link,
                                                        link->sentSeq,
                                                        link->sentAck,
                                                        data,
                                                        dataLen,
                                                        session);
                    if (packet)
                    {
                        if (last)
                        {
                            /* For last packet, save callback info to call when packet is sent out */
                            packet->cbContext      = context;
                            packet->callbackOnSend = (void*) callback;
                        }
                        else
                        {
                            packet->cbContext      = NULL;
                            packet->callbackOnSend = NULL;
                        }
                        iAP2LinkAddPacketAfter (sessSendPckList,
                                                iAP2ListArrayGetLastItemIndex (sessSendPckList),
                                                &packet);
                        data += dataLen;
                        doneLen += dataLen;
#if iAP2LINK_DEBUG
                        iAP2LogDbg ("%s:%d QueueSendData listCount=%u payload=%p payloadLen=%u data=%p dataLen=%u session=%u\n",
                                    __FILE__, __LINE__, iAP2ListArrayGetCount (sessSendPckList),
                                    payload, payloadLen, data, dataLen, session);
#endif
                    }
                    else
                    {
                        /* Ran out of send packets! */
                        iAP2LogError("%s:%d QueueSendData Ran out of Send Packets! listCount=%u payload=%p payloadLen=%u data=%p dataLen=%u session=%u\n",
                                     __FILE__, __LINE__, iAP2ListArrayGetCount (sessSendPckList),
                                     payload, payloadLen, data, dataLen, session);
                        result = FALSE;
                    }
                }
            }
			g_rec_mutex_unlock(loop_mutex_iAP2);
            /* Signal for the DataToSend event to be genereated (call iAP2LinkProcessSendBuff at a later point) */
            link->signalSendBuffCB (link);
        }
        else
        {
            iAP2LogError("%s:%d Invalid session(%u)!\n", __FILE__, __LINE__, session);
        }
    }
    else
    {
        iAP2LogError("%s:%d NULL link(%p) or payload(%p) or no payload (len=%u)!\n",
                     __FILE__, __LINE__, link, payload, payloadLen);
    }
    return result;
}


/*
 ****************************************************************
 **
 **  iAP2LinkQueueSendDataPacket
 **
 **  Input:
 **      link:       link structure
 **      packet:     data packet to send (seq and ack are modified at time of actual send)
 **      session:    sessionID
 **      context:    context to use when calling callback on data buffer send
 **      callback:   callback to call when data buffer has been sent
 **
 **  Output:
 **      link:  link structure is updated to reflect state after
 **                      processing packet.
 **
 **  Return:
 **      TRUE        if successfullly queued up data for send
 **      FALSE       if could not queue up data for send
 **
 **  Note: Queue packet for sending. Must cause a DataToSend event to be generated
 **          for the data to actually get sent out.
 **        Packet belongs to the link layer after call... do not delete.
 **
 ****************************************************************
 */
BOOL iAP2LinkQueueSendDataPacket (iAP2Link_t*           link,
                                  iAP2Packet_t*         packet,
                                  uint8_t               session,
                                  void*                 context,
                                  iAP2LinkDataSentCB_t  callback)
{
    BOOL result = FALSE;
#if iAP2LINK_DEBUG
    iAP2LogStart();
    iAP2LogDbg ("%s:%d packet=%p session=%u context=%p callback=%p\n",
                __FILE__, __LINE__, packet, session, context, callback);
    iAP2PacketDebugPrintPacketNL (packet, __FILE__, __LINE__, "QueueSendPacket", "");
    iAP2LogStop();
#endif
    if (link != NULL && iAP2PacketIsDataPacket (packet))
    {
        iAP2PacketSessionInfo_t* pSession = iAP2LinkGetSessionInfo (link, session);
        if (pSession)
        {
            /*
             * Put packet in a list per service type.
             */
            uint8_t* sessSendPckList = link->sessSendPckList[pSession->type];
            g_rec_mutex_lock( loop_mutex_iAP2 );
            if (sessSendPckList)
            {
                packet->pckData->sess    = session;
                packet->cbContext       = context;
                packet->callbackOnSend  = (void*) callback;
                if (iAP2LinkAddPacketAfter (sessSendPckList,
                                            iAP2ListArrayGetLastItemIndex (sessSendPckList),
                                            &packet) != kiAP2ListArrayInvalidIndex)
                {
                    result = TRUE;
#if iAP2LINK_DEBUG
                    iAP2LogDbg ("%s:%d QueueSendData listCount=%u packet=%p packetLen=%u session=%u\n",
                                __FILE__, __LINE__, iAP2ListArrayGetCount (sessSendPckList),
                                packet, packet->packetLen, session);
#endif
                }
                else
                {
                    iAP2LogError("%s:%d Could not queue packet to session send list! listCount=%u packet=%p\n",
                                 __FILE__, __LINE__, iAP2ListArrayGetCount (sessSendPckList), packet);
                }
            }
			g_rec_mutex_unlock(loop_mutex_iAP2);
            /* Signal for the DataToSend event to be genereated (call iAP2LinkProcessSendBuff at a later point) */
            link->signalSendBuffCB (link);
        }
        else
        {
            iAP2LogError("%s:%d Invalid session(%u)!\n", __FILE__, __LINE__, session);
        }
    }
    else
    {
        iAP2LogError("%s:%d NULL link(%p) or invalid packet (%p)!\n",
                     __FILE__, __LINE__, link, packet);
    }
    return result;
}


/*
****************************************************************
**
**  iAP2LinkSendWindowAvailable
**
**  Input:
**      link:  link structure
**
**  Output:
**      None
**
**  Return:
**      BOOL    returns TRUE if window available, else FALSE
**
****************************************************************
*/
BOOL iAP2LinkSendWindowAvailable (iAP2Link_t* link)
{
    BOOL result = FALSE;
    if (!link->bValidSentSeq || !link->bValidRecvAck ||
        iAP2PacketCalcSeqGap (link->recvAck,
                              link->sentSeq) < link->param.peerMaxOutstandingPackets)
    {
        result = TRUE;
    }
    return result;
}


/*
****************************************************************
**
**  iAP2LinkHandleReadyPacket
**
**  Input:
**      link:       link structure
**      packet:     packet to handle
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Process a parsed packet and generate appropriate event.
**
****************************************************************
*/
void iAP2LinkHandleReadyPacket (struct iAP2Link_st* link,
                                iAP2Packet_t*       packet)
{
    BOOL    bSameAsLastReceivedPacket;
    uint8_t seqDiff;
    BOOL bInvalidPacket = FALSE;
    BOOL bProcessPacket = FALSE;
    BOOL bDeletePacket = TRUE;
    assert(packet);

    if (packet->state == kiAP2PacketParseStateDETECT)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: DETECT\n", __FILE__, __LINE__);
#endif
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvDetect);
        iAP2PacketDelete (packet);
        return;
    }
    else if (packet->state == kiAP2PacketParseStateDETECTBAD)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: DETECT BAD\n", __FILE__, __LINE__);
#endif
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvDetectBad);
        iAP2PacketDelete (packet);
        return;
    }

    if (packet->packetLen > link->param.maxPacketSize) {
        iAP2LogErrorNL("Packet 0x%x has length = %d bytes > maxPacketLength (%d bytes)",
                       packet->pckData->seq, packet->packetLen, link->param.maxPacketSize);
        iAP2PacketDelete (packet);
        return;
    }

#if iAP2_LINK_ALLOW_STATS
    ++(link->packetsRcvd);
#endif

    /* TODO: cleanup previous recvPck, if any */
    link->recvPck = packet;
    bSameAsLastReceivedPacket = (link->recvSeq == packet->pckData->seq);
    seqDiff = iAP2PacketCalcSeqGap (link->recvSeq,
                                    packet->pckData->seq);
#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d PacketReadyHandler: %s packet, seqDiff=%u recvSeq=%u seq=%u bSameAsLastReceivedPacket=%d ack=%u control=%xh len=%u\n",
               __FILE__, __LINE__, iAP2PacketName(packet), seqDiff, link->recvSeq,
               packet->pckData->seq, bSameAsLastReceivedPacket, packet->pckData->ack,
               packet->pckData->ctl, packet->packetLen);
#endif

    if ((packet->pckData->ctl & kIAP2PacketControlMaskRST) != 0)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: RST, seqDiff=%u bValidRecvSeq=%d recvSeq=%u control=%xh seq=%u\n",
                   __FILE__, __LINE__, seqDiff, link->bValidRecvSeq, link->recvSeq, packet->pckData->ctl, packet->pckData->seq);
#endif
        bProcessPacket = TRUE;
    }
    else if ((packet->pckData->ctl & kIAP2PacketControlMaskSUS) != 0)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: SUS, seqDiff=%u bValidRecvSeq=%d recvSeq=%u control=%xh seq=%u\n",
                   __FILE__, __LINE__, seqDiff, link->bValidRecvSeq, link->recvSeq, packet->pckData->ctl, packet->pckData->seq);
#endif
        bProcessPacket = TRUE;
    }
    else if ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) != 0 ||
             !link->bValidRecvSeq ||
             seqDiff == 1)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: next packet or SYN, seqDiff=%u bValidRecvSeq=%d recvSeq=%u control=%xh seq=%u\n",
                   __FILE__, __LINE__, seqDiff, link->bValidRecvSeq, link->recvSeq, packet->pckData->ctl, packet->pckData->seq);
#endif
        /* Save data in list and handle in _CleanupRecvPackets */
        iAP2LinkAddPacketAfter (link->recvPckList,
                                iAP2ListArrayGetLastItemIndex (link->recvPckList),
                                &packet);
        bDeletePacket = FALSE;
        if ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) != 0 &&
            (packet->pckData->ctl & kIAP2PacketControlMaskACK) == 0)
        {
            /* Process SYN only packet right away... this is essentially a reset from accessory */
            bProcessPacket = TRUE;
        }
    }
    else if (seqDiff >= 0 && seqDiff <= link->param.maxOutstandingPackets &&
             (iAP2PacketIsEAK (packet) || iAP2PacketIsACKOnly (packet)))
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: EAK or ACK packet, seqDiff=%u recvSeq=%u(%x) seq=%u(%x) control=%u(%x) len=%u\n",
                   __FILE__, __LINE__, seqDiff, link->recvSeq, link->recvSeq,
                   packet->pckData->seq, packet->pckData->seq, packet->pckData->ctl,  packet->pckData->ctl, packet->packetLen);
#endif
        bProcessPacket = TRUE;
    }
    else if (seqDiff > 0 && seqDiff <= link->param.maxOutstandingPackets)
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: OUT OF ORDER packet, seqDiff=%u recvSeq=%u(%x) seq=%u(%x) control=%u(%x) len=%u\n",
                   __FILE__, __LINE__, seqDiff, link->recvSeq, link->recvSeq,
                   packet->pckData->seq, packet->pckData->seq, packet->pckData->ctl,  packet->pckData->ctl, packet->packetLen);
#endif
        if (iAP2PacketRequireACK(packet))
        {
            /* return EAK if we haven't received this one yet. */
            if (!_IsReceivedPacket(link, packet))
            {
#if iAP2_LINK_ALLOW_STATS
                ++(link->numOutOfOrder);
#endif

                /* Save data in list until recv'd in sequence */
                iAP2LinkAddPacketAfter (link->recvPckList,
                                        iAP2ListArrayGetLastItemIndex (link->recvPckList),
                                        &packet);
                bDeletePacket = FALSE;

                /*
                ** Start ACK timer
                ** on ACK expiry. check if need to send EAK instead of ACK.
                */
                _iAP2LinkStartSendAckTimerIfNotRunning (link, packet->pckData->seq);
            }
        }
        else if (iAP2PacketIsACKOnly (packet))
        {
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d PacketReadyHandler: ACK only packet, seqDiff=%u recvSeq=%u(%x) seq=%u(%x) control=%u(%x) len=%u\n",
                       __FILE__, __LINE__, seqDiff, link->recvSeq, link->recvSeq,
                       packet->pckData->seq, packet->pckData->seq, packet->pckData->ctl,  packet->pckData->ctl, packet->packetLen);
#endif
            bProcessPacket = TRUE;
        }
    }
    else if (bSameAsLastReceivedPacket &&
             !iAP2PacketIsACKOnly(packet))
    {
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvLastData);
    }
    else
    {
#if iAP2LINK_DEBUG
        iAP2LogDbg("%s:%d PacketReadyHandler: INVALID packet, seqDiff=%u recvSeq=%u(%x) seq=%u(%x) control=%u(%x) len=%u\n",
                   __FILE__, __LINE__, seqDiff, link->recvSeq, link->recvSeq,
                   packet->pckData->seq, packet->pckData->seq, packet->pckData->ctl,  packet->pckData->ctl, packet->packetLen);
#endif
        bInvalidPacket = TRUE;
    }

    if (!bInvalidPacket)
    {
        if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
        {
            seqDiff = iAP2PacketCalcSeqGap (link->recvAck,
                                            packet->pckData->ack);
            if (!link->bValidRecvAck || seqDiff <= kiAP2LinkSynValMaxOutstandingMax)
            {
                link->recvAck = packet->pckData->ack;
                link->bValidRecvAck = TRUE;
                _CleanupAckedPackets (link);
            }
#if iAP2LINK_DEBUG
            else
            {
                iAP2LogDbg("%s:%d PacketReadyHandler: Old ACK, bValidRecvAck=%d seqDiff=%u recvAck=%u(%x) seq=%u(%x) ack=%u(%x) control=%u(%x) len=%u\n",
                           __FILE__, __LINE__, link->bValidRecvAck, seqDiff, link->recvAck, link->recvAck,
                           packet->pckData->seq, packet->pckData->seq, packet->pckData->ack, packet->pckData->ack,
                           packet->pckData->ctl,  packet->pckData->ctl, packet->packetLen);
            }
#endif
        }

        if (bProcessPacket)
        {
            bInvalidPacket = (iAP2LinkProcessInOrderPacket (link, packet) == FALSE);
        }
        _CleanupRecvPackets (link);
    }

    if (bInvalidPacket)
    {
#if iAP2_LINK_ALLOW_STATS
        ++(link->invalidPackets);
#endif
    }

    if (packet && bDeletePacket)
    {
        iAP2PacketDelete (packet);
    }

    link->recvPck = NULL;
}


/*
****************************************************************
**
**  iAP2LinkHandleSuspend
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
****************************************************************
*/
void iAP2LinkHandleSuspend (struct iAP2Link_st* link)
{
    iAP2LogDbg("%s:%d suspend link=%p(type=%d state=%d)",
               __FUNCTION__, __LINE__, link, link->type, link->fsm);
    if (link && link->fsm && (link->type == kiAP2LinkTypeDevice))
    {
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventSuspend);
    }
}


 BOOL _iAP2LinkProcessInOrderPacketSYN (struct iAP2Link_st* link,
                                              iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;

    link->recvSeq = packet->pckData->seq;
    link->bValidRecvSeq = TRUE;
    
    /* SYN or SYN+ACK */
    if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
    {
        /* SYN+ACK Packet */
        ++(link->numRecvSYNACK);
        ++(link->numRecvCumSYNACK);
    }
    else
    {
        /* SYN Packet */
        ++(link->numRecvSYN);
        ++(link->numRecvCumSYN);
    }
    if (!link->bIgnoreSynRetryLimit &&
        (link->numRecvSYNACK + link->numRecvSYN) > kiAP2LinkSynRetries)
    {
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventMaxResend);
        bInvalidPacket = TRUE;
    }
    else if (packet->packetLen >= (kIAP2PacketHeaderLen
                                   + kIAP2PacketSynDataBaseLen
                                   + kIAP2PacketChksumLen))
    {
        BOOL validSYN;
        iAP2PacketSYNData_t synParam;

        iAP2PacketParseSYNData (iAP2PacketGetPayload (packet),
                                iAP2PacketGetPayloadLen (packet),
                                &synParam);
        validSYN = iAP2LinkIsValidSynParam (&synParam);

        if (!validSYN || (packet->pckData->ctl & kIAP2PacketControlMaskACK) != 0)
        {
            if (validSYN &&
                synParam.retransmitTimeout  == link->negotiatedParam.retransmitTimeout  &&
                synParam.cumAckTimeout      == link->negotiatedParam.cumAckTimeout      &&
                synParam.maxRetransmissions == link->negotiatedParam.maxRetransmissions &&
                synParam.maxCumAck          == link->negotiatedParam.maxCumAck          &&
                synParam.numSessionInfo     == link->negotiatedParam.numSessionInfo)
            {
                /* negotiable values are same */
                if (packet->pckData->ack == link->sentSeq)
                {
                    iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvSYNACK);
                }
                else
                {
                    iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvSYNACKOLD);
                }
            }
            else
            {
                /* negotiable values are not same or invalid syn param */
                iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvSYNACKNEW);
            }
        }
        else
        {
            /* SYN Packet */
            iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvSYN);
        }
    }
    else
    {
#if iAP2LINK_DEBUG
        iAP2LogStart();
        iAP2LogDbg("%s:%d %s: Invalid packetLen!, packetLen=%u (>= %u)\n",
                   __FILE__, __LINE__,  __FUNCTION__,
                   packet->packetLen, (kIAP2PacketHeaderLen
                                       + kIAP2PacketSynDataBaseLen
                                       + kIAP2PacketChksumLen));
        iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, __FUNCTION__, "");
        iAP2LogStop();
#endif
        bInvalidPacket = TRUE;
    }

    return (bInvalidPacket == FALSE);
}


 BOOL _iAP2LinkProcessInOrderPacketEAK (struct iAP2Link_st* link,
                                              iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;

    if (packet->packetLen > (kIAP2PacketHeaderLen + kIAP2PacketChksumLen))
    {
        /* EAK Packet with OutOfSeq data */
#if iAP2_LINK_ALLOW_STATS
        ++(link->numRecvEAK);
#endif
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventEAK);
    }
    else
    {
#if iAP2LINK_DEBUG
        iAP2LogStart();
        iAP2LogDbg("%s:%d %s: Invalid packetLen!, packetLen=%u (> %u)\n",
                   __FILE__, __LINE__, __FUNCTION__,
                   packet->packetLen, (kIAP2PacketHeaderLen + kIAP2PacketChksumLen));
        iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, __FUNCTION__, "");
        iAP2LogStop();
#endif
        bInvalidPacket = TRUE;
    }

    return (bInvalidPacket == FALSE);
}


 BOOL _iAP2LinkProcessInOrderPacketRST (struct iAP2Link_st* link,
                                              iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;

    if (packet->packetLen == kIAP2PacketHeaderLen)
    {
        /* RST Packet */
#if iAP2_LINK_ALLOW_STATS
        ++(link->numRecvRST);
#endif
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvRST);
    }
    else
    {
#if iAP2LINK_DEBUG
        iAP2LogStart();
        iAP2LogDbg("%s:%d %s: Invalid packetLen!, packetLen=%u (= %u)\n",
                   __FILE__, __LINE__, __FUNCTION__,
                   packet->packetLen, kIAP2PacketHeaderLen);
        iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, __FUNCTION__, "");
        iAP2LogStop();
#endif
        bInvalidPacket = TRUE;
    }

    return (bInvalidPacket == FALSE);
}


 BOOL _iAP2LinkProcessInOrderPacketACK (struct iAP2Link_st* link,
                                              iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;

    if (packet->packetLen > (kIAP2PacketHeaderLen + kIAP2PacketChksumLen))
    {
        uint8_t gap;

        /* Data Packet */
        link->recvSeq = packet->pckData->seq;
        link->bValidRecvSeq = TRUE;

#if iAP2_LINK_ALLOW_STATS
        ++(link->numRecvDATA);
#endif
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvData);

        gap = iAP2PacketCalcSeqGap(link->sentAck, link->recvSeq);
        if (gap > 0 && gap < kiAP2LinkSynValMaxOutstandingMax &&
            link->fsm->currentState >= kiAP2LinkStateConnected)
        {
            if (gap >= link->param.maxCumAck)
            {
                iAP2Packet_t* pck;

                pck = iAP2PacketCreateACKPacket (link,
                                                 link->sentSeq,
                                                 link->recvSeq,
                                                 NULL,
                                                 0,
                                                 kIAP2PacketReservedSessionID);
#if iAP2LINK_DEBUG
                iAP2LogDbg ("%s:%d %s:%s recvSeq=%u sentAck=%u\n",
                            __FILE__, __LINE__,
                            (link->type == kiAP2LinkTypeAccessory
                             ? "Accessory"
                             : "Device"),
                            __FUNCTION__,
                            link->recvSeq,
                            link->sentAck);
#endif
#if iAP2_LINK_ALLOW_STATS
                ++(link->numSentACK);
#endif
                iAP2LinkSendPacket (link, pck, FALSE,
                                    (link->type == kiAP2LinkTypeAccessory
                                     ? "Accessory:ProcessInOrderPacketACK"
                                     : "Device:ProcessInOrderPacketACK"));
            }
#if iAP2LINK_DEBUG
            else
            {
                iAP2LogDbg ("%s:%d %s:%s Don't send ACK recvSeq=%u sentAck=%u gap=%u maxCumAck=%u\n",
                            __FILE__, __LINE__,
                            (link->type == kiAP2LinkTypeAccessory
                             ? "Accessory"
                             : "Device"),
                            __FUNCTION__,
                            link->recvSeq,
                            link->sentAck,
                            gap,
                            link->param.maxCumAck);
            }
#endif
        }
#if iAP2LINK_DEBUG
        else
        {
            iAP2LogDbg ("%s:%d %s:%s INVALID gap or state! Don't send ACK recvSeq=%u sentAck=%u gap=%u maxCumAck=%u state=%d\n",
                        __FILE__, __LINE__,
                        (link->type == kiAP2LinkTypeAccessory
                         ? "Accessory"
                         : "Device"),
                        __FUNCTION__,
                        link->recvSeq,
                        link->sentAck,
                        gap,
                        link->param.maxCumAck,
                        link->fsm->currentState);
        }
#endif
    }
    else if (packet->packetLen == kIAP2PacketHeaderLen)
    {
        iAP2PacketSYNData_t synData;

        /* ACK only Packet */
#if iAP2_LINK_ALLOW_STATS
        ++(link->numRecvACK);
#endif
        memcpy (&synData,&link->negotiatedParam, sizeof(synData));
        synData.maxOutstandingPackets = synData.peerMaxOutstandingPackets;
        synData.maxPacketSize = synData.peerMaxPacketSize;
        if (link->fsm->currentState < kiAP2LinkStateConnected &&
            !iAP2LinkIsValidSynParam (&synData))
        {
            iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvACKBadLink);
        }
        else
        {
            iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventRecvACK);
        }
    }
    else
    {
        bInvalidPacket = TRUE;
    }

    if (!bInvalidPacket)
    {
        /*
        ** received ACK so signalSendBuff in case there are any packets that
        ** need to be sent out.
        */
        link->signalSendBuffCB (link);
    }

    return (bInvalidPacket == FALSE);
}


 BOOL _iAP2LinkProcessInOrderPacketSUS (struct iAP2Link_st* link,
                                              iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;
    
    if (packet->packetLen == kIAP2PacketHeaderLen)
    {
        /* SUS Packet */
        iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventSuspend);
    }
    else
    {
#if iAP2LINK_DEBUG
        iAP2LogStart();
        iAP2LogDbg("%s:%d %s: Invalid packetLen!, packetLen=%u (= %u)\n",
                   __FILE__, __LINE__, __FUNCTION__,
                   packet->packetLen, kIAP2PacketHeaderLen);
        iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, __FUNCTION__, "");
        iAP2LogStop();
#endif
        bInvalidPacket = TRUE;
    }
    
    return (bInvalidPacket == FALSE);
}


/*
****************************************************************
**
**  iAP2LinkProcessInOrderPacket
**
**  Input:
**      link:       link structure
**      packet:     packet to handle
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
                                   iAP2Packet_t*       packet)
{
    BOOL bInvalidPacket = FALSE;
#if iAP2LINK_DEBUG
    iAP2LogStart();
    iAP2LogDbg("%s:%d ProcessInOrderPacket: %s seq=%u ack=%u len=%u\n",
               __FILE__, __LINE__, iAP2PacketName(packet),
               packet->pckData->seq, packet->pckData->ack, packet->packetLen);
#if iAP2LINK_DEBUG_PACKET
    iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, "ProcessInOrderPacket", "");
#endif
    iAP2LogStop();
#endif
    link->recvPck = packet;

    if (packet->pckData->ctl & kIAP2PacketControlMaskSYN)
    {
        bInvalidPacket = ! _iAP2LinkProcessInOrderPacketSYN (link, packet);
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskEAK)
    {
        bInvalidPacket = ! _iAP2LinkProcessInOrderPacketEAK (link, packet);
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskRST)
    {
        bInvalidPacket = ! _iAP2LinkProcessInOrderPacketRST (link, packet);
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
    {
        bInvalidPacket = ! _iAP2LinkProcessInOrderPacketACK (link, packet);
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskSUS)
    {
        bInvalidPacket = ! _iAP2LinkProcessInOrderPacketSUS (link, packet);
    }

    if (bInvalidPacket) /* Invalid Packet! */
    {
        iAP2LogStart();
        iAP2LogError("%s:%d Received Invalid Packet (control=0x%x seq=%u sentSeq=%u recvAck=%u recvSeq=%u sentAck=%u)\n",
                     __FILE__, __LINE__,
                     packet->pckData->ctl, packet->pckData->seq,
                     link->sentSeq, link->recvAck,
                     link->recvSeq, link->sentAck);
        iAP2PacketDebugPrintPacketNL(packet, __FILE__, __LINE__, "ProcessInOrderPacket", "");
        iAP2LogStop();
    }

    return (bInvalidPacket == FALSE);
}


/*
****************************************************************
**
**  iAP2LinkSendPacketCommon
**
**  Input:
**      link:       link structure
**      packet:     packet to send
**      bResend:    this is a resend of a packet
**      tag:        tag for debug logging
**      bWaitSend:  true if wait until packet sent out, else false
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
 void _iAP2LinkSendPacketCommon (iAP2Link_t*    link,
                                       iAP2Packet_t*  packet,
                                       BOOL           bResend,
                                       const char*    tag,
                                       BOOL           bWaitSend)
{
    if (link != NULL && packet != NULL)
    {
        uint32_t curTimeMs;

#if iAP2_LINK_ALLOW_STATS
        ++(link->packetsSent);
        link->bytesSent += packet->packetLen;
#endif

        if ((packet->pckData->ctl & kIAP2PacketControlMaskACK) != 0)
        {
            /* This packet contains an ACK, stop sendAckTimer */

            if (packet->pckData->ack != link->recvSeq)
            {
                packet->pckData->ack = link->recvSeq;

                /*
                ** TODO: If buffer has already been generated (for resend), then
                **       re-calc header checksum based on the one byte (ack) change
                */
            }

            link->sentAck = packet->pckData->ack;
            link->bValidSentAck = TRUE;
        }

        curTimeMs = iAP2TimeGetCurTimeMs();
#if DEBUG
        if (bResend)
        {
#if iAP2_LINK_ALLOW_STATS
            link->totACKDelays += (curTimeMs - packet->timeStamp);
            ++(link->numACKDelays);
#endif
        }
#endif
        packet->timeStamp = curTimeMs;

        if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
        {
            /* Sending ACK so reset sendACK timer */
            iAP2TimeCancelTimer(link->mainTimer, link->sendAckTimeoutID);
            link->sendAckTimeoutID = INVALID_TIMEOUT_ID;
        }

#if iAP2LINK_DEBUG_PACKET
        iAP2PacketDebugPrintPacket (packet, __FILE__, __LINE__, tag, "");
#endif
        if (!bWaitSend)
        {
            (*link->sendPacketCB) (link, packet);
        }
        else
        {
            (*link->sendPacketWaitCB) (link, packet);
        }

        if (!bResend)
        {
            /* New sequence # being sent out. */
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d %s sentSeq(%u)->(%u) control=%xh(%s) ack=%u len=%u reTxCount=%d/%d bWaitSend=%d\n",
                       __FILE__, __LINE__,
                       (link->type == kiAP2LinkTypeAccessory
                        ? "Accessory:SendPacket"
                        : "Device:SendPacket"),
                       link->sentSeq,
                       packet->pckData->seq,
                       packet->pckData->ctl, iAP2PacketName (packet),
                       packet->pckData->ack,
                       packet->packetLen,
                       packet->retransmitCount,
                       link->param.maxRetransmissions,
                       bWaitSend);
#endif
            link->sentSeq = packet->pckData->seq;
            link->bValidSentSeq = TRUE;

            if (iAP2PacketIsACKOnly(packet))
            {
                /*
                ** increment seq number for ACKs so that it doesn't get
                ** deleted too soon.
                */
                packet->seqPlus = 1;
            }

            /*
            ** Save data in list until ACK'd
            ** Sent pure ACKs are stored here until cleanup occurs.
            */
			uint8_t RetIdx = 0;
           	RetIdx =  iAP2LinkAddPacketAfter (link->sendPckList,
                                    iAP2ListArrayGetLastItemIndex(link->sendPckList),
                                    &packet);
			if(RetIdx >= kiAP2ListArrayInvalidIndex){
				g_message("sendPckList is Full(lsk test)\n");
//				uint8_t item = iAP2ListArrayGetFirstItemIndex (link->sendPckList);
//				iAP2ListArrayDeleteItem (link->sendPckList, item, NULL);	
//				_DeletePckCB(&packet);
			}
			else{

			}

            if (iAP2PacketRequireACK(packet))
            {
                /* start ACK_Timer */
                uint8_t timerID = iAP2TimeCallbackAfter(link->mainTimer,
                                                        kiAP2LinkEventWaitACKTimeout,
                                                        link->param.retransmitTimeout);
                iAP2PacketAssignTimer (packet, timerID);
            }
        }
        else
        {
#if iAP2LINK_DEBUG
            iAP2LogDbg("%s:%d %s Resend sentSeq=%u seq=%u control=%xh ack=%u len=%u reTxCount=%d/%d bWaitSend=%d\n",
                       __FILE__, __LINE__,
                       (link->type == kiAP2LinkTypeAccessory
                        ? "Accessory:SendPacket"
                        : "Device:SendPacket"),
                       link->sentSeq,
                       packet->pckData->seq,
                       packet->pckData->ctl,
                       packet->pckData->ack,
                       packet->packetLen,
                       packet->retransmitCount,
                       link->param.maxRetransmissions,
                       bWaitSend);
#endif
            /* start ACK_Timer */
            {
                uint8_t timerID = iAP2TimeCallbackAfter(link->mainTimer,
                                                        kiAP2LinkEventWaitACKTimeout,
                                                        link->param.retransmitTimeout);
                iAP2PacketAssignTimer (packet, timerID);
            }
        }
    }
    else
    {
        iAP2LogError("%s:%d Invalid link(%p) or packet(%p)\n",
                     __FILE__, __LINE__, link, packet);
    }
}


/*
****************************************************************
**
**  iAP2LinkSendPacket
**
**  Input:
**      link:       link structure
**      packet:     packet to send
**      bResend:    this is a resend of a packet
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
                         const char*    tag)
{
    _iAP2LinkSendPacketCommon (link, packet, bResend, tag, FALSE);
}


/*
****************************************************************
**
**  iAP2LinkSendPacketWaitSend
**
**  Input:
**      link:       link structure
**      packet:     packet to send
**      bResend:    this is a resend of a packet
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
void iAP2LinkSendPacketWaitSend (iAP2Link_t*      link,
                                 iAP2Packet_t*    packet,
                                 BOOL             bResend,
                                 const char*      tag)
{
    _iAP2LinkSendPacketCommon (link, packet, bResend, tag, TRUE);
}


/*
****************************************************************
**
**  iAP2LinkResetSeqAck
**
**  Input:
**      link:       link structure
**      bOnlySend:  reset only seq# related info
**
**  Output:
**      none
**
**  Return:
**      none
**
****************************************************************
*/
void iAP2LinkResetSeqAck (iAP2Link_t* link, BOOL bOnlySend)
{
    _ResetAckedPackets (link);
    link->startSeq        = (uint8_t) rand();
    link->sentSeq         = 0;
    link->bValidSentSeq   = FALSE;
    link->sentAck         = 0;
    link->bValidSentAck   = FALSE;
    link->numSentSYN      = 0;
    link->numSentSYNACK   = 0;
    link->numResentSYN    = 0;
    link->numResentSYNACK = 0;
    if (!bOnlySend)
    {
        _ResetRecvPackets (link);
        link->recvSeq       = 0;
        link->bValidRecvSeq = FALSE;
        link->recvAck       = 0;
        link->bValidRecvAck = FALSE;
        link->numRecvSYN    = 0;
        link->numRecvSYNACK = 0;
    }
}


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
iAP2Packet_t* iAP2LinkPacketForIndex (uint8_t* listArrayBuffer, uint8_t index)
{
    iAP2Packet_t** pPck = (iAP2Packet_t**) iAP2ListArrayItemForIndex(listArrayBuffer, index);
    if (pPck)
    {
        return *pPck;
    }
    return NULL;
}


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
                            piAP2ListArrayCompareFunc   func)
{
    return iAP2ListArrayFindItem (listArrayBuffer, packet, func);
}


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
                                iAP2Packet_t**  packet)
{
    return iAP2ListArrayAddItemAfter (listArrayBuffer, prevItemIndex, packet);
}


/*
****************************************************************
**
**  iAP2LinkHandleWaitACKTimeoutLink
**
**  Input:
**      link:       link
**      curTime:    current time in ms.
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: This function is called when the WaitACKTimer expires.
**
****************************************************************
*/
void iAP2LinkHandleWaitACKTimeoutLink (iAP2Link_t* link, uint32_t curTime)
{
    assert (link);

    iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventWaitACKTimeout);
}


/*
****************************************************************
**
**  iAP2LinkHandleSendACKTimeoutLink
**
**  Input:
**      link:       link
**      curTime:    current time in ms.
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: This function is called when the SendACKTimer expires.
**
****************************************************************
*/
void iAP2LinkHandleSendACKTimeoutLink (iAP2Link_t* link, uint32_t curTime)
{
    assert (link);

    iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventSendACKTimeout);
}


/*
****************************************************************
**
**  iAP2LinkHandleWaitDetectTimeoutLink
**
**  Input:
**      link:       link
**      curTime:    current time in ms.
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: This function is called when the WaitDetectTimer expires.
**
****************************************************************
*/
void iAP2LinkHandleWaitDetectTimeoutLink (iAP2Link_t* link, uint32_t curTime)
{
    assert (link);

    iAP2FSMHandleEvent (link->fsm, kiAP2LinkEventWaitDetectTimeout);
}


/*
****************************************************************
**
**  iAP2LinkSendRST
**
**  Input:
**      link:   link structure
**
**  Output:
**      none
**
**  Return:
**      None
**
**  Note: Sends a RST packet.
**
****************************************************************
*/
void iAP2LinkSendRST (iAP2Link_t* link)
{
    iAP2Packet_t* pck;

    if (link->connectedCB)
    {
        /* Notify about the connection establishment */
        (*link->connectedCB) (link, FALSE);
    }

    iAP2LinkResetSeqAck (link, FALSE);
    iAP2LinkSetDefaultSYNParam (&(link->param));

    memcpy (&(link->negotiatedParam),
            &(link->initParam),
            sizeof(link->initParam));

    /* Send RST */
    pck = iAP2PacketCreateRSTPacket (link,
                                     link->sentSeq);
    iAP2LinkSendPacket (link, pck, FALSE, "SendRST");
}


/*
****************************************************************
**
**  iAP2LinkGetSessionInfo
**
**  Input:
**      link:       link structure
**      session:    session to return the info for
**
**  Output:
**      None
**
**  Return:
**      iAP2PacketSessionInfo_t*    Pointer to Session Info
**
****************************************************************
*/
iAP2PacketSessionInfo_t* iAP2LinkGetSessionInfo (iAP2Link_t* link,
                                                 uint8_t     session)
{
    iAP2PacketSessionInfo_t* pSession = NULL;
    if (link && link->param.numSessionInfo)
    {
        uint8_t i;
        for (i = 0; i < link->param.numSessionInfo; ++i)
        {
            if (link->param.sessionInfo[i].id == session)
            {
                pSession = &(link->param.sessionInfo[i]);
            }
        }
    }
    return pSession;
}


/*
****************************************************************
**
**  iAP2LinkGetSessionForService
**
**  Input:
**      link:       link structure
**      service:    service type
**
**  Output:
**      None
**
**  Return:
**      uint32_t    session ID for the service type returns 0 if not found.
**
****************************************************************
*/
uint32_t iAP2LinkGetSessionForService (iAP2Link_t*              link,
                                       iAP2PacketServiceType_t  service)
{
    uint32_t session = 0xFFFFFFFFUL;
    if (link)
    {
        int i;
        for (i = 0; i < link->param.numSessionInfo; ++i)
        {
            if (link->param.sessionInfo[i].type == service)
            {
                session = link->param.sessionInfo[i].id;
            }
        }
    }
    return session;
}


/*
****************************************************************
**
**  iAP2LinkGetMainTimer
**
**  Input:
**      link:       link structure
**
**  Output:
**      None
**
**  Return:
**      iAP2Timer_t*    Pointer to main timer
**
****************************************************************
*/
iAP2Timer_t* iAP2LinkGetMainTimer (iAP2Link_t* link)
{
    return link->mainTimer;
}


#if iAP2_LINK_USE_LINKRUNLOOP
/*
****************************************************************
**
**  iAP2LinkSetUseiAP2LinkRunLoop
**
**  Input:
**      link:   link structure
**
**  Output:
**      none
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkSetUseiAP2LinkRunLoop (iAP2Link_t* link)
{
    link->bUseiAP2LinkRunLoop = TRUE;
}
#endif


/*
****************************************************************
**
**  iAP2LinkDebugPrintPacketList
**
**  Input:
**      link:       link structure
**      packetList: listArrayBuffer pointer
**      index:      list index, if part of group of lists
**      name:       name of list to display
**      tag:        additional info to display with packet info
**      bDebug:     whether to print as debug with data or regular stats printout
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Should be called within iAP2LogStart(), iAP2LogStop() pair.
**
****************************************************************
*/
void iAP2LinkDebugPrintPacketList (iAP2Link_t*  link,
                                   uint8_t*     packetList,
                                   uint8_t      index,
                                   const char*  name,
                                   const char*  tag,
                                   BOOL         bDebug)
{
#if DEBUG
    __printPacketList (tag, name, link, packetList, FALSE, bDebug);
#endif /* DEBUG */
}


/*
****************************************************************
**
**  iAP2LinkDebugPrintLink
**
**  Input:
**      link:       link to print out info for.
**      file:       file called from
**      line:       line number called from
**      tag:        additional info to display with packet info
**      bDebug:     whether to print as debug with data or regular stats printout
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Should be called within iAP2LogStart(), iAP2LogStop() pair.
**
****************************************************************
*/
void iAP2LinkDebugPrintLink (iAP2Link_t* link,
                             const char* file,
                             int         line,
                             const char* tag,
                             BOOL        bDebug)
{
#if DEBUG
     char* stateName [kiAP2LinkStateCount] =
        { "Init", "Detached", "Detect", "Idle", "Pending", "Connected", "Suspend", "Failed" };

    iAP2LogType_t   type = (bDebug ? kiAP2LogTypeData : kiAP2LogTypeLog);
    int             i;

    if (link == NULL || file == NULL)
    {
        return;
    }

    iAP2LogTypeNL(type, "%s:%d link DEBUG----\n", file, line);
    iAP2LogTypeNL(type, "%s%slink(%p): context=%p\n", (tag ? tag : ""), (tag ? ": " : ""), link, link->context);
    iAP2LogTypeNL(type, "%s%s    type=%s fsm=%ph currentState=%s context=%p\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  (link->type == kiAP2LinkTypeDevice ? "Device"  : "Accessory"),
                  link->fsm,
                  stateName[link->fsm->currentState]);
    iAP2LogTypeNL(type, "%s%s    sendPacketCB=%ph recvDataCB=%ph\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->sendPacketCB, link->recvDataCB);
    iAP2LogTypeNL(type, "%s%s    connectedCB=%ph signalSendBuffCB=%ph\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->connectedCB, link->signalSendBuffCB);
#if iAP2_LINK_ALLOW_STATS
    iAP2LogTypeNL(type, "%s%s    totACKDelays=%u numACKDelays=%u avgACKDelays=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->totACKDelays, link->numACKDelays,
                  (link->numACKDelays > 0
                   ? link->totACKDelays / link->numACKDelays
                   : 0));
#endif
    iAP2LogTypeNL(type, "%s%s    startSeq=%u(%xh) recvSeq(%d)=%u(%xh) sentACK(%d)=%u(%xh) sentSeq(%d)=%u(%xh) recvAck(%d)=%u(%xh)\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  link->startSeq, link->startSeq,
                  link->bValidRecvSeq,
                  link->recvSeq, link->recvSeq,
                  link->bValidSentAck,
                  link->sentAck, link->sentAck,
                  link->bValidSentSeq,
                  link->sentSeq, link->sentSeq,
                  link->bValidRecvAck,
                  link->recvAck, link->recvAck);
#if iAP2_LINK_ALLOW_STATS
    iAP2LogTypeNL(type, "%s%s    noAckReTxCount=%d missingReTxCount=%d invalidPackets=%d numOutOfOrder=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->noAckReTxCount, link->missingReTxCount,
                  link->invalidPackets, link->numOutOfOrder);
    iAP2LogTypeNL(type, "%s%s    sent: tot=%u #SYN=%d #SYNACK=%d #EAK=%d #RST=%d #ACK=%d #DATA=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  (link->numSentSYN + link->numSentSYNACK + link->numSentEAK +
                   link->numSentRST + link->numSentACK + link->numSentDATA),
                  link->numSentSYN, link->numSentSYNACK, link->numSentEAK,
                  link->numSentRST, link->numSentACK, link->numSentDATA);
#endif
    iAP2LogTypeNL(type, "%s%s          #SYNcum=%d #SYNACKcum=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  link->numSentCumSYN, link->numSentCumSYNACK);
#if iAP2_LINK_ALLOW_STATS
    iAP2LogTypeNL(type, "%s%s    resent: tot=%u #SYN=%d #SYNACK=%d #ACK=%d #DATA=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  (link->numResentSYN + link->numResentSYNACK +
                   link->numResentACK + link->numResentDATA),
                  link->numResentSYN, link->numResentSYNACK,
                  link->numResentACK, link->numResentDATA);
#endif
    iAP2LogTypeNL(type, "%s%s            #SYNcum=%d #SYNACKcum=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  link->numResentCumSYN, link->numResentCumSYNACK);
#if iAP2_LINK_ALLOW_STATS
    iAP2LogTypeNL(type, "%s%s    rcvd: tot=%u #SYN=%d #SYNACK=%d #EAK=%d #RST=%d #ACK=%d #DATA=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  (link->numRecvSYN + link->numRecvSYNACK + link->numRecvEAK +
                   link->numRecvRST + link->numRecvACK + link->numRecvDATA),
                  link->numRecvSYN, link->numRecvSYNACK, link->numRecvEAK,
                  link->numRecvRST, link->numRecvACK, link->numRecvDATA);
    iAP2LogTypeNL(type, "%s%s          #SYNcum=%d #SYNACKcum=%d\n",
                  (tag ? tag : ""), (tag ? ": " : ""),
                  link->numRecvCumSYN, link->numRecvCumSYNACK);
    iAP2LogTypeNL(type, "%s%s    rcvd(b:%u/p:%u/db:%u/dp:%u) sent(b:%u/p:%u/db:%u/dp:%u)\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->bytesRcvd, link->packetsRcvd,
                  link->dataBytesRcvd, link->dataPacketsRcvd,
                  link->bytesSent, link->packetsSent,
                  link->dataBytesSent, link->dataPacketsSent);
#endif
    iAP2LogTypeNL(type, "%s%s    param: ver=%02xh\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.version);
    iAP2LogTypeNL(type, "%s%s           maxOutstandingPackets=%u maxRetransmissions=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.maxOutstandingPackets, link->param.maxRetransmissions);
    iAP2LogTypeNL(type, "%s%s           maxCumAck=%u maxPacketSize=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.maxCumAck, link->param.maxPacketSize);
    iAP2LogTypeNL(type, "%s%s           retransmitTimeout=%u cumAckTimeout=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.retransmitTimeout, link->param.cumAckTimeout);
    iAP2LogTypeNL(type, "%s%s           peerMaxOutstandingPackets=%u peerMaxPacketSize=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.peerMaxOutstandingPackets, link->param.peerMaxPacketSize);
    iAP2LogTypeNL(type, "%s%s           sessions=%u\n",
                  (tag ? tag : ""), (tag ? ": " : ""), link->param.numSessionInfo);
    {
        uint8_t index;
        for (index = 0;
             index < kIAP2PacketMaxSessions && index < link->param.numSessionInfo;
             ++index)
        {
            iAP2LogTypeNL(type, "%s%s               index %u [id=%u type=%u version=%u]\n",
                          (tag ? tag : ""), (tag ? ": " : ""), index,
                          link->param.sessionInfo[index].id,
                          link->param.sessionInfo[index].type,
                          link->param.sessionInfo[index].version);
        }
    }
    if (bDebug)
    {
        iAP2LogTypeNL(type, "%s%s    recvPacket=%ph\n", (tag ? tag : ""), (tag ? ": " : ""), link->recvPck);
        if (link->recvPck)
        {
            iAP2PacketDebugPrintPacketNL(link->recvPck, tag, 0, tag, "        ");
        }
        iAP2TimePrintInfo (link->mainTimer, FALSE);
    }
    iAP2LinkDebugPrintPacketList (link, link->sendPckList, 0, "sendPckList", tag, bDebug);
    iAP2LinkDebugPrintPacketList (link, link->recvPckList, 0, "recvPckList", tag, bDebug);
    for (i = 0; i < kIAP2PacketServiceTypeCount; ++i)
    {
        iAP2LinkDebugPrintPacketList (link, link->sessSendPckList[i], i, "sessSendPckList", tag, bDebug);
    }
#endif /* DEBUG */
}


/*
****************************************************************
**
**  iAP2LinkDebugIgnoreSynRetryLimit
**
**  Input:
**      link:   link to set IgnoreSynRetryLimit state on.
**      bFlag:  whether to IgnoreSynRetryLimit (TRUE) or not (FALSE)
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkDebugIgnoreSynRetryLimit (iAP2Link_t* link,
                                       BOOL        bFlag)
{
    if (link)
    {
        link->bIgnoreSynRetryLimit = bFlag;
    }
}


