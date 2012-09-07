#ifndef _GUI_LOG_H
#define _GUI_LOG_H

#include "../GUIClasses/guiDocument.h"

#define LOG_ACTION_NONE         0
#define LOG_ACTION_ZOOMTO       1
#define LOG_ACTION_UNITSCREEN   2
#define LOG_ACTION_TOWNSCREEN   3
// Memo : remember to also update Server::createLogMessage() when adding / changing action options

class LocalClient;
class guiLabel;
class guiButton;

class LogDlg : public guiDocument
{
  class LogItem : public BaseObject
  {
  public:
    LogItem(guiLabel * pLbl, guiButton * pBtn, int iTurn) { this->pLbl = pLbl; this->pBtn = pBtn; this->iTurn = iTurn; };
    guiLabel * pLbl;
    guiButton * pBtn;
    int iTurn;
  };
public:
  LogDlg(LocalClient * pLocalClient);
  ~LogDlg();

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

  // Other
  void logNewTurn();
  void log(char * sText, u8 uLevel = 0, u8 uAction = LOG_ACTION_NONE, void * pParam = NULL);

private:
  u16 m_uLogs;
  LocalClient * m_pLocalClient;
  bool m_bLastLogIsNewTurn;
  ObjectList * m_pMapPosList;
  ObjectList * m_pAllLogs;
};

#endif
