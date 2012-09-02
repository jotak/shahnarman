#ifndef _XML_OBJECT_H
#define _XML_OBJECT_H

#include "../Common/ObjectList.h"

class XMLLiteElement;

class XMLObject : public BaseObject
{
public:
  XMLObject();
  ~XMLObject();

  void readLocalizedElementsFromXml(XMLLiteElement * pNode);
  void addLocalizedElement(const wchar_t * sKey, const wchar_t * sValue, const wchar_t * sLanguage);
  void findLocalizedElement(wchar_t * sValue, int iSize, const wchar_t * sLanguage, const wchar_t * sKey);
  wchar_t m_sObjectId[NAME_MAX_CHARS];    // name, language-independant

protected:
  ObjectList * m_pLocalizedElements;

  class LocalizedElement : public BaseObject
  {
  public:
    LocalizedElement(const wchar_t * sKey, const wchar_t * sValue, const wchar_t * sLanguage);
    ~LocalizedElement();
    wchar_t * m_sValue;
    wchar_t m_sLanguage[64];
    wchar_t m_sKey[64];
  };
};

#endif
