#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../cfg/MiscSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/MiscSvrMsgLayer_r.h"
#include "../module/CloneBattleTable.h"
#include "../module/LevelRankTable.h"



class CDBWorkThread : public CThreadFrame
{
public:
    CDBWorkThread();
    ~CDBWorkThread(){}
     
    CloneBattleTeamTable& 	GetCloneBattleTable() { return m_oCloneBattleTable; }
    LevelRankTable& 	GetLevelRankTabke() { return m_oLevelRankTabke; }
    void SendPkg( PKGMETA::SSPKG& rstSsPkg );
    void SendToCloneBattleSvr(PKGMETA::SSPKG& rstSsPkg);
    void SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
protected:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
    
    int _HandleSvrMsg();
    int _CheckMysqlConn();
    
protected:
    CMysqlHandler 	m_oMysqlHandler;
    MISCSVRCFG*	m_pstConfig;
    CloneBattleTeamTable	m_oCloneBattleTable;
    LevelRankTable	m_oLevelRankTabke;
    
    MyTdrBuf		m_stSendBuf;
    PKGMETA::SSPKG 	m_stSsRecvPkg;

    MiscSvrMsgLayer_r m_oMsgLayer;
};

