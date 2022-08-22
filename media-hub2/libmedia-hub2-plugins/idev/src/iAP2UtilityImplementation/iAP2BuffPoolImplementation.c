/*
 *	File: iAP2BuffPoolImplementation.c
 *	Package: iAP2Link
 *	Abstract: n/a 
 *
 *	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple
 * 	Inc. ("Apple"), in your capacity as a current, and in good standing,
 *	Licensee in the MFi Licensing Program. Use of this Apple software is
 *	governed by and subject to the terms and conditions of your MFi License,
 *	including, but not limited to, the restrictions specified in the provision
 *	entitled â€œPublic Softwareâ€? and is further subject to your agreement to
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iAP2LinkConfig.h>
#include <iAP2Defines.h>
#include <iAP2BuffPool.h>
#include <iAP2Log.h>
#include <iAP2ListArray.h>
#include <iAP2Packet.h>
#include <iAP2BuffPoolImplementation.h>


#if iAP2_ALLOW_LARGE_PACKET_MALLOC == 0
#define MAX_ALLOC_SIZE 0x8000
#endif


/*
****************************************************************
**
** iAP2BuffPool implementation
**
****************************************************************
*/

#pragma mark -
#pragma mark iAP2BuffPool BuffList user implementations

/*
typedef struct __iAP2BuffPoolBuffListImplData_st
{

} __iAP2BuffPoolBuffListImplData_t;
*/


/* Function to return amount of memory required for proper operation */
uint32_t __iAP2BuffPoolGetBuffSizeBuffList (uint32_t maxBuffSize,
                                            uint16_t maxBuffCount)
{
    uint32_t result = 0;
/*
    result += sizeof (__iAP2BuffPoolBuffListImplData_t);
    result += (maxBuffSize * maxBuffCount));
 */
    return result;
}

/* Function to initialize send buff buffPool */
void __iAP2BuffPoolInitBuffList (iAP2BuffPool_t* buffPool,
                                 uintptr_t       context,
                                 uint32_t        maxBuffSize,
                                 uint16_t        maxBuffCount,
                                 uint8_t*        buff)
{
    buffPool->data = 0;

    /* Do nothing */
}

/* Function to cleanup send buff buffPool */
void __iAP2BuffPoolCleanupBuffList (iAP2BuffPool_t* buffPool)
{
    /* Do nothing */
}

/* Function to Get a send buff from the buffPool */
void* __iAP2BuffPoolGetBuff (iAP2BuffPool_t* buffPool, uint32_t buffLen)
{
    return malloc (buffLen);
}

/* Function to Return a send buff from the buffPool */
void __iAP2BuffPoolReturnBuff (iAP2BuffPool_t* buffPool, void* buff)
{
    free (buff);
}



#pragma mark -
#pragma mark iAP2BuffPool SendPacketList user implementations

/*
typedef struct __iAP2BuffPoolSendPacketListImplData_st
{

} __iAP2BuffPoolSendPacketListImplData_t;
*/


/* Function to return amount of memory required for proper operation */
uint32_t __iAP2BuffPoolGetBuffSizeSendPacketList (uint32_t maxBuffSize,
                                                  uint16_t maxBuffCount)
{
    uint32_t result = 0;
/*
    result += sizeof (__iAP2BuffPoolSendPacketListImplData_t);
    result += (maxBuffSize * maxBuffCount));
 */
    return result;
}

/* Function to initialize send packet buffPool */
void __iAP2BuffPoolInitSendPacketList (iAP2BuffPool_t* buffPool,
                                       uintptr_t       context,
                                       uint32_t        maxBuffSize,
                                       uint16_t        maxBuffCount,
                                       uint8_t*        buff)
{
    buffPool->data = 0;

    /* Do nothing */
}

/* Function to cleanup send packet buffPool */
void __iAP2BuffPoolCleanupSendPacketList (iAP2BuffPool_t* buffPool)
{
    /* Do nothing */
}

/* Function to Get a send packet from the buffPool */
void* __iAP2BuffPoolGetSendPacket (iAP2BuffPool_t* buffPool, uint32_t payloadLen)
{
    iAP2Packet_t* pck = NULL;
    uint32_t pckLen = kIAP2PacketHeaderLen + payloadLen + kIAP2PacketChksumLen;
#if iAP2_ALLOW_LARGE_PACKET_MALLOC
    pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t) + pckLen);
	if( pck == NULL )
	{
		perror( "malloc error" );
		return NULL;	
	}
//	assert(pck);
    pck->pckData = (iAP2PacketData_t*) (pck + 1);
#else
    if (pckLen <= MAX_ALLOC_SIZE)
    {
        pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t) + pckLen);
        assert(pck);
        pck->pckData = (iAP2PacketData_t*) (pck + 1);
    }
    else
    {
        pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t));
        assert(pck);
        pck->pckData = (iAP2PacketData_t*) malloc (pckLen);
        assert(pck->pckData);
    }
#endif
    return pck;
}

/* Function to Return a send packet to the buffPool */
void __iAP2BuffPoolReturnSendPacket (iAP2BuffPool_t* buffPool, void* packet)
{
#if iAP2_ALLOW_LARGE_PACKET_MALLOC
    free (packet);
#else
    iAP2Packet_t* pck = (iAP2Packet_t*) packet;
    if (pck->packetLen <= MAX_ALLOC_SIZE)
    {
        free (pck);
    }
    else
    {
        free (pck->pckData);
        free (pck);
    }
#endif
}



#pragma mark -
#pragma mark iAP2BuffPool RecvPacketList user implementations

/*
typedef struct __iAP2BuffPoolRecvPacketListImplData_st
{

} __iAP2BuffPoolRecvPacketListImplData_t;
*/


/* Function to return amount of memory required for proper operation */
uint32_t __iAP2BuffPoolGetBuffSizeRecvPacketList (uint32_t maxBuffSize,
                                                  uint16_t maxBuffCount)
{
    uint32_t result = 0;
/*
    result += sizeof (__iAP2BuffPoolRecvPacketListImplData_t);
    result += (maxBuffSize * maxBuffCount));
 */
    return result;
}

/* Function to initialize recv packet buffPool */
void __iAP2BuffPoolInitRecvPacketList (iAP2BuffPool_t* buffPool,
                                       uintptr_t       context,
                                       uint32_t        maxBuffSize,
                                       uint16_t        maxBuffCount,
                                       uint8_t*        buff)
{
    buffPool->data = 0;

    /* Do nothing */
}

/* Function to cleanup recv packet buffPool */
void __iAP2BuffPoolCleanupRecvPacketList (iAP2BuffPool_t* buffPool)
{
    /* Do nothing */
}

/* Function to Get a recv packet from the buffPool */
void* __iAP2BuffPoolGetRecvPacket (iAP2BuffPool_t* buffPool, uint32_t payloadLen)
{
    iAP2Packet_t* pck = NULL;
    uint32_t pckLen = kIAP2PacketHeaderLen + payloadLen + kIAP2PacketChksumLen;
#if iAP2_ALLOW_LARGE_PACKET_MALLOC
    pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t) + pckLen);
    assert(pck);
    pck->pckData = (iAP2PacketData_t*) (pck + 1);
#else
    if (pckLen <= MAX_ALLOC_SIZE)
    {
        pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t) + pckLen);
        assert(pck);
        pck->pckData = (iAP2PacketData_t*) (pck + 1);
    }
    else
    {
        pck = (iAP2Packet_t*) malloc (sizeof(iAP2Packet_t));
        assert(pck);
        pck->pckData = (iAP2PacketData_t*) malloc (pckLen);
        assert(pck->pckData);
    }
#endif
    return pck;
}

/* Function to Return a recv packet to the buffPool */
void __iAP2BuffPoolReturnRecvPacket (iAP2BuffPool_t* buffPool, void* packet)
{
#if iAP2_ALLOW_LARGE_PACKET_MALLOC
//jiangpengfei note!
    free (packet);
#else
    iAP2Packet_t* pck = (iAP2Packet_t*) packet;
    if (pck->packetLen <= MAX_ALLOC_SIZE)
    {
        free (pck);
    }
    else
    {
        free (pck->pckData);
        free (pck);
    }
#endif
}


