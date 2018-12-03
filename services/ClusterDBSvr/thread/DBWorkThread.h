#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../cfg/ClusterDBSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/ClusterDBSvrMsgLayer_r.h"
#include "../module/GuildExpeditionPlayerTable.h"
#include "../module/GuildExpeditionGuildTable.h"



class CDBWorkThread : public CThreadFrame
{
public:
	static const int CONST_BINBUF_SIZE = MAX(PKGMETA::MAX_LEN_GUILD_EXPEDITION_GUILD_BLOB, PKGMETA::MAX_LEN_GUILD_EXPEDITION_PLAYER_BLOB) * 2 + 1;
public:
    CDBWorkThread();
	~CDBWorkThread();
     
    GuildExpeditionGuildTable& 	GetGuildExpeditionGuildTable() { return m_oGuildExpeditionGuildTable; }
	GuildExpeditionPlayerTable& 	GetGuildExpeditionPlayerTable() { return m_oGuildExpeditionPlayerTable; }

    void SendPkg( PKGMETA::SSPKG& rstSsPkg );
    void SendToGuildExpeditionSvr(PKGMETA::SSPKG& rstSsPkg);
protected:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
    
    int _HandleSvrMsg();
    int _CheckMysqlConn();
    
protected:
    CMysqlHandler 	m_oMysqlHandler;
    CLUSTERDBSVRCFG*	m_pstConfig;
    MyTdrBuf		m_stSendBuf;
    PKGMETA::SSPKG 	m_stSsRecvPkg;
	GuildExpeditionGuildTable m_oGuildExpeditionGuildTable;
	GuildExpeditionPlayerTable m_oGuildExpeditionPlayerTable;
    ClusterDBSvrMsgLayer_r m_oMsgLayer;
	char* m_szBinBuf;   //�����buff�������� ��Ϊ�˱���ÿ��Table��ȥ����,û�б�Ҫ,�������б���һ�����ľ�����
};

