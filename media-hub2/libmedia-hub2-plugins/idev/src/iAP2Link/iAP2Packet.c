/*
 *	File: iAP2Packet.c
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iAP2Time.h>
#include <iAP2Log.h>
#include <iAP2BuffPool.h>

#include "iAP2Packet.h"
#include "iAP2Link.h"

#if DEBUG
#define iAP2PACKET_DEBUG 1
#define iAP2PACKET_DEBUG_PARSE 0
#endif

#if DEBUG
static const char* stateName [kiAP2PacketParseStateLAST + 1] =
{
    "SOP1",
    "SOP2",
    "LEN1",
    "LEN2",
    "CTRL",
    "SEQ",
    "ACK",
    "SESSID",
    "CHK",
    "PAYLOAD",
    "PAYLOADCHK",
    "FINISH",
    "DETECT",
    "DETECTBAD"
};
#endif


/*
 ****************************************************************
 **
 **  iAP2Packet function
 **
 ****************************************************************
 */

/*
****************************************************************
**
**  iAP2PacketParseBuffer
**
**  Input:
**      buffer:         Input buffer containing packet data to parse.
**      bufferLen:      Size of the Input buffer data.
**      maxPacketSize:  MaxPacketSize supported... any packets claiming to be
**                      larger will be ignored.
**
**  Output:
**      packet:     Provided packet structure is updated with parsed data.
**      bDetect:    Set to TRUE if iAP 1.0/2.0 detect or detect BAD ACK packet
**                  was received, else FALSE
**      failedChecksums:    if provided, value will be incremented when
**                          checksum failure is detected.
**      sopDetect:  if provided, value will be incremented whenvere SOP is detected.
**
**  Return:
**      uint32_t    Number of bytes parsed from the buffer.
**
****************************************************************
*/
uint32_t iAP2PacketParseBuffer (const uint8_t*  buffer,
                                uint32_t        bufferLen,
                                iAP2Packet_t*   packet,
                                uint32_t        maxPacketSize,
                                BOOL*           bDetect,
                                uint32_t*       failedChecksums,
                                uint32_t*       sopDetect)
{
    uint32_t totParsedLen = 0;

    *bDetect = FALSE;

    if (buffer != NULL &&  packet != NULL)
    {
        const uint8_t* pt    = buffer;
        uint32_t ptLen       = bufferLen;
        const uint8_t* ptEnd = pt + ptLen;
        uint16_t calcChecksum;

        while (pt < ptEnd && packet->state < kiAP2PacketParseStateFINISH)
        {
#if iAP2PACKET_DEBUG_PARSE
            iAP2LogDbg ("%s:%d state(%u:%s) dataCurLen=%u packetLen=%u bufferLen=%u\n",
                        __FILE__, __LINE__, packet->state, stateName [packet->state],
                        packet->dataCurLen, packet->packetLen, packet->bufferLen);
#endif
            switch (packet->state)
            {
                case kiAP2PacketParseStateSOP1:
                {
                    packet->dataCurLen = 0;
                    packet->dataChecksum = 0;
                    packet->packetLen = 0;
                    packet->bufferLen = 0;

                    if (kIAP2PacketSYNC == *pt)
                    {
                        packet->state = kiAP2PacketParseStateSOP2;
                        packet->pckData->sop1 = *pt;
                        packet->bufferLen += 1;
                    }
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateSOP2:
                {
                    if (kIAP2PacketSOP == *pt)
                    {
                        if (sopDetect)
                        {
                            ++(*sopDetect);
                        }
                        packet->state = kiAP2PacketParseStateLEN1;
                        packet->pckData->sop2 = *pt;
                        packet->bufferLen += 1;
                    }
                    else if (kIAP2PacketSOPOrig == *pt)
                    {
                        if (sopDetect)
                        {
                            ++(*sopDetect);
                        }
                        packet->state = kiAP2PacketParseStateLEN1;
                        packet->pckData->sop2 = *pt;
                        packet->bufferLen += 1;
                    }
                    else if (kIAP2PacketSYNC != *pt)
                    {
                        packet->state = kiAP2PacketParseStateSOP1;
                    }
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateLEN1:
                {
                    packet->pckData->len1 = *pt;
                    packet->packetLen = (((uint16_t) *pt) << 8);
                    packet->state = kiAP2PacketParseStateLEN2;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateLEN2:
                {
                    packet->pckData->len2 = *pt;
                    packet->packetLen += *pt;
                    if (((maxPacketSize == 0 ||
                          packet->packetLen <= maxPacketSize) &&
                         packet->packetLen >= kIAP2PacketHeaderLen) ||
                        kIAP2PacketDetectLEN == packet->packetLen ||
                        kIAP2PacketDetectNACKLEN == packet->packetLen)
                    {
                        packet->state = kiAP2PacketParseStateCTRL;
                        packet->bufferLen += 1;
                        totParsedLen += 1;
                        pt += 1;
                        ptLen -= 1;
                    }
                    else
                    {
                        /*
                        ** Parse is done on received packets only which are
                        ** allocated with maximum receive payload size so it
                        ** can be reused.
                        */
                        iAP2PacketResetPacket (packet);
                    }
                }
                    break;
                case kiAP2PacketParseStateCTRL:
                {
                    packet->pckData->ctl = *pt;
                    packet->state = kiAP2PacketParseStateSEQ;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateSEQ:
                {
                    packet->pckData->seq = *pt;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;

                    if (iAP2PacketCheckDetect (packet))
                    {
                        packet->state = kiAP2PacketParseStateDETECT;
                        *bDetect = TRUE;
                    }
                    else
                    {
                        packet->state = kiAP2PacketParseStateACK;
                    }
                }
                    break;
                case kiAP2PacketParseStateACK:
                {
                    packet->pckData->ack = *pt;
                    packet->state = kiAP2PacketParseStateSESSID;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateSESSID:
                {
                    packet->pckData->sess = *pt;
                    packet->state = kiAP2PacketParseStateCHK;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                }
                    break;
                case kiAP2PacketParseStateCHK:
                {
                    packet->pckData->chk = *pt;
                    packet->state = kiAP2PacketParseStatePAYLOAD;
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;

                    calcChecksum = iAP2PacketCalcChecksum (iAP2PacketGetBuffer(packet),
                                                           kIAP2PacketHeaderLen - kIAP2PacketChksumLen);
                    if (calcChecksum == packet->pckData->chk)
                    {
                        packet->pckData->chk = calcChecksum;
                        packet->dataCurLen = 0;
                        if (packet->packetLen == kIAP2PacketHeaderLen)
                        {
                            packet->state = kiAP2PacketParseStateFINISH;
                        }
                    }
                    else if (iAP2PacketCheckDetectNACK (packet))
                    {
                        packet->state = kiAP2PacketParseStateDETECTBAD;
                        *bDetect = TRUE;
                    }
                    else
                    {
                        /*
                        ** checksum failed, restart
                        ** Just ignore the bytes, let retransmit do recovery
                        */
                        if (failedChecksums)
                        {
                            ++(*failedChecksums);
                        }
#if iAP2PACKET_DEBUG
                        iAP2LogStart();
                        iAP2LogPrintData(iAP2PacketGetBuffer(packet), kIAP2PacketHeaderLen,
                                         "ParseBuffer",
                                         "%s:%d Header Checksum Failed! (%xh != %xh)",
                                         __FILE__, __LINE__, calcChecksum, packet->pckData->chk);
                        iAP2LogStop();
#endif
                        /*
                        ** Parse is done on received packets only which are
                        ** allocated with maximum receive payload size so it
                        ** can be reused.
                        */
                        iAP2PacketResetPacket (packet);
                    }
                }
                    break;
                case kiAP2PacketParseStatePAYLOAD:
                {
                    uint32_t bytesToCopy = iAP2PacketGetPayloadLen (packet) - packet->dataCurLen;
                    if (bytesToCopy > 0)
                    {
                        /* Get some payload bytes */
                        if (ptLen < bytesToCopy)
                        {
                            bytesToCopy = ptLen;
                        }

                        memcpy (&(iAP2PacketGetPayload (packet) [packet->dataCurLen]),
                                pt,
                                bytesToCopy);
                        packet->dataCurLen += bytesToCopy;
                        packet->bufferLen += bytesToCopy;
                        totParsedLen += bytesToCopy;
                        pt += bytesToCopy;
                        ptLen -= bytesToCopy;
                    }
                    if (packet->dataCurLen == iAP2PacketGetPayloadLen (packet))
                    {
                        packet->state = kiAP2PacketParseStatePAYLOADCHK;
                    }
                }
                    break;
                case kiAP2PacketParseStatePAYLOADCHK:
                {
                    iAP2PacketGetPayload (packet) [packet->dataCurLen] = *pt;
                    /* Don't include Checksum Byte in payload data len (dataCurLen). */
                    /*packet->dataCurLen += 1;*/
                    packet->bufferLen += 1;
                    totParsedLen += 1;
                    pt += 1;
                    ptLen -= 1;
                    if (packet->packetLen == packet->bufferLen)
                    {
                        /* Got all the payload bytes */

                        /* Need to check payload checksum */
                        uint8_t* payload = iAP2PacketGetPayload (packet);
                        uint16_t payloadLen = iAP2PacketGetPayloadLen (packet);
                        packet->dataChecksum = iAP2PacketCalcChecksum (payload, payloadLen);
                        if (packet->dataChecksum == payload [payloadLen])
                        {
                            packet->state = kiAP2PacketParseStateFINISH;
                        }
                        else
                        {
                            /* Checksum failed, ignore packet. */
                            if (failedChecksums)
                            {
                                ++(*failedChecksums);
                            }
#if iAP2PACKET_DEBUG
                            iAP2LogStart();
                            iAP2LogPrintData (payload,
                                              payloadLen,
                                              "ParseBuffer",
                                              "%s:%d Payload Checksum Failed! (%xh != %xh) buffLen=%u seq=%u ack=%u len=%u control=%xh(%s)",
                                              __FILE__, __LINE__,
                                              packet->dataChecksum, payload [payloadLen],
                                              packet->bufferLen,
                                              packet->pckData->seq, packet->pckData->ack, packet->packetLen,
                                              packet->pckData->ctl, iAP2PacketName(packet));
                            iAP2LogStop();
#endif
                            /*
                            ** Parse is done on received packets only which are
                            ** allocated with maximum receive payload size so it
                            ** can be reused.
                            */
                            iAP2PacketResetPacket (packet);
                        }
                    }
                }
                    break;
                case kiAP2PacketParseStateFINISH:
                case kiAP2PacketParseStateDETECT:
                case kiAP2PacketParseStateDETECTBAD:
                    break;
            } /* switch (packet->state) */
        } /* while (pt < ptEnd) */

    } /* if (buffer != NULL && bufferLen >= kIAP2PacketHeaderLen &&  packet != NULL) */

    return totParsedLen;
}


/*
****************************************************************
**
**  iAP2PacketIsComplete
**
**  Input:
**      pck:    packet to find out if it is complete
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is complete and ready to be processed, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsComplete (const iAP2Packet_t* pck)
{
    BOOL result = FALSE;
    if (pck)
    {
        result = (kiAP2PacketParseStateFINISH <= pck->state);
    }
    return result;
}


/*
****************************************************************
**
**  iAP2PacketIsACKOnly
**
**  Input:
**      pck:    packet to find out if it is ACK only packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is ACK only, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsACKOnly (const iAP2Packet_t* pck)
{
    return ((pck->pckData->ctl & (kIAP2PacketControlMaskSYN |
                                 kIAP2PacketControlMaskEAK |
                                 kIAP2PacketControlMaskRST |
                                 kIAP2PacketControlMaskACK)) == kIAP2PacketControlMaskACK &&
            pck->packetLen == kIAP2PacketHeaderLen);
}


/*
 ****************************************************************
 **
 **  iAP2PacketIsDataPacket
 **
 **  Input:
 **      pck:    packet to find out if it is a data packet (ACK plus data)
 **
 **  Output:
 **      None
 **
 **  Return:
 **      BOOL    TRUE if packet is data packet, else FALSE
 **
 ****************************************************************
 */
BOOL iAP2PacketIsDataPacket (const iAP2Packet_t* pck)
{
    return ((pck->pckData->ctl & (kIAP2PacketControlMaskSYN |
                                 kIAP2PacketControlMaskEAK |
                                 kIAP2PacketControlMaskRST |
                                 kIAP2PacketControlMaskACK)) == kIAP2PacketControlMaskACK &&
            pck->packetLen > kIAP2PacketHeaderLen);
}


/*
****************************************************************
**
**  iAP2PacketIsEAK
**
**  Input:
**      pck:    packet to find out if it is EAK packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is EAK, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsEAK (const iAP2Packet_t* pck)
{
    return ((pck->pckData->ctl & (kIAP2PacketControlMaskEAK)) == kIAP2PacketControlMaskEAK);
}


/*
****************************************************************
**
**  iAP2PacketEqual
**
**  Input:
**      packet1:    first packet to compare
**      packet2:    second packet to compare
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packets compare same, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketEqual (const iAP2Packet_t* packet1, const iAP2Packet_t* packet2)
{
    return (packet1->packetLen    == packet2->packetLen    &&
            packet1->pckData->ctl  == packet2->pckData->ctl  &&
            packet1->pckData->seq  == packet2->pckData->seq  &&
            packet1->pckData->ack  == packet2->pckData->ack  &&
            packet1->pckData->chk  == packet2->pckData->chk  &&
            packet1->dataChecksum == packet2->dataChecksum &&
            ((iAP2PacketGetPayloadLen (packet1) == 0 && iAP2PacketGetPayloadLen (packet2) == 0) ||
             (iAP2PacketGetPayloadLen (packet1) != 0 && iAP2PacketGetPayloadLen (packet2) != 0 &&
              memcmp(iAP2PacketGetPayload ((iAP2Packet_t*) packet1),
                     iAP2PacketGetPayload ((iAP2Packet_t*) packet2),
                     iAP2PacketGetPayloadLen (packet1)) == 0)));
}


/*
****************************************************************
**
**  iAP2PacketGetBuffSize
**
**  Input:
**      payloadSize:    size of payload data
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of buff required for proper operation
**
****************************************************************
*/
uint32_t iAP2PacketGetBuffSize (uint16_t payloadSize)
{
    uint32_t result = sizeof(iAP2Packet_t) + payloadSize;
    assert (sizeof(iAP2PacketHeader_t) == kIAP2PacketHeaderLen);
    assert (result <= (sizeof(iAP2Packet_t) + (kiAP2PacketLenMax - kIAP2PacketHeaderLen - kIAP2PacketChksumLen)));
    return result;
}


/*
****************************************************************
**
**  iAP2PacketCreateEmptyPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
**  Note: OBSOLETE: equivalent to iAP2PacketCreateEmptyRecvPacket
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptyPacket (void* link)
{
    return iAP2PacketCreateEmptyRecvPacket (link);
}


/*
****************************************************************
**
**  iAP2PacketCreateEmptySendPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptySendPacket (void* link)
{
    iAP2Packet_t* pck = (iAP2Packet_t*) iAP2BuffPoolGet (((iAP2Link_t*)link)->sendPckPool,
                                                         iAP2LinkGetMaxSendPayloadSize (link));
    assert(pck);
    assert(pck->pckData);

    pck->link       = link;
    pck->sendPacket = TRUE;
    pck->timer      = INVALID_TIMEOUT_ID;
    pck->debugData  = 0;
    iAP2PacketResetPacket(pck);

    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreateEmptyRecvPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptyRecvPacket (void* link)
{
    iAP2Packet_t* pck = (iAP2Packet_t*) iAP2BuffPoolGet (((iAP2Link_t*)link)->recvPckPool,
                                                         iAP2LinkGetMaxRecvPayloadSize (link));
    assert(pck);
    assert(pck->pckData);

    pck->link       = link;
    pck->sendPacket = FALSE;
    pck->timer      = INVALID_TIMEOUT_ID;
    pck->debugData  = 0;
    iAP2PacketResetPacket(pck);

    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreate
**
**  Input:
**      link:       link to associate with this packet
**      control:    Control bitmask (SYN, ACK, EAK, RST, CHK)
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      payload:    Input buffer containing packet payload data.
**      payloadLen: Size of the Input payload data.
**      session:    session for this payload
**      sendPacket: TRUE if this is for send packet, else FALSE.
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreate (void*           link,
                                uint8_t         control,
                                uint8_t         seq,
                                uint8_t         ack,
                                const uint8_t*  payload,
                                uint32_t        payloadLen,
                                uint8_t         session,
                                BOOL            sendPacket)
{
    iAP2Packet_t* pck;
    uint16_t checkSumLen = (payloadLen > 0 ? kIAP2PacketChksumLen : 0);

    if (sendPacket)
    {
        pck = iAP2PacketCreateEmptySendPacket (link);
    }
    else
    {
        pck = iAP2PacketCreateEmptyRecvPacket (link);
    }

    assert(pck);

    pck->packetLen      = kIAP2PacketHeaderLen + payloadLen + checkSumLen;
    pck->pckData->sop1   = kIAP2PacketSYNC;
    pck->pckData->sop2   = kIAP2PacketSOP;
    pck->pckData->len1   = IAP2_HI_BYTE(pck->packetLen);
    pck->pckData->len2   = IAP2_LO_BYTE(pck->packetLen);
    pck->pckData->ctl    = control;
    pck->pckData->seq    = seq;
    pck->pckData->ack    = ack;
    pck->pckData->sess   = session;
    pck->pckData->chk    = 0;
    pck->seqPlus        = 0;
    pck->sendPacket     = sendPacket;
    pck->debugData      = 0;

    pck->bufferLen  = pck->packetLen;
    if (payload != NULL && payloadLen > 0)
    {
        memcpy (iAP2PacketGetPayload (pck), payload, payloadLen);
    }
    pck->dataCurLen = iAP2PacketGetPayloadLen (pck);
    pck->dataChecksum = 0;

    pck->state = kiAP2PacketParseStateFINISH;
    pck->recvEAK = FALSE;
    pck->retransmitCount = 0;
    pck->timer = INVALID_TIMEOUT_ID;

    pck->cbContext = NULL;
    pck->callbackOnSend = NULL;

    return pck;
}


/*
****************************************************************
**
**  iAP2PacketDelete
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Reset packet should only be used for the same purpose (send or receive)
**          because the allocation size is direction specific.
**
****************************************************************
*/
void iAP2PacketDelete (iAP2Packet_t* pck)
{
    iAP2Link_t* link = (iAP2Link_t*) pck->link;

    iAP2PacketResetPacket (pck);

    if (pck->timer != INVALID_TIMEOUT_ID)
    {
        iAP2TimeCancelTimer (iAP2LinkGetMainTimer(pck->link),
                             pck->timer);
        pck->timer = INVALID_TIMEOUT_ID;
    }

#if DEBUG
    iAP2LogStart();
#endif
    if (pck->sendPacket)
    {
        iAP2BuffPoolReturn (((iAP2Link_t*)link)->sendPckPool, pck);
    }
    else
    {
        iAP2BuffPoolReturn (((iAP2Link_t*)link)->recvPckPool, pck);
    }
#if DEBUG
    iAP2LogStop();
#endif
}


/*
****************************************************************
**
**  iAP2PacketResetPacket
**
**  Input:
**      pck:                packet to reset
**      bFreePrivateData    free private data buffer as well if bFreeBuffers is TRUE
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketResetPacket (iAP2Packet_t* pck)
{
    pck->dataCurLen     = 0;
    pck->dataChecksum   = 0;
    pck->bufferLen      = 0;
    pck->packetLen      = 0;
    pck->timeStamp      = 0;
    pck->seqPlus        = 0;
    pck->state          = kiAP2PacketParseStateSOP1;
    /* clear the packet header information */
    memset (pck->pckData, 0, kIAP2PacketHeaderLen);
    pck->cbContext      = NULL;
    pck->callbackOnSend = NULL;
    pck->recvEAK        = FALSE;
    pck->retransmitCount = 0;
}


/*
****************************************************************
**
**  iAP2PacketGetSpaceLeft
**
**  Input:
**      packet:         Packet to find remaining space for.
**      maxPacketLen:   Max packet size.
**
**  Output:
**      None
**
**  Return:
**      number of bytes remaining for packet payload based on maxPacketLen.
**
****************************************************************
*/
uint16_t iAP2PacketGetSpaceLeft (iAP2Packet_t*  packet,
                                 uint16_t       maxPacketLen)
{
    uint16_t packetLen = packet->packetLen;
    uint16_t addChecksumLen = (packetLen > kIAP2PacketHeaderLen ? kIAP2PacketChksumLen : 0);

    return (maxPacketLen > (packetLen + addChecksumLen)
            ? maxPacketLen - (packetLen + addChecksumLen)
            : 0);
}


/*
****************************************************************
**
**  iAP2PacketCreateACKPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      payload:    Input buffer containing packet payload data.
**      payloadLen: Size of the Input payload data.
**      session:    session for this payload
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateACKPacket (void*          link,
                                         uint8_t        seq,
                                         uint8_t        ack,
                                         const uint8_t* payload,
                                         uint32_t       payloadLen,
                                         uint8_t        session)
{
    iAP2Packet_t* pck = iAP2PacketCreate (link,
                                          kIAP2PacketControlMaskACK,
                                          seq,
                                          ack,
                                          payload,
                                          payloadLen,
                                          session,
                                          TRUE);
    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreateSYNPacket
**
**  Input:
**      link:           link to associate with this packet
**      initSeq:        Initial Sequence number.
**      ack:            Buffer containing Acknowledgement number to be used.
**                          If not to be used, pass in NULL.
**      maxOutstanding  Max number of outstanding packets (tx window).
**      maxPacketSize   Max Packet size.
**      reTxTimeout     Re-transmit timeout (if ACK not received).
**      cumAckTimeout   Timeout before sending an ACK if no outbound packet.
**      maxRetransmit   Max number of times packet is re-transmitted before error.
**      maxCumAck       Max number of packets received without ACK being sent.
**      numSessionInfo  number of sessions
**      sessionInfo     pointer to buffer containing list of sessions
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateSYNPacket (void*          link,
                                         uint8_t        initSeq,
                                         const uint8_t* ack,
                                         uint8_t        version,
                                         uint8_t        maxOutstanding,
                                         uint16_t       maxPacketSize,
                                         uint16_t       reTxTimeout,
                                         uint16_t       cumAckTimeout,
                                         uint8_t        maxRetransmit,
                                         uint8_t        maxCumAck,
                                         uint8_t        numSessionInfo,
                                         iAP2PacketSessionInfo_t* sessionInfo)
{
    uint8_t buff [kIAP2PacketSynDataBaseLen + kIAP2PacketMaxSessions * sizeof(*sessionInfo)];
    uint8_t control = (kIAP2PacketControlMaskSYN +
                       (ack != NULL ? kIAP2PacketControlMaskACK : 0));
    iAP2Packet_t* pck = NULL;
    int i, j;

    buff [kIAP2PacketSynDataIdxVersion]             = version;
    buff [kIAP2PacketSynDataIdxMaxOutstanding]      = maxOutstanding;
    buff [kIAP2PacketSynDataIdxMaxPacketSize]       = IAP2_HI_BYTE(maxPacketSize);
    buff [kIAP2PacketSynDataIdxMaxPacketSize+1]     = IAP2_LO_BYTE(maxPacketSize);
    buff [kIAP2PacketSynDataIdxRetransmitTimeout]   = IAP2_HI_BYTE(reTxTimeout);
    buff [kIAP2PacketSynDataIdxRetransmitTimeout+1] = IAP2_LO_BYTE(reTxTimeout);;
    buff [kIAP2PacketSynDataIdxCumAckTimeout]       = IAP2_HI_BYTE(cumAckTimeout);
    buff [kIAP2PacketSynDataIdxCumAckTimeout+1]     = IAP2_LO_BYTE(cumAckTimeout);;
    buff [kIAP2PacketSynDataIdxMaxRetransmit]       = maxRetransmit;
    buff [kIAP2PacketSynDataIdxMaxCumACK]           = maxCumAck;
    for (i = 0, j = kIAP2PacketSynDataIdxSessionInfo;
         i < numSessionInfo && i < kIAP2PacketMaxSessions;
         ++i, j += kIAP2PacketSynSessionSize)
    {
        buff [j + kIAP2PacketSynSessionIdxID]       = sessionInfo[i].id;
        buff [j + kIAP2PacketSynSessionIdxType]     = sessionInfo[i].type;
        buff [j + kIAP2PacketSynSessionIdxVersion]  = sessionInfo[i].version;
    }

    pck = iAP2PacketCreate (link,
                            control,
                            initSeq,
                            (ack != NULL ? *ack : 0),
                            buff,
                            kIAP2PacketSynDataBaseLen + numSessionInfo * sizeof(*sessionInfo),
                            kIAP2PacketReservedSessionID,
                            TRUE);
    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreateEAKPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      outOfSeq:       Buffer containing list of out-of-seq seq#'s
**      outOfSeqCount:  Number of out-of-seq seq#'s
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEAKPacket (void*          link,
                                         uint8_t        seq,
                                         uint8_t        ack,
                                         const uint8_t* outOfSeq,
                                         uint32_t       outOfSeqCount)
{
    uint8_t control = (kIAP2PacketControlMaskACK +
                       kIAP2PacketControlMaskEAK);
    iAP2Packet_t* pck = iAP2PacketCreate (link,
                                          control,
                                          seq,
                                          ack,
                                          outOfSeq,
                                          outOfSeqCount,
                                          kIAP2PacketReservedSessionID,
                                          TRUE);
    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreateRSTPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateRSTPacket (void*   link,
                                         uint8_t seq)
{
    uint8_t control = (kIAP2PacketControlMaskRST);
    iAP2Packet_t* pck = iAP2PacketCreate (link,
                                          control,
                                          seq,
                                          0,
                                          NULL,
                                          0,
                                          kIAP2PacketReservedSessionID,
                                          TRUE);
    return pck;
}


/*
****************************************************************
**
**  iAP2PacketCreateSUSPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateSUSPacket (void*   link,
                                         uint8_t seq)
{
    uint8_t control = (kIAP2PacketControlMaskSUS);
    iAP2Packet_t* pck = iAP2PacketCreate (link,
                                          control,
                                          seq,
                                          0,
                                          NULL,
                                          0,
                                          kIAP2PacketReservedSessionID,
                                          TRUE);
    return pck;
}


/*
****************************************************************
**
**  iAP2PacketGenerateBuffer
**
**  Input:
**      packet:     packet to encode into bytes.
**
**  Output:
**      None
**
**  Return:
**      uint8_t*    buffer that contains the encoded bytes of the
**                  packet for transmission
**
****************************************************************
*/
uint8_t* iAP2PacketGenerateBuffer (iAP2Packet_t* packet)
{
    if (packet != NULL)
    {
        uint16_t pcklen = packet->packetLen;

        /*
        ** TODO: If buffer has already been generated (for resend), then
        **       re-calc header checksum based on the one byte (ack) change
        **       and don't bother re-generating whole packet buffer again.
        */

        /* Fill-in header */
        packet->pckData->sop1 = kIAP2PacketSYNC;
        packet->pckData->sop2 = kIAP2PacketSOP;
        packet->pckData->len1 = IAP2_HI_BYTE(pcklen);
        packet->pckData->len2 = IAP2_LO_BYTE(pcklen);

        assert(packet->bufferLen == packet->packetLen);
        if (packet->bufferLen >= pcklen)
        {
            uint8_t* data = iAP2PacketGetPayload (packet);
            uint16_t dataLen = iAP2PacketGetPayloadLen (packet);

            packet->pckData->chk = iAP2PacketCalcChecksum (iAP2PacketGetBuffer (packet),
                                                          kIAP2PacketHeaderLen - 1);

            /* Fill-in payload + payload checksum */
            if (data != NULL && dataLen > 0)
            {
                packet->dataChecksum = iAP2PacketCalcChecksum (data, dataLen);
                data [dataLen] = packet->dataChecksum;
            }
        }
    }
    else
    {
        iAP2LogError("%s:%d NULL packet pointer!\n", __FILE__, __LINE__);
    }

    return iAP2PacketGetBuffer (packet);
}


/*
 ****************************************************************
 **
 **  iAP2PacketGetBuffer
 **
 **  Input:
 **      packet:     packet to encode into bytes.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint8_t*    buffer that contains the encoded bytes of the
 **                  packet for transmission
 **
 ****************************************************************
 */
uint8_t* iAP2PacketGetBuffer (iAP2Packet_t* packet)
{
    return (uint8_t*) (packet ? packet->pckData : NULL);
}


/*
 ****************************************************************
 **
 **  iAP2PacketGetPayload
 **
 **  Input:
 **      packet:     packet to encode into bytes.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint8_t*    pointer to payload data in the buffer that contains the
 **                  encoded bytes of the packet for transmission
 **
 ****************************************************************
 */
uint8_t* iAP2PacketGetPayload (iAP2Packet_t* packet)
{
    return packet->pckData->data;
}


/*
 ****************************************************************
 **
 **  iAP2PacketGetPayloadLen
 **
 **  Input:
 **      packet:     packet to encode into bytes.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint16_t    payload length of the packet
 **
 ****************************************************************
 */
uint16_t iAP2PacketGetPayloadLen (const iAP2Packet_t* packet)
{
    uint16_t payloadLen = 0;
    if (packet->packetLen > kIAP2PacketHeaderLen)
    {
        payloadLen = packet->packetLen - kIAP2PacketHeaderLen - kIAP2PacketChksumLen;
    }
    return payloadLen;
}


/*
****************************************************************
**
**  iAP2PacketParseSYNData
**
**  Input:
**      const uint8_t*  data
**      uint32_t        dataLen
**
**  Output:
**      iAP2PacketSYNData_t* synData
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
void iAP2PacketParseSYNData (const uint8_t*         data,
                             uint32_t               dataLen,
                             iAP2PacketSYNData_t*   synData)
{
    if (kIAP2PacketSynDataBaseLen <= dataLen)
    {
        uint8_t numSessionInfo = (dataLen - kIAP2PacketSynDataBaseLen) / kIAP2PacketSynSessionSize;
        uint8_t i, j;

        synData->version               = data[kIAP2PacketSynDataIdxVersion];
        synData->maxOutstandingPackets = data[kIAP2PacketSynDataIdxMaxOutstanding];
        synData->maxPacketSize         = data[kIAP2PacketSynDataIdxMaxPacketSize];
        synData->maxPacketSize       <<= 8;
        synData->maxPacketSize        += data[kIAP2PacketSynDataIdxMaxPacketSize+1];
        synData->retransmitTimeout     = data[kIAP2PacketSynDataIdxRetransmitTimeout];
        synData->retransmitTimeout   <<= 8;
        synData->retransmitTimeout    += data[kIAP2PacketSynDataIdxRetransmitTimeout+1];
        synData->cumAckTimeout         = data[kIAP2PacketSynDataIdxCumAckTimeout];
        synData->cumAckTimeout       <<= 8;
        synData->cumAckTimeout        += data[kIAP2PacketSynDataIdxCumAckTimeout+1];
        synData->maxRetransmissions    = data[kIAP2PacketSynDataIdxMaxRetransmit];
        synData->maxCumAck             = data[kIAP2PacketSynDataIdxMaxCumACK];
        if (numSessionInfo > kIAP2PacketMaxSessions)
        {
            numSessionInfo = kIAP2PacketMaxSessions;
        }
        for (i = 0, j = kIAP2PacketSynDataIdxSessionInfo;
             i < numSessionInfo && i < kIAP2PacketMaxSessions;
             ++i, j += kIAP2PacketSynSessionSize)
        {
            synData->sessionInfo[i].id      = data[j + kIAP2PacketSynSessionIdxID];
            synData->sessionInfo[i].type    = data[j + kIAP2PacketSynSessionIdxType];
            synData->sessionInfo[i].version = data[j + kIAP2PacketSynSessionIdxVersion];
        }
        synData->numSessionInfo = numSessionInfo;
    }
}


/*
****************************************************************
**
**  iAP2PacketGetMissingSeqFromEAK
**
**  Input:
**      packet:     EAK packet to retreive the missing seq#s for.
**      window:     Max Outstanding Packets as negotiated
**
**  Output:
**      missingLen: pointer to len buffer... contains number of
**                  missing seq#'s returned.
**
**  Return:
**      uint8_t*    array of missing seq#'s (return NULL and 0 len if none)
**                  should be iAP2BuffPoolReturn'd by caller.
**                      iAP2BuffPoolReturn (link->buffPool, result);
**
****************************************************************
*/
uint8_t* iAP2PacketGetMissingSeqFromEAK (iAP2Packet_t* eakPacket,
                                         uint8_t       window,
                                         uint32_t*     missingLen)
{
    uint8_t* result = NULL;
    uint32_t resultLen = 0;
    if (eakPacket != NULL && missingLen != NULL)
    {
        iAP2Link_t* link = eakPacket->link;
        uint8_t* data;
        uint8_t* dataEnd;
        uint8_t  ack;
        int16_t* tmpResult = iAP2BuffPoolGet (link->buffPool, window * sizeof(*tmpResult));
        int16_t* pt;
        int16_t* ptEnd;
        uint8_t  gap;
        uint8_t  maxGap = 0;

        /* prepare a table of ACK values within the window */
        pt = tmpResult;
        ptEnd = tmpResult + window;
        ack = eakPacket->pckData->ack;
        while (pt < ptEnd)
        {
            *(pt++) = (++ack);
        }

        /* mark entries in table that have been received by peer */
        ack = eakPacket->pckData->ack;
        data = iAP2PacketGetPayload (eakPacket);
        dataEnd = data + iAP2PacketGetPayloadLen (eakPacket);
        resultLen = window;
        while (data < dataEnd)
        {
            gap = iAP2PacketCalcSeqGap (ack, *data);
            if (gap > 0 && gap <= window)
            {
#if iAP2PACKET_DEBUG
                iAP2LogDbg("%s:%d EAK ack=%u window=%u gap=%u got seq=%u maxGap=%u resultLen=%u data=%p dataEnd=%p\n",
                           __FILE__, __LINE__, ack, window, gap, *data, maxGap, resultLen, data, dataEnd);
#endif
                if (gap > maxGap)
                {
                    maxGap = gap;
                }
                if (tmpResult [gap - 1] != -1)
                {
                    --resultLen;
                }
                tmpResult [gap - 1] = -1;
            }
            ++data;
        }

        /* prepare a list of packets that need to be sent out */
        pt = tmpResult;
        ptEnd = tmpResult + window;
        resultLen -= (window - maxGap);
        if (resultLen > 0)
        {
            result = iAP2BuffPoolGet (link->buffPool, resultLen);;
        }
        data = result;
        dataEnd = data + resultLen;
#if iAP2PACKET_DEBUG
        {
        int count = 0;
        iAP2LogStart ();
#endif
        gap = 1;
        while (pt < ptEnd && data < dataEnd)
        {
            if (*pt != -1 && gap++ <= maxGap)
            {
#if iAP2PACKET_DEBUG
                iAP2LogDbgNL("%s:%d EAK ack=%u window=%u missing(%d/%d) seq=%u gap=%u maxGap=%u\n",
                             __FILE__, __LINE__, ack, window, ++count, resultLen, *pt, gap, maxGap);
#endif
                *(data++) = *pt;
            }
            ++pt;
        }
#if iAP2PACKET_DEBUG
        iAP2LogStop ();
        }
#endif
        iAP2BuffPoolReturn (link->buffPool, tmpResult);
    }
    if (missingLen != NULL)
    {
        *missingLen = resultLen;
    }
    return result;
}


/*
****************************************************************
**
**  iAP2PacketCalcChecksum
**
**  Input:
**      buffer:     Input buffer containing data to Checksum.
**      bufferLen:  Size of the Input buffer data.
**
**  Output:
**      None
**
**  Return:
**      uint8_t     Calculated checksum for provide buffer
**
****************************************************************
*/
uint8_t iAP2PacketCalcChecksum (const uint8_t*  buffer,
                                uint32_t        bufferLen)
{
    uint8_t checksum = 0;

    if (buffer != NULL && bufferLen > 0)
    {
        const uint8_t* bufferEnd = buffer + bufferLen;

        while (buffer != bufferEnd)
        {
            checksum += *buffer; /* add to checksum */
            buffer++;
        }

        checksum = (uint8_t)(0x100 - checksum); /* 2's complement negative checksum */
    }
    else
    {
        iAP2LogError("%s:%d NULL buffer(%p) pointer or nothing to copy (len=%u)!\n",
                     __FILE__, __LINE__, buffer, bufferLen);
    }

    return checksum;
}


/*
****************************************************************
**
**  iAP2PacketCopyAndCalcChecksum
**
**  Input:
**      dst:    buffer to copy to.
**      src;    source buffer to copy from.
**      len:    Size of the Input buffer data.
**
**  Output:
**      None
**
**  Return:
**      uint8_t     Calculated checksum
**
**  Note: Copies data and calculates the checksum at the same time.
**
****************************************************************
*/
uint8_t iAP2PacketCopyAndCalcChecksum (uint8_t *        dst,
                                       const uint8_t *  src,
                                       uint32_t         len)
{
    uint8_t checksum = 0;

    if (dst != NULL && src != NULL && len > 0)
    {
        const uint8_t*  srcEnd = src + len;

        while (src != srcEnd)
        {
            checksum += *src; /* add to checksum */
            *(dst++) = *(src++);
        }

        checksum = (uint8_t)(0x100 - checksum); /* 2's complement negative checksum */
    }
    else
    {
        iAP2LogError("%s:%d NULL dst(%p) or src(%p) pointer or nothing to copy (len=%u)!\n",
                     __FILE__, __LINE__, dst, src, len);
    }

    return checksum;
}


/*
****************************************************************
**
**  iAP2PacketCalcHeaderChecksum
**
**  Input:
**      pck:    packet to calculate header checksum on.
**
**  Output:
**      pck:    packet header checksum field is filled in with calculated value.
**
**  Return:
**      uint8_t     Calculated header checksum.
**
****************************************************************
*/
uint8_t iAP2PacketCalcHeaderChecksum (iAP2Packet_t* pck)
{
    uint8_t checksum = 0;

    if (pck != NULL)
    {
        uint8_t buff [kIAP2PacketHeaderLen];

        buff [kIAP2PacketIndexSYNC] = kIAP2PacketSYNC;
        buff [kIAP2PacketIndexSOP]  = kIAP2PacketSOP;
        buff [kIAP2PacketIndexLEN1] = IAP2_HI_BYTE(pck->packetLen);
        buff [kIAP2PacketIndexLEN2] = IAP2_LO_BYTE(pck->packetLen);
        buff [kIAP2PacketIndexCTRL] = pck->pckData->ctl;
        buff [kIAP2PacketIndexSEQ]  = pck->pckData->seq;
        buff [kIAP2PacketIndexACK]  = pck->pckData->ack;

        pck->pckData->chk = iAP2PacketCalcChecksum (buff,
                                                   (kIAP2PacketHeaderLen
                                                    - kIAP2PacketChksumLen));
        checksum = pck->pckData->chk;
    }
    else
    {
        iAP2LogError("%s:%d NULL packet pointer!\n", __FILE__, __LINE__);
    }

    return checksum;
}


/*
****************************************************************
**
**  iAP2PacketCalcPayloadChecksum
**
**  Input:
**      pck:    packet to calculate payload checksum on.
**
**  Output:
**      pck:    packet payload checksum field is filled in with calculated value.
**
**  Return:
**      uint8_t     Calculated payload checksum.
**
****************************************************************
*/
uint8_t iAP2PacketCalcPayloadChecksum (iAP2Packet_t* pck)
{
    uint8_t checksum = 0;
    if (pck != NULL)
    {
        pck->dataChecksum = 0;
        if (iAP2PacketGetPayload (pck) != NULL && iAP2PacketGetPayloadLen (pck) > 0)
        {
            pck->dataChecksum = iAP2PacketCalcChecksum (iAP2PacketGetPayload (pck),
                                                        iAP2PacketGetPayloadLen (pck));
        }
        checksum = pck->dataChecksum;
    }
    else
    {
        iAP2LogError("%s:%d NULL packet pointer!\n", __FILE__, __LINE__);
    }
    return checksum;
}


/*
****************************************************************
**
**  iAP2PacketNextSeq
**
**  Input:
**      curSeq: current sequence number
**
**  Output:
**      None
**
**  Return:
**      uint8_t next seqeunce number
**
****************************************************************
*/
uint8_t iAP2PacketNextSeq (uint8_t curSeq)
{
    return (uint8_t) (curSeq + 1);
}


/*
****************************************************************
**
**  iAP2PacketCalcSeqGap
**
**  Input:
**      curSeq: current sequence number
**      next:   next sequence number
**
**  Output:
**      None
**
**  Return:
**      uint8_t returns the gap between the current and next seqeunce numbers
**
****************************************************************
*/
uint8_t iAP2PacketCalcSeqGap (uint8_t curSeq, uint8_t nextSeq)
{
    if (curSeq < nextSeq)
    {
        return nextSeq - curSeq;
    }
    else if (curSeq == nextSeq)
    {
        return 0;
    }

    return nextSeq + (0x100 - curSeq);
}


/*
****************************************************************
**
**  iAP2PacketIsSeqACKd
**
**  Input:
**      seq:    sequence number being checked
**      ack:    sequence number last ACKd
**      window: size of window where packets can be sent without ACK
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if seq# has been ACKd, else FALSE
**
**  Note:   It is expected that seq number is always ahead of ack so
**          if the gap is larger than window, it must have been ACKd
**          already.
**
****************************************************************
*/
BOOL iAP2PacketIsSeqACKd (uint8_t seq, uint8_t ack, uint8_t window)
{
    uint8_t gap = iAP2PacketCalcSeqGap (seq, ack);
    if (gap > window)
    {
        return FALSE;
    }
    return TRUE;
}


/*
****************************************************************
**
**  iAP2PacketRequireACK
**
**  Input:
**      packet: pointer to packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if packet needs an ACK, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketRequireACK (iAP2Packet_t* packet)
{
    /*
    ** Packets that require ACK:
    ** - SYN packet and ACK packets that have data
    */

    if (packet->packetLen > kIAP2PacketHeaderLen &&
        (packet->pckData->ctl & (kIAP2PacketControlMaskACK +
                                kIAP2PacketControlMaskSYN)) != 0 &&
        (packet->pckData->ctl & kIAP2PacketControlMaskEAK) == 0)
    {
        return TRUE;
    }
    return FALSE;
}


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacket
**
**  Input:
**      pck:    packet to to print out info for.
**      file:   file called from
**      line:   line number called from
**      tag:    additional info to display with packet info
**      indent: additional text to add at the beginning of the line
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacket (iAP2Packet_t*  packet,
                                 const char*    file,
                                 int            line,
                                 const char*    tag,
                                 const char*    indent)
{
#if DEBUG
    iAP2LogStart();
    iAP2PacketDebugPrintPacketNL (packet, file, line, tag, indent);
    iAP2LogStop();
#endif
}


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketNL
**
**  Input:
**      pck:    packet to to print out info for.
**      file:   file called from
**      line:   line number called from
**      tag:    additional info to display with packet info
**      indent: additional text to add at the beginning of the line
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacketNL (iAP2Packet_t*  packet,
                                   const char*    file,
                                   int            line,
                                   const char*    tag,
                                   const char*    indent)
{
#if DEBUG
    iAP2LogTypeNL(kiAP2LogTypeData, "%s:%d %sPACKET DEBUG----\n", file, line, indent);
    if (packet->state == kiAP2PacketParseStateDETECT)
    {
        iAP2LogTypeNL(kiAP2LogTypeData,
                      "%s%s%s%s PACKET 'DETECT' packet(%p)\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent, packet);
    }
    else if (packet->state == kiAP2PacketParseStateDETECTBAD)
    {
        iAP2LogTypeNL(kiAP2LogTypeData,
                      "%s%s%s%s PACKET 'DETECT BAD ACK' packet(%p)\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent, packet);
    }
    else
    {
        uint8_t* p;
        uint8_t* pEnd;
        char lineStr[300];
        long lineStrLen;
        uint32_t count;

        iAP2LogTypeNL(kiAP2LogTypeData, "%s%s%s%s PACKET (%s%s%s%s) packet(%p)\n", (tag ? tag : ""), (tag ? ": " : ""), indent,
                      iAP2PacketName (packet),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) ? "SYN " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskACK) ? "ACK " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskEAK) ? "EAK " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskRST) ? "RST " : ""),
                      packet);
        iAP2LogTypeNL(kiAP2LogTypeData, "%s%s%s    state=%s rcvEAK=%d retxCnt=%d packetLen=%u(%xh)\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent,
                      stateName[packet->state],
                      packet->recvEAK,
                      packet->retransmitCount,
                      packet->packetLen, packet->packetLen);
        iAP2LogTypeNL(kiAP2LogTypeData, "%s%s%s    seq=%u(%xh) ack=%u(%xh) control=%xh(%s%s%s%s)\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent,
                      packet->pckData->seq, packet->pckData->seq,
                      packet->pckData->ack, packet->pckData->ack,
                      packet->pckData->ctl,
                      ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) ? "SYN " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskACK) ? "ACK " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskEAK) ? "EAK " : ""),
                      ((packet->pckData->ctl & kIAP2PacketControlMaskRST) ? "RST " : ""));
        iAP2LogTypeNL(kiAP2LogTypeData, "%s%s%s    session=%u hdrChecksum=%xh data(%d/%d)=%ph\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent,
                      packet->pckData->sess, packet->pckData->chk,
                      iAP2PacketGetPayloadLen (packet),
                      packet->dataCurLen,
                      iAP2PacketGetPayload (packet));
        iAP2LogTypeNL(kiAP2LogTypeData, "%s%s%s    dataChecksum=%xh buffer(%u)=%ph\n",
                      (tag ? tag : ""), (tag ? ": " : ""), indent,
                      packet->dataChecksum, packet->bufferLen, iAP2PacketGetBuffer(packet));
        /* Print BUFF */
        p = NULL;
        pEnd = NULL;
        lineStrLen = 0;
        count = 0;
        if (packet->bufferLen > 0)
        {
            char tmpStr[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
            char spaceStr[24] = "                        ";
            p = iAP2PacketGetBuffer(packet);
            pEnd = p + packet->bufferLen;
            sprintf (&lineStr[lineStrLen],
                     "%s%s%s %s[ %04x: ",
                     (tag ? tag : ""), (tag ? ": " : ""), indent,
                     "BUF", count);
            lineStrLen = strlen (lineStr);
            while (p < pEnd)
            {
                if (count && (count & 0x07) == 0)
                {
                    sprintf (&lineStr[lineStrLen],
                             "    %c%c%c%c%c%c%c%c\n",
                             iAP2LogDataChar (tmpStr[0]),
                             iAP2LogDataChar (tmpStr[1]),
                             iAP2LogDataChar (tmpStr[2]),
                             iAP2LogDataChar (tmpStr[3]),
                             iAP2LogDataChar (tmpStr[4]),
                             iAP2LogDataChar (tmpStr[5]),
                             iAP2LogDataChar (tmpStr[6]),
                             iAP2LogDataChar (tmpStr[7]));
                    iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
                    lineStrLen = 0;
                    sprintf (&lineStr[lineStrLen],
                             "%s%s%s      %04x: ",
                             (tag ? tag : ""), (tag ? ": " : ""), indent, count);
                    lineStrLen = strlen (lineStr);
                    memset(tmpStr,' ',8*sizeof(char));
                }
                tmpStr [count & 0x07] = *p;
                sprintf (&lineStr[lineStrLen],
                         "%02X ",  *(p++));
                lineStrLen = strlen (lineStr);
                ++count;
            }
            spaceStr[3*(8-(count & 0x07))] = 0;
            if ((count & 0x07) == 0 && count != 0)
            {
                spaceStr[0] = 0;
            }
            sprintf (&lineStr[lineStrLen],
                     "]   %s%c%c%c%c%c%c%c%c\n",
                     spaceStr,
                     iAP2LogDataChar (tmpStr[0]),
                     iAP2LogDataChar (tmpStr[1]),
                     iAP2LogDataChar (tmpStr[2]),
                     iAP2LogDataChar (tmpStr[3]),
                     iAP2LogDataChar (tmpStr[4]),
                     iAP2LogDataChar (tmpStr[5]),
                     iAP2LogDataChar (tmpStr[6]),
                     iAP2LogDataChar (tmpStr[7]));
            iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
        }
        /* Print DATA */
        if (iAP2PacketGetPayload (packet) != NULL && packet->dataCurLen > 0)
        {
            char tmpStr[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
            char spaceStr[24] = "                        ";
            p = iAP2PacketGetPayload (packet);
            pEnd = p + packet->dataCurLen;
            lineStrLen = 0;
            count = 0;
            sprintf (&lineStr[lineStrLen],
                     "%s%s%s %s[ %04x: ",
                     (tag ? tag : ""), (tag ? ": " : ""), indent,
                     "DAT", count);
            lineStrLen = strlen (lineStr);
            while (p < pEnd)
            {
                if (count && (count & 0x07) == 0)
                {
                    sprintf (&lineStr[lineStrLen],
                             "    %c%c%c%c%c%c%c%c\n",
                             iAP2LogDataChar (tmpStr[0]),
                             iAP2LogDataChar (tmpStr[1]),
                             iAP2LogDataChar (tmpStr[2]),
                             iAP2LogDataChar (tmpStr[3]),
                             iAP2LogDataChar (tmpStr[4]),
                             iAP2LogDataChar (tmpStr[5]),
                             iAP2LogDataChar (tmpStr[6]),
                             iAP2LogDataChar (tmpStr[7]));
                    iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
                    lineStrLen = 0;
                    sprintf (&lineStr[lineStrLen],
                             "%s%s%s      %04x: ",
                             (tag ? tag : ""), (tag ? ": " : ""), indent, count);
                    lineStrLen = strlen (lineStr);
                    memset(tmpStr,' ',8*sizeof(char));
                }
                tmpStr [count & 0x07] = *p;
                sprintf (&lineStr[lineStrLen],
                         "%02X ",  *(p++));
                lineStrLen = strlen (lineStr);
                ++count;
            }
            spaceStr[3*(8-(count & 0x07))] = 0;
            if ((count & 0x07) == 0 && count != 0)
            {
                spaceStr[0] = 0;
            }
            sprintf (&lineStr[lineStrLen],
                     "]   %s%c%c%c%c%c%c%c%c\n",
                     spaceStr,
                     iAP2LogDataChar (tmpStr[0]),
                     iAP2LogDataChar (tmpStr[1]),
                     iAP2LogDataChar (tmpStr[2]),
                     iAP2LogDataChar (tmpStr[3]),
                     iAP2LogDataChar (tmpStr[4]),
                     iAP2LogDataChar (tmpStr[5]),
                     iAP2LogDataChar (tmpStr[6]),
                     iAP2LogDataChar (tmpStr[7]));
            iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
        }
    }
#endif
}


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketShortNL
**
**  Input:
**      packet:     packet to to print out info for.
**      buff:       buffer to write string into
**      buffLen:    size of buff
**
**  Output:
**      buff:   debug output string is written into buff
**
**  Return:
**      char*   pointer to buff
**
****************************************************************
*/
char* iAP2PacketDebugStringPacketShortNL (iAP2Packet_t* packet,
                                          char*         buff,
                                          uint32_t      buffLen)
{
#if DEBUG
    if (packet->state == kiAP2PacketParseStateDETECT)
    {
        snprintf (buff, buffLen, "pck(%p/'DETECT')", packet);
    }
    else if (packet->state == kiAP2PacketParseStateDETECTBAD)
    {
        snprintf (buff, buffLen, "pck(%p/'DETECT BAD ACK')", packet);
    }
    else
    {
        snprintf (buff, buffLen, "pck(%p/ctl=%xh/seq=%u/ack=%u/sess=%u/size=%u(%s%s%s%s)",
                  packet, packet->pckData->ctl, packet->pckData->seq,
                  packet->pckData->ack, packet->pckData->sess,
                  packet->packetLen,
                  ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) ? "SYN " : ""),
                  ((packet->pckData->ctl & kIAP2PacketControlMaskACK) ? "ACK " : ""),
                  ((packet->pckData->ctl & kIAP2PacketControlMaskEAK) ? "EAK " : ""),
                  ((packet->pckData->ctl & kIAP2PacketControlMaskRST) ? "RST " : ""));
    }
#endif
    return buff;
}


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketShortNL
**
**  Input:
**      packet: packet to to print out info for.
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacketShortNL (iAP2Packet_t*  packet)
{
#if DEBUG
    if (packet->state == kiAP2PacketParseStateDETECT)
    {
        iAP2LogTypeNL(kiAP2LogTypeData, "pck(%p/'DETECT')", packet);
    }
    else if (packet->state == kiAP2PacketParseStateDETECTBAD)
    {
        iAP2LogTypeNL(kiAP2LogTypeData, "pck(%p/'DETECT BAD ACK')", packet);
    }
    else
    {
        iAP2LogTypePureNL(kiAP2LogTypeData, "pck(%p/ctl=%xh/seq=%u/ack=%u/sess=%u/size=%u(%s%s%s%s)",
                          packet, packet->pckData->ctl, packet->pckData->seq,
                          packet->pckData->ack, packet->pckData->sess,
                          packet->packetLen,
                          ((packet->pckData->ctl & kIAP2PacketControlMaskSYN) ? "SYN " : ""),
                          ((packet->pckData->ctl & kIAP2PacketControlMaskACK) ? "ACK " : ""),
                          ((packet->pckData->ctl & kIAP2PacketControlMaskEAK) ? "EAK " : ""),
                          ((packet->pckData->ctl & kIAP2PacketControlMaskRST) ? "RST " : ""));
    }
#endif
}


/*
****************************************************************
**
**  iAP2PacketCheckDetect
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if iAP 1.0/2.0 Detect packet detected,
**              else returne FALSE
**
****************************************************************
*/
BOOL iAP2PacketCheckDetect (iAP2Packet_t* pck)
{
    return (kIAP2PacketDetectLEN  == pck->packetLen &&
            kIAP2PacketDetectCTRL == pck->pckData->ctl &&
            kIAP2PacketDetectSEQ  == pck->pckData->seq);
}


/*
****************************************************************
**
**  iAP2PacketCheckDetectNACK
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if iAP 1.0/2.0 Detect BAD ACK packet detected,
**              else returne FALSE
**
****************************************************************
*/
BOOL iAP2PacketCheckDetectNACK (iAP2Packet_t* pck)
{
    return (kIAP2PacketDetectNACKLEN    == pck->packetLen &&
            kIAP2PacketDetectNACKCTRL   == pck->pckData->ctl &&
            kIAP2PacketDetectNACKSEQ    == pck->pckData->seq &&
            kIAP2PacketDetectNACKACK    == pck->pckData->ack &&
            kIAP2PacketDetectNACKSESSID == pck->pckData->sess);
}


/*
****************************************************************
**
**  iAP2PacketAssignTimer
**
**  Input:
**      pck:    packet to delete
**      timer:  timer to assign to packet
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketAssignTimer (iAP2Packet_t* pck, uint16_t timer)
{
    if (pck != NULL)
    {
        if (pck->timer != INVALID_TIMEOUT_ID && pck->timer != timer)
        {
            iAP2TimeCancelTimer (iAP2LinkGetMainTimer(pck->link),
                                 pck->timer);
        }
        pck->timer = timer;
    }
}


/*
****************************************************************
**
**  iAP2PacketRemoveTimer
**
**  Input:
**      pck:        packet to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketRemoveTimer (iAP2Packet_t* pck)
{
    if (pck != NULL)
    {
        if (pck->timer != INVALID_TIMEOUT_ID)
        {
            iAP2TimeCancelTimer(iAP2LinkGetMainTimer(pck->link),
                                pck->timer);
        }
        pck->timer = INVALID_TIMEOUT_ID;
    }
}


/*
****************************************************************
**
**  iAP2PacketName
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      const char* pointer to string containing Name of packet
**          eg. SYN, ACK, DATA, etc.
**
****************************************************************
*/
const char* iAP2PacketName (iAP2Packet_t* packet)
{
    if (packet->pckData->ctl & kIAP2PacketControlMaskSYN)
    {
        if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
        {
            return "SYN-ACK";
        }
        else
        {
            return "SYN";
        }
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskEAK)
    {
        return "EAK";
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskRST)
    {
        return "RST";
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskSUS)
    {
        return "SUS";
    }
    else if (packet->pckData->ctl & kIAP2PacketControlMaskACK)
    {
        if (packet->packetLen > kIAP2PacketHeaderLen)
        {
            return "DATA";
        }
        else
        {
            return "ACK";
        }
    }
    return "UNKNOWN";
}


/*
****************************************************************
**
**  iAP2PacketGetTimeoutID
**
**  Input:
**      pck:        packet to get timeoutID from
**
**  Output:
**      None
**
**  Return:
**      uint16_t    timeoutID associated with the packet
**
****************************************************************
*/
uint16_t iAP2PacketGetTimeoutID (iAP2Packet_t* packet)
{
    uint16_t timeoutID = INVALID_TIMEOUT_ID;
    if (packet != NULL)
    {
        timeoutID = packet->timer;
    }
    return timeoutID;
}



