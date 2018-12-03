#ifndef _OG_INI_H
#define _OG_INI_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#ifndef WIN32
#include <netinet/in.h>
#endif

#include "oi_str.h"
#include "oi_cfg.h"

#define MAX_CFG_ID_LEN	32
#define MAX_CFG_SECTION_MEMBER	100

typedef struct {
	char sID[MAX_CFG_ID_LEN];
	int iType;
	int iOffset;
	char sInitVal[256];
	int iArrayMax;
	char cSeparator;
} TypeDesc;

typedef struct {
	char sID[MAX_CFG_ID_LEN];
	int iType;
	int iOffset;
	TypeDesc stMember[MAX_CFG_SECTION_MEMBER];
	int iArrayNumOffset;
	int iArrayMemberSize;
	int iArrayMax;
} SectionDesc;

#ifdef __cplusplus
extern "C"
{
#endif

int CfgSetVal(void *pMember, TypeDesc *pstDesc, char *sVal);
int InitMember(void *pstSec, TypeDesc *pstDesc);
int CfgGetLine(FILE *f, char *sBuf, char *sSecID);
int ReadMember(FILE *f, void *pstSec, TypeDesc *pstTypeDesc, int iTotal, char *sNext);
int ReadCfg(char *sFile, void *pstCfg, TypeDesc *pstTypeDesc);
int InitSection(void *pstIni, SectionDesc *pstDesc);
int SkipSection(FILE *f, char *sSecID);
char *GetSectionID(FILE *f, char *sSecID, char *sNext);
int ReadSection(FILE *f, void *pstIni, SectionDesc *pstDesc, char *sSecID, char *sNext);
int ReadIni(char *sFile, void *pstIni, SectionDesc *pstSecDesc);

#ifdef __cplusplus
}
#endif

#endif
