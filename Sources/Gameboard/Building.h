#ifndef _BUILDING_H
#define _BUILDING_H

#include "../Data/LuaObject.h"
#include "../Server/NetworkData.h"

#define BUILDING_OBJECT_NAME   "buildings"

class Town;

class Building : public LuaObject
{
public:
  Building(int iX, int iY, char * sEdition, char * sObjectName, DebugManager * pDebug);
  Building(u32 uId, int iX, int iY, char * sEdition, char * sObjectName, DebugManager * pDebug);
  ~Building();

  virtual u32 getType() { return LUAOBJECT_BUILDING; };
  virtual void loadBasicData(DebugManager * pDebug);
  void getNetworkData(NetworkData * pData);
  char * getLocalizedName() { return m_sName; };
  char * getLocalizedDescription() { return m_sDescription; };
  char * getIconPath() { return m_sTexture; };
  u16 getProductionCost() { return m_uCost; };
  int getXPos() { return m_iXPxl; };
  int getYPos() { return m_iYPxl; };
  void setBuilt(bool b) { m_bIsBuilt = b; };
  bool isBuilt() { return m_bIsBuilt; };
  void setCaster(Town * pCaster) { m_pCaster = pCaster; };
  Town * getCaster() { return m_pCaster; };

protected:
  void init(int iX, int iY, DebugManager * pDebug);

  char m_sName[NAME_MAX_CHARS];
  char m_sDescription[DESCRIPTION_MAX_CHARS];
  char m_sTexture[MAX_PATH];
  u16 m_uCost;
  int m_iXPxl;
  int m_iYPxl;
  bool m_bIsBuilt;

  Town * m_pCaster; // Temporary data used when activating skill
};

#endif
