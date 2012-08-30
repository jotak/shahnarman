#ifndef _INPUT_ENGINE_H
#define _INPUT_ENGINE_H

#include "EventListener.h"
#include <stack>

enum ButtonEvent
{
  Event_None = 0,
  Event_Down,
  Event_Up,
  Event_Click,
  Event_DoubleClick,
  Event_Drag
};

enum InputButton
{
  Button1 = 0,
  Button2,
  ButtonZ,
  ButtonX,
  // From this point, ButtonStart, only put "uncursored" buttons
  ButtonStart,
  ButtonLeft,
  ButtonRight,
  ButtonUp,
  ButtonDown,
  ButtonNext,
  ButtonPrev,
  ButtonBack,
  NbButtons
};

#define MAX_CURSORED_EVENT_LISTENERS     2

class ButtonAction
{
public:
  InputButton eButton;
  ButtonEvent eEvent;
  int xPos, yPos;
  int xPosInit, yPosInit;
  int xOffset, yOffset;
  unsigned long dragDistance;
  double clickTime;
  EventListener * pTargetListener;
};

class DebugManager;

class InputEngine
{
public:
  InputEngine(bool bHasKeyboard);
  ~InputEngine();

  virtual void update(double delta);
  CoordsScreen getCurrentCursorPosition();
  void addCursoredEventListener(EventListener * pListener, DebugManager * pDebug);
  void removeCursoredEventListener(EventListener * pListener);
  void pushUncursoredEventListener(EventListener * pListener);
  EventListener * popUncursoredEventListener();
  bool hasKeyboard() { return m_bHasKeyboard; };

protected:
  void onPressButton(InputButton eButton);
  void onReleaseButton(InputButton eButton);
  void onAnalogicMove(int x, int y);
  void submitEvent(int iEvent);

  ButtonAction m_AllButtons[NbButtons];
  EventListener * m_pCursoredEventListeners[MAX_CURSORED_EVENT_LISTENERS];  // Event listener that will be chosen function of cursor position
  std::stack<EventListener*> m_pUncursoredEventListeners;                   // Event listener that will be chosen regardless cursor position
  int m_iCursorX, m_iCursorY;
  bool m_bHasKeyboard;
};

#endif
