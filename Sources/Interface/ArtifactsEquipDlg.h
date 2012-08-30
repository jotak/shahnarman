#ifndef _ARTIFACTS_EQUIP_DLG_H
#define _ARTIFACTS_EQUIP_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class Profile;
class AvatarData;
class Artifact;
class guiImage;
class guiPopup;

class ArtifactsEquipDlg : public guiDocument
{
public:
  ArtifactsEquipDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
  ~ArtifactsEquipDlg();

  // Input functions
  virtual void update(double delta);
  bool onClickStart();

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void onShow();
  void onHide();

protected:
  void reloadArtifacts();
  void onArtifactSelected(BaseObject*);
  void onAvatarSelected();
  void loadEditionButtons();

  LocalClient * m_pLocalClient;
  Profile * m_pCurrentPlayer;
  AvatarData * m_pCurrentAvatar;
  u8 m_uDraggingFrom;
  guiImage * m_pDragImage;
  guiImage * m_pPlayerBgImage;
  guiImage * m_pHighlightImage;
  guiImage * m_pArtifactsImages[5];
  guiContainer * m_pAvatarsPanel;
  guiContainer * m_pArtifactsPool;
  Artifact * m_pCurrentArtifact;
  guiPopup * m_pAvatarPopup;
  guiPopup * m_pObjectPopup;
  guiPopup ** m_pEditAvatarInfosPopup;
  guiPopup * m_pConfirmDelete;
  AvatarData * m_pCurrentEditingAvatar;
  guiDocument * m_pEditAvatarCaller;
};

#endif
