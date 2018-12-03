#pragma once

#include "define.h"
#include "singleton.h"
#include "mempool.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "mysql/MysqlHandler.h"
#include "../cfg/FriendSvrCfgDesc.h"
#include "hash_func.h"
#include "./FriendInfo/FriendTable.h"
#include <time.h>  
#include <map>
#include "./FriendInfo/Friend.h"
#include "./FriendInfo/DBWorkThread.h"
#include "../framework/FriendTransFrame.h"
#include "./PlayerInfo/RedisWorkThreadMgr.h"
using namespace PKGMETA;


class FriendMgr : public TSingleton<FriendMgr>
{
public:
	typedef hash_map_t< const char*, Friend*, __gnu_cxx::hash<const char*>, eqstr > FriendNameAddrMap_t;
	typedef map<uint64_t, Friend*> FriendUinAddrMap_t;
	const static uint16_t UPDATE_FREQ = 1000; // 玩家更新频率, ms
public:
	FriendMgr();
	virtual ~FriendMgr();

public:
	void HandleSSMsg(IN SSPKG& rstSSPkg);       //SS消息总接口, 事务完成或不做事务时直接调用处理
private: 
	//具体SS消息处理接口,在里面做回复
	void _HandleSSReqMsgHandle(SSPKG& rstSSReqPkg);                 //这个请求是玩家的 申请/添加/拒绝/同意  操作
	void _HandleSSReqMsgGetList(SSPKG& rstSSReqPkg);                //获取好友列表,同时会对玩家信息进行更新
	void _HandleSSReqMsgSearch(SSPKG& rstSSReqPkg);                 //好友搜索
	void _HandleSSNtfMsgEvent(SSPKG& rstSSReqPkg);                  //玩家信息变更通知 更新 上线/下线等
	void _HandleSSReqMsgSendAp(SSPKG& rstSSReqPkg);					//赠送体力给多位好友
	void _HandleSSReqMsgGetAp(SSPKG& rstSSReqPkg);					//获取被赠送的体力
	void _HandleSSReqMsgChangeName(SSPKG& rstSSNtfPkg);				//好友改名

public:
	int ApplyFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int AgreeFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int RejectFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int DeleteFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int GetFriendList(IN SS_PKG_FRIEND_GET_LIST_REQ& rstSSPkgBodyReq, OUT SS_PKG_FRIEND_GET_LIST_RSP& rstSSPkgBodyRsp);
	int SearchFriend(uint64_t ullUin, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
	int SearchFriend(const char*  cszName, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
	int UpdateFriendPlayerInfo(IN DT_FRIEND_PLAYER_INFO& rstPlayerInfo);                                                    //更新玩家信息 并通知其好友
public:
	bool Init(FRIENDSVRCFG* pstConfig) ;
	void Fini();
	void Update(bool bIdole);
	void Destory();
	void CastFriend(Friend* poFriendSender , uint64_t ullUinReceiver, uint8_t MsgId);                                       //向所有玩家的好友或向玩家操作(删除/同意/拒绝/申请)的好友发送通知
	void AddFriendToMap(Friend* poFriend);        //加入2张map里,并加入到TimeNodeList里
	void DelFriendFromMap(Friend* poFriend);
	Friend* GetFriendByUin(uint64_t ullUin);
	Friend* GetFriendByName(const char*  cszName);
	Friend* NewFriend();
	void ReleaseFriend(Friend* poFriend);
	void AddTimeList(Friend& poFriend);
	void Move2TimeListFirst(Friend& poFriend);
	void DelTimeList(Friend& poFriend);
	void AddDirtyList(Friend& poFriend);
	void DelDirtyList(Friend& poFriend);
	Friend* GetTimeListTail();
public:
	void HandleDBThreadRsp(DT_FRIEND_DB_RSP& rstDBRsp);
	Friend* GetFriendByUin(uint64_t ullUin, uint64_t ullTokenId);
	Friend* GetFriendByName(const char*  cszName, uint64_t ullTokenId);

public:
	void HandleRedisThreadRsp(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp);
	void _HandleRedisGetApplyAgree(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp);
	void _HandleRedisGetPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp);
public:
	list_head m_stTimeListHead;		// LRU TimeList 头节点
	list_head m_stDirtyListHead;	//待写数据链表头,回收内存池
	int m_iDirtyNum;				//待写数据个数
	time_t m_tLastUpdateTimeMs;		//上次回写数据库时间
	//先检查脏数据个数是否达到条件,达到条件回写
	//再检查上次回写时间,达到条件回写
	// 
	int m_iUpdateIntervalTime;			//回写策略, 间隔时间(毫秒) 
	int m_iUpdateDirtyNum;				//回写策略,脏数据个数


	DT_FRIEND_DB_REQ m_stDBReq;
	DT_FRIEND_PLAYERINFO_REDIS_REQ m_stRedisReq;
		  

private:
	SSPKG m_stSSRspPkg;
	CMemPool<Friend>::UsedIterator m_oUptIter;   // update iterator;
	// player data
	DT_FRIEND_WHOLE_DATA m_oFriendWholeData;
	DT_FRIEND_PLAYER_INFO m_stFriendPlayerInfo;
	DT_FRIEND_AGREE_FRONT_INFO m_stFriendAgreeFrontInfo;
	DT_FRIEND_AGREE_FRONT_INFO m_stFriendApplyFrontInfo;

	uint32_t m_dwFriendMaxNum;												// 内存池中的最大数量
	FriendUinAddrMap_t m_stFriendUinAddrMap;                                // Uin索引Friend信息
	FriendNameAddrMap_t m_stFriendNameAddrMap;                              // 昵称索引Friend信息
	FriendUinAddrMap_t::iterator m_stFriendUinAddrMapIter;                  // Uin索引Friend信息
	FriendNameAddrMap_t::iterator m_stFriendNameAddrMapIter;                // Uin索引Friend信息
	CMemPool<Friend> m_oFriendPool;									    // 好友池
};

