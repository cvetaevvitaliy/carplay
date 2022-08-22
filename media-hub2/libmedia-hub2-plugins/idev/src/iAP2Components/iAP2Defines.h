/*
 * =====================================================================================
 *
 *       Filename:  iAP2Defines.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2013 01:15:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */

#ifndef __IAP2_DEFINES_H__
#define __IAP2_DEFINES_H__

#define MAKE_WORD(h, l) (((h) << 8) | (l))
#define MAKE_DWORD(a, b, c, d)	(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define MAKE_DDWORD(a, b, c, d, e, f, g, h)(uint64_t)(((uint64_t)(a) << 56) | ((uint64_t)(b) << 48) | ((uint64_t)(c) << 40) | ((uint64_t)(d) << 32) | ((e) << 24) | ((f) << 16) | ((g) << 8) | (h))
#endif
