// -----------------------------------------------------------------
// PARAMETERS
// -----------------------------------------------------------------
#include "Parameters.h"
#include "../Debug/DebugManager.h"
#include "IniFile.h"
#include <locale.h>
#include <stdio.h>

// -----------------------------------------------------------------
// Name : Parameters
// -----------------------------------------------------------------
Parameters::Parameters()
{
  sLanguages = NULL;
  iLogLevel = 1;
  screenXSize = 800;
  screenYSize = 600;
  iNbLanguages = 0;
  iGameLogsLifetime = -1;
  iSoundVolume = 10;
  iMusicVolume = 10;
}

// -----------------------------------------------------------------
// Name : ~Parameters
// -----------------------------------------------------------------
Parameters::~Parameters()
{
  if (sLanguages != NULL)
  {
    for (int i = 0; i < iNbLanguages; i++)
      delete[] sLanguages[i];
    delete[] sLanguages;
  }
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void Parameters::Init(DebugManager * pDebug)
{
//  char sPath[MAX_PATH] = DATA_PATH;
//  strncat(sPath, "locale.ini", MAX_PATH);
//  FILE * pFile = NULL;
//  if (0 != fopen_s(&pFile, sPath, "r"))
//    pDebug->notifyErrorMessage("Error: can't open locale.ini file.");
//  fgets(sLocale, 32, pFile);
//  chop(sLocale);
//  fclose(pFile);
//  resetLocale(pDebug);
  loadParameters();
}

// -----------------------------------------------------------------
// Name : resetLocale
// -----------------------------------------------------------------
//void Parameters::resetLocale(DebugManager * pDebug)
//{
//  if (setlocale(LC_ALL, sLocale) == NULL)
//  {
//      char sError[512];
//      char swLocale[16];
//      strtow(swLocale, 16, sLocale);
//        snprintf(sError, 512, "Error: main locale not recognized: %s.", swLocale);
//      pDebug->notifyErrorMessage(sError);
//  }
//}

// -----------------------------------------------------------------
// Name : loadParameters
// -----------------------------------------------------------------
void Parameters::loadParameters()
{
  char sPath[MAX_PATH] = "config.ini";
  IniFile reader(sPath, 100);
  fullscreen = reader.findBoolValue("Fullscreen", false);
  wsafecpy(sGameModeString, 64, reader.findCharValue("GameModeString", "1280x1024:32"));
  winWidth = reader.findIntValue("WindowWidth", 1024);
  winHeight = reader.findIntValue("WindowHeight", 768);
  winXPos = reader.findIntValue("WindowX", 50);
  winYPos = reader.findIntValue("WindowY", 50);
  language = reader.findIntValue("Language", 0);
  fZoomStep = reader.findFloatValue("ZoomStep", 0.7f);
  iLogLevel = reader.findIntValue("LogLeve", 1);
  iGameLogsLifetime = reader.findIntValue("GameLogsLifetime", -1);
  iSoundVolume = reader.findIntValue("SoundVolume", 10);
  iMusicVolume = reader.findIntValue("MusicVolume", 10);
  char sKey[NAME_MAX_CHARS];
  iNbLanguages = 0;
  do
  {
    snprintf(sKey, NAME_MAX_CHARS, "Language%d", (int)iNbLanguages);
    iNbLanguages++;
  } while (reader.findValue(sKey) != NULL);
  iNbLanguages--;
  assert(iNbLanguages > 0);
  sLanguages = new char*[iNbLanguages];
  for (int i = 0; i < iNbLanguages; i++)
  {
    sLanguages[i] = new char[NAME_MAX_CHARS];
    snprintf(sKey, NAME_MAX_CHARS, "Language%d", i);
    wsafecpy(sLanguages[i], NAME_MAX_CHARS, reader.findCharValue(sKey));
  }
}

// -----------------------------------------------------------------
// Name : saveParameters
// -----------------------------------------------------------------
void Parameters::saveParameters()
{
  char sPath[MAX_PATH] = "config.ini";
  IniFile writer(sPath, 100);
  writer.setKeyAndBoolValue("Fullscreen", fullscreen);
  writer.setKeyAndCharValue("GameModeString", sGameModeString);
  writer.setKeyAndIntValue("WindowWidth", winWidth);
  writer.setKeyAndIntValue("WindowHeight", winHeight);
  writer.setKeyAndIntValue("WindowX", winXPos);
  writer.setKeyAndIntValue("WindowY", winYPos);
  writer.setKeyAndIntValue("Language", language);
  writer.setKeyAndFloatValue("ZoomStep", fZoomStep);
  writer.setKeyAndIntValue("LogLeve", iLogLevel);
  writer.setKeyAndIntValue("GameLogsLifetime", iGameLogsLifetime);
  writer.setKeyAndIntValue("SoundVolume", iSoundVolume);
  writer.setKeyAndIntValue("MusicVolume", iMusicVolume);
  writer.write(sPath);
}
