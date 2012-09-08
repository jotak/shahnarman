#ifdef LINUX
#include "../utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>

//void add_long_to_wstr(long iVal, int precision, char * sDst, int * iDst);
//void add_double_to_wstr(double fVal, int precision, char * sDst, int * iDst);

// Random selon Linux
u32 getRandom(u32 max)
{
    return ((u32)rand()) % max;
}

errno_t fopen_s(FILE ** pFile, const char * sFilename, const char * sMode)
{
    *pFile = fopen(sFilename, sMode);
    if (*pFile == NULL)
        return -1;
    return 0;
}

// type: 0=any, 0x4=folder, 0x8=file
int getDirectoryContent(std::string dir, std::vector<std::string> &files, unsigned char type)
{
    DIR * dp;
    struct dirent * dirp;
    if ((dp  = opendir(dir.c_str())) == NULL)
    {
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        if (type == 0 || type == dirp->d_type)
        {
            files.push_back(std::string(dirp->d_name));
        }
    }
    closedir(dp);
    return 0;
}

char sClipboard[1024];

bool copyStringToClipboard(const char * wsource)
{
    // TODO: real clipboard on linux??
    wsafecpy(sClipboard, 1024, wsource);
    return true;
}

char * getStringFromClipboard(char * sBuffer, int iBufSize)
{
    // TODO: real clipboard on linux??
    wsafecpy(sBuffer, iBufSize, sClipboard);
    return sBuffer;
}

int getAvailableDisplayModes(CoordsScreen * pResolution, int * pBpp, int iMaxEntries)
{
    int i = 0;
    if (i <= iMaxEntries)
    {
        pResolution[i].x = 800;
        pResolution[i].y = 600;
        pBpp[i] = 16;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 800;
        pResolution[i].y = 600;
        pBpp[i] = 32;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1024;
        pResolution[i].y = 768;
        pBpp[i] = 16;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1024;
        pResolution[i].y = 768;
        pBpp[i] = 32;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1152;
        pResolution[i].y = 864;
        pBpp[i] = 32;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1280;
        pResolution[i].y = 768;
        pBpp[i] = 32;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1280;
        pResolution[i].y = 960;
        pBpp[i] = 32;
        i++;
    }
    else
        return i;

    if (i <= iMaxEntries)
    {
        pResolution[i].x = 1280;
        pResolution[i].y = 1024;
        pBpp[i] = 32;
        i++;
    }

    return i;
}

#endif
