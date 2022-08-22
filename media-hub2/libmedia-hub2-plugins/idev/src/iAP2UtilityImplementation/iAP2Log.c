/*
 *	File: iAP2Log.c
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
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#include <iAP2LinkConfig.h>

#if iAP2_FOR_DEVICE
#include <syslog.h>
#endif

#include "iAP2Log.h"
#include "iAP2Defines.h"

#define LOG_STRING_SIZE 500

static pthread_mutex_t g_LogLock = PTHREAD_MUTEX_INITIALIZER;

static BOOL g_Enable [kiAP2LogTypeCount] =
{
    FALSE,   /* kiAP2LogTypeError */
    FALSE,  /* kiAP2LogTypeLog */
    FALSE,  /* kiAP2LogTypeLogDbg */
    FALSE   /* kiAP2LogTypeData */
};

static const char* g_LogTypeName [kiAP2LogTypeCount] =
{
    "ERROR",
    "LOG",
    "DEBUG",
    "DATA"
};

static void _PrintTime (char* outString, int outStringLen, iAP2LogType_t type, const char* prefix, const char* suffix)
{
    time_t     now;
    struct tm *ts;
    char       buf[80];

    /* Get the current time */
    now = time(NULL);

    /* Format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz" */
    ts = localtime(&now);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
    snprintf (outString, outStringLen, "%s%s: %s%s", prefix, buf, g_LogTypeName[type], suffix);
}


/*
****************************************************************
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
char iAP2LogDataChar (char byte)
{
    if (byte >= 0x20)
    {
        return byte;
    }
    return '.';
}


/*
****************************************************************
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
void iAP2LogEnable (iAP2LogType_t type)
{
    if (type < kiAP2LogTypeCount)
    {
        g_Enable [type] = TRUE;
    }
}


/*
****************************************************************
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
void iAP2LogDisable (iAP2LogType_t type)
{
    if (type < kiAP2LogTypeCount)
    {
        g_Enable [type] = FALSE;
    }
}


/*
****************************************************************
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
void iAP2LogType (iAP2LogType_t type, const char* format, ...)
{
    if (g_Enable[type])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypeVNL (type, format, args);
        va_end (args);
    }
}


/*
****************************************************************
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
void iAP2LogTypeNL (iAP2LogType_t type, const char* format, ...)
{
    if (g_Enable[type])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypeVNL (type, format, args);
        va_end (args);
    }
}


/*
****************************************************************
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
void iAP2LogTypePureNL (iAP2LogType_t type, const char* format, ...)
{
    if (g_Enable[type])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypePureVNL (type, format, args);
        va_end (args);
    }
}


/*
****************************************************************
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
void iAP2LogTypeVNL (iAP2LogType_t type, const char* format, va_list args)
{
    if (g_Enable[type])
    {
        char*           tmpStr = malloc (LOG_STRING_SIZE);
        unsigned long   index = 0;
        _PrintTime (tmpStr, LOG_STRING_SIZE, type, "", ": ");
        index = strlen(tmpStr);
        vsnprintf (&tmpStr[index], LOG_STRING_SIZE-index, format, args);
#if iAP2_FOR_DEVICE
        syslog ((kiAP2LogTypeError == type ? LOG_WARNING : LOG_NOTICE), "%s", tmpStr);
#else
        fprintf (stderr, "%s", tmpStr);
#endif
        free (tmpStr);
    }
}


/*
****************************************************************
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
void iAP2LogTypePureVNL (iAP2LogType_t type, const char* format, va_list args)
{
    if (g_Enable[type])
    {
        char* tmpStr = malloc (LOG_STRING_SIZE);
        vsnprintf (tmpStr, LOG_STRING_SIZE, format, args);
#if iAP2_FOR_DEVICE
        syslog ((kiAP2LogTypeError == type ? LOG_WARNING : LOG_NOTICE), "%s", tmpStr);
#else
        fprintf (stderr, "%s", tmpStr);
#endif
        free (tmpStr);
    }
}


/*
****************************************************************
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
void iAP2Log (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLog])
    {
        va_list args;
        va_start (args, format);
        iAP2LogVNL (format, args);
        va_end (args);
    }
}


/*
****************************************************************
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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**
****************************************************************
*/
void iAP2LogNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLog])
    {
        va_list args;
        va_start (args, format);
        iAP2LogVNL (format, args);
        va_end (args);
    }
}


/*
****************************************************************
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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPureNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLog])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypePureVNL (kiAP2LogTypeLog, format, args);
        va_end (args);
    }
}


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
void iAP2LogVNL (const char* format, va_list args)
{
    if (g_Enable[kiAP2LogTypeLog])
    {
        iAP2LogTypeVNL (kiAP2LogTypeLog, format, args);
    }
}


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
void iAP2LogDbg (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLogDbg])
    {
        va_list args;
        va_start (args, format);
        iAP2LogDbgVNL (format, args);
        va_end (args);
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**
****************************************************************
*/
void iAP2LogDbgNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLogDbg])
    {
        va_list args;
        va_start (args, format);
        iAP2LogDbgVNL (format, args);
        va_end (args);
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogDbgPureNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeLogDbg])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypePureVNL (kiAP2LogTypeLogDbg, format, args);
        va_end (args);
    }
}


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
void iAP2LogDbgVNL (const char* format, va_list args)
{
    if (g_Enable[kiAP2LogTypeLogDbg])
    {
        iAP2LogTypeVNL (kiAP2LogTypeLogDbg, format, args);
    }
}


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
void iAP2LogError (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeError])
    {
        va_list args;
        va_start (args, format);
        iAP2LogErrorVNL (format, args);
        va_end (args);
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**
****************************************************************
*/
void iAP2LogErrorNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeError])
    {
        va_list args;
        va_start (args, format);
        iAP2LogErrorVNL (format, args);
        va_end (args);
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogErrorPureNL (const char* format, ...)
{
    if (g_Enable[kiAP2LogTypeError])
    {
        va_list args;
        va_start (args, format);
        iAP2LogTypePureVNL (kiAP2LogTypeError, format, args);
        va_end (args);
    }
}


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
void iAP2LogErrorVNL (const char* format, va_list args)
{
    if (g_Enable[kiAP2LogTypeError])
    {
        iAP2LogTypeVNL (kiAP2LogTypeError, format, args);
    }
}


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
void iAP2LogStart (void)
{
    pthread_mutex_lock(&g_LogLock);
}


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
void iAP2LogStop (void)
{
    pthread_mutex_unlock(&g_LogLock);
}


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
                       ...)
{
    if (g_Enable[kiAP2LogTypeData])
    {
        if (data && dataLen > 0)
        {
#if DEBUG
            va_list args;
            va_start (args, format);
            iAP2LogPrintDataVNL(data, dataLen, tag, format, args);
            va_end (args);
#endif
        }
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**
****************************************************************
*/
void iAP2LogPrintDataNL (const unsigned char* data,
                         unsigned long        dataLen,
                         const char*          tag,
                         const char*          format,
                         ...)
{
    if (g_Enable[kiAP2LogTypeData])
    {
        if (data && dataLen > 0)
        {
#if DEBUG
            va_list args;
            va_start (args, format);
            iAP2LogPrintDataVNL(data, dataLen, tag, format, args);
            va_end (args);
#endif
        }
    }
}


/*
****************************************************************
**
**  iAP2LogPrintDataPureNL
**
**  Input:
**      data:       buffer pointer to print data for
**      dataLen:    length of buffer
**      format:     extra comment format string
**      ...:        extra comment parameters
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPrintDataPureNL (const unsigned char* data,
                             unsigned long        dataLen,
                             const char*          tag,
                             const char*          format,
                             ...)
{
    if (g_Enable[kiAP2LogTypeData])
    {
        if (data && dataLen > 0)
        {
#if DEBUG
            va_list args;
            va_start (args, format);
            iAP2LogPrintDataPureVNL(data, dataLen, tag, format, args);
            va_end (args);
#endif
        }
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**
****************************************************************
*/
void iAP2LogPrintDataVNL (const unsigned char* data,
                          unsigned long        dataLen,
                          const char*          tag,
                          const char*          format,
                          va_list              args)
{
    if (g_Enable[kiAP2LogTypeData])
    {
        if (data && dataLen > 0)
        {
#if DEBUG
            char* lineStr = malloc (LOG_STRING_SIZE);
            long lineStrLen;
            unsigned long i;
            char tmpStr[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
            char spaceStr[24] = "                        ";

            iAP2LogTypeVNL (kiAP2LogTypeData, format, args);
            iAP2LogTypePureNL (kiAP2LogTypeData, "\n");
            iAP2LogTypeNL (kiAP2LogTypeData, "%s    %d bytes\n", (tag ? tag : ""), dataLen);
            lineStrLen = 0;
            sprintf (&lineStr[lineStrLen], "%s    [ %04X: ", (tag ? tag : ""), 0);
            lineStrLen = strlen (lineStr);
            for (i = 0; i < dataLen; ++i)
            {
                if (i != 0 && (i % 8) == 0)
                {
                    sprintf (&lineStr[lineStrLen],
                             "    %c%c%c%c%c%c%c%c\n",
                             iAP2LogDataChar (tmpStr[0]),
                             iAP2LogDataChar (tmpStr[1]),
                             iAP2LogDataChar (tmpStr[2]),
                             iAP2LogDataChar (tmpStr[3]),
                             iAP2LogDataChar (tmpStr[4]),
                             iAP2LogDataChar (tmpStr[5]),
                             iAP2LogDataChar (tmpStr[6]),
                             iAP2LogDataChar (tmpStr[7]));
                    iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
                    lineStrLen = 0;
                    sprintf (&lineStr[lineStrLen],
                             "%s      %04lX: ", (tag ? tag : ""), i);
                    lineStrLen = strlen (lineStr);
                    memset(tmpStr,' ',8*sizeof(char));
                }
                tmpStr [i & 0x07] = data[i];
                sprintf (&lineStr[lineStrLen],
                         "%02X ", data[i]);
                lineStrLen = strlen (lineStr);
            }
            spaceStr[3*(8-(i & 0x07))] = 0;
            if ((i & 0x07) == 0 && i != 0)
            {
                spaceStr[0] = 0;
            }
            sprintf (&lineStr[lineStrLen],
                     "]   %s%c%c%c%c%c%c%c%c\n",
                     spaceStr,
                     iAP2LogDataChar (tmpStr[0]),
                     iAP2LogDataChar (tmpStr[1]),
                     iAP2LogDataChar (tmpStr[2]),
                     iAP2LogDataChar (tmpStr[3]),
                     iAP2LogDataChar (tmpStr[4]),
                     iAP2LogDataChar (tmpStr[5]),
                     iAP2LogDataChar (tmpStr[6]),
                     iAP2LogDataChar (tmpStr[7]));
            iAP2LogTypeNL (kiAP2LogTypeData, lineStr);
            free (lineStr);
#endif
        }
    }
}


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
**  Note:   No lock is used for this call... use iAP2LogStart/Stop to
**          acquire/release locks.
**          Pure versions do not print the prefix.
**
****************************************************************
*/
void iAP2LogPrintDataPureVNL (const unsigned char* data,
                              unsigned long        dataLen,
                              const char*          tag,
                              const char*          format,
                              va_list              args)
{
    if (g_Enable[kiAP2LogTypeData])
    {
        if (data && dataLen > 0)
        {
#if DEBUG
            char* lineStr = malloc (LOG_STRING_SIZE);
            long lineStrLen = 0;
            unsigned long i;
            char tmpStr[8] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
            char spaceStr[24] = "                        ";
            iAP2LogTypePureVNL (kiAP2LogTypeData, format, args);
            iAP2LogTypePureNL (kiAP2LogTypeData, "\n");
            iAP2LogTypePureNL (kiAP2LogTypeData, "%s    %d bytes\n", (tag ? tag : ""), dataLen);
            lineStrLen = 0;
            sprintf (&lineStr[lineStrLen], "%s    [ %04X: ", (tag ? tag : ""), 0);
            lineStrLen = strlen (lineStr);
            for (i = 0; i < dataLen; ++i)
            {
                if (i != 0 && (i % 8) == 0)
                {
                    sprintf (&lineStr[lineStrLen],
                             "    %c%c%c%c%c%c%c%c\n",
                             iAP2LogDataChar (tmpStr[0]),
                             iAP2LogDataChar (tmpStr[1]),
                             iAP2LogDataChar (tmpStr[2]),
                             iAP2LogDataChar (tmpStr[3]),
                             iAP2LogDataChar (tmpStr[4]),
                             iAP2LogDataChar (tmpStr[5]),
                             iAP2LogDataChar (tmpStr[6]),
                             iAP2LogDataChar (tmpStr[7]));
                    iAP2LogTypePureNL (kiAP2LogTypeData, lineStr);
                    lineStrLen = 0;
                    sprintf (&lineStr[lineStrLen],
                             "%s      %04lX: ", (tag ? tag : ""), i);
                    lineStrLen = strlen (lineStr);
                    memset(tmpStr,' ',8*sizeof(char));
                }
                tmpStr [i & 0x07] = data[i];
                sprintf (&lineStr[lineStrLen],
                         "%02X ", data[i]);
                lineStrLen = strlen (lineStr);
            }
            spaceStr[3*(8-(i & 0x07))] = 0;
            if ((i & 0x07) == 0 && i != 0)
            {
                spaceStr[0] = 0;
            }
            sprintf (&lineStr[lineStrLen],
                     "]   %s%c%c%c%c%c%c%c%c\n",
                     spaceStr,
                     iAP2LogDataChar (tmpStr[0]),
                     iAP2LogDataChar (tmpStr[1]),
                     iAP2LogDataChar (tmpStr[2]),
                     iAP2LogDataChar (tmpStr[3]),
                     iAP2LogDataChar (tmpStr[4]),
                     iAP2LogDataChar (tmpStr[5]),
                     iAP2LogDataChar (tmpStr[6]),
                     iAP2LogDataChar (tmpStr[7]));
            iAP2LogTypePureNL (kiAP2LogTypeData, lineStr);
            free (lineStr);
#endif
        }
    }
}


