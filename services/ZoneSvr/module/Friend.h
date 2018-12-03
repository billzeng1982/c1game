#pragma once
#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "player/Player.h"



class Friend : public TSingleton<Friend>
{
public:
	Friend(){};
	virtual ~Friend() {};
private:
public:
	void UpdatePlayerInfo(PlayerData* pstData, uint8_t bIsOnline = 1);
	void InitPlayerInfo(IN PlayerData* pstData, OUT DT_FRIEND_PLAYER_INFO& stPlayerInfo ) ;
	void InitAgreeInfo(IN PlayerData* pstData, OUT DT_FRIEND_AGREE_INFO& rstAgreeInfo);
	void UpdateAgreeInfo(PlayerData* pstData, uint8_t bType, uint64_t ullUinReceiver);
	bool IsFriend(PlayerData* pstData, uint64_t ullMateUin);	//是否为好友
	void UpdateServer();
	bool Init();
	void IsUpdatePlayerData(PlayerData* pstData, OUT uint64_t& ullTimestamp);
    
private:
	void _DeleteFriend(uint64_t ullUinSender, DT_FRIEND_AGREE_INFO& rstAgreeInfo, uint64_t ullUinReceiver);
	void _AgreeFriend(uint64_t ullUinSender, DT_FRIEND_AGREE_INFO& rstAgreeInfo, uint64_t ullUinReceiver);
private:
	SSPKG m_stSsPkg;
	uint64_t m_ullDayUpdateLastTime;	//上次更新重置时间
	int m_iUptTime;
};


