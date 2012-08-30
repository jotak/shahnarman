#ifndef _PLAYERSELECTOR_DLG_H
#define _PLAYERSELECTOR_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiContainer;
class guiImage;

typedef void CLBK_ON_CANCEL();

class PlayerSelectorDlg : public guiDocument
{
public:
  PlayerSelectorDlg(LocalClient * pLocalClient);
  ~PlayerSelectorDlg();

  void showPlayers(ObjectList * pPlayers, CLBK_ON_CANCEL * pCancelCallback);
  void hide();
  virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO) { *isLuaPlayerGO = 2; return (m_pTarget == NULL) ? NULL : m_pTarget->getAttachment(); };
  virtual void setTargetValid(bool bValid);

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);

protected:
  LocalClient * m_pLocalClient;
  guiImage * m_pSelectorImg;
  guiComponent * m_pTarget;
  CLBK_ON_CANCEL * m_pCancelCallback;
};

#endif
