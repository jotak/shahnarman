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
  XMLLiteAttribute() { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, ""); wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, ""); };
  ~XMLLiteAttribute() {};

  char * getName() { return m_sName; };
  char * getCharValue();
  long getIntValue();
  double getFloatValue();
  void setName(const char * sName) { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, sName); };
  void setValue(const char * sValue) { wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, sValue); };

private:
  char m_sName[XMLLITE_MAX_NAME_CHARS];
  char m_sValue[XMLLITE_MAX_VALUE_CHARS];
};

class XMLLiteElement : public BaseObject
{
public:
  // Constructor / destructor
  XMLLiteElement(short iType) { m_iType = iType; wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, ""); wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, ""); m_pChildren = new ObjectList(true); m_pAttributes = new ObjectList(true); };
  ~XMLLiteElement() { delete m_pChildren; delete m_pAttributes; };

  short getType() { return m_iType; };
  void setType(short iType) { m_iType = iType; };
  char * getName() { return m_sName; };
  void setName(const char * sName) { wsafecpy(m_sName, XMLLITE_MAX_NAME_CHARS, sName); };
  void setValue(const char * sValue) { wsafecpy(m_sValue, XMLLITE_MAX_VALUE_CHARS, sValue); };
  char * getCharValue();
  long getIntValue();
  double getFloatValue();
  XMLLiteElement * getFirstChild() { return (XMLLiteElement*) m_pChildren->getFirst(0); };
  XMLLiteElement * getNextChild() { return (XMLLiteElement*) m_pChildren->getNext(0); };
  XMLLiteElement * getLastChild() { return (XMLLiteElement*) m_pChildren->getLast(0); };
  XMLLiteElement * getPrevChild() { return (XMLLiteElement*) m_pChildren->getPrev(0); };
  void addChild(XMLLiteElement * pChild) { m_pChildren->addLast(pChild); };
  XMLLiteElement * getChildByName(const char * sName);
  XMLLiteAttribute * getFirstAttribute() { return (XMLLiteAttribute*) m_pAttributes->getFirst(0); };
  XMLLiteAttribute * getNextAttribute() { return (XMLLiteAttribute*) m_pAttributes->getNext(0); };
  void addAttribute(XMLLiteAttribute * pAttr) { m_pAttributes->addLast(pAttr); };
  XMLLiteAttribute * getAttributeByName(const char * sName);

private:
  char m_sName[XMLLITE_MAX_NAME_CHARS];
  char m_sValue[XMLLITE_MAX_VALUE_CHARS];
  ObjectList * m_pChildren;
  ObjectList * m_pAttributes;
  short m_iType;
};

#endif
