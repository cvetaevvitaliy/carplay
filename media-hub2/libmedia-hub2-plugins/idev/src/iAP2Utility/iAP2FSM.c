/*
 *	File: iAP2FSM.c
 *	Package: iAP2Utility
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "iAP2FSM.h"
#include "iAP2Log.h"

#if DEBUG
#define iAP2FSM_DEBUG 1
#endif

/*
****************************************************************
**
**  iAP2FSMGetActionForEvent
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure.
**      event       event to get the action handler for.
**
**  Input:
**
**  Output:
**      None
**
**  Return:
**      iAP2FSMEventAction_t    event action handler.
**
****************************************************************
*/
iAP2FSMEventAction_t iAP2FSMGetActionForEvent (iAP2FSM_t*     fsm,
                                               unsigned int   event)
{
    assert(fsm);
    return (fsm->states[fsm->currentState].events)[event].action;
}


/*
****************************************************************
**
**  iAP2FSMGetNextStateForEvent
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure.
**      event       event to get the action handler for.
**
**  Input:
**
**  Output:
**      None
**
**  Return:
**      unsigned int    next event to transition to.
**
****************************************************************
*/
unsigned int iAP2FSMGetNextStateForEvent (iAP2FSM_t*    fsm,
                                          unsigned int  event)
{
    assert(fsm);
    return (fsm->states[fsm->currentState].events)[event].nextState;
}


/*
****************************************************************
**
**  iAP2FSMGetBuffSize
**
**  Input:
**      None
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
uint32_t iAP2FSMGetBuffSize (void)
{
    uint32_t result = sizeof(iAP2FSM_t);
    return result;
}


/*
****************************************************************
**
**  iAP2FSMCreate
**
**  Input:
**      stateCount:     Number of states
**      initialState:   Initial state
**      eventCount:     Number of events
**      states:         Table describing state, action, transition
**      name:           Optional name to assign to this FSM.
**      stateNames:     List of state names for display in logging only.
**      eventNames:     List of event names for display in logging only.
**      fsmBuffer:      Pre-allocated buffer for storing iAP2FSM_t struct.
**
**  Output:
**      Passed in fsmBuffer is initiliazed.
**
**  Return:
**      iAP2FSM_t*  pointer to new allocated iAP2FSM_t structure
**                      or the passed in fsmBuffer.
**
****************************************************************
*/
iAP2FSM_t* iAP2FSMCreate (unsigned int                  stateCount,
                          unsigned int                  initialState,
                          unsigned int                  eventCount,
                          const iAP2FSMState_t*         states,
                          void*                         data,
                          const char*                   name,
                          const char**                  stateNames,
                          const char**                  eventNames,
                          uint8_t*                      fsmBuffer)
{
    iAP2FSM_t* fsm = (iAP2FSM_t*) fsmBuffer;
    assert(fsm);

    fsm->states         = states;
    fsm->eventCount     = eventCount;
    fsm->stateCount     = stateCount;
    fsm->currentState   = initialState;
    fsm->currentEvent   = eventCount;
    fsm->data           = data;
    fsm->name           = name;
    fsm->stateNames     = stateNames;
    fsm->eventNames     = eventNames;

    return fsm;
}


/*
****************************************************************
**
**  iAP2FSMDelete
**
**  Input:
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure to delete.
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2FSMDelete (iAP2FSM_t* fsm)
{
    /* Do Nothing */
}


/*
****************************************************************
**
**  iAP2FSMHandleEvent
**
**  Input:
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure to delete.
**      event       Event to handle.
**
**  Output:
**      iAP2FSM_t*  fsm is updated to reflect results of event handling
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2FSMHandleEvent (iAP2FSM_t* fsm, unsigned int event)
{
    unsigned int tmpEvent = event;
    if (fsm != NULL && tmpEvent < fsm->eventCount)
    {
        while (tmpEvent < fsm->eventCount)
        {
            iAP2FSMEventAction_t action = iAP2FSMGetActionForEvent (fsm, tmpEvent);
            assert(fsm->states);
            if (action != NULL)
            {
#if iAP2FSM_DEBUG
                unsigned int origState = fsm->currentState;
#endif
                unsigned int origEvent = tmpEvent;

                /* Processing event, set current event to event being handled */
                fsm->currentEvent = tmpEvent;

                /* Handle Action for event. */
                (*action) (fsm, &tmpEvent);

                /* Transition to state. */
                fsm->currentState = iAP2FSMGetNextStateForEvent (fsm, origEvent);
#if iAP2FSM_DEBUG
                iAP2LogDbg ("%s:%d '%s'(%p): Handle event(%d:%s)->(%d:%s) for state(%d:%s)->(%d:%s)\n",
                            __FILE__, __LINE__, (fsm->name ? fsm->name : ""), fsm,
                            origEvent, (fsm->eventNames ? fsm->eventNames[origEvent] : ""),
                            tmpEvent, (fsm->eventNames && tmpEvent < fsm->eventCount ? fsm->eventNames[tmpEvent] : ""),
                            origState, (fsm->stateNames ? fsm->stateNames[origState] : ""),
                            fsm->currentState, (fsm->stateNames ? fsm->stateNames[fsm->currentState] : ""));
#endif
                assert(fsm->currentState < fsm->stateCount);
            }
            else
            {
                iAP2LogError("%s:%d '%s'(%p): No handler for event(%d:%s) for state(%d:%s)\n",
                             __FILE__, __LINE__, (fsm->name ? fsm->name : ""), fsm,
                             tmpEvent, (fsm->eventNames ? fsm->eventNames[tmpEvent] : ""),
                             fsm->currentState, (fsm->stateNames ? fsm->stateNames[fsm->currentState] : ""));
                tmpEvent = fsm->eventCount;
            }
        }

        /* Finished processing events, set current event back to invalid */
        fsm->currentEvent = fsm->eventCount;
    }
    else
    {
        iAP2LogError("%s:%d '%s': NULL fsm(%p) pointer or invalid event(%d:%s)!\n",
                     __FILE__, __LINE__, (fsm && fsm->name ? fsm->name : ""), fsm,
                     tmpEvent, (fsm && fsm->eventNames ? fsm->eventNames[tmpEvent] : ""));
    }
}

