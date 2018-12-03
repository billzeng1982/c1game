#ifndef _T_GAME_DATA_MGR_H_
#define _T_GAME_DATA_MGR_H_

/*
	实现一个game data mgr的模板.
	一个模板只对应一个资源bin文件，因而只对应一个配置数据结构.
	某些game data mgr类, 比如status, 可能对应几个资源文件, 该game data mgr类就聚合多个模板实例.
	读某些资源文件可能会涉及到数据检查, 这种game data mgr类就单独实现了

	特别注意: (使用限制!!!)
	- 由于trl_find是二分查找法, key 是2,4,8字节整型必须严格区分, 比如:
	  2字节key(short)绝对不能调用4字节的find方法，反之亦然
	- key的大小不要超过相应字节整型(有符号)的最大值, 否则会找不到. 如int16_t,如果key>0x7FFF,就找不到了
	- key必须是资源映射文件的第一个元素,否则查找不到
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

// TKey目前只能是2,4,8字节int
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
	char* m_pszBuff; 	// 保存数据的数据区指针的指针 
    int   m_iBuff;		// 数据区的字节数 
    int   m_iUint;		// 单个资源信息结构体的在缓冲区中的存储空间大小 
    int   m_iResNum;    // 缓冲区中保存的资源信息结构体的个数 
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

