#ifndef _MAP_READER_H
#define _MAP_READER_H

#include "../Common/BaseObject.h"
#include <lua.hpp>

class LocalClient;
class ObjectList;
class Ethnicity;

struct TownData
{
  CoordsMap position;
  u8 size;
  wchar_t sEthnId[NAME_MAX_CHARS];
  wchar_t sEthnEdition[NAME_MAX_CHARS];
};

struct TempleData
{
  CoordsMap position;
  u8 mana;
  u8 amount;
};

class MapReader
{
public:
  class MapParameters : public BaseObject
  {
  public:
    MapParameters() { pPossibleValues = NULL; pPossibleValueLabels = NULL; nbValues = 0; defaultValueIndex = 0; };
    ~MapParameters() { FREEARR(pPossibleValues); if (pPossibleValueLabels != NULL) { for (int i = 0; i < nbValues; i++) FREEARR(pPossibleValueLabels[i]); delete[] pPossibleValueLabels; }};
    wchar_t sLabel[256];
    int nbValues;
    int * pPossibleValues;
    wchar_t ** pPossibleValueLabels;
    int defaultValueIndex;
  };

  // Constructor / destructor
  MapReader(LocalClient * pLocalClient);
  ~MapReader();

  // General
  bool init(const wchar_t * sMapPath);
  wchar_t * getMapFile() { return m_sLuaFile; };
  bool getMapName(wchar_t * sString, int size);
  bool getMapParameters(ObjectList * pList, int iLabelMaxSize);
  void setMapParameters(int * pCustomParams, int nbParams, int nbPlayers);
  static void deleteMapParameters(ObjectList * pList);
  bool generate();
  int * getMap() { return m_Map; };
  int getMapWidth() { return m_iWidth; };
  int getMapHeight() { return m_iHeight; };
  std::vector<TownData> * getTowns() { return &m_TownsList; };
  std::vector<TempleData> * getTemples() { return &m_TemplesList; };
  std::vector<CoordsMap> * getSpecialTiles() { return &m_SpecTilesList; };
  CoordsMap getPlayerPosition(int iPlayer) { return m_pPlayersPos[iPlayer]; };

private:
  bool isLuaCallValid(int iError, const wchar_t * sFuncName, const wchar_t * sParams);
  Ethnicity * computeEthnicity(int x, int y, ObjectList * pList);

  LocalClient * m_pLocalClient;
  lua_State * m_pLuaState;
  wchar_t m_sLuaFile[MAX_PATH];
  int * m_Map;    // Map data
  int m_iWidth;
  int m_iHeight;
  std::vector<TownData> m_TownsList;
  std::vector<TempleData> m_TemplesList;
  std::vector<CoordsMap> m_SpecTilesList;
  int m_iNbPlayers;
  CoordsMap * m_pPlayersPos;
};

#endif
