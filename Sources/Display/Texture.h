#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "../utils.h"
#include "../SystemHeaders.h"

class Texture
{
public:
    Texture(const char * sFilename, bool bMipMap);
    ~Texture();

    virtual s16 load();
    virtual void unload();

    char m_sFilename[MAX_PATH];
    GLuint m_iWidth, m_iHeight;
    GLfloat m_fU0, m_fU1, m_fV0, m_fV1;
    GLenum m_Format;          // RVB, RVBA, Luminance, Luminance Alpha
    GLint m_iBpp;             // Bits per pixels
    GLuint m_uGlId;
    GLubyte * m_pTexels;      // data
    bool m_bIsLoadedInVideo;
    s32 m_iMasterTexture;
    bool m_bMipMap;
};

#endif
