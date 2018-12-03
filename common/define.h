#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "pal/ttypes.h"

#define HARD_LIMIT_CLIENT 8000

#if __GNUC__ > 2
//#warning "__GNUC__ > 2"
#include        <ext/hash_map>
#define         hash_map_t     __gnu_cxx::hash_map

#else
//#warning "__GNUC__ <= 2"
#include        <hash_map>
#define         hash_map_t     std::hash_map

#endif


#ifndef MAX
#define MAX(x,y) ( (x) > (y) ? (x) : (y) )
#endif

#ifndef MIN
#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#endif

/*表示该参数只是输入参数*/
#ifndef IN
#define IN
#endif


/*表示该参数只是输出参数*/
#ifndef OUT
#define OUT
#endif


/*表示该参数既是输入参数，又是输出参数*/
#ifndef INOUT
#define INOUT
#endif

// 毫秒级的一小时
#define ONE_HOUR_BY_MS 3600000
#define ONE_MIN_BY_MS  60000

#ifndef UINT16_MAX
#define UINT16_MAX 0xFFFF
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF
#endif

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF
#endif

#define STRING_NULL "NULL"

#define GET_PROC_ID_ADDR(X) (((int)(X)<<16) | (((int)1)<<24))
#define GET_PROC_ID_ADDR2(X, Y) (((int)(X)<<16) | (((int)Y)<<24))

#define GET_PROC_FUNC_BY_ID(X) ((int)(X)>>16 & (int)0xff)
#define GET_PROC_INST_BY_ID(X) ((int)(X)>>24 & (int)0xff)
#define GET_TABLE_NUM(X) ( (X) & 0x3ff )
/*
    进程id规则: cluster.world.进程id.进程实例id
*/
enum
{
    	PROC_ZONE_CONN = 1,
    	PROC_ZONE_SVR = 2,
    	PROC_FIGHT_CONN = 3,
    	PROC_FIGHT_SVR = 4,
    	PROC_ACCOUNT_SVR = 5,
    	PROC_ROLE_SVR = 6,
    	PROC_MATCH_SVR = 7,
    	PROC_RANK_SVR = 8,
    	PROC_CLUSTER_GATE = 9,
    	PROC_REPLAY_SVR = 10,
    	PROC_GUILD_SVR = 11,
    	PROC_MESSAGE_SVR = 12,
    	PROC_MAIL_SVR = 13,
    	PROC_MAIL_DB_SVR = 14,
    	PROC_FRIEND_SVR = 15,
	PROC_XIYOU_SDK_SVR = 16,
	PROC_SDKDM_SVR = 17,
	PROC_SERIAL_NUM_SVR = 18,
	PROC_DIR_SVR = 19,
	PROC_IDIP_AGENT_SVR = 20,
	PROC_MISC_SVR = 21,
	PROC_LOG_QUERY_SVR = 22,
	PROC_ASYNC_PVP_SVR = 23,
	PROC_CLONE_BATTLE_SVR = 24,
	PROC_MINE_SVR = 25,
	PROC_MINEDB_SVR = 26,
	PROC_CLUSTER_ACCOUNT_SVR = 27,  // 全局帐号表
	PROC_GUILD_EXPEDITION_SVR = 28,
    PROC_MAX = 28,
	PROC_PASS_THROUGH = 9999	//透传
};


#define MD5_DIGEST_SIZE 16 // md5码长度

#define Index2ID(iIndex) ((iIndex) + 1)
#define ID2Index(dwID) ((int)((dwID) - 1))

// -------------------- 各种开关宏定义 ------------------------------------
#define MACRO_FOR_DEMO // 演示需要

// ------------------------------------------------------------------------

#endif

