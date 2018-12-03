#include "MsgLogicFriend.h"
#include "../module/FriendMgr.h"
#include "LogMacros.h"
#include "strutil.h"
#include "../framework/FriendSvrMsgLayer.h"
#include "ss_proto.h"
#include "../module/FriendTransaction.h"
#include "../framework/GameObjectPool.h"
#include "../framework/FriendTransFrame.h"

using namespace PKGMETA;

int CSsFriendHandleReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{    
	//预处理

	SS_PKG_FRIEND_HANDLE_REQ & rstSSPkgBodyReq = rstSsPkg.m_stBody.m_stFriendHandleReq;


	
	
	Friend* poFriendSender = FriendMgr::Instance().GetFriendByUin(rstSsPkg.m_stHead.m_ullUin);
	Friend* poFriendReceiver = FriendMgr::Instance().GetFriendByUin(rstSSPkgBodyReq.m_ullUin);
	if (NULL != poFriendSender && NULL != poFriendReceiver)
	{//有数据,不做事务
		FriendMgr::Instance().HandleSSMsg(rstSsPkg);
		return 0;
	}
	else
	{
		//逻辑处理
		FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
		assert(poGetListTrans != NULL);
		poGetListTrans->Reset();
		if (NULL == poFriendSender)
		{
			GetFriendAction* poGetFriendActionSender = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
			assert(poGetFriendActionSender != NULL);
			poGetFriendActionSender->Reset();
			poGetFriendActionSender->SetPlayerUin(rstSsPkg.m_stHead.m_ullUin);
			poGetListTrans->AddAction(poGetFriendActionSender);
		}
		if (NULL == poFriendReceiver)
		{
			GetFriendAction* poGetFriendActionReceiver = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
			assert(poGetFriendActionReceiver != NULL);
			poGetFriendActionReceiver->Reset();
			poGetFriendActionReceiver->SetPlayerUin(rstSSPkgBodyReq.m_ullUin); 
			poGetListTrans->AddAction(poGetFriendActionReceiver);
		}
		memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
		FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);
		return 0;
	}
}



int CSsFriendGetListReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{    
	//预处理

	SS_PKG_FRIEND_GET_LIST_REQ & rstSSPkgBodyReq = rstSsPkg.m_stBody.m_stFriendGetListReq;
	
	


	//逻辑处理
	FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
	assert(poGetListTrans!=NULL);
	GetCreateFriendAction* poGetCreateFriendAction = GET_GAMEOBJECT(GetCreateFriendAction, GAMEOBJ_GETCREATE_FRIEND_ACTION);
	assert(poGetCreateFriendAction!=NULL);
	GetAgreeApplyAction* poGetAgreeApplyAction = GET_GAMEOBJECT(GetAgreeApplyAction, GAMEOBJ_GET_AGREE_APPLY_ACTION);
	assert(poGetAgreeApplyAction!=NULL);
	poGetListTrans->Reset();
	poGetCreateFriendAction->Reset();
	poGetAgreeApplyAction->Reset();
	poGetCreateFriendAction->SetPlayerUin(rstSSPkgBodyReq.m_stPlayerInfo.m_ullUin);
	poGetCreateFriendAction->SetPlayerName(rstSSPkgBodyReq.m_stPlayerInfo.m_szName);
	poGetAgreeApplyAction->SetPlayerUin(rstSSPkgBodyReq.m_stPlayerInfo.m_ullUin);
	poGetListTrans->AddAction(poGetCreateFriendAction);
	poGetListTrans->AddAction(poGetAgreeApplyAction);
	memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去

	FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);

	return 0;
}


int CSsFriendSearchReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{    
	//预处理

	SS_PKG_FRIEND_SEARCH_REQ & rstSSPkgBodyReq = rstSsPkg.m_stBody.m_stFriendSearchReq;
	//SS_PKG_FRIEND_SEARCH_RSP & rstSSPkgBodyRsp = m_stSsPkg.m_stBody.m_stFriendSearchRsp;
	Friend* poFriendReceiver = NULL;
	 if (FRIEND_HANDLE_TYPE_SEARCH_UIN == rstSSPkgBodyReq.m_bType
         || FRIEND_HANDLE_TYPE_GET_PLAYER_INFO == rstSSPkgBodyReq.m_bType)
	 {
		 poFriendReceiver = FriendMgr::Instance().GetFriendByUin(rstSSPkgBodyReq.m_ullUin);
	 }
	 else
	 {
		 poFriendReceiver = FriendMgr::Instance().GetFriendByName(rstSSPkgBodyReq.m_szName);
	 }
	 if (NULL != poFriendReceiver)
	 {//有数据,不做事务
		 FriendMgr::Instance().HandleSSMsg(rstSsPkg);
		 return 0;
	 }
	 else
	{
		FriendTransaction* poSearchTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
		assert(poSearchTrans != NULL);
		GetFriendAction* poGetFriendActionReceiver = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
		assert(poGetFriendActionReceiver != NULL);
		GetPlayerInfoAction* poGetPlayerInfoAction = GET_GAMEOBJECT(GetPlayerInfoAction, GAMEOBJ_GET_PLAYER_INFO_ACTION);
		assert(poGetPlayerInfoAction!=NULL);
		poSearchTrans->Reset();
		poGetFriendActionReceiver->Reset();
		poGetPlayerInfoAction->Reset();
		if (FRIEND_HANDLE_TYPE_SEARCH_UIN == rstSSPkgBodyReq.m_bType 
            || FRIEND_HANDLE_TYPE_GET_PLAYER_INFO == rstSSPkgBodyReq.m_bType)
		{
			poGetFriendActionReceiver->SetPlayerUin(rstSSPkgBodyReq.m_ullUin);
			poGetPlayerInfoAction->SetPlayerUin(rstSSPkgBodyReq.m_ullUin);
		}
		else
		{
			poGetFriendActionReceiver->SetPlayerName(rstSSPkgBodyReq.m_szName);
			poGetFriendActionReceiver->SetUinType(2); //名字搜索
			poGetPlayerInfoAction->SetPlayerName(rstSSPkgBodyReq.m_szName);
		}
		poSearchTrans->AddAction(poGetFriendActionReceiver);
		poSearchTrans->AddAction(poGetPlayerInfoAction);


		memcpy(&poSearchTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
		FriendTransFrame::Instance().ScheduleTransaction(poSearchTrans);
		return 0;
	}
	return 0;
}
//玩家信息变更通知 更新 
int CSsFriendEventNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{    
	//预处理

	Friend* poFriendSender = FriendMgr::Instance().GetFriendByUin(rstSsPkg.m_stHead.m_ullUin);
	if (NULL != poFriendSender)
	{//有数据,不做事务
		FriendMgr::Instance().HandleSSMsg(rstSsPkg);
		return 0;
	}
	 else
	//逻辑处理
	{
		FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
		assert(poGetListTrans != NULL);
		poGetListTrans->Reset();
		if (NULL == poFriendSender)
		{
			GetFriendAction* poGetFriendActionSender = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
			assert(poGetFriendActionSender != NULL);
			poGetFriendActionSender->Reset();
			poGetFriendActionSender->SetPlayerUin(rstSsPkg.m_stHead.m_ullUin);
			poGetListTrans->AddAction(poGetFriendActionSender);
		}
		memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
		FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);
		return 0;
	}
}


int CSsFriendSendApReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	SS_PKG_FRIEND_SEND_AP_REQ& rstSsPkgBody = rstSsPkg.m_stBody.m_stFriendSendApReq;
	//预处理
	Friend* poFriendSender = FriendMgr::Instance().GetFriendByUin(rstSsPkg.m_stHead.m_ullUin);

	FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
	assert(poGetListTrans != NULL);
	poGetListTrans->Reset();
	if (NULL == poFriendSender)
	{
		GetFriendAction* poGetFriendActionSender = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
		assert(poGetFriendActionSender != NULL);
		poGetFriendActionSender->Reset();
		poGetFriendActionSender->SetPlayerUin(rstSsPkg.m_stHead.m_ullUin);
		poGetListTrans->AddAction(poGetFriendActionSender);
	}
	for (int i=0; i<rstSsPkgBody.m_wCount; i++)
	{
		uint64_t ullUin = rstSsPkgBody.m_ApReceiverUins[i];
		Friend* poFriendReceiver = FriendMgr::Instance().GetFriendByUin(ullUin);
		if (NULL == poFriendReceiver)
		{
			GetFriendAction* poGetFriendActionReceiver = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
			assert(poGetFriendActionReceiver != NULL);
			poGetFriendActionReceiver->Reset();
			poGetFriendActionReceiver->SetPlayerUin(ullUin);
			poGetListTrans->AddAction(poGetFriendActionReceiver);
		}
	}

	memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
	FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);
	return 0;

}

int CSsFriendGetApReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	//SS_PKG_FRIEND_SEND_AP_REQ& rstSsPkgBody = rstSsPkg.m_stBody.m_stFriendSendApReq;
	//预处理
	Friend* poFriendSender = FriendMgr::Instance().GetFriendByUin(rstSsPkg.m_stHead.m_ullUin);
	if (NULL != poFriendSender)
	{//有数据,不做事务
		FriendMgr::Instance().HandleSSMsg(rstSsPkg);
		return 0;
	}
	else
	{
		FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
		assert(poGetListTrans != NULL);
		poGetListTrans->Reset();

		GetFriendAction* poGetFriendActionSender = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
		assert(poGetFriendActionSender != NULL);
		poGetFriendActionSender->Reset();
		poGetFriendActionSender->SetPlayerUin(rstSsPkg.m_stHead.m_ullUin);
		poGetListTrans->AddAction(poGetFriendActionSender);

		memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
		FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);
		return 0;
	}
}

int CSsFriendChangeNameReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	//预处理
	Friend* poFriendSender = FriendMgr::Instance().GetFriendByUin(rstSsPkg.m_stHead.m_ullUin);
	if (NULL != poFriendSender)
	{//有数据,不做事务
		FriendMgr::Instance().HandleSSMsg(rstSsPkg);
		return 0;
	}
	else
	{
		FriendTransaction* poGetListTrans = GET_GAMEOBJECT(FriendTransaction, GAMEOBJ_FRIEND_TRANSACTION);
		assert(poGetListTrans != NULL);
		poGetListTrans->Reset();

		GetFriendAction* poGetFriendActionSender = GET_GAMEOBJECT(GetFriendAction, GAMEOBJ_GET_FRIEND_ACTION);
		assert(poGetFriendActionSender != NULL);
		poGetFriendActionSender->Reset();
		poGetFriendActionSender->SetPlayerUin(rstSsPkg.m_stHead.m_ullUin);
		poGetListTrans->AddAction(poGetFriendActionSender);

		memcpy(&poGetListTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
		FriendTransFrame::Instance().ScheduleTransaction(poGetListTrans);
		return 0;
	}
}



