/*
 * =====================================================================================
 *
 *       Filename:  order.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/09/2014 10:31:46 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __ORDER_H__
#define __ORDER_H__

#include <glib.h>

gint orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b );
gint orderByAlphabetForNagivi( gconstpointer a, gconstpointer b );
gint orderByAlphabet( gconstpointer a, gconstpointer b );
gint orderByPinyin( gconstpointer a, gconstpointer b );
gint string_orderByAlphabetForNagiviJp(gconstpointer a, gconstpointer b );
gint string_orderByAlphabetForNagivi(gconstpointer a, gconstpointer b );
gint string_orderByAlphabet( gconstpointer a, gconstpointer b );
gint string_orderByPinyin( gconstpointer a, gconstpointer b );
gint playlist_orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b, gpointer user_data );
gint playlist_orderByAlphabetForNagivi( gconstpointer a, gconstpointer b, gpointer user_data );
gint playlist_orderByAlphabet( gconstpointer a, gconstpointer b, gpointer user_data);
gint playlist_orderByPinyin( gconstpointer a, gconstpointer b, gpointer user_data);
gint playlist_orderByTrackId( gconstpointer a, gconstpointer b, gpointer user_data);
gint album_orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b );
gint album_orderByAlphabetForNagivi( gconstpointer a, gconstpointer b );
gint album_orderByAlphabet( gconstpointer a, gconstpointer b);
gint album_orderByPinyin( gconstpointer a, gconstpointer b );
#endif

