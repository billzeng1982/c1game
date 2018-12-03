#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../cfg/MailDBSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/MailDBSvrMsgLayer_r.h"
#include "../logic/MailPriTable.h"
#include "../logic/MailPubTable.h"



class CDBWorkThread : public CThreadFrame
{
public:
    CDBWorkThread();
    ~CDBWorkThread(){}
     
    MailPriTable& 	GetMailPriTable() { return m_oMailPriTable; }
    MailPubTable& 	GetMailPubTable() { return m_oMailPubTable; }
    void SendPkg( PKGMETA::SSPKG& rstSsPkg );

protected:
    virtual bool _ThreadAppInit(void* pvAppData);
    virtual void _ThreadAppFini();
    virtual int _ThreadAppProc();
    
    int _HandleMailSvrMsg();
    int _CheckMysqlConn();
    
protected:
    CMysqlHandler 	m_oMysqlHandler;
    MAILDBSVRCFG*	m_pstConfig;
    MailPriTable	m_oMailPriTable;
    MailPubTable	m_oMailPubTable;
    
    MyTdrBuf		m_stSendBuf;
    PKGMETA::SSPKG 	m_stSsRecvPkg;

    MailDBSvrMsgLayer_r m_oMsgLayer;
};

