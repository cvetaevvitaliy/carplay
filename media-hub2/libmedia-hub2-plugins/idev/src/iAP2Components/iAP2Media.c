/*
 * =====================================================================================
 *
 *       Filename:  iAP2Media.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 11:43:15 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iAP2Media.h"
#include <glib.h>

#include <assert.h>

#include "iAP2Defines.h"
#include "iAP2ControlSession.h"
#include "debug.h"
#include <iAP2Misc.h>
#include "iAP2Hid.h"
#include <iAP2FileTransfer.h>
#include "iAP2FileSession.h"
#include "dev_iap2.h"
#include <mh_contents.h>
#include <mh_misc.h>
#include <mh_pb.h>
#include <mh_playlist.h>
//extern MHDevIap2 * iAP2Object;
extern iAP2FileTransfer_t * cachedXfer;

static MHPbInfoData infoData;

static MHPbRepeatMode repeat_mode;

static MHPbShuffleMode shuffle_mode;

static uint32_t count = 0;

static bool ptime_s = FALSE;
//#define printf( ... ) NULL
GSList *gRadioList = NULL;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaPlayByIndex
 *  Description:  
 * =====================================================================================
 */
//void iAP2MediaPlayByIndex (uint32_t index)
//{
//	GSList * _item;
//	uint16_t _libIdLen;
//	gchar * _libId;
//	uint8_t * _params;
//	uint16_t _paramLen;
//	uint32_t _count	=	0;
//	uint8_t * _ids	=	NULL;
//
//	if(itemsList != NULL && index >= 0)
//	{
//		for(_item = itemsList; _item != NULL; _item = _item->next)
//		{
//			_ids	=	g_realloc(_ids, (_count + 1) * sizeof(((iAP2MediaItem *)_item->data)->itemId));
//
//			IAP2_MISC_WRITE_BIG64(_ids + _count * sizeof(((iAP2MediaItem *)_item->data)->itemId), ((iAP2MediaItem *)_item->data)->itemId);
//
//			_count ++;
//		}
//
//		if(index < _count)
//		{
//			uint32_t _itemsSize	=	_count * sizeof(((iAP2MediaItem *)_item->data)->itemId);
//			iAP2MediaGetLibraryId(&_libId, &_libIdLen);
//
//			_paramLen	=	4 + _itemsSize + 4 + sizeof(uint32_t) + 4 + _libIdLen;
//
//			_params	=	g_malloc0(_paramLen);
//
//			_params[0]	=	IAP2_HI_BYTE(4 + _itemsSize);
//			_params[1]	=	IAP2_LO_BYTE(4 + _itemsSize);
//			_params[2]	=	0x00;
//			_params[3]	=	0x00;               /* Items Persistent Identifiers */
//
//			memcpy(_params + 4, _ids, _itemsSize);
//
//			_params[4 + _itemsSize]		=	IAP2_HI_BYTE(4 + sizeof(uint32_t));
//			_params[4 + _itemsSize + 1]	=	IAP2_LO_BYTE(4 + sizeof(uint32_t));
//			_params[4 + _itemsSize + 2]	=	0x00;
//			_params[4 + _itemsSize + 3]	=	0x01; /* ItemsStartingIndex */
//			_params[4 + _itemsSize + 4]	=	index >> 24;
//			_params[4 + _itemsSize + 5]	=	index >> 16;
//			_params[4 + _itemsSize + 6]	=	index >> 8;
//			_params[4 + _itemsSize + 7]	=	index & 0xFF;
//
//			_params[4 + _itemsSize + 4 + sizeof(uint32_t)]		=	IAP2_HI_BYTE(4 + _libIdLen);
//			_params[4 + _itemsSize + 4 + sizeof(uint32_t) + 1]	=	IAP2_LO_BYTE(4 + _libIdLen);
//			_params[4 + _itemsSize + 4 + sizeof(uint32_t) + 2]	=	0x00;
//			_params[4 + _itemsSize + 4 + sizeof(uint32_t) + 3]	=	0x02; /* Media Library Unique ID */
//
//			memcpy(_params + 4 + _itemsSize + 4 + sizeof(uint32_t) + 4, _libId, _libIdLen);
//
//			iAP2SendControlMessage(iAP2InitGetLink(), 0x4C07, _params, _paramLen, IAP2_CONTROL_SESSION_ID());
//			iAP2FileCancelPreviousTransfer ();
//		}
//	}
//}		/* -----  end of function iAP2MediaPlayByIndex  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaParseLibraryInformation
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaParseLibraryInformation (MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen )
{
	int _offset	=	4;
	int groupLen = MAKE_WORD( data[ 0 ], data[ 1 ]);
	gchar * _libraryTypeStr[]	=	{
		"Local device library",
		"Unknown",
		"iTunes Radio library"
	};
	g_message("groupLen = %d, dataLen = %d", groupLen, dataLen);
	while( _offset < groupLen )
	{
		switch( MAKE_WORD( data[ _offset + 2 ], data[ _offset + 3 ]))
		{
		case 0x00:
			g_message( "Media Library Name: %s", data + _offset + 4 );
			_iAP2Object->mediaLibraryName	=	g_strdup( data + _offset + 4 );

			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		case 0x01:
			g_message( "Media Library Id: %s", data + _offset + 4 );
			_iAP2Object->mediaLibraryId	=	g_strdup( data + _offset + 4 );
			_iAP2Object->libraryIdLen	=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]) - 4;

			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		case 0x02:
			g_message( "Media Library Type: %s", _libraryTypeStr[ data[ _offset + 4 ] ] );

			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		default:
			g_assert_not_reached();
			break;
		}
	}
	
	//g_message("_offset = %d, dataLen = %d",_offset,  dataLen);
	groupLen = MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
	_offset = _offset + 4;
	_iAP2Object->radioLibraryIdLen = 0;
	while( _offset < dataLen )
	{
		switch( MAKE_WORD( data[ _offset + 2 ], data[ _offset + 3 ]))
		{
		case 0x00:
			g_message( "Radio Library Name: %s", data + _offset + 4 );
			_iAP2Object->radioLibraryName	=	g_strdup( data + _offset + 4 );			
			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		case 0x01:
			g_message( "Radio Library Id: %s", data + _offset + 4 );
			_iAP2Object->radioLibraryId	=	g_strdup( data + _offset + 4 );
			_iAP2Object->radioLibraryIdLen	=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]) - 4;
			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		case 0x02:
			g_message( "Radio Library Type: %s", _libraryTypeStr[ data[ _offset + 4 ] ] );
			_offset	+=	MAKE_WORD( data[ _offset ], data[ _offset + 1 ]);
			break;
		default:
			g_assert_not_reached();
			break;
		}
	}
	//	g_message("_offset = %d, dataLen = %d",_offset,  dataLen);
}		/* -----  end of function iAP2MediaParseLibraryInformation  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseMediaItem
 *  Description:  
 * =====================================================================================
 */
static MHMusic * parseMediaItem (MHDevIap2 * _iAP2Object,  uint8_t * data, int dataLen, bool flag)
{
	int _offset	=	0;
	int _paramLen, _paramId;
	MHMusic * _music;
	guint8 _artworkId;
	gint64 _itemId;
	gchar _buf[16];
	gchar * _mediaType[]	=	
	{
		"Music",
		"Podcast",
		"AudioBook",
		"iTunesU"
	};

	_music	=	g_object_new( MH_TYPE_MUSIC, NULL );

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);
		if( flag == TRUE )
		{
			switch(_paramId)
			{
				case 0:
					_itemId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);

					MH_ITEM( _music )->tagId	=	 _itemId ;
					infoData.media_info.tagId	=	_itemId;
//					printf("\tgroup param type is: MediaItemPersistentIdentifier[%llx]\n", MH_ITEM( _music )->tagId);
					break;
				case 1:
					_music->title	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.title != NULL )
					{
						g_free( infoData.media_info.title );
						infoData.media_info.title = NULL;
					}
					infoData.media_info.title	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemTitle[%s]", _music->title);
					break;
				case 2:
					_music->mediaType	=	data[_offset + 4];
					g_message("\tgroup param type is: MediaItemMediaType[%s]", _mediaType[data[_offset + 4]]);
					break;
				case 3:
					_music->rating	=	data[_offset + 4];
					infoData.media_info.rating	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemRating[%d]\n", _music->rating);
					break;
				case 4:
					_music->duration	=	MAKE_DWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);
					infoData.media_info.duration	=	MAKE_DWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);
//					printf("\tgroup param type is: MediaItemPlaybackDurationInMilliseconds[%d]\n", _music->duration);
					break;
				case 5:
//					_music->albumId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemAlbumPersistentIdentifer[%llx]\n", _item->albumId);
					break;
				case 6:
					_music->album_title	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.album_title != NULL )
					{
						g_free( infoData.media_info.album_title );
						infoData.media_info.album_title	=	NULL;
					}
					infoData.media_info.album_title	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemAlbumTitle[%s]", _music->album_title);
					break;
				case 7:
					_music->track	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
					infoData.media_info.track	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);	
//					printf("\tgroup param type is: MediaItemAlbumTrackNumber[%d]\n", _music->track);
					break;
				case 8:
					_music->track_count	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
					infoData.media_info.track_count	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//					printf("\tgroup param type is: MediaItemAlbumTrackCount[%d]\n", _music->track_count);
					break;
				case 9:
					_music->disc	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
					infoData.media_info.disc	=	_music->disc;
//					printf("\tgroup param type is: MediaItemAlbumDiscNumber[%d]\n", _music->disc);
					break;
				case 10:
					_music->disc_count	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
					infoData.media_info.disc_count	=	_music->disc_count;
//					printf("\tgroup param type is: MediaItemAlbumDiscCount[%d]\n", _music->disc_count);
					break;
				case 11:
//					_music->artistId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemArtistPersistentIdentifier[%llx]\n", _item->artistId);
					break;
				case 12:
					_music->artist	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.artist != NULL )
					{
						g_free( infoData.media_info.artist );
						infoData.media_info.artist	=	NULL;
					}
					infoData.media_info.artist	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemArtist[%s]", _music->artist);
					break;
				case 13:
//					_music->albumArtistId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemAlbumArtistPersistentIdentifier[%llx]\n", _item->albumArtistId);
					break;
				case 14:
					_music->album_artist	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.album_artist != NULL )
					{
						g_free( infoData.media_info.album_artist );
						infoData.media_info.album_artist	=	NULL;
					}
					infoData.media_info.album_artist	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemAlbumArtist[%s]", _music->album_artist);
					break;
				case 15:
//					_music->genreId		=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemGenrePersistentIdentifier[%llx]\n", _item->genreId);
					break;
				case 16:
					_music->genre	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.genre != NULL )
					{
						g_free( infoData.media_info.genre );
						infoData.media_info.genre	=	NULL;
					}
					infoData.media_info.genre	=	g_strdup((gchar *)data + _offset + 4);
//					printf("\tgroup param type is: MediaItemGenre[%s]\n", _music->genre);
					break;
				case 17:
					//				_music->composerId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
					//printf("\tgroup param type is: MediaItemComposerPersistentIdentifier[%llx]\n", _item->composerId);
					break;
				case 18:
					_music->composer	=	g_strdup((gchar *)data + _offset + 4);
					if( infoData.media_info.composer != NULL )
					{
						g_free( infoData.media_info.composer );
						infoData.media_info.composer	=	NULL;
					}
					infoData.media_info.composer	=	g_strdup((gchar *)data + _offset + 4);
					//				printf("\tgroup param type is: MediaItemComposer[%s]\n", _music->composer);
					break;
				case 19:
					_music->album_compliation	=	data[_offset + 4];
					break;
				case 20:
				case 26:
					_artworkId =	data[_offset + 4];
					count ++;
					if( count == 1000 )
						count	=	0;
					//				sprintf( _buf, "%016llX.jpg", MH_ITEM( _music )->tagId );
					sprintf( _buf, "%03d.jpg", count );
					g_message("\tgroup param type is: MediaItemArtworkFileTransferIdentifier [%d]\n", data[_offset + 4]);
					iAP2FileSetTransferType(_iAP2Object, data[_offset + 4], _artworkId, IAP2_ITEM, _buf );
					break;
				case 21:
					//				_music->isLikeSupported	=	data[_offset + 4];
					//printf("\tgroup param type is: MediaItemPropertyIsLikeSupported[%d]\n", _item->isLikeSupported);
					break;
				case 22:
					//				_music->isBanSupported	=	data[_offset + 4];
					//printf("\tgroup param type is: MediaItemPropertyIsBanSupported[%d]\n", _item->isBanSupported);
					break;
				case 23:
					//				_music->isLiked	=	data[_offset + 4];
					//printf("\tgroup param type is: MediaItemPropertyIsLiked[%d]\n", _item->isLiked);
					break;
				case 24:
					//				_music->isBanned	=	data[_offset + 4];
					//printf("\tgroup param type is: MediaItemPropertyIsBanned[%d]\n", _item->isBanned);
					break;
				case 25:
					//				_music->isResidentOnDevice	=	data[_offset + 4];
					//printf("\tgroup param type is: MediaItemPropertyIsResidentOnDevice[%d]\n", _item->isResidentOnDevice);
					break;
				case 27:
					//				g_message( "\tgroup param type is: MediaItemChapterCount [%d]\n", MAKE_WORD( data[_offset + 4], data[ _offset + 5 ]));
					break;
				default:
					g_message( "%d", _paramId );
					assert(0);
					break;
			}
		}else{
			switch(_paramId)
			{
				case 0:
					_itemId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);

					MH_ITEM( _music )->tagId	=	 _itemId ;
//					printf("\tgroup param type is: MediaItemPersistentIdentifier[%llx]\n", MH_ITEM( _music )->tagId);
					break;
				case 1:
					_music->title	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemTitle[%s]", _music->title);
					break;
				case 2:
					_music->mediaType	=	data[_offset + 4];
					g_message("\tgroup param type is: MediaItemMediaType[%s]", _mediaType[data[_offset + 4]]);
					break;
				case 3:
					_music->rating	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemRating[%d]\n", _music->rating);
					break;
				case 4:
					_music->duration	=	MAKE_DWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);
//					printf("\tgroup param type is: MediaItemPlaybackDurationInMilliseconds[%d]\n", _music->duration);
					break;
				case 5:
//					_music->albumId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemAlbumPersistentIdentifer[%llx]\n", _item->albumId);
					break;
				case 6:
					_music->album_title	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemAlbumTitle[%s]", _music->album_title);
					break;
				case 7:
					_music->track	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//					printf("\tgroup param type is: MediaItemAlbumTrackNumber[%d]\n", _music->track);
					break;
				case 8:
					_music->track_count	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//					printf("\tgroup param type is: MediaItemAlbumTrackCount[%d]\n", _music->track_count);
					break;
				case 9:
					_music->disc	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//					printf("\tgroup param type is: MediaItemAlbumDiscNumber[%d]\n", _music->disc);
					break;
				case 10:
					_music->disc_count	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//					printf("\tgroup param type is: MediaItemAlbumDiscCount[%d]\n", _music->disc_count);
					break;
				case 11:
//					_music->artistId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemArtistPersistentIdentifier[%llx]\n", _item->artistId);
					break;
				case 12:
					_music->artist	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemArtist[%s]", _music->artist);
					break;
				case 13:
//					_music->albumArtistId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemAlbumArtistPersistentIdentifier[%llx]\n", _item->albumArtistId);
					break;
				case 14:
					_music->album_artist	=	g_strdup((gchar *)data + _offset + 4);
					g_message("\tgroup param type is: MediaItemAlbumArtist[%s]", _music->album_artist);
					break;
				case 15:
//					_music->genreId		=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemGenrePersistentIdentifier[%llx]\n", _item->genreId);
					break;
				case 16:
					_music->genre	=	g_strdup((gchar *)data + _offset + 4);
//					printf("\tgroup param type is: MediaItemGenre[%s]\n", _music->genre);
					break;
				case 17:
//					_music->composerId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);
//					printf("\tgroup param type is: MediaItemComposerPersistentIdentifier[%llx]\n", _item->composerId);
					break;
				case 18:
					_music->composer	=	g_strdup((gchar *)data + _offset + 4);
//					printf("\tgroup param type is: MediaItemComposer[%s]\n", _music->composer);
					break;
				case 19:
					_music->album_compliation	=	data[_offset + 4];
					break;
				case 20:
				case 26:
					_artworkId =	data[_offset + 4];
					count ++;
					if( count == 1000 )
						count	=	0;
//					sprintf( _buf, "%016llX.jpg", MH_ITEM( _music )->tagId );
					sprintf( _buf, "%03d.jpg", count );
					g_message("\tgroup param type is: MediaItemArtworkFileTransferIdentifier [%d]\n", data[_offset + 4]);
					iAP2FileSetTransferType(_iAP2Object, data[_offset + 4], _artworkId, IAP2_ITEM, _buf );
					break;
				case 21:
//					_music->isLikeSupported	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemPropertyIsLikeSupported[%d]\n", _item->isLikeSupported);
					break;
				case 22:
//					_music->isBanSupported	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemPropertyIsBanSupported[%d]\n", _item->isBanSupported);
					break;
				case 23:
//					_music->isLiked	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemPropertyIsLiked[%d]\n", _item->isLiked);
					break;
				case 24:
//					_music->isBanned	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemPropertyIsBanned[%d]\n", _item->isBanned);
					break;
				case 25:
//					_music->isResidentOnDevice	=	data[_offset + 4];
//					printf("\tgroup param type is: MediaItemPropertyIsResidentOnDevice[%d]\n", _item->isResidentOnDevice);
					break;
				case 27:
//					g_message( "\tgroup param type is: MediaItemChapterCount [%d]\n", MAKE_WORD( data[_offset + 4], data[ _offset + 5 ]));
					break;
				default:
					g_message( "%d", _paramId );
//					assert(0);
					break;
			}

		}

		_offset	+=	_paramLen;
	}

	return _music;
}		/* -----  end of static function parseMediaItem  ----- */

void iAP2FreeRadioDate()
{
	iAP2RadioPlayListInfo *pstRadioInfo = NULL;
	g_message("%s", __func__);

	while (gRadioList != NULL)
	{
		pstRadioInfo = (iAP2RadioPlayListInfo *)gRadioList->data;
		g_message("name:%s,id=0x%llx", pstRadioInfo->listName, pstRadioInfo->listId);
		if (pstRadioInfo !=NULL && pstRadioInfo->listName != NULL)
		{
			g_free(pstRadioInfo->listName);
		}
		g_free(pstRadioInfo);
		gRadioList = gRadioList->next;
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseMediaPlaylist
 *  Description:  
 * =====================================================================================
 */
static void parseMediaPlaylist ( MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen )
{
	int _offset	=	0;
	int _paramLen, _paramId;
	uint64_t _listId	=	0, _parentId;
//	iAP2PlayListHeader * _hdr	=	g_new0(iAP2PlayListHeader, 1);
	gchar * _listName	=	NULL;
	iAP2RadioPlayListInfo *pstRadioInfo = NULL;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				_listId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);

				//printf("\tgroup param type is: MediaPlaylistPersistentIdentifer[%llx]\n", _listId);
				break;
			case 1:
				_listName	=	g_strdup((gchar *)data + _offset + 4);
//				printf("\tgroup param type is: MediaPlaylistName[%s]\n", data + _offset + 4);
				break;
			case 2:
				_parentId	=	IAP2_MISC_READ_BIG64(data + _offset + 4);

//				printf("\tgroup param type is: MediaPlaylistParentPersistentIdentifer[%llx]\n", _parentId);
				break;
			case 3:
				//printf("\tgroup param type is: MediaPlaylistIsGeniusMix[%d]\n", data[_offset + 4]);
				break;
			case 4:
//				_hdr->isFolder	=	data[_offset + 4];
				//printf("\tgroup param type is: MediaPlaylistIsFolder[%d]\n", data[_offset + 4]);
				break;
			case 5:
//				printf("\tgroup param type is: MediaPlaylistContainedMediaItemsFileTransferIdentifier[%d]\n",
//						data[_offset + 4]);

				iAP2FileSetTransferType(_iAP2Object, data[_offset + 4], _listId, IAP2_LIST, _listName );
				break;
			case 6:
				//printf("\tgroup param type is: MediaPlaylistPropertyIsiTunesRadioStation[%d]\n", data[_offset + 4]);
				_iAP2Object->radioLibraryFlag = 1; //updating
				pstRadioInfo = (iAP2RadioPlayListInfo *)malloc(sizeof(iAP2RadioPlayListInfo));
				pstRadioInfo->listId = _listId;
				pstRadioInfo->listName = _listName;
				if (_listId == 0x100000001)
				{	
					iAP2FreeRadioDate();
				}
				gRadioList = g_slist_append (gRadioList,  pstRadioInfo);
				break;
			default:
				assert(0);
				break;
		}

		_offset	+=	_paramLen;
	}
}		/* -----  end of static function parseMediaPlaylist  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  headerCompare
 *  Description:  
 * =====================================================================================
 */
static gint headerCompare ( gconstpointer a, gconstpointer b )
{
	const iAP2PlayListHeader * _hdr	=	(iAP2PlayListHeader *)a;
	const uint64_t * _id	=	(uint64_t *)b;

	return (_hdr->listId == *_id ? 0 : 1);
}		/* -----  end of static function headerCompare  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaGetPlaylistHeaderById
 *  Description:  
 * =====================================================================================
 */
iAP2PlayListHeader * iAP2MediaGetPlaylistHeaderById ( MHDevIap2 * _iAP2Object, uint64_t listId )
{
	iAP2PlayListHeader * _hdr	=	NULL;
	GSList * _list	=	g_slist_find_custom(_iAP2Object->playListHeaders, &listId, headerCompare);

	if(_list != NULL)
	{
		_hdr	=	(iAP2PlayListHeader *)_list->data;
	}
	
	return _hdr;
}		/* -----  end of function iAP2MediaGetPlaylistHeaderById  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  itemCompare
 *  Description:  
 * =====================================================================================
 */
static gint itemCompare ( gconstpointer a, gconstpointer b )
{
	iAP2MediaItem * _a	=	(iAP2MediaItem *)a;
	iAP2MediaItem * _b	=	(iAP2MediaItem *)b;

	return (_a->itemId == _b->itemId) ? 0 : 1;
}		/* -----  end of static function itemCompare  ----- */
extern struct timeval startTime;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaParseMLUMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaParseMLUMessage ( MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen)
{
	int _offset	=	0;
	int _paramLen, _paramId;
	MHMusic * _music;
	MHContents * _contents	=	mh_contents_instance();
	gint64 _nodeId, _itemId;
	gchar _buf[64];
	MHItem * _item;

	mh_contents_begin_transaction( _contents );

	/* Find if MediaPlaylistDeletePersistentIdentifier exists before we update database */
	while( _offset < dataLen )
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		if( _paramId == 6 )
		{
			if ( _iAP2Object->libraryRevision == NULL)
			{
				g_message("param type is: MediaLibraryReset\n");
				mh_contents_delete_node_by_device_id( _contents, MH_DEV( _iAP2Object )->uniqueId );
			        mh_contents_delete_playlist_by_device_id( _contents, MH_DEV( _iAP2Object )->uniqueId );
			}
			else
			{
				g_message("_iAP2Object->libraryRevision exist [%s]\n", _iAP2Object->libraryRevision);
			}
		}

		_offset	+=	_paramLen;
	}

	_offset	=	0;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				g_message( "MediaLibraryUniqueIdentifier[ %s ]\n", (gchar *)data + _offset + 4 );
				
				if(  _iAP2Object->radioLibraryId != NULL && strcmp( _iAP2Object->radioLibraryId, (gchar *)data + _offset + 4  ) == 0 )
				{
					g_message("radio list update !");
					g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, 0);
					_iAP2Object->radioLibraryFlag = 2; //finish;
				}
				break;
			case 1:
				g_message( "MediaLibraryRevision [ %s ]", data + _offset + 4 );
				
				//radiolibrary not need to updata;
				if ( _iAP2Object->radioLibraryFlag == 1 || strcmp("0", (gchar *)data + _offset + 4  ) == 0  )
				{
					printf("not need to update\n");
					if( _iAP2Object->radioLibraryRevision != NULL )
					{
						g_free( _iAP2Object->radioLibraryRevision );
						_iAP2Object->radioLibraryRevision	= NULL;
					}
					_iAP2Object->radioLibraryRevision =	g_strdup( data + _offset + 4 );
					_iAP2Object->radioLibraryFlag = 1;
					break;
				}
				
				if( _iAP2Object->libraryRevision != NULL )
				{
					g_free( _iAP2Object->libraryRevision );
					_iAP2Object->libraryRevision	= NULL;
				}

				_iAP2Object->libraryRevision	=	g_strdup( data + _offset + 4 );

				mh_contents_update_device_private( _contents, MH_DEV( _iAP2Object )->uniqueId, 
						_iAP2Object->libraryRevision, strlen( _iAP2Object->libraryRevision ) + 1 );
				break;
			case 2:
//				g_message("param type is: MediaItem");
				_music	=	parseMediaItem(_iAP2Object, data + _offset + 4, _paramLen - 4 ,FALSE);

				if( mh_contents_get_id_by_tagId( _contents, MH_DEV( _iAP2Object )->uniqueId, MH_ITEM( _music )->tagId ) == -1 )
				{
					_nodeId	=	mh_contents_add_node( _contents, MH_DEV( _iAP2Object )->uniqueId, MH_ITEM( _music ));

					MH_ITEM( _music )->uniqueId	=	_nodeId;
					_music->last_chgtime = 0;//compare by lastchangtime 
					mh_contents_add_music( _contents, _music,  MH_DEV( _iAP2Object )->uniqueId);//del db by deviceId

					g_hash_table_insert ( MH_DEV( _iAP2Object )->itemsHash, &( MH_ITEM( _music )->uniqueId ), MH_ITEM( _music));

					g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, 0);
				}else{
					mh_object_unref(( MHObject * )_music );
				}

				break;
			case 3:
//				g_message("param type is: MediaPlaylist\n");
				parseMediaPlaylist(_iAP2Object, data + _offset + 4, _paramLen - 4);
				break;
			case 4:
				g_message("param type is: MediaItemDeletePersistentIdentifier\n");
				_itemId	=	IAP2_MISC_READ_BIG64( data + _offset + 4 );

				_nodeId	=	mh_contents_get_id_by_tagId( _contents, MH_DEV( _iAP2Object )->uniqueId, _itemId );

				if( _nodeId > 0 )
				{
					_item	=	g_hash_table_lookup( MH_DEV( _iAP2Object)->itemsHash, &_nodeId);

					if( _item != NULL)
					{
						mh_contents_del_node( _contents, _item );
					}
					g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, 0);
				}
				break;
			case 5:
				g_message("param type is: MediaPlaylistDeletePersistentIdentifier\n");
				_itemId	=	IAP2_MISC_READ_BIG64( data + _offset + 4 );

				_nodeId	=	mh_contents_get_playlistid_by_tagId( _contents, _itemId);

				if( _nodeId > 0 )
				{
					mh_contents_delete_playlist( _contents, _nodeId );
				}

				g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, 0);
				break;
			case 6:
				/* We have dealt with MediaPlaylistDeletePersistentIdentifier before this loop */

				break;
			case 7:
				g_message( "updating library progress: %02d", *( data + _offset + 4 ));

				iAP2StopMediaLibraryUpdateTimeout(_iAP2Object);

				if( *( data + _offset + 4 ) != 100 )
					g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_UPDATE_FILE, MH_ITEM_NONE, 0, *( data + _offset + 4 ));

				if ( _iAP2Object->radioLibraryFlag == 1 )
				{
					g_message("not need to update progress");
					break;
				}

				if( *( data + _offset + 4 ) == 100 && !_iAP2Object->syncComplete )
				{
					struct timeval _endTime, _result;
					float _tmp;

					gettimeofday( &_endTime, NULL );

					timersub( &_endTime, &startTime, &_result );
					_tmp	=	_result.tv_sec + ( float )_result.tv_usec / 1000000;

					g_message( "Sync completed on: %03fs", _tmp );

					_iAP2Object->syncComplete	=	TRUE;

					/* Update current playing playlist */
					iAP2FileProcessCachedXfers(_iAP2Object);

					g_signal_emit_by_name( MH_DEV( _iAP2Object ), "dev_events", MH_DEV_FINISH, MH_ITEM_NONE, 0, 0);
				}

				break;
			case 8:
				g_message( "MediaLibraryIsHidingRemoteItems %s\n", 
						*( data + _offset + 5 ) ? "TRUE" : "FALSE" );
				break;
			case 9:
				g_message( "PlayAllSongsCapable %s\n", 
						*( data + _offset + 5 ) ? "TRUE" : "FALSE" );
				break;
			default:
				assert(0);
				break;
		}

		_offset	+=	_paramLen;
	}

	mh_contents_commit_transaction( _contents );
}		/* -----  end of function iAP2MediaParseMLUMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getMLUParam
 *  Description:  
 * =====================================================================================
 */
static void getMLUParam ( uint8_t ** mlParams, uint16_t * paramsLen )
{
	int i;
	uint8_t _tmpItemPro[]	=	{           /* Media Item Properties: 0 - 4 */
		0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x04, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03,
		0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x05, 0x00, 0x04, 0x00, 0x06, 0x00, 0x04, 0x00, 0x07,
		0x00, 0x04, 0x00, 0x08, 0x00, 0x04, 0x00, 0x09, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x04, 0x00, 0x0B,
		0x00, 0x04, 0x00, 0x0C, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x0F,
		0x00, 0x04, 0x00, 0x10, 0x00, 0x04, 0x00, 0x11, 0x00, 0x04, 0x00, 0x12, 0x00, 0x04, 0x00, 0x13,
		0x00, 0x04, 0x00, 0x19, 0x00, 0x04, 0x00, 0x1B
	};
	uint8_t _tmpListPro[]	=	{           /* Media Library Properties: 0 - 1 */
		0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x04, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03,
		0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x05, 0x00, 0x04, 0x00, 0x06
	};
	uint8_t * _params	=	NULL;
	int _offset	=	0;

	_params	=	(uint8_t *)g_malloc0( 4 + sizeof(_tmpItemPro) + 4 + sizeof(_tmpListPro) + 4 * 3 );

	_params[0]	=	IAP2_HI_BYTE(4 + sizeof(_tmpItemPro));
	_params[1]	=	IAP2_LO_BYTE(4 + sizeof(_tmpItemPro));
	_params[2]	=	0x00;                       /* Media Item Properties */
	_params[3]	=	0x02;

	_offset	+=	4;

	memcpy(_params + _offset, _tmpItemPro, sizeof(_tmpItemPro));

	_offset	+=	sizeof(_tmpItemPro);

	_params[_offset]		=	IAP2_HI_BYTE(4 + sizeof(_tmpListPro));
	_params[_offset + 1]	=	IAP2_LO_BYTE(4 + sizeof(_tmpListPro));
	_params[_offset + 2]	=	0x00; /* Media Playlist Properties */
	_params[_offset + 3]	=	0x03;

	_offset	+=	4;

	memcpy(_params + _offset, _tmpListPro, sizeof(_tmpListPro));

	_offset	+=	sizeof(_tmpListPro);

	_params[ _offset + 1 ]	=	0x04;
	_params[ _offset + 3 ]	=	0x04;

	_offset	+=	4;

	_params[ _offset + 1 ]	=	0x04;
	_params[ _offset + 3 ]	=	0x05;

	_offset	+=	4;

	_params[ _offset + 1 ]	=	0x04;
	_params[ _offset + 3 ]	=	0x06;

	_offset	+=	4;

	* mlParams	=	_params;
	* paramsLen	=	_offset;

}		/* -----  end of function getMLUParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaGetMLUMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaGetMLUMessage ( MHDevIap2 * _iAP2Object, uint8_t ** data, uint16_t * dataLen )
{
	uint8_t * _startMLUParam;
	uint16_t _startMLUParamLen, _revLen	=	0;
	uint8_t * _startMLUMessage;
	int _offset	=	0;

	getMLUParam(&_startMLUParam, &_startMLUParamLen );

	if( _iAP2Object->libraryRevision != NULL )
		_revLen	=	strlen( _iAP2Object->libraryRevision ) + 1;

	_startMLUMessage	=	(uint8_t *)malloc(4 + _iAP2Object->libraryIdLen + 4 + _revLen + _startMLUParamLen);

	_startMLUMessage[0]	=	IAP2_HI_BYTE(_iAP2Object->libraryIdLen + 4);
	_startMLUMessage[1]	=	IAP2_LO_BYTE(_iAP2Object->libraryIdLen + 4);
	_startMLUMessage[2]	=	0x00;
	_startMLUMessage[3]	=	0x00;

	_offset	+=	4;

	memcpy( _startMLUMessage + _offset, _iAP2Object->mediaLibraryId, _iAP2Object->libraryIdLen );

	_offset	+=	_iAP2Object->libraryIdLen;

	if( _iAP2Object->libraryRevision != NULL )
	{
		_startMLUMessage[ _offset + 0 ]	=	IAP2_HI_BYTE( _revLen + 4 );
		_startMLUMessage[ _offset + 1 ]	=	IAP2_LO_BYTE( _revLen + 4 );
		_startMLUMessage[ _offset + 2 ]	=	0x00;
		_startMLUMessage[ _offset + 3 ]	=	0x01;

		_offset	+=	4;

		memcpy( _startMLUMessage + _offset, _iAP2Object->libraryRevision, _revLen );

		_offset	+=	_revLen;
	}

	memcpy( _startMLUMessage + _offset, _startMLUParam, _startMLUParamLen );
	_offset	+=	_startMLUParamLen;

	g_free(_startMLUParam);

	* data		=	_startMLUMessage;
	* dataLen	=	_offset;
}		/* -----  end of function iAP2MediaGetMLUMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaGetMLURadioMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaGetMLURadioMessage ( MHDevIap2 * _iAP2Object, uint8_t ** data, uint16_t * dataLen )
{
	uint8_t * _startMLUParam;
	uint16_t _startMLUParamLen;
	uint16_t _revLen	=	0;
	uint8_t * _startMLUMessage;
	int _offset	=	0;

	getMLUParam(&_startMLUParam, &_startMLUParamLen );

	if( _iAP2Object->radioLibraryRevision != NULL )
		_revLen	=	strlen( _iAP2Object->radioLibraryRevision ) + 1;

	_startMLUMessage	=	(uint8_t *)malloc(4 + _iAP2Object->radioLibraryIdLen + 4 + _revLen + _startMLUParamLen);

	_startMLUMessage[0]	=	IAP2_HI_BYTE(_iAP2Object->radioLibraryIdLen + 4);
	_startMLUMessage[1]	=	IAP2_LO_BYTE(_iAP2Object->radioLibraryIdLen + 4);
	_startMLUMessage[2]	=	0x00;
	_startMLUMessage[3]	=	0x00;

	_offset	+=	4;

	memcpy( _startMLUMessage + _offset, _iAP2Object->radioLibraryId, _iAP2Object->radioLibraryIdLen );

	_offset	+=	_iAP2Object->radioLibraryIdLen;

	if( _iAP2Object->radioLibraryRevision != NULL )
	{
		_startMLUMessage[ _offset + 0 ]	=	IAP2_HI_BYTE( _revLen + 4 );
		_startMLUMessage[ _offset + 1 ]	=	IAP2_LO_BYTE( _revLen + 4 );
		_startMLUMessage[ _offset + 2 ]	=	0x00;
		_startMLUMessage[ _offset + 3 ]	=	0x01;

		_offset	+=	4;

		memcpy( _startMLUMessage + _offset, _iAP2Object->radioLibraryRevision, _revLen );

		_offset	+=	_revLen;
	}

	memcpy( _startMLUMessage + _offset, _startMLUParam, _startMLUParamLen );
	_offset	+=	_startMLUParamLen;

	g_free(_startMLUParam);

	* data		=	_startMLUMessage;
	* dataLen	=	_offset;
}		/* -----  end of function iAP2MediaGetMLUMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaGetNPUMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaGetNPUMessage (uint8_t ** data, uint16_t * dataLen)
{
	uint8_t * _startNPUMessage;
	uint8_t _itemAttribute[]		=	{       /* MediaItemAttributes */
		0x00, 0x04, 0x00, 0x00,                 /* MediaItemPersistentIdentifier */
		0x00, 0x04, 0x00, 0x01,                 /* MediaItemTitle */
		0x00, 0x04, 0x00, 0x04,                 /* MediaItemPlaybackDurationInMilliseconds */
		0x00, 0x04, 0x00, 0x06,                 /* MediaItemAlbumTitle */
		0x00, 0x04, 0x00, 0x07,                 /* MediaItemAlbumTrackNumber */
		0x00, 0x04, 0x00, 0x08,                 /* MediaItemAlbumTrackCount */
		0x00, 0x04, 0x00, 0x09,                 /* MediaItemAlbumDiscNumber */
		0x00, 0x04, 0x00, 0x0A,                 /* MediaItemAlbumDiscCount */
		0x00, 0x04, 0x00, 0x0C,                 /* MediaItemArtist */
		0x00, 0x04, 0x00, 0x10,                 /* MediaItemGenre */
		0x00, 0x04, 0x00, 0x12,                 /* MediaItemComposer */
//		0x00, 0x04, 0x00, 0x14,                 /* MediaItemArtworkFileTransferIdentifier */
		0x00, 0x04, 0x00, 0x15,                 /* MediaItemIsLikeSupported */
		0x00, 0x04, 0x00, 0x16,                 /* MediaItemIsBanSupported */
		0x00, 0x04, 0x00, 0x17,                 /* MediaItemIsLiked */
		0x00, 0x04, 0x00, 0x18,                 /* MediaItemIsBanned */
		0x00, 0x04, 0x00, 0x1A,                 /* MediaItemArtworkFileTransferIdentifier */
		0x00, 0x04, 0x00, 0x1B,                 /* MediaItemChapterCount */
	};
	uint8_t _playbackAttribute[]	=	{		/* PlaybackAttributes */
		0x00, 0x04, 0x00, 0x00,                 /* PlaybackStatus */
		0x00, 0x04, 0x00, 0x01,                 /* PlaybackElapsedTimeInMilliseconds */
		0x00, 0x04, 0x00, 0x02,                 /* PlaybackQueueIndex */
		0x00, 0x04, 0x00, 0x03,                 /* PlaybackQueueCount */
		0x00, 0x04, 0x00, 0x04,                 /* PlaybackQueueChapterIndex */
		0x00, 0x04, 0x00, 0x05,                 /* PlaybackShuffleMode */
		0x00, 0x04, 0x00, 0x06,                 /* PlaybackRepeatMode */
		0x00, 0x04, 0x00, 0x07,                 /* PlaybackAppName */
		0x00, 0x04, 0x00, 0x08,                 /* PlaybackMediaLibraryUniqueIdentifier */
		0x00, 0x04, 0x00, 0x09,                 /* PBiTunesRadioAd */
		0x00, 0x04, 0x00, 0x0A,                 /* PBiTunesRadioStationName */
		0x00, 0x04, 0x00, 0x0B,                 /* PBiTunesRadioStationMediaPlaylistPersistentID */
		0x00, 0x04, 0x00, 0x0C,                 /* PlaybackSpeed */
		0x00, 0x04, 0x00, 0x0D,                 /* SetElapsedTimeAvailable */
		0x00, 0x04, 0x00, 0x0E,                 /* PlaybackQueueListAvail */
		0x00, 0x04, 0x00, 0x0F,                 /* PlaybackQueueListTransferID */
		0x00, 0x04, 0x00, 0x10,                 /* PlaybackAppBundleID */
	};

	_startNPUMessage	=	(uint8_t *)g_malloc0(4 + sizeof(_itemAttribute) + 
			4 + sizeof(_playbackAttribute));

	_startNPUMessage[0]	=	IAP2_HI_BYTE(4 + sizeof(_itemAttribute));
	_startNPUMessage[1]	=	IAP2_LO_BYTE(4 + sizeof(_itemAttribute));
	_startNPUMessage[2]	=	0x00;
	_startNPUMessage[3]	=	0x00;

	memcpy(_startNPUMessage + 4, _itemAttribute, sizeof(_itemAttribute));

	_startNPUMessage[4 + sizeof(_itemAttribute)]	=	
		IAP2_HI_BYTE(4 + sizeof(_playbackAttribute));
	_startNPUMessage[4 + sizeof(_itemAttribute) + 1]	=	
		IAP2_LO_BYTE(4 + sizeof(_playbackAttribute));
	_startNPUMessage[4 + sizeof(_itemAttribute) + 2]	=	0x00;
	_startNPUMessage[4 + sizeof(_itemAttribute) + 3]	=	0x01;

	memcpy(_startNPUMessage + 4 + sizeof(_itemAttribute) + 4,
			_playbackAttribute, sizeof(_playbackAttribute));

	* data		=	_startNPUMessage;
	* dataLen	=	4 + sizeof(_itemAttribute) + 4 + sizeof(_playbackAttribute);
}		/* -----  end of function iAP2MediaGetNPUMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parsePlaybackAttribute
 *  Description:  
 * =====================================================================================
 */
static void parsePlaybackAttribute (MHDevIap2 * _iAP2Object, uint8_t * data, uint16_t dataLen)
{
	int _offset	=	0;
	uint16_t _paramLen, _paramId;
	MHPbInfoData *_info; 
	static const char * _pStatus[]	=	{
		"Stopped",
		"Playing",
		"Paused",
		"SeekForward",
		"SeekBackward"
	};

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
		case 0:                                 /* PlaybackStatus */
			printf("\tgroup param type is: PlaybackStatus [%d] [%s]\n", data[_offset + 4], _pStatus[data[_offset + 4]]);
			if( _iAP2Object->pb != NULL )
			{
				switch( data[_offset + 4] )
				{
					case 0:
						_iAP2Object->curStatus	=	READY_STATUS;
						_iAP2Object->curphoneStatus	=	READY_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( READY_STATUS ), NULL );	
						_iAP2Object->oldStatus	=	READY_STATUS;
						break;
					case 1:
//						_iAP2Object->oldStatus = PLAY_STATUS;
//						if( _iAP2Object->curStatus != SWITCHING_STATUS )
//							_iAP2Object->oldStatus	=	PLAY_STATUS;
						_iAP2Object->curphoneStatus	=	PLAY_STATUS;
						_iAP2Object->curStatus	=	PLAY_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( PLAY_STATUS ), NULL );
						_iAP2Object->oldStatus	=	PLAY_STATUS;
						break;
					case 2:
//						_iAP2Object->oldStatus = PAUSE_STATUS;
//						if( _iAP2Object->curStatus != SWITCHING_STATUS )
//							_iAP2Object->oldStatus	=	PAUSE_STATUS;
						_iAP2Object->curphoneStatus	=	PAUSE_STATUS;
						_iAP2Object->curStatus	=	PAUSE_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( PAUSE_STATUS ), NULL );
						_iAP2Object->oldStatus	=	PAUSE_STATUS;
						break;
					case 3:
					case 4:
						_iAP2Object->curphoneStatus	=	SEEKING_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( SEEKING_STATUS ), NULL );
						break;
					default:
						break;
				}
			}
			else{
				g_message("=================================== pb = NULL save status");
				switch( data[_offset + 4] )
				{
					case 0:
						_iAP2Object->curphoneStatus	=	READY_STATUS;
						break;
					case 1:
						_iAP2Object->curphoneStatus	=	PLAY_STATUS;
						break;
					case 2:
						_iAP2Object->curphoneStatus	=	PAUSE_STATUS;
						break;
					case 3:
					case 4:
						_iAP2Object->curphoneStatus	=	SEEKING_STATUS;
						break;
					default:
						break;
				}
				_iAP2Object->curStatusFlag	=	TRUE;
			}
			break;
		case 1:                                 /* PlaybackElapsedTimeInMilliseconds */
			printf("[%d]\r", MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]));
			fflush(stdout);
			_info = g_slice_new( MHPbInfoData );
			_info->ptime	=	MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);
			if( _iAP2Object->pb != NULL )
			{
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					if( _info->ptime < 500 )
					{
						if( ptime_s == FALSE )
						{
							ptime_s = TRUE;
							mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_PTIME_CHANGE, _info );
						}
					}else{
						if( ptime_s	== TRUE )
							ptime_s	=	FALSE;
						mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_PTIME_CHANGE, _info );
					}
				}else{
					g_message(" Ptime Invalid !");
				}
			}
			break;
		case 2:
			printf("\tgroup param type is: PlaybackQueueIndex[%d]\n", MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]));
			_info = g_slice_new( MHPbInfoData );
			_info->index	=	MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);

			_iAP2Object->currentQueueIndex	=	_info->index;
			if( _iAP2Object->pb != NULL )
			{
				if( _iAP2Object->oldStatus	!=	_iAP2Object->curStatus )
				{
					if( _iAP2Object->curStatus	!= SWITCHING_STATUS)
					{
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( _iAP2Object->curStatus ), NULL );
					}else{
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( _iAP2Object->oldStatus ), NULL );
					}
					
				}
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_QUEUE_INDEX, _info );
				}else{
					g_message("Queue Index Invalid !");
				}
			}
			break;
		case 3:
			printf("\tgroup param type is: PlaybackQueueCount[%d]\n", MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]));
			break;
		case 4:
			printf("\tgroup param type is: PlaybackQueueChapterIndex[%d]\n", MAKE_DWORD(
					data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]));
			break;
		case 5:
			printf("\tgroup param type is: PlaybackShuffleMode[%d]\n", data[_offset + 4]);
			switch( data[_offset + 4] )
			{
				case 0:
					shuffle_mode	=	MH_PB_SHUFFLE_OFF;
					break;
				case 1:
					shuffle_mode	=	MH_PB_SHUFFLE_ALL;
					break;
				case 2:
					shuffle_mode	=	MH_PB_SHUFFLE_ALBUMS;
					break;
				default:
					break;
			}
			if( _iAP2Object->pb != NULL )
			{
				_iAP2Object->pb->shuffle_mode	=	shuffle_mode;

				_info = g_slice_new( MHPbInfoData );
				_info->shuffle_mode	=	_iAP2Object->pb->shuffle_mode;
				
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_SHUFFLE_MODE, _info );
				}else{
					g_message(" Shuffle Mode Invalid !");
				}
			}else{
				_iAP2Object->shuffleFlag	=	TRUE;
			}
			break;
		case 6:
			printf("\tgroup param type is: PlaybackRepeatMode[%d]\n", data[_offset + 4]);
			switch( data[_offset + 4] )
			{
				case 0:
					repeat_mode	=	MH_PB_REPEAT_MODE_OFF;
					break;
				case 1:
					repeat_mode	=	MH_PB_REPEAT_MODE_ONE;
					break;
				case 2:
					repeat_mode	=	MH_PB_REPEAT_MODE_ALL;
					break;
				default:
					break;
			}
			if( _iAP2Object->pb != NULL )
			{
				_iAP2Object->pb->repeat_mode	=	repeat_mode;

				_info = g_slice_new( MHPbInfoData );
				_info->repeat_mode	=	_iAP2Object->pb->repeat_mode;
				
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_REPEAT_MODE, _info );
				}else{
					g_message(" Repeat Mode Invalid !");
				}
			}else{
				_iAP2Object->repeatFlag	=	TRUE;
			}
			break;
		case 7:
			printf("\tgroup param type is: PlaybackAppName[%s]\n", (gchar *)data + _offset + 4);
			break;
		case 8:
			printf( "\tgroup param type is: PBMediaLibraryUniqueIdentifier[ %s ]\n", (gchar *)data + _offset + 4 );
			break;
		case 9:
			printf( "\tgroup param type is: PBiTunesRadioAd[ %s ]\n", data[ _offset + 4 ] ? "TRUE" : "FALSE" );
			break;
		case 10:
			printf( "\tgroup param type is: PBiTunesRadioStationName[ %s ]\n", (gchar *)data + _offset + 4 );
			break;
		case 11:
			printf( "\tgroup param type is: PBiTunesRadioStationMediaPlaylistPersistentID\n" );
			break;
		case 12:
			printf( "\tgroup param type is: PlaybackSpeed[ %d ]\n", MAKE_WORD( data[ _offset + 4 ], data[ _offset + 5 ] ));
			_iAP2Object->speed = MAKE_WORD( data[ _offset + 4 ], data[ _offset + 5 ] );
			if( _iAP2Object->pb != NULL )
			{
				_info = g_slice_new( MHPbInfoData );
				_info->speed	=	_iAP2Object->speed;
				
				if( _iAP2Object->screenMode	!=	IAP2_DATA_INVALID )
				{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_PLAYBACK_SPEED, _info );
				}else{
					g_message(" Speed Mode Invalid !");
				}
			}else{
				_iAP2Object->speedFlag	=	TRUE;
			}
			break;
		case 13:
			printf( "\tgroup param type is: SetElapsedTimeAvailable[ %s ]\n", data[ _offset + 4 ] ? "TRUE" : "FALSE" );
			if( data[ _offset + 4 ] != 0 )
			{
				_iAP2Object->elapsedTimeAvailable	=	TRUE;	
			}else{
				_iAP2Object->elapsedTimeAvailable	=	FALSE;
			}
			break;
		case 14:
			printf( "\tgroup param type is: PlaybackQueueListAvail[ %s ]\n", data[ _offset + 4 ] ? "TRUE" : "FALSE" );
			break;
		case 15:
			printf( "\tgroup param type is: PlaybackQueueListTransferID[ %d ]\n", data[ _offset + 4 ] );
			iAP2FileSetTransferType(_iAP2Object, data[_offset + 4], 0, IAP2_LIST, NULL );
			break;
		case 16:
			printf( "\tgroup param type is: PlaybackAppBundleID[ %s ]\n", (gchar *)data + _offset + 4 );
			if( _iAP2Object->pb != NULL )
			{
				_info = g_slice_new( MHPbInfoData );
				_info->app_name	=	g_strdup((gchar *)data + _offset + 4);
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_APP_NAME, _info );
			}else{
				g_message("pb is NULL app name is not emit");
				_iAP2Object->app_name	=	g_strdup((gchar *)data + _offset + 4);
				_iAP2Object->appNameFlag	=	TRUE;
			}
			break;
		default:
			g_message( "Uncatched playback attribute: 0x%04X", _paramId );
			g_assert_not_reached();
			break;
		}

		_offset	+=	_paramLen;
	}
}		/* -----  end of static function parsePlaybackAttribute  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaParseNPUMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaParseNPUMessage (MHDevIap2 * _iAP2Object,  uint8_t * data, uint16_t dataLen)
{
	int _offset	=	0;
	uint16_t _paramLen;
	uint16_t _paramId;
	MHMusic * _music	=	NULL;
	
	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId		=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
		case 0:                                 /* MediaItemAttributes */
			_music	=	parseMediaItem(_iAP2Object, data + _offset + 4, _paramLen - 4 ,TRUE);

			if( _iAP2Object->pb != NULL ) 
			{
				if( infoData.media_info.title	==	NULL )
				{
					infoData.media_info.title	=	g_strdup( "" );
				}
				if( infoData.media_info.album_title	==	NULL )
				{
					infoData.media_info.album_title	=	g_strdup( "" );	
				}
				if(  infoData.media_info.artist	==	NULL )
				{
					infoData.media_info.artist	=	g_strdup( "" );	
				}
				if(  infoData.media_info.album_artist	==	NULL )
				{
					infoData.media_info.album_artist	=	g_strdup( "" );	
				}
				if(  infoData.media_info.genre	==	NULL )
				{
					infoData.media_info.genre	=	g_strdup( "" );	
				}
				if(  infoData.media_info.composer	==	NULL )
				{
					infoData.media_info.composer	=	g_strdup( "" );	
				}

				if(( g_strcmp0( infoData.media_info.title, "") == 0)  
						&& ( g_strcmp0( infoData.media_info.album_title, "") == 0)
						&& ( infoData.media_info.duration == 0 )
						&& ( _iAP2Object->screenMode	!= IAP2_DATA_FRIST_NOTIFY ))
				{
					g_message("  Media Info Invalid !");
					_iAP2Object->screenMode	= IAP2_DATA_INVALID;
				}else{
					mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_MEDIA, &infoData );
					_iAP2Object->screenMode	= IAP2_DATA_VALID;
				}

			}else{
				 _iAP2Object->mediaInfoFlag	=	TRUE;
				 g_message("%s pb is NULL \n",__func__);
			}

			mh_object_unref(( MHObject * )_music );
			break;
		case 1:                                 /* PlaybackAttributes */
			parsePlaybackAttribute(_iAP2Object, data + _offset + 4, _paramLen - 4);

			break;
		default:
			//printf("unknown Now Playing Update parameter[%d]\n", _paramId);
			assert(0);
			break;
		}

		_offset	+=	_paramLen;
	}
}		/* -----  end of function iAP2MediaParseNPUMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaParseDeviceNameMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaParseDeviceNameMessage(MHDevIap2 * _iAP2Object, uint8_t * data)
{
	if( _iAP2Object->pb != NULL ) 
	{
		MHPbInfoData *_info; 
		_info = g_slice_new( MHPbInfoData );
		_info->device_name	=	g_strdup((gchar *)data);
		mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_DEVICE_NAME, _info );
	}else{
		_iAP2Object->device_name	=	g_strdup((gchar *)data);
		_iAP2Object->deviceNameFlag	=	TRUE;
	}
}		/* -----  end of function iAP2MediaParseDeviceNameMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2MediaResendMessage
 *  Description:  
 * =====================================================================================
 */
void iAP2MediaResendMessage (MHDevIap2 * _iAP2Object,  iAP2ResendMode flag )
{
	switch(flag)
	{
		case RS_TAG:                                			
			if( _iAP2Object->pb != NULL ) 
			{
				if( infoData.media_info.title	==	NULL )
				{
					infoData.media_info.title	=	g_strdup( "" );
				}
				if( infoData.media_info.album_title	==	NULL )
				{
					infoData.media_info.album_title	=	g_strdup( "" );	
				}
				if(  infoData.media_info.artist	==	NULL )
				{
					infoData.media_info.artist	=	g_strdup( "" );	
				}
				if(  infoData.media_info.album_artist	==	NULL )
				{
					infoData.media_info.album_artist	=	g_strdup( "" );	
				}
				if(  infoData.media_info.genre	==	NULL )
				{
					infoData.media_info.genre	=	g_strdup( "" );	
				}
				if(  infoData.media_info.composer	==	NULL )
				{
					infoData.media_info.composer	=	g_strdup( "" );	
				}
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_MEDIA, &infoData );
			}		
			break;
		case RS_DEV_NAME:                                 /* device name */
			if( _iAP2Object->pb != NULL ) 
			{
				MHPbInfoData *_info; 
				_info = g_slice_new( MHPbInfoData );
				_info->device_name	=	g_strdup( _iAP2Object->device_name );
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_DEVICE_NAME, _info );
			}
			break;
		case RS_REPEAT:                               		
			if( _iAP2Object->pb != NULL ) 
			{
				MHPbInfoData *_info; 
				_info = g_slice_new( MHPbInfoData );
				_info->repeat_mode	=	repeat_mode;
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_REPEAT_MODE, _info );
			}
			break;
		case RS_SHUFFLE:                                 
			if( _iAP2Object->pb != NULL ) 
			{
				MHPbInfoData *_info; 
				_info = g_slice_new( MHPbInfoData );
				_info->shuffle_mode	=	shuffle_mode;
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_SHUFFLE_MODE, _info );
			}
			break;
		case RS_APP_NAME:                                 
			if( _iAP2Object->pb != NULL ) 
			{
				MHPbInfoData *_info; 
				_info = g_slice_new( MHPbInfoData );
				_info->app_name	=	g_strdup( _iAP2Object->app_name );
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_APP_NAME, _info );
			}else{

			}
			break;
		case RS_CUR_STATUS:                                 
			g_message("=================================== attch_pb _iAP2Object->curphoneStatus = %d",_iAP2Object->curphoneStatus);

			if( _iAP2Object->pb != NULL ) 
			{
				switch( _iAP2Object->curphoneStatus )
				{
					case READY_STATUS:
						_iAP2Object->curStatus	=	READY_STATUS;
						_iAP2Object->curphoneStatus	=	READY_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( READY_STATUS ), NULL );	
						_iAP2Object->oldStatus	=	READY_STATUS;
						break;
					case PLAY_STATUS:
						_iAP2Object->curphoneStatus	=	PLAY_STATUS;
						_iAP2Object->curStatus	=	PLAY_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( PLAY_STATUS ), NULL );
						_iAP2Object->oldStatus	=	PLAY_STATUS;
						break;
					case PAUSE_STATUS:
						_iAP2Object->curphoneStatus	=	PAUSE_STATUS;
						_iAP2Object->curStatus	=	PAUSE_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( PAUSE_STATUS ), NULL );
						_iAP2Object->oldStatus	=	PAUSE_STATUS;
						break;
					case SEEKING_STATUS:
						_iAP2Object->curphoneStatus	=	SEEKING_STATUS;
						mh_pb_dispatch( _iAP2Object->pb, EVENT_CHANGE, GUINT_TO_POINTER( SEEKING_STATUS ), NULL );
						break;
					default:
						break;
				}
			}
			break;
		case RS_PLAYBACK_SPEED:                                 
			if( _iAP2Object->pb != NULL ) 
			{
				MHPbInfoData *_info; 
				_info = g_slice_new( MHPbInfoData );
				_info->speed	=	_iAP2Object->speed;
				mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_PLAYBACK_SPEED, _info );
			}else{

			}
			break;
		default:
			break;
	}
}		/* -----  end of function iAP2MediaResendMessage  ----- */
