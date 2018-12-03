#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>

#include "og_ini.h"
#include "oi_str.h"

int CfgSetVal(void *pMember, TypeDesc *pstDesc, char *sVal)
{
char *p, *p1;
int i, iArrayNum = 0;

	// if (!sVal[0]) return 0;
	if ( NULL != (p = strchr(sVal, '\n')) ) *p = 0;
	switch (pstDesc->iType) {
		case CFG_INT:
			*(int *)pMember = atoi(sVal);
			break;
		case CFG_STRING:
			nGetVal((char *)pMember, sVal, 256);
			break;
		case CFG_LINE:
			strcpy(pMember, sVal);
			break;
		case CFG_IP:
			inet_aton(sVal, (struct in_addr *)pMember);
			break;
		case CFG_ADDR:
			p = strchr(sVal, ':');
			if (!p) break;
			*p = 0;
			if (!inet_aton(sVal, &((struct sockaddr_in *)pMember)->sin_addr)) break;
			((struct sockaddr_in *)pMember)->sin_port = htons(atoi(p + 1));
			break;
		case CFG_INT_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				if ( NULL != (p = strchr(sVal, pstDesc->cSeparator)) ) {
					*p = 0;
					*(int *)((char *)pMember + sizeof(int) * iArrayNum) = atoi(sVal);
					sVal = p + 1;
					iArrayNum++;
				} else {
					if (*sVal == 0) break;
					*(int *)((char *)pMember + sizeof(int) * iArrayNum) = atoi(sVal);
					iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				*(int *)((char *)pMember + sizeof(int) * i) = 0;
			break;
		case CFG_IP_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( NULL != (p = strchr(sVal, pstDesc->cSeparator)) ) {
					*p = 0;
					if (inet_aton(sVal, (struct in_addr *)((char *)pMember + sizeof(long) * iArrayNum))) iArrayNum++;
					sVal = p + 1;
				} else {
					if (*sVal == 0) break;
					if (inet_aton(sVal, (struct in_addr *)((char *)pMember + sizeof(long) * iArrayNum))) iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				*(long *)((char *)pMember + sizeof(int) * i) = 0;
			break;
		case CFG_ADDR_ARRAY:
			while (1) {
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( NULL != (p = strchr(sVal, pstDesc->cSeparator)) ) {
					*p = 0;
					if ( NULL != (p1 = strchr(sVal, ':')) ) {
						*p1 = 0;
						if (inet_aton(sVal, &((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_addr)) {
							((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_port = htons(atoi(p1 + 1));
							iArrayNum++;
						}
					}
					sVal = p + 1;
				} else {
					if (*sVal == 0) break;
					if ( NULL != (p1 = strchr(sVal, ':')) ) {
						*p1 = 0;
						if (inet_aton(sVal, &((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_addr)) {
							((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * iArrayNum))->sin_port = htons(atoi(p1 + 1));
							iArrayNum++;
						}
					}
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) 
				((struct sockaddr_in *)((char *)pMember + sizeof(struct sockaddr_in) * i))->sin_addr.s_addr = 0;
			break;

		case CFG_STRING_ARRAY:
			while (1){
				if (iArrayNum >= pstDesc->iArrayMax) break;
				sVal = SkipSpace(sVal);
				if ( NULL != (p = strchr(sVal, pstDesc->cSeparator)) ) {
					*p = 0;
					nGetVal((char *)pMember + LENGTH_CFG_STRING * iArrayNum, sVal, LENGTH_CFG_STRING);
					sVal = p + 1;
					iArrayNum++;
				} else {
					if (*sVal == 0) break;
					nGetVal((char *)pMember + LENGTH_CFG_STRING * iArrayNum, sVal, LENGTH_CFG_STRING);
					iArrayNum++;
					break;
				}
			}
			for (i = iArrayNum; i < pstDesc->iArrayMax; i++) {
				*((char *)pMember + LENGTH_CFG_STRING * i) = 0;
			}
			break;
	}
	return 0;
}

int InitMember(void *pstSec, TypeDesc *pstDesc)
{
int iTotal = 0;

	while (pstDesc->sID[0]) {
		CfgSetVal((char *)pstSec + pstDesc->iOffset, pstDesc, pstDesc->sInitVal);
		// printf("init %s: %s\n", pstDesc->sID, pstDesc->sInitVal);
		iTotal++;
		pstDesc++;
	}
	return iTotal;
}

int CfgGetLine(FILE *f, char *sBuf, char *sSecID)
{
static char sLine[1024];
char *p, *pEnd;

	if (!fgets(sLine, sizeof(sLine), f)) {
		sBuf[0] = 0;
		return -1;
	}
	p = sLine;
	p = SkipSpace(p);
	strcpy(sBuf, p);
	if (!sSecID) return 0;
	if (*p != '[') return 0;
	pEnd = strchr(p, ']');
	if (pEnd) *pEnd = 0;
	strncpy(sSecID, p + 1, MAX_CFG_ID_LEN);
	return 0;
}

int ReadMember(FILE *f, void *pstSec, TypeDesc *pstTypeDesc, int iTotal, char *sNext)
{
int i;
char *p, *pEq, sBuf[1024];
TypeDesc *pstDesc;

	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sNext) < 0) break;
		p = sBuf;
		if (*p == 0 || *p == ';' || *p == '#') continue;
		else if (*p == '[') break;
		pEq = strchr(p, '=');
		if (!pEq) continue;
		*pEq = 0;
		TrimSpace(p);
		pstDesc = pstTypeDesc;
		for (i = 0; i < iTotal; i++) {
		  if (!pstDesc->sID[0] || !strncasecmp(p, pstDesc->sID, strlen(p))) break;
		  pstDesc++;
		}
		if (i >= iTotal || !pstDesc->sID[0]) continue;
		p = pEq + 1;
		p = SkipSpace(p);
		TrimSpace(p);
		CfgSetVal((char *)pstSec + pstDesc->iOffset, pstDesc, p);
	}
	return 0;
}

int ReadCfg(char *sFile, void *pstCfg, TypeDesc *pstTypeDesc)
{
FILE *f;
int iTotal;

	iTotal = InitMember(pstCfg, pstTypeDesc);

	if (!(f = fopen(sFile, "r"))) {
		printf("Can not open config file %s.\n", sFile);
		return -1;
	}
	ReadMember(f, pstCfg, pstTypeDesc, iTotal, NULL);
	fclose(f);
	return 0;
}

int InitSection(void *pstIni, SectionDesc *pstDesc)
{
int i;

	while (pstDesc->sID[0]) {
		switch (pstDesc->iType) {
			case CFG_SECTION:
				InitMember((char *)pstIni + pstDesc->iOffset, pstDesc->stMember);
				break;
			case CFG_SECTION_ARRAY:
				*(int *)((char *)pstIni + pstDesc->iArrayNumOffset) = 0;
				for (i = 0; i < pstDesc->iArrayMax; i++) 
					InitMember((char *)pstIni + pstDesc->iOffset + pstDesc->iArrayMemberSize * i, pstDesc->stMember);
				break;
		}
		pstDesc++;
	}
	return 0;
}

int SkipSection(FILE *f, char *sSecID)
{
char sBuf[1024];

	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sSecID)) break;
		if (sBuf[0] == '[') return 0;
	}
	return 0;
}

char *GetSectionID(FILE *f, char *sSecID, char *sNext)
{
// long lFilePos;
// char *p, *pEnd, sBuf[1024];
  char sBuf[1024];

	if (sNext[0]) {
		strncpy(sSecID, sNext, MAX_CFG_ID_LEN);
		sNext[0] = 0;
		return sSecID;
	}

	sSecID[0] = 0;
	while (!feof(f)) {
		if (CfgGetLine(f, sBuf, sSecID) < 0) break;
		if (sSecID[0]) break;
	}
	if (feof(f)) return NULL;
	if (sSecID[0]) return sSecID;
	return NULL;
}

int ReadSection(FILE *f, void *pstIni, SectionDesc *pstDesc, char *sSecID, char *sNext)
{
int *piArrayNum;

	if (!GetSectionID(f, sSecID, sNext)) return -1;

	while (pstDesc->sID[0]) {
		if (!strncasecmp(sSecID, pstDesc->sID, strlen(pstDesc->sID))) break;
		pstDesc++;
	}
	if (!pstDesc->sID[0]) {
		SkipSection(f, sNext);
		return -2;
	}

	switch (pstDesc->iType) {
		case CFG_SECTION:
			ReadMember(f, (char *)pstIni + pstDesc->iOffset, pstDesc->stMember, MAX_CFG_SECTION_MEMBER, sNext);
			break;
		case CFG_SECTION_ARRAY:
			piArrayNum = (int *)((char *)pstIni + pstDesc->iArrayNumOffset);
			if (*piArrayNum >= pstDesc->iArrayMax) {
				SkipSection(f, sSecID);
				break;
			}
			ReadMember(f, (char *)pstIni + pstDesc->iOffset + pstDesc->iArrayMemberSize * (*piArrayNum), pstDesc->stMember, MAX_CFG_SECTION_MEMBER, sNext);
			// printf("size %d num: %d\n", pstDesc->iArrayMemberSize, *piArrayNum);
			(*piArrayNum)++;
			break;
	}
	return 0;
}

// 读有section的配置文件
int ReadIni(char *sFile, void *pstIni, SectionDesc *pstSecDesc)
{
    FILE *f;
    char sID[MAX_CFG_ID_LEN], sNext[MAX_CFG_ID_LEN];
    //int iRet;

    InitSection(pstIni, pstSecDesc);
    if (!(f = fopen(sFile, "r"))) {
        printf("Can not open INI file %s.\n", sFile);
        return -1;
    }
    sNext[0] = 0;
    while (!feof(f)) {
        //iRet = 
        ReadSection(f, pstIni, pstSecDesc, sID, sNext);
        // printf("get sec: %s  %d\n", sID, iRet);
    }
    fclose(f);
    return 0;
}

