#include "guiTabbedFrame.h"
#include "guiLabel.h"

// -----------------------------------------------------------------
// Name : guiTabbedFrame
//  Constructor
// -----------------------------------------------------------------
guiTabbedFrame::guiTabbedFrame() : guiFrame()
{
    m_pDocumentsList = new ObjectList(true);
    m_pTabsGeometry = NULL;
    m_iXPanelDecal = 0;
    m_FontId = (FontId)0;
    for (int i = 0; i < 6; i++)
        m_iTexList[i] = -1;
}

// -----------------------------------------------------------------
// Name : ~guiTabbedFrame
//  Destructor
// -----------------------------------------------------------------
guiTabbedFrame::~guiTabbedFrame()
{
    m_pDoc = NULL;   // it's going to be deleted below
    // We need to explicitly delete all guiDocuments as they won't be deleted from guiTabbedFrame_Document destructor
    guiTabbedFrame_Document * pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getFirst(0);
    while (pDoc != NULL)
    {
        delete pDoc->m_pDoc;
        pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getNext(0);
    }
    // Now we can delete the list
    delete m_pDocumentsList;
    FREE(m_pTabsGeometry);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiTabbedFrame::init(int * iTabTexs, FontId fontId, int xdecal, FramePosition positionType, FrameFitBehavior widthFit, FrameFitBehavior heightFit, int iMaxWidth, int iMaxHeight, int * iMainTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
    int iSelTabHeight = pDisplay->getTextureEngine()->getTexture(iTabTexs[0])->m_iHeight;
    guiFrame::init(positionType, widthFit, heightFit, 0, iSelTabHeight, iMaxWidth, iMaxHeight, iMainTexs, sCpntId, xPxl, yPxl, wPxl, hPxl, pDisplay);
    memcpy(m_iTexList, iTabTexs, 6 * sizeof(int));
    m_FontId = fontId;
    m_iXPanelDecal = xdecal;
    computeGeometry(pDisplay);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiTabbedFrame::clone()
{
    int frmtexs[8] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7) };
    guiTabbedFrame * pObj = new guiTabbedFrame();
    pObj->init(m_iTexList, m_FontId, m_iXPanelDecal, m_PositionType, m_WidthFit, m_HeightFit, m_iMaxWidth, m_iMaxHeight, frmtexs, m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
    return pObj;
}

// -----------------------------------------------------------------
// Name : computeGeometry
// -----------------------------------------------------------------
void guiTabbedFrame::computeGeometry(DisplayEngine * pDisplay)
{
    // n Quads
    int nQuads = 3 * m_pDocumentsList->size;
    if (nQuads == 0)
    {
        FREE(m_pTabsGeometry);
        return;
    }
    QuadData ** pQuads = new QuadData*[nQuads];

    int xstart, xend, ystart, yend;
    xstart = 0;
    yend = pDisplay->getTextureEngine()->getTexture(m_iTexList[0])->m_iHeight;
    int boxWidth = (m_iWidth - 2 * m_iXPanelDecal) / m_pDocumentsList->size;
    int i = 0;
    int iTexBase;
    guiTabbedFrame_Document * pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getFirst(0);
    while (pDoc != NULL)
    {
        if (pDoc->m_pDoc == m_pDoc)
        {
            ystart = 0;
            iTexBase = 0;
            pDoc->m_pLabel->setDiffuseColor(rgb(0.0f, 0.0f, 0.0f));
        }
        else
        {
            ystart = yend - pDisplay->getTextureEngine()->getTexture(m_iTexList[3])->m_iHeight;
            iTexBase = 3;
            if (pDoc->m_pDoc->didContentChange())
                pDoc->m_pLabel->setDiffuseColor(rgb(0.2f, 0.2f, 1.0f));
            else
                pDoc->m_pLabel->setDiffuseColor(rgb(0.5f, 0.5f, 0.5f));
        }

        xend = xstart + pDisplay->getTextureEngine()->getTexture(m_iTexList[0 + iTexBase])->m_iWidth;
        pQuads[i++] = new QuadData(xstart, xend, ystart, yend, m_iTexList[0 + iTexBase], pDisplay);
        xstart = xend;
        xend += boxWidth - pDisplay->getTextureEngine()->getTexture(m_iTexList[2 + iTexBase])->m_iWidth - pDisplay->getTextureEngine()->getTexture(m_iTexList[0 + iTexBase])->m_iWidth;
        pQuads[i++] = new QuadData(xstart, xend, ystart, yend, m_iTexList[1 + iTexBase], pDisplay);
        xstart = xend;
        xend += pDisplay->getTextureEngine()->getTexture(m_iTexList[2 + iTexBase])->m_iWidth;
        pQuads[i++] = new QuadData(xstart, xend, ystart, yend, m_iTexList[2 + iTexBase], pDisplay);
        xstart = xend;

        pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getNext(0);
    }

    if (m_pTabsGeometry == NULL)
        m_pTabsGeometry = new GeometryQuads(nQuads, pQuads, VB_Static);
    else
        m_pTabsGeometry->modify(nQuads, pQuads);
    QuadData::releaseQuads(nQuads, pQuads);
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiTabbedFrame::update(double delta)
{
    // Update frame and active document
    guiFrame::update(delta);

    // Update other documents
    guiTabbedFrame_Document * pTbdDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getFirst(0);
    while (pTbdDoc != NULL)
    {
        if (pTbdDoc->m_pDoc != m_pDoc)
            pTbdDoc->m_pDoc->update(delta);
        pTbdDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiTabbedFrame::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
    guiFrame::displayAt(iXOffset, iYOffset, cpntColor, docColor);
    if (m_pDocumentsList->size == 0)
        return;

    //if (m_Effect == FE_FadeAlpha)
    //  blendColor = F_RGBA_MULTIPLY(blendColor, m_EffectColor);
    //else
    //  blendColor = F_RGBA_MULTIPLY(blendColor, m_BlendColor);

//  bool bAlpha = blendColor.a >= 0 && blendColor.a < 1.0f;
//  if (bAlpha)
//    getDisplay()->enableBlending();

    int boxSize = (m_iWidth - 2 * m_iXPanelDecal) / m_pDocumentsList->size;
    if (boxSize < 50)
    {
        boxSize = 50;
        // display with arrows
    }
    else
    {
        CoordsScreen coords = CoordsScreen(iXOffset + m_iXPxl + m_iXPanelDecal, iYOffset + m_iYPxl, GUIPLANE);
        m_pTabsGeometry->display(coords, cpntColor);
        int boxWidth = (m_iWidth - 2 * m_iXPanelDecal) / m_pDocumentsList->size;
        int i = 0;
        guiTabbedFrame_Document * pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getFirst(0);
        while (pDoc != NULL)
        {
            if (pDoc->m_pDoc == m_pDoc)
                pDoc->m_pLabel->setDiffuseColor(rgb(0.0f, 0.0f, 0.0f));
            else
            {
                if (pDoc->m_pDoc->didContentChange())
                    pDoc->m_pLabel->setDiffuseColor(rgb(0.2f, 0.2f, 1.0f));
                else
                    pDoc->m_pLabel->setDiffuseColor(rgb(0.5f, 0.5f, 0.5f));
            }

            int xPxl = m_iXPxl + m_iXPanelDecal + i * boxWidth + (boxWidth - pDoc->m_pLabel->getWidth()) / 2;
            int yPxl = m_iYPxl + 3 + (getDisplay()->getTextureEngine()->getTexture(m_iTexList[0])->m_iHeight - pDoc->m_pLabel->getHeight()) / 2;
            pDoc->m_pLabel->displayAt(iXOffset + xPxl, iYOffset + yPxl, cpntColor);
            i++;
            pDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getNext(0);
        }
    }
//  if (bAlpha)
//    getDisplay()->disableBlending();
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiTabbedFrame::onButtonEvent(ButtonAction * pEvent)
{
    if (!m_bEnabled || !m_bVisible || m_pDoc == NULL)
        return NULL;

    int iPanel = getIPanel(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset);
    if (iPanel < 0)
        return guiFrame::onButtonEvent(pEvent);
    if (pEvent->eButton != Button1 || pEvent->eEvent != Event_Down)
        return NULL;

    int boxSize = (m_iWidth - 2 * m_iXPanelDecal) / m_pDocumentsList->size;
    if (boxSize < 50)
    {
        boxSize = 50;
        // special case TODO
    }
    else
        setMainDocument(((guiTabbedFrame_Document*) ((*m_pDocumentsList)[iPanel]))->m_pDoc);
    return NULL;  // We don't need to catch future drag or dbclick events
}

// -----------------------------------------------------------------
// Name : attachDocument
// -----------------------------------------------------------------
void guiTabbedFrame::attachDocument(guiDocument * pDoc, FrameFitBehavior OldWidthFit, FrameFitBehavior OldHeightFit, DisplayEngine * pDisplay)
{
    guiFrame::setDocument(pDoc);
    m_pDocumentsList->addFirst(new guiTabbedFrame_Document(pDoc, OldWidthFit, OldHeightFit, m_FontId, pDisplay));
    computeGeometry(getDisplay());
}

// -----------------------------------------------------------------
// Name : detachDocument
// -----------------------------------------------------------------
guiTabbedFrame_Document * guiTabbedFrame::detachDocument()
{
    guiDocument * pDoc = guiFrame::unsetDocument();
    if (pDoc == NULL)
        return NULL;
    if (m_pDocumentsList->size < 2)
    {
        setDocument(pDoc);
        return NULL;
    }

    guiTabbedFrame_Document * pTbdDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getFirst(0);
    while (pTbdDoc != NULL)
    {
        if (pTbdDoc->m_pDoc == pDoc)
        {
            m_pDocumentsList->deleteCurrent(0, true, true);
            break;
        }
        pTbdDoc = (guiTabbedFrame_Document*) m_pDocumentsList->getNext(0);
    }
    if (m_pDocumentsList->size == 0)
        m_pDoc = NULL;
    else
        setMainDocument(((guiTabbedFrame_Document*)m_pDocumentsList->getFirst(0))->m_pDoc);

    return pTbdDoc;
}

// -----------------------------------------------------------------
// Name : setMainDocument
// -----------------------------------------------------------------
void guiTabbedFrame::setMainDocument(guiDocument * pDoc)
{
    if (m_pDoc != pDoc)
    {
        m_pDoc = pDoc;
        computeGeometry(getDisplay());
    }
}

// -----------------------------------------------------------------
// Name : isPanelAt
// -----------------------------------------------------------------
bool guiTabbedFrame::isPanelAt(int xPxl, int yPxl)
{
    return (xPxl >= m_iXPxl + m_iXPanelDecal && xPxl <= m_iXPxl + m_iWidth - m_iXPanelDecal && yPxl >= m_iYPxl && yPxl <= m_iYPxl + (int)getDisplay()->getTextureEngine()->getTexture(m_iTexList[0])->m_iHeight);
}

// -----------------------------------------------------------------
// Name : getIPanel
// -----------------------------------------------------------------
short guiTabbedFrame::getIPanel(int xPxl, int yPxl)
{
    if (!isPanelAt(xPxl, yPxl))
        return -1;
    return ((xPxl - m_iXPxl - m_iXPanelDecal) * m_pDocumentsList->size) / (m_iWidth - 2 * m_iXPanelDecal);
}

// -----------------------------------------------------------------
// Name : isAt
// -----------------------------------------------------------------
bool guiTabbedFrame::isAt(int xPxl, int yPxl)
{
    if (m_bVisible && isPanelAt(xPxl, yPxl))
        return true;
    return guiFrame::isAt(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void guiTabbedFrame::moveTo(int xPxl, int yPxl)
{
    guiFrame::moveTo(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : createDefaultTabbedFrame
//  Static default constructor
// -----------------------------------------------------------------
guiTabbedFrame * guiTabbedFrame::createDefaultTabbedFrame(FrameFitBehavior widthFit, FrameFitBehavior heightFit, int width, int height, bool bAlpha, char * sId, DisplayEngine * pDisplay)
{
    guiTabbedFrame * pFrame = new guiTabbedFrame();
    int iTabs[6];
    iTabs[0] = pDisplay->getTextureEngine()->findTexture("interface:TabSelLeft");
    iTabs[1] = pDisplay->getTextureEngine()->findTexture("interface:TabSelMiddle");
    iTabs[2] = pDisplay->getTextureEngine()->findTexture("interface:TabSelRight");
    iTabs[3] = pDisplay->getTextureEngine()->findTexture("interface:TabLeft");
    iTabs[4] = pDisplay->getTextureEngine()->findTexture("interface:TabMiddle");
    iTabs[5] = pDisplay->getTextureEngine()->findTexture("interface:TabRight");
    int iTexs[8];
    iTexs[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmTL");
    iTexs[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
    iTexs[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
    iTexs[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmCL");
    iTexs[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
    iTexs[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmBL");
    iTexs[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
    iTexs[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
    pFrame->init(iTabs, Bookantiqua_wh_16,
                 3, FP_Floating, widthFit, heightFit, 0, 0,
                 iTexs, sId, 0, 0, width, height, pDisplay);
    return pFrame;
}







// -----------------------------------------------------------------
// Name : guiTabbedFrame_Document
//  Default constructor for guiTabbedFrame_Document
// -----------------------------------------------------------------
guiTabbedFrame_Document::guiTabbedFrame_Document(guiDocument * pDoc, FrameFitBehavior OldWidthFit, FrameFitBehavior OldHeightFit, FontId fontId, DisplayEngine * pDisplay)
{
    m_pDoc = pDoc;
    m_OldWidthFit = OldWidthFit;
    m_OldHeightFit = OldHeightFit;
    m_pLabel = new guiLabel();
    m_pLabel->init(pDoc->getTitle(), fontId, rgb(0,0,0), "", 0, 0, 0, 0, pDisplay);
}

// -----------------------------------------------------------------
// Name : ~guiTabbedFrame_Document
//  Default destructor for guiTabbedFrame_Document
// -----------------------------------------------------------------
guiTabbedFrame_Document::~guiTabbedFrame_Document()
{
    // Do not delete m_pDoc as we may want to delete a guiTabbedFrame_Document but keep the document
    delete m_pLabel;
}
