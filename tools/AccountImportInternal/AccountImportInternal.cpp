#include "GameDataMgr.h"
#include "common_proto.h"
#include "mysql/MysqlHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include "hash_func.h"

using namespace PKGMETA;

int main(int argc, char * argv[])
{
    char* m_szDBAddr = "192.168.1.28";
    //char* m_szDBAddr = "10.7.4.237";
    int m_iPort = 3306;
    char* m_szResFile;
	int iTableNum = 0;
    int m_iTableNum = 3; //分表数 目前为3
	
    if( argc == 2 )
    {
        m_szResFile = argv[1];
    }

    if( argc == 5 )
    {
        m_szDBAddr = argv[2];
        m_iPort = atoi(argv[3]);
        m_iTableNum = atoi(argv[4]);
    }
    
    CMysqlHandler m_stMysqlHandler;
    if( !m_stMysqlHandler.ConnectDB(m_szDBAddr, m_iPort, "rgame", "root", "123456") )
    {
        printf("Connect DB Err\n");
        return -2;
    }

    if( !m_szResFile )
    {
	    bool bRet = CGameDataMgr::Instance().Init();
        if( !bRet )
        {
            printf("GameData init failed");
            return -4;
        }
    }
    else
    {
        bool bRet = CGameDataMgr::Instance().Init(m_szResFile);
        if( !bRet )
        {
            printf("GameData init failed");
            return -4;
        }
    }
    
    ResAccountMgr_t& roResAccount = CGameDataMgr::Instance().GetResAccountMgr();
	uint32_t dwHash = 0;
	
	for(int i=0;i<roResAccount.GetResNum();i++)
	{
		RESACCOUNTIMPORTINTERNAL* pstResAccount= roResAccount.GetResByPos(i);
		dwHash = ::zend_inline_hash_func( pstResAccount->m_szAccount, strlen( pstResAccount->m_szAccount ) );;
	    iTableNum = dwHash % m_iTableNum; 
	    iTableNum = iTableNum & 0x000000ff;
	    iTableNum++;
        
        m_stMysqlHandler.FormatSql( "INSERT INTO tbl_account_%d "
                                     "SET AccountName='%s', "
                                     "PassWord='%s'", iTableNum, pstResAccount->m_szAccount,pstResAccount->m_szPassWord );

        if( m_stMysqlHandler.Execute() < 0 )
        {
           printf("excute sql failed: %s\n", m_stMysqlHandler.GetLastErrMsg() );
           continue;
        }
        else
        {
            // 生成uin
               m_stMysqlHandler.FormatSql( "SELECT "
                                            "id "
                                            "FROM tbl_account_%d where AccountName='%s'", iTableNum, pstResAccount->m_szAccount );
            
               if( m_stMysqlHandler.Execute() < 0 )
               {
                   printf("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
                   continue;
               }
               
               int iRowEffected = m_stMysqlHandler.StoreResult();
               if( iRowEffected < 0 )
               {
                   continue;
               }
            
               if( 1 == iRowEffected )
               {
                   CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
                   int i = 0;
                   oRowIter.Begin( m_stMysqlHandler.GetResult() );
            
                   uint64_t ullUin = (oRowIter.GetField(i++)->GetInteger() + 10000 << 8) + iTableNum;
            
                   m_stMysqlHandler.FormatSql( "UPDATE tbl_account_%d SET "
                                                "Uin=%lu "
                                                "where AccountName='%s'", iTableNum, ullUin, pstResAccount->m_szAccount );
                   if( m_stMysqlHandler.Execute() < 0 )
                   {
                       printf("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
                       continue;
                   }
               }
            printf("--- %s : %s ---\n", pstResAccount->m_szAccount, pstResAccount->m_szPassWord );
        }
        
	}	
    return 0;
}

