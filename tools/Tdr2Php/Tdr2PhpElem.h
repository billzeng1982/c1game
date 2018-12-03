#ifndef _TDR_2_PHP_ELEM_H_
#define _TDR_2_PHP_ELEM_H_

#include "tinyxml2.h"
#include <string>
#include <list>

using namespace tinyxml2;

enum PHP_TYPE
{
	PHP_TYPE_STRING  = 1,
	PHP_TYPE_NUMERIC = 2, // Êý×ÖÀàÐÍ( integer, float )
	PHP_TYPE_UNION	 = 3,
	PHP_TYPE_STRUCT  = 4,
};

#define PHP_COMMENT_STR "//"
#define PHP_INDENT_STR "\t"

struct STdrPhpTypeInfo
{
	char* pszTagName;  // tdr tag name
   
	char pszSPrefix[32];	// single prefix  in c/c++ head file.
    char pszMPrefix[32];	// multiple prefix(for array).

	int iPhpType;
	char pszPhpInitVal[32];
};

extern STdrPhpTypeInfo g_types_Tdr2Php[];

class CTdr2Php;

class CTdr2PhpElem
{
public:
	CTdr2PhpElem();
	CTdr2PhpElem( CTdr2Php* poTdr2Php );
	virtual ~CTdr2PhpElem(){}

public:
	virtual void Translate( XMLElement* poXmlElem ) { }

public:
	std::list<std::string> lines;
	CTdr2Php* poTdr2Php;
}; 

int GetTdr2PhpTypesSize();

#endif

