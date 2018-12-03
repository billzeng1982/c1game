#pragma once
#include "define.h"
#include "ss_proto.h"
#include "../framework/FriendSvrMsgLayer.h"
#include "object.h"
#include "IndexedPriorityQ.h"
#include "PriorityQueue.h"


using namespace PKGMETA;



struct Friend
{
	list_head m_stTimeListNode;			// LRU TimeList �ڵ�.
	list_head m_stDirtyListNode;		//��д������,�����ڴ��
	uint64_t m_ullUin;							
	char m_szName[MAX_NAME_LENGTH];                                         // �ǳ�
	DT_FRIEND_APPLY_INFO m_stApplyInfo;
	DT_FRIEND_AGREE_INFO m_stAgreeInfo;
	DT_FRIEND_SEND_AP_INFO m_stSendInfo;
	DT_FRIEND_RECV_AP_INFO m_stRecvInfo;
	DT_FRIEND_PLAYER_INFO m_stPlayerInfo;
	DT_FRIEND_WHOLE_DATA_FRONT m_stWholeDataFront;					//���к��Ѻ������ߵ���Ϣ  ����
public:
	void Reset();
	bool InitFromDB(DT_FRIEND_WHOLE_DATA &rstWholedata);
	int PackWholeData(OUT DT_FRIEND_WHOLE_DATA& rstWholedata);

	bool IsInApplyList(uint64_t ullUin);
	bool IsInAgreeList(uint64_t ullUin);
	int Add2ApplyList(uint64_t ullUin);
	int Add2AgreeList(uint64_t ullUin);
	void DeleteApplyList(uint64_t ullUin);
	void DeleteAgreeList(uint64_t ullUin);
	bool IsInSendApList(uint64_t ullUin);
	bool IsInRecvApList(uint64_t ullUin);
	int Add2SendApList(uint64_t ullUin);
	int Add2RecvApList(uint64_t ullUin);
	void DeleteSendApList(uint64_t ullUin);
	void DeleteRecvApList(uint64_t ullUin);
	void ClearSendApList();

	void UpdatePlayerInfo(DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
	void InitFriend(DT_FRIEND_PLAYER_INFO& rstPlayerInfo);
};