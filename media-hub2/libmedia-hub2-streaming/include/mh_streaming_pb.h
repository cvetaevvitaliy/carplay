/*
 * =====================================================================================
 *
 *       Filename:  mh_streaming_pb.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/15/2015 03:38:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MH_STREAMING_PB_H__
#define __MH_STREAMING_PB_H__
#include <mh_io.h>
#include <mh_streaming.h>
#include <mh_api.h>
/*
 * Type Macros
 */

#define MH_TYPE_STREAMING_PB \
	(mh_streaming_pb_get_type())
#define MH_STREAMING_PB(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), MH_TYPE_STREAMING_PB, MHStreamingPb))
#define MH_IS_STREAMING_PB(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), MH_TYPE_STREAMING_PB))
#define MH_STREAMING_PB_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), MH_TYPE_STREAMING_PB, MHStreamingPbClass))
#define MH_IS_STREAMING_PB_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), MH_TYPE_STREAMING_PB))
#define MH_STREAMING_PB_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MH_TYPE_STREAMING_PB, MHStreamingPbClass))

typedef struct _MHStreamingPb		MHStreamingPb;
typedef struct _MHStreamingPbClass	MHStreamingPbClass;

struct _MHStreamingPbClass
{
	MHIoClass parent_class;

	/* Class Ios */

	/* Class Methods */

	/* Class Properties */
};

typedef struct _pathInfo 
{
	gchar * path;
	gchar * name;
	MHItemType type;
} PathInfo;				/*  ----------  end of struct pathInfo  ---------- */ 

typedef struct _tagInfo {
	gchar * title;
	gchar * artist;
	gchar * album;
} TagInfo;

struct _MHStreamingPb
{
	MHIo parent;
	MHIPCConnection * connect;
	MHPlaylist * playlist;
	char * video_sink_name;
	char * audio_sink_name;

	guint * seq;
	gint index;
	gboolean shared;
};

/* used by MH_TYPE_STREAMING_PB */
GType mh_streaming_pb_get_type( void );

/* Ios List */

MHResult mh_streaming_add_media_mapping( const char * uri, gboolean shared );
MHResult mh_streaming_remove_media_mapping( const char * uri );

bool mh_streaming_pb_get_properties( MHStreamingPb * pb, const char * first_property_name, ...);
bool mh_streaming_pb_set_properties( MHStreamingPb * pb, const char * first_property_name, ...);
#endif

