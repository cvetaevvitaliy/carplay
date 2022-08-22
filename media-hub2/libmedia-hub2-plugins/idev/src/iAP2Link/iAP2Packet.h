/*
 *	File: iAP2Packet.h
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

#ifndef iAP2Link_iAP2Packet_h
#define iAP2Link_iAP2Packet_h

#include <stdint.h>
#include <iAP2Defines.h>
#include <iAP2Time.h>


/*
******************************************************
**
** iAP2Packet
**
** USAGE:
**
** On inbound data from port:
** 1) Create an empty packet object by calling iAP2PacketCreateEmpty()
** 2) Call iAP2PacketParseBuffer() with the incoming data and the empty packet
**      (the packet object keeps the parsing state)
** 3) After calling iAP2PacketParseBuffer(), call iAP2PacketIsComplete() to see
**      if a complete packet has been parsed.
**      If a complete packet has been parsed, process the packet and go back
**          to 1) to create a new packet object for parsing the next packet.
**      If a complete packet has NOT been parsed yet, go back to 2) continuing to
**          use the same packet object.
**
** On outbound data to the port:
** 1) Call iAP2PacketGenerateBuffer(); this will generate a flat buffer containing
**      the encoded packet.
** 2) Use iAP2PacketGetBuffer(packet) and packet->packetLen process the generated
**      encoded packet buffer. (by sending out the port)
**
******************************************************
*/

/*
******************************************************
**
** Packet structure:
**
** msb                             lsb
**  +---+---+---+---+---+---+---+---+
**  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
**  +---+---+---+---+---+---+---+---+
**  |   Start of Packet MSB (0xFF)  | byte 0
**  +-------------------------------+
**  |   Start of Packet LSB (0x5A)  | byte 1
**  +-------------------------------+
**  |       Packet Length MSB       | byte 2
**  +-------------------------------+
**  |       Packet Length LSB       | byte 3
**  +---+---+---+---+---+---+---+---+
**  |SYN|ACK|EAK|RST|SUS| 0 | 0 | 0 | byte 4
**  +---+---+---+---+---+---+---+---+
**  |     Packet Sequence Number    | byte 5
**  +-------------------------------+
**  | Packet Acknowledgement Number | byte 6
**  +-------------------------------+
**  |           Session ID          | byte 7
**  +-------------------------------+
**  |        Header Checksum        | byte 8
**  +-------------------------------+
**  |               .               |
**  |               .               |
**  |          Payload Data         | byte 9 to N - 1
**  |               .               |
**  |               .               |
**  +-------------------------------+
**  |        Payload Checksum       | byte N
**  +-------------------------------+
**
******************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif
    
enum IAP2PacketEnums
{
    /* Start of packet bytes */
    kIAP2PacketSYNC        = 0xFF,
    kIAP2PacketSOP         = 0x5A,
    kIAP2PacketSOPOrig     = 0x55,

    kIAP2PacketSOPLen      = 2,

    kiAP2PacketVersion     = 1,

    /* Packet field index */
    kIAP2PacketIndexSYNC   = 0,   /* Start of packet byte 1 */
    kIAP2PacketIndexSOP    = 1,   /* Start of packet byte 2 */
    kIAP2PacketIndexLEN1   = 2,   /* Packet length MSB */
    kIAP2PacketIndexLEN2   = 3,   /* Packet length LSB */
    kIAP2PacketIndexCTRL   = 4,   /* Control flags */
    kIAP2PacketIndexSEQ    = 5,   /* SEQ number */
    kIAP2PacketIndexACK    = 6,   /* ACK number */
    kIAP2PacketIndexSESSID = 7,   /* Session ID */
    kIAP2PacketIndexCHK    = 8,   /* Header checksum */

    /* iAP 1.0/2.0 packet detect: FF 55 02 00 EE 10 */
    kIAP2PacketDetectLEN    = 0x0200,
    kIAP2PacketDetectCTRL   = 0xEE,
    kIAP2PacketDetectSEQ    = 0x10,

    /* iAP 1.0/2.0 packet detect BAD ACK: FF 55 04 00 02 04 EE 08 */
    kIAP2PacketDetectNACKLEN    = 0x0400,
    kIAP2PacketDetectNACKCTRL   = 0x02,
    kIAP2PacketDetectNACKSEQ    = 0x04,
    kIAP2PacketDetectNACKACK    = 0xEE,
    kIAP2PacketDetectNACKSESSID = 0x08,

    /*
    ** Packet Header Len
    ** SOP(2) + LEN(2) + CONTROL(1) + SEQ(1) + ACK(1) + SESSID(1) + CHK(1)
    */
    kIAP2PacketHeaderLen = 9,
    kIAP2PacketChksumLen = 1,   /* checksum byte length */

    kiAP2PacketLenMax = 0xFFFF,
    kiAP2PacketMaxPayloadSize = (kiAP2PacketLenMax - kIAP2PacketHeaderLen - kIAP2PacketChksumLen),

    /* BitMask defines for bits in control byte */
    kIAP2PacketControlMaskSYN = 0x80,   /* synchronization */
    kIAP2PacketControlMaskACK = 0x40,   /* acknowledgement */
    kIAP2PacketControlMaskEAK = 0x20,   /* extended acknowledgement */
    kIAP2PacketControlMaskRST = 0x10,   /* reset */
    kIAP2PacketControlMaskSUS = 0x08,   /* suspend (sleep) */

    kIAP2PacketSynDataIdxVersion           = 0,
    kIAP2PacketSynDataIdxMaxOutstanding    = 1,
    kIAP2PacketSynDataIdxMaxPacketSize     = 2,
    kIAP2PacketSynDataIdxRetransmitTimeout = 4,
    kIAP2PacketSynDataIdxCumAckTimeout     = 6,
    kIAP2PacketSynDataIdxMaxRetransmit     = 8,
    kIAP2PacketSynDataIdxMaxCumACK         = 9,

    kIAP2PacketSynDataBaseLen              = 10,

    kIAP2PacketSynDataIdxSessionInfo       = 10,

    kIAP2PacketSynSessionIdxID             = 0,
    kIAP2PacketSynSessionIdxType           = 1,
    kIAP2PacketSynSessionIdxVersion        = 2,
    kIAP2PacketSynSessionSize              = 3,

    /* Session ID 0x0 shall not be used for any session types */
    kIAP2PacketReservedSessionID           = 0,

    kIAP2PacketMaxSessions                 = 10,

    kIAP2PacketSynOptionMaskLP             = 0x80,
    kIAP2PacketSynOptionMaskHP             = 0x40
};

typedef enum
{
    kiAP2PacketParseStateSOP1 = 0,
    kiAP2PacketParseStateSOP2,
    kiAP2PacketParseStateLEN1,
    kiAP2PacketParseStateLEN2,
    kiAP2PacketParseStateCTRL,
    kiAP2PacketParseStateSEQ,
    kiAP2PacketParseStateACK,
    kiAP2PacketParseStateSESSID,
    kiAP2PacketParseStateCHK,
    kiAP2PacketParseStatePAYLOAD,
    kiAP2PacketParseStatePAYLOADCHK,
    kiAP2PacketParseStateFINISH,
    kiAP2PacketParseStateDETECT,
    kiAP2PacketParseStateDETECTBAD,

    kiAP2PacketParseStateLAST = kiAP2PacketParseStateDETECTBAD

} kiAP2PacketParseState_t;

/* Session Service Type */
typedef enum
{
    kIAP2PacketServiceTypeControl  = 0, /* Control & Authentication */
    kIAP2PacketServiceTypeBuffer   = 1, /* Buffer (ie. Artwork, Workout) */
    kIAP2PacketServiceTypeEA       = 2, /* EA stream */

    kIAP2PacketServiceTypeCount

} iAP2PacketServiceType_t;


#define IAP2_HI_BYTE(X) (((X) >> 8) & 0xFF)
#define IAP2_LO_BYTE(X) ((X) & 0xFF)
    
    
#pragma pack(push,1)

typedef struct {
    uint8_t sop1;   /* 0xFF */
    uint8_t sop2;   /* 0x5A */
    uint8_t len1;
    uint8_t len2;
    uint8_t ctl;
    uint8_t seq;
    uint8_t ack;
    uint8_t sess;
    uint8_t chk;

} iAP2PacketHeader_t;

typedef struct {
    uint8_t sop1;   /* 0xFF */
    uint8_t sop2;   /* 0x5A */
    uint8_t len1;
    uint8_t len2;
    uint8_t ctl;
    uint8_t seq;
    uint8_t ack;
    uint8_t sess;
    uint8_t chk;

    uint8_t data [1];

} iAP2PacketData_t;

typedef struct {
    uint8_t sop1;   /* 0xFF */
    uint8_t sop2;   /* 0x5A */
    uint8_t len1;
    uint8_t len2;
    uint8_t ctl;
    uint8_t seq;
    uint8_t ack;
    uint8_t sess;
    uint8_t chk;

    uint8_t data [kiAP2PacketMaxPayloadSize];
    uint8_t datachk;

} iAP2PacketMax_t;

typedef struct
{
    uint8_t    id;
    uint8_t    type;        /* iAP2PacketServiceType_t */
    uint8_t    version;
    
} iAP2PacketSessionInfo_t;

typedef struct
{
    uint8_t    version;
    uint8_t    maxOutstandingPackets;
    uint8_t    maxRetransmissions;
    uint8_t    maxCumAck;
    uint16_t   maxPacketSize;
    uint16_t   retransmitTimeout;
    uint16_t   cumAckTimeout;
    uint8_t    numSessionInfo;
    uint8_t    peerMaxOutstandingPackets;   /* Not used for iAP2LinkCreate */
    uint16_t   peerMaxPacketSize;           /* Not used for iAP2LinkCreate */
    iAP2PacketSessionInfo_t sessionInfo[kIAP2PacketMaxSessions];

} iAP2PacketSYNData_t;
#pragma pack(pop)


/* To create and delete packets, must use functions provided here. */
typedef struct iAP2Packet_st
{
    void*       link;       /* link associated with this packet */

    void*       cbContext;
    void*       callbackOnSend; /* iAP2LinkDataSentCB_t */

    uint32_t    dataCurLen;
    uint32_t    bufferLen;

    uint32_t    timeStamp;  /* timestamp (ms) when packet was sent or received(and parse). */

    uint16_t    packetLen;

    uint8_t     dataChecksum;
    uint8_t     seqPlus;    /* Used to determine ACK location in sent queue */

    uint8_t     retransmitCount;
    uint8_t     timer;

    kiAP2PacketParseState_t state:8;

    BOOL        recvEAK:1;
    BOOL        sendPacket:1;

    /* reserve 4 bits for debug use */
    uint8_t     debugData:4;

    iAP2PacketData_t* pckData;

} iAP2Packet_t;


/*
****************************************************************
**
**  iAP2PacketParseBuffer
**
**  Input:
**      buffer:         Input buffer containing packet data to parse.
**      bufferLen:      Size of the Input buffer data.
**      maxPacketSize:  MaxPacketSize supported... any packets claiming to be
**                      larger will be ignored (value of 0 means don't care).
**
**  Output:
**      packet:     Provided packet structure is updated with parsed data.
**      bDetect:    Set to TRUE if iAP 1.0/2.0 detect or detect BAD ACK packet
**                  was received, else FALSE
**      failedChecksums:    if provided, value will be incremented when
**                          checksum failure is detected.
**      sopDetect:  if provided, value will be incremented whenvere SOP is detected.
**
**  Return:
**      uint32_t    Number of bytes parsed from the buffer.
**
****************************************************************
*/
uint32_t iAP2PacketParseBuffer (const uint8_t*  buffer,
                                uint32_t        bufferLen,
                                iAP2Packet_t*   packet,
                                uint32_t        maxPacketSize,
                                BOOL*           bDetect,
                                uint32_t*       failedChecksums,
                                uint32_t*       sopDetect);


/*
****************************************************************
**
**  iAP2PacketIsComplete
**
**  Input:
**      pck:    packet to find out if it is complete
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is complete and ready to be processed, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsComplete (const iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketIsACKOnly
**
**  Input:
**      pck:    packet to find out if it is ACK only packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is ACK only, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsACKOnly (const iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketIsDataPacket
**
**  Input:
**      pck:    packet to find out if it is a data packet (ACK plus data)
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is data packet, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsDataPacket (const iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketIsEAK
**
**  Input:
**      pck:    packet to find out if it is EAK packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packet is EAK, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketIsEAK (const iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketEqual
**
**  Input:
**      packet1:    first packet to compare
**      packet2:    second packet to compare
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if packets compare same, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketEqual (const iAP2Packet_t* packet1, const iAP2Packet_t* packet2);


/*
****************************************************************
**
**  iAP2PacketGetBuffSize
**
**  Input:
**      payloadSize:    size of payload data
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of buff required for proper operation
**
****************************************************************
*/
uint32_t iAP2PacketGetBuffSize (uint16_t payloadSize);


/*
****************************************************************
**
**  iAP2PacketCreateEmptyPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
**  Note: OBSOLETE: equivalent to iAP2PacketCreateEmptyRecvPacket
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptyPacket (void* link);


/*
****************************************************************
**
**  iAP2PacketCreateEmptySendPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptySendPacket (void* link);


/*
****************************************************************
**
**  iAP2PacketCreateEmptyRecvPacket
**
**  Input:
**      link:  link to associate with this packet
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEmptyRecvPacket (void* link);


/*
****************************************************************
**
**  iAP2PacketCreate
**
**  Input:
**      link:       link to associate with this packet
**      control:    Control bitmask (SYN, ACK, EAK, RST)
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      payload:    Input buffer containing packet payload data.
**      payloadLen: Size of the Input payload data.
**      session:    session for this payload
**      sendPacket: TRUE if this is for send packet, else FALSE.
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreate (void*           link,
                                uint8_t         control,
                                uint8_t         seq,
                                uint8_t         ack,
                                const uint8_t*  payload,
                                uint32_t        payloadLen,
                                uint8_t         session,
                                BOOL            sendPacket);


/*
****************************************************************
**
**  iAP2PacketDelete
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDelete (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketResetPacket
**
**  Input:
**      pck:                packet to reset
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note: Reset packet should only be used for the same purpose (send or receive)
**          because the allocation size is direction specific.
**
****************************************************************
*/
void iAP2PacketResetPacket (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketGetSpaceLeft
**
**  Input:
**      packet:         Packet to find remaining space for.
**      maxPacketLen:   Max packet size.
**
**  Output:
**      None
**
**  Return:
**      number of bytes remaining for packet payload based on maxPacketLen.
**
****************************************************************
*/
uint16_t iAP2PacketGetSpaceLeft (iAP2Packet_t*  packet,
                                 uint16_t       maxPacketLen);


/*
****************************************************************
**
**  iAP2PacketCreateACKPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      payload:    Input buffer containing packet payload data.
**      payloadLen: Size of the Input payload data.
**      session:    session for this payload
**
**  Output:
**      seq:        Sequence number is incremented based on data.
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateACKPacket (void*          link,
                                         uint8_t        seq,
                                         uint8_t        ack,
                                         const uint8_t* payload,
                                         uint32_t       payloadLen,
                                         uint8_t        session);


/*
****************************************************************
**
**  iAP2PacketCreateSYNPacket
**
**  Input:
**      link:           link to associate with this packet
**      initSeq:        Initial Sequence number.
**      ack:            Buffer containing Acknowledgement number to be used.
**                          If not to be used, pass in NULL.
**      version:        Link version.
**      maxOutstanding  Max number of outstanding packets (tx window).
**      maxPacketSize   Max Packet size.
**      reTxTimeout     Re-transmit timeout (if ACK not received).
**      cumAckTimeout   Timeout before sending an ACK if no outbound packet.
**      maxRetransmit   Max number of times packet is re-transmitted before error.
**      maxCumAck       Max number of packets received without ACK being sent.
**      numSessionInfo  number of sessions
**      sessionInfo     pointer to buffer containing list of sessions
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateSYNPacket (void*          link,
                                         uint8_t        initSeq,
                                         const uint8_t* ack,
                                         uint8_t        version,
                                         uint8_t        maxOutstanding,
                                         uint16_t       maxPacketSize,
                                         uint16_t       reTxTimeout,
                                         uint16_t       cumAckTimeout,
                                         uint8_t        maxRetransmit,
                                         uint8_t        maxCumAck,
                                         uint8_t        numSessionInfo,
                                         iAP2PacketSessionInfo_t* sessionInfo);


/*
****************************************************************
**
**  iAP2PacketCreateEAKPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**      ack:        Acknowledgement number to be used.
**      outOfSeq:       Buffer containing list of out-of-seq seq#'s
**      outOfSeqCount:  Number of out-of-seq seq#'s
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateEAKPacket (void*          link,
                                         uint8_t        seq,
                                         uint8_t        ack,
                                         const uint8_t* outOfSeq,
                                         uint32_t       outOfSeqCount);


/*
****************************************************************
**
**  iAP2PacketCreateRSTPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateRSTPacket (void*   link,
                                         uint8_t seq);


/*
****************************************************************
**
**  iAP2PacketCreateSUSPacket
**
**  Input:
**      link:       link to associate with this packet
**      seq:        Sequence number to be used.
**
**  Output:
**      None
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
iAP2Packet_t* iAP2PacketCreateSUSPacket (void*   link,
                                         uint8_t seq);


/*
****************************************************************
**
**  iAP2PacketGenerateBuffer
**
**  Input:
**      packet:     packet to encode into bytes.
**
**  Output:
**      None
**
**  Return:
**      uint8_t*    buffer that contains the encoded bytes of the
**                  packet for transmission
**
**  Note: Prepares packet buffer for sending out the transport.
**
****************************************************************
*/
uint8_t* iAP2PacketGenerateBuffer (iAP2Packet_t* packet);


/*
 ****************************************************************
 **
 **  iAP2PacketGetBuffer
 **
 **  Input:
 **      packet:     packet to get the buffer pointer for.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint8_t*    pointer to buffer that contains the encoded bytes of the
 **                  packet for transmission
 **
 ****************************************************************
 */
uint8_t* iAP2PacketGetBuffer (iAP2Packet_t* packet);


/*
 ****************************************************************
 **
 **  iAP2PacketGetPayload
 **
 **  Input:
 **      packet:     packet to encode into bytes.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint8_t*    pointer to payload data in the buffer that contains the
 **                  encoded bytes of the packet for transmission
 **
 ****************************************************************
 */
uint8_t* iAP2PacketGetPayload (iAP2Packet_t* packet);


/*
 ****************************************************************
 **
 **  iAP2PacketGetPayloadLen
 **
 **  Input:
 **      packet:     packet to encode into bytes.
 **
 **  Output:
 **      None
 **
 **  Return:
 **      uint16_t    payload length of the packet
 **
 ****************************************************************
 */
uint16_t iAP2PacketGetPayloadLen (const iAP2Packet_t* packet);


/*
****************************************************************
**
**  iAP2PacketParseSYNData
**
**  Input:
**      const uint8_t*  data
**      uint32_t        dataLen
**
**  Output:
**      iAP2PacketSYNData_t* synData
**
**  Return:
**      iAP2Packet_t*   pointer to packet struct
**
****************************************************************
*/
void iAP2PacketParseSYNData (const uint8_t*         data,
                             uint32_t               dataLen,
                             iAP2PacketSYNData_t*   synData);


/*
****************************************************************
**
**  iAP2PacketGetMissingSeqFromEAK
**
**  Input:
**      packet:     EAK packet to retreive the missing seq#s for.
**      window:     Max Outstanding Packets as negotiated
**
**  Output:
**      missingLen: pointer to len buffer... contains number of
**                  missing seq#'s returned.
**
**  Return:
**      uint8_t*    array of missing seq#'s (returne NULL and 0 len if none)
**
****************************************************************
*/
uint8_t* iAP2PacketGetMissingSeqFromEAK (iAP2Packet_t* eakPacket,
                                         uint8_t       window,
                                         uint32_t*     missingLen);


/*
****************************************************************
**
**  iAP2PacketCalcChecksum
**
**  Input:
**      buffer:     Input buffer containing data to Checksum.
**      bufferLen:  Size of the Input buffer data.
**
**  Output:
**      None
**
**  Return:
**      uint8_t     Calculated checksum for provide buffer
**
****************************************************************
*/
uint8_t iAP2PacketCalcChecksum (const uint8_t*  buffer,
                                uint32_t        bufferLen);


/*
****************************************************************
**
**  iAP2PacketCopyAndCalcChecksum
**
**  Input:
**      dst:    buffer to copy to.
**      src;    source buffer to copy from.
**      len:    Size of the Input buffer data.
**
**  Output:
**      None
**
**  Return:
**      uint8_t     Calculated checksum
**
**  Note: Copies data and calculates the checksum at the same time.
**
****************************************************************
*/
uint8_t iAP2PacketCopyAndCalcChecksum (uint8_t *        dst,
                                       const uint8_t *  src,
                                       uint32_t         len);


/*
****************************************************************
**
**  iAP2PacketCalcHeaderChecksum
**
**  Input:
**      pck:    packet to calculate header checksum on.
**
**  Output:
**      pck:    packet header checksum field is filled in with calculated value.
**
**  Return:
**      uint8_t     Calculated header checksum.
**
****************************************************************
*/
uint8_t iAP2PacketCalcHeaderChecksum (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketCalcPayloadChecksum
**
**  Input:
**      pck:    packet to calculate payload checksum on.
**
**  Output:
**      pck:    packet payload checksum field is filled in with calculated value.
**
**  Return:
**      uint8_t     Calculated payload checksum.
**
****************************************************************
*/
uint8_t iAP2PacketCalcPayloadChecksum (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketNextSeq
**
**  Input:
**      curSeq: current sequence number
**
**  Output:
**      None
**
**  Return:
**      uint8_t next seqeunce number
**
****************************************************************
*/
uint8_t iAP2PacketNextSeq (uint8_t curSeq);


/*
****************************************************************
**
**  iAP2PacketCalcSeqGap
**
**  Input:
**      curSeq: current sequence number
**      next:   next sequence number
**
**  Output:
**      None
**
**  Return:
**      uint8_t returns the gap between the current and next seqeunce numbers
**
****************************************************************
*/
uint8_t iAP2PacketCalcSeqGap (uint8_t curSeq, uint8_t nextSeq);


/*
****************************************************************
**
**  iAP2PacketIsSeqACKd
**
**  Input:
**      seq:    sequence number being checked
**      ack:    sequence number last ACKd
**      window: size of window where packets can be sent without ACK
**
**  Output:
**      None
**
**  Return:
**      BOOL    TRUE if seq# has been ACKd, else FALSE
**
**  Note:   It is expected that seq number is always ahead of ack so
**          if the gap is larger than window, it must have been ACKd
**          already.
**
****************************************************************
*/
BOOL iAP2PacketIsSeqACKd (uint8_t seq, uint8_t ack, uint8_t window);


/*
****************************************************************
**
**  iAP2PacketRequireACK
**
**  Input:
**      packet: pointer to packet
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if packet needs an ACK, else FALSE
**
****************************************************************
*/
BOOL iAP2PacketRequireACK (iAP2Packet_t* packet);


/*
****************************************************************
**
**  iAP2PacketCheckDetect
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if iAP 1.0/2.0 Detect packet detected,
**              else returne FALSE
**
****************************************************************
*/
BOOL iAP2PacketCheckDetect (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketCheckDetectNACK
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if iAP 1.0/2.0 Detect BAD ACK packet detected,
**              else returne FALSE
**
****************************************************************
*/
BOOL iAP2PacketCheckDetectNACK (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacket
**
**  Input:
**      pck:    packet to to print out info for.
**      file:   file called from
**      line:   line number called from
**      tag:    additional info to display with packet info
**      indent: additional text to add at the beginning of the line
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacket (iAP2Packet_t*  packet,
                                 const char*    file,
                                 int            line,
                                 const char*    tag,
                                 const char*    indent);


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketNL
**
**  Input:
**      pck:    packet to to print out info for.
**      file:   file called from
**      line:   line number called from
**      tag:    additional info to display with packet info
**      indent: additional text to add at the beginning of the line
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacketNL (iAP2Packet_t*  packet,
                                   const char*    file,
                                   int            line,
                                   const char*    tag,
                                   const char*    indent);


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketShortNL
**
**  Input:
**      packet:     packet to to print out info for.
**      buff:       buffer to write string into
**      buffLen:    size of buff
**
**  Output:
**      buff:   debug output string is written into buff
**
**  Return:
**      char*   pointer to buff
**
****************************************************************
*/
char* iAP2PacketDebugStringPacketShortNL (iAP2Packet_t* packet,
                                          char*         buff,
                                          uint32_t      buffLen);


/*
****************************************************************
**
**  iAP2PacketDebugPrintPacketShortNL
**
**  Input:
**      pck:    packet to to print out info for.
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketDebugPrintPacketShortNL (iAP2Packet_t*  packet);


/*
****************************************************************
**
**  iAP2PacketAssignTimer
**
**  Input:
**      pck:    packet to delete
**      timer:  timer to assign to packet
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketAssignTimer (iAP2Packet_t* pck, uint16_t timer);


/*
****************************************************************
**
**  iAP2PacketRemoveTimer
**
**  Input:
**      pck:        packet to delete
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketRemoveTimer (iAP2Packet_t* pck);


/*
****************************************************************
**
**  iAP2PacketName
**
**  Input:
**      None
**
**  Output:
**      None
**
**  Return:
**      const char* pointer to string containing Name of packet
**          eg. SYN, ACK, DATA, etc.
**
****************************************************************
*/
const char* iAP2PacketName (iAP2Packet_t* packet);


/*
****************************************************************
**
**  iAP2PacketGetTimeoutID
**
**  Input:
**      pck:        packet to get timeoutID from
**
**  Output:
**      None
**
**  Return:
**      uint16_t    timeoutID associated with the packet
**
****************************************************************
*/
uint16_t iAP2PacketGetTimeoutID (iAP2Packet_t* packet);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Link_iAP2Packet_h */
