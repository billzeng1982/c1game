#include "Tdr2PhpUnion.h"
#include "Tdr2Php.h"
#include "Tdr2PhpUtil.h"

CTdr2PhpUnion::CTdr2PhpUnion( CTdr2Php* poTdr2Php ) : CTdr2PhpElem( poTdr2Php ), entrySet( poTdr2Php )
{
}


void CTdr2PhpUnion::Translate( XMLElement* poXmlElem )
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
    // unionÎÞconstructor!
    this->_MakeUnpack( 1 );
    this->_MakePack( 1 );
    
    lines.push_back( "}" );
}


void CTdr2PhpUnion::_MakeUnpack( int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    std::list<STdr2PhpEntry*>::iterator it;

    lines.push_back( indentStr + "public function unpack( $Selector, $JsonObj )" );
    lines.push_back( indentStr + "{" );

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )   
    {
        pstEntry = *it;

        // ÕÒ³öselector string
        const char* pszID = poTdr2Php->FindConst( pstEntry->pszUnionId );
        if( !pszID )
        {
            throw string( string("Can not find union entry id: ") + pstEntry->pszUnionId );
        }

        if( it == entrySet.allEntryList.begin() )
        {
            lines.push_back(  indentStr2 + "if( " + pszID  + " == $Selector )" );    
        }else
        {
            lines.push_back( indentStr2 + "else if( " + pszID  + " == $Selector )" );    
        }

        lines.push_back( indentStr2 + "{" );
        this->_MakeOneUnpackEntry( pstEntry, indentLevel+2 );
        lines.push_back( indentStr2 + "}" );
    }

    lines.push_back( indentStr + "}" );
}


void CTdr2PhpUnion::_MakePack( int indentLevel )
{
    STdr2PhpEntry* pstEntry = NULL;
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    string& indentStr2 = poTdr2Php->GetIndentStr( indentLevel + 1 );
    std::list<STdr2PhpEntry*>::iterator it;

    lines.push_back( indentStr + "public function pack( $Selector, &$JsonStr )" );
    lines.push_back( indentStr + "{" );
    lines.push_back( indentStr2 + "$JsonStr .= '{';" );

    for( it = entrySet.allEntryList.begin(); it != entrySet.allEntryList.end(); it++ )   
    {
        pstEntry = *it;

        // ÕÒ³öselector string
        const char* pszID = poTdr2Php->FindConst( pstEntry->pszUnionId );
        if( !pszID )
        {
            throw string( string("Can not find union entry id: ") + pstEntry->pszUnionId );
        }

        if( it == entrySet.allEntryList.begin() )
        {
            lines.push_back( indentStr2 + "if( " + pszID + " == $Selector )" );    
        }else
        {
            lines.push_back( indentStr2 + "else if( " + pszID + " == $Selector )" );    
        }

        lines.push_back( indentStr2 + "{" );
        this->_MakeOnePackEntry( pstEntry, indentLevel+2 );
        lines.push_back( indentStr2 + "}" );
    }

    lines.push_back( indentStr2 + "$JsonStr .= '}';" );
    lines.push_back( indentStr + "}" );
}


void CTdr2PhpUnion::_MakeOneUnpackEntry( STdr2PhpEntry* pstEntry, int indentLevel )
{   
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );
    if( CTdr2PhpUtil::IsSimplePhpType( pstEntry->pstTypeInfo->iPhpType ) )
    {
        lines.push_back( indentStr + "$this->" + pstEntry->pszName + " = $JsonObj->{ '"+ pstEntry->pszName +"' };" );
    }else
    {
        lines.push_back( indentStr + "$this->" + pstEntry->pszName + " = new " + pstEntry->pszType + "();" );
        lines.push_back( indentStr + "$this->" + pstEntry->pszName + "->unpack( $JsonObj->{ '" + pstEntry->pszName + "' } );" );
    }
}

void CTdr2PhpUnion::_MakeOnePackEntry( STdr2PhpEntry* pstEntry, int indentLevel )
{
    string& indentStr = poTdr2Php->GetIndentStr( indentLevel );

    lines.push_back( indentStr + "$JsonStr .= '\"" + pstEntry->pszName + "\":';" );
    
    if( PHP_TYPE_STRING == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr + "$JsonStr . '\"" + pstEntry->pszName +"\":' . ' \"' . $this->" + pstEntry->pszName + ".'\"';" );
    }
    else if( PHP_TYPE_NUMERIC == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr + "$JsonStr . '\"" + pstEntry->pszName +  "\": . $this->" + pstEntry->pszName );
    }
    else if( PHP_TYPE_STRUCT == pstEntry->pstTypeInfo->iPhpType )
    {
        lines.push_back( indentStr + "$this->" + pstEntry->pszName + "->pack( $JsonStr ); " );
    }    
}

