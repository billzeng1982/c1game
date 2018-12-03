#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{

	if( !INIT_RES_MGR(m_oResAccountMgr) ) 		return false;
	
    return true;
}

bool CGameDataMgr::Init(char* szResFile)
{
    
		char szFilePath[PATH_MAX]; 
		snprintf( szFilePath, sizeof(szFilePath), "%s", szResFile); 
	    if ( !m_oResAccountMgr.Load( szFilePath ) ) 
	    { 
			printf( "\033[0;31;40m[LogErr]Load resource file: %s failed...[%s:%s():%d]\033[0m\n", szFilePath, __FILE__,__FUNCTION__, __LINE__ ); 
			return false; 
	    } 
	    return true; 
	
}

