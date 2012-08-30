// Avatars.cpp : définit le point d'entrée pour l'application console.
//
#ifdef LINUX
#include "../GameRoot.h"
#include "../SystemHeaders.h"

GameRoot * g_pMainGameRoot = NULL;
clock_t g_iLastUpdateTime;

void glutDisplay();
void glutIdle();
void cleanUp();
void glutResize(int w, int h);
void glutKeyboard(unsigned char key, int x, int y);
void glutMouseClick(int button, int state, int x, int y);
void glutMouseMove(int x, int y);


int main(int argc, char *argv[])
{
  // First init functions
  glutInit(&argc, argv);
  atexit(cleanUp);
  g_iLastUpdateTime = clock();
  srand(time(NULL));

  // Game root creation
  g_pMainGameRoot = new GameRoot();
  g_pMainGameRoot->Init();

  // register glut callbacks
  glutReshapeFunc(glutResize);
  glutDisplayFunc(glutDisplay);
  glutIdleFunc(glutIdle);
  glutKeyboardFunc(glutKeyboard);
  glutMouseFunc(glutMouseClick);
  glutMotionFunc(glutMouseMove);
  glutPassiveMotionFunc(glutMouseMove);

  // Start
  glutMainLoop();

  return EXIT_SUCCESS;
}

//**************************************************************************
// GLUT/system CALLBACKS FUNCTIONS
//**************************************************************************

void glutResize(int w, int h)
{
  if (g_pMainGameRoot == NULL)
    return;
  g_pMainGameRoot->m_pLocalClient->getClientParameters()->screenXSize = w;
  g_pMainGameRoot->m_pLocalClient->getClientParameters()->screenYSize = h;
  g_pMainGameRoot->m_pLocalClient->getDisplay()->resizeWindow();
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
  clock_t now = clock();
  double delta = (double)(now - g_iLastUpdateTime) / (double)CLOCKS_PER_SEC;
  g_pMainGameRoot->Update(delta);
  glutPostRedisplay();
  g_iLastUpdateTime = now;
}

void cleanUp()
{
  if (g_pMainGameRoot != NULL)
    delete g_pMainGameRoot;
}

void glutKeyboard(unsigned char key, int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  g_pMainGameRoot->m_pLocalClient->getInput()->onKeyboard(key, x, y);
}

void glutMouseClick(int button, int state, int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  g_pMainGameRoot->m_pLocalClient->getInput()->onMouseClick(button, state, x, y);
}

void glutMouseMove(int x, int y)
{
  if (g_pMainGameRoot == NULL)
    return;
  g_pMainGameRoot->m_pLocalClient->getInput()->onMouseMove(x, y);
}

#endif
