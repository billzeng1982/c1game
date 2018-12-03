#pragma once
#include "MsgBase.h"

class CSsCheckSerialNumReq : public IMsgBase
{
public:
	CSsCheckSerialNumReq(){}
	~CSsCheckSerialNumReq(){}
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
	//从序列号中得到它的批次
	uint32_t _GetSerialId(char* pszSerial);
	int8_t _GetValue(char* pszChar);
};

