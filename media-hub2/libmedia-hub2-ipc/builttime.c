/*
 * =====================================================================================
 *
 *       Filename:  builttime.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/12/2014 09:56:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

const char * mh_built_date()
{
	return __DATE__;
}

const char * mh_built_time()
{
	return __TIME__;
}

