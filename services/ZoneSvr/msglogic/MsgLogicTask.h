#pragma once

#include "MsgBase.h"

class TaskDraw_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );	
};

class TaskFinalAward_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );	
};

class TaskModify_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

