#include "NetLog.h"

bool CNetLog::Init(int iBuffSize, const char* pszConfPath, const char* pszCategory, const char* pszMetaFile )
{
    if( !iBuffSize || !pszConfPath || !pszCategory || !pszMetaFile )
    {
        printf("Invalid param");
        return false;
    }
    
    m_stFormatBuff.pszBuff = new char[iBuffSize];
    if( !m_stFormatBuff.pszBuff )
    {
        printf("cant alloc memery");
        return false;
    }
    m_stFormatBuff.iBuff = iBuffSize;

    m_pstLogCtx = tlog_init_from_file( pszConfPath );
    if( !m_pstLogCtx )
    {
        printf("Init CNetLog failed");
        return false;
    }

    m_pstLogCategory = tlog_get_category( m_pstLogCtx, pszCategory );
    if( !m_pstLogCategory )
    {
        printf("get category failed");
        return false;
    }

    int iRet = tdr_load_metalib( &m_pstMetaLib, pszMetaFile );
    if (TDR_ERR_IS_ERROR(iRet))
    {        
        printf("Load metalib failed");
        return false;
    }

    tdr_get_default_format( &m_stFormat );
    m_stFormat.pszSepStr = "|";
	m_stFormat.iNoVarName = 1;
	m_stFormat.chIndentChar = 0;
    m_stFormat.iWithVersion = 0;
    
    return true;
}

int CNetLog::Write(const char *pszTableName, LPTDRDATA pstHost )
{
    if( !pszTableName || '\0' == pszTableName || !pstHost->pszBuff || pstHost->iBuff < 0 )
    {
        printf("Invalid input");
        return -1;
    }

    LPTDRMETA pstMeta = (LPTDRMETA)tdr_get_meta_by_name(m_pstMetaLib, pszTableName);
    if( !pstMeta )
    {
        printf("tdr get meta failed");
        return -1;
    }

    int iNameLen = strlen( pszTableName );
    memcpy( m_stFormatBuff.pszBuff, pszTableName, iNameLen );

    TDRDATA stTmpBuff = { m_stFormatBuff.pszBuff+iNameLen, m_stFormatBuff.iBuff-iNameLen };
    int iRet = tdr_sprintf_ex( pstMeta, &stTmpBuff, pstHost, 0, &m_stFormat );
    if( iRet < 0 )
    {
        printf( "tdr sprintf ex failed" );
        return -1;
    }

    tlog_info( m_pstLogCategory, 0, 0, m_stFormatBuff.pszBuff );
    return 0;
}

int CNetLog::Write(const char * pszTableName, const char * pstLog, int iLen)
{
    TDRDATA stHost;
    stHost.pszBuff = (char *)pstLog;
	stHost.iBuff = iLen;
    return Write( pszTableName, &stHost );
}


