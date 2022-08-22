/*
 * =====================================================================================
 *
 *       Filename:  cover.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/08/2014 05:21:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <taglib/tbytevector.h>
#include <taglib/tfile.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v1tag.h>
#include <taglib/apetag.h>
#include <taglib/tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <unistd.h>
#include <limits.h>
#include <mh_extern_misc.h>
#include <asffile.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <tstringlist.h>
#include <taglib/wavfile.h>
using namespace std;
using namespace TagLib;

MiscResult mh_file_get_cover( char * file_path, char * save_path, int length, char ** cover_path)
{
	
	MiscResult _res	=	MISC_OK;

	char * _file_name, * _file_path, * _folder_path, * _save_path;
	char * _p, *_path, *_ext, *_tag_ext, *_full_path;
	uint32_t _size;
	static uint32_t picture_count = 0;

	_save_path	=	g_strdup( save_path);
	_p	=	_save_path + strlen( _save_path) -1;
	if( *_p == '/')
	{
		*_p	=	'\0';
	}
	//_path	=	g_strdup_printf("%s%s",_save_path, file_path);
	_path	=	g_strdup_printf("%s/%d",_save_path, picture_count);
	picture_count++;

	g_free( _save_path );

	g_message("_path:%s", _path);
	
//make directionary
	_folder_path	=	g_strdup( _path);
	_p	=	g_strrstr( _folder_path, "/");
	* _p	=	'\0';
	g_message("_folder_path:%s", _folder_path);
	if( access( _folder_path, F_OK) == 0)
	{
	
	}
	else
	{
		char * _cmd;
		_cmd	=	g_strdup_printf("mkdir -p \"%s\"", _folder_path);
		if( system( _cmd )	==	-1)
		{
			g_message("mkdir is failed\n");
			return MISC_COVER_MKDIR_FAIL;
		}
		else
		{
			g_message("mkdir is OK\n");
		}
		g_free( _cmd );
	}
	g_free( _folder_path );

//get file type
	_p	=	g_strrstr( file_path, ".");
	if( _p == NULL)
	{
		return MISC_COVER_INVALID_TYPE;
	}

	_ext	=	g_ascii_strdown( _p + 1, strlen(_p + 1));
	g_message("_ext:%s", _ext);
	if( g_strcmp0( _ext, "mp3")==0)
	{
		
	    MPEG::File _mpeg_file( file_path );
		ID3v2::Tag * _id3v2tag	=	_mpeg_file.ID3v2Tag();
		if( _id3v2tag )
		{

			TagLib::ID3v2::FrameList _framelist	= _mpeg_file.ID3v2Tag()->frameListMap()["APIC"];
			if ( ! _framelist.isEmpty() )
			{

				ID3v2::AttachedPictureFrame * _picture_frame
					= dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(_mpeg_file.ID3v2Tag()->frameListMap()["APIC"].front());

				TagLib::ID3v2::AttachedPictureFrame * _p_frame  =  static_cast<TagLib::ID3v2::AttachedPictureFrame *>(_framelist.front());
				 _size = _p_frame->picture().size();
				if(_size	> (unsigned int)length)
				{
					g_message("size=%d>length\n",_size);
					_res	=	MISC_COVER_INVALID_LENGTH;
				}
				else
				{
					FILE * _file = NULL;

					_tag_ext	=	g_strdup(_picture_frame->mimeType().toCString());
					g_message("_tag_ext:%s", _tag_ext);
					_p	=	g_strrstr( _tag_ext, "/");
					if( _p != NULL)
					{
						_full_path	=	g_strdup_printf("%s.%s",_path, _p+1);
						g_message("_full_path:%s", _full_path);
						_file	=	fopen( _full_path, "wt+");
						if (_file != NULL)
						{
							if(fwrite( (const unsigned char*) _p_frame->picture().data(),_size,1,_file) != 1)
							{
								printf("memory is not enough\n");
								//fclose(_file);
								_res	=	 MISC_COVER_WRITE_FAIL;
							}	
							else
							{
								*cover_path	=	_full_path;

							}
							fclose(_file);
						}
					
					}
					else
					{
						g_message("the mimeType is not like image/bmp");
						_res	=	MISC_COVER_INVALID_TYPE;
					}
					g_free(_tag_ext);
				}
			}
			else
			{
				_res	=	MISC_COVER_NOT_EXIST;
			}

		}
		else
		{
			_res	=	MISC_COVER_NOT_EXIST;
		}
	}
	else if( g_strcmp0( _ext , "wma")==0)
	{
		g_message("---------------------------->oh yeah wma");
		ASF::File _asf_file( file_path );
		if( _asf_file.tag())
		{
			ASF::AttributeList _attrList = _asf_file.tag()->attributeListMap()["WM/Picture"];
			if( ! _attrList.isEmpty())
			{
				ASF::Attribute	_attr	=	_attrList.front();
				ASF::Picture _pic = _attr.toPicture();
				if( _pic.isValid())
				{
					if( _pic.dataSize() > (unsigned int)length)
					{
						g_message("size=%d>length\n",_pic.dataSize());
						_res	=	MISC_COVER_INVALID_LENGTH;

					}
					else
					{
						FILE * _file = NULL;

						_tag_ext	=	g_strdup(_pic.mimeType().toCString());
						g_message("_tag_ext:%s", _tag_ext);
						_p	=	g_strrstr( _tag_ext, "/");
						if( _p != NULL)
						{
							_full_path	=	g_strdup_printf("%s.%s",_path, _p+1);

							g_message("_full_path:%s", _full_path);

							_file	=	fopen( _full_path, "wt+");
							if (_file != NULL)
							{
								if(fwrite( (const unsigned char*) _pic.picture().data(),_pic.dataSize(),1,_file) != 1)
								{
									printf("memory is not enough\n");
									_res	=	 MISC_COVER_WRITE_FAIL;
								}
								else
								{
									*cover_path	=	_full_path;

								}
								fclose(_file);
							}
						}
						else
						{
							g_message("the mimeType is not like image/bmp");
							_res	=	MISC_COVER_INVALID_TYPE;
						}
						g_free(_tag_ext);
					}
				}
				else
				{
					g_message("the picture is not valid");
					_res	=	MISC_COVER_INVALID_TYPE;
				}

			}
			else
			{
				g_message("_attrList.isEmpty()");
				_res	=	MISC_COVER_NOT_EXIST;
			}
		}
		else
		{
			_res	=	MISC_COVER_NOT_EXIST;
		}
	}
	else if( g_strcmp0( _ext , "ogg")==0 )
	{
		_res	=	 MISC_COVER_INVALID_TYPE;
//		g_message("---------------------------->oh yeah ogg");
//		Ogg::Vorbis::File _ogg_file( file_path );
//
//		if( _ogg_file.tag())
//		{
//			g_message("size----------------->%d----->%d", _ogg_file.tag()->fieldListMap()["COVERARTMIME"].size(), _ogg_file.tag()->fieldListMap()[ "COVERART" ].size()  );
//			if( _ogg_file.tag()->fieldListMap()["COVERARTMIME"].size() != 0 && _ogg_file.tag()->fieldListMap()[ "COVERART" ].size() != 0)
//			{
//				g_message("--------------->ogg-->%s", _ogg_file.tag()->fieldListMap()["COVERARTMIME"][0].toCString(true));
//				_res	=	MISC_COVER_NOT_EXIST;
//			}
//			else
//			{
//				g_message("_attrList.isEmpty()");
//				_res	=	MISC_COVER_NOT_EXIST;
//			}
//		}
//		else
//		{
//			_res	=	MISC_COVER_NOT_EXIST;
//		}

	}
	else if( g_strcmp0( _ext, "wav") == 0)
	{
		RIFF::WAV::File _wav_file( file_path);
		ID3v2::Tag * _wavId3v2Tag	=	_wav_file.tag();
		if( _wavId3v2Tag)
		{
			TagLib::ID3v2::FrameList _framelist	 = _wavId3v2Tag->frameListMap()[ "APIC" ];

			if ( ! _framelist.isEmpty() )
			{

				ID3v2::AttachedPictureFrame * _picture_frame
					= dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(_framelist.front());

				TagLib::ID3v2::AttachedPictureFrame * _p_frame  =  static_cast<TagLib::ID3v2::AttachedPictureFrame *>(_framelist.front());
				_size = _p_frame->picture().size();
				if(_size	> (unsigned int)length)
				{
					g_message("size=%d>length\n",_size);
					_res	=	MISC_COVER_INVALID_LENGTH;
				}
				else
				{
					FILE * _file = NULL;

					_tag_ext	=	g_strdup(_picture_frame->mimeType().toCString());
					g_message("_tag_ext:%s", _tag_ext);
					_p	=	g_strrstr( _tag_ext, "/");
					if( _p != NULL)
					{
						_full_path	=	g_strdup_printf("%s.%s",_path, _p+1);
						g_message("_full_path:%s", _full_path);
						_file	=	fopen( _full_path, "wt+");
						if (_file != NULL)
						{
							if(fwrite( (const unsigned char*) _p_frame->picture().data(),_size,1,_file) != 1)
							{
								printf("memory is not enough\n");
								//fclose(_file);
								_res	=	 MISC_COVER_WRITE_FAIL;
							}	
							else
							{
								*cover_path	=	_full_path;

							}
							fclose(_file);
						}
					
					}
					else
					{
						g_message("the mimeType is not like image/bmp");
						_res	=	MISC_COVER_INVALID_TYPE;
					}
					g_free(_tag_ext);
				}
			}
			else
			{
				_res	=	MISC_COVER_NOT_EXIST;
			}


		}

	}
	else
	{
        printf("file does not have a valid id3v2 tag\n");
		_res	=	 MISC_COVER_INVALID_TYPE;
		
	}
	g_free(_path);
	return _res;

	

}


