#ifndef _XMLLITE_READER_H
#define _XMLLITE_READER_H

#include "XMLLiteElement.h"

#define XMLLITE_ERROR_EOF_NOT_EXPECTED              1
#define XMLLITE_ERROR_LINEBREAK_IN_ELEMENT          2
#define XMLLITE_ERROR_ELEMENT_END_EXPECTED          3
#define XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE        4
#define XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE   5
#define XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH      6
#define XMLLITE_ERROR_CLOSING_TAG_EXPECTED          7
#define XMLLITE_ERROR_CANT_OPEN_FILE                8
#define XMLLITE_ERROR_ELEMENT_EXPECTED              9

class XMLLiteReader
{
public:
  // Constructor / destructor
  XMLLiteReader();
  ~XMLLiteReader();

  XMLLiteElement * parseFile(const char * sFileName, int * pError);
  int getCurrentLine() { return m_iCurrentLine; };
  int getCurrentCol() { return m_iCurrentCol; };

private:
  bool skipSpaces(FILE * pFile, int * c);
  void skipComments(FILE * pFile);
  void readElement(FILE * pFile, char cFirstChar, XMLLiteElement * pParent, int * pError);
  bool readAttribute(FILE * pFile, char cFirstChar, int * cfinal, XMLLiteElement * pParent, int * pError);
  void readClosingTag(FILE * pFile, char * sName, int * pError);
  void readWordUntil(FILE * pFile, char cUntil, char * sWord, int iSize, int * pError);
  bool readChar(FILE * pFile, int * c);

  XMLLiteElement * m_pRootNode;
  int m_iCurrentLine;
  int m_iCurrentCol;
};

#endif
