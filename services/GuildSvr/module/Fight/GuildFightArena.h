#pragma once
#include "../Guild/Guild.h"
#include "ss_proto.h"
#include "object.h"
#include <map>

using namespace PKGMETA;
using namespace std;

class FightPlayer;
class GuildFightPoint;

// ����ս  ս��
class GuildFightArena : public IObject
{
private:
    typedef map<uint16_t, GuildFightPoint*> MapId2Point_t;
    typedef map<uint64_t, FightPlayer*> MapId2Player_t;

public:
    GuildFightArena() {}
    virtual ~GuildFightArena() {}

    bool Init(uint64_t GuildList[], int iGuildCnt, uint8_t bAgainstId);

    void Clear();

    virtual void Update(int iDeltaTime);

    //ս���մ���ʱ����һ�ο�ս׼��ʱ�䣬׼��ʱ����󣬵��ô˺����л�����ʽ��ս״̬
    void Start();

    //״̬ͬ�����㲥������ս���ڵ��������
    void Broadcast();

    //����ս������rstArenaInfoΪս����ȫ����Ϣ
    int Join(SS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstSsPkgReq, DT_GUILD_FIGHT_ARENA_INFO& rstArenaInfo, uint64_t& ullTimeStamp);

    //�˳�ս������
    int Quit(uint64_t ullPlayerId);

    //ս�����ƶ����󣬷���ֵС��0Ϊ�����룬����ֵ����0Ϊ�ƶ���Ҫ��ʱ��
    int Move(uint64_t ullPlayerId, uint16_t wDstPoint);

    //ͨ��Id��Player
    FightPlayer* GetPlayer(uint64_t ullPlayerId);

    //ͨ��Id�Ҿݵ�
    GuildFightPoint* GetPoint(uint16_t wPointId);

    //��û���
    void GainScore(uint8_t bCamp, uint16_t wPointId, uint32_t dwScore);

	// ���ս��״̬ͬ����Ϣ
	void AddStateSync(DT_GUILD_FIGHT_ARENA_PLAYER_INFO& rstArenaPlayerInfo);

    //���ľ���ͳ����Ϣ
    void ChgStatisInfo(uint8_t bCamp, int iType, int iValue);

    //����
    uint64_t Settle();

	//������˻���������������
	void SettleRank();

private:
    bool _InitMap(); //��ʼ����ͼ
    bool _InitPlayer(); //��ʼ����ͼ
    bool _InitGuild(uint64_t GuildList[], int iGuildCnt); //��ʼ������˫������
    bool _InitBasePara(uint8_t bAgainstId);//��ʼ����������

	void _SettleRank(uint8_t bCount, FightPlayer* PlayerList[]);

public:
    //ս��ID,��GuildFightMgr����,ȫ��Ψһ,���ڱ�ʾս��
    uint8_t m_bAgainstId;

    //��ǰս��״̬,��Ϊ׼��״̬,��ս״̬
    uint8_t m_iState;

    //��սʱ��
    uint64_t m_ullStartTimeMs;

    //��ʤ��Ӫ�͹���id
    uint8_t m_bWinCamp;
    uint64_t m_ullWinGuild;

    //��ʤ�������
    uint32_t m_dwWinScore;

	//���ּӿ���Ҫռ��ľݵ���
	uint16_t m_wGainScorePoint1;
	uint16_t m_wGainScorePoint2;

    //ռ�����оݵ����ּӱ��ı���
    uint16_t m_wGainScoreRate1;
	uint16_t m_wGainScoreRate2;

    // �ݵ���Ϣ
	MapId2Point_t m_mapId2Point;
    MapId2Point_t::iterator m_mapId2PointIter;

    // ս�������Ϣ
	MapId2Player_t m_mapId2Player;
    MapId2Player_t::iterator m_mapId2PlayerIter;

    //���������Ϣ
    DT_GUILD_FIGHT_ARENA_GUILD_INFO m_GuildInfoList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //�������ͳ����Ϣ
    DT_GUILD_FIGHT_STATIS_INFO m_GuildStatisList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //ս������б����ھ���ս�㲥
    DT_GUILD_FIGHT_ONE_CAMP_PLAYER_LIST m_GuildPlayerList[MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD];

    //��ǰʱ��,��λ����
    uint64_t m_ullTimeMs;

    //����ս����ȴʱ��
    uint64_t m_ullJoinCoolTimeMs;

    //�¼��빫����ܲμӹ���ս������Ҫ������ʱ��
    uint64_t m_ullLeastPassTimeMs;

    SSPKG m_stSsPkg;

    //�㲥��Ϣ
    DT_GUILD_FIGHT_SYN_MSG m_stSynMsg;
};