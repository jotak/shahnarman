#include "guiList.h"
#include "../Input/KeyboardInputEngine.h"

// -----------------------------------------------------------------
// Name : guiList
// -----------------------------------------------------------------
guiList::guiList(KeyboardInputEngine * pInputs) : EventListener(0)
{
    m_pInputs = pInputs;
    m_pSelectionGeometry = NULL;
    m_FontId = (FontId)0;
    m_TextColor = F_RGBA_NULL;
    m_bHasFocus = false;
    m_pLastSelectedLabel = NULL;
    m_bCatchMouseUp = false;
    m_ActionOnSelection = (InputButton)0;
}

// -----------------------------------------------------------------
// Name : ~guiList
//  Destructor
// -----------------------------------------------------------------
guiList::~guiList()
{
    if (m_bHasFocus)
        onFocusLost();
    FREE(m_pSelectionGeometry);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiList::init(FontId fontId, F_RGBA textColor, int * iMainTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
    guiContainer::init(FB_FitDocumentToFrameWhenSmaller, FB_FitDocumentToFrameWhenSmaller, 0, 0, 0, 0, iMainTexs, sCpntId, xPxl, yPxl, wPxl, hPxl, pDisplay);
    m_FontId = fontId;
    m_TextColor = textColor;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiList::clone()
{
    int texlist[8] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7) };
    guiList * pObj = new guiList(m_pInputs);
    pObj->init(m_FontId, m_TextColor, texlist, m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
    return pObj;
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiList::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
    if (!m_bVisible)
        return;

    guiContainer::displayAt(iXOffset, iYOffset, cpntColor, docColor);

    if (m_pSelectionGeometry != NULL)
    {
        // display selection
        CoordsScreen coords(m_iInnerXPxl + iXOffset, m_iInnerYPxl + iYOffset, GUIPLANE);
        m_pStencilGeometry->fillStencil(coords, true);
        int iPreviousState = getDisplay()->setStencilState(2);
        F_RGBA color = F_RGBA_MULTIPLY(cpntColor, rgba(0.4, 0.4, 0.4, 0.5));
        CoordsScreen selcoords = coords + CoordsScreen(getDocument()->getXPos(), getDocument()->getYPos());
        m_pSelectionGeometry->display(selcoords, color);
        m_pStencilGeometry->fillStencil(coords, false);
        getDisplay()->setStencilState(iPreviousState);
    }
}

// -----------------------------------------------------------------
// Name : onCatchButtonEvent
//  Cursor-independant buttons (directly called from inputs)
// -----------------------------------------------------------------
bool guiList::onCatchButtonEvent(ButtonAction * pEvent)
{
    if ((pEvent->eEvent != Event_Down && pEvent->eEvent != Event_DoubleClick) || getDocument()->getComponentsCount() == 0)
        return false;
    switch (pEvent->eButton)
    {
    case ButtonUp:
    case ButtonDown:
    {
        guiListLabel * pClicked = NULL;
        if (m_pLastSelectedLabel == NULL)
            pClicked = (guiListLabel*) getDocument()->getFirstComponent();
        else
        {
            getDocument()->getComponentsList()->goTo(0, m_pLastSelectedLabel);
            pClicked = (pEvent->eButton == ButtonUp) ?
                       (guiListLabel*) getDocument()->getComponentsList()->getPrev(0) :
                       (guiListLabel*) getDocument()->getComponentsList()->getNext(0);
        }
        if (pClicked != NULL)
        {
            if (m_pInputs->isShiftPressed())
                shiftSelect(pClicked);
            else
            {
                guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
                while (pLbl != NULL)
                {
                    pLbl->setSelected(pLbl == pClicked);
                    pLbl = (guiListLabel*) getDocument()->getNextComponent();
                }
                updateSelectionGeometry();
            }
            m_pLastSelectedLabel = pClicked;
        }
        break;
    }
    default:
    {
        if (pEvent->eEvent == Event_Down)
            m_ActionOnSelection = pEvent->eButton;
        break;
    }
    }
    return false;
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiList::onButtonEvent(ButtonAction * pEvent)
{
    if (!m_bEnabled || !m_bVisible || m_pDoc == NULL)
        return NULL;

    if (m_pInputs != NULL && pEvent->eButton == Button1 && pEvent->eEvent == Event_Down)
    {
        // Get focus
        setFocus();
        m_bCatchMouseUp = false;
        guiObject * pObj = guiContainer::onButtonEvent(pEvent);
        if (pObj == NULL)
        {
            // Deselect all
            guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
            while (pLbl != NULL)
            {
                pLbl->setSelected(false);
                pLbl = (guiListLabel*) getDocument()->getNextComponent();
            }
            m_pLastSelectedLabel = NULL;
            updateSelectionGeometry();
        }
        return pObj;
    }
    return guiContainer::onButtonEvent(pEvent);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool guiList::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (pEvent->eEvent == Event_Down)
    {
        if (m_pInputs != NULL && pEvent->eButton == Button2)
            return false;

        guiListLabel * pClickedLabel = (guiListLabel*) pCpnt;
        if (m_pInputs != NULL && pEvent->eButton == Button1 && m_pInputs->isShiftPressed())
            shiftSelect(pClickedLabel);
        else if ((m_pInputs != NULL && pEvent->eButton == Button1 && m_pInputs->isCtrlPressed()) || (m_pInputs == NULL && pEvent->eButton == Button2))
        {
            // Add/remove current label to/from selection
            pClickedLabel->setSelected(!pClickedLabel->isSelected());
            updateSelectionGeometry();
        }
        else
        {
            if (!pClickedLabel->isSelected())
            {
                guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
                while (pLbl != NULL)
                {
                    pLbl->setSelected(pLbl == pClickedLabel);
                    pLbl = (guiListLabel*) getDocument()->getNextComponent();
                }
                updateSelectionGeometry();
            }
            else  // We'll do deselection on mouse up to allow dragging
                m_bCatchMouseUp = true;
        }
        m_pLastSelectedLabel = pClickedLabel;
        return true;
    }
    else if (pEvent->eEvent == Event_Drag)
    {
        m_bCatchMouseUp = false;
        return m_pOwner->onButtonEvent(pEvent, this);
    }
    else if (pEvent->eEvent == Event_Up)
    {
        bool bReturn = true;
        if (m_bCatchMouseUp && m_pLastSelectedLabel != NULL)
        {
            // Clear current selection and add label to it
            guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
            while (pLbl != NULL)
            {
                pLbl->setSelected(pLbl == m_pLastSelectedLabel);
                pLbl = (guiListLabel*) getDocument()->getNextComponent();
            }
            updateSelectionGeometry();
        }
        else if (!m_bCatchMouseUp)
            bReturn = m_pOwner->onButtonEvent(pEvent, this);
        m_bCatchMouseUp = false;
        return bReturn;
    }
    return true;
}

// -----------------------------------------------------------------
// Name : shiftSelect
// -----------------------------------------------------------------
void guiList::shiftSelect(guiListLabel * pClickedLabel)
{
    if (m_pLastSelectedLabel == NULL)
    {
        // Nothing selected yet ; only select clicked label
        pClickedLabel->setSelected(true);
    }
    else
    {
        // Select or deselect all labels between last selected label and current label
        bool bSelect = !pClickedLabel->isSelected();
        pClickedLabel->setSelected(bSelect);
        m_pLastSelectedLabel->setSelected(bSelect);
        if (pClickedLabel != m_pLastSelectedLabel)
        {
            bool bInBlock = false;
            guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
            while (pLbl != NULL)
            {
                if (!bInBlock && (pLbl == pClickedLabel || pLbl == m_pLastSelectedLabel))
                    bInBlock = true;
                else if (bInBlock)
                {
                    if (pLbl == pClickedLabel || pLbl == m_pLastSelectedLabel)
                        break;
                    pLbl->setSelected(bSelect);
                }
                pLbl = (guiListLabel*) getDocument()->getNextComponent();
            }
        }
    }
    updateSelectionGeometry();
}

// -----------------------------------------------------------------
// Name : onFocusLost
// -----------------------------------------------------------------
void guiList::onFocusLost()
{
    m_bHasFocus = false;
    if (m_pInputs != NULL)
    {
        m_pInputs->unsetKeyboardListener(this);
        EventListener * p = m_pInputs->popUncursoredEventListener();
        if (p != this && p != NULL)  // oops
            m_pInputs->pushUncursoredEventListener(p);
    }
}

// -----------------------------------------------------------------
// Name : onKeyDown
//  Catch some events like "Ctrl+a" to select all
//  See also guiList::onSpecialKeyDown
// -----------------------------------------------------------------
bool guiList::onKeyDown(unsigned char c)
{
    if (m_bHasFocus)
    {
        // ctrl+A = select all
        if (c == 1)  // It looks like ascii 1 is "ctrl+a". TODO : Must be tested on different systems!
        {
            guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
            while (pLbl != NULL)
            {
                pLbl->setSelected(true);
                pLbl = (guiListLabel*) getDocument()->getNextComponent();
            }
            updateSelectionGeometry();
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------
// Name : updateSelectionGeometry
// -----------------------------------------------------------------
void guiList::updateSelectionGeometry()
{
    // First, count number of selected items
    int nSelLabels = 0;
    guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
    while (pLbl != NULL)
    {
        if (pLbl->isSelected())
            nSelLabels++;
        pLbl = (guiListLabel*) getDocument()->getNextComponent();
    }

    // Get or load texture
    int texture = (m_pSelectionGeometry == NULL) ? getDisplay()->getTextureEngine()->loadTexture("EmptyWhiteSquare") : m_pSelectionGeometry->getTexture();
    if (nSelLabels == 0)
    {
        // Exit
        FREE(m_pSelectionGeometry);
        m_pLastSelectedLabel = NULL;
        return;
    }

    // Loop through labels again to build geometry
    QuadData ** pQuads = new QuadData*[nSelLabels];
    int iQuad = 0;
    pLbl = (guiListLabel*) getDocument()->getFirstComponent();
    while (pLbl != NULL)
    {
        if (pLbl->isSelected())
            pQuads[iQuad++] = new QuadData(0, m_iInnerWidth, pLbl->getYPos(), pLbl->getYPos() + pLbl->getHeight(), texture, getDisplay());
        pLbl = (guiListLabel*) getDocument()->getNextComponent();
    }

    if (m_pSelectionGeometry == NULL)
        m_pSelectionGeometry = new GeometryQuads(nSelLabels, pQuads, VB_Static);
    else
        m_pSelectionGeometry->modify(nSelLabels, pQuads);
    QuadData::releaseQuads(nSelLabels, pQuads);
}

// -----------------------------------------------------------------
// Name : setFocus
// -----------------------------------------------------------------
void guiList::setFocus()
{
    m_bHasFocus = true;
    if (m_pInputs != NULL)
    {
        m_pInputs->setKeyboardListener(this);
        m_pInputs->pushUncursoredEventListener(this);
    }
}

// -----------------------------------------------------------------
// Name : clear
// -----------------------------------------------------------------
void guiList::clear()
{
    getDocument()->deleteAllComponents();
    updateSelectionGeometry();
}

// -----------------------------------------------------------------
// Name : sort
// -----------------------------------------------------------------
void guiList::sort()
{
    SortInterface::sort(getDocument()->getComponentsList());
    int yPxl = 0;
    guiComponent * pCpnt = getDocument()->getFirstComponent();
    while (pCpnt != NULL)
    {
        pCpnt->moveTo(pCpnt->getXPos(), yPxl);
        yPxl += pCpnt->getHeight();
        pCpnt = getDocument()->getNextComponent();
    }
}

// -----------------------------------------------------------------
// Name : sortCompare
// -----------------------------------------------------------------
bool guiList::sortCompare(BaseObject * A, BaseObject * B)
{
    guiListLabel * pLblA = (guiListLabel*) A;
    guiListLabel * pLblB = (guiListLabel*) B;
    return (strcmp(pLblA->getText(), pLblB->getText()) > 0);
}

// -----------------------------------------------------------------
// Name : addItem
// -----------------------------------------------------------------
guiList::guiListLabel * guiList::addItem(char * sText, char * sId)
{
    guiComponent * pCpnt = getDocument()->getLastComponent();
    int yPxl = (pCpnt == NULL) ? 0 : pCpnt->getYPos() + pCpnt->getHeight();
    guiListLabel * pLbl = new guiListLabel();
    pLbl->init(sText, m_FontId, m_TextColor, sId, 5, yPxl, m_iInnerWidth - 10, 0, getDisplay());
    getDocument()->addComponent(pLbl);
    getDocument()->setHeight(yPxl + pLbl->getHeight());
    return pLbl;
}

// -----------------------------------------------------------------
// Name : removeSelection
// -----------------------------------------------------------------
void guiList::removeSelection()
{
    int yPxl = 0;
    guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
    while (pLbl != NULL)
    {
        if (pLbl->isSelected())
            pLbl = (guiListLabel*) getDocument()->deleteCurrentComponent(true);
        else
        {
            pLbl->moveTo(pLbl->getXPos(), yPxl);
            yPxl += pLbl->getHeight();
            pLbl = (guiListLabel*) getDocument()->getNextComponent();
        }
    }
    updateSelectionGeometry();
}

// -----------------------------------------------------------------
// Name : getFirstSelectedItem
// -----------------------------------------------------------------
guiList::guiListLabel * guiList::getFirstSelectedItem()
{
    guiListLabel * pLbl = (guiListLabel*) getDocument()->getFirstComponent();
    while (pLbl != NULL)
    {
        if (pLbl->isSelected())
            return pLbl;
        pLbl = (guiListLabel*) getDocument()->getNextComponent();
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextSelectedItem
// -----------------------------------------------------------------
guiList::guiListLabel * guiList::getNextSelectedItem()
{
    guiListLabel * pLbl = (guiListLabel*) getDocument()->getNextComponent();
    while (pLbl != NULL)
    {
        if (pLbl->isSelected())
            return pLbl;
        pLbl = (guiListLabel*) getDocument()->getNextComponent();
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : guiListDocument::onButtonEvent
// -----------------------------------------------------------------
bool guiList::guiListDocument::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    return m_pOwner->onButtonEvent(pEvent, pCpnt);
}

// -----------------------------------------------------------------
// Name : guiListLabel::onButtonEvent
// -----------------------------------------------------------------
guiObject * guiList::guiListLabel::onButtonEvent(ButtonAction * pEvent)
{
    if (!m_bEnabled || !m_bVisible)
        return NULL;

    if (pEvent->eButton == Button1 || pEvent->eButton == Button2)
    {
        if (m_pOwner->onButtonEvent(pEvent, this))
            return this;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : createDefaultList
//  Static default constructor
// -----------------------------------------------------------------
guiList * guiList::createDefaultList(int width, int height, const char * sId, KeyboardInputEngine * pInputs, DisplayEngine * pDisplay)
{
    int frmtex[8];
    frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:LstTL");
    frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:LstTC");
    frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:LstTR");
    frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:LstCL");
    frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:LstCR");
    frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:LstBL");
    frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:LstBC");
    frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:LstBR");
    guiList * pBox = new guiList(pInputs);
    pBox->init(
        TEXT_FONT, TEXT_COLOR, frmtex,
        sId, 0, 0, width, height, pDisplay);

    // Attach document
    guiDocument * pDoc = new guiListDocument(pBox);
    pDoc->init(
        "",
        pDisplay->getTextureEngine()->findTexture("interface:ComboListBg"),
        0, 0, 1, 1, pDisplay);
    pBox->setDocument(pDoc);

    return pBox;
}
