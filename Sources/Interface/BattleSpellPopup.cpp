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
  pDoc->init("BattleSpellPopupDocument",
    pDisplay->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pDisplay);

  char sText[128] = "";
  char sBuf[128] = "";
  i18n->getText("(s)_CAN_CAST_BATTLE_SPELLS", sBuf, 128);
  snprintf(sText, 128, sBuf, m_pPlayer->getAvatarName());
  guiLabel * pText = new guiLabel();
  pText->init(sText, TEXT_FONT, TEXT_COLOR, "Text", 2, 2, 0, 0, pDisplay);
  pDoc->addComponent(pText);

  int yPos = pText->getYPos() + pText->getHeight() + 10;
  if (m_fTimer > 0)
  {
    i18n->getText("FINISH", sBuf, 128);
    snprintf(sText, 128, "%s (%d)", sBuf, (int)m_fTimer);
  }
  else
    i18n->getText("FINISH", sText, 128);

  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "NotNow", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(2, yPos);
  pDoc->addComponent(pBtn);

  int xPos = pBtn->getXPos() + pBtn->getWidth() + 10;
  i18n->getText("FINISH_FOR_BATTLE", sText, 128);
  pBtn = guiButton::createDefaultNormalButton(sText, "NotThisBattle", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(xPos, yPos);
  pDoc->addComponent(pBtn);

  xPos = pBtn->getXPos() + pBtn->getWidth() + 10;
  i18n->getText("STOP_TIMER", sText, 128);
  pBtn = guiButton::createDefaultNormalButton(sText, "StopTimer", pDisplay);
  pBtn->autoPadWidth(6, 64);
  pBtn->moveTo(xPos, yPos);
  pDoc->addComponent(pBtn);

  int iWidth = max(pBtn->getXPos() + pBtn->getWidth(), pText->getXPos() + pText->getWidth()) + 5;
  pDoc->setDimensions(iWidth, pBtn->getYPos() + pBtn->getHeight() + 5);

  int iTexs[8];
  iTexs[0] = pDisplay->getTextureEngine()->findTexture("interface:FrmTL");
  iTexs[1] = pDisplay->getTextureEngine()->findTexture("interface:FrmTC");
  iTexs[2] = pDisplay->getTextureEngine()->findTexture("interface:FrmTR");
  iTexs[3] = pDisplay->getTextureEngine()->findTexture("interface:FrmCL");
  iTexs[4] = pDisplay->getTextureEngine()->findTexture("interface:FrmCR");
  iTexs[5] = pDisplay->getTextureEngine()->findTexture("interface:FrmBL");
  iTexs[6] = pDisplay->getTextureEngine()->findTexture("interface:FrmBC");
  iTexs[7] = pDisplay->getTextureEngine()->findTexture("interface:FrmBR");
  init(FP_Floating, FB_FitFrameToDocument, FB_FitFrameToDocument, 0, 0, 0, 0, iTexs, "BattleSpellPopup", 0, 0, 1, 1, pDisplay);
  setDocument(pDoc);
}

// -----------------------------------------------------------------
// Name : ~BattleSpellPopup
//  Destructor
// -----------------------------------------------------------------
BattleSpellPopup::~BattleSpellPopup()
{
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
      char sBuf[64];
      char sText[64];
      i18n->getText("FINISH", sBuf, 64);
      snprintf(sText, 64, "%s (%d)", sBuf, (int)m_fTimer);
      ((guiButton*)getDocument()->getComponent("NotNow"))->setText(sText);
    }
  }
  guiComponent * pCpnt = getClickedComponent();
  if (pCpnt != NULL)
  {
    if (strcmp(pCpnt->getId(), "NotNow") == 0)
      m_iResponse = BATTLESPELL_BUTTON_FINISHED;
    else if (strcmp(pCpnt->getId(), "NotThisBattle") == 0)
      m_iResponse = BATTLESPELL_BUTTON_NEVERBATTLE;
    else if (strcmp(pCpnt->getId(), "StopTimer") == 0)
    {
      m_fTimer = -1;
      char sText[64];
      i18n->getText("FINISH", sText, 64);
      ((guiButton*)getDocument()->getComponent("NotNow"))->setText(sText);
    }
  }
}
