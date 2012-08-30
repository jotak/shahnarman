#ifndef _FXMANAGER_H
#define _FXMANAGER_H

#include "../Common/ObjectList.h"

class PhysicalObject;
class LocalClient;

class FxManager
{
public:
  // Constructor / destructor
  FxManager(LocalClient * pLocalClient);
  ~FxManager();

  // Manager functions
  void Init();
  void Update(double delta);
  void Display();
  void displayHUD();

  // Zoom effects
  void startInitialCamMove();
  void unzoom();
  void zoomToMapPos(CoordsMap mapPos);
  void cancelZoom();
  void zoom(bool bIn);
  void dragInertness(Coords3D inertnessVector);

  // Messages effects
  void showMessage(wchar_t * sText);

  // Other
  bool isGameIntroFinished();

private:
  LocalClient * m_pLocalClient;
  PhysicalObject * m_pCameraModifier;
  Coords3D m_Inertness;
  bool m_bInertness;
  ObjectList * m_pAllMessages;
  ObjectList * m_pTimeControllers;
  double m_dLastMessageStarted;
  bool m_bInitialCamMove;
  double m_fInitialMoveTimer;

  // Mini-game: throw the map!
  Coords3D m_TTM_PosInit;
  bool m_bTTM_ThrowingMap;
};

#endif
