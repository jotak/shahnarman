#include "guiDocument.h"
#include "guiToggleButton.h"
#include "guiContainer.h"
#include "../Geometries/GeometryQuads.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : guiDocument
//  Constructor
// -----------------------------------------------------------------
guiDocument::guiDocument()
{
  m_pComponentsList = new ObjectList(true);
  m_iCpntIt1 = m_pComponentsList->getIterator();
  m_iCpntIt2 = m_pComponentsList->getIterator();
  m_pFocusedComponent = NULL;
  m_bNeedDestroy = false;
  m_bContentChanged = false;
  m_bEnabled = true;
  wsafecpy(m_sTitle, 32, L"");
  m_pOwner = NULL;
}

// -----------------------------------------------------------------
// Name : ~guiDocument
//  Destructor
// -----------------------------------------------------------------
guiDocument::~guiDocument()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy guiDocument\n");
#endif
  delete m_pComponentsList;
#ifdef DBG_VERBOSE1
  printf("End destroy guiDocument\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiDocument::init(const wchar_t * sTitle, int iTexId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiObject::init(xPxl, yPxl, wPxl, hPxl);
  wsafecpy(m_sTitle, 32, sTitle);

  QuadData quad(0, m_iWidth, 0, m_iHeight, iTexId, pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
  onLoad();
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiDocument::clone()
{
  guiDocument * pDoc = new guiDocument();
  pDoc->init(m_sTitle, ((GeometryQuads*)m_pGeometry)->getTexture(), m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pDoc;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiDocument::update(double delta)
{
  if (!m_bEnabled)
    return;
  guiComponent * cpnt = (guiComponent*) m_pComponentsList->getFirst(m_iCpntIt1);
  while (cpnt != NULL)
  {
    cpnt->update(delta);
    cpnt = (guiComponent*) m_pComponentsList->getNext(m_iCpntIt1);
  }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiDocument::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  // Display document background
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pGeometry->display(coords, docColor);

  // Display components
  guiComponent * cpnt = (guiComponent*) m_pComponentsList->getFirst(m_iCpntIt1);
  while (cpnt != NULL)
  {
    cpnt->displayAt(m_iXPxl + iXOffset, m_iYPxl + iYOffset, cpntColor, docColor);
    cpnt = (guiComponent*) m_pComponentsList->getNext(m_iCpntIt1);
  }
  m_bContentChanged = false;
}

// -----------------------------------------------------------------
// Name : getComponentAt
// -----------------------------------------------------------------
guiComponent * guiDocument::getComponentAt(int xPxl, int yPxl)
{
  guiComponent * cpnt = (guiComponent*) m_pComponentsList->getLast(m_iCpntIt1);
  while (cpnt != NULL)
  {
    if (cpnt->isAt(xPxl, yPxl))
      return cpnt;
    cpnt = (guiComponent*) m_pComponentsList->getPrev(m_iCpntIt1);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiDocument::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled)
    return NULL;

  if (pEvent->eEvent == Event_Down)
  {
    guiComponent * cpnt = getComponentAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
    if (cpnt != m_pFocusedComponent && pEvent->eButton == Button1)
      setFocusedComponent(cpnt);
    if (cpnt != NULL)
    {
      guiObject * pObj = cpnt->onButtonEvent(pEvent);
      if (pObj != NULL)
        return pObj;
    }
    if (pEvent->eButton == Button2)
      return this;  // Drag (=scroll) document
  }
  else if (pEvent->eEvent == Event_Drag)
  {
    moveBy(pEvent->xPos - pEvent->xPosInit, pEvent->yPos - pEvent->yPosInit);
    m_pOwner->checkDocumentPosition();
    return this;
  }

  return NULL;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * guiDocument::onCursorMoveEvent(int xPxl, int yPxl)
{
  guiComponent * cpnt = getComponentAt(xPxl, yPxl);
  if (cpnt != NULL)
    return cpnt->onCursorMoveEvent(xPxl, yPxl);
  else
    return this;
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiDocument::onResize(int iOldWidth, int iOldHeight)
{
  guiObject::onResize(iOldWidth, iOldHeight);
  if (m_iWidth == iOldWidth && m_iHeight == iOldHeight)
    return;
  if (m_pGeometry != NULL)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, ((GeometryQuads*)m_pGeometry)->getTexture(), getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(&quad);
  }
}

// -----------------------------------------------------------------
// Name : setFocusedComponent
// -----------------------------------------------------------------
void guiDocument::setFocusedComponent(guiComponent * pCpnt)
{
  if (m_pFocusedComponent != NULL)
    m_pFocusedComponent->onFocusLost();
  m_pFocusedComponent = pCpnt;
}

// -----------------------------------------------------------------
// Name : setTitleId
// -----------------------------------------------------------------
void guiDocument::setTitleId(const wchar_t * sTitleId)
{
  i18n->getText1stUp(sTitleId, m_sTitle, 32);
}

// -----------------------------------------------------------------
// Name : getComponent
// -----------------------------------------------------------------
guiComponent * guiDocument::getComponent(wchar_t * cpntId)
{
  guiComponent * cpnt = (guiComponent*) m_pComponentsList->getFirst(0);
  while (cpnt != NULL)
  {
    if (wcscmp(cpntId, cpnt->getId()) == 0)
      return cpnt;
    cpnt = (guiComponent*) m_pComponentsList->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : deleteAllComponents
// -----------------------------------------------------------------
void guiDocument::deleteAllComponents()
{
  m_pComponentsList->deleteAll();
  m_pFocusedComponent = NULL;
  m_bContentChanged = true;
}

// -----------------------------------------------------------------
// Name : deleteCurrentComponent
// -----------------------------------------------------------------
guiComponent * guiDocument::deleteCurrentComponent(bool bSetToNext)
{
  if (m_pFocusedComponent == m_pComponentsList->getCurrent(m_iCpntIt2))
    m_pFocusedComponent = NULL;
  m_bContentChanged = true;
  return (guiComponent*) m_pComponentsList->deleteCurrent(m_iCpntIt2, bSetToNext, false);
}

// -----------------------------------------------------------------
// Name : addComponent
// -----------------------------------------------------------------
void guiDocument::addComponent(guiComponent * cpnt)
{
  m_pComponentsList->addLast(cpnt);
  if (cpnt->getOwner() == NULL)
    cpnt->setOwner(this);
}

// -----------------------------------------------------------------
// Name : bringAbove
// -----------------------------------------------------------------
void guiDocument::bringAbove(guiComponent * cpnt)
{
  if (m_pComponentsList->goTo(0, cpnt))
    m_pComponentsList->moveCurrentToEnd(0);
}

// -----------------------------------------------------------------
// Name : doClick
// -----------------------------------------------------------------
void guiDocument::doClick(wchar_t * sCpntId)
{
  guiComponent * pCpnt = getComponent(sCpntId);
  if (pCpnt == NULL || !pCpnt->isVisible() || !pCpnt->isEnabled())
    return;
  ButtonAction action;
  action.eButton = Button1;
  action.eEvent = Event_Down;
  action.xOffset = action.yOffset = 0;
  action.xPos = action.xPosInit = pCpnt->getXPos() + 1;
  action.yPos = action.yPosInit = pCpnt->getYPos() + 1;
  pCpnt->onButtonEvent(&action);
  action.eEvent = Event_Up;
  pCpnt->onButtonEvent(&action);
}
