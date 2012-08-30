#ifndef _XMLLITE_ELEMENT_H
#define _XMLLITE_ELEMENT_H

#include "../Common/ObjectList.h"

#define XMLLITE_MAX_NAME_CHARS      64
#define XMLLITE_MAX_VALUE_CHARS     1024
#define XMLLITE_ELEMENT_TYPE_NODE   0
#define XMLLITE_ELEMENT_TYPE_VALUE  1

class XMLLiteAttribute : public BaseObject
{
public:
  // Constructor / destructor
  XMLLiteAttribute() { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, L""); wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, L""); };
  ~XMLLiteAttribute() {};

  wchar_t * getName() { return m_sName; };
  wchar_t * getCharValue();
  long getIntValue();
  double getFloatValue();
  void setName(wchar_t * sName) { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, sName); };
  void setValue(wchar_t * sValue) { wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, sValue); };

private:
  wchar_t m_sName[XMLLITE_MAX_NAME_CHARS];
  wchar_t m_sValue[XMLLITE_MAX_VALUE_CHARS];
};

class XMLLiteElement : public BaseObject
{
public:
  // Constructor / destructor
  XMLLiteElement(short iType) { m_iType = iType; wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, L""); wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, L""); m_pChildren = new ObjectList(true); m_pAttributes = new ObjectList(true); };
  ~XMLLiteElement() { delete m_pChildren; delete m_pAttributes; };

  short getType() { return m_iType; };
  void setType(short iType) { m_iType = iType; };
  wchar_t * getName() { return m_sName; };
  void setName(wchar_t * sName) { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, sName); };
  void setValue(wchar_t * sValue) { wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, sValue); };
  wchar_t * getCharValue();
  long getIntValue();
  double getFloatValue();
  XMLLiteElement * getFirstChild() { return (XMLLiteElement*) m_pChildren->getFirst(0); };
  XMLLiteElement * getNextChild() { return (XMLLiteElement*) m_pChildren->getNext(0); };
  XMLLiteElement * getLastChild() { return (XMLLiteElement*) m_pChildren->getLast(0); };
  XMLLiteElement * getPrevChild() { return (XMLLiteElement*) m_pChildren->getPrev(0); };
  void addChild(XMLLiteElement * pChild) { m_pChildren->addLast(pChild); };
  XMLLiteElement * getChildByName(wchar_t * sName);
  XMLLiteAttribute * getFirstAttribute() { return (XMLLiteAttribute*) m_pAttributes->getFirst(0); };
  XMLLiteAttribute * getNextAttribute() { return (XMLLiteAttribute*) m_pAttributes->getNext(0); };
  void addAttribute(XMLLiteAttribute * pAttr) { m_pAttributes->addLast(pAttr); };
  XMLLiteAttribute * getAttributeByName(wchar_t * sName);

private:
  wchar_t m_sName[XMLLITE_MAX_NAME_CHARS];
  wchar_t m_sValue[XMLLITE_MAX_VALUE_CHARS];
  ObjectList * m_pChildren;
  ObjectList * m_pAttributes;
  short m_iType;
};

#endif
