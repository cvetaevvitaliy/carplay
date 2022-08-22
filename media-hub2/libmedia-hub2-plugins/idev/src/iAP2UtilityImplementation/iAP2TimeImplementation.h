/*
 *	File: iAP2TimeImplementation.h
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

#ifndef iAP2Utility_iAP2TimeImplementation_h
#define iAP2Utility_iAP2TimeImplementation_h

#include <stdint.h>
#include <sys/time.h>
#include <iAP2Defines.h>
#include <iAP2Time.h>
#include <iAP2ListArray.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct iAP2Timeout_st
{
    uint32_t    timeout;
    uint8_t     id;
    uint8_t     type;   /* user defined timeout type */
} iAP2Timeout_t;

struct iAP2Timer_st
{
    uint8_t*            timeBuff;
    uint8_t*            timeBuffNextFree;
    void*               link;   /* context */
    void*               context3;
    iAP2TimeExpireCB_t  expiredCB;
    iAP2TimeCancelCB_t  cancelCB;
    uint8_t*            listBuffer;
    uint32_t            runningTimeOut;
    uint8_t             nextID;
    uint8_t             maxTimeOuts;

    /*
    ** Following are for compatibility with previous implementation...
    ** can be removed once all dependent code is fixed.
    */
    uint32_t            origTime;
    uint32_t            timeout;
    const char*         tag;
    uint32_t            tagData;
    void*               packet;
    BOOL                inReleaseList;

};


/*
****************************************************************
**
** User implemented functions
**
****************************************************************
*/

/*
****************************************************************
**
**  _iAP2TimeCleanupCallback
**
**  Input:
**      timer:      pointer to timer to cleanup
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void _iAP2TimeCleanupCallback (iAP2Timer_t* timer);


/*
****************************************************************
**
**  _iAP2TimeCallbackAfter
**
**  Input:
**      timer:      timer
**      delayMs:    amount of time to wait before callback is called
**      callback:   function to callback on timeout
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if successful in registering callback, else FALSE
**
**  Note:   This must be implemented by user of the library.
**          Call must return without blocking...
**          timer should be processed asynchronously.
**          Call to callback must occur either on the same thread
**          as the one processing link layer state machine.
**
****************************************************************
*/
BOOL _iAP2TimeCallbackAfter (iAP2Timer_t* timer,
                             uint32_t     delayMs,
                             iAP2TimeCB_t callback);


/*
****************************************************************
**
**  _iAP2TimePerformCallback
**
**  Input:
**      timer:      timer
**      callback:   function to callback on timeout
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   This must be implemented by user of the library.
**          Call must return without blocking...
**          callback should be processed asynchronously.
**          Call to callback must occur either on the same thread
**          as the one processing link layer state machine.
**
****************************************************************
*/
void _iAP2TimePerformCallback (iAP2Timer_t* timer,
                               iAP2TimeCB_t callback);


/*
****************************************************************
**
**  _iAP2TimeCancelCallback
**
**  Input:
**      timer:      timer
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   This must be implemented by user of the library.
**
****************************************************************
*/
void _iAP2TimeCancelCallback (iAP2Timer_t* timer);


#ifdef __cplusplus
}
#endif

#endif
