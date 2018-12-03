#ifndef _TDR_2_PHP_STRUCT_H_
#define _TDR_2_PHP_STRUCT_H_

#include "Tdr2PhpElem.h"
#include "Tdr2PhpEntrySet.h"


class CTdr2PhpStruct : public CTdr2PhpElem
{
public:
	CTdr2PhpStruct( CTdr2Php* poTdr2Php );
	virtual ~CTdr2PhpStruct(){}

	virtual void Translate( XMLElement* poXmlElem );

private:
	void _MakeConstructor( int indentLevel );
	void _MakeArrayConstructor(int indentLevel, STdr2PhpEntry* pstEntry);

	void _MakeUnpack( int indentLevel );
	void _MakePack( int indentLevel );

	void _MakeUnpackArray( int indentLevel, STdr2PhpEntry* pstEntry );
	void _MakePackArray( int indentLevel, STdr2PhpEntry* pstEntry );

private:
	CTdr2PhpEntrySet entrySet;
};

#endif
