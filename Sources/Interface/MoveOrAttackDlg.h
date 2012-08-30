#ifndef _MOVEORATTACK_DLG_H
#define _MOVEORATTACK_DLG_H

#include "../GUIClasses/guiDocument.h"

class Unit;
class LocalClient;

class MoveOrAttackDlg : public guiDocument
{
public:
  MoveOrAttackDlg(LocalClient * pLocalClient, Unit * pUnit, CoordsMap mapPos);
  ~MoveOrAttackDlg();

  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);
  virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO) { *isLuaPlayerGO = 3; return m_pTarget; };
  virtual void setTargetValid(bool bValid);

protected:
  LocalClient * m_pLocalClient;
  int m_iMoveToTex;
  GraphicObject * m_pTarget;
};

#endif
