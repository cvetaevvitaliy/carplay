/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#include "CSocketConnector.h"
#include "CCarLifeLog.h"

using namespace std;

CSocketConnector::CSocketConnector(string serverIP, u32 port, string interfaceName)
{
    if(serverIP.find(":",0)!=string::npos){
	CCarLifeLog::carLifeLogLnWithPrefix("iSockset=&socketv6");
	iSocket=&socketv6;
    }else{
    	CCarLifeLog::carLifeLogLnWithPrefix("iSockset=&socket");
    	iSocket=&socket;
    }
	
    setConnectStatus(false);

    mdServerIP = serverIP;
    mdServerPort = port;
    networkCardInterfaceName=interfaceName;
}
CSocketConnector::~CSocketConnector()
{

}

bool CSocketConnector::getConnectStatus()
{
    return isConnected;
}

bool CSocketConnector::connectToServer()
{
    if ( ! iSocket->create() )
    {
	CCarLifeLog::carLifeLogLnWithPrefix("socket.create fail!");
        return false;
    }

    if ( ! iSocket->connect ( mdServerIP, mdServerPort, networkCardInterfaceName ) )
    {	
	CCarLifeLog::carLifeLogWithPrefix("mdServerIP: ");
	CCarLifeLog::carLifeLog(mdServerIP);
	CCarLifeLog::carLifeLog(" mdServerPort: ");
	CCarLifeLog::carLifeLog(mdServerPort);
	CCarLifeLog::carLifeLog("\n");
	
	CCarLifeLog::carLifeLogLnWithPrefix("socket.connect fail!");
        return false;
    }
    setConnectStatus(true);
    return true;
}

























