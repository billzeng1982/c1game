#ifndef _TDR_XML_CFG_H_
#define _TDR_XML_CFG_H_

/*
	∂¡tdr xml≈‰÷√µµ
*/

#include <string.h>
#include <assert.h>
#include "tdr/tdr.h"

template< typename TCONF >
class CTdrXmlCfg
{
public:
	CTdrXmlCfg() { bzero( &m_stConfig, sizeof(m_stConfig) ); }
	virtual ~CTdrXmlCfg() {}

	bool ReadCfg( const char* pszCfgFile, LPTDRMETALIB pstCfgMetaLib, const char* pszMetaName );
	TCONF& GetCfg() { return m_stConfig; }

protected:
	// ◊”¿‡ºÏ≤È
	virtual bool _CheckCfg() { return true; }

protected:
	TCONF m_stConfig;
};


template< typename TCONF >
bool CTdrXmlCfg<TCONF>::ReadCfg( const char* pszCfgFile, LPTDRMETALIB pstCfgMetaLib, const char* pszMetaName )
{
	if( !pszCfgFile	|| !pstCfgMetaLib || !pszMetaName )	
	{
		assert( false );
		return false;
	}

	LPTDRMETA pstMeta =  tdr_get_meta_by_name(pstCfgMetaLib, pszMetaName);
	if( !pstMeta )
	{
		printf("tdr_get_meta_by_name failed! CfgFile <%s>, MetaName <%s>\n", pszCfgFile, pszMetaName);
		return false;
	}

	TDRDATA stHost;
	stHost.iBuff = sizeof(m_stConfig);
	stHost.pszBuff = (char*)&m_stConfig;

	// input file
	int iRet = tdr_input_file( pstMeta, &stHost, pszCfgFile, \
						       tdr_get_meta_current_version(pstMeta), TDR_XML_DATA_FORMAT_LIST_ENTRY_NAME );

	if ( TDR_ERR_IS_ERROR(iRet) )
	{
		printf("tdr_input_file failed , for %s\n", tdr_error_string(iRet));
    	return false;
	}

	return this->_CheckCfg();
}

#endif

