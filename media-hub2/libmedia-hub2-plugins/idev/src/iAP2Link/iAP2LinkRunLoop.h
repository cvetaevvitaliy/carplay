/*
 *	File: iAP2LinkRunLoop.h
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
*********************************************************************
**
** iAP2LinkRunLoop
**
** USAGE:
**
** A) Using a single control loop to run Link layer and rest of system:
**
**      - iAP2LinkRunLoopInitImplementation: can be stub function.
**      - iAP2LinkRunLoopCleanupImplementation: can be stub function.
**      - iAP2LinkRunLoopProtectedCall: straight pass through call to callback
**      - iAP2LinkRunLoopWait: stub function needed (return TRUE)
**      - iAP2LinkRunLoopSignal: stub function needed;
**                               may setup a flag to indicate that
**                               iAP2LinkRunLoopRunceOnce should be called in
**                               the next iteration of the control loop.
**
**  1) Create linkRunLoop:                  iAP2LinkRunLoopCreateAccessory/Device
**  2) At each iteration, call              iAP2LinkRunLoopRunOnce
**  3) On accessory/device attach:          iAP2LinkRunLoopAttached
**  4a) On receipt of packet from port:     iAP2LinkRunLoopHandleReadyPacket
**  4b) On receipt of data from session:    iAP2LinkRunLoopQueueSendData
**  5) On accessory/device detach:          iAP2LinkRunLoopDetached
**  6) On port disappearing/shutdown:       iAP2LinkRunLoopDelete
**
**
** B) Using dedicated thread to run Link layer:
**
**      - iAP2LinkRunLoopInitImplementation: can be implemented to create thread
**                                           and other associated items.
**      - iAP2LinkRunLoopCleanupImplementation: can be implemented to clenaup.
**      - iAP2LinkRunLoopProtectedCall: implementation needed;
**                                      should be implemented to make sure link
**                                      layer processing only occurs from one thread.
**      - iAP2LinkRunLoopWait: implementation needed
**      - iAP2LinkRunLoopSignal: implementation needed
**
**  1) Create linkRunLoop:                  iAP2LinkRunLoopCreateAccessory/Device
**  2) Create thread and have thread call   iAP2LinkRunLoopRun
**      This can be done in                 iAP2LinkRunLoopInitImplementation
**  3) On accessory/device attach:          iAP2LinkRunLoopAttached
**  4a) On receipt of packet from port:     iAP2LinkRunLoopHandleReadyPacket
**  4b) On receipt of data from session:    iAP2LinkRunLoopQueueSendData
**  5) On accessory/device detach:          iAP2LinkRunLoopDetached
**  6) On port disappearing/shutdown:       iAP2LinkRunLoopDelete
**
**
** C) Alternative threaded implementation:
**
**      - iAP2LinkRunLoopInitImplementation: can be implemented to create thread
**                                           and other assoicated items.
**      - iAP2LinkRunLoopCleanupImplementation: can be implemented to clenaup.
**      - iAP2LinkRunLoopProtectedCall: implementation needed;
**                                      should be implemented to make sure link
**                                      layer processing only occurs from one thread.
**      - iAP2LinkRunLoopWait: stub function needed (return TRUE)
**      - iAP2LinkRunLoopSignal: implementation needed;
**                               should be implemented to call back into
**                               iAP2LinkRunLoopRunOnce under protection of
**                               iAP2LinkRunLoopProtectedCall.
**
**  1) Create linkRunLoop:                  iAP2LinkRunLoopCreateAccessory/Device
**  2) Create thread for calling            iAP2LinkRunLoopRunOnce
**      This can be done in                 iAP2LinkRunLoopInitImplementation
**  2a) On iAP2LinkRunLoopSignal call, call iAP2LinkRunLoopProtectedCall
**      for calling back into               iAP2LinkRunLoopRunOnce
**  3) On accessory/device attach:          iAP2LinkRunLoopAttached
**  4a) On receipt of packet from port:     iAP2LinkRunLoopHandleReadyPacket
**  4b) On receipt of data from session:    iAP2LinkRunLoopQueueSendData
**  5) On accessory/device detach:          iAP2LinkRunLoopDetached
**  6) On port disappearing/shutdown:       iAP2LinkRunLoopDelete
**
**
** See iAP2Link.h for specific info about link layer processing.
**
*********************************************************************
*/

#ifndef iAP2Link_iAP2LinkRunLoop_h
#define iAP2Link_iAP2LinkRunLoop_h

#include <stdint.h>

#include <iAP2LinkConfig.h>
#include "iAP2Packet.h"
#include "iAP2Link.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    kiAP2LinkRunLoopEventMaskAttach             = 0x0001,
    kiAP2LinkRunLoopEventMaskDetach             = 0x0002,
    kiAP2LinkRunLoopEventMaskSuspend            = 0x0004,
    kiAP2LinkRunLoopEventMaskWaitACKTimeout     = 0x0010,
    kiAP2LinkRunLoopEventMaskSendACKTimeout     = 0x0020,
    kiAP2LinkRunLoopEventMaskWaitDetectTimeout  = 0x0040,

    kiAP2LinkRunLoopEventMaskTimeouts           = 0x0070,

    kiAP2LinkRunLoopEventMaskNone               = 0

} iAP2LinkRunLoopEventMask_t;


typedef struct iAP2LinkRunLoop_st
{
#if iAP2_LINK_ALLOW_MALLOC != 0
#define iAP2_LINK_RUNLOOP_BUFFER_OFFSET 4
#endif
    uint8_t*            linkRLBuffer;
    uint8_t*            linkRLBufferNext;
    void*               context;
    iAP2Link_t*         link;
    void*               otherData;
    uint32_t            eventMask;
    BOOL                shuttingDown;

} iAP2LinkRunLoop_t;


/*
*****************************************************************
**
**  iAP2LinkRunLoopGetLinkBuffSize
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
*****************************************************************
*/
uint32_t iAP2LinkRunLoopGetBuffSize (uint8_t maxPacketSentAtOnce);


/*
*****************************************************************
**
**  iAP2LinkRunLoopCreate
**  iAP2LinkRunLoopCreateAccessory (type = kiAP2LinkTypeAccessory)
**  iAP2LinkRunLoopCreateDevice (type = kiAP2LinkTypeDevice)
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
**                              iAP2TansportProcessSendbuff() (asynchronously)
**                              to process outgoing data.
**      bValidateSYN        Flag indicating whether to check for valid SYN param.
**                              (for debugging/testing purpose)
**      maxPacketSentAtOnce Max number of packets that will be sent at once
**                              regarless of the maxOutstandingPackets value.
**      linkRLBuffer        Pre-allocated buffer to use for storing iAP2LinkRunLoop_t
**                              and other related structures.
**
**  Output:
**      Passed in linkRLBuffer is initiliazed.
**
**  Return:
**      iAP2LinkRunLoop_t*  pointer to new allocated iAP2LinkRunLoop_t structure
**                          or the passed in linkRLBuffer.
**
*****************************************************************
*/
iAP2LinkRunLoop_t* iAP2LinkRunLoopCreate (iAP2LinkType_t              type,
                                          void*                       context,
                                          iAP2PacketSYNData_t*        synParam,
                                          iAP2LinkSendPacketCB_t      sendPacketCB,
                                          iAP2LinkSendPacketWaitCB_t  sendPacketWaitCB,
                                          iAP2LinkDataReadyCB_t       recvDataCB,
                                          iAP2LinkConnectedCB_t       connectedCB,
                                          iAP2LinkSendDetectCB_t      sendDetectCB,
                                          BOOL                        bValidateSYN,
                                          uint8_t                     maxPacketSentAtOnce,
                                          uint8_t*                    linkRLBuffer);

iAP2LinkRunLoop_t* iAP2LinkRunLoopCreateAccessory (iAP2PacketSYNData_t*       synParam,
                                                   void*                      context,
                                                   iAP2LinkSendPacketCB_t     sendPacketCB,
                                                   iAP2LinkDataReadyCB_t      recvDataCB,
                                                   iAP2LinkConnectedCB_t      connectedCB,
                                                   iAP2LinkSendDetectCB_t     sendDetectCB,
                                                   BOOL                       bValidateSYN,
                                                   uint8_t                    maxPacketSentAtOnce,
                                                   uint8_t*                   linkRLBuffer);

iAP2LinkRunLoop_t* iAP2LinkRunLoopCreateDevice (iAP2PacketSYNData_t*          synParam,
                                                void*                         context,
                                                iAP2LinkSendPacketCB_t        sendPacketCB,
                                                iAP2LinkSendPacketWaitCB_t    sendPacketWaitCB,
                                                iAP2LinkDataReadyCB_t         recvDataCB,
                                                iAP2LinkConnectedCB_t         connectedCB,
                                                iAP2LinkSendDetectCB_t        sendDetectCB,
                                                BOOL                          bValidateSYN,
                                                uint8_t                       maxPacketSentAtOnce,
                                                uint8_t*                      linkRLBuffer);


/*
*****************************************************************
**
**  iAP2LinkRunLoopDelete
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/
void iAP2LinkRunLoopDelete (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopRunOnce
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**      arg:            argument for the call (unused)
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if more left, else FALSE
**
**  Note: This call is used to process link layer tasks such as processing
**          system events (attach/detach/suspend), received packets, packets
**          to send, etc.
**
*****************************************************************
*/
BOOL iAP2LinkRunLoopRunOnce (iAP2LinkRunLoop_t* linkRunLoop, void* arg);
    
    
/*
*****************************************************************
**
**  iAP2LinkRunLoopRun
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if existed without error (RunLoop finished), else FALSE
**
**  Note: This call is used when running the link layer control loop from
**          a dedicated thread with support for support for signal/wait.
**
*****************************************************************
*/
BOOL iAP2LinkRunLoopRun (iAP2LinkRunLoop_t* linkRunLoop);
    
    
/*
*****************************************************************
**
**  iAP2LinkRunLoopTimeout
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**      timeoutID:      expired timeoutID
**      timeoutType:    expired timeout type
**      curTime:        current time for timeout
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Should be called via iAP2LinkRunLoopProtectedCall
**
*****************************************************************
*/
void iAP2LinkRunLoopTimeout (iAP2LinkRunLoop_t* linkRunLoop,
                             uint16_t           timeoutID,
                             uint8_t            timeoutType,
                             uint32_t           curTime);


/*
*****************************************************************
**
**  iAP2LinkRunLoopAttached
**
**  Input:
**      linkRunLoop:    linkRunLoop structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Notify link that the transport has been established.
**
*****************************************************************
*/
void iAP2LinkRunLoopAttached (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopDetached
**
**  Input:
**      linkRunLoop:    linkRunLoop structure
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Notify link that the transport has been torn down.
**
*****************************************************************
*/
void iAP2LinkRunLoopDetached (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopHandleSuspend
**
**  Input:
**      linkRunLoop:    linkRunLoop structure
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/
void iAP2LinkRunLoopHandleSuspend (struct iAP2LinkRunLoop_st* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopHandleReadyPacket
**
**  Input:
**      linkRunLoop:    link structure
**      packet:         packet to handle: consider packet belongs to link layer.
**                                        caller should not delete the packet.
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
*****************************************************************
*/
void iAP2LinkRunLoopHandleReadyPacket (iAP2LinkRunLoop_t* linkRunLoop,
                                       void*              packetArg);


/*
*****************************************************************
**
**  iAP2LinkRunLoopQueueSendData
**
**  Input:
**      linkRunLoop:    linkRunLoop structure
**      payload:        payload data buffer to send
**      payloadLen:     size of the payload data buffer
**      session:        sessionID
**      context:        context to use when calling callback on data buffer send
**      callback:       callback to call when data buffer has been sent
**
**  Output:
**      linkRunLoop:    linkRunLoop structure is updated to reflect state after
**                          processing payload data.
**
**  Return:
**      None
**
**  Note: Queue data for sending. Must cause a DataToSend event to be generated
**          for the data to actually get sent out.
**
*****************************************************************
*/
void iAP2LinkRunLoopQueueSendData (iAP2LinkRunLoop_t*   linkRunLoop,
                                   const uint8_t*       payload,
                                   uint32_t             payloadLen,
                                   uint8_t              session,
                                   void*                context,
                                   iAP2LinkDataSentCB_t callback);


/*
*****************************************************************
**
**  iAP2LinkRunLoopDebugPrint
**
**  Input:
**      linkRunLoop:    linkRunLoop to print out info for.
**      file:           file called from
**      line:           line number called from
**      tag:            additional info to display with packet info
**      debug:          whether to print as debug with data or regular stats printout
**
**  Output:
**      None
**
**  Return:
**      None
**
*****************************************************************
*/
void iAP2LinkRunLoopDebugPrint (iAP2LinkRunLoop_t* linkRunLoop,
                                const char*        file,
                                int                line,
                                const char*        tag,
                                BOOL               bDebug);


/*
*****************************************************************
**
**  Functions to be implemented by user of iAP2LinkRunLoop
**
*****************************************************************
*/

/*
*****************************************************************
**
**  iAP2LinkRunLoopInitImplementation
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Implementation specific initialization.
**          Implementation specific data can be stored in iAP2LinkRunLoop_t.otherData
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**
**
*****************************************************************
*/
void iAP2LinkRunLoopInitImplementation (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopCleanupImplementation
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Implementation specific cleanup.
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**
**
*****************************************************************
*/
void iAP2LinkRunLoopCleanupImplementation (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopWait
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if successfully waited and got signal, else FALSE
**
**  Note:   Waits for a signal (via iAP2LinkRunLoopSignal) before continuing.
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**
**
*****************************************************************
*/
BOOL iAP2LinkRunLoopWait (iAP2LinkRunLoop_t* linkRunLoop);


/*
*****************************************************************
**
**  iAP2LinkRunLoopSignal
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**      arg:            argument to call back iAP2LinkRunLoopRunOnce with.
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Signals RunLoop to continue processing (wait is via iAP2LinkRunLoopWait.
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**
*****************************************************************
*/
void iAP2LinkRunLoopSignal (iAP2LinkRunLoop_t* linkRunLoop, void* arg);


/*
*****************************************************************
**
**  iAP2LinkRunLoopProtectedCall
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**      arg:            argument for use when calling func
**      func:           function to call
**
**  Output:
**      None
**
**  Return:
**      BOOL    return value of function called with protection.
**
**  Note:   Call the indicated function with simultaneous call protection.
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**
*****************************************************************
*/
BOOL iAP2LinkRunLoopProtectedCall (iAP2LinkRunLoop_t* linkRunLoop,
                                   void* arg,
                                   BOOL (*func)(iAP2LinkRunLoop_t* linkRunLoop, void* arg));


/*
*****************************************************************
**
**  iAP2LinkRunLoopSetEventMaskBit
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**      bit:            eventMask bit to set (iAP2LinkRunLoopEventMask_t)
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**                If access to evenMask needs to be protected must make
**                sure operation on linkRunLoop->eventMask is atomic.
**
*****************************************************************
*/
void iAP2LinkRunLoopSetEventMaskBit (iAP2LinkRunLoop_t*         linkRunLoop,
                                     iAP2LinkRunLoopEventMask_t bit);


/*
*****************************************************************
**
**  iAP2LinkRunLoopGetResetEventMask
**
**  Input:
**      linkRunLoop:    linkRunLoop structure to delete
**
**  Output:
**      None
**
**  Return:
**      uint32_t    bit mask of events currently pending (iAP2LinkRunLoopEventMask_t)
**
**  Note:   Retrieve the eventMask and reset it to kiAP2LinkRunLoopEventMaskNone.
**          TODO: Needs to be implemented by the user of iAP2LinkRunLoop
**                If access to evenMask needs to be protected must make
**                sure operation on linkRunLoop->eventMask is atomic.
**
*****************************************************************
*/
uint32_t iAP2LinkRunLoopGetResetEventMask (iAP2LinkRunLoop_t* linkRunLoop);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Link_iAP2LinkRunLoop_h */
