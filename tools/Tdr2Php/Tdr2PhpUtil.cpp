#include "Tdr2PhpUtil.h"
#include <stdlib.h>

extern STdrPhpTypeInfo g_types_Tdr2Php[];

STdrPhpTypeInfo* CTdr2PhpUtil::GetTdrPhpTypeInfo( const char* pszTagName )
{
    STdrPhpTypeInfo* pstTypeInfo = NULL;

    pstTypeInfo = &g_types_Tdr2Php[0];
    int i;

    int iTableSize = ::GetTdr2PhpTypesSize();
    for( i = 0; i < iTableSize; i++ )
    {
        if( 0 == strcmp( pstTypeInfo->pszTagName, pszTagName ) )
        {
            return pstTypeInfo;
        }
        
        pstTypeInfo++;
    }

    return NULL;
}


void CTdr2PhpUtil::AddComment( XMLElement* poXmlElem, std::string& line )
{
    XMLAttribute* poXmlAttr = poXmlElem->FindAttribute("desc");
    if( poXmlAttr )
    {
        const char* pszComment = poXmlAttr->Value();
        if( pszComment[0] != '\0' )
        {
            line = line + " " + PHP_COMMENT_STR + " " + pszComment;
        }
    }
}


void CTdr2PhpUtil::AddComment( const char* pszComment, std::string& line )
{
    if( pszComment && pszComment[0] != '\0' )
    {
        line = line + " " + PHP_COMMENT_STR + " " + pszComment;
    }
}


bool CTdr2PhpUtil::IsSimplePhpType( int iPhpType )
{
    return ( PHP_TYPE_STRING== iPhpType || PHP_TYPE_NUMERIC == iPhpType ) ? true : false;
}


// 判断string是否为数字
bool CTdr2PhpUtil::IsDigits( const char* pszString )
{
    char* endptr = NULL;

    if( !pszString || '\0' == pszString[0] )
    {
        assert( false );
        return false;
    }

	strtol( pszString, &endptr, 0 );
	if( *endptr == 0 )
    {
        //是整数
        return true;
    }else
    {
        strtof( pszString, &endptr );
        if( *endptr  == 0 )
        {
            // 是浮点数
            return true;
        }
    }

    return false;
}

