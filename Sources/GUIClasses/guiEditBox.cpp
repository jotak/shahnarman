#include "guiEditBox.h"
#include "../Geometries/GeometryText.h"
#include "../Geometries/GeometryQuads.h"
#include "../Geometries/StencilGeometry.h"
#include "../Input/KeyboardInputEngine.h"

#define CARET_BLINK_DELAY     0.4f

// -----------------------------------------------------------------
// Name : guiEditBox
// -----------------------------------------------------------------
guiEditBox::guiEditBox(KeyboardInputEngine * pInputs) : guiComponent()
{
  m_pInputs = pInputs;
  m_pStencilGeometry = NULL;
  m_pTextGeometry = NULL;
  m_pCaretGeometry = NULL;
  m_pSelectionGeometry = NULL;
  m_iNbLines = 0;
  wsafecpy(m_sText, LABEL_MAX_CHARS, "");
  m_FontId = (FontId)0;
  m_TextColor = F_RGBA_NULL;
  m_iCaretPos = 0;
  m_bHasFocus = false;
  m_fBlinkTimer = 0;
  m_iXScrollPos = m_iYScrollPos = 0;
  m_iSelectionStart = -1;
  m_bMultiLines = false;
  m_iSelectionEnd = 0;
}

// -----------------------------------------------------------------
// Name : ~guiEditBox
//  Destructor
// -----------------------------------------------------------------
guiEditBox::~guiEditBox()
{
  m_pInputs->unsetKeyboardListener(this);
  FREE(m_pStencilGeometry);
  FREE(m_pTextGeometry);
  FREE(m_pCaretGeometry);
  FREE(m_pSelectionGeometry);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiEditBox::init(int iCaretTex, const char * sText, FontId fontId, F_RGBA textColor, int iNbLines, bool bMultiLines, int * iMainTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_FontId = fontId;
  setText(sText);
  m_TextColor = textColor;
  m_bMultiLines = bMultiLines;
  setNbLines(iNbLines);

  Texture * pTLTex = pDisplay->getTextureEngine()->getTexture(iMainTexs[0]);
  Texture * pBRTex = pDisplay->getTextureEngine()->getTexture(iMainTexs[8]);

  // Set height according to number of lines
  setHeight(pTLTex->m_iHeight + pBRTex->m_iHeight + m_iNbLines * pDisplay->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]));

  // Main geometry (borders & background)
  QuadData ** pQuads;
  int nQuads = computeQuadsList(&pQuads, iMainTexs, pDisplay);
  m_pGeometry = new GeometryQuads(nQuads, pQuads, VB_Static);
  QuadData::releaseQuads(nQuads, pQuads);

  // Stencil
  m_pStencilGeometry = new StencilGeometry(m_iWidth - pTLTex->m_iWidth - pBRTex->m_iWidth, m_iHeight - pTLTex->m_iHeight - pBRTex->m_iHeight, VB_Static, pDisplay);

  // Text
  m_pTextGeometry = new GeometryText(m_sText, m_aiAllFonts[(int)m_FontId], VB_Static, pDisplay);

  // Caret
  QuadData quad(0, pDisplay->getTextureEngine()->getTexture(iCaretTex)->m_iWidth, 0, pDisplay->getTextureEngine()->getTexture(iCaretTex)->m_iHeight, iCaretTex, pDisplay);
  m_pCaretGeometry = new GeometryQuads(&quad, VB_Static);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiEditBox::clone()
{
  int texlist[9] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7), ((GeometryQuads*)m_pGeometry)->getTexture(8) };
  guiEditBox * pObj = new guiEditBox(m_pInputs);
  pObj->init(m_pCaretGeometry->getTexture(), m_sText, m_FontId, m_TextColor, m_iNbLines, m_bMultiLines, texlist, m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : computeQuadsList
// -----------------------------------------------------------------
int guiEditBox::computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay)
{
  // 9 Quads
  *pQuads = new QuadData*[9];
  int xPxlMiddleStart = pDisplay->getTextureEngine()->getTexture(iTextures[0])->m_iWidth;
  int xPxlMiddleEnd = m_iWidth - pDisplay->getTextureEngine()->getTexture(iTextures[2])->m_iWidth;
  int yPxlMiddleStart = pDisplay->getTextureEngine()->getTexture(iTextures[0])->m_iHeight;
  int yPxlMiddleEnd = m_iHeight - pDisplay->getTextureEngine()->getTexture(iTextures[5])->m_iHeight;
  (*pQuads)[0] = new QuadData(0,               xPxlMiddleStart,  0,                yPxlMiddleStart,  iTextures[0], pDisplay);
  (*pQuads)[1] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd,    0,                yPxlMiddleStart,  iTextures[1], pDisplay);
  (*pQuads)[2] = new QuadData(xPxlMiddleEnd,   m_iWidth,         0,                yPxlMiddleStart,  iTextures[2], pDisplay);
  (*pQuads)[3] = new QuadData(0,               xPxlMiddleStart,  yPxlMiddleStart,  yPxlMiddleEnd,    iTextures[3], pDisplay);
  (*pQuads)[4] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd,    yPxlMiddleStart,  yPxlMiddleEnd,    iTextures[4], pDisplay);
  (*pQuads)[5] = new QuadData(xPxlMiddleEnd,   m_iWidth,         yPxlMiddleStart,  yPxlMiddleEnd,    iTextures[5], pDisplay);
  (*pQuads)[6] = new QuadData(0,               xPxlMiddleStart,  yPxlMiddleEnd,    m_iHeight,        iTextures[6], pDisplay);
  (*pQuads)[7] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd,    yPxlMiddleEnd,    m_iHeight,        iTextures[7], pDisplay);
  (*pQuads)[8] = new QuadData(xPxlMiddleEnd,   m_iWidth,         yPxlMiddleEnd,    m_iHeight,        iTextures[8], pDisplay);
  return 9;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiEditBox::update(double delta)
{
  guiComponent::update(delta);
  if (m_bHasFocus)
  {
    m_fBlinkTimer += delta;
    if (m_fBlinkTimer > CARET_BLINK_DELAY)
      m_fBlinkTimer = -CARET_BLINK_DELAY;
  }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiEditBox::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (!m_bVisible)
    return;

  // display borders and background
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pGeometry->display(coords, cpntColor);

  Texture * pTex = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
  coords += CoordsScreen(pTex->m_iWidth, pTex->m_iHeight);
  CoordsScreen stencilCoords = coords;
  m_pStencilGeometry->fillStencil(stencilCoords, true);
  int iPreviousState = getDisplay()->setStencilState(2);
  {
    coords += CoordsScreen(m_iXScrollPos, m_iYScrollPos);

    if (m_pSelectionGeometry != NULL)
    {
      // display selection
//      color = F_RGBA_MULTIPLY(blendColor, rgb(1-m_TextColor.r, 1-m_TextColor.g, 1-m_TextColor.b));
      m_pSelectionGeometry->display(coords, rgb(0.8, 0.8, 0.8));
    }

    // draw text
    F_RGBA color = F_RGBA_MULTIPLY(cpntColor, m_TextColor);
    m_pTextGeometry->display(coords, color);

    if (m_bHasFocus && m_fBlinkTimer >= 0)
    {
      // Display caret
      coords += getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
      m_pCaretGeometry->display(coords, cpntColor);
    }
  }
  m_pStencilGeometry->fillStencil(stencilCoords, false);
  getDisplay()->setStencilState(iPreviousState);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiEditBox::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled || !m_bVisible)
    return NULL;
  switch (pEvent->eButton)
  {
  case Button1:
    {
      switch (pEvent->eEvent)
      {
      case Event_Down:
        onButton1Down(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
        break;
      case Event_Drag:
        onButton1Drag(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
        break;
      case Event_DoubleClick:
        onButton1DoubleClick(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
        break;
        default:
        break;
      }
      return this;
    }
  case ButtonZ:
    {
      // TODO
      return this;
    }
  case ButtonX:
    {
      // TODO
      return this;
    }
        default:
        break;
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : onButton1Down
// -----------------------------------------------------------------
void guiEditBox::onButton1Down(int xPxl, int yPxl)
{
  m_bHasFocus = true;
  m_pInputs->setKeyboardListener(this);
  FREE(m_pSelectionGeometry);

  Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
  Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
  CoordsScreen cs(xPxl - m_iXPxl - pTex1->m_iWidth, yPxl - m_iYPxl - pTex1->m_iHeight);
  m_iCaretPos = getDisplay()->getFontEngine()->getCharacterPosition(cs, m_sText, m_aiAllFonts[(int)m_FontId]);

  cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
  if (cs.x > getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth)
    m_iXScrollPos = (getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth) - cs.x - 4;
  else if (cs.x + m_iXScrollPos < 0)
    m_iXScrollPos = 0;
  if (cs.y + getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) > getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight)
    m_iYScrollPos = (getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight) - cs.y - getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
  else if (cs.y + m_iYScrollPos < 0)
    m_iYScrollPos = 0;
  m_fBlinkTimer = 0;
}

// -----------------------------------------------------------------
// Name : onButton1Drag
// -----------------------------------------------------------------
void guiEditBox::onButton1Drag(int xPxl, int yPxl)
{
  if (m_pSelectionGeometry == NULL)
    m_iSelectionStart = m_iSelectionEnd = m_iCaretPos;
  int oldCaretPos = m_iCaretPos;
  Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
  Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
  m_iCaretPos = getDisplay()->getFontEngine()->getCharacterPosition(CoordsScreen(xPxl - m_iXPxl - pTex1->m_iWidth, yPxl - m_iYPxl - pTex1->m_iHeight), m_sText, m_aiAllFonts[(int)m_FontId]);

  if (oldCaretPos != m_iCaretPos)
  {
    if (m_iCaretPos < m_iSelectionStart)
    {
      m_iSelectionStart = m_iCaretPos;
      updateSelectionGeometry();
    }
    else if (m_iCaretPos > m_iSelectionEnd)
    {
      m_iSelectionEnd = m_iCaretPos;
      updateSelectionGeometry();
    }
    else if (oldCaretPos == m_iSelectionStart)
    {
      m_iSelectionStart = m_iCaretPos;
      if (m_iSelectionEnd < m_iSelectionStart)
      {
        m_iSelectionStart = m_iSelectionEnd;
        m_iSelectionEnd = m_iCaretPos;
        updateSelectionGeometry();
      }
      else if (m_iSelectionStart == m_iSelectionEnd)
      {
        FREE(m_pSelectionGeometry);
      }
      else
        updateSelectionGeometry();
    }
    else if (oldCaretPos == m_iSelectionEnd)
    {
      m_iSelectionEnd = m_iCaretPos;
      if (m_iSelectionEnd < m_iSelectionStart)
      {
        m_iSelectionEnd = m_iSelectionStart;
        m_iSelectionStart = m_iCaretPos;
        updateSelectionGeometry();
      }
      else if (m_iSelectionStart == m_iSelectionEnd)
      {
        FREE(m_pSelectionGeometry);
      }
      else
        updateSelectionGeometry();
    }
    CoordsScreen cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
    if (cs.x > getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth)
      m_iXScrollPos = (getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth) - cs.x - 4;
    else if (cs.x + m_iXScrollPos < 0)
      m_iXScrollPos = 0;
    if (cs.y + getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) > getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight)
      m_iYScrollPos = (getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight) - cs.y - getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
    else if (cs.y + m_iYScrollPos < 0)
      m_iYScrollPos = 0;
    m_fBlinkTimer = 0;
  }
}

// -----------------------------------------------------------------
// Name : onButton1DoubleClick
// -----------------------------------------------------------------
void guiEditBox::onButton1DoubleClick(int xPxl, int yPxl)
{
  m_iSelectionStart = getStartOfWord(m_iCaretPos);
  m_iCaretPos = m_iSelectionEnd = getEndOfWord(m_iCaretPos);
  if (m_iSelectionEnd != m_iSelectionStart)
    updateSelectionGeometry();
  else
    FREE(m_pSelectionGeometry);
  Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
  Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
  CoordsScreen cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
  if (cs.x > getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth)
    m_iXScrollPos = (getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth) - cs.x - 4;
  else if (cs.x + m_iXScrollPos < 0)
    m_iXScrollPos = 0;
  if (cs.y + getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) > getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight)
    m_iYScrollPos = (getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight) - cs.y - getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
  else if (cs.y + m_iYScrollPos < 0)
    m_iYScrollPos = 0;
  m_fBlinkTimer = 0;
}

// -----------------------------------------------------------------
// Name : onFocusLost
// -----------------------------------------------------------------
void guiEditBox::onFocusLost()
{
  m_bHasFocus = false;
  m_pInputs->unsetKeyboardListener(this);
}

// -----------------------------------------------------------------
// Name : onKeyDown
// -----------------------------------------------------------------
bool guiEditBox::onKeyDown(unsigned char c)
{
  if (m_bHasFocus)
  {
    if (m_pInputs->isCtrlPressed())
    {
      if (c == 22 || c == 24 || c == 3) // Ctrl+c/v/x : copy/paste/cut ; TODO : does it work in other systems?
      {
        if (m_pSelectionGeometry != NULL) // find current selection
        {
          if (c == 3 || c == 24) // get selected text
          {
            int nbChars = m_iSelectionEnd - m_iSelectionStart;
            char * sClipboard = new char[nbChars+1];
            for (int i = m_iSelectionStart; i <= m_iSelectionEnd; i++)
              sClipboard[i-m_iSelectionStart] = m_sText[i];
            sClipboard[nbChars] = '\0';
            copyStringToClipboard(sClipboard);
            delete[] sClipboard;
          }
          if (c == 24 || c == 22) // delete current selection
          {
            FREE(m_pSelectionGeometry);
            int nDel = m_iSelectionEnd - m_iSelectionStart;
            m_iCaretPos = m_iSelectionStart;
            int iLength = (int) strlen(m_sText);
            for (int i = m_iCaretPos; i < 1 + iLength - nDel; i++)
              m_sText[i] = m_sText[i+nDel];
            if (m_pTextGeometry != NULL)
              ((GeometryText*)m_pTextGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
          }
        }
        if (c == 22) // paste
        {
          char sBuffer[1024] = "";
          getStringFromClipboard(sBuffer, 1024);
          int iLength = (int) strlen(m_sText);
          int iBufLen = (int) strlen(sBuffer);
          if (iLength + iBufLen < LABEL_MAX_CHARS-1) // paste only if no overflow
          {
            m_sText[iLength+iBufLen] = '\0';
            for (int i = iLength-1; i >= m_iCaretPos; i--)
              m_sText[i+iBufLen] = m_sText[i];
            for (int i = 0; i < iBufLen; i++)
              m_sText[i+m_iCaretPos] = sBuffer[i];
            m_iCaretPos += iBufLen;
            if (m_pTextGeometry != NULL)
              ((GeometryText*)m_pTextGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
          }
        }
      }
      return true;
    }
    if (c == 13)
      c = '\n';
    if (c == '\n' && !m_bMultiLines)
      return false;
    if (c == 8) // backspace
    {
      if (m_iCaretPos == 0 && m_pSelectionGeometry == NULL)
        return true;
      // move backward and delete
      if (m_pSelectionGeometry == NULL)
        m_iCaretPos--;
      c = 127;
    }
    if (c == 127 || m_pSelectionGeometry != NULL) // delete
    {
      int nDel = 1;
      if (m_pSelectionGeometry != NULL) // delete selection
      {
        FREE(m_pSelectionGeometry);
        nDel = m_iSelectionEnd - m_iSelectionStart;
        m_iCaretPos = m_iSelectionStart;
      }
      int iLength = (int) strlen(m_sText);
      for (int i = m_iCaretPos; i < 1 + iLength - nDel; i++)
        m_sText[i] = m_sText[i+nDel];
      if (m_pTextGeometry != NULL)
        ((GeometryText*)m_pTextGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
    }
    if (c != 127)
    {
      int iLength = (int) strlen(m_sText);
      for (int i = iLength; i > m_iCaretPos; i--)
        m_sText[i] = m_sText[i-1];
      if (iLength == LABEL_MAX_CHARS - 1)
        m_sText[iLength] = '\0';
      else
        m_sText[iLength+1] = '\0';
      char str[2] = "a";
      str[0] = (char) c;
      m_sText[m_iCaretPos] = str[0];
      if (m_pTextGeometry != NULL)
        ((GeometryText*)m_pTextGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
      m_iCaretPos++;
    }
    Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
    Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
    CoordsScreen cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
    if (cs.x > getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth)
      m_iXScrollPos = (getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth) - cs.x - 4;
    else if (cs.x + m_iXScrollPos < 0)
      m_iXScrollPos = 0;
    if (cs.y + getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) > getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight)
      m_iYScrollPos = (getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight) - cs.y - getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
    else if (cs.y + m_iYScrollPos < 0)
      m_iYScrollPos = 0;
    m_fBlinkTimer = 0;
    return true;
  }
  return false;
}

// -----------------------------------------------------------------
// Name : onSpecialKeyDown
// -----------------------------------------------------------------
bool guiEditBox::onSpecialKeyDown(int key)
{
  if (m_bHasFocus)
  {
    int oldCaretPos = -1;
    if (m_pInputs->isShiftPressed())
    {
      if (m_pSelectionGeometry == NULL)
        m_iSelectionStart = m_iSelectionEnd = m_iCaretPos;
      oldCaretPos = m_iCaretPos;
    }
    else
      FREE(m_pSelectionGeometry);
    CoordsScreen cs;
    switch (key)
    {
    case SPECKEY_LEFT:
      {
        if (m_pInputs->isCtrlPressed())
          m_iCaretPos = getStartOfWord(m_iCaretPos);
        else
          m_iCaretPos = max(m_iCaretPos-1, 0);
        break;
      }
    case SPECKEY_RIGHT:
      {
        if (m_pInputs->isCtrlPressed())
          m_iCaretPos = getEndOfWord(m_iCaretPos);
        else
          m_iCaretPos = min(m_iCaretPos+1, (int) strlen(m_sText));
        break;
      }
    case SPECKEY_UP:
      {
        cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
        cs.y -= (getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) - 1);
        m_iCaretPos = getDisplay()->getFontEngine()->getCharacterPosition(cs, m_sText, m_aiAllFonts[(int)m_FontId]);
        break;
      }
    case SPECKEY_DOWN:
      {
        cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
        cs.y += getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) + 1;
        m_iCaretPos = getDisplay()->getFontEngine()->getCharacterPosition(cs, m_sText, m_aiAllFonts[(int)m_FontId]);
        break;
      }
    case SPECKEY_HOME:
      {
        if (m_pInputs->isCtrlPressed())
          m_iCaretPos = 0;
        else
          m_iCaretPos = getStartOfLine(m_iCaretPos);
        break;
      }
    case SPECKEY_END:
      {
        if (m_pInputs->isCtrlPressed())
          m_iCaretPos = (int) strlen(m_sText);
        else
          m_iCaretPos = getEndOfLine(m_iCaretPos);
        break;
      }
    }
    Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
    Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
    cs = getDisplay()->getFontEngine()->getCharacterPosition(m_iCaretPos, m_sText, m_aiAllFonts[(int)m_FontId]);
    if (cs.x > getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth)
      m_iXScrollPos = (getWidth() - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth) - cs.x - 4;
    else if (cs.x + m_iXScrollPos < 0)
      m_iXScrollPos = 0;
    if (cs.y + getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]) > getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight)
      m_iYScrollPos = (getHeight() - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight) - cs.y - getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
    else if (cs.y + m_iYScrollPos < 0)
      m_iYScrollPos = 0;
    m_fBlinkTimer = 0;
    if (oldCaretPos >= 0 && oldCaretPos != m_iCaretPos)
    {
      if (m_iCaretPos < m_iSelectionStart)
      {
        m_iSelectionStart = m_iCaretPos;
        updateSelectionGeometry();
      }
      else if (m_iCaretPos > m_iSelectionEnd)
      {
        m_iSelectionEnd = m_iCaretPos;
        updateSelectionGeometry();
      }
      else if (oldCaretPos == m_iSelectionStart)
      {
        m_iSelectionStart = m_iCaretPos;
        if (m_iSelectionEnd < m_iSelectionStart)
        {
          m_iSelectionStart = m_iSelectionEnd;
          m_iSelectionEnd = m_iCaretPos;
          updateSelectionGeometry();
        }
        else if (m_iSelectionStart == m_iSelectionEnd)
        {
          FREE(m_pSelectionGeometry);
        }
        else
          updateSelectionGeometry();
      }
      else if (oldCaretPos == m_iSelectionEnd)
      {
        m_iSelectionEnd = m_iCaretPos;
        if (m_iSelectionEnd < m_iSelectionStart)
        {
          m_iSelectionEnd = m_iSelectionStart;
          m_iSelectionStart = m_iCaretPos;
          updateSelectionGeometry();
        }
        else if (m_iSelectionStart == m_iSelectionEnd)
        {
          FREE(m_pSelectionGeometry);
        }
        else
          updateSelectionGeometry();
      }
    }
    return true;
  }
  return false;
}

// -----------------------------------------------------------------
// Name : setText
// -----------------------------------------------------------------
void guiEditBox::setText(const char * sText)
{
  if (strcmp(sText, m_sText) == 0)
    return;
  wsafecpy(m_sText, LABEL_MAX_CHARS, sText);
  if (m_pTextGeometry != NULL)
    ((GeometryText*)m_pTextGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiEditBox::onResize(int iOldWidth, int iOldHeight)
{
  guiComponent::onResize(iOldWidth, iOldHeight);
  if (m_iWidth == iOldWidth && m_iHeight == iOldHeight)
    return;
  // Main geometry (borders & background)
  if (m_pGeometry != NULL)
  {
    int texlist[9] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7), ((GeometryQuads*)m_pGeometry)->getTexture(8) };
    QuadData ** pQuads;
    int nQuads = computeQuadsList(&pQuads, texlist, getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(nQuads, pQuads);
    QuadData::releaseQuads(nQuads, pQuads);
  }
  // Stencil
  if (m_pStencilGeometry != NULL)
  {
    Texture * pTex1 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
    Texture * pTex2 = m_pGeometry->getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(8));
    m_pStencilGeometry->resize(m_iWidth - (int)pTex1->m_iWidth - (int)pTex2->m_iWidth, m_iHeight - (int)pTex1->m_iHeight - (int)pTex2->m_iHeight);
  }
}

// -----------------------------------------------------------------
// Name : updateSelectionGeometry
// -----------------------------------------------------------------
void guiEditBox::updateSelectionGeometry()
{
  int nbSelLines = 1;
  for (int i = m_iSelectionStart; i < m_iSelectionEnd; i++)
  {
    if (m_sText[i] == '\n')
      nbSelLines++;
  }

  int texture = (m_pSelectionGeometry == NULL) ? getDisplay()->getTextureEngine()->loadTexture("EmptyWhiteSquare") : m_pSelectionGeometry->getTexture();
  QuadData ** pQuads = new QuadData*[nbSelLines];

  // Upper-left quad 1
  CoordsScreen cs1 = getDisplay()->getFontEngine()->getCharacterPosition(m_iSelectionStart, m_sText, m_aiAllFonts[(int)m_FontId]);
  CoordsScreen cs2;
  int iQuad = 0;
  for (int i = m_iSelectionStart; i < m_iSelectionEnd; i++)
  {
    if (m_sText[i] == '\n')
    {
      cs2 = getDisplay()->getFontEngine()->getCharacterPosition(i, m_sText, m_aiAllFonts[(int)m_FontId]);
      cs2.y += getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
      pQuads[iQuad] = new QuadData(cs1.x, cs2.x, cs1.y, cs2.y, texture, getDisplay());
      iQuad++;
      cs1.x = 0;
      cs1.y = cs2.y;
    }
  }
  cs2 = getDisplay()->getFontEngine()->getCharacterPosition(m_iSelectionEnd, m_sText, m_aiAllFonts[(int)m_FontId]);
  cs2.y += getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_FontId]);
  pQuads[iQuad] = new QuadData(cs1.x, cs2.x, cs1.y, cs2.y, texture, getDisplay());

  if (m_pSelectionGeometry == NULL)
    m_pSelectionGeometry = new GeometryQuads(nbSelLines, pQuads, VB_Static);
  else
    m_pSelectionGeometry->modify(nbSelLines, pQuads);
  QuadData::releaseQuads(nbSelLines, pQuads);
}

// -----------------------------------------------------------------
// Name : getStartOfWord
// -----------------------------------------------------------------
int guiEditBox::getStartOfWord(int caretpos)
{
  int start = caretpos;
  if (caretpos-1 < 0)
    return start;
  start--;
  bool isFirstWord = !(m_sText[start] == ' ' ||
                  m_sText[start] == '\n' ||
                  m_sText[start] == '\t' ||
                  m_sText[start] == '.' ||
                  m_sText[start] == '-' ||
                  m_sText[start] == ',' ||
                  m_sText[start] == '!' ||
                  m_sText[start] == '?' ||
                  m_sText[start] == ';' ||
                  m_sText[start] == '<' ||
                  m_sText[start] == '>' ||
                  m_sText[start] == '(' ||
                  m_sText[start] == ')' ||
                  m_sText[start] == '\"' ||
                  m_sText[start] == '\'' ||
                  m_sText[start] == '[' ||
                  m_sText[start] == ']' ||
                  m_sText[start] == '{' ||
                  m_sText[start] == '}' ||
                  m_sText[start] == '%' ||
                  m_sText[start] == '*' ||
                  m_sText[start] == ':' ||
                  m_sText[start] == '\\');
  for (int i = caretpos-2; i >= 0; i--)
  {
    bool isWord = !(m_sText[start-1] == ' ' ||
                    m_sText[start-1] == '\n' ||
                    m_sText[start-1] == '\t' ||
                    m_sText[start-1] == '.' ||
                    m_sText[start-1] == '-' ||
                    m_sText[start-1] == ',' ||
                    m_sText[start-1] == '!' ||
                    m_sText[start-1] == '?' ||
                    m_sText[start-1] == ';' ||
                    m_sText[start-1] == '<' ||
                    m_sText[start-1] == '>' ||
                    m_sText[start-1] == '(' ||
                    m_sText[start-1] == ')' ||
                    m_sText[start-1] == '\"' ||
                    m_sText[start-1] == '\'' ||
                    m_sText[start-1] == '[' ||
                    m_sText[start-1] == ']' ||
                    m_sText[start-1] == '{' ||
                    m_sText[start-1] == '}' ||
                    m_sText[start-1] == '%' ||
                    m_sText[start-1] == '*' ||
                    m_sText[start-1] == ':' ||
                    m_sText[start-1] == '\\');
    if (isWord == isFirstWord)
      start--;
    else
      break;
  }
  return start;
}

// -----------------------------------------------------------------
// Name : getEndOfWord
// -----------------------------------------------------------------
int guiEditBox::getEndOfWord(int caretpos)
{
  int end = caretpos;
  int iLength = (int) strlen(m_sText);
  if (end >= iLength)
    return end;
  bool isFirstWord = !(m_sText[end] == ' ' ||
                  m_sText[end] == '\n' ||
                  m_sText[end] == '\t' ||
                  m_sText[end] == '.' ||
                  m_sText[end] == '-' ||
                  m_sText[end] == ',' ||
                  m_sText[end] == '!' ||
                  m_sText[end] == '?' ||
                  m_sText[end] == ';' ||
                  m_sText[end] == '<' ||
                  m_sText[end] == '>' ||
                  m_sText[end] == '(' ||
                  m_sText[end] == ')' ||
                  m_sText[end] == '\"' ||
                  m_sText[end] == '\'' ||
                  m_sText[end] == '[' ||
                  m_sText[end] == ']' ||
                  m_sText[end] == '{' ||
                  m_sText[end] == '}' ||
                  m_sText[end] == '%' ||
                  m_sText[end] == '*' ||
                  m_sText[end] == ':' ||
                  m_sText[end] == '\\');
  for (int i = caretpos+1; i <= iLength; i++)
  {
    bool isWord = !(m_sText[end] == ' ' ||
                    m_sText[end] == '\n' ||
                    m_sText[end] == '\t' ||
                    m_sText[end] == '.' ||
                    m_sText[end] == '-' ||
                    m_sText[end] == ',' ||
                    m_sText[end] == '!' ||
                    m_sText[end] == '?' ||
                    m_sText[end] == ';' ||
                    m_sText[end] == '<' ||
                    m_sText[end] == '>' ||
                    m_sText[end] == '(' ||
                    m_sText[end] == ')' ||
                    m_sText[end] == '\"' ||
                    m_sText[end] == '\'' ||
                    m_sText[end] == '[' ||
                    m_sText[end] == ']' ||
                    m_sText[end] == '{' ||
                    m_sText[end] == '}' ||
                    m_sText[end] == '%' ||
                    m_sText[end] == '*' ||
                    m_sText[end] == ':' ||
                    m_sText[end] == '\\');
    if (isWord == isFirstWord)
      end++;
    else
      break;
  }
  return end;
}

// -----------------------------------------------------------------
// Name : getStartOfLine
// -----------------------------------------------------------------
int guiEditBox::getStartOfLine(int caretpos)
{
  while (caretpos - 1 >= 0 && m_sText[caretpos-1] != '\n' && m_sText[caretpos-1] != '\r')
    caretpos--;
  return caretpos;
}

// -----------------------------------------------------------------
// Name : getEndOfLine
// -----------------------------------------------------------------
int guiEditBox::getEndOfLine(int caretpos)
{
  int len = (int) strlen(m_sText);
  while (caretpos + 1 <= len && m_sText[caretpos] != '\n' && m_sText[caretpos] != '\r')
    caretpos++;
  return caretpos;
}

// -----------------------------------------------------------------
// Name : setFocus
// -----------------------------------------------------------------
void guiEditBox::setFocus()
{
  m_bHasFocus = true;
  m_pInputs->setKeyboardListener(this);
}

// -----------------------------------------------------------------
// Name : createDefaultEditBox
//  Static default constructor
// -----------------------------------------------------------------
guiEditBox * guiEditBox::createDefaultEditBox(int iNbLines, bool bMultiLines, int wPxl, const char * sId, KeyboardInputEngine * pInputs, DisplayEngine * pDisplay)
{
  int iTexs[9];
  iTexs[0] = pDisplay->getTextureEngine()->findTexture("interface:LstTL");
  iTexs[1] = pDisplay->getTextureEngine()->findTexture("interface:LstTC");
  iTexs[2] = pDisplay->getTextureEngine()->findTexture("interface:LstTR");
  iTexs[3] = pDisplay->getTextureEngine()->findTexture("interface:LstCL");
  iTexs[4] = pDisplay->getTextureEngine()->findTexture("interface:EditBg");
  iTexs[5] = pDisplay->getTextureEngine()->findTexture("interface:LstCR");
  iTexs[6] = pDisplay->getTextureEngine()->findTexture("interface:LstBL");
  iTexs[7] = pDisplay->getTextureEngine()->findTexture("interface:LstBC");
  iTexs[8] = pDisplay->getTextureEngine()->findTexture("interface:LstBR");
  guiEditBox * pBox = new guiEditBox(pInputs);
  pBox->init(
    pDisplay->getTextureEngine()->findTexture("interface:Caret"),
    "", TEXT_FONT, TEXT_COLOR_DARK, iNbLines, bMultiLines, iTexs,
    sId, 0, 0, wPxl, 1, pDisplay);
  return pBox;
}
