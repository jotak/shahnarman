// -----------------------------------------------------------------
// Some utils functions...
// -----------------------------------------------------------------
#include "utils.h"

F_RGBA rgba(float r, float g, float b, float a)
{
  F_RGBA color;
  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;
  return color;
}

F_RGBA rgb(float r, float g, float b)
{
  F_RGBA color;
  color.r = r;
  color.g = g;
  color.b = b;
  color.a = 1.0f;
  return color;
}

void wchop(wchar_t * str)
{
  int len = (int) wcslen(str) - 1;
  while (len >= 0 && (str[len] == L'\n' || str[len] == L'\r' || str[len] == L'\t' || str[len] == L' '))
  {
    str[len] = L'\0';
    len--;
  }
}

void chop(char * str)
{
  int len = (int) strlen(str) - 1;
  while (len >= 0 && (str[len] == '\n' || str[len] == '\r' || str[len] == '\t' || str[len] == ' '))
  {
    str[len] = '\0';
    len--;
  }
}

void add_long_to_wstr(long iVal, int precision, wchar_t * sDst, int * iDst)
{
  if (iVal < 0)
  {
    sDst[(*iDst)++] = L'-';
    precision--;
    iVal = -iVal;
  }
  wchar_t sBuf[128];
  int iBuf = 0;
  do {
    u8 digit = (u8) (iVal % 10);
    sBuf[iBuf++] = L'0' + digit;
    iVal /= 10;
  } while (iVal != 0);
  while (precision-iBuf > 0)
  {
    sDst[(*iDst)++] = L'0';
    precision--;
  }
  while (iBuf > 0)
    sDst[(*iDst)++] = sBuf[--iBuf];
}

void add_double_to_wstr(double fVal, int precision, wchar_t * sDst, int * iDst)
{
  if (fVal < 0)
  {
    sDst[(*iDst)++] = L'-';
    fVal = -fVal;
  }
  wchar_t sBuf[128];
  int iBuf = 0;
  long tronc = (long) fVal;
  fVal = fVal - (double) tronc;
  do {
    u8 digit = (u8) (tronc % 10);
    sBuf[iBuf++] = L'0' + digit;
    tronc /= 10;
  } while (tronc != 0);
  while (iBuf > 0)
    sDst[(*iDst)++] = sBuf[--iBuf];
  if ((fVal == 0.0f && precision < 0) || precision == 0)
    return;
  sDst[(*iDst)++] = L'.';
  while (fVal != 0.0f)
  {
    fVal *= 10;
    u8 digit = (u8) fVal;
    sDst[(*iDst)++] = L'0' + digit;
    fVal = fVal - (double) digit;
    precision--;
    if (precision == 0 || precision == -5)  // 4 decimals by default if precision was -1
      return;
  }
}

void wsafecpy(wchar_t * dst, unsigned int size, const wchar_t * src)
{
  if (size < wcslen(src)+1)
  {
    unsigned int i = 0;
    while (i < size - 4)
    {
      dst[i] = src[i];
      i++;
    }
    while (i < size - 1)
      dst[i++] = L'.';
    dst[i] = L'\0';
  }
  else
    wcscpy_s(dst, size, src);
}

void wsafecat(wchar_t * dst, unsigned int size, const wchar_t * src)
{
  unsigned int idest = wcslen(dst);
  if (size < idest + wcslen(src)+1)
  {
    unsigned int i = 0;
    while (idest < size - 4)
      dst[idest++] = src[i++];
    while (idest < size - 1)
      dst[idest++] = L'.';
    dst[idest] = L'\0';
  }
  else
    wcscat_s(dst, size, src);
}
