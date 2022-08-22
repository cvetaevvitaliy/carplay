/*
 *	File: README.txt
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
 *	Copyright (C) 2013 Apple Inc. All Rights Reserved.
 *
 */

Introduction
============

The iAP2Link package is a reference implementation of the iAP2 link as documented in
the MFi Accessory Interface Specification.

The following source files are part of this package:

    README.txt
    iAP2LinkConfig_sample.h
    iAP2Link/
        iAP2Link.c
        iAP2Link.h
        iAP2LinkPrivate.h
        iAP2LinkAccessory.c
        iAP2LinkRunLoop.c
        iAP2LinkRunLoop.h
        iAP2Packet.c
        iAP2Packet.h
        iAP2FileTransfer.c
        iAP2FileTransfer.h
    iAP2Utility/
        iAP2BuffPool.c
        iAP2BuffPool.h
        iAP2BuffPoolImplementation.h
        iAP2Defines.h
        iAP2FSM.c
        iAP2FSM.h
        iAP2ListArray.c
        iAP2ListArray.h
        iAP2Log.h
        iAP2Misc.h
        iAP2Time.h
    iAP2UtilityImplementation/
        iAP2BuffPoolImplementation.c
        iAP2Log.c
        iAP2Time.c
        iAP2TimeImplementation.h


iAP2LinkConfig.h
----------------

This file must be provided by the accessory developer and
configures the iAP2 link. A typical configuration is provided
in iAP2LinkConfig_sample.h.


iAP2Link
--------

This folder contains the core iAP2 link implementation.

iAP2Packet.h, iAP2Link.h, and iAP2LinkRunLoop.h describe the
higher-level interfaces.
See the comments in those files for information on usage.

iAP2FileTransfer.c and iAP2FileTransfer.h provide an example
implementation of an iAP2 File Transfer session. See iAP2FileTransfer.h
for information on usage.

Reference code contained in this folder is designed to be used as
is and modifications are strongly discouraged.

The only exception is iAP2Runloop.h. Developers may need to provide
implementations for the following functions (depending on actual usage):

    iAP2LinkRunLoopInitImplementation
    iAP2LinkRunLoopCleanupImplementation
    iAP2LinkRunLoopProtectedCall
    iAP2LinkRunLoopWait
    iAP2LinkRunLoopSignal


iAP2Utility
-----------

This folder contains utility function implementations used by iAP2Link.
Some files implement abstractions for hardware dependencies such as
timers and memory management.


The .h files are designed to be used as is and modifications are
strongly discouraged. The .c files may be modified or used as is.


iAP2UtilityImplementation
-------------------------

This folder contains hardware-dependent implementation samples for
certain functions used by iAP2Utility.

The files in this folder must be modified to match the target hardware platform.

Specifically, iAP2TimeImplementation.h is not a complete timer
implementation and must be modified before use.

Usage Notes
===========

Other than the functions that need to be implemented, the following
functions defined in iAP2Packet.h, iAP2Link.h, and iAP2LinkRunLoop.h
are used to make use of the iAP2 link protocol reference implementation.

    iAP2PacketCreateEmptyRecvPacket
    iAP2PacketParseBuffer
    iAP2PacketIsComplete
    iAP2PacketCreateEmptySendPacket
    iAP2PacketGenerateBuffer
    iAP2PacketGetBuffer
    iAP2PacketDelete

    iAP2LinkRunLoopCreateAccessory
    iAP2LinkRunLoopRunOnce
    iAP2LinkRunLoopAttached
    iAP2LinkRunLoopDetached
    iAP2LinkRunLoopHandleReadyPacket
    iAP2LinkRunLoopQueueSendData

    iAP2LinkQueueSendData
    iAP2LinkQueueSendDataPacket


Device/Accessory Attach/Detach
------------------------------

On attach to Device, call:

    iAP2LinkRunLoopAttached


On detach from Device, call:

    iAP2LinkRunLoopDetached


Inbound iAP2 traffic
--------------------

The accessory should perform the following on incoming iAP2 traffic:

1) Create iAP2Packet by calling iAP2PacketCreateEmptyRecvPacket
2) Parse the incoming data by calling iAP2PacketParseBuffer
   (pass in data buffer and iAP2Packet structure)
3) Check if a full iAP2Packet has been parsed by calling iAP2PacketIsComplete
4) If a complete iAP2Packet has not been parsed, go to (2)
5) Pass the complete iAP2Packet to the link layer by calling
   iAP2LinkRunLoopHandleReadyPacket


Outbound iAP2 traffic
---------------------

The accessory should perform the following on outgoing iAP2 traffic:

1a) Create a data buffer containing the session data, or
1b) Create iAP2Packet by calling iAP2PacketCreateEmptySendPacket
    and fill the payload with session data.
2a) Call iAP2LinkRunLoopQueueSendData or iAP2LinkQueueSendData
    with a session data buffer, or
2b) Call iAP2LinkQueueSendDataPacket with iAP2Packet send packet
    with a payload containing session data.

Once the packet is sent out successfully and has been ACK'd,
iAP2PacketDelete will be called by iAP2Link.


Notes
-----

iAP2LinkRunLoopRunOnce should be called on every iteration of the
accessory firmware's main run loop. Most link layer operations are
handled by iAP2Link within iAP2LinkRunLoopRunOnce.

Processing of iAP2 session data should be performed as quickly
as possible; any lengthy handling should be spread over multiple run
loop cycles or performed in a separate processing thread. This
especially applies to the accessory authentication process;
calculation of the challenge response may take a while compared to
the link layer timeout parameters.