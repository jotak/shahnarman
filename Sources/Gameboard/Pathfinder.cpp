// -----------------------------------------------------------------
// PATHFINDER
// -----------------------------------------------------------------
#include "Pathfinder.h"
#include "MapObject.h"

// -----------------------------------------------------------------
// Name : Pathfinder
// -----------------------------------------------------------------
Pathfinder::Pathfinder(MapTile *** pMap, u16 mapXSize, u16 mapYSize)
{
  m_pMap = pMap;
  m_uMapXSize = mapXSize;
  m_uMapYSize = mapYSize;
}

// -----------------------------------------------------------------
// Name : ~Pathfinder
// -----------------------------------------------------------------
Pathfinder::~Pathfinder()
{
}

// -----------------------------------------------------------------
// Name : aStar
//  cf http://fr.wikipedia.org/wiki/Algorithme_A%2A
// -----------------------------------------------------------------
s16 Pathfinder::aStar(MapObject * mapObj, CoordsMap goal, CoordsMap ** solution)
{
  // if goal is an invalid target, then don't try to find path
  if (mapObj->getMoveCost(m_pMap[goal.x][goal.y]->m_uTerrainType) < 0)
    return -1;

  ASTAR_NODE * openList = new ASTAR_NODE[m_uMapXSize * m_uMapYSize];
  ASTAR_NODE * closedList = new ASTAR_NODE[m_uMapXSize * m_uMapYSize];
  int bestN;
  short bestNCost;

  // Initialiser la liste OPEN à vide
  int openListSize = 0;
  // Initialiser la liste CLOSED à vide
  int closedListSize = 0;

  // Ajouter START à la liste OPEN
  aStar_AddToList(openList, &openListSize, aStar_CreateNode(mapObj->getMapPos().x, mapObj->getMapPos().y, goal, mapObj));

  // Tant que la liste OPEN n'est pas vide
  while (openListSize > 0)
  {
    // Retirer le nœud n de la liste OPEN tel que f(n) soit le plus petit
    bestN = 0;
    bestNCost = openList[0].costFromStart + openList[0].costToGoal;
    for (int i = 1; i < openListSize; i++)
    {
      if (openList[i].costFromStart + openList[i].costToGoal < bestNCost)
//          || (openList[i].costFromStart + openList[i].costToGoal == bestNCost
//          && openList[i].xMap != openList[i].parentX
//          && openList[i].yMap != openList[i].parentY))
      {
        bestNCost = openList[i].costFromStart + openList[i].costToGoal;
        bestN = i;
      }
    }
    ASTAR_NODE curNode = aStar_RemoveFromList(openList, bestN, &openListSize);

    // Ajouter n a CLOSED
    aStar_AddToList(closedList, &closedListSize, curNode);

    // Si n est le GOAL retourner la solution ;
    if (curNode.xMap == goal.x && curNode.yMap == goal.y)
    {
      s16 nbMoves;
      if (solution != NULL) // If solution is null then we just want to know if it's true or false
        nbMoves = aStar_ExtractSolution(closedList, closedListSize, openList, openListSize, goal, solution);
      else
        nbMoves = aStar_GetSolutionSize(closedList, closedListSize, openList, openListSize);

      delete[] openList;
      delete[] closedList;
      return nbMoves; // YOU WIN
    }

    // Pour chaque successeur n´ du nœud n
    for (int x = -1; x <= 1; x++)
    {
      for (int y = -1; y <= 1; y++)
      {
        if ((x == 0 && y == 0) || curNode.xMap + x < 0 || curNode.yMap + y < 0 || curNode.xMap + x >= m_uMapXSize || curNode.yMap + y >= m_uMapYSize)
          continue;

        // Heuristique H = estimation du coût de n´ au GOAL
        // Stocker dans G_tmp g(n´), le coût g(n) + le coût pour aller de n à n´
        // Stocker dans F_tmp f(n´), g(n´) + H ; c'est l'heuristique
        ASTAR_NODE nPrime = aStar_CreateNode(curNode.xMap + x, curNode.yMap + y, goal, mapObj);
        if (nPrime.cost < 0)
          continue;
        nPrime.costFromStart = curNode.costFromStart + nPrime.cost;
        if (x != 0 && y != 0)
          nPrime.costFromStart += 0.001f;

        // Si n´ se trouve déjà dans OPEN avec un f(n´) meilleur on passe au point n´ suivant
        bool bContinue = false;
        int iOpen = -1;
        for (int i = 0; i < openListSize; i++)
        {
          if (openList[i].xMap == nPrime.xMap && openList[i].yMap == nPrime.yMap)
          {
            if (nPrime.costFromStart + nPrime.costToGoal < openList[i].costFromStart + openList[i].costToGoal)
              iOpen = i;
            else
              bContinue = true;
            break;
          }
        }
        if (bContinue)
          continue;

        // Si n´ se trouve déjà dans CLOSED avec un f(n´) meilleur on passe au point n´ suivant
        int iClosed = -1;
        for (int i = 0; i < closedListSize; i++)
        {
          if (closedList[i].xMap == nPrime.xMap && closedList[i].yMap == nPrime.yMap)
          {
            if (nPrime.costFromStart + nPrime.costToGoal < closedList[i].costFromStart + closedList[i].costToGoal)
              iClosed = i;
            else
              bContinue = true;
            break;
          }
        }
        if (bContinue)
          continue;

        // Mettre n dans parent(n')
        nPrime.parentX = curNode.xMap;
        nPrime.parentY = curNode.yMap;

        // Retirer n´ des deux listes OPEN et CLOSED
        if (iOpen != -1)
          aStar_RemoveFromList(openList, iOpen, &openListSize);
        if (iClosed != -1)
          aStar_RemoveFromList(closedList, iClosed, &closedListSize);

        // Ajouter n´ à la liste OPEN
        aStar_AddToList(openList, &openListSize, nPrime);
      }
    }
  }

  delete[] openList;
  delete[] closedList;
  return -1;
}

// -----------------------------------------------------------------
// Name : aStar
//  same as previous, but with a group of moving objects
// -----------------------------------------------------------------
s16 Pathfinder::aStar(ObjectList * pList, CoordsMap goal, CoordsMap ** solution)
{
  // if goal is an invalid target, then don't try to find path
  if (getHighestMoveCost(pList, goal.x, goal.y) < 0)
    return -1;

  ASTAR_NODE * openList = new ASTAR_NODE[m_uMapXSize * m_uMapYSize];
  ASTAR_NODE * closedList = new ASTAR_NODE[m_uMapXSize * m_uMapYSize];
  int bestN;
  short bestNCost;

  // Initialiser la liste OPEN à vide
  int openListSize = 0;
  // Initialiser la liste CLOSED à vide
  int closedListSize = 0;

  // Ajouter START à la liste OPEN
  CoordsMap mp = ((MapObject*)pList->getFirst(0))->getMapPos();
  aStar_AddToList(openList, &openListSize, aStar_CreateNode(mp.x, mp.y, goal, pList));

  // Tant que la liste OPEN n'est pas vide
  while (openListSize > 0)
  {
    // Retirer le nœud n de la liste OPEN tel que f(n) soit le plus petit
    bestN = 0;
    bestNCost = openList[0].costFromStart + openList[0].costToGoal;
    for (int i = 1; i < openListSize; i++)
    {
      if (openList[i].costFromStart + openList[i].costToGoal < bestNCost)
//          || (openList[i].costFromStart + openList[i].costToGoal == bestNCost
//          && openList[i].xMap != openList[i].parentX
//          && openList[i].yMap != openList[i].parentY))
      {
        bestNCost = openList[i].costFromStart + openList[i].costToGoal;
        bestN = i;
      }
    }
    ASTAR_NODE curNode = aStar_RemoveFromList(openList, bestN, &openListSize);

    // Ajouter n a CLOSED
    aStar_AddToList(closedList, &closedListSize, curNode);
    if (closedListSize >= m_uMapXSize * m_uMapYSize)
      return -97; // closed list overflow

    // Si n est le GOAL retourner la solution ;
    if (curNode.xMap == goal.x && curNode.yMap == goal.y)
    {
      s16 nbMoves = 1;
      if (solution != NULL) // If solution is null then we just want to know if it's true or false
        nbMoves = aStar_ExtractSolution(closedList, closedListSize, openList, openListSize, goal, solution);
      delete[] openList;
      delete[] closedList;
      return nbMoves; // YOU WIN
    }

    // Pour chaque successeur n´ du nœud n
    for (int x = -1; x <= 1; x++)
    {
      for (int y = -1; y <= 1; y++)
      {
        if ((x == 0 && y == 0) || curNode.xMap + x < 0 || curNode.yMap + y < 0 || curNode.xMap + x >= m_uMapXSize || curNode.yMap + y >= m_uMapYSize)
          continue;

        // Heuristique H = estimation du coût de n´ au GOAL
        // Stocker dans G_tmp g(n´), le coût g(n) + le coût pour aller de n à n´
        // Stocker dans F_tmp f(n´), g(n´) + H ; c'est l'heuristique
        ASTAR_NODE nPrime = aStar_CreateNode(curNode.xMap + x, curNode.yMap + y, goal, pList);
        if (nPrime.cost < 0)
          continue;
        nPrime.costFromStart = curNode.costFromStart + nPrime.cost;
        if (x != 0 && y != 0)
          nPrime.costFromStart += 0.001f;

        // Si n´ se trouve déjà dans OPEN avec un f(n´) meilleur on passe au point n´ suivant
        bool bContinue = false;
        int iOpen = -1;
        for (int i = 0; i < openListSize; i++)
        {
          if (openList[i].xMap == nPrime.xMap && openList[i].yMap == nPrime.yMap)
          {
            if (nPrime.costFromStart + nPrime.costToGoal < openList[i].costFromStart + openList[i].costToGoal)
              iOpen = i;
            else
              bContinue = true;
            break;
          }
        }
        if (bContinue)
          continue;

        // Si n´ se trouve déjà dans CLOSED avec un f(n´) meilleur on passe au point n´ suivant
        int iClosed = -1;
        for (int i = 0; i < closedListSize; i++)
        {
          if (closedList[i].xMap == nPrime.xMap && closedList[i].yMap == nPrime.yMap)
          {
            if (nPrime.costFromStart + nPrime.costToGoal < closedList[i].costFromStart + closedList[i].costToGoal)
              iClosed = i;
            else
              bContinue = true;
            break;
          }
        }
        if (bContinue)
          continue;

        // Mettre n dans parent(n')
        nPrime.parentX = curNode.xMap;
        nPrime.parentY = curNode.yMap;

        // Retirer n´ des deux listes OPEN et CLOSED
        if (iOpen != -1)
          aStar_RemoveFromList(openList, iOpen, &openListSize);
        if (iClosed != -1)
          aStar_RemoveFromList(closedList, iClosed, &closedListSize);

        // Ajouter n´ à la liste OPEN
        aStar_AddToList(openList, &openListSize, nPrime);
        if (openListSize >= m_uMapXSize * m_uMapYSize)
          return -98; // open list overflow
      }
    }
  }

  delete[] openList;
  delete[] closedList;
  return -1;
}

// -----------------------------------------------------------------
// Name : aStar_CreateNode
// -----------------------------------------------------------------
ASTAR_NODE Pathfinder::aStar_CreateNode(short xMap, short yMap, CoordsMap goal, MapObject * mapObj)
{
  ASTAR_NODE node;
  node.xMap = xMap;
  node.yMap = yMap;
  node.cost = mapObj->getMoveCost(m_pMap[xMap][yMap]->m_uTerrainType);
  node.costFromStart = 0;
  node.costToGoal = max(abs(goal.x - xMap), abs(goal.y - yMap));
  node.parentX = node.parentY = -1;

  return node;
}

// -----------------------------------------------------------------
// Name : aStar_CreateNode
// -----------------------------------------------------------------
ASTAR_NODE Pathfinder::aStar_CreateNode(short xMap, short yMap, CoordsMap goal, ObjectList * pList)
{
  ASTAR_NODE node;
  node.xMap = xMap;
  node.yMap = yMap;
  node.cost = getHighestMoveCost(pList, xMap, yMap);
  node.costFromStart = 0;
  node.costToGoal = max(abs(goal.x - xMap), abs(goal.y - yMap));
  node.parentX = node.parentY = -1;

  return node;
}

// -----------------------------------------------------------------
// Name : aStar_AddToList
// -----------------------------------------------------------------
void Pathfinder::aStar_AddToList(ASTAR_NODE * list, int * listSize, ASTAR_NODE node)
{
  list[*listSize] = node;
  (*listSize)++;
}

// -----------------------------------------------------------------
// Name : aStar_RemoveFromList
// -----------------------------------------------------------------
ASTAR_NODE Pathfinder::aStar_RemoveFromList(ASTAR_NODE * list, int position, int * listSize)
{
  ASTAR_NODE node = list[position];
  while (++position < *listSize)
    list[position - 1] = list[position];
  (*listSize)--;

  return node;
}

// -----------------------------------------------------------------
// Name : aStar_GetSolutionSize
// -----------------------------------------------------------------
s16 Pathfinder::aStar_GetSolutionSize(ASTAR_NODE * closedList, int closedListSize, ASTAR_NODE * openList, int openListSize)
{
  int iSol = 1;
  int iClosed = closedListSize - 1;
  CoordsMap current;

  current.x = closedList[iClosed].parentX;
  current.y = closedList[iClosed].parentY;

  // BUG sometimes infinite loop here
  // Put a spy to track it
  int spy = 0;
  while (current.x != -1)
  {
    spy++;
    if (spy > 1000)
      return -99;
    bool bFound = false;
    for (iClosed = 0; iClosed < closedListSize; iClosed++)
    {
      if (closedList[iClosed].xMap == current.x && closedList[iClosed].yMap == current.y)
      {
        iSol++;
        current.x = closedList[iClosed].parentX;
        current.y = closedList[iClosed].parentY;
        bFound = true;
        break;
      }
    }
    if (!bFound) {
      // Not found in closed list => search in open list
      for (iClosed = 0; iClosed < openListSize; iClosed++)
      {
        if (openList[iClosed].xMap == current.x && openList[iClosed].yMap == current.y)
        {
          iSol++;
          current.x = openList[iClosed].parentX;
          current.y = openList[iClosed].parentY;
          bFound = true;
          break;
        }
      }
    }
    if (!bFound)
      return -96;
  }
  return iSol - 1;
}

// -----------------------------------------------------------------
// Name : aStar_ExtractSolution
// -----------------------------------------------------------------
s16 Pathfinder::aStar_ExtractSolution(ASTAR_NODE * closedList, int closedListSize, ASTAR_NODE * openList, int openListSize, CoordsMap goal, CoordsMap ** solution)
{
  int iSol = 0;
  (*solution)[iSol++] = goal;
  int iClosed = closedListSize - 1;

  (*solution)[iSol].x = closedList[iClosed].parentX;
  (*solution)[iSol].y = closedList[iClosed].parentY;

  // BUG sometimes infinite loop here
  // Put a spy to track it
  int spy = 0;
  while ((*solution)[iSol].x != -1)
  {
    spy++;
    if (spy > 1000)
      return -99;
    bool bFound = false;
    for (iClosed = 0; iClosed < closedListSize; iClosed++)
    {
      if (closedList[iClosed].xMap == (*solution)[iSol].x && closedList[iClosed].yMap == (*solution)[iSol].y)
      {
        iSol++;
        (*solution)[iSol].x = closedList[iClosed].parentX;
        (*solution)[iSol].y = closedList[iClosed].parentY;
        bFound = true;
        break;
      }
    }
    if (!bFound) {
      // Not found in closed list => search in open list
      for (iClosed = 0; iClosed < openListSize; iClosed++)
      {
        if (openList[iClosed].xMap == (*solution)[iSol].x && openList[iClosed].yMap == (*solution)[iSol].y)
        {
          iSol++;
          (*solution)[iSol].x = openList[iClosed].parentX;
          (*solution)[iSol].y = openList[iClosed].parentY;
          bFound = true;
          break;
        }
      }
    }
    if (!bFound)
      return -96;
  }

  int iRev = 0;
  iSol--;
  CoordsMap tmp;
  while (iRev < iSol - iRev)
  {
    tmp = (*solution)[iRev];
    (*solution)[iRev] = (*solution)[iSol-iRev];
    (*solution)[iSol-iRev] = tmp;
    iRev++;
  }

  return iSol;
}

// -----------------------------------------------------------------
// Name : getHighestMoveCost
// -----------------------------------------------------------------
int Pathfinder::getHighestMoveCost(ObjectList * pList, int x, int y)
{
  int highest = -1;
  MapObject * mapObj = (MapObject*) pList->getFirst(0);
  while (mapObj != NULL)
  {
    int cost = mapObj->getMoveCost(m_pMap[x][y]->m_uTerrainType);
    if (cost == -1)
      return -1;
    else if (cost > highest)
      highest = cost;
    mapObj = (MapObject*) pList->getNext(0);
  }
  return highest;
}
