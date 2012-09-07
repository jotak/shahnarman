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
  void addLocalizedElement(const char * sKey, const char * sValue, const char * sLanguage);
  void findLocalizedElement(char * sValue, int iSize, const char * sLanguage, const char * sKey);
  char m_sObjectId[NAME_MAX_CHARS];    // name, language-independant

protected:
  ObjectList * m_pLocalizedElements;

  class LocalizedElement : public BaseObject
  {
  public:
    LocalizedElement(const char * sKey, const char * sValue, const char * sLanguage);
    ~LocalizedElement();
    char * m_sValue;
    char m_sLanguage[64];
    char m_sKey[64];
  };
};

#endif
