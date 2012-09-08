#ifndef _FONT_H
#define _FONT_H

#include "../utils.h"

class DisplayEngine;
class TextureEngine;

class CharDescriptor
{
public:
    int x, y, width, height, xoffset, yoffset, xadvance;
    char c;
};

class Font
{
public:
    Font();

    s16 load(const char * sFontName, TextureEngine * pTexEngine);
    char * getFontName()
    {
        return m_sFontName;
    };
    int getStringLength(const char * sText);
    u8 getFontHeight()
    {
        return m_uFontHeight;
    };
    int putStringInBox(char * sText, int iBoxWidth);
    CharDescriptor * findCharDescriptor(char c, bool checkAcute = true);
    CharDescriptor * getLastAcuteDescriptor()
    {
        return m_pLastAcute;
    };
    int getTextureId()
    {
        return m_iTexId;
    };
    CoordsScreen getCharacterPosition(int iPos, const char * sText);
    int getCharacterPosition(CoordsScreen cs, const char * sText);

    static void initUnicodeTables();

private:
    void storeData(char*, char*, char*);

    TextureEngine * m_pTexEngine;
    char m_sFontName[64];
    u8 m_uFontHeight;
    CharDescriptor m_AllChars[256];
    int m_iNbDescriptor;
    int m_iTexId;
    CharDescriptor * m_pLastAcute;

    static ch_hash m_hmUnicodeReplacementTable;
    static ch_hash m_hmUnicodeReplacementAcutesTable;
};

#endif
