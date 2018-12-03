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

    //��MailBox�����ݴ����rstData��
    int PackMailBoxInfo(OUT DT_MAIL_BOX_DATA& rstData);

    //�����ݿ��ʼ��MailBox
    int InitFromDB(IN DT_MAIL_BOX_DATA& rstData);

    //�ı��ʼ�״̬
    int ChgMailState(uint8_t bMailType, uint32_t dwId, uint8_t bNewState);

    //���PRI�ʼ�
    int AddPriMail(DT_MAIL_DATA& rstData);

    //ͨ�����ݵ�ID���PRI�ʼ�
    int AddPriMail(uint32_t dwMailResId);

    //���¹����ʼ�������
    int UpdatePubMail(bool bSendFlag);

    //���ʼ�����ͬ�����ͻ���
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
    //���������Ϣ
    DT_MAIL_BASE_INFO m_stBaseInfo;

private:
    //�����ʼ������鱣�棬ֻ��״̬
    uint16_t m_wPubMailCount;
    DT_MAIL_PUB_PLAYER_DATA m_astPubMailData[MAX_MAIL_BOX_PUBLIC_NUM];

    //˽���ʼ���ֻ��ָ�룬��ʡ�ڴ�
    uint16_t m_wPriMailCount;
    MapId2Mail_t m_oPriMailMap;
    MapId2Mail_t::iterator m_oMailMapIter;

};

