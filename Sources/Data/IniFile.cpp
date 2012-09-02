// -----------------------------------------------------------------
// INI FILE
// -----------------------------------------------------------------
#include "IniFile.h"
#include <stdio.h>

// -----------------------------------------------------------------
// Name : IniFile
// -----------------------------------------------------------------
IniFile::IniFile(const wchar_t * sFileName, int iMaxLines)
{
  FILE * pFile = NULL;

  m_iMaxLines = iMaxLines;
  m_sAllValues = NULL;
  m_sAllKeys = NULL;

  if (0 != wfopen(&pFile, sFileName, L"r"))
    throw INIREADER_ERROR_CANT_OPEN_FILE;

  m_sAllValues = new wchar_t*[iMaxLines];
  m_sAllKeys = new wchar_t*[iMaxLines];
  for (int i = 0; i < iMaxLines; i++)
  {
    m_sAllValues[i] = new wchar_t[INI_READER_MAX_CHARS];
    m_sAllKeys[i] = new wchar_t[INI_READER_MAX_CHARS];
    wsafecpy(m_sAllValues[i], INI_READER_MAX_CHARS, L"");
    wsafecpy(m_sAllKeys[i], INI_READER_MAX_CHARS, L"");
  }

  int iLine = 0;
  int iChar = 0;
  bool bKey = true;
  while (!feof(pFile))
  {
    wchar_t c = fgetwc(pFile);
    if (bKey)
    {
      if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') || c == L'_')
      {
        m_sAllKeys[iLine][iChar++] = c;
        m_sAllKeys[iLine][iChar] = L'\0';
      }
      else if (c == L'=')
      {
        bKey = false;
        iChar = 0;
      }
    }
    else
    {
      if (iChar == 0 && (c == L' ' || c == '\t'))
        continue;
      if (c == L'\r' || c == '\n')
      {
        bKey = true;
        iChar = 0;
        iLine++;
        if (iLine == iMaxLines)
        {
          fclose(pFile);
          throw INIREADER_ERROR_MAX_LINES_REACHED;
        }
      }
      else
      {
        m_sAllValues[iLine][iChar++] = c;
        m_sAllValues[iLine][iChar] = L'\0';
      }
    }
  }

  fclose(pFile);
}

// -----------------------------------------------------------------
// Name : ~IniFile
// -----------------------------------------------------------------
IniFile::~IniFile()
{
  if (m_sAllValues != NULL)
  {
    for (int i = 0; i < m_iMaxLines; i++)
    {
      if (m_sAllValues[i] != NULL)
        delete[] m_sAllValues[i];
    }
    delete[] m_sAllValues;
  }
  if (m_sAllKeys != NULL)
  {
    for (int i = 0; i < m_iMaxLines; i++)
    {
      if (m_sAllKeys[i] != NULL)
        delete[] m_sAllKeys[i];
    }
    delete[] m_sAllKeys;
  }
}

// -----------------------------------------------------------------
// Name : findValue
// -----------------------------------------------------------------
wchar_t * IniFile::findValue(const wchar_t * sKey)
{
  for (int i = 0; i < m_iMaxLines; i++)
  {
    if (m_sAllKeys[i] != NULL && wcscmp(m_sAllKeys[i], sKey) == 0)
      return m_sAllValues[i];
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findCharValue
// -----------------------------------------------------------------
const wchar_t * IniFile::findCharValue(const wchar_t * sKey, const wchar_t * sDefault)
{
  const wchar_t * res = findValue(sKey);
  return (res == NULL ? sDefault : res);
}

// -----------------------------------------------------------------
// Name : findIntValue
// -----------------------------------------------------------------
int IniFile::findIntValue(const wchar_t * sKey, int iDefault)
{
  wchar_t * res = findValue(sKey);
  if (res == NULL)
    return iDefault;
  int ires = 0;
  swscanf_s(res, L"%d", &ires);
  return ires;
}

// -----------------------------------------------------------------
// Name : findBoolValue
// -----------------------------------------------------------------
bool IniFile::findBoolValue(const wchar_t * sKey, bool bDefault)
{
  wchar_t * res = findValue(sKey);
  if (res == NULL)
    return bDefault;
  return (wcscmp(res, L"1") == 0 || wcscmp(res, L"true") == 0);
}

// -----------------------------------------------------------------
// Name : findFloatValue
// -----------------------------------------------------------------
float IniFile::findFloatValue(const wchar_t * sKey, float fDefault)
{
  wchar_t * res = findValue(sKey);
  if (res == NULL)
    return fDefault;
  float fres = 0;
  swscanf_s(res, L"%f", &fres);
  return fres;
}

// -----------------------------------------------------------------
// Name : setKeyAndCharValue
// -----------------------------------------------------------------
void IniFile::setKeyAndCharValue(const wchar_t * sKey, const wchar_t * sValue)
{
  wchar_t * res = findValue(sKey);
  if (res == NULL)
  {
    for (int i = 0; i < m_iMaxLines; i++)
    {
      if (wcscmp(m_sAllKeys[i], L"") == 0)
      {
        wsafecpy(m_sAllKeys[i], INI_READER_MAX_CHARS, sKey);
        wsafecpy(m_sAllValues[i], INI_READER_MAX_CHARS, sValue);
        break;
      }
    }
  }
  else
    wsafecpy(res, INI_READER_MAX_CHARS, sValue);
}

// -----------------------------------------------------------------
// Name : setKeyAndBoolValue
// -----------------------------------------------------------------
void IniFile::setKeyAndBoolValue(const wchar_t * sKey, bool bValue)
{
  setKeyAndCharValue(sKey, bValue ? L"true" : L"false");
}

// -----------------------------------------------------------------
// Name : setKeyAndIntValue
// -----------------------------------------------------------------
void IniFile::setKeyAndIntValue(const wchar_t * sKey, int iValue)
{
  wchar_t sValue[INI_READER_MAX_CHARS];
  swprintf_s(sValue, INI_READER_MAX_CHARS, L"%d", iValue);
  setKeyAndCharValue(sKey, sValue);
}

// -----------------------------------------------------------------
// Name : setKeyAndFloatValue
// -----------------------------------------------------------------
void IniFile::setKeyAndFloatValue(const wchar_t * sKey, float fValue)
{
  wchar_t sValue[INI_READER_MAX_CHARS];
  swprintf_s(sValue, INI_READER_MAX_CHARS, L"%f", fValue);
  setKeyAndCharValue(sKey, sValue);
}

// -----------------------------------------------------------------
// Name : write
// -----------------------------------------------------------------
void IniFile::write(const wchar_t * sFileName)
{
  FILE * pFile = NULL;
  if (0 != wfopen(&pFile, sFileName, L"w"))
    throw INIREADER_ERROR_CANT_OPEN_FILE;

  for (int i = 0; i < m_iMaxLines; i++)
  {
    if (m_sAllKeys[i] != NULL && wcscmp(m_sAllKeys[i], L"") != 0)
      fwprintf(pFile, L"%s = %s\n", m_sAllKeys[i], m_sAllValues[i]);
  }

  fclose(pFile);
}
