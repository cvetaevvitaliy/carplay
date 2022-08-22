/*
 * =====================================================================================
 *
 *       Filename:  iAP2FileSession.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/10/2013 03:06:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_FILESESSION_H__
#define __IAP2_FILESESSION_H__
#include <glib.h>
#include <dev_iap2.h>
typedef enum _iAP2FileType_
{
	IAP2_ITEM,
	IAP2_LIST
} iAP2FileType;				/* ----------  end of enum iAP2FileType  ---------- */

typedef struct _iAP2FileInfo_ 
{
	uint64_t itemId;
	iAP2FileType fileType;
	gchar * name;
} iAP2FileInfo;				/* ----------  end of struct iAP2FileInfo  ---------- */

void iAP2ParseFileSession ( iAP2Link_t * link, uint8_t * data, int dataLen, uint8_t session);
void iAP2FileSetTransferType ( MHDevIap2 * _iAP2Object, uint8_t buffId, uint64_t itemId, iAP2FileType type, const gchar * name );
void iAP2FileCancelPreviousTransfer ();
void iAP2FileProcessCachedXfers( MHDevIap2 * _iAP2Object);
void iAP2FileCleanupTransfer (MHDevIap2 * _iAP2Object);

#endif
