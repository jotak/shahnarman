#include "LocalisationTool.h"
#include "XMLLiteReader.h"
#include "../Debug/DebugManager.h"
#include "Parameters.h"
#include <stdarg.h>
#include <stdio.h>

LocalisationTool * LocalisationTool::mInst = NULL;

// -----------------------------------------------------------------
// Name : LocalisationTool
// -----------------------------------------------------------------
LocalisationTool::LocalisationTool()
{
    m_pDebug = NULL;
}

// -----------------------------------------------------------------
// Name : ~LocalisationTool
// -----------------------------------------------------------------
LocalisationTool::~LocalisationTool()
{
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void LocalisationTool::Init(Parameters * pParams, DebugManager * pDebug)
{
    m_pDebug = pDebug;

    FILE * pFile = NULL;
    char sPath[MAX_PATH] = "";
    char sError[1024] = "";
    char sLine[1024] = "";
    char sKey[64] = "";
    char sValue[1024] = "";

    // Free current data, if any
    m_sAllTexts.clear();

    // Build path
    strncpy(m_sLanguage, pParams->sLanguages[pParams->language], 64);
    snprintf(sPath, MAX_PATH, "%si18n/%s.po", DATA_PATH, m_sLanguage);

    // Open file
    if (0 != fopen_s(&pFile, sPath, "r"))
    {
        snprintf(sError, 1024, "Could not open language file for %s. Check out file %s.", m_sLanguage, sPath);
        pDebug->notifyErrorMessage(sError);
        return;
    }

    // Read file
    while (!feof(pFile))
    {
        // Get line by line
        fgets(sLine, 1024, pFile);
        if (sscanf(sLine, "msgid \"%s", sKey))  // read sKey but don't rely on that
        {
            wsafecpy(sKey, 64, &(sLine[7]));  // Copy sLine starting from 7th character (after 'msgid "')
            sKey[strlen(sKey) - 2] = '\0'; // Remove last characters which should be '"' + \n
        }
        if (strcmp(sKey, "") != 0 && sscanf(sLine, "msgstr \"%s", sValue))
        {
            wsafecpy(sValue, 1024, &(sLine[8]));  // Copy sLine starting from 8th character (after 'msgstr "')
            sValue[strlen(sValue) - 2] = '\0'; // Remove last characters which should be '"' + \n
            m_sAllTexts[sKey] = sValue;
        }
    }

    fclose(pFile);
}

// -----------------------------------------------------------------
// Name : readLocalizedXMLNode
// -----------------------------------------------------------------
char * LocalisationTool::readLocalizedXMLNode(XMLLiteElement * pNode, char * sBuf, int iBufSize)
{
    XMLLiteElement * pElt = pNode->getChildByName(m_sLanguage);
    if (pElt != NULL)
        wsafecpy(sBuf, iBufSize, pElt->getCharValue());
    else
        wsafecpy(sBuf, iBufSize, "error: text not found");

    return sBuf;
}

// -----------------------------------------------------------------
// Name : getText
//  Fields order is managed (%$1x)
//  pArgs must contain all the variables to replace in model
// -----------------------------------------------------------------
char * LocalisationTool::getText(const char * sKey, char * sBuf, int iSize, void ** pArgs)
{
    // If pArgs is null we can skip all variable replacement stuff
    if (pArgs == NULL)
    {
        // Get text in hashmap
        str_hash::iterator it = m_sAllTexts.find(sKey);
        if (it == m_sAllTexts.end())
        {
            if (m_pDebug != NULL)
            {
                char sErr[128];
                snprintf(sErr, 128, "L12N key not found: %s", sKey);
                m_pDebug->notifyErrorMessage(sErr);
            }
            wsafecpy(sBuf, iSize, sKey);
        }
        else
            wsafecpy(sBuf, iSize, it->second.c_str());
        return sBuf;
    }

    char * sModel = new char[iSize];

    // Get text in hashmap
    str_hash::iterator it = m_sAllTexts.find(sKey);
    if (it == m_sAllTexts.end())
        wsafecpy(sModel, iSize, sKey);
    else
        wsafecpy(sModel, iSize, it->second.c_str());

    // If text contains variable fields, fill them
    int iDst = 0;
    int iSrc = 0;
    int iVarPos = 0;
    while (sModel[iSrc] != '\0')
    {
        if (sModel[iSrc] == '%')
        {
            iSrc++;
            // is '%' character?
            if (sModel[iSrc] == '%')
                sBuf[iDst++] = sModel[iSrc++];
            else
            {
                // ordered element?
                if (sModel[iSrc] == '$')
                {
                    iVarPos = (int) (sModel[++iSrc] - '1');
                    iSrc++;
                }
                int precision = -1;
//        u8 length = 1;
                // precision?
                if (sModel[iSrc] == '.')
                {
                    precision = (int) (sModel[++iSrc] - '0');
                    iSrc++;
                }
                // length?
                if (sModel[iSrc] == 'h')
                {
//          length = 0; // short
                    iSrc++;
                }
                else if (sModel[iSrc] == 'L' || sModel[iSrc] == 'l')
                {
//          length = 2; // long
                    iSrc++;
                }
                // specifier
                switch (sModel[iSrc])
                {
                case 'u':
                {
                    unsigned uVal = *(unsigned*)(pArgs[iVarPos]);
                    add_long_to_wstr((long)uVal, precision, sBuf, &iDst);
                    iSrc++;
                    iVarPos++;
                    break;
                }
                case 'd':
                {
                    long iVal = *(long*)(pArgs[iVarPos]);
                    add_long_to_wstr(iVal, precision, sBuf, &iDst);
                    iSrc++;
                    iVarPos++;
                    break;
                }
                case 'f':
                {
                    double fVal = *(double*)(pArgs[iVarPos]);
                    add_double_to_wstr(fVal, precision, sBuf, &iDst);
                    iSrc++;
                    iVarPos++;
                    break;
                }
                case 'c':
                {
                    int cVal = *(int*)(pArgs[iVarPos]);
                    sBuf[iDst++] = (char) cVal;
                    iSrc++;
                    iVarPos++;
                    break;
                }
                case 's':
                {
                    char * sVal = (char*)(pArgs[iVarPos]);
                    sBuf[iDst] = '\0';
                    wsafecat(sBuf, iSize, sVal);
                    iDst = strlen(sBuf);
                    iSrc++;
                    iVarPos++;
                    break;
                }
                }
            }
        }
        else
            sBuf[iDst++] = sModel[iSrc++];
    }
    sBuf[iDst] = '\0';
    delete[] sModel;

    return sBuf;
}

// -----------------------------------------------------------------
// Name : getText1stUp
//  Upper case for 1st letter
// -----------------------------------------------------------------
char * LocalisationTool::getText1stUp(const char * sKey, char * sBuf, int iSize)
{
    // Get text in hashmap
    str_hash::iterator it = m_sAllTexts.find(sKey);
    if (it == m_sAllTexts.end())
    {
        if (m_pDebug != NULL)
        {
            char sErr[128];
            snprintf(sErr, 128, "L12N key not found: %s", sKey);
            m_pDebug->notifyErrorMessage(sErr);
        }
        wsafecpy(sBuf, iSize, sKey);
    }
    else
        wsafecpy(sBuf, iSize, it->second.c_str());
    if (sBuf[0] >= 'a' && sBuf[0] <= 'z')
        sBuf[0] += 'A' - 'a';
    return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextUp
//  Upper case
// -----------------------------------------------------------------
char * LocalisationTool::getTextUp(const char * sKey, char * sBuf, int iSize)
{
    // Get text in hashmap
    str_hash::iterator it = m_sAllTexts.find(sKey);
    if (it == m_sAllTexts.end())
    {
        if (m_pDebug != NULL)
        {
            char sErr[128];
            snprintf(sErr, 128, "L12N key not found: %s", sKey);
            m_pDebug->notifyErrorMessage(sErr);
        }
        wsafecpy(sBuf, iSize, sKey);
    }
    else
        wsafecpy(sBuf, iSize, it->second.c_str());
    int i = 0;
    while (sBuf[i] != '\0')
    {
        if (sBuf[i] >= 'a' && sBuf[i] <= 'z')
            sBuf[i] += 'A' - 'a';
        i++;
    }
    return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextLow
//  Upper case
// -----------------------------------------------------------------
char * LocalisationTool::getTextLow(const char * sKey, char * sBuf, int iSize)
{
    // Get text in hashmap
    str_hash::iterator it = m_sAllTexts.find(sKey);
    if (it == m_sAllTexts.end())
    {
        if (m_pDebug != NULL)
        {
            char sErr[128];
            snprintf(sErr, 128, "L12N key not found: %s", sKey);
            m_pDebug->notifyErrorMessage(sErr);
        }
        wsafecpy(sBuf, iSize, sKey);
    }
    else
        wsafecpy(sBuf, iSize, it->second.c_str());
    int i = 0;
    while (sBuf[i] != '\0')
    {
        if (sBuf[i] >= 'A' && sBuf[i] <= 'Z')
            sBuf[i] += 'a' - 'A';
        i++;
    }
    return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextLow
//  Generate readable text from hashmap, with given hash keys
// -----------------------------------------------------------------
char * LocalisationTool::long_hashToString(char * sBuf, int iBufSize, const char * sSeparator, long_hash * hm, int nArgs, ...)
{
    char sLine[80];
    char sTranslatedKey[64];
    char s2P[8];
    char sSep[8] = "";
    va_list pArgs;

    wsafecpy(sBuf, iBufSize, "");
    i18n->getText("2P", s2P, 8);
    va_start(pArgs, nArgs);
    for (int i = 0; i < nArgs; i++)
    {
        char * sKey = va_arg(pArgs, char*);
        long_hash::const_iterator it = hm->find(sKey);
//    long_hash::iterator it = hm->find(sKey);
        if (it != hm->end())
        {
            i18n->getText1stUp(sKey, sTranslatedKey, 64);
            snprintf(sLine, 80, "%s%s%s%ld", sSep, sTranslatedKey, s2P, it->second);
            wsafecat(sBuf, iBufSize, sLine);
            wsafecpy(sSep, 8, sSeparator);
        }
    }
    va_end(pArgs);
    return sBuf;
}
