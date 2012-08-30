// -----------------------------------------------------------------
// XML OBJECT
// -----------------------------------------------------------------
#include "XMLObject.h"

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
#ifdef DBG_VERBOSE1
  printf("Begin destroy XMLObject\n");
#endif
  delete m_pLocalizedElements;
#ifdef DBG_VERBOSE1
  printf("End destroy XMLObject\n");
#endif
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
  if (wcscmp(sLanguage, L"english") != 0)
    findLocalizedElement(sValue, iSize, L"english", sKey);
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
