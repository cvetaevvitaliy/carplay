#include <gtk/gtk.h>
#include <mh_api.h>

static gboolean
on_delete_event (GtkWidget *widget,
                 GdkEvent  *event,
                 gpointer   data)
{
  g_print ("delete event occurred\n");

  return TRUE;
}

/* GtkWidget is the storage type for widgets */
GtkWidget *window;
GtkWidget * btStop, * btPlay, * btPause, * btPrevious, * btNext;
GtkWidget * lxPlaylist, *lxFolder;
GtkWidget * lbMetadata, * lbPtime, * lbDuration;
//GtkWidget * boxGlobal, * boxLeft, * boxRight, * boxRTop, * boxRBottom;
GtkWidget * swFolder, * swPlaylist;
GtkWidget * grGlobal;
GtkWidget * abControls;
GtkWidget * scProgress;
GtkAdjustment * amRange;

MHPb * pb;
MHFilter * music_filter;
MHFilter * folder_filter;
MHFilter * all_filter;
MHFilter * movie_filter;
MHFilter * picture_filter;
static int folderCount;
static bool resumePlayed	=	false;
MHPlaylist * playlist;
MHItem ** folderItems;
MHDev * currentDev;

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_stop_clicked
 *  Description:  
 * =====================================================================================
 */
static void on_stop_clicked( GtkWidget * widget, gpointer user_data )
{
	mh_pb_stop( pb );
}		/* -----  end of static function on_stop_clicked  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_play_clicked
 *  Description:  
 * =====================================================================================
 */
static void on_play_clicked( GtkWidget * widget, gpointer user_data )
{
	mh_pb_play( pb );
}		/* -----  end of static function on_play_clicked  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_pause_clicked
 *  Description:  
 * =====================================================================================
 */
static void on_pause_clicked( GtkWidget * widget, gpointer user_data )
{
	mh_pb_pause( pb );
}		/* -----  end of static function on_pause_clicked  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_previous_clicked
 *  Description:  
 * =====================================================================================
 */
static void on_previous_clicked( GtkWidget * widget, gpointer user_data )
{
	mh_pb_previous( pb );
}		/* -----  end of static function on_previous_clicked  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_next_clicked
 *  Description:  
 * =====================================================================================
 */
static void on_next_clicked( GtkWidget * widget, gpointer user_data )
{
	mh_pb_next( pb );
}		/* -----  end of static function on_next_clicked  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  add_playlist_item
 *  Description:  
 * =====================================================================================
 */
static bool add_playlist_item( void * data, void * user_data )
{
	GtkWidget * _label;
	MHItemData * _data	=	( MHItemData * )data;

	_label	=	gtk_label_new( _data->name );
	gtk_widget_show( _label );
	gtk_list_box_insert( GTK_LIST_BOX( lxPlaylist ), _label, -1 );

	return false;
}		/* -----  end of static function add_playlist_item  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _detach_event
 *  Description:  
 * =====================================================================================
 */
static void _detach_event(	MHDev * dev, void * user_data)
{
	printf("sample.c:_detach_event\n");
}		/* -----  end of static function _detach_event  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _each_item
 *  Description:  
 * =====================================================================================
 */
static bool _each_item( MHItem * item, MHItemData * data, void * user_data )
{
	GtkWidget * _label;

	if( data->type == MH_ITEM_FOLDER )
		folderCount ++;

	_label	=	gtk_label_new( data->name );
	g_object_set( G_OBJECT( _label ), "halign", GTK_ALIGN_START, NULL );
	gtk_widget_show( _label );
	gtk_list_box_insert( GTK_LIST_BOX( lxFolder ), _label, -1 );

	return false;
}		/* -----  end of static function _each_item  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _remove_row
 *  Description:  
 * =====================================================================================
 */
static void _remove_row( GtkWidget * widget, gpointer user_data )
{
	GtkWidget * _listbox	=	GTK_WIDGET( user_data );

	gtk_container_remove( GTK_CONTAINER( _listbox ), widget );
}		/* -----  end of static function _remove_row  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _get_children
 *  Description:  
 * =====================================================================================
 */
static MHItem ** _get_children( MHFolder * self,MHFilter * filter, MHFolderPosition pos,int * count )
{
	MHItem ** _items	=	NULL;
	_items	=	mh_folder_get_children( self,filter, FOLDER_BEGIN, count, MH_ITEM_ORDER_BY_ALPHABET );
	int i	=	0;
	MHItemType _type;
	char * name;
	MHFolder * _root;

	gtk_container_foreach( GTK_CONTAINER( lxFolder ), _remove_row, lxFolder );

	mh_object_get_properties( ( MHObject * )currentDev, "base", &_root, NULL );

	if( _root != self )
	{
		GtkWidget * _label	=	gtk_label_new( ".." );

		g_object_set( G_OBJECT( _label ), "halign", GTK_ALIGN_START, NULL );

		gtk_widget_show( _label );

		gtk_list_box_insert( GTK_LIST_BOX( lxFolder ), _label, -1 );

		folderCount	=	1;
	}
	else
	{
		folderCount	=	0;
	}

	if( *count > 0 )
		mh_item_foreach( _items, * count, _each_item, NULL );

	return _items;
}		/* -----  end of static function _get_children  ----- */

typedef struct _info_dev_folder 
{
	MHDev * dev;
	MHFolder * folder;
} info_dev_folder;				/* ----------  end of struct info_dev_folder  ---------- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _first_file
 *  Description:  
 * =====================================================================================
 */
static gboolean _first_file(gpointer user_data)
{
	g_message("%s\n",__func__);
	info_dev_folder * _info 	=	(info_dev_folder *)user_data;
	int _count	=	-1;
	MHFolder * _folder;
	MHPlaylist * playlist;
	MHDev * dev;
	_folder	=	_info->folder;
	dev	=	_info->dev;
	mh_object_get_properties(( MHObject * )dev, "base", &_folder, NULL );;

	folderItems	=	_get_children( _folder, music_filter, FOLDER_BEGIN, & _count);

	playlist	=	mh_folder_create_playlist( _folder, music_filter, false);
	_count=-1;
	mh_object_get_properties(( MHObject * )playlist, "count", &_count, NULL );
	printf( "get playlist: %p, count is: %d\n", playlist, _count );

	mh_pb_play_by_list( pb, playlist, 0 );

	mh_playlist_foreach( playlist,0, -1, add_playlist_item,NULL);

	return FALSE;
}		/* -----  end of static function _first_file  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _folder_display
 *  Description:  
 * =====================================================================================
 */
static gboolean _folder_display( gpointer user_data)
{
	info_dev_folder * _info 	=	(info_dev_folder *)user_data;
	int _count	=	-1;
	MHFolder * _folder;
	MHPlaylist * playlist;
	MHDev * dev;
	_folder	=	_info->folder;
	dev	=	_info->dev;

	mh_object_get_properties(( MHObject * )dev, "base", &_folder, NULL );;

	folderItems	=	_get_children( _folder, music_filter, FOLDER_BEGIN, & _count);

	return FALSE;
}		/* -----  end of static function _folder_display  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _scan_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_event(MHDev * dev,  MHDevScanCbType scan_type, MHItemType item_type, void * data,void * user_data)
{
	static MHFolder * _folder;
	int _count	=	-1;
	
	if(scan_type	==	MH_DEV_FINISH )
	{
		info_dev_folder * user_data	=	g_new0( info_dev_folder,1);
		user_data->dev	=	dev;
		user_data->folder	=	_folder;
		GSource * _source	=	g_idle_source_new();
		GMainContext * _content	=	g_main_context_default();
	

		if(resumePlayed	==	FALSE)
		{
			g_message("----------------------->%s\n",__func__);
			g_source_set_callback(_source,_first_file,user_data,g_free);
					}
		else
		{
			g_source_set_callback( _source, _folder_display, user_data, g_free);
		}

		g_source_attach(_source,_content);

		g_source_unref( _source );

	}
	if(scan_type	==	MH_DEV_FIRST_FILE)
	{
		if(item_type	==	MH_ITEM_MUSIC )
		{
			_folder	=	(MHFolder *) data;
		}
	}
}		/* -----  end of static function _register_scan_callback  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _ppm_play
 *  Description:  
 * =====================================================================================
 */
static gboolean  _ppm_play( gpointer user_data)
{
	MHPlaylist * playlist	=	(MHPlaylist *)user_data;
	mh_playlist_foreach( playlist, 0, -1, add_playlist_item, NULL );

	mh_pb_play_by_list( pb, playlist, 0 );

	return FALSE;
}		/* -----  end of static function _ppm_play  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_attached
 *  Description:  
 * =====================================================================================
 */
static void dev_attached( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	char * _serial;
	MHDevDetachListener _detach_listener	=	
	{
		.callback	=	_detach_event,
		.user_data	=	NULL
	};
	MHDevEventsListener _event_listener		=
	{
		.callback	=	_dev_event,
		.user_data	=	NULL
	};

	currentDev	=	dev;

	mh_object_get_properties(( MHObject * )dev, "serial", &_serial, NULL );

	if( _serial != NULL )
	{
//		GtkWindow * _window	=	GTK_WINDOW( user_data );
//
//		gtk_window_set_title( _window, _serial );
	}

	playlist	=	mh_dev_restore_playlist( dev, "last_played" );

	if( playlist != NULL )
	{
		GSource * _source	=	g_idle_source_new();
		g_source_set_callback( _source, _ppm_play, playlist, NULL);
		GMainContext * _content	=	g_main_context_default();

		g_source_attach(_source,_content);
		
		g_source_unref( _source );

		resumePlayed	=	true;
	}

	mh_dev_register_detach_listener( dev, &_detach_listener );
	mh_dev_register_events_listener( dev, &_event_listener );

	mh_dev_start_scan( dev,SCAN_FOLDER );
}		/* -----  end of static function dev_attached  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _ptime_change
 *  Description:  
 * =====================================================================================
 */
static gboolean _ptime_change( gpointer user_data)
{
	MHPbInfoData * _info	=	(MHPbInfoData *)user_data;
	_info->time_info.current_time	/=	1000;
	int _sec, _min, _hour;
	char _buf[1024];
	_sec	=	_info->time_info.current_time % 60;
	_min	=	( _info->time_info.current_time / 60 ) % 60;
	_hour	=	_info->time_info.current_time / 3600;

	sprintf( _buf, "%02d:%02d:%02d", _hour, _min, _sec );
	gtk_label_set_text( GTK_LABEL( lbPtime ), _buf );

	gtk_adjustment_set_value( amRange, _info->time_info.current_time );

	return FALSE;
}		/* -----  end of static function _ptime_change  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _track_top
 *  Description:  
 * =====================================================================================
 */
static gboolean _track_top(gpointer user_data)
{
	MHPbInfoData * _info;
	_info = ( MHPbInfoData * )user_data;

	gtk_adjustment_set_upper( amRange, 0 );
	gtk_list_box_select_row( GTK_LIST_BOX( lxPlaylist ), gtk_list_box_get_row_at_index(
	GTK_LIST_BOX( lxPlaylist ), _info->track_info.index ));

	return FALSE;
}		/* -----  end of static function _track_top  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _duration
 *  Description:  
 * =====================================================================================
 */
static gboolean  _duration( gpointer user_data)
{
	MHPbInfoData * _info;
	int _sec, _min, _hour;
	char _buf[1024];

	_info = ( MHPbInfoData * )user_data;

		_info->time_info.duration	/=	1000;

		_sec	=	_info->time_info.duration % 60;
		_min	=	( _info->time_info.duration / 60 ) % 60;
		_hour	=	_info->time_info.duration / 3600;

		sprintf( _buf, "%02d:%02d:%02d", _hour, _min, _sec );

		gtk_label_set_text( GTK_LABEL( lbDuration ), _buf );

		gtk_adjustment_set_upper( amRange, _info->time_info.duration );

	return FALSE;
}		/* -----  end of static function _duration  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _info_tag
 *  Description:  
 * =====================================================================================
 */
static gboolean _info_tag(gpointer user_data)
{

	MHPbInfoData * _info;
	_info = ( MHPbInfoData * )user_data;
	char _buf[1024];
	sprintf( _buf, "%s\n%s\n%s", _info->tag_info.title, _info->tag_info.artist,
			   _info->tag_info.album );

			gtk_label_set_text( GTK_LABEL( lbMetadata ), _buf );
	return FALSE;
}		/* -----  end of static function _info_tag  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _pbs_callback
 *  Description:  
 * =====================================================================================
 */
void _pbs_callback (MHPb * pb, MHPbInfoType type, MHPbInfoData * pdata, void * user_data )
{
	MHPbInfoData * _info;
	int _sec, _min, _hour;
	char _buf[1024];

	if ( pdata != NULL )
	{
		_info = ( MHPbInfoData * )pdata;
	}
	MHPbInfoData * _user_data	=	g_new0(MHPbInfoData,1);
	switch( type )
	{
	case MH_PB_INFO_PTIME_CHANGE:
		{
			_user_data->time_info.current_time	=	_info->time_info.current_time;
			GSource * _source = g_idle_source_new();
			g_source_set_callback( _source, _ptime_change,_user_data,g_free);
			GMainContext * _content	=	g_main_context_default();
			g_source_attach( _source, _content);
			g_source_unref( _source );
		}
//		break;
//	case MH_PB_INFO_DURATION:
		{
			_user_data->time_info.current_time	=	_info->time_info.current_time;
	
			GSource * _source = g_idle_source_new();
			g_source_set_callback( _source, _duration,_user_data,NULL);
			GMainContext * _content	=	g_main_context_default();
			g_source_attach( _source, _content);
			g_source_unref( _source );
		}
		break;
	case MH_PB_INFO_TAG:
		{
			_user_data->tag_info.title	=	g_strdup(_info->tag_info.title);
			_user_data->tag_info.artist	=	g_strdup(_info->tag_info.artist);
			_user_data->tag_info.album	=	g_strdup(_info->tag_info.album);
			GSource * _source = g_idle_source_new();
			g_source_set_callback( _source, _info_tag,_user_data,NULL);
			GMainContext * _content	=	g_main_context_default();
			g_source_attach( _source, _content);
			g_source_unref( _source );

		}
		break;
	case MH_PB_INFO_EOS:
		break;
	case MH_PB_INFO_ERROR:
		break;
	case MH_PB_INFO_TRACK_TOP:
		{
			_user_data->track_info.index	=	_info->track_info.index;
			_user_data->track_info.uri		=	g_strdup(_info->track_info.uri);
			_user_data->track_info.name		=	g_strdup(_info->track_info.name);
			GSource * _source = g_idle_source_new();
			g_source_set_callback( _source, _track_top,_user_data,NULL);
			GMainContext * _content	=	g_main_context_default();
			g_source_attach( _source, _content);
			g_source_unref( _source );
		}

			break;
	default:
		break;
	}
}		/* -----  end of static function _pbs_callback  ----- */

void event_arrived( MHCore * core, MHCoreEvent event, void * user_data )
{
	MHPbEventsListener _listener =
	{
		.callback = _pbs_callback
	};

	switch( event )
	{
	case MH_CORE_STARTED:
		printf( "Media-Hub v2.0 has been started\n" );
		all_filter	=	NULL;
		music_filter	=	mh_filter_create("mp3;wma;aac;wav;ogg;mp2");
		movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");
		picture_filter	=	mh_filter_create("jpg;png;bmp;jpeg;jfif;jpe;ioc;gif;tiff;tif");
		folder_filter	=	mh_filter_create("");

		pb	=	mh_pb_create();
		_listener.user_data	=	pb;

		mh_pb_register_events_listener( pb, &_listener );

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

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_folder_row_selected
 *  Description:  
 * =====================================================================================
 */
static void on_folder_row_selected( GtkListBox * box, GtkListBoxRow * row, gpointer user_data )
{
	gint _index	=	gtk_list_box_row_get_index( row );
	gint _count	=	-1;
	MHFolder * _root, * _parent;
	MHFolder * _folder;

	if( folderItems != NULL )
	{
		if( _index < folderCount )
		{
			mh_object_get_properties(( MHObject * )folderItems[ _index ], "parent", &_parent, NULL );

			if( _index == 0 )
			{
				mh_object_get_properties(( MHObject * )currentDev, "base", &_root, NULL );

				if( _root == _parent )
				{
					_folder	=	( MHFolder * )folderItems[ _index ];
				}
				else
				{
					mh_object_get_properties(( MHObject * )folderItems[ _index ], "parent", &_parent, NULL );
					mh_object_get_properties(( MHObject * )_parent, "parent", &_folder, NULL );
				}
			}
			else
			{
				_folder	=	( MHFolder * )folderItems[ _index ];
			}

			g_free( folderItems );

			folderItems	=	_get_children( _folder, music_filter, FOLDER_BEGIN, &_count );
		}
		else
		{
			MHFolder * _parent	=	NULL;

			if( playlist != NULL )
				mh_object_unref(( MHObject * )playlist );

			mh_object_get_properties( ( MHObject * )folderItems[ _index ], "parent", &_parent, NULL );

			playlist	=	mh_folder_create_playlist( _parent, music_filter, false );

			gtk_container_foreach( GTK_CONTAINER( lxPlaylist ), _remove_row, lxPlaylist );

			mh_playlist_foreach( playlist, 0, -1, add_playlist_item, NULL );

			mh_pb_playlist_change(pb,playlist);

			mh_object_set_properties(( MHObject * )pb, "index", _index - folderCount , NULL);	
		}
	}
}		/* -----  end of static function on_folder_row_selected  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_playlist_row_selected
 *  Description:  
 * =====================================================================================
 */
static void on_playlist_row_selected( GtkListBox * box, GtkListBoxRow * row, gpointer user_data )
{
	gint _index =   gtk_list_box_row_get_index( row );

	mh_object_set_properties(( MHObject * )pb, "index", _index, NULL );
}		/* -----  end of static function on_playlist_row_selected  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  on_change_value
 *  Description:  
 * =====================================================================================
 */
static gboolean on_change_value( GtkRange * range, GtkScrollType scroll, gdouble value, gpointer user_data )
{
	mh_pb_seek( pb, value * 1000 );

	return FALSE;
}		/* -----  end of static function on_change_value  ----- */

int
main (int   argc,//
      char *argv[])
{
	g_message("----------------------->main\n");
	MHEventsListener _eventListener	=	
	{
		.callback	=	event_arrived,
		.user_data	=	NULL
	};
	MHDevicesListener _devListener	=	
	{
		.callback	=	dev_attached,
		.user_data	=	NULL
	};

  /* This is called in all GTK applications. Arguments are parsed
   * from the command line and are returned to the application.
   */
  gtk_init (&argc, &argv);

#ifdef USE_MH2_IPC
  mh_ipc_client_init();
#endif

  /* create a new window, and set its title */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (window), "Media-Hub v2.0 Player");
  gtk_window_set_default_size( GTK_WINDOW( window ), 800, 480 );

  g_signal_connect (window, "delete-event", G_CALLBACK (on_delete_event), NULL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  grGlobal	=	gtk_grid_new();
  gtk_container_add( GTK_CONTAINER( window ), grGlobal );
  g_object_set( G_OBJECT( grGlobal ), "row-spacing", 5, "column-spacing", 5, NULL );

  swFolder	=	gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swFolder), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(swFolder, 320, 480);
  gtk_grid_attach( GTK_GRID( grGlobal ), swFolder, 0, 0, 8, 12 );

  lxFolder	=	gtk_list_box_new();
  gtk_container_add( GTK_CONTAINER( swFolder ), lxFolder );

  g_signal_connect( lxFolder, "row-activated", G_CALLBACK( on_folder_row_selected ), NULL );

  swPlaylist	=	gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swPlaylist), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(swPlaylist, 480, 100);
  gtk_grid_attach( GTK_GRID( grGlobal ), swPlaylist, 8, 0, 12, 6 );

  lxPlaylist	=	gtk_list_box_new();
  gtk_container_add( GTK_CONTAINER( swPlaylist ), lxPlaylist );

  g_signal_connect( lxPlaylist, "row-activated", G_CALLBACK( on_playlist_row_selected ), NULL );

  lbMetadata	=	gtk_label_new( "This\nis\nMetadata" );
  g_object_set( G_OBJECT( lbMetadata ), "wrap", TRUE, NULL );
  gtk_grid_attach( GTK_GRID( grGlobal ), lbMetadata, 8, 6, 12, 4 );

  lbPtime	=	gtk_label_new( "00:00:00" );
  gtk_grid_attach( GTK_GRID( grGlobal ), lbPtime, 8, 10, 1, 1 );

  amRange	=	gtk_adjustment_new( 0, 0, 0, 1, 1, 1 );

  scProgress	=	gtk_scale_new( GTK_ORIENTATION_HORIZONTAL, amRange );
  g_signal_connect( G_OBJECT( scProgress ), "change-value", G_CALLBACK( on_change_value ), NULL );
  g_object_set( G_OBJECT( scProgress ), "draw-value", FALSE, NULL );
  gtk_grid_attach( GTK_GRID( grGlobal ), scProgress, 9, 10, 10, 1 );

  lbDuration	=	gtk_label_new( "00:00:00" );
  gtk_grid_attach( GTK_GRID( grGlobal ), lbDuration, 19, 10, 1, 1 );

  abControls	=	gtk_action_bar_new();
  gtk_grid_attach( GTK_GRID( grGlobal ), abControls, 8, 11, 12, 1 );

  btStop = gtk_button_new_with_label ("■");
  gtk_action_bar_pack_start( GTK_ACTION_BAR( abControls ), btStop );
  g_signal_connect (btStop, "clicked", G_CALLBACK (on_stop_clicked), NULL);
  btPlay = gtk_button_new_with_label ("▶");
  gtk_action_bar_pack_start( GTK_ACTION_BAR( abControls ), btPlay );
  g_signal_connect (btPlay, "clicked", G_CALLBACK (on_play_clicked), NULL);
  btPause = gtk_button_new_with_label ("||");
  gtk_action_bar_pack_start( GTK_ACTION_BAR( abControls ), btPause );
  g_signal_connect (btPause, "clicked", G_CALLBACK (on_pause_clicked), NULL);
  btPrevious = gtk_button_new_with_label ("←");
  gtk_action_bar_pack_start( GTK_ACTION_BAR( abControls ), btPrevious );
  g_signal_connect (btPrevious, "clicked", G_CALLBACK (on_previous_clicked), NULL);
  btNext = gtk_button_new_with_label ("→");
  gtk_action_bar_pack_start( GTK_ACTION_BAR( abControls ), btNext );
  g_signal_connect (btNext, "clicked", G_CALLBACK (on_next_clicked), NULL);

//  g_signal_connect_swapped (btQuit, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  
  gtk_widget_show_all (window);

  mh_core_register_events_listener( &_eventListener );
  mh_core_register_devices_listener( &_devListener );

  mh_core_start();
  /* All GTK applications must have a gtk_main(). Control ends here
   * and waits for an event to occur (like a key press or a mouse event),
   * until gtk_main_quit() is called.
   */
  gtk_main ();

  return 0;
}
