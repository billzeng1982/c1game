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
    void SendSynNtf();																// ֪ͨZoneSvr��������¼���б�
    int UploadReplay(DT_REPLAY_RECORD_FILE_HEADER* pReplayFileHead, char* pszURL);	// ����ͻ��˵��ϴ�¼������

 private:
    //���¼���ļ��Ƿ����
    bool CheckReplayExist(uint32_t dwRaceNumber);

    //ɾ��¼���ļ�
    void RemoveReplay(uint32_t dwRaceNumber);

    void _UpdateToPushReplay();

    void _UpdateToCheckedList();

    void _CheckReplay(DT_REPLAY_INFO * pstReplayInfo);

    void _WriteToFile();

    bool _InitFile();

    bool _InitFromFile();

    bool _CreatFile();

private:
    // URL�ĸ�����
    char m_szURLRoot[URLROOT_MAXLEN];

    // ���¼���ļ��ĸ�Ŀ¼
    char m_szRootDir[ROOTDIR_MAXLEN];

    // Ԥ�����ϴ�����ʱ�䣬����ǰʱ��-��ʼ�ϴ���ʱ��>Ԥ�����ϴ�����ʱ��,��¼���ļ������ڣ�����Ϊ�ϴ�ʧ��
    uint32_t m_dwCheckInterval;

    // ��������¼���б��ʱ����
    uint32_t m_dwUpdateInterval;

    // �ϴθ�������¼���б��ʱ��
    uint64_t m_ullLastUpdateTime;

    //������Ƿ��ϴ��ɹ���Replay
    ListReplayInfo_t m_ToCheckedList;

    //��ǰʱ�θ�����λ��¼��
    DT_REPLAY_INFO * m_ReplayList[MAX_NUM_GRADE];

    //��̬�ڴ��
    DynMempool<DT_REPLAY_INFO> m_oReplayPool;

    //����¼���б�
    DT_WHOLE_REPLAY_LIST   m_stWholeReplayList;

    FILE* m_fp;
    char m_szFileName[MAX_LEN_FILEPATH];

    SSPKG m_stSsPkg;
};

