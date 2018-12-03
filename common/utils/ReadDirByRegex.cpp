#include <assert.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include "ReadDirByRegex.h"

// sg - static global, gobal only used within this cpp file
static DIR *sg_pstDir = NULL;
static regex_t sg_reg;

bool ReadDirByRegexInit( const char* pszDir, const char* pszFileRegex )
{
    if( NULL == pszDir || NULL == pszFileRegex )
    {
        assert( false );
        return false;
    }

    bzero( &sg_reg, sizeof(sg_reg) );
    
    char szErrBuf[256];
    int iRet = regcomp( &sg_reg, pszFileRegex, REG_EXTENDED );
    if( iRet != 0 )
    {
        regerror( iRet, &sg_reg, szErrBuf, sizeof(szErrBuf) );
        printf("Init file regex failed! %s: pattern '%s'",
            szErrBuf, pszFileRegex );
        return false;
    }
    
    sg_pstDir = opendir( pszDir );
    if (!sg_pstDir)
    {
        printf("Dir <%s> open failed: %s", pszDir, strerror(errno));
        regfree( &sg_reg );
        return false;
    }

    return true;
}

char* GetNextFileNameByRegex( )
{
    if( NULL == sg_pstDir )
    {
        assert(false);
        return NULL;
    }

    struct dirent *pstDirEnt = NULL;
    regmatch_t arrMatch[1];
    int iRet = 0;

    while ( (pstDirEnt = readdir(sg_pstDir)) )
    {
        iRet = regexec( &sg_reg, pstDirEnt->d_name, 1, arrMatch, 0 );
        if( 0 == iRet && 0 == arrMatch[0].rm_so )
        {
            // 匹配文件模式
            return pstDirEnt->d_name;
        }else
        {
            // 不匹配继续找
            continue;
        }
    }

    return NULL;
}

void ReadDirByRegexEnd( )
{
    if( sg_pstDir )
    {
        closedir(sg_pstDir);
    }
    regfree(&sg_reg);
}

