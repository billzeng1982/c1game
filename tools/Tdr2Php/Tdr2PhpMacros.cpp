#include "Tdr2PhpMacros.h"
#include "Tdr2Php.h"
#include "Tdr2PhpUtil.h"

CTdr2PhpMacros::CTdr2PhpMacros( CTdr2Php* poTdr2Php ) : CTdr2PhpElem( poTdr2Php )
{
}


void CTdr2PhpMacros::Translate( XMLElement* poXmlElem )
{
    const char* pszElemName = poXmlElem->Name();
    if( 0 == strcmp( pszElemName, "macro") )
    {
        std::string line;
        this->_TranslateMacro( poXmlElem, line, 0 );
        lines.push_back( line );
    }else if( 0 == strcmp( pszElemName, "macrosgroup" ) )
    {
        std::string line;
        XMLAttribute* poXmlAttr = NULL;
        poXmlAttr =  poXmlElem->FindAttribute("name");    
        assert( poXmlAttr );

        line = "class ";
        line += poXmlAttr->Value();
        CTdr2PhpUtil::AddComment( poXmlElem, line );
        lines.push_back( line );
        lines.push_back( "{" );

        XMLElement* poMacroElem = NULL;
        for( poMacroElem = poXmlElem->FirstChildElement(); poMacroElem; poMacroElem = poMacroElem->NextSiblingElement() )
        {
            std::string macroLine;
            this->_TranslateMacro( poMacroElem, macroLine, 1 );
            lines.push_back( macroLine );
        }

        lines.push_back( "}" );
    }
    else
    {
        throw std::string( std::string("Unknwon type namne: ") + pszElemName );
    }
}


void CTdr2PhpMacros::_TranslateMacro( XMLElement* poXmlElem, std::string& line, int iIndentLevel )
{
    std::string& indentStr = poTdr2Php->GetIndentStr( iIndentLevel );
    line += indentStr;

    line += "const ";

    XMLAttribute* poXmlAttr = NULL;
    poXmlAttr =  poXmlElem->FindAttribute("name");
    assert( poXmlAttr );
    line += poXmlAttr->Value();
    line += " = ";
    poXmlAttr =  poXmlElem->FindAttribute("value");
    assert( poXmlAttr );
    line += poXmlAttr->Value();
    line += ";";

    CTdr2PhpUtil::AddComment( poXmlElem, line );
}
