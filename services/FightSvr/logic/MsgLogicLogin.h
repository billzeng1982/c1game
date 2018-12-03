#pragma once
#include "MsgBase.h"

class FightLogin_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class FightLogout_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class FightLoadingProgress_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

