#pragma once
#include "MsgBase_r.h"

// 在多线程里处理消息

class CSsRoleCreateReq : public IMsgBase_r
{
public:
	CSsRoleCreateReq(){}
	virtual ~CSsRoleCreateReq(){}
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsRoleLoginReq : public IMsgBase_r
{
public:
	CSsRoleLoginReq(){}
	virtual ~CSsRoleLoginReq(){}
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsRoleUptReq : public IMsgBase_r
{
public:
	CSsRoleUptReq(){}
	virtual ~CSsRoleUptReq(){}
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsMajestyRegReq : public IMsgBase_r
{
public:
	
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


class CSsRoleChgNameQueryReq : public IMsgBase_r
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};
class CSsGmDataOp : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
	int deleteProps(DT_GM_DEL_PROPS m_stHandleInfo, void* pvPara);
};

//class CSsRoleChgNameReq : public IMsgBase_r
//{
//public:
//	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
//};
