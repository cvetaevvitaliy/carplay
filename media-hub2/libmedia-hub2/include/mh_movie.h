/*
 * Generated by plugin-codegen.
 */
#ifndef __MH_MOVIE_H__
#define __MH_MOVIE_H__

#include <gio/gio.h>
#include <mh_item.h>

/*
 * Type Macros
 */

#define MH_TYPE_MOVIE \
	(mh_movie_get_type())
#define MH_MOVIE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), MH_TYPE_MOVIE, MHMovie))
#define MH_IS_MOVIE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), MH_TYPE_MOVIE))
#define MH_MOVIE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), MH_TYPE_MOVIE, MHMovieClass))
#define MH_IS_MOVIE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), MH_TYPE_MOVIE))
#define MH_MOVIE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MH_TYPE_MOVIE, MHMovieClass))

typedef struct _MHMovie		MHMovie;
typedef struct _MHMovieClass	MHMovieClass;

struct _MHMovieClass
{
	MHItemClass parent_class;

	/* Class Ios */

	/* Class Methods */

	/* Class Properties */
};

struct _MHMovie
{
	MHItem parent;

	/* Instance Members */
	gchar * title;
	guint year;
	gchar * director;
	guint duration;
	gchar * genre;
	guint fps;
	guint xres;
	guint yres;
	gchar * language;


};

/* used by MH_TYPE_MOVIE */
GType mh_movie_get_type( void );

/* Ios List */



#endif /* __MH_MOVIE_H__ */
