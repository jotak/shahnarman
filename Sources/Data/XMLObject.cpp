// -----------------------------------------------------------------
// XML OBJECT
// -----------------------------------------------------------------
#include "XMLObject.h"
#include "XMLLiteElement.h"

// -----------------------------------------------------------------
// Name : XMLObject
//  Constructor
// -----------------------------------------------------------------
XMLObject::XMLObject()
{
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, L"");
  m_pLocalizedElements = new ObjectList(true);
}

// -----------------------------------------------------------------
// Name : ~XMLObject
//  Destructor
// -----------------------------------------------------------------
XMLObject::~XMLObject()
{
  delete m_pLocalizedElements;
}

// -----------------------------------------------------------------
// Name : readLocalizedElementsFromXml
// -----------------------------------------------------------------
void XMLObject::readLocalizedElementsFromXml(XMLLiteElement * pNode)
{
  XMLLiteElement * pSubNode = pNode->getFirstChild();
  while (pSubNode != NULL) {
    if (_wcsicmp(pSubNode->getName(), L"l12n") == 0) {
      XMLLiteAttribute * pKey = pSubNode->getAttributeByName(L"key");
      if (pKey != NULL && wcslen(pKey->getCharValue()) > 0) {
        XMLLiteElement * pLang = pSubNode->getFirstChild();
        while (pLang != NULL) {
          addLocalizedElement(pKey->getCharValue(), pLang->getCharValue(), pLang->getName());
          pLang = pSubNode->getNextChild();
        }
      }
    }
    pSubNode = pNode->getNextChild();
  }
}

// -----------------------------------------------------------------
// Name : addLocalizedElement
// -----------------------------------------------------------------
void XMLObject::addLocalizedElement(const wchar_t * sKey, wchar_t * sValue, wchar_t * sLanguage)
{
  LocalizedElement * pLocElt = new LocalizedElement(sKey, sValue, sLanguage);
  m_pLocalizedElements->addLast(pLocElt);
}

// -----------------------------------------------------------------
// Name : findLocalizedElement
// -----------------------------------------------------------------
void XMLObject::findLocalizedElement(wchar_t * sValue, int iSize, wchar_t * sLanguage, wchar_t * sKey)
{
  wsafecpy(sValue, iSize, L"");
  LocalizedElement * pElt = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
  while (pElt != NULL)
  {
    if (_wcsicmp(sLanguage, pElt->m_sLanguage) == 0 && _wcsicmp(sKey, pElt->m_sKey) == 0)
    {
      wsafecpy(sValue, iSize, pElt->m_sValue);
      return;
    }
    pElt = (LocalizedElement*) m_pLocalizedElements->getNext(0);
  }
  // Not found; try english
  pElt = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
  while (pElt != NULL)
  {
    if (_wcsicmp(L"english", pElt->m_sLanguage) == 0 && _wcsicmp(sKey, pElt->m_sKey) == 0)
    {
      wsafecpy(sValue, iSize, pElt->m_sValue);
      return;
    }
    pElt = (LocalizedElement*) m_pLocalizedElements->getNext(0);
  }
  // Not found; take first available
  pElt = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
  while (pElt != NULL)
  {
    if (_wcsicmp(sKey, pElt->m_sKey) == 0)
    {
      wsafecpy(sValue, iSize, pElt->m_sValue);
      return;
    }
    pElt = (LocalizedElement*) m_pLocalizedElements->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : LocalizedElement
//  constructor
// -----------------------------------------------------------------
XMLObject::LocalizedElement::LocalizedElement(const wchar_t * sKey, wchar_t * sValue, wchar_t * sLanguage)
{
  int length = (int) wcslen(sValue);
  m_sValue = new wchar_t[length + 1];
  wsafecpy(m_sValue, length + 1, sValue);
  wsafecpy(m_sKey, 64, sKey);
  wsafecpy(m_sLanguage, 64, sLanguage);
}

// -----------------------------------------------------------------
// Name : ~LocalizedElement
//  destructor
// -----------------------------------------------------------------
XMLObject::LocalizedElement::~LocalizedElement()
{
  delete[] m_sValue;
}
