
#ifndef __MH_CONTENTS_H__
#define __MH_CONTENTS_H__

#include <gio/gio.h>
#include <glib-object.h>
#include <mh_item.h>
#include <mh_music.h>
#include <mh_picture.h>
#include <mh_movie.h>
#include <semaphore.h>

/*
 * Type Macros
 */

#define MH_TYPE_CONTENTS \
	(mh_contents_get_type())
#define MH_CONTENTS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), MH_TYPE_CONTENTS, MHContents))
#define MH_IS_CONTENTS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), MH_TYPE_CONTENTS))
#define MH_CONTENTS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), MH_TYPE_CONTENTS, MHContentsClass))
#define MH_IS_CONTENTS_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), MH_TYPE_CONTENTS))
#define MH_CONTENTS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MH_TYPE_CONTENTS, MHContentsClass))

typedef struct _MHContents		MHContents;
typedef struct _MHContentsClass	MHContentsClass;

typedef struct _MHStmt MHStmt;

typedef struct _MHNode 
{
	guint device_id;
	gpointer name;
	guint size;
	guint c_time;
	guint m_time;
	guint type;
} MHNode;				/* ----------  end of struct MHNode  ---------- */

typedef struct _MHClosure 
{
	guint descendant;
	guint ancestor;
	guint parent;
} MHClosure;				/* ----------  end of struct MHClosure  ---------- */


typedef enum _MHDev_type
{
		MH_DEV_USB,
		MH_DEV_IPOD,

		MH_DEV_MAX
} MHDev_type;				/*  ----------  end of enum MHDev_type  ---------- */ 
typedef struct _indexer_element_t 
{
	MHItem * item;
	gboolean flag;
} indexer_element_t;	
typedef enum _MHDev_exist
{
	MH_DEV_EXIST,
	MH_DEV_NOT_EXIST
} MHDev_exist;				/* ----------  end of enum MHDev_exist  ---------- */

/* Ios List */
struct _MHContentsClass
{
	GObjectClass parent_class;

	/* Class Ios */

	/* Class Methods */
	MHContents * (* instance)( );
	gint64 (* get_device)( MHContents *, char *);
	gint64 (* add_device)( MHContents *, char *);
	gpointer (* get_device_private)(MHContents *, gint64 ,int * );
	gint64 (* update_device_private)(MHContents *, gint64, gpointer, guint);
	gint64 (* get_id_by_tagId)(MHContents *, gint64, gint64);
	void (* delete_node_by_device_id)(MHContents *, gint64);
	void (* delete_playlist_by_device_id)( MHContents *, gint64);	
	gint64 (* add_node)( MHContents *, gint64 , MHItem * );
	void (* add_closure)( MHContents *, gint64 descendant, gint64 ancestor, gint64 parent, gint64 device_id);//del db by deviceId
	void (* add_music)( MHContents *, MHMusic * , gint64 device_id);//del db by deviceId
	void (* add_movie)( MHContents *, MHMovie *, gint64 device_id);//del db by deviceId
	void (* add_picture)( MHContents *, MHPicture *, gint64 device_id);//del db by deviceId

	void (* begin_transaction)( MHContents * );
	void (* commit_transaction)( MHContents * );
	void (* rollback_transaction)( MHContents * );
	MHStmt * (* prepare)( MHContents *, char * sql );
	gboolean (* step)( MHContents *,MHStmt *, va_list );
	void (* release)(MHContents *, MHStmt *);
	void (* del_node)(MHContents *, MHItem *);

	gpointer (* restore_playlist )( MHContents *, gint64, guint *);
	char ** (* get_data_names)( MHContents *, MHDev *);
	void ( * update_node)(MHContents * ,MHItem * );
	void ( * update_error_file)( MHContents *, gint64 , MHItemValid );
	void (* bind_int64)( MHContents* , MHStmt *, uint32_t, gint64);
	void (* reset)( MHContents * ,MHStmt *);

	gint64 (* save_playlist)( MHContents *, MHDev *, const char *, gint64, gpointer, guint64 );
	void (* update_playlist)( MHContents *, MHDev *, gint64, const char *, gint64, gpointer, guint64);
	void (* delete_playlist)( MHContents *, gint64);
	gint64 (* get_playlistid_by_tagId)( MHContents *, gint64);
	gboolean (* deinit)( MHContents *);
	void (* db_restore )( MHContents *);
	void (* delete_device )( MHContents *, gint64);
	gint64 * (* get_descendant )(MHContents *, gint64 , uint32_t *);
	gint64 * (* get_ancestor)(MHContents * , gint64 , uint32_t *);

	void (*update_music)( MHContents *, gint64, MHMusic *);
	void (*update_movie)( MHContents *, gint64, MHMovie *);
	void (*update_picture)( MHContents *, gint64, MHPicture *);

	
	/* Class Properties */
};

typedef enum _MHContentsResult
{
	MH_CONTENTS_OK,
	MH_CONTENTS_INVALID_PARA,
	MH_CONTENTS_STEP_CONTINUE,
	MH_CONTENTS_STEP_FINISH
} MHContentsResult;				/* ----------  end of enum MHContentsResult  ---------- */

struct _MHContents
{
	GObject parent;

	/* Instance Members */
};
#define SQL_CMD_MAX        (4096)
#define INDEXER_MUSIC	 (0x0001u)
#define INDEXER_MOVIE	 (0x0002u)
#define INDEXER_PICTURE	 (0x0004u)
/* used by MH_TYPE_CONTENTS */
GType mh_contents_get_type( void );

MHContents * mh_contents_instance();
gint64 mh_contents_get_device( MHContents * self, char *  serial_number);
gint64 mh_contents_add_device(MHContents * self, char *  serial_number );
gint64 mh_contents_update_device_private(MHContents * self, gint64 device_id, gpointer private_data, guint size);
gpointer mh_contents_get_device_private(MHContents * self, gint64 device_id, int * size );
gint64 mh_contents_get_id_by_tagId(MHContents * self, gint64 device_id,gint64 tagId);


gint64 mh_contents_add_node(MHContents * self, gint64 device_id, MHItem * item);
void mh_contents_add_closure(MHContents * self, gint64 descendant, gint64 ancestor, gint64 parent, gint64 device_id);//del db by deviceId
void mh_contents_add_music(MHContents * self, MHMusic * music, gint64 device_id);//del db by deviceId
void mh_contents_add_movie(MHContents * self, MHMovie * movie, gint64 device_id);//del db by deviceId
void mh_contents_add_picture(MHContents * self, MHPicture * picture, gint64 device_id);//del db by deviceId
void mh_contents_begin_transaction( MHContents * self );
void mh_contents_commit_transaction( MHContents * self );
void mh_contents_rollback_transaction( MHContents * self );

MHStmt * mh_contents_prepare( MHContents * self, char *sql );
MHContentsResult mh_contents_step( MHContents * self, MHStmt * statement,... );
void mh_contents_release( MHContents * self, MHStmt * statement );
void mh_contents_bind_int64( MHContents * self, MHStmt * statement,uint32_t index, gint64 data);
void mh_contents_reset( MHContents * self, MHStmt * statement);


gpointer mh_contents_restore_playlist( MHContents * self, gint64 playlist_id, guint * size);
char ** mh_contents_get_data_names( MHContents * self, MHDev * dev );
void mh_contents_update_node( MHContents * self, MHItem *);
void mh_contents_update_error_file( MHContents * self, gint64 id, MHItemValid error_flag);


gint64 mh_contents_save_playlist( MHContents *self, MHDev * dev, const char *name, gint64 tag_id, gpointer data, guint64 size);
void mh_contents_update_playlist( MHContents * self, MHDev * dev, gint64 playlist_id, const char * name, gint64 tag_id, gpointer data, guint64 size);

gint64 mh_contents_get_playlistid_by_tagId(MHContents * self, gint64 tagId);

gboolean mh_contents_deinit(MHContents * self );
void mh_contents_db_restore( MHContents * self );

void mh_contents_delete_device( MHContents * self, gint64 device_id);
void mh_contents_delete_node_by_device_id(MHContents * self, gint64 device_id);
void mh_contents_delete_playlist_by_device_id( MHContents * self, gint64 device_id);
void mh_contents_del_node( MHContents * self, MHItem * item);

void mh_contents_delete_playlist( MHContents * self, gint64 playlist_id);
gint64 * mh_contents_get_descendant( MHContents * self, gint64 node_id, uint32_t * count);
gint64 * mh_contents_get_ancestor( MHContents * self, gint64 node_id, uint32_t * count);
void mh_contents_update_music( MHContents * self, gint64 node_id, MHMusic * music);
void mh_contents_update_movie( MHContents * self, gint64 node_id, MHMovie * movie);
void mh_contents_update_picture( MHContents * self, gint64 node_id, MHPicture * picture);
#endif /* __MH_CONTENTS_H__ */
