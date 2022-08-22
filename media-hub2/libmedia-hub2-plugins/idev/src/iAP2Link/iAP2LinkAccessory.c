/*
 *	File: iAP2LinkAccessory.c
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

#include <iAP2LinkConfig.h>
#include <iAP2FSM.h>
#include <iAP2Log.h>

#include "iAP2Packet.h"
#include "iAP2Link.h"
#include "iAP2LinkPrivate.h"


#if iAP2_FOR_ACCESSORY

void iAP2LinkAccessoryActionSendDetect (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkAccessoryActionResendSYN (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkAccessoryActionConnectedACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkAccessoryActionSendSYNACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkAccessoryActionSendSYNNewSeq (struct iAP2FSM_st* fsm, unsigned int* nextEvent);
void iAP2LinkAccessoryActionHandleSuspend (struct iAP2FSM_st* fsm, unsigned int* nextEvent);

/*
** Accessory States
** Each table is an event action/nextState lookup table for a particular state.
** See iAP2Link.h for list of events and states.
** Entry of {NULL, kiAP2LinkStateCount} indicates that the event is not expected in this state.
*/

const iAP2FSMEvent_t iAP2LinkAccessoryInitStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {iAP2LinkActionNone,                            kiAP2LinkStateDetached},   /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvRST */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryDetachedStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {iAP2LinkAccessoryActionSendDetect,             kiAP2LinkStateDetect},     /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvRST */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryDetectStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvRST */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {iAP2LinkActionDetach,                          kiAP2LinkStateDetached},   /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {iAP2LinkAccessoryActionSendSYNNewSeq,          kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvDetect */
    {iAP2LinkActionSwitchToiAP1,                    kiAP2LinkStateFailed},     /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {iAP2LinkAccessoryActionSendDetect,             kiAP2LinkStateDetect},     /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryIdleStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {iAP2LinkAccessoryActionConnectedACK,           kiAP2LinkStateConnected},  /* kiAP2LinkEventRecvSYNACK */
    {iAP2LinkActionSendACK,                         kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvSYNACKOLD */
    {iAP2LinkAccessoryActionSendSYNACK,             kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvSYNACKNEW */
    {iAP2LinkAccessoryActionSendSYNNewSeq,          kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvRST */
    {iAP2LinkAccessoryActionResendSYN,              kiAP2LinkStateIdle},       /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {iAP2LinkActionDetach,                          kiAP2LinkStateDetached},   /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {iAP2LinkActionNone,                            kiAP2LinkStateFailed},     /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryPendingStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvRST */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryConnectedStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {iAP2LinkAccessoryActionSendSYNNewSeq,          kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvRST */
    {iAP2LinkActionResendData,                      kiAP2LinkStateConnected},  /* kiAP2LinkEventWaitACKTimeout */
    {iAP2LinkActionSendACK,                         kiAP2LinkStateConnected},  /* kiAP2LinkEventSendACKTimeout */
    {iAP2LinkActionDetach,                          kiAP2LinkStateDetached},   /* kiAP2LinkEventDetach */
    {iAP2LinkActionHandleData,                      kiAP2LinkStateConnected},  /* kiAP2LinkEventRecvData */
    {iAP2LinkActionSendACK,                         kiAP2LinkStateConnected},  /* kiAP2LinkEventRecvLastData */
    {iAP2LinkActionResendMissing,                   kiAP2LinkStateConnected},  /* kiAP2LinkEventEAK */
    {iAP2LinkActionSendData,                        kiAP2LinkStateConnected},  /* kiAP2LinkEventDataToSend */
    {iAP2LinkActionNotifyConnectionFail,            kiAP2LinkStateFailed},     /* kiAP2LinkEventMaxResend */
    {iAP2LinkActionHandleACK,                       kiAP2LinkStateConnected},  /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {iAP2LinkAccessoryActionHandleSuspend,          kiAP2LinkStateSuspend}     /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessorySuspendStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {iAP2LinkAccessoryActionSendSYNNewSeq,          kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvRST */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventWaitACKTimeout */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventSendACKTimeout */
    {iAP2LinkActionDetach,                          kiAP2LinkStateDetached},   /* kiAP2LinkEventDetach */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventRecvData */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventRecvLastData */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventEAK */
    {iAP2LinkActionNone,                            kiAP2LinkStateSuspend},    /* kiAP2LinkEventDataToSend */
    {iAP2LinkActionNotifyConnectionFail,            kiAP2LinkStateFailed},     /* kiAP2LinkEventMaxResend */
    {iAP2LinkActionHandleACK,                       kiAP2LinkStateSuspend},    /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};

const iAP2FSMEvent_t iAP2LinkAccessoryFailedStateEvents [kiAP2LinkEventCount] =
{
    /* Action handler  ...........................  Next State  ................  Event
     * ---------------                              -----------                   ------ */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventInitDone */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventAttach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYN */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKOLD */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvSYNACKNEW */
    {iAP2LinkAccessoryActionSendSYNNewSeq,          kiAP2LinkStateIdle},       /* kiAP2LinkEventRecvRST */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitACKTimeout */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventSendACKTimeout */
    {iAP2LinkActionDetach,                          kiAP2LinkStateDetached},   /* kiAP2LinkEventDetach */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvLastData */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventEAK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventDataToSend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventMaxResend */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACK */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvACKBadLink */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetect */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecvDetectBad */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventRecviAP1Packet */
    {NULL,                                          kiAP2LinkStateCount},      /* kiAP2LinkEventWaitDetectTimeout */
    {NULL,                                          kiAP2LinkStateCount}       /* kiAP2LinkEventSuspend */
};



const iAP2FSMState_t iAP2LinkAccessoryStates [kiAP2LinkStateCount] =
{
    {iAP2LinkAccessoryInitStateEvents},      /* kiAP2LinkStateInit */
    {iAP2LinkAccessoryDetachedStateEvents},  /* kiAP2LinkStateDetached */
    {iAP2LinkAccessoryDetectStateEvents},    /* kiAP2LinkStateDetect */
    {iAP2LinkAccessoryIdleStateEvents},      /* kiAP2LinkStateIdle */
    {iAP2LinkAccessoryPendingStateEvents},   /* kiAP2LinkStatePending */
    {iAP2LinkAccessoryConnectedStateEvents}, /* kiAP2LinkStateConnected */
    {iAP2LinkAccessorySuspendStateEvents},   /* kiAP2LinkStateSuspend */
    {iAP2LinkAccessoryFailedStateEvents}     /* kiAP2LinkStateFailed */
};



/*
****************************************************************
**
**  iAP2LinkAccessoryActionSendDetect
**
****************************************************************
*/
void iAP2LinkAccessoryActionSendDetect (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t* link;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

#if iAP2LINK_DEBUG
    iAP2LogDbg("%s:%d %s sendDetectCB=%p\n",
               __FILE__, __LINE__,
               "Accessory:SendDetect",
               link->sendDetectCB);
#endif
    if (link->detectAckTimeoutID != INVALID_TIMEOUT_ID)
    {
        iAP2TimeCancelTimer (link->mainTimer, link->detectAckTimeoutID);
        link->detectAckTimeoutID = INVALID_TIMEOUT_ID;
    }

    /* Reset some states */
    iAP2LinkResetSeqAck (link, FALSE);

    if (link->sendDetectCB)
    {
        /* Send Detect byte seqeunce */
        (*link->sendDetectCB) (link, FALSE);

        /* start ACK_Timer */
        {
            link->detectAckTimeoutID = iAP2TimeCallbackAfter (link->mainTimer,
                                                              kiAP2LinkEventWaitDetectTimeout,
                                                              1000); /* 1 second */
        }
    }
}


/*
****************************************************************
**
**  iAP2LinkAccessoryActionResendSYN
**
****************************************************************
*/
void iAP2LinkAccessoryActionResendSYN (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t*   link;
    iAP2Packet_t* pck;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    if (link->detectAckTimeoutID != INVALID_TIMEOUT_ID)
    {
        iAP2TimeCancelTimer (link->mainTimer, link->detectAckTimeoutID);
        link->detectAckTimeoutID = INVALID_TIMEOUT_ID;
    }

    memcpy (&(link->negotiatedParam),
            &(link->initParam),
            sizeof(link->initParam));

    if (link->connectedCB)
    {
        /* Inform not connected */
        (*link->connectedCB) (link, FALSE);
    }

    /* Send SYN */
    if (!link->bValidSentSeq)
    {
        link->sentSeq = link->startSeq;
        link->bValidSentSeq = TRUE;
    }
    pck = iAP2PacketCreateSYNPacket (link,
                                     link->sentSeq,
                                     NULL,
                                     link->initParam.version,
                                     link->negotiatedParam.maxOutstandingPackets,
                                     link->negotiatedParam.maxPacketSize,
                                     link->negotiatedParam.retransmitTimeout,
                                     link->negotiatedParam.cumAckTimeout,
                                     link->negotiatedParam.maxRetransmissions,
                                     link->negotiatedParam.maxCumAck,
                                     link->negotiatedParam.numSessionInfo,
                                     link->negotiatedParam.sessionInfo);
    if (link->bIgnoreSynRetryLimit ||
        link->numResentSYN < link->param.maxRetransmissions)
    {
        ++(link->numResentSYN);
#if iAP2_LINK_ALLOW_STATS
        ++(link->noAckReTxCount);
#endif

        pck->retransmitCount = link->numResentSYN;

        iAP2LinkSendPacket (link, pck, TRUE, "Accessory:ResendSYN");
    }
    else
    {
        iAP2LogError("%s:%d %s Resend too many times! max=%d\n",
                     __FILE__, __LINE__, "Accessory:ResendSYN", link->param.maxRetransmissions);
        *nextEvent = kiAP2LinkEventMaxResend;
        iAP2PacketDelete (pck);
    }
}


/*
****************************************************************
**
**  iAP2LinkAccessoryActionSendSYNNewSeq
**
****************************************************************
*/
void iAP2LinkAccessoryActionSendSYNNewSeq (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t*   link;
    iAP2Packet_t* pck;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    if (link->detectAckTimeoutID != INVALID_TIMEOUT_ID)
    {
        iAP2TimeCancelTimer (link->mainTimer, link->detectAckTimeoutID);
        link->detectAckTimeoutID = INVALID_TIMEOUT_ID;
    }

    iAP2LinkSetDefaultSYNParam (&(link->param));

    /* reset seq and ack numbers */
    iAP2LinkResetSeqAck (link, FALSE);

    /* reset negotiatedParam */
    memcpy (&(link->negotiatedParam),
            &(link->initParam),
            sizeof(link->initParam));

    /* Send SYN */
    if (!link->bValidSentSeq)
    {
        link->sentSeq = link->startSeq;
        link->bValidSentSeq = TRUE;
    }
    pck = iAP2PacketCreateSYNPacket (link,
                                     link->sentSeq+1,
                                     NULL,
                                     link->initParam.version,
                                     link->negotiatedParam.maxOutstandingPackets,
                                     link->negotiatedParam.maxPacketSize,
                                     link->negotiatedParam.retransmitTimeout,
                                     link->negotiatedParam.cumAckTimeout,
                                     link->negotiatedParam.maxRetransmissions,
                                     link->negotiatedParam.maxCumAck,
                                     link->negotiatedParam.numSessionInfo,
                                     link->negotiatedParam.sessionInfo);
    ++(link->numSentSYN);
    ++(link->numSentCumSYN);
    if (!link->bIgnoreSynRetryLimit &&
        (link->numSentSYNACK + link->numSentSYN) > kiAP2LinkSynRetries)
    {
        *nextEvent = kiAP2LinkEventMaxResend;
    }
    iAP2LinkSendPacket (link, pck, FALSE, "Accessory:SendSYN");
}


/*
****************************************************************
**
**  iAP2LinkAccessoryActionConnectedACK
**
****************************************************************
*/
void iAP2LinkAccessoryActionConnectedACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t*         link;
    iAP2Packet_t*       recvPck;
    iAP2PacketSYNData_t synParam;
    iAP2Packet_t*       pck;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    if (fsm->currentState == kiAP2LinkStatePending)
    {
        if (link->connectedCB)
        {
            /* Notify about the connection establishment */
            (*link->connectedCB) (link, FALSE);
        }
    }

    /* Handle SYN Param negotiation */
    recvPck = link->recvPck;
    if (recvPck != NULL)
    {
        uint8_t  maxOutstandingPackets  = link->negotiatedParam.maxOutstandingPackets;
        uint16_t maxPacketSize          = link->negotiatedParam.maxPacketSize;

        assert ((recvPck->pckData->ctl & kIAP2PacketControlMaskSYN) != 0);

        iAP2PacketParseSYNData (iAP2PacketGetPayload (recvPck),
                                iAP2PacketGetPayloadLen (recvPck),
                                &synParam);

        memcpy (&(link->negotiatedParam),
                &synParam,
                sizeof(synParam));

        link->negotiatedParam.maxOutstandingPackets     = maxOutstandingPackets;
        link->negotiatedParam.maxPacketSize             = maxPacketSize;
        link->negotiatedParam.peerMaxOutstandingPackets = synParam.maxOutstandingPackets;
        link->negotiatedParam.peerMaxPacketSize         = synParam.maxPacketSize;
    }

    /* Update syn param with negotiated one */
    memcpy (&(link->param),
            &(link->negotiatedParam),
            sizeof(link->negotiatedParam));

    /* Send ACK */
    pck = iAP2PacketCreateACKPacket (link,
                                     link->sentSeq,
                                     link->recvSeq,
                                     NULL,
                                     0,
                                     0);
#if iAP2_LINK_ALLOW_STATS
    ++(link->numSentACK);
#endif
    iAP2LinkSendPacket (link, pck, FALSE, "Accessory:SendACK");

    /* Call connected callback */
    if (link->connectedCB)
    {
        /* Notify about the connection establishment */
        (*link->connectedCB) (link, TRUE);
    }
}


/*
****************************************************************
**
**  iAP2LinkAccessoryActionSendSYNACK
**
****************************************************************
*/
void iAP2LinkAccessoryActionSendSYNACK (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    iAP2Link_t*         link;
    iAP2Packet_t*       recvPck;
    iAP2PacketSYNData_t synParam;
    iAP2Packet_t*       pck;

    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;

    link = (iAP2Link_t*) fsm->data;

    /* Handle SYN Param negotiation */
    recvPck = link->recvPck;
    if (recvPck != NULL)
    {
        iAP2PacketParseSYNData (iAP2PacketGetPayload (recvPck),
                                iAP2PacketGetPayloadLen (recvPck),
                                &synParam);
        if (link->negotiatedParam.version > synParam.version)
        {
            iAP2Log("%s:%d Older protocol Version detected (%d->%d)\n",
                    __FILE__, __LINE__, link->negotiatedParam.version, synParam.version);
            link->negotiatedParam.version = synParam.version;
        }

        iAP2LinkValidateSynParam (&synParam);
        iAP2LinkValidateSynParam (&(link->negotiatedParam));

        if (link->negotiatedParam.version > synParam.version)
        {
            link->negotiatedParam.version = synParam.version;
        }
        if (link->negotiatedParam.maxRetransmissions > synParam.maxRetransmissions)
        {
            link->negotiatedParam.maxRetransmissions = synParam.maxRetransmissions;
        }
        if (link->negotiatedParam.maxCumAck > synParam.maxCumAck)
        {
            link->negotiatedParam.maxCumAck = synParam.maxCumAck;
        }
        if (link->negotiatedParam.retransmitTimeout > synParam.retransmitTimeout)
        {
            link->negotiatedParam.retransmitTimeout = synParam.retransmitTimeout;
        }
        if (link->negotiatedParam.cumAckTimeout > synParam.cumAckTimeout)
        {
            link->negotiatedParam.cumAckTimeout = synParam.cumAckTimeout;
        }

        link->negotiatedParam.peerMaxOutstandingPackets = synParam.maxOutstandingPackets;
        link->negotiatedParam.peerMaxPacketSize = synParam.maxPacketSize;

        link->negotiatedParam.numSessionInfo = synParam.numSessionInfo;
        memcpy (link->negotiatedParam.sessionInfo,
                synParam.sessionInfo,
                synParam.numSessionInfo * sizeof(synParam.sessionInfo[0]));
    }
    else
    {
        memcpy (&synParam, &link->negotiatedParam, sizeof(link->negotiatedParam));
    }

#if DEBUG
    iAP2LogStart();
    iAP2LogNL("%s:%d SYN negotiated parameters:\n", __FILE__, __LINE__);
    iAP2LogNL("%s:%d     version (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.version, synParam.version);
    iAP2LogNL("%s:%d     maxOutstandingPackets (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.maxOutstandingPackets, synParam.maxOutstandingPackets);
    iAP2LogNL("%s:%d     maxRetransmissions (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.maxRetransmissions, synParam.maxRetransmissions);
    iAP2LogNL("%s:%d     maxCumAck (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.maxCumAck, synParam.maxCumAck);
    iAP2LogNL("%s:%d     maxPacketSize (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.maxPacketSize, synParam.maxPacketSize);
    iAP2LogNL("%s:%d     retransmitTimeout (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.retransmitTimeout, synParam.retransmitTimeout);
    iAP2LogNL("%s:%d     cumAckTimeout (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.cumAckTimeout, synParam.cumAckTimeout);
    iAP2LogNL("%s:%d     peerMaxOutstandingPackets (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.peerMaxOutstandingPackets, synParam.peerMaxOutstandingPackets);
    iAP2LogNL("%s:%d     peerMaxPacketSize (%d, %d)\n", __FILE__, __LINE__,
              link->negotiatedParam.peerMaxPacketSize, synParam.peerMaxPacketSize);
    iAP2LogNL("%s:%d     numSessionInfo (%xh, %xh)\n", __FILE__, __LINE__,
              link->negotiatedParam.numSessionInfo, synParam.numSessionInfo);
    {
        int i;
        for (i = 0; i < synParam.numSessionInfo; ++i)
        {
            iAP2LogNL("%s:%d     session %u [id=%u type=%u ver=%u], [id=%u type=%u ver=%u]\n",
                      __FILE__, __LINE__,
                      i,
                      link->negotiatedParam.sessionInfo[i].id,
                      link->negotiatedParam.sessionInfo[i].type,
                      link->negotiatedParam.sessionInfo[i].version,
                      synParam.sessionInfo[i].id,
                      synParam.sessionInfo[i].type,
                      synParam.sessionInfo[i].version);
        }
    }
    iAP2LogStop();
#endif

    /* Send SYN,ACK */
    if (!link->bValidSentSeq)
    {
        link->sentSeq = link->startSeq;
        link->bValidSentSeq = TRUE;
    }
    pck = iAP2PacketCreateSYNPacket (link,
                                     link->sentSeq+1,
                                     &link->recvSeq,
                                     link->initParam.version,
                                     link->negotiatedParam.maxOutstandingPackets,
                                     link->negotiatedParam.maxPacketSize,
                                     link->negotiatedParam.retransmitTimeout,
                                     link->negotiatedParam.cumAckTimeout,
                                     link->negotiatedParam.maxRetransmissions,
                                     link->negotiatedParam.maxCumAck,
                                     link->negotiatedParam.numSessionInfo,
                                     link->negotiatedParam.sessionInfo);
    ++(link->numSentSYNACK);
    ++(link->numSentCumSYNACK);
    if (!link->bIgnoreSynRetryLimit &&
        (link->numSentSYNACK + link->numSentSYN) > kiAP2LinkSynRetries)
    {
        *nextEvent = kiAP2LinkEventMaxResend;
    }
    iAP2LinkSendPacket (link, pck, FALSE, "Accessory:SendSYNACK");
}


/*
****************************************************************
**
**  iAP2LinkAccessoryActionHandleSuspend
**
****************************************************************
*/
void iAP2LinkAccessoryActionHandleSuspend (struct iAP2FSM_st* fsm, unsigned int* nextEvent)
{
    assert(fsm);
    assert(nextEvent);
    *nextEvent = kiAP2LinkEventCount;
}


#endif /*#if iAP2_FOR_ACCESSORY*/


