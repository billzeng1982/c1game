#ifndef _OI_STR_H_2005_06_07_
#define _OI_STR_H_2005_06_07_

#ifdef __cplusplus
extern "C" {
#endif

char* get_val(char* desc, char* src);
char* get_val2(char* desc, char* src);
char* get_unit(char* desc, char* src);
char* getunit(char* desc, char* src);
char* nGetUnit(char* sDesc, char* sSrc, int iLen);
char* nGetVal(char* sDesc, char* sSrc, int iLen);
char *encode(char *buffer,char *toencode);
char *randstr(char* buffer, int len);
char* MyQuote(char* dest, char* src);
void HexShow(unsigned char* sStr, int iLen, int iFlag);
char* HexToBin(char* sHex, char* sBuf, int* piLen);
char *HexString(unsigned char* sStr, int iLen);
char* MemSrch(char* sMem, int iMemLen, char* sCmp, int iLen);
char* MemSrchHex(char* sMem, int iMemLen, char* sHex);
char* SkipSpace(register char* sStr);
char* TrimSpace(register char* sStr);

unsigned int StrHash(char* sStr, unsigned int iModule);
char StrHashByte(char* sStr, unsigned char cModule);
unsigned int StrHashLong(char* sStr, unsigned int iModule);

#ifdef __cplusplus
}
#endif

#endif
