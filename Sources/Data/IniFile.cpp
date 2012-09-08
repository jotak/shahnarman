// -----------------------------------------------------------------
// INI FILE
// -----------------------------------------------------------------
#include "IniFile.h"
#include <stdio.h>
#include <stdlib.h>

// -----------------------------------------------------------------
// Name : IniFile
// -----------------------------------------------------------------
IniFile::IniFile(const char * sFileName, int * pError)
{
  FILE * pFile = NULL;
  *pError = 0;

  if (0 != fopen_s(&pFile, sFileName, "r"))
  {
      *pError = INIREADER_ERROR_CANT_OPEN_FILE;
      return;
  }

  int iChar = 0;
  bool bKey = true;
  char sBuf[1024];
  std::string sKey;

  while (!feof(pFile))
  {
    char c = fgetc(pFile);
    if (bKey)
    {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
      {
        sBuf[iChar++] = c;
        sBuf[iChar] = '\0';
      }
      else if (c == '=')
      {
          sKey = std::string(sBuf);
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
          m_hmData[sKey] = std::string(sBuf);
        bKey = true;
        iChar = 0;
      }
      else
      {
        sBuf[iChar++] = c;
        sBuf[iChar] = '\0';
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
    m_hmData.clear();
}

// -----------------------------------------------------------------
// Name : findValue
// -----------------------------------------------------------------
std::string * IniFile::findValue(std::string sKey)
{
    str_hash::iterator it = m_hmData.find(sKey);
    if (it == m_hmData.end())
        return NULL;
    return &(it->second);
}

// -----------------------------------------------------------------
// Name : findCharValue
// -----------------------------------------------------------------
const char * IniFile::findCharValue(std::string sKey, const char * sDefault)
{
  std::string * res = findValue(sKey);
  return (res == NULL ? sDefault : res->c_str());
}

// -----------------------------------------------------------------
// Name : findIntValue
// -----------------------------------------------------------------
int IniFile::findIntValue(std::string sKey, int iDefault)
{
  std::string * res = findValue(sKey);
  return (res == NULL ? iDefault : atoi(res->c_str()));
}

// -----------------------------------------------------------------
// Name : findBoolValue
// -----------------------------------------------------------------
bool IniFile::findBoolValue(std::string sKey, bool bDefault)
{
  std::string * res = findValue(sKey);
  return (res == NULL ? bDefault : (strcmp(res->c_str(), "1") == 0 || strcmp(res->c_str(), "true") == 0));
}

// -----------------------------------------------------------------
// Name : findFloatValue
// -----------------------------------------------------------------
float IniFile::findFloatValue(std::string sKey, float fDefault)
{
  std::string * res = findValue(sKey);
  return (res == NULL ? fDefault : atof(res->c_str()));
}

// -----------------------------------------------------------------
// Name : setKeyAndCharValue
// -----------------------------------------------------------------
void IniFile::setKeyAndCharValue(std::string sKey, std::string sValue)
{
    m_hmData[sKey] = sValue;
}

// -----------------------------------------------------------------
// Name : setKeyAndBoolValue
// -----------------------------------------------------------------
void IniFile::setKeyAndBoolValue(std::string sKey, bool bValue)
{
  setKeyAndCharValue(sKey, std::string(bValue ? "true" : "false"));
}

// -----------------------------------------------------------------
// Name : setKeyAndIntValue
// -----------------------------------------------------------------
void IniFile::setKeyAndIntValue(std::string sKey, int iValue)
{
  char sValue[32];
  snprintf(sValue, 32, "%d", iValue);
  setKeyAndCharValue(sKey, std::string(sValue));
}

// -----------------------------------------------------------------
// Name : setKeyAndFloatValue
// -----------------------------------------------------------------
void IniFile::setKeyAndFloatValue(std::string sKey, float fValue)
{
  char sValue[32];
  snprintf(sValue, 32, "%f", fValue);
  setKeyAndCharValue(sKey, std::string(sValue));
}

// -----------------------------------------------------------------
// Name : write
// -----------------------------------------------------------------
void IniFile::write(const char * sFileName, int * pError)
{
    *pError = 0;
  FILE * pFile = NULL;
  if (0 != fopen_s(&pFile, sFileName, "w"))
  {
      *pError = INIREADER_ERROR_CANT_OPEN_FILE;
      return;
  }

  str_hash::iterator it;
  for (it = m_hmData.begin(); it != m_hmData.end(); ++it)
  {
      fprintf(pFile, "%s = %s\n", it->first.c_str(), it->second.c_str());
  }

  fclose(pFile);
}
