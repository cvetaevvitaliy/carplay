/*
 * =====================================================================================
 *
 *       Filename:  mh_carplay.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/29/2015 05:24:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_CARPLAY_H__
#define __MH_CARPLAY_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <mh_api.h>

typedef enum _MHCarplaySiriAction
{
	MH_CARPLAY_SIRI_PREWARM,
	MH_CARPLAY_SIRI_BUTTONDOWN,
	MH_CARPLAY_SIRI_BUTTONUP
} MHCarplaySiriAction;				/* ----------  end of enum MHCarplaySiriAction  ---------- */

typedef enum _MHCarplayResourceId
{
	MH_CARPLAY_RESOURCE_MAIN_SCREEN = 0,
	MH_CARPLAY_RESOURCE_MAIN_AUDIO
} MHCarplayResourceId;

typedef enum _MHCarplayTransferType
{
	MH_CARPLAY_TRANSFERTYPE_NOTAPP = 0,
	MH_CARPLAY_TRANSFERTYPE_TAKE,
	MH_CARPLAY_TRANSFERTYPE_UNTAKE,
	MH_CARPLAY_TRANSFERTYPE_BORROW,
	MH_CARPLAY_TRANSFERTYPE_UNBORROW
} MHCarplayTransferType;

typedef enum _MHCarplayTransferPriority
{
	MH_CARPLAY_TRANSFERPRIORITY_NOAPP = 0,
	MH_CARPLAY_TRANSFERPRIORITY_NICE_TO_HAVE,
	MH_CARPLAY_TRANSFERPRIORITY_USER_INIT
} MHCarplayTransferPriority;

typedef enum _MHCarplayResourceConstraint
{
	MH_CARPLAY_CONSTRAINT_NOAPP = 0,
	MH_CARPLAY_CONSTRAINT_ANYTIME,
	MH_CARPLAY_CONSTRAINT_USER_INIT,
	MH_CARPLAY_CONSTRAINT_NERVER
} MHCarplayResourceConstraint;

typedef enum _MHCarplayAppStateId
{
	MH_CARPLAY_APP_SPEECH = 0,
	MH_CARPLAY_APP_PHONECALL,
	MH_CARPLAY_APP_TURNTYTURN
} MHCarplayAppStateId;

typedef enum _MHCarplaySpeechMode
{
	MH_CARPLAY_APPMODE_SPEECH_NOAPP = 0,
	MH_CARPLAY_APPMODE_SPEECH_NONE,
	MH_CARPLAY_APPMODE_SPEECH_SPEAKING,
	MH_CARPLAY_APPMODE_SPEECH_RECOGNIZING_SPEECH
} MHCarplaySpeechMode;

typedef enum _MHCarplayAppMode
{
	MH_CARPLAY_APPMODE_NOAPP = 0,
	MH_CARPLAY_APPMODE_FALSE,
	MH_CARPLAY_APPMODE_TRUE
} MHCarplayAppMode;

typedef enum _MHCarplayMediaBtn
{
	MH_CARPLAY_BTN_PLAY = 1,
	MH_CARPLAY_BTN_PAUSE,
	MH_CARPLAY_BTN_NEXT,
	MH_CARPLAY_BTN_PREVIOUS,
	MH_CARPLAY_BTN_PLAY_PAUSE,	
	MH_CARPLAY_BTN_AC_HOME,
	MH_CARPLAY_BTN_AC_BACK
} MHCarplayMediaBtn;

typedef enum _MHCarplayPhoneKey
{
	MH_CARPLAY_PHONE_HOOK_SWITCH = 1,
	MH_CARPLAY_PHONE_FLASH,
	MH_CARPLAY_PHONE_DROP,
	MH_CARPLAY_PHONE_MUTE,	

	MH_CARPLAY_PHONE_KEY_ZERO = 8,
	MH_CARPLAY_PHONE_KEY_ONE,
	MH_CARPLAY_PHONE_KEY_TWO,
	MH_CARPLAY_PHONE_KEY_THREE,
	MH_CARPLAY_PHONE_KEY_FOUR,	
	MH_CARPLAY_PHONE_KEY_FIVE,
	MH_CARPLAY_PHONE_KEY_SIX,
	MH_CARPLAY_PHONE_KEY_SEVEN,
	MH_CARPLAY_PHONE_KEY_EIGHT,
	MH_CARPLAY_PHONE_KEY_NINE,
	MH_CARPLAY_PHONE_KEY_STAR,
	MH_CARPLAY_PHONE_KEY_POUND
} MHCarplayPhoneKey;

typedef enum _MHCarplayAudioTrkType
{
	MH_CARPLAY_AUDIO_TRK_UNKNOWN,
	MH_CARPLAY_AUDIO_TRK_SIRI,
	MH_CARPLAY_AUDIO_TRK_VOICE_CALL,
	MH_CARPLAY_AUDIO_TRK_MEDIA,
	MH_CARPLAY_AUDIO_TRK_ALT
} MHCarplayAudioTrkType;

void * carplay_stub_open_pcm_device();
bool carplay_stub_set_pcm_params( void * device, int sample_rate, int channel_num );
bool carplay_stub_read_pcm_data( void * device, uint8_t * buf, int number );
void carplay_stub_close_pcm_device( void * device );

void mh_dev_carplay_take_screen( MHDev * dev );
void mh_dev_carplay_untake_screen( MHDev * dev );
void mh_dev_carplay_borrow_screen( MHDev * dev );
void mh_dev_carplay_unborrow_screen( MHDev * dev );
void mh_dev_carplay_take_main_audio( MHDev * dev );
void mh_dev_carplay_untake_main_audio( MHDev * dev );
void mh_dev_carplay_borrow_main_audio( MHDev * dev );
void mh_dev_carplay_unborrow_main_audio( MHDev * dev );
void mh_dev_carplay_request_ui( MHDev * dev, const char * ui );
void mh_dev_carplay_request_siri_prewarm( MHDev * dev );
void mh_dev_carplay_request_siri_button_down( MHDev * dev );
void mh_dev_carplay_request_siri_button_up( MHDev * dev );
void mh_dev_carplay_set_night_mode( MHDev * dev, bool mode );
void mh_dev_carplay_send_signal_touch( MHDev * dev, int press, int x, int y );

void mh_dev_carplay_force_key_frame( MHDev * dev );
void mh_dev_carplay_change_resource_mode( MHDev * dev , const uint8_t * mode );
void mh_dev_carplay_change_app_mode( MHDev * dev , const uint8_t * mode );
void mh_dev_carplay_send_media_button( MHDev * dev, int btn );
void mh_dev_carplay_send_phone_button( MHDev * dev, int key );
void mh_dev_carplay_set_limited_ui( MHDev * dev, bool flag );

#ifdef __cplusplus
}
#endif

#endif

