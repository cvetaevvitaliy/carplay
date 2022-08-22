/*
 * =====================================================================================
 *
 *       Filename:  sample.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/17/2014 03:44:32 PM
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
#include <mh_api.h>
#include <glib.h>
#include <unistd.h>
#include <ilm/ilm_control.h>
#include <ilm_common.h>
MHDev * global_dev ;
static GMainLoop *test_loop;
MHFilter * _movie_filter;
MHPb * _pb;
MHPlaylist * playlist	=	NULL;
t_ilm_uint layer;
t_ilm_uint screen;
static int number_of_surfaces;
t_ilm_uint screenWidth;
t_ilm_uint screenHeight;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _detach_event
 *  Description:  
 * =====================================================================================
 */
static void _detach_event(	MHDev * dev, void * user_data)
{
	global_dev	=	dev;

	mh_object_unref((MHObject *)playlist);
	mh_object_unref((MHObject *)dev);
	mh_object_unref((MHObject *)_pb);

}		/* -----  end of static function _detach_event  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  _scan_callback
 *  Description:  
 * =====================================================================================
 */
static void _dev_event(MHDev * dev,  MHDevScanCbType scan_type, MHItemType item_type, void *  data, uint32_t pefrsent,void * user_data)
{
	MHFolder * _folder;

	if(scan_type	==	MH_DEV_FIRST_FILE)
	{
		if(item_type	==	MH_ITEM_MOVIE )
		{

			_folder	=	(MHFolder *) data;
			playlist	=	mh_folder_create_playlist( _folder,_movie_filter, false );
			mh_object_set_properties(( MHObject * )_pb, "video_sink", "mfxsink", "surfaceid", 888, "streamid", "abc", NULL );
			mh_object_set_properties( ( MHObject * )_pb, "audio_sink", "ahsink","buffer_time", 200000, NULL );

			mh_pb_play_by_list( _pb, playlist, 0 );
		}
	}

}		/* -----  end of static function _register_scan_callback  ----- */


static int __count=0;

void dev_event( MHCore * core, MHDev * dev, MHDevEvents event, void * user_data )
{
	printf("-------------------------------------------------------------------->count:%d\n",__count++);
	char  *_dev_type;
	MHDevEventsListener _event_listener		=
	{
		.callback	=	_dev_event,
		.user_data	=	NULL
	};

	
	mh_object_get_properties(( MHObject * )dev, "type", &_dev_type, NULL);
	
	if( _dev_type && !g_strcmp0( _dev_type, "storage"))
	{
		mh_object_ref((MHObject *)dev);
		MHDevDetachListener _detach_listener	=	
		{
			.callback	=	_detach_event,
			.user_data	=	NULL
		};
		mh_dev_register_detach_listener( dev, & _detach_listener);

		mh_dev_register_events_listener( dev, & _event_listener);

		_pb = mh_pb_create();


		mh_dev_start_scan( dev, SCAN_FOLDER);
	}
	free( _dev_type );
	
	global_dev=dev;
}



static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
{
    ilm_surfaceSetDestinationRectangle(id, 0, 0, width, height);
    printf("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
	ilm_surfaceSetSourceRectangle(id, 0, 0, width,height);

    printf("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)\n", id, width, height);
    ilm_surfaceSetVisibility(id, ILM_TRUE);
    printf("SetVisibility          : surface ID (%d), ILM_TRUE\n", id);
    ilm_layerAddSurface(layer,id);
    printf("layerAddSurface        : surface ID (%d) is added to layer ID (%d)\n", id, layer);
    ilm_commitChanges();
}

static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
    {
        configure_ilm_surface(id,sp->origSourceWidth,sp->origSourceHeight);

    }
}
static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
    (void)user_data;
	struct ilmSurfaceProperties sp;

	if (object == ILM_SURFACE)
	{
		if (created)
		{
			//            if (number_of_surfaces > 0) {
			if(id ==888)
			{
				number_of_surfaces--;
				printf("surface                : %d created\n",id);
				ilm_getPropertiesOfSurface(id, &sp);
				if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
				{   // surface is already configured
					configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
				} 
				else
				{
					// wait for configured event
					ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
					ilm_commitChanges();
				}
			}
		}

		else if(!created)
			printf("surface: %d destroyed\n",id);

	}
	else if (object == ILM_LAYER)
	{
		if (created)
			printf("layer: %d created\n",id);
		else if(!created)
			printf("layer: %d destroyed\n",id);
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	ilm_init();
	screen	=	1;
	layer	=	1000;
	number_of_surfaces	=	1000;
	 struct ilmScreenProperties screenProperties;
    struct ilmScreenExtendedProperties screenExtendedProperties;

    ilm_getPropertiesOfScreen(screen, &screenProperties);
    ilm_getExtendedPropertiesOfScreen(screen, &screenExtendedProperties);

    screenWidth = screenProperties.screenWidth;
    screenHeight = screenProperties.screenHeight;
    ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
    printf("CreateWithDimension          : layer ID (%d), Width (%u), Height (%u)\n", layer, screenWidth, screenHeight);
    ilm_layerSetDestinationRectangle(layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
    printf("layerSetDestinationRectangle : layer ID (%d), X (%u), Y (%u), Width (%u), Height (%u)\n", layer, screenExtendedProperties.x, screenExtendedProperties.y, screenWidth, screenHeight);
    ilm_layerSetVisibility(layer,ILM_TRUE);
    printf("SetVisibility                : layer ID (%d), ILM_TRUE\n", layer);

    t_ilm_int i, length = 0;
    t_ilm_layer* pArray = NULL;
    ilm_getLayerIDs(&length, &pArray);

    t_ilm_layer renderOrder[length + 1];
    renderOrder[length] = layer;

    for(i=0;i<length;i++) {
        renderOrder[i] = pArray[i];
    }

    ilm_displaySetRenderOrder(0,renderOrder,length + 1);
    ilm_commitChanges();
    ilm_registerNotification(callbackFunction, NULL);

	MHDevicesListener _devListener	=	
	{
		.callback	=	dev_event,
		.user_data	=	NULL
	};

//	ProfilerStart( "sample.prof" );


	printf( "Media-Hub v2.0 Built Time: %s %s\n", mh_built_date(), mh_built_time() );
	mh_misc_set_filter(MH_MISC_FILTER_MUSIC,"mp3;wma;aac;wav;ogg;mp2",
					   MH_MISC_FILTER_MOVIE,"mp4;3gp;avi;mkv;mpg;mpeg;wmv;vob;flv;swf;mov;dat",
					   MH_MISC_FILTER_PICTURE,"jpg;png;bmp;jpeg;jfif;jpe;ico;gif;tiff;tif",NULL);

	_movie_filter	=	mh_filter_create("mp4;3gp;avi;mkv;mpeg;wmv;vob;flv;swf;mov;dat");

	mh_core_register_devices_listener( &_devListener );
	mh_core_start();

	
	test_loop = g_main_loop_new( NULL, FALSE );
	g_main_loop_run( test_loop );
	g_main_loop_unref( test_loop );
	getchar();

	return 0;
}				/* ----------  end of function main  ---------- */

