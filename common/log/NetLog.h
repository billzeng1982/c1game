#ifndef _NET_LOG_H_
#define _NET_LOG_H_

#include "singleton.h"
#include "tlog/tlog.h"
#include "tloghelp/tlogload.h"

class CNetLog
{
public:
	CNetLog()
	{
		m_iLogBuffLen = 0;
		m_iLogBuffUsedLen = 0;
		m_pstLogCtx = NULL;
		bzero( &m_stFormat, sizeof( m_stFormat ) );
    	bzero( &m_stFormatBuff, sizeof( m_stFormatBuff ) );
	}
	~CNetLog()
	{
		tlog_fini_ctx( &m_pstLogCtx );
		if (m_pstMetaLib)
	    {
	        tdr_free_lib(&m_pstMetaLib);
		}
	}

	bool Init( int iBuffSize, const char* pszConfPath, const char* pszCategory, const char* pszMetaFile );

	int Write( const char* pszTableName, LPTDRDATA pstHost );
	int Write( const char* pszTableName, const char *pstLog, int iLen );
	
private:
    int     m_iLogBuffLen;
    int     m_iLogBuffUsedLen;
	LPTLOGCTX 			m_pstLogCtx;
	LPTLOGCATEGORYINST 	m_pstLogCategory;
	LPTDRMETALIB		m_pstMetaLib;
	TDRDATA				m_stFormatBuff;
	TDRPRINTFORMAT 		m_stFormat;
};

#endif

