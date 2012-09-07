#include "AILuaSolver.h"
#include "Server.h"
#include "AISolver.h"
#include "TurnSolver.h"
#include "SpellsSolver.h"
#include "../Data/DataFactory.h"
#include "../Players/Player.h"
#include "../Players/AISpell.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/MapTile.h"
#include "../Gameboard/SpecialTile.h"

// -----------------------------------------------------------------
// Name : AILuaSolver
//  Constructor
// -----------------------------------------------------------------
AILuaSolver::AILuaSolver(Server * pServer)
{
  m_fCurrentInterest = 0;
  m_pCurrentPlayer = NULL;
  m_pCurrentLua = NULL;
  m_pServer = pServer;
  m_pEvaluationTargets = new ObjectList(true);
  m_pTryTargets = new ObjectList(true);
}

// -----------------------------------------------------------------
// Name : ~AILuaSolver
//  Destructor
// -----------------------------------------------------------------
AILuaSolver::~AILuaSolver()
{
  delete m_pEvaluationTargets;
  delete m_pTryTargets;
}

// -----------------------------------------------------------------
// Name : resolveAISpells
// -----------------------------------------------------------------
void AILuaSolver::resolveAISpells(Player * pPlayer)
{
  // Analyser chaque LUA, stocker les résultats y compris des LUA trop chers en mana, et décider
  //  (à mélanger avec la méthode d'évaluation des objectifs globaux de l'idée n°1 ; mais ici on évite d'avoir à cloner le monde X fois pour évaluer les actions "unitaires")
  ObjectList * pAISpellsList = new ObjectList(true);
  Spell * pSpell = (Spell*) pPlayer->m_pHand->getFirst(0);
  while (pSpell != NULL) {
    AISpell * pClone = evaluateSpell(pPlayer, pSpell);
    pAISpellsList->addLast(pClone);
    pSpell = (Spell*) pPlayer->m_pHand->getNext(0);
  }

  // Loop through AI spells to determine which one(s) to cast
  ObjectList * pAISpellsToCast = new ObjectList(false);
  AISpell * pSpellToCast = NULL;
  Mana spentMana = pPlayer->m_SpentMana;
  do {
    Mana totalMana = pPlayer->getMana();
    Mana availableMana = totalMana - spentMana;
    double bestRatio = 0;
    AISpell * pAISpell = (AISpell*) pAISpellsList->getFirst(0);
    while (pAISpell != NULL) {
      double ratio = (double) pAISpell->getInterest() / (0.0001f + (double) pAISpell->getCost().amount());
      bool bCanBeCastNow = pAISpell->getCost() <= availableMana;
      bool bCanBeCastLater = pAISpell->getCost() <= totalMana;
      if (!bCanBeCastLater)
        ratio = 0;
      else if (!bCanBeCastNow)
        ratio /= 2;
      if (ratio > bestRatio) {
        bestRatio = ratio;
        pSpellToCast = pAISpell;
      }
      pAISpell = (AISpell*) pAISpellsList->getNext(0);
    }
    if (pSpellToCast != NULL) {
      if (pSpellToCast->getCost() <= availableMana) {
        // Add spell to cast
        pAISpellsToCast->addLast(pSpellToCast);
        spentMana += pSpellToCast->getCost();
      }
      else {
        // Spare mana for later
        pSpellToCast = NULL;
      }
    }
  } while (pSpellToCast != NULL);

  if (pAISpellsToCast->size > 0) {
    // Construct cast spell message
    NetworkData spellmsg(NETWORKMSG_SEND_CAST_SPELLS_DATA);
    spellmsg.addLong((long)pPlayer->m_uPlayerId);
    spellmsg.addLong((long)pAISpellsToCast->size);
    AISpell * pAISpell = (AISpell*) pAISpellsToCast->getFirst(0);
    while (pAISpell != NULL) {
      spellmsg.addLong((long) pAISpell->getInstanceId());
      spellmsg.addString(pAISpell->getBestInterestParameters());
      pAISpell = (AISpell*) pAISpellsToCast->getNext(0);
    }
    spellmsg.readLong();  // remove message type
    m_pServer->getSolver()->getSpellsSolver()->receiveSpells(&spellmsg);
  }
  //Server * pClonedServer = m_pServer->cloneForAI();
  //pPlayer->addWorldStateValue(pClonedServer, computeStateValue(pClonedServer)); // this function must also update objectives (not estimate them, just update (units may have died or benn created, etc.))
  //if (!pPlayer->hasGlobalObjective())
  //  pPlayer->chooseObjective();
  // Iterate toward objective

  // Idée générale n°1:
  // Prévoir une méthode de calcul de la valeur globale d'un état. Exemple : en ce moment (T0), la valeur globale de cet état est égal à la somme au carré des intérêts de toutes les actions faisables.
  // On enregistre cette valeur accompagnée d'une description + ou - fidèle de l'état (je me trouve dans une ville, je me trouve à portée de l'ennemi, etc.)
  // Je me déplace vers un objectif + ou - aléatoire
  // Après ce déplacement, je calcule la valeur de ce nouvel état.
  // Quelques itérations afin d'avoir une base de connaissance suffisante
  // Puis un cocktail heuristique : une convergence globale vers le meilleur état, agrémenté de quelques "mutations" vers des états moins bons pour les réévaluer.
  // En outre, la qualité d'un état donne une idée de la qualité des autres états du même type.
  // Les descriptions d'état sont :
  //    - Dans une ville [id ville]
  //    - Sur un ennemi [id player + id unit]
  //    - A portée d'un ennemi [id player + id unit]
  //    - Sur une case quelconque [x y]
  // Avant d'aller sur une ville / près d'un ennemi => évaluer la dangerosité (évaluer rapport bénéfice/risque)
  // Pour éviter trop de calculs inutiles, dans les fichiers LUA, mettre un paramètre "ai self interest" => "positive", "negative", "mixed"
  // Pour chaque sort et chaque skill activable, l'IA va tester, en début de partie puis de façon assez régulière, son effet indépendemment de l'état actuel sur les cibles possibles
  //    même hors de portée afin d'identifier les états à rechercher.
}

// -----------------------------------------------------------------
// Name : evaluateSpell
//  This function initializes the environment to start evaluating a spell. It is called through AI resolution for each spell of the current player.
//  (mainly set m_pEvaluationTargets to true)
//  Then it "casts" the spell (in simulation mode)
//  Eventually the spell may need targets ; so it calls "getTargetsForSpell" which will recursively construct
//    the whole tree of possible targets (list of lists)
//  For each set of targets the spell is "resolved" (still in simulation mode), which allows to get an interest value
//    for the spell associated to a set of targets.
//  Only the best interest targets set is kept per spell.
// -----------------------------------------------------------------
AISpell * AILuaSolver::evaluateSpell(Player * pCaster, Spell * pRealSpell)
{
  // Clone spell to avoid any modification in real LUA spell due to evaluation
  AISpell * pClone = new AISpell(pRealSpell, m_pServer->getDebug());
  // Activate evaluation mode
  extern bool g_bLuaEvaluationMode;
  g_bLuaEvaluationMode = true;
  m_pEvaluationTargets->deleteAll();
  m_pTryTargets->deleteAll();
  // Cast spell (in evaluation mode)
  extern u32 g_uLuaCurrentObjectType;
  g_uLuaCurrentObjectType = LUAOBJECT_SPELL;
  pClone->resetResolveParameters();
  pClone->setInterest(0);
  pClone->setExtraMana(Mana());
  pClone->callLuaFunction(L"onCast", 0, L"");
  getTargetsForSpell(pCaster, pClone, NULL, NULL);
  MetaObjectList * pList = (MetaObjectList*) m_pTryTargets->getFirst(0);
  while (pList != NULL) {
    pClone->resetResolveParameters();
    TargetData * pTarget = (TargetData*) pList->getFirst(0);
    while (pTarget != NULL) {
      pClone->addResolveParameters(pTarget->params);
      pTarget = (TargetData*) pList->getNext(0);
    }
    m_pCurrentPlayer = pCaster;
    m_pCurrentLua = pClone;
    m_fCurrentInterest = 0;
    pClone->callLuaFunction(L"onResolve", 0, L"sil", pClone->getResolveParameters(), (int) pCaster->m_uPlayerId, (long) pClone->getInstanceId());
    // TODO : evaluate other function like "getMod..."
    if (m_fCurrentInterest > pClone->getInterest()) {
      pClone->setInterest(m_fCurrentInterest);
      pClone->setBestInterestParameters(pClone->getResolveParameters());
    }
    pList = (MetaObjectList*) m_pTryTargets->getNext(0);
  }
  // Release data
  g_bLuaEvaluationMode = false;
  return pClone;
}

// -----------------------------------------------------------------
// Name : findTargetForLua
//  During the LUA evaluation process (like to evaluate interest of a spell), this function is called by "getTargetsForSpell"
//  (See getTargetsForSpell comments for global algorithm)
//  The "pInfoPtr" argument contains the demanded target type and current iterators to loop through game data
//  If it's NULL, the info is retrieved from member "m_pEvaluationTargets" (the spell may need several targets, so it's a list)
//  The main behavior of this function is to retrieve the current loop process using iterators (or initiate a new loop if necessary)
//  and then return one target that fits type & constraints
// -----------------------------------------------------------------
AILuaSolver::TargetData * AILuaSolver::findTargetForLua(Player * pCaster, EvaluationTargetInfo ** pInfoPtr)
{
  // pPreviousTarget is null when we enter this function for a new iteration process... so start iterate and store iteration data
  if (*pInfoPtr == NULL) {
    *pInfoPtr = (EvaluationTargetInfo*) m_pEvaluationTargets->getFirst(0);
    if (*pInfoPtr != NULL) {
      m_pEvaluationTargets->deleteCurrent(0, false, true);  // keep in mem
    }
  }
  if (*pInfoPtr == NULL)  // No target
    return NULL;
  EvaluationTargetInfo * pInfo = *pInfoPtr;
  TargetData * pTarget = new TargetData();

  switch (pInfo->m_uTargetType)
  {
  case SELECT_TYPE_UNIT:
    {
      // Get or retrieve current player
      Player * pPlayer = NULL;
      if (pInfo->it1 == 0) {
        pInfo->it1 = m_pServer->getSolver()->getPlayersList()->getIterator();
        pPlayer = m_pServer->getSolver()->getNeutralPlayer();
        pInfo->bCurrentIsNeutral = true;
      }
      else if (pInfo->bCurrentIsNeutral)
        pPlayer = m_pServer->getSolver()->getNeutralPlayer();
      else
        pPlayer = (Player*) m_pServer->getSolver()->getPlayersList()->getCurrent(pInfo->it1);

      // (Re-)start loop in players
      while (pPlayer != NULL) {
        if ((!(pInfo->m_uConstraints & SELECT_CONSTRAINT_OWNED) || (pPlayer == pCaster))
          && (!(pInfo->m_uConstraints & SELECT_CONSTRAINT_OPPONENT) || (pPlayer != pCaster)))
        {
          // Retrieve current unit or set first
          Unit * pUnit = NULL;
          if (pInfo->it2 == 0) {
            pInfo->it2 = pPlayer->m_pUnits->getIterator();
            pUnit = (Unit*) pPlayer->m_pUnits->getFirst(pInfo->it2);
          }
          else {
            pUnit = (Unit*) pPlayer->m_pUnits->getNext(pInfo->it2);
          }
          while (pUnit != NULL) {
            if (!(pInfo->m_uConstraints & SELECT_CONSTRAINT_NOT_AVATAR) || !(pPlayer->getAvatar() == pUnit)) {
              // Ok, we found a unit that fits the constraints ; use it now
              pTarget->m_pTarget = pUnit;
              swprintf(pTarget->params, 64, L"%d %ld", (int) pUnit->getOwner(), (long) pUnit->getId());
              return pTarget;
            }
            pUnit = (Unit*) pPlayer->m_pUnits->getNext(pInfo->it2);
          }
          pPlayer->m_pUnits->releaseIterator(pInfo->it2);
          pInfo->it2 = 0;
        }
        if (pInfo->bCurrentIsNeutral) {
          pPlayer = (Player*) m_pServer->getSolver()->getPlayersList()->getFirst(pInfo->it1);
          pInfo->bCurrentIsNeutral = false;
        }
        else
          pPlayer = (Player*) m_pServer->getSolver()->getPlayersList()->getNext(pInfo->it1);
      }
      m_pServer->getSolver()->getPlayersList()->releaseIterator(pInfo->it1);
      pInfo->it1 = 0;
    }
  }
  pTarget->m_pTarget = NULL;  // Set m_pTarget to NULL to say that nothing more fits this spell
  return pTarget;
}

// -----------------------------------------------------------------
// Name : getTargetsForSpell
//  During the LUA evaluation process (like to evaluate interest of a spell), this function is called once per spell with parameters
//  EvaluationTargetInfo * pInfo and MetaObjectList * pList set to NULL.
//  The aim of this function is to fill "pList", which contains a list of possible sets of targets for the spell.
//  Note that some spells can have multiple targets, so pList is a List of Lists.
//  The function calls "findTargetForLua" to find 1 allowed target for the spell
//  Then it calls itself recursively in 2 cases:
//  1/ When current spell needs extra targets
//  2/ Else, when a potential target was found, to try another one
// -----------------------------------------------------------------
void AILuaSolver::getTargetsForSpell(Player * pCaster, Spell * pSpell, EvaluationTargetInfo * pInfo, MetaObjectList * pList)
{
  // pList is NULL when the function is called the first time, and for every new solution (solution = set of parameters)
  // pList will contain the list of parameters for 1 solution
  if (pList == NULL) {
    pList = new MetaObjectList(true);
    m_pTryTargets->addLast(pList);
  }
  TargetData * pTargetData = findTargetForLua(pCaster, &pInfo);
  if (pTargetData == NULL) {
    // no target demanded for this spell
    return;
  }
  else {
    if (pTargetData->m_pTarget == NULL) {
      // no valid target found
      delete pTargetData;
      m_pTryTargets->deleteObject(pList, true);
      return;
    }
    else {
      // We have a target.
      pList->addLast(pTargetData);
      // Does the spell ask for a second, third etc. target???
      extern wchar_t g_sLuaSelectCallback[128];
      if (wcscmp(g_sLuaSelectCallback, L"") != 0) {
        pSpell->callLuaFunction(g_sLuaSelectCallback, 0, L"");
        getTargetsForSpell(pCaster, pSpell, NULL, pList);
      }
      getTargetsForSpell(pCaster, pSpell, pInfo, NULL);
    }
  }
}

// -----------------------------------------------------------------
// Name : addCurrentSpellInterestForDamages
// -----------------------------------------------------------------
void AILuaSolver::addCurrentSpellInterestForDamages(Player * pPlayer, Unit * pUnit, u8 damages)
{
  AISolver * pAISolver = m_pServer->getSolver()->getAISolver();
  // Interest is negative if damages an friend; positive on a foe, but divided by 2 on a neutral
  float fSign = (pPlayer == m_pCurrentPlayer) ? -1.0f : (pPlayer->m_uPlayerId == 0 ? 0.5f : 1.0f);
  float fRoughInterest = pAISolver->evaluateUnit(pUnit);
  long iLife = pUnit->getValue(STRING_LIFE);
  float fInterest = fSign * fRoughInterest * (iLife <= damages ? 1.0f : 0.33f);
  fInterest += fSign * damages * AI_INTEREST_LIFE;
  if (pPlayer->getAvatar() == pUnit && iLife <= damages) {
    fInterest *= 4;
  }
  else if (fSign > 0.0f) {
    // Modulate by distance: the nearest it is from an allie, the best it is
    // Loop through player's unit
    int _it = m_pCurrentPlayer->m_pUnits->getIterator();
    int minDistance = -1;
    Unit * pFriend = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(_it);
    while (pFriend != NULL) {
      s16 distance = m_pServer->getMap()->findPath(pUnit, pFriend->getMapPos(), NULL);
      if (minDistance < 0 || (distance < minDistance && distance > 0)) {
        distance = minDistance;
      }
      pFriend = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(_it);
    }
    m_pCurrentPlayer->m_pUnits->releaseIterator(_it);
    if (minDistance < 0 || minDistance >= 10) {
      // Very far => less interesting
      fInterest *= 0.33f;
    }
    else if (minDistance >= 5) {
      // Still a bit far
      fInterest *= 0.5f;
    }
    else if (minDistance >= 2) {
      // Not so far
      fInterest *= 0.75f;
    }
    // else: Too close! keep full interest
  }
  pAISolver->addInterestForCurrentSpell(fInterest);
}
