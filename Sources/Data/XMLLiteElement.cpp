// -----------------------------------------------------------------
// XMLLITE ELEMENT (and related classes)
// -----------------------------------------------------------------
#include "XMLLiteElement.h"
#include <stdlib.h>

// -----------------------------------------------------------------
// Name : getCharValue
// -----------------------------------------------------------------
char * XMLLiteAttribute::getCharValue()
{
  // return copy of value instead?
  return m_sValue;
}

// -----------------------------------------------------------------
// Name : getIntValue
// -----------------------------------------------------------------
long XMLLiteAttribute::getIntValue()
{
  return atol(m_sValue);
}

// -----------------------------------------------------------------
// Name : getFloatValue
// -----------------------------------------------------------------
double XMLLiteAttribute::getFloatValue()
{
    return atof(m_sValue);
}

// -----------------------------------------------------------------
// Name : getCharValue
// -----------------------------------------------------------------
char * XMLLiteElement::getCharValue()
{
  // return copy of value instead?
  return m_sValue;
}

// -----------------------------------------------------------------
// Name : getIntValue
// -----------------------------------------------------------------
long XMLLiteElement::getIntValue()
{
  return atol(m_sValue);
}

// -----------------------------------------------------------------
// Name : getFloatValue
// -----------------------------------------------------------------
double XMLLiteElement::getFloatValue()
{
    return atof(m_sValue);
}

// -----------------------------------------------------------------
// Name : getChildByName
// -----------------------------------------------------------------
XMLLiteElement * XMLLiteElement::getChildByName(const char * sName)
{
  XMLLiteElement * pChild = getFirstChild();
  while (pChild != NULL)
  {
    if (strcmp(sName, pChild->getName()) == 0)
      return pChild;
    pChild = getNextChild();
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getAttributeByName
// -----------------------------------------------------------------
XMLLiteAttribute * XMLLiteElement::getAttributeByName(const char * sName)
{
  XMLLiteAttribute * pAttr = getFirstAttribute();
  while (pAttr != NULL)
  {
    if (strcmp(sName, pAttr->getName()) == 0)
      return pAttr;
    pAttr = getNextAttribute();
  }
  return NULL;
}
