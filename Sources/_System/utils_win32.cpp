#ifdef WIN32
#include "../utils.h"
#include <io.h>

// Random selon Windows
u32 getRandom(u32 max)
{
  return ((u32)rand()) % max;
}

size_t strtow(wchar_t * sDst, int sizemax, const char * sSrc)
{
  size_t nbConvertedChars = 0;
  mbstowcs_s(&nbConvertedChars, sDst, sizemax, sSrc, _TRUNCATE);
  return nbConvertedChars;
}

size_t wtostr(char * sDst, int sizemax, const wchar_t * sSrc)
{
  size_t nbConvertedChars = 0;
  wcstombs_s(&nbConvertedChars, sDst, sizemax, sSrc, _TRUNCATE);
  return nbConvertedChars;
}

bool copyStringToClipboard(wchar_t * wsource)
{
  if (!OpenClipboard(NULL))
    return false;
  int len = wcslen(wsource) + 1;
  char * source = new char[len];
  wtostr(source, len, wsource);
  HGLOBAL clipbuffer;
  char * buffer;
  EmptyClipboard();
  clipbuffer = GlobalAlloc(GMEM_DDESHARE, len);
  buffer = (char*)GlobalLock(clipbuffer);
  strcpy_s(buffer, len, source);
  GlobalUnlock(clipbuffer);
  SetClipboardData(CF_TEXT, clipbuffer);
  CloseClipboard();
  delete[] source;
  return true;
}

wchar_t * getStringFromClipboard(wchar_t * sBuffer, int iBufSize)
{
  if (!OpenClipboard(NULL))
    return NULL;
  char * buffer = NULL;
  HANDLE hData = GetClipboardData( CF_TEXT );
  buffer = (char*)GlobalLock( hData );
  GlobalUnlock( hData );
  CloseClipboard();
  strtow(sBuffer, iBufSize, buffer);
  return sBuffer;
}

int getAvailableDisplayModes(CoordsScreen * pResolution, int * pBpp, int iMaxEntries)
{
  DEVMODE mode;
  mode.dmSize = sizeof(DEVMODE);
  int iGoodMode = 0;
  int iTestMode = 0;
  while (EnumDisplaySettings(NULL, iTestMode++, &mode))
  {
    if (iGoodMode < iMaxEntries)
    {
      if (mode.dmPelsWidth >= 800 && mode.dmPelsHeight >= 600 && mode.dmBitsPerPel >= 16)
      {
        pResolution[iGoodMode].x = mode.dmPelsWidth;
        pResolution[iGoodMode].y = mode.dmPelsHeight;
        pBpp[iGoodMode] = mode.dmBitsPerPel;
        iGoodMode++;
      }
    }
    else
      break;
  }
  return iGoodMode;
}

// type: 0=any, 0x4=folder, 0x8=file
int getDirectoryContent(string dir, vector<string> &files, unsigned char type)
{
  _wfinddata_t finddata;
  int result = 0;
  char sFileSearch[MAX_PATH] = dir;
  strncat(sFileSearch, "*", MAX_PATH);

  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
    return -1;

  while (result == 0)
  {
    if ((type == 0x4 && (finddata.attrib & _A_SUBDIR))
        || (type == 0x8 && !(finddata.attrib & _A_SUBDIR))
        || type == 0)
    {
        files.push_back(string(finddata.name));
    }
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
}

#endif
