/*
 * =====================================================================================
 *
 *       Filename:  mh_carlife.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/07/2016 12:37:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_CARLIFE_H__
#define __MH_CARLIFE_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <mh_api.h>

typedef enum _MHCarlifeHardkey
{
	MH_CARLIFE_HARDKEY_HOME	=	0x00000001,
	MH_CARLIFE_HARDKEY_PHONE_CALL	=	0x00000002,
	MH_CARLIFE_HARDKEY_PHONE_END	=	0x00000003,
	MH_CARLIFE_HARDKEY_HFP	=	0x00000005,
	MH_CARLIFE_HARDKEY_SELECTOR_NEXT	=	0x00000006,
	MH_CARLIFE_HARDKEY_SELECTOR_PREVIOUS	=	0x00000007,
	MH_CARLIFE_HARDKEY_MEDIA	=	0x00000009,
	MH_CARLIFE_HARDKEY_NAVI	=	0x0000000B,
	MH_CARLIFE_HARDKEY_BACK	=	0x0000000E,
	MH_CARLIFE_HARDKEY_SEEK_SUB	=	0x0000000F,

	MH_CARLIFE_HARDKEY_SEEK_ADD	=	0x00000010,
	MH_CARLIFE_HARDKEY_MUTE	=	0x00000013,
	MH_CARLIFE_HARDKEY_OK	=	0x00000014,
	MH_CARLIFE_HARDKEY_MOVE_LEFT	=	0x00000015,
	MH_CARLIFE_HARDKEY_MOVE_RIGHT	=	0x00000016,
	MH_CARLIFE_HARDKEY_MOVE_UP	=	0x00000017,
	MH_CARLIFE_HARDKEY_MOVE_DOWN	=	0x00000018,
	MH_CARLIFE_HARDKEY_MOVE_UP_LEFT	=	0x00000019,
	MH_CARLIFE_HARDKEY_MOVE_UP_RIGHT	=	0x0000001A,
	MH_CARLIFE_HARDKEY_MOVE_DOWN_LEFT	=	0x0000001B,
	MH_CARLIFE_HARDKEY_MOVE_DOWN_RIGHT	=	0x0000001C,
	MH_CARLIFE_HARDKEY_TEL	=	0x0000001D,
	MH_CARLIFE_HARDKEY_MAIN	=	0x0000001E,
	MH_CARLIFE_HARDKEY_MEDIA_START	=	0x0000001F,

	MH_CARLIFE_HARDKEY_MEDIA_STOP	=	0x00000020,
	MH_CARLIFE_HARDKEY_VR_START	=	0x00000021,
	MH_CARLIFE_HARDKEY_VR_STOP	=	0x00000022,
	MH_CARLIFE_HARDKEY_NUMBER_0	=	0x00000023,
	MH_CARLIFE_HARDKEY_NUMBER_1	=	0x00000024,
	MH_CARLIFE_HARDKEY_NUMBER_2	=	0x00000025,
	MH_CARLIFE_HARDKEY_NUMBER_3	=	0x00000026,
	MH_CARLIFE_HARDKEY_NUMBER_4	=	0x00000027,
	MH_CARLIFE_HARDKEY_NUMBER_5	=	0x00000028,
	MH_CARLIFE_HARDKEY_NUMBER_6	=	0x00000029,
	MH_CARLIFE_HARDKEY_NUMBER_7	=	0x0000002A,
	MH_CARLIFE_HARDKEY_NUMBER_8	=	0x0000002B,
	MH_CARLIFE_HARDKEY_NUMBER_9	=	0x0000002C,
	MH_CARLIFE_HARDKEY_NUMBER_STAR	=	0x0000002D,
	MH_CARLIFE_HARDKEY_NUMBER_POUND	=	0x0000002E,
	MH_CARLIFE_HARDKEY_NUMBER_DEL	=	0x0000002F,
	MH_CARLIFE_HARDKEY_NUMBER_CLEAR	=	0x00000030,
	MH_CARLIFE_HARDKEY_NUMBER_PLUS	=	0x00000031,
} MHCarlifeHardkey;				/* ----------  end of enum MHCarlifeHardkey  ---------- */

typedef struct _MHCarVelocity 
{
	uint32_t speed;
	uint64_t timeStamp;
} MHCarlifeCarVelocity;				/* ----------  end of struct MHCarVelocity  ---------- */

typedef struct _MHCarlifeCarGPS 
{
	uint32_t antennaState;
	uint32_t signalQuality;
	uint32_t latitude;
	uint32_t longitude;
	uint32_t height;

	uint32_t speed;
	uint32_t heading;
	uint32_t year;
	uint32_t month;
	uint32_t day;

	uint32_t hour;
	uint32_t min;
	uint32_t sec;
	uint32_t fix;
	uint32_t hdop;

	uint32_t pdop;
	uint32_t vdop;
	uint32_t satsUsed;
	uint32_t satsVisible;
	uint32_t horPosError;

	uint32_t vertPosError;
	uint32_t northSpeed;
	uint32_t eastSpeed;
	uint32_t vertSpeed;

	uint64_t timeStamp;
	
} MHCarlifeCarGPS;				/* ----------  end of struct MHCarlifeCarGPS  ---------- */

typedef struct _MHCarlifeCarGyroscope 
{
	int32_t gyroType;
	double gyroX;
	double gyroY;
	double gyroZ;
	uint64_t timeStamp;
} MHCarlifeCarGyroscope;				/* ----------  end of struct MHCarlifeCarGyroscope  ---------- */

typedef struct _MHCarlifeCarAcceleration 
{
	double accX ;
	double accY ;
	double accZ ;
	uint64_t timeStamp ;

} MHCarlifeCarAcceleration;				/* ----------  end of struct MHCarlifeCarAcceleration  ---------- */

typedef struct _MHCarlifeCarOil 
{
	int32_t level;
	int32_t range;
	bool lowFullWarning;
	//reserved
} MHCarlifeCarOil;				/* ----------  end of struct MHCarlifeCarOil  ---------- */


typedef enum _MHCarlifeLaunchMode
{
	CARLIFE_LAUNCH_NULL, 	//do nothing
	CARLIFE_LAUNCH_NORMAL, //home
	CARLIFE_LAUNCH_PHONE,
	CARLIFE_LAUNCH_MAP,
	CARLIFE_LAUNCH_MUSIC
} MHCarlifeLaunchMode;				/* ----------  end of enum MHCarlifeLaunchMode  ---------- */

typedef struct _MHCarlifeCarInfo 
{
	char * os;
	char * board ;
	char * bootloader ;
	char * brand;
	char * cpu_abi;
	char * cpu_abi2 ;
	char * device ;
	char * display ;
	char * fingerprint ;
	char * hardware ;
	char * host ;
	char * cid ;
	char * manufacturer ;
	char * model ;
	char * product ;
	char * serial;
	char * codename ;
	char * incremental;
	char * release ;
	char * sdk ;
	uint32_t sdk_int;
	char * token;

	char * btaddress;
} MHCarlifeCarInfo;				/* ----------  end of struct MHCarlifeCarInfo  ---------- */

MHResult mh_dev_carlife_send_hardkey( MHDev * dev, MHCarlifeHardkey key);
MHResult mh_dev_carlife_video_start( MHDev * dev);
MHResult mh_dev_carlife_video_pause( MHDev * dev);
MHResult mh_dev_carlife_video_reset( MHDev * dev);
MHResult mh_dev_carlife_car_velocity( MHDev * dev, MHCarlifeCarVelocity * velocity);
MHResult mh_dev_carlife_car_GPS( MHDev * dev, MHCarlifeCarGPS * gps);
MHResult mh_dev_carlife_car_gyroscope( MHDev * dev, MHCarlifeCarGyroscope * gyroscope);
MHResult mh_dev_carlife_car_acceleration( MHDev *dev, MHCarlifeCarAcceleration * acceleration);
MHResult mh_dev_carlife_car_oil( MHDev * dev, MHCarlifeCarOil * oil);
MHResult mh_dev_carlife_launch_mode( MHDev * dev, MHCarlifeLaunchMode mode);

typedef enum _MHCarlifeSampleFormat
{
	CARLIFE_SAMPLE_S16LE
} MHCarlifeSampleFormat;				/* ----------  end of enum MHCarlifeSampleFormat  ---------- */

void * mh_dev_carlife_open_pcm_device( MHCarlifeSampleFormat format, uint32_t sample_rate, uint32_t channel);
int mh_dev_carlife_read_pcm_data( void * device,  char * buf, uint32_t len);
void mh_dev_carlife_close_pcm_device( void * device);

MHResult mh_dev_carlife_car_info( MHDev * dev, MHCarlifeCarInfo * info);
//////////////bt///////////////

typedef struct _MHCarlifeBtStartIdentifyReq 
{
	char * address;
} MHCarlifeBtStartIdentifyReq;				/* ----------  end of struct MHCarlifeBtStartIdenReq  ---------- */

MHResult mh_dev_carlife_bt_start_identify_req( MHDev *dev, MHCarlifeBtStartIdentifyReq * bt_address );

typedef struct _MHCarlifeBtPairInfo 
{	
	char * address;
	char * passKey;
	char * hash;
	char * randomizer;
	char * uuid;
	char * name;
	uint32_t status;
} MHCarlifeBtPairInfo;				/* ----------  end of struct MHCarlifeBtPairInfo  ---------- */

MHResult mh_dev_carlife_send_bt_pair_info( MHDev * dev, MHCarlifeBtPairInfo * info);

typedef enum _MHCarlifeBtHfpRequestType
{
	CARLIFE_REQUEST_START_CALL	=	1,
	CARLIFE_REQUEST_TERMINATE_CALL,
	CARLIFE_ANSWER_CALL,
	CARLIFE_REJECT_CALL,
	CARLIFE_DTMF_CODE,
	CARLIFE_MUTE_MIC
} MHCarlifeBtHfpRequestType;				/* ----------  end of enum MHCarlifeBtHfpRequestType  ---------- */

typedef struct _MHCarlifeBtHfpRequest 
{
	MHCarlifeBtHfpRequestType type;
	char * phoneNum;
	int32_t dtmfCode;
} MHCarlifeBtHfpRequest;				/* ----------  end of struct MHCarlifeBtHfpRequest  ---------- */

typedef void (* carlife_bt_hfp_request_cb)(MHDev * dev, MHCarlifeBtHfpRequest * request, void * user_data);

typedef struct _MHCarlifeBtHfpRequestListener 
{
	carlife_bt_hfp_request_cb callback;
	void * user_data;
} MHCarlifeBtHfpRequestListener;				/* ----------  end of struct MHCarlifeBtHfpRequestListener  ---------- */


MHResult mh_dev_register_bt_hfp_request( MHDev * dev, MHCarlifeBtHfpRequestListener * listener);


typedef enum _MHCarlifeBtHfpIndicationType
{
	CARLIFE_BT_INDICATION_NULL,
	CARLIFE_BT_INDICATION_INCOMMING_CALL = 1,
	CARLIFE_BT_INDICATION_OUTGOING_CALL,
	CARLIFE_BT_INDICATION_CALL_ACTIVE,
	CARLIFE_BT_INDICATION_CALL_INACTIVE,
	CARLIFE_BT_INDICATION_MULTICALL_ACTIVE,

} MHCarlifeBtHfpIndicationType;				/* ----------  end of enum MHCarlifeBtHfpIndicationType  ---------- */

typedef struct _MHCarlifeBtHfpIndication 
{
	MHCarlifeBtHfpIndicationType type;
	char * phoneNum;
	char * name;
	char * address;
	
} MHCarlifeBtHfpIndication;				/* ----------  end of struct MHCarlifeBtHfpIndication  ---------- */

MHResult mh_dev_carlife_send_bt_hfp_indication( MHDev * dev, MHCarlifeBtHfpIndication * indication);


typedef enum _MHCarlifeBtHfpConnType
{
	CARLIFE_BT_HFP_DISCONNECTED,  
	CARLIFE_BT_HFP_CONNECTING,
	CARLIFE_BT_HFP_CONNECTED
} MHCarlifeBtHfpConnType;				/* ----------  end of enum MHCarlifeBtHfpConnType  ---------- */

typedef struct _MHCarlifeBtHfpConnection 
{
	MHCarlifeBtHfpConnType type;
	char * address;
	char * name;
} MHCarlifeBtHfpConnection;				/* ----------  end of struct MHCarlifeBtHfpConnection  ---------- */

MHResult mh_dev_carlife_send_bt_hfp_connection( MHDev * dev, MHCarlifeBtHfpConnection * connection); 


typedef enum _MHCarlifeBtHfpResponseStatus
{
	BT_HFP_RESPONSE_FAIL	=	0,
	BT_HFP_RESPONSE_SUCCESS
} MHCarlifeBtHfpResponseStatus;				/* ----------  end of enum MHCarlifeBtHfpResponseStatus  ---------- */

typedef enum _MHCarlifeBtHfpResponseCmd
{
	BT_HFP_PLACE_OUTGOING_CALL	=	1,
	BT_HFP_TERMINATE_CALL,
	BT_HFP_ANSWER_CALL,
	BT_HFP_REJECT_CALL,
	BT_HFP_SEND_DTMP,
	BT_HFP_MUTE_MIC,
	BT_HFP_UNMUTE_MIC

} MHCarlifeBtHfpResponseCmd;				/* ----------  end of enum MHCarlifeBtHfpResponseCmd  ---------- */

typedef struct _MHCarlifeBtHfpResponse 
{
	MHCarlifeBtHfpResponseStatus status;
	MHCarlifeBtHfpResponseCmd cmd;
	uint32_t dtmfCode;
	
} MHCarlifeBtHfpResponse;				/* ----------  end of struct MHCarlifeBtHfpResponse  ---------- */

MHResult mh_dev_carlife_send_bt_hfp_response( MHDev * dev, MHCarlifeBtHfpResponse * response);


#ifdef __cplusplus
}
#endif

#endif
