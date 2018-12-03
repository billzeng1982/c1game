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
	CLIENT_STATE_LOGIN = 1,  // ����
	CLIENT_STATE_INGAME = 2, // ����Ϸ��
	CLIENT_STATE_LOGOUT = 3, // �˳�
};


// client ͳ����Ϣ
struct SCltStatsInfo
{
	uint64_t	m_ulSendPkgs;
	uint64_t	m_ulSendBytes;
	uint64_t	m_ulRecvPkgs;
	uint64_t	m_ulRecvBytes;
};

// ZoneConnͳ����Ϣ
struct SLogicSvrConnStatsInfo
{
	uint64_t	m_ulCltConnNum;		// ����Client��������������
	uint64_t	m_ulRecvBusPkgs; 	// ��bus��ȡ������Ϣ����
	uint64_t	m_ulRecvBusBytes; 	// ��bus��ȡ������Ϣ�ֽ���
	uint64_t	m_ulRecvCltPkgs;	// ��client��ȡ������Ϣ����
	uint64_t	m_ulRecvCltBytes; 	// ��client��ȡ������Ϣ�ֽ���
	uint64_t	m_ulSendCltPkgs;	// ����client����Ϣ����
	uint64_t	m_ulSendCltBytes;	// ����client����Ϣ�ֽ���
};


// ���ݻ���(���а�)
struct SBuffBlock
{
	TLISTNODE	m_stNode;			// �Զ��з�ʽ������SBuffBlockSet::m_stBufBlkHead��
	char 		m_szBuff[MAX_BUFF_BLOCK_SIZE];
	int 		m_iHead;			// �򻺴��������ʱ,m_iHead����
	int 		m_iTail;			// �򻺴�д����ʱ��m_iTail����

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


// һ��client�����л������ݿ����
struct SBuffBlockQ
{
	int			m_iNodeNum;			// ����SBuffBlock����
	bool		m_bCloseAftSnd;		// ������з����꼴�ر�flag
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
	int 		m_iHnd;				// ���ṹhandle, ���� pos+1
	char		m_szAccountName[PKGMETA::MAX_NAME_LENGTH];	// �˺�����
	uint64_t  m_ullUin;
	
	int 		m_iSock;
	uint32_t	m_dwSessionId;		// ��client�����ڼ�, bus pkg��zone_connect��zone_server�䴫�ݵ�session id
	char 		m_szIPStr[32];
	uint32_t 	m_dwIP;				// ������, ��¼�Զ�ip
	uint16_t 	m_wPort;			// ������, ��¼�Զ�port
	uint32_t	m_dwCltVer;			// client�汾��
	time_t		m_lLoginTime;		// ����ʱ��
	time_t  	m_lActiveTime; 		// ���һ�λ�Ծʱ�䣬��
	int			m_iState; 			// ��ǰ״̬
	bool		m_bExitByLogicSvr;	// ע��, �˱��Ϊtrue, ����zone_server�Ѿ�stop��, zone_connect��Ҫ�ٷ�SESSION_STOP��zone_server
	int			m_iRecvLen;			// һ��client pkg���ѽ���bytes
	int			m_iCltPkgLen;		// һ��client pkg���ܳ�
	char	    m_szCltPkg[ sizeof(PKGMETA::CSPKG) ];	// client�Ϸ��İ�
	time_t		m_lRecvStatsTime;	// �Խ���Client������ͳ�Ƶ��ϴ�ʱ��
	uint16_t	m_wRecvStatsPkgs; 	// ͳ�������ڶ����İ���
	uint32_t	m_dwRecvStatsBytes;  // ͳ�������ڶ������ֽ���
	SBuffBlockQ	m_stBuffBlockQ;		// �������ݻ������
	SCltStatsInfo m_StatsInfo;		// ����client��ͳ����Ϣ
	CSocketUDP  m_oSocketUDP;
	void*		m_poUDPBuffIndex;
	uint32_t 	m_dwCurSendSeq;		// �������кż���
	uint32_t 	m_dwCurRecvSeq;		// �������кż���
	uint16_t	m_wTokenId;        // �������ӳ�����
	uint16_t    m_wRTT;
    uint16_t    m_wDev;
    uint16_t    m_wRTO;
    uint16_t    m_wPeerCurRecvSeq;  //�Զ˵�ǰ������seq��
    uint8_t     m_bRepeatSeqCount;  //�յ��Զ��ظ���CurRecvSeq�Ĵ���
	bool 		m_bResendTimeOut;
	char		m_szAesKey[PKGMETA::MAX_AES_KEY_LEN];  //���ܺͽ��ܵ���Կ����ÿ��client��¼ʱ����������
};

#define IS_CLIENT_IN_STATE( pstClient, _state_ ) ( (pstClient)->m_iState == (_state_) )
#define SET_CLIENT_IN_STATE( pstClient, _state_ ) ( (pstClient)->m_iState = _state_ )


#endif

