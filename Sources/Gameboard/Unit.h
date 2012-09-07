#ifndef _UNIT_H
#define _UNIT_H

#include "MapObject.h"
#include "../Physics/PhysicalObject.h"
#include "../DeckData/UnitData.h"
#include "../Players/Mana.h"

enum UnitOrder
{
  OrderNone = 0,
  OrderFortify,
  OrderMove,
  OrderAttack,
  OrderSkill
};

enum UnitStatus
{
  US_Normal = 0,
  US_Dead,
  US_Removed
};

class GameboardManager;
class LocalClient;
class UnitOptionsDlg;
class Server;

class Unit : public MapObject, public PhysicalObject
{
friend class PlayerManagerAbstract;
friend class TurnSolver;
public:
  class AStarStep : public BaseObject
  {
  public:
    AStarStep(CoordsMap pos) { this->pos = pos; };
    CoordsMap pos;
  };

  // Constructor / destructor
  Unit(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects);
  ~Unit();

  void serialize(NetworkData * pData);
  void deserialize(NetworkData * pData, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData);
  void serializeForUpdate(NetworkData * pData);
  void deserializeForUpdate(NetworkData * pData, LocalClient * pLocalClient);
  void initGraphics(DisplayEngine * pDisplay);

  // GraphicObject virtual functions
  virtual u32 getType() { return MapObject::getType() | (m_Status == US_Dead ? GOTYPE_DEAD_UNIT : (m_Status == US_Removed ? GOTYPE_REMOVED_UNIT : GOTYPE_UNIT)); };
  virtual void display();
  virtual void update(double delta);

  // MapObject virtual functions
  virtual short getMoveCost(unsigned char terrainType);
  virtual int getTexture();
  virtual void updateIdentifiers() { snprintf(m_sIdentifiers, 16, "unit %d %ld", (int) m_uOwner, (long) m_uUnitId); };

  // Move related function
  virtual void resetMoveData();
  bool recomputePath();

  // Order functions
  UnitOrder getOrder() { return m_Order; };
  virtual void unsetOrder();
  void setFortifyOrder();
  bool setMoveOrder(CoordsMap mapPos, UnitOptionsDlg * pDlg);
  bool setAttackOrder(Unit * pAttackTarget, UnitOptionsDlg * pDlg);
  bool setGroupMoveOrder(CoordsMap mapPos, UnitOptionsDlg * pDlg);
  bool setGroupAttackOrder(Unit * pAttackTarget, UnitOptionsDlg * pDlg);
  void retrievePreviousOrder(UnitOptionsDlg * pDlg);
  void saveCurrentOrder();
  Unit * getAttackTarget() { return m_pAttackTarget; };
  void setSkillOrder(u32 uSkillId, u16 uEffectId);
  ChildEffect * getSkillOrder();

  // Other functions
  virtual char * getInfo(char * sBuf, int iSize, InfoDest eDest);
  void addSkill(Skill * pSkill);
  void setNameDescriptionTexture(char * sName, char * sDesc, char * sTex); // for avatar only

  // Member access
  u32 getId() { return m_uUnitId; };
  char * getName() { return m_sName; };
  CoordsMap getDestination();
  CoordsMap getPathTurnPosition() { return ((AStarStep*) m_pAStarPath->getCurrent(m_iPathTurnIterator))->pos; };
  ObjectList * getPath() { return m_pAStarPath; };
  UnitStatus getStatus() { return m_Status; };
  void setStatus(UnitStatus status) { m_Status = status; };
  void setGroup(MetaObjectList * pGroup) { m_pGroup = pGroup; };
  MetaObjectList * getGroup() { return m_pGroup; };
  ObjectList * getSkillsRef() { return m_pSkillsRef; };
  void setPlayerColor(F_RGBA color) { m_PlayerColor = color; };
  virtual long getValue(const char * sName, bool bBase = false, bool * bFound = NULL);
  char * getUnitModelId() { return m_sUnitId; };
  char * getUnitEdition() { return m_sEdition; };

  // find data
  Skill * findSkill(u32 uSkillId, bool * isActive = NULL);
  UnitData * getUnitData(LocalClient * pLocalClient);
  UnitData * getUnitData(Server * pServer);

  // Server-side functions only
  void init(u32 uUnitId, u8 uPlayerId, UnitData * pData, DebugManager * pDebug);
  bool setBaseValue(const char * sName, long val);
  void setMapPos(CoordsMap coords);
  void setHasAttacked(bool bHasAttacked) { m_bHasAttacked = bHasAttacked; };
  bool canAttack() { return !m_bHasAttacked && m_Status == US_Normal; };
  bool canDefend() { return m_Status == US_Normal; };
  void onNewTurn();
  bool wasModified() { return m_bModified; };
  void setModified(bool bMod) { m_bModified = bMod; };

protected:
  bool computePath(CoordsMap mapPos);
  bool computeGroupPath(CoordsMap mapPos);
  int findNextTurnPosition(CoordsMap * path, int pathSize);

  // Permanent data
  u32 m_uUnitId;                      // UnitId identify uniquely this unit on the gameboard
  char m_sUnitId[NAME_MAX_CHARS];  // this identifies the unit's model
  char m_sEdition[NAME_MAX_CHARS];
  char m_sName[NAME_MAX_CHARS];
  char m_sDescription[DESCRIPTION_MAX_CHARS];
  char m_sTexture[MAX_PATH];

  // Less permanent data
  UnitStatus m_Status;
  UnitOrder m_Order;
  UnitOrder m_PreviousOrder;
  MetaObjectList * m_pGroup;

  // Attack targets
  Unit * m_pAttackTarget;
  Unit * m_pPreviousAttackTarget;

  // Skill activation data
  u32 m_uActivatedSkillId;
  u16 m_uActivatedSkillEffectId;

  // Other
  F_RGBA m_PlayerColor;

  // Path
  CoordsMap m_PreviousGoal;
  ObjectList * m_pAStarPath;
  int m_iPathTurnIterator;

  // Server-side variables only
  bool m_bModified;
  bool m_bHasAttacked;
  bool m_bNewTurnDone;

private:
  ObjectList * m_pSkillsRef;  // This list is only used to instanciate unit skills. It's a copy of UnitData::m_pSkills. To use any skill effect, use list LuaTargetable::m_pEffects.
};

#endif
