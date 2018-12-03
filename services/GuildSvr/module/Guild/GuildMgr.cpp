#include "GuildMgr.h"
#include "common_proto.h"
#include "LogMacros.h"
#include "hash_func.h"
#include "../Player/PlayerMgr.h"
#include "GuildRank.h"
#include "../../gamedata/GameDataMgr.h"

using namespace PKGMETA;


bool GuildMgr::Init(GUILDSVRCFG * pstConfig)
{
    if (pstConfig == NULL)
    {
        LOGERR_r("pstConfig is NULL");
        return false;
    }

    //初始化update相关数据
    m_iRefreshTimeVal = pstConfig->m_iRefreshTimeVal;
    m_ullLastUptTimestamp = CGameTime::Instance().GetCurrTimeMs();

    //初始化时间链表
    TLIST_INIT(&m_stTimeListHead);

    //初始化脏数据相关
    m_iDirtyNodeNum = 0;
    m_iDirtyNodeMax = pstConfig->m_iDirtyNodeMax;
    m_iWriteTimeVal = pstConfig->m_iWriteTimeVal * 1000;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrTimeMs();
    TLIST_INIT(&m_stDirtyListHead);

    //初始化mempool
    m_iCurSize = 0;
    m_iMaxSize = pstConfig->m_iGuildMaxNum;
    if (m_GuildPool.CreatePool(m_iMaxSize) < 0)
    {
        LOGERR_r("Create Guild pool[num=%d] failed.", m_iMaxSize);
        return false;
    }
    m_GuildPool.RegisterSlicedIter(&m_oUptIter);

    //初始化工作线程
    m_iWorkThreadNum = pstConfig->m_iWorkThreadNum;
    m_astWorkThreads = new GuildDBThread[m_iWorkThreadNum];
    if( !m_astWorkThreads )
    {
        LOGERR_r("Init DBWorkThread failed.");
        return false;
    }
    key_t iShmKey = pstConfig->m_iThreadQBaseShmKey;
    bool bRet = 0;
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        bRet = m_astWorkThreads[i].InitThread(i, pstConfig->m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)pstConfig, &iShmKey);
        if( !bRet )
        {
            LOGERR_r("Init thread(%d) failed", i);
            return false;
        }
    }

    //初始化生成公会Id相关参数
    m_wSvrId = pstConfig->m_iSvrID;
    m_ullLastGenerateTime = CGameTime::Instance().GetCurrSecond();
    m_dwSeq = 0;

    //公会Boss刷新
    m_bBossSingleUptFlag = true;
    m_bBossUptFlag = true;
    SetGuildBossSingleUptTime();
    SetGuildBossUptTime();

	RESBASIC *poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
	if (!poResBasic)
	{
		LOGERR_r("Load gamedata error, poResBasic id<%u> is NULL ", COMMON_UPDATE_TIME);
		return false;
	}
	m_bFundUptHour = (uint8_t) poResBasic->m_para[0];
	m_ullFundUptLastUptTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_bFundUptHour);
	if(CGameTime::Instance().GetCurrHour() < m_bFundUptHour)
	{
		m_ullFundUptLastUptTime -= SECONDS_OF_DAY;
	}
	m_bFundUptFlag = false;

    return true;
}


void GuildMgr::Fini()
{
    //将公会信息回写数据库
    Guild* poGuild = NULL;
    m_oUptIter.Begin();
    for (int i=0; !m_oUptIter.IsEnd(); m_oUptIter.Next(), i++)
    {
        if (i % GUILD_UPT_NUM_PER_FRAME == 0)
        {
            MsSleep(10);
        }

        GuildNode* pstGuildNode = m_oUptIter.CurrItem();
        if (NULL == pstGuildNode)
        {
            LOGERR_r("In GuildMgr fini, pstGuildNode is null");
            break;
        }

        poGuild = &pstGuildNode->m_oGuild;
        _SaveGuild(poGuild);
    }

    //等待线程结束
    for( int i = 0; i < m_iWorkThreadNum; i++ )
    {
        m_astWorkThreads[i].FiniThread();
    }

    LOGRUN_r("GuildMgr Fini success");

    return;
}


Guild* GuildMgr::NewGuild(DT_GUILD_WHOLE_DATA& rstGuildInfo)
{
    Guild* poGuild = NULL;
    GuildNode* pstGuildNode = NULL;

    //mempool中有空余时
    if (m_iCurSize < m_iMaxSize)
    {
        pstGuildNode = m_GuildPool.NewData();
        if (NULL == pstGuildNode)
        {
            LOGERR_r("pstGuildNode is NULL, get GuildNode from GuildPool failed");
            return NULL;
        }

        //初始化Guild
        poGuild = &pstGuildNode->m_oGuild;
        poGuild->InitFromDB(rstGuildInfo);

        //加入时间链表头
        TLIST_INSERT_NEXT(&m_stTimeListHead, &(pstGuildNode->m_stTimeListNode));
        m_iCurSize++;
    }
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    else
    {
        //取时间链表尾节点
        int iCount = 0;
        TLISTNODE* pstTempNode = &m_stTimeListHead;
        TLISTNODE* pstSwapNode = NULL;
        do
        {
            iCount++;
            pstSwapNode = TLIST_PREV(pstTempNode);
            pstGuildNode = container_of(pstSwapNode, GuildNode, m_stTimeListNode);
            pstTempNode = pstSwapNode;
            poGuild = &pstGuildNode->m_oGuild;
            if (poGuild->GetCanSwap() || iCount >= m_iMaxSize)
            {
                break;
            }
        }while (true);

        //没有可以置换的节点
        if (iCount >= m_iMaxSize)
        {
            return NULL;
        }

        //被置换的GuildNode需要回写数据库
        _SaveGuild(poGuild);

        //被置换的GuildNode需要从Map中删除
        this->DelGuildFromMap(poGuild);

        //初始化新的Guild
        poGuild->InitFromDB(rstGuildInfo);

        //将此节点从时间链表尾移到链表头
        TLIST_DEL(pstSwapNode);
        TLIST_INSERT_NEXT(&m_stTimeListHead, pstSwapNode);
    }

    //加入Map
    this->AddGuildToMap(poGuild);

    return poGuild;
}


void GuildMgr::Update()
{
    this->_HandleThreadMsg();

    this->_UpdateGuild();

    this->_WriteDirtyToDB();
}


//处理DB线程消息
void GuildMgr::_HandleThreadMsg()
{
    for (int i=0; i < m_iWorkThreadNum; i++)
    {
        GuildDBThread& rstWorkThread = m_astWorkThreads[i];
        int iRecvBytes = rstWorkThread.Recv(CThreadFrame::MAIN_THREAD);
        if (iRecvBytes < 0)
        {
            LOGERR_r("GuildMgr Main Thread Recv Failed, iRecvBytes=%d", iRecvBytes);
            continue;
        }
        else if (0 == iRecvBytes)
        {
            continue;
        }

        MyTdrBuf* pstRecvBuf = rstWorkThread.GetRecvBuf(CThreadFrame::MAIN_THREAD);
        TdrError::ErrorType iRet = m_stDBRspPkg.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR_r("GuildMgr Main Thread unpack pkg failed! errno : %d", iRet);
            continue;
        }

        switch(m_stDBRspPkg.m_bType)
        {
            //只处理Create和Get消息
            case GUILD_DB_CREATE:
                this->_HandleCreateMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            case GUILD_DB_GET:
                this->_HandleGetMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            case GUILD_DB_GET_BY_NAME:
                this->_HandleGetByNameMsg(m_stDBRspPkg.m_stData, m_stDBRspPkg.m_nErrNo, m_stDBRspPkg.m_ullActionId);
                break;
            default:
                LOGERR_r("GuildMgr handle thread msg error, msg type(%d) error", m_stDBRspPkg.m_bType);
                break;
        }
    }
}


void GuildMgr::_HandleCreateMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId)
{
    assert(ullActionId != 0);

    const char* szName = rstGuildInfo.m_stBaseInfo.m_szName;
    Guild* poGuild = NULL;

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Create Guild(%s) failed", szName);
    }
    else
    {
        poGuild = NewGuild(rstGuildInfo);
    }

    GuildTransFrame::Instance().AsyncActionDone(ullActionId, poGuild, sizeof(DT_GUILD_WHOLE_DATA));
}


void GuildMgr::_HandleGetMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId)
{
    assert(ullActionId != 0);

    uint64_t ullGuildId = rstGuildInfo.m_stBaseInfo.m_ullGuildId;
    Guild* poGuild = this->GetGuild(ullGuildId);

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Get Guild(%lu) Info failed, iRet=%d", ullGuildId, iErrNo);
    }
    else if (poGuild == NULL)
    {
        poGuild = NewGuild(rstGuildInfo);
    }

    GuildTransFrame::Instance().AsyncActionDone(ullActionId, poGuild, sizeof(DT_GUILD_WHOLE_DATA));
}

void GuildMgr::_HandleGetByNameMsg(DT_GUILD_WHOLE_DATA& rstGuildInfo, int iErrNo, uint64_t ullActionId)
{
    assert(ullActionId != 0);

    Guild* poGuild = this->GetGuild(rstGuildInfo.m_stBaseInfo.m_szName);

    if (iErrNo != ERR_NONE)
    {
        LOGERR_r("Get Guild(%s) Info failed, iRet=%d", rstGuildInfo.m_stBaseInfo.m_szName, iErrNo);
    }
    else if (poGuild == NULL)
    {
        poGuild = NewGuild(rstGuildInfo);
    }

    GuildTransFrame::Instance().AsyncActionDone(ullActionId, poGuild, sizeof(DT_GUILD_WHOLE_DATA));
}


// TODO: 刷新时间使用配置
void GuildMgr::_UpdateGuild()
{
    uint8_t bHour = CGameTime::Instance().GetCurrHour();
    uint8_t bWeekday = CGameTime::Instance().GetCurDayOfWeek();
    uint8_t bDayOfMonth = CGameTime::Instance().GetCurrDay();
    //***********boss刷新时间计算
    //全部刷新
    if (bDayOfMonth < 4 && bHour == m_bBossUptHour && m_bBossUptFlag)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (bWeekday == m_szBossUptWeekday[i])
            {
                m_bBossUptFlag = false;
                //m_bRefreshCompetitorFlag = true;
                m_bBossSingleUptFlag = false;
                m_ullBossUptLastUptTime = CGameTime::Instance().GetCurrSecond();
                m_ullBossSingleUptLastUptTime = CGameTime::Instance().GetCurrSecond();
                break;
            }
        }
    }
    else if (bDayOfMonth >= 4)
    {
        m_bBossUptFlag = true;
    }

    //单独刷新最新Boss
    for (int i = 0; i != 3; ++i)
    {
        if (bWeekday == m_szBossUptWeekday[i] && bHour == m_bBossUptHour && m_bBossSingleUptFlag)
        {
            m_bBossSingleUptFlag = false;
            m_bRefreshCompetitorFlag = true;
            m_ullBossSingleUptLastUptTime = CGameTime::Instance().GetCurrSecond();
            break;
        }
        else if (bWeekday != m_szBossUptWeekday[i] && bHour != m_bBossUptHour)
        {
            m_bBossSingleUptFlag = true;
            break;
        }
    }
    //**********boss刷新时间计算 end

	//**每日公会资金上限刷新时间计算
	if (bHour == m_bFundUptHour && m_bFundUptFlag)
	{
		m_bFundUptFlag = false;
		m_ullFundUptLastUptTime = CGameTime::Instance().GetCurrSecond();
	}
	else if( bHour != m_bFundUptHour )
	{
		m_bFundUptFlag = true;
	}

	//**每日公会资金上限刷新时间计算 end

    if (m_GuildPool.GetUsedNum() == 0)
    {
        return;
    }

    uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();

    if ((ullCurTime- m_ullLastUptTimestamp) < (uint64_t)m_iRefreshTimeVal)
    {
        return;
    }
    m_ullLastUptTimestamp = ullCurTime;


    int iCheckNum = GUILD_UPT_NUM_PER_FRAME;

    if (m_oUptIter.IsEnd())
    {
        m_oUptIter.Begin();
    }

    Guild* poGuild = NULL;
    GuildNode* pstGuildNode = NULL;
    for (int i=0; i<iCheckNum&&!m_oUptIter.IsEnd(); i++, m_oUptIter.Next())
    {
        pstGuildNode = m_oUptIter.CurrItem();
        if (pstGuildNode == NULL)
        {
            LOGERR("pstGuildNode is NULL");
            continue;
        }
        poGuild = &pstGuildNode->m_oGuild;
        poGuild->Update(m_ullBossUptLastUptTime, m_ullBossSingleUptLastUptTime, m_ullFundUptLastUptTime);
    }
};

void GuildMgr::AddToDirtyList(Guild* poGuild)
{
    GuildNode* pstGuildNode = container_of(poGuild, GuildNode, m_oGuild);
    TLISTNODE* pstNode = &pstGuildNode->m_stDirtyListNode;

    //已经加入DirtyList,不能重复加入
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }

    //加入DirtyList表头
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstNode);
    m_iDirtyNodeNum++;

    LOGRUN_r("Add Guild to DirtyList, GuildId=(%lu), GuildName=(%s), Time=(%lu) ",
              poGuild->GetGuildId(), poGuild->GetGuildName(), CGameTime::Instance().GetCurrSecond());
}

void GuildMgr::_SaveGuild(Guild* poGuild)
{
    m_stDBReqPkg.m_bType = GUILD_DB_UPDATE;
    if (poGuild->PackGuildWholeData(m_stDBReqPkg.m_stData, 0) == false)
    {
        LOGERR_r("Guild(%lu) pack guild whole data failed.", poGuild->GetGuildId());
        return;
    }
    m_astWorkThreads[poGuild->GetGuildId() % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);
}

void GuildMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    GuildNode* pstGuildNode = NULL;
    Guild* poGuild = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();

    if (m_iDirtyNodeNum >= m_iDirtyNodeMax ||
        ullCurTime - m_ullLastWriteTimestamp >= (uint64_t)m_iWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstGuildNode = TLIST_ENTRY(pstPos, GuildNode, m_stDirtyListNode);
            poGuild = &pstGuildNode->m_oGuild;
            _SaveGuild(poGuild);

            LOGRUN_r("Write DirtyList to DB, GuildId=(%lu), GuildName=(%s), Time=(%lu) ",
                      poGuild->GetGuildId(), poGuild->GetGuildName(), ullCurTime/1000);

            TLIST_INIT(pstPos);
        }

        TLIST_INIT(pstHead);
        m_iDirtyNodeNum = 0;
    }

}


int GuildMgr::CreateGuild(const char* pszName, TActionToken ullTokenId)
{
    //检查map是否有重名公会
    m_NameToGuildMapIter = m_NameToGuildMap.find(pszName);
    if (m_NameToGuildMapIter != m_NameToGuildMap.end())
    {
        return ERR_EXIST_GUILDNAME;
    }

    //生成GuildId
    uint64_t ullGuildId = 0;
    int iRet = GenerateGuildId(ullGuildId);
    if (iRet != ERR_NONE)
    {
        return iRet;
    }

    //内存中没有找到则向数据库处理线程发消息
    m_stDBReqPkg.m_bType = GUILD_DB_CREATE;
    m_stDBReqPkg.m_ullActionId = ullTokenId;

    DT_GUILD_WHOLE_DATA& rstGuildInfo = m_stDBReqPkg.m_stData;
    rstGuildInfo.m_stBaseInfo.m_ullGuildId = ullGuildId;
    StrCpy(rstGuildInfo.m_stBaseInfo.m_szName, pszName, MAX_NAME_LENGTH);

    m_astWorkThreads[ullGuildId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return ERR_NONE;
}


Guild* GuildMgr::GetGuild(uint64_t ullGuildId, TActionToken ullTokenId)
{
    assert(ullGuildId != 0);

    //先在内存中找
    m_IdToGuildMapIter = m_IdToGuildMap.find(ullGuildId);
    if (m_IdToGuildMapIter != m_IdToGuildMap.end())
    {
        //此Guild被访问,移至时间链表头
        Move2TimeListFirst(m_IdToGuildMapIter->second);
        return m_IdToGuildMapIter->second;
    }

    if (ullTokenId==0)
    {
        return NULL;
    }

    //内存中没有找到则向数据库处理线程发消息
    m_stDBReqPkg.m_bType = GUILD_DB_GET;
    m_stDBReqPkg.m_ullActionId = ullTokenId;
    m_stDBReqPkg.m_stData.m_stBaseInfo.m_ullGuildId = ullGuildId;
    m_astWorkThreads[ullGuildId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    return NULL;
}


Guild* GuildMgr::GetGuild(const char* pszName, TActionToken ullTokenId)
{
    m_NameToGuildMapIter = m_NameToGuildMap.find(pszName);
    if (m_NameToGuildMapIter != m_NameToGuildMap.end())
    {
        return m_NameToGuildMapIter->second;
    }

    if (ullTokenId==0)
    {
        return NULL;
    }

    //内存中没有找到则向数据库处理线程发消息
    m_stDBReqPkg.m_bType = GUILD_DB_GET_BY_NAME;
    m_stDBReqPkg.m_ullActionId = ullTokenId;
    StrCpy(m_stDBReqPkg.m_stData.m_stBaseInfo.m_szName, pszName, MAX_NAME_LENGTH);
    m_astWorkThreads[0].SendReqPkg(m_stDBReqPkg);

    return NULL;
}


void GuildMgr:: Move2TimeListFirst(Guild* poGuild)
{
    GuildNode* pstGuildNode = container_of(poGuild, GuildNode, m_oGuild);
    TLISTNODE* pstNode = &pstGuildNode->m_stTimeListNode;

    //将此节点移到时间链表头
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstNode);
}



bool GuildMgr::DeleteGuild(uint64_t ullGuildId)
{
    assert(ullGuildId != 0);

    Guild* poGuild = NULL;

    //从map中删除
    m_IdToGuildMapIter = m_IdToGuildMap.find(ullGuildId);
    if (m_IdToGuildMapIter == m_IdToGuildMap.end())
    {
        return false;
    }

    poGuild = m_IdToGuildMapIter->second;
    this->DelGuildFromMap(poGuild);

    //从么mempool中删除
    if (poGuild != NULL)
    {
        GuildNode* pstGuildNode = (GuildNode*)poGuild;
        TLIST_DEL(&pstGuildNode->m_stTimeListNode);
        int iRet = m_GuildPool.DeleteData(pstGuildNode);
        if (iRet != 0)
        {
            LOGERR("Delete Guilld(%lu) from mempool failed, iRet=%d", ullGuildId, iRet);
            return false;
        }
        m_iCurSize--;
    }

    //从数据库中删除
    m_stDBReqPkg.m_bType = GUILD_DB_DEL;
    m_stDBReqPkg.m_stData.m_stBaseInfo.m_ullGuildId = ullGuildId;
    m_astWorkThreads[ullGuildId % m_iWorkThreadNum].SendReqPkg(m_stDBReqPkg);

    //从军团排行及军团战排行中删除
    GuildRankMgr::Instance().Delete(ullGuildId);
    GFightRankMgr::Instance().Delete(ullGuildId);

    return true;
}


void GuildMgr::AddGuildToMap(Guild* poGuild)
{
    assert(poGuild);

    //加入IdMap
    m_IdToGuildMapIter = m_IdToGuildMap.find(poGuild->GetGuildId());
    if (m_IdToGuildMapIter == m_IdToGuildMap.end())
    {
        m_IdToGuildMap.insert(GuildIdMap_t::value_type(poGuild->GetGuildId(), poGuild));
    }
    else
    {
        LOGERR_r("Add Guild to IdMap failed, GuildId=(%lu)", poGuild->GetGuildId());
    }

    //加入NameMap
    m_NameToGuildMapIter = m_NameToGuildMap.find(poGuild->GetGuildName());
    if (m_NameToGuildMapIter == m_NameToGuildMap.end())
    {
        m_NameToGuildMap.insert(GuildNameMap_t::value_type(poGuild->GetGuildName(), poGuild));
    }
    else
    {
        LOGERR_r("Add Guild to NameMap failed, GuildName=(%s)", poGuild->GetGuildName());
    }
}


void GuildMgr::DelGuildFromMap(Guild* poGuild)
{
    assert(poGuild);

    poGuild->Clear();

    //从IdMap中删除
    m_IdToGuildMapIter = m_IdToGuildMap.find(poGuild->GetGuildId());
    if (m_IdToGuildMapIter != m_IdToGuildMap.end())
    {
        m_IdToGuildMap.erase(m_IdToGuildMapIter);
    }
    else
    {
        LOGERR_r("Del Guild from IdMap failed, GuildId=(%lu) ", poGuild->GetGuildId());
    }

    //从NameMap中删除
    m_NameToGuildMapIter = m_NameToGuildMap.find(poGuild->GetGuildName());
    if (m_NameToGuildMapIter != m_NameToGuildMap.end())
    {
        m_NameToGuildMap.erase(m_NameToGuildMapIter);
    }
    else
    {
        LOGERR_r("Del Guild from NameMap failed, GuildName=(%s)", poGuild->GetGuildName());
    }
}


int GuildMgr::RefreshGuildList(uint8_t& rbGuildCount, DT_GUILD_BRIEF_INFO* pstGuildList)
{
    uint8_t bIter = 0;
    Guild* poGuild = NULL;
    m_IdToGuildMapIter = m_IdToGuildMap.begin();

    while (m_IdToGuildMapIter != m_IdToGuildMap.end() && bIter < MAX_NUM_GUILDLIST)
    {
        poGuild = m_IdToGuildMapIter->second;
        assert(poGuild);
        poGuild->GetBriefInfo(pstGuildList[bIter]);

        m_IdToGuildMapIter++;
        bIter++;
    }

    rbGuildCount = bIter;
    return ERR_NONE;
}

//生成军团ID
int GuildMgr::GenerateGuildId(uint64_t& ullGuildId)
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_ullLastGenerateTime != ullCurTime)
    {
        m_ullLastGenerateTime = ullCurTime;
        m_dwSeq = 0;
    }
    else
    {
        if (++m_dwSeq >= MAX_NUM_CREATE_ONE_SEC)
        {
            return ERR_DEFAULT;
        }
    }

    //ID生成算法
    // 26位(时间戳,单位:秒)  |  4位(当前时间戳内的序号)   |  10位(服务器ID)

    uint64_t ullTimestamp = m_ullLastGenerateTime - BASE_TIMESTAMP;

    ullGuildId = ullTimestamp << 14;
    ullGuildId |= m_dwSeq << 10;
    ullGuildId |= m_wSvrId;

    return ERR_NONE;
}

void GuildMgr::SetGuildBossSingleUptTime()
{
    m_bBossUptHour = 5;
    uint8_t szInitUptWeekDay[3] = { 1,3,5 };
    memcpy(m_szBossUptWeekday, szInitUptWeekDay, 3);
    m_bBossUptFlag = false;
    m_bBossSingleUptFlag = false;
    uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();
    uint64_t szBossUptSec[3];
    for (int i = 0; i != 3; ++i)
    {
        szBossUptSec[i] = CGameTime::Instance().GetSecByWeekDay(m_szBossUptWeekday[i], m_bBossUptHour);
    }
    uint64_t ullTempUptTime = 0;
    //计算正确的刷新时间
    if (ullCurrTime < szBossUptSec[0])
    {
        //如果当前时间落在本周一刷新之前，那么上次刷新时间就是在上周五
        ullTempUptTime = CGameTime::Instance().GetSecByLastWeekDay(m_szBossUptWeekday[2], m_bBossUptHour);
    }
    else if (ullCurrTime < szBossUptSec[1])
    {
        //如果当前时间落在周一和周三刷新之间，那么上次刷新时间就是在本周一
        ullTempUptTime = szBossUptSec[0];
    }
    else if (ullCurrTime < szBossUptSec[2])
    {
        //如果当前时间落在周三和周五之间，那么上次刷新时间就是在本周三
        ullTempUptTime = szBossUptSec[1];
    }
    else
    {
        //如果当前时间在周五之后，那么上次刷新时间就是在本周五
        ullTempUptTime = szBossUptSec[2];
    }

    m_ullBossSingleUptLastUptTime = ullTempUptTime;
}

void GuildMgr::SetGuildBossUptTime()
{
    //uint8_t bCurrDay = CGameTime::Instance().GetCurrDay();
    uint8_t bMonth = CGameTime::Instance().GetCurrMonth() + 1;
    uint16_t wYear = CGameTime::Instance().GetCurrYear() + 1900;
    uint8_t bDay = 1;
    uint8_t bWeek;
    for (; bDay != 4; ++bDay)
    {
        bWeek = CGameTime::Instance().GetDayOfWeekBySomeDay(wYear, bMonth, bDay);
        if (bWeek == 1 || bWeek == 3 || bWeek == 5)
        {
            break;
        }
    }
    struct tm stBossUptTime;
    stBossUptTime.tm_year = wYear - 1900;
    stBossUptTime.tm_mon = bMonth - 1;
    stBossUptTime.tm_mday = bDay;
    stBossUptTime.tm_hour = 5;
    stBossUptTime.tm_min = 0;
    stBossUptTime.tm_sec = 0;
    m_ullBossUptLastUptTime = mktime(&stBossUptTime);
    uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();
    if (ullCurrTime < m_ullBossUptLastUptTime)
    {
        bDay = 1;
        for (; bDay != 4; ++bDay)
        {
            bWeek = CGameTime::Instance().GetDayOfWeekBySomeDay(wYear, bMonth, bDay);
            if (bWeek == 1 || bWeek == 3 || bWeek == 5)
            {
                break;
            }
        }
        stBossUptTime.tm_year = wYear - 1900;
        stBossUptTime.tm_mon = bMonth - 2;
        stBossUptTime.tm_mday = bDay;
        stBossUptTime.tm_hour = 5;
        stBossUptTime.tm_min = 0;
        stBossUptTime.tm_sec = 0;
        m_ullBossUptLastUptTime = mktime(&stBossUptTime);
    }
}

uint16_t GuildMgr::GetRank(uint64_t ullGuildId)
{
    int i = 0;
    for (; i != MAX_LEN_ROLE_GUILD; ++i)
    {
        if (m_astGuildRankList[i].m_ullGuildId == ullGuildId)
        {
            break;
        }
        if (m_astGuildRankList[i].m_ullGuildId == 0)
        {
            return ERR_NOT_FOUND;
        }
    }
    return i;
}

uint16_t GuildMgr::GetRankNum()
{
    int i = 0;
    for (; i != MAX_LEN_ROLE_GUILD; ++i)
    {
        if (m_astGuildRankList[i].m_ullGuildId == 0)
        {
            break;
        }
    }
    return i;
}

DT_GUILD_RANK_INFO * GuildMgr::GetGuildByRank(uint16_t wGuildRank)
{
    if (m_astGuildRankList[wGuildRank].m_ullGuildId == 0)
    {
        LOGERR("Guild Not Found");
        return NULL;
    }
    return &m_astGuildRankList[wGuildRank];
}

