/*
 *	File: iAP2Link.h
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

/*
********************************************************************
**
** iAP2Link
**
** NOTE: iAP2Link is not multi-thread safe and it is expected to be
**       called from a main link handling thread only if the system
**       supports multiple threads.
**
** USAGE:
**
** 1) Create link:                      iAP2LinkCreateAccessory/Device
** 2) Start/Init link FSM:              iAP2LinkStart
** 3) On accessory/device attach:       iAP2LinkAttached
** 4a) On receipt of packet from port:  iAP2LinkHandleReadyPacket
** 4b) On receipt of data from session: iAP2LinkQueueSendData
** 5) On accessory/device detach:       iAP2LinkDetached
** 6) On port disappearing/shutdown:    iAP2LinkDelete
**
** Callbacks:
** 
** iAP2LinkSendPacketCB_t
**      - Called by link to actually send out the packet on the port.
** iAP2LinkDataReadyCB_t
**      - Called by link to notify system about the received data
** iAP2LinkConnectedCB_t
**      - Called by link to notify system that the link FSM has
**          entered the Connected state.
** iAP2LinkSendDetectCB_t
**      - Called by link to actually send out the DETECT byte sequence
**          on the port.
** iAP2LinkSignalSendBuffCB_t
**      - link calls this callback to request the system to call
**          link's iAP2LinkProcessSendBuff call on the main
**          link handling thread asynchronously (ie. queue it up for
**          callback later).
**      - iAP2LinkProcessSendBuff is used to process the outbound data
**          queue to handle windowing.
**
** Other:
**
** iAP2LinkGetSessionInfo
**      - This call can be used by the system when handling the
**          iAP2LinkDataReadyCB_t callback to determine information
**          about the session, ie. service type.
**
********************************************************************
*/
#ifndef iAP2Link_iAP2Link_h
#define iAP2Link_iAP2Link_h

#include <stdint.h>

/*
** Minimum required defines in iAP2LinkConfig.h
**  #define iAP2_LINK_USE_LINKRUNLOOP 0 ** iAP2LinkRunLoop not used
**  #define iAP2_LINK_ALLOW_MALLOC 0 or 1   ** Disallow/Allow link layer code to malloc buffers as necessary
*/
#include <iAP2LinkConfig.h>

#include <iAP2FSM.h>
#include <iAP2ListArray.h>
#include <iAP2Time.h>
#include <iAP2BuffPool.h>

#include "iAP2Packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    kiAP2LinkTypeAccessory = 0,
    kiAP2LinkTypeDevice

} iAP2LinkType_t;


typedef enum
{
    kiAP2LinkStateInit = 0,     /* Initial starting state */
    kiAP2LinkStateDetached,     /* Device/Accessory is detached */
    kiAP2LinkStateDetect,       /* Waiting for DETECT */
    kiAP2LinkStateIdle,         /* Waiting on link negotiation (SYN, SYN+ACK) */
    kiAP2LinkStatePending,      /* Waiting on Acknowledgement (only used by Device) */
    kiAP2LinkStateConnected,    /* Link has been established and normal communication occurring */
    kiAP2LinkStateSuspend,      /* Device has gone to sleep/suspended state */
    kiAP2LinkStateFailed,       /* Link has gone into failure state */

    kiAP2LinkStateCount

} kiAP2LinkState_t;


typedef enum
{
    kiAP2LinkEventInitDone = 0,         /* Initialization finished */
    kiAP2LinkEventAttach,               /* Device/Accessory attach event */
    kiAP2LinkEventRecvSYN,              /* SYN packet receive event */
    kiAP2LinkEventRecvSYNACK,           /* SYN+ACK packet receive event */
    kiAP2LinkEventRecvSYNACKOLD,        /* SYN+ACK packet with old seq# (resend) receive event */
    kiAP2LinkEventRecvSYNACKNEW,        /* SYN+ACK packet with new info receive event */
    kiAP2LinkEventRecvRST,              /* RST packet receive event (only sent by Device) */
    kiAP2LinkEventWaitACKTimeout,       /* Wait ACK timeout event, need to resend */
    kiAP2LinkEventSendACKTimeout,       /* Send ACK timeout event, need to send ACK */
    kiAP2LinkEventDetach,               /* Device/Accessory detach event */
    kiAP2LinkEventRecvData,             /* Data (ACK with payload) packet receive event */
    kiAP2LinkEventRecvLastData,         /* Data (ACK with payload) packet with previous seq# receive event */
    kiAP2LinkEventEAK,                  /* EAK packet receive event */
    kiAP2LinkEventDataToSend,           /* There is data buffer to send out notification event */
    kiAP2LinkEventMaxResend,            /* Max resend limit has been reached event */
    kiAP2LinkEventRecvACK,              /* ACK packet (no payload) receive event */
    kiAP2LinkEventRecvACKBadLink,       /* ACK packet receive event even though bad LINK negotiation parameters */
    kiAP2LinkEventRecvDetect,           /* DETECT byte sequence receive event (iAP2 supported) */
    kiAP2LinkEventRecvDetectBad,        /* Bad DETECT byte sequence received event (iAP2 not supported) */
    kiAP2LinkEventRecviAP1Packet,       /* iAP1 packet receive event */
    kiAP2LinkEventWaitDetectTimeout,    /* Wait for Detect/DetectBad, Resend */
    kiAP2LinkEventSuspend,              /* SUS packet receive event (sent by Device when going to sleep) */
    kiAP2LinkEventCount

} iAP2LinkEvent_t;

/* Min Max values for SYN parameters */
enum
{
    kiAP2LinkSynValVersionMin           = 1,
    kiAP2LinkSynValVersionMax           = 1,
    kiAP2LinkSynValMaxOutstandingMin    = 1,
    kiAP2LinkSynValMaxOutstandingMax    = 127,
    kiAP2LinkSynValMaxPacketSizeMin     = 24,
    kiAP2LinkSynValMaxPacketSizeMax     = 65535,
    kiAP2LinkSynValRetransmitTimeoutMin = 20,
    kiAP2LinkSynValRetransmitTimeoutMax = 65535,
    kiAP2LinkSynValCumAckTimeoutMin     = 10,
    kiAP2LinkSynValCumAckTimeoutMax     = 32767,
    kiAP2LinkSynValMaxRetransmitMin     = 1,
    kiAP2LinkSynValMaxRetransmitMax     = 30,
    kiAP2LinkSynValMaxCumAckMin         = 0,
    kiAP2LinkSynValMaxCumAckMax         = 127,

    /* Default SYN parameters */
    kiAP2LinkSynDefaultVersion           = 1,
    kiAP2LinkSynDefaultMaxOutstanding    = 1,
    kiAP2LinkSynDefaultMaxPacketSize     = 128,
    kiAP2LinkSynDefaultRetransmitTimeout = 1000,
    kiAP2LinkSynDefaultCumAckTimeout     = 0,
    kiAP2LinkSynDefaultMaxRetransmit     = 30,
    kiAP2LinkSynDefaultMaxCumAck         = 1,
    kiAP2LinkSynDefaultNumSessionInfo    = 0,

    /* Maximum number of renegotiations before failing the link */
    kiAP2LinkSynRetries                  = 10
};

struct iAP2Link_st;


/*
****************************************************************
**
**  Callback Functions to be implemented by user of iAP2Link
**
****************************************************************
*/

/*
** Callback function to call when received data is ready.
** Returns:
**  TRUE if data is processed and can be discarded.
**  FALSE if data cannot be processed at this time.
** If FALSE is returned, the packet should not be ACK'd.
** Callback function should return as quickly as possible while guaranteeing
** that the data will not be lost... ie. if it can queue the data and can
** guarantee that the data in the queue will eventually be serviced, it can
** return TRUE and if the queue if full, it would return FALSE.
** If callback function returns TRUE, the callback function is responsible
** for cleaning up the data buffer.
*/
typedef BOOL (*iAP2LinkDataReadyCB_t) (struct iAP2Link_st* link,
                                       uint8_t*            data,
                                       uint32_t            dataLen,
                                       uint8_t             session);

/*
** Callback function to call when packet is ready to be sent.
** This callback will actually send the packet out.
*/
typedef void (*iAP2LinkSendPacketCB_t) (struct iAP2Link_st* link,
                                        iAP2Packet_t*       packet);

/*
** Callback function to call when packet is ready to be sent.
** This callback will actually send the packet out and wait for packet to be
** sent out.
*/
typedef void (*iAP2LinkSendPacketWaitCB_t) (struct iAP2Link_st* link,
                                            iAP2Packet_t*       packet);

/*
** Callback function to call when link connection is UP/DOWN.
*/
typedef void (*iAP2LinkConnectedCB_t) (struct iAP2Link_st* link,
                                       BOOL                bConnected);

/*
** Callback function to call to send detect byte sequence.
*/
typedef void (*iAP2LinkSendDetectCB_t) (struct iAP2Link_st* link,
                                        BOOL                bBad);

/*
** Callback function to signal a call back into
** iAP2LinkProcessSendbuff() (asynchronously) to process outgoing data.
*/
typedef void (*iAP2LinkSignalSendBuffCB_t) (struct iAP2Link_st* link);

/*
** Callback function to signal a data buffer has been processed and sent out
*/
typedef void (*iAP2LinkDataSentCB_t) (struct iAP2Link_st* link,
                                      void*               context);


/* OBSOLETE */
typedef struct iAP2LinkData_st
{
    uint8_t     session;
    uint8_t*    cur;
    uint32_t    curLen;
    uint8_t*    data;
    uint32_t    dataLen;

    void*       context;
    iAP2LinkDataSentCB_t callback;

} iAP2LinkData_t;

typedef struct iAP2Link_st
{
#if iAP2_LINK_ALLOW_MALLOC != 0
#define iAP2_LINK_BUFFER_OFFSET 4
#endif
    uint8_t*        linkBuffer;
    uint8_t*        linkBufferNext;

    iAP2FSM_t*      fsm;

    iAP2LinkSendPacketCB_t      sendPacketCB;
    iAP2LinkSendPacketWaitCB_t  sendPacketWaitCB;
    iAP2LinkDataReadyCB_t       recvDataCB;
    iAP2LinkConnectedCB_t       connectedCB;
    iAP2LinkSendDetectCB_t      sendDetectCB;
    iAP2LinkSignalSendBuffCB_t  signalSendBuffCB;

    void*           context;

    iAP2LinkType_t  type;

    uint8_t         startSeq;
    uint8_t         recvSeq;
    uint8_t         recvAck;
    uint8_t         sentSeq;
    uint8_t         sentAck;

    uint8_t         detectAckTimeoutID;
    uint8_t         sendAckTimeoutID;
    iAP2Timer_t*    mainTimer;
    
    uint32_t        timeSinceSentLastACK;
    

    BOOL            bValidRecvSeq:1;
    BOOL            bValidRecvAck:1;
    BOOL            bValidSentSeq:1;
    BOOL            bValidSentAck:1;
    BOOL            bUseiAP2LinkRunLoop:1;
    BOOL            bIgnoreSynRetryLimit:1;

    iAP2BuffPool_t* recvPckPool;
    iAP2BuffPool_t* sendPckPool;
    iAP2BuffPool_t* buffPool;

    // Stats
    iAP2Packet_t*   recvPck;
    uint8_t*        recvPckList;
    uint8_t*        sendPckList;
    /*
    ** This supports all session types but if the implementation does not
    ** need to support all session types, only the supported ones need have
    ** the sessSendPckList created.
    */
    uint8_t*        sessSendPckList [kIAP2PacketServiceTypeCount];

    /* SYN params */
    iAP2PacketSYNData_t initParam;
    iAP2PacketSYNData_t negotiatedParam;
    iAP2PacketSYNData_t param;

    /* Counters */
    uint32_t    numRecvSYN;
    uint32_t    numRecvSYNACK;
    uint32_t    numRecvCumSYN;
    uint32_t    numRecvCumSYNACK;
    uint32_t    numSentSYN;
    uint32_t    numSentSYNACK;
    uint32_t    numSentCumSYN;
    uint32_t    numSentCumSYNACK;
    uint32_t    numResentSYN;
    uint32_t    numResentSYNACK;
    uint32_t    numResentCumSYN;
    uint32_t    numResentCumSYNACK;

    /* Stats */
#if iAP2_LINK_ALLOW_STATS
    uint32_t    bytesSent;
    uint32_t    bytesRcvd;
    uint32_t    packetsSent;
    uint32_t    packetsRcvd;
    uint32_t    dataBytesSent;
    uint32_t    dataBytesRcvd;
    uint32_t    dataPacketsSent;
    uint32_t    dataPacketsRcvd;
    uint32_t    noAckReTxCount;
    uint32_t    missingReTxCount;
    uint32_t    invalidPackets;
    uint32_t    numOutOfOrder;
    uint32_t    numRecvEAK;
    uint32_t    numRecvRST;
    uint32_t    numRecvACK;
    uint32_t    numRecvDATA;
    uint32_t    numSentEAK;
    uint32_t    numSentRST;
    uint32_t    numSentACK;
    uint32_t    numSentDATA;
    uint32_t    totACKDelays;
    uint32_t    numACKDelays;
    uint32_t    numResentACK;
    uint32_t    numResentDATA;
#endif /*#if iAP2_LINK_ALLOW_STATS*/

} iAP2Link_t;


extern const uint8_t    kIap2PacketDetectData[];
extern uint32_t         kIap2PacketDetectDataLen;
extern const uint8_t    kIap2PacketDetectBadData[];
extern uint32_t         kIap2PacketDetectBadDataLen;


/*
****************************************************************
**
**  iAP2LinkGetBuffSize
**
**  Input:
**      maxPacketSentRcvdAtOnce:    Max number of packets that will be sent
**                                  (or recvd) at once regardless of the
**                                  maxOutstandingPackets value.
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of buff required for proper operation
**
**  Note:   It is assumed that send count will not exceed our receive max.
**
****************************************************************
*/
uint32_t iAP2LinkGetBuffSize (uint8_t maxPacketSentRcvdAtOnce);


/*
****************************************************************
**
**  iAP2LinkCreate
**  iAP2LinkCreateAccessory (type = kiAP2LinkTypeAccessory)
**  iAP2LinkCreateDevice (type = kiAP2LinkTypeDevice)
**
**  Input:
**      type:               Indicates whether this is for Accessory or Device.
**      context:            Context info to store with the link.
**      synParam:           SYN packet parameters
**      sendPacketCB:       Callback function to call when packet is ready to be sent.
**                              This callback will actually send the packet out.
**      recvDataCB:         Callback function to call when received data is ready.
**      connectedCB:        Callback function to call when link connection is UP/DOWN.
**      sendDetectCB:       Callback function to call to send detect byte sequence.
**      signalSendBuffCB:   Callback function to signal a call back into
**                              iAP2LinkProcessSendbuff() (asynchronously)
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
                            uint8_t*                    linkBuffer);

iAP2Link_t* iAP2LinkCreateAccessory (iAP2PacketSYNData_t*       synParam,
                                     void*                      context,
                                     iAP2LinkSendPacketCB_t     sendPacketCB,
                                     iAP2LinkDataReadyCB_t      recvDataCB,
                                     iAP2LinkConnectedCB_t      connectedCB,
                                     iAP2LinkSendDetectCB_t     sendDetectCB,
                                     iAP2LinkSignalSendBuffCB_t signalSendBuffCB,
                                     BOOL                       bValidateSYN,
                                     uint8_t                    maxPacketSentAtOnce,
                                     uint8_t*                   linkBuffer);

iAP2Link_t* iAP2LinkCreateDevice (iAP2PacketSYNData_t*          synParam,
                                  void*                         context,
                                  iAP2LinkSendPacketCB_t        sendPacketCB,
                                  iAP2LinkSendPacketWaitCB_t    sendPacketWaitCB,
                                  iAP2LinkDataReadyCB_t         recvDataCB,
                                  iAP2LinkConnectedCB_t         connectedCB,
                                  iAP2LinkSendDetectCB_t        sendDetectCB,
                                  iAP2LinkSignalSendBuffCB_t    signalSendBuffCB,
                                  BOOL                          bValidateSYN,
                                  uint8_t                       maxPacketSentAtOnce,
                                  uint8_t*                      linkBuffer);


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
void iAP2LinkResetSend (iAP2Link_t* link);


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
void iAP2LinkDelete (iAP2Link_t* link);


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
void iAP2LinkStart (iAP2Link_t* link);


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
BOOL iAP2LinkProcessSendBuff (iAP2Link_t* link);


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
**  Note: Notify link that the transport has been established.
**
****************************************************************
*/
void iAP2LinkAttached (iAP2Link_t* link);


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
**  Note: Notify link that the transport has been torn down.
**
****************************************************************
*/
void iAP2LinkDetached (iAP2Link_t* link);


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
BOOL iAP2LinkIsDetached (iAP2Link_t* link);


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
**      link:  link structure is updated to reflect state after
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
BOOL iAP2LinkQueueSendData (iAP2Link_t*          link,
                            const uint8_t*       payload,
                            uint32_t             payloadLen,
                            uint8_t              session,
                            void*                context,
                            iAP2LinkDataSentCB_t callback);


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
                                  iAP2LinkDataSentCB_t  callback);


/*
****************************************************************
**
**  iAP2LinkHandleReadyPacket
**
**  Input:
**      link:   link structure
**      packet: packet to handle: consider packet belongs to link layer.
**                                caller should not delete the packet.
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Process a parsed packet and generate appropriate event.
**        This is callback handler, called after packet parsing finishes.
**
****************************************************************
*/
void iAP2LinkHandleReadyPacket (struct iAP2Link_st* link,
                                iAP2Packet_t*       packet);


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
void iAP2LinkHandleSuspend (struct iAP2Link_st* link);


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
void iAP2LinkHandleWaitACKTimeoutLink (iAP2Link_t* link, uint32_t curTime);


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
void iAP2LinkHandleSendACKTimeoutLink (iAP2Link_t* link, uint32_t curTime);


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
void iAP2LinkHandleWaitDetectTimeoutLink (iAP2Link_t* link, uint32_t curTime);


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
BOOL iAP2LinkSendWindowAvailable (iAP2Link_t* link);


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
                                                 uint8_t     session);


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
uint32_t iAP2LinkGetSessionForService (iAP2Link_t*             link,
                                       iAP2PacketServiceType_t service);


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
**      uint32_t max payload size.
**
**  Note: OBSOLETE: equivalent to iAP2LinkGetMaxSendPayloadSize
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxPayloadSize (iAP2Link_t* link);


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
**      uint32_t max send payload size.
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxSendPayloadSize (iAP2Link_t* link);


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
uint32_t iAP2LinkGetMaxRecvPayloadSize (iAP2Link_t* link);


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
**      uint32_t max send packet size.
**
****************************************************************
*/
uint32_t iAP2LinkGetMaxSendPacketSize (iAP2Link_t* link);


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
uint32_t iAP2LinkGetMaxRecvPacketSize (iAP2Link_t* link);


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
iAP2Timer_t* iAP2LinkGetMainTimer (iAP2Link_t* link);


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
                                   BOOL         bDebug);


/*
****************************************************************
**
**  iAP2LinkDebugPrintLink
**
**  Input:
**      link:   link to print out info for.
**      file:   file called from
**      line:   line number called from
**      tag:    additional info to display with packet info
**      debug:  whether to print as debug with data or regular stats printout
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LinkDebugPrintLink (iAP2Link_t* link,
                             const char* file,
                             int         line,
                             const char* tag,
                             BOOL        bDebug);


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
                                       BOOL        bFlag);


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
void iAP2LinkSendRST (iAP2Link_t* link);


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
void iAP2LinkSetUseiAP2LinkRunLoop (iAP2Link_t* link);
#endif


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Link_iAP2Link_h */
