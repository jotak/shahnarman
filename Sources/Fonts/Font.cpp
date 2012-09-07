// -----------------------------------------------------------------
// FONT
// Contains all different classes of fonts
// -----------------------------------------------------------------
#include "Font.h"
#include "../Display/TextureEngine.h"
#include "../SystemHeaders.h"
#include "../errorcodes.h"


wch_hash Font::m_hmUnicodeReplacementTable;
wch_hash Font::m_hmUnicodeReplacementAcutesTable;

// -----------------------------------------------------------------
// Name : Font
//  Constructor
// -----------------------------------------------------------------
Font::Font()
{
    m_iTexId = -1;
    m_pTexEngine = NULL;
    m_iNbDescriptor = 0;
    m_pLastAcute = NULL;
    m_uFontHeight = 0;
    if (m_hmUnicodeReplacementTable.size() == 0)
        Font::initUnicodeTables();
}

// -----------------------------------------------------------------
// Name : load
// -----------------------------------------------------------------
s16 Font::load(const wchar_t * sFontName, TextureEngine * pTexEngine)
{
    wsafecpy(m_sFontName, 64, sFontName);

    wchar_t descFile[MAX_PATH] = LDATA_PATH;
    wsafecat(descFile, MAX_PATH, L"fonts/");
    wsafecat(descFile, MAX_PATH, sFontName);
    wsafecat(descFile, MAX_PATH, L".fnt");
    FILE * f = NULL;
    errno_t err = wfopen(&f, descFile, L"r");
    if (err != 0)
    {
        switch (err)
        {
        case ENOENT:
            return FNT_FILENOTFOUND;
        default:
            return FNT_ERRORONREADING;
        }
    }

    wchar_t sTitle[64];
    wchar_t sKey[64];
    wchar_t sValue[64];
    bool bGuillemets;
    int curchar = 0;
    while (!feof(f))
    {
        wint_t c;
        do
        {
            c = fgetwc(f);
        }
        while (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r');
        do
        {
            sTitle[curchar++] = (wchar_t)c;
            c = fgetwc(f);
        }
        while (c != L' ' && c != L'\t' && c != L'\n' && c != L'\r' && !feof(f));
        sTitle[curchar] = '\0';
        curchar = 0;
        if (feof(f))
            break;
        do
        {
            c = fgetwc(f);
        }
        while (c == L' ' || c == L'\t');
        while (c != L'\n' && c != L'\r')
        {
            do
            {
                sKey[curchar++] = (wchar_t)c;
                c = fgetwc(f);
            }
            while (c != L'=');
            sKey[curchar] = '\0';
            curchar = 0;
            c = fgetwc(f);
            if (c == L'\"')
            {
                bGuillemets = true;
                c = fgetwc(f);
            }
            else
                bGuillemets = false;
            while ((bGuillemets && c != L'\"') || (!bGuillemets && c != L' ' && c != L'\t' && c != L'\n' && c != L'\r'))
            {
                sValue[curchar++] = (wchar_t)c;
                c = fgetwc(f);
            }
            sValue[curchar] = '\0';
            curchar = 0;
            storeData(sTitle, sKey, sValue);
            if (bGuillemets)
                c = fgetwc(f);
            while (c == L' ' || c == L'\t')
                c = fgetwc(f);
        }
    }
    fclose(f);
    wsafecpy(descFile, MAX_PATH, sFontName);
//  wsafecat(descFile, MAX_PATH, L"");
    m_iTexId = pTexEngine->loadTexture(descFile);
    m_pTexEngine = pTexEngine;
    return FNT_OK;
}

// -----------------------------------------------------------------
// Name : storeData
// -----------------------------------------------------------------
void Font::storeData(wchar_t * sTitle, wchar_t * sKey, wchar_t * sValue)
{
    int iVal;
    if (wcscmp(sTitle, L"common") == 0 && wcscmp(sKey, L"lineHeight") == 0)
    {
        swscanf(sValue, L"%d", &iVal);
        m_uFontHeight = (u8) iVal;
    }
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"id") == 0)
    {
        swscanf(sValue, L"%d", &iVal);
        m_AllChars[m_iNbDescriptor++].c = (wint_t)iVal;
    }
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"x") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].x));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"y") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].y));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"width") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].width));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"height") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].height));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"xoffset") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].xoffset));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"yoffset") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].yoffset));
    else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"xadvance") == 0)
        swscanf(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].xadvance));
}

// ------------------------------------------------------------------
// Name : getStringLength
// ------------------------------------------------------------------
int Font::getStringLength(const wchar_t * sText)
{
    assert(m_pTexEngine != NULL);
    assert(m_iTexId >= 0);
    int iWidth = 0;
    int iMaxWidth = 0;
    int iLength = (int) wcslen(sText);
    for (int i = 0; i < iLength; i++)
    {
        if (sText[i] == L'\n')
        {
            if (iWidth > iMaxWidth)
                iMaxWidth = iWidth;
            iWidth = 0;
            continue;
        }
        CharDescriptor * charDesc = findCharDescriptor(sText[i]);
        if (charDesc == NULL)
            continue;
        iWidth += charDesc->xadvance;
    }
    if (iWidth > iMaxWidth)
        iMaxWidth = iWidth;

    return iMaxWidth;
}

// ------------------------------------------------------------------
// Name : findCharDescriptor
// ------------------------------------------------------------------
CharDescriptor * Font::findCharDescriptor(wchar_t c, bool checkAcute)
{
    if (checkAcute)
        m_pLastAcute = NULL;
    for (int i = 0; i < m_iNbDescriptor; i++)
    {
        if (m_AllChars[i].c == c)
            return &(m_AllChars[i]);
    }
    if (checkAcute)
    {
        wch_hash::iterator it = Font::m_hmUnicodeReplacementTable.find(c);
        if (it != Font::m_hmUnicodeReplacementTable.end())
        {

            wch_hash::iterator itAcute = Font::m_hmUnicodeReplacementAcutesTable.find(c);
            if (itAcute != Font::m_hmUnicodeReplacementTable.end())
            {

                m_pLastAcute = findCharDescriptor(itAcute->second, false);
            }
            return findCharDescriptor(it->second, false);
        }
    }
    return NULL;
}

// ------------------------------------------------------------------
// Name : putStringInBox
// ------------------------------------------------------------------
int Font::putStringInBox(wchar_t * sText, int iBoxWidth)
{
    assert(m_pTexEngine != NULL);
    assert(m_iTexId >= 0);
    int iWidth = 0;
    int iHeight = m_uFontHeight;
    int iLength = (int) wcslen(sText);
    wchar_t * sOldText = new wchar_t[iLength+1];
    wsafecpy(sOldText, iLength+1, sText);
    int iNew = 0;
    int iLastSpace = -1;
    int iWidthSinceLastSpace = 0;
    for (int i = 0; i < iLength; i++)
    {
        sText[iNew] = sOldText[i];
        if (sOldText[i] == L'\n')
        {
            iHeight += m_uFontHeight;
            iWidth = 0;
            iWidthSinceLastSpace = 0;
            iLastSpace = -1;
            iNew++;
            sText[iNew] = '\0';
            continue;
        }
        CharDescriptor * charDesc = findCharDescriptor(sOldText[i]);
        if (charDesc == NULL)
            continue;
        iWidth += charDesc->xadvance;
        iWidthSinceLastSpace += charDesc->xadvance;
        if (sOldText[i] == L' ' || sOldText[i] == L'\t')
        {
            iLastSpace = iNew;
            iWidthSinceLastSpace = 0;
        }
        iNew++;
        sText[iNew] = '\0';
        if (iWidth > iBoxWidth)
        {
            if (iLastSpace >= 0)
                sText[iLastSpace] = L'\n';
            else
                sText[iNew++] = L'\n';
            iHeight += m_uFontHeight;
            iWidth = iWidthSinceLastSpace;
            iWidthSinceLastSpace = 0;
            iLastSpace = -1;
        }
    }
    delete[] sOldText;
    return iHeight;
}

// ------------------------------------------------------------------
// Name : getCharacterPosition
// ------------------------------------------------------------------
CoordsScreen Font::getCharacterPosition(int iPos, const wchar_t * sText)
{
    assert(m_pTexEngine != NULL);
    assert(m_iTexId >= 0);
    CoordsScreen cs(0,0);
    int iLength = (int) wcslen(sText);
    for (int i = 0; i < iPos; i++)
    {
        if (i >= iLength)
            break;
        if (sText[i] == L'\n')
        {
            cs.x = 0;
            cs.y += m_uFontHeight;
            continue;
        }
        CharDescriptor * charDesc = findCharDescriptor(sText[i]);
        if (charDesc == NULL)
            continue;
        cs.x += charDesc->xadvance;
    }
    return cs;
}

// ------------------------------------------------------------------
// Name : getCharacterPosition
// ------------------------------------------------------------------
int Font::getCharacterPosition(CoordsScreen cs, const wchar_t * sText)
{
    assert(m_pTexEngine != NULL);
    assert(m_iTexId >= 0);
    CoordsScreen cs2(0,m_uFontHeight);
    int iLength = (int) wcslen(sText);
    for (int i = 0; i < iLength; i++)
    {
        if (sText[i] == L'\n')
        {
            if (cs.y <= cs2.y)  // clicked on previous line
                return i;
            cs2.x = 0;
            cs2.y += m_uFontHeight;
            continue;
        }
        CharDescriptor * charDesc = findCharDescriptor(sText[i]);
        if (charDesc == NULL)
            continue;
        int adv = charDesc->xadvance / 2;
        cs2.x += adv;
        if (cs.x <= cs2.x && cs.y <= cs2.y)
            return i;
        cs2.x += charDesc->xadvance - adv;
        if (cs.x <= cs2.x && cs.y <= cs2.y)
            return i+1;
    }
    return iLength;
}

// ------------------------------------------------------------------
// Name : initUnicodeTables
// ------------------------------------------------------------------
void Font::initUnicodeTables()
{
//wch_hash Font::m_hmUnicodeReplacementTable;
//wch_hash Font::m_hmUnicodeReplacementAcutesTable;

// See http://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode_%280000-0FFF%29
    m_hmUnicodeReplacementTable[L'\u00C0'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C1'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C2'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C3'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C4'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C5'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C6'] = L'A';
    Font::m_hmUnicodeReplacementTable[L'\u00C7'] = L'C';
    Font::m_hmUnicodeReplacementTable[L'\u00C8'] = L'E';
    Font::m_hmUnicodeReplacementTable[L'\u00C9'] = L'E';
    Font::m_hmUnicodeReplacementTable[L'\u00CA'] = L'E';
    Font::m_hmUnicodeReplacementTable[L'\u00CB'] = L'E';
    Font::m_hmUnicodeReplacementTable[L'\u00CC'] = L'I';
    Font::m_hmUnicodeReplacementTable[L'\u00CD'] = L'I';
    Font::m_hmUnicodeReplacementTable[L'\u00CE'] = L'I';
    Font::m_hmUnicodeReplacementTable[L'\u00CF'] = L'I';
    Font::m_hmUnicodeReplacementTable[L'\u00D0'] = L'D';
    Font::m_hmUnicodeReplacementTable[L'\u00D1'] = L'N';
    Font::m_hmUnicodeReplacementTable[L'\u00D2'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D3'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D4'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D5'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D6'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D7'] = L'X';
    Font::m_hmUnicodeReplacementTable[L'\u00D8'] = L'O';
    Font::m_hmUnicodeReplacementTable[L'\u00D9'] = L'U';
    Font::m_hmUnicodeReplacementTable[L'\u00DA'] = L'U';
    Font::m_hmUnicodeReplacementTable[L'\u00DB'] = L'U';
    Font::m_hmUnicodeReplacementTable[L'\u00DC'] = L'U';
    Font::m_hmUnicodeReplacementTable[L'\u00DD'] = L'Y';
    Font::m_hmUnicodeReplacementTable[L'\u00DE'] = L'D';
    Font::m_hmUnicodeReplacementTable[L'\u00DF'] = L's';
    Font::m_hmUnicodeReplacementTable[L'\u00E0'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E1'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E2'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E3'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E4'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E5'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E6'] = L'a';
    Font::m_hmUnicodeReplacementTable[L'\u00E7'] = L'c';
    Font::m_hmUnicodeReplacementTable[L'\u00E8'] = L'e';
    Font::m_hmUnicodeReplacementTable[L'\u00E9'] = L'e';
    Font::m_hmUnicodeReplacementTable[L'\u00EA'] = L'e';
    Font::m_hmUnicodeReplacementTable[L'\u00EB'] = L'e';
    Font::m_hmUnicodeReplacementTable[L'\u00EC'] = L'i';
    Font::m_hmUnicodeReplacementTable[L'\u00ED'] = L'i';
    Font::m_hmUnicodeReplacementTable[L'\u00EE'] = L'i';
    Font::m_hmUnicodeReplacementTable[L'\u00EF'] = L'i';
    Font::m_hmUnicodeReplacementTable[L'\u00F0'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F1'] = L'n';
    Font::m_hmUnicodeReplacementTable[L'\u00F2'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F3'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F4'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F5'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F6'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F7'] = L'/';
    Font::m_hmUnicodeReplacementTable[L'\u00F8'] = L'o';
    Font::m_hmUnicodeReplacementTable[L'\u00F9'] = L'u';
    Font::m_hmUnicodeReplacementTable[L'\u00FA'] = L'u';
    Font::m_hmUnicodeReplacementTable[L'\u00FB'] = L'u';
    Font::m_hmUnicodeReplacementTable[L'\u00FC'] = L'u';
    Font::m_hmUnicodeReplacementTable[L'\u00FD'] = L'y';
    Font::m_hmUnicodeReplacementTable[L'\u00FE'] = L'b';
    Font::m_hmUnicodeReplacementTable[L'\u00FF'] = L'y';

    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C0'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C1'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C2'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C3'] = L'\u007E';  // ~
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C4'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C8'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00C9'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CA'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CB'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CC'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CD'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CE'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00CF'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D2'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D3'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D4'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D5'] = L'\u007E';  // ~
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D6'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00D9'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00DA'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00DB'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00DC'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E0'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E1'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E2'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E3'] = L'\u007E';  // ~
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E4'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E8'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00E9'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00EA'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00EB'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00EC'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00ED'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00EE'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00EF'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F2'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F3'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F4'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F5'] = L'\u007E';  // ~
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F6'] = L'\u00A8';  // ¨
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00F9'] = L'\u0060';  // `
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00FA'] = 171;
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00FB'] = L'\u005E';  // ^
    Font::m_hmUnicodeReplacementAcutesTable[L'\u00FC'] = L'\u00A8';  // ¨
}
