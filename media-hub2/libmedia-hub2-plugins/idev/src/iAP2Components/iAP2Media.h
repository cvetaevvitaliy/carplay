/*
 * =====================================================================================
 *
 *       Filename:  iAP2Media.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 11:53:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_MEDIA_H__
#define __IAP2_MEDIA_H__

#include <iAP2Link.h>
#include <dev_iap2.h>
#include <glib.h>

typedef enum _iAP2MediaType_
{
	MT_MUSIC,
	MT_PODCAST,
	MT_AUDIOBOOK,
	MT_ITUNESU
} iAP2MediaType;				/* ----------  end of enum iAP2MediaType  ---------- */

typedef enum _iAP2PlaybackStatus_
{
	PS_STOPPED,
	PS_PLAYING,
	PS_PAUSED,
	PS_SEEKFORWARD,
	PS_SEEKBACKWARD
} iAP2PlaybackStatus;				/* ----------  end of enum iAP2PlaybackStatus  ---------- */

typedef enum _iAP2ShuffleMode_
{
	SH_OFF,
	SH_SONGS,
	SH_ALBUMS
} iAP2ShuffleMode;				/* ----------  end of enum iAP2ShuffleMode  ---------- */

typedef enum _iAP2RepeatMode_
{
	RP_OFF,
	RP_ONE,
	RP_ALL
} iAP2RepeatMode;				/* ----------  end of enum iAP2RepeatMode  ---------- */

typedef enum _iAP2ResendMode_
{
	RS_TAG,
	RS_DEV_NAME,
	RS_REPEAT,
	RS_SHUFFLE,
	RS_APP_NAME,
	RS_CUR_STATUS,
	RS_PLAYBACK_SPEED
} iAP2ResendMode;				/* ----------  end of enum iAP2ResendMode  ---------- */

typedef struct _iAP2MediaItem_ 
{
	uint64_t itemId;
	char * title;
	iAP2MediaType mediaType;
	uint8_t rating;
	uint32_t duration;
	uint64_t albumId;
	char * albumTitle;
	uint16_t albumTrackNo;
	uint16_t albumTrackCount;
	uint16_t albumDiscNo;
	uint16_t albumDiscCount;
	uint64_t artistId;
	char * artist;
	uint64_t albumArtistId;
	char * albumArtist;
	uint64_t genreId;
	char * genre;
	uint64_t composerId;
	char * composer;
	uint8_t isPartOfCompilation;
	uint8_t artworkTransferId;
	uint8_t isLikeSupported;
	uint8_t isBanSupported;
	uint8_t isLiked;
	uint8_t isBanned;
	uint8_t isResidentOnDevice;
} iAP2MediaItem;				/* ----------  end of struct iAP2MediaItem  ---------- */

typedef struct _iAP2PlaybackAttr_ 
{
	iAP2PlaybackStatus status;
	uint32_t ptime;
	uint32_t queueIndex;
	uint32_t queueCount;
	uint32_t queueChapterIndex;
	iAP2ShuffleMode shuffleMode;
	iAP2RepeatMode repeatMode;
	char * appName;
} iAP2PlaybackAttr;				/* ----------  end of struct iAP2PlaybackAttr  ---------- */

typedef struct _iAP2PlayListHeader_ 
{
	uint64_t listId;
	gchar * listName;
	uint64_t parentId;
	gboolean isFolder;
	GSList * playlist;
} iAP2PlayListHeader;				/* ----------  end of struct iAP2PlayListHeader  ---------- */

typedef struct _iAP2RadioPlayListInfo_ 
{
	uint64_t listId;
	gchar * listName;
} iAP2RadioPlayListInfo;				/* ----------  end of struct iAP2RadioPlayListInfo  ---------- */

void iAP2MediaParseMLUMessage ( MHDevIap2 * _iAP2Object, unsigned char * data, int dataLen );
void iAP2MediaParseLibraryInformation (MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen );
void iAP2MediaGetMLUMessage (MHDevIap2 * _iAP2Object, uint8_t ** data, uint16_t * dataLen );
void iAP2MediaGetMLURadioMessage ( MHDevIap2 * _iAP2Object, uint8_t ** data, uint16_t * dataLen );
void iAP2MediaGetNPUMessage (uint8_t ** data, uint16_t * dataLen);
void iAP2MediaParseNPUMessage (MHDevIap2 * _iAP2Object, uint8_t * data, uint16_t dataLen);
iAP2PlayListHeader * iAP2MediaGetPlaylistHeaderById ( MHDevIap2 * _iAP2Object, uint64_t listId );
void iAP2MediaResendMessage (MHDevIap2 * _iAP2Object,  iAP2ResendMode flag );
void iAP2MediaParseDeviceNameMessage(MHDevIap2 * _iAP2Object, uint8_t * data );
void iAP2FreeRadioDate();
#endif
