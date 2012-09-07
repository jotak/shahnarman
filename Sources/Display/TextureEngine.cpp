// -----------------------------------------------------------------
// Texture Engine
// -----------------------------------------------------------------
#include "TextureEngine.h"
#include "../Debug/DebugManager.h"
#include "../errorcodes.h"
#include "../Data/XMLLiteReader.h"

// -----------------------------------------------------------------
// Name : TextureEngine
//  Constructor
// -----------------------------------------------------------------
TextureEngine::TextureEngine(DebugManager * pDebug)
{
  m_pDebug = pDebug;
}

// -----------------------------------------------------------------
// Name : ~TextureEngine
//  Destructor
// -----------------------------------------------------------------
TextureEngine::~TextureEngine()
{
  for (u16 i = 0; i < m_AllTextures.size(); i++)
    delete m_AllTextures[i];
  m_AllTextures.clear();
}

// -----------------------------------------------------------------
// Name : loadTexture
// -----------------------------------------------------------------
s32 TextureEngine::loadTexture(const char * sFilename, bool bMipmap, int ustart, int uend, int vstart, int vend)
{
  if (sFilename == NULL || strcmp(sFilename, "") == 0)
    return -1;

  s16 res = findTexture(sFilename);
  if (res >= 0) // Texture already loaded
    return res;

  int i = m_AllTextures.size();
  Texture * pTex = new Texture(sFilename, bMipmap);
  m_AllTextures.push_back(pTex);
  s16 err = pTex->load();
  if (err != TEX_OK)
  {
    m_pDebug->notifyErrorMessage(err, sFilename);
    return i;
  }
  else
    textureLoaded(pTex);

  pTex->m_bIsLoadedInVideo = true;

  // Set u/v coords
  if (ustart >= 0)
  {
    pTex->m_fU0 = (GLfloat)ustart / (GLfloat)pTex->m_iWidth;
    pTex->m_fU1 = (GLfloat)uend / (GLfloat)pTex->m_iWidth;
  }
  else
  {
    pTex->m_fU0 = 0.0f;
    pTex->m_fU1 = 1.0f;
  }
  if (vstart >= 0)
  {
    pTex->m_fV0 = (GLfloat)vstart / (GLfloat)pTex->m_iHeight;
    pTex->m_fV1 = (GLfloat)vend / (GLfloat)pTex->m_iHeight;
  }
  else
  {
    pTex->m_fV0 = 0.0f;
    pTex->m_fV1 = 1.0f;
  }
  return i;
}

// -----------------------------------------------------------------
// Name : textureLoaded
// -----------------------------------------------------------------
void TextureEngine::textureLoaded(Texture * pTex)
{
  // Register texture in Gl's engine
  glGenTextures(1, &(pTex->m_uGlId));
  glBindTexture(GL_TEXTURE_2D, pTex->m_uGlId);

  // Setup texture filters
  if (pTex->m_bMipMap)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

  if (pTex->m_bMipMap)
    gluBuild2DMipmaps(GL_TEXTURE_2D, pTex->m_iBpp, pTex->m_iWidth, pTex->m_iHeight, pTex->m_Format, GL_UNSIGNED_BYTE, pTex->m_pTexels);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, pTex->m_iBpp, pTex->m_iWidth, pTex->m_iHeight, 0, pTex->m_Format, GL_UNSIGNED_BYTE, pTex->m_pTexels);

  pTex->unload();  // We don't need to keep it in main memory
}

// -----------------------------------------------------------------
// Name : findTexture
// -----------------------------------------------------------------
s32 TextureEngine::findTexture(const char * sFilename)
{
  if (sFilename == NULL || strcmp(sFilename, "") == 0)
    return -1;

  for (u16 i = 0; i < m_AllTextures.size(); i++)
  {
    if (strcmp(m_AllTextures[i]->m_sFilename, sFilename) == 0)
      return i;
  }
  return -1;
}

// -----------------------------------------------------------------
// Name : loadComposedTexture
// -----------------------------------------------------------------
s32 TextureEngine::loadComposedTexture(const char * sFilename)
{
  // Get "master texture", ie big texture
  int iMasterTexture = loadTexture(sFilename);
  if (iMasterTexture < 0)
    return -1;
  Texture * pMaster = m_AllTextures[iMasterTexture];

  // Build file names
  char pngfile[MAX_PATH] = GAME_TEXTURES_PATH;
  wsafecat(pngfile, MAX_PATH, sFilename);
  char xmlfile[MAX_PATH];
  wsafecpy(xmlfile, MAX_PATH, pngfile);
  wsafecat(xmlfile, MAX_PATH, ".xm");

  // Read XML
  XMLLiteReader reader;
  XMLLiteElement * pRootNode = NULL;
  try {
    pRootNode = reader.parseFile(xmlfile);
  }
  catch (int errorCode)
  {
    m_pDebug->notifyXMLErrorMessage(xmlfile, errorCode, reader.getCurrentLine(), reader.getCurrentCol());
    return iMasterTexture;
  }
  assert(pRootNode != NULL);

  char sError[1024];
  XMLLiteElement * pChild = pRootNode->getFirstChild();
  while (pChild != NULL)
  {
    if (0 != strcasecmp(pChild->getName(), "item"))
    {
      pChild = pRootNode->getNextChild();
      continue;
    }
    XMLLiteAttribute * pAttr = pChild->getAttributeByName("name");
    if (pAttr == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"name\" attribute in node item. Check out file %s.", pRootNode->getName());
      m_pDebug->notifyErrorMessage(sError);
      pChild = pRootNode->getNextChild();
      continue;
    }
    XMLLiteElement * pUStartElt = pChild->getChildByName("ustart");
    if (pUStartElt == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"ustart\" node in node item. Check out file %s.", pRootNode->getName());
      m_pDebug->notifyErrorMessage(sError);
      pChild = pRootNode->getNextChild();
      continue;
    }
    XMLLiteElement * pUEndElt = pChild->getChildByName("uend");
    if (pUEndElt == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"uend\" node in node item. Check out file %s.", pRootNode->getName());
      m_pDebug->notifyErrorMessage(sError);
      pChild = pRootNode->getNextChild();
      continue;
    }
    XMLLiteElement * pVStartElt = pChild->getChildByName("vstart");
    if (pVStartElt == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"vstart\" node in node item. Check out file %s.", pRootNode->getName());
      m_pDebug->notifyErrorMessage(sError);
      pChild = pRootNode->getNextChild();
      continue;
    }
    XMLLiteElement * pVEndElt = pChild->getChildByName("vend");
    if (pVEndElt == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"vend\" node in node item. Check out file %s.", pRootNode->getName());
      m_pDebug->notifyErrorMessage(sError);
      pChild = pRootNode->getNextChild();
      continue;
    }

    // Create kind of virtual texture
    char sTexPath[MAX_PATH];
    snprintf(sTexPath, MAX_PATH, "%s:%s", sFilename, pAttr->getCharValue());
    Texture * pTex = new Texture(sTexPath, false);
    int ustart = (int) pUStartElt->getIntValue();
    int uend = (int) pUEndElt->getIntValue();
    int vstart = (int) pVStartElt->getIntValue();
    int vend = (int) pVEndElt->getIntValue();
    pTex->m_iWidth = 1 + uend - ustart;
    pTex->m_iHeight = 1 + vend - vstart;
    pTex->m_fU0 = (0.5f + (float) ustart) / (float) (pMaster->m_iWidth);
    pTex->m_fU1 = (0.5f + (float) uend) / (float) (pMaster->m_iWidth);
    pTex->m_fV0 = (0.5f + (float) vstart) / (float) (pMaster->m_iHeight);
    pTex->m_fV1 = (0.5f + (float) vend) / (float) (pMaster->m_iHeight);
    pTex->m_uGlId = pMaster->m_uGlId;
    pTex->m_Format = pMaster->m_Format;
    pTex->m_bIsLoadedInVideo = pMaster->m_bIsLoadedInVideo;
    pTex->m_iMasterTexture = iMasterTexture;
    m_AllTextures.push_back(pTex);

    // Continue loop
    pChild = pRootNode->getNextChild();
  }

  return iMasterTexture;
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
Texture * TextureEngine::getTexture(u16 iIndex)
{
  assert(iIndex >= 0 && iIndex < m_AllTextures.size());
  return m_AllTextures[iIndex];
}

// -----------------------------------------------------------------
// Name : reloadAllTextures
// -----------------------------------------------------------------
void TextureEngine::reloadAllTextures()
{
  for (u16 i = 0; i < m_AllTextures.size(); i++)
  {
    Texture * pTex = m_AllTextures[i];
    if (pTex->m_iMasterTexture < 0)  // not composed
    {
      s16 err = pTex->load();
      if (err != TEX_OK)
        m_pDebug->notifyErrorMessage(err, pTex->m_sFilename);
      else
        textureLoaded(pTex);
    }
  }
  for (u16 i = 0; i < m_AllTextures.size(); i++)
  {
    Texture * pTex = m_AllTextures[i];
    if (pTex->m_iMasterTexture >= 0)  // composed
      pTex->m_uGlId = m_AllTextures[pTex->m_iMasterTexture]->m_uGlId;
  }
}
