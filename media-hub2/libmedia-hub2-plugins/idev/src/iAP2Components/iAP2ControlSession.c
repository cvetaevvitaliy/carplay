/*
 * =====================================================================================
 *
 *       Filename:  iAP2ControlSession.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 11:39:01 AM
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
#include <assert.h>

#include <glib.h>

#include <iAP2Link.h>
#include <iAP2LinkPrivate.h>

#include "ama_ipod_drm.h"

#include "iAP2ControlSession.h"
#include "iAP2AuthAndIdentify.h"
#include "iAP2Media.h"
#include "iAP2Hid.h"
#include "debug.h"
#include "dev_iap2.h"
#include <mh_core.h>
#include <mh_misc.h>

//extern MHDevIap2 * iAP2Object;
extern GSList * _iAP2GlobalInfolist;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseRecentsList
 *  Description:  
 * =====================================================================================
 */
static void parseRecentsList ( MHPbInfoData * info, uint8_t * data, int dataLen, int i )
{
	int _offset	=	0;
	int _paramLen, _paramId;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				i	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
				info->recentslist_updates.recentList[i].index	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//				g_message("\tgroup param type is: Index[%d]\n", MAKE_WORD(data[_offset + 4], data[_offset + 5]));
				break;
			case 1:
				info->recentslist_updates.recentList[i].remoteID	=	g_strdup((gchar *)data + _offset + 4);
//				g_message("\tgroup param type is: RemoteID[%s]\n", data + _offset + 4);	
				break;
			case 2:
				info->recentslist_updates.recentList[i].displayName	=	g_strdup((gchar *)data + _offset + 4);
//				g_message("\tgroup param type is: DisplayName[%s]\n", data + _offset + 4);
				break;
			case 3:
				info->recentslist_updates.recentList[i].label	=	g_strdup((gchar *)data + _offset + 4);
//				g_message("\tgroup param type is: Label[%s]\n", data + _offset + 4);
				break;
			case 4:
				info->recentslist_updates.recentList[i].addressBookID	=	g_strdup((gchar *)data + _offset + 4);
//				g_message("\tgroup param type is: AddressBookID[%s]\n", data + _offset + 4);
				break;
			case 5:
				info->recentslist_updates.recentList[i].service	=	data[_offset + 4];
//				g_message("\tgroup param type is: Service[%d]\n", data[_offset + 4]);
				break;
			case 6:
				info->recentslist_updates.recentList[i].type	=	data[_offset + 4];
//				g_message("\tgroup param type is: Type[%d]\n", data[_offset + 4]);
				break;
			case 7:
				info->recentslist_updates.recentList[i].unixTimestamp	=	MAKE_DDWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7],
								data[_offset + 8], data[_offset + 9], data[_offset + 10], data[_offset + 11]);
//				g_message("\tgroup param type is: UnixTimestamp[%lld]\n", MAKE_DDWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7],
//								data[_offset + 8], data[_offset + 9], data[_offset + 10], data[_offset + 11]));
				break;
			case 8:
				info->recentslist_updates.recentList[i].duration	=	MAKE_DWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]);
//				g_message("\tgroup param type is: Duration[%d]\n", MAKE_DWORD(data[_offset + 4], data[_offset + 5], data[_offset + 6], data[_offset + 7]));
				break;
			case 9:
				info->recentslist_updates.recentList[i].occurrences	=	data[_offset + 4];
//				g_message("\tgroup param type is: Occurrences[%d]\n", data[_offset + 4]);	
				break;
		default:
			break;
		}
		_offset	+=	_paramLen;
	}
}		/* -----  end of function parseRecentsList  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseFavoritesList
 *  Description:  
 * =====================================================================================
 */
static void parseFavoritesList ( MHPbInfoData * info, uint8_t * data, int dataLen, int i )
{
	int _offset	=	0;
	int _paramLen, _paramId;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				i	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
				info->favoriteslist_updates.favoritesList[i].index	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
				g_message("\tgroup parseFavoritesList param type is: Index[%d]\n", MAKE_WORD(data[_offset + 4], data[_offset + 5]));
				break;
			case 1:
				info->favoriteslist_updates.favoritesList[i].remoteID	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup parseFavoritesList param type is: RemoteID[%s]\n", data + _offset + 4);	
				break;
			case 2:
				info->favoriteslist_updates.favoritesList[i].displayName	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup parseFavoritesList param type is: DisplayName[%s]\n", data + _offset + 4);
				break;
			case 3:
				info->favoriteslist_updates.favoritesList[i].label	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup parseFavoritesList param type is: Label[%s]\n", data + _offset + 4);
				break;
			case 4:
				info->favoriteslist_updates.favoritesList[i].addressBookID	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup parseFavoritesList param type is: AddressBookID[%s]\n", data + _offset + 4);
				break;
			case 5:
				info->favoriteslist_updates.favoritesList[i].service	=	data[_offset + 4];
				g_message("\tgroup parseFavoritesList param type is: Service[%d]\n", data[_offset + 4]);
				break;
			default:
				break;
		}
		_offset	+=	_paramLen;
	}
}		/* -----  end of function parseFavoritesList  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2ParseListUpdates
 *  Description:  
 * =====================================================================================
 */
static void iAP2ParseListUpdates ( MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen )
{
	int _offset	=	0;
	int _paramLen, _paramId;
	int i = 0;
	MHPbInfoData * _info	=	NULL;

	static int recentsListCount		= 0;
	static int recentsListAvailable	= 0;

	static int favoritesListCount		= 0;
	static int favoritesListAvailable	= 0;

	if(( recentsListCount != 0 ) && ( recentsListAvailable != 0 ))
	{
		_info = g_slice_new( MHPbInfoData );
		_info->recentslist_updates.recentsListAvailable	= recentsListAvailable;
		_info->recentslist_updates.recentList			= (MHPbRecentListInfo *)g_malloc0( sizeof(MHPbRecentListInfo) * recentsListCount );
		_info->recentslist_updates.recentsListCount	  	= recentsListCount;
	}

	if(( favoritesListCount != 0 ) && ( favoritesListAvailable != 0 ))
	{
		_info = g_slice_new( MHPbInfoData );
		_info->favoriteslist_updates.favoritesListAvailable = favoritesListAvailable;
		_info->favoriteslist_updates.favoritesList			= (MHPbFavoritesListInfo *)g_malloc0( sizeof(MHPbFavoritesListInfo) * favoritesListCount );
		_info->favoriteslist_updates.favoritesListCount	  	= favoritesListCount;
	}

	while(_offset < dataLen)
	{
		_paramLen		=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId		=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				recentsListAvailable	=	data[_offset + 4];
//				g_message("\tgroup param type is: RecentsListAvailable[%d]\n", data[_offset + 4]);
				break;
			case 1:
				parseRecentsList( _info, data + _offset + 4, _paramLen - 4, i );
				i++;
				break;
			case 2:
				recentsListCount	=	MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//				g_message("\tgroup param type is: RecentsListCount[%d]\n", MAKE_WORD(data[_offset + 4], data[_offset + 5]));
				break;
			case 5:
				favoritesListAvailable	=	data[_offset + 4];
//				g_message("\tgroup param type is: FavoritesListAvailable[%d]\n", data[_offset + 4]);
				break;
			case 6:
				parseFavoritesList( _info, data + _offset + 4, _paramLen - 4, i );
				i++;
				break;
			case 7:
				favoritesListCount	  	= MAKE_WORD(data[_offset + 4], data[_offset + 5]);
//				g_message("\tgroup param type is: FavoritesListCount[%d]\n", MAKE_WORD(data[_offset + 4], data[_offset + 5]));
				break;
		default:
			break;
		}
		_offset	+=	_paramLen;
	}

	if(( recentsListCount != 0)&&( i != 0))
	{
		mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_RECENTS_LIST_UPDATES, _info );
		recentsListCount	 =	0;
		recentsListAvailable =	0;
	}

	if(( favoritesListCount != 0)&&( i != 0))
	{
		mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_FAVORITES_LIST_UPDATES, _info );
		favoritesListCount	 	=	0;
		favoritesListAvailable 	=	0;
	}
}		/* -----  end of function iAP2ParseListUpdates  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2SendListUpdatesMessage
 *  Description:  
 * =====================================================================================
 */
static void iAP2SendListUpdatesMessage(iAP2Link_t * link)
{
	uint8_t * _startListUpdatesMessage;
	uint8_t   _startListUpdatesMessageLen;

	uint8_t _recentsList[]		=	{       	/* RecentsListProperties */
		0x00, 0x04, 0x00, 0x00,                 /* Index */
		0x00, 0x04, 0x00, 0x01,                 /* RemoteID */
		0x00, 0x04, 0x00, 0x02,                 /* DisplayName */
		0x00, 0x04, 0x00, 0x03,                 /* Label */
		0x00, 0x04, 0x00, 0x04,                 /* AddressBookID */
		0x00, 0x04, 0x00, 0x05,					/* Service */
		0x00, 0x04, 0x00, 0x06,                 /* Type */
		0x00, 0x04, 0x00, 0x07,                 /* UnixTimestamp */
		0x00, 0x04, 0x00, 0x08,                 /* Duration */
		0x00, 0x04, 0x00, 0x09,                 /* Occurrences */
	};

	uint8_t _favoritesList[]		=	{       /* FavoritesListProperties */
		0x00, 0x04, 0x00, 0x00,                 /* Index */
		0x00, 0x04, 0x00, 0x01,                 /* RemoteID */
		0x00, 0x04, 0x00, 0x02,                 /* DisplayName */
		0x00, 0x04, 0x00, 0x03,                 /* Label */
		0x00, 0x04, 0x00, 0x04,                 /* AddressBookID */
		0x00, 0x04, 0x00, 0x05,					/* Service */
	};

	uint8_t _recentsListMax[]	=	{
		0x00, 0x06, 0x00, 0x03, 0x00, 0x00,
	};

	uint8_t	_recentsListCombine[]	=	{
		0x00, 0x05, 0x00, 0x04, 0x01,
	};

	uint8_t _favoritesListMax[]	=	{
		0x00, 0x06, 0x00, 0x08, 0x00, 0x00,
	};

	_startListUpdatesMessageLen	=	4 + sizeof(_recentsList) + 6 + 5 + 4 + sizeof(_favoritesList) + 6;

	_startListUpdatesMessage	=	(uint8_t *)g_malloc0( _startListUpdatesMessageLen );

	_startListUpdatesMessage[0]	=	IAP2_HI_BYTE(4 + sizeof(_recentsList));
	_startListUpdatesMessage[1]	=	IAP2_LO_BYTE(4 + sizeof(_recentsList));
	_startListUpdatesMessage[2]	=	0x00;
	_startListUpdatesMessage[3]	=	0x01;

	memcpy(_startListUpdatesMessage + 4, _recentsList, sizeof(_recentsList));
	memcpy(_startListUpdatesMessage + 4 + sizeof(_recentsList), _recentsListMax, sizeof(_recentsListMax));
	memcpy(_startListUpdatesMessage + 4 + sizeof(_recentsList) + 6, _recentsListCombine, sizeof(_recentsListCombine));

	_startListUpdatesMessage[4 + sizeof(_recentsList) + 6 + 5]		=	IAP2_HI_BYTE(4 + sizeof(_favoritesList));
	_startListUpdatesMessage[4 + sizeof(_recentsList) + 6 + 5 + 1]	=	IAP2_LO_BYTE(4 + sizeof(_favoritesList));
	_startListUpdatesMessage[4 + sizeof(_recentsList) + 6 + 5 + 2]	=	0x00;
	_startListUpdatesMessage[4 + sizeof(_recentsList) + 6 + 5 + 3]	=	0x06;
	
	memcpy(_startListUpdatesMessage + 4 + sizeof(_recentsList) + 6 + 5 + 4, _favoritesList, sizeof(_favoritesList));
	memcpy(_startListUpdatesMessage + 4 + sizeof(_recentsList) + 6 + 5 + 4 + sizeof(_favoritesList), _favoritesListMax, sizeof(_favoritesListMax));
	
	iAP2SendControlMessage(link, 0x4170, _startListUpdatesMessage, _startListUpdatesMessageLen, IAP2_CONTROL_SESSION_ID);
}		/* -----  end of function iAP2SendListUpdatesMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2ParseCallState
 *  Description:  
 * =====================================================================================
 */
static void iAP2ParseCallState ( MHDevIap2 * _iAP2Object, uint8_t * data, int dataLen )
{
	int _offset	=	0;
	int _paramLen, _paramId;

	MHPbInfoData * _info = g_slice_new( MHPbInfoData );

	_info->call_state_info.remoteID 		= NULL;
	_info->call_state_info.displayName 		= NULL;
	_info->call_state_info.status 			= 0xFF;
	_info->call_state_info.direction 		= 0xFF;
	_info->call_state_info.callUUID 		= NULL;
	_info->call_state_info.addressBookID 	= NULL;
	_info->call_state_info.label 			= NULL;
	_info->call_state_info.service 			= 0xFF;
	_info->call_state_info.isConferenced 	= 0xFF;
	_info->call_state_info.conferenceGroup 	= NULL;
	_info->call_state_info.disconnectReason = 0xFF;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 0:
				_info->call_state_info.remoteID	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup param type is: RemoteID[%s]\n", data + _offset + 4);
				break;
			case 1:
				_info->call_state_info.displayName	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup param type is: DisplayName[%s]\n", data + _offset + 4);
				break;
			case 2:
				_info->call_state_info.status	=	data[_offset + 4];
				g_message("\tgroup param type is: Status [%d]\n",  data[_offset + 4] );
				break;
			case 3:
				_info->call_state_info.direction	=	data[_offset + 4];
				g_message("\tgroup param type is: Direction[%d]\n", data[_offset + 4]);
				break;
			case 4:
				_info->call_state_info.callUUID		=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup param type is: CallUUID[%s]\n",  data + _offset + 4 );
				break;
			case 6:
				_info->call_state_info.addressBookID	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup param type is: AddressBookID[%s]\n",  data + _offset + 4 );
				break;
			case 7:
				_info->call_state_info.label	=	g_strdup((gchar *)data + _offset + 4);
				g_message("\tgroup param type is: Label[%s]\n",  data + _offset + 4 );
				break;
			case 8:
				_info->call_state_info.service	=	data[_offset + 4];
				g_message("\tgroup param type is: Service[%d]\n", data[_offset + 4]);
				break;
			case 9:
				_info->call_state_info.isConferenced	=	data[_offset + 4];
				g_message("\tgroup param type is: IsConferenced[%d]\n", data[_offset + 4]);
				break;
			case 10:
				_info->call_state_info.conferenceGroup	=	g_strdup((gchar *)data + _offset + 4);	
				g_message("\tgroup param type is: ConferenceGroup[%s]\n", data + _offset + 4);
				break;
			case 11:
				_info->call_state_info.disconnectReason	=	data[_offset + 4];
				g_message("\tgroup param type is: DisconnectReason[%d]\n", *( data + _offset + 4 ));
				break;
			default:
				assert(0);
				break;
		}

		_offset	+=	_paramLen;
	}

	if(  _info->call_state_info.remoteID	==	NULL )
		_info->call_state_info.remoteID	=	g_strdup( "" );	

	if(  _info->call_state_info.displayName	==	NULL )
		_info->call_state_info.displayName	=	g_strdup( "" );	

	if(  _info->call_state_info.callUUID	==	NULL )
		_info->call_state_info.callUUID	=	g_strdup( "" );	

	if(  _info->call_state_info.addressBookID	==	NULL )
		_info->call_state_info.addressBookID	=	g_strdup( "" );	

	if(  _info->call_state_info.label	==	NULL )
		_info->call_state_info.label	=	g_strdup( "" );	

	if(  _info->call_state_info.conferenceGroup	==	NULL )
		_info->call_state_info.conferenceGroup	=	g_strdup( "" );	

	mh_pb_set_media_info( _iAP2Object->pb, MH_PB_IP_INFO_CALL_STATE_UPDATE, _info );

}		/* -----  end of static function iAP2ParseCallState  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2SendCallStateUpdateMessage
 *  Description:  
 * =====================================================================================
 */
static void iAP2SendCallStateUpdateMessage(iAP2Link_t * link)
{
	uint8_t _callState[]		=	{       	/* StartCallStateUpdates */
		0x00, 0x04, 0x00, 0x00,                 /* RemoteID */
		0x00, 0x04, 0x00, 0x01,                 /* DisplayName */
		0x00, 0x04, 0x00, 0x02,                 /* Status */
		0x00, 0x04, 0x00, 0x03,                 /* Direction */
		0x00, 0x04, 0x00, 0x04,                 /* CallUUID */
		0x00, 0x04, 0x00, 0x06,                 /* AddressBookID */
		0x00, 0x04, 0x00, 0x07,                 /* Label */
		0x00, 0x04, 0x00, 0x08,                 /* Service */
		0x00, 0x04, 0x00, 0x09,                 /* IsConferenced */
		0x00, 0x04, 0x00, 0x0A,                 /* ConferenceGroup */
		0x00, 0x04, 0x00, 0x0B,                 /* DisconnectReason */
	};
	iAP2SendControlMessage(link, 0x4154, _callState, sizeof(_callState), IAP2_CONTROL_SESSION_ID);
}		/* -----  end of function iAP2SendCallStateUpdateMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StartVehicleStatusUpdatesMessage
 *  Description:  
 * =====================================================================================
 */
uint8_t iAP2StartVehicleStatusUpdatesMessage (uint8_t * data, uint16_t dataLen)
{
	int _offset	=	0;
	uint16_t _paramLen;
	uint16_t _paramId;
	
	uint8_t status = 0;

	while( _offset < dataLen )
	{
		_paramLen	=	MAKE_WORD( data[_offset], data[_offset + 1] );
		_paramId	=	MAKE_WORD( data[_offset + 2], data[_offset + 3] );
		g_message("_paramId = %d\n",_paramId);
		switch(_paramId)
		{
			case 3: //Range
				status |= 0x1; 
				break;
			case 4: //OutsideTemperature
				status |= 0x2;
				break;
			case 6: //RangeWarning
				status |= 0x4;
				break;
			default:
				g_message("unknown status parameter[%d]\n", _paramId);
				break;
		}
		
		if(_paramLen == 0){
			g_message("CarPlay status Err\n");
			break;
		}

		_offset	+=	_paramLen;
	}
	
	return status;

}		/* -----  end of function iAP2StartVehicleStatusUpdatesMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StartLocationMessage
 *  Description:  
 * =====================================================================================
 */
uint8_t iAP2StartLocationMessage (uint8_t * data, uint16_t dataLen)
{
	int _offset	=	0;
	uint16_t _paramLen;
	uint16_t _paramId;
	
	uint8_t GNNSMode = 0;

	while(_offset < dataLen)
	{
		_paramLen	=	MAKE_WORD(data[_offset], data[_offset + 1]);
		_paramId	=	MAKE_WORD(data[_offset + 2], data[_offset + 3]);

		switch(_paramId)
		{
			case 1: //GPGGA
				GNNSMode |= 0x1; 
				break;
			case 2: //GPRMC
				GNNSMode |= 0x2;
				break;
			case 3: //GPGSV
				GNNSMode |= 0x4;
				break;
			case 4: //PASCD
				GNNSMode |= 0x8;
				break;
			case 5: //PAGCD
				GNNSMode |= 0x10;
				break;
			case 6: //PAACD
				GNNSMode |= 0x20;
				break;
			case 7: //GPHDT
				GNNSMode |= 0x40;
				break;
			default:
				g_message("unknown GNSS parameter[%d]\n", _paramId);
				break;
		}
		
		if(_paramLen == 0){
			g_message("CarPlay Location Err\n");
			break;
		}

		_offset	+=	_paramLen;
	}
	
	return GNNSMode;

}		/* -----  end of function iAP2StartLocationMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StartPowerUpdates
 *  Description:  
 * =====================================================================================
 */
static void iAP2StartPowerUpdates(iAP2Link_t * link)
{
	uint8_t _startPowerUpdates[]	=	{
		0x00, 0x04, 0x00, 0x00,
		0x00, 0x04, 0x00, 0x01,
		0x00, 0x04, 0x00, 0x04,
		0x00, 0x04, 0x00, 0x05,
		0x00, 0x04, 0x00, 0x06,
	};

	iAP2SendControlMessage(link, 0xAE00, _startPowerUpdates, sizeof(_startPowerUpdates), IAP2_CONTROL_SESSION_ID);
}		/* -----  end of function iAP2StartPowerUpdates  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2SendPowerMessage
 *  Description:  
 * =====================================================================================
 */
static void iAP2SendPowerMessage(iAP2Link_t * link)
{
	int _paramsLen = 4 + sizeof( uint16_t ) + 4 + sizeof( bool );

	g_message("%d, %lld, %lld",_paramsLen,(long long int)sizeof( uint16_t ),(long long int)sizeof( bool ));

	uint8_t * _powerMessage = g_malloc0( _paramsLen );

	_powerMessage[0] = IAP2_HI_BYTE(4 + sizeof( uint16_t ));
	_powerMessage[1] = IAP2_LO_BYTE(4 + sizeof( uint16_t ));
	_powerMessage[2] = 0x00;
	_powerMessage[3] = 0x00;
//	_powerMessage[4] = 0x09;
//	_powerMessage[5] = 0x60;
	_powerMessage[4] = 0x03;
	_powerMessage[5] = 0xE8;
	_powerMessage[6] = IAP2_HI_BYTE(4 + sizeof( bool ));
	_powerMessage[7] = IAP2_LO_BYTE(4 + sizeof( bool ));
	_powerMessage[8] = 0x00;
	_powerMessage[9] = 0x01;
	_powerMessage[10] = 0x01;

	iAP2SendControlMessage(link, 0xAE03, _powerMessage, _paramsLen, IAP2_CONTROL_SESSION_ID );

	g_free(_powerMessage);
}		/* -----  end of function iAP2SendPowerMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2ParseControlSession
 *  Description:  
 * =====================================================================================
 */
void iAP2ParseControlSession (iAP2Link_t * link, uint8_t * data, uint32_t dataLen, uint8_t session )
{
	int _tmpLen	=	0, _payloadLen;
	uint8_t * _messageParam	=	NULL;
    uint16  _sample_rate[] = {
             8000,
            11025,
            12000,
            16000,
            22050,
            24000,
            32000,
            44100,
            48000
            };
	gchar * _libraryId;
	uint16_t _idLen;
	uint8_t * _startMLUMessage;
	uint16_t _startMLUMessageLen;
	uint8_t * _startNPUMessage;
	uint16_t _startNPUMessageLen;
	uint8_t * _cert, * _response;
	int16_t _certLen, _resLen;
	MHDevIap2 * _iAP2Object = NULL;

	_payloadLen	=	MAKE_WORD(data[2], data[3]) - 6;

	GSList *iterator = NULL;

	for (iterator = _iAP2GlobalInfolist; iterator; iterator = iterator->next) 
	{
		if (link == ((iAP2GlobalInfo *)iterator->data)->piAP2Object->plinkRunLoop->link)
		{
			_iAP2Object = ((iAP2GlobalInfo *)iterator->data)->piAP2Object;
			break;
		}
	}

	if (NULL == _iAP2Object)
	{
		g_message("iAP2ParseControlSession not find iAP2Object");
	}

	switch(MAKE_WORD(data[4], data[5]))
	{
	case 0xAA00:
		g_message("request authentication\n");

		_certLen	=	iAP2AuthReadCertData( &_cert );

		iAP2SendControlMessageWithParam(link, 0xAA01, 0x00, _cert, _certLen, session);
	
		break;
	case 0xAA02:
		g_message("request challenge data\n");
		_tmpLen	=	MAKE_WORD(data[6], data[7]);

		iAP2AuthWriteChallengeData( data + 10, _tmpLen - 4 );

		_resLen	=	iAP2AuthReadChallengeResponse( &_response );

		iAP2SendControlMessageWithParam(link, 0xAA03, 0x00, _response, _resLen, session);

		if( _response != NULL )
			g_free( _response );

		break;
	case 0xAA04:
		g_message("authentication failed\n");
		DEBUG_HEX_DISPLAY(data, dataLen);

		g_signal_emit_by_name( MH_DEV( _iAP2Object), "dev_status", IAP_AUTH_FAILED);
		break;
	case 0xAA05:
		g_message("authentication succeeded\n");
		DEBUG_HEX_DISPLAY(data, dataLen);

		g_signal_emit_by_name( MH_DEV( _iAP2Object), "dev_status", IAP_AUTH_SUCCESS);
//		mh_core_attach_dev( MH_DEV( iAP2Object ));
#if 1 //tongxsh
		if((_iAP2Object->transportType	==	MH_DEV_BT_IAP)
		   && ( mh_misc_get_iap_device_mode() == MISC_IAP_CARPLAY ))
		{
			MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
			_devParam->type				=	MH_DEV_WIFI_CARPLAY;
			_devParam->mac_addr			=	g_strdup( _iAP2Object->macAddress );
			_devParam->connect_status	=	3;	

			mh_core_find_dev( _devParam );

			g_free( _devParam->mac_addr );
			g_free( _devParam );
			break;
		}	
#endif
		/* Start iAP2 POWER */
		iAP2StartPowerUpdates( link );

		iAP2SendPowerMessage( link );

		/* Start iAP2 HID */
		if( _iAP2Object->mode == MISC_IAP )
		{
			iAP2GetHidStartParams(&_messageParam, &_tmpLen);
			iAP2SendControlMessage(link, 0x6800, _messageParam, _tmpLen, IAP2_CONTROL_SESSION_ID);

			g_free(_messageParam);
		}
		/* Start iAP2 Now Playing Update */
		if(( _iAP2Object->mode == MISC_IAP_CARPLAY || _iAP2Object->mode == MISC_IAP_CARPLAY_CARLIFE || _iAP2Object->mode == MISC_IAP_CARLIFE) && ( iAP2GetNowPlayingUpdateFlag() == 0 ))
		{
		}else{
			iAP2MediaGetNPUMessage(&_startNPUMessage, &_startNPUMessageLen);
			iAP2SendControlMessage(link, 0x5000, _startNPUMessage, _startNPUMessageLen, session);

			g_free(_startNPUMessage);
		}
		if( _iAP2Object->mode != MISC_IAP_CARLIFE)
		{
			if( iAP2GetCallStateUpdateFlag() == 1 )
			{
				iAP2SendCallStateUpdateMessage( link );
			}
		}

		if( iAP2GetListUpdatesFlag() == 1 )
		{
			iAP2SendListUpdatesMessage( link );
		}
		break;
	case 0x1D00:
		g_message("request identification\n");
		g_signal_emit_by_name(MH_DEV(_iAP2Object), "dev_status", IAP_AUTH_START);
		iAP2GetIndentifyParam( _iAP2Object, &_messageParam, &_tmpLen );
		iAP2SendControlMessage(link, 0x1D01, _messageParam, _tmpLen, session);

		g_free(_messageParam);

		break;
	case 0x1D02:
		g_message("identification accepted\n");
		break;
	case 0x1D03:
		g_message("identification rejected\n");
		DEBUG_HEX_DISPLAY(data, dataLen);
		if( _iAP2Object->indentifyFlag == TRUE )
			g_signal_emit_by_name( MH_DEV( _iAP2Object), "dev_status", IAP_AUTH_FAILED);

		if( _iAP2Object->indentifyFlag	==	FALSE )
		{
			_iAP2Object->indentifyFlag	=	TRUE;	
			g_message("request identification second\n");
			iAP2GetIndentifyParam( _iAP2Object, &_messageParam, &_tmpLen );
			iAP2SendControlMessage(link, 0x1D01, _messageParam, _tmpLen, session);

			g_free(_messageParam);

		}
		break;
	case 0x4C01:
		g_message("media library information\n");

		iAP2MediaParseLibraryInformation( _iAP2Object, data + 6, _payloadLen); /* omit message header */

//		iAP2SendControlMessageWithParam(link, 0x4C06, 0x0000, (uint8_t *)_libraryId, _idLen, session); /* Play Media Library Current Selection */

		if( !_iAP2Object->hostMode && _iAP2Object->stopUpdateFlag == 0)
		{
			g_message("send 0xDA00 to StartUSBDeviceModeAudio");
			iAP2SendControlMessage(link, 0xDA00, NULL, 0, session); /* StartUSBDeviceModeAudio */
		}
		
		//start local media
		iAP2MediaGetMLUMessage( _iAP2Object, &_startMLUMessage, &_startMLUMessageLen );
		iAP2SendControlMessage(link, 0x4C03, _startMLUMessage, _startMLUMessageLen, session); /* Start Media Library Update */
		g_free(_startMLUMessage);

		if (0 != _iAP2Object->radioLibraryIdLen)
		{
			//start radio
			iAP2MediaGetMLURadioMessage(_iAP2Object, &_startMLUMessage, &_startMLUMessageLen );
			iAP2SendControlMessage(link, 0x4C03, _startMLUMessage, _startMLUMessageLen, session); /* Start Radio Library Update */
			g_free(_startMLUMessage);
		}

		iAP2StartMediaLibraryUpdateTimeout( _iAP2Object );
		break;
	case 0xDA01:
		/* SampleRate */
        if( (MAKE_WORD(data[6], data[7]) == 0x0005)   //parameter length
         && (MAKE_WORD(data[8], data[9]) == 0x0000) ) //parameter ID
        {
            g_message("sample rate index = [%d] sample rate = [%d]\n", data[10] ,  _sample_rate[ data[10] ]);

			if( _iAP2Object != NULL )
			{
				if( _sample_rate[ data[10] ] != _iAP2Object->sample_rate )
				{
					g_message( "sample rate change!\n");
					_iAP2Object->sample_rate_num	=	_sample_rate[ data[10]];	
					iAP2SetResample( _iAP2Object );
				}
			}
		}

		break;
	case 0x5001:
		/* Now Playing Update */
		iAP2MediaParseNPUMessage( _iAP2Object, data + 6, _payloadLen);
		break;
	case 0x4C04:
		/* Media Library Update */
		iAP2MediaParseMLUMessage( _iAP2Object, data + 6, _payloadLen);
		break;
	case 0x4E09:
		g_message("Device Information Update: [%s]\n", data + 10);
		iAP2MediaParseDeviceNameMessage( _iAP2Object, data + 10);
		break;
	case 0x4E0A:
		g_message("Device Language Update: [%s]\n", data + 10);
		break;
		
	case 0x4E0C:
		g_message("UUID#################################: [%s]\n", data + 10);
		break;
	case 0x4E0E:
		g_message("0x4E0E!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		g_message("mac address#################################: length=%d, id = %d name:[%s]\n", 
			( data[6] << 8 ) | data[7], ( data[8] << 8 ) | data[9], data + 10);
		g_message("usb serialNum#################################: length=%d, id = %d name:[%s]\n", 
			( data[6 + (( data[6] << 8 ) | data[7])] << 8 ) | data[7 + (( data[6] << 8 ) | data[7])], ( data[8 + (( data[6] << 8 ) | data[7])] << 8 ) | data[9 + (( data[6] << 8 ) | data[7])], data + 10 + (( data[6] << 8 ) | data[7]));
		break;
		
	case 0xEA00:
		g_message("%s ===> ea_session_start dev = %p data[10] = %d ( data[15] << 8 ) | data[16] = %d",__func__, _iAP2Object, data[10], ( data[15] << 8 ) | data[16]);
#ifdef __x86_64__
		g_signal_emit_by_name( _iAP2Object, "ea_session_start", data[10], ( data[15] << 8 ) | data[16],(guint64)_iAP2Object->pb );
#else
		g_signal_emit_by_name( _iAP2Object, "ea_session_start", data[10], ( data[15] << 8 ) | data[16],(guint)_iAP2Object->pb );
#endif
		break;
	case 0xEA01:
		g_message("%s ===> ea_session_stop dev = %p",__func__, _iAP2Object);
		g_signal_emit_by_name( _iAP2Object, "ea_session_stop", ( data[10] << 8 ) | data[11] );
		break;
	case 0xAE01:
		g_message("power update\n");
		break;
	case 0xFFFA:
		{
		uint8_t location_id	=	iAP2StartLocationMessage( data + 6, _payloadLen );
		g_signal_emit_by_name( _iAP2Object, "start_location_info", location_id );
		}
		break;
	case 0xFFFC:
		g_signal_emit_by_name( _iAP2Object, "stop_location_info" );
		break;
	case 0xA100:
		{
		uint8_t status_id = iAP2StartVehicleStatusUpdatesMessage( data + 6, _payloadLen );
		g_signal_emit_by_name( _iAP2Object, "start_vehicle_status_updates", status_id );
		}
		break;
	case 0xA102:
		g_signal_emit_by_name( _iAP2Object, "stop_vehicle_status_updates" );
		break;
	case 0x4155:
		iAP2ParseCallState( _iAP2Object, data + 6, _payloadLen );
		break;
	case 0x4171:
		iAP2ParseListUpdates( _iAP2Object, data + 6, _payloadLen );
		break;
	case 0x5702:
		g_message("%s ===> req_accessory_wifi_conf_info",__func__);
		g_signal_emit_by_name( _iAP2Object, "req_accessory_wifi_conf_info" );
		break;
	case 0x4E0D:
		g_signal_emit_by_name( _iAP2Object, "wifi_carplay_update", data[10] );
		break;
 	default:
		g_message("unhandled control session message 0x%02X%02X\n", data[4], data[5]);
		assert(0);
		break;
	}
}		/* -----  end of function iAP2ParseControlSession  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2SendControlMessage
 *  Description:  
 * =====================================================================================
 */
int8_t iAP2SendControlMessage ( iAP2Link_t * link, uint16_t messageId, uint8_t * data, uint32_t dataLen, uint8_t session )
{
	uint8_t * _message;

	_message	=	(uint8_t *)g_malloc0(dataLen + 6);
	
	_message[0]	=	0x40;
	_message[1]	=	0x40;
	_message[2]	=	IAP2_HI_BYTE(dataLen + 6);
	_message[3]	=	IAP2_LO_BYTE(dataLen + 6);
	_message[4]	=	IAP2_HI_BYTE(messageId);
	_message[5]	=	IAP2_LO_BYTE(messageId);

	memcpy(_message + 6, data, dataLen);

	iAP2LinkQueueSendData(link, _message, dataLen + 6, session, NULL, NULL);

	g_free(_message);

	return 0;
}		/* -----  end of function iAP2SendControlMessage  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2SendControlMessageWithParam
 *  Description:  
 * =====================================================================================
 */
void iAP2SendControlMessageWithParam (iAP2Link_t * link, uint16_t messageId, uint16_t paramId, 
		uint8_t * data, uint32_t dataLen, uint8_t session)
{
	uint8_t * _param;

	g_message("%s: message id 0x%04X, param id 0x%04X, data length %d\n", __func__, messageId, paramId, dataLen);

	_param	=	(uint8_t *)g_malloc0(dataLen + 4);

	_param[0]	=	IAP2_HI_BYTE(dataLen + 4);
	_param[1]	=	IAP2_LO_BYTE(dataLen + 4);
	_param[2]	=	IAP2_HI_BYTE(paramId);                          /* Authentication Certificate ID */
	_param[3]	=	IAP2_LO_BYTE(paramId);

	memcpy(_param + 4, data, dataLen);

	iAP2SendControlMessage(link, messageId, _param, dataLen + 4, session);

	g_free(_param);
}		/* -----  end of function iAP2SendControlMessageWithParam  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StartMLUpdate
 *  Description:  
 * =====================================================================================
 */
void iAP2StartMLUpdate( iAP2Link_t * link )
{
	iAP2SendControlMessage( link, 0x4C00, NULL, 0, IAP2_CONTROL_SESSION_ID );
}		/* -----  end of function iAP2StartMLUpdate  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StopMLUpdate
 *  Description:  
 * =====================================================================================
 */
void iAP2StopMLUpdate( iAP2Link_t * link )
{
	iAP2SendControlMessage( link, 0x4C02, NULL, 0, IAP2_CONTROL_SESSION_ID );
}		/* -----  end of function iAP2StopMLUpdate  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2StopMLMessageUpdate
 *  Description:  
 * =====================================================================================
 */
void iAP2StopMLMessageUpdate( iAP2Link_t * link, uint8_t * data, uint32_t dataLen)
{
	uint8_t * _stopMLUMessage = NULL;

	_stopMLUMessage	=	(uint8_t *)malloc(4 + dataLen);

	_stopMLUMessage[0]	=	IAP2_HI_BYTE(dataLen + 4);
	_stopMLUMessage[1]	=	IAP2_LO_BYTE(dataLen + 4);
	_stopMLUMessage[2]	=	0x00;
	_stopMLUMessage[3]	=	0x00;

	memcpy( _stopMLUMessage + 4, data, dataLen);

	iAP2SendControlMessage( link, 0x4C05, _stopMLUMessage,  dataLen + 4, IAP2_CONTROL_SESSION_ID );
	free(_stopMLUMessage);
}		/* -----  end of function iAP2StopMLMessageUpdate  ----- */

