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
  char sPath[MAX_PATH] = DATA_PATH;
  strncat(sPath, "locale.ini", MAX_PATH);
  FILE * pFile = NULL;
  if (0 != fopen_s(&pFile, sPath, "r"))
    pDebug->notifyErrorMessage(L"Error: can't open locale.ini file.");
  fgets(sLocale, 32, pFile);
  chop(sLocale);
  fclose(pFile);
  resetLocale(pDebug);
  loadParameters();
}

// -----------------------------------------------------------------
// Name : resetLocale
// -----------------------------------------------------------------
void Parameters::resetLocale(DebugManager * pDebug)
{
  if (setlocale(LC_ALL, sLocale) == NULL)
  {
      wchar_t sError[512];
      wchar_t swLocale[16];
      strtow(swLocale, 16, sLocale);
        swprintf(sError, 512, L"Error: main locale not recognized: %s.", swLocale);
      pDebug->notifyErrorMessage(sError);
  }
}

// -----------------------------------------------------------------
// Name : loadParameters
// -----------------------------------------------------------------
void Parameters::loadParameters()
{
  wchar_t sPath[MAX_PATH] = L"config.ini";
  IniFile reader(sPath, 100);
  fullscreen = reader.findBoolValue(L"Fullscreen", false);
  wsafecpy(sGameModeString, 64, reader.findCharValue(L"GameModeString", L"1280x1024:32"));
  winWidth = reader.findIntValue(L"WindowWidth", 1024);
  winHeight = reader.findIntValue(L"WindowHeight", 768);
  winXPos = reader.findIntValue(L"WindowX", 50);
  winYPos = reader.findIntValue(L"WindowY", 50);
  language = reader.findIntValue(L"Language", 0);
  fZoomStep = reader.findFloatValue(L"ZoomStep", 0.7f);
  iLogLevel = reader.findIntValue(L"LogLevel", 1);
  iGameLogsLifetime = reader.findIntValue(L"GameLogsLifetime", -1);
  iSoundVolume = reader.findIntValue(L"SoundVolume", 10);
  iMusicVolume = reader.findIntValue(L"MusicVolume", 10);
  wchar_t sKey[NAME_MAX_CHARS];
  iNbLanguages = 0;
  do
  {
    swprintf(sKey, NAME_MAX_CHARS, L"Language%d", (int)iNbLanguages);
    iNbLanguages++;
  } while (reader.findValue(sKey) != NULL);
  iNbLanguages--;
  assert(iNbLanguages > 0);
  sLanguages = new wchar_t*[iNbLanguages];
  for (int i = 0; i < iNbLanguages; i++)
  {
    sLanguages[i] = new wchar_t[NAME_MAX_CHARS];
    swprintf(sKey, NAME_MAX_CHARS, L"Language%d", i);
    wsafecpy(sLanguages[i], NAME_MAX_CHARS, reader.findCharValue(sKey));
  }
}

// -----------------------------------------------------------------
// Name : saveParameters
// -----------------------------------------------------------------
void Parameters::saveParameters()
{
  wchar_t sPath[MAX_PATH] = L"config.ini";
  IniFile writer(sPath, 100);
  writer.setKeyAndBoolValue(L"Fullscreen", fullscreen);
  writer.setKeyAndCharValue(L"GameModeString", sGameModeString);
  writer.setKeyAndIntValue(L"WindowWidth", winWidth);
  writer.setKeyAndIntValue(L"WindowHeight", winHeight);
  writer.setKeyAndIntValue(L"WindowX", winXPos);
  writer.setKeyAndIntValue(L"WindowY", winYPos);
  writer.setKeyAndIntValue(L"Language", language);
  writer.setKeyAndFloatValue(L"ZoomStep", fZoomStep);
  writer.setKeyAndIntValue(L"LogLevel", iLogLevel);
  writer.setKeyAndIntValue(L"GameLogsLifetime", iGameLogsLifetime);
  writer.setKeyAndIntValue(L"SoundVolume", iSoundVolume);
  writer.setKeyAndIntValue(L"MusicVolume", iMusicVolume);
  writer.write(sPath);
}
