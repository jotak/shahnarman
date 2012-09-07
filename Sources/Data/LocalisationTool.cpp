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
  wchar_t sPath[MAX_PATH] = L"";
  wchar_t sError[1024] = L"";
  wchar_t sLine[1024] = L"";
  wchar_t sKey[64] = L"";
  wchar_t sValue[1024] = L"";

  // Free current data, if any
  m_sAllTexts.clear();

  // Build path
  wsafecpy(m_sLanguage, 64, pParams->sLanguages[pParams->language]);
  swprintf(sPath, MAX_PATH, L"%si18n/%s.po", LDATA_PATH, m_sLanguage);

  // Open file
  if (0 != wfopen(&pFile, sPath, L"r"))
  {
    swprintf(sError, 1024, L"Could not open language file for %s. Check out file %s.", m_sLanguage, sPath);
    pDebug->notifyErrorMessage(sError);
    return;
  }

  // Read file
  while (!feof(pFile))
  {
    // Get line by line
    fgetws(sLine, 1024, pFile);
    if (swscanf(sLine, L"msgid \"%s", sKey, 64))  // read sKey but don't rely on that
    {
      wsafecpy(sKey, 64, &(sLine[7]));  // Copy sLine starting from 7th character (after 'msgid "')
      sKey[wcslen(sKey) - 2] = L'\0'; // Remove last characters which should be '"' + \n
    }
    if (wcscmp(sKey, L"") != 0 && swscanf(sLine, L"msgstr \"%s", sValue, 1024))
    {
      wsafecpy(sValue, 1024, &(sLine[8]));  // Copy sLine starting from 8th character (after 'msgstr "')
      sValue[wcslen(sValue) - 2] = L'\0'; // Remove last characters which should be '"' + \n
      m_sAllTexts[sKey] = sValue;
    }
  }

  fclose(pFile);
}

// -----------------------------------------------------------------
// Name : readLocalizedXMLNode
// -----------------------------------------------------------------
wchar_t * LocalisationTool::readLocalizedXMLNode(XMLLiteElement * pNode, wchar_t * sBuf, int iBufSize)
{
  XMLLiteElement * pElt = pNode->getChildByName(m_sLanguage);
  if (pElt != NULL)
    wsafecpy(sBuf, iBufSize, pElt->getCharValue());
  else
    wsafecpy(sBuf, iBufSize, L"error: text not found");

  return sBuf;
}

// -----------------------------------------------------------------
// Name : getText
//  Fields order is managed (%$1x)
//  pArgs must contain all the variables to replace in model
// -----------------------------------------------------------------
wchar_t * LocalisationTool::getText(const wchar_t * sKey, wchar_t * sBuf, int iSize, void ** pArgs)
{
  // If pArgs is null we can skip all variable replacement stuff
  if (pArgs == NULL)
  {
    // Get text in hashmap
    wstr_hash::iterator it = m_sAllTexts.find(sKey);
    if (it == m_sAllTexts.end())
    {
      if (m_pDebug != NULL)
      {
        wchar_t sErr[128];
        swprintf(sErr, 128, L"L12N key not found: %s", sKey);
        m_pDebug->notifyErrorMessage(sErr);
      }
      wsafecpy(sBuf, iSize, sKey);
    }
    else
      wsafecpy(sBuf, iSize, it->second.c_str());
    return sBuf;
  }

  wchar_t * sModel = new wchar_t[iSize];

  // Get text in hashmap
  wstr_hash::iterator it = m_sAllTexts.find(sKey);
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
    if (sModel[iSrc] == L'%')
    {
      iSrc++;
      // is '%' character?
      if (sModel[iSrc] == L'%')
        sBuf[iDst++] = sModel[iSrc++];
      else
      {
        // ordered element?
        if (sModel[iSrc] == L'$')
        {
          iVarPos = (int) (sModel[++iSrc] - L'1');
          iSrc++;
        }
        int precision = -1;
//        u8 length = 1;
        // precision?
        if (sModel[iSrc] == L'.')
        {
          precision = (int) (sModel[++iSrc] - L'0');
          iSrc++;
        }
        // length?
        if (sModel[iSrc] == L'h')
        {
//          length = 0; // short
          iSrc++;
        }
        else if (sModel[iSrc] == L'l' || sModel[iSrc] == L'L')
        {
//          length = 2; // long
          iSrc++;
        }
        // specifier
        switch (sModel[iSrc])
        {
        case L'u':
          {
            unsigned uVal = *(unsigned*)(pArgs[iVarPos]);
            add_long_to_wstr((long)uVal, precision, sBuf, &iDst);
            iSrc++;
            iVarPos++;
            break;
          }
        case L'd':
          {
            long iVal = *(long*)(pArgs[iVarPos]);
            add_long_to_wstr(iVal, precision, sBuf, &iDst);
            iSrc++;
            iVarPos++;
            break;
          }
        case L'f':
          {
            double fVal = *(double*)(pArgs[iVarPos]);
            add_double_to_wstr(fVal, precision, sBuf, &iDst);
            iSrc++;
            iVarPos++;
            break;
          }
        case L'c':
          {
            int cVal = *(int*)(pArgs[iVarPos]);
            sBuf[iDst++] = (wchar_t) cVal;
            iSrc++;
            iVarPos++;
            break;
          }
        case L's':
          {
            wchar_t * sVal = (wchar_t*)(pArgs[iVarPos]);
            sBuf[iDst] = L'\0';
            wsafecat(sBuf, iSize, sVal);
            iDst = wcslen(sBuf);
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
  sBuf[iDst] = L'\0';
  delete[] sModel;

  return sBuf;
}

// -----------------------------------------------------------------
// Name : getText1stUp
//  Upper case for 1st letter
// -----------------------------------------------------------------
wchar_t * LocalisationTool::getText1stUp(const wchar_t * sKey, wchar_t * sBuf, int iSize)
{
  // Get text in hashmap
  wstr_hash::iterator it = m_sAllTexts.find(sKey);
  if (it == m_sAllTexts.end())
  {
    if (m_pDebug != NULL)
    {
      wchar_t sErr[128];
      swprintf(sErr, 128, L"L12N key not found: %s", sKey);
      m_pDebug->notifyErrorMessage(sErr);
    }
    wsafecpy(sBuf, iSize, sKey);
  }
  else
    wsafecpy(sBuf, iSize, it->second.c_str());
  if (sBuf[0] >= L'a' && sBuf[0] <= L'z')
    sBuf[0] += L'A' - L'a';
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextUp
//  Upper case
// -----------------------------------------------------------------
wchar_t * LocalisationTool::getTextUp(const wchar_t * sKey, wchar_t * sBuf, int iSize)
{
  // Get text in hashmap
  wstr_hash::iterator it = m_sAllTexts.find(sKey);
  if (it == m_sAllTexts.end())
  {
    if (m_pDebug != NULL)
    {
      wchar_t sErr[128];
      swprintf(sErr, 128, L"L12N key not found: %s", sKey);
      m_pDebug->notifyErrorMessage(sErr);
    }
    wsafecpy(sBuf, iSize, sKey);
  }
  else
    wsafecpy(sBuf, iSize, it->second.c_str());
  int i = 0;
  while (sBuf[i] != L'\0')
  {
    if (sBuf[i] >= L'a' && sBuf[i] <= L'z')
      sBuf[i] += L'A' - L'a';
    i++;
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextLow
//  Upper case
// -----------------------------------------------------------------
wchar_t * LocalisationTool::getTextLow(const wchar_t * sKey, wchar_t * sBuf, int iSize)
{
  // Get text in hashmap
  wstr_hash::iterator it = m_sAllTexts.find(sKey);
  if (it == m_sAllTexts.end())
  {
    if (m_pDebug != NULL)
    {
      wchar_t sErr[128];
      swprintf(sErr, 128, L"L12N key not found: %s", sKey);
      m_pDebug->notifyErrorMessage(sErr);
    }
    wsafecpy(sBuf, iSize, sKey);
  }
  else
    wsafecpy(sBuf, iSize, it->second.c_str());
  int i = 0;
  while (sBuf[i] != L'\0')
  {
    if (sBuf[i] >= L'A' && sBuf[i] <= L'Z')
      sBuf[i] += L'a' - L'A';
    i++;
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getTextLow
//  Generate readable text from hashmap, with given hash keys
// -----------------------------------------------------------------
wchar_t * LocalisationTool::long_hashToString(wchar_t * sBuf, int iBufSize, const wchar_t * sSeparator, long_hash * hm, int nArgs, ...)
{
  wchar_t sLine[80];
  wchar_t sTranslatedKey[64];
  wchar_t s2P[8];
  wchar_t sSep[8] = L"";
  va_list pArgs;

  wsafecpy(sBuf, iBufSize, L"");
  i18n->getText(L"2P", s2P, 8);
  va_start(pArgs, nArgs);
  for (int i = 0; i < nArgs; i++)
  {
    wchar_t * sKey = va_arg(pArgs, wchar_t*);
    long_hash::const_iterator it = hm->find(sKey);
//    long_hash::iterator it = hm->find(sKey);
    if (it != hm->end())
    {
      i18n->getText1stUp(sKey, sTranslatedKey, 64);
      swprintf(sLine, 80, L"%s%s%s%ld", sSep, sTranslatedKey, s2P, it->second);
      wsafecat(sBuf, iBufSize, sLine);
      wsafecpy(sSep, 8, sSeparator);
    }
  }
  va_end(pArgs);
  return sBuf;
}
