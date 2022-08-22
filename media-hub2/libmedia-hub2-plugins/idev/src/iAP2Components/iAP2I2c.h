/*
 * =====================================================================================
 *
 *       Filename:  iAP2I2c.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/20/2015 02:17:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __IAP2_I2C__
#define __IAP2_I2C__
#include <glib.h>

gboolean iAP2I2cReadBlock( int fd, uint8_t reg, uint8_t * data, int16_t len );
gboolean iAP2I2cWriteBlock( int fd, uint8_t reg, uint8_t * data, int16_t len );

#endif
