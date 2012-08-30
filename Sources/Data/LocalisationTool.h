#ifndef _LOCALISATION_TOOL_H
#define _LOCALISATION_TOOL_H

#include "../utils.h"

class XMLLiteElement;
class DebugManager;
class Parameters;

// This is a singleton
class LocalisationTool
{
public:
  ~LocalisationTool();
  static LocalisationTool * getInstance() { if (mInst == NULL) mInst = new LocalisationTool(); return mInst; };

  void Init(Parameters * pParams, DebugManager * pDebug);
  wchar_t * getCurrentLanguageName() { return m_sLanguage; };
  wchar_t * getText(const wchar_t * sKey, wchar_t * sBuf, int iSize, void ** pArgs = NULL);
  wchar_t * getText1stUp(const wchar_t * sKey, wchar_t * sBuf, int iSize);
  wchar_t * getTextUp(const wchar_t * sKey, wchar_t * sBuf, int iSize);
  wchar_t * getTextLow(const wchar_t * sKey, wchar_t * sBuf, int iSize);
  wchar_t * readLocalizedXMLNode(XMLLiteElement * pNode, wchar_t * sBuf, int iBufSize);
  wchar_t * long_hashToString(wchar_t * sBuf, int iBufSize, const wchar_t * sSeparator, long_hash * hm, int nArgs, ...);

private:
	LocalisationTool();
  static LocalisationTool * mInst;
  wchar_t m_sLanguage[64];
  wstr_hash m_sAllTexts;
  DebugManager * m_pDebug;
};

// Helper alias
#define i18n LocalisationTool::getInstance()

#endif
