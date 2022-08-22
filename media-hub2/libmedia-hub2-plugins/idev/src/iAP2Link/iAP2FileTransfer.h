/*
 *	File: iAP2FileTransfer.h
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

#ifndef iAP2Link_iAP2FileTransfer_h
#define iAP2Link_iAP2FileTransfer_h

#include <stdint.h>

#include <iAP2Defines.h>

#include "iAP2Link.h"

/*
*******************************************************
**
**  iAP2FileTransfer
**
**  USAGE:
**
**  Send Buffer:
**  0) Generate a new BufferID - done outside iAP2FileTransfer
**  1) Call iAP2FileTransferCreate to create FileTransfer object.
**  2) Call iAP2FileTransferStart to start sending a buffer.
**  3) if a callback is registered with iAP2FileTransferStart, it will
**      be called with the SerivceBuffer object state set to one of
**          kiAP2FileTransferStateFinishSend - successfully sent
**      or
**          kiAP2FileTransferStateFailSend - send failed
**      or
**          kiAP2FileTransferStateCancelSend - transfer cancelled.
**  Note: For send, setting bSendAsStream to true indicates that
**        the client will provide the next pieces of buffer to be sent
**        as they previous ones are transmitted.
**        Client shall call iAP2FileTransferSendNext to setup the next
**        piece of the buffer to transfer.
**        Each buffer pieace must not be larger than maxPayloadSize.
**
**  Receive Buffer:
**  0) Retrieve BufferID used by sender - done outside iAP2FileTransfer
**  1) Call iAP2FileTransferCreate to create FileTransfer object for this
**      transfer with bufferID from the sender.
**  2) Call iAP2FileTransferHandleRecv to handle incoming received data.
**  3) When successfullly received a buffer, callback will be called and
**      FileTransfer object state will be set to either
**          kiAP2FileTransferStateFinishRecv - successfully received
**      or
**          kiAP2FileTransferStateCancelRecv - transfer cancelled
**  4a) If transfer finished (FinishRecv or StopRecv) then call
**      iAP2FileTransferCleanup to cleanup and reset the FileTransfer.
**  Note: For receive, setting bRecvAsStream to true indicates that
**        the callback should be called for every buffer received and
**        not just at the end.
**  Note: If the bufferID received for Setup is in the wrong direction
**        or not setup yet, then a Cancel will be sent back to the sender.
**        direction verification can be done via iAP2FileTransferValidateBufferID().
**        Cancel can be generated with just link, session, and bufferID by
**        calling iAP2FileTransferCancelSetup().
**
**  Buffer Data consists of following common header:
**
**  Bit     7       6       5       4       3       2       1       0
**      +-------+-------+-------+-------+-------+-------+-------+-------+
**      |  Dir  |                    Buffer Object ID                   |
**      +-------+-------+-------+-------+-------+-------+-------+-------+
**      |  1st  | Last  |   0   |   0   |         Packet Type           |
**      +-------+-------+-------+-------+-------+-------+-------+-------+
**
**  Dir  -  0 = from Accessory,
**          1 = from Device
**  1st  -  0 = not first buffer data packet,
**          1 = first buffer data packet
**  Last -  0 = not last buffer data packet,
**          1 = last buffer data packet
**
**  1st and Last set indicates one packet contains all of the Buffer Object Data
**
**  Setup packet consists of 4 bytes of buffer length.
**
*******************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

#define IAP2_SERV_BUFF_HI_BYTE(X) (((X) >> 8) & 0xFF)
#define IAP2_SERV_BUFF_LO_BYTE(X) ((X) & 0xFF)

typedef enum
{
    kiAP2FileTransferStateIdle = 0,
    kiAP2FileTransferStateSetup,
    kiAP2FileTransferStateSend,
    kiAP2FileTransferStatePauseSend,
    kiAP2FileTransferStateCancelSend,
    kiAP2FileTransferStateWaitStatus,
    kiAP2FileTransferStateFinishSend,
    kiAP2FileTransferStateFailSend,
    kiAP2FileTransferStateRecv,
    kiAP2FileTransferStatePauseRecv,
    kiAP2FileTransferStateCancelRecv,
    kiAP2FileTransferStateFinishRecv,
    kiAP2FileTransferStateFailRecv

} iAP2FileTransferState_t;

typedef enum
{
    kiAP2FileTransferControlFirst  = 0x80,
    kiAP2FileTransferControlLast   = 0x40,
    kiAP2FileTransferControlType   = 0x0F

} iAP2FileTransferControl_t;

typedef enum
{
    kiAP2FileTransferPacketTypeData    = 0x00,
    kiAP2FileTransferPacketTypeStart   = 0x01,
    kiAP2FileTransferPacketTypeCancel  = 0x02,
    kiAP2FileTransferPacketTypePause   = 0x03,
    kiAP2FileTransferPacketTypeSetup   = 0x04,
    kiAP2FileTransferPacketTypeSuccess = 0x05,
    kiAP2FileTransferPacketTypeFailure = 0x06

} iAP2FileTransferPacketType_t;

enum
{
    kiAP2FileTransferHdrIdxID          = 0,
    kiAP2FileTransferHdrIdxControl     = 1,
    kiAP2FileTransferHdrBaseLen        = 2,
    kiAP2FileTransferHdrIdxData        = 2,

    /* Setup */
    kiAP2FileTransferHdrIdxSetupSize   = 2,
    kiAP2FileTransferHdrSetupBaseLen   = 10,

    /* Direction */
    kiAP2FileTransferDirBit            = 0x80,
    kiAP2FileTransferDirFromDevice     = 0x80,
    kiAP2FileTransferDirFromAccessory  = 0x00,
    kiAP2FileTransferNonDirection      = 0x7F
};

struct iAP2FileTransfer_st;

/* Callback to indicate send has finished/cancelled */
typedef BOOL (*iAP2FileTransferEndCB_t) (struct iAP2FileTransfer_st* fileXfer, void* userInfo);

/* Callback to indicate data received */
typedef BOOL (*iAP2FileTransferGotDataCB_t) (struct iAP2FileTransfer_st* fileXfer, void* userInfo);

typedef struct iAP2FileTransfer_st
{
    iAP2Link_t* link;

    uint8_t*    pCurPos;
    uint8_t*    pBuffer;

    uint64_t    sentSize;
    uint64_t    totalSize;
    uint64_t    buffSize;
    uint64_t    buffSentSize;

    iAP2FileTransferEndCB_t     endCB;
    void*                       endCBUserInfo;
    iAP2FileTransferGotDataCB_t gotCB;
    void*                       gotCBUserInfo;

    iAP2FileTransferState_t state;
    uint8_t     session;
    uint8_t     bufferID;

    BOOL        bDeleteBuffOnFinish:1;
    BOOL        bStream:1;
    BOOL        bIsReceive:1;

} iAP2FileTransfer_t;



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
                                       uint8_t      bufferID);


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
                                  uint8_t      bufferID);


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
**                          all of the received buffer (return TRUE)
**                          or none (return FALSE).
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
                                            iAP2FileTransfer_t*         fileXferBuff);


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
void iAP2FileTransferDelete (iAP2FileTransfer_t* fileXfer);


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
void iAP2FileTransferCleanup (iAP2FileTransfer_t* fileXfer);


/*
*****************************************************************
**
**  iAP2FileTransferStart
**
**  Input:
**      fileXfer:       fileTransfer to stop transfer on
**      buff:           buffer to transfer
**      buffLen:        length of the buffer
**      totalLen:       total length of data to be transferred
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
                            BOOL                    bDeleteBuffOnFinish);


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
**      None
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
void iAP2FileTransferSendNext (iAP2FileTransfer_t*   fileXfer,
                               uint8_t*              buff,
                               uint32_t              buffLen,
                               BOOL                  bDeleteBuffOnFinish,
                               BOOL                  bFinal);


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
BOOL iAP2FileTransferHandleRecv (iAP2FileTransfer_t* fileXfer,
                                 const uint8_t*      data,
                                 uint32_t            dataLen);


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
void iAP2FileTransferCancel (iAP2FileTransfer_t* fileXfer);


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
void iAP2FileTransferPause (iAP2FileTransfer_t* fileXfer);


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
void iAP2FileTransferResume (iAP2FileTransfer_t* fileXfer);


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
void iAP2FileTransferSuccess (iAP2FileTransfer_t* fileXfer);


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
void iAP2FileTransferFailure (iAP2FileTransfer_t* fileXfer);


#ifdef __cplusplus
}
#endif

#endif

