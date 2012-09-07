#include "guiComboBox.h"
#include "../Interface/InterfaceManager.h"

// -----------------------------------------------------------------
// Name : guiComboBox
//  Constructor
// -----------------------------------------------------------------
guiComboBox::guiComboBox(InterfaceManager * pInterface) : guiComponent()
{
  m_pInterface = pInterface;
  m_pLabel = new guiLabel();
  m_pList = new guiContainer();
  m_pList->setDocument(new guiDocument());
  m_pList->setVisible(false);
  m_pListButtonTemplate = new guiButton();
  m_dListPos = -1;
}

// -----------------------------------------------------------------
// Name : ~guiComboBox
//  Destructor
// -----------------------------------------------------------------
guiComboBox::~guiComboBox()
{
  m_pInterface->cancelTopDisplay(m_pList);
  delete m_pList;
  delete m_pLabel;
  delete m_pListButtonTemplate;
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiComboBox::init(int * iMainTex, int iDocTex, F_RGBA textColor, FontId fontId, FrameFitBehavior wfit, FrameFitBehavior hfit, int iMaxWidth, int iMaxHeight, int btnTex1, int btnTex2, BtnClickOptions btnClickOpt, int btnHeight, int * frameTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_pLabel->init("", fontId, textColor, "", xPxl + 5, yPxl + 3, 0, 0, pDisplay);
  m_pList->init(wfit, hfit, 0, 0, iMaxWidth, iMaxHeight, frameTexs, "ComboContainer", xPxl, yPxl + hPxl, wPxl, iMaxHeight, pDisplay);
  m_pList->getDocument()->init("ComboDoc", iDocTex, 0, 0, m_pList->getInnerWidth(), m_pList->getInnerHeight(), pDisplay);
  m_pListButtonTemplate->init("", fontId, textColor, -1, BCO_None, btnTex2, btnClickOpt, btnTex1, "TemplateComboButton", 0, 0, m_pList->getInnerWidth(), btnHeight, pDisplay);

  QuadData ** pQuads;
  int nQuads = computeQuadsList(&pQuads, iMainTex, pDisplay);
  m_pGeometry = new GeometryQuads(nQuads, pQuads, VB_Static);
  QuadData::releaseQuads(nQuads, pQuads);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiComboBox::clone()
{
  int texlist[3] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2) };
  GeometryQuads * pListGeo = (GeometryQuads*) m_pList->getGeometry();
  int lsttexlist[8] = { pListGeo->getTexture(0), pListGeo->getTexture(1), pListGeo->getTexture(2), pListGeo->getTexture(3), pListGeo->getTexture(4), pListGeo->getTexture(5), pListGeo->getTexture(6), pListGeo->getTexture(7) };
  guiComboBox * pObj = new guiComboBox(m_pInterface);
  pObj->init(
    texlist,
    ((GeometryQuads*)m_pList->getDocument()->getGeometry())->getTexture(),
    m_pLabel->getDiffuseColor(),
    m_pLabel->getFontId(),
    m_pList->getWidthFitBehavior(),
    m_pList->getHeightFitBehavior(),
    m_pList->getMaxWidth(),
    m_pList->getMaxHeight(),
    m_pListButtonTemplate->getNormalGeometry()->getTexture(),
    m_pListButtonTemplate->getClickedGeometry()->getTexture(),
    m_pListButtonTemplate->getClickOption(),
    m_pListButtonTemplate->getHeight(),
    lsttexlist,
    m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : computeQuadsList
// -----------------------------------------------------------------
int guiComboBox::computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay)
{
  *pQuads = new QuadData*[3];
  int xPxlMiddleStart = pDisplay->getTextureEngine()->getTexture(iTextures[0])->m_iWidth;
  int xPxlMiddleEnd = m_iWidth - pDisplay->getTextureEngine()->getTexture(iTextures[2])->m_iWidth;
  (*pQuads)[0] = new QuadData(0, xPxlMiddleStart, 0, m_iHeight, iTextures[0], pDisplay);
  (*pQuads)[1] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd, 0, m_iHeight, iTextures[1], pDisplay);
  (*pQuads)[2] = new QuadData(xPxlMiddleEnd, m_iWidth, 0, m_iHeight, iTextures[2], pDisplay);
  return 3;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiComboBox::update(double delta)
{
  guiComponent::update(delta);
  if (m_pList->isVisible())
    m_pList->update(delta);
  if (m_dListPos >= 0)
  {
    m_dListPos += 2*delta;
    if (m_dListPos >= 1)
      m_dListPos = -1;
  }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiComboBox::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (!m_bVisible)
    return;
//  if (!m_bEnabled)
//    blendColor = F_RGBA_MULTIPLY(blendColor, rgb(0.3f, 0.3f, 0.3f));
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pGeometry->display(coords, cpntColor);
  m_pLabel->displayAt(iXOffset, iYOffset, cpntColor, docColor);

  if (m_pList->isVisible())
  {
    if (m_dListPos >= 0)
    {
      cpntColor = F_RGBA_MULTIPLY(cpntColor, rgba(1.0f, 1.0f, 1.0f, m_dListPos));
      docColor = F_RGBA_MULTIPLY(docColor, rgba(1.0f, 1.0f, 1.0f, m_dListPos));
    }
    if (docColor.a >= 0)
      docColor.a = (docColor.a + 3) / 4;
    m_pInterface->topDisplay(m_pList, iXOffset, iYOffset, cpntColor, docColor);
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiComboBox::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled || !m_bVisible)
    return NULL;
  if (pEvent->eButton == Button1 && pEvent->eEvent == Event_Down)
  {
    m_pOwner->bringAbove(this);
    if (m_pList->isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset))
      return m_pList->onButtonEvent(pEvent);
    else
    {
      if (m_pList->isVisible())
      {
        m_pList->setVisible(false);
        m_dListPos = -1;
      }
      else
      {
        m_pList->setVisible(true);
        m_dListPos = 0;
      }
    }
    return this;
  }
  else if ((pEvent->eButton == ButtonZ || pEvent->eButton == ButtonX) && m_pList->isVisible())
    return m_pList->onButtonEvent(pEvent);
  return NULL;
}

// -----------------------------------------------------------------
// Name : onFocusLost
// -----------------------------------------------------------------
void guiComboBox::onFocusLost()
{
  m_pList->setVisible(false);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
//  This function will be called when a list button is clicked.
// -----------------------------------------------------------------
bool guiComboBox::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  m_pList->setVisible(false);
  m_pLabel->setText(((guiButton*)pCpnt)->getText());
  centerLabel();
  return m_pOwner->onButtonEvent(pEvent, pCpnt);
}

// -----------------------------------------------------------------
// Name : isAt
// -----------------------------------------------------------------
bool guiComboBox::isAt(int xPxl, int yPxl)
{
  if (!m_bVisible)
    return false;

  if (m_pList->isVisible())
    return guiComponent::isAt(xPxl, yPxl) || m_pList->isAt(xPxl, yPxl);
  else
    return guiComponent::isAt(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : addString
// -----------------------------------------------------------------
guiButton * guiComboBox::addString(const char * sText, const char * sId)
{
  // Create button
  guiButton * pBtn = (guiButton*) m_pListButtonTemplate->clone();
  pBtn->setId(sId);
  pBtn->setText(sText);

  // Find lowest existing button in list
  int yPxl = 0;
  guiComponent * pCpnt = m_pList->getDocument()->getFirstComponent();
  while (pCpnt != NULL)
  {
    yPxl += pCpnt->getHeight();
    pCpnt = m_pList->getDocument()->getNextComponent();
  }

  // Update new button info and add to document
  pBtn->moveBy(0, yPxl);
  m_pList->getDocument()->addComponent(pBtn);
  pBtn->setOwner(this); // change button owner to catch clicks
  m_pList->getDocument()->setHeight(yPxl + pBtn->getHeight());

  return pBtn;
}

// -----------------------------------------------------------------
// Name : setItem
// -----------------------------------------------------------------
void guiComboBox::setItem(int id)
{
  if (id < 0)
    m_pLabel->setText("");
  else
  {
    guiComponent * pCpnt = (guiComponent*) m_pList->getDocument()->getComponentsList()->goTo(0, id);
    if (pCpnt != NULL && pCpnt->getType() & GOTYPE_BUTTON)
    {
      m_pLabel->setText(((guiButton*)pCpnt)->getText());
      centerLabel();
    }
  }
}

// -----------------------------------------------------------------
// Name : getItem
// -----------------------------------------------------------------
guiButton * guiComboBox::getItem(const char * sId)
{
  return (guiButton*) m_pList->getDocument()->getComponent(sId);
}

// -----------------------------------------------------------------
// Name : getItem
// -----------------------------------------------------------------
guiButton * guiComboBox::getItem(u16 uId)
{
  return (guiButton*) m_pList->getDocument()->getComponentsList()->goTo(0, uId);
}

// -----------------------------------------------------------------
// Name : getSelectedItem
// -----------------------------------------------------------------
guiButton * guiComboBox::getSelectedItem()
{
  guiButton * pBtn = (guiButton*) m_pList->getDocument()->getFirstComponent();
  while (pBtn != NULL)
  {
    if (strcmp(pBtn->getText(), m_pLabel->getText()) == 0)
      return pBtn;
    pBtn = (guiButton*) m_pList->getDocument()->getNextComponent();
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getSelectedItemId
// -----------------------------------------------------------------
int guiComboBox::getSelectedItemId()
{
  int id = 0;
  guiButton * pBtn = (guiButton*) m_pList->getDocument()->getFirstComponent();
  while (pBtn != NULL)
  {
    if (strcmp(pBtn->getText(), m_pLabel->getText()) == 0)
      return id;
    id++;
    pBtn = (guiButton*) m_pList->getDocument()->getNextComponent();
  }
  return -1;
}

// -----------------------------------------------------------------
// Name : getItemsCount
// -----------------------------------------------------------------
u16 guiComboBox::getItemsCount()
{
  return (u16) m_pList->getDocument()->getComponentsCount();
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiComboBox::onResize(int iOldWidth, int iOldHeight)
{
  guiComponent::onResize(iOldWidth, iOldHeight);
  if (m_iWidth == iOldWidth && m_iHeight == iOldHeight)
    return;
  m_pList->setWidth(m_iWidth);
  centerLabel();
  if (m_pGeometry != NULL)
  {
    int texlist[3] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2) };
    QuadData ** pQuads;
    int nQuads = computeQuadsList(&pQuads, texlist, getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(nQuads, pQuads);
    QuadData::releaseQuads(nQuads, pQuads);
  }
}

// -----------------------------------------------------------------
// Name : centerLabel
// -----------------------------------------------------------------
void guiComboBox::centerLabel()
{
  if (m_pGeometry != NULL)
  {
    int w1 = getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0))->m_iWidth;
    int w2 = getWidth() - getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(2))->m_iWidth - w1;
    m_pLabel->moveTo(getXPos() + w1 + w2 / 2 - m_pLabel->getWidth() / 2, getYPos() + getHeight() / 2 - m_pLabel->getHeight() / 2);
  }
}

// -----------------------------------------------------------------
// Name : setWidth
// -----------------------------------------------------------------
void guiComboBox::setWidth(int iWidth)
{
  guiComponent::setWidth(iWidth);
  if (iWidth > m_pList->getMaxWidth())
    m_pList->setMaxWidth(iWidth);
  m_pList->setWidth(iWidth);
  m_pListButtonTemplate->setWidth(m_pList->getInnerWidth());
}

// -----------------------------------------------------------------
// Name : setDimensions
// -----------------------------------------------------------------
void guiComboBox::setDimensions(int iWidth, int iHeight)
{
  guiComponent::setDimensions(iWidth, iHeight);
  if (iWidth > m_pList->getMaxWidth())
    m_pList->setMaxWidth(iWidth);
  m_pList->setWidth(iWidth);
  m_pListButtonTemplate->setWidth(m_pList->getInnerWidth());
}

// -----------------------------------------------------------------
// Name : createDefaultComboBox
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiComboBox * guiComboBox::createDefaultComboBox(const char * sId, InterfaceManager * pInterface, DisplayEngine * pDisplay)
{
  guiComboBox * pCombo = new guiComboBox(pInterface);
  int maintexs[3];
  maintexs[0] = pDisplay->getTextureEngine()->findTexture("interface:ComboLeft");
  maintexs[1] = pDisplay->getTextureEngine()->findTexture("interface:ComboMiddle");
  maintexs[2] = pDisplay->getTextureEngine()->findTexture("interface:ComboRight");
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:ComboListBorder");
  pCombo->init(maintexs,
    pDisplay->getTextureEngine()->findTexture("interface:ComboListBg"),
    TEXT_COLOR, TEXT_FONT, FB_FitDocumentToFrame, FB_FitFrameToDocumentWhenSmaller,
    0, 200,
    pDisplay->getTextureEngine()->findTexture("interface:Transparent"),
    pDisplay->getTextureEngine()->findTexture("interface:Transparent"),
    BCO_Enlight, 16,
    frmtex, sId, 0, 0, 200, 28, pDisplay);
  return pCombo;
}
