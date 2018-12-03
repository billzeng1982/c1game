#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "AppFrame.h"
#include "comm_func.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "CpuSampleStats.h"

bool g_bExit = false;
bool g_bReloadGamedata = false;
bool g_bReloadConfig = false;

// 信号处理函数
void app_on_sigterm( int iSigNo )
{
    switch( iSigNo )
    {
        default:
        {
            g_bExit = true; 
            break;
        }
    }
}

void app_on_siguser1( int iSigNo )
{
    switch( iSigNo )
    {
        default:
        {
            g_bReloadConfig = true;
            break;
        }
    }
}

void app_on_sigusr2( int iSigNo )
{
    switch( iSigNo )
    {
        default:
        {
            g_bReloadGamedata = true; 
            break;
        }
    }
}

void app_on_sigio( int iSigNo )
{
    switch( iSigNo )
    {
        default:
        {
            // dump log immediately
            CBufferedLog &oBufferedLog = CSingleton<CBufferedLog>::Instance();
            if (oBufferedLog.IsBufferedEnabled())
            {
                oBufferedLog.LogFlush();               
            }
            CpuSampleStats::Instance().Dump();
            break;
        }
    }
}


CAppFrame::CAppFrame()
{
    bzero( &m_stAppCfg, sizeof(m_stAppCfg) );
    m_iDaemon = 0;
    m_argc = 0;
    m_argv = NULL;
    m_dwLogTypes = 0;
    bzero( m_szAppName, sizeof(m_szAppName) );
    bzero( &m_tvBootTime, sizeof(m_tvBootTime) );
}

// pszAppName 为NULL，则appName来自于配置文件的名字
bool CAppFrame::SvrInit( int argc, char** argv, const char* pszAppName, uint32_t dwLogTypes )
{
    m_argc = argc;
    m_argv = argv;
    m_dwLogTypes = dwLogTypes;
    
    if( !this->_ProcessArgs( ) )
    {
        return false;
    }

    if( pszAppName && strlen(pszAppName) > 0 )
    {
        StrCpy( m_szAppName, pszAppName, sizeof(m_szAppName) );
    }else
    {
        char* xmlName = CConfXml::string();
        if( 0 == strlen(xmlName) )
        {
            return false;
        }

        char* p = strstr(xmlName, ".xml");
        if( !p )
        {
            return false;
        }

        strncpy( m_szAppName, xmlName, p-xmlName );
    }

    this->_InitLog();
    
    if( !this->_ReadCfg() )
    {
        return false;
    }
    
    // start deamon
    if( m_iDaemon )
    {    
        if( StartDaemonProcess( ) != 0 )
        {
            LOGERR("Start ZoneConn svr daemon failed!");
            return -1;
        }
    }

    // set sighandler;
    signal( SIGTERM, app_on_sigterm ); // 进程退出
    signal( SIGUSR1, app_on_siguser1 ); // reload config file
    signal( SIGUSR2, app_on_sigusr2 ); // reload gamedata
    signal( SIGIO,   app_on_sigio ); // dump local log immediately
    
    this->_KillPre();

    CGameTime::Instance().UpdateTime();
    // init app
    if( !this->AppInit() )
    {
        return false;
    }

    // 记录启动时间    
    m_tvBootTime = *CGameTime::Instance().GetCurrTime();
    return true;
}


void CAppFrame::SvrRun( )
{
    while( !g_bExit )
    {
        CGameTime::Instance().UpdateTime(); // 每帧开始，先刷新游戏时间
        if (g_bReloadGamedata)
        {
			g_bReloadGamedata = false;
            if (!this->ReloadTBusChannel())
            {
                LOGERR("App reload tbus channel failed!");
                break;
            }

            if (!this->ReloadGamedata())
            {
                LOGERR("App reload gamedata failed!");
                break;
            }
        }

        if (g_bReloadConfig)
        {
            g_bReloadConfig = false;
            if (!this->_ReadCfg())
            {
                LOGERR("App reload config failed!");
                break;
            }
        }
        
        this->AppUpdate();
    }

    this->AppFini();
}

bool CAppFrame::_ReadCfg( )
{
    this->_SetAppCfg( );
	LPTDRMETA pstMeta =  tdr_get_meta_by_name(m_stAppCfg.m_pstMetaLib, m_stAppCfg.m_szMetaName);
	if( !pstMeta )
	{
		printf("tdr_get_meta_by_name failed! CfgFile <%s>, MetaName <%s>\n", m_stAppCfg.m_szCfgFile, m_stAppCfg.m_szMetaName);
		return false;
	}

	TDRDATA stHost;
	stHost.iBuff = m_stAppCfg.m_dwLen;
	stHost.pszBuff = m_stAppCfg.m_pszBuff;

	// input file
	int iRet = tdr_input_file( pstMeta, &stHost, m_stAppCfg.m_szCfgFile, \
						       tdr_get_meta_current_version(pstMeta), TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME );

	if ( TDR_ERR_IS_ERROR(iRet) )
	{
		LOGERR("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
    	     return false;
	}

	return true;
}


void CAppFrame::_InitLog( )
{
    // errer和run日志一定有!
    char szLogPath[512];   
    snprintf( szLogPath, sizeof(szLogPath), "%s/log", CWorkDir::string());
    CSingleton<CBufferedLog>::Instance().Init( szLogPath, m_szAppName, 0, 10, LOG_FILE_SIZE );

    // release版本缓存日志，debug直接写  
#ifdef NDEBUG
    CSingleton<CBufferedLog>::Instance().SetLogBuffLen( CBufferedLog::LOGRUN,   LOG_BUFF_SIZE );
    CSingleton<CBufferedLog>::Instance().SetLogBuffLen( CBufferedLog::LOGERR,   LOG_BUFF_SIZE );

    
    if((m_dwLogTypes & CBufferedLog::LOGWARN))    CSingleton<CBufferedLog>::Instance().SetLogBuffLen( CBufferedLog::LOGWARN,  LOG_BUFF_SIZE );
    if((m_dwLogTypes & CBufferedLog::LOGCORE))   CSingleton<CBufferedLog>::Instance().SetLogBuffLen( CBufferedLog::LOGCORE,  LOG_BUFF_SIZE );
    if((m_dwLogTypes & CBufferedLog::LOGSTATS))  CSingleton<CBufferedLog>::Instance().SetLogBuffLen( CBufferedLog::LOGSTATS, LOG_BUFF_SIZE );
#endif
}


void CAppFrame::_KillPre( )
{
    char szPidFile[512];
    int iProcID = this->GetProcID();

    //获取procId最后一段,用result存
    char* tmp = strtok(inet_ntoa(*(struct in_addr *)(&iProcID)), ".");
    char result[4];
    while(tmp)
    {
        snprintf(result, 4, "%s", tmp);
        tmp = strtok(NULL,".");
    }

    snprintf( szPidFile, sizeof(szPidFile), "%s/pid/%s_%s.pid", CWorkDir::string(), m_szAppName, result);
	
    if ( KillPrevious(szPidFile) < 0 ) 
    {
        LOGERR("Can not kill previous process. exit!");
        exit(0);
    }
    WritePid(szPidFile);
}


bool CAppFrame::_ProcessArgs( )
{
    struct option long_options[] = 
    {  
        { "work-dir", required_argument, 0, 'w'},
        { "daemon", no_argument, 0, 'd'},
        { "sample", no_argument, 0, 's' },
        { "config-xml", optional_argument, 0, 'x'},
        { 0, 0, 0, 0},  
    };

    if( m_argc > 1 && !strcasecmp(m_argv[1], "stop") )
    {
        StopProcess( m_argv[0] );
        exit( 0 );
    }

    int iOpt = 0;
    char* pszWorkDir = NULL;
    char* pszConfXml = NULL;
    
    while( ( iOpt = getopt_long( m_argc, m_argv, "", long_options, NULL ) ) != -1 )
    {
        switch( iOpt )
        {
            case 'd':
            {
                m_iDaemon = 1; 
                break;
            }
            case 'w':
            {
                pszWorkDir = optarg;
                break;
            }
            case 's':
            {
                //Cpu消耗统计开关设置
                CpuSampleStats::Instance().OpenCpuSample();
                break;
            }
            case 'x':
            {
                pszConfXml = optarg;
                // 获得XML配置文件
                if ( !CConfXml::InitConfXml( pszConfXml ) )
                {
                    printf( "invalid xml file\n" );
                    return false;
                }
                break;
            }
            default:
                break;
        }
    }

    // 获得work dir
    if( !CWorkDir::InitWorkDir( pszWorkDir ) )
    {
        printf( "invalid work dir\n" );
        return false;
    }

    return true;
}


