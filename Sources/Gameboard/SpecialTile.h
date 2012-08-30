#ifndef _SPECIAL_TILE_H
#define _SPECIAL_TILE_H

#include "../Data/LuaObject.h"

#define SPECTILE_OBJECT_NAME   L"spectiles"

class SpecialTile : public LuaObject
{
public:
  SpecialTile(int iFreq, CoordsMap mapPos, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug);
  SpecialTile(u32 uId, int iFreq, CoordsMap mapPos, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug);
  ~SpecialTile();

  virtual u32 getType() { return LUAOBJECT_SPECIALTILE; };
  virtual void loadBasicData(DebugManager * pDebug);
  static SpecialTile * deserialize(NetworkData * pData, DebugManager * pDebug);
  void serialize(NetworkData * pData);
  wchar_t * getLocalizedName() { return m_sName; };
  wchar_t * getLocalizedDescription() { return m_sDescription; };
  CoordsMap getMapPos();
  int getFrequency() { return m_iFreq; };
  SpecialTile * instanciate(CoordsMap mapPos, DebugManager * pDebug);
  wchar_t * getIconPath() { return m_sTexPath; };
  bool isAIAttracting() { return m_bAttractAI; };

protected:
  void init(int iFreq, CoordsMap mapPos, DebugManager * pDebug);

  wchar_t m_sName[NAME_MAX_CHARS];
  wchar_t m_sDescription[DESCRIPTION_MAX_CHARS];
  wchar_t m_sTexPath[MAX_PATH];
  int m_iFreq;
  bool m_bAttractAI;
};

#endif
