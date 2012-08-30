#ifndef _CREATE_AVATAR_DLG_H
#define _CREATE_AVATAR_DLG_H

#include "../GUIClasses/guiDocument.h"

#define MELEE       0
#define RANGE       1
#define ARMOR       2
#define ENDURANCE   3
#define SPEED       4

class LocalClient;
class Edition;
class Ethnicity;
class Skill;
class guiPopup;

class CreateAvatarDlg : public guiDocument
{
public:
  class SkillData : public BaseObject
  {
  public:
    SkillData(Skill * pSkill, int iCost, int iNbSelected) { m_pSkill = pSkill; m_iCost = iCost; m_iNbSelected = iNbSelected; };
    Skill * m_pSkill;
    int m_iCost;
    int m_iNbSelected;
  };

  CreateAvatarDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
  ~CreateAvatarDlg();

  void update(double delta);
  void onShow();

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

protected:
  void onEditionChanged(Edition * pEdition);
  void onEthnicityChanged(Ethnicity * pEthn);
  void createLeftRightArrows(guiComponent * pCpnt, guiDocument * pDoc = NULL);
  void onArrowClick(guiComponent * pArrow);
  void checkArrows(guiComponent * pCpnt, guiDocument * pDoc = NULL);
  void computePoints();
  void saveAndQuit();

  LocalClient * m_pLocalClient;
  guiContainer * m_pSkillsPanel;
  ObjectList * m_pEditionRelativeCpnt;
  Edition * m_pCurrentEdition;
  Ethnicity * m_pCurrentPeople;
  int m_iPointsLeft;
  int m_iGoldPaid;
  int m_iCarac[5];
  int m_iImage;
  ObjectList * m_pAllSkillData;
  ObjectList * m_pImagesSubList;
  guiPopup * m_pRemainPointsPopup;
};

#endif
