/*
 * =====================================================================================
 *
 *       Filename:  mh-api.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/12/2014 10:03:38 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __MH_EXTERN_MISC_H__
#define __MH_EXTERN_MISC_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>

typedef enum _MiscResult
{
	MISC_OK	=	0,

	MISC_COVER_INVALID_LENGTH,
	MISC_COVER_MKDIR_FAIL,
	MISC_COVER_INVALID_TYPE,
	MISC_COVER_WRITE_FAIL,
	MISC_COVER_NOT_EXIST,
	MISC_RESULT_NUM
} MiscResult;
MiscResult mh_file_get_cover( char * file_path, char * save_path, int length, char ** cover_path);
#ifdef __cplusplus
}
#endif
#endif

