#include "Player.h"
#include "Spell.h"
#include "Artifact.h"
#include "../Gameboard/GameboardManager.h"
#include "../Data/LocalisationTool.h"
#include "../Gameboard/Unit.h"
#include "../Display/DisplayEngine.h"
#include "../Geometries/GeometryQuads.h"
#include "../DeckData/AvatarData.h"
#include "../DeckData/Edition.h"
#include "../Data/DataFactory.h"
#include "../Data/NetworkSerializer.h"
#include "../LocalClient.h"

// -----------------------------------------------------------------
// Name : Player
//  Constructor
// -----------------------------------------------------------------
Player::Player(u8 playerId, u8 clientId, ObjectList ** pGlobalEffects) : LuaTargetable(pGlobalEffects, L"")
{
  m_pUnits = new ObjectList(true);
  m_pDeadUnits = new ObjectList(true);
  m_pDeck = new ObjectList(true);
  m_pHand = new ObjectList(true);
  m_pActiveSpells = new ObjectList(true);
  m_pDiscard = new ObjectList(true);
  m_pCastSpells = new ObjectList(false);
  m_uPlayerId = playerId;
  m_uClientId = clientId;
  wsafecpy(m_sProfileName, NAME_MAX_CHARS, L"");
  wsafecpy(m_sBanner, 64, L"");
  m_iBannerTex = 0;
  m_Color = F_RGBA_NULL;
  m_pAvatarData = NULL;
  m_pAvatar = NULL;
  registerValue(STRING_NBDRAWN, 1);
  registerValue(STRING_MAXSPELLS, 6);
  registerValue(STRING_SPELLSRANGE, 6);
  registerValue(STRING_MANA_LIFE, 0);
  registerValue(STRING_MANA_LAW, 0);
  registerValue(STRING_MANA_DEATH, 0);
  registerValue(STRING_MANA_CHAOS, 0);
  m_pCapitalTown = NULL;
  m_pBannerGeometry = NULL;
  m_uXPTownPoints = 0;
  m_uXPGross = 0;
  m_uXPNet = 0;
  m_iWonGold = 0;
  m_pWonSpells = new ObjectList(false);
  m_pWonArtifacts = new ObjectList(false);
  m_pWonAvatars = new ObjectList(false);
  resetState();
  swprintf_s(m_sIdentifiers, 16, L"player %d", (int) m_uPlayerId);
  for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
    m_MagicCirclePos[i].x = -1;
  m_bIsAI = false;
}

// -----------------------------------------------------------------
// Name : ~Player
//  Destructor
// -----------------------------------------------------------------
Player::~Player()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Player %d\n", mPlayerId);
#endif
  delete m_pUnits;
  delete m_pDeadUnits;
  delete m_pDeck;
  delete m_pHand;
  delete m_pActiveSpells;
  delete m_pDiscard;
  delete m_pCastSpells;
  FREE(m_pBannerGeometry);
  FREE(m_pAvatarData);
  delete m_pWonSpells;
  delete m_pWonArtifacts;
  delete m_pWonAvatars;
#ifdef DBG_VERBOSE1
  printf("End destroy Player %d\n", mPlayerId);
#endif
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Player::serialize(NetworkData * pData, bool bShort)
{
  // General player data
  if (!bShort)
  {
    pData->addLong(m_uPlayerId);
    pData->addLong(m_uClientId);
    pData->addString(m_sProfileName);
    pData->addString(m_sBanner);
    pData->addLong(m_Color.r);
    pData->addLong(m_Color.g);
    pData->addLong(m_Color.b);
    pData->addLong(m_Color.a);
    pData->addLong(m_bIsAI ? 1 : 0);
  }
  LuaTargetable::serializeValues(pData);
  pData->addLong((long) m_State);
//  pData->addLong(m_uNbMagicCircles);
  for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
  {
    pData->addLong(m_MagicCirclePos[i].x);
    pData->addLong(m_MagicCirclePos[i].y);
  }
  for (int i = 0; i < 4; i++)
    pData->addLong(m_SpentMana[i]);
  if (!bShort)
  {
    pData->addLong(m_uXPTownPoints);
    pData->addLong(m_uXPGross);
    pData->addLong(m_uXPNet);
    pData->addLong(m_iWonGold);
    pData->addLong(m_pWonSpells->size);
    Spell * pSpell = (Spell*) m_pWonSpells->getFirst(0);
    while (pSpell != NULL)
    {
      pData->addString(pSpell->getObjectEdition());
      pData->addString(pSpell->getObjectName());
      pSpell = (Spell*) m_pWonSpells->getNext(0);
    }
    pData->addLong(m_pWonArtifacts->size);
    Artifact * pArtifact = (Artifact*) m_pWonArtifacts->getFirst(0);
    while (pArtifact != NULL)
    {
      pData->addString(pArtifact->getEdition());
      pData->addString(pArtifact->m_sObjectId);
      pArtifact = (Artifact*) m_pWonArtifacts->getNext(0);
    }
    pData->addLong(m_pWonAvatars->size);
    AvatarData * pAvatar = (AvatarData*) m_pWonAvatars->getFirst(0);
    while (pAvatar != NULL)
    {
      pData->addString(pAvatar->m_sEdition);
      pData->addString(pAvatar->m_sObjectId);
      pAvatar = (AvatarData*) m_pWonAvatars->getNext(0);
    }
  }
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void Player::deserialize(NetworkData * pData, bool bShort, LocalClient * pLocalClient)
{
  // General player data
  if (!bShort)
  {
    m_uPlayerId = (u8) pData->readLong();
    m_uClientId = (u8) pData->readLong();
    pData->readString(m_sProfileName);
    pData->readString(m_sBanner);
    m_Color.r = pData->readLong();
    m_Color.g = pData->readLong();
    m_Color.b = pData->readLong();
    m_Color.a = pData->readLong();
    m_bIsAI = (pData->readLong() == 1);
  }
  LuaTargetable::deserializeValues(pData);
  setState((PlayerState)pData->readLong());
//  m_uNbMagicCircles = (u8) pData->readLong();
  for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
  {
    m_MagicCirclePos[i].x = (int) pData->readLong();
    m_MagicCirclePos[i].y = (int) pData->readLong();
  }
  // retrieve mana values
  for (int i = 0; i < 4; i++)
    m_SpentMana.mana[i] = (u8) pData->readLong();
  if (!bShort)
  {
    m_uXPTownPoints = pData->readLong();
    m_uXPGross = pData->readLong();
    m_uXPNet = pData->readLong();
    m_iWonGold = pData->readLong();
    int size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      wchar_t sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      wchar_t sName[NAME_MAX_CHARS];
      pData->readString(sName);
      Spell * pSpell = pLocalClient->getDataFactory()->findSpell(sEdition, sName);
      assert(pSpell != NULL);
      m_pWonSpells->addLast(pSpell);
    }
    size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      wchar_t sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      wchar_t sName[NAME_MAX_CHARS];
      pData->readString(sName);
      Edition * pEdition = pLocalClient->getDataFactory()->findEdition(sEdition);
      assert(pEdition != NULL);
      Artifact * pArtifact = pEdition->findArtifact(sName);
      assert(pArtifact != NULL);
      m_pWonArtifacts->addLast(pArtifact);
    }
    size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      wchar_t sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      wchar_t sName[NAME_MAX_CHARS];
      pData->readString(sName);
      AvatarData * pAvatar = (AvatarData*) pLocalClient->getDataFactory()->getUnitData(sEdition, sName);
      assert(pAvatar != NULL);
      m_pWonAvatars->addLast(pAvatar);
    }
    swprintf_s(m_sIdentifiers, 16, L"player %d", (int) m_uPlayerId);
  }
}

// -----------------------------------------------------------------
// Name : serializeSpells
// -----------------------------------------------------------------
void Player::serializeSpells(NetworkData * pData)
{
  // Spells in hand
  pData->addLong(m_pHand->size);
  Spell * pSpell = (Spell*) m_pHand->getFirst(0);
  while (pSpell != NULL)
  {
    pSpell->serialize(pData);
    pSpell = (Spell*) m_pHand->getNext(0);
  }
  // Spells in deck
  pData->addLong(m_pDeck->size);
  pSpell = (Spell*) m_pDeck->getFirst(0);
  while (pSpell != NULL)
  {
    pSpell->serialize(pData);
    pSpell = (Spell*) m_pDeck->getNext(0);
  }
  // Spells in play
  pData->addLong(m_pActiveSpells->size);
  pSpell = (Spell*) m_pActiveSpells->getFirst(0);
  while (pSpell != NULL)
  {
    pSpell->serialize(pData);
    pSpell = (Spell*) m_pActiveSpells->getNext(0);
  }
  // Spells in discard
  pData->addLong(m_pDiscard->size);
  pSpell = (Spell*) m_pDiscard->getFirst(0);
  while (pSpell != NULL)
  {
    pSpell->serialize(pData);
    pSpell = (Spell*) m_pDiscard->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : deserializeSpells
// -----------------------------------------------------------------
void Player::deserializeSpells(NetworkData * pData, LocalClient * pLocalClient)
{
  // Spells in hand
  m_pHand->deleteAll();
  long count = pData->readLong();
  while (count > 0)
  {
    Spell * pSpell = Spell::deserialize(pData, pLocalClient->getDebug());
    m_pHand->addLast(pSpell);
    count--;
  }
  // Spells in deck
  m_pDeck->deleteAll();
  count = pData->readLong();
  while (count > 0)
  {
    Spell * pSpell = Spell::deserialize(pData, pLocalClient->getDebug());
    m_pDeck->addLast(pSpell);
    count--;
  }
  // Spells in play
  m_pActiveSpells->deleteAll();
  count = pData->readLong();
  while (count > 0)
  {
    Spell * pSpell = Spell::deserialize(pData, pLocalClient->getDebug());
    m_pActiveSpells->addLast(pSpell);
    count--;
  }
  // Spells in discard
  m_pDiscard->deleteAll();
  count = pData->readLong();
  while (count > 0)
  {
    Spell * pSpell = Spell::deserialize(pData, pLocalClient->getDebug());
    m_pDiscard->addLast(pSpell);
    count--;
  }
}

// -----------------------------------------------------------------
// Name : serializeUnits
// -----------------------------------------------------------------
void Player::serializeUnits(NetworkData * pData)
{
  // Different cases here if it's neutral player or not
  if (m_uPlayerId != 0)
  {
    // Avatar data
    m_pAvatarData->serialize(NetworkSerializer::getInstance(pData));
    m_pAvatar->serialize(pData);
    // Units
    if (m_pAvatar->getStatus() != US_Normal)
      pData->addLong(m_pUnits->size);
    else
      pData->addLong(m_pUnits->size - 1);
    Unit * pUnit = (Unit*) m_pUnits->getFirst(0);
    while (pUnit != NULL)
    {
      if (pUnit != m_pAvatar)
        pUnit->serialize(pData);
      pUnit = (Unit*) m_pUnits->getNext(0);
    }
  }
  else
  {
    pData->addLong(m_pUnits->size);
    Unit * pUnit = (Unit*) m_pUnits->getFirst(0);
    while (pUnit != NULL)
    {
      pUnit->serialize(pData);
      pUnit = (Unit*) m_pUnits->getNext(0);
    }
  }
}

// -----------------------------------------------------------------
// Name : deserializeUnits
// -----------------------------------------------------------------
void Player::deserializeUnits(NetworkData * pData, Map * pMap, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData)
{
  // Different cases here if it's neutral player or not
  if (m_uPlayerId != 0)
  {
    // Avatar
    m_pUnits->deleteAll();
    FREE(m_pAvatarData);
    m_pAvatarData = new AvatarData();
    m_pAvatarData->deserialize(NetworkSerializer::getInstance(pData), pLocalClient->getDebug());
    m_pAvatar = new Unit(CoordsMap(0, 0), pMap, m_pGlobalEffects);
    m_pAvatar->deserialize(pData, pLocalClient, relinkPtrData);
    m_pAvatar->setPlayerColor(m_Color);
    m_pAvatar->setNameDescriptionTexture(m_pAvatarData->m_sCustomName, m_pAvatarData->m_sCustomDescription, m_pAvatarData->m_sTextureFilename);
    if (m_pAvatar->getStatus() == US_Normal)
      m_pUnits->addLast(m_pAvatar);
    // Units
    long count = pData->readLong();
    while (count > 0)
    {
      Unit * pUnit = new Unit(CoordsMap(0, 0), pMap, m_pGlobalEffects);
      pUnit->deserialize(pData, pLocalClient, relinkPtrData);
      pUnit->setPlayerColor(m_Color);
      m_pUnits->addLast(pUnit);
      count--;
    }
  }
  else
  {
    m_pUnits->deleteAll();
    long count = pData->readLong();
    while (count > 0)
    {
      Unit * pUnit = new Unit(CoordsMap(0, 0), pMap, m_pGlobalEffects);
      pUnit->deserialize(pData, pLocalClient, relinkPtrData);
      pUnit->setPlayerColor(m_Color);
      m_pUnits->addLast(pUnit);
      count--;
    }
  }
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Player::init(DisplayEngine * pDisplay)
{
  QuadData quad(0.0f, 0.3f, 0.0f, 0.3f, m_sBanner, pDisplay);
  m_pBannerGeometry = new GeometryQuads(&quad, VB_Static);
  m_iBannerTex = m_pBannerGeometry->getTexture();
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void Player::update(double delta)
{
  Unit * unit = (Unit*) m_pUnits->getFirst(0);
  while (unit != NULL)
  {
    unit->update(delta);
    unit = (Unit*) m_pUnits->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : getAvatar
// -----------------------------------------------------------------
Unit * Player::getAvatar()
{
  return m_pAvatar;
}

// -----------------------------------------------------------------
// Name : setAvatar
// -----------------------------------------------------------------
void Player::setAvatar(AvatarData * pAvatarData, Unit * pUnit)
{
  m_pAvatarData = pAvatarData;
  m_pAvatar = pUnit;
}

// -----------------------------------------------------------------
// Name : getAvatarName
// -----------------------------------------------------------------
wchar_t * Player::getAvatarName()
{
  if (m_uPlayerId == 0)
    return m_sProfileName;
  else
    return m_pAvatarData->m_sCustomName;//m_pAvatar->getName();
}

// -----------------------------------------------------------------
// Name : getAvatarTexture
// -----------------------------------------------------------------
int Player::getAvatarTexture()
{
  if (m_uPlayerId == 0)
    return m_iBannerTex;
  else
    return m_pAvatar->getTexture();
}

// -----------------------------------------------------------------
// Name : getFirstUnplayedUnit
// -----------------------------------------------------------------
Unit * Player::getFirstUnplayedUnit()
{
  Unit * firstUnit = (Unit*) m_pUnits->getFirst(0);
  if (firstUnit == NULL)
    return NULL; // There is no unit at all

  Unit * firstGoodUnit = firstUnit;
  //while (firstGoodUnit->getValue(STRING_MOVES) <= 0)
  //{
  //  firstGoodUnit = (Unit*) m_pUnits->getNext(0);
  //  if (firstGoodUnit == firstUnit)  // Every units have finished moving
  //    return NULL;
  //}

  return firstGoodUnit;
}

// -----------------------------------------------------------------
// Name : getNextUnplayedUnit
// -----------------------------------------------------------------
Unit * Player::getNextUnplayedUnit()
{
  Unit * nextUnit = (Unit*) m_pUnits->getNext(0, true);
  if (nextUnit == NULL)
    return NULL; // There is no unit at all

  Unit * nextGoodUnit = nextUnit;
  //while (nextGoodUnit->getValue(STRING_MOVES) <= 0)
  //{
  //  nextGoodUnit = (Unit*) m_pUnits->getNext(0, true);
  //  if (nextGoodUnit == nextUnit)  // Every units have finished moving
  //    return NULL;
  //}

  return nextGoodUnit;
}

// -----------------------------------------------------------------
// Name : findUnit
// -----------------------------------------------------------------
Unit * Player::findUnit(u32 uUnit)
{
  Unit * pUnit = (Unit*) m_pUnits->getFirst(0);
  while (pUnit != NULL)
  {
    if (pUnit->getId() == uUnit)
      return pUnit;
    pUnit = (Unit*) m_pUnits->getNext(0);
  }
  pUnit = (Unit*) m_pDeadUnits->getFirst(0);
  while (pUnit != NULL)
  {
    if (pUnit->getId() == uUnit)
      return pUnit;
    pUnit = (Unit*) m_pDeadUnits->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : castSpell
// -----------------------------------------------------------------
void Player::castSpell(Spell * pSpell)
{
  pSpell->callLuaFunction(L"onCast", 0, L"");
}

// -----------------------------------------------------------------
// Name : findSpell
// -----------------------------------------------------------------
Spell * Player::findSpell(int _it, u32 id, ObjectList * pList)
{
  Spell * pSpell = (Spell*) pList->getFirst(_it);
  while (pSpell != NULL)
  {
    if (pSpell->getInstanceId() == id)
      return pSpell;
    pSpell = (Spell*) pList->getNext(_it);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : shuffleDeck
// -----------------------------------------------------------------
void Player::shuffleDeck()
{
  ObjectList * pNewDeck = new ObjectList(true);
  while (m_pDeck->size > 0)
  {
    u32 rnd = getRandom(m_pDeck->size);
    BaseObject * pObj = m_pDeck->goTo(0, rnd);
    assert(pObj != NULL);
    pNewDeck->addLast(pObj);
    m_pDeck->deleteCurrent(0, false, true);
  }
  delete m_pDeck;
  m_pDeck = pNewDeck;
}

// -----------------------------------------------------------------
// Name : getMana
// -----------------------------------------------------------------
Mana Player::getMana()
{
  Mana mana((u8) getValue(STRING_MANA_LIFE),
    (u8) getValue(STRING_MANA_LAW),
    (u8) getValue(STRING_MANA_DEATH),
    (u8) getValue(STRING_MANA_CHAOS));
  return mana;
}

// -----------------------------------------------------------------
// Name : getBaseMana
// -----------------------------------------------------------------
Mana Player::getBaseMana()
{
  Mana mana((u8) getValue(STRING_MANA_LIFE, true),
    (u8) getValue(STRING_MANA_LAW, true),
    (u8) getValue(STRING_MANA_DEATH, true),
    (u8) getValue(STRING_MANA_CHAOS, true));
  return mana;
}

// -----------------------------------------------------------------
// Name : getMana
// -----------------------------------------------------------------
u8 Player::getMana(u8 uColor)
{
  switch (uColor) {
  case MANA_LIFE:
    return (u8) getValue(STRING_MANA_LIFE);
  case MANA_LAW:
    return (u8) getValue(STRING_MANA_LAW);
  case MANA_DEATH:
    return (u8) getValue(STRING_MANA_DEATH);
  case MANA_CHAOS:
    return (u8) getValue(STRING_MANA_CHAOS);
  }
  return 0; // won't happen
}

// -----------------------------------------------------------------
// Name : getBaseMana
// -----------------------------------------------------------------
u8 Player::getBaseMana(u8 uColor)
{
  switch (uColor) {
  case MANA_LIFE:
    return (u8) getValue(STRING_MANA_LIFE, true);
  case MANA_LAW:
    return (u8) getValue(STRING_MANA_LAW, true);
  case MANA_DEATH:
    return (u8) getValue(STRING_MANA_DEATH, true);
  case MANA_CHAOS:
    return (u8) getValue(STRING_MANA_CHAOS, true);
  }
  return 0; // won't happen
}

// -----------------------------------------------------------------
// Name : setBaseMana
// -----------------------------------------------------------------
void Player::setBaseMana(Mana mana)
{
  setBaseValue(STRING_MANA_LIFE, mana[MANA_LIFE]);
  setBaseValue(STRING_MANA_LAW, mana[MANA_LAW]);
  setBaseValue(STRING_MANA_DEATH, mana[MANA_DEATH]);
  setBaseValue(STRING_MANA_CHAOS, mana[MANA_CHAOS]);
}

// -----------------------------------------------------------------
// Name : setBaseMana
// -----------------------------------------------------------------
void Player::setBaseMana(u8 uColor, u8 value)
{
  switch (uColor) {
  case MANA_LIFE:
    setBaseValue(STRING_MANA_LIFE, value);
    break;
  case MANA_LAW:
    setBaseValue(STRING_MANA_LAW, value);
    break;
  case MANA_DEATH:
    setBaseValue(STRING_MANA_DEATH, value);
    break;
  case MANA_CHAOS:
    setBaseValue(STRING_MANA_CHAOS, value);
    break;
  }
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
wchar_t * Player::getInfo(wchar_t * sBuf, int iSize)
{
  wchar_t sAvatar[64];
  wchar_t sAvailMana[64];
  wchar_t sDeuxPoints[4];
  i18n->getText1stUp(L"AVATAR", sAvatar, 64);
  i18n->getText1stUp(L"AVAILABLE_MANA", sAvailMana, 64);
  i18n->getText(L"2P", sDeuxPoints, 4);
  swprintf_s(sBuf, iSize, L"%s%s%s\n%s%s\n  ",
        sAvatar,
        sDeuxPoints,
        m_pAvatar->getName(),
        sAvailMana,
        sDeuxPoints
  );
  getInfo_AddValue(sBuf, iSize, STRING_MANA_LIFE, L"\n  ");
  getInfo_AddValue(sBuf, iSize, STRING_MANA_LAW, L"\n  ");
  getInfo_AddValue(sBuf, iSize, STRING_MANA_DEATH, L"\n  ");
  getInfo_AddValue(sBuf, iSize, STRING_MANA_CHAOS, L"");
  return sBuf;
}
