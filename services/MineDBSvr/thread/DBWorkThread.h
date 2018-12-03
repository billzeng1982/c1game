#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../cfg/MineDBSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/MineDBSvrMsgLayer_r.h"
#include "../module/MineOreTable.h"
#include "../module/MinePlayerTable.h"



class CDBWorkThread : public CThreadFrame
{
public:
    static const int CONST_MINE_ORE_INFO_BINBUF_SIZE = MAX(PKGMETA::MAX_MINE_PLAYER_BLOB_LEN, PKGMETA::MAX_MINE_ORE_BLOB_LEN) * 2 + 1;
public:
    CDBWorkThread();
    ~CDBWorkThread();
     
    MineOreTable& 	GetMineOreTable() { return m_oMineOreTable; }
    MinePlayerTable& 	GetMinePlayerTable() { return m_oMinePlayerTable; }
    void SendPkg( PKGMETA::SSPKG& rstSsPkg );
    void SendToMineSvr(PKGMETA::SSPKG& rstSsPkg);
protected:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
    
    int _HandleSvrMsg();
    int _CheckMysqlConn();
    
protected:
    CMysqlHandler 	m_oMysqlHandler;
    MINEDBSVRCFG*	m_pstConfig;
    MineOreTable	m_oMineOreTable;
    MinePlayerTable	m_oMinePlayerTable;
    MyTdrBuf		m_stSendBuf;
    PKGMETA::SSPKG 	m_stSsRecvPkg;
    MineDBSvrMsgLayer_r m_oMsgLayer;
    char* m_szBinBuf;   //�����buff�������� ��Ϊ�˱���ÿ��Table��ȥ����,û�б�Ҫ,�������б���һ�����ľ�����
};

