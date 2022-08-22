/*
 * =====================================================================================
 *
 *       Filename:  video_artwork.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/31/2016 09:27:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifdef __cplusplus
//extern "C"
//{
#endif
	
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#include <stdbool.h>

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <mh_video_artwork.h>
extern "C"
{
#ifndef INT64_C  
#define INT64_C(c) (c ## LL)  
#define UINT64_C(c) (c ## ULL)  
#endif 
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/md5.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <mh_api.h>
}
using namespace std;

#define BI_RGB 0x0 
#pragma pack(1) 

//typedef struct tagBITMAPFILEHEADER    
//{    
//	unsigned short  bfType;     //2 位图文件的类型，必须为“BM”    
//	unsigned int bfSize;        //4 位图文件的大小，以字节为单位
//	unsigned short bfReserved1; //2 位图文件保留字，必须为0    
//	unsigned short bfReserved2; //2 位图文件保留字，必须为0    
//	unsigned int bfOffBits;		//4 位图数据的起始位置，以相对于位图文件头的偏移量表示，以字节为单位
//} BITMAPFILEHEADER;				//该结构占据14个字节。    
//
//typedef struct tagBITMAPINFOHEADER
//{    
//	unsigned int biSize;		//4 本结构所占用字节数
//	int biWidth;				//4 位图的宽度，以像素为单位
//	int biHeight;				//4 位图的高度，以像素为单位
//	unsigned short biPlanes;    //2 目标设备的平面数不清，必须为1    
//	unsigned short biBitCount;  //2 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一    
//	unsigned int biCompression;	//4 位图压缩类型，必须是 0(不压缩),1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一
//	unsigned int biSizeImage;	//4 位图的大小，以字节为单位
//	int biXPelsPerMeter;		//4 位图水平分辨率，每米像素数
//	int biYPelsPerMeter;		//4 位图垂直分辨率，每米像素数
//	unsigned int biClrUsed;		//4 位图实际使用的颜色表中的颜色数
//	unsigned int biClrImportant;//4 位图显示过程中重要的颜色数
//} BITMAPINFOHEADER;				//该结构占据40个字节。  
//
//bool CreateBmp(const char *filename, uint8_t *pRGBBuffer, int width, int height, int bpp)  
//{  
//	BITMAPFILEHEADER bmpheader;  
//	BITMAPINFOHEADER bmpinfo;  
//	FILE *fp = NULL;  
//
//	fp = fopen(filename,"wb");  
//	if( fp == NULL )  
//	{  
//		return FALSE;  
//	}  
//
//	bmpheader.bfType = ('M' <<8)|'B';  
//	bmpheader.bfReserved1 = 0;  
//	bmpheader.bfReserved2 = 0;  
//	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
//	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp/8;  
//
//	bmpinfo.biSize = sizeof(BITMAPINFOHEADER);  
//	bmpinfo.biWidth = width;  
//	bmpinfo.biHeight = 0 - height;  
//	bmpinfo.biPlanes = 1;  
//	bmpinfo.biBitCount = bpp;  
//	bmpinfo.biCompression = BI_RGB;  
//	bmpinfo.biSizeImage = 0;  
//	bmpinfo.biXPelsPerMeter = 100;  
//	bmpinfo.biYPelsPerMeter = 100;  
//	bmpinfo.biClrUsed = 0;  
//	bmpinfo.biClrImportant = 0;  
//
//	fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,fp);  
//	fwrite(&bmpinfo,sizeof(BITMAPINFOHEADER),1,fp);  
//	fwrite(pRGBBuffer,width*height*bpp/8,1,fp);  
//	fclose(fp);  
//	fp = NULL;  
//
//	return TRUE;  
//}  

int MyWriteJPEG(AVFrame* pFrame, int width, int height, const char *filename)  
{  
	// Alloc VFormatContext handle
	AVFormatContext* pFormatCtx = avformat_alloc_context();  
	// setup file output format  
	pFormatCtx->oformat = av_guess_format( "mjpeg", NULL, NULL );  
	// init uri related AVIOContext  
	if( avio_open( &pFormatCtx->pb, filename, AVIO_FLAG_READ_WRITE ) < 0 ) {  
		g_message(" Couldn't open output file\n" );  
		return -1;  
	}  
	// create new stream  
	AVStream* pAVStream = avformat_new_stream( pFormatCtx, 0 );  
	if( pAVStream == NULL ) {  
		return -1;  
	}  
	// setup stream info   
	AVCodecContext* pCodecCtx = pAVStream->codec;  

	pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;  
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;  
	pCodecCtx->width = width;  
	pCodecCtx->height = height;  
	pCodecCtx->time_base.num = 1;  
	pCodecCtx->time_base.den = 25;  

	// Begin Output some information  
	av_dump_format( pFormatCtx, 0, filename, 1 );  
	// End Output some information  
	
	// find decode 
	AVCodec* pCodec = avcodec_find_encoder( pCodecCtx->codec_id );  
	if( !pCodec ) {  
		g_message(" Codec not found\n" );  
		return -1;  
	}  
    // setup pCodecCtx decode is pCodec  
	if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ) {  
		g_message(" Could not open codec \n");  
		return -1;  
	}  

	//Write Header  
	int ret	=	avformat_write_header(pFormatCtx, NULL);  

	int y_size = pCodecCtx->width * pCodecCtx->height;  
	
	//Encode  
    // AVPacket alloc mem  
	AVPacket pkt;  
	av_new_packet(&pkt, y_size * 3);

	int got_picture = 0;  
	ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);  
	if( ret < 0 ) {  
		g_message("Encode Error\n");  
		return -1;  
	}  
	if( got_picture == 1 ) {  
		//pkt.stream_index = pAVStream->index;  
		ret = av_write_frame(pFormatCtx, &pkt);  
	}  

	av_packet_unref(&pkt); 

    //Write Trailer  
	av_write_trailer(pFormatCtx);  

	g_message("Encode Successful\n");  

	if( pAVStream ) {  
		avcodec_close(pAVStream->codec);  
	}  
	avio_close(pFormatCtx->pb);  
	avformat_free_context(pFormatCtx);  

	return 0;  
}

MiscResult mh_file_get_video_artwork( char * file_path, char * save_path, int key_num, int64_t * duration, char ** cover_path)
{
	MiscResult _res	=	MISC_OK;
	gint32 i;
	gint32 videoStream = -1;
	AVFormatContext	*	pFormatCtx	=	NULL;
	AVCodecContext	*	pCodecCtx	=	NULL;
	AVCodec         *	pCodec 	= 	NULL;
	SwsContext		* 	img_convert_ctx =	NULL;
	AVFrame 		*	pFrame	=	NULL;
	AVFrame 		*	pFrameRGB = NULL;
	uint8_t 		*	buffer = NULL;
	int numBytes;
	int frameFinished;
	AVPacket packet;

	static uint32_t picture_count = 0;
	char * _folder_path = NULL ;
	char * _p, * _path, * _save_path;

	//register ffmpeg support format and codec
	av_register_all();

	//open file
	if( avformat_open_input( &pFormatCtx, file_path, NULL, NULL )!=0 ) 
	{
		g_message( "avformat_open_input(%s) fail\n",file_path );
		return MISC_GET_ARTWORK_FAIL;
	}

	//get stream info
	if (avformat_find_stream_info( pFormatCtx, NULL) < 0)
	{
		g_message("Couldn't find stream information\n");
		return	MISC_GET_ARTWORK_FAIL;
	}
	
	//get duration
	*duration	=	(pFormatCtx->duration)/AV_TIME_BASE;//get duration

	//get video stream
	for( i = 0; i < pFormatCtx->nb_streams; i++) 
	{
		if(pFormatCtx->streams[i] == NULL){ continue;}
		if(pFormatCtx->streams[i]->codec == NULL){ continue;}
		if((pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				&&((pFormatCtx->streams[i]->disposition
						& AV_DISPOSITION_ATTACHED_PIC) == 0))
		{
			videoStream = i;
			break;
		}
	}

	if( videoStream == -1 )
	{
		g_message("Didn't find a video stream\n");
		return	MISC_GET_ARTWORK_FAIL;
	}

	//find decode
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	if( pCodecCtx == NULL )
	{
		g_message("count not find codec context\n");
		return  MISC_GET_ARTWORK_FAIL; 
	}

	pCodec = avcodec_find_decoder( pCodecCtx->codec_id );

	if (pCodec == NULL) 
	{
		g_message("Unsupported codec for decode(%d)\n", pCodecCtx->codec_id );
		return MISC_GET_ARTWORK_FAIL; // Codec not found
	}

	//open decode
	if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)     
	{
		g_message( "could not open codec content\n" );
		return  MISC_GET_ARTWORK_FAIL; 
	}
	
	//debug function
	av_dump_format( pFormatCtx, videoStream, file_path, 0 );

	//every frame alloc mem
	pFrame = av_frame_alloc();						
	pFrameRGB = av_frame_alloc();

	if(( pFrameRGB == NULL ) || ( pFrame == NULL ))
	{
		g_message( "pFrameRGB or pFrame alloc failed\n" );
		return MISC_GET_ARTWORK_FAIL;
	}							

	// ensure pic size
	numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height, 1);
	buffer = ( uint8_t * ) av_malloc ( numBytes * sizeof( uint8_t ));

	if(buffer == NULL)
	{
		g_message( "av_malloc failed\n" );
		return MISC_GET_ARTWORK_FAIL; 
	}

	av_image_fill_arrays( pFrameRGB->data, pFrameRGB->linesize, buffer,
			AV_PIX_FMT_YUVJ420P, pCodecCtx->width, pCodecCtx->height, 1 );

	av_init_packet( &packet );

	i=0;
	picture_count++;	

	while(av_read_frame(pFormatCtx, &packet)>=0) 
	{
		if(packet.stream_index==videoStream) 
		{
			// Decode video frame pCodecCtx, pFrame, &frameFinished, &packet
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Did we get a video frame?
			if(frameFinished) {
				if(pFrame->key_frame==1) // key frame
				{
					// Convert the image from its native format to RGB
					img_convert_ctx = sws_getContext( pCodecCtx->width,  
							pCodecCtx->height,  
							AV_PIX_FMT_YUVJ420P,  
							pCodecCtx->width,  
							pCodecCtx->height,  
							AV_PIX_FMT_RGB32,  
							SWS_BILINEAR, NULL, NULL, NULL); 

					if(img_convert_ctx == NULL) 
					{
						g_message( "Cannot init the conv!\n"  );
						return MISC_GET_ARTWORK_FAIL;
					}

//					uint8_t *rgb_data = static_cast<uint8_t*>(av_malloc(pCodecCtx->width*pCodecCtx->height*4));  
//					uint8_t *rgb_src[3]= {rgb_data, NULL, NULL};  
//					int rgb_stride[3]={4*pCodecCtx->width, 0, 0};
//
//					sws_scale(
//							img_convert_ctx,
//							(const uint8_t * const*)pFrame->data,
//							pFrame->linesize,
//							0,
//							pCodecCtx->height,
//							rgb_src,
//							rgb_stride);

					// Save the frame to disk
					if(++i != (key_num - 1) )
					{
						_save_path	=	g_strdup( save_path);
						_path	=	g_strdup_printf("%s/%d.jpg",_save_path, picture_count);
						g_free( _save_path );

						g_message("_path:%s", _path);

						* cover_path = _path;
						//																CreateBmp(_path, rgb_data, pCodecCtx->width, pCodecCtx->height, 32);  
						MyWriteJPEG(pFrame, pCodecCtx->width, pCodecCtx->height, _path);
//						::memset(rgb_data, 0, pCodecCtx->width*pCodecCtx->height*4);  
//						av_freep(&rgb_data);  
						sws_freeContext(img_convert_ctx); 

					}
					if( i > (key_num - 1))
						break;
				}
			}
		}
		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
	}

	if( _folder_path != NULL )
		g_free( _folder_path );		

	if( buffer != NULL )
		av_free(buffer);

	if( pFrameRGB != NULL )
		av_free(pFrameRGB);

	if( pFrame != NULL )
		av_free(pFrame);

	if( pCodecCtx != NULL )
		avcodec_close(pCodecCtx);

	if( pFormatCtx != NULL )
		avformat_close_input(&pFormatCtx);

	return _res;
}
#ifdef __cplusplus
//}
#endif

