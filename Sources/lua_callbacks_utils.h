#ifndef _LUA_CALLBACKS_UTILS_H
#define _LUA_CALLBACKS_UTILS_H

#include <lua5.1/lua.hpp>

#include "GameRoot.h"
#include "LocalClient.h"
#include "Data/LocalisationTool.h"
#include "Gameboard/Unit.h"
#include "Gameboard/Town.h"
#include "Gameboard/Building.h"
#include "Gameboard/Temple.h"
#include "Gameboard/GameboardManager.h"
#include "Gameboard/GameboardInputs.h"
#include "Players/PlayerManager.h"
#include "Players/Player.h"
#include "Players/Spell.h"
#include "Interface/InterfaceManager.h"
#include "Interface/SpellDlg.h"
#include "Interface/InfoboxDlg.h"
#include "Interface/SpellsSelectorDlg.h"
#include "Interface/PlayerSelectorDlg.h"
#include "Interface/UnitOptionsDlg.h"
#include "Interface/StatusDlg.h"
#include "GUIClasses/guiFrame.h"
#include "Debug/DebugManager.h"
#include "Server/Server.h"
#include "Server/TurnSolver.h"

extern u8 g_uLuaSelectTargetType;
extern u32 g_uLuaSelectConstraints;
extern char g_sLuaSelectCallback[128];
extern bool g_bLuaSelectOnResolve;
extern u32 g_uLuaCurrentObjectType;
extern bool g_bLuaEvaluationMode;
extern lua_State * g_pLuaStateForTarget;

extern bool setValidFor(u32 uTypes, const char * sFuncName);
extern Server * checkServerResolving(const char * sFuncName);
extern Map * getServerOrLocalMap();
extern PlayerManagerAbstract * getServerOrLocalPlayerManager();
extern Player * getServerOrLocalPlayer(u8 uId);
extern bool checkNumberOfParams(lua_State * pState, int iExpected, const char * sFuncName);
extern int checkNumberOfParams(lua_State * pState, int iMinExpected, int iMaxExpected, const char * sFuncName);
extern bool readTargetData(const char * sType, const char * sConstraints, const char * sCallback, const char * sFuncName);
extern bool checkCustomConstraints(CoordsMap mp, BaseObject * pObj, u8 uType);
extern bool checkRange(Player * pCaster, Unit * pUnit, CoordsMap mp);
extern bool checkTileConstraints(CoordsMap mp);
extern bool checkMapObjConstraints(MapObject * pObj);
extern bool checkSpellConstraints(Spell * pObj);
extern bool checkTargetPlayer(Player * pPlayer);
extern bool checkTargetObject(MapObject * pObj);
extern bool checkTargetSpell(Spell * pObj);
extern LuaTargetable * checkTargetTile(CoordsMap mp, bool * bMultipleTargets);
extern void clbkSelectTarget_cancelSelection(u32 uType, int iResolve);
extern void clbkSelectTarget_cancelSelection();
extern void clbkSelectTarget_OnMouseOverGameboard(CoordsMap mp);
extern void clbkSelectTarget_OnClickGameboard(CoordsMap mp);
extern bool clbkSelectTarget_OnMouseOverInterface(BaseObject * pBaseObj, u8 enum1Spell2Player3Mapobj);
extern bool clbkSelectTarget_OnClickInterface(BaseObject * pObj, u8 enum1Spell2Player3Mapobj);
extern int LUA_split(lua_State * pState);
extern int LUA_splitint(lua_State * pState);

#endif
