#pragma once
#include <set>
#include <map>
#include "utils/singleton.h"
#include "mng/mempool.h"
#include "mng/DynMempool.h"
#include "../cfg/GuildExpeditionSvrCfgDesc.h"
#include "comm/tlist.h"
#include "coroutine/CoDataFrame.h"
#include "ss_proto.h"
#include "../framework/GuildExpeditionSvrMsgLayer.h"
#include "./Guild.h"
#include "./Player.h"


#define DATA_TYPE_GUILD     1
#define DATA_TYPE_PLAYER    2




struct DataKey
{
    uint8_t     m_bType;    //1#Guild|2#Player
    uint64_t    m_ullKey;
};
struct DataResult
{
    DataResult() { }
    ~DataResult() { Reset(); }
    void Reset(){ m_bType = 0; m_pstData = NULL; }
    uint8_t m_bType;        //1#Guild|2#Player
    void* m_pstData;
};

class AsyncGetDataAction : public CoGetDataAction
{
public:
	AsyncGetDataAction() { m_stKey.m_ullKey = 0; m_stKey.m_bType = 0; }
    virtual ~AsyncGetDataAction() {}

    virtual bool Execute()
    {
		PKGMETA::SSPKG* pstSsPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
		if (pstSsPkg == NULL)
		{
			return false;
		}
        if (m_stKey.m_bType == DATA_TYPE_GUILD)
        {

			pstSsPkg->m_stHead.m_wMsgId = PKGMETA::SS_MSG_GUILD_EXPEDITION_GET_GUILD_DATA_REQ;
            PKGMETA::SS_PKG_GUILD_EXPEDITION_GET_GUILD_DATA_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionGetGuildDataReq;
            rstReq.m_Key[0] = m_stKey.m_ullKey;
            rstReq.m_chCount = 1;
            rstReq.m_ullTokenId = this->GetToken();
            GuildExpeditionSvrMsgLayer::Instance().SendToClusterDBSvr(pstSsPkg);
        }
        else
        {

			pstSsPkg->m_stHead.m_wMsgId = PKGMETA::SS_MSG_GUILD_EXPEDITION_GET_PLAYER_DATA_REQ;
            PKGMETA::SS_PKG_GUILD_EXPEDITION_GET_PLAYER_DATA_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionGetPlayerDataReq;
            rstReq.m_Key[0] = m_stKey.m_ullKey;
			rstReq.m_chCount = 1;
            rstReq.m_ullTokenId = this->GetToken();
			GuildExpeditionSvrMsgLayer::Instance().SendToClusterDBSvr(pstSsPkg);
        }

        return true;
    }
    void SetKey(DataKey& rstKey) { m_stKey = rstKey; }

private:
    DataKey m_stKey;
};



class DataMgr : public TSingleton<DataMgr>, public CoDataFrame
{


public:
    typedef std::map<uint64_t, Guild*> Key2Guild_t;
    typedef std::map<uint64_t, Player*> Key2Player_t;
public:
    DataMgr() {};
    ~DataMgr() {};

    bool Init(GUILDEXPEDITIONSVRCFG* pstConfig);
    void Update(bool bIdle);
    void Fini();

    
public:
    //(继承自CoDataFrame,由继承的类具体实现) 判断数据是否在内存中
    virtual bool IsInMem(void* pResult);

    //(继承自CoDataFrame,由继承的类具体实现) 将结果保存在内存中
    virtual bool SaveInMem(void* pResult);
protected:
    //(继承自CoDataFrame,由继承的类具体实现) 通过key在内存中查找数据
    virtual void* _GetDataInMem(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 创建获取数据action
    virtual CoGetDataAction* _CreateGetDataAction(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 释放获取数据的action
    virtual void _ReleaseGetDataAction(CoGetDataAction* poAction);


public:
	//获取一个新节点
	Guild* GetNewGuild();
	Player* GetNewPlayer();
	//匹配工会
	int MatchGuild(uint64_t ullGuildID, uint64_t ullLi, OUT PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO *pMatchedGuildIdList);

    //增加到Time表
    void AddToTimeList(Guild* pstGuild);
	void AddToTimeList(Player* pstPlayer);

    //增加到Dirty表
    void AddToDirtyList(Guild* pstGuild);
	void AddToDirtyList(Player* pstPlayer);

    //移动到TimeList的首个
    void MoveToTimeListFirst(Guild* pstGuild);
	void MoveToTimeListFirst(Player* pstPlayer);

    //从Time表删除
    void DelFromTimeList(Guild* pstGuild);
	void DelFromTimeList(Player* pstPlayer);

    //从Dirty表删除
    void DelFromDirtyList(Guild* pstGuild);
	void DelFromDirtyList(Player* pstPlayer);

	//增加到匹配列表
	void AddToMatchList(Guild* pstGuild);
	void DelFromMatchList(Guild* pstGuild);
protected:
    void _WriteDirtyToDB();

public:
	 //保存到数据库
	 void SaveGuild(Guild* pstGuild);

	 void DelGuild(Guild* pstGuild);

	 void SavePlayer(Player* pstPlayer);
	 void AddGuildToMap(Guild* pstGuild);
	 void DelGuildFromMap(Guild* pstGuild);
	 void AddPlayerToMap(Player* pstPlayer);
	 void DelPlayerToMap(Player* pstPlayer);
     Guild* GetGuild(uint64_t ullUid);

     Player* GetPlayer(uint64_t ullUin);







private:
    int                                         m_iUptHour;
    uint64_t                                    m_ullLastUptTimeMs;           //上次更新时间





private:
    GUILDEXPEDITIONSVRCFG*                      m_pstConfig;

    TLISTNODE                                   m_stGuildDirtyListHead;          //脏数据链表头,用于回写数据库
    TLISTNODE                                   m_stGuildTimeListHead;           //时间链表头,用于LRU算法
    uint32_t                                    m_dwGuildDirtyListSize;          //脏数据链表长度
    uint32_t                                    m_dwGuildTimeListSize;           //时间链表长度长度
    uint32_t                                    m_dwGuildWriteTimeVal;           //回写的时间间隔
    uint64_t                                    m_ullGuildLastWriteTimestamp;    //上次回写脏数据的时间

	TLISTNODE                                   m_stPlayerDirtyListHead;          //脏数据链表头,用于回写数据库
	TLISTNODE                                   m_stPlayerTimeListHead;           //时间链表头,用于LRU算法
	uint32_t                                    m_dwPlayerDirtyListSize;          //脏数据链表长度
	uint32_t                                    m_dwPlayerTimeListSize;           //时间链表长度长度
	uint32_t                                    m_dwPlayerWriteTimeVal;           //回写的时间间隔
	uint64_t                                    m_ullPlayerLastWriteTimestamp;    //上次回写脏数据的时间

	

	std::map<uint64_t, Guild*>					m_oMatchMap;
	std::map<uint64_t, Guild*>::iterator			m_oMatchMapIter;
    Key2Guild_t                                 m_oGuildMap;
    Key2Guild_t::iterator                       m_oGuildMapIter;
    Key2Player_t                                m_oPlayerMap;
    Key2Player_t::iterator                      m_oPlayerMapIter;

    DynMempool<AsyncGetDataAction>				m_oAsyncActionPool;
    DynMempool<Guild>							m_oGuildPool;
    DynMempool<Player>							m_oPlayerPool;
};


