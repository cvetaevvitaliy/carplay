/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_CONNECTION_SETUP_MODULE
#define C_CONNECTION_SETUP_MODULE
#include "CTranRecvPackageProcess.h"

class CConnectionSetupModule{
	public:
		static CConnectionSetupModule* getInstance();

		int connectionSetup();
		int connectionSetup(string mdIPAddress);
		int connectionSetup(string mdIPAddress, string interfaceName);

	private:
		CConnectionSetupModule();
		
		static CConnectionSetupModule* pInstance;
};


































#endif

