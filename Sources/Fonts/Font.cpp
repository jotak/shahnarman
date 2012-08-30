// -----------------------------------------------------------------
// FONT
// Contains all different classes of fonts
// -----------------------------------------------------------------
#include "Font.h"
#include "../Display/TextureEngine.h"
#include "../SystemHeaders.h"
#include "../errorcodes.h"

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
}

// -----------------------------------------------------------------
// Name : load
// -----------------------------------------------------------------
s16 Font::load(wchar_t * sFontName, TextureEngine * pTexEngine)
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
    do {
      c = fgetwc(f);
    } while (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r');
    do {
      sTitle[curchar++] = (wchar_t)c;
      c = fgetwc(f);
    } while (c != L' ' && c != L'\t' && c != L'\n' && c != L'\r' && !feof(f));
    sTitle[curchar] = '\0';
    curchar = 0;
    if (feof(f))
      break;
    do {
      c = fgetwc(f);
    } while (c == L' ' || c == L'\t');
    while (c != L'\n' && c != L'\r')
    {
      do {
        sKey[curchar++] = (wchar_t)c;
        c = fgetwc(f);
      } while (c != L'=');
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
    swscanf_s(sValue, L"%d", &iVal);
    m_uFontHeight = (u8) iVal;
  }
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"id") == 0)
  {
    swscanf_s(sValue, L"%d", &iVal);
    m_AllChars[m_iNbDescriptor++].c = (wint_t)iVal;
  }
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"x") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].x));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"y") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].y));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"width") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].width));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"height") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].height));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"xoffset") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].xoffset));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"yoffset") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].yoffset));
  else if (wcscmp(sTitle, L"char") == 0 && wcscmp(sKey, L"xadvance") == 0)
    swscanf_s(sValue, L"%d", &(m_AllChars[m_iNbDescriptor-1].xadvance));
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
    switch (c)
    {
    case L'é':
      m_pLastAcute = findCharDescriptor(171, false);  // hack
      return findCharDescriptor(L'e', false);
    case L'è':
      m_pLastAcute = findCharDescriptor(L'`', false);
      return findCharDescriptor(L'e', false);
    case L'ê':
      m_pLastAcute = findCharDescriptor(L'^', false);
      return findCharDescriptor(L'e', false);
    case L'ë':
      m_pLastAcute = findCharDescriptor(L'¨', false);
      return findCharDescriptor(L'e', false);
    case L'ô':
      m_pLastAcute = findCharDescriptor(L'^', false);
      return findCharDescriptor(L'o', false);
    case L'ö':
      m_pLastAcute = findCharDescriptor(L'¨', false);
      return findCharDescriptor(L'o', false);
    case L'à':
      m_pLastAcute = findCharDescriptor(L'`', false);
      return findCharDescriptor(L'a', false);
    case L'â':
      m_pLastAcute = findCharDescriptor(L'^', false);
      return findCharDescriptor(L'a', false);
    case L'ù':
      m_pLastAcute = findCharDescriptor(L'`', false);
      return findCharDescriptor(L'u', false);
    case L'ü':
      m_pLastAcute = findCharDescriptor(L'¨', false);
      return findCharDescriptor(L'u', false);
    case L'û':
      m_pLastAcute = findCharDescriptor(L'^', false);
      return findCharDescriptor(L'u', false);
    case L'ï':
      m_pLastAcute = findCharDescriptor(L'¨', false);
      return findCharDescriptor(L'i', false);
    case L'î':
      m_pLastAcute = findCharDescriptor(L'^', false);
      return findCharDescriptor(L'i', false);
    case L'ñ':
      m_pLastAcute = findCharDescriptor(L'~', false);
      return findCharDescriptor(L'n', false);
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
