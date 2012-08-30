#ifdef WIN32
#include "../utils.h"
#include "../Common/md5.h"
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

bool _md5folder_rec(wchar_t * sFolder, struct md5_ctx * ctx)
{
  // Loop recursively into folders, and do checksum on files
  _wfinddata_t finddata;
  wchar_t sFileSearch[MAX_PATH];
  swprintf_s(sFileSearch, MAX_PATH, L"%s/*", sFolder);

  int result = 0;
  intptr_t hfile = _wfindfirst(sFileSearch, &finddata);
  if (hfile == -1)
    return true;  // no file may not be an error
  while (result == 0)
  {
    if (wcscmp(finddata.name, L".") != 0 && wcscmp(finddata.name, L"..") != 0)  // skip . and .. folders
    {
      if (finddata.attrib & _A_SUBDIR)  // subdir: loop into it
      {
        wchar_t sNewFolder[MAX_PATH];
        swprintf_s(sNewFolder, MAX_PATH, L"%s/%s", sFolder, finddata.name);
        if (!_md5folder_rec(sNewFolder, ctx))
          return false;
      }
      else  // normal file: do checksum
      {
        wchar_t sFile[MAX_PATH];
        swprintf_s(sFile, MAX_PATH, L"%s/%s", sFolder, finddata.name);
        FILE * pFile = NULL;
        if (0 != wfopen(&pFile, sFile, L"r"))
          return false; // error
        while (!feof(pFile))
        {
          ctx->size += fread(ctx->buf + ctx->size, 1, MD5_BUFFER - ctx->size, pFile);
          md5_update(ctx);
        }
        fclose(pFile);
      }
    }
    result = _wfindnext(hfile, &finddata);
  }
  _findclose(hfile);
  return true;
}

bool md5folder(wchar_t * sFolder, wchar_t * swDigest)
{
	struct md5_ctx ctx;
  unsigned char digest[16];
  md5_init(&ctx);
  bool bResult = _md5folder_rec(sFolder, &ctx);
  md5_final(digest, &ctx);
  char result[64];
	for (int i = 0; i < 16; i++)
		sprintf_s(&(result[2*i]), 16, "%02x", digest[i]);
  strtow(swDigest, 32, result);
  if (ctx.buf)
    free(ctx.buf);
  return bResult;
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

#endif
