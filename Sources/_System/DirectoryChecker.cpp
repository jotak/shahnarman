#include "../utils.h"
#include <stdio.h>
#include <vector>
#include <string.h>
#include "../Common/md5.h"

using namespace std;

// type: 0=any, 0x4=folder, 0x8=file
extern int getDirectoryContent(string dir, vector<string> &files, unsigned char type);

// -----------------------------------------------------------------
// Name : getEditions
// -----------------------------------------------------------------
int getEditions(wchar_t ** sEditionsList, unsigned int iListSize, int iEditionNameSize)
{
    unsigned int count = 0;
    char dir[MAX_PATH];
    wtostr(dir, MAX_PATH, EDITIONS_PATH);
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x4);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (count >= iListSize)
            break;
        if (files[i][0] != L'.')  // skip . and .. folders
        {
            strtow(sEditionsList[count], iEditionNameSize, files[i].c_str());
            count++;
        }
    }
    return count;
}

// -----------------------------------------------------------------
// Name : getSkills
// -----------------------------------------------------------------
int getSkills(wchar_t ** sSkillsList, unsigned int iListSize, int iSkillNameSize, const wchar_t * sEdition)
{
    unsigned int count = 0;
    char dir[MAX_PATH];
    wchar_t wdir[MAX_PATH];
    swprintf(wdir, MAX_PATH, L"%s%s/skills/", EDITIONS_PATH, sEdition);
    wtostr(dir, MAX_PATH, wdir);
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x8);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (count >= iListSize)
            break;
        unsigned int length = files[i].size();
        if (length > 4
                && files[i][length-4] == '.'
                && files[i][length-3] == 'l'
                && files[i][length-2] == 'u'
                && files[i][length-1] == 'a')
        {
            strtow(sSkillsList[count], iSkillNameSize, files[i].c_str());
            // Remove ".lua"
            sSkillsList[count][length-4] = L'\0';
            count++;
        }
    }
    return count;
}

// -----------------------------------------------------------------
// Name : getProfiles
// -----------------------------------------------------------------
int getProfiles(wchar_t ** sProfilesList, unsigned int iListSize, int iProfileNameSize)
{
    unsigned int count = 0;
    char dir[MAX_PATH];
    wtostr(dir, MAX_PATH, PROFILES_PATH);
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x8);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (count >= iListSize)
            break;
        unsigned int length = files[i].size();
        if (length > 4
                && files[i][length-4] == '.'
                && files[i][length-3] == 'd'
                && files[i][length-2] == 'a'
                && files[i][length-1] == 't')
        {
            strtow(sProfilesList[count], iProfileNameSize, files[i].c_str());
            // Remove ".dat"
            sProfilesList[count][length-4] = L'\0';
            count++;
        }
    }
    return count;
}

// -----------------------------------------------------------------
// Name : getSavedGames
// -----------------------------------------------------------------
int getSavedGames(wchar_t ** sSavesList, unsigned int iListSize, int iSavesNameSize)
{
    unsigned int count = 0;
    char dir[MAX_PATH];
    wtostr(dir, MAX_PATH, SAVES_PATH);
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x8);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (count >= iListSize)
            break;
        unsigned int length = files[i].size();
        if (length > 4
                && files[i][length-4] == '.'
                && files[i][length-3] == 's'
                && files[i][length-2] == 'a'
                && files[i][length-1] == 'v')
        {
            strtow(sSavesList[count], iSavesNameSize, files[i].c_str());
            // Remove ".sav"
            sSavesList[count][length-4] = L'\0';
            count++;
        }
    }
    return count;
}

// -----------------------------------------------------------------
// Name : getMaps
// -----------------------------------------------------------------
int getMaps(wchar_t ** sMapsList, unsigned int iListSize, int iMapNameSize)
{
    unsigned int count = 0;
    char dir[MAX_PATH];
    wtostr(dir, MAX_PATH, MAPS_PATH);
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x8);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (count >= iListSize)
            break;
        unsigned int length = files[i].size();
        if (length > 4
                && files[i][length-4] == '.'
                && files[i][length-3] == 'l'
                && files[i][length-2] == 'u'
                && files[i][length-1] == 'a')
        {
            strtow(sMapsList[count], iMapNameSize, files[i].c_str());
            count++;
        }
    }
    return count;
}

bool _md5folder_rec(wchar_t * sFolder, struct md5_ctx * ctx)
{
    // Loop recursively into folders, and do checksum on files
    wchar_t wdir[MAX_PATH];
    swprintf(wdir, MAX_PATH, L"%s/", sFolder);
    char dir[MAX_PATH];
    wtostr(dir, MAX_PATH, wdir);

    // Get files
    vector<string> files = vector<string>();
    getDirectoryContent(string(dir), files, 0x8);
    for (unsigned int i = 0; i < files.size(); i++)
    {
        char sFile[MAX_PATH];
        snprintf(sFile, MAX_PATH, "%s%s", dir, files[i].c_str());
        FILE * pFile = NULL;
        if (0 != fopen_s(&pFile, sFile, "r"))
            return false; // error
        while (!feof(pFile))
        {
            ctx->size += fread(ctx->buf + ctx->size, 1, MD5_BUFFER - ctx->size, pFile);
            md5_update(ctx);
        }
        fclose(pFile);
    }

    // Get dirs
    vector<string> dirs = vector<string>();
    getDirectoryContent(string(dir), dirs, 0x4);
    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (strcmp(files[i].c_str(), ".") == 0 || strcmp(files[i].c_str(), "..") == 0)  // skip . and .. folders
            continue;

        char sNewFolder[MAX_PATH];
        snprintf(sNewFolder, MAX_PATH, "%s%s", dir, files[i].c_str());
        wchar_t swNewFolder[MAX_PATH];
        strtow(swNewFolder, MAX_PATH, sNewFolder);
        if (!_md5folder_rec(swNewFolder, ctx))
            return false;
    }
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
        snprintf(&(result[2*i]), 16, "%02x", digest[i]);
    strtow(swDigest, 32, result);
    if (ctx.buf)
        free(ctx.buf);
    return bResult;
}
