#ifndef _XML_OBJECT_H
#define _XML_OBJECT_H

#include "../Common/ObjectList.h"

class XMLObject : public BaseObject
{
public:
  XMLObject();
  ~XMLObject();

  void addLocalizedElement(const wchar_t * sKey, wchar_t * sValue, wchar_t * sLanguage);
  void findLocalizedElement(wchar_t * sValue, int iSize, wchar_t * sLanguage, wchar_t * sKey);
  wchar_t m_sObjectId[NAME_MAX_CHARS];    // name, language-independant

protected:
  ObjectList * m_pLocalizedElements;

  class LocalizedElement : public BaseObject
  {
  public:
    LocalizedElement(const wchar_t * sKey, wchar_t * sValue, wchar_t * sLanguage);
    ~LocalizedElement();
    wchar_t * m_sValue;
    wchar_t m_sLanguage[64];
    wchar_t m_sKey[64];
  };
};

#endif
