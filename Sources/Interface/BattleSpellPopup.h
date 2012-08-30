#ifndef _BATTLESPELL_POPUP_H
#define _BATTLESPELL_POPUP_H

#include "../GUIClasses/guiPopup.h"
#include "../Players/Player.h"

#define BATTLESPELL_BUTTON_NEVERBATTLE    1
#define BATTLESPELL_BUTTON_FINISHED       2

class BattleSpellPopup : public guiPopup
{
public:
  BattleSpellPopup(Player * pPlayer, int iTimer, DisplayEngine * pDisplay);
  ~BattleSpellPopup();

  void update(double delta);
  int getResponse() { return m_iResponse; };
  void setReponse(int resp) { m_iResponse = resp; };

protected:
  Player * m_pPlayer;
  double m_fTimer;
  int m_iResponse;
};

#endif
