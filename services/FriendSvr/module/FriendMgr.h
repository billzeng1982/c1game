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
	const static uint16_t UPDATE_FREQ = 1000; // ��Ҹ���Ƶ��, ms
public:
	FriendMgr();
	virtual ~FriendMgr();

public:
	void HandleSSMsg(IN SSPKG& rstSSPkg);       //SS��Ϣ�ܽӿ�, ������ɻ�������ʱֱ�ӵ��ô���
private: 
	//����SS��Ϣ����ӿ�,���������ظ�
	void _HandleSSReqMsgHandle(SSPKG& rstSSReqPkg);                 //�����������ҵ� ����/���/�ܾ�/ͬ��  ����
	void _HandleSSReqMsgGetList(SSPKG& rstSSReqPkg);                //��ȡ�����б�,ͬʱ��������Ϣ���и���
	void _HandleSSReqMsgSearch(SSPKG& rstSSReqPkg);                 //��������
	void _HandleSSNtfMsgEvent(SSPKG& rstSSReqPkg);                  //�����Ϣ���֪ͨ ���� ����/���ߵ�
	void _HandleSSReqMsgSendAp(SSPKG& rstSSReqPkg);					//������������λ����
	void _HandleSSReqMsgGetAp(SSPKG& rstSSReqPkg);					//��ȡ�����͵�����
	void _HandleSSReqMsgChangeName(SSPKG& rstSSNtfPkg);				//���Ѹ���

public:
	int ApplyFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int AgreeFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int RejectFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int DeleteFriend(uint64_t ullUin, uint64_t ullFriendUin);
	int GetFriendList(IN SS_PKG_FRIEND_GET_LIST_REQ& rstSSPkgBodyReq, OUT SS_PKG_FRIEND_GET_LIST_RSP& rstSSPkgBodyRsp);
	int SearchFriend(uint64_t ullUin, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
	int SearchFriend(const char*  cszName, OUT DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
	int UpdateFriendPlayerInfo(IN DT_FRIEND_PLAYER_INFO& rstPlayerInfo);                                                    //���������Ϣ ��֪ͨ�����
public:
	bool Init(FRIENDSVRCFG* pstConfig) ;
	void Fini();
	void Update(bool bIdole);
	void Destory();
	void CastFriend(Friend* poFriendSender , uint64_t ullUinReceiver, uint8_t MsgId);                                       //��������ҵĺ��ѻ�����Ҳ���(ɾ��/ͬ��/�ܾ�/����)�ĺ��ѷ���֪ͨ
	void AddFriendToMap(Friend* poFriend);        //����2��map��,�����뵽TimeNodeList��
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
	list_head m_stTimeListHead;		// LRU TimeList ͷ�ڵ�
	list_head m_stDirtyListHead;	//��д��������ͷ,�����ڴ��
	int m_iDirtyNum;				//��д���ݸ���
	time_t m_tLastUpdateTimeMs;		//�ϴλ�д���ݿ�ʱ��
	//�ȼ�������ݸ����Ƿ�ﵽ����,�ﵽ������д
	//�ټ���ϴλ�дʱ��,�ﵽ������д
	// 
	int m_iUpdateIntervalTime;			//��д����, ���ʱ��(����) 
	int m_iUpdateDirtyNum;				//��д����,�����ݸ���


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

	uint32_t m_dwFriendMaxNum;												// �ڴ���е��������
	FriendUinAddrMap_t m_stFriendUinAddrMap;                                // Uin����Friend��Ϣ
	FriendNameAddrMap_t m_stFriendNameAddrMap;                              // �ǳ�����Friend��Ϣ
	FriendUinAddrMap_t::iterator m_stFriendUinAddrMapIter;                  // Uin����Friend��Ϣ
	FriendNameAddrMap_t::iterator m_stFriendNameAddrMapIter;                // Uin����Friend��Ϣ
	CMemPool<Friend> m_oFriendPool;									    // ���ѳ�
};

