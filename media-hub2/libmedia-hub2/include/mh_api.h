/*
 * =====================================================================================
 *
 *       Filename:  mh-api.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/12/2014 10:03:38 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __MH_API_H__
#define __MH_API_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#define MH_DEV_UUID_LENGTH	37                  /* 37 characters - 32 hex digits + 4 dashes + 1 zero terminal */
#define MH_STRING_SHORT		8
#define MH_STRING_MIDDLE	32
#define MH_STRING_LONG		256
#define MH_STRING_PATH		1024	

typedef struct _MHObject		MHObject;
typedef struct _MHCore			MHCore;
typedef struct _MHDev			MHDev;
typedef struct _MHCol			MHCol;
typedef struct _MHItem			MHItem;
typedef struct _MHFolder		MHFolder;
typedef struct _MHPlaylist		MHPlaylist;
typedef struct _MHFilter		MHFilter;
typedef struct _MHPb		    MHPb;
typedef struct _MHPlayer		MHPlayer;
typedef struct _MHContents      MHContents;

typedef struct _MHIPCServer		MHIPCServer;
typedef struct _MHIPCConnection	MHIPCConnection;

typedef enum _MHMiscFilterType 
{
	MH_MISC_FILTER_MUSIC = 0,
	MH_MISC_FILTER_MOVIE,
	MH_MISC_FILTER_PICTURE,
	MH_MISC_FILTER_PLAYLIST
} MHMiscFilterType;				/* ----------  end of enum MHMiscFilterType  ---------- */


typedef enum _MHResult
{
	MH_OK	=	0,
	MH_INVALID_PARAM,
	MH_INVALID_SEQ,
	MH_FUNC_NOT_IMPLEMENT,
	MH_IPC_ERROR,
	MH_MAP_FAILED,
	MH_SHM_OPEN_FAILED,
	MH_RESULT_NUM
} MHResult;

typedef enum _MHItemType
{
	MH_ITEM_FOLDER	=	0,
	MH_ITEM_MUSIC,
	MH_ITEM_MOVIE,
	MH_ITEM_PICTURE,
	MH_ITEM_PLAYLIST,
	MH_ITEM_NONE
} MHItemType;				/* ----------  end of enum MHItemType  ---------- */

/* Sub type of MH_ITEM_MUSIC */
typedef enum _MHMediaType
{
	MH_MEDIA_MUSIC	=	0,
	MH_MEDIA_PODCAST,
	MH_MEDIA_AUDIOBOOK,
	MH_MEDIA_ITUNESU,
	
	MH_MEDIA_NONE
} MHMediaType;				/* ----------  end of enum MHMediaType  ---------- */

typedef enum _MHItemOrderType
{
	MH_ITEM_ORDER_BY_ALPHABET,
	MH_ITEM_ORDER_BY_PINYIN,
	MH_ITEM_ORDER_BY_TRACKID,
	MH_ITEM_ORDER_BY_ALPHABET_FOR_NAGIVI,
	MH_ITEM_ORDER_BY_ALPHABET_FOR_NAGIVI_JP,

	MH_ITEM_ORDER_BY_DEFAULT
} MHItemOrderType;				/* ----------  end of enum MHItemOrderType  ---------- */

/* Methods List */
/* Core APIs */
const char * mh_built_date();
const char * mh_built_time();

typedef enum _MHCoreEvent
{
	MH_CORE_STARTED,
	MH_CORE_STOPED,
	MH_CORE_PLUGIN_NOT_FOUND,
	MH_CORE_PLUGIN_INVALID,
	MH_CORE_PLUGIN_LOAD_SUCCESS,
	MH_CORE_PLUGIN_LOAD_FAILED,

	MH_CORE_EVENTS_NUM
} MHCoreEvent;				/* ----------  end of enum MHCoreEvent  ---------- */

typedef enum _MHDevEvents
{
	MH_DEV_ATTACHED,
	MH_DEV_DETACHED,

	MH_DEV_EVENTS_NUM
} MHDevEvents;				/* ----------  end of enum MHDevEvents  ---------- */

typedef void ( * events_cb )( MHCore * core, MHCoreEvent event,const char * type, void * user_data );

typedef void ( * dev_cb )( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data );

typedef enum _MHColFilterType
{
	COL_FILTER_NONE	= 1,
	COL_FILTER_TITLE,
	COL_FILTER_ARTIST,
	COL_FILTER_COMPOSER,
	COL_FILTER_ALBUM,
	COL_FILTER_TRACK,
	COL_FILTER_GENRE,
	COL_FILTER_YEAR,
	COL_FILTER_COMPLIATION,
	COL_FILTER_ALBUM_ARTIST,
	COL_FILTER_NAME,
	COL_FILTER_DURATION,

	COL_FILTER_MAX

} MHColFilterType;				/* ----------  end of enum MHColFilterType  ---------- */

//typedef enum _MHColType
//{
//	COL_MUSIC		=	1,
//	COL_MOVIE		=	1 << 1,
//	COL_PICTURE		=	1 << 2,
//	COL_PODCAST		=	1 << 3,
//	COL_AUDIOBOOK	=	1 << 4,
//	COL_ITUNESU		=	1 << 5,
//	COL_MAX
//} MHColType;				/* ----------  end of enum MHColType  ---------- */
		
typedef enum _MHFolderPosition
{
	FOLDER_BEGIN,
	FOLDER_CUR,

	FOLDER_NUM
} MHFolderPosition;				/* ----------  end of enum MHFolderPosition  ---------- */

typedef struct _MHEventsListener 
{
	events_cb callback;
	void * user_data;
} MHEventsListener;				/* ----------  end of struct MHEventsListener  ---------- */
	
typedef struct _MHDevicesListener 
{
	dev_cb callback;
	void * user_data;
} MHDevicesListener;				/* ----------  end of struct MHDevicesListener  ---------- */


typedef enum _MHDevSearchType 
{
	DEPTH_FIRST,
	BREADTH_FIRST
} MHDevSearchType;				/* ----------  end of enum MHDevSearchType  ---------- */

typedef struct _MHPlaylistInfo 
{
	int64_t playlistid;
	char * name;
} MHPlaylistInfo;				/* ----------  end of struct MHPlaylistInfo  ---------- */

typedef enum _MHItemValid
{
	MH_ITEM_VALID	=	0,
	MH_ITEM_INVALID,
	MH_ITEM_UNKNOWN,
	MH_ITEM_NOT_EXIST

} MHItemValid;				/* ----------  end of enum MHItemEnable  ---------- */


typedef struct _MHItemData 
{
	char * uri;
	MHItemType type;
	char * name;
	int64_t size;
	int64_t uniqueId;
	MHItemValid valid;
	int64_t tagId;
	bool favorite;
	union
	{
		struct
		{
			char * title;
			char * album;
			char * artist;
			char * genre;
			char * composer;
			int32_t year;
			int32_t track;
			int32_t trackCount;
			MHMediaType	mediaType;
			int32_t duration;
		} music;
	} metadata;
} MHItemData;				/* ----------  end of struct MHItemData  ---------- */


typedef enum _MHSortType
{
	MH_SORT_NONE,
	
	MH_SORT_NAME,
	MH_SORT_TITLE,
	MH_SORT_ALBUM,
	MH_SORT_ARTIST,
	MH_SORT_TRACKID
} MHSortType;				/* ----------  end of enum MHSortType  ---------- */

typedef enum{
	MH_PB_REPEAT_MODE_OFF =	1	,/* *< repeat off  ,default value*/
	MH_PB_REPEAT_MODE_ONE 		,/* *< repeat one 				*/
	MH_PB_REPEAT_MODE_ALL 		, /* *< current category repeat	*/
} MHPbRepeatMode;

typedef enum _MHPbShuffleMode 
{
	MH_PB_SHUFFLE_OFF,
	MH_PB_SHUFFLE_ALL,
	MH_PB_SHUFFLE_ALBUMS,
} MHPbShuffleMode;				/* ----------  end of enum MHPbShuffleMode  ---------- */

typedef enum _MHPbAudioBookSpeed 
{
	MH_PB_SPEED_RESET,
	MH_PB_SPEED_INCREASE,
	MH_PB_SPEED_DECREASE,
} MHPbAudioBookSpeed;

typedef struct _MHPbRecentListInfo 
{
	uint32_t index;
	char * remoteID;
	char * displayName;
	char * label;
	char * addressBookID;
	uint32_t service;
	uint32_t type; 
	uint64_t unixTimestamp;
	uint32_t duration;
	uint32_t occurrences;
}MHPbRecentListInfo;

typedef struct _MHPbFavoritesListInfo 
{
	uint32_t index;
	char * remoteID;
	char * displayName;
	char * label;
	char * addressBookID;
	uint32_t service;
}MHPbFavoritesListInfo;

typedef union {                                                                    
	struct {
		uint32_t current_time;                                                              
		uint32_t duration;
	} time_info;
	struct { 
		char * title;
		char * artist;
		char * album;
		char * genre;
	} tag_info;  
	struct {
		uint32_t index;							/*  *<the item index allocated in the list. */
		char * uri;		/*  *<The handle string for a file.*/
		char * name;
	} track_info;
	MHPlaylist * playlist;
	uint32_t ptime;
	uint32_t index;
	uint32_t speed;
	struct {
		int64_t tagId;
		char * title;
		uint32_t rating;
		uint32_t duration;
		char * album_title;
		uint32_t track;
		uint32_t track_count;
		uint32_t disc;
		uint32_t disc_count;
		char * artist;
		char * album_artist;
		char * genre;
		char * composer;
	} media_info;
	MHPbRepeatMode repeat_mode;
	MHPbShuffleMode shuffle_mode;
	char * cover_path;
	char * app_name;
	char * device_name;

	struct {
		uint32_t list_count;
		uint32_t * shuffle_seq;
	} sf_list_info;

	struct {
		char * remoteID;
		char * displayName;
		uint32_t status;
		uint32_t direction;
		char * callUUID;
		char * addressBookID;
		char * label;
		uint32_t service;
		bool isConferenced;
		char * conferenceGroup;
		uint32_t disconnectReason;
	}call_state_info;

	struct {
		bool recentsListAvailable;
		MHPbRecentListInfo * recentList;
		uint32_t recentsListCount;
	}recentslist_updates;

	struct {
		bool favoritesListAvailable;
		MHPbFavoritesListInfo * favoritesList;
		uint32_t favoritesListCount;
	}favoriteslist_updates;
	struct {
		uint32_t band;
		double * bands;
		double * amplitudes;
	} frequency_analysis_result;
}MHPbInfoData; 

typedef struct _MHPbTrackInfo 
{
	uint32_t total_count;
	int32_t current_count;
	char ** track_name;
} MHPbTrackInfo;	

typedef struct _MHPbSubtitleInfo 
{
	uint32_t total_count;
	int32_t current_count;
	char ** subtitle_name;
} MHPbSubtitleInfo;	

typedef enum{
	MH_PB_INFO_PTIME_CHANGE,  		  /* !< playing time change 	*/	
	MH_PB_INFO_TAG,					  /* !< tag info 	*/
	MH_PB_INFO_EOS,					  /* !< end of track 	*/
	MH_PB_INFO_ERROR,				  /* !< playing error  	*/
	MH_PB_INFO_TRACK_TOP, 			  /* !< top of track 	*/ 
	MH_PB_INFO_PLAYLIST_CHANGE,
	MH_PB_INFO_ERROR_NOT_EXIST,

	MH_PB_IP_INFO_PTIME_CHANGE,
	MH_PB_IP_INFO_QUEUE_INDEX,
	MH_PB_IP_INFO_MEDIA,
	MH_PB_IP_INFO_REPEAT_MODE,
	MH_PB_IP_INFO_SHUFFLE_MODE,
	MH_PB_IP_INFO_COVER_PATH,
	MH_PB_IP_INFO_APP_NAME,
	MH_PB_IP_INFO_DEVICE_NAME,
	MH_PB_IP_INFO_FUNC_UNSUPPORT,
	
	MH_PB_INFO_STATE_ERROR,
	MH_PB_IP_INFO_SHUFFLE_LIST,
	MH_PB_IP_INFO_CALL_STATE_UPDATE,
	MH_PB_IP_INFO_RECENTS_LIST_UPDATES,
	MH_PB_IP_INFO_FAVORITES_LIST_UPDATES,
	MH_PB_IP_INFO_PLAYBACK_SPEED,
	MH_PB_FREQUENCY_ANALYSIS_RESULT
}MHPbInfoType;

typedef enum {
	MH_PB_STATE_READY,
	MH_PB_STATE_PLAYING,
	MH_PB_STATE_PAUSE,
	MH_PB_STATE_SWITCHING,
	MH_PB_STATE_SEEKING,
	MH_PB_STATE_ERROR,
}MHPbStatusType;
typedef void ( * MHPbUserCallback )( MHPb * pb, MHPbInfoType type, MHPbInfoData *data, void * user_data ); 
typedef struct _MHPbEventsListener 
{
	MHPbUserCallback callback;
	void * user_data;
} MHPbEventsListener;			/* ----------  end of struct MHPbEventsListener  ---------- */
typedef void ( * MHPbStatusCallback )( MHPb * pb, MHPbStatusType type, void * user_data ); 
typedef struct _MHPbStatusListener 
{
	MHPbStatusCallback callback;
	void * user_data;
} MHPbStatusListener;			/* ----------  end of struct MHPbStateListener  ---------- */

typedef enum
{
	SCAN_FOLDER,
	SCAN_TAG
} MHDevScanType;				/* ----------  end of struct MHDevScanType  ---------- */

typedef enum _MHDevScanCbType
{
	MH_DEV_FIRST_FILE,
	MH_DEV_UPDATE_FILE,
	MH_DEV_FINISH
} MHDevScanCbType;				/* ----------  end of enum MHDevScanCbType  ---------- */

typedef void (* dev_events_cb)(MHDev * self, MHDevScanCbType scan_type, MHItemType item_type, void * data, uint32_t percent, void * user_data);
typedef struct _MHDevEventsListener 
{
	dev_events_cb callback;
	void * user_data;
} MHDevEventsListener;			/* ----------  end of struct MHDevEventsListener  ---------- */

typedef void (* dev_detach_cb )(MHDev * dev, void * user_data);
typedef struct _MHDevDetachListener 
{
	dev_detach_cb callback;
	void * user_data;
} MHDevDetachListener;				/* ----------  end of struct MHDevDetachListener  ---------- */

typedef bool ( * MHFunc )( void * data, void * user_data );
typedef void (* MHDevScanCallback )(MHDev * self,MHItemType item_type, MHDevScanCbType scan_type,void * data,void * user_data);
/* For string type property, the returned pointer must be freed after used */
typedef void ( *MHFirstFileFunc )(MHItemType type, MHItem * item, void * user_data);
typedef bool ( * MHItemFunc )( MHItem * item, MHItemData * data, void * user_data );
/*  misc APIs */
MHResult mh_misc_set_filter(MHMiscFilterType type, const char * filter, ... );
MHResult mh_misc_reset();
MHResult mh_misc_db_restore();
MHResult mh_misc_db_delete_by_serial_number(const char * serialNum);//del db by deviceId
typedef enum _MiscIapDeviceMode
{
	MISC_IAP_UNKNOWN	=	0,

	MISC_IAP,
	MISC_IAP_CARPLAY,
	MISC_IAP_CARLIFE,
	MISC_IAP_CARPLAY_CARLIFE
} MiscIapDeviceMode;
MHResult mh_misc_set_iap_device_mode( MiscIapDeviceMode mode );
MHResult mh_misc_set_bluetoothids( const char * ids );
MHResult mh_misc_set_righthand( uint32_t status );
MHResult mh_misc_save_pb( MHPb * pb );
MHResult mh_misc_carplay_init_modes( const uint8_t * modes );
/*  core  APIs*/
MHResult mh_core_register_events_listener( MHEventsListener * listener );
MHResult mh_core_register_devices_listener( MHDevicesListener * listener );

MHResult mh_core_start();
MHResult mh_core_stop();

typedef enum _MHDeviceType
{
	MH_DEV_STORAGE,
	MH_DEV_BT_IAP,
	MH_DEV_WIFI_IAP,
	MH_DEV_USB_IAP,
	MH_DEV_USB_CARPLAY,
	MH_DEV_WIFI_CARPLAY
} MHDeviceType;

typedef struct _MHDevParam 
{
	MHDeviceType type;
	char * mac_addr;	
	int32_t connect_status;/* 0-disconnected 1-connect */
} MHDevParam;	
MHResult mh_core_find_dev( MHDevParam * devParam );
/*  playlist APIs */
MHResult mh_playlist_sort( MHPlaylist * self, MHSortType sort_type, MHItemOrderType order_type);
MHResult mh_playlist_foreach( MHPlaylist * self, int32_t start, int32_t count, MHFunc func, void * user_data );
MHResult mh_playlist_append( MHPlaylist * self, MHItem ** items, uint32_t count );
MHResult mh_playlist_append_playlist( MHPlaylist * self,  MHPlaylist * _another);
MHResult mh_playlist_insert( MHPlaylist * self, uint32_t index, MHItem ** items, uint32_t count );
MHResult mh_playlist_remove( MHPlaylist * self, uint32_t index, uint32_t count );

/* Playback APIs */
MHResult mh_pb_play_items( uint64_t * items, uint32_t index );
MHResult mh_pb_stop( MHPb * self );
MHResult mh_pb_close( MHPb * self );
MHResult mh_pb_play( MHPb * self );
MHResult mh_pb_pause( MHPb * self );
MHResult mh_pb_play_pause(MHPb * self);
MHResult mh_pb_resume();
MHResult mh_pb_set_position( uint32_t second );
MHResult mh_pb_forward( MHPb * self );
MHResult mh_pb_forward_done( MHPb * self );
MHResult mh_pb_backward( MHPb * self );
MHResult mh_pb_backward_done( MHPb * self );
MHResult mh_pb_audiobook_playback_speed( MHPb * self, uint32_t speed );
//void mh_pb_set_index( MHPb * self, uint32_t index );
MHResult mh_pb_set_speed( uint32_t speed );
//void mh_pb_set_repeat( MHPb * self, MHPbRepeatMode repeat_mode );
MHResult mh_pb_set_shuffle();
MHResult mh_pb_play_by_list( MHPb * self, MHPlaylist * playlist, uint32_t index );
MHResult mh_pb_play_radio_by_index( MHDev * self, MHPb * pb, uint32_t index );
MHResult mh_pb_next( MHPb * self );
MHResult mh_pb_previous( MHPb * self );
MHResult mh_pb_seek( MHPb * self, uint32_t second );
MHResult mh_pb_playlist_change( MHPb * self, MHPlaylist * playlist );
MHResult mh_pb_resize( MHPb * self, uint32_t offsetx, uint32_t offsety, uint32_t width, uint32_t height );
MHResult mh_pb_pixel_aspect_ratio( MHPb * self, uint32_t pixel_n, uint32_t pixel_d );
MHPb * mh_pb_create();
MHResult mh_pb_register_events_listener( MHPb * self, MHPbEventsListener * listener );
MHResult mh_pb_register_status_listener( MHPb * self, MHPbStatusListener * listener );
MHPbTrackInfo * mh_pb_get_track_info( MHPb * self );
MHPbSubtitleInfo * mh_pb_get_subtitle_info( MHPb * self );
MHResult mh_pb_set_pipeline_status( MHPb * self, uint32_t status );

/*  Filter APIs */
MHFilter * mh_filter_create( const char * filter);
bool mh_filter_match( MHFilter * filter, char * file);

/* Devices APIs */
MHResult mh_dev_start_scan( MHDev * self, MHDevScanType type );
MHResult mh_dev_scan_abort( MHDev * self );
uint32_t mh_dev_register_detach_listener( MHDev * self, MHDevDetachListener * listener);
uint32_t mh_dev_register_events_listener( MHDev * self, MHDevEventsListener * listener);
MHPlaylistInfo * mh_dev_get_playlist_info( MHDev * self );
MHPlaylist * mh_dev_restore_playlist( MHDev * self, int64_t playlist_id );
MHResult mh_dev_save_playlist( MHDev * self, const char * name, MHPlaylist * playlist);

MHResult mh_dev_update_playlist( MHDev * self, const char * name, MHPlaylist * playlist, int64_t playlist_id);
MHResult mh_dev_delete_playlist( MHDev * self, int64_t playlist_id);
char **  mh_dev_get_radiolist( MHDev * self, int * count);
MHResult mh_dev_attach_pb( MHDev * self, MHPb * pb );
MHItem * mh_dev_get_item_by_uri(MHDev * self, const char * uri);
MHPlaylist * mh_dev_create_empty_playlist( MHDev * dev );
MHItem * mh_dev_add_file( MHDev * dest_dev, MHFolder * dest, MHItem * source);
MHResult mh_dev_delete_file( MHDev * dev, MHItem * item);
MHItem ** mh_dev_search_name( MHDev * dev, MHItemType type, char * string, int32_t * count);
/*  Folder APIs  */
MHResult mh_folder_seek( MHFolder * self, MHFilter * filter, int32_t pos, MHItemOrderType order);
MHItem ** mh_folder_get_children( MHFolder * self, MHFilter * filter, MHFolderPosition pos, int32_t * count, MHItemOrderType order );

MHPlaylist * mh_folder_create_playlist( MHFolder * self, MHFilter * filter, bool recursive );

/* Item APIs */

MHResult mh_item_foreach( MHItem ** items, int32_t count, MHItemFunc func, void * user_data );


/* Collection APIs */
MHCol * mh_col_create( MHDev * dev );
MHResult mh_col_add_filter( MHCol * col, MHColFilterType type, ...);
MHResult mh_col_filter_clear( MHCol * col);
MHResult mh_col_set_retrieve_key( MHCol * col, MHColFilterType type);
MHResult mh_col_set_order_type( MHCol * col, MHItemOrderType order);
char ** mh_col_retrieve_data( MHCol * col, MHItemType item_type,MHMediaType media_type,int32_t * count, bool fuzzy);
MHPlaylist * mh_col_create_playlist( MHCol * col, MHItemType item_type, MHMediaType media_type, bool fuzzy);
MHResult mh_col_set_favorite( MHCol * col, MHItemType item_type, bool favorite);

typedef struct _MHAlbumInfo 
{
	char * album_title;
	char * album_artist;
	bool album_compliation;
} MHAlbumInfo;				/* ----------  end of struct MHAlbumInfo  ---------- */

MHAlbumInfo * mh_col_retrieve_album( MHCol * col, MHItemType item_type, MHMediaType media_type, int * count, bool fuzzy);
/* Object APIs */
MHResult mh_object_get_properties( MHObject * self, const char * first_property_name, ... );
MHResult mh_object_set_properties( MHObject * self, const char * first_property_name, ... );
MHObject * mh_object_ref( MHObject * object );
MHResult mh_object_unref( MHObject * object );
uint64_t mh_object_signal_connect( MHObject * object, const char * signal, void * handler, void * user_data );
MHResult mh_object_signal_disconnect( uint64_t signal_id );
/* contents APIs */

/* IPC APIs */
MHIPCConnection * mh_ipc_connection_create();
MHResult mh_ipc_media_client_init( MHIPCConnection * conn );

MHIPCServer * mh_ipc_server_create();
MHResult mh_ipc_start_media_engine( MHIPCServer * server );
MHResult mh_ipc_server_run( MHIPCServer * server );

typedef enum _MHServerEventType
{
	SERVER_INIT_FINISH,

	SERVER_EVENT_MAX
} MHServerEventType;				/* ----------  end of enum MHServerEventType  ---------- */

typedef void (*MHServerEventsCallback)(MHIPCServer * server, MHServerEventType type, void * user_data); 
typedef struct _MHServerEventsListener 
{
	MHServerEventsCallback callback;
	void * user_data;
} MHServerEventsListener;				/* ----------  end of struct MHServerEventsListener  ---------- */

MHResult mh_ipc_server_register_events_listener( MHIPCServer * server, MHServerEventsListener * listener);
/* Apple devices depended APIs */
MHResult mh_dev_request_app_launch( MHDev * self, const char * app_bundle_id );
int32_t mh_dev_write_ea_data( MHDev * self, const uint8_t * buf, int32_t len );
int32_t mh_dev_write_ea_native_data( MHDev * self, const uint8_t *buf, int32_t len);
MHResult mh_dev_write_location_data( MHDev * self, const char * data );
MHResult mh_dev_send_vehicle_status( MHDev * self, uint32_t remainingRange, int32_t outsideTempreture, int32_t rangeWarning ); 
int32_t mh_dev_write_bt_data( MHDev * self, const uint8_t * buf, int32_t len );
MHResult mh_dev_send_wifi_conf_info( MHDev * self, const uint8_t * ssid, const uint8_t * pass, uint8_t securityType, uint8_t channel ); 
/*extern misc*/
typedef enum _MiscResult
{
	MISC_OK	=	0,

	MISC_COVER_INVALID_LENGTH,
	MISC_COVER_MKDIR_FAIL,
	MISC_COVER_INVALID_TYPE,
	MISC_COVER_WRITE_FAIL,
	MISC_COVER_NOT_EXIST,
	MISC_RESULT_NUM,
	MISC_GET_ARTWORK_FAIL
} MiscResult;
MiscResult mh_file_get_cover( char * file_path, char * save_path, int length, char ** cover_path);
MiscResult mh_file_get_video_artwork( char * file_path, char * save_path, int key_num, int64_t * duration, char ** cover_path);
typedef struct _MHTagInfo 
{
	char * title;
	char * album;
	char * artist;
	char * genre;
	int    duration;	
	int year;
	int track;
} MHTagInfo;				/* ----------  end of struct MHTagInfo  ---------- */
MHTagInfo * mh_file_get_tag( char * file_path);


typedef enum _MHDevStatus
{
	CARLIFE_CONNECT_SUCCESS,		//100%
	CARLIFE_VIDEO_INIT_DONE,
	CARLIFE_FOREGROUND,
	CARLIFE_BACKGROUND,
	CARLIFE_SCREEN_ON,
	CARLIFE_SCREEN_OFF,
	CARLIFE_USER_PRESENT, //unlock the MD
	CARLIFE_APP_EXIT,
	CARLIFE_GOTO_DESKTOP,
	CARLIFE_MIC_RECORD_WAKEUP_START,
	CARLIFE_MIC_RECORD_END,
	CARLIFE_MIC_RECORD_RECOG_START,
	CARLIFE_FEATURE_CONFIG_DONE,
	CARLIFE_START_BT_AUTOPAIR,
	CARLIFE_BT_IDENTIFY_FAILED,
	CARLIFE_BT_IDENTIFY_SUCCESS,
	CARLIFE_BT_RECV_MD_INFO,
	CARLIFE_AOA_DETECT_DEVICE, //AOA 10%
	CARLIFE_AOA_INIT, //AOA 30%
	CARLIFE_AOA_GET_VERSION, //AOA 40%
	CARLIFE_AOA_CHANGE_MODE, //AOA 50%
	CARLIFE_AOA_GET_EP, //AOA 60%
	CARLIFE_AOA_CREATE_SOCET_DONE, //AOA 80%
	CARLIFE_AOA_CONNECT_SOCKET_DONE, //AOA 100%


	CARLIFE_NULL,

	IAP_AUTH_SUCCESS	=	25,
	IAP_AUTH_FAILED,
	IAP_AUTH_START,
	CARLIFE_UI_ACTION_SOUND,		//Touch-tone
	CARLIFE_GET_MD_VERSION,			//30%
	CARLIFE_RUN_BDIM,				//50%
	CARLIFE_LAUNCH_APP, 			//60%
	CARLIFE_RECV_MD_INFO,
	CARLIFE_VR_START,
	CARLIFE_VR_STOP,
	CARLIFE_ROLE_SWITCH, //EAP 10%
	CARLIFE_IAP_AUTH,	//EAP 30%
	CARLIFE_IAP_AUTH_SUCCESS, //EAP 60%
	CARLIFE_DEVICE_OPEN,	//EAP 80%
	CARLIFE_EAP_SOCKET_DONE,	//EAP 100%
	CARLIFE_NAVI_TTS_START,
	CARLIFE_NAVI_TTS_STOP


} MHDevStatus;	

typedef void (*dev_status_cb )(MHDev * self, MHDevStatus status, void * user_data);

typedef struct _MHDevStatusListener
{
	dev_status_cb callback;
	void * user_data;
}MHDevStatusListener;

typedef enum _MHTouchType
{
	TOUCH_DOWN,
	TOUCH_UP,
	TOUCH_MOVE,
	TOUCH_SIGNAL_CLICK,
	TOUCH_DOUBLE_CLICK,
	TOUCH_LONG_PRESS
} MHTouchType;				/* ----------  end of enum MHTouchType  ---------- */

MHResult mh_dev_send_signal_touch( MHDev * dev, MHTouchType action, int x, int y );
MHResult mh_dev_start( MHDev * dev);
MHResult mh_dev_register_status_listener( MHDev * dev, MHDevStatusListener * listener);
MHResult mh_misc_create_dev( char * type, char *serial, char * path);	
////////////////////////////carlife///////////////////////


typedef enum _ModuleId
{
	PHONE_MODULE	=	1,
	NAVI_MODULE,
	MUSIC_MODULE,
	VR_MODULE,
	CONNECT_MODULE,
	MIC_MODULE,
	MEDIA_PCM_MODULE
}ModuleId;

typedef enum _PhoneState
{
	PHONE_STATUS_IDLE	=	0,
	PHONE_STATUS_INCOMING,
	PHONE_STATUS_OUTING,
	PHONE_STATUS_CONNECTED
}PhoneState;

typedef enum _NaviState
{
	NAVI_STATUS_IDLE	=	0,
	NAVI_STATUS_RUNNING,
	NAVI_STATUS_STOP
}NaviState;

typedef enum _MusicState
{
	MUSIC_STATUS_IDLE	=	0,
	MUSIC_STATUS_RUNNING
}MusicState;
typedef enum _VRState
{
	VR_STATUS_RECORD_IDLE	=	0,
	VR_STATUS_RECORD_RUNNING,
	VR_STATUS_NOT_SUPPORT,
}VRState;
typedef enum _ConnectState
{
	CONNECT_STATUS_ADB	=	0,
	CONNECT_STATUS_AOA,
	CONNECT_STATUS_NCM_ANDROID,
	CONNECT_STATUS_NCM_IOS,
	CONNECT_STATUS_WIFI
}ConnectState;

typedef enum _MicState
{
	MIC_STATUS_USE_VEHICLE_MIC	=	0,
	MIC_STATUS_USE_MOBILE_MIC,
	MIC_STATUS_NOT_SUPPORT
}MicStatei;


typedef enum _MediaPCMState
{
	MEDIA_PCM_STATUS_IDLE	=	0,
	MEDIA_PCM_STATUS_RUNNING
} MediaPCMState;				/* ----------  end of enum MediaPCMState  ---------- */

typedef struct _ModuleStatus 
{
	uint32_t module;
	uint32_t status;
} ModuleStatus;				

typedef struct _ModuleStatusList 
{
	uint32_t count;
	ModuleStatus * list;

} ModuleStatusList;			

typedef void( * module_status_cb)(MHDev *dev, ModuleStatusList * list, void * user_data); 


typedef struct _ModuleStatusListener 
{
	module_status_cb callback;
	void * user_data;
} ModuleStatusListener;				

MHResult mh_dev_register_module_status( MHDev * dev, ModuleStatusListener * listener);
MHResult mh_dev_module_control( MHDev * dev, uint32_t module, uint32_t status);


#ifdef __cplusplus
}
#endif
#endif

