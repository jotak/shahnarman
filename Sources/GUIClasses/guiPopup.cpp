#include "guiPopup.h"
#include "../Data/LocalisationTool.h"
#include "FrameEffects/EffectComeIn.h"
#include "FrameEffects/guiFrameOutro.h"
#include "FrameEffects/guiFrameMouseFocus.h"

// -----------------------------------------------------------------
// Name : guiPopup
//  Constructor ; do not use (private)
// -----------------------------------------------------------------
guiPopup::guiPopup() : guiFrame()
{
  m_fTimer = -1;
  m_bAutoClose = false;
}

// -----------------------------------------------------------------
// Name : ~guiPopup
//  Destructor
// -----------------------------------------------------------------
guiPopup::~guiPopup()
{
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiPopup::update(double delta)
{
  if (!m_bVisible)
    return;
  guiFrame::update(delta);
  if (m_fTimer >= 1)
  {
    m_fTimer -= delta;
    if (m_fTimer < 1)
    {
      guiFrameOutro * pEffect = new guiFrameOutro(POPUP_EFFECT_OUTRO, 1.0f, EFFECT_DELFRM_ON_FINISHED);
      addEffect(pEffect);
      pEffect->setActive(true);
    }
  }
  if (m_bAutoClose)
  {
    guiComponent * pCpnt = getClickedComponent();
    if (pCpnt != NULL)
    {
      if (strcmp(pCpnt->getId(), "OkButton") == 0)
      {
        getDocument()->close();
        m_bAutoClose = false;
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : getButton
// -----------------------------------------------------------------
guiButton * guiPopup::getButton(int iButton)
{
  char str[64] = "0";
  str[0] = iButton - '0';
  return (guiButton*) getDocument()->getComponent(str);
}

// -----------------------------------------------------------------
// Name : createEmptyPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createEmptyPopup(DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "EmptyPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "EmptyPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pDisplay);
  pPopup->setDocument(pDoc);

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createOkAutoclosePopup
// -----------------------------------------------------------------
guiPopup * guiPopup::createOkAutoclosePopup(const char * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "OkAutoclosePopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "OkAutoclosePopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Button
  yPxl += pLbl->getHeight() + 20;
  char str[64] = "";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("OK", str, 64), "OkButton", pDisplay);
  pBtn->moveTo(iWidth / 2 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  // Init popup
  pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 10);
  pPopup->setDocument(pDoc);
  pPopup->m_bAutoClose = true;

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createOkCancelPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createOkCancelPopup(const char * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "OkCancelPopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "OkCancelPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  char str[64] = "";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("OK", str, 64), "OkButton", pDisplay);
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("CANCE", str, 64), "CancelButton", pDisplay);
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  // Init popup
  pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 10);
  pPopup->setDocument(pDoc);

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createYesNoPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createYesNoPopup(const char * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "YesNoPopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "YesNoPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  char str[64] = "";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("YES", str, 64), "YesButton", pDisplay);
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("NO", str, 64), "NoButton", pDisplay);
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  // Init popup
  pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 10);
  pPopup->setDocument(pDoc);

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createTextAndMultiButtonsPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createTextAndMultiButtonsPopup(const char * sText, int iNbButtons, int iWidth, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "MultiButtonsPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "MultiButtonsPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Init position for components
  int yPxl = 10;

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  int height = yPxl;
  char str[64] = "0";
  int boxw = iNbButtons > 0 ? iWidth / iNbButtons : 0;
  for (int i = 0; i < iNbButtons; i++)
  {
    str[0] = '0' + i;
    guiButton * pBtn = guiButton::createDefaultNormalButton("", str, pDisplay);
    pBtn->moveTo(i * boxw + boxw / 2 - pBtn->getWidth() / 2, yPxl);
    pDoc->addComponent(pBtn);
    height = yPxl + pBtn->getHeight();
  }

  // Init popup
  pDoc->setDimensions(iWidth, height + 10);
  pPopup->setDocument(pDoc);

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createTextInputPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createTextInputPopup(const char * sText, int iNbLines, bool bMultiLines, int iBoxWidth, InputEngine * pInput, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "TextInputPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "TextInputPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pDisplay);

  // Init size & position for components
  int iWidth = iBoxWidth + 10;
  int yPxl = 10;

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iBoxWidth, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  yPxl += pLbl->getHeight() + 10;
  if (pInput->hasKeyboard())
  {
    // Edit box
    guiEditBox * pBox = guiEditBox::createDefaultEditBox(iNbLines, bMultiLines, iBoxWidth, "DefaultEditBox", (KeyboardInputEngine*)pInput, pDisplay);
    pBox->moveTo(5, yPxl);
    pDoc->addComponent(pBox);
    pBox->setFocus();
    yPxl += pBox->getHeight() + 20;
  }
  else
  {
    // TODO : Virtual keyboard
  }

  // Buttons
  char str[64] = "";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("CANCE", str, 64), "CancelButton", pDisplay);
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp("OK", str, 64), "OkButton", pDisplay);
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  // Init popup
  pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 10);
  pPopup->setDocument(pDoc);

  guiFrameEffect * pEffect = new guiFrameMouseFocus(POPUP_EFFECT_FOCUS, 1.0f);
  pPopup->addEffect(pEffect);

  pEffect = new EffectComeIn(POPUP_EFFECT_INTRO, 0.3f);
  pPopup->addEffect(pEffect);
  pEffect->reset();

  return pPopup;
}

// -----------------------------------------------------------------
// Name : createTimedPopup
//  Static default constructor
// -----------------------------------------------------------------
guiPopup * guiPopup::createTimedPopup(const char * sText, double fTimer, int iWidth, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmT");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmC");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmB");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, "TimedPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    "TimedPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, 10);
  pDoc->addComponent(pLbl);

  // Init popup
  pDoc->setDimensions(iWidth, pLbl->getHeight() + 20);
  pPopup->setDocument(pDoc);

  pPopup->m_fTimer = fTimer;
  return pPopup;
}
