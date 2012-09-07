#ifndef _TOWN_H
#define _TOWN_H

#include "MapObject.h"
#include "../DeckData/Ethnicity.h"

#define STRING_GROWTH           "growth"
#define STRING_PRODUCTIVITY     "productivity"
#define STRING_UNITPROD         "unitprod"
#define STRING_HAPPINESS        "happiness"
#define STRING_FEAR             "fear"
#define STRING_RADIUS           "radius"
#define STRING_HEROECHANCES     "heroechances"

class Building;
class Player;
class Server;
class LogDlg;
class DataFactory;

class Town : public MapObject
{
public:
  // Constructor / destructor
  Town(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects);
  ~Town();

  void init(u32 uTownId, u8 uSize, Ethnicity * pEthn, LocalClient * pLocalClient);
  void serialize(NetworkData * pData);
  void deserialize(NetworkData * pData, LocalClient * pLocalClient);
  void serializeForUpdate(NetworkData * pData);
  void deserializeForUpdate(NetworkData * pData);
  void initGraphics(DisplayEngine * pDisplay);

  // GraphicObject virtual functions
  virtual u32 getType() { return MapObject::getType() | GOTYPE_TOWN; };
  virtual void display();

  // MapObject virtual functions
  virtual void initServer();
  virtual int getTexture();
  virtual int getBigTexture();
  virtual void updateIdentifiers() { snprintf(m_sIdentifiers, 16, "town %d %ld", (int) m_uOwner, (long) m_uTownId); };

  // Get / set from network
  void getOrders(NetworkData * pData);
  void updateOrders(NetworkData * pData);

  // Other functions
  virtual char * getInfo(char * sBuf, int iSize, InfoDest eDest);
  u8 getUnitProdTime();
  void newTurn(Player * pOwner, Server * pServer);
  u16 getMaxFoodStorage();
  s16 getUnitProdBonus();
  void buildBuilding(const char * sName, Server * pServer);
  long getInfluenceAt(CoordsMap mp);
  bool isBuildingAllowed(Building * pBuild);

  // Member access
  u32 getId() { return m_uTownId; };
  char * getName() { return m_sName; };
  char * getEthnicityEdition() { return m_sEthnicityEdition; };
  char * getEthnicityId() { return m_sEthnicityId; };
  u8 getSize() { return m_uSize; };
  s16 getFoodPerTurn();
  u16 getFoodInStock() { return m_uFoodInStock; };
  u16 getProdPerTurn();
  u16 getProdInStock() { return m_uProdInStock; };
  u8 getUnitProdInStock() { return m_uUnitProdInStock; };
  Building * getCurrentBuilding();
  void setCurrentBuilding(const char * sName);
  Ethnicity::TownUnit * getCurrentUnit() { return m_pCurrentBuildingUnit; };
  void setCurrentUnit(Ethnicity::TownUnit * pUnit);

  // List access
  ObjectList * getBuildingsList() { return m_pBuildings; };
  Building * getFirstBuilding(int it);
  Building * getNextBuilding(int it);
  ObjectList * getExtraBuildableUnit();

protected:
  void calculateUsedTiles();
  void updateHeroes(Server * pServer);

  // Permanent data
  u32 m_uTownId;                      // TownId identifies uniquely this unit on the gameboard
  char m_sName[NAME_MAX_CHARS];
  char m_sEthnicityEdition[NAME_MAX_CHARS];
  char m_sEthnicityId[NAME_MAX_CHARS];
  char m_sTextures[5][MAX_PATH];
  char m_sBigPicture[MAX_PATH];

  // Variable data
  u8 m_uSize;
  s16 m_iFoodPerTurn;
  u16 m_uFoodInStock;
  u16 m_uProdPerTurn;
  u16 m_uProdInStock;
  u8 m_uUnitProdInStock;

  // Buildings
  ObjectList * m_pBuildings;
  char m_sCurrentBuilding[NAME_MAX_CHARS];

  // Units
  ObjectList * m_pExtraBuildableUnits;
  Ethnicity::TownUnit * m_pCurrentBuildingUnit;
  ObjectList * m_pHeroes;

  // Other
  LocalClient * m_pLocalClient;
};

#endif
