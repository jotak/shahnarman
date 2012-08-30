#ifndef _BUILD_DECK_DLG_H
#define _BUILD_DECK_DLG_H

#include "../GUIClasses/guiList.h"

class LocalClient;
class Profile;
class AvatarData;
class GeometryQuads;
class guiImage;
class Spell;
class guiPopup;

class BuildDeckDlg : public guiDocument
{
public:
  BuildDeckDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
  ~BuildDeckDlg();

  // Input functions
  virtual void update(double delta);
  virtual void updateEditAvatarInfosPopup();
  bool onClickStart();

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void onShow();
  void onHide();

  guiPopup ** showEditAvatarInfosPopup(AvatarData * pAvatar, guiDocument * pCaller);

protected:
  void moveSelectedSpells(guiList * pFrom, guiList * pTo);
  void reloadSpells();
  void onSpellSelected(BaseObject*);
  void onAvatarSelected();
  void loadEditionButtons();

  LocalClient * m_pLocalClient;
  guiList * m_pEquippedSpells;
  guiList * m_pNotEquippedSpells;
  Profile * m_pCurrentPlayer;
  AvatarData * m_pCurrentAvatar;
  u8 m_uDraggingFrom;
  guiImage * m_pDragImage;
  guiContainer * m_pAvatarsPanel;
  Spell * m_pCurrentSpell;
  guiPopup * m_pAvatarPopup;
  guiPopup * m_pObjectPopup;
  guiPopup * m_pEditAvatarInfosPopup;
  guiPopup * m_pConfirmDelete;
  AvatarData * m_pCurrentEditingAvatar;
  guiDocument * m_pEditAvatarCaller;
  bool m_bEquipedDeployed;
  bool m_bNotEquipedDeployed;
};

#endif
