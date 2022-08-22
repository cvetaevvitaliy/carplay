/*
 *	File: iAP2BuffPoolImplementation.h
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

#ifndef iAP2Utility_iAP2BuffPoolImplementation_h
#define iAP2Utility_iAP2BuffPoolImplementation_h

#include <iAP2BuffPool.h>

#ifdef __cplusplus
extern "C" {
#endif



/*
****************************************************************
**
**  iAP2BuffPool user implemented
**
**  - Could be implemented as just a list of pre-allocated buffers of
**      maximum expected size.
**
****************************************************************
*/



/*
****************************************************************
**
**  Buff List
**
**  This is a generic buff pool used by:
**      iAP2Packet - iAP2PacketGetMissingSeqFromEAK()
**                      for temporary and return buffer to store
**                      upto max packet window size elements of at most uint16.
**                      Max size is (maxSendWindow * 2).
**                      (the returned buffer is iAP2BuffPoolReturn'ed in iAP2Link).
**      iAP2FileTransfer - Receive/Send data buffers.
**                          Should account for max number of simultaneous
**                          file transfers and max data size expected during
**                          the transfers.
**
**  The list of buffers could be tracked separately based on expected size requirements.
**
****************************************************************
*/

/*
****************************************************************
**
**  __iAP2BuffPoolGetBuffSizeBuffList
**
**  Input:
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
uint32_t __iAP2BuffPoolGetBuffSizeBuffList (uint32_t maxBuffSize,
                                            uint16_t maxBuffCount);

/*
****************************************************************
**
**  __iAP2BuffPoolInitBuffList
**
**  Input:
**      buffPool:       iAP2BuffPool_t type pointer to buff
**      context:        context to maintain with buffPool
**      maxBuffSize:    size of buffers
**      maxBuffCount:   number of buffers that will be maintained
**      buff:           memory to use for iAP2BuffPool
**
**  Output:
**      Initialized buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolInitBuffList (iAP2BuffPool_t* buffPool,
                                 uintptr_t       context,
                                 uint32_t        maxBuffSize,
                                 uint16_t        maxBuffCount,
                                 uint8_t*        buff);

/*
****************************************************************
**
**  __iAP2BuffPoolCleanupBuffList
**
**  Input:
**      buffPool:   buffPool to cleanup
**
**  Output:
**      Cleaned up buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolCleanupBuffList (iAP2BuffPool_t* buffPool);

/*
****************************************************************
**
**  __iAP2BuffPoolGetBuff
**
**  Input:
**      buffPool:   buffPool to get buffer from
**      payloadLen: hint on size of buffer required
**
**  Output:
**      A buffer is reserved/allocated
**
**  Return:
**      void*   pointer to reserved/allocated buffer
**
****************************************************************
*/
void* __iAP2BuffPoolGetBuff (iAP2BuffPool_t* buffPool, uint32_t payloadLen);

/*
****************************************************************
**
**  __iAP2BuffPoolReturnBuff
**
**  Input:
**      buffPool:   buffPool to return buffer to
**      buff:       buffer to return
**
**  Output:
**      A buffer is returned to pool (free'd)
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolReturnBuff (iAP2BuffPool_t* buffPool, void* buff);



/*
****************************************************************
**
**  Send Packet List
**
**  This is a packet buff pool used for send direction.
**  Size of packet buff is dependent on max send packet size of ther link layer.
**  The max send packet size maybe based on peer or based on implementation's
**  send packet size limitation.
**
****************************************************************
*/

/*
****************************************************************
**
**  __iAP2BuffPoolGetBuffSizeSendPacketList
**
**  Input:
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
uint32_t __iAP2BuffPoolGetBuffSizeSendPacketList (uint32_t maxBuffSize,
                                                  uint16_t maxBuffCount);

/*
****************************************************************
**
**  __iAP2BuffPoolInitSendPacketList
**
**  Input:
**      buffPool:       iAP2BuffPool_t type pointer to buff
**      context:        context to maintain with buffPool
**      maxBuffSize:    size of buffers
**      maxBuffCount:   number of buffers that will be maintained
**      buff:           memory to use for iAP2BuffPool
**
**  Output:
**      Initialized buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolInitSendPacketList (iAP2BuffPool_t* buffPool,
                                       uintptr_t       context,
                                       uint32_t        maxBuffSize,
                                       uint16_t        maxBuffCount,
                                       uint8_t*        buff);

/*
****************************************************************
**
**  __iAP2BuffPoolCleanupSendPacketList
**
**  Input:
**      buffPool:   buffPool to cleanup
**
**  Output:
**      Cleaned up buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolCleanupSendPacketList (iAP2BuffPool_t* buffPool);

/*
****************************************************************
**
**  __iAP2BuffPoolGetSendPacket
**
**  Input:
**      buffPool:   buffPool to get packet from
**      payloadLen: hint on size of packet buffer required
**
**  Output:
**      A packet buffer is reserved/allocated
**
**  Return:
**      void*   pointer to reserved/allocated packet
**
****************************************************************
*/
void* __iAP2BuffPoolGetSendPacket (iAP2BuffPool_t* buffPool, uint32_t payloadLen);

/*
****************************************************************
**
**  __iAP2BuffPoolReturnSendPacket
**
**  Input:
**      buffPool:   buffPool to return packet to
**      packet:     packet to return
**
**  Output:
**      A packet is returned to pool (free'd)
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolReturnSendPacket (iAP2BuffPool_t* buffPool, void* packet);



/*
****************************************************************
**
**  Recv Packet List
**
**  This is a packet buff pool used for send direction.
**  Size of packet buff is dependent on max send packet size of ther link layer.
**  The max send packet size maybe based on peer or based on implementation's
**  send packet size limitation.
**
****************************************************************
*/

/*
****************************************************************
**
**  __iAP2BuffPoolGetBuffSizeRecvPacketList
**
**  Input:
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
uint32_t __iAP2BuffPoolGetBuffSizeRecvPacketList (uint32_t maxBuffSize,
                                                  uint16_t maxBuffCount);

/*
****************************************************************
**
**  __iAP2BuffPoolInitRecvPacketList
**
**  Input:
**      buffPool:       iAP2BuffPool_t type pointer to buff
**      context:        context to maintain with buffPool
**      maxBuffSize:    size of buffers
**      maxBuffCount:   number of buffers that will be maintained
**      buff:           memory to use for iAP2BuffPool
**
**  Output:
**      Initialized buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolInitRecvPacketList (iAP2BuffPool_t* buffPool,
                                       uintptr_t       context,
                                       uint32_t        maxBuffSize,
                                       uint16_t        maxBuffCount,
                                       uint8_t*        buff);

/*
****************************************************************
**
**  __iAP2BuffPoolCleanupRecvPacketList
**
**  Input:
**      buffPool:   buffPool to cleanup
**
**  Output:
**      Cleaned up buffPool
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolCleanupRecvPacketList (iAP2BuffPool_t* buffPool);

/*
****************************************************************
**
**  __iAP2BuffPoolGetRecvPacket
**
**  Input:
**      buffPool:   buffPool to get packet from
**      payloadLen: hint on size of packet buffer required
**
**  Output:
**      A packet buffer is reserved/allocated
**
**  Return:
**      void*   pointer to reserved/allocated packet
**
****************************************************************
*/
void* __iAP2BuffPoolGetRecvPacket (iAP2BuffPool_t* buffPool, uint32_t payloadLen);

/*
****************************************************************
**
**  __iAP2BuffPoolReturnRecvPacket
**
**  Input:
**      buffPool:   buffPool to return packet to
**      packet:     packet to return
**
**  Output:
**      A packet is returned to pool (free'd)
**
**  Return:
**      None
**
****************************************************************
*/
void __iAP2BuffPoolReturnRecvPacket (iAP2BuffPool_t* buffPool, void* packet);


#ifdef __cplusplus
}
#endif

#endif
