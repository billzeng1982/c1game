#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>

#include "comm_func.h"


// start daemon process 
int StartDaemonProcess( const char* pszRoot )
{
	int iPid;

	iPid = fork();
	if( 0 != iPid )
	{
		exit( 0 );
	}

	// daemon process start   
	setsid( );

	signal( SIGHUP,  SIG_IGN );

#ifndef _DEBUG
	signal( SIGINT,  SIG_IGN );
#endif

	signal( SIGQUIT, SIG_IGN );
	signal( SIGPIPE, SIG_IGN );
	signal( SIGTTIN, SIG_IGN );
	signal( SIGTTOU, SIG_IGN );
	//signal( SIGTERM, SIG_IGN ); // ��׽,�Զ����˳�
    signal( SIGCHLD, SIG_IGN );

	// change to work dir
	if( pszRoot && 0 != chdir( pszRoot ) )
	{
		fprintf( stderr, 
				 "change to work dir %s failed!\n"
				 "server is shutting down...\n", 
				 pszRoot );

		return -1;
	}

	// close stdio
	fclose( stdin );
	fclose( stdout );
	fclose( stderr );

	return 0;
}


// is process single
int IsProcessSingle( const char* pszProcessName )
{
	char szCmd[128]; 
	char szResult[128];
	FILE *pFile;

	snprintf( szCmd, sizeof(szCmd),
			"ps -ef | grep \"%s\" | " 
	        "grep -v grep | "
	        "wc -l",
	        basename((char*)pszProcessName) );

	pFile = popen( szCmd, "r" );
	if( NULL == pFile )
	{
		return -1;
	}

	fgets( szResult, sizeof(szResult), pFile );
	pclose( pFile );

	if( atoi(szResult) - 1 > 0 )
	{
		return -1;
	}

	return 0;
}

// stop process
int StopProcess( const char* pszProcessName )
{
	char szCmd[128];
	signal( SIGUSR1, SIG_IGN );

	snprintf( szCmd, sizeof(szCmd),
			"killall -SIGUSR1 %s",
			basename((char*)pszProcessName) );

	system( szCmd );

	SLEEP( 2000 );

	if( -1 == IsProcessSingle(pszProcessName) )
	{
		snprintf( szCmd, sizeof(szCmd),
				  "killall -SIGKILL %s",
				  basename((char*)pszProcessName) );

		system( szCmd );
	}

	return 0;
}

//  make ip string, ע�⴫������ip��������
char* MakeIPStr( int iIPAddr )
{
	return inet_ntoa( *(struct in_addr *)(&iIPAddr) );
}


int Str2Date( char *szDate )
{
    char szTemp[20] , sz1[5] ;
    struct tm tmTime = {0};
    time_t lTime ;

    strncpy( szTemp, szDate, sizeof(szTemp)-1 );
    szTemp[19] = 0;
    
    snprintf(sz1, sizeof(sz1), "%s",&szTemp[8]);
    szTemp[7]=0;
    tmTime.tm_mday=atoi(sz1) ;
    
    snprintf(sz1, sizeof(sz1), "%s",&szTemp[5]);
    szTemp[4]=0;
    tmTime.tm_mon=atoi(sz1) -1 ;   

    if( atoi(szTemp) <= 1970 )
    {
        return 1;
    }
    
    tmTime.tm_year=atoi(szTemp)-1900;
    
    tmTime.tm_isdst=0;     //��׼ʱ�䣬������ʱ
    lTime =mktime(&tmTime);	

    return lTime;
}


int SetRLimit( int nfds )
{
    if( nfds <= 0 )
	{
		return -1;
	}
	
    struct rlimit rlim;

    rlim.rlim_cur = rlim.rlim_max = nfds + 100;
    if ( setrlimit(RLIMIT_NOFILE, &rlim) < 0 )
    {
        printf( "setlimit wrong! nfds=%d", nfds );
        perror( "setlimit" );

        return -1;
    }

    getrlimit(RLIMIT_NOFILE, &rlim);
    printf( "Max Open file: %d\n", (int)(rlim.rlim_cur) );

    return rlim.rlim_cur;
}


// �޸�uint32��ֵ����, ����ʵ�ʸı���
int ChgValue_uint32( uint32_t& dwTotal, int iChg , uint32_t dwMax )
{
    int iRealChg = 0;
    uint32_t dwOldTotal = dwTotal;
    
    if( iChg >= 0 )
    {
    	    if( dwTotal >= dwMax )
		{
			return 0;
		}
		
          if( dwMax - dwTotal >= (uint32_t)iChg )
        	{
        		dwTotal += iChg;
    			iRealChg = iChg;
        	}else
		{
			dwTotal = dwMax;
			iRealChg = dwMax - dwOldTotal;
		}
    }
    else  // iChg < 0
    {
        if( dwTotal != 0 )
        {
            if( dwTotal < (uint32_t)(-iChg) )
            {
                dwTotal = 0;
            }else
            {
                dwTotal += iChg;
            }

            iRealChg = -1*( dwOldTotal - dwTotal ); // old��
        }
    }

    return iRealChg;
}


// �޸�uint16��ֵ����, ����ʵ�ʸı���
int ChgValue_uint16( uint16_t& wTotal, int iChg, uint16_t wMax )
{
    int iRealChg = 0;
    uint16_t wOldTotal = wTotal;

    if( iChg >= UINT16_MAX )
    {
        iChg = UINT16_MAX;
    }

    if( iChg < -1*UINT16_MAX )
    {
        iChg = -1*UINT16_MAX;
    }
    
    if( iChg >= 0 )
    {
    	if( wTotal >= wMax )
		{
			return 0;
		}
		
        if( wMax - wTotal >= (uint16_t)iChg )
    	{
    		wTotal += iChg;
			iRealChg = iChg;
    	}else
		{
			wTotal = wMax;
			iRealChg = wMax - wOldTotal;
		}
    }
    else  // iChg < 0
    {
        if( wTotal != 0 )
        {
            if( wTotal < (uint16_t)(-iChg) )
            {
                wTotal = 0;
            }else
            {
                wTotal += iChg;
            }

            iRealChg = -1*( wOldTotal - wTotal ); // old��
        }
    }

    return iRealChg;
}

/* BSR �� Bit Scan Reverse
 * ����input(32λ)���λ(31)��ʼ�ң��ҳ���һ��bitΪ1�ĸ�λ��index
 * ����0x7F00FF00�����ú�������30��0x0F00FF00����27
 * ����-1��ʾ����λ����0
 */
int bsr_int32( uint32_t input )
{
    register int result;

    __asm__("bsrl %1, %0\n\t" //bsr��mov�����l��ָ4�ֽ����ݿ��,
         "jnz 1f\n\t"
         "movl $-1,%0\n\t"
         "1:"
         :"=r"(result):"r"(input));

    return result;
}

int bsr_int64( uint64_t input )
{
    register int64_t result;
    __asm__(
            "bsrq %1, %0\n\t" //bsr��mov�����q��ָ8�ֽ����ݿ��,ÿ�л������β��Ҫ�ӻ��з�\n\t
            "jnz 1f\n\t"      //�Ĵ���ZF��־Ϊ0,%0�н����Чֱ����ת�����1,f��ָ��ǰ��ת
            "movq $-1,%0\n\t" //�Ĵ���ZF��־Ϊ1���������е�λ����0,���Է���-1
            "1:"
            :"=q"(result):"q"(input));
    return (int)result;
}

// �ҳ����ڵ���input����С�� 2^n
int ceilingPowerOfTwo(int input)
{
    // the least possible power-of-two value is 1
    if (input <= 1) return 1;
    int highestBit = bsr_int32(input);
    int mask = input & ((1<< highestBit)-1); // �൱��input��2^highestBit����
    highestBit += ( mask > 0 );
    return (1<<highestBit);
}


