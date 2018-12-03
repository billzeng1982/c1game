#pragma once
#include "ss_proto.h"
#include "../../framework/GuildSvrMsgLayer.h"
#include "GuildMember.h"
#include "GuildApply.h"
#include "GuildPvpRoom.h"
#include "GuildReplay.h"
#include "GuildBoss.h"
#include "GuildSociety.h"

using namespace PKGMETA;

class Guild
{
private:
    typedef list<DT_REPLAY_INFO*> ListReplay_t;

    static const int URLROOT_MAXLEN = 128;
    static const int MAX_DEAL_NUM_PER_SEC = 5;
	static const int CHECK_TIMEVAL = 300;

public:
    Guild();
    ~Guild();

    void Update(uint64_t ullBossUptTime, uint64_t ullBossSingleUptTime, uint64_t ullFundUptTime);

    void Clear();

	//新建公会初始化
    bool NewInit();

    //从数据库初始化Guild,所有的Guild初始化都必须走此函数
    bool InitFromDB(IN DT_GUILD_WHOLE_DATA& rstGuildWholeData);

    //将Guild的全部信息打包到rstGuildWholeData中
    bool PackGuildWholeData(OUT DT_GUILD_WHOLE_DATA& rstGuildWholeData, uint16_t wVersion);

	//获取远征信息
	void GetGuildExpeditionInfo(OUT DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo);

    //公会成员的添加，删除，查找
    int AddMember(DT_ONE_GUILD_MEMBER& rstOneMember, DT_ONE_GUILD_MEMBER* pstOneOfficer = NULL);
    int DelMember(uint64_t ullUin);
    DT_ONE_GUILD_MEMBER* FindMember(uint64_t ullUin);

    //公会申请的添加，删除，查找
    int AddApply(DT_ONE_GUILD_MEMBER& rstOneApply);
    int DelApply(uint64_t ullUin);
    DT_ONE_GUILD_MEMBER* FindApply(uint64_t ullUin);
    //设置，查看公会的加入是否需要申请，默认不需要申请
    void SetNeedApply(uint8_t bNeedApply);
    uint8_t GetNeedApply() { return m_stGuildGlobalInfo.m_bNeedApply; }

	//设置，查看公会的加入等级下限
	void SetJoinLevel(uint8_t bLvLimit);
	uint8_t GetJoinLevel() { return m_stGuildGlobalInfo.m_bLvLimit; }

    //刷新成员信息
    int RefreshMemberInfo(SS_PKG_REFRESH_MEMBERINFO_NTF& rstRefreshNtf);

    //刷新房间信息
    int RefreshRoomInfo(SS_PKG_REFRESH_GUILD_PVP_ROOM_NTF& rstRefreshNtf);

    //获取公会房间信息
    void GetGuildRoomInfo(DT_GUILD_ROOM_INFO& rstGuildRoom);

    //设置成员职位
    int SetGuildMemJob(DT_ONE_GUILD_MEMBER* pstOfficer, uint64_t ullMemberId, uint8_t bJob);

	//踢人
	int KickGuildMem(DT_ONE_GUILD_MEMBER* pstOfficer, uint64_t ullMemberId);

    //获取成员职位
    uint8_t GetGuildMemJob(uint64_t ullMemberId);

    //解散公会
    int Dissolve();

    //更新公会公告
    int UptGuildNotice(const char* pszNotice);

	//事件公告（炮夺钱饷，军团管卡等）
	int EventNotice(SS_PKG_GUILD_LEVEL_EVENT_NTF& rstNtf);

    //增加公会资金
    int AddFund(int iFund, DT_GUILD_DONATE_INFO* pstDonateInfo=NULL);

	//增加公会活跃度
    int AddVitality(int iVitality, DT_ONE_GUILD_MEMBER* pstOneMember);

	//发送活跃的奖励
	void SendVitalityReward();

	//增加科技树经验
	int AddSocietyExp(uint8_t bType, int iExp, DT_GUILD_DONATE_INFO* pstDonateInfo=NULL);

    //升级
    int LevelUp();

    //公会战报名
    int FightApply();

    //重置公会战报名资金
    void ResetFightApplyFund();

    //增加公会战积分
    void AddGFightScore(int iScoreDeta);

    //发送公会邮件
    void SendGuildMail(int iMailId);

    //发送邮件给会长/副会长
    //  vVice=true会给副军团长发送
    void SendGuildLeaderMail(int iMailId, bool bVice = false);

    //获取公会简要信息
    int GetBriefInfo(OUT DT_GUILD_BRIEF_INFO& rstBriefInfo);

    //Gm Handle
    void GmUpdateGuildInfo(SS_PKG_GUILD_GM_UPDATE_INFO_REQ& rstReq);

    //发送公会广播
    void SendBroadcastMsg(DT_GUILD_NTF_MSG& rstGuilNtfMsg, uint16_t MsgId);

    //设置公会长
    void SetLeader(DT_ONE_GUILD_MEMBER& rstOneMember);

    //上传录像
    int UploadReplay(DT_REPLAY_RECORD_FILE_HEADER* pstReplayHead, char* pszURL);

    //Boss战斗结算
    void SettleBoss(uint64_t ullUin, SS_PKG_GUILD_BOSS_FIGHT_SETTLE_NTF& rstSsPkgNtf);

    //重置军团Boss
    void ResetBoss(uint64_t ullUin);

    //获取当前BossID
    uint32_t GetCurBossID() { return m_oGuildBoss.GetBossInfo().m_dwCurBossId; }

    //获取当前Boss剩余血量
    uint32_t GetCurBossHp() { return m_oGuildBoss.GetBossLeftHp(GetCurBossID()); }

    GuildBoss& GetGuildBoss() { return m_oGuildBoss; }

    int EnterBossFight(uint64_t ullUin, uint32_t dwFLevelId, OUT SS_PKG_GUILD_BOSS_ENTER_FIGHT_RSP& rstSsPkgRsp);

    uint64_t GetGuildId() { return m_stGuildBaseInfo.m_ullGuildId; }
    uint8_t GetGuildLevel() { return m_stGuildGlobalInfo.m_bGuildLevel; }
    uint8_t GetGuildBossResetNum() { return m_oGuildBoss.GetBossInfo().m_bResetNum; }
    uint32_t GetFightApplyFund() {return m_stGuildGlobalInfo.m_dwFightApplyFund; }
    const char* GetGuildName() { return m_stGuildBaseInfo.m_szName; }
    DT_GUILD_FIGHT_STATE_INFO& GetGuildFightState() { return m_stFightState; }
    bool GetCanSwap() { return m_bCanSwap; }
    void SetCanSwap(bool bFlag) { m_bCanSwap = bFlag; }
    GuildMember& GetGuildMemInfo() { return m_oGuildMember; }

	void BroadMemberInfoChange(DT_ONE_GUILD_MEMBER& rstOneMember);

    //军团练兵场为他人加速
    int SpeedPartner(uint64_t ullSpeeder, uint64_t ullSpeeded);
    //军团练兵场获取被加速次数
    int GetSpeedInfo(uint64_t ullUin, uint8_t& bCount);
    //关注|取关
    int ChangeStar(uint64_t ullOperator, uint64_t ullTarget, uint8_t bType, DT_GUILD_HANG_STAR_INFO& rstHangStarInfo);

	//********** 军团远征
	//上传军团远征信息
	void UploadGuildExpeditionInfo();

	//处理远征发奖
	void HandleExpeditionAward(uint8_t bAwardType);

	//更新远征中已设置防守的成员信息
	void UpdateExpeidtionDefendInfo(DT_GUILD_PKG_EXPEDITION_UPDATE_DEFEND_INFO& rstDefendInfo);

	//发送当前公会状态给远征服务器
	//Type=1#公会解散|2#玩家退出公会
	void SendGuildStateToExpeditionSvr(uint8_t bType, uint64_t ullUin);

	//远征是否开启
	bool IsExpeditionOpen();

    //获取公会科技信息
    GuildSociety& GetGuildSocietyInfo() { return m_oGuildSociety; }
protected:
	DT_GUILD_RANK_INFO& GetGuildRankInfo();
	DT_GFIGHT_RANK_INFO& GetGFightRankInfo();

    //发公会全局信息变化的通知 GUILD_CHANGE_TYPE_DEFAULT=0
    void _SendUptGuildGlobalNtf(uint8_t bChangeType = 0, DT_GUILD_DONATE_INFO* pstDonateInfo=NULL);

    //发送公会成员信息变化的通知
    void _SendUptGuildMemberNtf(uint8_t bIsAdd, DT_ONE_GUILD_MEMBER& rstOneMember, DT_ONE_GUILD_MEMBER* pstOneMemberOfficer = NULL);

    //发送公会申请列表变化的通知
    void _SendUptGuildApplyNtf(uint8_t bIsAdd, DT_ONE_GUILD_MEMBER& rstOneApply);

    //发送公会录像列表变化的通知
    void _SendUptGuildReplayNtf(DT_REPLAY_INFO& rstOneReplay);

    //发送公会房间变化的通知
    void _SendUptGuildRoomNtf(uint8_t bType, DT_PVP_ROOM_INFO& rstRoomInfo);

    //发送军团战状态变化的通知
    void _SendFightStateChgNtf();

    //发送军团解散的通知
    void _SendGuildDissolveNtf();

    //检查录像文件是否存在
    bool _CheckReplayExist(uint32_t dwRaceNumber);

    //更新录像
    void _UpdateReplay();

    //公会Boss信息通知
    //  当bKilled= 1 时,dwBossId 才有效,表示,某个Boss被杀死
    void _SendGuildBossNtf(uint8_t bKilled = 0, uint32_t dwBossId = 0);

    void _UpdateBossInfo();

	//fund够了工会自动升级，故重写个升级函数，原有的保留，以防万一
	void _LevelUp();

	//科技树信息通知
	void _SendGuildSocietyNtf(uint8_t bType, DT_GUILD_DONATE_INFO* pstDonateInfo=NULL);

	//获取当前职位允许的最大人数
	int _GetTitleNum(uint8_t bTitle);

	//广播升职公告
	void _SendGuildIdentityChangeNtf(DT_ONE_GUILD_MEMBER& rstOneMember, uint8_t bType);

	//广播入会/离会公告
	void _SendGuildJoinOrLeaveEventNtf(DT_ONE_GUILD_MEMBER& rstOneMember, uint8_t bType);

    //ullUin是否已被加过速
    bool _IsInSpeederInfo(DT_GUILD_HANG_SPEED_PARTNER_INFO& rstSpeederInfo, uint64_t ullUin);

	//检查军团长
	void _CheckLeader();

	

private:
    //军团基本信息
    DT_GUILD_BASE_INFO m_stGuildBaseInfo;

    //军团全局信息
    DT_GUILD_GLOBAL_INFO m_stGuildGlobalInfo;

    //军团成员信息
    GuildMember m_oGuildMember;

    //军团申请信息
    GuildApply m_oGuildApply;

    //军团房间信息
    GuildReplay m_oGuildReplay;

    //军团房间信息
    GuildPvpRoom m_oGuildRoom;

    //军团战状态信息
    DT_GUILD_FIGHT_STATE_INFO  m_stFightState;

    //军团排名信息
    DT_GUILD_RANK_INFO m_stGuildRankInfo;

    //军团战排名信息
    DT_GFIGHT_RANK_INFO m_stGFightRankInfo;

    //军团中副军团长的个数
    int m_iDeputyNum;

	//军团中精英(长老)的个数
	int m_iEliteNum;

    //待检查录像列表
    ListReplay_t m_listToCheckedReplay;

    //军团BOSS信息
    GuildBoss m_oGuildBoss;

	//公会科技树
	GuildSociety m_oGuildSociety;

    //军团Boss伤害排行榜人数
    uint16_t m_wGuildBossDamageRankNum;

    //是否能从内存中换出标志(处于军团战的公会，不能从内存中换出)
    bool m_bCanSwap;

    //上次检查时间
    uint64_t m_ullLastCheckTime;

	//最近上传军团远征信息的时间
	uint64_t m_ullLastUploadGuildExpeditionTime;

	//上传公会信息次数控制
	uint16_t m_wUploadExpeditionCtrl;


};
