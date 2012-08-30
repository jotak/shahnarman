#ifndef _BUILDING_H
#define _BUILDING_H

#include "../Data/LuaObject.h"
#include "../Server/NetworkData.h"

#define BUILDING_OBJECT_NAME   L"buildings"

class Town;

class Building : public LuaObject
{
public:
  Building(int iX, int iY, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug);
  Building(u32 uId, int iX, int iY, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug);
  ~Building();

  virtual u32 getType() { return LUAOBJECT_BUILDING; };
  virtual void loadBasicData(DebugManager * pDebug);
  void getNetworkData(NetworkData * pData);
  wchar_t * getLocalizedName() { return m_sName; };
  wchar_t * getLocalizedDescription() { return m_sDescription; };
  wchar_t * getIconPath() { return m_sTexture; };
  u16 getProductionCost() { return m_uCost; };
  int getXPos() { return m_iXPxl; };
  int getYPos() { return m_iYPxl; };
  void setBuilt(bool b) { m_bIsBuilt = b; };
  bool isBuilt() { return m_bIsBuilt; };
  void setCaster(Town * pCaster) { m_pCaster = pCaster; };
  Town * getCaster() { return m_pCaster; };

protected:
  void init(int iX, int iY, DebugManager * pDebug);

  wchar_t m_sName[NAME_MAX_CHARS];
  wchar_t m_sDescription[DESCRIPTION_MAX_CHARS];
  wchar_t m_sTexture[MAX_PATH];
  u16 m_uCost;
  int m_iXPxl;
  int m_iYPxl;
  bool m_bIsBuilt;

  Town * m_pCaster; // Temporary data used when activating skill
};

#endif
