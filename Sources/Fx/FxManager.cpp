// -----------------------------------------------------------------
// FX MANAGER
// Gère les effets spéciaux, effets de caméra etc.
// -----------------------------------------------------------------
#include "FxManager.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Physics/FasloMove.h"
#include "../Physics/PhysicalObject.h"
#include "../Display/DisplayEngine.h"
#include "../Geometries/GeometryText.h"
#include "../Geometries/GeometryQuads.h"
#include "../Geometries/ModProgressiveScaling.h"
#include "../Common/TimeController.h"
#include "../Input/InputEngine.h"
#include "../Interface/InterfaceManager.h"

#define CAMMOVE_GLOBALZOOM        0
#define CAMMOVE_STEPBYSTEPZOOM    1
#define CAMMOVE_INERTNESS         2

#define CAMERA_UNLOOKAT           95.0f
#define CAMERA_ZOOMED_Z           99.5f
#define CAMERA_UNZOOMED_Z         98.0f
#define INIT_POINT                Coords3D(0, 25, 75)

#define INIT_TIMER_DELAY          1.0f;

#define MESSAGE_DELAY             3.0f
#define DELAY_BETWEEN_MESSAGES    1.0f

// -----------------------------------------------------------------
// Name : FxManager
// -----------------------------------------------------------------
FxManager::FxManager(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pCameraModifier = NULL;
  m_pAllMessages = new ObjectList(true);
  m_pTimeControllers = new ObjectList(true);
  m_dLastMessageStarted = 0;
  m_bTTM_ThrowingMap = false;
  m_bInertness = false;
  m_bInitialCamMove = false;
  m_fInitialMoveTimer = -1;
}

// -----------------------------------------------------------------
// Name : ~FxManager
// -----------------------------------------------------------------
FxManager::~FxManager()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy FxManager\n");
#endif
  FREE(m_pCameraModifier);
  delete m_pAllMessages;
  delete m_pTimeControllers;
#ifdef DBG_VERBOSE1
  printf("End destroy FxManager\n");
#endif
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void FxManager::Init()
{
  GeometryQuads * pMenuBg = m_pLocalClient->getInterface()->getMenuBackground();
  float scaleMax = 10.0f;
  float fCoef = (scaleMax - 1.0f) / INIT_TIMER_DELAY;
  ModProgressiveScaling * pMod = new ModProgressiveScaling(100, 1.0f, scaleMax, 1.0f, fCoef, m_pLocalClient->getClientParameters()->screenXSize/2, m_pLocalClient->getClientParameters()->screenYSize/2, PSB_Stop);
  pMenuBg->bindModifier(pMod);
//  pMenuBg->bindShader("startGameIntro.vert", "startGameIntro.frag");
//  gRadius = pMenuBg->registerShaderVariable("radius");
//  gCenter = pMenuBg->registerShaderVariable("center");
  //iTex = pMenuBg->registerShaderVariable("tex");
  //pMenuBg->beginSetShaderVariables();
  //pMenuBg->setShaderInt(gRadius, 0);
  //pMenuBg->setShaderIvec2(gCenter, CoordsScreen(m_pLocalClient->getClientParameters()->screenXSize/2, m_pLocalClient->getClientParameters()->screenYSize/2));
  //pMenuBg->setShaderInt(iTex, 0);
  //pMenuBg->endSetShaderVariables();
  //pMenuBg->activateShader();

  m_pCameraModifier = new PhysicalObject();
  m_bInertness = false;
  m_Inertness = Coords3D();
  startInitialCamMove();
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void FxManager::Update(double delta)
{
  if (m_bInertness)       // inertness wasn't set to false on last frame => the user stopped dragging
  {
    m_bInertness = false;
    Movement * pMvt = new FasloMove(CAMMOVE_INERTNESS, m_Inertness * 3.0f);
    m_pCameraModifier->bindMovement(pMvt);
    m_Inertness = Coords3D();
    m_TTM_PosInit = m_pLocalClient->getDisplay()->getCamera();
    m_bTTM_ThrowingMap = true;
  }
  else if (m_Inertness != Coords3D())
    m_bInertness = true;

  m_pCameraModifier->moveTo(m_pLocalClient->getDisplay()->getCamera());
  m_pCameraModifier->update(delta);
  Coords3D pos = m_pCameraModifier->get3DPos();
  // Game intro
  if (m_bInitialCamMove)
  {
    if (m_fInitialMoveTimer > 0)  // wait a bit before starting cam move
    {
      GeometryQuads * pMenuBg = m_pLocalClient->getInterface()->getMenuBackground();
      m_fInitialMoveTimer -= delta;
      if (m_fInitialMoveTimer <= 0)
      {
        Movement * pMvt = new FasloMove(CAMMOVE_GLOBALZOOM, Coords3D(0.0f, 0.0f, CAMERA_UNZOOMED_Z) - m_pLocalClient->getDisplay()->getCamera());
        m_pCameraModifier->bindMovement(pMvt);
        // Unbind menu bg modifier
        pMenuBg->unbindModifier(100, true, true);
      }
      else
        pMenuBg->update(delta);
    }
    else if (pos.z >= CAMERA_UNLOOKAT)  // Check if intro cam move is finished
    {
      m_bInitialCamMove = false;
      pos.x = pos.y = 0;
      m_pCameraModifier->moveTo(pos);
      m_pLocalClient->getDisplay()->setLookAtMode(false);
      unzoom();
    }
    else  // Intro cam trajectory
    {
      pos.x = 0;
      pos.y = 0.0625f * pos.z * pos.z - 11.875f * pos.z + 564.0625f;  // Formula given by calculation in document: "calculs_traj_init.ods"
      m_pCameraModifier->moveTo(pos);
    }
  }
  else {
    // Z-bounded cam (except during intro)
    if (pos.z < BOARDPLANE - 5.0f)
    {
      pos.z = BOARDPLANE - 5.0f;
      m_pCameraModifier->moveTo(pos);
    }
    else if (pos.z > BOARDPLANE - 0.15f)
    {
      pos.z = BOARDPLANE - 0.15f;
      m_pCameraModifier->moveTo(pos);
    }
  }
  m_pLocalClient->getDisplay()->moveCameraTo(m_pCameraModifier->get3DPos());
  m_pCameraModifier->unbindInactiveMovements(true);

  // Mini-game: throw the map!
  if (m_bTTM_ThrowingMap)
  {
    Movement * pMvt = m_pCameraModifier->findMovement(CAMMOVE_INERTNESS);
    if (!pMvt)
    {
      m_bTTM_ThrowingMap = false;
      Coords3D diff = m_pLocalClient->getDisplay()->getCamera() - m_TTM_PosInit;
      long score = (diff.x * diff.x + diff.y * diff.y) / 10;
      if (score >= 500)
      {
        void * p = &score;
        char sText[256];
        i18n->getText("YOU_GOT_%$1ld_AT_THROW_THE_MAP", sText, 256, &p);
        char sComment[256];
        if (score < 3000)
          i18n->getText("TTH_SCORE_1", sComment, 256);
        else if (score < 10000)
          i18n->getText("TTH_SCORE_2", sComment, 256);
        else if (score < 100000)
          i18n->getText("TTH_SCORE_3", sComment, 256);
        else if (score < 500000)
          i18n->getText("TTH_SCORE_4", sComment, 256);
        else if (score < 1000000)
          i18n->getText("TTH_SCORE_5", sComment, 256);
        else
          i18n->getText("TTH_SCORE_6", sComment, 256);
        showMessage(sComment);
        showMessage(sText);
      }
    }
  }

  // Update messages geometry
  Geometry * pGeo = (Geometry*) m_pAllMessages->getFirst(0);
  while (pGeo != NULL)
  {
    pGeo->update(delta);
    pGeo = (Geometry*) m_pAllMessages->getNext(0);
  }

  // Update time controllers
  if (m_dLastMessageStarted > 0)
    m_dLastMessageStarted -= delta;
  TimeController * pCtrl = (TimeController*) m_pTimeControllers->getFirst(0);
  while (pCtrl != NULL)
  {
    bool bToNext = true;
    switch (m_pTimeControllers->getCurrentType(0))
    {
    case 1: // message
      {
        if (pCtrl->getState() == TC_Stopped && m_dLastMessageStarted <= 0)
        {
          // This message is waiting to be started, and we're ready
          pCtrl->start(MESSAGE_DELAY);
          m_dLastMessageStarted = DELAY_BETWEEN_MESSAGES;
        }
        pCtrl->update(delta);
        if (pCtrl->getState() == TC_DelayReached)
        {
          m_pAllMessages->deleteObject(pCtrl->getAttachment(), true);
          pCtrl = (TimeController*) m_pTimeControllers->deleteCurrent(0, true);
          bToNext = false;
          break;
        }
      }
    }
    if (bToNext)
      pCtrl = (TimeController*) m_pTimeControllers->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void FxManager::Display()
{
}

// -----------------------------------------------------------------
// Name : displayHUD
// -----------------------------------------------------------------
void FxManager::displayHUD()
{
  // Logs / messages
  GeometryText * pGeo = (GeometryText*) m_pAllMessages->getFirst(0);
  while (pGeo != NULL)
  {
    TimeController * pCtrl = (TimeController*) pGeo->getAttachment();
    if (pCtrl->getState() != TC_Stopped)
    {
      int y = 100 - 100.0f * pCtrl->getTimer() / MESSAGE_DELAY;
      float color = 1.0f - 0.3f * pCtrl->getTimer() / MESSAGE_DELAY;
      float alpha = 1.0f - 0.5f * pCtrl->getTimer() / MESSAGE_DELAY;
      pGeo->display(CoordsScreen(100, y, GUIPLANE), rgba(1, color, color, alpha));
    }
    pGeo = (GeometryText*) m_pAllMessages->getNext(0);
  }

  // Intro: display menu bg with alpha value
  if (m_bInitialCamMove)
  {
    if (m_fInitialMoveTimer > 0)
    {
      float fAlpha = m_fInitialMoveTimer / INIT_TIMER_DELAY;
      GeometryQuads * pMenuBg = m_pLocalClient->getInterface()->getMenuBackground();
      pMenuBg->display(CoordsScreen(0, 0, GUIPLANE), rgba(1, 1, 1, fAlpha*fAlpha));
    }
  }
}

// -----------------------------------------------------------------
// Name : startInitialCamMove
// -----------------------------------------------------------------
void FxManager::startInitialCamMove()
{
  m_bInitialCamMove = true;
  m_pCameraModifier->moveTo(INIT_POINT);
  m_pLocalClient->getDisplay()->moveCameraTo(m_pCameraModifier->get3DPos());
  m_pLocalClient->getDisplay()->setLookAtMode(true);
  m_pCameraModifier->unbindMovement(CAMMOVE_GLOBALZOOM, true, true);
  m_fInitialMoveTimer = INIT_TIMER_DELAY;
}

// -----------------------------------------------------------------
// Name : unzoom
// -----------------------------------------------------------------
void FxManager::unzoom()
{
  if (m_bInitialCamMove)
    return;
  m_pCameraModifier->unbindMovement(CAMMOVE_GLOBALZOOM, true, true);
  Movement * pMvt = new FasloMove(CAMMOVE_GLOBALZOOM, Coords3D(0.0f, 0.0f, CAMERA_UNZOOMED_Z) - m_pLocalClient->getDisplay()->getCamera());
  m_pCameraModifier->bindMovement(pMvt);
}

// -----------------------------------------------------------------
// Name : zoomToMapPos
// -----------------------------------------------------------------
void FxManager::zoomToMapPos(CoordsMap mapPos)
{
  Coords3D goal = m_pLocalClient->getDisplay()->get3DCoords(mapPos, CAMERA_ZOOMED_Z);
  m_pCameraModifier->unbindMovement(CAMMOVE_GLOBALZOOM, true, true);
  Movement * pMvt = new FasloMove(CAMMOVE_GLOBALZOOM, goal - m_pLocalClient->getDisplay()->getCamera());
  m_pCameraModifier->bindMovement(pMvt);
}

// -----------------------------------------------------------------
// Name : cancelZoom
// -----------------------------------------------------------------
void FxManager::cancelZoom()
{
  m_pCameraModifier->unbindAllMovements(true);
}

// -----------------------------------------------------------------
// Name : zoom
// -----------------------------------------------------------------
void FxManager::zoom(bool bIn)
{
  float fStep = m_pLocalClient->getClientParameters()->fZoomStep;
  if (m_pLocalClient->getDisplay()->getCamera().z >= 8.0f)
    fStep /= 4.0f;
  else if (m_pLocalClient->getDisplay()->getCamera().z >= 6.0f)
    fStep /= 2.0f;
  CoordsScreen mouse = m_pLocalClient->getInput()->getCurrentCursorPosition();
  mouse.z = BOARDPLANE;
  Coords3D mouse3D = m_pLocalClient->getDisplay()->get3DCoords(mouse, DMS_3D);
  Coords3D cam = m_pLocalClient->getDisplay()->getCamera();
  float dx, dy;
  if (BOARDPLANE - cam.z < 0.25f)
    dx = dy = 0;
  else
  {
    dx = (mouse3D.x - cam.x) * fStep / (BOARDPLANE - cam.z);
    dy = (mouse3D.y - cam.y) * fStep / (BOARDPLANE - cam.z);
  }
  Coords3D vector(bIn ? dx : -dx, bIn ? dy : -dy, bIn ? fStep : -fStep);
  m_pCameraModifier->unbindMovement(CAMMOVE_GLOBALZOOM, true, true);
  Movement * pMvt = new FasloMove(CAMMOVE_STEPBYSTEPZOOM, vector, 4.0f);
  m_pCameraModifier->bindMovement(pMvt);
}

// -----------------------------------------------------------------
// Name : dragInertness
// -----------------------------------------------------------------
void FxManager::dragInertness(Coords3D inertnessVector)
{
  m_Inertness = inertnessVector;
  m_bInertness = false;
  m_pCameraModifier->unbindAllMovements(true);
}

// -----------------------------------------------------------------
// Name : showMessage
// -----------------------------------------------------------------
void FxManager::showMessage(char * sText)
{
  int font = m_pLocalClient->getDisplay()->getFontEngine()->registerFont("Arabolical_32", m_pLocalClient->getDisplay()->getTextureEngine());
  GeometryText * pGeo = new GeometryText(sText, font, VB_Static, m_pLocalClient->getDisplay());
  m_pAllMessages->addLast(pGeo);
  TimeController * pCtrl = new TimeController();
  pCtrl->setAttachment(pGeo);
  pGeo->setAttachment(pCtrl);
  m_pTimeControllers->addLast(pCtrl, 1);  // Stands for "type is message"
}

// -----------------------------------------------------------------
// Name : isGameIntroFinished
// -----------------------------------------------------------------
bool FxManager::isGameIntroFinished()
{
  return m_bInitialCamMove == false;
}
