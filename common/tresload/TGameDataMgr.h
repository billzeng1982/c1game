#ifndef _T_GAME_DATA_MGR_H_
#define _T_GAME_DATA_MGR_H_

/*
	ʵ��һ��game data mgr��ģ��.
	һ��ģ��ֻ��Ӧһ����Դbin�ļ������ֻ��Ӧһ���������ݽṹ.
	ĳЩgame data mgr��, ����status, ���ܶ�Ӧ������Դ�ļ�, ��game data mgr��;ۺ϶��ģ��ʵ��.
	��ĳЩ��Դ�ļ����ܻ��漰�����ݼ��, ����game data mgr��͵���ʵ����

	�ر�ע��: (ʹ������!!!)
	- ����trl_find�Ƕ��ֲ��ҷ�, key ��2,4,8�ֽ����ͱ����ϸ�����, ����:
	  2�ֽ�key(short)���Բ��ܵ���4�ֽڵ�find��������֮��Ȼ
	- key�Ĵ�С��Ҫ������Ӧ�ֽ�����(�з���)�����ֵ, ������Ҳ���. ��int16_t,���key>0x7FFF,���Ҳ�����
	- key��������Դӳ���ļ��ĵ�һ��Ԫ��,������Ҳ���
*/

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "tresloader.h"
#include "define.h"
#include "workdir.h"


#define DECL_INIT_RES_MGR( MgrObj, Path ) \
	bool Init##MgrObj() \
	{\
		char szFilePath[PATH_MAX]; \
		snprintf( szFilePath, sizeof(szFilePath), "%s/%s", CWorkDir::string(), Path); \
	    if ( !MgrObj.Load( szFilePath ) ) \
	    { \
			printf( "\033[0;31;40m[LogErr]Load resource file: %s failed...[%s:%s():%d]\033[0m\n", szFilePath, __FILE__,__FUNCTION__, __LINE__ ); \
			return false; \
	    } \
	    return true; \
	}

#define INIT_RES_MGR( MgrObj ) Init##MgrObj()

// TKeyĿǰֻ����2,4,8�ֽ�int
template <typename TRES, typename TKEY = int>
class TGameDataMgr
{
public:
	TGameDataMgr()
	{
		m_pszBuff = NULL;
		m_iBuff	  = 0;
		m_iUint   = 0;
		m_iResNum = 0;
	}

	~TGameDataMgr()
	{
		trl_unload( &m_pszBuff ); 
	}

	bool	Load( char* pszResFilePath );
    bool    Reload( char* pszResFilePath )
    {
        Free();
        char szFilePath[PATH_MAX];
		snprintf( szFilePath, sizeof(szFilePath), "%s/%s", CWorkDir::string(), pszResFilePath);
        return Load(szFilePath);
    }

	TRES* 	Find( TKEY nKey );

	int    GetResNum() { return m_iResNum; }
	char*  GetResBuf( ) { return m_pszBuff; }
	int    GetResBufSize() { return m_iBuff; }
	int    GetUnitSize() { return m_iUint; }

	TRES*  GetResByPos( int iPos );

	void   Free( ) 
	{ 
		trl_unload( &m_pszBuff ); 
		m_pszBuff = NULL;
        m_iBuff = 0;
        m_iUint = 0;
        m_iResNum = 0;
	}
	
private:
	char* m_pszBuff; 	// �������ݵ�������ָ���ָ�� 
    int   m_iBuff;		// ���������ֽ��� 
    int   m_iUint;		// ������Դ��Ϣ�ṹ����ڻ������еĴ洢�ռ��С 
    int   m_iResNum;    // �������б������Դ��Ϣ�ṹ��ĸ��� 
};


template <typename TRES, typename TKEY>
bool TGameDataMgr<TRES, TKEY>::Load( char* pszResFilePath )
{
	if( NULL == pszResFilePath )
	{
		assert( false );
		return false;
	}

	int iRet = trl_load( &m_pszBuff, &m_iBuff, &m_iUint, pszResFilePath, NULL, 0 );
	if( iRet <= 0 )
	{
		return false;
	}

	m_iResNum = iRet;
	return true;
}

template <typename TRES, typename TKEY>
TRES* TGameDataMgr<TRES, TKEY>::Find( TKEY nKey )
{
	if( !m_pszBuff )
	{
		return NULL;
	}

	char* pszRes = NULL;
	switch( sizeof(TKEY) )
	{
		case 2:
		{
			pszRes = trl_find_n( m_pszBuff, m_iResNum, m_iUint, nKey );
		}
		break;
		
		case 4:
		{
			pszRes = trl_find( m_pszBuff, m_iResNum, m_iUint, nKey );
		}
		break;

		case 8:
		{
			pszRes = trl_find_ll( m_pszBuff, m_iResNum, m_iUint, nKey );
		}
		break;
	}

	return (TRES*)pszRes;
}


template <typename TRES, typename TKEY>
TRES*  TGameDataMgr<TRES, TKEY>::GetResByPos( int iPos )
{
	if( iPos < 0 || iPos >= m_iResNum )	
	{
		return NULL;
	}

	return (TRES*)( m_pszBuff + iPos * sizeof(TRES) );
}

#endif

