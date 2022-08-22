/*******************************************************
	Author: 
		Liu Caiquan
	Date: 
		@16th-March-2016@

	CarLife Protocol version:
		@v1.0.11@
							Copyright (C) Under BaiDu, Inc.
*******************************************************/
#include "CVideoChannelModule.h"
#include "CConnectManager.h"
#include<iostream>

using namespace std;

CVideoChannelModule* CVideoChannelModule::pInstance=NULL;

CVideoChannelModule* CVideoChannelModule::getInstance(){
	if(NULL==pInstance){
		pInstance= new CVideoChannelModule();
		}

	return pInstance;
}

bool CVideoChannelModule::receiveVideoPackageHead(){
	return tranRecvPackageProcess.receivePackageHead();
}

void CVideoChannelModule::videoPackageHeadAnalysis(){
	tranRecvPackageProcess.packageHeadAnalysis();
}

bool CVideoChannelModule::receiveVideoData(){
	return tranRecvPackageProcess.receivePackageData();
}

void CVideoChannelModule::videoDataAnalysis(){
	tranRecvPackageProcess.packageDataAnalysis();
}

void CVideoChannelModule::videoRegisterDataReceive(void (*pFunc)(u8 *data, u32 len)){
	tranRecvPackageProcess.videoRegisterDataReceive(pFunc);
}

void CVideoChannelModule::videoRegisterHeartBeat(void (*pFunc)(void)){
	tranRecvPackageProcess.videoRegisterHeartBeat(pFunc);
}


























