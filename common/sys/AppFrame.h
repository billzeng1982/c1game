#ifndef _APP_FRAME_H_
#define _APP_FRAME_H_

/*
	进程框架, 继承使用
*/

#include <time.h>
#include "tdr/tdr.h"
#include "define.h"
#include "LogMacros.h"

struct SAppCfg
{
	char* m_pszBuff;
	uint32_t m_dwLen; // buff length
	char m_szMetaName[32];
	LPTDRMETALIB m_pstMetaLib;
	char m_szCfgFile[ 512 ];
};


class CAppFrame
{
public:
	CAppFrame();
	virtual ~CAppFrame(){}

	bool SvrInit( int argc, char** argv, const char* pszAppName, uint32_t dwLogTypes );
	void SvrRun( );
	
public:

	virtual bool AppInit() = 0;
	//处理应用逻辑，处理一帧操作，每帧由框架调用
	virtual void AppUpdate() = 0;
	virtual int GetProcID() = 0;

	// 处理SIGUSR2, 用于重新加载gamedata
	virtual bool ReloadGamedata() { return true; }
	virtual bool ReloadTBusChannel() { return true; }
	
	//释放应用
	virtual void AppFini() {};	

protected:
	virtual bool _ReadCfg( );
	void _InitLog( );
	void _KillPre( );

	virtual bool _ProcessArgs();

	virtual void _SetAppCfg( ) = 0;

protected:
	/* svr runtime context */
	SAppCfg 	m_stAppCfg;
	//bool 		m_bExit;
	//bool		m_bReload;
	int	 		m_iDaemon;
	int 		m_argc;
	char** 		m_argv;
	uint32_t 	m_dwLogTypes;
	char		m_szAppName[256];
	struct timeval	m_tvBootTime;	// 启动时间
};


extern bool g_bExit;
extern bool g_bReloadGamedata;
extern bool g_bReloadConfig;

void app_on_siguser1( int iSigNo );
void app_on_sigusr2( int iSigNo );
void app_on_sigio( int iSigNo );

#endif

