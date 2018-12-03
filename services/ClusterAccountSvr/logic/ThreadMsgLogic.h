#pragma once

#include "MsgBase_r.h"

class SsClusterAccNewRoleReg : public IMsgBase_r
{
public: 
	SsClusterAccNewRoleReg() {}
	virtual ~SsClusterAccNewRoleReg() {}

	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL ); 
};

class SsClusterAccChgRoleName : public IMsgBase_r
{
public: 
	SsClusterAccChgRoleName() {}
	virtual ~SsClusterAccChgRoleName() {}

	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL ); 	
};

class SsGetOneClusterAccInfoReq : public IMsgBase_r
{
public:
	SsGetOneClusterAccInfoReq() {}
	virtual ~SsGetOneClusterAccInfoReq(){}
	
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL ); 
};

