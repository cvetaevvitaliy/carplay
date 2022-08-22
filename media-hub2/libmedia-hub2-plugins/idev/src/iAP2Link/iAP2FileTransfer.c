/*
 *	File: iAP2FileTransfer.c
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iAP2Defines.h>
#include <iAP2Log.h>
#include <iAP2BuffPool.h>

#include "iAP2FileTransfer.h"
#include "iAP2Link.h"
#include "iAP2Packet.h"


void __iAP2FileTransferDataSentCB (struct iAP2Link_st*  link,
                                   void*                context);

/* Send a Buffer Service Packet */
static void __iAP2FileTransferSendBufferPacket (iAP2FileTransfer_t*          fileXfer,
                                                iAP2FileTransferPacketType_t pckType,
                                                BOOL                         controlOnly)
{
    iAP2LogDbg("%s:%d fileXfer=%p session=%d buffID=%d pckType=%d controlOnly=%d pBuffer=%p buffSize=%d curPos=%p\n",
               __FUNCTION__, __LINE__, fileXfer, fileXfer->session, fileXfer->bufferID,
               pckType, controlOnly, fileXfer->pBuffer, fileXfer->buffSize, fileXfer->pCurPos);

    if (controlOnly)
    {
        uint32_t payloadLen = kiAP2FileTransferHdrBaseLen;
        uint8_t payload[kiAP2FileTransferHdrBaseLen];
        payload [kiAP2FileTransferHdrIdxID]      = fileXfer->bufferID;
        payload [kiAP2FileTransferHdrIdxControl] = pckType;

        iAP2LinkQueueSendData (fileXfer->link,
                               payload,
                               payloadLen,
                               fileXfer->session,
                               NULL,
                               NULL);
    }
    else if (fileXfer->state == kiAP2FileTransferStateSetup)
    {
        uint32_t payloadLen = kiAP2FileTransferHdrSetupBaseLen;
        uint8_t payload[kiAP2FileTransferHdrSetupBaseLen];
        payload [kiAP2FileTransferHdrIdxID]      = fileXfer->bufferID;
        payload [kiAP2FileTransferHdrIdxControl] = kiAP2FileTransferPacketTypeSetup;
        payload [kiAP2FileTransferHdrIdxSetupSize+0] = ((fileXfer->totalSize >> 56) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+1] = ((fileXfer->totalSize >> 48) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+2] = ((fileXfer->totalSize >> 40) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+3] = ((fileXfer->totalSize >> 32) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+4] = ((fileXfer->totalSize >> 24) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+5] = ((fileXfer->totalSize >> 16) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+6] = ((fileXfer->totalSize >> 8) & 0xFF);
        payload [kiAP2FileTransferHdrIdxSetupSize+7] = ((fileXfer->totalSize) & 0xFF);

        iAP2LogDbg("%s:%d Send Buffer Setup, QueueSendData data=%p dataLen=%u fileXfer=%p session=%u\n",
                   __FUNCTION__, __LINE__, payload, payloadLen, fileXfer, fileXfer->session);

        /*
        ** Send setup packet, start data transfer in paused state...
        ** otherside must acknowledge with start first.
        */
        fileXfer->state = kiAP2FileTransferStatePauseSend;
        iAP2LinkQueueSendData (fileXfer->link,
                               payload,
                               payloadLen,
                               fileXfer->session,
                               fileXfer,
                               __iAP2FileTransferDataSentCB);
    }
    else if (fileXfer->state == kiAP2FileTransferStateWaitStatus)
    {
        fileXfer->state = kiAP2FileTransferStateFinishSend;
        if (fileXfer->endCB)
        {
            (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
        }
        iAP2FileTransferCleanup (fileXfer);
    }
    else
    {
        uint64_t dataLen = (fileXfer->buffSize - fileXfer->buffSentSize);
        uint64_t payloadLen = (dataLen + kiAP2FileTransferHdrBaseLen);
        iAP2Packet_t* packet = iAP2PacketCreateEmptySendPacket (fileXfer->link);
        uint8_t* payload = iAP2PacketGetBuffer (packet) + kIAP2PacketHeaderLen;

        packet->pckData->ctl = kIAP2PacketControlMaskACK;
        packet->pckData->sess = fileXfer->session;
        packet->packetLen = kIAP2PacketHeaderLen + payloadLen + kIAP2PacketChksumLen;
        packet->bufferLen = packet->packetLen;
        payload [kiAP2FileTransferHdrIdxID]      = fileXfer->bufferID;
        payload [kiAP2FileTransferHdrIdxControl] = pckType;

        if (fileXfer->sentSize == 0)
        {
            payload [kiAP2FileTransferHdrIdxControl] |= kiAP2FileTransferControlFirst;
        }
        if (payloadLen > iAP2LinkGetMaxPayloadSize (fileXfer->link))
        {
            payloadLen = iAP2LinkGetMaxPayloadSize (fileXfer->link);
            dataLen = (payloadLen - kiAP2FileTransferHdrBaseLen);
            memcpy (&payload [kiAP2FileTransferHdrIdxData],
                    fileXfer->pCurPos,
                    dataLen);
            /* Re-calc packetLen based on new payloadLen */
            packet->packetLen = kIAP2PacketHeaderLen + payloadLen + kIAP2PacketChksumLen;
            packet->bufferLen = packet->packetLen;

            iAP2LogDbg("%s:%d Send Buffer Data Packet, QueueSendDataPacket fileXfer=%p payloadLen/dataLen=%d/%d packet=%p packet->packetLen=%u session=%u sentSize=%u totalSize=%u\n",
                       __FUNCTION__, __LINE__, fileXfer, payloadLen, dataLen, packet, packet->packetLen, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize);

            iAP2LinkQueueSendDataPacket (fileXfer->link,
                                         packet,
                                         fileXfer->session,
                                         fileXfer,
                                         __iAP2FileTransferDataSentCB);
            fileXfer->pCurPos  += dataLen;
            fileXfer->sentSize += dataLen;
            fileXfer->buffSentSize += dataLen;
        }
        else if (dataLen + fileXfer->sentSize < fileXfer->totalSize ||
                 (0 == fileXfer->totalSize && fileXfer->bStream))
        {
            /*
            ** If totalSize is 0 and bStream, we don't know the final
            ** size of the data.
            */

            /*
            ** Final bit of data for buffer
            ** If total size is 0, it is unknown size and we continue until
            */
            memcpy (&payload [kiAP2FileTransferHdrIdxData],
                    fileXfer->pCurPos,
                    dataLen);

            iAP2LogDbg("%s:%d Send Buffer Data (end current buffer), QueueSendData payload=%p payloadLen=%u fileXfer=%p session=%u buffID=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
                       __FUNCTION__, __LINE__, payload, payloadLen,
                       fileXfer, fileXfer->session, fileXfer->bufferID, fileXfer->sentSize,
                       fileXfer->totalSize, fileXfer->endCB, fileXfer->endCBUserInfo);

            iAP2LinkQueueSendDataPacket (fileXfer->link,
                                         packet,
                                         fileXfer->session,
                                         fileXfer,
                                         __iAP2FileTransferDataSentCB);
            fileXfer->pCurPos  += dataLen;
            fileXfer->sentSize += dataLen;
            fileXfer->buffSentSize += dataLen;
            if (fileXfer->endCB)
            {
                (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
            }
        }
        else /* if (dataLen + fileXfer->sentSize >= fileXfer->totalSize) */
        {
            /* Final bit of data, set back to idle */
            payload [kiAP2FileTransferHdrIdxControl] |= kiAP2FileTransferControlLast;
            if (dataLen > 0)
            {
                memcpy (&payload [kiAP2FileTransferHdrIdxData],
                        fileXfer->pCurPos,
                        dataLen);
            }

            iAP2LogDbg("%s:%d Send Buffer Data (final), QueueSendData payload=%p payloadLen=%u fileXfer=%p session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
                       __FUNCTION__, __LINE__, payload, payloadLen,
                       fileXfer, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
                       fileXfer->endCB, fileXfer->endCBUserInfo);

            iAP2LinkQueueSendDataPacket (fileXfer->link,
                                         packet,
                                         fileXfer->session,
                                         fileXfer,
                                         __iAP2FileTransferDataSentCB);
            fileXfer->pCurPos  += dataLen;
            fileXfer->sentSize += dataLen;
            fileXfer->buffSentSize += dataLen;
            fileXfer->state = kiAP2FileTransferStateWaitStatus;
            if (fileXfer->endCB)
            {
                (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
            }
        }
    }
}


/* Callback handler for data/packet sent notification */
void __iAP2FileTransferDataSentCB (struct iAP2Link_st* link,
                                   void*               context)
{
    iAP2FileTransfer_t* fileXfer = (iAP2FileTransfer_t*) context;

    if (link && fileXfer && fileXfer->link == link)
    {
        switch (fileXfer->state)
        {
            case kiAP2FileTransferStateSend:
            {
                __iAP2FileTransferSendBufferPacket (fileXfer,
                                                    kiAP2FileTransferPacketTypeData,
                                                    FALSE);
            }
                break;

            case kiAP2FileTransferStateWaitStatus:
            case kiAP2FileTransferStateSetup:
            case kiAP2FileTransferStateRecv:
            case kiAP2FileTransferStatePauseSend:
            case kiAP2FileTransferStatePauseRecv:
                break;

            case kiAP2FileTransferStateIdle:
            default:
                iAP2LogError ("%s:%d Data send callback with invalid state(%u)!\n",
                              __FILE__, __LINE__, fileXfer->state);
                break;
        }
    }
    else
    {
        iAP2LogError ("%s:%d Data send callback with invalid link(%p)/context(%p) combo\n",
                      __FILE__, __LINE__, link, context);
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferValidateBufferID
**
**  Input:
**      link:           pointer to iap2 link to use
**      session:        service session
**      bufferID:       buffer object identifier (should have the msb set to
**                          indicate direction)
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if bufferID is in the correct direction
**              FALSE if bufferID is in the wrong direction
**
*****************************************************************
*/
BOOL iAP2FileTransferValidateBufferID (iAP2Link_t*  link,
                                       uint8_t      session,
                                       uint8_t      bufferID)
{
    iAP2PacketSessionInfo_t* sessInfo = iAP2LinkGetSessionInfo (link, session);
    if (kIAP2PacketServiceTypeBuffer == sessInfo->type)
    {
        switch (link->type)
        {
            case kiAP2LinkTypeAccessory:
                if ((bufferID & kiAP2FileTransferDirBit) == kiAP2FileTransferDirFromDevice)
                {
                    return TRUE;
                }
                break;
            case kiAP2LinkTypeDevice:
                if ((bufferID & kiAP2FileTransferDirBit) == kiAP2FileTransferDirFromAccessory)
                {
                    return TRUE;
                }
                break;
            default:
                iAP2LogError ("%s:%d invalid bufferID: link=%p(type=%d) session=%d bufferID=%d\n",
                              __FILE__, __LINE__, link, link->type, session, bufferID);
                break;
        }
    }
    return FALSE;
}


/*
*****************************************************************
**
**  iAP2FileTransferCancelSetup
**
**  Input:
**      link:           pointer to iap2 link to use
**      session:        service session
**      bufferID:       buffer object identifier (should have the msb set to
**                          indicate direction)
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferCancelSetup (iAP2Link_t*  link,
                                  uint8_t      session,
                                  uint8_t      bufferID)
{
    uint32_t payloadLen;
    uint8_t payload[2];

    iAP2LogDbg("%s:%d Send Cancel, session=%u bufferID=%xh\n",
               __FUNCTION__, __LINE__, session, bufferID);

    payloadLen = 2;
    payload [kiAP2FileTransferHdrIdxID]      = bufferID;
    payload [kiAP2FileTransferHdrIdxControl] = kiAP2FileTransferPacketTypeCancel;

    iAP2LinkQueueSendData (link,
                           payload,
                           payloadLen,
                           session,
                           NULL,
                           NULL);
}


/*
*****************************************************************
**
**  iAP2FileTransferCreate
**
**  Input:
**      link:           pointer to iap2 link to use
**      session:        service session
**      bufferID:       buffer object identifier (should have the msb set to
**                          indicate direction)
**      callback:       callback to call when buffer received
**      userInfo:       user info to use for callback calls.
**      bRecvAsStream:  callback should be called for every piece of buffer
**                          received.  Called function must either process
**                          all of the received buffer (return true)
**                          or none (return false).
**      fileXferBuff:   pointer to pre-allocated iAP2FileTransfer_t struct buffer.
**                          if NULL, iAP2BuffPoolGet will be used to attempt
**                          to retrieve a buffer.
**
**  Output:
**      None
**
**  Return:
**      iAP2FileTransfer_t*     initialized passed in fileXferBuff or
**                                  new fileTransfer struct
**
*****************************************************************
*/

iAP2FileTransfer_t* iAP2FileTransferCreate (iAP2Link_t*                 link,
                                            uint8_t                     session,
                                            uint8_t                     bufferID,
                                            iAP2FileTransferGotDataCB_t callback,
                                            void*                       userInfo,
                                            BOOL                        bRecvAsStream,
                                            iAP2FileTransfer_t*         fileXferBuff)
{
#if iAP2_LINK_ALLOW_MALLOC != 0
    if (NULL == fileXferBuff)
    {
        fileXferBuff = (iAP2FileTransfer_t*) iAP2BuffPoolGet (link->buffPool,
                                                              sizeof(iAP2FileTransfer_t));
    }
#else
    assert (fileXferBuff);
#endif

    iAP2LogDbg("%s:%d link=%p session=%u bufferID=%xh callback=%p userInfo=%p bRecvAsStream=%d\n",
               __FUNCTION__, __LINE__,
               link, session, bufferID, callback, userInfo, bRecvAsStream);

    fileXferBuff->state           = kiAP2FileTransferStateIdle;
    fileXferBuff->link            = link;
    fileXferBuff->session         = session;
    fileXferBuff->bufferID        = bufferID;
    fileXferBuff->sentSize        = 0;
    fileXferBuff->totalSize       = 0;
    fileXferBuff->pCurPos         = NULL;
    fileXferBuff->pBuffer         = NULL;
    fileXferBuff->buffSize        = 0;
    fileXferBuff->buffSentSize    = 0;
    fileXferBuff->endCB           = NULL;
    fileXferBuff->endCBUserInfo   = NULL;
    fileXferBuff->gotCB           = callback;
    fileXferBuff->gotCBUserInfo   = userInfo;
    fileXferBuff->bStream         = (bRecvAsStream && callback != NULL);
    fileXferBuff->bDeleteBuffOnFinish = FALSE;
    fileXferBuff->bIsReceive      = FALSE;

    return fileXferBuff;
}


/*
*****************************************************************
**
**  iAP2FileTransferDelete
**
**  Input:
**      fileXfer: fileTransfer to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferDelete (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p) pBuffer=%p buffSize=%d\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo, fileXfer->pBuffer, fileXfer->buffSize);

    if (fileXfer)
    {
        iAP2FileTransferCleanup (fileXfer);
#if iAP2_LINK_ALLOW_MALLOC != 0
        iAP2BuffPoolReturn (fileXfer->link->buffPool,
                            fileXfer);
#endif
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferCleanup
**
**  Input:
**      fileXfer: fileTransfer to cleanup transfer
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferCleanup (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p) pBuffer=%p buffSize=%d\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo, fileXfer->pBuffer, fileXfer->buffSize);

    fileXfer->state = kiAP2FileTransferStateIdle;
    if (fileXfer->pBuffer && (fileXfer->bDeleteBuffOnFinish || fileXfer->bIsReceive))
    {
        iAP2BuffPoolReturn (fileXfer->link->buffPool,
                            fileXfer->pBuffer);
    }
    fileXfer->pBuffer         = NULL;
    fileXfer->buffSize        = 0;
    fileXfer->buffSentSize    = 0;
    fileXfer->pCurPos         = NULL;
    fileXfer->totalSize       = 0;
    fileXfer->sentSize        = 0;
    fileXfer->endCB           = NULL;
    fileXfer->endCBUserInfo   = NULL;
    fileXfer->bIsReceive      = FALSE;
}


/*
*****************************************************************
**
**  iAP2FileTransferStart
**
**  Input:
**      fileXfer:       fileTransfer to stop transfer on
**      buff:           buffer to transfer
**      buffLen:        length of the buffer
**      callback:       callback to call when transfer ends
**      userInfo:       user info to use for callback calls.
**      bSendAsStream:  callback should be called for every piece of buffer
**                          sent.  Called function must either process
**                          all of the received buffer (return TRUE)
**                          or none (return FALSE).
**      bDeleteBuffOnFinish:    delete the buff when transfer finishes.
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if data was serviced and can be discarded, else FALSE
**
**  Note:
**  If there is already a transfer in progress this this fileXfer
**  session and bufferID, the call will fail and return FALSE.
**  setupData is copied and is safe to delete after returning from the call.
**  buff is used as is by iAP2FileTransfer and released when send finishes,
**      caller should not release the buff after the call.
**  If bDeleteBuffOnFinish is true, iAP2FileTransfer will release the buffer
**  buffer upon end of sending the buffer data.
**
*****************************************************************
*/

BOOL iAP2FileTransferStart (iAP2FileTransfer_t*     fileXfer,
                            uint8_t*                buff,
                            uint32_t                buffLen,
                            uint32_t                totalLen,
                            iAP2FileTransferEndCB_t callback,
                            void*                   userInfo,
                            BOOL                    bSendAsStream,
                            BOOL                    bDeleteBuffOnFinish)
{
    BOOL result = FALSE;

    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u buff=%p buffLen=%d totalLen=%d callback=%p userInfo=%p bSendAsStream=%d bDeleteBuffOnFinish=%d\n",
               __FUNCTION__, __LINE__, fileXfer, fileXfer->state, fileXfer->session,
               buff, buffLen, totalLen, callback, userInfo, bSendAsStream, bDeleteBuffOnFinish);

    if (fileXfer && fileXfer->state == kiAP2FileTransferStateIdle)
    {
        uint32_t payloadLen = kiAP2FileTransferHdrSetupBaseLen;

        /* Assume setup data will fit in one packet payload */
        assert (payloadLen <= iAP2LinkGetMaxPayloadSize (fileXfer->link));
        fileXfer->state         = kiAP2FileTransferStateSetup;
        fileXfer->totalSize     = totalLen;
        fileXfer->pBuffer       = buff;
        fileXfer->pCurPos       = fileXfer->pBuffer;
        fileXfer->buffSize      = buffLen;
        fileXfer->buffSentSize  = 0;
        fileXfer->endCB         = callback;
        fileXfer->endCBUserInfo = userInfo;
        fileXfer->bStream       = (bSendAsStream && callback != NULL);;
        fileXfer->bDeleteBuffOnFinish = bDeleteBuffOnFinish;
        fileXfer->bIsReceive    = FALSE;
        iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Start, send Setup, buff=%p len=%u totalSize=%u bStream=%d bDeleteBuffOnFinish=%d\n",
                    __FILE__, __LINE__,
                    fileXfer,
                    fileXfer->bufferID,
                    fileXfer->pBuffer,
                    fileXfer->buffSize,
                    fileXfer->totalSize,
                    fileXfer->bStream,
                    fileXfer->bDeleteBuffOnFinish);
        __iAP2FileTransferSendBufferPacket (fileXfer,
                                            kiAP2FileTransferPacketTypeSetup,
                                            FALSE);
        result = TRUE;
    }

    return result;
}


/*
*****************************************************************
**
**  iAP2FileTransferSendNext
**
**  Input:
**      fileXfer:       fileTransfer to stop transfer on
**      buff:           buffer to transfer
**      buffLen:        length of the buffer
**      bDeleteBuffOnFinish:    delete the buff when transfer finishes.
**      bFinal:         this is the final buffer for this file transfer
**                          should only set if totalLen was unknown (0)
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if data was serviced and can be discarded, else FALSE
**
**  Note:
**      Called in response to callback during send buffer operation
**      with bSendAsStream set to TRUE.
**      Allows the client to setup the next piece of buffer to send.
**      If bDeleteBuffOnFinish is true, iAP2FileTransfer will release the buffer
**      buffer upon end of sending the buffer data.
**
*****************************************************************
*/

void iAP2FileTransferSendNext (iAP2FileTransfer_t* fileXfer,
                               uint8_t*            buff,
                               uint32_t            buffLen,
                               BOOL                bDeleteBuffOnFinish,
                               BOOL                bFinal)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u buff=%p buffLen=%d bDeleteBuffOnFinish=%d bFinal=%d\n",
               __FUNCTION__, __LINE__, fileXfer, fileXfer->state, fileXfer->session,
               buff, buffLen, bDeleteBuffOnFinish, bFinal);

    if (fileXfer->state == kiAP2FileTransferStateSend ||
        fileXfer->state == kiAP2FileTransferStatePauseSend)
    {
        if (fileXfer->pBuffer && fileXfer->bDeleteBuffOnFinish)
        {
            iAP2BuffPoolReturn (fileXfer->link->buffPool,
                                fileXfer->pBuffer);
        }
        fileXfer->pBuffer       = buff;
        fileXfer->pCurPos       = fileXfer->pBuffer;
        fileXfer->buffSize      = buffLen;
        fileXfer->buffSentSize  = 0;
        fileXfer->bDeleteBuffOnFinish = bDeleteBuffOnFinish;
        if (bFinal)
        {
            fileXfer->totalSize = fileXfer->sentSize + fileXfer->buffSize;
        }

        iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Next piece to send buff=%p len=%p sent=%u/%u bDeleteBuffOnFinish=%d\n",
                    __FILE__, __LINE__,
                    fileXfer,
                    fileXfer->bufferID,
                    fileXfer->pBuffer,
                    fileXfer->buffSize,
                    fileXfer->sentSize,
                    fileXfer->totalSize,
                    fileXfer->bDeleteBuffOnFinish);
    }
    else
    {
        iAP2LogError("%s:%d fileXfer=%p buffID=0x%X Wrong state to send buff=%p len=%p sent=%u/%u\n",
                     __FILE__, __LINE__,
                     fileXfer,
                     fileXfer->bufferID,
                     fileXfer->pBuffer,
                     fileXfer->buffSize,
                     fileXfer->sentSize,
                     fileXfer->totalSize);
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferHandleRecv
**
**  Input:
**      fileXfer:       fileTransfer to stop transfer on
**      data:           received data buffer
**      dataLen:        length of the received data buffer
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if finished transfer and fileXfer can be deleted.
**              FALSE if not finished transfer yet.
**
**  Note:
**      When receive finished, fileXfer->state will be set to
**      one of:
**          kiAP2FileTransferStateFinishRecv
**              - successfully finished receive... data is in fileXfer->pBuffer
**                  with setup info in fileXfer->pSetupData
**          kiAP2FileTransferStateCancelRecv
**              - transfer was halted, caller must call iAP2FileTransferCleanup
**                  to cleanup the transfer state.
*****************************************************************
*/

BOOL iAP2FileTransferHandleRecv (iAP2FileTransfer_t*    fileXfer,
                                 const uint8_t*         data,
                                 uint32_t               dataLen)
{
    BOOL needDelete = FALSE;
    if (fileXfer && data && dataLen >= kiAP2FileTransferHdrBaseLen)
    {
        uint8_t control = data [kiAP2FileTransferHdrIdxControl];
        BOOL    isFirst = ((control & kiAP2FileTransferControlFirst) != 0);
        BOOL    isLast  = ((control & kiAP2FileTransferControlLast) != 0);
        iAP2FileTransferPacketType_t packetType = (control & kiAP2FileTransferControlType);

        switch (packetType)
        {
            case kiAP2FileTransferPacketTypeData:
            {
                uint64_t actualBuffLen = (dataLen - kiAP2FileTransferHdrBaseLen);

                /* Handle First packet */
                if (isFirst)
                {
                    iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X First packet, totalSize=%u\n",
                                __FILE__, __LINE__,
                                fileXfer, fileXfer->bufferID, fileXfer->totalSize);
                    fileXfer->sentSize = 0;
                    if (fileXfer->pBuffer != NULL)
                    {
                        free (fileXfer->pBuffer);
                        fileXfer->pBuffer = NULL;
                        fileXfer->pCurPos = NULL;
                        fileXfer->buffSize = 0;
                        fileXfer->buffSentSize = 0;
                    }
                    if (fileXfer->bStream &&
                        (fileXfer->totalSize == 0 ||
                         fileXfer->totalSize > actualBuffLen))
                    {
                        fileXfer->buffSize = actualBuffLen;
                    }
                    else
                    {
                        fileXfer->buffSize = fileXfer->totalSize;
                    }
                    fileXfer->pBuffer = (uint8_t*) iAP2BuffPoolGet (fileXfer->link->buffPool,
                                                                    (uintptr_t) fileXfer->buffSize);
                    fileXfer->pCurPos = fileXfer->pBuffer;
                }
                else if (fileXfer->bStream &&
                         (fileXfer->totalSize == 0 ||
                          fileXfer->totalSize > actualBuffLen))
                {
                    if (fileXfer->pBuffer != NULL)
                    {
                        iAP2BuffPoolReturn  (fileXfer->link->buffPool,
                                             fileXfer->pBuffer);
                        fileXfer->pBuffer = NULL;
                        fileXfer->pCurPos = NULL;
                        fileXfer->buffSize = 0;
                        fileXfer->buffSentSize = 0;
                    }
                    fileXfer->pBuffer = (uint8_t*) iAP2BuffPoolGet (fileXfer->link->buffPool,
                                                                    (uintptr_t) actualBuffLen);
                    fileXfer->buffSize = actualBuffLen;
                    fileXfer->pCurPos = fileXfer->pBuffer;
                }

                /* Handle received data */
                if (fileXfer->totalSize != 0 &&
                    actualBuffLen > (fileXfer->totalSize - fileXfer->sentSize))
                {
                    iAP2LogError ("%s:%d Got too much data for fileXfer=%p, buffID=0x%X expected %u bytes, got %u\n",
                                  __FILE__, __LINE__,
                                  fileXfer,
                                  fileXfer->bufferID,
                                  fileXfer->totalSize,
                                  fileXfer->sentSize + dataLen - kiAP2FileTransferHdrBaseLen);
                    actualBuffLen = (fileXfer->totalSize - fileXfer->sentSize);
                }
                memcpy (fileXfer->pCurPos,
                        &data [kiAP2FileTransferHdrIdxData],
                        actualBuffLen);
                fileXfer->pCurPos += actualBuffLen;
                fileXfer->sentSize += actualBuffLen;
                fileXfer->buffSentSize += actualBuffLen;
                iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Recv'd bytes %u/%u\n",
                            __FILE__, __LINE__,
                            fileXfer,
                            fileXfer->bufferID,
                            fileXfer->sentSize,
                            fileXfer->totalSize);

                /* Handle Last packet */
                if (isLast)
                {
                    iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Last packet (gotCB=%p userInfo=%p) pBuffer=%p buffSize=%d\n",
                                __FILE__, __LINE__,
                                fileXfer, fileXfer->bufferID,
                                fileXfer->gotCB, fileXfer->gotCBUserInfo,
                                fileXfer->pBuffer, fileXfer->buffSize);
                    fileXfer->state = kiAP2FileTransferStateFinishRecv;
                    if (fileXfer->gotCB)
                    {
                        /*
                        ** gotCB is responsible for sending Success/Failure by calling
                        ** iAP2FileTransferSuccess or iAP2FileTransferFailure
                        */
                        if ((fileXfer->gotCB) (fileXfer, fileXfer->gotCBUserInfo))
                        {
                            /* Client processed the buffer, adjust. */
                            if (fileXfer->pBuffer)
                            {
                                free (fileXfer->pBuffer);
                            }
                            fileXfer->pBuffer = NULL;
                            fileXfer->buffSize = 0;
                            fileXfer->buffSentSize = 0;
                            fileXfer->pCurPos = fileXfer->pBuffer;
                        }
                    }
                    else
                    {
                        /* No where to go... just assume success */
//                        iAP2FileTransferSuccess (fileXfer);
                    }
                    iAP2FileTransferSuccess (fileXfer);
                    needDelete = TRUE;
                }
                else if (fileXfer->bStream && fileXfer->gotCB)
                {
                    /*
                    ** not last buffer but client wants to be notified for each
                    ** piece of buffer received.
                    */
                    if ((fileXfer->gotCB) (fileXfer, fileXfer->gotCBUserInfo))
                    {
                        /* Client processed the buffer, adjust. */
                        free (fileXfer->pBuffer);
                        fileXfer->pBuffer = NULL;
                        fileXfer->buffSize = 0;
                        fileXfer->buffSentSize = 0;
                        fileXfer->pCurPos = fileXfer->pBuffer;
                    }
                }
            }
                break;

            case kiAP2FileTransferPacketTypeSetup:
            {
                if (dataLen >= kiAP2FileTransferHdrSetupBaseLen)
                {
                    uint32_t size;

                    fileXfer->bIsReceive = TRUE;

                    iAP2LogStart();
                    iAP2LogPrintData (data, dataLen, "Buffer Setup",
                                      "%s:%d fileXfer=%p buffID=0x%X Setup\n",
                                      __FILE__, __LINE__,
                                      fileXfer, fileXfer->bufferID);
                    iAP2LogStop();

                    /* Setup Packet */
                    size = 0;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+0];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+1];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+2];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+3];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+4];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+5];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+6];
                    size <<= 8;
                    size += data [kiAP2FileTransferHdrIdxSetupSize+7];

                    iAP2FileTransferCleanup (fileXfer);

                    fileXfer->totalSize = size;
                    iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Setup size=%u, send Resume\n",
                                __FILE__, __LINE__,
                                fileXfer, fileXfer->bufferID, fileXfer->totalSize);

                    /*
                    ** Start transmission by sending START to sender... it is in
                    ** PAUSE state after sending SETUP.
                    */
                    fileXfer->state = kiAP2FileTransferStatePauseRecv;
                    iAP2FileTransferResume (fileXfer);
                }
            }
                break;

            case kiAP2FileTransferPacketTypeStart:
            {
                iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Control Start curState=%d\n",
                            __FILE__, __LINE__,
                            fileXfer, fileXfer->bufferID, fileXfer->state);
                if (fileXfer->state == kiAP2FileTransferStatePauseRecv)
                {
                    fileXfer->state = kiAP2FileTransferStateRecv;
                }
                else if (fileXfer->state == kiAP2FileTransferStatePauseSend ||
                         fileXfer->state == kiAP2FileTransferStateSetup)
                {
                    fileXfer->state = kiAP2FileTransferStateSend;
                    __iAP2FileTransferSendBufferPacket (fileXfer,
                                                        kiAP2FileTransferPacketTypeData,
                                                        FALSE);
                }
            }
                break;

            case kiAP2FileTransferPacketTypePause:
            {
                iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Control Pause curState=%d\n",
                            __FILE__, __LINE__,
                            fileXfer, fileXfer->bufferID, fileXfer->state);
                if (fileXfer->state == kiAP2FileTransferStateRecv)
                {
                    fileXfer->state = kiAP2FileTransferStatePauseRecv;
                }
                else if (fileXfer->state == kiAP2FileTransferStateSend)
                {
                    fileXfer->state = kiAP2FileTransferStatePauseSend;
                    /* Send Pause status to receiver */
                    __iAP2FileTransferSendBufferPacket (fileXfer,
                                                        kiAP2FileTransferPacketTypePause,
                                                        TRUE);
                }
            }
                break;

            case kiAP2FileTransferPacketTypeCancel:
            {
                iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Control Cancel curState=%d (endCB=%p userInfo=%p) pBuffer=%p buffSize=%d\n",
                            __FILE__, __LINE__,
                            fileXfer, fileXfer->bufferID, fileXfer->state,
                            fileXfer->endCB, fileXfer->endCBUserInfo,
                            fileXfer->pBuffer, fileXfer->buffSize);
                if (fileXfer->state == kiAP2FileTransferStateSend ||
                    fileXfer->state == kiAP2FileTransferStatePauseSend)
                {
                    /* Send Stop status to receiver */
                    __iAP2FileTransferSendBufferPacket (fileXfer,
                                                        kiAP2FileTransferPacketTypeCancel,
                                                        TRUE);
                    fileXfer->state = kiAP2FileTransferStateCancelSend;
                    if (fileXfer->endCB)
                    {
                        (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
                    }
                    needDelete = TRUE;
                }
                if (fileXfer->state == kiAP2FileTransferStateRecv ||
                    fileXfer->state == kiAP2FileTransferStatePauseRecv)
                {
                    fileXfer->state = kiAP2FileTransferStateCancelRecv;
                    if (fileXfer->endCB)
                    {
                        (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
                    }
                    needDelete = TRUE;
                }
            }
                break;

            case kiAP2FileTransferPacketTypeSuccess:
            {
                if (fileXfer->state == kiAP2FileTransferStateWaitStatus)
                {
                    fileXfer->state = kiAP2FileTransferStateFinishSend;
                    if (fileXfer->endCB)
                    {
                        (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
                    }
                    needDelete = TRUE;
                }
            }
                break;

            case kiAP2FileTransferPacketTypeFailure:
            {
                if (fileXfer->state == kiAP2FileTransferStateWaitStatus)
                {
                    fileXfer->state = kiAP2FileTransferStateFailSend;
                    if (fileXfer->endCB)
                    {
                        (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
                    }
                    needDelete = TRUE;
                }
            }
                break;

            default:
                iAP2LogError("%s:%d fileXfer=%p buffID=0x%X Invalid packet type! %u(0x%x) pBuffer=%p buffSize=%d\n",
                            __FILE__, __LINE__, fileXfer, fileXfer->bufferID, packetType, packetType,
                             fileXfer->pBuffer, fileXfer->buffSize);
                break;
        } /* switch */
    }
    return needDelete;
}


/*
*****************************************************************
**
**  iAP2FileTransferCancel
**
**  Input:
**      fileXfer: fileTransfer to stop transfer on
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferCancel (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo);

    if (fileXfer)
    {
        if (fileXfer->state == kiAP2FileTransferStateSend ||
            fileXfer->state == kiAP2FileTransferStatePauseSend ||
            fileXfer->state == kiAP2FileTransferStateRecv ||
            fileXfer->state == kiAP2FileTransferStatePauseRecv)
        {
            iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Send Control Cancel\n",
                        __FILE__, __LINE__,
                        fileXfer, fileXfer->bufferID);
            /* Send Stop status/command to receiver/sender */
            __iAP2FileTransferSendBufferPacket (fileXfer,
                                                kiAP2FileTransferPacketTypeCancel,
                                                TRUE);
            if (fileXfer->state == kiAP2FileTransferStateSend ||
                fileXfer->state == kiAP2FileTransferStatePauseSend)
            {
                fileXfer->state = kiAP2FileTransferStateCancelSend;
            }
            else
            {
                fileXfer->state = kiAP2FileTransferStateCancelRecv;
            }
            if (fileXfer->endCB)
            {
                (fileXfer->endCB) (fileXfer, fileXfer->endCBUserInfo);
            }
        }
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferPause
**
**  Input:
**      fileXfer: fileTransfer to pause transfer on
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferPause (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo);

    if (fileXfer)
    {
        if (fileXfer->state == kiAP2FileTransferStateSend ||
            fileXfer->state == kiAP2FileTransferStateRecv)
        {
            uint32_t payloadLen = kiAP2FileTransferHdrBaseLen;
            uint8_t payload[kiAP2FileTransferHdrBaseLen];

            iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Send Control Pause\n",
                        __FILE__, __LINE__,
                        fileXfer, fileXfer->bufferID);

            /* Send Pause status/command to receiver/sender */
            payload [kiAP2FileTransferHdrIdxID]      = fileXfer->bufferID;
            payload [kiAP2FileTransferHdrIdxControl] = kiAP2FileTransferPacketTypePause;
            iAP2LinkQueueSendData (fileXfer->link,
                                   payload,
                                   payloadLen,
                                   fileXfer->session,
                                   NULL,
                                   NULL);
            if (fileXfer->state == kiAP2FileTransferStateSend)
            {
                fileXfer->state = kiAP2FileTransferStatePauseSend;
            }
            else
            {
                fileXfer->state = kiAP2FileTransferStatePauseRecv;
            }
        }
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferResume
**
**  Input:
**      fileXfer: fileTransfer to resume transfer on
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferResume (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo);

    if (fileXfer)
    {
        if (fileXfer->state == kiAP2FileTransferStatePauseSend)
        {
            iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Resume send\n",
                        __FILE__, __LINE__,
                        fileXfer, fileXfer->bufferID);
            /* Send Resume(not pause) status to receiver */
            fileXfer->state = kiAP2FileTransferStateSend;
            __iAP2FileTransferSendBufferPacket (fileXfer,
                                                kiAP2FileTransferPacketTypeData,
                                                FALSE);
        }
        else if (fileXfer->state == kiAP2FileTransferStatePauseRecv)
        {
            iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Resume, send Control Start\n",
                        __FILE__, __LINE__,
                        fileXfer, fileXfer->bufferID);
            /* Send Resume(not pause) commmand to sender */
            fileXfer->state = kiAP2FileTransferStateRecv;
            __iAP2FileTransferSendBufferPacket (fileXfer,
                                                kiAP2FileTransferPacketTypeStart,
                                                TRUE);
        }
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferSuccess
**
**  Input:
**      fileXfer: fileTransfer to notify successful transfer on
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferSuccess (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p) pBuffer=%p buffSize=%d\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo, fileXfer->pBuffer, fileXfer->buffSize);

    if (fileXfer && fileXfer->state == kiAP2FileTransferStateFinishRecv)
    {
        iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Successful transfer, send Control Success\n",
                    __FILE__, __LINE__,
                    fileXfer, fileXfer->bufferID);
        __iAP2FileTransferSendBufferPacket (fileXfer,
                                            kiAP2FileTransferPacketTypeSuccess,
                                            TRUE);
    }
}


/*
*****************************************************************
**
**  iAP2FileTransferFailure
**
**  Input:
**      fileXfer: fileTransfer to notify failed transfer on
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/

void iAP2FileTransferFailure (iAP2FileTransfer_t* fileXfer)
{
    iAP2LogDbg("%s:%d fileXfer=%p state=%d session=%u sentSize=%u totalSize=%u (endCB=%p userInfo=%p)\n",
               __FUNCTION__, __LINE__,
               fileXfer, fileXfer->state, fileXfer->session, fileXfer->sentSize, fileXfer->totalSize,
               fileXfer->endCB, fileXfer->endCBUserInfo);

    if (fileXfer &&
        (fileXfer->state == kiAP2FileTransferStateRecv ||
         fileXfer->state == kiAP2FileTransferStatePauseRecv ||
         fileXfer->state == kiAP2FileTransferStateCancelRecv ||
         fileXfer->state == kiAP2FileTransferStateFinishRecv))
    {
        iAP2LogDbg ("%s:%d fileXfer=%p buffID=0x%X Failed transfer, send Control Failure\n",
                    __FILE__, __LINE__,
                    fileXfer, fileXfer->bufferID);
        __iAP2FileTransferSendBufferPacket (fileXfer,
                                            kiAP2FileTransferPacketTypeFailure,
                                            TRUE);
    }
}

