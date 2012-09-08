#ifndef _CLIENT_PARAMETERS_H
#define _CLIENT_PARAMETERS_H

#include "../utils.h"

class DebugManager;
class DisplayEngine;

class Parameters
{
public:
  // Constructor / destructor
  Parameters();
  ~Parameters();

  void Init(DebugManager * pDebug);
//  void resetLocale(DebugManager * pDebug);
  void loadParameters(DebugManager * pDebug);
  void saveParameters(DebugManager * pDebug);

  u16 screenXSize, screenYSize;
  u16 winXPos, winYPos;
  u16 winWidth, winHeight;
  bool fullscreen;
  float fZoomStep;
  int language;
  char sLocale[32];
  char sGameModeString[64];
  int iNbLanguages;
  char ** sLanguages;
  int iLogLevel;
  int iGameLogsLifetime;
  int iSoundVolume;
  int iMusicVolume;
};

#endif
