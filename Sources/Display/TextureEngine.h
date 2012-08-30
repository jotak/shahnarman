#ifndef _TEXTURE_ENGINE_H
#define _TEXTURE_ENGINE_H

#include "Texture.h"
#include "../Common/BaseObject.h"

class DebugManager;

class TextureEngine
{
public:
  // Constructor / destructor
  TextureEngine(DebugManager * pDebug);
  ~TextureEngine();

  // Texture loading funtions
  s32 loadComposedTexture(const wchar_t * sFilename);
  s32 loadTexture(const wchar_t * sFilename, bool bMipmap = false, int ustart = -1, int uend = -1, int vstart = -1, int vend = -1);
  s32 findTexture(const wchar_t * sFilename);
  void reloadAllTextures();

  // Member access functions
  Texture * getTexture(u16 iIndex);

protected:
  void textureLoaded(Texture * pTex);

  std::vector<Texture*> m_AllTextures;
  DebugManager * m_pDebug;
};

#endif
