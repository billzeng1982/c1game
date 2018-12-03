#ifndef _TDR_2_PHP_ENTRY_SET_H_
#define _TDR_2_PHP_ENTRY_SET_H_

#include "Tdr2PhpElem.h"
#include <list>

class CTdr2Php;

struct STdr2PhpEntry
{
	STdrPhpTypeInfo* pstTypeInfo;
	const char* pszName;
	const char* pszType;
	const char* pszUnionId; // union����ʹ��
	const char* pszDesc;
	std::string select; // union entryʹ��

	bool IsArray;
	const char* pszArrSize; // �����С
	const char* pszRefer;  	// ����refer

	STdr2PhpEntry()
	{
		pstTypeInfo = NULL;
		pszName 	= NULL;
		pszType 	= NULL;
		pszUnionId 	= NULL;
		pszDesc 	= NULL;
		IsArray 	= false;
		pszArrSize  = NULL;
		pszRefer	= NULL;
	}
};


class CTdr2PhpEntrySet
{
public:
	CTdr2PhpEntrySet( CTdr2Php* poTdr2Php );
	~CTdr2PhpEntrySet(){}

	void Build( XMLElement* poXmlElem );

public:
	CTdr2Php* poTdr2Php;
	std::list<STdr2PhpEntry*> allEntryList;
	// union��struct��������
	std::list<STdr2PhpEntry*> compositeEntryList;
};

#endif

