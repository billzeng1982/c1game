#include "FriendMgr.h"
#include "GameTime.h"
#include "utils/og_comm.h"
#include "TableDefine.h"
#include "../framework/FriendSvrMsgLayer.h"
#include "./FriendInfo/DBWorkThreadMgr.h"

using namespace std;
using namespace PKGMETA;

#define THREAD_INDEX(UIN) (((UIN) & 0xff) % m_iWorkThreadNum)

FriendMgr::FriendMgr()
{
}

FriendMgr::~FriendMgr()
{
}

bool FriendMgr::Init(FRIENDSVRCFG* pstConfig)
{
	m_dwFriendMaxNum = pstConfig->m_wFriendMaxNum;
	if (m_oFriendPool.CreatePool(m_dwFriendMaxNum))
	{
		LOGERR_r("Create oBuff Friend pool[num=%d] failed.", m_dwFriendMaxNum);
		return false;
	}
	m_oFriendPool.RegisterSlicedIter(&m_oUptIter);
	m_iUpdateIntervalTime = pstConfig->m_iUpdateIntervalTime;
	m_iUpdateDirtyNum = pstConfig->m_iUpdateDirtyNum;
	if (0 == m_iUpdateIntervalTime && 0 == m_iUpdateDirtyNum)
	{
		LOGERR_r("error:m_iUpdateIntervalTime and m_iUpdateDirtyNum are all '0");
		return false;
	}
	
	INIT_LIST_HEAD(&m_stTimeListHead);
	INIT_LIST_HEAD(&m_stDirtyListHead);
	m_iDirtyNum = 0;
	return true;
}

 //加入2张map里
void FriendMgr::AddFriendToMap(Friend* poFriend)
{
	if (NULL == poFriend)
	{
		LOGERR_r("poFriend is null.");
		return ;
	}
	m_stFriendUinAddrMap.insert(FriendUinAddrMap_t::value_type(poFriend->m_ullUin, poFriend));
	m_stFriendNameAddrMap.insert(FriendNameAddrMap_t::value_type(poFriend->m_szName, poFriend));
	//AddTimeList(poFriend);
	return;
}

void FriendMgr::DelFriendFromMap(Friend* poFriend)
{
	if (NULL == poFriend)
	{
		LOGERR_r("poFriend is null.");
		return ;
	}
		m_stFriendUinAddrMapIter = m_stFriendUinAddrMap.find(poFriend->m_ullUin);
	if (m_stFriendUinAddrMapIter != m_stFriendUinAddrMap.end()) 
	{
		m_stFriendUinAddrMap.erase(m_stFriendUinAddrMapIter);
	}
	m_stFriendNameAddrMapIter = m_stFriendNameAddrMap.find(poFriend->m_szName);
	if (m_stFriendNameAddrMapIter != m_stFriendNameAddrMap.end()) 
	{
		m_stFriendNameAddrMap.erase(m_stFriendNameAddrMapIter);
	}
	return;
}

//释放2张map和内存池
void FriendMgr::Destory()
{
	m_stFriendUinAddrMap.clear();
	m_stFriendNameAddrMap.clear();
	m_oFriendPool.DestroyPool();
	INIT_LIST_HEAD(&m_stTimeListHead);
	INIT_LIST_HEAD(&m_stDirtyListHead);
	return;
}

//  ullUinSender为调用对象, ullFriendUinReceiver为被操作者
int FriendMgr::ApplyFriend(uint64_t ullUinSender, uint64_t ullUinReceiver)
{
	Friend* poFriendReceiver = GetFriendByUin(ullUinReceiver);
	Friend* poFriendSender = GetFriendByUin(ullUinSender);
	if (NULL == poFriendSender || NULL == poFriendReceiver)
	{
		LOGERR_r("poFriendReceiver or poFriendSender is null");
		return ERR_SYS;
	}
	if (poFriendSender->m_stAgreeInfo.m_wCount >= MAX_FRIEND_AGREE_LIST_CNT)
	{//自己好友列表已满
		return ERR_FRIEND_AGREE_FULL;
	}
	//检查自己好友列表里有对方  //暂时不检查
	//检查对方好友列表里有自己  //
	//双方好友列表 不一致 怎么处理 ? 修正,删除 ?
	int iRet = poFriendReceiver->Add2ApplyList(ullUinSender);
	if (ERR_NONE == iRet)
	{
		AddDirtyList(*poFriendReceiver);
		DelTimeList(*poFriendReceiver);
		CastFriend(poFriendSender, ullUinReceiver, FRIEND_NTF_TYPE_APPLY);
	}
	return iRet;
}
//  ullUinSender为调用对象, ullUinReceiver为被操作者
int FriendMgr::AgreeFriend(uint64_t ullUinSender, uint64_t ullUinReceiver)
{
	
	Friend* poFriendReceiver = GetFriendByUin(ullUinReceiver);
	Friend* poFriendSender = GetFriendByUin(ullUinSender);
	if (NULL == poFriendSender || NULL == poFriendReceiver)
	{
		LOGERR_r("poFriendReceiver or poFriendSender is null");
		return ERR_SYS;
	}
	int iRet = poFriendSender->Add2AgreeList(ullUinReceiver);
	if (ERR_NONE == iRet)
	{// 这里 需要 改成事务吗?
		iRet = poFriendReceiver->Add2AgreeList(ullUinSender);
		if (ERR_NONE != iRet)
		{

			poFriendSender->DeleteAgreeList(ullUinReceiver);
			LOGERR_r("Agree friend erro<%d>! AgreeUin<%lu>,ApplyUin<%lu>", iRet, ullUinSender, ullUinReceiver);
			if (ERR_FRIEND_AGREE_FULL == iRet)
			{//对方好友已满
				return ERR_FRIEND_OPERATE_FULL;
			}
			return iRet;
		}
		//这里删除 AB在对方的申请列表
		poFriendSender->DeleteApplyList(ullUinReceiver);
		if (poFriendReceiver->IsInApplyList(ullUinSender))
		{
			poFriendReceiver->DeleteApplyList(ullUinSender);
		}
	}
	else
	{
		return iRet;
	}
	AddDirtyList(*poFriendSender);
	DelTimeList(*poFriendSender);
	AddDirtyList(*poFriendReceiver);
	DelTimeList(*poFriendReceiver);
	CastFriend(poFriendSender, ullUinReceiver, FRIEND_NTF_TYPE_AGREE);
	return ERR_NONE;
}

//  Uin为调用对象, FriendUin为被操作者
int FriendMgr::RejectFriend(uint64_t ullUinSender, uint64_t ullUinReceiver)
{
	Friend* poFriendReceiver = GetFriendByUin(ullUinReceiver);
	Friend* poFriendSender = GetFriendByUin(ullUinSender);
	if (NULL == poFriendReceiver || NULL == poFriendSender)
	{
		LOGERR_r("poFriendReceiver or poFriendSender is null");
		return ERR_SYS;
	}
	poFriendSender->DeleteApplyList(ullUinReceiver);
	AddDirtyList(*poFriendSender);
	DelTimeList(*poFriendSender);
	CastFriend(poFriendSender, ullUinReceiver, FRIEND_NTF_TYPE_REJECT);
	return ERR_NONE;
}

int FriendMgr::DeleteFriend(uint64_t ullUinSender, uint64_t ullUinReceiver)
{
	Friend* poFriendReceiver = GetFriendByUin(ullUinReceiver);
	Friend* poFriendSender = GetFriendByUin(ullUinSender);
	if (NULL == poFriendReceiver || NULL == poFriendSender)
	{
		LOGERR_r("poFriendReceiver or poFriendSender is null");
		return ERR_SYS;
	}
	poFriendSender->DeleteAgreeList(ullUinReceiver);
	poFriendReceiver->DeleteAgreeList(ullUinSender);
	if (poFriendSender->IsInRecvApList(ullUinReceiver))
	{
		poFriendSender->DeleteRecvApList(ullUinReceiver);
	}
	if (poFriendReceiver->IsInRecvApList(ullUinSender))
	{
		poFriendReceiver->DeleteRecvApList(ullUinSender);
	}
	
	AddDirtyList(*poFriendSender);
	DelTimeList(*poFriendSender);
	AddDirtyList(*poFriendReceiver);
	DelTimeList(*poFriendReceiver);
	CastFriend(poFriendSender, ullUinReceiver, FRIEND_NTF_TYPE_DELETE);
	return ERR_NONE;
}



Friend* FriendMgr::GetFriendByUin(uint64_t ullUin)
{
	//先在内存中找
	m_stFriendUinAddrMapIter = m_stFriendUinAddrMap.find(ullUin);
	if (m_stFriendUinAddrMapIter != m_stFriendUinAddrMap.end())
	{
		Move2TimeListFirst(*m_stFriendUinAddrMapIter->second);
		return m_stFriendUinAddrMapIter->second;
	}
	return NULL;
}

Friend* FriendMgr::GetFriendByUin(uint64_t ullUin, uint64_t ullTokenId)
{
	//先在内存中找
	m_stFriendUinAddrMapIter = m_stFriendUinAddrMap.find(ullUin);
	if (m_stFriendUinAddrMapIter != m_stFriendUinAddrMap.end())
	{
		//Move2TimeListFirst(*m_stFriendUinAddrMapIter->second);
		return m_stFriendUinAddrMapIter->second;
	}
	//内存中没有找到则向数据库处理线程发消息
	bzero(&m_stDBReq, sizeof(m_stDBReq));
	m_stDBReq.m_bType = FRIEND_DB_TYPE_GET_FRIEND;
	m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin = ullUin;
	m_stDBReq.m_ullToken = ullTokenId;
	DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
	return NULL;
}


Friend* FriendMgr::NewFriend()
{
	Friend* poFriend = NULL ; //m_oFriendPool.NewData();
	if (m_oFriendPool.GetUsedNum() < (int)m_dwFriendMaxNum) //m_dwFriendMaxNum可以人工保证不会溢出
	{
		poFriend = m_oFriendPool.NewData();
		if (NULL == poFriend)
		{
			LOGERR_r("poFriend is NULL, Get New Friend faild");
			return NULL;
		}
		poFriend->Reset();
		return poFriend;
	}
	else
	{
		list_head* pstSwapNode = NULL;
		if (list_empty(&m_stTimeListHead))
		{//LRU列表为空,回写DirtyList
			pstSwapNode = m_stDirtyListHead.prev;
			poFriend = list_entry(pstSwapNode, Friend, m_stDirtyListNode);
			DelDirtyList(*poFriend);
			LOGWARN_r("NewFriend:TimeList is empty, write DirtyList!");
			bzero(&m_stDBReq, sizeof(m_stDBReq));
			if (poFriend->PackWholeData(m_stDBReq.m_stWholeData) != TdrError::TDR_NO_ERROR)
			{
				LOGERR_r("pack PackWholeData failed, Uin=<%lu>", poFriend->m_ullUin);
				return NULL;
			}
			m_stDBReq.m_bType = FRIEND_DB_TYPE_UPDATE_FRIEND;
			DBWorkThreadMgr::Instance().SendReq(m_stDBReq);             
		}
		else
		{//回收TimeList中的节点,
			pstSwapNode = m_stTimeListHead.prev;
			poFriend = list_entry(pstSwapNode, Friend, m_stTimeListNode);
			DelTimeList(*poFriend);
			LOGWARN_r("NewFriend: frome TimeList!");
		}
		DelFriendFromMap(poFriend);
		poFriend->Reset();
		return poFriend;
	}
}

// 在这里开始初始化信息
int FriendMgr::GetFriendList(IN SS_PKG_FRIEND_GET_LIST_REQ& rstSSPkgBodyReq, OUT SS_PKG_FRIEND_GET_LIST_RSP& rstSSPkgBodyRsp)
{
	DT_FRIEND_PLAYER_INFO& rstPlayerInfo = rstSSPkgBodyReq.m_stPlayerInfo;

	Friend* poFriend = GetFriendByUin(rstPlayerInfo.m_ullUin);
	if (NULL == poFriend)
	{//玩家第一次登陆初始化
		LOGERR_r("Fatal Error!GetFriendList: poFriend is NULL,Uin<%lu>,Name<%s>",rstPlayerInfo.m_ullUin, rstPlayerInfo.m_szName);
		return ERR_SYS;
	}
	//更新数据
    LOGWARN_r("GetFriendListUpdate:Uin<%lu>", rstPlayerInfo.m_ullUin);
	UpdateFriendPlayerInfo(rstPlayerInfo);
	//这个是从Redis取出的
	memcpy(&rstSSPkgBodyRsp.m_stWholeData, &poFriend->m_stWholeDataFront, sizeof(DT_FRIEND_WHOLE_DATA_FRONT));
	memcpy(&rstSSPkgBodyRsp.m_stAgreeInfo, &poFriend->m_stAgreeInfo, sizeof(DT_FRIEND_AGREE_INFO));

	if (rstSSPkgBodyReq.m_ullResetTimestamp != 0)
	{
		LOGRUN("clear the send list. Uin<%lu>", rstPlayerInfo.m_ullUin);
		poFriend->ClearSendApList();
		AddDirtyList(*poFriend);
		DelTimeList(*poFriend);
	}
	rstSSPkgBodyRsp.m_ullResetTimestamp = rstSSPkgBodyReq.m_ullResetTimestamp;
	
	memcpy(&rstSSPkgBodyRsp.m_stSendApInfo, &poFriend->m_stSendInfo, sizeof(DT_FRIEND_SEND_AP_INFO));
	memcpy(&rstSSPkgBodyRsp.m_stRecvApInfo, &poFriend->m_stRecvInfo, sizeof(DT_FRIEND_RECV_AP_INFO));
	return ERR_NONE;

}

//向所有玩家的好友或向玩家操作(删除/同意/拒绝/申请)的好友发送通知
void FriendMgr::CastFriend(Friend* poFriendSender , uint64_t ullUinReceiver, uint8_t MsgId)
{
	if (NULL == poFriendSender )
	{
		LOGERR_r("poFriend is null");
		return;
	}
	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_EVENT_NTF;
	m_stSSRspPkg.m_stHead.m_ullUin = poFriendSender->m_ullUin;
	SS_PKG_FRIEND_EVENT_NTF& rstNtf = m_stSSRspPkg.m_stBody.m_stFriendEventNtf;
	rstNtf.m_bMsgId = MsgId;
	if (FRIEND_NTF_TYPE_UPDATE_PLAYER_INFO == MsgId)
	{//向poFriendSender的所有好友广播其信息
		for (int i = 0; i < poFriendSender->m_stAgreeInfo.m_wCount; i++)
		{
			rstNtf.m_Uins[i] = poFriendSender->m_stAgreeInfo.m_List[i];
		}
		rstNtf.m_wCount = poFriendSender->m_stAgreeInfo.m_wCount;
	}
	else
	{//向玩家操作(删除/同意/拒绝/申请)的好友发送通知
			rstNtf.m_Uins[0] = ullUinReceiver;
			rstNtf.m_wCount = 1;
	}
	memcpy(&rstNtf.m_stPlayerInfo, &poFriendSender->m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);

	/*@TODO 功能验收后删除  *************
	char *pBuf = new char[1024]; size_t t=0;
	rstNtf.visualize(pBuf, 1024, &t, -1, '|');
	LOGRUN_r("CastFriend = %s!!", pBuf);
	delete [] pBuf;
	//@TODO 功能验收后删除  *************/
	return ;
}


int FriendMgr::SearchFriend(uint64_t ullUin, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo)
{
	Friend* poFriend = GetFriendByUin(ullUin);
	if (NULL == poFriend)
	{
		return ERR_NOT_FOUND;
	}
	memcpy(&rstPlayerInfo, &poFriend->m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
	return ERR_NONE;
}

int FriendMgr::SearchFriend(const char* cszName, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo)
{
	Friend* poFriend = GetFriendByName(cszName);
	if (NULL == poFriend)
	{
		return ERR_NOT_FOUND;
	}
	//LOGERR("Search name<%s> addr<%p>", cszName, poFriend);
	memcpy(&rstPlayerInfo, &poFriend->m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
	return ERR_NONE;
}

Friend* FriendMgr::GetFriendByName(const char*  cszName)
{
	//先在内存中找
	m_stFriendNameAddrMapIter = m_stFriendNameAddrMap.find(cszName);
	if (m_stFriendNameAddrMapIter != m_stFriendNameAddrMap.end())
	{
		Move2TimeListFirst(*m_stFriendNameAddrMapIter->second);
		return m_stFriendNameAddrMapIter->second;
	}
	return NULL;
}

Friend* FriendMgr::GetFriendByName(const char*  cszName, uint64_t ullTokenId)
{
	//先在内存中找
	m_stFriendNameAddrMapIter = m_stFriendNameAddrMap.find(cszName);
	if (m_stFriendNameAddrMapIter != m_stFriendNameAddrMap.end())
	{
		//Move2TimeListFirst(*m_stFriendNameAddrMapIter->second);
		return m_stFriendNameAddrMapIter->second;
	}
	//内存中没有找到则向数据库处理线程发消息
	bzero(&m_stDBReq, sizeof(m_stDBReq));
	m_stDBReq.m_bType = FRIEND_DB_TYPE_GET_FRIEND_BY_NAME;
	StrCpy(m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, cszName, MAX_NAME_LENGTH);
	m_stDBReq.m_ullToken = ullTokenId;
	DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
	return NULL;
}


void FriendMgr::Update(bool bIdle)
{
	time_t tCurTimeMs = CGameTime::Instance().GetCurrTimeMs();
	Friend* poFriend = NULL;
	//脏数据个数达到条件,触发回写
	//上次回写数据库时间到现在的间隔达到要求,触发回写
	//脏数据个数优先级更高
	if (!(	(0 != m_iUpdateDirtyNum  && m_iDirtyNum >= m_iUpdateDirtyNum) || 
		(0 != m_iUpdateIntervalTime && tCurTimeMs - m_tLastUpdateTimeMs >=  m_iUpdateIntervalTime)	))
	{//不满足回写条件,
		return;
	}
	int iCheckNum = MIN(bIdle ? 100: 50, m_iDirtyNum);
	for (int iNum = 0; iNum < iCheckNum; iNum++)
	{
		poFriend = list_entry(m_stDirtyListHead.prev, Friend, m_stDirtyListNode);
		LOGWARN_r(" update poFriend to DB, id<%lu>, ", poFriend->m_ullUin);
		bzero(&m_stDBReq, sizeof(m_stDBReq));
		if (poFriend->PackWholeData(m_stDBReq.m_stWholeData) != TdrError::TDR_NO_ERROR)
		{
			LOGERR_r("pack PackWholeData failed");
			continue;
		}
		m_stDBReq.m_bType = FRIEND_DB_TYPE_UPDATE_FRIEND;
		DBWorkThreadMgr::Instance().SendReq(m_stDBReq);         
		DelDirtyList(*poFriend);
		DelFriendFromMap(poFriend);
		m_oFriendPool.DeleteData(poFriend);
	}
	m_tLastUpdateTimeMs = tCurTimeMs;
}

void FriendMgr::Fini()
{
	m_oUptIter.Begin();
	int iRet = 0;
	int count = 0;
	for ( ; !m_oUptIter.IsEnd(); m_oUptIter.Next() )
	{
		Friend* poFriend = m_oUptIter.CurrItem();
		if (NULL == poFriend)
		{
			LOGERR_r("poFriend is null");
			continue;
		}
		iRet = poFriend->PackWholeData(m_stDBReq.m_stWholeData);
		if (iRet != ERR_NONE)
		{
			LOGERR_r("pack poFriend(%lu) PackWholeData failed, Ret=%d", poFriend->m_ullUin, iRet);
			continue;
		}
		m_stDBReq.m_bType = FRIEND_DB_TYPE_UPDATE_FRIEND;
		DBWorkThreadMgr::Instance().SendReq(m_stDBReq);   
		
		m_stRedisReq.m_bType = FRIEND_REDIS_TYPE_SAVE_PLAYER_INFO;
		memcpy(&m_stRedisReq.m_stPlayerInfo, &poFriend->m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
		RedisWorkThreadMgr::Instance().SendReq(m_stRedisReq);

        ++count;
		count = count % 50;
		if ( 0 == count )
		{
			MsSleep(1); //让DB 线程消化
		}
	}
	return;
}


//更新玩家信息 并通知其好友
int FriendMgr::UpdateFriendPlayerInfo(IN DT_FRIEND_PLAYER_INFO& rstPlayerInfo)
{
	Friend* poFriend = GetFriendByUin(rstPlayerInfo.m_ullUin);
    LOGWARN_r("UpdateIN:Uin<%lu>",rstPlayerInfo.m_ullUin );
	if (NULL == poFriend)
	{
		LOGERR_r("poFriend<%lu> is NULL! ", rstPlayerInfo.m_ullUin);
		return ERR_SYS;
	}
    if (rstPlayerInfo.m_szName[0] == '\0')
    {
        LOGERR_r("rstPlayerInfo<%lu> is empty! ", rstPlayerInfo.m_ullUin);
        return ERR_NONE;
    }
    
	//玩家信息不会写Mysql,实时更新到Redis中,不用记录DirtyList
	poFriend->UpdatePlayerInfo(rstPlayerInfo);	
	CastFriend(poFriend, 0, FRIEND_NTF_TYPE_UPDATE_PLAYER_INFO);
	bzero(&m_stRedisReq, sizeof(m_stRedisReq));
	m_stRedisReq.m_bType = FRIEND_REDIS_TYPE_SAVE_PLAYER_INFO;
	memcpy(&m_stRedisReq.m_stPlayerInfo, &rstPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
	RedisWorkThreadMgr::Instance().SendReq(m_stRedisReq);
	return ERR_NONE;
}


//此函数只在GetUin调用,其它情况不调用
void FriendMgr::Move2TimeListFirst(Friend& poFriend)
{
	if (list_node_empty(&poFriend.m_stTimeListNode))
	{//TimeListNode为空,在DirtyList中,不必移动
		return;
	}
	//移动成为第一个
	//LOGWARN_r("Move2TimeListFirst <%lu> ", poFriend.m_ullUin);
	list_move(&poFriend.m_stTimeListNode, &m_stTimeListHead);
}

void FriendMgr::AddTimeList(Friend& poFriend)
{
	if (!list_node_empty(&poFriend.m_stTimeListNode))
	{//不为空,不用添加
		return;
	}
	//LOGWARN_r("AddTimeList <%lu> ", poFriend.m_ullUin);
	list_add(&poFriend.m_stTimeListNode, &m_stTimeListHead);
}

void FriendMgr::DelTimeList(Friend& poFriend)
{
	if (list_node_empty(&poFriend.m_stTimeListNode))
	{//为空,不用删除
		return;
	}
	//LOGWARN_r("DelTimeList <%lu> ", poFriend.m_ullUin);
	list_del(&poFriend.m_stTimeListNode);
}

void FriendMgr::AddDirtyList(Friend& poFriend)
{
	if (!list_node_empty(&poFriend.m_stDirtyListNode))
	{//不为空,不用添加
		return;
	}
	//LOGWARN_r("AddDirtyList <%lu> ", poFriend.m_ullUin);
	list_add(&poFriend.m_stDirtyListNode, &m_stDirtyListHead);
	m_iDirtyNum++;
}

void FriendMgr::DelDirtyList(Friend& poFriend)
{
	if (list_node_empty(&poFriend.m_stDirtyListNode))
	{//为空,不用删除
		return;
	}
	//LOGWARN_r("DelDirtyList <%lu> ", poFriend.m_ullUin);
	list_del(&poFriend.m_stDirtyListNode);
	m_iDirtyNum--;
	
}

void FriendMgr::ReleaseFriend(Friend* poFriend)
{
	if (NULL == poFriend)
	{
		return;
	}
	m_oFriendPool.DeleteData(poFriend);
}
Friend* FriendMgr::GetTimeListTail()
{
	if (list_empty(&m_stTimeListHead))
	{
		return NULL;
	}
	return list_entry( m_stTimeListHead.prev, Friend, m_stTimeListNode );
	
}

void FriendMgr::HandleDBThreadRsp(DT_FRIEND_DB_RSP& rstDBRsp)
{
	   //@TODO_DEBUG_DEL  删除
	   LOGWARN_r("_FriendDBTypeGetFriend to DB! Errno<%d> Type<%d>, Token<%lu> , Uin<%lu>, Name<%s> " , rstDBRsp.m_nErrNo,
		   rstDBRsp.m_bType ,rstDBRsp.m_ullToken, rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin, rstDBRsp.m_stWholeData.m_stBaseInfo.m_szName);
	   if (NULL != FriendMgr::GetFriendByUin(rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin))
	   {
		   //别的请求已获取到了,放弃本次得到的数据包,唤醒事务
		   FriendTransFrame::Instance().AsyncActionDone(rstDBRsp.m_ullToken, (void*)1, 1);
		   return;
	   }
	   Friend* poFriend = NULL;
	   if (ERR_NONE == rstDBRsp.m_nErrNo)
	   {
		   poFriend = NewFriend();
		   if (NULL != poFriend)
		   {
			   if (poFriend->InitFromDB(rstDBRsp.m_stWholeData))
			   {
				   AddFriendToMap(poFriend);   //加入map里
				   AddTimeList(*poFriend);
			   }
			   else
			   {
				   m_oFriendPool.DeleteData(poFriend);
				   poFriend = NULL;
			   }
		   }
	   }
	   //唤醒事务
	   FriendTransFrame::Instance().AsyncActionDone(rstDBRsp.m_ullToken, (void*)1, 1);
	   return;
}


//SS消息总接口, 事务完成或不做事务时直接调用处理
void FriendMgr::HandleSSMsg(IN SSPKG& rstSSReqPkg)
{
	switch (rstSSReqPkg.m_stHead.m_wMsgId)
	{
	case SS_MSG_FRIEND_HANDLE_REQ:
		_HandleSSReqMsgHandle(rstSSReqPkg);
		break;
	case SS_MSG_FRIEND_GET_LIST_REQ:
		_HandleSSReqMsgGetList(rstSSReqPkg); 
		break;
	case SS_MSG_FRIEND_SEARCH_REQ:
		_HandleSSReqMsgSearch(rstSSReqPkg);    
		break;
	case SS_MSG_FRIEND_EVENT_NTF:
		_HandleSSNtfMsgEvent(rstSSReqPkg);
		break;
	case SS_MSG_FRIEND_SEND_AP_REQ:
		_HandleSSReqMsgSendAp(rstSSReqPkg);
		break;
	case SS_MSG_FRIEND_GET_AP_REQ:
		_HandleSSReqMsgGetAp(rstSSReqPkg);
		break;
	case SS_MSG_FRIEND_CHANGE_NAME_REQ:
		_HandleSSReqMsgChangeName(rstSSReqPkg);
		break;
	default:
		break;
	}
}
//这个请求是玩家的 申请/添加/拒绝/同意  操作
void FriendMgr::_HandleSSReqMsgHandle(SSPKG& rstSSReqPkg)
{
	SS_PKG_FRIEND_HANDLE_REQ & rstSSPkgBodyReq = rstSSReqPkg.m_stBody.m_stFriendHandleReq;
	SS_PKG_FRIEND_HANDLE_RSP & rstSSPkgBodyRsp = m_stSSRspPkg.m_stBody.m_stFriendHandleRsp;
	rstSSPkgBodyRsp.m_ullUin = rstSSPkgBodyReq.m_ullUin;
	rstSSPkgBodyRsp.m_bHandleType = rstSSPkgBodyReq.m_bHandleType;
    Friend* poFriendSender = GetFriendByUin(rstSSReqPkg.m_stHead.m_ullUin);
    if (NULL != poFriendSender)
    {
        memcpy(&poFriendSender->m_stPlayerInfo, &rstSSPkgBodyReq.m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
    }
    
    int iRet = ERR_NONE;
    //逻辑处理
    if (rstSSReqPkg.m_stHead.m_ullUin == rstSSPkgBodyReq.m_ullUin)
    {
        LOGERR_r("Player<%lu> cheat,handle type<%d>!!!!!!!!", rstSSReqPkg.m_stHead.m_ullUin, rstSSPkgBodyReq.m_bHandleType);
        iRet = ERR_SYS;
    }
    else
    {
        switch (rstSSPkgBodyReq.m_bHandleType)
        {
        case FRIEND_HANDLE_TYPE_APPLY:
            //逻辑处理
            iRet = FriendMgr::Instance().ApplyFriend(rstSSReqPkg.m_stHead.m_ullUin, rstSSPkgBodyReq.m_ullUin);
            break;
        case FRIEND_HANDLE_TYPE_AGREE:
            iRet = FriendMgr::Instance().AgreeFriend(rstSSReqPkg.m_stHead.m_ullUin, rstSSPkgBodyReq.m_ullUin);
            break;
        case FRIEND_HANDLE_TYPE_DELETE:
            iRet = FriendMgr::Instance().DeleteFriend(rstSSReqPkg.m_stHead.m_ullUin, rstSSPkgBodyReq.m_ullUin);
            break;
        case FRIEND_HANDLE_TYPE_REJECT:
            iRet = FriendMgr::Instance().RejectFriend(rstSSReqPkg.m_stHead.m_ullUin, rstSSPkgBodyReq.m_ullUin);
            break;
        default:
            iRet = ERR_SYS;
            break;
        }
    }
	
	//填写回复包
	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_HANDLE_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSReqPkg.m_stHead.m_ullUin;
	rstSSPkgBodyRsp.m_nErrNo = iRet;
	rstSSPkgBodyRsp.m_bHandleType = rstSSPkgBodyReq.m_bHandleType;
	rstSSPkgBodyRsp.m_ullUin = rstSSPkgBodyReq.m_ullUin;
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	return;
}

//获取好友列表,同时会对玩家信息进行更新
void FriendMgr::_HandleSSReqMsgGetList(SSPKG& rstSSReqPkg)
{
	//准备
	SS_PKG_FRIEND_GET_LIST_REQ & rstSSPkgBodyReq = rstSSReqPkg.m_stBody.m_stFriendGetListReq;
	SS_PKG_FRIEND_GET_LIST_RSP & rstSSPkgBodyRsp = m_stSSRspPkg.m_stBody.m_stFriendGetListRsp;
	//逻辑处理
	int iRet = FriendMgr::Instance().GetFriendList(rstSSPkgBodyReq, rstSSPkgBodyRsp);
	//填写回复包
	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_GET_LIST_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSReqPkg.m_stHead.m_ullUin;
	rstSSPkgBodyRsp.m_nErrNo = iRet;
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	return;
}

//好友搜索
void FriendMgr::_HandleSSReqMsgSearch(SSPKG& rstSSReqPkg)
{
	SS_PKG_FRIEND_SEARCH_REQ & rstSSPkgBodyReq = rstSSReqPkg.m_stBody.m_stFriendSearchReq;
	SS_PKG_FRIEND_SEARCH_RSP & rstSSPkgBodyRsp = m_stSSRspPkg.m_stBody.m_stFriendSearchRsp;
	int iRet = ERR_NONE;

	//逻辑处理
	if (FRIEND_HANDLE_TYPE_SEARCH_UIN == rstSSPkgBodyReq.m_bType
        || FRIEND_HANDLE_TYPE_GET_PLAYER_INFO == rstSSPkgBodyReq.m_bType)
	{
		iRet = FriendMgr::Instance().SearchFriend(rstSSPkgBodyReq.m_ullUin,  rstSSPkgBodyRsp.m_stPlayerInfo);
	}
	else
	{
		iRet = FriendMgr::Instance().SearchFriend(rstSSPkgBodyReq.m_szName,  rstSSPkgBodyRsp.m_stPlayerInfo);
	}
	//填写回复包
	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_SEARCH_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSReqPkg.m_stHead.m_ullUin;
	rstSSPkgBodyRsp.m_nErrNo = iRet;
    rstSSPkgBodyRsp.m_bHandleType = rstSSPkgBodyReq.m_bType;
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	return;
}

//玩家信息变更通知 更新 上线/下线等
void FriendMgr::_HandleSSNtfMsgEvent(SSPKG& rstSSReqPkg)
{
	//预处理
	SS_PKG_FRIEND_EVENT_NTF & rstSSPkgBodyNtf = rstSSReqPkg.m_stBody.m_stFriendEventNtf;
    if (rstSSPkgBodyNtf.m_stPlayerInfo.m_szName[0] == '\0' || rstSSPkgBodyNtf.m_stPlayerInfo.m_bMajestyLevel == 0)
    {
        LOGERR_r("PlayerInfo<%lu> is empty", rstSSPkgBodyNtf.m_stPlayerInfo.m_ullUin);
        return;
    }
	//逻辑处理
    LOGWARN_r("Update:Uin<%lu>", rstSSPkgBodyNtf.m_stPlayerInfo.m_ullUin);
	if (FRIEND_HANDLE_TYPE_UPDATE_PLAYER_INFO == rstSSPkgBodyNtf.m_bMsgId)
	{//这个更新 包含等级变化 下线通知等
		UpdateFriendPlayerInfo(rstSSPkgBodyNtf.m_stPlayerInfo);
	}
	return;
}

//赠送体力给多位好友
void FriendMgr::_HandleSSReqMsgSendAp(SSPKG& rstSSReqPkg)
{
	SS_PKG_FRIEND_SEND_AP_REQ& rstSSReqPkgBody = rstSSReqPkg.m_stBody.m_stFriendSendApReq;
	uint64_t ullUinSender = rstSSReqPkg.m_stHead.m_ullUin;
	Friend* poSender = GetFriendByUin(ullUinSender);
	if (poSender == NULL)
	{
		LOGRUN("Friend uin<%lu> is not in found.", ullUinSender);
		return;
	}

	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_SEND_AP_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSReqPkg.m_stHead.m_ullUin;
	SS_PKG_FRIEND_SEND_AP_RSP& rstPkgRsp = m_stSSRspPkg.m_stBody.m_stFriendSendApRsp;
	rstPkgRsp.m_stReceiverListInfo.m_wCount = 0;

	for (int i=0; i<rstSSReqPkgBody.m_wCount; i++)
	{
		uint64_t ullUinReceiver = rstSSReqPkgBody.m_ApReceiverUins[i];
		poSender->Add2SendApList(ullUinReceiver);

		Friend* poReceiver = GetFriendByUin(ullUinReceiver);
		if (poReceiver == NULL)
		{
			LOGRUN("Friend uin<%lu> is not in found.", ullUinReceiver);
			continue;
		}
		
		if (poReceiver->Add2RecvApList(ullUinSender) == ERR_NONE)
		{
			rstPkgRsp.m_stReceiverListInfo.m_List[rstPkgRsp.m_stReceiverListInfo.m_wCount++] = ullUinReceiver;
		}
		

		AddDirtyList(*poReceiver);
		DelTimeList(*poReceiver);
	}
	if (rstSSReqPkgBody.m_wCount != 0)
	{
		AddDirtyList(*poSender);
		DelTimeList(*poSender);
	}
	
	memcpy(&rstPkgRsp.m_stSendApInfo, &poSender->m_stSendInfo, sizeof(DT_FRIEND_SEND_AP_INFO));
	rstPkgRsp.m_nErrNo = ERR_NONE;
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	return;
}

//获取被赠送的体力
void FriendMgr::_HandleSSReqMsgGetAp(SSPKG& rstSSReqPkg)
{
	SS_PKG_FRIEND_GET_AP_RSP& rstSSRspPkgBody = m_stSSRspPkg.m_stBody.m_stFriendGetApRsp;
	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_GET_AP_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSReqPkg.m_stHead.m_ullUin;
	rstSSRspPkgBody.m_nCount = 0;

	SS_PKG_FRIEND_GET_AP_REQ& rstSSReqPkgBody = rstSSReqPkg.m_stBody.m_stFriendGetApReq;
	uint64_t ullUinPuller = rstSSReqPkg.m_stHead.m_ullUin;
	Friend* poPuller = GetFriendByUin(ullUinPuller);
	if (poPuller == NULL)
	{
		LOGRUN("Friend uin<%lu> is not in found.", ullUinPuller);
		return;
	}

    for (int i = 0; i < rstSSReqPkgBody.m_wCount; i++) 
	{
        uint64_t ullUinGiver = rstSSReqPkgBody.m_ApGiverUins[i];
		if (poPuller->IsInRecvApList(ullUinGiver))
		{
			rstSSRspPkgBody.m_nCount++;
			poPuller->DeleteRecvApList(ullUinGiver);
		}
    }
	AddDirtyList(*poPuller);
	DelTimeList(*poPuller);
	memcpy(&rstSSRspPkgBody.m_stRecvApInfo, &poPuller->m_stRecvInfo, sizeof(DT_FRIEND_RECV_AP_INFO));
	rstSSRspPkgBody.m_nErrNo = ERR_NONE;
	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	return;
}

void FriendMgr::_HandleSSReqMsgChangeName(SSPKG& rstSSNtfPkg)
{
	Friend* poFriend = GetFriendByUin(rstSSNtfPkg.m_stHead.m_ullUin);

	if (poFriend == NULL)
	{
		LOGRUN("Friend uin<%lu> is not in found.", rstSSNtfPkg.m_stHead.m_ullUin);
		return;
	}
	

	DelFriendFromMap(poFriend);
	memcpy(poFriend->m_szName, rstSSNtfPkg.m_stBody.m_stFriendChangeNameReq.m_szName, MAX_NAME_LENGTH);
	AddFriendToMap(poFriend);
	AddDirtyList(*poFriend);
	DelTimeList(*poFriend);

	m_stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_CHANGE_NAME_RSP;
	m_stSSRspPkg.m_stHead.m_ullUin = rstSSNtfPkg.m_stHead.m_ullUin;
	SS_PKG_FRIEND_CHANGE_NAME_RSP& rstBodyRsp = m_stSSRspPkg.m_stBody.m_stFriendChangeNameRsp;
	memcpy(rstBodyRsp.m_szName, rstSSNtfPkg.m_stBody.m_stFriendChangeNameReq.m_szName, MAX_NAME_LENGTH);
	memcpy(&rstBodyRsp.m_stAgreeInfo, &poFriend->m_stAgreeInfo, sizeof(DT_FRIEND_AGREE_INFO));

	FriendSvrMsgLayer::Instance().SendToZoneSvr(m_stSSRspPkg);
	
	return;
}

void FriendMgr::HandleRedisThreadRsp(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp)
{
	switch (rstRsp.m_bType)
	{
	case FRIEND_REDIS_TYPE_GET_AGREE_APPLY_PLAYER_INFO:
		_HandleRedisGetApplyAgree(rstRsp);
		break;
	case FRIEND_REDIS_TYPE_GET_PLAYER_INFO:
		_HandleRedisGetPlayerInfo(rstRsp);
		break;
	default:
		break;
	}
}

void FriendMgr::_HandleRedisGetApplyAgree(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp)
{
	Friend* poFriend = GetFriendByUin(rstRsp.m_stPlayerInfo.m_ullUin);
	if (NULL == poFriend)
	{
		LOGERR_r("_HandleRedisGetApplyAgree:poFriend is null !");
		FriendTransFrame::Instance().AsyncActionDone(rstRsp.m_ullToken, (void*)1, 1);
		return;
	}
	memcpy(&poFriend->m_stWholeDataFront, &rstRsp.m_stWholeDataFront, sizeof(DT_FRIEND_WHOLE_DATA_FRONT));
	FriendTransFrame::Instance().AsyncActionDone(rstRsp.m_ullToken, (void*)1, 1);
	return;
}

void FriendMgr::_HandleRedisGetPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp)
{
	Friend* poFriend = GetFriendByUin(rstRsp.m_stPlayerInfo.m_ullUin);
	if (NULL == poFriend)
	{
		LOGERR_r("_HandleRedisGetPlayerInfo:poFriend is null !");
		FriendTransFrame::Instance().AsyncActionDone(rstRsp.m_ullToken, (void*)1, 1);
		return;
	}
	memcpy(&poFriend->m_stPlayerInfo, &rstRsp.m_stPlayerInfo, sizeof(rstRsp.m_stPlayerInfo));
	FriendTransFrame::Instance().AsyncActionDone(rstRsp.m_ullToken, (void*)1, 1);
	return;
}
