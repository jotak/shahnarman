#ifdef LINUX
#include "../utils.h"
#include <stdlib.h>
#include <stdarg.h>

void add_long_to_wstr(long iVal, int precision, wchar_t * sDst, int * iDst);
void add_double_to_wstr(double fVal, int precision, wchar_t * sDst, int * iDst);

// Random selon Linux
u32 getRandom(u32 max)
{
  return ((u32)rand()) % max;
}

// Not all standard "printf" specifiers are managed here. Improve this function if needed.
void swprintf_s(wchar_t * sDst, int iSize, const wchar_t * sModel, ...)
{
  va_list pArgs;
  int iDst = 0;
  int iSrc = 0;
  va_start(pArgs, sModel);
  while (sModel[iSrc] != '\0')
  {
    if (sModel[iSrc] == L'%')
    {
      iSrc++;
      // is '%' character?
      if (sModel[iSrc] == L'%')
        sDst[iDst++] = sModel[iSrc++];
      else
      {
        int precision = -1;
 //       u8 length = 1;
        // precision?
        if (sModel[iSrc] == L'.')
        {
          precision = (int) (sModel[++iSrc] - L'0');
          iSrc++;
        }
        // length?
        if (sModel[iSrc] == L'h')
        {
 //         length = 0; // short
          iSrc++;
        }
        else if (sModel[iSrc] == L'l' || sModel[iSrc] == L'L')
        {
 //         length = 2; // long
          iSrc++;
        }
        // specifier
        switch (sModel[iSrc])
        {
        case L'u':
          {
            unsigned uVal = va_arg(pArgs, unsigned);
            add_long_to_wstr((long)uVal, precision, sDst, &iDst);
            iSrc++;
            break;
          }
        case L'd':
          {
            long iVal = va_arg(pArgs, int);
            add_long_to_wstr(iVal, precision, sDst, &iDst);
            iSrc++;
            break;
          }
        case L'f':
          {
            double fVal = va_arg(pArgs, double);
            add_double_to_wstr(fVal, precision, sDst, &iDst);
            iSrc++;
            break;
          }
        case L'c':
          {
            int cVal = va_arg(pArgs, int);
            sDst[iDst++] = (wchar_t) cVal;
            iSrc++;
            break;
          }
        case L's':
          {
            wchar_t * sVal = va_arg(pArgs, wchar_t*);
            sDst[iDst] = L'\0';
            wsafecat(sDst, iSize, sVal);
            iDst = wcslen(sDst);
            iSrc++;
            break;
          }
        }
      }
    }
    else
      sDst[iDst++] = sModel[iSrc++];
  }
  sDst[iDst] = L'\0';
  va_end(pArgs);
}

//int swscanf_s(const wchar_t * sSrc, const wchar_t * sModel, ...)
//{
//  return 0;
//}

errno_t wfopen(FILE ** pFile, const wchar_t * sFilename, const wchar_t * sMode)
{
  char sLFilename[MAX_PATH];
  wcsrtombs(sLFilename, &sFilename, MAX_PATH, NULL);
  char sLMode[32];
  wcsrtombs(sLMode, &sMode, 32, NULL);
  *pFile = fopen(sLFilename, sLMode);
  if (*pFile == NULL)
    return -1;
  return 0;
}

errno_t fopen_s(FILE ** pFile, const char * sFilename, const char * sMode)
{
  *pFile = fopen(sFilename, sMode);
  if (*pFile == NULL)
    return -1;
  return 0;
}

size_t strtow(wchar_t * sDst, int sizemax, const char * sSrc)
{
  return mbstowcs(sDst, sSrc, sizemax);
}

size_t wtostr(char * sDst, int sizemax, const wchar_t * sSrc)
{
  return wcstombs(sDst, sSrc, sizemax);
}

#endif
