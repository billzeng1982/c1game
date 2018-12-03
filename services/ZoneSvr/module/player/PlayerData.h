#pragma once
#include <set>
#include <vector>
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "PlayerEquipInfo.h"
#include "strutil.h"

using namespace PKGMETA;
using namespace std;

class Player;
class PlayerData
{
public:
    PlayerData();
    virtual ~PlayerData(){};

    void Init(Player* pPlayer);
    void Clear();

private:
    DT_ROLE_BASE_INFO 		 m_stRoleBaseInfo;
    DT_ROLE_MAJESTY_INFO 	 m_stMajestyInfo;
    PlayerEquipInfo 		 m_stEquipInfo;
    DT_ROLE_GCARD_INFO 		 m_stGCardInfo;
    DT_ROLE_MSKILL_INFO 	 m_stMSkillInfo;
    DT_ROLE_PROPS_INFO 	 	 m_stPropsInfo;
	DT_ROLE_ITEMS_INFO		 m_stItemsInfo;
    DT_ROLE_ELO_INFO 		 m_stELOInfo;
    DT_ROLE_TASK_INFO 		 m_stTaskInfo;
    DT_ROLE_PVE_INFO 		 m_stPveInfo;
    DT_ROLE_MISC_INFO 		 m_stMiscInfo;
    DT_ROLE_GUILD_INFO 		 m_stGuildInfo;
    DT_ROLE_TACTICS_INFO     m_stTacticsInfo;

    unsigned char m_szRoleBaseInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRoleMajestyInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRoleGCardInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRoleMSkillInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRolePropsInfo_md5[MD5_DIGEST_SIZE];
	unsigned char m_szRoleItemsInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRoleELOInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szRoleTaskInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szPveInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szMiscInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szGuildInfo_md5[MD5_DIGEST_SIZE];
    unsigned char m_szTacticsInfo_md5[MD5_DIGEST_SIZE];

public:
    // temp data
    Player* m_pOwner;
    uint64_t m_ullUin;
    uint64_t m_ullDungeonTimeMs;
    uint64_t m_ullPveTimeStamp;
    uint64_t m_ullRoomNo;
    uint8_t m_bBossResetNum;    //只做判断用,每次从公会刷新数据会更新
    DT_FIGHT_PLAYER_INFO	m_oSelfInfo;
    DT_FIGHT_PLAYER_INFO	m_oOpponentInfo;
    DT_FRIEND_AGREE_INFO	m_oFriendAgreeInfo;	//好友关系信息
    uint32_t m_dwCloneBattleFightBossId;        //克隆战打的BossId
    uint8_t m_bMatchState;
    uint32_t m_dwPubMailSeq;
	uint32_t m_dwLeaderValue; //统帅值

    bool m_bIsJoinGuild;
    bool m_bIsNeedCalLv;
    float m_afMSkillAttr[MAX_ATTR_ADD_NUM];     //军师技属性加成
    bool m_bIsMSkillInit;
    bool m_bIsGardFateInit;
    std::set<uint32_t> m_setOpenGCardFate;           //武将缘分已激活Id
    vector<uint32_t> m_OpenedActIdVector;            //已开启的活动列表
	bool m_bIsFeedTrainInit;
	float m_afWeiFeedTrainAttr[MAX_ATTR_ADD_NUM];	//魏属性加成
	float m_afShuFeedTrainAttr[MAX_ATTR_ADD_NUM];	//蜀属性加成
	float m_afWuFeedTrainAttr[MAX_ATTR_ADD_NUM];	//吴属性加成
	float m_afOtherFeedTrainAttr[MAX_ATTR_ADD_NUM];	//群属性加成
	float m_afAllFeedTrainAttr[MAX_ATTR_ADD_NUM];	//全体属性加成
    map<uint32_t, DT_ACT_TIME>                  m_PrivateActMap;         //私有活动开启 <活动Id, 活动时间>
    uint64_t m_ullPrivateActLastUpdateTime;         //最近更新时间

	uint8_t m_bGuildExpeditionFightSceneNum;

	uint64_t m_ullCardLastUptTime;

private:
    void _NewInitRoleBaseInfo();
    void _NewInitRoleMajestyInfo();
    void _NewInitRoleEquipInfo();
    void _NewInitRoleGCardInfo();
    void _NewInitRoleMSkillInfo();
    void _NewInitRolePropsInfo();
	void _NewInitRoleItemsInfo();
    void _NewInitRoleELOInfo();
    void _NewInitRoleTaskInfo();
    void _NewInitRolePveInfo();
    void _NewInitRoleMiscInfo();
    void _NewInitRoleGuildInfo();
    void _NewInitRoleTacticsInfo();

    bool _PackRoleBaseInfo(DT_ROLE_BASE_INFO& rstInfo);
    bool _PackRoleMajestyInfo(DT_ROLE_MAJESTY_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleEquipInfo(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleGCardInfo(DT_ROLE_GCARD_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleMSkillInfo(DT_ROLE_MSKILL_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRolePropsInfo(DT_ROLE_PROPS_BLOB& rstBlob, uint16_t wVersion = 0);
	bool _PackRoleItemsInfo(DT_ROLE_ITEMS_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleELOInfo(DT_ROLE_ELO_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleTaskInfo(DT_ROLE_TASK_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRolePveInfo(DT_ROLE_PVE_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleMiscInfo(DT_ROLE_MISC_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleGuildInfo(DT_ROLE_GUILD_BLOB& rstBlob, uint16_t wVersion = 0);
    bool _PackRoleTacticsInfo(DT_ROLE_TACTICS_BLOB& rstBlob, uint16_t wVersion = 0);

public:
    void NewInit();
    bool InitFromDB(DT_ROLE_WHOLE_DATA& rstRoleWholeData);
    void SetMajestyName(const char* pszName) { StrCpy(m_stMajestyInfo.m_szName, pszName, MAX_NAME_LENGTH); }
    bool PackRoleDataUpt(INOUT PKGMETA::SS_PKG_ROLE_UPDATE_REQ& rstRoleUptReq);
    bool PackRoleWholeData(INOUT PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData);

    DT_ROLE_BASE_INFO& GetRoleBaseInfo() { return m_stRoleBaseInfo; }
    DT_ROLE_MAJESTY_INFO& GetMajestyInfo() { return m_stMajestyInfo; }
    PlayerEquipInfo& GetEquipInfo() { return m_stEquipInfo; }
    DT_ROLE_GCARD_INFO& GetGCardInfo() { return m_stGCardInfo; }
    DT_ROLE_MSKILL_INFO& GetMSkillInfo() { return m_stMSkillInfo; }
    DT_ROLE_PROPS_INFO& GetPropsInfo() { return m_stPropsInfo; }
	DT_ROLE_ITEMS_INFO& GetItemsInfo() { return m_stItemsInfo; }
    DT_ROLE_ELO_INFO& GetELOInfo() { return m_stELOInfo; }
    DT_ROLE_TASK_INFO& GetTaskInfo() { return m_stTaskInfo; }
    DT_ROLE_PVE_INFO& GetPveInfo() { return m_stPveInfo; }
    DT_ROLE_MISC_INFO& GetMiscInfo() { return m_stMiscInfo; }
    DT_ROLE_GUILD_INFO& GetGuildInfo() { return m_stGuildInfo; }
    DT_ROLE_CLONE_BATTLE_INFO& GetCloneBattleInfo() { return m_stELOInfo.m_stCloneBattleInfo; }
    DT_ROLE_MINE_INFO& GetMineInfo() { return m_stELOInfo.m_stMineInfo; }
    DT_ROLE_TACTICS_INFO& GetTacticsInfo() { return m_stTacticsInfo; }

    uint32_t GetGold() { return m_stMajestyInfo.m_dwGold; }
    uint32_t GetDiamond() { return m_stMajestyInfo.m_dwDiamond; }
    uint32_t GetAP() { return m_stMajestyInfo.m_dwAP; }
    uint8_t GetVIPLv() { return m_stMajestyInfo.m_bVipLv; }
    uint16_t GetLv() { return m_stMajestyInfo.m_wLevel; }
	uint64_t GetGuildId() { return m_stGuildInfo.m_ullGuildId; }
	uint16_t GetHeadIconId() { return m_stMajestyInfo.m_wIconId; }
	uint16_t GetHeadFrameId() { return m_stMajestyInfo.m_wFrameId; }
	uint32_t GetHighestLi() { return m_stMajestyInfo.m_dwHighestLi; }
	void GetRoleName(OUT char* pszName ) { StrCpy(pszName, m_stMajestyInfo.m_szName, MAX_NAME_LENGTH); }
    const char* GetRoleName() { return m_stMajestyInfo.m_szName; }
    unsigned char* GetRoleBaseInfo_md5() { return m_szRoleBaseInfo_md5; }
    unsigned char* GetRoleMajestyInfo_md5() { return m_szRoleMajestyInfo_md5; }
    unsigned char* GetRoleGCardInfo_md5() { return m_szRoleGCardInfo_md5; }
    unsigned char* GetRoleMSkillInfo_md5() { return m_szRoleMSkillInfo_md5; }
    unsigned char* GetRolePropsInfo_md5() { return m_szRolePropsInfo_md5; }
	unsigned char* GetRoleItemsInfo_md5() { return m_szRoleItemsInfo_md5; }
    unsigned char* GetRoleELOInfo_md5() { return m_szRoleELOInfo_md5; }
    unsigned char* GetRoleTaskInfo_md5() { return m_szRoleTaskInfo_md5; }
    unsigned char* GetRolePveInfo_md5() { return m_szPveInfo_md5; }
    unsigned char* GetRoleMiscInfo_md5() { return m_szMiscInfo_md5; }
    unsigned char* GetRoleGuildInfo_md5() { return m_szGuildInfo_md5; }
    unsigned char* GetRoleTacTicsInfo_md5() { return m_szTacticsInfo_md5; }
};

