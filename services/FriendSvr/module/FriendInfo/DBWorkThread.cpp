#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "DBWorkThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../../framework/FriendSvrMsgLayer.h"


CDBWorkThread::CDBWorkThread() : m_stSendBuf(sizeof(DT_FRIEND_DB_REQ)*2+1)
{
	m_pstConfig = NULL;
}

bool CDBWorkThread::_ThreadAppInit(void* pvAppData)
{        
	m_pstConfig = (FRIENDSVRCFG*)pvAppData;
	
	// init mysql hanlder
	if( !m_oMysqlHandler.ConnectDB( m_pstConfig->m_szDBAddr, 
									m_pstConfig->m_wPort, 
									m_pstConfig->m_szDBName, 
									m_pstConfig->m_szUser, 
									m_pstConfig->m_szPassword ) )
	{
		LOGERR_r( "Connect mysql failed!" );
		return false;
	}

	m_oFriendTable.CInit( FRIEND_TABLE_NAME, &m_oMysqlHandler, m_pstConfig);
	return true;
}

void CDBWorkThread::_ThreadAppFini()
{

}

int CDBWorkThread::_ThreadAppProc()
{
	int iRet = _CheckMysqlConn();
	if (iRet <= 0)
	{
		return -1;
	}
	
	iRet = _HandleFriendMgrMsg();
	if (iRet <= 0)
	{
		return -1;
	}
	
	return 0;
}

//处理主线程发来的消息
int CDBWorkThread::_HandleFriendMgrMsg()
{
	int iRecvBytes = this->Recv( WORK_THREAD );
	if (iRecvBytes < 0)
	{
		return -1;
	}
	else if (0 == iRecvBytes)
	{
		return 0;
	}
	MyTdrBuf* pstRecvBuf = GetRecvBuf(WORK_THREAD);
	int iRet = (int) m_stReq.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("Get msg from FriendMgr ! Type<%d>, Token<%lu>, Uin<%lu> Name<%s>", m_stReq.m_bType, m_stReq.m_ullToken, 
	//        m_stReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stReq.m_stWholeData.m_stBaseInfo.m_szName);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("unpack pkg failed! errno : %d", iRet);
		return 1;
	}     
	bzero(&m_stRsp,sizeof(m_stRsp));
	switch (m_stReq.m_bType)
	{
	case FRIEND_DB_TYPE_GETCREATE_FRIEND:
		//该分支只是在数据库中没有使创建新纪录,返回用Type = FRIEND_DB_TYPE_GET_FRIEND 即可
		iRet = m_oFriendTable.CheckExist(m_stReq.m_stWholeData.m_stBaseInfo.m_ullUin);
		if (0 == iRet)
		{//数据库中没有
			iRet = m_oFriendTable.CreateFriend(m_stReq.m_stWholeData);
			memcpy(&m_stRsp.m_stWholeData, &m_stReq.m_stWholeData, sizeof(m_stReq.m_stWholeData));
		}
		else if (0 < iRet)
		{//数据库中有
			iRet = m_oFriendTable.GetFriendWholeData(m_stReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stRsp.m_stWholeData);
		}
		m_stRsp.m_bType = FRIEND_DB_TYPE_GET_FRIEND;
		break;
	case FRIEND_DB_TYPE_GET_FRIEND:
		iRet = m_oFriendTable.GetFriendWholeData(m_stReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stRsp.m_stWholeData);
		m_stRsp.m_bType = FRIEND_DB_TYPE_GET_FRIEND;
		break;
	case FRIEND_DB_TYPE_GET_FRIEND_BY_NAME:
		iRet = m_oFriendTable.GetFriendWholeData(m_stReq.m_stWholeData.m_stBaseInfo.m_szName, m_stRsp.m_stWholeData);
		m_stRsp.m_bType = FRIEND_DB_TYPE_GET_FRIEND;
		break;
	case FRIEND_DB_TYPE_UPDATE_FRIEND:
		iRet = m_oFriendTable.SaveWholeData(m_stReq.m_stWholeData);
		return 1;
	default:
		break;
	}
	m_stRsp.m_ullToken = m_stReq.m_ullToken;
	m_stRsp.m_nErrNo = iRet;
	SendRsp(m_stRsp);

	//@TODO_DEBUG_DEL
	//LOGRUN_r("Send msg pkg ! errno : <%d>, Token<%lu>, Uin<%lu>, Name<%s>", iRet, m_stReq.m_ullToken, m_stRsp.m_stWholeData.m_stBaseInfo.m_ullUin, m_stRsp.m_stWholeData.m_stBaseInfo.m_szName);
	return 1;
}

int CDBWorkThread::_CheckMysqlConn()
{
	// check alive
	if (!m_oMysqlHandler.IsConnAlive())
	{
		if (!m_oMysqlHandler.ReconnectDB())
		{
			return -1; //change to idle
		}
	}
	
	time_t lCurTime = CGameTime::Instance().GetCurrSecond();
	if (lCurTime - m_oMysqlHandler.GetLastPingTime() >= m_pstConfig->m_iPingFreq)
	{
		// ping mysql
		m_oMysqlHandler.Ping();
		m_oMysqlHandler.SetLastPingTime(lCurTime);
	}

	return 1;
}

void CDBWorkThread::SendRsp(DT_FRIEND_DB_RSP& rstRsp)
{
	TdrError::ErrorType iRet = rstRsp.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack DT_FRIEND_DB_RSP pkg error! MsgId<%d>, Uin<%lu>", rstRsp.m_bType, rstRsp.m_stWholeData.m_stBaseInfo.m_ullUin);
		return;
	}

	m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);

	//@TODO_DEBUG_DEL  删除
	//LOGRUN_r("SendRsp to Logic! Type<%lu>,    Uin<%lu> " , rstRsp.m_bType ,rstRsp.m_stWholeData.m_stBaseInfo.m_ullUin);
	return;
}

void CDBWorkThread::SendReq(DT_FRIEND_DB_REQ& rstReq)
{
	TdrError::ErrorType iRet = rstReq.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack DT_FRIEND_DB_REQ pkg error! Type<%d>, Uin<%lu>", rstReq.m_bType, rstReq.m_stWholeData.m_stBaseInfo.m_ullUin);
		return;
	}

	m_oReqQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
	//@TODO_DEBUG_DEL  删除
	//LOGRUN_r("Send to DB! Type<%d>,  , Uin<%lu> " , rstReq.m_bType ,rstReq.m_stWholeData.m_stBaseInfo.m_ullUin);
	return;
}


