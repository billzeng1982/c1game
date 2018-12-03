#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "MailSvrCfgDesc.h"
#include "PriorityQueue.h"
#include "GameDataMgr.h"
#include <time.h>

using namespace PKGMETA;
using namespace std;


class ComparePubMailStartTime
{
public:
    bool operator() (const DT_MAIL_PUB_SER_DATA stLeft, const DT_MAIL_PUB_SER_DATA stRight)
    {
        return stLeft.m_ullStartTimeSec < stRight.m_ullStartTimeSec; /*> 升序 小堆*/
    }
};

#if 0
class ComparePubMailEndTime
{
public:
    bool operator() (const DT_MAIL_PUB_SER_DATA stLeft, const DT_MAIL_PUB_SER_DATA stRight)
    {
        return stLeft.m_ullEndTimeSec > stRight.m_ullEndTimeSec; /*> 升序 小堆*/
    }
};
#endif

class MailPubServData : public TSingleton<MailPubServData>
{
public:
    MailPubServData();
    ~MailPubServData();

    bool Init();

    void Update();

    void Fini();

    //通过邮件ID获取邮件信息
    DT_MAIL_PUB_SER_DATA* GetPubSerDataById(uint32_t dwMailId);

    //通过邮件Pos获取邮件信息
    DT_MAIL_PUB_SER_DATA* GetPubSerDataByPos(int iPos);

    //通过邮件ID获取邮件位置
    int GetPosById(uint32_t dwMailId);

    //外部添加公共邮件
    void AddPubMailToWaitQueue(DT_MAIL_PUB_SER_DATA& rstPubMailData);

    int GetPubMailCount() { return m_iPubMailsCount; }

    uint32_t GetPubMailSeq() { return m_iPubMailsCount == 0 ? 0 : m_astPubMails[m_iPubMailsCount-1].m_dwId; }

private:
    bool _InitBase();
    bool _InitRes();
    bool _ConvResToData(RESPUBMAIL* pResMail, DT_MAIL_PUB_SER_DATA* pstPubMailData);

    void _AddPubMail(DT_MAIL_PUB_SER_DATA* pstData);

    void _SyncToZoneSvr();

private:
    //当前公共邮件数量
    int m_iPubMailsCount;

    // 当前公共邮件,按ID升序排列
    DT_MAIL_PUB_SER_DATA m_astPubMails[MAX_MAIL_BOX_PUBLIC_NUM];

    // 当前还未生效的邮件,按生效时间排序
    PriorityQueue<DT_MAIL_PUB_SER_DATA, ComparePubMailStartTime> m_stPubMailWaitQueue;
};



