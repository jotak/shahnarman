#include "MoveOrAttackDlg.h"
#include "../Data/LocalisationTool.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/GameboardManager.h"
#include "../LocalClient.h"
#include "../GUIClasses/guiToggleButton.h"
#include "InterfaceManager.h"
#include "InfoboxDlg.h"

// -----------------------------------------------------------------
// Name : MoveOrAttackDlg
//  Constructor
// -----------------------------------------------------------------
MoveOrAttackDlg::MoveOrAttackDlg(LocalClient * pLocalClient, Unit * pUnit, CoordsMap mapPos) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pTarget = NULL;
  m_iMoveToTex = pLocalClient->getDisplay()->getTextureEngine()->loadTexture(L"moveto_icon");

  // Document
  init(L"MoveOrAttackPopupDocument",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  // Button "Move to position"
  int xPxl = 0;
  int yPxl = 0;
  int iButton = 1;
  guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(m_iMoveToTex, SMALL_ICON_SIZE, L"MoveBtn", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  addComponent(pBtn);
  wchar_t sText[LABEL_MAX_CHARS] = L"";
  i18n->getText(L"MOVE_TO_POS", sText, LABEL_MAX_CHARS);
  pBtn->setTooltipText(sText);

  // Find foe units on this tile
  wchar_t sBuf[LABEL_MAX_CHARS] = L"";
  xPxl += SMALL_ICON_SIZE + SMALL_ICON_SPACING;
  MapTile * pTile = pLocalClient->getGameboard()->getMap()->getTileAt(mapPos);
  assert(pTile != NULL);
  u8 owner = pUnit->getOwner();
  Unit * pOther = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT);
  while (pOther != NULL)
  {
    if (pOther->getOwner() != owner)
    {
      xPxl = (iButton % 8) * (SMALL_ICON_SIZE + SMALL_ICON_SPACING);
      yPxl = (iButton / 8) * (SMALL_ICON_SIZE + SMALL_ICON_SPACING);
      int itex = pOther->getTexture();
      pBtn = guiToggleButton::createDefaultTexturedToggleButton(itex, SMALL_ICON_SIZE, L"EnnemyBtn", pLocalClient->getDisplay());
      pBtn->moveTo(xPxl, yPxl);
      pBtn->setAttachment(pOther);
      addComponent(pBtn);
      i18n->getText(L"ATTACK_(s)", sBuf, LABEL_MAX_CHARS);
      swprintf(sText, LABEL_MAX_CHARS, sBuf, pOther->getName());
      pBtn->setTooltipText(sText);
      iButton++;
    }
    pOther = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
  }

  if (iButton < 8)
    setDimensions(iButton * (SMALL_ICON_SIZE + SMALL_ICON_SPACING) - SMALL_ICON_SPACING, SMALL_ICON_SIZE);
  else
    setDimensions(8 * (SMALL_ICON_SIZE + SMALL_ICON_SPACING) - SMALL_ICON_SPACING, (iButton / 8) * (SMALL_ICON_SIZE + SMALL_ICON_SPACING) - SMALL_ICON_SPACING);
}

// -----------------------------------------------------------------
// Name : ~MoveOrAttackDlg
//  Destructor
// -----------------------------------------------------------------
MoveOrAttackDlg::~MoveOrAttackDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy MoveOrAttackDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy MoveOrAttackDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * MoveOrAttackDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
  m_pTarget = NULL;
  guiComponent * cpnt = getFirstComponent();
  while (cpnt != NULL)
  {
    if (cpnt->isAt(xPxl, yPxl))
    {
      if (wcscmp(cpnt->getId(), L"MoveBtn") == 0)
      {
        m_pTarget = cpnt;
        break;
      }
      MapObject * pAttachment = (MapObject*) cpnt->getAttachment();
      if (pAttachment != NULL)
      {
        m_pTarget = pAttachment;
        wchar_t sBuf[LABEL_MAX_CHARS] = L"";
        m_pLocalClient->getInterface()->getInfoDialog()->setInfoText(pAttachment->getInfo(sBuf, LABEL_MAX_CHARS, Dest_InfoDialog));
      }
      break;
    }
    cpnt = getNextComponent();
  }
  return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void MoveOrAttackDlg::setTargetValid(bool bValid)
{
  guiComponent * pOther = getFirstComponent();
  while (pOther != NULL)
  {
    if (pOther != m_pTarget && pOther->getAttachment() != m_pTarget && pOther->getType() & GOTYPE_BUTTON)
      ((guiToggleButton*)pOther)->setClickState(false);
    else
      ((guiToggleButton*)pOther)->setClickState(bValid);
    pOther = getNextComponent();
  }
}
