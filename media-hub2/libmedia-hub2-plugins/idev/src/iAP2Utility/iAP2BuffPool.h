/*
 *	File: iAP2BuffPool.h
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

#ifndef iAP2Utility_iAP2BuffPool_h
#define iAP2Utility_iAP2BuffPool_h

#include <stdint.h>
#include <sys/time.h>
#include <iAP2Defines.h>


#ifdef __cplusplus
extern "C" {
#endif


enum
{
    kiAP2BuffPoolTypeBuff           = 0,    /* Buffer for general; send/recv buffers: param is buff size */
    kiAP2BuffPoolTypeSendPacket     = 1,    /* Buffer for send packet: param is payload size */
    kiAP2BuffPoolTypeRecvPacket     = 2,    /* Buffer for recv packet: param is payload size */

    kiAP2BuffPoolTypeCount
};


typedef struct iAP2BuffPool_st
{
    uint8_t     type;
    uint16_t    buffCount;
    uint32_t    buffSize;
    uintptr_t   context;
    uintptr_t   data;

} iAP2BuffPool_t;


/*
 ****************************************************************
 **
 **  iAP2BuffPoolGetBuffSize
 **
 **  Input:
 **      buffType:       Type of buffer
 **      maxBuffSize:    size of buffers
 **      maxBuffCount:   number of buffers that will be maintained
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint32_t    minimum size of memory required for proper operation
 **
 ****************************************************************
 */
uint32_t iAP2BuffPoolGetBuffSize (uint8_t         buffType,
                                  uint32_t        maxBuffSize,
                                  uint16_t        maxBuffCount);


/*
****************************************************************
**
**  iAP2BuffPoolInit
**
**  Input:
**      buffType:       Type of buffer for this buffPool
**      context:        context to associate with BuffPool
**      maxBuffSize:    size of buffers
**      maxBuffCount:   number of buffers that will be maintained
**      buff:           memory to use for iAP2BuffPool
**
**  Output:
**      None
**
**  Return:
**      iAP2BuffPool_t*     pointer to initialized iAP2BuffPool_t structure
**
**  Note: Must be called before using any other functions.
**        It is implementation specific how the pool of buffers is maintained.
**
****************************************************************
*/
iAP2BuffPool_t* iAP2BuffPoolInit (uint8_t   buffType,
                                  uintptr_t context,
                                  uint32_t  maxBuffSize,
                                  uint16_t  maxBuffCount,
                                  uint8_t*  buff);


/*
****************************************************************
**
**  iAP2BuffPoolCleanup
**
**  Input:
**      buffPool:   iAP2BuffPool_t structure to initialize
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Called when iAP2BuffPool functions are no longer required.
**        It is implementation specific how the pool of buffers is maintained.
**
****************************************************************
*/
void iAP2BuffPoolCleanup (iAP2BuffPool_t* buffPool);


/*
****************************************************************
**
**  iAP2BuffPoolGet
**
**  Input:
**      buffPool:   iAP2BuffPool_t structure
**      param:      Parameter associtated with buffType for getting a free buffer
**
**  Output:
**      Passed in timeBuff is initialized.
**
**  Return:
**      iAP2BuffPool_t*    passed in timeBuff
**
**  Note: It is implementation specific how the pool of buffers is maintained.
**        (per type with max size; malloc heap; ...)
**
****************************************************************
*/
void* iAP2BuffPoolGet (iAP2BuffPool_t*  buffPool,
                       uintptr_t        param);


/*
****************************************************************
**
**  iAP2BuffPoolReturn
**
**  Input:
**      buffPool:   iAP2BuffPool_t structure
**      buff:       Buffer to return to pool
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2BuffPoolReturn (iAP2BuffPool_t*    buffPool,
                         void*              buff);


#ifdef __cplusplus
}
#endif

#endif
