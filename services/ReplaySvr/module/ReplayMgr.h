#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "object.h"
#include <list>
#include "DynMempool.h"


using namespace std;
using namespace PKGMETA;


class ReplayMgr : public TSingleton<ReplayMgr>
{
private:
    typedef list<DT_REPLAY_INFO*> ListReplayInfo_t;

private:
    static const int MAX_DEAL_NUM_PER_FRAME = 20;
    static const int URLROOT_MAXLEN = 128;
    static const int ROOTDIR_MAXLEN = 128;
    static const int REPLAY_POOL_INIT_NUM = 100;
    static const int REPLAY_POOL_DELTA_NUM = 50;

public:
	ReplayMgr();
	virtual ~ReplayMgr();
    bool Init(char* pszURLRoot, char* pszRootDir, uint32_t dwCheckInterval, uint32_t dwUpdateInterval, const char* pszFileName);
    void Update();
    void AppFini();
    void SendSynNtf();																// 通知ZoneSvr更新推送录像列表
    int UploadReplay(DT_REPLAY_RECORD_FILE_HEADER* pReplayFileHead, char* pszURL);	// 处理客户端的上传录像请求

 private:
    //检查录像文件是否存在
    bool CheckReplayExist(uint32_t dwRaceNumber);

    //删除录像文件
    void RemoveReplay(uint32_t dwRaceNumber);

    void _UpdateToPushReplay();

    void _UpdateToCheckedList();

    void _CheckReplay(DT_REPLAY_INFO * pstReplayInfo);

    void _WriteToFile();

    bool _InitFile();

    bool _InitFromFile();

    bool _CreatFile();

private:
    // URL的根部分
    char m_szURLRoot[URLROOT_MAXLEN];

    // 存放录像文件的根目录
    char m_szRootDir[ROOTDIR_MAXLEN];

    // 预估的上传所需时间，若当前时间-开始上传的时间>预估的上传所需时间,且录像文件不存在，则认为上传失败
    uint32_t m_dwCheckInterval;

    // 更新推送录像列表的时间间隔
    uint32_t m_dwUpdateInterval;

    // 上次更新推送录像列表的时间
    uint64_t m_ullLastUpdateTime;

    //待检查是否上传成功的Replay
    ListReplayInfo_t m_ToCheckedList;

    //当前时段各个段位的录像
    DT_REPLAY_INFO * m_ReplayList[MAX_NUM_GRADE];

    //动态内存池
    DynMempool<DT_REPLAY_INFO> m_oReplayPool;

    //所有录像列表
    DT_WHOLE_REPLAY_LIST   m_stWholeReplayList;

    FILE* m_fp;
    char m_szFileName[MAX_LEN_FILEPATH];

    SSPKG m_stSsPkg;
};

