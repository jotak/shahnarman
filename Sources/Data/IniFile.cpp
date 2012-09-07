// -----------------------------------------------------------------
// INI FILE
// -----------------------------------------------------------------
#include "IniFile.h"
#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------
// Name : IniFile
// -----------------------------------------------------------------
IniFile::IniFile(const char * sFileName, int iMaxLines)
{
  FILE * pFile = NULL;

  m_iMaxLines = iMaxLines;
  m_sAllValues = NULL;
  m_sAllKeys = NULL;

  if (0 != fopen_s(&pFile, sFileName, "r"))
    throw INIREADER_ERROR_CANT_OPEN_FILE;

  m_sAllValues = new char*[iMaxLines];
  m_sAllKeys = new char*[iMaxLines];
  for (int i = 0; i < iMaxLines; i++)
  {
    m_sAllValues[i] = new char[INI_READER_MAX_CHARS];
    m_sAllKeys[i] = new char[INI_READER_MAX_CHARS];
    wsafecpy(m_sAllValues[i], INI_READER_MAX_CHARS, "");
    wsafecpy(m_sAllKeys[i], INI_READER_MAX_CHARS, "");
  }

  int iLine = 0;
  int iChar = 0;
  bool bKey = true;
  while (!feof(pFile))
  {
    char c = fgetwc(pFile);
    if (bKey)
    {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
      {
        m_sAllKeys[iLine][iChar++] = c;
        m_sAllKeys[iLine][iChar] = '\0';
      }
      else if (c == '=')
      {
        bKey = false;
        iChar = 0;
      }
    }
    else
    {
      if (iChar == 0 && (c == ' ' || c == '\t'))
        continue;
      if (c == '\r' || c == '\n')
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
        m_sAllValues[iLine][iChar] = '\0';
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
char * IniFile::findValue(const char * sKey)
{
  for (int i = 0; i < m_iMaxLines; i++)
  {
    if (m_sAllKeys[i] != NULL && strcmp(m_sAllKeys[i], sKey) == 0)
      return m_sAllValues[i];
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findCharValue
// -----------------------------------------------------------------
const char * IniFile::findCharValue(const char * sKey, const char * sDefault)
{
  const char * res = findValue(sKey);
  return (res == NULL ? sDefault : res);
}

// -----------------------------------------------------------------
// Name : findIntValue
// -----------------------------------------------------------------
int IniFile::findIntValue(const char * sKey, int iDefault)
{
  char * res = findValue(sKey);
  if (res == NULL)
    return iDefault;
  return atoi(res);
}

// -----------------------------------------------------------------
// Name : findBoolValue
// -----------------------------------------------------------------
bool IniFile::findBoolValue(const char * sKey, bool bDefault)
{
  char * res = findValue(sKey);
  if (res == NULL)
    return bDefault;
  return (strcmp(res, "1") == 0 || strcmp(res, "true") == 0);
}

// -----------------------------------------------------------------
// Name : findFloatValue
// -----------------------------------------------------------------
float IniFile::findFloatValue(const char * sKey, float fDefault)
{
  char * res = findValue(sKey);
  if (res == NULL)
    return fDefault;
  return atof(res);
}

// -----------------------------------------------------------------
// Name : setKeyAndCharValue
// -----------------------------------------------------------------
void IniFile::setKeyAndCharValue(const char * sKey, const char * sValue)
{
  char * res = findValue(sKey);
  if (res == NULL)
  {
    for (int i = 0; i < m_iMaxLines; i++)
    {
      if (strcmp(m_sAllKeys[i], "") == 0)
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
void IniFile::setKeyAndBoolValue(const char * sKey, bool bValue)
{
  setKeyAndCharValue(sKey, bValue ? "true" : "false");
}

// -----------------------------------------------------------------
// Name : setKeyAndIntValue
// -----------------------------------------------------------------
void IniFile::setKeyAndIntValue(const char * sKey, int iValue)
{
  char sValue[INI_READER_MAX_CHARS];
  snprintf(sValue, INI_READER_MAX_CHARS, "%d", iValue);
  setKeyAndCharValue(sKey, sValue);
}

// -----------------------------------------------------------------
// Name : setKeyAndFloatValue
// -----------------------------------------------------------------
void IniFile::setKeyAndFloatValue(const char * sKey, float fValue)
{
  char sValue[INI_READER_MAX_CHARS];
  snprintf(sValue, INI_READER_MAX_CHARS, "%f", fValue);
  setKeyAndCharValue(sKey, sValue);
}

// -----------------------------------------------------------------
// Name : write
// -----------------------------------------------------------------
void IniFile::write(const char * sFileName)
{
  FILE * pFile = NULL;
  if (0 != fopen_s(&pFile, sFileName, "w"))
    throw INIREADER_ERROR_CANT_OPEN_FILE;

  for (int i = 0; i < m_iMaxLines; i++)
  {
    if (m_sAllKeys[i] != NULL && strcmp(m_sAllKeys[i], "") != 0)
      fprintf(pFile, "%s = %s\n", m_sAllKeys[i], m_sAllValues[i]);
  }

  fclose(pFile);
}
