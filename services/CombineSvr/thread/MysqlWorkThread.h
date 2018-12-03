#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"

#include "MyTdrBuf.h"
#include "../cfg/CombineSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../module/BaseTable.h"



class CMysqlWorkThread : public CThreadFrame
{
public:
	CMysqlWorkThread() 
	{
		m_pstBaseTable = NULL;
		m_pstConfig = NULL;
		m_bOnceRun = false;
	};
	~CMysqlWorkThread()
	{
		m_pstBaseTable = NULL; 
		m_pstConfig = NULL;
	}
protected:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
	//int _HandleMsg();
    int _CheckMysqlConn();
    
	int CreateTable();
	int Work();


protected:
    CMysqlHandler 	m_oMysqlHandler;
    COMBINESVRCFG*	m_pstConfig;
	BaseTable* m_pstBaseTable;

	bool m_bOnceRun;

};

