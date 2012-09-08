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
XMLLiteElement * XMLLiteReader::parseFile(const char * sFileName, int * pError)
{
    *pError = 0;
    FILE * pFile = NULL;

    if (0 != fopen_s(&pFile, sFileName, "r"))
    {
        *pError = XMLLITE_ERROR_CANT_OPEN_FILE;
        return NULL;
    }

    if (m_pRootNode != NULL)
        delete m_pRootNode;
    m_pRootNode = new XMLLiteElement(XMLLITE_ELEMENT_TYPE_NODE);
    m_pRootNode->setName(sFileName);

    m_iCurrentLine = m_iCurrentCol = 1;
    while (true)
    {
        int c;
        bool bEscape = skipSpaces(pFile, &c);
        if (c == EOF) // EOF
            break;
        else if (c != (int)'<' || bEscape)
        {
            *pError = XMLLITE_ERROR_ELEMENT_EXPECTED;
            delete m_pRootNode;
            m_pRootNode = NULL;
            break;
        }
        bEscape = skipSpaces(pFile, &c);
        readElement(pFile, (char)c, m_pRootNode, pError);
        if (*pError != 0)
        {
            delete m_pRootNode;
            m_pRootNode = NULL;
            break;
        }
    }
    fclose(pFile);

    return m_pRootNode;
}

// -----------------------------------------------------------------
// Name : readElement
// -----------------------------------------------------------------
void XMLLiteReader::readElement(FILE * pFile, char cFirstChar, XMLLiteElement * pParent, int * pError)
{
    *pError = 0;
    int c;
    char sName[XMLLITE_MAX_NAME_CHARS] = "";
    int cursor = 1;
    sName[0] = cFirstChar;
    bool bEscape;

    while (true)
    {
        bEscape = readChar(pFile, &c);
        m_iCurrentCol++;
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return;
        }
        else if (c == (int)'\r' || c == (int)'\n')
        {
            *pError = XMLLITE_ERROR_LINEBREAK_IN_ELEMENT;
            return;
        }
        else if (c == (int)' ' || c == (int)'\t' || (c == (int)'/' && !bEscape) || (c == (int)'>' && !bEscape))
            break;
        else
            sName[cursor++] = (char)c;
        if (cursor == 3)
        {
            if (sName[0] == '!' && sName[1] == '-' && sName[2] == '-')
            {
                skipComments(pFile);
                return;
            }
        }
    }
    sName[cursor] = '\0';
    if (c == (int)' ' || c == (int)'\t')
        bEscape = skipSpaces(pFile, &c);

    XMLLiteElement * pElt = new XMLLiteElement(XMLLITE_ELEMENT_TYPE_NODE);  // we don't know yet if it's a node or a value. In case of value, we'll change it later
    pElt->setName(sName);
    pParent->addChild(pElt);
    while (bEscape || (c != (int)'/' && c != (int)'>'))   // it's attributes
    {
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return;
        }
        bEscape = readAttribute(pFile, c, &c, pElt, pError);
        if (*pError != 0)
            return;
    }

    if (c == (int)'/' && !bEscape)
    {
        bEscape = skipSpaces(pFile, &c);
        if (c != (int)'>' || bEscape)
        {
            *pError = XMLLITE_ERROR_ELEMENT_END_EXPECTED;
            return;
        }
        pElt->setType(XMLLITE_ELEMENT_TYPE_VALUE);
        pElt->setValue("");
        return;
    }

    bEscape = skipSpaces(pFile, &c);
    if (c == EOF)
    {
        *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
        return;
    }
    else if (c == (int)'<' && !bEscape)
    {
        while (true)
        {
            // closing tag?
            bEscape = skipSpaces(pFile, &c);
            if (c == EOF)
            {
                *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
                return;
            }
            else if (c == (int)'/' && !bEscape)
            {
                readClosingTag(pFile, sName, pError);
                return;
            }
            else
            {
                // it's a node ; so, read child elements
                readElement(pFile, (char)c, pElt, pError);

                if (*pError != 0)
                    return;
                bEscape = skipSpaces(pFile, &c);
                if (c == EOF)
                {
                    *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
                    return;
                }
                if (c != (int)'<' || bEscape)
                {
                    *pError = XMLLITE_ERROR_ELEMENT_EXPECTED;
                    return;
                }
            }
        }
    }
    else
    {
        // it's a value ; read it
        pElt->setType(XMLLITE_ELEMENT_TYPE_VALUE);
        char sValue[XMLLITE_MAX_VALUE_CHARS] = "";
        cursor = 1;
        sValue[0] = (char)c;
        while (true)
        {
            bEscape = readChar(pFile, &c);
            m_iCurrentCol++;
            if (c == EOF)
            {
                *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
                return;
            }
            else if (c == (int)'<' && !bEscape)
                break;
            sValue[cursor++] = (char)c;
        }
        sValue[cursor] = '\0';
        pElt->setValue(sValue);

        // closing tag
        bEscape = skipSpaces(pFile, &c);
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return;
        }
        else if (c != (int)'/' || bEscape)
        {
            *pError = XMLLITE_ERROR_CLOSING_TAG_EXPECTED;
            return;
        }
        else
        {
            readClosingTag(pFile, sName, pError);
            return;
        }
    }
}

// -----------------------------------------------------------------
// Name : readAttribute
// -----------------------------------------------------------------
bool XMLLiteReader::readAttribute(FILE * pFile, char cFirstChar, int * cfinal, XMLLiteElement * pParent, int * pError)
{
    *pError = 0;
    int c;
    char sName[XMLLITE_MAX_NAME_CHARS] = "";
    char sValue[XMLLITE_MAX_VALUE_CHARS] = "";
    int cursor = 1;
    sName[0] = cFirstChar;
    bool bEscape;

    while (true)
    {
        bEscape = readChar(pFile, &c);
        m_iCurrentCol++;
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return false;
        }
        else if (c == (int)'\r' || c == (int)'\n')
        {
            *pError = XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE;
            return false;
        }
        else if (c == (int)' ' || c == (int)'\t' || (c == (int)'=' && !bEscape))
            break;
        else
            sName[cursor++] = (char)c;
    }
    sName[cursor] = '\0';
    if (c == (int)' ' || c == (int)'\t')
        bEscape = skipSpaces(pFile, &c);

    if (c != (int)'=' || bEscape)
    {
        *pError = XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE;
        return false;
    }

    bEscape = skipSpaces(pFile, &c);
    cursor = 0;
    if (c == (int)'\'' && !bEscape)
    {
        readWordUntil(pFile, '\'', sValue, XMLLITE_MAX_VALUE_CHARS, pError);
        if (*pError != 0)
            return false;
        bEscape = skipSpaces(pFile, &c);
    }
    else if (c == (int)'\"' && !bEscape)
    {
        readWordUntil(pFile, '\"', sValue, XMLLITE_MAX_VALUE_CHARS, pError);
        if (*pError != 0)
            return false;
        bEscape = skipSpaces(pFile, &c);
    }
    else
    {
        while (true)
        {
            if (c == EOF)
            {
                *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
                return false;
            }
            else if (c == (int)'\r' || c == (int)'\n')
            {
                *pError = XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE;
                return false;
            }
            else if (c == (int)' ' || c == (int)'\t' || (c == (int)'/' && !bEscape) || (c == (int)'>' && !bEscape))
                break;
            sValue[cursor++] = (char)c;
            bEscape = readChar(pFile, &c);
            m_iCurrentCol++;
        }
        sValue[cursor] = '\0';
        if (c == (int)' ' || c == (int)'\t')
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
void XMLLiteReader::readClosingTag(FILE * pFile, char * sName, int * pError)
{
    *pError = 0;
    int c;
    bool bEscape = skipSpaces(pFile, &c);
    int cursor = 1;
    bool bMatch = ((int)(sName[0]) == c);
    while (bMatch)
    {
        bEscape = readChar(pFile, &c);
        m_iCurrentCol++;
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return;
        }
        else if (c == (int)'\r' || c == (int)'\n')
        {
            *pError = XMLLITE_ERROR_LINEBREAK_IN_ELEMENT;
            return;
        }
        else if (c == (int)' ' || c == (int)'\t' || (c == (int)'>' && !bEscape))
            break;
        else
            bMatch = ((int)(sName[cursor++]) == c);
    }
    if (c == (int)' ' || c == (int)'\t')
        bEscape = skipSpaces(pFile, &c);
    if (c != (int)'>' || bEscape)
    {
        *pError = XMLLITE_ERROR_ELEMENT_END_EXPECTED;
        return;
    }
    if (!bMatch)
        *pError = XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH;
}

// -----------------------------------------------------------------
// Name : readWordUntil
// -----------------------------------------------------------------
void XMLLiteReader::readWordUntil(FILE * pFile, char cUntil, char * sWord, int iSize, int * pError)
{
    *pError = 0;
    int c;
    int cursor = 0;
    bool b = readChar(pFile, &c);
    while (c != (int)cUntil || b)
    {
        m_iCurrentCol++;
        if (c == EOF)
        {
            *pError = XMLLITE_ERROR_EOF_NOT_EXPECTED;
            return;
        }
        sWord[cursor++] = (char)c;
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
bool XMLLiteReader::skipSpaces(FILE * pFile, int * c)
{
    bool bEscape;
    while (true)
    {
        bEscape = readChar(pFile, c);
#ifdef WIN32
        if (*c == (int)'\n' && !bEscape)
        {
            m_iCurrentLine++;
            m_iCurrentCol = 1;
        }
        else if ((*c == (int)'\r' || *c == (int)'\t' || *c == (int)' ') && !bEscape)
            m_iCurrentCol++;
#endif
#ifdef UNIX
        if (*c == (int)'\n' && !bEscape)
        {
            m_iCurrentLine++;
            m_iCurrentCol = 1;
        }
        else if ((*c == (int)'\r' || *c == (int)'\t' || *c == (int)' ') && !bEscape)
            m_iCurrentCol++;
#endif
#ifdef LINUX
        if (*c == (int)'\n' && !bEscape)
        {
            m_iCurrentLine++;
            m_iCurrentCol = 1;
        }
        else if ((*c == (int)'\r' || *c == (int)'\t' || *c == (int)' ') && !bEscape)
            m_iCurrentCol++;
#endif
#ifdef MACOS
        if (*c == (int)'\r' && !bEscape)
        {
            m_iCurrentLine++;
            m_iCurrentCol = 1;
        }
        else if ((*c == (int)'\n' || *c == (int)'\t' || *c == (int)' ') && !bEscape)
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
    int c;
    int endcomment = 0;
    while (true)
    {
        skipSpaces(pFile, &c);
        if (c == (int)'-')
        {
            if (endcomment == 0 || endcomment == 1)
                endcomment++;
        }
        else if (c == (int)'>' && endcomment == 2)
            break;
        else
            endcomment = 0;
    }
}

// -----------------------------------------------------------------
// Name : readChar
//  return true if character was a special character (escape sequence "\")
// -----------------------------------------------------------------
bool XMLLiteReader::readChar(FILE * pFile, int * c)
{
    *c = fgetc(pFile);
    if (*c == (int)'\\')
    {
        *c = fgetc(pFile);
        return true;
    }
    return false;
}
