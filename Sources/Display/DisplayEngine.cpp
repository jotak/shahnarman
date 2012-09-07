#include "DisplayEngine.h"
#include "../SystemHeaders.h"
#include "../Geometries/Geometry.h"
#include "../Debug/DebugManager.h"

#define BASE_WIDTH    1024.0f
#define BASE_HEIGHT   576.0f

// ------------------------------------------------------------------
// Name : DisplayEngine
//  Constructor
// ------------------------------------------------------------------
DisplayEngine::DisplayEngine()
{
  m_pClientParams = NULL;
  m_pTexEngine = NULL;
  m_pFntEngine = NULL;
  m_pDebug = NULL;
  m_ModeState = DMS_Undefined;
  m_bReady = false;
  m_iStencilState = 0;
  m_iWindow = -1;
  m_pRegisteredGeometries = new ObjectList(false);
  m_iMapWidth = m_iMapHeight = 0;
  m_bAdditive = false;
  m_iStencilDepth = 0;
  m_dScreenRatio = 1;
  m_bLookAtMode = false;
}

// ------------------------------------------------------------------
// Name : ~DisplayEngine
//  Destructor
// ------------------------------------------------------------------
DisplayEngine::~DisplayEngine()
{
  if (m_pTexEngine != NULL)
    delete m_pTexEngine;
  if (m_pFntEngine != NULL)
    delete m_pFntEngine;
  delete m_pRegisteredGeometries;
}

// ------------------------------------------------------------------
// Name : Init
// ------------------------------------------------------------------
void DisplayEngine::Init(Parameters * clientParams, DebugManager * pDebug)
{
  m_pClientParams = clientParams;
  m_pDebug = pDebug;

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_ALPHA);
  m_f3CamPos = Coords3D(0.0f, 0.0f, 0.0f);

  initGlutWindow();

  m_pTexEngine = new TextureEngine(pDebug);
  m_pFntEngine = new FontEngine(this, pDebug);
}

// ------------------------------------------------------------------
// Name : initGlutWindow
// ------------------------------------------------------------------
void DisplayEngine::initGlutWindow()
{
  extern bool g_bIgnoreNextResize;
  g_bIgnoreNextResize = true;

  if (m_iWindow == -2)  // was fullscreen
    glutLeaveGameMode();
  else if (m_iWindow >= 0)  // was windowed
  {
    m_pClientParams->winXPos = glutGet(GLUT_WINDOW_X);
    m_pClientParams->winYPos = glutGet(GLUT_WINDOW_Y);
    glutDestroyWindow(m_iWindow);
  }

  extern int g_iOldW;
  extern int g_iOldH;
  g_iOldW = m_pClientParams->screenXSize;
  g_iOldH = m_pClientParams->screenYSize;
  if (m_pClientParams->fullscreen)
  {
    glutGameModeString(m_pClientParams->sGameModeString);
    glutEnterGameMode();
    m_pClientParams->screenXSize = glutGameModeGet(GLUT_GAME_MODE_WIDTH);
    m_pClientParams->screenYSize = glutGameModeGet(GLUT_GAME_MODE_HEIGHT);
    m_iWindow = -2;
  }
  else
  {
    glutInitWindowPosition(m_pClientParams->winXPos, m_pClientParams->winYPos);
    m_pClientParams->screenXSize = m_pClientParams->winWidth;
    m_pClientParams->screenYSize = m_pClientParams->winHeight;
    glutInitWindowSize(m_pClientParams->screenXSize, m_pClientParams->screenYSize);
    m_iWindow = glutCreateWindow("Shahnarman");
  }
  resizeWindow();

  glShadeModel(GL_SMOOTH);
  glDisable(GL_BLEND);
  glAlphaFunc(GL_GREATER, 0.0001f);
  glEnable(GL_ALPHA_TEST);

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.05f, 0.1f, 0.0f);
  glClearDepth(1.0f);
  glClearStencil(0);

  GLenum err = glewInit();
  if (err != GLEW_OK)
  {
    char sLog[1024] = "";
    snprintf(sLog, 1024, "Error in glewInit: %s.\n", glewGetErrorString(err));
    m_pDebug->notifyErrorMessage(sLog);
  }
  if (!glewIsSupported("GL_ARB_shading_language_100"))
    m_pDebug->notifyErrorMessage("Warning: extension GL_ARB_shading_language_100 not supported.");
  if (!glewIsSupported("GL_ARB_shader_objects"))
    m_pDebug->notifyErrorMessage("Warning: extension GL_ARB_shader_objects not supported.");
  if (!glewIsSupported("GL_ARB_vertex_shader"))
    m_pDebug->notifyErrorMessage("Warning: extension GL_ARB_vertex_shader not supported.");
  if (!glewIsSupported("GL_ARB_fragment_shader"))
    m_pDebug->notifyErrorMessage("Warning: extension GL_ARB_fragment_shader not supported.");

  Geometry * pGeometry = (Geometry*) m_pRegisteredGeometries->getFirst(0);
  while (pGeometry != NULL)
  {
    pGeometry->reload();
    pGeometry = (Geometry*) m_pRegisteredGeometries->getNext(0);
  }

  if (m_pTexEngine != NULL)
    m_pTexEngine->reloadAllTextures();
}

// ------------------------------------------------------------------
// Name : resizeWindow
// ------------------------------------------------------------------
void DisplayEngine::resizeWindow()
{
  //if (m_bLookAtMode)
  //{
  //  m_dScreenRatio = 16.0f / 9.0f;
  //  double hMargin = 0;
  //  double vMargin = 0;
  //  if (m_pClientParams->screenXSize > m_dScreenRatio * (double) m_pClientParams->screenYSize)
  //    vMargin = ((double) m_pClientParams->screenXSize - m_dScreenRatio * (double) m_pClientParams->screenYSize) / 2;
  //  else if (m_pClientParams->screenYSize > (double) m_pClientParams->screenXSize / m_dScreenRatio)
  //    hMargin = ((double) m_pClientParams->screenYSize - (double) m_pClientParams->screenXSize / m_dScreenRatio) / 2;
  //	glViewport(vMargin, hMargin, m_pClientParams->screenXSize - vMargin, m_pClientParams->screenYSize - hMargin);
  //}
  //else
  //{
  	glViewport(0, 0, m_pClientParams->screenXSize, m_pClientParams->screenYSize);
    m_dScreenRatio = BASE_WIDTH / BASE_HEIGHT;
//    m_dScreenRatio = (double) m_pClientParams->screenXSize / (double) m_pClientParams->screenYSize;
  //}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

  glFrustum(-m_dScreenRatio, m_dScreenRatio, 1.0f, -1.0f, NEARPLANE, FARPLANE);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  moveCameraTo(m_f3CamPos);
}

// ------------------------------------------------------------------
// Name : beginDisplay
// ------------------------------------------------------------------
void DisplayEngine::beginDisplay()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_iStencilDepth = 0;
  enableBlending();
}

// ------------------------------------------------------------------
// Name : endDisplay
// ------------------------------------------------------------------
void DisplayEngine::endDisplay()
{
  glutSwapBuffers();
}

// ------------------------------------------------------------------
// Name : begin2D
// ------------------------------------------------------------------
void DisplayEngine::begin2D()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, (GLdouble)m_pClientParams->screenXSize, (GLdouble)m_pClientParams->screenYSize, 0.0f, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  m_ModeState = DMS_2D;
}

// ------------------------------------------------------------------
// Name : end2D
// ------------------------------------------------------------------
void DisplayEngine::end2D()
{
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  m_ModeState = DMS_Undefined;
}

// ------------------------------------------------------------------
// Name : begin3D
// ------------------------------------------------------------------
void DisplayEngine::begin3D()
{
  m_ModeState = DMS_3D;
}

// ------------------------------------------------------------------
// Name : end3D
// ------------------------------------------------------------------
void DisplayEngine::end3D()
{
  m_ModeState = DMS_Undefined;
}

// ------------------------------------------------------------------
// Name : moveCameraBy
// ------------------------------------------------------------------
void DisplayEngine::moveCameraBy(Coords3D d3Delta)
{
  moveCameraTo(m_f3CamPos + d3Delta);
}

// ------------------------------------------------------------------
// Name : moveCameraTo
// ------------------------------------------------------------------
void DisplayEngine::moveCameraTo(Coords3D d3Pos)
{
  m_f3CamPos = d3Pos;
  glLoadIdentity();
  if (m_bLookAtMode)
    gluLookAt(m_f3CamPos.x, m_f3CamPos.y, -m_f3CamPos.z, 0, 0, -m_f3CamPos.z-BOARDPLANE, 0, 1, 0);
  else
    glTranslatef(-m_f3CamPos.x, -m_f3CamPos.y, m_f3CamPos.z);
}

#define X2X   (2.0f * m_dScreenRatio)
#define X2Y   (2.0f)
#define M1X   (m_dScreenRatio)
#define M1Y   (1.0f)

// ------------------------------------------------------------------
// Name : getMapCoords
// ------------------------------------------------------------------
CoordsMap DisplayEngine::getMapCoords(CoordsScreen screenCoords)
{
  screenCoords.z = BOARDPLANE;
  Coords3D coords3D = get3DCoords(screenCoords, DMS_3D);
  return getMapCoords(coords3D);
}

// ------------------------------------------------------------------
// Name : getMapCoords
// ------------------------------------------------------------------
CoordsMap DisplayEngine::getMapCoords(Coords3D d3Coords)
{
  if (m_iMapWidth == 0 || m_iMapHeight == 0)
    return CoordsMap();
  if (d3Coords.x < 0.0f && d3Coords.x != (double)(int)d3Coords.x)
    d3Coords.x -= 1.0f;
  if (d3Coords.y < 0.0f && d3Coords.y != (double)(int)d3Coords.y)
    d3Coords.y -= 1.0f;
  CoordsMap mapCoords((int)d3Coords.x + m_iMapWidth/2, (int)d3Coords.y + m_iMapHeight/2);
//  CoordsMap mapCoords((int)d3Coords.x + m_pServerParams->mapXSize/2, m_pServerParams->mapYSize/2 - (int)d3Coords.y);
  return mapCoords;
}

// ------------------------------------------------------------------
// Name : getScreenCoords
// ------------------------------------------------------------------
CoordsScreen DisplayEngine::getScreenCoords(CoordsMap mapCoords)
{ // TODO : A REVOIR
  if (m_iMapWidth == 0 || m_iMapHeight == 0 || m_pClientParams == NULL)
    return CoordsScreen();
  CoordsScreen screenCoords((mapCoords.x * m_pClientParams->screenXSize) / m_iMapWidth, (mapCoords.y * m_pClientParams->screenYSize) / m_iMapHeight);
  return screenCoords;
}

// ------------------------------------------------------------------
// Name : getScreenCoords
// ------------------------------------------------------------------
CoordsScreen DisplayEngine::getScreenCoords(Coords3D d3Coords, DisplayModeState modeState)
{
  CoordsScreen screenCoords; // TODO
  return screenCoords;
}

// ------------------------------------------------------------------
// Name : get3DCoords
// ------------------------------------------------------------------
Coords3D DisplayEngine::get3DCoords(CoordsMap mapCoords, double fZPlane)
{
  if (m_iMapWidth == 0 || m_iMapHeight == 0)
    return Coords3D();
  Coords3D d3Coords((double)(mapCoords.x - m_iMapWidth/2), (double)(mapCoords.y - m_iMapHeight/2), fZPlane);
//  Coords3D d3Coords((float)(mapCoords.x - m_pServerParams->mapXSize/2), (float)(m_pServerParams->mapYSize/2 - mapCoords.y), fZPlane);
  return d3Coords;
}

// ------------------------------------------------------------------
// Name : get3DCoords
// ------------------------------------------------------------------
Coords3D DisplayEngine::get3DCoords(CoordsScreen screenCoords, DisplayModeState modeState)
{
  switch (modeState)
  {
  case DMS_2D:
    return Coords3D((double)screenCoords.x, (double)screenCoords.y, screenCoords.z);
  case DMS_3DCamIndependant:
    {
      if (m_pClientParams == NULL)
        return Coords3D();
      Coords3D d3Coords(((double)screenCoords.x * X2X / (double)m_pClientParams->screenXSize) - M1X, ((double)screenCoords.y * X2Y / (double)m_pClientParams->screenYSize) - M1Y, screenCoords.z);
//      Coords3D d3Coords(((float)screenCoords.x * 2.0f / (float)m_pClientParams->screenXSize) - 1.0f, 1.0f - ((float)screenCoords.y * 2.0f / (float)m_pClientParams->screenYSize), screenCoords.z);
      return d3Coords;
    }
  case DMS_3D:
  default:
    {
      if (m_pClientParams == NULL)
        return Coords3D();
      double fX = ((double)screenCoords.x * X2X / (double)m_pClientParams->screenXSize) - M1X;
      double fY = ((double)screenCoords.y * X2Y / (double)m_pClientParams->screenYSize) - M1Y;
//      float fX = ((float)screenCoords.x * 2.0f / (float)m_pClientParams->screenXSize) - 1.0f;
//      float fY = 1.0f - ((float)screenCoords.y * 2.0f / (float)m_pClientParams->screenYSize);
      Coords3D d3Coords(m_f3CamPos.x + fX * (screenCoords.z - m_f3CamPos.z) / NEARPLANE, m_f3CamPos.y + fY * (screenCoords.z - m_f3CamPos.z) / NEARPLANE, 0.0f);
      return d3Coords;
    }
  }
}

// ------------------------------------------------------------------
// Name : get3DDistance
// ------------------------------------------------------------------
Coords3D DisplayEngine::get3DDistance(CoordsScreen screenDist, DisplayModeState modeState)
{
  switch (modeState)
  {
  case DMS_2D:
    return Coords3D((double)screenDist.x, (double)screenDist.y, screenDist.z);
  case DMS_3D:
  default:
    {
      if (m_pClientParams == NULL)
        return Coords3D();
      double fX = -(double)screenDist.x * X2X / (double)m_pClientParams->screenXSize;
      double fY = -(double)screenDist.y * X2Y / (double)m_pClientParams->screenYSize;
//      float fX = -(float)screenDist.x * 2.0f / (float)m_pClientParams->screenXSize;
//      float fY = (float)screenDist.y * 2.0f / (float)m_pClientParams->screenYSize;
      Coords3D d3Coords(fX * (screenDist.z - m_f3CamPos.z) / NEARPLANE, fY * (screenDist.z - m_f3CamPos.z) / NEARPLANE, 0.0f);
      return d3Coords;
    }
  }
}

// ------------------------------------------------------------------
// Name : isZoomToMapPosNeeded
// ------------------------------------------------------------------
bool DisplayEngine::isZoomToMapPosNeeded(CoordsMap mapPos)
{
  if (m_f3CamPos.z < BOARDPLANE / 2)
    return true;
//  if (m_pGameParams->fUserZoom != m_fZoom)
//    return true;

  //TODO : redo it
//  float xMargin = absPxlToLogicX(m_pGameParams->screenXSize / 10);
//  float yMargin = absPxlToLogicY(m_pGameParams->screenYSize / 10);
//  return (fX < m_fLeft + xMargin) || (fX >= m_fRight + xMargin) || (fY < m_fTop + yMargin) || (fY >= m_fBottom + yMargin);
  return false;
}

// ------------------------------------------------------------------
// Name : setStencilState
//  return previous state
// ------------------------------------------------------------------
int DisplayEngine::setStencilState(int iState)
{
  int previous = m_iStencilState;
  m_iStencilState = iState;
  switch (iState)
  {
  case 0: // no stencil
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    break;
  case 1: // fill stencil
    glEnable(GL_STENCIL_TEST);
    // Increase stencil to a higher depth when it's filled for current depth
    glStencilFunc(GL_EQUAL, m_iStencilDepth, 0xffffffff);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    m_iStencilDepth++;
    break;
  case 2: // write with stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, m_iStencilDepth, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    break;
  case 3: // unfill stencil
    glEnable(GL_STENCIL_TEST);
    // Decrease stencil to lower depth when it's filled for current depth
    glStencilFunc(GL_EQUAL, m_iStencilDepth, 0xffffffff);
    glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    m_iStencilDepth--;
    break;
  }
  return previous;
}

// ------------------------------------------------------------------
// Name : enableBlending
// ------------------------------------------------------------------
void DisplayEngine::enableBlending()
{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDepthMask(false);
}

// ------------------------------------------------------------------
// Name : disableBlending
// ------------------------------------------------------------------
void DisplayEngine::disableBlending()
{
  glDepthMask(true);
  glDisable(GL_BLEND);
}

// ------------------------------------------------------------------
// Name : setMaskBlending
// ------------------------------------------------------------------
void DisplayEngine::setMaskBlending(bool bGreyDarken)
{
//  glBlendEquation(GL_MIN);
//  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  if (bGreyDarken)
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
  else
    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
}

// ------------------------------------------------------------------
// Name : startMaskBlending
// ------------------------------------------------------------------
void DisplayEngine::startMaskBlending()
{
//  glBlendEquation(GL_FUNC_ADD);
//  glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
//  glBlendFunc(GL_ONE, GL_ONE);
  glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
}

// ------------------------------------------------------------------
// Name : stopMaskBlending
// ------------------------------------------------------------------
void DisplayEngine::stopMaskBlending()
{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// ------------------------------------------------------------------
// Name : registerGeometry
// ------------------------------------------------------------------
void DisplayEngine::registerGeometry(Geometry * pGeometry)
{
  m_pRegisteredGeometries->addLast(pGeometry);
}

// ------------------------------------------------------------------
// Name : unregisterGeometry
// ------------------------------------------------------------------
void DisplayEngine::unregisterGeometry(Geometry * pGeometry)
{
  m_pRegisteredGeometries->deleteObject(pGeometry, false);
}

// ------------------------------------------------------------------
// Name : getWindowData
// ------------------------------------------------------------------
void DisplayEngine::getWindowData()
{
  if (m_iWindow >= 0)  // windowed
  {
    m_pClientParams->winXPos = glutGet(GLUT_WINDOW_X);
    m_pClientParams->winYPos = glutGet(GLUT_WINDOW_Y);
  }
}

// ------------------------------------------------------------------
// Name : setAdditiveMode
// ------------------------------------------------------------------
bool DisplayEngine::setAdditiveMode(bool bAdd)
{
  bool bPrev = m_bAdditive;
  if (!bPrev && bAdd)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
    m_bAdditive = true;
  }
  else if (bPrev && !bAdd)
  {
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    m_bAdditive = false;
  }
  return bPrev;
}

// ------------------------------------------------------------------
// Name : setLookAtMode
// ------------------------------------------------------------------
void DisplayEngine::setLookAtMode(bool bLookAt)
{
	m_bLookAtMode = bLookAt;
  moveCameraTo(m_f3CamPos);
//  resizeWindow();
}

// ------------------------------------------------------------------
// Name : loadShader
// ------------------------------------------------------------------
bool DisplayEngine::loadShader(GLuint * uShader, GLenum type, const char * sShader)
{
  char sError[1024];
  *uShader = glCreateShader(type);
  if (*uShader == 0)
  {
    wsafecpy(sError, 1024, "Error in loadShader: can't create shader.");
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }

  // Open source file and read it
  char sFilePath[MAX_PATH] = SHADERS_PATH;
  strncat(sFilePath, sShader, MAX_PATH);
  FILE * f = NULL;
  errno_t err = fopen_s(&f, sFilePath, "r");
  if (err == ENOENT)
  {
    glDeleteShader(*uShader);
    *uShader = 0;
    snprintf(sError, 1024, "Error in loadShader: can't find source file %s.", sShader);
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }
  else if (err != 0)
  {
    glDeleteShader(*uShader);
    *uShader = 0;
    snprintf(sError, 1024, "Error in loadShader: can't read source file %s.", sShader);
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);
  char * sContent = new char[size+1];
  size = fread(sContent, sizeof(char), size, f);
  sContent[size] = '\0';
  fclose(f);

  // Compile shader
  GLint status = GL_TRUE;
  glShaderSource(*uShader, 1, (const GLchar**) &sContent, NULL);
  glCompileShader(*uShader);
  delete[] sContent;
  glGetShaderiv(*uShader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
  {
    char sLog[1024] = "";
    GLint size = 1024;  // wtf?
    glGetShaderInfoLog(*uShader, size, &size, sLog);
    snprintf(sError, 1024, "Error in loadShader: can't compile %s.\n", sShader);
    wsafecat(sError, 1024, sLog);
    m_pDebug->notifyErrorMessage(sError);
    glDeleteShader(*uShader);
    *uShader = 0;
    return false;
  }
  return true;
}

// ------------------------------------------------------------------
// Name : linkShaders
// ------------------------------------------------------------------
bool DisplayEngine::linkShaders(GLuint * uProgram, GLuint uVxShader, GLuint uPxShader)
{
  char sError[1024];
  if (uVxShader == 0 && uPxShader == 0)
  {
    snprintf(sError, 1024, "Error in linkShaders: invalid shaders.");
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }
  *uProgram = glCreateProgram();
  if (uVxShader != 0)
    glAttachShader(*uProgram, uVxShader);
  if (uPxShader != 0)
    glAttachShader(*uProgram, uPxShader);

  GLint status = GL_TRUE;
  glLinkProgram(*uProgram);
  glGetProgramiv(*uProgram, GL_LINK_STATUS, &status);
  if (status != GL_TRUE)
  {
    char sLog[1024] = "";
    GLint size = 1024;  // wtf?
    glGetProgramInfoLog(*uProgram, size, &size, sLog);
    snprintf(sError, 1024, "Error in linkShaders: can't link program.\n");
    wsafecat(sError, 1024, sLog);
    m_pDebug->notifyErrorMessage(sError);
    glDeleteProgram(*uProgram);
    *uProgram = 0;
    return false;
  }
  return true;
}
