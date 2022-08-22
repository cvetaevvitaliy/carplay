/*
 * =====================================================================================
 *
 *       Filename:  iAP2FileSession.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/10/2013 11:25:19 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include "debug.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <iAP2Link.h>
#include <iAP2FileTransfer.h>
#include "iAP2FileSession.h"
#include "iAP2Media.h"
#include <iAP2Misc.h>
#include "dev_iap2.h"
#include <mh_contents.h>
#include <mh_playlist.h>
#include <mh_pb.h>

static iAP2FileTransfer_t * xfers[0x100];
static iAP2FileTransfer_t * nowXfer;
extern GSList * _iAP2GlobalInfolist;

static guint8 * cacheData;
static gint cacheLen;

//extern MHDevIap2 * iAP2Object;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2FileProcessCachedXfers
 *  Description:  
 * =====================================================================================
 */
void iAP2FileProcessCachedXfers( MHDevIap2 * _iAP2Object )
{
	gboolean _changed	=	TRUE;
	MHPlaylist * _playlist	=	NULL;

	if( cacheData != NULL )
	{
		/* Omit duplicated data transfer */
		if( _iAP2Object->currentData != NULL && _iAP2Object->currentLen == cacheLen )
			if( memcmp( _iAP2Object->currentData, cacheData, cacheLen ) == 0 )
				_changed	=	FALSE;

		if( _changed )
		{
			g_message( "current playlist changed" );
			_playlist	=	restore_playlist_from_data( MH_DEV( _iAP2Object ), cacheData, cacheLen );

			mh_pb_playlist_by_change( _iAP2Object->pb, _playlist );

			if( _iAP2Object->currentData != NULL )
				g_free( _iAP2Object->currentData );

			_iAP2Object->currentData	=	cacheData;
			_iAP2Object->currentLen	=	cacheLen;

			cacheData	=	NULL;
		}
	}
}		/* -----  end of function iAP2FileProcessCachedXfers  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  fileXferGotDataCB
 *  Description:  
 * =====================================================================================
 */
static BOOL fileXferGotDataCB(iAP2FileTransfer_t * xfer, void * userInfo)
{
	iAP2FileInfo * _fileInfo	=	(iAP2FileInfo *)userInfo;
	int	_fileName;

	GSList *iterator = NULL;
	MHDevIap2 * _iAP2Object = NULL;

	for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
	{
		if (xfer->link == ((iAP2GlobalInfo *)iterator->data)->piAP2Object->plinkRunLoop->link)
		{
			_iAP2Object = ((iAP2GlobalInfo *)iterator->data)->piAP2Object;			 
			break;
		}
	}
	if (_iAP2Object == NULL)
	{
		g_message("not find iap2object fileXferGotDataCB");
		return FALSE;
	}

	if(_fileInfo != NULL)
	{
		switch(_fileInfo->fileType)
		{
		case IAP2_ITEM:
			if(xfer->buffSize != 0)
			{
				g_message("Got File Data %d %s\n", xfer->bufferID, _fileInfo->name );

				_fileInfo->name	=	g_strdup_printf( "/tmp/%s", _fileInfo->name );

				g_file_set_contents(_fileInfo->name, (gchar *)xfer->pBuffer, xfer->buffSize,
						NULL);
			}
			else
			{
				printf("no artwork\n");
			}
			if( _iAP2Object->pb != NULL )
			{
				MHPbInfoData * _info = g_slice_new( MHPbInfoData );

				if( xfer->buffSize	!=	0 )
				{
					_info->cover_path	=	g_strdup( _fileInfo->name );
				}else{
					_info->cover_path	=	g_strdup( "" ); 
				}
				g_message("cover_path = %s",_info->cover_path);
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_COVER_PATH, _info );
				}else{
					g_message("  cover data invalid!!");
				}
			}

			break;
		case IAP2_LIST:
			if( _fileInfo->name != NULL )
			{
				g_message( "playlist: %s", _fileInfo->name );
				gint64 _playlistId	=	mh_contents_get_playlistid_by_tagId( mh_contents_instance(), _fileInfo->itemId );

				if( _playlistId < 0 )
				{
					mh_contents_save_playlist( mh_contents_instance(), MH_DEV( _iAP2Object ), _fileInfo->name, 
						_fileInfo->itemId, xfer->pBuffer, xfer->buffSize );
				}
				else
				{
					mh_contents_update_playlist( mh_contents_instance(), MH_DEV( _iAP2Object ), _playlistId,
							_fileInfo->name, _fileInfo->itemId, xfer->pBuffer, xfer->buffSize );
				}
				
				g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, 0);
			}
			else
			{
				g_message( "playlist: current, %lld", (long long int)xfer->buffSize / 8 );

				cacheLen	=	xfer->buffSize;
				cacheData	=	g_malloc0( xfer->buffSize );

				memcpy( cacheData, xfer->pBuffer, xfer->buffSize );

				/* Update current playing playlist */
				if( _iAP2Object->syncComplete )
				{
					iAP2FileProcessCachedXfers( _iAP2Object );
				}
			}

			break;
		default:
			printf("Unknown File Type\n");
			g_assert_not_reached();
			break;
		}

		if( _fileInfo->name != NULL )
			g_free( _fileInfo->name );

		g_free( userInfo );

		iAP2FileTransferCleanup(xfer);

		_iAP2Object->xfers[ xfer->bufferID ]	=	NULL;

		return TRUE;
	}
	else
	{
		g_warning( "Transfer wasn't setup %d", xfer->bufferID );

		return FALSE;
	}
}		/* -----  end of static function fileXferGotDataCB  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2FileGetTransferById
 *  Description:  
 * =====================================================================================
 */
iAP2FileTransfer_t * iAP2FileGetTransferById (iAP2Link_t * link, uint8_t session, 
		uint8_t buffId)
{
	uint8_t _buffId	=	buffId;
	
	MHDevIap2 * _iAP2Object = NULL;
	GSList *iterator = NULL;
	for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
	{
		if (link == ((iAP2GlobalInfo *)iterator->data)->piAP2Object->plinkRunLoop->link)
		{
			_iAP2Object = ((iAP2GlobalInfo *)iterator->data)->piAP2Object;
			break;
		}
	}

	if(_iAP2Object->xfers[_buffId] == NULL)
	{
		_iAP2Object->xfers[_buffId]	=	iAP2FileTransferCreate(link, session, buffId, fileXferGotDataCB,
				NULL, FALSE, NULL); 

		_iAP2Object->xfers[_buffId]->bDeleteBuffOnFinish	=	TRUE;
	}

	return _iAP2Object->xfers[_buffId];
}		/* -----  end of function iAP2FileGetTransferById  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2FileSetTransferType
 *  Description:  
 * =====================================================================================
 */
void iAP2FileSetTransferType ( MHDevIap2 * _iAP2Object, uint8_t buffId, uint64_t itemId, iAP2FileType type, const gchar * name )
{
	nowXfer	=	iAP2FileGetTransferById(_iAP2Object->plinkRunLoop->link, IAP2_FILE_SESSION_ID, buffId);

	nowXfer->gotCBUserInfo	=	g_new0(iAP2FileInfo, 1);

	((iAP2FileInfo *)nowXfer->gotCBUserInfo)->itemId	=	itemId;
	((iAP2FileInfo *)nowXfer->gotCBUserInfo)->fileType	=	type;
	((iAP2FileInfo *)nowXfer->gotCBUserInfo)->name	=	g_strdup( name );

	if( nowXfer->state == kiAP2FileTransferStateFinishRecv )
	{
		nowXfer->gotCB( nowXfer, nowXfer->gotCBUserInfo );

		if (nowXfer->pBuffer)
		{
			free (nowXfer->pBuffer);
		}
		nowXfer->pBuffer = NULL;
		nowXfer->buffSize = 0;
		nowXfer->buffSentSize = 0;
		nowXfer->pCurPos = nowXfer->pBuffer;
	}
}		/* -----  end of function iAP2FileSetTransferType  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2ParseFileSession
 *  Description:  
 * =====================================================================================
 */
void iAP2ParseFileSession ( iAP2Link_t * link, uint8_t * data, int dataLen, uint8_t session)
{
	static iAP2FileTransfer_t * _xfer;

	_xfer	=	iAP2FileGetTransferById( link, IAP2_FILE_SESSION_ID, data[kiAP2FileTransferHdrIdxID]);

	if(_xfer != NULL)
	{
		iAP2FileTransferHandleRecv(_xfer, data, dataLen);
	}
}		/* -----  end of function iAP2ParseFileSession  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2FileCancelPreviousTransfer
 *  Description:  
 * =====================================================================================
 */
void iAP2FileCancelPreviousTransfer ()
{
	if(nowXfer != NULL)
	{
		iAP2FileTransferCancel(nowXfer);
	}
}		/* -----  end of function iAP2FileCancelPreviousTransfer  ----- */

void iAP2FileCleanupTransfer (MHDevIap2 * _iAP2Object)
{
	int i = 0;
	g_message("------->iAP2FileCleanupTransfer");
	for (i = 0; i<0x100; i++)
	{
		if(_iAP2Object->xfers[i] == NULL)
			continue;
		iAP2FileTransferCleanup(_iAP2Object->xfers[i]);

		_iAP2Object->xfers[ i ] =	NULL;
	}
}

