#ifndef _TDR_2_PHP_MACROS_H_
#define _TDR_2_PHP_MACROS_H_

#include "Tdr2PhpElem.h"

class CTdr2PhpMacros : public CTdr2PhpElem
{
public:
	CTdr2PhpMacros( CTdr2Php* poTdr2Php );
	virtual ~CTdr2PhpMacros(){}

	virtual void Translate( XMLElement* poXmlElem );

private:
	void _TranslateMacro( XMLElement* poXmlElem, std::string& line, int iIndentLevel );

};

#endif

