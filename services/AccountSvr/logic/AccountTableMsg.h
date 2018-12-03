#pragma once

#include "MsgBase_r.h"

class CSsAccLoginReq : public IMsgBase_r
{
public:
	CSsAccLoginReq(){}
	virtual ~CSsAccLoginReq(){}
	
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsGmUpdateBanTimeReq : public IMsgBase_r
{
public:
	CSsGmUpdateBanTimeReq(){}
	virtual ~CSsGmUpdateBanTimeReq(){}

	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};
