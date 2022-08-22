/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_CONNECTION_SETUP_MODULE
#define C_CONNECTION_SETUP_MODULE
#include "CTranRecvPackageProcess.h"

class CConnectionSetupModule{
	public:
		~CConnectionSetupModule();
		static CConnectionSetupModule* getInstance();

		int connectionSetup();
		int connectionSetup(string mdIPAddress);
		int connectionSetup(string mdIPAddress, string interfaceName);

	private:
		CConnectionSetupModule();
		
		static CConnectionSetupModule* pInstance;
};


































#endif

