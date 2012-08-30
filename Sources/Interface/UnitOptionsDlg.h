#ifndef _GUI_UNIT_OPTIONS_H
#define _GUI_UNIT_OPTIONS_H

#include "../GUIClasses/guiDocument.h"

class guiToggleButton;
class Unit;
class LocalClient;
class guiComponent;

class UnitOptionsDlg : public guiDocument
{
public:
  UnitOptionsDlg(LocalClient * pLocalClient);
  ~UnitOptionsDlg();

  virtual bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void resetOtherButtons(guiToggleButton * pBtn);

  void updateOrder();
  void setUnit(Unit * unit);
  Unit * getUnit() { return m_pUnit; };
  void cancelSkillAction(Unit * pUnit = NULL);
  void redoSkillAction(Unit * pUnit = NULL);

protected:
  Unit * m_pUnit;
  LocalClient * m_pLocalClient;
};

#endif
