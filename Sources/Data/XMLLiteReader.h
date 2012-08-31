#ifndef _XMLLITE_READER_H
#define _XMLLITE_READER_H

#include "XMLLiteElement.h"

#define XMLLITE_ERROR_ELEMENT_EXPECTED              0
#define XMLLITE_ERROR_EOF_NOT_EXPECTED              1
#define XMLLITE_ERROR_LINEBREAK_IN_ELEMENT          2
#define XMLLITE_ERROR_ELEMENT_END_EXPECTED          3
#define XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE        4
#define XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE   5
#define XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH      6
#define XMLLITE_ERROR_CLOSING_TAG_EXPECTED          7
#define XMLLITE_ERROR_CANT_OPEN_FILE                8

class XMLLiteReader
{
public:
  // Constructor / destructor
  XMLLiteReader();
  ~XMLLiteReader();

  XMLLiteElement * parseFile(wchar_t * sFileName);
  int getCurrentLine() { return m_iCurrentLine; };
  int getCurrentCol() { return m_iCurrentCol; };

private:
  bool skipSpaces(FILE * pFile, wint_t * c);
  void skipComments(FILE * pFile);
  void readElement(FILE * pFile, wchar_t cFirstChar, XMLLiteElement * pParent);
  bool readAttribute(FILE * pFile, wchar_t cFirstChar, wint_t * cfinal, XMLLiteElement * pParent);
  void readClosingTag(FILE * pFile, wchar_t * sName);
  void readWordUntil(FILE * pFile, wchar_t cUntil, wchar_t * sWord, int iSize);
  bool readChar(FILE * pFile, wint_t * c);

  XMLLiteElement * m_pRootNode;
  int m_iCurrentLine;
  int m_iCurrentCol;
};

#endif