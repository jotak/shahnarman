#ifndef _FONT_ENGINE_H
#define _FONT_ENGINE_H

#include "Font.h"

#define MAX_FONTS                   15
#define IS_VALID_FONTID(i)          (i <= 0 && i > -MAX_FONTS)
#define INVALID_FONTID              1

class DebugManager;

class FontEngine
{
public:
  // Constructor / destructor
  FontEngine(DisplayEngine * pDisplay, DebugManager * pDebug);
  ~FontEngine();

  // Font loading funtions
  void resetAllFonts();
  int registerFont(const wchar_t * sFontName, TextureEngine * pTexEngine);

  // Member access functions
  Font * getFont(int iIndex);

  // Misc
  int getStringLength(const wchar_t * sText, int iIndex);
  int getStringHeight(const wchar_t * sText, int iIndex);
  int getFontHeight(int iIndex);
  int putStringInBox(wchar_t * sText, int iBoxWidth, int iIndex);
  CoordsScreen getCharacterPosition(int iPos, const wchar_t * sText, int iIndex);
  int getCharacterPosition(CoordsScreen cs, const wchar_t * sText, int iIndex);

protected:
  Font * m_pAllFonts[MAX_FONTS];
  DisplayEngine * m_pDisplay;
  DebugManager * m_pDebug;
};

#endif
