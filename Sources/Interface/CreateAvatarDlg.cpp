#include "CreateAvatarDlg.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "SelectPlayerAvatarDlg.h"
#include "../GUIClasses/guiContainer.h"
#include "../GUIClasses/guiButton.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiPopup.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Profile.h"
#include "../DeckData/ShahmahCreation.h"
#include "../DeckData/Ethnicity.h"
#include "../DeckData/AvatarData.h"
#include "../Gameboard/Skill.h"

#define SPACING           4
#define WIDE_Y_SPACING    18
#define WIDE_X_SPACING    50

#define MELEE_COST        4
#define RANGE_COST        6
#define ARMOR_COST        7
#define ENDURANCE_COST    2
#define SPEED_COST        4
#define GOLD_COST         1
#define INITIAL_POINTS    60

// -----------------------------------------------------------------
// Name : CreateAvatarDlg
//  Constructor
// -----------------------------------------------------------------
CreateAvatarDlg::CreateAvatarDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pSkillsPanel = NULL;
  m_iPointsLeft = INITIAL_POINTS;
  m_iGoldPaid = 0;
  m_iCarac[MELEE] = m_iCarac[RANGE] = m_iCarac[ARMOR] = 0;
  m_iCarac[ENDURANCE] = m_iCarac[SPEED] = 1;
  m_iImage = 0;
  m_pCurrentEdition = NULL;
  m_pCurrentPeople = NULL;
  m_pRemainPointsPopup = NULL;
  m_pEditionRelativeCpnt = new ObjectList(false);
  m_pAllSkillData = new ObjectList(true);
  m_pImagesSubList = new ObjectList(false);
  init(L"",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  // Back button
  wchar_t sText[LABEL_MAX_CHARS] = L"";
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"BACK", sText, LABEL_MAX_CHARS), L"BackButton", m_pLocalClient->getDisplay());
  int yPxlBottom = iHeight - SPACING - pBtn->getHeight();
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxlBottom);
  addComponent(pBtn);

  // OK button
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"OK", sText, LABEL_MAX_CHARS), L"OkButton", m_pLocalClient->getDisplay());
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxlBottom);
  pBtn->setEnabled(false);
  addComponent(pBtn);

  // Edition label
  int yPxl = SPACING;
  guiLabel * pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"EDITION", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);

  // Edition combo
  guiComboBox * pBox = guiComboBox::createDefaultComboBox(L"EditionCombo", m_pLocalClient->getInterface(), m_pLocalClient->getDisplay());
  pBox->moveTo(iWidth / 2 - pBox->getWidth() / 2, yPxl);
  addComponent(pBox);
  pLbl->moveTo(iWidth / 2 - (pLbl->getWidth() + pBox->getWidth() + SPACING) / 2, yPxl + 5);
  pBox->moveTo(pLbl->getXPos() + pLbl->getWidth() + 3*SPACING, yPxl);

  // Points label
  wchar_t sBuf[32];
  yPxl += pBox->getHeight() + WIDE_Y_SPACING;
  i18n->getText(L"POINTS_ABREV", sBuf, 32);
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d %s", m_iPointsLeft, sBuf);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, L"PointsLabel", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(iWidth / 3 - pLbl->getWidth() / 2, yPxl);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);

  // Pay gold label
  pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"PAY_GOLD", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->setTooltipText(i18n->getText(L"ALLOWS_TO_INCREASE_AVAILABLE_POINTS", sText, LABEL_MAX_CHARS));
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  guiLabel * pLblGld = pLbl;

  // Gold paid label
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iGoldPaid);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"GoldValue", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  pLblGld->moveTo(2 * iWidth / 3 - (pLblGld->getWidth() + WIDE_X_SPACING + pLbl->getWidth()) / 2, yPxl);
  pLbl->moveTo(pLblGld->getXPos() + pLblGld->getWidth() + WIDE_X_SPACING, yPxl);
  createLeftRightArrows(pLbl);

  // Ethnicity label
  yPxl += pLbl->getHeight() + WIDE_Y_SPACING + 25;
  pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"ETHNICITY", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);

  // Ethnicity combo
  pBox = guiComboBox::createDefaultComboBox(L"EthnicityCombo", m_pLocalClient->getInterface(), m_pLocalClient->getDisplay());
  addComponent(pBox);
  m_pEditionRelativeCpnt->addLast(pBox);
  pLbl->moveTo(iWidth / 4 - (pLbl->getWidth() + pBox->getWidth() + SPACING) / 2, yPxl + 5);
  pBox->moveTo(pLbl->getXPos() + pLbl->getWidth() + 3*SPACING, yPxl);

  // Shahmah image
  yPxl += pBox->getHeight() / 2 - 50;
  int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(L"EmptyWhiteSquare");
  guiImage * pImg = new guiImage();
  pImg->init(iTex, L"ShahmahImage", 3 * iWidth / 4 - 50, yPxl, 100, 100, m_pLocalClient->getDisplay());
  addComponent(pImg);
  m_pEditionRelativeCpnt->addLast(pImg);
  createLeftRightArrows(pImg);

  // Characteristics
  yPxl += pImg->getHeight() + WIDE_Y_SPACING;
  int yTop = yPxl;
  // Melee label
  pLbl = new guiLabel();
  pLbl->init(i18n->getText1stUp(L"melee", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(WIDE_X_SPACING, yPxl);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  int xMax = pLbl->getWidth();

  // Range label
  yPxl += pLbl->getHeight() + 2*SPACING;
  int y2 = yPxl;
  pLbl = new guiLabel();
  pLbl->init(i18n->getText1stUp(L"range", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(WIDE_X_SPACING, yPxl);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  xMax = max(xMax, pLbl->getWidth());

  // Armor label
  yPxl += pLbl->getHeight() + 2*SPACING;
  int y3 = yPxl;
  pLbl = new guiLabel();
  pLbl->init(i18n->getText1stUp(L"armor", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(WIDE_X_SPACING, yPxl);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  xMax = max(xMax, pLbl->getWidth());
  int y4 = yPxl + pLbl->getHeight();

  // Melee value label
  yPxl = yTop;
  int xPxl = pLbl->getXPos() + xMax + WIDE_X_SPACING;
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iCarac[MELEE]);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"MeleeValue", xPxl, yPxl, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  createLeftRightArrows(pLbl);

  // Range value label
  yPxl = y2;
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iCarac[RANGE]);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"RangeValue", xPxl, yPxl, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  createLeftRightArrows(pLbl);

  // Armor value label
  yPxl = y3;
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iCarac[ARMOR]);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"ArmorValue", xPxl, yPxl, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  createLeftRightArrows(pLbl);

  // Endurance label
  pLbl = new guiLabel();
  pLbl->init(i18n->getText1stUp(L"endurance", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(iWidth / 2, yTop);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  xMax = pLbl->getWidth();

  // Speed label
  pLbl = new guiLabel();
  pLbl->init(i18n->getText1stUp(L"speed", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(iWidth / 2, y2);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  xMax = max(xMax, pLbl->getWidth());

  // Endurance value label
  xPxl = pLbl->getXPos() + xMax + WIDE_X_SPACING;
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iCarac[ENDURANCE]);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"EnduranceValue", xPxl, yTop, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  createLeftRightArrows(pLbl);

  // Speed value label
  pLbl = new guiLabel();
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d", m_iCarac[SPEED]);
  pLbl->init(sText, H2_FONT, H2_COLOR, L"SpeedValue", xPxl, y2, 0, 0, m_pLocalClient->getDisplay());
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);
  createLeftRightArrows(pLbl);

  // Skills label
  yPxl = y4 + WIDE_Y_SPACING;
  pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"SKILLS", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(iWidth / 2 - pLbl->getWidth() / 2, yPxl);
  addComponent(pLbl);
  m_pEditionRelativeCpnt->addLast(pLbl);

  // Skills panel
  yPxl += pLbl->getHeight() + SPACING;
  yPxlBottom -= WIDE_Y_SPACING;
  m_pSkillsPanel = guiContainer::createDefaultPanel(iWidth - 2 * SPACING, yPxlBottom - yPxl, L"SkillsPanel", m_pLocalClient->getDisplay());
  m_pSkillsPanel->moveTo(SPACING, yPxl);
  m_pSkillsPanel->setHeightFitBehavior(FB_FitFrameToDocumentWhenSmaller);
  addComponent(m_pSkillsPanel);
  m_pEditionRelativeCpnt->addLast(m_pSkillsPanel);
}

// -----------------------------------------------------------------
// Name : ~CreateAvatarDlg
//  Destructor
// -----------------------------------------------------------------
CreateAvatarDlg::~CreateAvatarDlg()
{
  delete m_pEditionRelativeCpnt;
  delete m_pAllSkillData;
  delete m_pImagesSubList;
}

// -----------------------------------------------------------------
// Name : createLeftRightArrows
// -----------------------------------------------------------------
void CreateAvatarDlg::createLeftRightArrows(guiComponent * pCpnt, guiDocument * pDoc)
{
  if (!pDoc)
    pDoc = this;
  wchar_t sId[128] = L"";
  swprintf_s(sId, 128, L"%s_left", pCpnt->getId());
  int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:LeftArrow");
  guiButton * pBtn = guiButton::createDefaultImageButton(iTex, sId, m_pLocalClient->getDisplay());
  pBtn->setMultiClicks(true);
  pBtn->moveTo(pCpnt->getXPos() - 2*SPACING - pBtn->getWidth(), pCpnt->getYPos() + pCpnt->getHeight() / 2 - pBtn->getHeight() / 2);
  pDoc->addComponent(pBtn);
  pBtn->setAttachment(pCpnt);
  m_pEditionRelativeCpnt->addLast(pBtn);
  if (pDoc != this)
    pBtn->setOwner(this);

  swprintf_s(sId, 128, L"%s_right", pCpnt->getId());
  iTex = m_pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:RightArrow");
  pBtn = guiButton::createDefaultImageButton(iTex, sId, m_pLocalClient->getDisplay());
  pBtn->setMultiClicks(true);
  pBtn->moveTo(pCpnt->getXPos() + pCpnt->getWidth() + 2*SPACING, pCpnt->getYPos() + pCpnt->getHeight() / 2 - pBtn->getHeight() / 2);
  pDoc->addComponent(pBtn);
  pBtn->setAttachment(pCpnt);
  m_pEditionRelativeCpnt->addLast(pBtn);
  if (pDoc != this)
    pBtn->setOwner(this);
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void CreateAvatarDlg::update(double delta)
{
  guiDocument::update(delta);
  if (m_pRemainPointsPopup != NULL)
  {
    guiComponent * pCpnt = m_pRemainPointsPopup->getClickedComponent();
    if (pCpnt != NULL)
    {
      if (wcscmp(pCpnt->getId(), L"YesButton") == 0)
      {
        // Remove frame
        m_pLocalClient->getInterface()->deleteFrame(m_pRemainPointsPopup);
        m_pRemainPointsPopup = NULL;
        setEnabled(true);
        saveAndQuit();
      }
      else if (wcscmp(pCpnt->getId(), L"NoButton") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pRemainPointsPopup);
        m_pRemainPointsPopup = NULL;
        setEnabled(true);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool CreateAvatarDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (wcscmp(pCpnt->getId(), L"OkButton") == 0)
  {
    // Show popup when click OK and remains unused points
    if (m_iPointsLeft > 0)
    {
      setEnabled(false);
      void * p = &m_iPointsLeft;
      wchar_t str[LABEL_MAX_CHARS];
      m_pRemainPointsPopup = guiPopup::createYesNoPopup(i18n->getText(L"REMAINS_%$1d_UNUSED_POINTS_WISH_CONTINUE", str, LABEL_MAX_CHARS, &p), getDisplay());
      m_pLocalClient->getInterface()->registerFrame(m_pRemainPointsPopup);
      m_pRemainPointsPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pRemainPointsPopup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pRemainPointsPopup->getHeight()) / 2);
    }
    else
      saveAndQuit();
  }
  else if (wcscmp(pCpnt->getId(), L"BackButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
    return false;
  }
  else if (wcscmp(pCpnt->getId(), L"EditionButton") == 0)
  {
    onEditionChanged((Edition*) pCpnt->getAttachment());
  }
  else if (wcscmp(pCpnt->getId(), L"EthnicityButton") == 0)
  {
    onEthnicityChanged((Ethnicity*) pCpnt->getAttachment());
  }
  else if (wcscmp(pCpnt->getId(), L"ShahmahImage_left") == 0
        || wcscmp(pCpnt->getId(), L"ShahmahImage_right") == 0
        || wcscmp(pCpnt->getId(), L"GoldValue_left") == 0
        || wcscmp(pCpnt->getId(), L"GoldValue_right") == 0
        || wcscmp(pCpnt->getId(), L"MeleeValue_left") == 0
        || wcscmp(pCpnt->getId(), L"MeleeValue_right") == 0
        || wcscmp(pCpnt->getId(), L"RangeValue_left") == 0
        || wcscmp(pCpnt->getId(), L"RangeValue_right") == 0
        || wcscmp(pCpnt->getId(), L"ArmorValue_left") == 0
        || wcscmp(pCpnt->getId(), L"ArmorValue_right") == 0
        || wcscmp(pCpnt->getId(), L"EnduranceValue_left") == 0
        || wcscmp(pCpnt->getId(), L"EnduranceValue_right") == 0
        || wcscmp(pCpnt->getId(), L"SpeedValue_left") == 0
        || wcscmp(pCpnt->getId(), L"SpeedValue_right") == 0
        || wcscmp(pCpnt->getId(), L"SkillSelection_left") == 0
        || wcscmp(pCpnt->getId(), L"SkillSelection_right") == 0)
  {
    onArrowClick(pCpnt);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onArrowClick
// -----------------------------------------------------------------
void CreateAvatarDlg::onArrowClick(guiComponent * pArrow)
{
  assert(m_pCurrentEdition != NULL);
  guiComponent * pCpnt = (guiComponent*) pArrow->getAttachment();
  if (wcscmp(pCpnt->getId(), L"ShahmahImage") == 0)
  {
    if (wcscmp(pArrow->getId(), L"ShahmahImage_left") == 0)
    {
      assert(m_iImage > 0);
      m_iImage--;
    }
    else
    {
      m_iImage++;
      assert(m_iImage < m_pImagesSubList->size);
    }
    StringObject * pString = (StringObject*) m_pImagesSubList->goTo(0, m_iImage);
    assert(pString != NULL);
    wchar_t sPath[MAX_PATH];
    swprintf_s(sPath, MAX_PATH, L"%s/%s", m_pCurrentEdition->m_sObjectId, pString->m_sString);
    int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(sPath);
    ((guiImage*)pCpnt)->setImageTexture(iTex);
    checkArrows(pCpnt);
    guiComponent * pCpnt2 = getComponent(L"ShahmahImage_left");
    assert(pCpnt2 != NULL);
    pCpnt2->setEnabled(m_iImage > 0);
    pCpnt2 = getComponent(L"ShahmahImage_right");
    assert(pCpnt2 != NULL);
    pCpnt2->setEnabled(m_iImage < m_pImagesSubList->size - 1);
  }
  else if (wcscmp(pCpnt->getId(), L"GoldValue") == 0)
  {
    Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
    assert(pPlayer != NULL);
    int maxGold = min(10, pPlayer->getCash());
    if (wcscmp(pArrow->getId(), L"GoldValue_left") == 0)
    {
      assert(m_iGoldPaid > 0);
      m_iGoldPaid--;
    }
    else
    {
      m_iGoldPaid++;
      assert(m_iGoldPaid <= maxGold);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iGoldPaid);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"MeleeValue") == 0)
  {
    if (wcscmp(pArrow->getId(), L"MeleeValue_left") == 0)
    {
      assert(m_iCarac[MELEE] > 0);
      m_iCarac[MELEE]--;
    }
    else
    {
      m_iCarac[MELEE]++;
      assert(m_iCarac[MELEE] <= 100);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iCarac[MELEE]);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"RangeValue") == 0)
  {
    if (wcscmp(pArrow->getId(), L"RangeValue_left") == 0)
    {
      assert(m_iCarac[RANGE] > 0);
      m_iCarac[RANGE]--;
    }
    else
    {
      m_iCarac[RANGE]++;
      assert(m_iCarac[RANGE] <= 100);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iCarac[RANGE]);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"ArmorValue") == 0)
  {
    if (wcscmp(pArrow->getId(), L"ArmorValue_left") == 0)
    {
      assert(m_iCarac[ARMOR] > 0);
      m_iCarac[ARMOR]--;
    }
    else
    {
      m_iCarac[ARMOR]++;
      assert(m_iCarac[ARMOR] <= 100);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iCarac[ARMOR]);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"EnduranceValue") == 0)
  {
    if (wcscmp(pArrow->getId(), L"EnduranceValue_left") == 0)
    {
      assert(m_iCarac[ENDURANCE] > 1);
      m_iCarac[ENDURANCE]--;
    }
    else
    {
      m_iCarac[ENDURANCE]++;
      assert(m_iCarac[ENDURANCE] <= 100);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iCarac[ENDURANCE]);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"SpeedValue") == 0)
  {
    if (wcscmp(pArrow->getId(), L"SpeedValue_left") == 0)
    {
      assert(m_iCarac[SPEED] > 1);
      m_iCarac[SPEED]--;
    }
    else
    {
      m_iCarac[SPEED]++;
      assert(m_iCarac[SPEED] <= 100);
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", m_iCarac[SPEED]);
    ((guiLabel*)pCpnt)->setText(sValue);
    checkArrows(pCpnt);
  }
  else if (wcscmp(pCpnt->getId(), L"SkillSelection") == 0)
  {
    SkillData * pData = (SkillData*) pCpnt->getAttachment();
    if (wcscmp(pArrow->getId(), L"SkillSelection_left") == 0)
    {
      assert(pData->m_iNbSelected > 0);
      pData->m_iNbSelected--;
    }
    else
    {
      pData->m_iNbSelected++;
    }
    computePoints();
    wchar_t sValue[8];
    swprintf_s(sValue, 8, L"%d", pData->m_iNbSelected);
    ((guiLabel*)pCpnt)->setText(sValue);
  }
}

// -----------------------------------------------------------------
// Name : onEthnicityChanged
// -----------------------------------------------------------------
void CreateAvatarDlg::onEthnicityChanged(Ethnicity * pEthn)
{
  m_pCurrentPeople = pEthn;

  // Get the "ShahmahCreation" object relative to this edition
  ShahmahCreation * pCreat = m_pCurrentEdition->getShahmahCreationData();
  assert(pCreat != NULL);

  // Construct m_pImagesSubList
  m_pImagesSubList->deleteAll();
  StringObject * pString = (StringObject*) pCreat->m_pImages->getFirst(0);
  while (pString != NULL)
  {
    if (pString->getAttachment() == m_pCurrentPeople)
      m_pImagesSubList->addLast(pString);
    pString = (StringObject*) pCreat->m_pImages->getNext(0);
  }

  // Get allowed images list and set automatically first image
  m_iImage = 0;
  guiImage * pImg = (guiImage*) getComponent(L"ShahmahImage");
  assert(pImg != NULL);
  pString = (StringObject*) m_pImagesSubList->getFirst(0);
  if (pString != NULL)
  {
    wchar_t sPath[MAX_PATH];
    swprintf_s(sPath, MAX_PATH, L"%s/%s", m_pCurrentEdition->m_sObjectId, pString->m_sString);
    int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(sPath);
    pImg->setImageTexture(iTex);
    pImg->setVisible(true);
  }
  else
  {
    int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(L"EmptyWhiteSquare");
    pImg->setImageTexture(iTex);
    pImg->setVisible(false);
  }
  checkArrows(pImg);
  guiComponent * pCpnt2 = getComponent(L"ShahmahImage_left");
  assert(pCpnt2 != NULL);
  pCpnt2->setEnabled(m_iImage > 0);
  pCpnt2 = getComponent(L"ShahmahImage_right");
  assert(pCpnt2 != NULL);
  pCpnt2->setEnabled(m_iImage < m_pImagesSubList->size - 1);

  computePoints();
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void CreateAvatarDlg::onShow()
{
  // Fill combo
  wchar_t sText[LABEL_MAX_CHARS] = L"";
  guiComboBox * pBox = (guiComboBox*) getComponent(L"EditionCombo");
  assert(pBox != NULL);
  pBox->clearList();
  Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
  while (pEdition != NULL)
  {
    ShahmahCreation * pCreat = pEdition->getShahmahCreationData();
    if (pCreat != NULL)
    {
      pEdition->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
      guiButton * pBtn = pBox->addString(sText, L"EditionButton");
      pBtn->setAttachment(pEdition);
    }
    pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
  }
  onEditionChanged(NULL);
}

// -----------------------------------------------------------------
// Name : onEditionChanged
// -----------------------------------------------------------------
void CreateAvatarDlg::onEditionChanged(Edition * pEdition)
{
  if (m_pCurrentEdition == pEdition && pEdition != NULL)
    return;

  m_pCurrentEdition = pEdition;
  // If edition is null, then hide (almost) everything
  if (pEdition == NULL)
  {
    guiComponent * pCpnt = (guiComponent*) m_pEditionRelativeCpnt->getFirst(0);
    while (pCpnt != NULL)
    {
      pCpnt->setVisible(false);
      pCpnt = (guiComponent*) m_pEditionRelativeCpnt->getNext(0);
    }
    return;
  }
  // Edition is not null: show everything
  guiComponent * pCpnt = (guiComponent*) m_pEditionRelativeCpnt->getFirst(0);
  while (pCpnt != NULL)
  {
    pCpnt->setVisible(true);
    pCpnt = (guiComponent*) m_pEditionRelativeCpnt->getNext(0);
  }

  // Get the "ShahmahCreation" object relative to this edition
  ShahmahCreation * pCreat = pEdition->getShahmahCreationData();
  assert(pCreat != NULL);

  // Reset paid gold
  m_iGoldPaid = 0;
  guiLabel * pLbl = (guiLabel*) getComponent(L"GoldValue");
  assert(pLbl != NULL);
  wchar_t sValue[8];
  swprintf_s(sValue, 8, L"%d", m_iGoldPaid);
  pLbl->setText(sValue);
  checkArrows(pLbl);
  
  // Reset caracs
  m_iCarac[MELEE] = m_iCarac[RANGE] = m_iCarac[ARMOR] = 0;
  m_iCarac[ENDURANCE] = m_iCarac[SPEED] = 1;
  pLbl = (guiLabel*) getComponent(L"MeleeValue");
  assert(pLbl != NULL);
  swprintf_s(sValue, 8, L"%d", m_iCarac[MELEE]);
  pLbl->setText(sValue);
  checkArrows(pLbl);
  pLbl = (guiLabel*) getComponent(L"RangeValue");
  assert(pLbl != NULL);
  swprintf_s(sValue, 8, L"%d", m_iCarac[RANGE]);
  pLbl->setText(sValue);
  checkArrows(pLbl);
  pLbl = (guiLabel*) getComponent(L"ArmorValue");
  assert(pLbl != NULL);
  swprintf_s(sValue, 8, L"%d", m_iCarac[ARMOR]);
  pLbl->setText(sValue);
  checkArrows(pLbl);
  pLbl = (guiLabel*) getComponent(L"EnduranceValue");
  assert(pLbl != NULL);
  swprintf_s(sValue, 8, L"%d", m_iCarac[ENDURANCE]);
  pLbl->setText(sValue);
  checkArrows(pLbl);
  pLbl = (guiLabel*) getComponent(L"SpeedValue");
  assert(pLbl != NULL);
  swprintf_s(sValue, 8, L"%d", m_iCarac[SPEED]);
  pLbl->setText(sValue);
  checkArrows(pLbl);

  // Ethnicities
  m_pCurrentPeople = NULL;
  wchar_t sText[LABEL_MAX_CHARS];
  wchar_t sBuf1[LABEL_MAX_CHARS];
  wchar_t sBuf2[32];
  wchar_t s2P[8];
  i18n->getText(L"COST", sBuf2, 32);
  i18n->getText(L"2P", s2P, 8);
  guiComboBox * pBox = (guiComboBox*) getComponent(L"EthnicityCombo");
  assert(pBox != NULL);
  pBox->clearList();
  Ethnicity * pEthn = (Ethnicity*) pCreat->m_pPeoples->getFirst(0);
  while (pEthn != NULL)
  {
    pEthn->findLocalizedElement(sBuf1, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
    int cost = pCreat->m_pPeoples->getCurrentType(0);
    swprintf_s(sText, LABEL_MAX_CHARS, L"%s (%s%s%d)", sBuf1, sBuf2, s2P, cost);
    guiButton * pBtn = pBox->addString(sText, L"EthnicityButton");
    pBtn->setAttachment(pEthn);
    pEthn = (Ethnicity*) pCreat->m_pPeoples->getNext(0);
  }

  // Images
  m_pImagesSubList->deleteAll();
  m_iImage = 0;
  guiImage * pImg = (guiImage*) getComponent(L"ShahmahImage");
  assert(pImg != NULL);
  int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(L"EmptyWhiteSquare");
  pImg->setImageTexture(iTex);
  pImg->setVisible(false);
  checkArrows(pImg);
  guiComponent * pCpnt2 = getComponent(L"ShahmahImage_left");
  assert(pCpnt2 != NULL);
  pCpnt2->setEnabled(false);
  pCpnt2 = getComponent(L"ShahmahImage_right");
  assert(pCpnt2 != NULL);
  pCpnt2->setEnabled(false);

  // Skills
  m_pAllSkillData->deleteAll();
  int x[3] = { SPACING, SPACING + m_pSkillsPanel->getWidth() / 3, SPACING + 2 * m_pSkillsPanel->getWidth() / 3 };
  int iX = 0;
  int yPxl = SPACING;
  int imgSize = 48;
  Skill * pSkill = (Skill*) pCreat->m_pSkills->getFirst(0);
  while (pSkill != NULL)
  {
    // Skill picture
    swprintf_s(sText, LABEL_MAX_CHARS, L"%s - %s", pSkill->getLocalizedName(), pSkill->getLocalizedDescription());
    guiImage * pImg = new guiImage();
    pImg->init(getDisplay()->getTextureEngine()->loadTexture(pSkill->getIconPath()), L"SkillImage", x[iX], yPxl, imgSize, imgSize, getDisplay());
    pImg->setTooltipText(sText);
    m_pSkillsPanel->getDocument()->addComponent(pImg);
    // Cost label
    int cost = pCreat->m_pSkills->getCurrentType(0);
    swprintf_s(sText, LABEL_MAX_CHARS, L"%s%s%d", sBuf2, s2P, cost);
    pLbl = new guiLabel();
    pLbl->init(sText, H2_FONT, H2_COLOR, L"", x[iX] + imgSize + SPACING, yPxl, 0, 0, getDisplay());
    m_pSkillsPanel->getDocument()->addComponent(pLbl);
    // Select skill label
    SkillData * pData = new SkillData(pSkill, cost, 0);
    pLbl = new guiLabel();
    pLbl->init(L"0", H2_FONT, H2_COLOR, L"SkillSelection", x[iX] + imgSize + 25, yPxl + imgSize / 2, 0, 0, getDisplay());
    pLbl->setAttachment(pData);
    pData->setAttachment(pLbl);
    m_pSkillsPanel->getDocument()->addComponent(pLbl);
    createLeftRightArrows(pLbl, m_pSkillsPanel->getDocument());
    m_pAllSkillData->addLast(pData);

    iX++;
    if (iX == 3)
    {
      iX = 0;
      yPxl += pImg->getHeight() + WIDE_Y_SPACING;
    }
    pSkill = (Skill*) pCreat->m_pSkills->getNext(0);
  }

  computePoints();
}

// -----------------------------------------------------------------
// Name : checkArrows
// -----------------------------------------------------------------
void CreateAvatarDlg::checkArrows(guiComponent * pCpnt, guiDocument * pDoc)
{
  if (pDoc == NULL)
    pDoc = this;
  // First check that position is still ok
  wchar_t sId[128] = L"";
  swprintf_s(sId, 128, L"%s_left", pCpnt->getId());
  guiButton * pBtn = (guiButton*) pDoc->getComponent(sId);
  assert(pBtn != NULL);
  pBtn->moveTo(pCpnt->getXPos() - SPACING - pBtn->getWidth(), pCpnt->getYPos() + pCpnt->getHeight() / 2 - pBtn->getHeight() / 2);
  guiButton * pLeft = pBtn;

  swprintf_s(sId, 128, L"%s_right", pCpnt->getId());
  pBtn = (guiButton*) pDoc->getComponent(sId);
  assert(pBtn != NULL);
  pBtn->moveTo(pCpnt->getXPos() + pCpnt->getWidth() + SPACING, pCpnt->getYPos() + pCpnt->getHeight() / 2 - pBtn->getHeight() / 2);
}

// -----------------------------------------------------------------
// Name : computePoints
// -----------------------------------------------------------------
void CreateAvatarDlg::computePoints()
{
  m_iPointsLeft = INITIAL_POINTS
    + m_iGoldPaid * GOLD_COST
    - m_iCarac[MELEE] * MELEE_COST
    - m_iCarac[RANGE] * RANGE_COST
    - m_iCarac[ARMOR] * ARMOR_COST
    - (m_iCarac[ENDURANCE]-1) * ENDURANCE_COST
    - (m_iCarac[SPEED]-1) * SPEED_COST;
  if (m_pCurrentEdition != NULL && m_pCurrentPeople != NULL)
  {
    ShahmahCreation * pCreat = m_pCurrentEdition->getShahmahCreationData();
    Ethnicity * pEthn = (Ethnicity*) pCreat->m_pPeoples->getFirst(0);
    while (pEthn != NULL)
    {
      if (pEthn == m_pCurrentPeople)
      {
        m_iPointsLeft -= pCreat->m_pPeoples->getCurrentType(0);
        break;
      }
      pEthn = (Ethnicity*) pCreat->m_pPeoples->getNext(0);
    }
  }

  // Skills
  SkillData * pData = (SkillData*) m_pAllSkillData->getFirst(0);
  while (pData != NULL)
  {
    m_iPointsLeft -= pData->m_iCost * pData->m_iNbSelected;
    pData = (SkillData*) m_pAllSkillData->getNext(0);
  }

  // Change points display
  guiLabel * pLbl = (guiLabel*) getComponent(L"PointsLabel");
  assert(pLbl != NULL);
  wchar_t sText[LABEL_MAX_CHARS];
  wchar_t sBuf[32];
  i18n->getText(L"POINTS_ABREV", sBuf, 32);
  swprintf_s(sText, LABEL_MAX_CHARS, L"%d %s", m_iPointsLeft, sBuf);
  pLbl->setText(sText);

  // Check arrows
  // Gold
  guiComponent * pCpnt = getComponent(L"GoldValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= GOLD_COST && m_iGoldPaid > 0);
  pCpnt = getComponent(L"GoldValue_right");
  assert(pCpnt != NULL);
  Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
  assert(pPlayer != NULL);
  pCpnt->setEnabled(m_iGoldPaid < min(10, pPlayer->getCash()));

  // Melee
  pCpnt = getComponent(L"MeleeValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iCarac[MELEE] > 0);
  pCpnt = getComponent(L"MeleeValue_right");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= MELEE_COST);
  // Range
  pCpnt = getComponent(L"RangeValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iCarac[RANGE] > 0);
  pCpnt = getComponent(L"RangeValue_right");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= RANGE_COST);
  // Armor
  pCpnt = getComponent(L"ArmorValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iCarac[ARMOR] > 0);
  pCpnt = getComponent(L"ArmorValue_right");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= ARMOR_COST);
  // Endurance
  pCpnt = getComponent(L"EnduranceValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iCarac[ENDURANCE] > 1);
  pCpnt = getComponent(L"EnduranceValue_right");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= ENDURANCE_COST);
  // Speed
  pCpnt = getComponent(L"SpeedValue_left");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iCarac[SPEED] > 1);
  pCpnt = getComponent(L"SpeedValue_right");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(m_iPointsLeft >= SPEED_COST);

  // Check ethnicity
  ShahmahCreation * pCreat = m_pCurrentEdition->getShahmahCreationData();
  guiComboBox * pBox = (guiComboBox*) getComponent(L"EthnicityCombo");
  assert(pBox != NULL);
  u16 nbItems = pBox->getItemsCount();
  for (u16 i = 0; i < nbItems; i++)
  {
    guiButton * pBtn = pBox->getItem(i);
    assert(pBtn != NULL);
    Ethnicity * pEthn2 = (Ethnicity*) pBtn->getAttachment();
    assert(pEthn2 != NULL);
    Ethnicity * pEthn = (Ethnicity*) pCreat->m_pPeoples->getFirst(0);
    while (pEthn != NULL)
    {
      if (pEthn == pEthn2)
      {
        pBtn->setEnabled(m_iPointsLeft >= pCreat->m_pPeoples->getCurrentType(0));
        break;
      }
      pEthn = (Ethnicity*) pCreat->m_pPeoples->getNext(0);
    }
  }

  // Check skills
  // Here we need to loop through right and left buttons
  pCpnt = m_pSkillsPanel->getDocument()->getFirstComponent();
  while (pCpnt != NULL)
  {
    if (wcscmp(pCpnt->getId(), L"SkillSelection_left") == 0)
    {
      // First attachment is the guiLabel linked to this button; second attachment is the corresponding SkillData structure
      SkillData * pData = (SkillData*) pCpnt->getAttachment()->getAttachment();
      pCpnt->setEnabled(pData->m_iNbSelected > 0);
    }
    else if (wcscmp(pCpnt->getId(), L"SkillSelection_right") == 0)
    {
      // First attachment is the guiLabel linked to this button; second attachment is the corresponding SkillData structure
      SkillData * pData = (SkillData*) pCpnt->getAttachment()->getAttachment();
      pCpnt->setEnabled(m_iPointsLeft >= pData->m_iCost && (pData->m_iNbSelected == 0 || pData->m_pSkill->isCumulative() || pData->m_pSkill->isMergeable()));
    }
    pCpnt = m_pSkillsPanel->getDocument()->getNextComponent();
  }

  // Enable or disable OK button (form validation)
  // Ethnicity is the only mandatory data
  guiComponent * pBtn = getComponent(L"OkButton");
  assert(pBtn != NULL);
  pBtn->setEnabled(m_pCurrentPeople != NULL);
}

// -----------------------------------------------------------------
// Name : saveAndQuit
// -----------------------------------------------------------------
void CreateAvatarDlg::saveAndQuit()
{
  // Create Avatar
  AvatarData * pAvatar = new AvatarData();
  wsafecpy(pAvatar->m_sObjectId, NAME_MAX_CHARS, L"");
  wsafecpy(pAvatar->m_sEdition, NAME_MAX_CHARS, m_pCurrentEdition->m_sObjectId);
  wsafecpy(pAvatar->m_sEthnicityId, NAME_MAX_CHARS, m_pCurrentPeople->m_sObjectId);
  StringObject * pStr = (StringObject*) m_pImagesSubList->goTo(0, m_iImage);
  swprintf_s(pAvatar->m_sTextureFilename, MAX_PATH, L"%s/%s",  m_pCurrentEdition->m_sObjectId, pStr->m_sString);
  pAvatar->m_lValues.insert(long_hash_pair(STRING_MELEE, m_iCarac[MELEE]));
  pAvatar->m_lValues.insert(long_hash_pair(STRING_RANGE, m_iCarac[RANGE]));
  pAvatar->m_lValues.insert(long_hash_pair(STRING_ARMOR, m_iCarac[ARMOR]));
  pAvatar->m_lValues.insert(long_hash_pair(STRING_ENDURANCE, m_iCarac[ENDURANCE]));
  pAvatar->m_lValues.insert(long_hash_pair(STRING_SPEED, m_iCarac[SPEED]));
  // Skills
  SkillData * pData = (SkillData*) m_pAllSkillData->getFirst(0);
  while (pData != NULL)
  {
    if (pData->m_iNbSelected > 0)
    {
      Skill * pSkill = pData->m_pSkill->clone(false, m_pLocalClient->getDebug());
      pAvatar->m_pSkills->addLast(pSkill);
      while (pData->m_iNbSelected > 1)
      {
        if (pSkill->isMergeable())
          pSkill->merge(pData->m_pSkill);
        else
        {
          pSkill = pData->m_pSkill->clone(false, m_pLocalClient->getDebug());
          pAvatar->m_pSkills->addLast(pSkill);
        }
        pData->m_iNbSelected--;
      }
    }
    pData = (SkillData*) m_pAllSkillData->getNext(0);
  }
  wsafecpy(pAvatar->m_pProgression[0].sTreeName, NAME_MAX_CHARS, m_pCurrentPeople->m_sObjectId);

  // Save Avatar
  Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
  pAvatar->m_pOwner = pPlayer;
  pPlayer->addAvatar(pAvatar, m_iGoldPaid);
}
