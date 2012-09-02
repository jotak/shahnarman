// -----------------------------------------------------------------
// XMLLITE READER
// -----------------------------------------------------------------
#include "XMLLiteReader.h"
#include <stdio.h>

// -----------------------------------------------------------------
// Name : XMLLiteReader
// -----------------------------------------------------------------
XMLLiteReader::XMLLiteReader()
{
  m_pRootNode = NULL;
  m_iCurrentLine = m_iCurrentCol = 0;
}

// -----------------------------------------------------------------
// Name : ~XMLLiteReader
// -----------------------------------------------------------------
XMLLiteReader::~XMLLiteReader()
{
  if (m_pRootNode != NULL)
    delete m_pRootNode;
}

// -----------------------------------------------------------------
// Name : parseFile
// -----------------------------------------------------------------
XMLLiteElement * XMLLiteReader::parseFile(wchar_t * sFileName)
{
  FILE * pFile = NULL;

  if (0 != wfopen(&pFile, sFileName, L"r"))
    throw XMLLITE_ERROR_CANT_OPEN_FILE;

  if (m_pRootNode != NULL)
    delete m_pRootNode;
  m_pRootNode = new XMLLiteElement(XMLLITE_ELEMENT_TYPE_NODE);
  m_pRootNode->setName(sFileName);

  try {
    m_iCurrentLine = m_iCurrentCol = 1;
    while (true)
    {
      wint_t c;
      bool bEscape = skipSpaces(pFile, &c);
      if (c == WEOF) // EOF
        break;
      else if (c != (wint_t)L'<' || bEscape)
        throw XMLLITE_ERROR_ELEMENT_EXPECTED;
      bEscape = skipSpaces(pFile, &c);
      readElement(pFile, (wchar_t)c, m_pRootNode);
    }
  }
  catch (int errorCode)
  {
    fclose(pFile);
    throw errorCode;
  }
  fclose(pFile);

  return m_pRootNode;
}

// -----------------------------------------------------------------
// Name : readElement
// -----------------------------------------------------------------
void XMLLiteReader::readElement(FILE * pFile, wchar_t cFirstChar, XMLLiteElement * pParent)
{
  wint_t c;
  wchar_t sName[XMLLITE_MAX_NAME_CHARS] = L"";
  int cursor = 1;
  sName[0] = cFirstChar;
  bool bEscape;

  while (true)
  {
    bEscape = readChar(pFile, &c);
    m_iCurrentCol++;
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    else if (c == (wint_t)L'\r' || c == (wint_t)L'\n')
      throw XMLLITE_ERROR_LINEBREAK_IN_ELEMENT;
    else if (c == (wint_t)L' ' || c == (wint_t)L'\t' || (c == (wint_t)L'/' && !bEscape) || (c == (wint_t)L'>' && !bEscape))
      break;
    else
      sName[cursor++] = (wchar_t)c;
    if (cursor == 3)
    {
      if (sName[0] == L'!' && sName[1] == L'-' && sName[2] == L'-')
      {
        skipComments(pFile);
        return;
      }
    }
  }
  sName[cursor] = L'\0';
  if (c == (wint_t)L' ' || c == (wint_t)L'\t')
    bEscape = skipSpaces(pFile, &c);

  XMLLiteElement * pElt = new XMLLiteElement(XMLLITE_ELEMENT_TYPE_NODE);  // we don't know yet if it's a node or a value. In case of value, we'll change it later
  pElt->setName(sName);
  pParent->addChild(pElt);
  while (bEscape || (c != (wint_t)L'/' && c != (wint_t)L'>'))   // it's attributes
  {
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    bEscape = readAttribute(pFile, c, &c, pElt);
  }

  if (c == (wint_t)L'/' && !bEscape)
  {
    bEscape = skipSpaces(pFile, &c);
    if (c != (wint_t)L'>' || bEscape)
      throw XMLLITE_ERROR_ELEMENT_END_EXPECTED;
    pElt->setType(XMLLITE_ELEMENT_TYPE_VALUE);
    pElt->setValue(L"");
    return;
  }

  bEscape = skipSpaces(pFile, &c);
  if (c == WEOF)
    throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
  else if (c == (wint_t)L'<' && !bEscape)
  {
    while (true)
    {
      // closing tag?
      bEscape = skipSpaces(pFile, &c);
      if (c == WEOF)
        throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
      else if (c == (wint_t)L'/' && !bEscape)
      {
        readClosingTag(pFile, sName);
        return;
      }
      else
      {
        // it's a node ; so, read child elements
        readElement(pFile, (wchar_t)c, pElt);
        bEscape = skipSpaces(pFile, &c);
        if (c == WEOF)
          throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
        if (c != (wint_t)L'<' || bEscape)
          throw XMLLITE_ERROR_ELEMENT_EXPECTED;
      }
    }
  }
  else
  {
    // it's a value ; read it
    pElt->setType(XMLLITE_ELEMENT_TYPE_VALUE);
    wchar_t sValue[XMLLITE_MAX_VALUE_CHARS] = L"";
    cursor = 1;
    sValue[0] = (wchar_t)c;
    while (true)
    {
      bEscape = readChar(pFile, &c);
      m_iCurrentCol++;
      if (c == WEOF)
        throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
      else if (c == (wint_t)L'<' && !bEscape)
        break;
      sValue[cursor++] = (wchar_t)c;
    }
    sValue[cursor] = L'\0';
    pElt->setValue(sValue);

    // closing tag
    bEscape = skipSpaces(pFile, &c);
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    else if (c != (wint_t)L'/' || bEscape)
      throw XMLLITE_ERROR_CLOSING_TAG_EXPECTED;
    else
    {
      readClosingTag(pFile, sName);
      return;
    }
  }
}

// -----------------------------------------------------------------
// Name : readAttribute
// -----------------------------------------------------------------
bool XMLLiteReader::readAttribute(FILE * pFile, wchar_t cFirstChar, wint_t * cfinal, XMLLiteElement * pParent)
{
  wint_t c;
  wchar_t sName[XMLLITE_MAX_NAME_CHARS] = L"";
  wchar_t sValue[XMLLITE_MAX_VALUE_CHARS] = L"";
  int cursor = 1;
  sName[0] = cFirstChar;
  bool bEscape;

  while (true)
  {
    bEscape = readChar(pFile, &c);
    m_iCurrentCol++;
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    else if (c == (wint_t)L'\r' || c == (wint_t)L'\n')
      throw XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE;
    else if (c == (wint_t)L' ' || c == (wint_t)L'\t' || (c == (wint_t)L'=' && !bEscape))
      break;
    else
      sName[cursor++] = (wchar_t)c;
  }
  sName[cursor] = L'\0';
  if (c == (wint_t)L' ' || c == (wint_t)L'\t')
    bEscape = skipSpaces(pFile, &c);

  if (c != (wint_t)L'=' || bEscape)
    throw XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE;

  bEscape = skipSpaces(pFile, &c);
  cursor = 0;
  if (c == (wint_t)L'\'' && !bEscape)
  {
    readWordUntil(pFile, L'\'', sValue, XMLLITE_MAX_VALUE_CHARS);
    bEscape = skipSpaces(pFile, &c);
  }
  else if (c == (wint_t)L'\"' && !bEscape)
  {
    readWordUntil(pFile, L'\"', sValue, XMLLITE_MAX_VALUE_CHARS);
    bEscape = skipSpaces(pFile, &c);
  }
  else
  {
    while (true)
    {
      if (c == WEOF)
        throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
      else if (c == (wint_t)L'\r' || c == (wint_t)L'\n')
        throw XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE;
      else if (c == (wint_t)L' ' || c == (wint_t)L'\t' || (c == (wint_t)L'/' && !bEscape) || (c == (wint_t)L'>' && !bEscape))
        break;
      sValue[cursor++] = (wchar_t)c;
      bEscape = readChar(pFile, &c);
      m_iCurrentCol++;
    }
    sValue[cursor] = L'\0';
    if (c == (wint_t)L' ' || c == (wint_t)L'\t')
      bEscape = skipSpaces(pFile, &c);
  }

  XMLLiteAttribute * pAttr = new XMLLiteAttribute();
  pAttr->setName(sName);
  pAttr->setValue(sValue);
  pParent->addAttribute(pAttr);

  *cfinal = c;
  return bEscape;
}

// -----------------------------------------------------------------
// Name : readClosingTag
// -----------------------------------------------------------------
void XMLLiteReader::readClosingTag(FILE * pFile, wchar_t * sName)
{
  wint_t c;
  bool bEscape = skipSpaces(pFile, &c);
  int cursor = 1;
  bool bMatch = ((wint_t)(sName[0]) == c);
  while (bMatch)
  {
    bEscape = readChar(pFile, &c);
    m_iCurrentCol++;
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    else if (c == (wint_t)L'\r' || c == (wint_t)L'\n')
      throw XMLLITE_ERROR_LINEBREAK_IN_ELEMENT;
    else if (c == (wint_t)L' ' || c == (wint_t)L'\t' || (c == (wint_t)L'>' && !bEscape))
      break;
    else
      bMatch = ((wint_t)(sName[cursor++]) == c);
  }
  if (c == (wint_t)L' ' || c == (wint_t)L'\t')
    bEscape = skipSpaces(pFile, &c);
  if (c != (wint_t)L'>' || bEscape)
    throw XMLLITE_ERROR_ELEMENT_END_EXPECTED;
  if (!bMatch)
    throw XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH;
}

// -----------------------------------------------------------------
// Name : readWordUntil
// -----------------------------------------------------------------
void XMLLiteReader::readWordUntil(FILE * pFile, wchar_t cUntil, wchar_t * sWord, int iSize)
{
  wint_t c;
  int cursor = 0;
  bool b = readChar(pFile, &c);
  while (c != (wint_t)cUntil || b)
  {
    m_iCurrentCol++;
    if (c == WEOF)
      throw XMLLITE_ERROR_EOF_NOT_EXPECTED;
    sWord[cursor++] = (wchar_t)c;
    if (cursor >= iSize)
    {
      m_iCurrentCol--;
      cursor--;
      break;
    }
    b = readChar(pFile, &c);
  }
  m_iCurrentCol++;
  sWord[cursor] = '\0';
}

// -----------------------------------------------------------------
// Name : skipSpaces
// -----------------------------------------------------------------
bool XMLLiteReader::skipSpaces(FILE * pFile, wint_t * c)
{
  bool bEscape;
  while (true)
  {
    bEscape = readChar(pFile, c);
#ifdef WIN32
    if (*c == (wint_t)L'\n' && !bEscape)
    {
      m_iCurrentLine++;
      m_iCurrentCol = 1;
    }
    else if ((*c == (wint_t)L'\r' || *c == (wint_t)L'\t' || *c == (wint_t)L' ') && !bEscape)
      m_iCurrentCol++;
#endif
#ifdef UNIX
    if (*c == (wint_t)L'\n' && !bEscape)
    {
      m_iCurrentLine++;
      m_iCurrentCol = 1;
    }
    else if ((*c == (wint_t)L'\r' || *c == (wint_t)L'\t' || *c == (wint_t)L' ') && !bEscape)
      m_iCurrentCol++;
#endif
#ifdef LINUX
    if (*c == (wint_t)L'\n' && !bEscape)
    {
      m_iCurrentLine++;
      m_iCurrentCol = 1;
    }
    else if ((*c == (wint_t)L'\r' || *c == (wint_t)L'\t' || *c == (wint_t)L' ') && !bEscape)
      m_iCurrentCol++;
#endif
#ifdef MACOS
    if (*c == (wint_t)L'\r' && !bEscape)
    {
      m_iCurrentLine++;
      m_iCurrentCol = 1;
    }
    else if ((*c == (wint_t)L'\n' || *c == (wint_t)L'\t' || *c == (wint_t)L' ') && !bEscape)
      m_iCurrentCol++;
#endif
    else
    {
      m_iCurrentCol++;
      break;
    }
  }
  return bEscape;
}

// -----------------------------------------------------------------
// Name : skipComments
// -----------------------------------------------------------------
void XMLLiteReader::skipComments(FILE * pFile)
{
  wint_t c;
  int endcomment = 0;
  while (true)
  {
    skipSpaces(pFile, &c);
    if (c == (wint_t)L'-')
    {
      if (endcomment == 0 || endcomment == 1)
        endcomment++;
    }
    else if (c == (wint_t)L'>' && endcomment == 2)
      break;
    else
      endcomment = 0;
  }
}

// -----------------------------------------------------------------
// Name : readChar
//  return true if character was a special character (escape sequence "\")
// -----------------------------------------------------------------
bool XMLLiteReader::readChar(FILE * pFile, wint_t * c)
{
  *c = fgetwc(pFile);
  if (*c == (wint_t)L'\\')
  {
    *c = fgetwc(pFile);
    return true;
  }
  return false;
}
