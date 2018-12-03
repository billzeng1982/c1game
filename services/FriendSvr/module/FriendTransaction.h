#pragma once


#include "object.h"
#include "TransactionFrame.h"
#include "../module/FriendInfo/Friend.h"
#include "ss_proto.h"

using namespace PKGMETA;

class FriendTransaction : public Transaction
{
public:
	FriendTransaction() {}
	virtual ~FriendTransaction() {}

	virtual void Reset() ;
	virtual void OnFinished(int iErrCode);
public:
	void SetSendUin(uint64_t ullUin){ m_ullSendUin = ullUin; }
	void SetRecvUin(uint64_t ullUin){ m_ullRecvUin = ullUin; }
	void SetSendFriend(Friend* poFriend) { m_poSendFriend = poFriend; }
	void SetRecvFriend(Friend* poFriend) { m_poRecvFriend = poFriend; }
	uint64_t GetSendUin() { return m_ullSendUin; }
	uint64_t GetRecvUin() { return m_ullRecvUin; }
	Friend* GetSendFriend() { return m_poSendFriend; }
	Friend* GetRecvFriend() { return m_poRecvFriend; }
private:
// 	void _HandleGetListReq(int iErrNo);
// 	void _HandleHandleReq(int iErrNo);
// 	void _HandleSearchReq(int iErrNo);
public:
	SSPKG m_stSsReqPkg;
	SSPKG m_stSsRspPkg;
	int m_iActionErrNo;
private:
	uint64_t m_ullSendUin;      //���Ѳ����������
	uint64_t m_ullRecvUin;      //�������߱�֪ͨ���
	Friend* m_poSendFriend;     //���Ѳ����������
	Friend* m_poRecvFriend;     //�������߱�֪ͨ���
};


class GetFriendAction : public IAction
{
public:
	GetFriendAction() { }
	virtual ~GetFriendAction() { }
	virtual void Reset();
	virtual void OnFinished() { LOGRUN_r("GetFriendAction OnFinished! Uin<%lu>, Token<%lu>",m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, (uint64_t)GetToken()); }
	virtual int Execute(Transaction* pObjTrans);
	virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);
	void SetPlayerUin(uint64_t ullUin){ m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin = ullUin;}
	void SetPlayerName(const char* pszName) { StrCpy(m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, pszName, MAX_NAME_LENGTH); }
	void SetUinType(uint8_t bType){ m_bGetType = bType;}

private:
	uint8_t m_bGetType; //=1#ͨ��Uin��ȡ(Ĭ��ֵ)|����#ͨ�����ֻ�ȡ
	FriendTransaction* m_pObjTrans;
	DT_FRIEND_DB_REQ m_stDBReq;
};

//��GetFriendList����: ������ݿ�û��,��Ҫ�����µļ�¼
class GetCreateFriendAction : public IAction
{
public:
	GetCreateFriendAction() { }
	virtual ~GetCreateFriendAction() { }
	virtual void Reset();
	virtual void OnFinished() { LOGRUN_r("GetCreateFriendAction OnFinished! Uin<%lu>, Token<%lu>",m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin, (uint64_t)GetToken()); }
	virtual int Execute(Transaction* pObjTrans);
	virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);
	void SetPlayerUin(uint64_t ullUin){ m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin = ullUin;}
	void SetPlayerName(const char* pszName) { StrCpy(m_stDBReq.m_stWholeData.m_stBaseInfo.m_szName, pszName, MAX_NAME_LENGTH); }

private:
	FriendTransaction* m_pObjTrans;
	DT_FRIEND_DB_REQ m_stDBReq;
};


//��ȡ���к��Ѻ���������Ϣ�Ķ���
class GetAgreeApplyAction : public IAction
{
public:
	GetAgreeApplyAction() { }
	virtual ~GetAgreeApplyAction() { }
	virtual void Reset();
	virtual void OnFinished() { LOGRUN_r("GetAgreeApplyAction OnFinished! Uin<%lu>, Token<%lu>",m_stReq.m_stPlayerInfo.m_ullUin, (uint64_t)GetToken()); }
	virtual int Execute(Transaction* pObjTrans);
	virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);
	void SetPlayerUin(uint64_t ullUin){ m_stReq.m_stPlayerInfo.m_ullUin = ullUin;}
private:
	FriendTransaction* m_pObjTrans;
	DT_FRIEND_PLAYERINFO_REDIS_REQ m_stReq;
};

//��ȡ�����Ϣ
class GetPlayerInfoAction : public IAction
{
public:
	GetPlayerInfoAction() { }
	virtual ~GetPlayerInfoAction() { }
	virtual void Reset();
	virtual void OnFinished() { LOGRUN_r("GetPlayerInfoAction OnFinished! Uin<%lu>, Token<%lu>",m_stReq.m_stPlayerInfo.m_ullUin , (uint64_t)GetToken()); }
	virtual int Execute(Transaction* pObjTrans);
	virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);
	void SetPlayerUin(uint64_t ullUin){ m_stReq.m_stPlayerInfo.m_ullUin  = ullUin;}
	void SetPlayerName(const char* pszName){ StrCpy(m_stReq.m_stPlayerInfo.m_szName, pszName, MAX_NAME_LENGTH);}
private:
	FriendTransaction* m_pObjTrans;
	DT_FRIEND_PLAYERINFO_REDIS_REQ m_stReq;
};
