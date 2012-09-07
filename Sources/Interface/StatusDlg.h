#ifndef _STATUS_DLG_H
#define _STATUS_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;

class StatusDlg : public guiDocument
{
public:
  StatusDlg(LocalClient * pLocalClient);
  ~StatusDlg();

  // Other
  void showStatus(char * sMessage);
  void hide();

private:
  LocalClient * m_pLocalClient;
};

#endif
