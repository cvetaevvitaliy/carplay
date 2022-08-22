/*
 *	File: iAP2FSM.h
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

#ifndef iAP2Utility_iAP2FSM_h
#define iAP2Utility_iAP2FSM_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iAP2FSM_st;

/*
****************************************************************
**
**  iAP2FSMEventAction_t
**
**  Input:
**      iAP2FSM_t*      fsm:        Pointer to iAP2FSM_t structure.
**
**  Output:
**      unsgined int*   nextEvent:  Next event to generate if applicable
**
**  Return:
**      None
**
**  Note:   nextEvent >= eventCount is returned if no next event
**          should be generated.
****************************************************************
*/
typedef void (*iAP2FSMEventAction_t) (struct iAP2FSM_st* fsm, unsigned int* nextEvent);


typedef struct iAP2FSMEvent_st
{
    iAP2FSMEventAction_t    action;
    unsigned int            nextState;

} iAP2FSMEvent_t;

typedef struct iAP2FSMState_st
{
    const iAP2FSMEvent_t*   events;

} iAP2FSMState_t;

typedef struct iAP2FSM_st
{
    /* State and Event enumeration is zero based. */

    const iAP2FSMState_t* states;

    unsigned int    stateCount;
    unsigned int    eventCount;

    unsigned int    currentState;
    unsigned int    currentEvent;

    void*           data;

    const char*     name;
    const char**    stateNames;
    const char**    eventNames;

} iAP2FSM_t;


/*
****************************************************************
**
**  iAP2FSMGetActionForEvent
**
**  Input:
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure.
**      event       event to get the action handler for.
**
**  Output:
**      None
**
**  Return:
**      iAP2FSMEventHandler_t  event action handler.
**
****************************************************************
*/
iAP2FSMEventAction_t iAP2FSMGetActionForEvent (iAP2FSM_t*     fsm,
                                               unsigned int   event);


/*
****************************************************************
**
**  iAP2FSMGetNextStateForEvent
**
**  Input:
**      iAP2FSM_t*  Pointer to iAP2FSM_t structure.
**      event       event to get the action handler for.
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
                                          unsigned int  event);


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
uint32_t iAP2FSMGetBuffSize (void);


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
                          uint8_t*                      fsmBuffer);


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
void iAP2FSMDelete (iAP2FSM_t* fsm);


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
void iAP2FSMHandleEvent (iAP2FSM_t* fsm, unsigned int event);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Utility_iAP2FSM_h */
