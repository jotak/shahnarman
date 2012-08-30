#include "BattleSpellPopup.h"
#include "../Data/LocalisationTool.h"
#include "../Gameboard/Unit.h"

// -----------------------------------------------------------------
// Name : BattleSpellPopup
//  Constructor
// -----------------------------------------------------------------
BattleSpellPopup::BattleSpellPopup(Player * pPlayer, int iTimer, DisplayEngine * pDisplay) : guiPopup()
{
  m_pPlayer = pPlayer;
  m_fTimer = (double) iTimer;
  m_iResponse = -1;

  guiDocument * pDoc = new guiPopupDocument();
  pDoc->init(L"BattleSpellPopupDocument",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pDisplay);

  wchar_t sText[128] = L"";
  wchar_t sBuf[128] = L"";
  i18n->getText(L"(s)_CAN_CAST_BATTLE_SPELLS", sBuf, 128);
  swprintf_s(sText, 128, sBuf, m_pPlayer->getAvatarName());
  guiLabel * pText = new guiLabel();
  pText->init(sText, TEXT_FONT, TEXT_COLOR, L"Text", 2, 2, 0, 0, pDisplay);
  pDoc->addComponent(pText);

  int yPos = pText->getYPos() + pText->getHeight() + 10;
  if (m_fTimer > 0)
  {
    i18n->getText(L"FINISH", sBuf, 128);
    swprintf_s(sText, 128, L"%s (%d)", sBuf, (int)m_fTimer);
  }
  else
    i18n->getText(L"FINISH", sText, 128);

  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, L"NotNow", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(2, yPos);
  pDoc->addComponent(pBtn);

  int xPos = pBtn->getXPos() + pBtn->getWidth() + 10;
  i18n->getText(L"FINISH_FOR_BATTLE", sText, 128);
  pBtn = guiButton::createDefaultNormalButton(sText, L"NotThisBattle", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(xPos, yPos);
  pDoc->addComponent(pBtn);

  xPos = pBtn->getXPos() + pBtn->getWidth() + 10;
  i18n->getText(L"STOP_TIMER", sText, 128);
  pBtn = guiButton::createDefaultNormalButton(sText, L"StopTimer", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(xPos, yPos);
  pDoc->addComponent(pBtn);

  int iWidth = max(pBtn->getXPos() + pBtn->getWidth(), pText->getXPos() + pText->getWidth()) + 5;
  pDoc->setDimensions(iWidth, pBtn->getYPos() + pBtn->getHeight() + 5);

  int iTexs[8];
  iTexs[0] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTL");
  iTexs[1] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTC");
  iTexs[2] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmTR");
  iTexs[3] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCL");
  iTexs[4] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmCR");
  iTexs[5] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBL");
  iTexs[6] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBC");
  iTexs[7] = pDisplay->getTextureEngine()->findTexture(L"interface:FrmBR");
  init(FP_Floating, FB_FitFrameToDocument, FB_FitFrameToDocument, 0, 0, 0, 0, iTexs, L"BattleSpellPopup", 0, 0, 1, 1, pDisplay);
  setDocument(pDoc);
}

// -----------------------------------------------------------------
// Name : ~BattleSpellPopup
//  Destructor
// -----------------------------------------------------------------
BattleSpellPopup::~BattleSpellPopup()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy BattleSpellPopup\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy BattleSpellPopup\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void BattleSpellPopup::update(double delta)
{
  guiFrame::update(delta);
  if (!isEnabled())
    return;
  if (m_fTimer > 0)
  {
    m_fTimer -= delta;
    if (m_fTimer <= 0)
      m_iResponse = BATTLESPELL_BUTTON_FINISHED;
    else
    {
      wchar_t sBuf[64];
      wchar_t sText[64];
      i18n->getText(L"FINISH", sBuf, 64);
      swprintf_s(sText, 64, L"%s (%d)", sBuf, (int)m_fTimer);
      ((guiButton*)getDocument()->getComponent(L"NotNow"))->setText(sText);
    }
  }
  guiComponent * pCpnt = getClickedComponent();
  if (pCpnt != NULL)
  {
    if (wcscmp(pCpnt->getId(), L"NotNow") == 0)
      m_iResponse = BATTLESPELL_BUTTON_FINISHED;
    else if (wcscmp(pCpnt->getId(), L"NotThisBattle") == 0)
      m_iResponse = BATTLESPELL_BUTTON_NEVERBATTLE;
    else if (wcscmp(pCpnt->getId(), L"StopTimer") == 0)
    {
      m_fTimer = -1;
      wchar_t sText[64];
      i18n->getText(L"FINISH", sText, 64);
      ((guiButton*)getDocument()->getComponent(L"NotNow"))->setText(sText);
    }
  }
}
