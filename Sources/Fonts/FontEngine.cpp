// -----------------------------------------------------------------
// Font Engine
// -----------------------------------------------------------------
#include "FontEngine.h"
#include "../Display/DisplayEngine.h"
#include "../Display/TextureEngine.h"
#include "../Debug/DebugManager.h"
#include "../errorcodes.h"

// -----------------------------------------------------------------
// Name : FontEngine
//  Constructor
// -----------------------------------------------------------------
FontEngine::FontEngine(DisplayEngine * pDisplay, DebugManager * pDebug)
{
    m_pDisplay = pDisplay;
    m_pDebug = pDebug;
    for (int i = 0; i < MAX_FONTS; i++)
        m_pAllFonts[i] = NULL;
}

// -----------------------------------------------------------------
// Name : ~FontEngine
//  Destructor
// -----------------------------------------------------------------
FontEngine::~FontEngine()
{
    for (int i = 0; i < MAX_FONTS; i++)
    {
        if (m_pAllFonts[i] != NULL)
            delete m_pAllFonts[i];
    }
}

// -----------------------------------------------------------------
// Name : resetAllFonts
// -----------------------------------------------------------------
void FontEngine::resetAllFonts()
{
    for (int i = 0; i < MAX_FONTS; i++)
    {
        if (m_pAllFonts[i] != NULL)
        {
            delete m_pAllFonts[i];
            m_pAllFonts[i] = NULL;
        }
    }
}

// -----------------------------------------------------------------
// Name : registerFont
//  return negative number if success, to avoid confusion with guiObject::FontId
// -----------------------------------------------------------------
int FontEngine::registerFont(const char * sFontName, TextureEngine * pTexEngine)
{
    int iFont = 0;
    while (m_pAllFonts[iFont] != NULL)
    {
        if (strcmp(m_pAllFonts[iFont]->getFontName(), sFontName) == 0)
            return -iFont;
        iFont++;
        if (iFont == MAX_FONTS)
            return 1;
    }
    m_pAllFonts[iFont] = new Font();
    s16 err = m_pAllFonts[iFont]->load(sFontName, pTexEngine);
    if (err != FNT_OK)
    {
        m_pDebug->notifyErrorMessage(err, sFontName);
        delete m_pAllFonts[iFont];
        m_pAllFonts[iFont] = NULL;
        return 1;
    }
    else
        return -iFont;
}

// -----------------------------------------------------------------
// Name : getFont
// -----------------------------------------------------------------
Font * FontEngine::getFont(int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    return m_pAllFonts[-iIndex];
}

// ------------------------------------------------------------------
// Name : getStringLength
// ------------------------------------------------------------------
int FontEngine::getStringLength(const char * sText, int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    return m_pAllFonts[-iIndex]->getStringLength(sText);
}

// ------------------------------------------------------------------
// Name : getStringHeight
// ------------------------------------------------------------------
int FontEngine::getStringHeight(const char * sText, int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    int nbLines = 1;
    int i = 0;
    while (sText[i] != '\0')
    {
        if (sText[i++] == '\n')
            nbLines++;
    }
    return m_pAllFonts[-iIndex]->getFontHeight() * nbLines;
}

// ------------------------------------------------------------------
// Name : putStringInBox
// ------------------------------------------------------------------
int FontEngine::putStringInBox(char * sText, int iBoxWidth, int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    return m_pAllFonts[-iIndex]->putStringInBox(sText, iBoxWidth);
}

// ------------------------------------------------------------------
// Name : getFontHeight
// ------------------------------------------------------------------
int FontEngine::getFontHeight(int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    return m_pAllFonts[-iIndex]->getFontHeight();
}

// ------------------------------------------------------------------
// Name : getCharacterPosition
// ------------------------------------------------------------------
CoordsScreen FontEngine::getCharacterPosition(int iPos, const char * sText, int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    return m_pAllFonts[-iIndex]->getCharacterPosition(iPos, sText);
}

// ------------------------------------------------------------------
// Name : getCharacterPosition
// ------------------------------------------------------------------
int FontEngine::getCharacterPosition(CoordsScreen cs, const char * sText, int iIndex)
{
    assert(IS_VALID_FONTID(iIndex));
    assert(m_pAllFonts[-iIndex] != NULL);
    return m_pAllFonts[-iIndex]->getCharacterPosition(cs, sText);
}
