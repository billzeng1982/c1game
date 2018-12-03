#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>     
#include "og_comm.h"
#include "oi_misc.h"
#include "tdr/tdr_net.h"
#include <iconv.h>

unsigned short HeadCheckSum(void* sHead)
{
unsigned short shSum = 0;
int i;

    for (i = 0; i < 8; i++) shSum ^= *(short *)((char *)sHead + i * 2);
    return shSum;
}

int AddString(char* p, char* sStr)
{
    *(int *)p = htonl(strlen(sStr) + 1);
    strcpy(p + 4, sStr);
    return strlen(sStr) + 5;
}

int AddString2(char* p, char* sStr)
{
    strcpy(p, sStr);
    return strlen(sStr) + 1;
}

char* GetString(char* p, char* sStr, int iStrLen, int *piGetLen, int iDataLen)
{
int iLen;
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += 4;
    if (*piGetLen > iDataLen) return NULL;
    iLen = ntohl(*(int *)p1);
    *piGetLen += iLen;
    if (*piGetLen > iDataLen) return NULL;
    if (iLen > iStrLen) return NULL;
    strncpy(sStr, p1 + 4, iLen);
    return sStr;
}

char* AddStr(char* p, int *piLen, char *s)
{
    strcpy(p + *piLen, s);
    *piLen += strlen(s);
    return p;
}

char* AddStr2(char* p, int *piLen, ...)
{
va_list ap;
char *s;

    va_start(ap, piLen);
    s = va_arg(ap, char *);
    while (s) {
        strcpy(p + *piLen, s);
        *piLen += strlen(s);
        s = va_arg(ap, char *);
    }
    va_end(ap);
    return p;
}

int AddChar(char* p, int *piLen, char c)
{
    *(p + *piLen) = c;
    (*piLen)++;
    return 1;
}

char GetChar(char* p, int *piGetLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    if (++(*piGetLen) > iDataLen) return 0;
    return *p1;
}

int AddShort(char* p, int *piLen, short sh)
{
    *(short *)(p + *piLen) = htons(sh);
    *piLen += sizeof(short);
    return sizeof(short);
}

int AddShort2(char* p, int *piLen, short sh)
{
    *(short *)(p + *piLen) = sh;
    *piLen += sizeof(short);
    return sizeof(short);
}

short GetShort(char* p, int *piGetLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += sizeof(short);
    if (*piGetLen > iDataLen) return 0;
    return ntohs(*(short *)p1);
}

short GetShort2(char* p, int *piGetLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += sizeof(short);
    if (*piGetLen > iDataLen) return 0;
    return *(short *)p1;
}

int AddInt(char* p, int *piLen, int i)
{
    *(int *)(p + *piLen) = htonl(i);
    *piLen += sizeof(int);
    return sizeof(int);
}

int AddInt2(char* p, int *piLen, int i)
{
    *(int *)(p + *piLen) = i;
    *piLen += sizeof(int);
    return sizeof(int);
}

int GetInt(char* p, int *piGetLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += sizeof(int);
    if (*piGetLen > iDataLen) return 0;
    return ntohl(*(int *)p1);
}

int GetInt2(char* p, int *piGetLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += sizeof(int);
    if (*piGetLen > iDataLen) return 0;
    return *(int *)p1;
}

int AddInt64( char* p, int *piLen, int64_t i64 )
{
	*(int64_t *)(p + *piLen) = tdr_hton64(i64);
    *piLen += sizeof(int64_t);
    return sizeof(int64_t);
}

int64_t GetInt64( char* p, int *piGetLen, int iDataLen )
{
	char *p1;

    p1 = p + *piGetLen;
    *piGetLen += sizeof(int64_t);
    if (*piGetLen > iDataLen) return 0;
    return tdr_ntoh64( *(int64_t *)p1 );
}

int AddBin(char* p, int *piLen, char *sSrc, int iSrcLen)
{
    memcpy(p + *piLen, sSrc, iSrcLen);
    *piLen += iSrcLen;
    return iSrcLen;
}

char* GetBin(char* p, int *piGetLen, char* sDst, int iLen, int iDataLen)
{
char *p1;

    p1 = p + *piGetLen;
    *piGetLen += iLen;
    if (*piGetLen > iDataLen) return 0;
    memcpy(sDst, p1, iLen);
    return sDst;
}

unsigned long MsPass( struct timeval* pstTv1, struct timeval* pstTv2 )
{
    if ( TvBefore(pstTv1,pstTv2 ) > 0)
        return  MsPass(pstTv2,pstTv1);

    long lSecPass = pstTv1->tv_sec - pstTv2->tv_sec;
    long lMSecPass = ((long)pstTv1->tv_usec - (long)pstTv2->tv_usec) / 1000;

    return  lSecPass * 1000 + lMSecPass;
}

unsigned long long UsPass(struct timeval* pstTv1, struct timeval * pstTv2)
{
    long long llPass = ((long)pstTv1->tv_sec - (long)pstTv2->tv_sec) * 1000000;
    llPass += (long)pstTv1->tv_usec - (long)pstTv2->tv_usec;
    if (llPass < 0)
    {
        return UsPass(pstTv2, pstTv1);
    }
    else
    {
        return (unsigned long long)llPass;
    }
}

struct timeval* TvAddMs(struct timeval* pstTv, int iMs)
{
    pstTv->tv_sec += iMs / 1000;
    pstTv->tv_usec += (iMs % 1000) * 1000;
    if (pstTv->tv_usec >= 1000000) {
        pstTv->tv_usec -= 1000000;
        pstTv->tv_sec ++;
    }
    return pstTv;
}

struct timeval* TvDelMs(struct timeval* pstTv, int iMs)
{
    pstTv->tv_sec -= iMs / 1000;
    pstTv->tv_usec -= (iMs % 1000) * 1000;
    if (pstTv->tv_usec < 0) {
        pstTv->tv_usec += 1000000;
        pstTv->tv_sec --;
    }
    return pstTv;
}

int TvBefore(const struct timeval* pstTv1, const struct timeval* pstTv2)
{
    if (pstTv1->tv_sec < pstTv2->tv_sec) return 1;
    if (pstTv1->tv_sec > pstTv2->tv_sec) return 0;
    if (pstTv1->tv_usec < pstTv2->tv_usec) return 1;
    return 0;
}

unsigned long long Timeval2Us(const struct timeval* ptvTime)
{
    return ptvTime->tv_sec*1000000 + ptvTime->tv_usec;
}

unsigned long long Timeval2Ms( const struct timeval* ptvTime )
{
	return ptvTime->tv_sec * 1000 + (ptvTime->tv_usec / 1000);
}

int CmpTimeVal(const struct timeval* pTime1, const struct timeval* pTime2)
{
    if(pTime1->tv_sec > pTime2->tv_sec)
    {
        return 1;
    }
    else if(pTime1->tv_sec < pTime2->tv_sec)
    {
        return -1;
    }
    else if(pTime1->tv_usec > pTime2->tv_usec)
    {
        return 1;
    }
    else if(pTime1->tv_usec < pTime2->tv_usec)
    {
        return -1;
    }

    return 0;
}


int IntCompare(const void *p1, const void *p2)
{
register int *pI1, *pI2;

    pI1 = (int *)p1;
    pI2 = (int *)p2;
    return *pI1 - *pI2;
}

// 默认pid文件为/temp/procid.pid
int KillPre(int iProcID) 
{
char sFile[80];

    sprintf(sFile, "/tmp/%s.pid", inet_ntoa(*(struct in_addr *)(&iProcID)));
    if (KillPrevious(sFile) < 0) {
        printf("Can not kill previous process. exit\n");
        exit(0);
    }
    WritePid(sFile);
    return 0;
}

// 判断两个时间是否是同一天 1: yes, 0: no
int IsSameDay( time_t time1, time_t time2 )
{
	struct tm stTm1;
    struct tm stTm2;

    localtime_r(&time1, &stTm1);
    localtime_r(&time2, &stTm2);

    if(stTm1.tm_year == stTm2.tm_year &&
    stTm1.tm_yday == stTm2.tm_yday)
    {
    	// 同一天
        return 1;
    }
	
    return 0;
}

__inline__ unsigned long long GetTicks(void)
{
	unsigned a, d;     
	asm("cpuid");     
	asm volatile("rdtsc" : "=a" (a), "=d" (d));     
	return (((unsigned long long)a) | (((unsigned long long)d) << 32));
}

int code_convert(const char *from_charset, const char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
{
	iconv_t cd = 0;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0)
	{
		return -1;
	}

	memset(outbuf,0,outlen);

	int iRet = iconv(cd,pin,(size_t*)&inlen,pout,(size_t*)&outlen);
	if (-1==iRet) 
	{
		iconv_close(cd);
		return -1;	
	}
	
	iconv_close(cd);
	return 0;
}

int _GB2312ToUTF8(char *inbuf, int inlen, char *outbuf, int outlen)
{
	return code_convert("gbk","utf-8", inbuf, inlen, outbuf, outlen);
}

