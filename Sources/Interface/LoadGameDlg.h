#ifndef _LOAD_GAME_DLG_H
#define _LOAD_GAME_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiComponent;
class guiContainer;

class LoadGameDlg : public guiDocument
{
public:
  LoadGameDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
  ~LoadGameDlg();

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

protected:
  void loadGamesList();
  void loadGameInfo(wchar_t * sGameName);

  LocalClient * m_pLocalClient;
  guiContainer * m_pGamesPanel;
  guiContainer * m_pGameInfoPanel;
  wchar_t m_sSelectedGameName[MAX_PATH];
};

#endif
