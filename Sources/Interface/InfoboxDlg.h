#ifndef _GUI_INFOBOX_H
#define _GUI_INFOBOX_H

#include "../GUIClasses/guiDocument.h"
#include "../Input/KeyboardListener.h"

class MapTile;
class StackGroupInterface;
class Player;
class LocalClient;
class MetaObjectList;
class guiImage;
class guiLabel;
class guiFrame;

class InfoboxDlg : public guiDocument, public KeyboardListener
{
public:
  InfoboxDlg(int iWidth, LocalClient * pLocalClient);
  ~InfoboxDlg();

  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void update(double delta);
  void setInfoText(char * sText);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);
  void setMapTile(MapTile * pTile, u32 uDisplayFilter, u32 uClickFilter, StackGroupInterface * pGroupInterface, Player * pCurrentPlayer, GraphicObject * pDefaultSelectedObject);
  virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO) { *isLuaPlayerGO = 3; return (m_pTarget == NULL) ? NULL : m_pTarget->getAttachment(); };
  virtual void setTargetValid(bool bValid);
  void updatePlayersState();

protected:
  void checkSize();
  int addObjects(int yPxl, Player * pCurrentPlayer, bool bButtons);
  bool addButton(GraphicObject * pObj, int xPxl, int yPxl, int iSize);
  bool addImage(GraphicObject * pObj, int xPxl, int yPxl, int iSize);

  MapTile * m_pMapTile;
  u32 m_uDisplayFilter;
  u32 m_uClickFilter;
  StackGroupInterface * m_pGroupInterface;
  guiComponent * m_pTarget;
  MetaObjectList * m_pCurrentGroup;
  LocalClient * m_pLocalClient;
  int m_iLittleSquareYDecal;
  guiImage * m_pLittleSquare;
  guiLabel * m_pPlayersList;
  guiLabel * m_pInfoText;
  //int * m_pPlayerStates;
  //int m_iLastTurnTimer;
//  char m_sLastTurnTimer[64];
  int m_iStackHeight;
  bool m_bIsSpellDlgSticked;
  guiFrame * m_pSpellFrm;
};

#endif
