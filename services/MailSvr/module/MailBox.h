#pragma once
#include <map>
#include "ss_proto.h"
#include "define.h"

using namespace PKGMETA;
using namespace std;

class MailBoxInfo
{
private:
    typedef map<uint32_t, DT_MAIL_DATA*> MapId2Mail_t;

public:
    MailBoxInfo();
    ~MailBoxInfo();
    void Clear();

    //将MailBox的数据打包到rstData中
    int PackMailBoxInfo(OUT DT_MAIL_BOX_DATA& rstData);

    //从数据库初始化MailBox
    int InitFromDB(IN DT_MAIL_BOX_DATA& rstData);

    //改变邮件状态
    int ChgMailState(uint8_t bMailType, uint32_t dwId, uint8_t bNewState);

    //添加PRI邮件
    int AddPriMail(DT_MAIL_DATA& rstData);

    //通过数据档ID添加PRI邮件
    int AddPriMail(uint32_t dwMailResId);

    //更新公共邮件的数据
    int UpdatePubMail(bool bSendFlag);

    //将邮件数据同步给客户端
    int SendMailBoxInfoToClient();

    uint64_t GetPlayerId() { return m_stBaseInfo.m_ullUin; }

private:
    int _ChgPriMailState(uint32_t dwId, uint8_t bNewState);
    int _CheckPriMailState(DT_MAIL_DATA* pstPriMailData, uint8_t bNewState);
    int _DelPriMail(uint32_t dwId);

    int _ChgPubMailState(uint32_t dwId, uint8_t bNewState);
    int _CheckPubMailState(DT_MAIL_PUB_PLAYER_DATA* pstPubMailData, uint8_t bNewState);
    int _DelPubMail(uint32_t dwId);

    void _GenMailData(DT_MAIL_PUB_SER_DATA& rstPubMailData, DT_MAIL_DATA& rstMailData, uint8_t bState);

    int _SendOneMail(DT_MAIL_DATA& rstData);

public:
    //邮箱基本信息
    DT_MAIL_BASE_INFO m_stBaseInfo;

private:
    //公共邮件，数组保存，只存状态
    uint16_t m_wPubMailCount;
    DT_MAIL_PUB_PLAYER_DATA m_astPubMailData[MAX_MAIL_BOX_PUBLIC_NUM];

    //私有邮件，只存指针，节省内存
    uint16_t m_wPriMailCount;
    MapId2Mail_t m_oPriMailMap;
    MapId2Mail_t::iterator m_oMailMapIter;

};

