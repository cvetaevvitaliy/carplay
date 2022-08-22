/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_TTS_CHANNEL_MODULE_H
#define C_TTS_CHANNEL_MODULE_H
#include "CTranRecvPackageProcess.h"

class CTTSChannelModule{
	public:
		static CTTSChannelModule* getInstance();

		bool receiveTTSPackageHead();
		void ttsPackageHeadAnalysis();

		bool receiveTTSData();
		void ttsDataAnalysis();

		void ttsRegisterInit(void (*pFunc)(S_AUDIO_INIT_PARAMETER*));
		void ttsRegisterNormalData(void (*pFunc)(u8 *data, u32 len));
		void ttsRegisterStop(void (*pFunc)(void));

	private:
		CTTSChannelModule():tranRecvPackageProcess(TTS_CHANNEL){
			}
		
		static CTTSChannelModule* pInstance;

		CTranRecvPackageProcess tranRecvPackageProcess;
};





































#endif








































