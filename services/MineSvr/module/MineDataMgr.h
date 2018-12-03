#pragma once
#include <set>
#include <map>
#include "singleton.h"
#include "mempool.h"
#include "DynMempool.h"
#include "../cfg/MineSvrCfgDesc.h"
#include "comm/tlist.h"
#include "coroutine/CoDataFrame.h"
#include "ss_proto.h"
#include "../framework/MineSvrMsgLayer.h"
#include "./MineOre.h"
#include "./MinePlayer.h"
#include "./MineUidSet.h"

#define DATA_TYPE_ORE       1
#define DATA_TYPE_PLAYER    2

struct DataKey
{
    uint8_t     m_bType;    //1#Ore|2#Player
    uint64_t    m_ullKey;
};
struct DataResult
{
    DataResult() { }
    ~DataResult() { Reset(); }
    void Reset(){ m_bType = 0; m_pstData = NULL; }
    uint8_t m_bType;        //1#Ore|2#Player
    void* m_pstData;
};

class AsyncGetOreDataAction : public CoGetDataAction
{
public:
    AsyncGetOreDataAction() { m_stKey.m_ullKey = 0; m_stKey.m_bType = 0; }
    virtual ~AsyncGetOreDataAction() {}

    virtual bool Execute()
    {
        if (m_stKey.m_bType == DATA_TYPE_ORE)
        {
            PKGMETA::SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
            rstSsPkg.m_stHead.m_wMsgId = PKGMETA::SS_MSG_MINE_GET_ORE_DATA_REQ;
            PKGMETA::SS_PKG_MINE_GET_ORE_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stMineGetOreDataReq;
            rstReq.m_OreUidList[0] = m_stKey.m_ullKey;
            rstReq.m_bOreCount = 1;
            rstReq.m_ullTokenId = this->GetToken();
            MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
        }
        else
        {
            PKGMETA::SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
            rstSsPkg.m_stHead.m_wMsgId = PKGMETA::SS_MSG_MINE_GET_PLAYER_DATA_REQ;
            PKGMETA::SS_PKG_MINE_GET_PLAYER_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stMineGetPlayerDataReq;
            rstReq.m_ullPlayerUin = m_stKey.m_ullKey;
            rstReq.m_ullTokenId = this->GetToken();
            MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
        }

        return true;
    }
    void SetKey(DataKey& rstKey) { m_stKey = rstKey; }

private:
    DataKey m_stKey;
};



class MineDataMgr : public TSingleton<MineDataMgr>, public CoDataFrame
{
public:
    static const uint32_t BASIC_ORE_RATE_PARA = 9300;
public:
    typedef std::map<uint64_t, MineOre*> Key2Ore_t;
    typedef std::map<uint64_t, MinePlayer*> Key2Player_t;
public:
    MineDataMgr() {};
    ~MineDataMgr() {};

    bool Init(MINESVRCFG* pstConfig);
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

    //增加到Time表
    void AddToTimeList(MineOre* pstOre);
	void AddToTimeList(MinePlayer* pstPlayer);

    //增加到Dirty表
    void AddToDirtyList(MineOre* pstOre);
	void AddToDirtyList(MinePlayer* pstPlayer);

    //移动到TimeList的首个
    void MoveToTimeListFirst(MineOre* pstOre);
	void MoveToTimeListFirst(MinePlayer* pstPlayer);

    //从Time表删除
    void DelFromTimeList(MineOre* pstOre);
	void DelFromTimeList(MinePlayer* pstPlayer);

    //从Dirty表删除
    void DelFromDirtyList(MineOre* pstOre);
	void DelFromDirtyList(MinePlayer* pstPlayer);

	//增加到空列表中
	void AddToEmptyList(MineOre* pstOre);
	
	//从空列表中删除
	void DelFromEmptyList(MineOre* pstOre);

protected:
    void _WriteDirtyToDB();

    //保存到数据库
    void _SaveOre(MineOre* pstOre);

    //从数据库中删除
    void _DelOre(MineOre* pstOre);

    void _SavePlayer(MinePlayer* pstPlayer);
    //获取一个新节点
    MineOre* _GetNewOre();     
    MinePlayer* _GetNewPlayer();
    void _AddOreToMap(MineOre* pstOre);
    void _DelOreFromMap(MineOre* pstOre);
    void _AddPlayerToMap(MinePlayer* pstPlayer);
    void _DelPlayerToMap(MinePlayer* pstPlayer);


 public:
     //获取矿信息
     MineOre* GetMineOre(uint64_t ullUid);

	 MinePlayer* GetMinePlayer(uint64_t ullUin);

	 //获取玩家 不创建
	 MinePlayer* GetMinePlayerNoCreate(uint64_t ullUin);

     //生成矿的Uid
     uint64_t CreateOreUid();

    //生成一个矿资源
     MineOre* CreateOre(uint8_t bOreType, uint32_t dwOreId, uint32_t dwCreateNum);

     //随机探索矿
     int RandomOre(uint64_t ullUin, uint32_t dwTeamLi ,OUT uint8_t& rbNum, OUT DT_MINE_ORE_INFO* pstOreInfo);

	//增加矿的数量
	void AddOreNum(MineOre* pstMineOre);

	//减少矿的数量
	void SubOreNum(MineOre* pstMineOre);

	//减少空矿的数量
	void SubEmptyNum(MineOre* pstMineOre);

	//增加空旷的数量
	void AddEmptyNum(MineOre* pstMineOre);

	//从数据库拉去矿信息
	void GetOreFromDB();

	//获取一个空旷
	MineOre* GetEmptyOre(uint8_t bOreType, uint8_t bCurIndex);

	int GetNormalOreEmptyNum() { return m_iNormalOreEmptyNum; }
	int GetNormalOreTotalNum() { return m_iNormalOreTotalNum; }
	int GetSupperOreEmptyNum() { return m_iSupperOreEmptyNum; }
	int GetSupperOreTotalNum() { return m_iSupperOreTotalNum; }
	int GetInvestigateOreNum() { return m_iInvestigateNum; }

	//获得低级矿空置率
	float GetNormalOreEmptyRate() { return m_iNormalOreTotalNum ? 1.0 * m_iNormalOreEmptyNum / m_iNormalOreTotalNum : 0; }
	float GetSupperOreEmptyRate() { return m_iSupperOreTotalNum ? 1.0 * m_iSupperOreEmptyNum / m_iSupperOreTotalNum : 0; }
	bool IsMineUidSetEmpty() { return m_oMineUidSet.GetUidNum() <= 0; }
	bool IsGetOreSwitchOpen() { return m_bGetOreSwitch; }
	void SetGetOreSwitch(bool OK) { m_bGetOreSwitch = OK; }
private:
    int                                         m_iUptHour;
    uint64_t                                    m_ullLastUptTimeMs;           //上次更新时间
    uint32_t                                    m_dwSeq;
    uint64_t                                    m_ullLastCreateUinTime;
	MineUidSet									m_oMineUidSet;
	bool										m_bGetOreSwitch;			//从数据库获取矿信息的开关
private:
    MINESVRCFG*                                 m_pstConfig;

    TLISTNODE                                   m_stOreDirtyListHead;          //脏数据链表头,用于回写数据库
    TLISTNODE                                   m_stOreTimeListHead;           //时间链表头,用于LRU算法
    uint32_t                                    m_dwOreDirtyListSize;          //脏数据链表长度
    uint32_t                                    m_dwOreTimeListSize;           //时间链表长度长度
    uint32_t                                    m_dwOreWriteTimeVal;           //回写的时间间隔
    uint64_t                                    m_ullOreLastWriteTimestamp;    //上次回写脏数据的时间

	TLISTNODE                                   m_stPlayerDirtyListHead;          //脏数据链表头,用于回写数据库
	TLISTNODE                                   m_stPlayerTimeListHead;           //时间链表头,用于LRU算法
	uint32_t                                    m_dwPlayerDirtyListSize;          //脏数据链表长度
	uint32_t                                    m_dwPlayerTimeListSize;           //时间链表长度长度
	uint32_t                                    m_dwPlayerWriteTimeVal;           //回写的时间间隔
	uint64_t                                    m_ullPlayerLastWriteTimestamp;    //上次回写脏数据的时间

	

    Key2Ore_t                                   m_oOrerMap;
    Key2Ore_t::iterator                         m_oOreMapIter;
    Key2Player_t                                m_oPlayerMap;
    Key2Player_t::iterator                      m_oPlayerMapIter;


    map<uint8_t, vector<size_t> >               m_oOreTypeToAddrVectMap;
    map<uint8_t, vector<size_t> >::iterator     m_oOreTypeToAddrVectMapIter;

    DynMempool<AsyncGetOreDataAction>           m_oAsyncActionPool;
    DynMempool<MineOre>                         m_oMineOrePool;
    DynMempool<MinePlayer>                      m_oMinePlayerPool;
    
    uint32_t        m_dwOreRateOfNormal;
    uint32_t        m_dwOreRateOfSupper;
    uint32_t        m_dwOreRateOfInvestigate;


	int											m_iNormalOreEmptyNum;		//无人低级矿 数量
	int											m_iNormalOreTotalNum;		//低级矿总数量
	int											m_iSupperOreEmptyNum;		//无人高级矿	数量
	int											m_iSupperOreTotalNum;		//高级矿总数量
	int											m_iInvestigateNum;			//调查矿	数量
	TLISTNODE									m_stNormalOreEmptyListHead;			//普通无人旷列表
	TLISTNODE									m_stSupperOreEmptyListHead;			//高级无人旷列表
};


