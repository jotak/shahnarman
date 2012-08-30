#ifndef _INTERFACEMANAGER_H
#define _INTERFACEMANAGER_H

#include "../Common/BaseObject.h"
#include "../Input/EventListener.h"

#define SMALL_ICON_SIZE       32
#define SMALL_ICON_SPACING    4

class LocalClient;
class Player;
class guiPopup;
class StartMenuDlg;
class SelectPlayerAvatarDlg;
class BuildDeckDlg;
class ArtifactsEquipDlg;
class ShopDlg;
class LoadGameDlg;
class SpellDlg;
class LogDlg;
class InfoboxDlg;
class UnitOptionsDlg;
class ResolveDlg;
class GameOverDlg;
class Tooltip;
class GraphicObject;
class guiFrame;
class ObjectList;
class Unit;
class MapTile;
class guiDocument;
class guiObject;
class StackGroupInterface;
class HostGameDlg;
class guiComponent;
class LevelUpDlg;
class OptionsDlg;
class MoveOrAttackDlg;
class MapObjectDlg;
class SpellsSelectorDlg;
class PlayerSelectorDlg;
class StatusDlg;
class CreateAvatarDlg;
class GeometryQuads;

typedef bool CLBK_INTERFACE_TARGET_ON_MOUSE_OVER(BaseObject*,u8);
typedef bool CLBK_INTERFACE_TARGET_ON_CLICK(BaseObject*,u8);  // must return false if don't handle click (eg. invalid target)

class InterfaceManager : public EventListener
{
public:
  class TopDisplayObject : public BaseObject
  {
  public:
    guiComponent * pObj;
    int iX, iY;
    F_RGBA cpntColor;
    F_RGBA docColor;
  };
  // Constructor / destructor
  InterfaceManager(LocalClient * pLocalClient);
  ~InterfaceManager();

  // Manager functions
  void Init();
  void InitMenu();
  void InitGame();
  void Update(double delta);
  void Display();

  // Input functions
  bool onCatchButtonEvent(ButtonAction * pEvent);
  bool onCursorMoveEvent(int xPxl, int yPxl);

  // Frame management
  void registerFrame(guiFrame * pFrm);
  guiFrame * findFrame(const wchar_t * frmId);
  guiFrame * findFrameFromDoc(guiDocument * pDoc);
  void deleteFrame(guiFrame * pFrm);
  guiFrame * deleteCurrentFrame(int iterator);
  void bringFrameAbove(guiFrame * pFrm);
  guiComponent * getObjectAt(int xPxl, int yPxl, int * xoffset, int * yoffset);

  // Members access functions
  LogDlg * getLogDialog() { return m_pLogWnd; };
  InfoboxDlg * getInfoDialog() { return m_pInfoWnd; };
  ResolveDlg * getResolveDialog() { return m_pResolveWnd; };
  SpellDlg * getSpellDialog() { return m_pSpellWnd; };
  MapObjectDlg * getMapObjectDialog() { return m_pMapObjectWnd; };
  GameOverDlg * getGameOverDialog() { return m_pGameOverWnd; };
  UnitOptionsDlg * getUnitOptionsDialog() { return m_pUnitOptionsWnd; };
  StatusDlg * getStatusDialog() { return m_pStatusWnd; };
  GeometryQuads * getMenuBackground() { return m_pMenuBgGeometry; };

  // Other functions
  void enableNextPlayer(Player * pPlayer);
  void disableNoPlayer();
  void waitLocalPlayer();
  void updateUnitOptionsDialog(Unit * unit);
  void showStack(MapTile * pTile, u32 uDisplayFilter, u32 uClickFilter, StackGroupInterface * pGroupInterface, GraphicObject * pDefaultSelectedObject);
  void showResolveDialog();
  void hideResolveDialog();
  void showMapObjectDialog(MapTile * pTile);
  void hideMapObjectDialog();
  void showGameOverDialog(ObjectList * pWinners);
  void hideGameOverDialog();
  void showMoveOrAttackDialog(Unit * pUnit, CoordsMap mapPos);
  void hideMoveOrAttackDialog();
  void setTargetMode(CLBK_INTERFACE_TARGET_ON_MOUSE_OVER * pCallback1, CLBK_INTERFACE_TARGET_ON_CLICK * pCallback2);
  void unsetTargetMode() { m_bTargetMode = false; };
  void topDisplay(guiComponent * pObj, int iX, int iY, F_RGBA cpntColor, F_RGBA docColor);
  void cancelTopDisplay(guiComponent * pObj);
  void showInGameMenu();
  void onResize(int oldw, int oldh);
  bool onClickStart();
  void resetSharedPointers(guiObject * pObj = NULL);
  void getRichText(guiDocument * pDest, CoordsScreen offset, wchar_t * sSource);

  // Menu functions
  StartMenuDlg * getStartMenuDialog() { return m_pStartMenuWindow; };
  LoadGameDlg * getLoadGameDialog() { return m_pLoadGameWnd; };
  SelectPlayerAvatarDlg * getSelectPlayerDialog() { return m_pSelectPlayerAvatarWnd; };
  BuildDeckDlg * getBuildDeckDialog() { return m_pBuildDeckWnd; };
  ArtifactsEquipDlg * getArtifactsEquipDialog() { return m_pArtifactsEquipWnd; };
  ShopDlg * getShopDialog() { return m_pShopWnd; };
  HostGameDlg * getHostGameDialog() { return m_pHostGameWnd; };
  LevelUpDlg * getLevelUpDialog() { return m_pLevelUpWnd; };
  OptionsDlg * getOptionsDialog() { return m_pOptionsWnd; };
  SpellsSelectorDlg * getSpellsSelectorDialog() { return m_pSpellsSelectorWnd; };
  PlayerSelectorDlg * getPlayerSelectorDialog() { return m_pPlayerSelectorWnd; };
  CreateAvatarDlg * getCreateAvatarDlg() { return m_pCreateAvatarDlg; };
  void setUniqueDialog(guiDocument * pDoc);

  // Called from LUA
  void askForExtraMana(wchar_t * sDescription, u16 mana, int min, int max, wchar_t * sCallback, u32 uSourceType);

private:
  void deleteAllFrames();
  void createInGameMenu();

  // Popup onclicks
  void onClickNextLocalPlayer(guiComponent * pCpnt);
  void onClickInGameMenu(guiComponent * pCpnt);
  void onClickExtraMana(guiComponent * pCpnt);

  LocalClient * m_pLocalClient;
  ObjectList * m_pFrameList;

  StartMenuDlg * m_pStartMenuWindow;
  SelectPlayerAvatarDlg * m_pSelectPlayerAvatarWnd;
  BuildDeckDlg * m_pBuildDeckWnd;
  ArtifactsEquipDlg * m_pArtifactsEquipWnd;
  ShopDlg * m_pShopWnd;
  HostGameDlg * m_pHostGameWnd;
  LevelUpDlg * m_pLevelUpWnd;
  OptionsDlg * m_pOptionsWnd;
  LoadGameDlg * m_pLoadGameWnd;
  SpellDlg * m_pSpellWnd;
  LogDlg * m_pLogWnd;
  InfoboxDlg * m_pInfoWnd;
  UnitOptionsDlg * m_pUnitOptionsWnd;
  ResolveDlg * m_pResolveWnd;
  GameOverDlg * m_pGameOverWnd;
  SpellsSelectorDlg * m_pSpellsSelectorWnd;
  PlayerSelectorDlg * m_pPlayerSelectorWnd;
  StatusDlg * m_pStatusWnd;
  guiPopup * m_pNextLocalPlayerDlg;
  guiPopup * m_pTooltip;
  guiPopup * m_pInGameMenu;
  guiPopup * m_pExtraMana;
  MoveOrAttackDlg * m_pMoveOrAttackDlg;
  MapObjectDlg * m_pMapObjectWnd;
  CreateAvatarDlg * m_pCreateAvatarDlg;
  float m_fTooltipTime;
  int m_iFrmIt;
  bool m_bTargetMode;
  CLBK_INTERFACE_TARGET_ON_MOUSE_OVER * m_pTargetOnMouseOverClbk;
  CLBK_INTERFACE_TARGET_ON_CLICK * m_pTargetOnClickClbk;
  BaseObject * m_pTargetedObject;
  u8 m_uIsLuaPlayerGO;
  guiObject * m_pClickedObjects[2];
  guiObject * m_pPointedObject;
  ObjectList * m_pTopDisplayList;
  GeometryQuads * m_pMenuBgGeometry;
};

#endif
