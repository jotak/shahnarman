// Avatars.cpp : définit le point d'entrée pour l'application console.
//

#ifdef WIN32
#include "stdafx.h"
//#define VLD_AGGREGATE_DUPLICATES
//#include <vld.h>
#include "../GameRoot.h"
#include "../LocalClient.h"
#include "../SystemHeaders.h"
#include "../Data/Parameters.h"
#include "../Display/DisplayEngine.h"
#include "../Input/PCInputEngine.h"
#include "../Interface/InterfaceManager.h"
#include "../Debug/DebugManager.h"

GameRoot * g_pMainGameRoot = NULL;
DWORD g_iLastUpdateTime = 0;
bool g_bIgnoreNextResize = false;
float g_fRestartGlutTimer = 0.0f;
int g_iOldW = -1;
int g_iOldH = -1;

// glut callbacks / system callbacks
void glutDisplay();
void glutIdle();
int cleanUp();
void glutResize(int w, int h);
void glutKeyboard(unsigned char key, int x, int y);
void glutMouseClick(int button, int state, int x, int y);
void glutMouseMove(int x, int y);
void glutSpecialKeyboard(int key, int x, int y);
void restartGlut();

int _tmain(int argc, char** argv)
{
  // First init functions
  glutInit(&argc, argv);
  _onexit(cleanUp);
  srand(timeGetTime());

  // Game root creation
  g_pMainGameRoot = new GameRoot();
  g_pMainGameRoot->Init();

  // register glut callbacks
  glutReshapeFunc(glutResize);
  glutDisplayFunc(glutDisplay);
  glutIdleFunc(glutIdle);
  glutKeyboardFunc(glutKeyboard);
  glutSpecialFunc(glutSpecialKeyboard);
  glutMouseFunc(glutMouseClick);
  glutMotionFunc(glutMouseMove);
  glutPassiveMotionFunc(glutMouseMove);

  // Start
  glutMainLoop();

  return 0;
}



//**************************************************************************
// GLUT/system CALLBACKS FUNCTIONS
//**************************************************************************

void glutResize(int w, int h)
{
  if (g_pMainGameRoot == NULL)
    return;
  if (w <= 0 || h <= 0)
    return;
  if (g_bIgnoreNextResize)
  {
    g_bIgnoreNextResize = false;
    return;
  }
  g_pMainGameRoot->m_pLocalClient->getClientParameters()->winWidth = w;
  g_pMainGameRoot->m_pLocalClient->getClientParameters()->winHeight = h;
  g_fRestartGlutTimer = 1.0f;
}

void glutDisplay()
{
  if (g_pMainGameRoot == NULL)
    return;
  g_pMainGameRoot->Display();
}

void glutIdle()
{
  if (g_pMainGameRoot == NULL)
    return;
  DWORD time = timeGetTime();
  double delta = (double)(time - g_iLastUpdateTime) / 1000;
  g_pMainGameRoot->Update(delta);
  glutPostRedisplay();
  g_iLastUpdateTime = time;
  if (g_fRestartGlutTimer > 0.0f)
  {
    g_fRestartGlutTimer -= (float) delta;
    if (g_fRestartGlutTimer <= 0.0f)
      restartGlut();
  }
}

int cleanUp()
{
  if (g_pMainGameRoot != NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDisplay()->getWindowData();
    g_pMainGameRoot->m_pLocalClient->getClientParameters()->saveParameters();
    delete g_pMainGameRoot;
  }
  return 0;
}

void glutKeyboard(unsigned char key, int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  if (key == 127)
    g_pMainGameRoot->m_pLocalClient->getDebug()->clear();
  ((PCInputEngine*)g_pMainGameRoot->m_pLocalClient->getInput())->onKeyboard(key, x, y);
}

void glutSpecialKeyboard(int key, int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  if (key == 1) // F1
    g_pMainGameRoot->m_pLocalClient->getDebug()->switchShowFPS();
  else if (key == 2) // F2
    g_pMainGameRoot->m_pLocalClient->getDisplay()->setLookAtMode(false);
  else if (key == 3) // F3
    g_pMainGameRoot->m_pLocalClient->getDisplay()->setLookAtMode(true);
  ((PCInputEngine*)g_pMainGameRoot->m_pLocalClient->getInput())->onSpecialKeyboard(key, x, y);
}

void glutMouseClick(int button, int state, int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  ((PCInputEngine*)g_pMainGameRoot->m_pLocalClient->getInput())->onMouseClick(button, state, x, y);
}

void glutMouseMove(int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  ((PCInputEngine*)g_pMainGameRoot->m_pLocalClient->getInput())->onMouseMove(x, y);
}

// OpenGL & glut context is lost after switching fullscreen on/off. So we have to reset it entirely.
void restartGlut()
{
  g_pMainGameRoot->m_pLocalClient->getDisplay()->initGlutWindow();
  // register glut callbacks again
  glutReshapeFunc(glutResize);
  glutDisplayFunc(glutDisplay);
  glutIdleFunc(glutIdle);
  glutKeyboardFunc(glutKeyboard);
  glutSpecialFunc(glutSpecialKeyboard);
  glutMouseFunc(glutMouseClick);
  glutMotionFunc(glutMouseMove);
  glutPassiveMotionFunc(glutMouseMove);
  if (g_iOldW > 0)
    g_pMainGameRoot->m_pLocalClient->getInterface()->onResize(g_iOldW, g_iOldH);
}

#endif
