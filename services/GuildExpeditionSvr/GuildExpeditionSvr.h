#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "./cfg/GuildExpeditionSvrCfgDesc.h"

class GuildExpeditionSvr : public CAppFrame, public TSingleton<GuildExpeditionSvr>
{
public:
	GuildExpeditionSvr() {}
	virtual ~GuildExpeditionSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	GUILDEXPEDITIONSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	GUILDEXPEDITIONSVRCFG m_stConfig;
};

