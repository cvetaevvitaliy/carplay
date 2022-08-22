/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_VR_CHANNEL_MODULE_H
#define C_VR_CHANNEL_MODULE_H

#include "CTranRecvPackageProcess.h"

class CVRChannelModule{
	public:
		static CVRChannelModule* getInstance();
		int sendVRRecordData(u8* data, u32 size, u32 timeStamp);

		bool receiveVRPackageHead();
		void vrPackageHeadAnalysis();

		bool receiveVRData();
		void vrDataAnalysis();

		void vrRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER*));
		void vrRegisterNormalData(void (*pFunc)(u8 *data, u32 len));
		void vrRegisterStop(void (*pFunc)(void));

	private:
		CVRChannelModule():tranRecvPackageProcess(VR_CHANNEL){
			}
		
		static CVRChannelModule* pInstance;

		CTranRecvPackageProcess tranRecvPackageProcess;
};

























































#endif
































