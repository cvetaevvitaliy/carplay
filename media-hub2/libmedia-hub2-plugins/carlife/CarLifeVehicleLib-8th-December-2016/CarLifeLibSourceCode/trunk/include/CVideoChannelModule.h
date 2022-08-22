/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_VIDEO_CHANNEL_MODULE_H
#define C_VIDEO_CHANNEL_MODULE_H

#include "CTranRecvPackageProcess.h"

class CVideoChannelModule{
	public:
		~CVideoChannelModule();
		static CVideoChannelModule* getInstance();

		bool receiveVideoPackageHead();
		void videoPackageHeadAnalysis();

		bool receiveVideoData();
		void videoDataAnalysis();

		void videoRegisterDataReceive(void (*pFunc)(u8 *data, u32 len));
		void videoRegisterHeartBeat(void (*pFunc)(void));

	private:
		CVideoChannelModule():tranRecvPackageProcess(VIDEO_CHANNEL){
			}
		
		static CVideoChannelModule* pInstance;

		CTranRecvPackageProcess tranRecvPackageProcess;
};

























































#endif






























