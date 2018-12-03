/*
    把日志描述文件转化成thinkphp的表配置文件, 避免手工编码
*/

#include "tinyxml2.h"
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

using namespace tinyxml2;

#define OUTPUT_FILENAME "table.php"

struct PhpField
{
    std::string m_sName;
    std::string m_sDesc;
};

class PhpItem
{
public:
    PhpItem(){}
    ~PhpItem();

public:
    std::string  m_sPhpConfTableName;
    std::string  m_sLogTableName;
    std::vector<PhpField*>  m_vFields;
   
public:
    // 序列化成string, 利于输出, 注意前提是数据已准备好
    void Serialize( std::string& sResult );
};


PhpItem::~PhpItem()
{
    std::vector<PhpField*>::iterator it = m_vFields.begin();
    for( ; it != m_vFields.end(); ++it )
    {
        PhpField* pstPhpField = *it;
        delete( pstPhpField);
    }
    m_vFields.clear();
}

void PhpItem::Serialize( std::string& sResult )
{
    PhpField* pstPhpField = NULL;

    sResult+="'";
    sResult+=m_sPhpConfTableName;
    sResult+="'";
    sResult+="=> array(\n";

    sResult+="\t'TABLE_NAME' => '";
    sResult+=m_sLogTableName;
    sResult+="',\n";

    sResult+="\t'FIELDS' => '";
    std::vector<PhpField*>::iterator it = m_vFields.begin();
    std::vector<PhpField*>::iterator itN;
    for( ; it != m_vFields.end(); ++it )
    {
        itN = it+1;
        pstPhpField = *it;
        sResult += pstPhpField->m_sName;
        if( itN != m_vFields.end() )
        {
            sResult += ", ";
        }
    }
    sResult+="',\n";

    sResult+="\t'FIELDS_DESC'=> array( \n";
    for( it = m_vFields.begin(); it != m_vFields.end(); ++it )
    {
        itN = it+1;
        pstPhpField = *it;
        sResult += "\t\t";
        sResult += "'";
        sResult += pstPhpField->m_sName;
        sResult += "' => '";
        sResult += pstPhpField->m_sDesc;
        sResult += "',\n";
    }     
    sResult+="\t),\n";  // FIELDS_DESC array

    sResult+="),\n\n";
}


class PhpTable
{
public:
    PhpTable(){}
    ~PhpTable();

public:
    std::vector<PhpItem*> m_vPhpItems;

public:
    void Serialize( std::string& sResult );
    bool SaveFile( char* filename );
};


PhpTable::~PhpTable()
{
    std::vector<PhpItem*>::iterator it = m_vPhpItems.begin();
    for( ; it != m_vPhpItems.end(); ++it )
    {
        PhpItem* pstPhpItem = *it;
        delete( pstPhpItem);
    }
    m_vPhpItems.clear();
}

void PhpTable::Serialize( std::string& sResult )
{
    sResult.clear();
    sResult += "<?php\n";
    sResult += "return array(\n";

    std::vector<PhpItem*>::iterator it = m_vPhpItems.begin();
    for( ; it != m_vPhpItems.end(); ++it )
    {
        (*it)->Serialize( sResult );
    }

    sResult += ");\n";
}

bool PhpTable::SaveFile( char* filename )
{
    std::string sResult;

    FILE* fp = fopen( filename, "w+" );
    if( !fp )
    {
        printf("open file %s failed!\n", filename);
        return false;
    }

    this->Serialize(sResult);

    fputs( sResult.c_str(), fp );
    
    fflush( fp );
    fclose( fp );

    printf("Save file: %s\n", filename);
    return true;
}

static void ConvertXmlFile( char* pszInputFile )
{
    PhpTable oPhpTable;
    XMLDocument oXmlDoc;

    FILE* fpIn = fopen( pszInputFile, "r" );
    if( !fpIn )
    {
        fprintf( stderr, "Can not open input file %s failed!\n", pszInputFile);
        return;
    }

    if( oXmlDoc.LoadFile( fpIn ) != XML_NO_ERROR )
    {
        fprintf( stderr, "Load xml file %s error:%s\n", pszInputFile, oXmlDoc.GetErrorStr1() );
        return;
    }

    XMLElement* poMetalibElem = oXmlDoc.FirstChildElement( "metalib" );
    if( !poMetalibElem )
    {
        fprintf( stderr, "Can not find building elem!\n" );
        return;
    }

    XMLElement* poStructElem = NULL;
    XMLElement* poEntryElem = NULL;
    XMLAttribute* poXmlAttr = NULL;
    PhpItem* pstPhpItem = NULL;
    for( poStructElem = poMetalibElem->FirstChildElement("struct"); poStructElem; poStructElem = poStructElem->NextSiblingElement() )
    {
        pstPhpItem = new PhpItem();
        poXmlAttr = poStructElem->FindAttribute("name");
        assert( poXmlAttr );

        pstPhpItem->m_sLogTableName = poXmlAttr->Value();
        pstPhpItem->m_sPhpConfTableName = pstPhpItem->m_sLogTableName + "_TBL";

        for( poEntryElem = poStructElem->FirstChildElement("entry"); poEntryElem; poEntryElem = poEntryElem->NextSiblingElement() )
        {
            poXmlAttr = poEntryElem->FindAttribute("name");
            if( 0 == strcasecmp( poXmlAttr->Value(), "id" ) ||
                0 == strcasecmp( poXmlAttr->Value(), "EventId" ) )
            {
                continue;
            }

            PhpField* pstPhpField = new PhpField();
            pstPhpField->m_sName = poXmlAttr->Value();
            poXmlAttr = poEntryElem->FindAttribute("desc");
            if( poXmlAttr )
            {
                pstPhpField->m_sDesc = poXmlAttr->Value();
            }
            pstPhpItem->m_vFields.push_back( pstPhpField );
        }

        oPhpTable.m_vPhpItems.push_back( pstPhpItem );
    }

    oPhpTable.SaveFile( OUTPUT_FILENAME );
}


int main( int argc, char** argv )
{
    const char* optstring="i:";  // -i XmlFile

    int iOpt;
    char szInputFile[512];

    szInputFile[0] = '\0';

    if( argc < 3 )
    {
        printf( "Usage: %s -i XMLFILE\n", argv[0] );
        return -1;
    }

    while( (iOpt = getopt(argc, argv, optstring)) != -1 )  
    {
        switch( iOpt )
        {
            case 'i':
            {
                strncpy( szInputFile, optarg, sizeof(szInputFile) );
            }
            break;
            default:
            {
                printf( "Usage: %s -i XMLFILE\n", argv[0] );
                return -1;
            }
        }
    }

    if( strlen( szInputFile ) == 0 )
    {
        printf( "filename is empty!\n" );
        return -1;
    }

    ConvertXmlFile( szInputFile );
    
    return 0;
}

