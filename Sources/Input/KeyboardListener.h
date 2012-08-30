#ifndef _KEYBOARD_LISTENER_H
#define _KEYBOARD_LISTENER_H

class KeyboardListener
{
public:
  virtual bool onKeyDown(unsigned char c) { return false; };
  virtual bool onSpecialKeyDown(int key) { return false; };
};

#endif
