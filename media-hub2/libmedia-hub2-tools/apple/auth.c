
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
#include <sys/socket.h>
#include <arpa/inet.h>


#define I2C_BUS             "/dev/i2c-0"

struct i2c_msg {
	__u16 addr;							/* slave address			*/
	unsigned short flags;
#define I2C_M_TEN			0x10		/* we have a ten bit chip address	*/
#define I2C_M_RD			0x01
#define I2C_M_NOSTART		0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
	short len;							/* msg length				*/
	char *buf;							/* pointer to msg data			*/
};

#define I2C_DEV_ADDR	    0x11
#define I2C_RETRY_COUNT	    100

gboolean iAP2I2cReadBlock( int fd, uint8_t reg, uint8_t * data, int16_t len )
{
	struct i2c_rdwr_ioctl_data _data;
	struct i2c_msg _msg;
	int _retry	=	0, _len;

	_data.nmsgs	=	1;
	_data.msgs	=	&_msg;

	_data.msgs[0].addr	=	I2C_DEV_ADDR;
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

	_data.msgs[0].addr	=	I2C_DEV_ADDR;
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

	_data.msgs[0].addr	=	I2C_DEV_ADDR;
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
static int fd	=	0;
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthReadCertData
 *  Description:  
 * =====================================================================================
 */
int16_t iAP2AuthReadCertData( uint8_t ** data )
{
	static uint8_t * certData	=	NULL;
	static int16_t certLen	=	0;
	uint8_t _buf[2];
	int i;

	if( fd == 0 )
	{
		fd	=	open( I2C_BUS, O_RDWR );

		g_assert( fd > 0 );
	}

	if( certData == NULL )
	{
		g_assert( iAP2I2cReadBlock( fd, 0x30, _buf, 2 ));

		certLen	=	_buf[0] << 8 | _buf[1];
		certData	=	g_malloc0( certLen );

		for( i = 0; i < certLen / 128; i ++ )
		{
			iAP2I2cReadBlock( fd, 0x31 + i, certData + i * 128, 128 );
		}

		if( certLen > i * 128 )
		{
			iAP2I2cReadBlock( fd, 0x31 + i, certData + i * 128, certLen - i * 128 );
		}
	}

	* data	=	certData;

	return certLen;
}		/* -----  end of function iAP2AuthReadCertData  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthWriteChallengeData
 *  Description:  
 * =====================================================================================
 */
gboolean iAP2AuthWriteChallengeData( uint8_t * data, uint16_t len )
{
	uint8_t _buf[2]	=	{0};
	gboolean _result	=	FALSE;

	if( fd == 0 )
	{
		fd	=	open( I2C_BUS, O_RDWR );

		g_assert( fd > 0 );
	}

	_buf[0]	=	len >> 8;
	_buf[1]	=	len & 0xFF;

	g_assert( iAP2I2cWriteBlock( fd, 0x20, _buf, 2 ));

	g_assert( iAP2I2cWriteBlock( fd, 0x21, data, len ));

	_buf[0]	=	1;
	g_assert( iAP2I2cWriteBlock( fd, 0x10, _buf, 1 ));

	g_assert( iAP2I2cReadBlock( fd, 0x10, _buf, 1 ));

	if( _buf[0] & 0xF0 == 0x10 )
		_result	=	TRUE;

	return _result;
}		/* -----  end of function iAP2AuthWriteChallengeData  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  iAP2AuthReadChallengeResponse
 *  Description:  
 * =====================================================================================
 */
int16_t iAP2AuthReadChallengeResponse( uint8_t ** data )
{
	uint8_t _buf[2];
	int16_t _resLen;
	uint8_t * _data;

	g_assert( iAP2I2cReadBlock( fd, 0x11, _buf, 2 ));

	_resLen	=	_buf[0] << 8 | _buf[1];
	_data	=	g_malloc0( _resLen );

	g_assert( iAP2I2cReadBlock( fd, 0x12, _data, _resLen ));

	* data	=	_data;

	close( fd );

	fd	=	0;

	return _resLen;
}		/* -----  end of function iAP2AuthReadChallengeResponse  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  
 * =====================================================================================
 */
int main ( int argc, char *argv[] )
{
	int _fd, _cfd;
	struct sockaddr_in _addr, _caddr;
	socklen_t _clen;
	unsigned char _buf[1024];

	if((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket Create");

		return 1;
	}

	memset(&_addr, 0, sizeof(struct sockaddr_in));

	_addr.sin_family	=	AF_INET;
	_addr.sin_addr.s_addr	=	htonl(INADDR_ANY);
	_addr.sin_port		=	htons(5196);

	bind(_fd, (struct sockaddr *)&_addr, sizeof(struct sockaddr_in));

	listen(_fd, 1);

	_clen	=	sizeof(struct sockaddr_in);

	while(1)
	{
		uint16_t _certLen, _resLen;
		uint8_t * _cert, * _res;

		_cfd	=	accept(_fd, (struct sockaddr *)&_caddr, &_clen);

		recv(_cfd, _buf, 50, 0);

		switch(_buf[0])
		{
		case 0:
			_certLen	=	iAP2AuthReadCertData( &_cert );
			_buf[0]	=	_certLen >> 8;
			_buf[1]	=	_certLen & 0xFF;

			memcpy(_buf + 2, _cert, _certLen);
			send(_cfd, _buf, _certLen + 2, 0);
			break;
		case 1:
			iAP2AuthWriteChallengeData( _buf + 3, _buf[1] << 8 | _buf[2] );

			_resLen	=	iAP2AuthReadChallengeResponse( &_res );

			_buf[0]	=	_resLen >> 8;
			_buf[1]	=	_resLen & 0xFF;

			memcpy(_buf + 2, _res, _resLen);
			send(_cfd, _buf, _resLen + 2, 0);
			break;
		default:
			break;
		}

		close(_cfd);
	}

	return 0;
}				/* ----------  end of function main  ---------- */

