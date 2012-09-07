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
  IniFile(const char * sFileName, int iMaxLines);
  ~IniFile();

  char * getValueAt(int idx) { return m_sAllValues[idx]; };
  char * getKeyAt(int idx) { return m_sAllKeys[idx]; };
  char * findValue(const char * sKey);
  const char * findCharValue(const char * sKey, const char * sDefault = "");
  bool findBoolValue(const char * sKey, bool bDefault = false);
  int findIntValue(const char * sKey, int iDefault = 0);
  float findFloatValue(const char * sKey, float fDefault = 0.0f);
  void write(const char * sFileName);
  void setKeyAndCharValue(const char * sKey, const char * sValue);
  void setKeyAndBoolValue(const char * sKey, bool bValue);
  void setKeyAndIntValue(const char * sKey, int iValue);
  void setKeyAndFloatValue(const char * sKey, float fValue);

private:
  int m_iMaxLines;
  char ** m_sAllValues;
  char ** m_sAllKeys;
};

#endif
