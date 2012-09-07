#include "guiButton.h"
#include "ComponentOwnerInterface.h"
#include "../Audio/AudioManager.h"

#define MULTICLICKS_TIMER_FIRST     1.0f
#define MULTICLICKS_TIMER_NEXT      0.1f

// -----------------------------------------------------------------
// Name : guiButton
//  Constructor
// -----------------------------------------------------------------
guiButton::guiButton() : guiImage()
{
  m_ClickOption = BCO_None;
  m_OverOption = BCO_None;
  m_bMouseDown = m_bMouseOver = false;
  m_bClickState = false;
  m_pLabel = new guiLabel();
  m_pGeometryClicked = NULL;
  m_pGeometryOver = NULL;
  m_pGeometryNormal = NULL;
  m_pGeometryAttachedImage = NULL;
  m_bCatchButton2Events = false;
  m_bCatchDoubleClicks = false;
  m_bMultiClicks = false;
  m_fMultiClicksTimer = 0.0f;
}

// -----------------------------------------------------------------
// Name : ~guiButton
//  Destructor
// -----------------------------------------------------------------
guiButton::~guiButton()
{
  FREE(m_pLabel);
  FREE(m_pGeometryClicked);
  FREE(m_pGeometryOver);
  FREE(m_pGeometryAttachedImage);
  m_pGeometry = m_pGeometryNormal;
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiButton::init(const wchar_t * sText, FontId fontId, F_RGBA textColor, int iClickedTex, BtnClickOptions clickOption, int iOverTex, BtnClickOptions overOption, int iTexId, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiImage::init(iTexId, sCpntId, xPxl, yPxl, wPxl, hPxl, pDisplay);
  m_pLabel->init(sText, fontId, textColor, L"ButtonLabel", 0, 0, 0, 0, pDisplay);
  m_pLabel->centerOnComponent(this);
  m_pGeometryNormal = (GeometryQuads*) m_pGeometry;
  m_ClickOption = clickOption;
  if (m_ClickOption == BCO_ReplaceTex || m_ClickOption == BCO_AddTex)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, iClickedTex, pDisplay);
    m_pGeometryClicked = new GeometryQuads(&quad, VB_Static);
  }
  m_OverOption = overOption;
  if (m_OverOption == BCO_ReplaceTex || m_OverOption == BCO_AddTex)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, iOverTex, pDisplay);
    m_pGeometryOver = new GeometryQuads(&quad, VB_Static);
  }
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiButton::clone()
{
  guiButton * pObj = new guiButton();
  pObj->init(m_pLabel->getText(), m_pLabel->getFontId(), m_pLabel->getDiffuseColor(), (m_pGeometryClicked == NULL) ? -1 : m_pGeometryClicked->getTexture(), m_ClickOption, (m_pGeometryOver == NULL) ? -1 : m_pGeometryOver->getTexture(), m_OverOption, m_pGeometryNormal->getTexture(), m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  if (m_pGeometryAttachedImage != NULL)
    pObj->attachImage(m_pGeometryAttachedImage->getTexture());
  pObj->setMultiClicks(m_bMultiClicks);
  return pObj;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiButton::update(double delta)
{
  guiImage::update(delta);
  if (m_bEnabled && m_bVisible && m_bMultiClicks && m_bMouseDown && m_bMouseOver && m_fMultiClicksTimer > 0)
  {
    m_fMultiClicksTimer -= delta;
    if (m_fMultiClicksTimer <= 0)
    {
      m_pOwner->onButtonEvent(&m_MultiClicksEvent, this);
      m_fMultiClicksTimer = MULTICLICKS_TIMER_NEXT;
    }
  }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiButton::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (m_bVisible)
  {
    m_pGeometry = m_pGeometryNormal;
    bool bNeedPop = false;
    bool bAddMode = false;
    if (m_bClickState)
    {
      switch (m_ClickOption)
      {
      case BCO_None:
        break;
      case BCO_ReplaceTex:
        m_pGeometry = m_pGeometryClicked;
        break;
      case BCO_AddTex:
        guiImage::displayAt(iXOffset, iYOffset, cpntColor);
        m_pGeometry = m_pGeometryClicked;
        break;
      case BCO_Decal:
        iXOffset += 3;
        iYOffset += 3;
        break;
      case BCO_Scale:
        {
          float coef = 1.2f;
          Coords3D fCenter = getDisplay()->get3DCoords(CoordsScreen(
                iXOffset + getXPos() + getWidth() / 2,
                iYOffset + getYPos() + getHeight() / 2),
                DMS_2D);
          glPushMatrix();
          glTranslatef(fCenter.x * (1 - coef), fCenter.y * (1 - coef), 0.0f);
          glScalef(coef, coef, 1.0f);
          bNeedPop = true;
          break;
        }
      case BCO_Enlight:
        {
          if (F_RGBA_ISNULL(cpntColor))
            cpntColor = rgb(1, 1, 1);
          cpntColor = cpntColor + rgb(1, 1, 1);
          cpntColor = cpntColor / 2;
          bAddMode = true;
          break;
        }
      }
    }

    if (!m_bEnabled)
      cpntColor = F_RGBA_MULTIPLY(cpntColor, rgba(1,1,1, 0.3f));
    else if (m_bMouseOver && !m_bClickState)
    {
      switch (m_OverOption)
      {
      case BCO_None:
        break;
      case BCO_ReplaceTex:
        m_pGeometry = m_pGeometryOver;
        break;
      case BCO_AddTex:
        guiImage::displayAt(iXOffset, iYOffset, cpntColor);
        m_pGeometry = m_pGeometryOver;
        break;
      case BCO_Decal:
        iXOffset += 3;
        iYOffset += 3;
        break;
      case BCO_Scale:
        {
          float coef = 1.2f;
          Coords3D fCenter = getDisplay()->get3DCoords(CoordsScreen(
                iXOffset + getXPos() + getWidth() / 2,
                iYOffset + getYPos() + getHeight() / 2),
                DMS_2D);
          glPushMatrix();
          glTranslatef(fCenter.x * (1 - coef), fCenter.y * (1 - coef), 0.0f);
          glScalef(coef, coef, 1.0f);
          bNeedPop = true;
          break;
        }
      case BCO_Enlight:
        {
          if (F_RGBA_ISNULL(cpntColor))
            cpntColor = rgb(1, 1, 1);
          cpntColor = cpntColor + rgb(1, 1, 1);
          cpntColor = cpntColor / 2;
          bAddMode = true;
          break;
        }
      }
    }
    bool bPrevMode = false;
    if (bAddMode)
      bPrevMode = getDisplay()->setAdditiveMode(true);
    guiImage::displayAt(iXOffset, iYOffset, cpntColor);
    m_pLabel->displayAt(iXOffset, iYOffset, cpntColor);
    if (m_pGeometryAttachedImage != NULL)
    {
      CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
      m_pGeometryAttachedImage->display(coords, F_RGBA_MULTIPLY(cpntColor, m_DiffuseColor));
    }
    if (bAddMode)
      getDisplay()->setAdditiveMode(bPrevMode);
    if (bNeedPop)
      glPopMatrix();
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiButton::onButtonEvent(ButtonAction * pEvent)
{
  if ((pEvent->eButton != Button1 && (pEvent->eButton != Button2 || !m_bCatchButton2Events)) || !m_bEnabled || !m_bVisible)
    return NULL;
  switch (pEvent->eEvent)
  {
  case Event_Down:
    {
      m_bMouseDown = true;
      m_bMouseOver = true;
      m_bClickState = true;
      AudioManager::getInstance()->playSound(SOUND_CLICK);
      if (m_bMultiClicks)
      {
        m_fMultiClicksTimer = MULTICLICKS_TIMER_FIRST;
        memcpy(&m_MultiClicksEvent, pEvent, sizeof(ButtonAction));
        bool bClose = false;
        if (m_pOwner != NULL)
          bClose = !m_pOwner->onButtonEvent(pEvent, this);
        return bClose ? NULL : this;
      }
      return this;
    }
  case Event_Up:
    {
      m_bMouseDown = false;
      m_bClickState = false;
      bool bClose = false;
      if (!m_bMultiClicks && m_bMouseOver && m_pOwner != NULL)
        bClose = !m_pOwner->onButtonEvent(pEvent, this);
      return bClose ? NULL : this;
    }
  case Event_Drag:
    {
      m_bClickState = m_bMouseOver = isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
      return this;
    }
  case Event_DoubleClick:
    {
      if (m_bCatchDoubleClicks)
      {
        bool bClose = false;
        if (m_bMouseOver && m_pOwner != NULL)
          bClose = !m_pOwner->onButtonEvent(pEvent, this);
        return bClose ? NULL : this;
      }
    }
    default:
    break;
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * guiButton::onCursorMoveEvent(int xPxl, int yPxl)
{
  m_bMouseOver = true;
  return this;
}

// -----------------------------------------------------------------
// Name : onCursorMoveOutEvent
// -----------------------------------------------------------------
void guiButton::onCursorMoveOutEvent()
{
  m_bMouseOver = false;
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiButton::onResize(int iOldWidth, int iOldHeight)
{
  Geometry * ptmp = m_pGeometry;
  m_pGeometry = m_pGeometryNormal;
  guiImage::onResize(iOldWidth, iOldHeight);
  m_pGeometry = m_pGeometryClicked;
  guiImage::onResize(iOldWidth, iOldHeight);
  m_pGeometry = m_pGeometryAttachedImage;
  guiImage::onResize(iOldWidth, iOldHeight);
  m_pGeometry = ptmp;
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : setNormalTexture
// -----------------------------------------------------------------
void guiButton::setNormalTexture(int iTexId)
{
  m_pGeometryNormal->setTexture(iTexId);
}

// -----------------------------------------------------------------
// Name : setText
// -----------------------------------------------------------------
void guiButton::setText(const wchar_t * sText)
{
  m_pLabel->setText(sText);
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void guiButton::moveTo(int xPxl, int yPxl)
{
  guiComponent::moveTo(xPxl, yPxl);
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : moveBy
// -----------------------------------------------------------------
void guiButton::moveBy(int xPxl, int yPxl)
{
  guiComponent::moveBy(xPxl, yPxl);
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : autoPad
// -----------------------------------------------------------------
void guiButton::autoPad(int margin)
{
  setWidth(m_pLabel->getWidth() + 2 * margin);
  setHeight(m_pLabel->getHeight() + 2 * margin);
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : autoPadWidth
// -----------------------------------------------------------------
void guiButton::autoPadWidth(int margin, int minWidth)
{
  int newWidth = m_pLabel->getWidth() + 2 * margin;
  if (newWidth < minWidth)
    setWidth(minWidth);
  else
    setWidth(newWidth);
  m_pLabel->centerOnComponent(this);
}

// -----------------------------------------------------------------
// Name : attachImage
// -----------------------------------------------------------------
void guiButton::attachImage(int iTex)
{
  if (m_pGeometryAttachedImage == NULL)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, iTex, getDisplay());
    m_pGeometryAttachedImage = new GeometryQuads(&quad, VB_Static);
  }
  else
    m_pGeometryAttachedImage->setTexture(iTex);
}

// -----------------------------------------------------------------
// Name : setEnabled
// -----------------------------------------------------------------
void guiButton::setEnabled(bool bEnabled)
{
  guiImage::setEnabled(bEnabled);
  if (!bEnabled)
  {
    m_bMouseDown = m_bMouseOver = false;
    m_bClickState = false;
  }
}

// -----------------------------------------------------------------
// Name : createDefaultNormalButton
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiButton * guiButton::createDefaultNormalButton(const wchar_t * sText, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiButton * pBtn = new guiButton();
  pBtn->init(
    sText, H2_FONT, H2_COLOR,
    pDisplay->getTextureEngine()->findTexture(L"interface:Transparent"),
    BCO_None,
    pDisplay->getTextureEngine()->findTexture(L"interface:Transparent"),
    BCO_Decal,
    pDisplay->getTextureEngine()->findTexture(L"interface:Transparent"),
    sId, 0, 0, 50, 32, pDisplay);
  return pBtn;
}

// -----------------------------------------------------------------
// Name : createDefaultSmallButton
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiButton * guiButton::createDefaultSmallButton(const wchar_t * sText, int width, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiButton * pBtn = new guiButton();
  pBtn->init(
    sText, TEXT_FONT, TEXT_COLOR,
    pDisplay->getTextureEngine()->findTexture(L"interface:SmallButtonClicked"),
    BCO_ReplaceTex,
    -1, BCO_None,
    pDisplay->getTextureEngine()->findTexture(L"interface:SmallButtonNormal"),
    sId, 0, 0, width, 20, pDisplay);
  return pBtn;
}

// -----------------------------------------------------------------
// Name : createDefaultWhiteButton
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiButton * guiButton::createDefaultWhiteButton(const wchar_t * sText, int width, int height, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiButton * pBtn = new guiButton();
  pBtn->init(
    sText, TEXT_FONT, TEXT_COLOR,
    pDisplay->getTextureEngine()->findTexture(L"interface:WhiteButtonClicked"),
    BCO_ReplaceTex,
    -1, BCO_None,
    pDisplay->getTextureEngine()->findTexture(L"interface:WhiteButtonNormal"),
    sId, 0, 0, width, height, pDisplay);
  return pBtn;
}

// -----------------------------------------------------------------
// Name : createDefaultImageButton
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiButton * guiButton::createDefaultImageButton(int iTex, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiButton * pBtn = new guiButton();
  pBtn->init(
    L"", TEXT_FONT, TEXT_COLOR,
    0, BCO_Decal, -1, BCO_None,
    iTex, sId, 0, 0, -1, -1, pDisplay);
  return pBtn;
}
