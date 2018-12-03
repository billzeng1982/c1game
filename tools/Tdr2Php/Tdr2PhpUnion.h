#ifndef _TDR_2_PHP_UNION_H_
#define _TDR_2_PHP_UNION_H_

#include "Tdr2PhpElem.h"
#include "Tdr2PhpEntrySet.h"

/*
	1.union 不存在constructor
	2.union 不存在数组entry
	3.union 不存在union entry
*/

class CTdr2PhpUnion : public CTdr2PhpElem
{
public:
	CTdr2PhpUnion( CTdr2Php* poTdr2Php );
	virtual ~CTdr2PhpUnion(){}

	virtual void Translate( XMLElement* poXmlElem );

private:
	void _MakeUnpack( int indentLevel );
	void _MakePack( int indentLevel );

	void _MakeOneUnpackEntry( STdr2PhpEntry* pstEntry, int indentLevel );
	void _MakeOnePackEntry( STdr2PhpEntry* pstEntry, int indentLevel );
	
private:
	CTdr2PhpEntrySet entrySet;
};

#endif

