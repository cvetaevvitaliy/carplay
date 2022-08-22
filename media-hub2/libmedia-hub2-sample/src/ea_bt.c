/*
 * =====================================================================================
 *
 *       Filename:  carplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/29/2015 03:03:28 PM
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
#include <unistd.h>
#include <mh_api.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "debug.h"
#include <glib.h>
#include "fundef.h"

MHDev * iap2;
uint64_t start_sig;
uint64_t stop_sig;
uint64_t data_sig;
uint64_t pb_sig;
uint64_t bt_sig;

pthread_t write_tid;
MHPb * _pb;
uint32_t devCount = 0;
ANW_BD_ADDR  mIapMacAddr[2];

bool g_bInquiry 			=	false;
bool g_bPairing 			=	false;
bool g_bPBDownloding[2] 	=	{false};
bool g_bSMSDownloding[2]	= 	{false};
bool g_bRunPowerOffProcess	= 	false;
bool g_bBTPowerStatus 		= 	false;
SPP_CONNECTION g_SPPConnectInfo[4];

int g_HFPConnectStatus[2] 	= 	{0};
int g_A2DPConnectStatus 	=	0;
int g_HIDConnectStatus		=	0;
int g_SCOConnectStatus[2]	=	{0};
int g_AVRCPBrowsingConnectStatus = 0;

int   mSearchCarPlayDev = 0;
//ANW_BD_ADDR  mCarPlayMacAddr[2];
unsigned int mIapIndex;

void event_arrived( MHCore * core, MHCoreEvent event, const char * type, void * user_data )
{
	switch( event )
	{
	case MH_CORE_STARTED:
		printf( "Media-Hub v2.0 has been started\n" );

		break;
	case MH_CORE_PLUGIN_INVALID:
		printf( "Invalid Media-Hub v2.0 plugin be found\n" );

		break;
	case MH_CORE_PLUGIN_NOT_FOUND:
		printf( "No plugins be found\n" );

		break;
	case MH_CORE_PLUGIN_LOAD_SUCCESS:
		printf( "Success load a plugin\n" );
		break;
	case MH_CORE_PLUGIN_LOAD_FAILED:
		printf( "Failed load a plugin\n" );
		break;
	default:
		break;
	}
}

void bt_data( void * _, MHDev * dev, GVariant * var, void * user_data )
{
	GVariant * _var	=	g_variant_get_variant( var );
	const uint8_t * _buf;
	gsize _len	=	0;
	int nWritten =0;
	
	_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( uint8_t ));
	uint8_t * _bt_buf;	

	if( _len > 0x28A )
	{
		int _len_p = 0;
		int _last_len = _len%0x28A; 

		for( int i = 0; i< _len/0x28A; i++  )
		{
			_bt_buf	=	g_malloc0( 0x28A );

			memcpy( _bt_buf, _buf + _len_p, 0x28A);
			g_message("bt_data  ************************** for start \n");
			DEBUG_HEX_DISPLAY( _bt_buf,  0x28A);
			g_message("bt_data  ************************** for end \n");
			
			ANW_SPPWrite( mIapIndex, _bt_buf, 0x28A , &nWritten);
			g_free( _bt_buf );
	
			_len_p += 0x28A; 
		}
		
		if( _last_len != 0 )
		{
			_bt_buf	=	g_malloc0( _last_len );

			memcpy( _bt_buf, _buf + _len_p, _last_len);
			g_message("bt_data  ************************** if start \n");
			DEBUG_HEX_DISPLAY( _bt_buf,  _last_len);
			g_message("bt_data  ************************** if end \n");

			ANW_SPPWrite( mIapIndex, _bt_buf, _last_len , &nWritten);
			g_free( _bt_buf );
		}

	}else{
		_bt_buf	=	g_malloc0( _len );

		memcpy( _bt_buf, _buf, _len);
		g_message("bt_data  ************************** start _len = %d  %d\n",_len,sizeof( _bt_buf));
		DEBUG_HEX_DISPLAY( _bt_buf,  _len);
		g_message("bt_data  ************************** end\n");
		ANW_SPPWrite( mIapIndex, _bt_buf, _len , &nWritten);

		g_free( _bt_buf );
	}
	g_variant_unref( _var );
}

void * write_task( void * user_data )
{
	int i, _len;
	uint8_t _buf[1024]	=	{0};
	uint64_t _sessionId	=	( uint64_t )user_data;

	_buf[0]	=	( _sessionId >> 8 ) & 0xFF;
	_buf[1]	=	_sessionId & 0xFF;

	for( i = 2; i < sizeof( _buf ); i ++ )
		_buf[i]	=	i;

	while( 1 )
	{
		_len	=	mh_dev_write_ea_data( iap2, _buf, sizeof( _buf ) );

		printf( "wrote %d\n", _len );

		sleep( 1 );
	}

	return NULL;
}

void ea_session_start( void * _, MHDev * dev, uint64_t protocol_id, uint64_t session_id, uint64_t pb ,void * user_data )
{
	printf( "ea.c ea_session_start ====================== pb: %d \n", pb );
	if (0 != pb)
	{
		_pb = (MHPb *)pb;
		printf( "ea.c ea_session_start ====================== _pb: %p \n", _pb );
		mh_pb_set_pipeline_status(_pb,1);
	}
	printf( "started: %x %x\n", protocol_id, session_id );

	pthread_create( &write_tid, NULL, write_task, ( void * )session_id );
}

void ea_session_stop( void * _, MHDev * dev, uint32_t session_id, void * user_data )
{
	printf( "stopped: %x \n", session_id );
	mh_pb_set_pipeline_status(_pb,0);
	pthread_cancel( write_tid );

	write_tid	=	0;
}

void ea_session_data( void * _, MHDev * dev, GVariant * var, void * user_data )
{
	GVariant * _var	=	g_variant_get_variant( var );
	const uint8_t * _buf;
	int64_t _len;

	_buf	=	g_variant_get_fixed_array( _var, &_len, sizeof( uint8_t ));

	DEBUG_HEX_DISPLAY( _buf, _len );

	g_variant_unref( _var );
}

void ea_session_send_pb( void * _, MHDev * dev, uint64_t pb, void * user_data )
{
	printf( "ea.c ea_session_send_pb ====================== pb: %d \n", pb );
	_pb = (MHPb *)pb;
	printf( "ea.c ea_session_send_pb ====================== _pb: %p \n", _pb );
	mh_pb_set_pipeline_status(_pb,1);
}

static void _detach_event(	MHDev * dev, void * user_data)
{
	mh_object_signal_disconnect( start_sig );
	mh_object_signal_disconnect( stop_sig );
	mh_object_signal_disconnect( data_sig );
	mh_object_signal_disconnect( pb_sig );

	iap2	=	NULL;

	if( write_tid != 0 ) pthread_cancel( write_tid );
}		/* -----  end of static function _detach_event  ----- */

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char * _dev_type;

	mh_object_get_properties( ( MHObject * )dev, "type", &_dev_type, NULL );

	if( _dev_type && strcmp( _dev_type, "iap2_bt" ) == 0 )
	{
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};

		iap2	=	dev;

		start_sig	=	mh_object_signal_connect( ( MHObject * )dev, "ea_session_start", ea_session_start, NULL );
		stop_sig	=	mh_object_signal_connect( ( MHObject * )dev, "ea_session_stop", ea_session_stop, NULL );
		data_sig	=	mh_object_signal_connect( ( MHObject * )dev, "ea_session_data", ea_session_data, NULL );
		pb_sig		=	mh_object_signal_connect( ( MHObject * )dev, "ea_session_send_pb", ea_session_send_pb, NULL );
		bt_sig 		=	mh_object_signal_connect( ( MHObject * )dev, "bt_data", bt_data, NULL );

		mh_dev_register_detach_listener( dev, &_detach_listener );
	}
}

ANW_uint8 mAccessoryCarPlayUUID[] = { 0x48, 0x43, 0x88, 0xEC,
									  0x41, 0xCD,
									  0xA2, 0x40,
									  0x97, 0x27, 0x57, 0x5D, 0x50, 0xBF, 0x1F, 0xD3
									};

ANW_uint8 mAccessoryIAP2UUID[] =  { 0x00, 0x00, 0x00, 0x00,
								   0xCA, 0xDE,
								   0xDE, 0xFA,
								   0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF
								 };

//iPhone CarPlay UUID 0x2D8D2466E14D451C88BC7301ABEA291A
ANW_uint8 mAppleCarPlayUUID[] =  { 0x66, 0x24, 0x8d, 0x2d,
								   0x4d, 0xe1,
								   0x1c, 0x45,
								   0x88, 0xbc, 0x73, 0x01, 0xab, 0xea, 0x29, 0x1a
								 };
//iPhone iAP2 UUID,0xFECACADEAFDECADEDEFACADE00000000
ANW_uint8 mAppleIAP2UUID[] = { 0x00, 0x00, 0x00, 0x00,
							   0xCA, 0xDE,
							   0xDE, 0xFA,
							   0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFE
							 };

int PairStatusCallback(PAIRSTATUS_TYPE nType,unsigned int data,ANW_BD_ADDR* bd_addr,int cod)
{
	printf( "\033[1;32;40m PairStatusCallback(%d) \033[0m\n", nType);
	printf("\n\n\n\n%s--->nType:%d, data:%d, address:%2x%2x%2x%2x%2x%2x, cod:%d\n\n\n\n", 
			__func__, nType, data, bd_addr->bytes[0], bd_addr->bytes[1], bd_addr->bytes[2], bd_addr->bytes[3],
			bd_addr->bytes[4], bd_addr->bytes[5], cod);
	switch(nType)
	{
		case PAIR_INCOMING_REQUEST:
			g_bPairing = 1;
			ANW_DeviceInversePair(*bd_addr,"0000");
			break;
		case SSP_PAIR_INCOMING_REQUEST:
			g_bPairing = 1;
			ANW_DeviceSSPAccept(*bd_addr);
			break;
		case PAIR_STATUS:
			if(bd_addr)
			{
				if(data == 1)
					printf( "\033[1;32;40m Device pair success(%02x:%02x:%02x:%02x:%02x:%02x) \033[0m\n",bd_addr->bytes[5],bd_addr->bytes[4],bd_addr->bytes[3],bd_addr->bytes[2],bd_addr->bytes[1],bd_addr->bytes[0]);
				else
					printf( "\033[1;32;40m Device pair fail(%02x:%02x:%02x:%02x:%02x:%02x) \033[0m\n",bd_addr->bytes[5],bd_addr->bytes[4],bd_addr->bytes[3],bd_addr->bytes[2],bd_addr->bytes[1],bd_addr->bytes[0]);
			}
			else
			{
				if(data == 1)
					printf( "\033[1;32;40m Device pair success \033[0m\n");
				else
					printf( "\033[1;32;40m  Device pair fail \033[0m\n");
			}
			g_bPairing = 0;
			
			break;
	}

	return 1;
}

int ConnectStatusCallback(int nIndex,Connect_Channel Channel,int nStatus, ANW_BD_ADDR bd_addr)
{
	char szProfile[256];
	char szConnectStatus[256];
	szProfile[0]='\0';
	szConnectStatus[0] = '\0';
	if(nStatus == 1)
		sprintf(szConnectStatus,"connected");
	else
	{
	if(nStatus ==-1)
		sprintf(szConnectStatus,"disconnected : linklost");
	else
		sprintf(szConnectStatus,"disconnected");
	}
	switch(Channel)
	{
		case HF_CHANNEL: // HF
			if(nIndex>=0 && nIndex<2)
			g_HFPConnectStatus[nIndex] = nStatus;
			sprintf(szProfile,"HFP");
			break;
		case PIMDATA_CHANNEL: // Data
			sprintf(szProfile,"PIM");
			break;
		case AUDIO_STREAM_CHANNEL: // A2DP
			g_A2DPConnectStatus =  nStatus;
			sprintf(szProfile,"A2DP");
			break;
		case AUDIO_CONTROL_CHANNEL: // AVRCP
			sprintf(szProfile,"AVRCP");
			break;
		case SPP_CHANNEL: 
			mSearchCarPlayDev = 0;
			sprintf(szProfile,"SPP");
			break;			
		case HID_CONTROL_CHANNEL: 
			g_HIDConnectStatus = nStatus;
			sprintf(szProfile,"HID Control");
			break;			
		case HID_INTERRUPT_CHANNEL: 
			sprintf(szProfile,"HID interrupt");
			break;			
		case SCO_CHANNEL: 
			sprintf(szProfile,"SCO");
			if(nIndex>=0 && nIndex<2)
			g_SCOConnectStatus[nIndex]= nStatus;
			break;			
		case AVRCP_BROWSING_CHANNEL: 
			g_AVRCPBrowsingConnectStatus = nStatus;
			sprintf(szProfile,"AVRCP browsing");
			break;	
		case PHONEBOOK_CHANNEL: 
			sprintf(szProfile,"Phonebook");
			break;						
		case SMS_CHANNEL: 
			sprintf(szProfile,"SMS");
			break;	
		default:
			break;
	}
	printf( "\033[1;32;40m  %s (%02x:%02x:%02x:%02x:%02x:%02x) %s -index =%d \033[0m\n",szProfile,bd_addr.bytes[0],bd_addr.bytes[1],bd_addr.bytes[2],bd_addr.bytes[3],bd_addr.bytes[4],bd_addr.bytes[5],szConnectStatus,nIndex);

	return 1;
}
int ServiceConnectRequestCallBack(Connect_Channel nType,ANW_BD_ADDR bd_addr,ANW_CHAR* szdeviceName,int nIndex)
{
	printf( "\033[1;32;40m  nIndex = %d ServiceConnectRequestCallBack(%d) \033[0m\n",nIndex,nType);
	switch(nType)
	{
		case HF_CHANNEL:
			ANW_HFAcceptConnection(nIndex);
			break;
		case HID_CONTROL_CHANNEL:
			ANW_HidAccept();
			break;
		case AUDIO_STREAM_CHANNEL:
			ANW_AudioAcceptConnection(TYPE_A2DP);
			break;
		case AUDIO_CONTROL_CHANNEL:
			ANW_AudioAcceptConnection(TYPE_AVRCP);
			break;
	}
	return 1;
}

ANW_BOOL SPPEventCallBack(SPP_EVEN_TTYPE nEventType,void* pEventData,int nEventdataSize)
{
	switch(nEventType)
	{
		case SPP_EVENT_CONNECT_STATE:
			{
				SPP_CONNECTSTATE_EVENTDATA *pData =(SPP_CONNECTSTATE_EVENTDATA*) pEventData;
				if(pData->nIndex >=0 && pData->nIndex<4)
//				if( pData->nIndex == mCarPlayIndex )
				{
					memcpy(&g_SPPConnectInfo[pData->nIndex].bt_address,&pData->bt_address,sizeof(ANW_BD_ADDR));
					g_SPPConnectInfo[pData->nIndex].nConnectStatus = pData->nConnectStatus;
					printf( "\033[1;32;40m **SPP connect report :%02x:%02x:%02x:%02x:%02x:%02x index = %d Connect status = %d \033[0m\n",
					pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0],pData->nIndex,pData->nConnectStatus);
			
					if( pData->nConnectStatus == 0 )
					{
						int i,j,k = 0;
						int find_dev = 1;
						for( j = 0; j < devCount; j++ )	
						{
							for( i = 0; i < 6; i++ )// Detects whether the mac address is a connected device
							{
								if( pData->bt_address.bytes[i] != mIapMacAddr[j].bytes[i] )	
								{
									find_dev = 0;
									break;
								}
							}
							if( find_dev == 0 )
							{
								continue;	
							}else{
								k = j;
								break;
							}
						}

						if(( iap2 != NULL )&& ( find_dev == 1)&& ( mIapMacAddr[k].bytes[0] != 0))// send detach device 
						{
							memset( mIapMacAddr[k].bytes, 0, 6 );

							char _address[17];
							
							sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",
									pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0]);
							g_message(" _address = %s \n",_address );
					
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_BT_IAP;
							_devParam->mac_addr			=	g_strdup( _address );
							_devParam->connect_status	=	0;	

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );
						}
					}
					else
					{
						if( pData->nIndex == mIapIndex )
						{
							char _address[17];

							sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",
									pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0]);

							g_message(" _address = %s \n",_address );

							//Notify the mac address of the connected device
							MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
							_devParam->type				=	MH_DEV_BT_IAP;
							_devParam->mac_addr			=	g_strdup( _address );
							_devParam->connect_status	=	1;	//connect

							mh_core_find_dev( _devParam );

							g_free( _devParam->mac_addr );
							g_free( _devParam );
						}
						else
						{
							g_message(" index is error\n");
						}
					}

				}
			}
			break;
		case SPP_EVENT_DATA_IND:
			{
				SPP_DATAIND_EVENTDATA *pData = (SPP_DATAIND_EVENTDATA*) pEventData;
//				if(pData->nIndex >=0 && pData->nIndex<4 && pData->nDataSize >0)
				if( pData->nIndex == mIapIndex )//If the data is connected to the device, send data to mediahub
				{
					int i=0;
					char szTemp1[256];
					char szTemp2[256];
					szTemp1[0]='\0';
					szTemp2[0]='\0';
					printf( "\033[1;32;40m **SPP Receive data : index = %d ,size = %d \033[0m\n",pData->nIndex,pData->nDataSize);

					uint8_t * _buf;
					_buf = g_malloc0( pData->nDataSize );
					for(i=0 ;i<pData->nDataSize;i++)
					{
						sprintf(szTemp2,"%02x ",*(pData->pDataBuffer+i));
						strcat(szTemp1,szTemp2);
							_buf[i]	=	*(pData->pDataBuffer+i);
						if(i%16 ==0 && i!=0)
						{
							printf( "\033[1;32;40m szTemp1 = %s \033[0m\n", szTemp1);
							szTemp1[0]='\0';
						}	
					
					}
					printf( "\033[1;32;40m szTemp1 = %s \033[0m\n", szTemp1);
					g_message("spp callback *********************start\n");
					DEBUG_HEX_DISPLAY( _buf, pData->nDataSize);
					g_message("spp callback *********************end\n");
					mh_dev_write_bt_data(iap2, _buf, pData->nDataSize );
					g_free(_buf);

				}
			}
			break;
		case SPP_EVENT_CONNECT_REQUEST:
			{
				SPP_CONNECTREQUEST_EVENTDATA *pData = (SPP_CONNECTREQUEST_EVENTDATA*) pEventData;

				printf( "\033[1;32;40m **SPP channel connect with %s  %02x:%02x:%02x:%02x:%02x:%02x \033[0m\n",
						pData->szdeviceName,
						pData->bt_address.bytes[0],
						pData->bt_address.bytes[1],
						pData->bt_address.bytes[2],
						pData->bt_address.bytes[3],
						pData->bt_address.bytes[4],
						pData->bt_address.bytes[5]);

				int i,j;
				int connect = 1;
				// Detects whether the mac address is a connected device
				for( j = 0; j < devCount; j++ )	
				{
					for( i = 0; i < 6; i++ )
					{
						if( pData->bt_address.bytes[i] != mIapMacAddr[j].bytes[i] )	
						{
							connect = 0;
							break;
						}
					}
					if( connect == 0 )
					{
						continue;	
					}else{
						break;
					}
				}

				if( connect == 1 )
				{
					ANW_SPPAccept(pData->nIndex);	//spp connect
					mIapIndex	=	pData->nIndex;

//					char _address[17];
//					
//					sprintf( _address, "%02x:%02x:%02x:%02x:%02x:%02x",
//						pData->bt_address.bytes[5],pData->bt_address.bytes[4],pData->bt_address.bytes[3],pData->bt_address.bytes[2],pData->bt_address.bytes[1],pData->bt_address.bytes[0]);
//
//					g_message(" _address = %s \n",_address );
//
//					//Notify the mac address of the connected device
//					MHDevParam * _devParam 		= 	( MHDevParam * )g_new0( MHDevParam, 1 );
//					_devParam->type				=	MH_DEV_BT_IAP;
//					_devParam->mac_addr			=	g_strdup( _address );
//					_devParam->connect_status	=	1;	//connect
//
//					mh_core_find_dev( _devParam );
//
//					g_free( _devParam->mac_addr );
//					g_free( _devParam );
				}else{
					printf( "\033[1;32;40m **SPP channel connect Failed  \033[0m\n");
				}
			}
			break;
	}
	return TRUE;
}

unsigned char InquiryResponse(BLUETOOTH_DEVICE device, ANW_BOOL bComplete)
{
	if(bComplete ==0)
	{
		printf( "\033[1;32;40m InquiryResponse %s  %02x:%02x:%02x:%02x:%02x:%02x \033[0m\n",device.bd_name,
				device.bt_address.bytes[5],device.bt_address.bytes[4],device.bt_address.bytes[3],device.bt_address.bytes[2],device.bt_address.bytes[1],device.bt_address.bytes[0]);

		if( (device.EIRData.ad_mask & ANW_BT_ADV_BIT_SERVICE) != 0  )
		{	
			int i = 0;
			devCount	=	0;
			for (i = 0; i < device.EIRData.service_count; i++) 
			{
				int isEq2	=	0;

				//Check Apple CarPlay EIR
				if (( device.EIRData.service[i].service_type == ANW_SERVICE_128BITS )
						||  ( device.EIRData.service[i].service_type == ANW_SERVICE_SOL_128BITS )) {
					g_message ( "find service_type: %d\n", device.EIRData.service[i].service_type);

					if(( device.EIRData.service[i].service_uuid.uuid_128b.service_uuid	==	0x00000000 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data2	==	0xDECA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data3	==	0xFADE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[0]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[1]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[2]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[3]	==	0xAF )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[4]	==	0xDE )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[5]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[6]	==	0xCA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[7]	==	0xFE ))
					{
						g_message ("Find Apple iAP2 EIR:[%d] = %02x:%02x:%02x:%02x:%02x:%02x\n" ,i,
								device.bt_address.bytes[5],device.bt_address.bytes[4],device.bt_address.bytes[3],device.bt_address.bytes[2],device.bt_address.bytes[1],device.bt_address.bytes[0]);

						mIapMacAddr[devCount].bytes[0] = device.bt_address.bytes[0];
						mIapMacAddr[devCount].bytes[1] = device.bt_address.bytes[1];
						mIapMacAddr[devCount].bytes[2] = device.bt_address.bytes[2];
						mIapMacAddr[devCount].bytes[3] = device.bt_address.bytes[3];
						mIapMacAddr[devCount].bytes[4] = device.bt_address.bytes[4];
						mIapMacAddr[devCount].bytes[5] = device.bt_address.bytes[5];
						devCount++;
					}else{
						g_message ("Don't Find Apple iAP2 EIR\n");								
						g_message ("service_uuid = %08X\n", device.EIRData.service[i].service_uuid.uuid_128b.service_uuid);	
						g_message ("data2 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data2);
						g_message ("data3 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data3);
						g_message ("data4[0] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[0]);
						g_message ("data4[1] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[1]);
						g_message ("data4[2] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[2]);
						g_message ("data4[3] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[3]);
						g_message ("data4[4] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[4]);
						g_message ("data4[5] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[5]);
						g_message ("data4[6] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[6]);
						g_message ("data4[7] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[7]);
					}

					if(( device.EIRData.service[i].service_uuid.uuid_128b.service_uuid	==	0x2D8D2466 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data2	==	0xE14D )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data3	==	0x451C )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[0]	==	0x88 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[1]	==	0xBC )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[2]	==	0x73 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[3]	==	0x01 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[4]	==	0xAB )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[5]	==	0xEA )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[6]	==	0x29 )
							&& ( device.EIRData.service[i].service_uuid.uuid_128b.data4[7]	==	0x1A ))
					{
	
						g_message ("Find Apple carplay EIR:[%d] = %02x:%02x:%02x:%02x:%02x:%02x\n" ,i,
								device.bt_address.bytes[5],device.bt_address.bytes[4],device.bt_address.bytes[3],device.bt_address.bytes[2],device.bt_address.bytes[1],device.bt_address.bytes[0]);

//						mCarPlayMacAddr[devCount].bytes[0] = device.bt_address.bytes[0];
//						mCarPlayMacAddr[devCount].bytes[1] = device.bt_address.bytes[1];
//						mCarPlayMacAddr[devCount].bytes[2] = device.bt_address.bytes[2];
//						mCarPlayMacAddr[devCount].bytes[3] = device.bt_address.bytes[3];
//						mCarPlayMacAddr[devCount].bytes[4] = device.bt_address.bytes[4];
//						mCarPlayMacAddr[devCount].bytes[5] = device.bt_address.bytes[5];
//						devCount++;
					}else{
						g_message ("Don't Find Apple carplay EIR\n");								
						g_message ("service_uuid = %08X\n", device.EIRData.service[i].service_uuid.uuid_128b.service_uuid);	
						g_message ("data2 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data2);
						g_message ("data3 = %04X\n", device.EIRData.service[i].service_uuid.uuid_128b.data3);
						g_message ("data4[0] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[0]);
						g_message ("data4[1] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[1]);
						g_message ("data4[2] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[2]);
						g_message ("data4[3] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[3]);
						g_message ("data4[4] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[4]);
						g_message ("data4[5] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[5]);
						g_message ("data4[6] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[6]);
						g_message ("data4[7] = %02X \n", device.EIRData.service[i].service_uuid.uuid_128b.data4[7]);
					}
				}
			}
		}
	}
	else
	{
		printf( "\033[1;32;40m   Inquiry Completed \033[0m\n");
		g_bInquiry =0;
	}
	return 1;
}

int PL_BTPower(bool bOn)
{
	int nRet = Anw_SUCCESS ;

	if(bOn == true && g_bBTPowerStatus == false)
	{
		system("echo 0 > /sys/class/gpio/gpio449/value");
		system("echo 449 > /sys/class/gpio/unexport");
		usleep(500*1000);
		system("echo 449 > /sys/class/gpio/export");
		system("echo out > /sys/class/gpio/gpio449/direction");
		system("echo 1 > /sys/class/gpio/gpio449/value");
		printf("system cmd is finish\n");

		//BT Module Power on & initial
		nRet	=	ANW_ManagerInitialEx("/media/ivi-data/anw");

		if( nRet != Anw_SUCCESS )
		{
			printf("ANW_ManagerInitialEx is Failed!\n\n");
			return nRet;
		}

		nRet	=	ANW_DeviceInitial();

		if( nRet != Anw_SUCCESS )
		{
			printf("ANW_DeviceInitial is Failed!\n\n");
			return nRet;
		}

		ANW_EnableLog(1);

		g_bBTPowerStatus = true;
		//Set callback function
		ANW_SetConnectStatusCallBack(ConnectStatusCallback);
		ANW_SetPairStatusCallBack(PairStatusCallback);
		ANW_SetServiceConnectRequestCallBack(ServiceConnectRequestCallBack); 

		//Profile initial
		memset(&g_SPPConnectInfo,0,sizeof(SPP_CONNECTION)*4);
		{

			ANW_BT_ADV_DATA EIRData ;
			EIRData.ad_mask = ANW_BT_ADV_BIT_DEV_NAME | ANW_BT_ADV_BIT_SERVICE;
			EIRData.bt_name[0]	= 'H';
			EIRData.bt_name[1]	= 'S';
			EIRData.bt_name[1]	= '7';
			EIRData.service_count = 2;
			EIRData.service[0].service_type = ANW_SERVICE_128BITS;
	
			EIRData.service[0].service_uuid.uuid_128b.service_uuid	=	0x484388EC;
			EIRData.service[0].service_uuid.uuid_128b.data2	=	0x41CD;
			EIRData.service[0].service_uuid.uuid_128b.data3	=	0xA240;

			EIRData.service[0].service_uuid.uuid_128b.data4[0]	=	0x97;
			EIRData.service[0].service_uuid.uuid_128b.data4[1]	=	0x27;
			EIRData.service[0].service_uuid.uuid_128b.data4[2]	=	0x57;
			EIRData.service[0].service_uuid.uuid_128b.data4[3]	=	0x5D;
			EIRData.service[0].service_uuid.uuid_128b.data4[4]	=	0x50;
			EIRData.service[0].service_uuid.uuid_128b.data4[5]	=	0xBF;
			EIRData.service[0].service_uuid.uuid_128b.data4[6]	=	0x1F;
			EIRData.service[0].service_uuid.uuid_128b.data4[7]	=	0xD3;

			EIRData.service[1].service_uuid.uuid_128b.service_uuid	=	0x00000000;
			EIRData.service[1].service_uuid.uuid_128b.data2	=	0xCADE;
			EIRData.service[1].service_uuid.uuid_128b.data3	=	0xDEFA;

			EIRData.service[1].service_uuid.uuid_128b.data4[0]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[1]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[2]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[3]	=	0xAF;
			EIRData.service[1].service_uuid.uuid_128b.data4[4]	=	0xDE;
			EIRData.service[1].service_uuid.uuid_128b.data4[5]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[6]	=	0xCA;
			EIRData.service[1].service_uuid.uuid_128b.data4[7]	=	0xFF;

			ANW_SetEIRData(EIRData);

			ANW_SPPInit( SPPEventCallBack, mAccessoryCarPlayUUID, 16 );

			ANW_SPPAddService ( mAccessoryIAP2UUID, 16 );

			mSearchCarPlayDev++;
		}

		ANW_DeviceInquiry(InquiryResponse);

	}
	else if(bOn == false && g_bBTPowerStatus == true)
	{
		g_bRunPowerOffProcess = true;
		//Stop all processing
		if(g_bInquiry ==1)
		{
			ANW_DeviceInquiryStop();
			
			while(g_bInquiry ==1)
				sleep(1);
		}
		if(g_bPBDownloding[0] == 1 || g_bSMSDownloding[0] == 1 ||g_bPBDownloding[1] == 1 || g_bSMSDownloding[1] == 1)
		{			
			while(g_bPBDownloding[0] == 1 || g_bSMSDownloding[0] == 1 ||g_bPBDownloding[1] == 1 || g_bSMSDownloding[1] == 1)
				sleep(1);
		}		
		//Profile deinit
		ANW_SPPDenit();

		//BT module deinit & power of
		nRet = ANW_DeviceClose();
		g_bBTPowerStatus = false;
		g_bRunPowerOffProcess = false;
	}
	return nRet;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	g_message("\n\n\n\n     ea sample           \n\n\n\n\n");
	PL_BTPower(true);	//bt power on 

	MHEventsListener _eventListener	=	
	{
		.callback	=	event_arrived,
		.user_data	=	NULL
	};
	MHDevicesListener _devListener	=	
	{
		.callback	=	dev_event,
		.user_data	=	NULL
	};

	MHIPCConnection * _conn	=	mh_ipc_connection_create();
	mh_ipc_media_client_init( _conn );

	mh_core_register_events_listener( &_eventListener );
	mh_core_register_devices_listener( &_devListener );

	mh_misc_set_iap_device_mode( MISC_IAP );
	mh_misc_set_bluetoothids("C5:F5:B3:FD:FE:4C");
	mh_core_start();

	pause();

	return 0;
}				/* ----------  end of function main  ---------- */
