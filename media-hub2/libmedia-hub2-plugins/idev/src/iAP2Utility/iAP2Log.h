/*
 *	File: iAP2Log.h
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

#ifndef iAP2Utility_iAP2Log_h
#define iAP2Utility_iAP2Log_h

#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    kiAP2LogTypeError = 0,
    kiAP2LogTypeLog,
    kiAP2LogTypeLogDbg,
    kiAP2LogTypeData,

    kiAP2LogTypeCount

} iAP2LogType_t;


/*
******************************************************************
**
**  iAP2LogDataChar
**
**  Input:
**      byte:   byte to return a printable character for
**
**  Output:
**      None
**
**  Return:
**      char    printable character for byte
**
****************************************************************
*/
char iAP2LogDataChar (char byte);


/*
******************************************************************
**
**  iAP2LogEnable
**
**  Input:
**      type:   type of logging to enable
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LogEnable (iAP2LogType_t type);


/*
******************************************************************
**
**  iAP2LogDisable
**
**  Input:
**      type:   type of logging to enable
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2LogDisable (iAP2LogType_t type);


/*
******************************************************************
**
**  iAP2LogType
**
**  Input:
**      type:   type of logging
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Uses iAP2LockAcquire/Release to make sure multi-line
**          logs don't get interrupted.
**
****************************************************************
*/
void iAP2LogType (iAP2LogType_t type, const char* format, ...);


/*
******************************************************************
**
**  iAP2LogTypeNL
**
**  Input:
**      type:   type of logging
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogTypeNL (iAP2LogType_t type, const char* format, ...);


/*
******************************************************************
**
**  iAP2LogTypePureNL
**
**  Input:
**      type:   type of logging
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogTypePureNL (iAP2LogType_t type, const char* format, ...);


/*
******************************************************************
**
**  iAP2LogTypeVNL
**
**  Input:
**      type:   type of logging
**      format: format string
**      args:   format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogTypeVNL (iAP2LogType_t type, const char* format, va_list args);


/*
******************************************************************
**
**  iAP2LogTypePureVNL
**
**  Input:
**      type:   type of logging
**      format: format string
**      args:   format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogTypePureVNL (iAP2LogType_t type, const char* format, va_list args);


/*
******************************************************************
**
**  iAP2Log
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Uses iAP2LockAcquire/Release to make sure multi-line
**          logs don't get interrupted.
**
****************************************************************
*/
void iAP2Log (const char* format, ...);


/*
******************************************************************
**
**  iAP2LogNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogNL (const char* format, ...);


/*
******************************************************************
**
**  iAP2LogPureNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPureNL (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogVNL
**
**  Input:
**      format: format string
**      args:   format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogVNL (const char* format, va_list args);


/*
****************************************************************
**
**  iAP2LogDbg
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Uses iAP2LockAcquire/Release to make sure multi-line
**          logs don't get interrupted.
**
****************************************************************
*/
void iAP2LogDbg (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogDbgNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogDbgNL (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogDbgPureNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogDbgPureNL (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogDbgVNL
**
**  Input:
**      format: format string
**      args:   format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogDbgVNL (const char* format, va_list args);


/*
****************************************************************
**
**  iAP2LogError
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Uses iAP2LockAcquire/Release to make sure multi-line
**          logs don't get interrupted.
**
****************************************************************
*/
void iAP2LogError (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogErrorNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogErrorNL (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogErrorPureNL
**
**  Input:
**      format: format string
**      ...:    parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogErrorPureNL (const char* format, ...);


/*
****************************************************************
**
**  iAP2LogErrorVNL
**
**  Input:
**      format: format string
**      args:   format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogErrorVNL (const char* format, va_list args);


/*
****************************************************************
**
**  iAP2LogStart
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
**  Note:   Acquires lock for uninterrupted logging... use
**          iAP2LogDbgNL or iAP2LogErrorNL to do logging and
**          don't forget to release the lock afterwards by
**          calling iAP2LogStop.
**
****************************************************************
*/
void iAP2LogStart (void);


/*
****************************************************************
**
**  iAP2LogStop
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
**  Note:   Acquires lock for uninterrupted logging... use
**          iAP2LogDbgNL or iAP2LogErrorNL to do logging and
**          don't forget to release the lock afterwards by
**          calling iAP2LogStop.
**
****************************************************************
*/
void iAP2LogStop (void);


/*
****************************************************************
**
**  iAP2LogPrintData
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      tag:        string to add at the beginning of log line
**      format:     extra comment format string
**      ...:        extra comment parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   Uses iAP2LockAcquire/Release to make sure multi-line
**          logs don't get interrupted.
**
****************************************************************
*/
void iAP2LogPrintData (const unsigned char* data,
                       unsigned long        dataLen,
                       const char*          tag,
                       const char*          format,
                       ...);


/*
****************************************************************
**
**  iAP2LogPrintDataNL
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      tag:        string to add at the beginning of log line
**      format:     extra comment format string
**      ...:        extra comment parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogPrintDataNL (const unsigned char* data,
                         unsigned long        dataLen,
                         const char*          tag,
                         const char*          format,
                         ...);


/*
****************************************************************
**
**  iAP2LogPrintDataPureNL
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      tag:        string to add at the beginning of log line
**      format:     extra comment format string
**      ...:        extra comment parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPrintDataPureNL (const unsigned char* data,
                             unsigned long        dataLen,
                             const char*          tag,
                             const char*          format,
                             ...);


/*
****************************************************************
**
**  iAP2LogPrintDataVNL
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      tag:        string to add at the beginning of log line
**      format:     extra comment format string
**      args:       format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**
****************************************************************
*/
void iAP2LogPrintDataVNL (const unsigned char* data,
                          unsigned long        dataLen,
                          const char*          tag,
                          const char*          format,
                          va_list              args);


/*
****************************************************************
**
**  iAP2LogPrintDataPureVNL
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      tag:        string to add at the beginning of log line
**      format:     extra comment format string
**      args:       format argument list
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop
**          to acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPrintDataPureVNL (const unsigned char* data,
                              unsigned long        dataLen,
                              const char*          tag,
                              const char*          format,
                              va_list              args);


#ifdef __cplusplus
}
#endif

#endif
