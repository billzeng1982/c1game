#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/MailSvrCfgDesc.h"

class MailSvr : public CAppFrame, public TSingleton<MailSvr>
{
public:
	MailSvr();
	virtual ~MailSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }

public:
	MAILSVRCFG& GetConfig(){ return m_stConfig; }

private:
	virtual void _SetAppCfg( );
	virtual bool _ReadCfg( );

private:
	MAILSVRCFG m_stConfig;
};

