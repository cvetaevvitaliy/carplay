/*
 *	File: iAP2Time.h
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

#ifndef iAP2Utility_iAP2Time_h
#define iAP2Utility_iAP2Time_h

#include <stdint.h>
#include <sys/time.h>
#include "iAP2Defines.h"


#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_TIMEOUT_TYPE    0xFF
#define INVALID_TIMEOUT_ID      0xFF
#define INVALID_TIMEOUT_VAL     0xFFFFFFFF
#define MAX_TIMEOUT_VAL         0xFFFFFFFE

struct iAP2Timer_st;

typedef void (*iAP2TimeExpireCB_t) (struct iAP2Timer_st* timer, uint8_t timerID, uint8_t timerType, uint32_t curTime);
typedef void (*iAP2TimeCancelCB_t) (struct iAP2Timer_st* timer, uint8_t timerID);
typedef void (*iAP2TimeCB_t) (struct iAP2Timer_st* timer, uint32_t curTime);

typedef struct iAP2Timer_st iAP2Timer_t;


/*
****************************************************************
**
**  iAP2TimeGetCurTimeMs
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      uint32_t    current time in ms.
**
****************************************************************
*/
uint32_t iAP2TimeGetCurTimeMs (void);


/*
****************************************************************
**
**  iAP2TimeGetCurTimeMsInt64
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeGetCurTimeMsInt64 (void);
    
/*
****************************************************************
**
**  iAP2TimeGetCurTimeMsInt64
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeGetCurTimeMsInt64 (void);
    
/*
****************************************************************
**
**  iAP2TimeGetCurTimeMsInt64
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeGetCurTimeMsInt64 (void);
    
/*
****************************************************************
**
**  iAP2TimeSpecToMs
**
**  Input:
**      time:   pointer to timespec to convert
**
**  Output:
**      None
**
**  Return:
**      uint32_t    current time in ms.
**
****************************************************************
*/
uint32_t iAP2TimeSpecToMs (struct timespec* time);


/*
****************************************************************
**
**  iAP2TimeValToMs
**
**  Input:
**      time:   pointer to timeval to convert
**
**  Output:
**      None
**
**  Return:
**      uint32_t    current time in ms.
**
****************************************************************
*/
uint32_t iAP2TimeValToMs (struct timeval* time);

    
/*
****************************************************************
**
**  iAP2TimeValToMsInt64
**
**  Input:
**      time:   pointer to timeval to convert
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeValToMsInt64 (struct timeval* time);
   
    
/*
****************************************************************
**
**  iAP2TimeValToMsInt64
**
**  Input:
**      time:   pointer to timeval to convert
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeValToMsInt64 (struct timeval* time);
   
    
/*
****************************************************************
**
**  iAP2TimeValToMsInt64
**
**  Input:
**      time:   pointer to timeval to convert
**
**  Output:
**      None
**
**  Return:
**      uint64_t    current time in ms.
**
****************************************************************
*/
uint64_t iAP2TimeValToMsInt64 (struct timeval* time);
   
    
/*
****************************************************************
**
**  iAP2TimeInit
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Must be called before using any other functions.
**        OBSOLETE!!!!
**
****************************************************************
*/
void iAP2TimeInit (void);


/*
****************************************************************
**
**  iAP2TimeCleanup
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Called when iAP2Time functions are no longer required.
**        OBSOLETE!!!!
**
****************************************************************
*/
void iAP2TimeCleanup (void);


/*
****************************************************************
**
**  iAP2TimeGetBuffSize
**
**  Input:
**      maxTimeouts:    max number of timers that can be "active" at once.
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of timeBuff required for proper operation
**
****************************************************************
*/
uint32_t iAP2TimeGetBuffSize (uint8_t maxTimeouts);


/*
****************************************************************
**
**  iAP2TimeCreate
**
**  Input:
**      context:        context to use on calls to user implemented functions.
**                          (should be pointer to link)
**      expiredCB:      function to callback on timer expired
**      cancelCB:       function to callback on timer cancel
**      maxTimeouts:    max number of timers that will be "active" at once.
**
**  Output:
**      Passed in timeBuff is initialized.
**
**  Return:
**      iAP2Timer_t*    passed in timeBuff
**
**  Note:   Only one iAP2TimeInit call and one iAP2Timer_t instance is
**          required per link.
**          One iAP2Timer_t can handle multiple timeOuts upto maxTimeouts.
**
****************************************************************
*/
iAP2Timer_t* iAP2TimeCreate (void*               context,
                             iAP2TimeExpireCB_t  expiredCB,
                             iAP2TimeCancelCB_t  cancelCB,
                             uint8_t             maxTimeouts,
                             uint8_t*            timeBuff);


/*
****************************************************************
**
**  iAP2TimeDelete
**
**  Input:
**      timer:  pointer to timer to cancel
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2TimeDelete (iAP2Timer_t* timer);


/*
****************************************************************
**
**  iAP2TimeGetContext
**
**  Input:
**      timer:  pointer to timer
**
**  Output:
**      None
**
**  Return:
**      void* context associated with the timer
**
****************************************************************
*/
void* iAP2TimeGetContext (iAP2Timer_t* timer);


/*
****************************************************************
**
**  iAP2TimeCallbackAfter
**
**  Input:
**      timer:      timer
**      type:           user defined timeout type
**      delayMs:    amount of time to wait before callback is called
**
**  Output:
**      None
**
**  Return:
**      uint8_t     return INVALID_TIMEOUT_ID if error, else returns the timeoutID
**                  of registered timeOut
**
**  Note:   After call to this function, expireCB function registered in
**          iAP2TimeInit will be called after specified delayMs.
**
****************************************************************
*/
uint8_t iAP2TimeCallbackAfter (iAP2Timer_t* timer,
                               uint8_t      type,
                               uint32_t     delayMs);


/*
****************************************************************
**
**  iAP2TimeCancelTimer
**
**  Input:
**      timer:      timer
**      timeoutID:  timeoutID to cancel
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2TimeCancelTimer (iAP2Timer_t* timer,
                          uint8_t      timeoutID);


/*
****************************************************************
**
**  iAP2TimeGetType
**
**  Input:
**      timer:      timer
**      timeoutID:  timeoutID to type for
**
**  Output:
**      None
**
**  Return:
**      uint8_t     user defined type for this timeoutID
**
****************************************************************
*/
uint8_t iAP2TimeGetType (iAP2Timer_t* timer,
                         uint8_t      timeoutID);


/*
****************************************************************
**
**  iAP2TimePrintInfo
**
**  Input:
**      timer:          timer
**      needStartStop:  TRUE if iAP2LogStart was not called by the caller.
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2TimePrintInfo (iAP2Timer_t* timer, BOOL needStartStop);


#ifdef __cplusplus
}
#endif

#endif
