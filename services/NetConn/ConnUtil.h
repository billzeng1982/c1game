#ifndef _CONN_UTIL_H_
#define _CONN_UTIL_H_

#include <time.h>
#include "ss_proto.h"
#include "cs_proto.h"
#include "list_i.h"
#include "NetUtil.h"

#define PER_CLT_MAX_BUFF_BLOCK 10
#define MAX_BUFF_BLOCK 2000
#define MAX_BUFF_BLOCK_SIZE 20480

enum CLIENT_STATE
{
	CLIENT_STATE_NULL = 0,
	CLIENT_STATE_LOGIN = 1,  // 登入
	CLIENT_STATE_INGAME = 2, // 在游戏中
	CLIENT_STATE_LOGOUT = 3, // 退出
};


// client 统计信息
struct SCltStatsInfo
{
	uint64_t	m_ulSendPkgs;
	uint64_t	m_ulSendBytes;
	uint64_t	m_ulRecvPkgs;
	uint64_t	m_ulRecvBytes;
};

// ZoneConn统计信息
struct SLogicSvrConnStatsInfo
{
	uint64_t	m_ulCltConnNum;		// 来自Client的连接请求数量
	uint64_t	m_ulRecvBusPkgs; 	// 从bus读取到的消息数量
	uint64_t	m_ulRecvBusBytes; 	// 从bus读取到的消息字节数
	uint64_t	m_ulRecvCltPkgs;	// 从client读取到的消息数量
	uint64_t	m_ulRecvCltBytes; 	// 从client读取到的消息字节数
	uint64_t	m_ulSendCltPkgs;	// 发往client的消息数量
	uint64_t	m_ulSendCltBytes;	// 发往client的消息字节数
};


// 数据缓存(下行包)
struct SBuffBlock
{
	TLISTNODE	m_stNode;			// 以队列方式串接上SBuffBlockSet::m_stBufBlkHead上
	char 		m_szBuff[MAX_BUFF_BLOCK_SIZE];
	int 		m_iHead;			// 向缓存读出数据时,m_iHead增加
	int 		m_iTail;			// 向缓存写数据时，m_iTail增加

    SBuffBlock()
    {
        this->Reset();
    }

	void Reset()
	{
		TLIST_INIT(&m_stNode);
        m_szBuff[0] = '\0';
		m_iHead = 0;
		m_iTail = 0;
	}
};


// 一个client的下行缓存数据块队列
struct SBuffBlockQ
{
	int			m_iNodeNum;			// 串接SBuffBlock个数
	bool		m_bCloseAftSnd;		// 缓存队列发送完即关闭flag
	TLISTNODE	m_stBufBlkHead;

	void Reset()
	{
		m_iNodeNum = 0;
		m_bCloseAftSnd = false;
		TLIST_INIT(&m_stBufBlkHead);
	}
};

struct SClient
{
	int 		m_iHnd;				// 本结构handle, 等于 pos+1
	char		m_szAccountName[PKGMETA::MAX_NAME_LENGTH];	// 账号名称
	uint64_t  m_ullUin;
	
	int 		m_iSock;
	uint32_t	m_dwSessionId;		// 该client在线期间, bus pkg在zone_connect和zone_server间传递的session id
	char 		m_szIPStr[32];
	uint32_t 	m_dwIP;				// 本机序, 记录对端ip
	uint16_t 	m_wPort;			// 本机序, 记录对端port
	uint32_t	m_dwCltVer;			// client版本号
	time_t		m_lLoginTime;		// 登入时间
	time_t  	m_lActiveTime; 		// 最近一次活跃时间，秒
	int			m_iState; 			// 当前状态
	bool		m_bExitByLogicSvr;	// 注意, 此标记为true, 表明zone_server已经stop了, zone_connect不要再发SESSION_STOP给zone_server
	int			m_iRecvLen;			// 一个client pkg的已接收bytes
	int			m_iCltPkgLen;		// 一个client pkg的总长
	char	    m_szCltPkg[ sizeof(PKGMETA::CSPKG) ];	// client上发的包
	time_t		m_lRecvStatsTime;	// 对接收Client包进行统计的上次时间
	uint16_t	m_wRecvStatsPkgs; 	// 统计周期内读到的包数
	uint32_t	m_dwRecvStatsBytes;  // 统计周期内读到的字节数
	SBuffBlockQ	m_stBuffBlockQ;		// 下行数据缓冲队列
	SCltStatsInfo m_StatsInfo;		// 单个client的统计信息
	CSocketUDP  m_oSocketUDP;
	void*		m_poUDPBuffIndex;
	uint32_t 	m_dwCurSendSeq;		// 发送序列号计数
	uint32_t 	m_dwCurRecvSeq;		// 接收序列号计数
	uint16_t	m_wTokenId;        // 检验连接持有者
	uint16_t    m_wRTT;
    uint16_t    m_wDev;
    uint16_t    m_wRTO;
    uint16_t    m_wPeerCurRecvSeq;  //对端当前处理到的seq号
    uint8_t     m_bRepeatSeqCount;  //收到对端重复的CurRecvSeq的次数
	bool 		m_bResendTimeOut;
	char		m_szAesKey[PKGMETA::MAX_AES_KEY_LEN];  //加密和解密的秘钥，在每次client登录时服务器产生
};

#define IS_CLIENT_IN_STATE( pstClient, _state_ ) ( (pstClient)->m_iState == (_state_) )
#define SET_CLIENT_IN_STATE( pstClient, _state_ ) ( (pstClient)->m_iState = _state_ )


#endif

