#include "Tdr2Php.h"
#include "tdr/tdr.h"
#include <string.h>
#include "tdr/tdr_XMLtags.h" 
#include "Tdr2PhpMacros.h"
#include "Tdr2PhpStruct.h"
#include "Tdr2PhpUnion.h"
#include "Tdr2PhpUtil.h"

CTdr2Php::CTdr2Php()
{
    bzero( szPhpFile, sizeof(szPhpFile) );
    poXmlMetalib = NULL;

    this->indentsMap[0] = "";
    for (int j = 1; j <= 10; j++)
    {
        this->indentsMap[j] = this->indentsMap[j - 1] + PHP_INDENT_STR;
    }
}


string& CTdr2Php::GetIndentStr( int indentLevel )
{
    // indent by progress
    if (indentLevel > (int)this->indentsMap.size())
    {
        throw string("indent-level error");
    } else if (indentLevel == (int)this->indentsMap.size())
    {
        this->indentsMap[indentLevel] = this->indentsMap[indentLevel - 1] + PHP_INDENT_STR;
    }

    return this->indentsMap[indentLevel];
}


bool CTdr2Php::Translate( const char* pszXmlFile )
{    
    if( !this->_MakePhpFile( pszXmlFile ) )
    {
        return false;    
    }

    XMLDocument oXmlDoc;
    FILE* fpIn = fopen( pszXmlFile, "r" );
    if( !fpIn )
    {
        fprintf( stderr, "Can not open input file %s failed!\n", pszXmlFile);
        return false;
    }

    if( oXmlDoc.LoadFile( fpIn ) != XML_NO_ERROR )
    {
        fprintf( stderr, "Load xml file %s error:%s\n", pszXmlFile, oXmlDoc.GetErrorStr1() );
        return false;
    }

    this->poXmlMetalib = oXmlDoc.FirstChildElement( "metalib" );
    if( !this->poXmlMetalib )
    {
        fprintf( stderr, "Can not find building elem!\n" );
        return false;
    }

    this->_BuildTokenTable();

    try {
        XMLElement* poXmlElem = NULL;
        CTdr2PhpElem* poPhpElem = NULL;
        for( poXmlElem = this->poXmlMetalib->FirstChildElement(); poXmlElem; poXmlElem = poXmlElem->NextSiblingElement() )
        {
            poPhpElem = this->_CreatePhpElem( poXmlElem->Name() );
            if( !poPhpElem )
            {
                continue;
            }
            poPhpElem->Translate( poXmlElem );
            phpElems.push_back( poPhpElem );
        }

        this->_Output();
    }catch( const string& s )
    {
        printf( "error: %s\n", s.c_str() );
        return false;
    }catch( ... )
    {
        printf( "error: traslate failed! unknown error!\n");
        return false;
    }

    return true;
}


bool CTdr2Php::_MakePhpFile( const char* pszXmlFile )
{
    strncpy( szPhpFile, pszXmlFile, sizeof(szPhpFile)-1 );

    char* p = NULL;
    p = strstr( szPhpFile, ".xml" );
    if( !p )
    {
        fprintf( stderr, "Find name error! %s\n", pszXmlFile );
        return false;
    }

    strcpy( p, ".class.php");

    return true;
}


bool CTdr2Php::_Output( )
{
    FILE* fp = fopen( szPhpFile, "w+" );
    if( !fp )
    {
        printf("open file %s failed!\n", szPhpFile);
        return false;
    }

    //head
    fputs(  "<?php\n\n", fp );

    CTdr2PhpElem* poPhpElem = NULL;
    std::list<CTdr2PhpElem*>::iterator phpElemIt = phpElems.begin();
    for( ; phpElemIt != phpElems.end(); ++phpElemIt )
    {
        poPhpElem = *phpElemIt;
        std::list<std::string>::iterator lineIter = poPhpElem->lines.begin();
        for( ; lineIter !=  poPhpElem->lines.end();  ++lineIter )
        {
            std::string& line = *lineIter;
            line += "\n";
            fputs( line.c_str(), fp );   
        }
        fputs( "\n", fp );
    }

    // tail
    fputs( "?>\n", fp );
    fflush( fp );
    fclose( fp );

    printf("Save file: %s\n", szPhpFile);
    return true;
}


const char* CTdr2Php::FindConst( const char* pszConstName )
{
    ConstTable_t::iterator it;
    it = constTable.find( pszConstName );
    if( it == constTable.end() )
    {
        return NULL;
    }
    return it->second.c_str();
}


char CTdr2Php::FindCompositeType( const char* pszTypeName )
{
    CompositeTypeTable_t::iterator it;
    it = compositeTypeTable.find( pszTypeName );
    if( it == compositeTypeTable.end() )
    {
        return -1;
    }
    return it->second;
}


void CTdr2Php::_BuildTokenTable()
{
    XMLElement* poXmlElem = NULL;
    XMLAttribute* poXmlAttr1 = NULL;
    //XMLAttribute* poXmlAttr2 = NULL;
    
    for( poXmlElem = this->poXmlMetalib->FirstChildElement(); poXmlElem; poXmlElem = poXmlElem->NextSiblingElement() )
    {
        const char* pszElemName = poXmlElem->Name();

        if( 0 == strcmp( pszElemName, "macro") )
        {
            poXmlAttr1 =  poXmlElem->FindAttribute("name");
            assert( poXmlAttr1 );
            //poXmlAttr2 =  poXmlElem->FindAttribute("value");
            //assert( poXmlAttr2 );
            std::string macroStr( poXmlAttr1->Value() );
            constTable.insert( ConstTable_t::value_type( poXmlAttr1->Value(), macroStr )); 
        }
        else if( 0 == strcmp( pszElemName, "macrosgroup" ) )
        {
            const char* pszGroupName = poXmlElem->FindAttribute("name")->Value();
        
            XMLElement* poMacroElem = NULL;
            for( poMacroElem = poXmlElem->FirstChildElement(); poMacroElem; poMacroElem = poMacroElem->NextSiblingElement() )
            {
                poXmlAttr1 =  poMacroElem->FindAttribute("name");
                assert( poXmlAttr1 );
                //poXmlAttr2 =  poMacroElem->FindAttribute("value");
                //assert( poXmlAttr2 );
                std::string macroStr( pszGroupName );
                macroStr = macroStr + "::" + poXmlAttr1->Value();
                constTable.insert( ConstTable_t::value_type( poXmlAttr1->Value(), macroStr ));                 
            }
        }
        else if( 0 == strcmp( pszElemName, "struct" ) )
        {
            poXmlAttr1 =  poXmlElem->FindAttribute("name");
            assert( poXmlAttr1 );
            compositeTypeTable.insert( CompositeTypeTable_t::value_type( poXmlAttr1->Value(), TDR_TYPE_STRUCT ) );
        }
        else if( 0 == strcmp( pszElemName, "union" ) )
        {
            poXmlAttr1 =  poXmlElem->FindAttribute("name");
            assert( poXmlAttr1 );
            compositeTypeTable.insert( CompositeTypeTable_t::value_type( poXmlAttr1->Value(), TDR_TYPE_UNION ) );
        }
    }
}


CTdr2PhpElem* CTdr2Php::_CreatePhpElem( const char* pszType )
{
    if( 0 == strcmp( pszType, "macro") || 0 == strcmp( pszType, "macrosgroup" ) )
    {
        return new CTdr2PhpMacros( this );
    }
    else if( 0 == strcmp( pszType, "struct" ) )
    {
        return new CTdr2PhpStruct( this );
    }
    else if( 0 == strcmp( pszType, "union" ) )
    {
        return new CTdr2PhpUnion( this );
    }

    //printf( "unknown type: %s!\n", pszType );
    return NULL;
}


void CTdr2Php::MakeMembers( CTdr2PhpEntrySet& entrySet, std::list<string>& lines,int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = this->GetIndentStr( indentLevel );
    std::list<STdr2PhpEntry*>::iterator it;

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )
    {
        pstEntry = *it;

        std::string entryLine = indentStr;
        entryLine += "public $";
        if( pstEntry->IsArray )
        {
            entryLine = entryLine + pstEntry->pszName + " = array();"; 
        }else
        {
             entryLine = entryLine + pstEntry->pszName + " = " + pstEntry->pstTypeInfo->pszPhpInitVal + ";";
        }

        if( !CTdr2PhpUtil::IsSimplePhpType( pstEntry->pstTypeInfo->iPhpType ) )
        {
            entryLine = entryLine + " " + PHP_COMMENT_STR + " type: " + pstEntry->pszType + " ";
        }

        CTdr2PhpUtil::AddComment( pstEntry->pszDesc, entryLine );

        lines.push_back( entryLine );
    }
}

