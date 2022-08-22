/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#include "socket.h"
#include <iostream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "CommonUtil.h"
#include "CCarLifeLog.h"

using namespace std;

const int MAXCONNECTIONS = 10;

Socket::Socket() : m_sock ( -1 )
{
    memset(&m_addr, 0, sizeof ( m_addr ) );
}

Socket::~Socket()
{
    if ( is_valid() )::close ( m_sock );
}

bool Socket::create()
{
    m_sock = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( ! is_valid() )
        return false;

    int on = 1;
    if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
        return false;

    return true;
}


/*
bool Socket::bind ( const u32 port )
{
    if ( ! is_valid() )
    {
        return false;
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons ( port );

    int bind_return = ::bind ( m_sock, ( struct sockaddr * ) &m_addr, sizeof ( m_addr ) );

    if ( bind_return == -1 )
    {
        return false;
    }

    return true;
}
*/

/*
bool Socket::listen() const
{
    if ( ! is_valid() )
    {
        return false;
    }

    int listen_return = ::listen ( m_sock, MAXCONNECTIONS );

    if ( listen_return == -1 )
    {
        return false;
    }

    return true;
}
*/
/*
bool Socket::accept ( Socket& new_socket ) const
{
    int addr_length = sizeof ( m_addr );
    new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

    if ( new_socket.m_sock <= 0 )
        return false;
    else
        return true;
}
*/

u32 Socket::send ( const std::string s ) const
{
    return ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
}


bool Socket::send(u8* buf,u32 len) const
{
    int number = 0;
    while( len > 0 )
    {
        //LOG_DEBUG(mformat("writeData(): begin write packet +++, fd = %d", client_s));
        number = ::send(m_sock, buf, len, MSG_NOSIGNAL);
        //LOG_DEBUG(mformat("writeData(): end write packet +++, fd = %d", client_s));
        if( number > 0 )
        {
            len -= number;
            buf += number;
        }
        else
        {
	     CCarLifeLog::carLifeLogWithPrefix("writeData(): fd = ");
	     CCarLifeLog::carLifeLog(m_sock);
	     CCarLifeLog::carLifeLog(", ret = ");
	     CCarLifeLog::carLifeLog(number);
	     CCarLifeLog::carLifeLog(", errno = ");
	     CCarLifeLog::carLifeLog(errno);
	     CCarLifeLog::carLifeLog(": ");
	     CCarLifeLog::carLifeLog(strerror(errno));
	     CCarLifeLog::carLifeLog("\n");
            
            if(number < 0 && (errno == EINTR)) continue;
            return false;
        }
    }
    return true;
}

bool Socket::recv(u8* buf,u32 len) const
{
    //memset ( buf, 0, lenth );
    int number = 0;

    while( len > 0 )
    {
         number = ::recv(m_sock, buf, len, 0);
        if(  number > 0 )
        {
            len -= number;
            buf += number;
        }
        else
        {
	     CCarLifeLog::carLifeLogWithPrefix("writeData(): fd = ");
	     CCarLifeLog::carLifeLog(m_sock);
	     CCarLifeLog::carLifeLog(", ret = ");
	     CCarLifeLog::carLifeLog(number);
	     CCarLifeLog::carLifeLog(", errno = ");
	     CCarLifeLog::carLifeLog(errno);
	     CCarLifeLog::carLifeLog(": ");
	     CCarLifeLog::carLifeLog(strerror(errno));
	     CCarLifeLog::carLifeLog("\n");
           
            if(number < 0 && (errno == EINTR)) continue;
            return false;
        }
    }

    return true;
}


bool Socket::connect ( const std::string host, const u32 port, string interfaceName )
{
    if ( ! is_valid() ){ 
		CCarLifeLog::carLifeLogLnWithPrefix("connect: is_valid failed");
		
		return false;
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons ( port );

    int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

    if ( errno == EAFNOSUPPORT ){
		CCarLifeLog::carLifeLogLnWithPrefix("connect: errno == EAFNOSUPPORT failed");
		return false;
    }

    status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

    if ( status == 0 ){	
        return true;
    }else{
    	CCarLifeLog::carLifeLogWithPrefix("connect status= ");
	CCarLifeLog::carLifeLog(status);
	CCarLifeLog::carLifeLogLn(" :failed");
        return false;
    }
}

void Socket::set_non_blocking ( const bool b )
{
    int opts;

    opts = fcntl ( m_sock, F_GETFL );

    if ( opts < 0 )
    {
        return;
    }

    if ( b )
        opts = ( opts | O_NONBLOCK );
    else
        opts = ( opts & ~O_NONBLOCK );

    fcntl ( m_sock,F_SETFL,opts );
}
