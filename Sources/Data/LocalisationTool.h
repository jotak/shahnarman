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
    static LocalisationTool * getInstance()
    {
        if (mInst == NULL) mInst = new LocalisationTool();
        return mInst;
    };

    void Init(Parameters * pParams, DebugManager * pDebug);
    char * getCurrentLanguageName()
    {
        return m_sLanguage;
    };
    char * getText(const char * sKey, char * sBuf, int iSize, void ** pArgs = NULL);
    char * getText1stUp(const char * sKey, char * sBuf, int iSize);
    char * getTextUp(const char * sKey, char * sBuf, int iSize);
    char * getTextLow(const char * sKey, char * sBuf, int iSize);
    char * readLocalizedXMLNode(XMLLiteElement * pNode, char * sBuf, int iBufSize);
    char * long_hashToString(char * sBuf, int iBufSize, const char * sSeparator, long_hash * hm, int nArgs, ...);

private:
    LocalisationTool();
    static LocalisationTool * mInst;
    char m_sLanguage[64];
    str_hash m_sAllTexts;
    DebugManager * m_pDebug;
};

// Helper alias
#define i18n LocalisationTool::getInstance()

#endif
