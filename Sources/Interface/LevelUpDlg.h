#ifndef _LEVELUP_DLG_H
#define _LEVELUP_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class AvatarData;
//class guiComboBox;
//class guiImage;
//class guiToggleButton;
//class guiLabel;
class guiPopup;
class ProgressionElement;
class ProgressionTree;

class LevelUpDlg : public guiDocument
{
  //class LevelUpTree
  //{
  //public:
  //  guiLabel * pTopLabel;
  //  guiComboBox * pTopCombo;
  //  guiImage * pAllImages[5][2];
  //  guiToggleButton * pAllButtons[5][2];
  //};

public:
  LevelUpDlg(LocalClient * pLocalClient);
  ~LevelUpDlg();

  virtual void update(double delta);

  // Misc.
  void doAvatarLevelUp(AvatarData * pAvatar, guiDocument * pCaller, bool bFirstCall);

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void onHide();

private:
  void setCurrentAvatar(AvatarData * pAvatar);
  void addChoiceButton(int xPxl, int yPxl, int btnSize, ProgressionElement * pElt, u8 uState);
  ProgressionElement * getChosenElementAtLevel(ProgressionTree * pTree, int iTree, int iLevel);

  LocalClient * m_pLocalClient;
  AvatarData * m_pCurrentAvatar;
  ProgressionElement * m_pSelectedElement;
  guiDocument * m_pCaller;
  guiPopup ** m_pEditAvatarInfosPopup;
  s8 m_iSpecialLevel;
};

#endif
