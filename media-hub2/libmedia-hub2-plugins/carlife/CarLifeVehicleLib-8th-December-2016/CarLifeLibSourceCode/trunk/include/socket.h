/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef SOCKET_H
#define SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "CommonUtil.h"
#include "ISocket.h"

class Socket:public ISocket
{
public:
    Socket();
    virtual ~Socket();

    bool create();

     // Server initialization
    //bool bind ( const u32 port );
    //bool listen() const;
    //bool accept ( Socket& ) const;

    // Client initialization
    bool connect ( const std::string host, const u32 port, std::string interfaceName );

    // Data Transimission
    u32 send ( const std::string ) const;

    bool recv(u8* buf,u32 lenth) const;
    bool send(u8* buf,u32 lenth) const;

    void set_non_blocking ( const bool );

    bool is_valid() const
    {
        return m_sock != -1;
    }

public:
    int m_sock;
    sockaddr_in m_addr;


};
#endif // SOCKET_H
