#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "oi_str.h"
#include "oi_iparea.h"

#define HZ_GB 0
#define HZ_BIG5 1
#define HZ_JPN 2
#define HZ_GBK 3
#define HZ_TGB 4
#define HZ_TSIGN 5
#define HZ_TJPN 6

#define BOOL int


BOOL	ISDBCSLEAD(int CodeType,unsigned char c)
{
	switch(CodeType){
		case HZ_GB:
			return(c>0xa0 && c<0xff);
		case HZ_BIG5:
			return(c>0xa0 && c<0xff);
		case HZ_JPN:
			return((c>0x80 && c<0xa0)||(c>0xdf && c<0xfd));
		case HZ_GBK:
			return(c>0x80 && c<0xff);
		case HZ_TGB:
			return(c>0xaf && c<0xf8);
		case HZ_TSIGN:
			return((c == 0xa1)||(c==0xa3));
		case HZ_TJPN:
			return((c == 0xa4)||(c==0xa5));
	}
	return	0;
}

BOOL	ISDBCSNEXT(int CodeType,unsigned char c)
{
	switch(CodeType){
		case HZ_GB:
			return(c>0xa0 && c<0xff);
		case HZ_BIG5:
			return((c>0x3f && c<0x7f)||(c>0xa0 && c<0xff));
		case HZ_JPN:
			return(c>0x3f && c<0xfd);
		case HZ_GBK:
			return((c>0x3f && c<0x7f)||(c>0x7f && c<0xff));
		case HZ_TGB:
			return(c>0xa0 && c<0xff);
		case HZ_TSIGN:
			return(c>0xa0 && c<0xff);
		case HZ_TJPN:
			return(c>0xa0 && c<0xf4);
	}
	return	0;
}

int is_bad_string(char *buf)
{
	int i;
    int j = 0; //t = 0; u = 0;
//	char cflt[256];
//	char eflt[256];
// 	char *p , *q ,*r;
// 	
// 	p = buf;
// 	q = buf;
// 	r = buf;

	i = strlen(buf);
	if (i > 255)
		i = 255;
	for( j = 0 ; j < i; j++)
	{
		if ( isprint(*(buf + j)) )
		{
			//eflt[u++] = (char) tolower((int)*(buf + j));;
			continue;
		}
		if (ISDBCSLEAD(HZ_TGB,*(buf + j)))
		{
			if ( ISDBCSNEXT( HZ_TGB,*(buf + j + 1)) )
			{
// 				cflt[t++] = *(buf + j);
// 				cflt[t++] = *(buf + j + 1);
				j++;
				continue;
			}
		}
		if (ISDBCSLEAD(HZ_TSIGN,*(buf + j)))
		{
			if ( ISDBCSNEXT( HZ_TSIGN,*(buf + j + 1)) )
			{
// 				cflt[t++] = *(buf + j);
// 				cflt[t++] = *(buf + j + 1);
				j++;
				continue;
			}
		}

		if (ISDBCSLEAD(HZ_TJPN,*(buf + j)))
		{
			if ( ISDBCSNEXT( HZ_TJPN,*(buf + j + 1)) )
			{
// 				cflt[t++] = *(buf + j);
// 				cflt[t++] = *(buf + j + 1);
				j++;
				continue;
			}
		}
		return 1; /* 不可识别字符 */
	}
//	cflt[t] = 0;
	//eflt[u] = 0;
// 	p = cflt;
// 	q = eflt;
 	
	return 0;
}

int is_simple_str(char *sBuf)
{

	int	digits = 0;
	int	uppers = 0;
	int	lowers = 0;
	int	others = 0;
	int	i;

	if (strlen(sBuf) < 7) return 1;	
	
	for (i = 0;i < strlen(sBuf);i++) {
		if (isdigit(sBuf[i]))
			digits++;
		else if (isupper(sBuf[i]))
			uppers++;
		else if (islower(sBuf[i]))
			lowers++;
		else
			others++;
	}
	
	if (strlen(sBuf) < 10){
		if (digits == strlen(sBuf)){
			return 1;
		}else if (lowers == strlen(sBuf)){
			return 1;
		}else if (uppers == strlen(sBuf)){
			return 1;
		}
	}

	return 0;
}

char* get_val(char* desc, char* src)
{
char *descp=desc, *srcp=src;
int mtime=0, space=0;

	while ( mtime!=2 && *srcp != '\0' ) {
		switch(*srcp) {
			case ' ':
			case '\t':
			case '\0':
			case '\n':
			case US:
				space=1;
				srcp++;
				break;
			default:
				if (space||srcp==src) mtime++;
				space=0;
				if ( mtime==2 ) break;
				*descp=*srcp;
				descp++;
				srcp++;
		}
	}
	*descp='\0';
	strcpy(src, srcp);
	return desc;
}


char* get_val2(char* desc, char* src)
{
char *descp=desc, *srcp=src;
int mtime=0, space=0;

	while ( mtime!=2 && *srcp != '\0' ) {
		switch(*srcp) {
			case ' ':
			case '\t':
			case '\0':
			case '\n':
			case '-':
			case US:
				space=1;
				srcp++;
				break;
			default:
				if (space||srcp==src) mtime++;
				space=0;
				if ( mtime==2 ) break;
				*descp=*srcp;
				descp++;
				srcp++;
		}
	}
	*descp='\0';
	strcpy(src, srcp);
	return desc;
}


char* get_unit(char* desc, char* src)
{
char *descp=desc, *srcp=src;

	if ( *srcp == ETX ) {
		*descp='\0';
		return desc;
	}
	while ( *srcp != ETX && *srcp != US ) {
		if ( *srcp=='\'' ) *descp++='\\';
		*descp++=*srcp++;
	}
	*descp='\0';
	if ( *srcp == ETX ) {
		*src=*srcp;
		return desc;
	}
	do { *src++=*(++srcp); } while ( *srcp && *srcp != ETX ); 
	*src = '\0';
	return desc;
}

char* getunit(char* desc, char* src)
{
char *descp=desc, *srcp=src;

	if (*srcp == ETX) {
		*descp='\0';
		return desc;
	}
	while (*srcp != ETX && *srcp != US) {
		*descp++=*srcp++;
	}
	*descp='\0';
	if (*srcp == ETX) {
		*src=*srcp;
		return desc;
	}
	do {*src++=*(++srcp);} while (*srcp != ETX); 
	*src = '\0';
	return desc;
}

char* nGetVal(char* desc, char* src, int iLen)
{
char *descp=desc, *srcp=src;
int mtime=0, space=0;
int iCurrLen = 0;

	while (mtime!=2 && *srcp != '\0' && iCurrLen < iLen - 1) {
		switch(*srcp) {
			case ' ':
			case '\t':
			case '\0':
			case '\n':
			case US:
				space=1;
				srcp++;
				break;
			default:
				if (space||srcp==src) mtime++;
				space=0;
				if ( mtime==2 ) break;
				*descp=*srcp;
				descp++;
				iCurrLen++;
				srcp++;
		}
	}
	*descp='\0';

	return desc;
}

char* nGetUnit(char* sDesc, char* sSrc, int iLen)
{
char *sDescp=sDesc, *sSrcp=sSrc;
int iCurrLen = 0;

	if (*sSrcp == ETX) {
		*sDescp='\0';
		return sDesc;
	}
	while (*sSrcp != ETX && *sSrcp != US && *sSrcp != RS && iCurrLen++ < iLen) {
		*sDescp++=*sSrcp++;
	}
	*sDescp='\0';
	if (*sSrcp == ETX) {
		*sSrc=*sSrcp;
		return sDesc;
	}
	do {*sSrc++=*(++sSrcp);} while (*sSrcp != ETX); 
	*sSrc = '\0';
	return sDesc;
}

char* nGetUnit2(char* sDesc, char* sSrc, int iLen)
{
char *sDescp=sDesc, *sSrcp=sSrc, cLen;
int iCurrentLen=0;

	if (*sSrcp == ETX) {
		*sDescp='\0';
		return sDesc;
	}
	cLen=*sSrcp;
	while (*sSrcp != ETX && iCurrentLen <= cLen && iCurrentLen <= iLen) {
		*sDescp++=*(++sSrcp);
		++iCurrentLen;
	}
	*sDescp='\0';
	if (*sSrcp == ETX) {
		*sSrc=*sSrcp;
		return sDesc;
	}
	do {*sSrc++=*(++sSrcp);} while (*sSrcp != ETX); 
	*sSrc = '\0';
	return sDesc;
}

char* nFetchUnit(char* sDesc, char* sSrc, int iLen)
{
static char *sSrcp;
char *sDescp=sDesc;
int iCurrLen = 0;

	if (sSrc) sSrcp = sSrc;
	if (*sSrcp == ETX) {
		*sDescp='\0';
		return sDesc;
	}
	while (*sSrcp != ETX && *sSrcp != US && iCurrLen++ < iLen) {
		*sDescp++=*sSrcp++;
	}
	*sDescp='\0';
	return sDesc;
}

char *encode(char *buffer,char *toencode)
{
  int  i;

  buffer[0]='\0';
  for (i=0;i<strlen(toencode);i++) 
	switch(toencode[i]) {
	  case '&':strcat(buffer,"%26");break;
	  case '=':strcat(buffer,"%3D");break;
	  case '%':strcat(buffer,"%25");break;
	  case '+':strcat(buffer,"%2B");break;
	  case '\n':strcat(buffer,"%0A");break;
	  case '\r':strcat(buffer,"%0D");break;
	  case ' ':strcat(buffer,"+");break;
	  default: sprintf(buffer,"%s%c",buffer,toencode[i]);
	}
  return buffer;
}

// buffer的最后一个字节填0
char *randstr(char* buffer, int len)
{
  //char *chars="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
  char *chars="ABCDEFGHIJKMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz23456789";   // 为减少误判， 去掉0、1、O、o、L、l
  int	i,chars_len;

  chars_len=strlen(chars);

  for ( i=0;i<len-1;i++) 
	buffer[i]=chars[ rand()%chars_len];
  buffer[len-1]='\0';
  return buffer;
} 


// buffer的最后一个字节填0
char *randstr2(char* buffer, int len)
{
  char *chars="abcdefghijkmnpqrstuvwxyz23456789";   // 为减少误判， 去掉0、1、O、o、L、l
  int	i,chars_len;

  chars_len=strlen(chars);
  for ( i=0;i<len-1;i++) 
	buffer[i]=chars[ rand()%chars_len ];
  buffer[len-1]='\0';
  return buffer;
}

char *randstr_number(char* buffer, int len)
{
  char *chars="0123456789"; 
  int	i;

  //chars_len=strlen(chars);
  for ( i=0;i<len;i++) 
	buffer[i]=chars[ (int)( (float)strlen(chars)*rand()/(RAND_MAX+1.0) ) ];
  buffer[len]='\0';
  return buffer;
}


char* MyQuote(char* dest, char* src)
{
register int i, j;
//int n;

	i = j = 0;
	while (src[i]) {
		switch((unsigned char)src[i]) {
			case '\n':
				dest[j++] = '\\';
				dest[j++] = 'n';
				break;
			case '\t':
				dest[j++] = '\\';
				dest[j++] = 't';
				break;
			case '\r':
				dest[j++] = '\\';
				dest[j++] = 'r';
				break;
			case '\b':
				dest[j++] = '\\';
				dest[j++] = 'b';
				break;
			case '\'':
				dest[j++] = '\\';
				dest[j++] = '\'';
				break;
			case '\"':
				dest[j++] = '\\';
				dest[j++] = '\"';
				break;
			case '\\':
				dest[j++] = '\\';
				dest[j++] = '\\';
				break;
			default:
				dest[j++] = src[i];
		}
		i++;
	}
	dest[j]=0;
	return dest;
}

char* MyQuoteWild(char* dest, char* src)
{
register int i, j;
// int n;

	i = j = 0;
	while (src[i]) {
		switch((unsigned char)src[i]) {
			case '\n':
				dest[j++] = '\\';
				dest[j++] = 'n';
				break;
			case '\t':
				dest[j++] = '\\';
				dest[j++] = 't';
				break;
			case '\r':
				dest[j++] = '\\';
				dest[j++] = 'r';
				break;
			case '\b':
				dest[j++] = '\\';
				dest[j++] = 'b';
				break;
			case '\'':
				dest[j++] = '\\';
				dest[j++] = '\'';
				break;
			case '\"':
				dest[j++] = '\\';
				dest[j++] = '\"';
				break;
			case '\\':
				dest[j++] = '\\';
				dest[j++] = '\\';
				break;
			case '%':
				dest[j++] = '\\';
				dest[j++] = '%';
				break;
			case '_':
				dest[j++] = '\\';
				dest[j++] = '_';
				break;
			default:
				dest[j++] = src[i];
		}
		i++;
	}
	dest[j]=0;
	return dest;
}

/* Show Hex val of a string, if iFlag = 1, show printable character as char */
void HexShow(unsigned char* sStr, int iLen, int iFlag)
{
register int iCount;

	for (iCount = 0; iCount < iLen; iCount++) {
		if (iCount % 25 == 0) printf("\n");
		if (iFlag && sStr[iCount] > 0x1f) printf("%2c ", sStr[iCount]);
		else printf("%.2x ", sStr[iCount]);
	}
	if ((iCount - 1) % 25) printf("\n");
}

char *HexString(unsigned char* sStr, int iStrLen)
{
int iLen, iCount;
static char sBuffer[20480];
	
	for (iLen = iCount = 0; iCount < iStrLen; iCount++) 
	{
		if (iLen > sizeof(sBuffer) - 5) break;
		if (iCount % 25 == 24) sprintf(sBuffer + iLen, "%.2x\n", sStr[iCount]);
		else sprintf(sBuffer + iLen, "%.2x ", sStr[iCount]);
		iLen += 3;
	}
	return sBuffer;
}

char *HexStringCompact(unsigned char* sStr, int iLen)
{
	int iCount;
	static char sBuffer[2048];
	char sTmp[10];
	
	strcpy(sBuffer, "");
	for (iCount = 0; iCount < iLen; iCount++) 
	{
		if (strlen(sBuffer) > sizeof(sBuffer)) break;
		sprintf(sTmp, "%.2x", sStr[iCount]);
		strcat(sBuffer, sTmp);
	}
	return sBuffer;
}

char* SkipSpace(register char* sStr)
{
	while(*sStr == ' ' || *sStr == '\t' || *sStr == '\r' || *sStr == '\n') sStr++;
	return sStr;
}

char* TrimSpace(register char* sStr)
{
int i;

	i = strlen(sStr) - 1;
	while (sStr[i] == ' ' || sStr[i] == '\t' || sStr[i] == '\r' || sStr[i] == '\n') sStr[i--] = 0;
	return sStr;
}

int IsGsmNo(char* sGsmNo)
{
int iCount, iNoLen;

	iNoLen = strlen(sGsmNo);
	if (iNoLen != 11 && iNoLen != 10) return 0;
	for (iCount = 0; iCount < iNoLen; iCount++)
		if (!isdigit(sGsmNo[iCount])) return 0;
	if (strncmp(sGsmNo, "13", 2) && strncmp(sGsmNo, "852", 3) && strncmp(sGsmNo, "853", 3)) return 0;
	return 1;
}

char* IsEmailAddr(char* sEmail)
{
char *sPtr, *sPtr1;
int iLen;

	if ((iLen = strlen(sEmail)) < 7) return NULL;
	if (!isalpha(sEmail[iLen - 1])) return NULL;
	if (!(sPtr = strchr(sEmail, '@')) || sPtr == sEmail) return NULL;
	if (!(sPtr1 = strchr(sPtr, '.')) || sPtr1 - sPtr < 3) return NULL;
	return sEmail;
}

//added by leafye 2005-04-06, changed from glib
unsigned int StrHash(char* sStr, unsigned int iModule)
{
	const char *p = sStr;
	unsigned int  h = *p;

	if (h)
		for (p += 1; *p != '\0'; p++)
			h = (h << 5) - h + *p;
	h = h % iModule;

	return h;
}

char StrHashByte(char* sStr, unsigned char cModule)
{
unsigned long lNumerator;
int iLen;
unsigned char cRemainder = 0;
register int i;

	iLen = strlen(sStr);
	for (i = 0; i <= iLen - 3; i += 3) {
		cRemainder = ((cRemainder << 24) + (sStr[i] << 16) + (sStr[i + 1] << 8) + sStr[i + 2]) % cModule;
	}
	lNumerator = cRemainder;
	for (i = iLen - i; i > 0; i--) {
		lNumerator = (lNumerator << 8) + sStr[iLen - i];
	}
	return lNumerator % cModule;
}

unsigned short StrHashWord(char* sStr, unsigned short shModule)
{
unsigned long lNumerator;
int iLen;
unsigned short shRemainder = 0;
register int i;

	iLen = strlen(sStr);
	for (i = 0; i <= iLen - 2; i += 2) {
		shRemainder = ((shRemainder << 16) + (sStr[i] << 8) + sStr[i + 1]) % shModule;
	}
	lNumerator = shRemainder;
	for (i = iLen - i; i > 0; i--) {
		lNumerator = (lNumerator << 8) + sStr[iLen - i];
	}
	return lNumerator % shModule;
}

unsigned int StrHashLong(char* sStr, unsigned int iModule)
{
//unsigned long long llNumerator;
    int iLen;
    unsigned long long llRemainder = 0;
    register int i;

	iLen = strlen(sStr);
	for (i = 0; i <= iLen - 4; i += 4) {
		llRemainder = llRemainder << 32;
		llRemainder = (llRemainder + *(int *)(sStr + i)) % iModule;
	}
	//llNumerator = llRemainder;
	for (i = iLen - i; i > 0; i--) {
		llRemainder = llRemainder << 8;
		llRemainder = llRemainder + sStr[iLen - i];
	}
	return llRemainder % iModule;
}

int MinInt(int iVal1, int iVal2)
{
	if (iVal1 > iVal2) return iVal2;
	else return iVal1;
}
 
int MaxInt(int iVal1, int iVal2)
{
	if (iVal1 < iVal2) return iVal2;
	else return iVal1;
}

unsigned char HexToC(char* sHex)
{
unsigned char c;

	c = 0;
	if (sHex[0] >= '0' && sHex[0] <= '9') c = (sHex[0] - '0') * 16;
	else if (sHex[0] >= 'a' && sHex[0] <= 'f') c = (sHex[0] - 'a' + 10) * 16;
	if (sHex[1] >= '0' && sHex[1] <= '9') c += sHex[1] - '0';
	else if (sHex[1] >= 'a' && sHex[1] <= 'f') c += sHex[1] - 'a' + 10;
	return c;
} 

char* HexToBin(char* sHex, char* sBuf, int* piLen)
{
char sTmp[10];
int iTo;

	iTo = 0;
	while (1) {
		nGetVal(sTmp, sHex, sizeof(sTmp));
		if (!strlen(sTmp)) break;
		sBuf[iTo] = HexToC(sTmp);
		iTo++;
	}
	*piLen = iTo;
	return sBuf;
}

char* MemSrch(char* sMem, int iMemLen, char* sCmp, int iLen)
{
// char c, *p;
char *p;
int i, iSrchLen;

	p = sMem;
	iSrchLen = 0;
	while (iSrchLen <= iMemLen - iLen) {
		p = memchr(sMem + iSrchLen, sCmp[0], iMemLen - iSrchLen);
		if (p) {
			for (i = 1; i < iLen; i++) 
				if (*(p + i) != sCmp[i]) break;
			if (i == iLen) return p;
		} else return NULL;
		iSrchLen = p - sMem + 1;
	}
	return NULL;
}

char* MemSrchHex(char* sMem, int iMemLen, char* sHex)
{
char sBin[20], sTmp[60];
int iLen;

	strncpy(sTmp, sHex, sizeof(sTmp));
	HexToBin(sTmp, sBin, &iLen);
	return MemSrch(sMem, iMemLen, sBin, iLen);
}
