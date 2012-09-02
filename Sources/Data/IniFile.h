#ifndef _INIFILE_H
#define _INIFILE_H

#include "../utils.h"

#define INI_READER_MAX_CHARS                128
#define INIREADER_ERROR_CANT_OPEN_FILE      0
#define INIREADER_ERROR_MAX_LINES_REACHED   1

class IniFile
{
public:
  // Constructor / destructor
  IniFile(const wchar_t * sFileName, int iMaxLines);
  ~IniFile();

  wchar_t * getValueAt(int idx) { return m_sAllValues[idx]; };
  wchar_t * getKeyAt(int idx) { return m_sAllKeys[idx]; };
  wchar_t * findValue(const wchar_t * sKey);
  const wchar_t * findCharValue(const wchar_t * sKey, const wchar_t * sDefault = L"");
  bool findBoolValue(const wchar_t * sKey, bool bDefault = false);
  int findIntValue(const wchar_t * sKey, int iDefault = 0);
  float findFloatValue(const wchar_t * sKey, float fDefault = 0.0f);
  void write(const wchar_t * sFileName);
  void setKeyAndCharValue(const wchar_t * sKey, const wchar_t * sValue);
  void setKeyAndBoolValue(const wchar_t * sKey, bool bValue);
  void setKeyAndIntValue(const wchar_t * sKey, int iValue);
  void setKeyAndFloatValue(const wchar_t * sKey, float fValue);

private:
  int m_iMaxLines;
  wchar_t ** m_sAllValues;
  wchar_t ** m_sAllKeys;
};

#endif
