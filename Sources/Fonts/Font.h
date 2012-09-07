#ifndef _FONT_H
#define _FONT_H

#include "../utils.h"

class DisplayEngine;
class TextureEngine;

class CharDescriptor
{
public:
  int x, y, width, height, xoffset, yoffset, xadvance;
  wchar_t c;
};

class Font
{
public:
  Font();

  s16 load(const wchar_t * sFontName, TextureEngine * pTexEngine);
  wchar_t * getFontName() { return m_sFontName; };
  int getStringLength(const wchar_t * sText);
  u8 getFontHeight() { return m_uFontHeight; };
  int putStringInBox(wchar_t * sText, int iBoxWidth);
  CharDescriptor * findCharDescriptor(wchar_t c, bool checkAcute = true);
  CharDescriptor * getLastAcuteDescriptor() { return m_pLastAcute; };
  int getTextureId() { return m_iTexId; };
  CoordsScreen getCharacterPosition(int iPos, const wchar_t * sText);
  int getCharacterPosition(CoordsScreen cs, const wchar_t * sText);

    static void initUnicodeTables();

private:
  void storeData(wchar_t*, wchar_t*, wchar_t*);

  TextureEngine * m_pTexEngine;
  wchar_t m_sFontName[64];
  u8 m_uFontHeight;
  CharDescriptor m_AllChars[256];
  int m_iNbDescriptor;
  int m_iTexId;
  CharDescriptor * m_pLastAcute;

  static wch_hash m_hmUnicodeReplacementTable;
  static wch_hash m_hmUnicodeReplacementAcutesTable;
};

#endif
