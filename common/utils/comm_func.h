#ifndef _COMM_FUNC_H_
#define _COMM_FUNC_H_

#include <unistd.h>
#include "define.h" 

// sleep wrapper
#define SLEEP(X) do{ usleep(X*1000); }while(0)

// safe delete
#define SAFE_DEL(X) do{ if(NULL!=X){ delete X; X=NULL; } }while(0)
#define SAFE_DEL_ARRAY(X) do{ if(NULL!=X){ delete [] X; X=NULL; } }while(0)

int StartDaemonProcess( const char* pszRoot = NULL );
int IsProcessSingle( const char* pszProcessName );
int StopProcess( const char* pszProcessName );
char* MakeIPStr( int iIPAddr );
int Str2Date( char *szDate );
int SetRLimit( int nfds );

// �޸�uint32��ֵ����, ����ʵ�ʸı���
int ChgValue_uint32( uint32_t& dwTotal, int iChg, uint32_t dwMax=UINT32_MAX );

// �޸�uint16��ֵ����, ����ʵ�ʸı���
int ChgValue_uint16( uint16_t& wTotal, int iChg, uint16_t wMax=UINT16_MAX );

// �ҳ����ڵ���input����С�� 2^n
int ceilingPowerOfTwo(int input);

int bsr_int32( uint32_t input );
int bsr_int64( uint64_t input );

#endif

