#include "Tdr2PhpStruct.h"
#include "Tdr2Php.h"
#include "Tdr2PhpUtil.h"

CTdr2PhpStruct::CTdr2PhpStruct( CTdr2Php* poTdr2Php ) : CTdr2PhpElem( poTdr2Php ), entrySet( poTdr2Php )
{
}


void CTdr2PhpStruct::Translate( XMLElement* poXmlElem )
{
    this->entrySet.Build(poXmlElem);

    std::string line;
    XMLAttribute* poXmlAttr = NULL;
    poXmlAttr =  poXmlElem->FindAttribute("name");    
    assert( poXmlAttr );

    line = "class ";
    line += poXmlAttr->Value();
    CTdr2PhpUtil::AddComment( poXmlElem, line );
    lines.push_back( line );
    lines.push_back( "{" );

    poTdr2Php->MakeMembers( entrySet, lines, 1 );
    this->_MakeConstructor( 1 );
    this->_MakeUnpack( 1 );
    this->_MakePack( 1 );

    lines.push_back( "}" );
}


void CTdr2PhpStruct::_MakeConstructor( int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    std::list<STdr2PhpEntry*>::iterator it;

    lines.push_back( indentStr + "public function __construct()" );
    lines.push_back( indentStr + "{" );

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )
    {
        pstEntry = *it;
        if( !pstEntry->IsArray )
        {
            if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType || 
                PHP_TYPE_UNION == pstEntry->pstTypeInfo->iPhpType )
            {
                std::string line;
                line = indentStr2 + "$this->" + pstEntry->pszName + " = new " + pstEntry->pszType + "();";
                lines.push_back( line );
            }
        }else
        {
            // 数组需要单独出来构造
            this->_MakeArrayConstructor( indentLevel+1, pstEntry );
        }
    }

    lines.push_back( indentStr + "}" );
}


void CTdr2PhpStruct::_MakeUnpack( int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    std::list<STdr2PhpEntry*>::iterator it;

    lines.push_back( indentStr + "public function unpack( $JsonObj )" );
    lines.push_back( indentStr + "{" );

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )
    {
        pstEntry = *it;

        if( pstEntry->IsArray )
        {
            this->_MakeUnpackArray( indentLevel+1, pstEntry ); 
        }else
        {
            std::string line;
            if( CTdr2PhpUtil::IsSimplePhpType( pstEntry->pstTypeInfo->iPhpType ) )
            {
                line = indentStr2 + "$this->" + pstEntry->pszName + " = " + "$JsonObj->{ '" + pstEntry->pszName + "' };";
            }
            else if( PHP_TYPE_UNION == pstEntry->pstTypeInfo->iPhpType )
            {
                line = indentStr2 + "$this->" + pstEntry->pszName + "->unpack( $this->" + pstEntry->select + ", $JsonObj->{ '" + pstEntry->pszName + "' } );";
            }
            else if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType )
            {
                line = indentStr2 + "$this->" +  pstEntry->pszName + "->unpack( $JsonObj->{ '" + pstEntry->pszName + "' } );";
            }    

            lines.push_back( line );
        }
    }

    lines.push_back( indentStr + "}" );
}


void CTdr2PhpStruct::_MakePack( int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    std::list<STdr2PhpEntry*>::iterator it;
    std::list<STdr2PhpEntry*>::iterator itN;

    lines.push_back( indentStr + "public function pack( &$JsonStr )" );
    lines.push_back( indentStr + "{" );
    lines.push_back( indentStr2 + "$JsonStr .= '{';" );

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )
    {
        pstEntry = *it;
        if( pstEntry->IsArray )
        {
            this->_MakePackArray( indentLevel+1, pstEntry ); 
        }else 
        {
            if( PHP_TYPE_STRING == pstEntry->pstTypeInfo->iPhpType )
            {
                lines.push_back( indentStr2 + "$JsonStr = $JsonStr . '\"" + pstEntry->pszName +"\":' . '\"' . $this->" + pstEntry->pszName + " . '\"';" );
            }
            else if( PHP_TYPE_NUMERIC == pstEntry->pstTypeInfo->iPhpType )
            {
                lines.push_back( indentStr2 + "$JsonStr = $JsonStr . '\"" + pstEntry->pszName +  "\":' . $this->" + pstEntry->pszName + ";" );
            }
            else if( PHP_TYPE_UNION == pstEntry->pstTypeInfo->iPhpType )
            {
                lines.push_back( indentStr2 + "$JsonStr .= '\"" + pstEntry->pszName + "\":';" );
                lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "->pack( $this->" + pstEntry->select + ", $JsonStr ); " );
            }
            else if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType )
            {
                lines.push_back( indentStr2 + "$JsonStr .= '\"" + pstEntry->pszName + "\":';" );
                lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "->pack( $JsonStr ); " );
            }    
        }

        itN = it;
        itN++;
        if( itN != entrySet.allEntryList.end() )
        {
            lines.push_back( indentStr2 + "$JsonStr .= ',';" );
        }
    }

    lines.push_back( indentStr2 + "$JsonStr .= '}';" );
    lines.push_back( indentStr + "}" );
}


void CTdr2PhpStruct::_MakePackArray( int indentLevel, STdr2PhpEntry* pstEntry )
{
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    string& indentStr3 = poTdr2Php->GetIndentStr( indentLevel + 2 );

    lines.push_back( indentStr + "$JsonStr = $JsonStr . '\"" + pstEntry->pszName + "\":[';");
    lines.push_back( indentStr + "for( $i=0; $i < $this->" + pstEntry->pszRefer + " && $i < " + pstEntry->pszArrSize + "; $i++ )" );
    lines.push_back( indentStr + "{" );

    if( PHP_TYPE_STRING == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr2 + "$JsonStr .= '\"'" + " . $this->" + pstEntry->pszName + "[$i] . '\"';"  );
    }
    else if( PHP_TYPE_NUMERIC == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr2 + "$JsonStr .= $this->" + pstEntry->pszName + "[$i];"  );
    }
    else if( PHP_TYPE_UNION == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]->pack( $this->" + pstEntry->select + ", $JsonStr ); " );
    }
    else if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]->pack( $JsonStr ); " );
    }    
    lines.push_back( indentStr2 + "if( $i < $this->" + pstEntry->pszRefer + " - 1 )" );
    lines.push_back( indentStr2 + "{" );
    lines.push_back( indentStr3 + "$JsonStr .= ',';" );
    lines.push_back( indentStr2 + "}" );

    lines.push_back( indentStr + "}" );
    lines.push_back( indentStr + "$JsonStr .= ']';" );
    
}


void CTdr2PhpStruct::_MakeUnpackArray( int indentLevel, STdr2PhpEntry* pstEntry )
{
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );

    lines.push_back( indentStr + "for( $i=0; $i < $this->" + pstEntry->pszRefer + " && $i < " + pstEntry->pszArrSize + "; $i++ )" );

    lines.push_back( indentStr + "{" );
    if( CTdr2PhpUtil::IsSimplePhpType( pstEntry->pstTypeInfo->iPhpType ) )
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]" + " = " + "$JsonObj->{ '" + pstEntry->pszName + "' }[$i];");
    }
    else 
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]->unpack( $JsonObj->{ '" + pstEntry->pszName + "' }[$i] );");
    }
    
    lines.push_back( indentStr + "}" );
}


void CTdr2PhpStruct::_MakeArrayConstructor(int indentLevel, STdr2PhpEntry* pstEntry)
{
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );

    lines.push_back( indentStr + "for( $i=0; $i < " + pstEntry->pszArrSize + "; $i++ )" );
    lines.push_back( indentStr + "{" );

    if( CTdr2PhpUtil::IsSimplePhpType( pstEntry->pstTypeInfo->iPhpType ) )
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]" + " = " + pstEntry->pstTypeInfo->pszPhpInitVal + ";" );
    }else
    {
        lines.push_back( indentStr2 + "$this->" + pstEntry->pszName + "[$i]" + " = new " + pstEntry->pszType + "();" );
    }
    
    lines.push_back( indentStr + "}" );
}

