#include "Tdr2PhpEntrySet.h"
#include "Tdr2Php.h"
#include "Tdr2PhpUtil.h"
#include "tdr/tdr.h"


CTdr2PhpEntrySet::CTdr2PhpEntrySet( CTdr2Php* poTdr2Php )
{
    this->poTdr2Php = poTdr2Php;
}


void CTdr2PhpEntrySet::Build( XMLElement* poXmlElem )
{
    XMLElement* poEntryElem = NULL; 
    XMLAttribute* poAttr = NULL;
    for( poEntryElem = poXmlElem->FirstChildElement(); poEntryElem; poEntryElem = poEntryElem->NextSiblingElement() )
    {
        // 对每个entry
        STdr2PhpEntry* pstEntry = new STdr2PhpEntry();
        poAttr =  poEntryElem->FindAttribute("name");
        pstEntry->pszName = poAttr->Value();

        poAttr = poEntryElem->FindAttribute("desc");
        if( poAttr )
        {
            pstEntry->pszDesc = poAttr->Value();
        }

        XMLAttribute* poAttr = poEntryElem->FindAttribute("count");
        if( poAttr )
        {
            // 数组
            pstEntry->IsArray = true;
            if( CTdr2PhpUtil::IsDigits(poAttr->Value()) )
            {
                pstEntry->pszArrSize = poAttr->Value();
            }else
            {
                pstEntry->pszArrSize = poTdr2Php->FindConst( poAttr->Value() );
                if( !pstEntry->pszArrSize )
                {
                    throw std::string( string("Can not find count: ") + pstEntry->pszArrSize );
                }
            }
        
            XMLAttribute* poAttr = poEntryElem->FindAttribute("refer");
            if( !poAttr )
            {
                throw std::string( std::string("array entry ") + pstEntry->pszName + ": can not find refer!" );
            }
            pstEntry->pszRefer = poAttr->Value();
        }

        poAttr = poEntryElem->FindAttribute("id");
        if( poAttr )
        {
            pstEntry->pszUnionId = poAttr->Value();
        }

        poAttr = poEntryElem->FindAttribute("type");
        pstEntry->pstTypeInfo = CTdr2PhpUtil::GetTdrPhpTypeInfo( poAttr->Value() );
        if( !pstEntry->pstTypeInfo )
        {
            // 自定义复合类型entry
            pstEntry->pszType = poAttr->Value();
            int iType = poTdr2Php->FindCompositeType( pstEntry->pszType );
            if( TDR_TYPE_STRUCT == iType )
            {
                pstEntry->pstTypeInfo = CTdr2PhpUtil::GetTdrPhpTypeInfo( "struct" );
            }
            else if( TDR_TYPE_UNION == iType )
            {
                pstEntry->pstTypeInfo = CTdr2PhpUtil::GetTdrPhpTypeInfo( "union" );
                // 替换select string
                const char* pszSelect = poEntryElem->FindAttribute("select")->Value();
                char temp[128];
                int len = strlen( pszSelect );
                char* p = &temp[0];
                for( int i = 0; i < len; i++ )
                {
                    if( pszSelect[i] == '.' )
                    {
                        // . 换成 ->
                        *p++ = '-';
                        *p++ = '>';
                    }else
                    {
                        *p++ = pszSelect[i];
                    }
                }
                *p = '\0';
                pstEntry->select = temp;
            }else
            {
                throw std::string( std::string("Unknown entry type: ") + poAttr->Value() );
            }
        }

        allEntryList.push_back( pstEntry );
        if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType || PHP_TYPE_UNION == pstEntry->pstTypeInfo->iPhpType  )
        {
            compositeEntryList.push_back( pstEntry );
        }    
    }
}

