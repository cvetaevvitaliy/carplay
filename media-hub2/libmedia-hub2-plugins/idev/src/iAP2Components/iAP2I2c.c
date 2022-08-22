/*
 * =====================================================================================
 *
 *       Filename:  iAP2I2c.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/20/2015 02:08:08 PM
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
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <glib.h>

struct i2c_msg {
	__u16 addr;	/* slave address			*/
	unsigned short flags;
#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD	0x01
#define I2C_M_NOSTART	0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
	short len;		/* msg length				*/
	char *buf;		/* pointer to msg data			*/
};

//#define I2C_DEV_ADDR	0x11
#define I2C_RETRY_COUNT	100

static uint32_t i2c_addr = 0;

void get_i2c_dev_addr( )
{
	const char * _addr	=	getenv("MH_I2C_DEV_ADDR");

	_addr	=	_addr ? _addr : "17" ;

	i2c_addr	=	(guint)atoi( _addr);
}

gboolean iAP2I2cReadBlock( int fd, uint8_t reg, uint8_t * data, int16_t len )
{
	struct i2c_rdwr_ioctl_data _data;
	struct i2c_msg _msg;
	int _retry	=	0, _len;

	_data.nmsgs	=	1;
	_data.msgs	=	&_msg;

	if( i2c_addr == 0)
	{
		get_i2c_dev_addr();
	}

	_data.msgs[0].addr	=	i2c_addr;
	_data.msgs[0].flags	=	0;
	_data.msgs[0].len	=	1;
	_data.msgs[0].buf	=	&reg;

	while( ioctl( fd, I2C_RDWR, ( unsigned long )&_data ) < 0 )
	{
		usleep( 10000 );                          /* Waiting for device response */

		_retry	++;
		if( _retry	> I2C_RETRY_COUNT )
			break;
	}

	if( _retry > I2C_RETRY_COUNT )
	{
		perror( "iAP2I2cReadBlock write" );

		return FALSE;
	}

	_retry	=	0;

	_data.msgs[0].addr	=	i2c_addr;
	_data.msgs[0].flags	=	I2C_M_RD;
	_data.msgs[0].len	=	len;
	_data.msgs[0].buf	=	data;

	while(( _len = ioctl( fd, I2C_RDWR, ( unsigned long )&_data )) < 0 )
	{
		usleep( 10000 );                          /* Waiting for device response */

		_retry	++;
		if( _retry	> I2C_RETRY_COUNT )
			break;
	}

	if( _retry > I2C_RETRY_COUNT )
	{
		perror( "iAP2I2cReadBlock read" );

		return FALSE;
	}

	return TRUE;
}

gboolean iAP2I2cWriteBlock( int fd, uint8_t reg, uint8_t * data, int16_t len )
{
	struct i2c_rdwr_ioctl_data _data;
	struct i2c_msg _msg;
	uint8_t _buf[ 1 + len ];
	int _retry	=	0;

	_data.nmsgs	=	1;
	_data.msgs	=	&_msg;

	_buf[0]	=	reg;
	memcpy( _buf + 1, data, len );

	if( i2c_addr == 0)
	{
		get_i2c_dev_addr();
	}

	_data.msgs[0].addr	=	i2c_addr;
	_data.msgs[0].flags	=	0;
	_data.msgs[0].len	=	1 + len;
	_data.msgs[0].buf	=	_buf;

	while( ioctl( fd, I2C_RDWR, ( unsigned long )&_data ) < 0 )
	{
		usleep( 10000 );                            /* Waiting for device response */

		_retry	++;
		if( _retry	> I2C_RETRY_COUNT )
			break;
	}

	if( _retry > I2C_RETRY_COUNT )
	{
		perror( "iAP2I2cWriteBlock write" );

		return FALSE;
	}

	return TRUE;
}


