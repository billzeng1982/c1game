#ifndef _COMM_BUS_LAYER_H_
#define _COMM_BUS_LAYER_H_

#include "define.h"
#include "tbus/tbus.h"
#include "MyTdrBuf.h"
#include "cs_proto.h"

#define CSPKG_HEAD_LEN sizeof(PKGMETA::CSPKGHEAD)
#define CSPKG_BODY_LEN( pszCsPkg ) ( tdr_ntoh16(((PKGMETA::CSPKG*)(pszCsPkg))->m_stHead.m_wBodyLen) )
#define CSPKG_LEN( pszCsPkg) ( CSPKG_HEAD_LEN + CSPKG_BODY_LEN( pszCsPkg ) )
#define CSPKG_MSGID( pszCsPkg ) ( tdr_ntoh16(((PKGMETA::CSPKG*)(pszCsPkg))->m_stHead.m_wMsgId) ) 
#define CSPKG_PROTOTYPE( pszCsPkg ) (((PKGMETA::CSPKG*)(pszCsPkg))->m_stHead.m_bProtoType)
#define CSPKG_SEQNUM( pszCsPkg ) ( tdr_ntoh32(((PKGMETA::CSPKG*)(pszCsPkg))->m_stHead.m_dwSeqNum) ) 
#define CSPKG_TOKENID( pszCsPkg ) ( tdr_ntoh16(((PKGMETA::CSPKG*)(pszCsPkg))->m_stHead.m_wTokenId) ) 


#define SCPKG_HEAD_LEN sizeof(PKGMETA::SCPKGHEAD)
#define SCPKG_BODY_LEN( pszScPkg ) ( tdr_ntoh16(((PKGMETA::SCPKG*)(pszScPkg))->m_stHead.m_wBodyLen) )
#define SCPKG_LEN( pszScPkg) ( SCPKG_HEAD_LEN + SCPKG_BODY_LEN( pszScPkg ) )
#define SCPKG_MSGID( pszScPkg ) ( tdr_ntoh16(((PKGMETA::SCPKG*)(pszScPkg))->m_stHead.m_wMsgId) ) 
#define SCPKG_SEQNUM( pszScPkg ) ( tdr_ntoh32(((PKGMETA::SCPKG*)(pszScPkg))->m_stHead.m_dwSeqNum) ) 
#define SCPKG_PROTOTYPE( pszScPkg ) ((((PKGMETA::SCPKG*)(pszScPkg))->m_stHead.m_bProtoType)) 

#define SSPKG_MSGID( pszSsPkg ) ( tdr_ntoh16( ((PKGMETA::SSPKG*)(pszSsPkg))->m_stHead.m_wMsgId ) )
#define SSPKG_RESERVID( pszSsPkg ) ( tdr_ntoh64( ((PKGMETA::SSPKG*)(pszSsPkg))->m_stHead.m_ullReservId ) )
#define SSPKG_DST_ADDR( pszSsPkg ) ( tdr_ntoh32( ((PKGMETA::SSPKG*)(pszSsPkg))->m_stHead.m_iDstProcId ) )
#define SSPKG_SRC_ADDR( pszSsPkg ) ( tdr_ntoh32( ((PKGMETA::SSPKG*)(pszSsPkg))->m_stHead.m_iSrcProcId ) )
#define SSPKG_HEAD_UIN( pszSsPkg ) ( tdr_ntoh64(((PKGMETA::SSPKG*)(pszSsPkg))->m_stHead.m_ullUin ))

#define DEAL_PKG_PER_LOOP 10

class CCommBusLayer
{
public:
	CCommBusLayer();
	~CCommBusLayer();

	char*   GetErrMsg() { return m_szErrMsg; }
	int     GetRecvMsgSrc()     const { return m_iSrc; }
    int     GetRecvMsgDst()     const { return m_iDst; }
	MyTdrBuf* GetRecvTdrBuf( ) { return &m_stRecvBuf; }

    int Init(int iGCIMKey, int iAppAddr, int iFlag = TBUS_INIT_FLAG_USE_SAVED_TIME );

	/*
		�����յ������ֽ���
		0 - no msg
		<0 - error
	*/
	int Recv( );

	/*
		���ط��͵��ֽ���
		<0 - error
	*/
	int Send( TBUSADDR iDstAddr, const MyTdrBuf& rstTdrBuf );
	int Send( TBUSADDR iDstAddr, char* pszPackBuff, size_t uPackLen );

	uint32_t GetRecvMsgLen() { return m_stRecvBuf.m_uPackLen; }

	// �����ת��Ϣ������·����Ϣ
	bool Backward( char* pszPkg, size_t uPkgLen );
	bool Backward();

	
    /*! 
    @brief ת��ָ����������
    @param[in]  iDstAddr: ת��Ŀ���ַ
    @param[in]  pszPkg: ת�����ݱ���
    @param[in]  iPkgLen: ת�����ĳ���
    @return �Ƿ�ת���ɹ�
    */
    bool    Forward(TBUSADDR iDstAddr, char* pszPkg, size_t uPkgLen );

    /*! 
    @brief ֱ��ת����ǰ����
    @param[in]  iDstAddr: ת��Ŀ���ַ
    @return �Ƿ�ת���ɹ�
    */
    bool    Forward(TBUSADDR iDstAddr);

	/* ����ȫ��GCIM�е�����ˢ��tbus�����������ͨ��, ���ڲ�ͣ����̬���ͨ�Ž���
	*/
	bool RefreshHandle();
	
private:
	char m_szErrMsg[256];
	int m_iBusHandle;

	int m_iLocalAddr;
	int m_iConnAddr;

	int m_iSrc;
	int m_iDst;
	uint32_t m_uiDyeID;       // Ⱦɫ��Ϣ������

	MyTdrBuf m_stRecvBuf;
};

#endif

