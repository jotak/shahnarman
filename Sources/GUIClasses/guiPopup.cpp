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
      if (wcscmp(pCpnt->getId(), L"OkButton") == 0)
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
  wchar_t str[64] = L"0";
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
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"EmptyPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"EmptyPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
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
guiPopup * guiPopup::createOkAutoclosePopup(const wchar_t * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"OkAutoclosePopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"OkAutoclosePopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Button
  yPxl += pLbl->getHeight() + 20;
  wchar_t str[64] = L"";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"OK", str, 64), L"OkButton", pDisplay);
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
guiPopup * guiPopup::createOkCancelPopup(const wchar_t * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"OkCancelPopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"OkCancelPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  wchar_t str[64] = L"";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"OK", str, 64), L"OkButton", pDisplay);
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"CANCEL", str, 64), L"CancelButton", pDisplay);
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
guiPopup * guiPopup::createYesNoPopup(const wchar_t * sText, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"YesNoPopup", 0, 0, 1, 1, pDisplay);

  // Init position for components
  int iWidth = 250;
  int yPxl = 10;

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"YesNoPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  wchar_t str[64] = L"";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"YES", str, 64), L"YesButton", pDisplay);
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"NO", str, 64), L"NoButton", pDisplay);
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
guiPopup * guiPopup::createTextAndMultiButtonsPopup(const wchar_t * sText, int iNbButtons, int iWidth, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"MultiButtonsPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"MultiButtonsPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Init position for components
  int yPxl = 10;

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  // Buttons
  yPxl += pLbl->getHeight() + 20;
  int height = yPxl;
  wchar_t str[64] = L"0";
  int boxw = iNbButtons > 0 ? iWidth / iNbButtons : 0;
  for (int i = 0; i < iNbButtons; i++)
  {
    str[0] = L'0' + i;
    guiButton * pBtn = guiButton::createDefaultNormalButton(L"", str, pDisplay);
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
guiPopup * guiPopup::createTextInputPopup(const wchar_t * sText, int iNbLines, bool bMultiLines, int iBoxWidth, InputEngine * pInput, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"TextInputPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"TextInputPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pDisplay);

  // Init size & position for components
  int iWidth = iBoxWidth + 10;
  int yPxl = 10;

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iBoxWidth, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
  pDoc->addComponent(pLbl);

  yPxl += pLbl->getHeight() + 10;
  if (pInput->hasKeyboard())
  {
    // Edit box
    guiEditBox * pBox = guiEditBox::createDefaultEditBox(iNbLines, bMultiLines, iBoxWidth, L"DefaultEditBox", (KeyboardInputEngine*)pInput, pDisplay);
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
  wchar_t str[64] = L"";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"CANCEL", str, 64), L"CancelButton", pDisplay);
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);

  pBtn = guiButton::createDefaultNormalButton(i18n->getText1stUp(L"OK", str, 64), L"OkButton", pDisplay);
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
guiPopup * guiPopup::createTimedPopup(const wchar_t * sText, double fTimer, int iWidth, DisplayEngine * pDisplay)
{
  // Create empty popup (frame)
  guiPopup * pPopup = new guiPopup();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  pPopup->init(
    FP_Floating,
    FB_FitFrameToDocument,
    FB_FitFrameToDocument,
    0, 0, 0, 0, frmtex, L"TimedPopup", 0, 0, 1, 1, pDisplay);

  // Create popup inner document
  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(
    L"TimedPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pDisplay);

  // Top label
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"TopLabel", 0, 0, iWidth - 10, 0, pDisplay);
  pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, 10);
  pDoc->addComponent(pLbl);

  // Init popup
  pDoc->setDimensions(iWidth, pLbl->getHeight() + 20);
  pPopup->setDocument(pDoc);

  pPopup->m_fTimer = fTimer;
  return pPopup;
}
