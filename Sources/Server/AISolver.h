#ifndef _AI_SOLVER_H
#define _AI_SOLVER_H

#include "../utils.h"

class Unit;
class MapTile;
class Server;

class AISolver
{
public:
  // Constructor / destructor
  AISolver(Server * pServer);
  ~AISolver();

  void resolveNeutralAI(Unit * pUnit);

  // Simulation functions
  int getOpponentInterest(Unit * pUnit, Unit * pOpponent, bool * bRange = NULL);

protected:
  bool isInterestedByTile(Unit * pUnit, MapTile * pTile, Unit ** pOpponent);

  Server * m_pServer;
};

#endif
