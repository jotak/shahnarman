#include "Temple.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : Temple
//  Constructor
// -----------------------------------------------------------------
Temple::Temple(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects) : MapObject(mapPos, pMap, pGlobalEffects, "")
{
  m_uTempleId = 0;
  registerValue(STRING_MANATYPE, 0);
  registerValue(STRING_AMOUNT, 0);
}

// -----------------------------------------------------------------
// Name : ~Temple
//  Destructor
// -----------------------------------------------------------------
Temple::~Temple()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Temple\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy Temple\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Temple::init(u32 uTempleId, u8 uType, u8 uAmount)
{
  m_uTempleId = uTempleId;
  setBaseValue(STRING_MANATYPE, uType);
  setBaseValue(STRING_AMOUNT, uAmount);
  updateIdentifiers();

  // Texture
  switch (uType)
  {
  case MANA_LAW:
    wsafecpy(m_sTexture, MAX_PATH, "temple_law");
    break;
  case MANA_LIFE:
    wsafecpy(m_sTexture, MAX_PATH, "temple_life");
    break;
  case MANA_DEATH:
    wsafecpy(m_sTexture, MAX_PATH, "temple_death");
    break;
  case MANA_CHAOS:
    wsafecpy(m_sTexture, MAX_PATH, "temple_chaos");
    break;
  }
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Temple::serialize(NetworkData * pData)
{
  pData->addLong(m_uTempleId);
  pData->addLong(m_uOwner);
  CoordsMap pos = getMapPos();
  pData->addLong(pos.x);
  pData->addLong(pos.y);
  LuaTargetable::serializeValues(pData);
//  LuaTargetable::serializeEffects(pData, getAllEffects());
//  LuaTargetable::serializeEffects(pData, getDisabledEffects());
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void Temple::deserialize(NetworkData * pData, LocalClient * pLocalClient)
{
  m_uTempleId = pData->readLong();
  m_uOwner = pData->readLong();
  updateIdentifiers();
  CoordsMap pos;
  pos.x = pData->readLong();
  pos.y = pData->readLong();
  setMapPos(pos);
  LuaTargetable::deserializeValues(pData);

  // Lua effects
//  removeAllEffects();
//  LuaTargetable::deserializeEffects(pData, getAllEffects(), m_pBuildings, pLocalClient, NULL, 0);
//  LuaTargetable::deserializeEffects(pData, getDisabledEffects(), m_pBuildings, pLocalClient, NULL, 0);

  // Texture
  switch (getValue(STRING_MANATYPE))
  {
  case MANA_LAW:
    wsafecpy(m_sTexture, MAX_PATH, "temple_law");
    break;
  case MANA_LIFE:
    wsafecpy(m_sTexture, MAX_PATH, "temple_life");
    break;
  case MANA_DEATH:
    wsafecpy(m_sTexture, MAX_PATH, "temple_death");
    break;
  case MANA_CHAOS:
    wsafecpy(m_sTexture, MAX_PATH, "temple_chaos");
    break;
  }
}

// -----------------------------------------------------------------
// Name : initGraphics
// -----------------------------------------------------------------
void Temple::initGraphics(DisplayEngine * pDisplay)
{
  QuadData quad(0.0f, 0.8f, 0.0f, 0.8f, m_sTexture, pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void Temple::display()
{
  Coords3D pos3D = getDisplay()->get3DCoords(getMapPos(), BOARDPLANE);
  pos3D.x += 0.1f;
  pos3D.y += 0.1f;
  m_pGeometry->display(pos3D, F_RGBA_NULL);
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
int Temple::getTexture()
{
  return ((GeometryQuads*)m_pGeometry)->getTexture();
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
char * Temple::getInfo(char * sBuf, int iSize, InfoDest eDest)
{
  char sText[128];
  char sType[32];
  char sAmount[32];
  char sSigns[4] = MANA_SIGNS;
  u8 uType = getValue(STRING_MANATYPE);
  u8 uAmount = getValue(STRING_AMOUNT);
  switch (uType)
  {
  case MANA_LIFE:
    i18n->getText("LIFE", sType, 32);
    break;
  case MANA_LAW:
    i18n->getText("LAW", sType, 32);
    break;
  case MANA_DEATH:
    i18n->getText("DEATH", sType, 32);
    break;
  case MANA_CHAOS:
    i18n->getText("CHAOS", sType, 32);
    break;
  }
  snprintf(sAmount, 32, "%c %d", sSigns[uType], (int) uAmount);
  void * pPhraseArgs[2] = { sType, sAmount };
  i18n->getText("TEMPLE_OF_%$1s_(%$2s)", sText, 128, pPhraseArgs);

  wsafecpy(sBuf, iSize, sText);
  return sBuf;
}
