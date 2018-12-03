#ifndef _TDR_2_PHP_H_
#define _TDR_2_PHP_H_

#include <string>
#include <list>
#include <stdio.h>
#include <map>
#include "tinyxml2.h"
#include "Tdr2PhpElem.h"
#include "functors.h"
#include "Tdr2PhpEntrySet.h"

using namespace tinyxml2;
using std::map;
using std::string;

class CTdr2Php
{
public:
	typedef map< const char*, std::string, ltstr > ConstTable_t;
	typedef map< const char*, char, ltstr > CompositeTypeTable_t; // 复合类型table, value: TDR_TYPE_UNION or TDR_TYPE_STRUCT

public:
	CTdr2Php();
	~CTdr2Php(){}

	bool Translate( const char* pszXmlFile );

	string& GetIndentStr( int indentLevel );

	const char* FindConst( const char* pszConstName );
	char FindCompositeType( const char* pszTypeName );

	void MakeMembers( CTdr2PhpEntrySet& entrySet, std::list<string>& lines,int indentLevel );

private:
	bool _Output( );
	bool _MakePhpFile( const char* pszXmlFile );
	void _BuildTokenTable( );
	CTdr2PhpElem* _CreatePhpElem( const char* pszType );

private:
	std::list<CTdr2PhpElem*> phpElems;
	char szPhpFile[256];
	
	map<unsigned int, string> indentsMap;

	XMLElement* poXmlMetalib;
	
	ConstTable_t 			constTable; // 存放const值
	CompositeTypeTable_t 	compositeTypeTable;
};

#endif

