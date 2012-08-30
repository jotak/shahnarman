#ifdef WIN32
#include "../utils.h"
#include "../SystemHeaders.h"
#include <wchar.h>
#include <io.h>

// -----------------------------------------------------------------
// Name : getEditions
// -----------------------------------------------------------------
int getEditions(wchar_t ** sEditionsList, int iListSize, int iEditionNameSize)
{
  _wfinddata_t finddata;
  int count = 0;
  int result = 0;
  wchar_t sFileSearch[MAX_PATH] = EDITIONS_PATH;
  wsafecat(sFileSearch, MAX_PATH, L"*");

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
    return 0;
  while (result == 0)
  {
    if (count >= iListSize)
      break;
    if ((finddata.attrib & _A_SUBDIR) && finddata.name[0] != L'.')  // skip . and .. folders
    {
      wsafecpy(sEditionsList[count], iEditionNameSize, finddata.name);
      count++;
    }
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return count;
}

// -----------------------------------------------------------------
// Name : getSkills
// -----------------------------------------------------------------
int getSkills(wchar_t ** sSkillsList, int iListSize, int iSkillNameSize, const wchar_t * sEdition)
{
  _wfinddata_t finddata;
  int count = 0;
  int result = 0;
  wchar_t sFileSearch[MAX_PATH];;
  swprintf_s(sFileSearch, MAX_PATH, L"%s%s/skills/*.lua", EDITIONS_PATH, sEdition);

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
    return 0;
  while (result == 0)
  {
    if (count >= iListSize)
      break;
    wsafecpy(sSkillsList[count], iSkillNameSize, finddata.name);
    // Remove ".lua"
    sSkillsList[count][wcslen(sSkillsList[count])-4] = L'\0';
    count++;
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return count;
}

// -----------------------------------------------------------------
// Name : getProfiles
// -----------------------------------------------------------------
int getProfiles(wchar_t ** sProfilesList, int iListSize, int iProfileNameSize)
{
  _wfinddata_t finddata;
  int count = 0;
  int result = 0;
  wchar_t sFileSearch[MAX_PATH] = PROFILES_PATH;
  wsafecat(sFileSearch, MAX_PATH, L"*.dat");

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
  {
    if (errno == ENOENT)
      _wmkdir(PROFILES_PATH);
    return 0;
  }
  while (result == 0)
  {
    if (count >= iListSize)
      break;
    wsafecpy(sProfilesList[count], iProfileNameSize, finddata.name);
    // Remove ".dat"
    sProfilesList[count][wcslen(sProfilesList[count])-4] = L'\0';
    count++;
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return count;
}

// -----------------------------------------------------------------
// Name : getSavedGames
// -----------------------------------------------------------------
int getSavedGames(wchar_t ** sSavesList, int iListSize, int iSavesNameSize)
{
  _wfinddata_t finddata;
  int count = 0;
  int result = 0;
  wchar_t sFileSearch[MAX_PATH] = SAVES_PATH;
  wsafecat(sFileSearch, MAX_PATH, L"*.sav");

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
  {
    if (errno == ENOENT)
      _wmkdir(SAVES_PATH);
    return 0;
  }
  while (result == 0)
  {
    if (count >= iListSize)
      break;
    wsafecpy(sSavesList[count], iSavesNameSize, finddata.name);
    count++;
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return count;
}

// -----------------------------------------------------------------
// Name : getMaps
// -----------------------------------------------------------------
int getMaps(wchar_t ** sMapsList, int iListSize, int iMapNameSize)
{
  _wfinddata_t finddata;
  int count = 0;
  int result = 0;
  wchar_t sFileSearch[MAX_PATH] = MAPS_PATH;
  wsafecat(sFileSearch, MAX_PATH, L"*.lua");

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
  {
    if (errno == ENOENT)
      _wmkdir(MAPS_PATH);
    return 0;
  }
  while (result == 0)
  {
    if (count >= iListSize)
      break;
    wsafecpy(sMapsList[count], iMapNameSize, finddata.name);
    count++;
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return count;
}

#endif
