#ifndef _PC_INPUT_ENGINE_H
#define _PC_INPUT_ENGINE_H

#include "KeyboardInputEngine.h"

class PCInputEngine : public KeyboardInputEngine
{
public:
  // Constructor / destructor
  PCInputEngine();
  ~PCInputEngine();

  // Data providing functions
  virtual bool isShiftPressed();
  virtual bool isCtrlPressed();

  // Input glut functions (must only be called from lowlevel)
  void onKeyboard(unsigned char key, int x, int y);
  void onSpecialKeyboard(int key, int x, int y);
  void onMouseClick(int button, int state, int x, int y);
  void onMouseMove(int x, int y);

private:
  int m_iModifiers;
  unsigned char m_cZ, m_cX, m_cN, m_cB; // variable according to keyboard layout
};

#endif
