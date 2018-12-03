#ifndef _OI_CFG_C_
#define _OI_CFG_C_

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "oi_cfg.h"
#include "oi_str.h"

#define MAX_CONFIG_LINE_LEN 1023

void InitDefault(va_list ap)
{
	char *sParam, *sVal, *sDefault;
	double *pdVal, dDefault;
	long *plVal, lDefault;
	int iType, *piVal, iDefault;
	long lSize;

	sParam = va_arg(ap, char *);
	while (sParam != NULL)
	{
		iType = va_arg(ap, int);
		switch(iType)
		{
			case CFG_LINE:
				sVal = va_arg(ap, char *);
				sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				strncpy(sVal, sDefault, (int)lSize-1);
				sVal[lSize-1] = 0;
				break;
			case CFG_STRING:
				sVal = va_arg(ap, char *);
				sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				strncpy(sVal, sDefault, (int)lSize-1);
				sVal[lSize-1] = 0;
				break;
			case CFG_LONG:
				plVal = va_arg(ap, long *);
				lDefault = va_arg(ap, long);
				*plVal = lDefault;
				break;
			case CFG_INT:
				piVal = va_arg(ap, int *);
				iDefault = va_arg(ap, int);
				*piVal = iDefault;
				break;
			case CFG_DOUBLE:
				pdVal = va_arg(ap, double *);
				dDefault = va_arg(ap, double);
				*pdVal = dDefault;
				break;
		}
		sParam = va_arg(ap, char *);
	}
}

void SetVal(va_list ap, char *sP, char *sV)
{
    char *sParam, *sVal; //*sDefault;
	double *pdVal, dDefault;
    long *plVal; //lDefault;
    int iType, *piVal; //iDefault;
	long lSize;
	char sLine[MAX_CONFIG_LINE_LEN+1], sLine1[MAX_CONFIG_LINE_LEN+1];

	strcpy(sLine, sV);
	strcpy(sLine1, sV);
	get_val(sV, sLine1);
	sParam = va_arg(ap, char *);
	while (sParam != NULL)
	{
		iType = va_arg(ap, int);
		switch(iType)
		{
			case CFG_LINE:
				sVal = va_arg(ap, char *);
				//sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				if (strcmp(sP, sParam) == 0) {
					if (sLine[strlen(sLine) - 1] == '\n') sLine[strlen(sLine) - 1] = 0;
					strncpy(sVal, sLine, (int)lSize-1);
					sVal[lSize-1] = 0;
				}
				break;
			case CFG_STRING:
				sVal = va_arg(ap, char *);
				//sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				break;
			case CFG_LONG:
				plVal = va_arg(ap, long *);
				//lDefault = va_arg(ap, long);
				/*
				if (strcmp(sP, sParam) == 0)
				{
					*plVal = atol(sV);
				}
				*/
				break;
			case CFG_INT:
				piVal = va_arg(ap, int *);
				//iDefault = va_arg(ap, int);
				/*
				if (strcmp(sP, sParam) == 0)
				{
					*piVal = iDefault;
				}
				*/
				break;
			case CFG_DOUBLE:
				pdVal = va_arg(ap, double *);
				dDefault = va_arg(ap, double);
				*pdVal = dDefault;
				break;
		}

		if (strcmp(sP, sParam) == 0)
		{
			switch(iType)
			{
				case CFG_STRING:
					strncpy(sVal, sV, (int)lSize-1);
					sVal[lSize-1] = 0;
					break;
				case CFG_LONG:
					*plVal = atol(sV);
					break;
				case CFG_INT:
					*piVal = atoi(sV);
					break;
				case CFG_DOUBLE:
					*pdVal = atof(sV);
					break;
			}
			return;
		}

		sParam = va_arg(ap, char *);
	}
}

int GetParamVal(char *sLine, char *sParam, char *sVal)
{

	get_val(sParam, sLine);
	strcpy(sVal, sLine);
	
	if (sParam[0] == '#')
		return 1;
		
	return 0;
}

void GetConfig(char *sConfigFilePath, ...)
{
	FILE *pstFile;
	char sLine[MAX_CONFIG_LINE_LEN+1], sParam[MAX_CONFIG_LINE_LEN+1], sVal[MAX_CONFIG_LINE_LEN+1];
	va_list ap;
	
	va_start(ap, sConfigFilePath);
	InitDefault(ap);
	va_end(ap);

	if ((pstFile = fopen(sConfigFilePath, "r")) == NULL)
	{
		// printf("Can't open Config file '%s', ignore.\n", sConfigFilePath);
		return;
	}

	while (1)
	{
		fgets(sLine, sizeof(sLine), pstFile);
		if (feof(pstFile))
		{
			break; 
		}
		
		if (GetParamVal(sLine, sParam, sVal) == 0)
		{
			va_start(ap, sConfigFilePath);
			SetVal(ap, sParam, sVal);
			va_end(ap);
		}
	}	
	fclose(pstFile);
	
}

#endif
