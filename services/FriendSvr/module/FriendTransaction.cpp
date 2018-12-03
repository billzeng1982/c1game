#include "../module/FriendMgr.h"
#include "../framework/FriendTransFrame.h"
#include "LogMacros.h"
#include "../framework/FriendSvrMsgLayer.h"
#include "FriendTransaction.h"
#include "../module/PlayerInfo/RedisWorkThreadMgr.h"
#include "./FriendInfo/DBWorkThreadMgr.h"

void FriendTransaction::Reset()
{
	Transaction::Reset();
	m_ullSendUin = 0; 
	m_ullRecvUin = 0; 
	m_iActionErrNo = ERR_NONE;
	m_poSendFriend = NULL; 
	m_poRecvFriend = NULL; 
	return;
}
//事务中的动作执行完后开始执行业务逻辑
void FriendTransaction::OnFinished(int iErrCode)
{
	//LOGRUN_r("FriendTransaction OnFinished! iErrCode<%d>, ReqMsgId<%u>",iErrCode, m_stSsReqPkg.m_stHead.m_wMsgId);
	FriendMgr::Instance().HandleSSMsg(m_stSsReqPkg);
	return;
}


void GetFriendAction::Reset()
{
	IAction::Reset();
	m_pObjTrans = NULL;
	m_bGetType = 1;
	bzero(&m_stDBReq, sizeof(m_stDBReq));
	return;
}

int GetFriendAction::Execute(Transaction* pObjTrans)
{
	assert(pObjTrans != NULL);
	m_pObjTrans = (FriendTransaction*)pObjTrans;
	uint64_t ullToken = (uint64_t)this->GetToken();
	Friend* poFriend = NULL;
	//@TODO_OPTIMIZE 优化吧
	if (1 == m_bGetType)
	{
		poFriend = FriendMgr::Instance().GetFriendByUin(m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, ullToken);
	}
	else
	{
		poFriend = FriendMgr::Instance().GetFriendByName(m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, ullToken);
	}
	if (NULL == poFriend)
	{
		//@TODO_DEBUG_DEL
		//LOGRUN_r("GetFriendAction Wait Async OK! Uin<%lu>, Name<%s>, Token<%lu>", m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, ullToken);
		return 0;
	}

	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetFriendAction Execute OK! Uin<%lu>, Name<%s>, Token<%lu>", m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, ullToken);
	this->SetFiniFlag(1);
	return 1;
}

void GetFriendAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
	//_SetResult((Friend*) pResult);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetFriendAction OnAsyncRspMsg OK! Uin<%lu>, Name<%s>, Token<%lu>", m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, (uint64_t)GetToken());
	this->SetFiniFlag(1);
}




void GetAgreeApplyAction::Reset()
{
	IAction::Reset();
	m_pObjTrans = NULL;
	bzero(&m_stReq, sizeof(m_stReq));
}

int GetAgreeApplyAction::Execute(Transaction* pObjTrans)
{
	assert(pObjTrans != NULL);
	m_pObjTrans = (FriendTransaction*)pObjTrans;
	m_stReq.m_ullToken = (uint64_t)this->GetToken();
	Friend* poFriend = FriendMgr::Instance().GetFriendByUin(m_stReq.m_stPlayerInfo.m_ullUin);
	if (NULL == poFriend)
	{//这是前一个动作,执行错误,直接退出
		LOGERR_r("GetAgreeApplyAction poFriend is NULL , Uin<%lu>, Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, m_stReq.m_ullToken);
		this->SetFiniFlag(1);
		return 1;	
	}

	m_stReq.m_bType = FRIEND_REDIS_TYPE_GET_AGREE_APPLY_PLAYER_INFO;
	memcpy(&m_stReq.m_stAgreeInfo, &poFriend->m_stAgreeInfo, sizeof(DT_FRIEND_AGREE_INFO));
	memcpy(&m_stReq.m_stApplyInfo, &poFriend->m_stApplyInfo, sizeof(DT_FRIEND_APPLY_INFO));
	RedisWorkThreadMgr::Instance().SendReq(m_stReq);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetAgreeApplyAction Execute OK! Uin<%lu>,  Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, m_stReq.m_ullToken);
	return 0;
}

void GetAgreeApplyAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
	//_SetResult((Friend*) pResult);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetAgreeApplyAction OnAsyncRspMsg OK! Uin<%lu>,  Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, (uint64_t)GetToken());
	this->SetFiniFlag(1);
}

void GetPlayerInfoAction::Reset()
{
	IAction::Reset();
	m_pObjTrans = NULL;
	bzero(&m_stReq, sizeof(m_stReq));
}

void GetPlayerInfoAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
	//_SetResult((Friend*) pResult);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetPlayerInfoAction OnAsyncRspMsg OK! Uin<%lu>,  Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, (uint64_t)GetToken());
	this->SetFiniFlag(1);
}

int GetPlayerInfoAction::Execute(Transaction* pObjTrans)
{
	assert(pObjTrans != NULL);
	m_pObjTrans = (FriendTransaction*)pObjTrans;
	m_stReq.m_ullToken = (uint64_t)this->GetToken();
	Friend* poFriend = NULL;
	if (m_stReq.m_stPlayerInfo.m_ullUin != 0)
	{
		poFriend = FriendMgr::Instance().GetFriendByUin(m_stReq.m_stPlayerInfo.m_ullUin);
	}
	else
	{
		poFriend = FriendMgr::Instance().GetFriendByName(m_stReq.m_stPlayerInfo.m_szName);
	}
	if (NULL == poFriend)
	{//上个动作出错,目标的Friend未初始化,跳出
		LOGERR_r("GetPlayerInfoAction poFriend is NULL , Uin<%lu> or Name<%s> Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, 
			m_stReq.m_stPlayerInfo.m_szName, m_stReq.m_ullToken );
		this->SetFiniFlag(1);
		return 1;	
	}
	m_stReq.m_bType = FRIEND_REDIS_TYPE_GET_PLAYER_INFO;
	m_stReq.m_stPlayerInfo.m_ullUin = poFriend->m_ullUin;
	RedisWorkThreadMgr::Instance().SendReq(m_stReq);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetPlayerInfoAction Execute OK! Uin<%lu> or Name<%s>,  Token<%lu>", m_stReq.m_stPlayerInfo.m_ullUin, 
	//	m_stReq.m_stPlayerInfo.m_szName, m_stReq.m_ullToken );
	return 0;
}

void GetCreateFriendAction::Reset()
{
	IAction::Reset();
	m_pObjTrans = NULL;
	bzero(&m_stDBReq, sizeof(m_stDBReq));
}

void GetCreateFriendAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
	//_SetResult((Friend*) pResult);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetCreateFriendAction OnAsyncRspMsg OK! Uin<%lu>,  Token<%lu>", m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, (uint64_t)GetToken());
	this->SetFiniFlag(1);
}

int GetCreateFriendAction::Execute(Transaction* pObjTrans)
{
	assert(pObjTrans != NULL);
	m_pObjTrans = (FriendTransaction*)pObjTrans;
	m_stDBReq.m_ullToken  = (uint64_t)this->GetToken();
	m_stDBReq.m_bType = FRIEND_DB_TYPE_GETCREATE_FRIEND;
	DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
	//@TODO_DEBUG_DEL
	//LOGRUN_r("GetCreateFriendAction Execute OK! Uin<%lu>,  Token<%lu>", m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, m_stDBReq.m_ullToken);
	return 0;
}
