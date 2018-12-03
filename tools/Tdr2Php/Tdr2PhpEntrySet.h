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
	const char* pszUnionId; // union类型使用
	const char* pszDesc;
	std::string select; // union entry使用

	bool IsArray;
	const char* pszArrSize; // 数组大小
	const char* pszRefer;  	// 数组refer

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
	// union和struct单独出来
	std::list<STdr2PhpEntry*> compositeEntryList;
};

#endif

