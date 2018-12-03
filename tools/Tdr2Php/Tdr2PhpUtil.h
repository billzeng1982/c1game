#ifndef _TDR_2_PHP_UTIL_H_
#define _TDR_2_PHP_UTIL_H_

#include "Tdr2PhpElem.h"
#include "Tdr2PhpEntrySet.h"

class CTdr2PhpUtil
{
public:
	CTdr2PhpUtil(){}
	~CTdr2PhpUtil(){}

	static STdrPhpTypeInfo* GetTdrPhpTypeInfo( const char* pszTagName );
	static void AddComment( XMLElement* poXmlElem, std::string& line );
	static void AddComment( const char* pszComment, std::string& line );
	static bool IsSimplePhpType( int iPhpType );
	// ÅÐ¶ÏstringÊÇ·ñÎªÊý×Ö
	static bool IsDigits( const char* pszString );
};

#endif

