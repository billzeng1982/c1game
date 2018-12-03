
#ifndef _OG_COMM_H_
#define _OG_COMM_H_

#include "pal/ttypes.h"

#define INIT_CONN_ENCRYPT_KEY  "bQr$dZnXP_St2ds'"       //  即通发布的初始密钥

#define ZONE_LISTEN_PORT 6299
#define ZONE_CONNECT_TIMEOUT 25
#define ZONE_CONNECT_MAX_KICK_TIME 3

#define OBJECT_NAME_LEN 50

typedef struct {
	unsigned char cWorld;
	unsigned char cZone;
	unsigned char cModule;
	unsigned char cInstance;
} ProcIDChar;

#ifdef __cplusplus
extern "C"
{
#endif

int IntCompare(const void *p1, const void *p2);
int KillPre(int iProcID);
unsigned short HeadCheckSum(void* sHead);

unsigned long MsPass(struct timeval * pstTv1, struct timeval * pstTv2);
unsigned long long UsPass(struct timeval * pstTv1, struct timeval * pstTv2);
struct timeval* TvAddMs(struct timeval* pstTv, int iMs);
struct timeval* TvDelMs(struct timeval* pstTv, int iMs);
int TvBefore(const struct timeval* pstTv1, const struct timeval* pstTv2);
unsigned long long Timeval2Us(const struct timeval* ptvTime);
unsigned long long Timeval2Ms( const struct timeval* ptvTime );
int CmpTimeVal(const struct timeval* pTime1, const struct timeval* pTime2);

int AddString(char* p, char* sStr);
int AddString2(char* p, char* sStr);
char* GetString(char* p, char* sStr, int iStrLen, int *piGetLen, int iDataLen);
char* AddStr(char* p, int *piLen, char *s);
char* AddStr2(char* p, int *piLen, ...);
int AddChar(char* p, int *piLen, char c);
char GetChar(char* p, int *piGetLen, int iDataLen);
int AddShort(char* p, int *piLen, short sh);
int AddShort2(char* p, int *piLen, short sh);
short GetShort(char* p, int *piGetLen, int iDataLen);
short GetShort2(char* p, int *piGetLen, int iDataLen);
int AddInt(char* p, int *piLen, int i);
int AddInt2(char* p, int *piLen, int i);
int GetInt(char* p, int *piGetLen, int iDataLen);
int GetInt2(char* p, int *piGetLen, int iDataLen);

int AddInt64( char* p, int *piLen, int64_t i64 );
int64_t GetInt64( char* p, int *piGetLen, int iDataLen );

int AddBin(char* p, int *piLen, char *sSrc, int iSrcLen);
char* GetBin(char* p, int *piGetLen, char* sDst, int iLen, int iDataLen);

unsigned long long GetTicks(void);

int code_convert(const char *from_charset, const char *to_charset, char *inbuf,int inlen,char *outbuf,int outlen);
int _GB2312ToUTF8(char *inbuf, int inlen, char *outbuf, int outlen);

#ifdef __cplusplus
}
#endif

#endif
