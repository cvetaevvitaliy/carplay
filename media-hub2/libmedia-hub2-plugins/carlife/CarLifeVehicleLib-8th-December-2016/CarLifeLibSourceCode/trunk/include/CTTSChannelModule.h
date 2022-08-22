/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@8th-December-2016@

	CarLife Protocol version:
		@V1.2.4@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#ifndef C_TTS_CHANNEL_MODULE_H
#define C_TTS_CHANNEL_MODULE_H
#include "CTranRecvPackageProcess.h"

class CTTSChannelModule{
	public:
		~CTTSChannelModule();
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








































