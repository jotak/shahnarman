// -----------------------------------------------------------------
// XMLLITE ELEMENT (and related classes)
// -----------------------------------------------------------------
#include "XMLLiteElement.h"

// -----------------------------------------------------------------
// Name : getCharValue
// -----------------------------------------------------------------
wchar_t * XMLLiteAttribute::getCharValue()
{
  // return copy of value instead?
  return m_sValue;
}

// -----------------------------------------------------------------
// Name : getIntValue
// -----------------------------------------------------------------
long XMLLiteAttribute::getIntValue()
{
  long value;
  swscanf_s(m_sValue, L"%ld", &value);
  return value;
}

// -----------------------------------------------------------------
// Name : getFloatValue
// -----------------------------------------------------------------
double XMLLiteAttribute::getFloatValue()
{
  double value;
  swscanf_s(m_sValue, L"%lf", &value);
  return value;
}

// -----------------------------------------------------------------
// Name : getCharValue
// -----------------------------------------------------------------
wchar_t * XMLLiteElement::getCharValue()
{
  // return copy of value instead?
  return m_sValue;
}

// -----------------------------------------------------------------
// Name : getIntValue
// -----------------------------------------------------------------
long XMLLiteElement::getIntValue()
{
  long value;
  swscanf_s(m_sValue, L"%ld", &value);
  return value;
}

// -----------------------------------------------------------------
// Name : getFloatValue
// -----------------------------------------------------------------
double XMLLiteElement::getFloatValue()
{
  double value;
  swscanf_s(m_sValue, L"%lf", &value);
  return value;
}

// -----------------------------------------------------------------
// Name : getChildByName
// -----------------------------------------------------------------
XMLLiteElement * XMLLiteElement::getChildByName(wchar_t * sName)
{
  XMLLiteElement * pChild = getFirstChild();
  while (pChild != NULL)
  {
    if (wcscmp(sName, pChild->getName()) == 0)
      return pChild;
    pChild = getNextChild();
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getAttributeByName
// -----------------------------------------------------------------
XMLLiteAttribute * XMLLiteElement::getAttributeByName(wchar_t * sName)
{
  XMLLiteAttribute * pAttr = getFirstAttribute();
  while (pAttr != NULL)
  {
    if (wcscmp(sName, pAttr->getName()) == 0)
      return pAttr;
    pAttr = getNextAttribute();
  }
  return NULL;
}
