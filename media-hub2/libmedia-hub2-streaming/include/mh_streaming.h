/*
 * =====================================================================================
 *
 *       Filename:  mh_streaming.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/13/2015 05:18:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_STREAMING_H__
#define __MH_STREAMING_H__
#include <mh_api.h>

typedef struct _MHStreamingPb MHStreamingPb;

void mh_ipc_streaming_client_init( MHIPCConnection * conn );
void mh_ipc_start_streaming_engine( MHIPCServer * server );

void mh_streaming_pb_next( MHStreamingPb * pb );
void mh_streaming_pb_previous( MHStreamingPb * pb );
MHStreamingPb * mh_streaming_pb_create( MHIPCConnection *conn);

bool mh_streaming_pb_get_properties( MHStreamingPb * pb, const char * first_property_name, ...);
bool mh_streaming_pb_set_properties( MHStreamingPb * pb, const char * first_property_name, ...);

bool mh_streaming_pb_play_by_list( MHStreamingPb * pb, MHPlaylist * playlist, uint32_t index );
void mh_streaming_pb_register_events_listener( MHStreamingPb * self, MHPbEventsListener * listener );
void mh_streaming_pb_register_status_listener( MHStreamingPb * self, MHPbStatusListener * listener );

void mh_streaming_pb_stop( MHStreamingPb * pb );
void mh_streaming_pb_play( MHStreamingPb * pb );
void mh_streaming_pb_pause( MHStreamingPb * pb );
void mh_streaming_pb_forward( MHStreamingPb * pb );
void mh_streaming_pb_backward( MHStreamingPb * pb );
void mh_streaming_pb_seek( MHStreamingPb * pb, uint32_t second );
void mh_streaming_pb_playlist_change( MHStreamingPb * pb, MHPlaylist * playlist );
void mh_streaming_pb_resize( MHStreamingPb * pb, uint32_t offsetx, uint32_t offsety, uint32_t width, uint32_t height );
void mh_streaming_pb_unref( MHStreamingPb * pb);
#endif

