#ifndef _PATHFINDER_H
#define _PATHFINDER_H

#include "MapTile.h"

typedef struct struct_astarnode
{
  short xMap;
  short yMap;
  short cost;
  float costFromStart;
  short costToGoal;
  short parentX;
  short parentY;
} ASTAR_NODE;

class MapObject;

class Pathfinder
{
public:
  // Constructor / destructor
  Pathfinder(MapTile *** pMap, u16 mapXSize, u16 mapYSize);
  ~Pathfinder();

  s16 aStar(MapObject * mapObj, CoordsMap goal, CoordsMap ** solution);
  s16 aStar(ObjectList * pList, CoordsMap goal, CoordsMap ** solution);

private:
  // Pathfinder functions
  ASTAR_NODE aStar_CreateNode(short xMap, short yMap, CoordsMap goal, MapObject * mapObj);
  ASTAR_NODE aStar_CreateNode(short xMap, short yMap, CoordsMap goal, ObjectList * pList);
  void aStar_AddToList(ASTAR_NODE * list, int * listSize, ASTAR_NODE node);
  ASTAR_NODE aStar_RemoveFromList(ASTAR_NODE * list, int position, int * listSize);
  s16 aStar_ExtractSolution(ASTAR_NODE * closedList, int closedListSize, ASTAR_NODE * openList, int openListSize, CoordsMap goal, CoordsMap ** solution);
  s16 aStar_GetSolutionSize(ASTAR_NODE * closedList, int closedListSize, ASTAR_NODE * openList, int openListSize);
  int getHighestMoveCost(ObjectList * pList, int x, int y);

  // Map data
  MapTile *** m_pMap;
  u16 m_uMapXSize;
  u16 m_uMapYSize;
};

#endif
