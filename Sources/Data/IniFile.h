#ifndef _INIFILE_H
#define _INIFILE_H

#include "../utils.h"

#define INIREADER_ERROR_CANT_OPEN_FILE      1

class IniFile
{
public:
    // Constructor / destructor
    IniFile(const char * sFileName, int * pError);
    ~IniFile();

    std::string * findValue(std::string sKey);
    const char * findCharValue(std::string sKey, const char * sDefault = "");
    bool findBoolValue(std::string sKey, bool bDefault = false);
    int findIntValue(std::string sKey, int iDefault = 0);
    float findFloatValue(std::string sKey, float fDefault = 0.0f);
    void write(const char * sFileName, int * pError);
    void setKeyAndCharValue(std::string sKey, std::string sValue);
    void setKeyAndBoolValue(std::string sKey, bool bValue);
    void setKeyAndIntValue(std::string sKey, int iValue);
    void setKeyAndFloatValue(std::string sKey, float fValue);

private:
    str_hash m_hmData;
};

#endif
